#!/usr/bin/env python3
"""Convert the community IDC (yugecin/nfsu2-re, branch idc) into a C header and prototype list.

Reads SPEED2.idc as text (never executes it) and writes, under build/types/:
  ug2_types.h   forward declarations + struct definitions with explicit member offsets
  protos.json   {"0x4017e0": "int __thiscall f(struct X *this, ...)", ...} from SetType(addr, ...)
  names.json    {"0x4017e0": "MenuCarInstance::Get_10_2E0", ...} from set_name for code/data

Types are community-supplied analysis leads, not evidence. Unknown type names become `int` (or
`void *` for LP*/P* and handle-like names) so the header always parses. Struct member gaps are
filled with `char _padNNNN[]`; overlapping members (unions) are dropped after the first.

Usage: idc_to_c.py SPEED2.idc [--out build/types]
"""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

BASE = {'void', 'char', 'short', 'int', 'long', 'float', 'double', 'unsigned', 'signed', 'struct', 'union', 'enum',
        'const', 'volatile', '__int64', '__int8', '__int16', '__int32', 'bool', '__cdecl', '__stdcall', '__fastcall', '__thiscall', 'sizeof'}
KNOWN = {'BYTE': 'unsigned char', 'WORD': 'unsigned short', 'DWORD': 'unsigned long', 'BOOL': 'int', 'UINT': 'unsigned int',
         'ULONG': 'unsigned long', 'LONG': 'long', 'USHORT': 'unsigned short', 'UCHAR': 'unsigned char', 'INT': 'int',
         'LPVOID': 'void *', 'HANDLE': 'void *', 'LPCSTR': 'const char *', 'LPSTR': 'char *', 'size_t': 'unsigned int',
         '_BYTE': 'unsigned char', '_WORD': 'unsigned short', '_DWORD': 'unsigned long',
         'uint': 'unsigned int', 'ushort': 'unsigned short', 'uchar': 'unsigned char'}

SZ = {0x0: ('unsigned char', 1), 0x1: ('unsigned short', 2), 0x2: ('unsigned int', 4), 0x3: ('__int64', 8),
      0x8: ('float', 4), 0x9: ('double', 8)}


def clean_type(t):
    t = t.strip().rstrip(';')
    t = re.sub(r'\$', '_', t)
    t = re.sub(r'::', '__', t)
    return t


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('idc', type=Path)
    ap.add_argument('--out', type=Path, default=ROOT / 'build/types')
    a = ap.parse_args()
    text = a.idc.read_text(errors='replace')
    structs = {}   # name -> {'members': {off: (name, size, flags, ctype|None)}, 'order': [...]}
    cur = None
    cur_name = None
    last_member_by_off = {}
    protos, names = {}, {}
    order = []
    for line in text.splitlines():
        s = line.strip()
        m = re.match(r'id = add_struc\(-1,"([^"]+)",(\d+)\);', s)
        if m:
            cur_name = clean_type(m.group(1))
            structs.setdefault(cur_name, {'members': {}, 'union': m.group(2) == '1'})
            order.append(cur_name)
            continue
        m = re.match(r'id = get_struc_id\("([^"]+)"\);', s)
        if m:
            cur_name = clean_type(m.group(1))
            structs.setdefault(cur_name, {'members': {}, 'union': False})
            continue
        m = re.match(r'mid = add_struc_member\(id,"([^"]*)",\s*(0[xX][0-9a-fA-F]+|\d+),\s*(0[xX][0-9a-fA-F]+),\s*-?\d+,\s*(\d+)\);', s)
        if m and cur_name:
            off = int(m.group(2), 0)
            structs[cur_name]['members'][off] = (m.group(1), int(m.group(4)), int(m.group(3), 16), None)
            continue
        m = re.match(r'SetType\(get_member_id\(id,\s*(0[xX][0-9a-fA-F]+|\d+)\),\s*"(.*)"\);', s)
        if m and cur_name:
            off = int(m.group(1), 0)
            if off in structs[cur_name]['members']:
                n, sz, fl, _ = structs[cur_name]['members'][off]
                structs[cur_name]['members'][off] = (n, sz, fl, clean_type(m.group(2)))
            continue
        m = re.match(r'SetType\((0[xX][0-9a-fA-F]+),\s*"(.*)"\);', s)
        if m:
            protos['0x%x' % int(m.group(1), 16)] = m.group(2)
            continue
        m = re.match(r'set_name\((0[xX][0-9a-fA-F]+),\s*"([^"]*)"', s)
        if m:
            names['0x%x' % int(m.group(1), 16)] = m.group(2)

    # collect type identifiers that are not defined structs/known
    ids = set()
    def strip_fn_name(pr):
        # drop the declared function's own name (the identifier right before the first '(')
        return re.sub(r'\b[A-Za-z_]\w*(?=\s*\()', '', pr, count=1)
    texts = [t for sd in structs.values() for (_, _, _, t) in sd['members'].values() if t] + [strip_fn_name(v) for v in protos.values()]
    for t in texts:
        for w in re.findall(r'[A-Za-z_]\w*', clean_type(t)):
            if w not in BASE:
                ids.add(w)
    struct_names = set(structs)
    # words that directly follow `struct`/`union`/`enum` are tags, not typedefs
    tags = set()
    for t in texts:
        for w in re.findall(r'\b(?:struct|union|enum)\s+([A-Za-z_]\w*)', clean_type(t)):
            tags.add(w)
    typedefs = {}
    for w in sorted(ids - tags):
        if w in KNOWN:
            typedefs[w] = KNOWN[w]
        elif w in struct_names:
            continue
        elif re.match(r'^(LP|P|H)[A-Z]', w):
            typedefs[w] = 'void *'
        else:
            typedefs[w] = 'int'

    def member_decl(name, size, flags, ctype, remaining):
        nm = re.sub(r'\W', '_', name) or 'field'
        if nm[0].isdigit():
            nm = '_' + nm
        if ctype:
            t = ctype
            # IDA writes arrays as `type [N]`
            m = re.match(r'^(.*?)\s*\[(\d+)\]$', t)
            if m:
                return '%s %s[%s];' % (m.group(1), nm, m.group(2))
            if '(*' in t or '(__' in t:
                return None  # function-pointer members: emit as pointer below
            return '%s %s;' % (t, nm)
        dt = (flags >> 28) & 0xF
        base, bsz = SZ.get(dt, ('unsigned char', 1))
        if dt == 0x6 or size > bsz and size % bsz:
            return 'char %s[%d];' % (nm, size)
        if (flags & 0x0F000000) == 0x05000000 and size == 4:
            return 'void *%s;' % nm
        n = size // bsz
        return '%s %s;' % (base, nm) if n <= 1 else '%s %s[%d];' % (base, nm, n)

    args = a
    args.out.mkdir(parents=True, exist_ok=True)
    out = ['/* Generated by tools/idc_to_c.py from the community IDC. Leads, not evidence. */',
           '#ifndef UG2_TYPES_H', '#define UG2_TYPES_H']
    for w, t in sorted(typedefs.items()):
        out.append('typedef %s %s;' % (t, w))
    for n in sorted(struct_names | tags):
        if n in typedefs:
            continue
        out.append('struct %s;' % n)
        out.append('typedef struct %s %s;' % (n, n))
    emitted = 0
    for n in sorted(structs):
        members = structs[n]['members']
        if not members:
            out.append('struct %s { char _empty; };' % n)
            continue
        lines = []
        pos = 0
        for off in sorted(members):
            name, size, flags, ctype = members[off]
            if off < pos:
                continue  # overlap/union member
            if off > pos:
                lines.append('    char _pad%04X[%d];' % (pos, off - pos))
            d = member_decl(name, size, flags, ctype, 0)
            if d is None:
                d = 'void *%s;' % (re.sub(r'\W', '_', name) or 'field')
                size = 4
            lines.append('    ' + d + ' /* +0x%X */' % off)
            pos = off + max(size, 1)
        out.append('struct %s {\n%s\n};' % (n, '\n'.join(lines)))
        emitted += 1
    out.append('#endif')
    (args.out / 'ug2_types.h').write_text('\n'.join(out) + '\n')
    (args.out / 'protos.json').write_text(json.dumps(protos, indent=0))
    (args.out / 'names.json').write_text(json.dumps(names, indent=0))
    (args.out / 'protos.tsv').write_text(''.join('%s\t%s\n' % (k[2:], re.sub(r'\$', '_', v.replace('::', '__'))) for k, v in sorted(protos.items())))
    print('structs', emitted, 'typedefs', len(typedefs), 'protos', len(protos), 'names', len(names))


if __name__ == '__main__':
    main()
