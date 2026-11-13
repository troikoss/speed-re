#!/usr/bin/env python3
"""Re-verify the behaviour-only tier (config/SPEED2_EXE_1_2/behavior-matched.json).

For each entry: the original bytes must still hash to the manifest value, and the source file must
still be differentially equivalent to the original on random inputs (tools/diff_test.py, seeds that
differ from the acceptance run so a lucky acceptance is unlikely to repeat). Behavioural evidence
only; the functions are NOT byte-identical. Exit status is non-zero on any failure.
Usage: verify_behavior_matched.py [--jobs N] [--trials N]
"""
import argparse
import concurrent.futures as cf
import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import auto_match as am  # noqa: E402
import diff_test  # noqa: E402


def check(job):
    key, entry, trials, seed = job
    addr = int(key, 16)
    am.INDEX, am.IMAGE = am.INDEX or am.load_inputs()[0], am.IMAGE or am.load_inputs()[1]
    f = am.INDEX[addr]
    body = bytes(am.IMAGE[addr - 0x400000:addr - 0x400000 + f['size']])
    if hashlib.sha256(body.rstrip(b'\xcc\x90')).hexdigest() != entry['sha256']:
        return key, False, 'original bytes changed'
    verdict, detail = diff_test.test_function(addr, (ROOT / entry['file']).read_text(), trials=trials, seed=seed)
    return key, verdict == 'equivalent', verdict + ' ' + detail


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--trials', type=int, default=48)
    ap.add_argument('--seed', type=int, default=977, help='use a different seed for each re-check so passes are independent')
    a = ap.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/behavior-matched.json').read_text())
    am.INDEX, am.IMAGE = am.load_inputs()
    jobs = [(k, v, a.trials, a.seed) for k, v in manifest['functions'].items()]
    bad = 0
    with cf.ProcessPoolExecutor(a.jobs) as ex:
        for key, ok, msg in ex.map(check, jobs):
            if not ok:
                bad += 1
                print('FAIL', key, msg)
    print('%d behaviour-tier functions, %d failed' % (len(jobs), bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
