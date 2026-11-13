#!/usr/bin/env python3
"""Behaviour pass: accept near-miss functions that are equivalent to the original on random inputs.

For results in build/auto_match/results.jsonl that compiled but did not byte-match, rebuild the
wrapped source (tools/auto_match.py) and run tools/diff_test.py. Equivalent functions are written to
`src/Speed/Indep/Recovered/Behavior/<band>/FUN_<addr>.c` and recorded in
`config/SPEED2_EXE_1_2/behavior-matched.json`. THIS IS WEAKER THAN BYTE MATCHING: random-input
differential execution in Unicorn, with branch coverage limited by the generated inputs. Keep the two
tiers separate in every report.

Usage: behavior_pass.py [--limit N] [--jobs N] [--max-size N] [--trials N] [--decomp FILE]
"""
import argparse
import concurrent.futures as cf
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import auto_match as am  # noqa: E402
import diff_test  # noqa: E402

MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/behavior-matched.json'


def work(job):
    addr, c, trials = job
    am.INDEX, am.IMAGE = am.INDEX or am.load_inputs()[0], am.IMAGE or am.load_inputs()[1]
    for dt in ('undefined4', 'float'):
        src, why = am.build_source(addr, c, am.INDEX, am.IMAGE, dt)
        if src is None:
            return addr, 'skip', why, None
        try:
            verdict, detail = diff_test.test_function(addr, src, trials=trials)
        except Exception as e:  # emulator/linker problems are not verdicts
            return addr, 'error', repr(e)[:100], None
        if verdict == 'equivalent':
            return addr, verdict, detail, src
        if verdict != 'different':
            return addr, verdict, detail, None
    return addr, 'different', detail, None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--limit', type=int, default=200)
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--max-size', type=int, default=400)
    ap.add_argument('--trials', type=int, default=40)
    ap.add_argument('--decomp', default='build/ghidra/decomp3.jsonl')
    a = ap.parse_args()
    am.INDEX, am.IMAGE = am.load_inputs()
    results = {}
    for line in (ROOT / 'build/auto_match/results.jsonl').read_text().splitlines():
        try:
            r = json.loads(line)
        except ValueError:
            continue
        results[int(r['addr'], 16)] = r
    manifest = json.loads(MANIFEST.read_text()) if MANIFEST.exists() else {'functions': {}, 'tried': {}}
    manifest.setdefault('tried', {})
    jobs = []
    for line in (ROOT / a.decomp).read_text().splitlines():
        try:
            e = json.loads(line)
        except ValueError:
            continue
        if 'c' not in e:
            continue
        addr = int(e['a'], 16)
        r = results.get(addr)
        f = am.INDEX.get(addr)
        key = '0x%06x' % addr
        if not f or f['size'] > a.max_size or key in manifest['functions'] or key in manifest['tried']:
            continue
        if not r or r['status'] != 'miss' or r['info'].startswith('f0') or 'error C' in r['info']:
            continue
        jobs.append((addr, e['c'], a.trials))
        if len(jobs) >= a.limit:
            break
    stats = {}
    with cf.ProcessPoolExecutor(a.jobs) as ex:
        for addr, verdict, detail, src in ex.map(work, jobs):
            stats[verdict] = stats.get(verdict, 0) + 1
            key = '0x%06x' % addr
            if verdict == 'equivalent':
                f = am.INDEX[addr]
                body = bytes(am.IMAGE[addr - 0x400000:addr - 0x400000 + f['size']])
                out = ROOT / 'src/Speed/Indep/Recovered/Behavior' / ('0x%02x' % (addr >> 16)) / ('FUN_%08x.c' % addr)
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_text('// Original UG2 PC v1.2 0x%x, %d bytes. BEHAVIOUR-VERIFIED ONLY (%s):\n'
                               '// not byte-identical to speed2.exe; equivalent to the original on random inputs under\n'
                               '// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.\n'
                               % (addr, f['size'], detail) + src)
                manifest['functions'][key] = {'size': f['size'], 'sha256': hashlib.sha256(body.rstrip(b'\xcc\x90')).hexdigest(),
                                              'evidence': detail, 'file': str(out.relative_to(ROOT))}
            else:
                manifest['tried'][key] = verdict
    manifest['functions'] = dict(sorted(manifest['functions'].items()))
    MANIFEST.write_text(json.dumps(manifest, indent=1) + '\n')
    print(stats)


if __name__ == '__main__':
    main()
