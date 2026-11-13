#!/usr/bin/env python3
"""Compile-and-compare recovery: turn decompiler output into verified-identical C.

For every function in a Ghidra decompilation export (build/ghidra/decomp.jsonl, local only), this
wraps the C in a small prelude, compiles it with MSVC 7.1 under wibo, and compares the object code
with the original function bytes from speed2.exe. Relocated operands (call targets) are masked on
both sides; absolute data addresses are emitted as `(*(T *)0xADDR)` macros so they need no mask.
A function that is byte-identical under any tried option set is written to
`build/auto_match/matched/<addr>.c` and listed in `build/auto_match/results.jsonl`.

This is an analysis aid. A match is *byte-matching evidence for that function*; callers, callee
conventions, and data types are inferred (see docs/AUTO_MATCH.md), so promote a match into `src/`
only after reading it, naming it, and adding it to the usual manifest/verifier flow.

Usage: auto_match.py [--max-size N] [--limit N] [--jobs N] [--addr HEX ...]
Needs: build/tools/wibo, build/compilers/Win32/7.1, build/analysis/function-index.json,
       build/ghidra/decomp.jsonl, orig/SPEED2_EXE_1_2/speed2.exe
"""
import argparse
import concurrent.futures as cf
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from pc_coff import read_object  # noqa: E402

OUT = ROOT / 'build/auto_match'
PRELUDE = r'''
typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
#define CONCAT44(a, b) ((((unsigned __int64)(unsigned int)(a)) << 32) | (unsigned int)(b))
#define CONCAT22(a, b) ((((unsigned int)(unsigned short)(a)) << 16) | (unsigned short)(b))
#define CONCAT11(a, b) ((((unsigned short)(unsigned char)(a)) << 8) | (unsigned char)(b))
#define SUB41(x, n) ((unsigned char)(((unsigned int)(x)) >> ((n) * 8)))
#define SUB42(x, n) ((unsigned short)(((unsigned int)(x)) >> ((n) * 8)))
#define SUB84(x, n) ((unsigned int)(((unsigned __int64)(x)) >> ((n) * 8)))
#define SQRT(x) sqrt(x)
double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
typedef unsigned short wchar_t16;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
'''
OPTION_SETS = (['/Ox', '/Ob0'], ['/Ox', '/Ob2'], ['/O2', '/Ob0', '/Gy'])
# Statically linked CRT/library code (hot-patch prologue `mov edi, edi`) comes from MSVC 8.0.
OPTION_SETS_80 = (['CC8.0', '/Ox', '/Ob0', '/hotpatch', '/GS-'], ['CC8.0', '/Ox', '/Ob0', '/hotpatch'],
                  ['CC8.0', '/O2', '/Ob0', '/hotpatch', '/GS-', '/Gy'], ['CC8.0', '/Ox', '/Ob0', '/GS-'])
LIB_REGION = 0x63b000
IDENT = re.compile(r'\b(_?(?:FUN|DAT|PTR_FUN|PTR_DAT|thunk_FUN)_)([0-9a-fA-F]{8})\b')
RAMID = re.compile(r'\b(p?[iufdcbswlp])Ram([0-9a-fA-F]{8})\b')
RAM_TYPES = {'i': 'int', 'u': 'uint', 'f': 'float', 'd': 'double', 'c': 'char', 'b': 'byte', 's': 'short', 'w': 'ushort', 'l': 'int', 'p': 'void *'}
STRID = re.compile(r'\b((?:PTR_)?s_[A-Za-z0-9_]*?_)([0-9a-fA-F]{8})\b')


def load_inputs():
    index = {int(f['start'], 16): f for f in json.loads((ROOT / 'build/analysis/function-index.json').read_text())['functions']}
    pe = pefile.PE(str(ROOT / 'orig/SPEED2_EXE_1_2/speed2.exe'))
    image = pe.get_memory_mapped_image()
    return index, image


def is_thiscall(addr, index, image):
    """True when the callee reads ecx before writing it (MSVC thiscall: `this` in ecx)."""
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_REG, X86_OP_MEM
    f = index.get(addr)
    if not f:
        return False
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    code = bytes(image[addr - 0x400000:addr - 0x400000 + min(f['size'], 48)])
    for ins in md.disasm(code, addr):
        regs_read, regs_written = ins.regs_access()
        names_r = {ins.reg_name(r) for r in regs_read}
        names_w = {ins.reg_name(r) for r in regs_written}
        if ins.mnemonic in ('push',) and ins.op_str == 'ecx':
            return False  # push ecx is a stack-slot idiom as often as an argument
        if 'ecx' in names_r or 'cx' in names_r or 'cl' in names_r:
            return True
        if 'ecx' in names_w:
            return False
        if ins.mnemonic in ('call', 'jmp', 'ret') or ins.mnemonic.startswith('j'):
            return False
    return False


def split_args(text):
    args, depth, cur = [], 0, ''
    for ch in text:
        if ch in '([':
            depth += 1
        elif ch in ')]':
            depth -= 1
        if ch == ',' and depth == 0:
            args.append(cur.strip())
            cur = ''
        else:
            cur += ch
    if cur.strip():
        args.append(cur.strip())
    return args


def cast_calls(c, targets):
    """C++ mode: FUN_x(a, b) -> FUN_x((int)(a), (int)(b)) so pointer arguments convert."""
    for a in targets:
        pat = 'FUN_%08x(' % a
        out, pos = '', 0
        while True:
            i = c.find(pat, pos)
            if i < 0:
                out += c[pos:]
                break
            j, depth = i + len(pat), 1
            while j < len(c) and depth:
                depth += (c[j] == '(') - (c[j] == ')')
                j += 1
            args = split_args(c[i + len(pat):j - 1])
            out += c[pos:i] + pat + ', '.join('(int)(%s)' % x for x in args) + ')'
            pos = j
        c = out
    return c


def rewrite_thiscalls(c, targets):
    """FUN_x(a, b) -> ((TC_x *)(a))->m(b) for thiscall callees; returns (code, changed)."""
    changed = False
    for a in targets:
        pat = 'FUN_%08x(' % a
        out, pos = '', 0
        while True:
            i = c.find(pat, pos)
            if i < 0:
                out += c[pos:]
                break
            j, depth = i + len(pat), 1
            while j < len(c) and depth:
                depth += (c[j] == '(') - (c[j] == ')')
                j += 1
            args = split_args(c[i + len(pat):j - 1])
            out += c[pos:i]
            if not args:
                out += c[i:j]
            else:
                out += '((TC_%08x *)(%s))->m(%s)' % (a, args[0], ', '.join('(int)(%s)' % x for x in args[1:]))
                changed = True
            pos = j
        c = out
    return c, changed


def callee_decl(addr, index, image):
    """Declaration for a callee from its own bytes: `ret N` means stdcall with N/4 arguments."""
    f = index.get(addr)
    if not f:
        return 'int FUN_%08x();' % addr
    body = bytes(image[addr - 0x400000:addr - 0x400000 + f['size']].rstrip(b'\xcc\x90'))
    m = body[-3:]
    if len(body) >= 3 and m[0] == 0xC2 and m[2] == 0:
        n = m[1] // 4
        return 'int __stdcall FUN_%08x(%s);' % (addr, ','.join(['int'] * n) if n else 'void')
    return 'int __cdecl FUN_%08x();' % addr


_NAMES = None


def ghidra_function_names():
    """name -> address for functions Ghidra labelled with real names (CRT etc.), from build/ghidra/names.txt."""
    global _NAMES
    if _NAMES is None:
        _NAMES = {}
        f = ROOT / 'build/ghidra/names.txt'
        if f.exists():
            for ln in f.read_text().splitlines():
                k, a, n = ln.split(' ', 2)
                if k == 'F' and not n.startswith(('FUN_', 'thunk_')) and re.fullmatch(r'[0-9a-fA-F]{8}', a):
                    _NAMES.setdefault(n, int(a, 16))
    return _NAMES


_TYPES = None


def _load_types():
    """Parse build/types/ug2_types.h into {name: (block, deps)}; MSVC-friendly (no calling conventions)."""
    f = ROOT / 'build/types/ug2_types.h'
    blocks = {}
    if f.exists() and not os.environ.get('UG2_NO_TYPES'):
        t = '\n'.join(l for l in f.read_text().splitlines() if not l.startswith('#'))
        t = re.sub(r'__(thiscall|fastcall|stdcall|cdecl)', '', t)
        for m in re.finditer(r'^struct (\w+) \{.*?^\};', t, re.M | re.S):
            blocks['struct ' + m.group(1)] = m.group(0)
        for m in re.finditer(r'^typedef struct (\w+) (\w+);', t, re.M):
            blocks['typedef ' + m.group(2)] = m.group(0)
        for m in re.finditer(r'^struct (\w+);', t, re.M):
            blocks['fwd ' + m.group(1)] = m.group(0)
        for m in re.finditer(r'^typedef (?!struct)([^;{]*?)\b(\w+);', t, re.M):
            blocks['typedef ' + m.group(2)] = m.group(0)
    return blocks


def type_header(code):
    """Header text containing only the types `code` uses (and what those need), plus the names it defines."""
    global _TYPES
    if _TYPES is None:
        _TYPES = _load_types()
    if not _TYPES:
        return '', set()
    words = set(re.findall(r'[A-Za-z_]\w*', code))
    need, todo = set(), [w for w in words]
    while todo:
        w = todo.pop()
        for key in ('typedef ' + w, 'struct ' + w, 'fwd ' + w):
            if key in _TYPES and key not in need:
                need.add(key)
                todo.extend(re.findall(r'[A-Za-z_]\w*', _TYPES[key]))
    order = sorted(need, key=lambda k: (0 if k.startswith('fwd') else 1 if k.startswith('typedef') else 2, k))
    names = {k.split(' ', 1)[1] for k in need if k.startswith('typedef')}
    return '\n'.join(_TYPES[k] for k in order) + '\n', names


DAT_TYPES = ('undefined4', 'float', 'double', 'undefined1', 'undefined2')


def pad_stack_params(c, addr, ret_n):
    """Pad the target's signature with unused parameters so its `ret N` size matches the original."""
    m = re.search(r'FUN_%08x\(' % addr, c)
    if not m or ret_n <= 0:
        return c
    i = m.end()
    depth, j = 1, i
    while j < len(c) and depth:
        depth += (c[j] == '(') - (c[j] == ')')
        j += 1
    params = c[i:j - 1].strip()
    head = c[:m.start()]
    lst = [] if params in ('', 'void') else split_args(params)
    regs = 0
    if '__fastcall' in head[-40:]:
        if lst and '_ecx_unused' in lst[0]:
            regs = 1
        elif len(lst) > 1 and '_edx_unused' in lst[1]:
            regs = 2
        else:
            return c  # genuine fastcall: leave to the decompiler
    stack = max(0, len(lst) - regs)
    want = ret_n // 4
    if stack >= want:
        return c
    lst += ['int _pad%d' % k for k in range(want - stack)]
    return c[:i] + ', '.join(lst) + c[j - 1:]


def build_source(addr, c, index, image, dat_type='undefined4'):
    if re.search(r'\bunaff_|\bin_(?!stack)', c):
        return None, 'register-argument function'
    # MSVC 7.1 rejects __thiscall: emit __fastcall with a dummy unused edx parameter instead.
    def thiscall(m):
        params = m.group(2).strip()
        if params in ('', 'void'):
            return m.group(1) + '__fastcall FUN_%08x(int _ecx_unused)' % addr
        parts = params.split(',', 1)
        rest = (',' + parts[1]) if len(parts) > 1 else ''
        return m.group(1) + '__fastcall FUN_%08x(%s,int _edx_unused%s)' % (addr, parts[0], rest)
    c = re.sub(r'^([^\n(]*?)__thiscall\s+\w+\s*\(([^)]*)\)', thiscall, c, count=1, flags=re.M)
    if '__thiscall' in c:
        return None, 'thiscall in body'
    # resolve Ghidra-labelled callees (e.g. _free) to their addresses, rename `this` for C++
    names = ghidra_function_names()
    c = re.sub(r'\b([A-Za-z_]\w*)(?=\s*\()', lambda m: ('FUN_%08x' % names[m.group(1)]) if m.group(1) in names and m.group(1) not in ('if', 'while', 'for', 'switch', 'sizeof', 'return') else m.group(0), c)
    c = re.sub(r'\bthis\b', '_this', c)
    decls, macros = [], []
    seen = set()
    cpp = False
    tc = []
    for prefix, hexa in set(IDENT.findall(c)):
        a = int(hexa, 16)
        if prefix.endswith('FUN_') and not prefix.endswith('PTR_FUN_') and a != addr and is_thiscall(a, index, image):
            tc.append(a)
    for prefix, hexa in IDENT.findall(c):
        key = (prefix, hexa)
        if key in seen:
            continue
        seen.add(key)
        a = int(hexa, 16)
        if prefix.endswith('FUN_') and not prefix.endswith('PTR_FUN_'):
            if a != addr and a not in tc:
                decls.append(callee_decl(a, index, image))
        elif prefix.endswith('PTR_FUN_'):
            macros.append('#define %s%s (*(code *)0x%s)' % (prefix, hexa, hexa))
        elif prefix.endswith('PTR_DAT_'):
            macros.append('#define %s%s (*(undefined4 *)0x%s)' % (prefix, hexa, hexa))
        elif prefix.endswith('DAT_'):
            macros.append('#define %s%s (*(%s *)0x%s)' % (prefix, hexa, dat_type, hexa))
        else:
            return None, 'unsupported identifier ' + prefix
    c2 = re.sub(r'\bthunk_FUN_', 'FUN_', c)
    for pre, hexa in sorted(set(RAMID.findall(c2))):
        t = RAM_TYPES.get(pre[-1], 'int') + (' *' if pre.startswith('p') and len(pre) > 1 else '')
        macros.append('#define %sRam%s (*(%s *)0x%s)' % (pre, hexa, t if not t.endswith('*') else t[:-2] + ' *', hexa))
    for pre, hexa in sorted(set(STRID.findall(c2))):
        macros.append('#define %s%s ((char *)0x%s)' % (pre, hexa, hexa))
    own = index.get(addr)
    if own:
        ob = bytes(image[addr - 0x400000:addr - 0x400000 + own['size']].rstrip(b'\xcc\x90'))
        if len(ob) >= 3 and ob[-3] == 0xC2 and ob[-1] == 0:
            c2 = pad_stack_params(c2, addr, ob[-2])
        if len(ob) >= 3 and ob[-3] == 0xC2 and ob[-1] == 0 and not re.search(r'__(stdcall|fastcall|cdecl)', c2.split('{')[0]):
            c2 = re.sub(r'\b(FUN_%08x|[A-Za-z_]\w*)\s*\(' % addr, lambda m: '__stdcall ' + m.group(0), c2, count=1)
    # force the defined function to the canonical name (Ghidra may use a recovered label)
    m = re.search(r'^([^\n(]*?)\b([A-Za-z_][A-Za-z_0-9]*)\s*\(', c2.lstrip(), re.M)
    if m and m.group(2) != 'FUN_%08x' % addr:
        c2 = c2.replace(m.group(0), m.group(1) + 'FUN_%08x(' % addr, 1)
    hdr, hnames = type_header(c)
    prelude_text = PRELUDE
    if hdr:
        keep = []
        for ln in PRELUDE.splitlines():
            m = re.match(r'typedef (.*?)\s+([A-Za-z_]\w*);$', ln)
            names_in = re.findall(r'\b(\w+)(?=[,;])', ln) if ln.startswith('typedef') else []
            if ln.startswith('typedef') and any(n in hnames for n in names_in):
                # drop only the clashing aliases
                for n in names_in:
                    if n in hnames:
                        ln = re.sub(r',\s*%s\b' % n, '', ln)
                        ln = re.sub(r'\b%s\s*,\s*' % n, '', ln)
                if re.fullmatch(r'typedef [^,;]*;', ln) and ln.split()[-1].rstrip(';') in hnames:
                    continue
            keep.append(ln)
        prelude_text = '\n'.join(keep) + '\n'
        prelude_text = hdr + prelude_text
    prelude = prelude_text
    if tc:
        c2, cpp = rewrite_thiscalls(c2, tc)
        if cpp:
            plain = [int(h, 16) for p_, h in set(IDENT.findall(c2)) if p_.endswith('FUN_') and not p_.endswith('PTR_FUN_') and int(h, 16) != addr and int(h, 16) not in tc]
            c2 = cast_calls(c2, plain)
            c2 = re.sub(r'\b(FUN_[0-9a-fA-F]{8})\b(?!\s*\()', lambda m: '((code)%s)' % m.group(1) if m.group(1) != 'FUN_%08x' % addr else m.group(1), c2)
            decls = [d.replace('int __cdecl FUN_', 'int __cdecl FUN_').replace('();', '(...);') for d in decls]
            for a in tc:
                f = index[a]
                body = bytes(image[a - 0x400000:a - 0x400000 + f['size']].rstrip(b'\xcc\x90'))
                n = body[-2] // 4 if len(body) >= 3 and body[-3] == 0xC2 and body[-1] == 0 else 0
                decls.append('struct TC_%08x { int m(%s); };' % (a, ','.join(['int'] * n) if n else ''))
            prelude = ('//CPP\n' + prelude_text.replace('typedef unsigned char bool;\n', '').replace('#define true 1\n', '').replace('#define false 0\n', '')
                       .replace('typedef void (*code)();', 'typedef void (*code)(...);'))
        else:
            for a in tc:
                decls.append(callee_decl(a, index, image))
    return prelude + '\n'.join(sorted(set(decls))) + '\n' + '\n'.join(macros) + '\n' + c2 + '\n', None


def compile_and_compare(addr, src, opts, orig_body, work):
    c = work / ('f%08x.c' % addr)
    o = work / ('f%08x_%s.obj' % (addr, ''.join(ch for ch in ''.join(opts) if ch.isalnum())))
    c.write_text(src)
    cc = '8.0' if opts and opts[0] == 'CC8.0' else '7.1'
    opts = [x for x in opts if x != 'CC8.0']
    cdir = ROOT / 'build/compilers/Win32' / cc
    env = dict(os.environ, TMP=str(work), TEMP=str(work), INCLUDE=str(cdir / 'Include'))
    r = subprocess.run([str(ROOT / 'build/tools/wibo'), str(cdir / 'cl.exe'), '/nologo', '/c', '/TP' if '//CPP' in src[:800] else '/TC', '/Zl',
                        '/w'] + opts + ['/Fo' + o.name, c.name], cwd=work, env=env, capture_output=True, text=True)
    if r.returncode != 0 or not o.exists():
        err = [l for l in (r.stdout + r.stderr).splitlines() if 'error' in l]
        return False, (err[0][-120:] if err else 'compile failed')
    obj = read_object(o)
    text = next(s for s in obj['sections'] if s['name'] == '.text' or s['name'].startswith('.text'))
    syms = [s for s in obj['symbols'].values() if s['section'] > 0 and s['kind'] == 0x20 and re.fullmatch(r'[_@?]FUN_%08x(@\d+)?(@@.*)?' % addr, s['name'])]
    if not syms:
        return False, 'symbol missing'
    start = syms[0]['value']
    # function ends at the next function symbol or the section end
    ends = sorted(s['value'] for s in obj['symbols'].values() if s['section'] > 0 and s['kind'] == 0x20 and s['value'] > start)
    body = bytearray(text['body'][start:(ends[0] if ends else len(text['body']))].rstrip(b'\xcc\x90'))
    orig = bytearray(orig_body.rstrip(b'\xcc\x90'))
    mask = set()
    for off, _, _ in text['relocations']:
        if start <= off < start + len(body):
            for k in range(4):
                mask.add(off - start + k)
    if len(body) != len(orig):
        return False, 'size %d vs %d' % (len(body), len(orig))
    for i in mask:
        body[i] = 0
        orig[i] = 0
    if body != orig:
        first = next(i for i in range(len(body)) if body[i] != orig[i])
        return False, 'bytes differ at +%d' % first
    return True, 'identical'


def attempt(job):
    addr, c, size, orig_body, index, image_unused = job
    work = Path(tempfile.mkdtemp(prefix='am', dir=OUT / 'tmp'))
    last = ''
    has_dat = bool(re.search(r'\b_?DAT_[0-9a-fA-F]{8}\b', c))
    for vi, dt in enumerate(DAT_TYPES if has_dat else DAT_TYPES[:1]):
        src, why = build_source(addr, c, INDEX, IMAGE, dt)
        if src is None:
            return addr, 'skip', why, None, None
        lib = orig_body[:2] == b'\x8b\xff' or addr >= LIB_REGION
        base_sets = OPTION_SETS_80 + OPTION_SETS if orig_body[:2] == b'\x8b\xff' else (OPTION_SETS + OPTION_SETS_80[3:] if lib else OPTION_SETS)
        for opts in (base_sets if vi == 0 else base_sets[:2]):
            ok, msg = compile_and_compare(addr, src, opts, orig_body, work)
            if ok:
                return addr, 'match', ' '.join(opts) + ' dat=' + dt, src, c
            if vi == 0:
                last = msg
    return addr, 'miss', last, None, None


INDEX = IMAGE = None


def main():
    global INDEX, IMAGE
    ap = argparse.ArgumentParser()
    ap.add_argument('--max-size', type=int, default=64)
    ap.add_argument('--min-size', type=int, default=1)
    ap.add_argument('--limit', type=int, default=200)
    ap.add_argument('--jobs', type=int, default=4)
    ap.add_argument('--decomp', default='build/ghidra/decomp2.jsonl', help='Ghidra export (two-pass with committed signatures)')
    ap.add_argument('--addr', nargs='*')
    ap.add_argument('--redo', action='store_true', help='retry functions already in results.jsonl')
    ap.add_argument('--redo-info', default=None, help='with --redo: only retry results whose info matches this regex')
    ap.add_argument('--redo-status', default='miss,skip', help='with --redo: statuses to retry')
    a = ap.parse_args()
    INDEX, IMAGE = load_inputs()
    (OUT / 'matched').mkdir(parents=True, exist_ok=True)
    (OUT / 'tmp').mkdir(exist_ok=True)
    jobs = []
    want = {int(x, 16) for x in a.addr} if a.addr else None
    done = {}
    rf = OUT / 'results.jsonl'
    if rf.exists():
        for ln in rf.read_text().splitlines():
            try:
                r = json.loads(ln)
                done[int(r['addr'], 16)] = r['status']
                if a.redo_info and r['status'] == 'miss' and not re.search(a.redo_info, r.get('info', '')):
                    done[int(r['addr'], 16)] = 'keep'
            except ValueError:
                pass
    retry = set(a.redo_status.split(',')) if a.redo else set()
    for line in (ROOT / a.decomp).read_text().splitlines():
        try:
            e = json.loads(line)
        except ValueError:
            continue  # partial line while Ghidra is still writing
        if 'c' not in e:
            continue
        addr = int(e['a'], 16)
        f = INDEX.get(addr)
        if addr in done and done[addr] not in retry:
            continue
        if not f or (want and addr not in want) or not (a.min_size <= f['size'] <= a.max_size):
            continue
        body = bytes(IMAGE[addr - 0x400000:addr - 0x400000 + f['size']])
        jobs.append((addr, e['c'], f['size'], body, None, None))
        if len(jobs) >= a.limit:
            break
    stats = {'match': 0, 'miss': 0, 'skip': 0}
    reasons = {}
    with open(OUT / 'results.jsonl', 'a') as res, cf.ThreadPoolExecutor(a.jobs) as ex:
        for addr, status, info, src, c in ex.map(attempt, jobs):
            stats[status] += 1
            reasons[info if status != 'match' else 'match'] = reasons.get(info if status != 'match' else 'match', 0) + 1
            if status == 'match':
                (OUT / 'matched' / ('%08x.c' % addr)).write_text(src)
            res.write(json.dumps({'addr': '%08x' % addr, 'status': status, 'info': info}) + '\n')
    print(stats)
    for k, v in sorted(reasons.items(), key=lambda kv: -kv[1])[:12]:
        print(v, k)


if __name__ == '__main__':
    main()
