#!/usr/bin/env python3
"""Re-verify every auto-matched function: recompile with MSVC 7.1 and compare with speed2.exe.

Reads config/SPEED2_EXE_1_2/auto-matched.json. For each entry: the original bytes must still hash
to the manifest value, and the source file must compile (flags from the manifest) to the same
bytes, relocation operands masked. Exits non-zero on any mismatch. Byte matching only; no
behavioural claim. Run sequentially with the other wibo verifiers. Usage: verify_auto_matched.py [--jobs N]
"""
import argparse
import concurrent.futures as cf
import hashlib
import json
import sys
import tempfile
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import auto_match  # noqa: E402


def check(item):
    key, e, orig = item
    addr = int(key, 16)
    if hashlib.sha256(orig.rstrip(b'\xcc\x90')).hexdigest() != e['sha256']:
        return key, False, 'original bytes changed'
    src = (ROOT / e['file']).read_text()
    opts = e['flags'].split(' dat=')[0].split()
    work = Path(tempfile.mkdtemp(prefix='av', dir=ROOT / 'build'))
    ok, msg = auto_match.compile_and_compare(addr, src, opts, orig, work)
    return key, ok, msg


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--jobs', type=int, default=4)
    a = ap.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/auto-matched.json').read_text())
    pe = pefile.PE(str(ROOT / 'orig/SPEED2_EXE_1_2/speed2.exe'))
    image = pe.get_memory_mapped_image()
    items = [(k, e, bytes(image[int(k, 16) - 0x400000:int(k, 16) - 0x400000 + e['size']])) for k, e in manifest['functions'].items()]
    bad = 0
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        for key, ok, msg in ex.map(check, items):
            if not ok:
                bad += 1
                print('FAIL', key, msg)
    print('%d functions, %d failed' % (len(items), bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
