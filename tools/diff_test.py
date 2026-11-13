#!/usr/bin/env python3
"""Differential test: run a candidate C function against the original bytes on random inputs.

The retail image is mapped into Unicorn at its original addresses. The candidate is compiled with MSVC
(same wrapper as tools/auto_match.py), linked with its callees bound to the *original* callee code,
and run next to the original function on identical random arguments and identical random memory.
Compared: eax/edx, esp (stack discipline), x87 result when one was pushed, every byte of the
writable image and of a random-filled arena. Calls into unmapped code (imports) fault in both runs
and count as inconclusive for that trial.

A function passes when >= MIN_OK trials run to completion in both and nothing differs. This is
behavioural evidence on random inputs, weaker than byte matching; record it separately.

Usage: diff_test.py ADDR_HEX [--source file.c] [--trials N] [--seed S]
       (without --source the decompiled function from build/ghidra is used with auto_match's wrapper)
"""
import argparse
import random
import re
import struct
import sys
import tempfile
from pathlib import Path

import pefile
from unicorn import (Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ_UNMAPPED, UC_HOOK_MEM_WRITE_UNMAPPED)
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                               UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_FPSW, UC_X86_REG_FP0, UC_X86_REG_FPCW)

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import auto_match as am  # noqa: E402
from pc_coff import link_objects  # noqa: E402

STACK, STACK_SIZE = 0x2000000, 0x100000
ARENA, ARENA_SIZE = 0x3000000, 0x80000
SENTINEL = 0x2100000
LINK_BASE = 0x2200000
MIN_OK = 6


class Image:
    def __init__(self):
        pe = pefile.PE(str(ROOT / 'orig/SPEED2_EXE_1_2/speed2.exe'))
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xFFF) & ~0xFFF
        self.data = bytes(pe.get_memory_mapped_image())
        self.writable = [(self.base + s.VirtualAddress, max(s.Misc_VirtualSize, 1)) for s in pe.sections
                         if s.Characteristics & 0x80000000 and not s.Characteristics & 0x20000000 and s.Name.rstrip(b'\0') in (b'.data', b'.rdata')]


def rand_words(rnd, n):
    words = bytearray()
    for _ in range(n):
        r = rnd.random()
        if r < 0.30:
            v = 0
        elif r < 0.55:
            v = ARENA + (rnd.randrange(0, ARENA_SIZE - 16) & ~3)
        elif r < 0.75:
            v = rnd.randrange(0, 64)
        elif r < 0.85:
            v = struct.unpack('<I', struct.pack('<f', rnd.uniform(-100, 100)))[0]
        else:
            v = rnd.getrandbits(32)
        words += struct.pack('<I', v)
    return bytes(words)


def make_machine(img, blocks, seed, nwords, args_spec):
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(img.base, img.size)
    uc.mem_write(img.base, img.data[:img.size])
    uc.mem_map(STACK, STACK_SIZE)
    uc.mem_map(ARENA, ARENA_SIZE)
    uc.mem_map(SENTINEL, 0x1000)
    uc.mem_write(SENTINEL, b'\xf4' * 16)
    for addr, body in blocks:
        start = addr & ~0xFFF
        end = (addr + len(body) + 0xFFF) & ~0xFFF
        try:
            uc.mem_map(start, end - start)
        except UcError:
            pass
        uc.mem_write(addr, body)
    rnd = random.Random(seed)
    uc.mem_write(ARENA, rand_words(rnd, ARENA_SIZE // 4))
    if args_spec.get('randdata'):
        for base, size in img.writable:
            uc.mem_write(base, rand_words(rnd, (size + 3) // 4)[:size])
    return uc


MAX_LAZY = 96


def run_once(img, blocks, entry, args_spec, seed):
    uc = make_machine(img, blocks, seed, 0, args_spec)
    lazy = {}

    def on_unmapped(uc_, access, address, size, value, user):
        # map the page on demand with contents derived only from (seed, page): identical in both runs
        page = address & ~0xFFF
        if page < 0x1000 or len(lazy) >= MAX_LAZY:
            return False
        try:
            uc_.mem_map(page, 0x1000)
        except UcError:
            return False
        uc_.mem_write(page, rand_words(random.Random((seed << 20) ^ page), 0x400))
        lazy[page] = True
        return True
    uc.hook_add(UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED, on_unmapped)
    esp = STACK + STACK_SIZE - 0x1000
    stack_args = args_spec['stack']
    for v in reversed(stack_args):
        esp -= 4
        uc.mem_write(esp, struct.pack('<I', v & 0xFFFFFFFF))
    esp -= 4
    uc.mem_write(esp, struct.pack('<I', SENTINEL))
    uc.reg_write(UC_X86_REG_ESP, esp)
    uc.reg_write(UC_X86_REG_EBP, STACK + STACK_SIZE - 0x800)
    uc.reg_write(UC_X86_REG_ECX, args_spec['ecx'])
    uc.reg_write(UC_X86_REG_EDX, args_spec['edx'])
    for reg, v in ((UC_X86_REG_EBX, 0x11111111), (UC_X86_REG_ESI, 0x22222222), (UC_X86_REG_EDI, 0x33333333)):
        uc.reg_write(reg, v)
    uc.reg_write(UC_X86_REG_FPCW, 0x27F)
    top0 = (uc.reg_read(UC_X86_REG_FPSW) >> 11) & 7
    err = None
    try:
        uc.emu_start(entry, SENTINEL, count=300000)
    except UcError as e:
        err = e.errno
    if err is not None:
        return dict(fault=err)
    eip_done = uc.reg_read(UC_X86_REG_ESP)
    top1 = (uc.reg_read(UC_X86_REG_FPSW) >> 11) & 7
    out = dict(eax=uc.reg_read(UC_X86_REG_EAX), edx=uc.reg_read(UC_X86_REG_EDX), esp=eip_done,
               ebx=uc.reg_read(UC_X86_REG_EBX), esi=uc.reg_read(UC_X86_REG_ESI), edi=uc.reg_read(UC_X86_REG_EDI),
               ebp=uc.reg_read(UC_X86_REG_EBP))
    if top1 != top0:
        out['fp0'] = uc.reg_read(UC_X86_REG_FP0)
    mem = {}
    for base, size in img.writable:
        mem[base] = bytes(uc.mem_read(base, size))
    mem['arena'] = bytes(uc.mem_read(ARENA, ARENA_SIZE))
    for page in sorted(lazy):
        mem['lazy%x' % page] = bytes(uc.mem_read(page, 0x1000))
    out['mem'] = mem
    return out


def diff(a, b, check_edx=False):
    if 'fault' in a or 'fault' in b:
        return None if ('fault' in a and 'fault' in b) else 'fault mismatch %r %r' % (a.get('fault'), b.get('fault'))
    for k in ('eax', 'edx', 'esp', 'ebx', 'esi', 'edi', 'ebp'):
        if k == 'edx' and not check_edx:
            continue
        if a[k] != b[k]:
            return '%s %#x vs %#x' % (k, a[k], b[k])
    if a.get('fp0') != b.get('fp0'):
        return 'fp0 %r vs %r' % (a.get('fp0'), b.get('fp0'))
    if set(a['mem']) != set(b['mem']):
        return 'memory access pattern differs (%s)' % sorted(set(a['mem']) ^ set(b['mem']))[:2]
    for k, v in a['mem'].items():
        if v != b['mem'][k]:
            i = next(i for i in range(len(v)) if v[i] != b['mem'][k][i])
            return 'memory %s+%#x differs' % (k if isinstance(k, str) else hex(k), i)
    return 'ok'


def test_function(addr, source, trials=24, seed=1):
    """Returns (verdict, detail). verdict in {'equivalent','different','inconclusive','compile-error'}."""
    img = Image()
    work = Path(tempfile.mkdtemp(prefix='dt', dir=ROOT / 'build'))
    index, image = am.load_inputs()
    body = bytes(image[addr - 0x400000:addr - 0x400000 + index[addr]['size']])
    # compile with the first option set that works
    obj = None
    for opts in am.OPTION_SETS:
        ok, msg = am.compile_and_compare(addr, source, opts, body, work)
        cand = list(work.glob('f%08x_*.obj' % addr))
        if cand:
            obj = max(cand, key=lambda p: p.stat().st_mtime)
            break
    if obj is None:
        return 'compile-error', msg
    externals = {}
    from pc_coff import read_object
    for s in read_object(obj)['symbols'].values():
        if s['section'] == 0 and s['storage'] == 2:
            m = re.search(r'(?:FUN|TC)_([0-9a-fA-F]{8})', s['name'])
            if s['name'] == '__fltused':
                externals[s['name']] = ARENA
            elif m:
                externals[s['name']] = int(m.group(1), 16)
            else:
                gn = am.ghidra_function_names()
                base = re.sub(r'@\d+$', '', s['name']).lstrip('_@')
                hit = next((gn[c] for c in (base, '_' + base, '__' + base) if c in gn), None)
                if hit is None:
                    return 'inconclusive', 'unresolved external ' + s['name']
                externals[s['name']] = hit
    linked = link_objects([obj], externals, start=LINK_BASE)
    entry_name = next((n for n in linked['exports'] if re.fullmatch(r'[_@?]FUN_%08x(@.*)?' % addr, n)), None)
    if not entry_name:
        return 'inconclusive', 'entry symbol missing'
    cand_entry = linked['exports'][entry_name]
    fast = entry_name.startswith('@')
    ob = body.rstrip(b'\xcc\x90')
    nstack = ob[-2] // 4 if len(ob) >= 3 and ob[-3] == 0xC2 and ob[-1] == 0 else 6
    check_edx = bool(re.search(r'(undefined8|longlong|ulonglong|__int64)\s+(?:__\w+\s+)?FUN_%08x' % addr, source))
    ok_runs, mismatch, faults = 0, None, 0
    for t in range(trials):
        rnd = random.Random(seed * 7919 + t)

        def val():
            r = rnd.random()
            if r < 0.5:
                return ARENA + (rnd.randrange(0, ARENA_SIZE - 0x2000) & ~3)
            if r < 0.8:
                return rnd.randrange(0, 40)
            return rnd.getrandbits(32)
        spec = dict(stack=[val() for _ in range(nstack)], ecx=val(), edx=val(), randdata=(t % 2 == 1))
        a = run_once(img, linked['blocks'], addr, spec, seed + t)
        b = run_once(img, linked['blocks'], cand_entry, spec, seed + t)
        d = diff(a, b, check_edx)
        if d is None:
            faults += 1
        elif d == 'ok':
            ok_runs += 1
        else:
            return 'different', 'trial %d: %s' % (t, d)
    if ok_runs >= MIN_OK:
        return 'equivalent', '%d clean trials, %d both-fault' % (ok_runs, faults)
    return 'inconclusive', 'only %d clean trials (%d both-fault)' % (ok_runs, faults)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('addr')
    ap.add_argument('--source')
    ap.add_argument('--trials', type=int, default=24)
    ap.add_argument('--seed', type=int, default=1)
    a = ap.parse_args()
    addr = int(a.addr, 16)
    am.INDEX, am.IMAGE = am.load_inputs()
    if a.source:
        src = Path(a.source).read_text()
    else:
        import json
        src = None
        for line in (ROOT / 'build/ghidra/decomp2.jsonl').read_text().splitlines():
            try:
                e = json.loads(line)
            except ValueError:
                continue
            if int(e['a'], 16) == addr and 'c' in e:
                src, why = am.build_source(addr, e['c'], am.INDEX, am.IMAGE)
                break
        if src is None:
            sys.exit('no decompilation / unsupported function')
    print(*test_function(addr, src, a.trials, a.seed))


if __name__ == '__main__':
    main()
