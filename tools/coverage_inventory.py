#!/usr/bin/env python3
"""Coverage inventory: which functions of the UG2 PC v1.2 executable have recovered source.

Reads the local retail SPEED2.EXE (never written, never copied), discovers function entry
points, and compares them with the original addresses cited in `src/` and `config/`.
Writes a Markdown report plus a JSON list of unresolved functions.

A function counts as *cited* when its entry address appears in a recovery comment, a
manifest or a verifier address list. Citation is NOT proof of a verified recovery (see
docs/DECOMPILATION_SCOPE.md); it only bounds how much is still unaccounted for.

Usage: coverage_inventory.py SPEED2.EXE [--report docs/COVERAGE.md] [--json out.json]
                             [--names yugecin-nfsu2-re/docs/funcs.html]
                             [--index build/analysis/function-index.json]

--index uses the function boundaries and names from tools/recover_pc_index.py (community IDC
seeds) instead of this script's own recursive-descent discovery.

--names reads the function list of https://github.com/yugecin/nfsu2-re (community names, an
analysis lead only, not evidence) so unresolved functions can be labelled and grouped.
Requires: pip install capstone pefile
"""
import argparse
import bisect
import collections
import json
import re
import struct
import sys
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM

ROOT = Path(__file__).resolve().parents[1]
ADDR_RE = re.compile(r'(?<![0-9A-Za-z_])0[xX]([4-8][0-9a-fA-F]{5})(?![0-9A-Za-z_])')


def load(exe):
    pe = pefile.PE(str(exe))
    base = pe.OPTIONAL_HEADER.ImageBase
    secs = []
    for s in pe.sections:
        secs.append((s.Name.rstrip(b'\0').decode('latin1'), base + s.VirtualAddress,
                     max(s.Misc_VirtualSize, s.SizeOfRawData), s.get_data(), s.Characteristics))
    return base, pe.OPTIONAL_HEADER.AddressOfEntryPoint + base, secs


def discover(base, entry, secs):
    text = next(s for s in secs if s[0] == '.text')
    tva, tsz, tdata = text[1], len(text[3]), text[3]
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True

    def in_text(a):
        return tva <= a < tva + tsz

    funcs = {entry}
    # seeds: code pointers stored in data sections (vtables, callback tables)
    for name, va, _, data, ch in secs:
        if ch & 0x20000000:
            continue
        for off in range(0, len(data) - 3, 4):
            v = struct.unpack_from('<I', data, off)[0]
            if in_text(v) and tdata[v - tva] in (0x55, 0x53, 0x56, 0x57, 0x83, 0x81, 0x8b, 0x6a, 0x64, 0xb8, 0x33, 0x51, 0x52, 0xa1, 0x68, 0xe9, 0xff, 0xc3, 0x31, 0x8a, 0x80, 0xd9, 0xdd, 0xf6):
                funcs.add(v)
    cov = bytearray(tsz)
    queue = collections.deque(funcs)
    seen = set(funcs)
    while queue:
        va = queue.popleft()
        while in_text(va):
            off = va - tva
            if cov[off]:
                break
            ins = next(md.disasm(tdata[off:off + 16], va), None)
            if ins is None:
                break
            for k in range(ins.size):
                if off + k < tsz:
                    cov[off + k] = 1
            m = ins.mnemonic
            op = ins.operands[0] if ins.operands else None
            if m == 'call':
                if op is not None and op.type == X86_OP_IMM and in_text(op.imm) and op.imm not in seen:
                    seen.add(op.imm)
                    funcs.add(op.imm)
                    queue.append(op.imm)
                va = ins.address + ins.size
                continue
            if m == 'jmp':
                if op is not None and op.type == X86_OP_IMM:
                    va = op.imm
                    continue
                break
            if m.startswith('j'):
                if op is not None and op.type == X86_OP_IMM and op.imm not in seen:
                    seen.add(op.imm)
                    queue.append(op.imm)
                va = ins.address + ins.size
                continue
            if m in ('ret', 'retf', 'int3', 'hlt', 'ud2'):
                break
            va = ins.address + ins.size
    starts = sorted(f for f in funcs if in_text(f))
    return tva, tsz, starts, cov


def cited_addresses():
    cited = collections.defaultdict(set)
    for sub in ('src', 'config', 'tools', 'docs', 'modding'):
        for p in (ROOT / sub).rglob('*'):
            if p.name in ('COVERAGE.md', 'SOURCE_INDEX.md', 'auto-matched.json', 'behavior-matched.json') or not p.is_file() or p.suffix not in ('.cpp', '.h', '.inc', '.json', '.md', '.py', '.c', '.txt'):
                continue
            try:
                text = p.read_text(errors='replace')
            except OSError:
                continue
            for m in ADDR_RE.finditer(text):
                cited[int(m.group(1), 16)].add(sub)
    return cited


def load_names(path):
    text = Path(path).read_text(errors='replace')
    return {int(a, 16): n.strip() for a, n in
            re.findall(r'<li><a href=#([0-9A-F]{6})>\s*([^<]+)</a>', text)}


def group_of(name):
    if '::' in name:
        return name.split('::')[0]
    for key in ('LoadBinSection', 'UnloadBinSection', 'Create', 'FE', 'UI', 'Sound', 'Audio',
                'Hud', 'HUD', 'Network', 'Online', 'Career', 'Car', 'Menu', 'Debug', 'Path',
                'Route', 'Texture', 'Solid', 'Anim', 'Physics', 'Camera', 'Language'):
        if key in name:
            return key
    return '(other)'


# Rough subsystem split by address range is NOT available without symbols, so group by
# 4 KiB-aligned 64 KiB bands and report the band; humans name bands in docs/COVERAGE.md.
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('--report', default=str(ROOT / 'docs' / 'COVERAGE.md'))
    ap.add_argument('--json', default=None)
    ap.add_argument('--names', default=None)
    ap.add_argument('--index', default=None)
    a = ap.parse_args()
    base, entry, secs = load(a.exe)
    tva, tsz, starts, cov = discover(base, entry, secs)
    index_names = {}
    index_sizes = None
    if a.index:
        idx = json.loads(Path(a.index).read_text())['functions']
        starts = sorted(int(f['start'], 16) for f in idx)
        index_sizes = {int(f['start'], 16): f['size'] for f in idx}
        index_names = {int(f['start'], 16): f['name'] for f in idx if f['community_named']}
    cited = cited_addresses()
    ends = starts[1:] + [tva + tsz]
    sizes = index_sizes if index_sizes else {s: e - s for s, e in zip(starts, ends)}
    recovered = [s for s in starts if s in cited and 'src' in cited[s]]
    documented = [s for s in starts if s in cited and 'src' not in cited[s]]
    missing = [s for s in starts if s not in cited]
    tb = sum(sizes.values())
    rb = sum(sizes[s] for s in recovered)
    db = sum(sizes[s] for s in documented)
    bands = collections.OrderedDict()
    for s in starts:
        b = s & ~0xFFFF
        e = bands.setdefault(b, [0, 0, 0, 0, 0])
        e[0] += 1
        e[1] += sizes[s]
        if s in recovered:
            e[2] += 1
            e[3] += sizes[s]
        elif s in documented:
            e[4] += 1
    out = []
    w = out.append
    w('# Recovery coverage inventory\n')
    w('Generated by `tools/coverage_inventory.py` from UG2 PC v1.2 `SPEED2.EXE` '
      '(SHA-256 is checked by the native runtime, not here).\n')
    w('"Cited" means the entry address appears in `src/`. It is **not** proof of a verified '
      'recovery; see `docs/DECOMPILATION_SCOPE.md`. Function discovery is recursive descent '
      'from the entry point plus code pointers in data, so it is a lower bound on the real '
      'function count.\n')
    w('## Totals\n')
    w('| Metric | Functions | Bytes |\n| --- | ---: | ---: |')
    w(f'| Discovered in `.text` | {len(starts)} | {tb} |')
    w(f'| Cited in `src/` | {len(recovered)} ({100*len(recovered)/len(starts):.1f} %) | {rb} ({100*rb/tb:.1f} %) |')
    w(f'| Cited only in docs/tools/config | {len(documented)} | {db} |')
    w(f'| Not cited anywhere | {len(missing)} ({100*len(missing)/len(starts):.1f} %) | {tb-rb-db} |')
    w('\n## By 64 KiB address band\n')
    w('| Band | Functions | Bytes | Cited in src | Bytes cited | Doc-only |\n| --- | ---: | ---: | ---: | ---: | ---: |')
    for b, (n, nb, r, rbb, d) in bands.items():
        w(f'| `0x{b:06x}` | {n} | {nb} | {r} | {rbb} | {d} |')
    w('\n## Largest unresolved functions (top 100)\n')
    w('| Address | Size |\n| --- | ---: |')
    for s in sorted(missing, key=lambda x: -sizes[x])[:100]:
        w(f'| `0x{s:06x}` | {sizes[s]} |')
    if a.names or index_names:
        names = load_names(a.names) if a.names else {}
        names.update(index_names)
        nm_missing = [s for s in missing if s in names]
        nm_rec = [s for s in recovered if s in names]
        w(f'\n## Community-named functions ({len(names)} names)\n')
        w(f'{len(nm_rec)} named functions are cited in `src/`; **{len(nm_missing)}** named functions '
          'are unresolved. Names are leads, not evidence.\n')
        groups = collections.defaultdict(list)
        for s_ in nm_missing:
            groups[group_of(names[s_])].append(s_)
        w('| Group | Unresolved named functions | Bytes |\n| --- | ---: | ---: |')
        for g, lst in sorted(groups.items(), key=lambda kv: -sum(sizes[x] for x in kv[1])):
            w(f'| {g} | {len(lst)} | {sum(sizes[x] for x in lst)} |')
        w('\n### Largest unresolved named functions (top 100)\n')
        w('| Address | Size | Name |\n| --- | ---: | --- |')
        for s_ in sorted(nm_missing, key=lambda x: -sizes[x])[:100]:
            w(f'| `0x{s_:06x}` | {sizes[s_]} | `{names[s_]}` |')
    Path(a.report).write_text('\n'.join(out) + '\n')
    if a.json:
        Path(a.json).write_text(json.dumps({
            'recovered': recovered, 'doc_only': documented,
            'missing': [{'addr': s, 'size': sizes[s]} for s in missing]}, indent=1))
    print('\n'.join(out[:14]))


if __name__ == '__main__':
    sys.exit(main())
