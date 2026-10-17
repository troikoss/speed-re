#!/usr/bin/env python3
"""Observe rendered pause-icon motion; native integration, separate from PC equivalence."""
import argparse
from pathlib import Path
import subprocess
from native_gui_test import add_render_arguments, render_environment
ROOT = Path(__file__).resolve().parents[1]


def icon_centre(path):
    magic, dimensions, maximum, pixels = path.read_bytes().split(b'\n', 3)
    width, height = map(int, dimensions.split())
    assert magic == b'P6' and maximum == b'255' and len(pixels) == width * height * 3
    # Track the Options texture rectangle; the green selection frame stays fixed.
    longest = (0, 0, 0)
    for y in range(int(height * .431), int(height * .445)):
        start = None
        for x in range(int(width * .42), int(width * .82)):
            r, g, b = pixels[(y * width + x) * 3:(y * width + x) * 3 + 3]
            bright = min(r, g, b) > 145 and g - r < 30 and g - b < 65
            if bright and start is None:
                start = x
            elif not bright and start is not None:
                if x - start > longest[0]: longest = (x - start, start, x - 1)
                start = None
    assert longest[0] > width * .025, ('missing Options icon edge', path, longest)
    return (longest[1] + longest[2]) * .5, width


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--demo', type=Path, default=ROOT / 'build/native/ug2demo')
    parser.add_argument('--game', type=Path, default=ROOT / 'Need for Speed Underground 2')
    add_render_arguments(parser)
    args = parser.parse_args()
    out = ROOT / 'build/analysis/native-pause-scroller'; out.mkdir(parents=True, exist_ok=True)
    centres = []
    for frames in (53, 60, 75):
        image = out / ('motion-%d.ppm' % frames)
        image.unlink(missing_ok=True)
        result = subprocess.run([str(args.demo), '--game', str(args.game), '--car', 'SKYLINE', '--frames', str(frames),
                                 '--pause-at', '20', '--pause-keys', 'R', '--screenshot', str(image)], cwd=ROOT,
                                env=render_environment(args, UG2_DEBUG_PAUSE='1'), capture_output=True, text=True, timeout=90)
        log = result.stdout + result.stderr; (out / ('motion-%d.log' % frames)).write_text(log)
        assert result.returncode == 0 and 'unrecovered original code' not in log, (frames, result.returncode)
        centre, width = icon_centre(image); centres.append(centre)
        print('PASS rendered selected-icon centre at frame %d: %.1f' % (frames, centre), flush=True)
    assert centres[0] > centres[1] > centres[2], centres
    assert centres[0] - centres[2] > width * .05, centres
    assert abs(centres[2] - width * .5) < width * .015, centres
    print('PASS pause-icon motion and settled centring (integration only)')


if __name__ == '__main__': main()
