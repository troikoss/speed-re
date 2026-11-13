#!/usr/bin/env python3
"""Promote byte-identical auto matches into src/ and the manifest.

Reads build/auto_match/results.jsonl + matched/*.c (from tools/auto_match.py), and for each match
writes `src/Speed/Indep/Recovered/<band>/FUN_<addr>.c` with a provenance header, and records
address, size, SHA-256 of the original bytes and compiler flags in
`config/SPEED2_EXE_1_2/auto-matched.json`. Existing entries are kept (idempotent).

Matches are byte-identical to speed2.exe under MSVC 7.1 (call targets masked). They are NOT yet
named, typed or integrated: parameter/global types are decompiler guesses and callee conventions are
inferred from callee bytes. Rename and retype when a function is understood.
"""
import hashlib
import json
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/auto-matched.json'
DEST = ROOT / 'src/Speed/Indep/Recovered'


def main():
    pe = pefile.PE(str(ROOT / 'orig/SPEED2_EXE_1_2/speed2.exe'))
    image = pe.get_memory_mapped_image()
    index = {f['start']: f for f in json.loads((ROOT / 'build/analysis/function-index.json').read_text())['functions']}
    exe_sha = hashlib.sha256((ROOT / 'orig/SPEED2_EXE_1_2/speed2.exe').read_bytes()).hexdigest()
    manifest = json.loads(MANIFEST.read_text()) if MANIFEST.exists() else {'executable_sha256': exe_sha, 'functions': {}}
    assert manifest['executable_sha256'] == exe_sha
    added = 0
    for line in (ROOT / 'build/auto_match/results.jsonl').read_text().splitlines():
        r = json.loads(line)
        if r['status'] != 'match':
            continue
        addr = int(r['addr'], 16)
        key = '0x%06x' % addr
        if key in manifest['functions']:
            continue
        src = (ROOT / 'build/auto_match/matched' / ('%08x.c' % addr)).read_text()
        f = index[key] if key in index else index['0x%x' % addr]
        body = bytes(image[addr - 0x400000:addr - 0x400000 + f['size']])
        band = '0x%02x' % (addr >> 16)
        out = DEST / band / ('FUN_%08x.c' % addr)
        out.parent.mkdir(parents=True, exist_ok=True)
        info = r['info']
        header = ('// Original UG2 PC v1.2 0x%x, %d bytes. Auto-matched: byte-identical to speed2.exe under\n'
                  '// MSVC 7.1 (%s), call targets masked. Source is Ghidra decompiler output with an inferred\n'
                  '// prelude; names, types and callee conventions are NOT verified beyond the bytes.\n'
                  '// Do not build into ug2recovered: globals are absolute-address macros.\n' % (addr, f['size'], info))
        out.write_text(header + src)
        manifest['functions'][key] = {'size': f['size'], 'sha256': hashlib.sha256(body.rstrip(b'\xcc\x90')).hexdigest(),
                                      'flags': info, 'file': str(out.relative_to(ROOT))}
        added += 1
    manifest['functions'] = dict(sorted(manifest['functions'].items()))
    MANIFEST.write_text(json.dumps(manifest, indent=1) + '\n')
    print('added', added, 'total', len(manifest['functions']))


if __name__ == '__main__':
    main()
