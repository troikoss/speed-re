#!/usr/bin/env python3
"""Headless engine-sound smoke test for the native demo (native integration, not original-code equivalence).

Runs build/native/ug2demo with SDL's disk audio driver and the scripted throttle profile, then reads the raw S16
stereo output the driver writes. Checks:
- the output is not silent: overall RMS above a threshold, and the audio callbacks released packets to the
  synthesizer with few underrun frames (the demo prints these counters);
- the pitch follows the engine speed: 100 ms windows of the output have a zero-crossing frequency, and the rpm of
  each window is the rpm the demo logged for the same moment. Both timelines are SDL ticks: the device timeline starts
  at the audio open tick recorded in the log header. Requires a positive Spearman correlation and a higher mean pitch
  at high rpm than at low rpm.

Logs and the raw output stay in the ignored build/analysis/ginsu directory.
"""
import argparse
import bisect
import math
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/analysis/ginsu'
RATE = 44100
WINDOW = 0.1
RMS_THRESHOLD = 200.0          # S16 LSB; a silent or inaudible output stays well below this
MIN_CORRELATION = 0.5
MAX_UNDERRUN_FRACTION = 0.01
COUNTERS = re.compile(r'ug2demo: audio (\d+) callbacks, (\d+) packets released, (\d+) underrun frames')


def run_demo(game, frames, gin, raw_path, log_path):
    env = dict(os.environ)
    for key in ('DISPLAY', 'WAYLAND_DISPLAY'):
        env.pop(key, None)
    env.update(SDL_VIDEODRIVER='offscreen', LIBGL_ALWAYS_SOFTWARE='1', SDL_AUDIODRIVER='disk',
               SDL_DISKAUDIOFILE=str(raw_path), UG2_ENGINE_LOG=str(log_path))
    command = [str(ROOT / 'build/native/ug2demo'), '--game', game, '--car', 'SKYLINE', '--scripted',
               '--frames', str(frames), '--engine-gin', gin, '--no-hud', '--width', '640', '--height', '360']
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=900)
    (OUT / 'engine-smoke-demo.log').write_text(result.stdout + result.stderr)
    return result


def read_rpm_log(path):
    start = None
    ticks, rpms = [], []
    for line in path.read_text().splitlines():
        if line.startswith('# audio_start_ticks'):
            start = int(line.split()[-1])
            continue
        fields = line.split()
        if len(fields) == 3:
            ticks.append(int(fields[0]))
            rpms.append(float(fields[2]))
    if start is None or not ticks:
        raise SystemExit('engine log has no audio start or no frames: %s' % path)
    return start, ticks, rpms


def window_stats(raw):
    frames = len(raw) // 4
    samples = struct.unpack('<%dh' % (2 * frames), raw[:4 * frames])
    total = sum(s * s for s in samples)
    overall_rms = math.sqrt(total / len(samples)) if samples else 0.0
    mono = [(samples[2 * i] + samples[2 * i + 1]) * 0.5 for i in range(frames)]
    size = int(RATE * WINDOW)
    windows = []
    for start in range(0, frames - size + 1, size):
        block = mono[start:start + size]
        crossings = sum(1 for i in range(1, len(block)) if (block[i - 1] < 0) != (block[i] < 0))
        rms = math.sqrt(sum(x * x for x in block) / len(block))
        pitch = crossings / 2.0 / WINDOW
        windows.append(((start + size / 2.0) / RATE, rms, pitch))
    return overall_rms, windows


def ranks(values):
    order = sorted(range(len(values)), key=lambda i: values[i])
    result = [0.0] * len(values)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and values[order[j + 1]] == values[order[i]]:
            j += 1
        for k in range(i, j + 1):
            result[order[k]] = (i + j) / 2.0 + 1.0
        i = j + 1
    return result


def spearman(xs, ys):
    rx, ry = ranks(xs), ranks(ys)
    n = len(xs)
    mx, my = sum(rx) / n, sum(ry) / n
    cov = sum((a - mx) * (b - my) for a, b in zip(rx, ry))
    vx = math.sqrt(sum((a - mx) ** 2 for a in rx))
    vy = math.sqrt(sum((b - my) ** 2 for b in ry))
    return cov / (vx * vy) if vx > 0 and vy > 0 else 0.0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--game', default=os.environ.get('UG2_DIR', ''), help='game directory (default: $UG2_DIR)')
    parser.add_argument('--frames', type=int, default=600)
    parser.add_argument('--gin', default='GIN_Nissan_Skyline.gin')
    parser.add_argument('--reuse', action='store_true', help='analyse the existing output without running the demo')
    args = parser.parse_args()
    if not args.game:
        raise SystemExit('set UG2_DIR or pass --game')
    OUT.mkdir(parents=True, exist_ok=True)
    raw_path = OUT / 'engine.raw'
    log_path = OUT / 'engine-rpm.log'
    if not args.reuse:
        for path in (raw_path, log_path):
            path.unlink(missing_ok=True)
        result = run_demo(args.game, args.frames, args.gin, raw_path, log_path)
        counters = COUNTERS.search(result.stdout + result.stderr)
        if result.returncode != 0 or counters is None:
            raise SystemExit('demo failed (exit %d)\n%s' % (result.returncode, result.stderr[-2000:]))
        callbacks, releases, underruns = (int(value) for value in counters.groups())
    else:
        callbacks = releases = underruns = None
        text = (OUT / 'engine-smoke-demo.log').read_text() if (OUT / 'engine-smoke-demo.log').exists() else ''
        counters = COUNTERS.search(text)
        if counters is not None:
            callbacks, releases, underruns = (int(value) for value in counters.groups())

    raw = raw_path.read_bytes()
    start, ticks, rpms = read_rpm_log(log_path)
    overall_rms, windows = window_stats(raw)
    duration = len(raw) / (4.0 * RATE)
    frames_out = len(raw) // 4
    underrun_fraction = (underruns / frames_out) if (underruns is not None and frames_out) else float('nan')

    paired = []
    for centre, rms, pitch in windows:
        index = bisect.bisect_right(ticks, start + centre * 1000.0) - 1
        if index < 0:
            continue
        paired.append((centre, rms, pitch, rpms[index]))
    if len(paired) < 10:
        raise SystemExit('too few windows paired with rpm: %d' % len(paired))
    pitches = [p for _, _, p, _ in paired]
    speeds = [r for _, _, _, r in paired]
    rho = spearman(speeds, pitches)
    ordered = sorted(paired, key=lambda item: item[3])
    third = len(ordered) // 3
    low = sum(p for _, _, p, _ in ordered[:third]) / third
    high = sum(p for _, _, p, _ in ordered[-third:]) / third

    print('audio: %.2f s, %d stereo frames, overall RMS %.1f LSB' % (duration, frames_out, overall_rms))
    print('callbacks %s, packets released %s, underrun frames %s (%.4f of the output)'
          % (callbacks, releases, underruns, underrun_fraction))
    print('rpm range in windows: %.0f .. %.0f over %d windows of %.0f ms' % (min(speeds), max(speeds), len(paired), WINDOW * 1000))
    print('zero-crossing pitch: low-rpm third %.1f Hz, high-rpm third %.1f Hz' % (low, high))
    print('Spearman correlation of pitch with rpm: %.3f' % rho)

    failures = []
    if overall_rms <= RMS_THRESHOLD:
        failures.append('output is silent (RMS %.1f <= %.1f)' % (overall_rms, RMS_THRESHOLD))
    if releases is None or releases == 0:
        failures.append('the player released no packets')
    if underruns is not None and underrun_fraction > MAX_UNDERRUN_FRACTION:
        failures.append('underrun fraction %.4f above %.2f' % (underrun_fraction, MAX_UNDERRUN_FRACTION))
    if rho < MIN_CORRELATION:
        failures.append('pitch does not follow rpm (Spearman %.3f < %.2f)' % (rho, MIN_CORRELATION))
    if high <= low:
        failures.append('pitch at high rpm is not above pitch at low rpm')
    if failures:
        print('FAIL: ' + '; '.join(failures))
        sys.exit(1)
    print('PASS')


if __name__ == '__main__':
    main()
