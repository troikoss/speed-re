#!/usr/bin/env python3
"""Compare recovered 0x48d860/0x60fef0 with PC v1.2 machine code.

Both sides execute in Unicorn. Compare the complete fixture, returned record,
stack cleanup and callee-saved registers. Slot setter EAX is unspecified (void).
This is bounded behavioral evidence, separate from byte matching and native GL.
"""
import argparse
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_PROT_ALL
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecords.cpp'
BUILD = ROOT / 'build/analysis/car-render-records'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-records.json'
ARENA, SIZE = 0x2000000, 0x100000
STACK, STOP = 0x2200000, 0x2210000
OBJ, TABLE, SLOTS = ARENA, ARENA + 0x100, ARENA + 0x1000
INDICES, RECORDS = ARENA + 0x10000, ARENA + 0x80000
GLOBAL = 0x8a1cc8
ENTRIES = {'slot': 0x48d860, 'part': 0x60fef0}


def link(source):
    # MSVC/wibo is intentionally sequential, including mutation builds.
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'car-render-records.obj'
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe',
                    '/nologo', '/c', '/Ox', '/Ob0', '/EHsc',
                    '/Isrc/Speed/Indep/Src/World', '/Fo' + str(obj.relative_to(ROOT)),
                    str(source.relative_to(ROOT) if source.is_relative_to(ROOT) else source)],
                   cwd=ROOT, check=True)
    parsed = read_object(obj)
    external = {}
    for s in parsed['symbols'].values():
        if s['section'] == 0 and s['name']:
            assert s['name'] == '_ug2CarPartTable8A1CC8', s
            external[s['name']] = GLOBAL
    linked = link_objects([obj], external, start=0x2400000)
    linked['entries'] = {}
    for kind, prefix in (('slot', '?SetSlot@CarRenderSolid@@'), ('part', '?FindRecord@CarRenderPart@@')):
        linked['entries'][kind] = next(a for n, a in linked['exports'].items() if n.startswith(prefix))
    return linked


class Machine:
    def __init__(self, image, linked=None):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095, UC_PROT_ALL)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, data in linked['blocks']:
                u.mem_write(a, data)
        self.linked = linked

    def run(self, kind, data, args):
        u = self.u
        u.mem_write(ARENA, bytes(data))
        u.mem_write(GLOBAL, struct.pack('<I', TABLE))
        sp = STACK + 0x8000
        u.mem_write(sp, struct.pack('<III', STOP, *args))
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, reg in enumerate(saved):
            u.reg_write(reg, 0x12345000 + i * 0x111)
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        u.reg_write(xc.UC_X86_REG_ECX, OBJ)
        u.reg_write(xc.UC_X86_REG_EAX, 0xabcdef01)
        u.emu_start(self.linked['entries'][kind] if self.linked else ENTRIES[kind], STOP, count=1000000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP, 'instruction budget exhausted'
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 12, 'thiscall stack cleanup'
        assert [u.reg_read(reg) for reg in saved] == [0x12345000 + i * 0x111 for i in range(4)], 'saved registers'
        assert bytes(u.mem_read(GLOBAL, 4)) == struct.pack('<I', TABLE)
        result = u.reg_read(xc.UC_X86_REG_EAX) if kind == 'part' else None
        return result, bytes(u.mem_read(ARENA, SIZE))


def put(data, address, fmt, *values):
    struct.pack_into('<' + fmt, data, address - ARENA, *values)


def slot_case(rng, count):
    data = bytearray(SIZE)
    key = rng.choice((0, 0xffffffff, rng.getrandbits(32)))
    value = rng.choice((0, 1, 0xffffffff, rng.getrandbits(32)))
    put(data, OBJ + 0xc, 'I', rng.choice((0, TABLE, TABLE, TABLE)))
    put(data, TABLE + 0x1a, 'b', count)
    put(data, TABLE + 0x3c, 'I', SLOTS)
    for i in range(127):
        put(data, SLOTS + i * 8, 'II', key if i % 3 else key ^ 1, rng.getrandbits(32))
    # Poison unused pointers to prove the early-out paths do not dereference them.
    if count <= 0 or value == 0:
        put(data, TABLE + 0x3c, 'I', 0xdead0000)
    return data, (key, value)


def part_case(rng, count, index, previous_kind, hash_kind):
    data = bytearray(SIZE)
    put(data, OBJ + 0xa, 'H', index)
    put(data, TABLE + 0x14, 'I', INDICES)
    put(data, TABLE + 0x1c, 'I', RECORDS)
    put(data, INDICES + 2 * index, 'h', count)
    # Signed extremes are valid: RECORDS points into the middle of the fixture.
    pool = (-32768, -3, -1, 0, 1, 2, 32767)
    indices = [rng.choice(pool) for _ in range(max(count, 0))]
    for i, ix in enumerate(indices):
        put(data, INDICES + 2 * (index + i + 1), 'h', ix)
    for ix in pool:
        put(data, RECORDS + ix * 8, 'II', 0x1234 if ix % 2 else 0x5678, rng.getrandbits(32))
    previous = 0
    if previous_kind == 'absent':
        previous = RECORDS + 24
    elif indices and previous_kind != 'none':
        k = {'first': 0, 'last': len(indices) - 1, 'middle': len(indices) // 2}[previous_kind]
        previous = RECORDS + indices[k] * 8
    hash_value = {'wildcard': 0, 'hit': 0x1234, 'miss': 0xffffffff}[hash_kind]
    return data, (hash_value, previous)


def verify(image, linked, cases):
    machines = Machine(image), Machine(image, linked)
    counts = {'slot': 0, 'part': 0}
    for kind, data, args in cases:
        a, b = [m.run(kind, data, args) for m in machines]
        if a != b:
            offsets = [hex(i) for i, (x, y) in enumerate(zip(a[1], b[1])) if x != y][:8]
            raise AssertionError(('behavior mismatch', kind, args, a[0], b[0], offsets))
        counts[kind] += 1
    return counts


def cases(seed, random_cases):
    rng = random.Random(seed)
    for count in range(-128, 128):
        for _ in range(4):
            yield 'slot', *slot_case(rng, count)
    for count in (-32768, -1, 0, 1, 2, 3, 7, 127):
        for index in (0, 1, 123, 0xfffe, 0xffff):
            for previous in ('none', 'first', 'middle', 'last', 'absent'):
                for hash_kind in ('wildcard', 'hit', 'miss'):
                    yield 'part', *part_case(rng, count, index, previous, hash_kind)
    # Maximum signed count, with no-match traversal and previous absent.
    for previous in ('none', 'absent'):
        yield 'part', *part_case(rng, 32767, 0, previous, 'miss')
    for _ in range(random_cases):
        yield 'slot', *slot_case(rng, rng.randint(-128, 127))
        yield 'part', *part_case(rng, rng.randint(-4, 64), rng.choice((0, 1, 0xfffe, 0xffff)),
                                 rng.choice(('none', 'first', 'last', 'absent')),
                                 rng.choice(('wildcard', 'hit', 'miss')))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=512)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[off:off + f['size']]).hexdigest() == f['sha256']
    linked = link(args.replace or SOURCE)
    counts = verify(image, linked, cases(0x60fef0, args.cases))
    report = {'comparisons': counts, 'failures': 0, 'byte_identity': 'not established',
              'limitations': ['bounded synthetic tables; retail table construction remains unrecovered',
                              'native demo has no slot tables or rim parts; no visual change claimed']}
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'zero-value-gate': ('table && value', 'table'),
            'unsigned-slot-count': ('int count = table->count1A;', 'int count = (unsigned char)table->count1A;'),
            'first-slot-only': ('slot->value = value;', 'slot->value = value; break;'),
            'wrong-key': ('slot->key == key', 'slot->key != key'),
            'missing-sentinel': ('index0A == 0xffff', 'index0A == 0xfffe'),
            'unsigned-count': ('int count = indices[0];', 'int count = (unsigned short)indices[0];'),
            'unsigned-record-index': ('indices[i + 1] * 8', '(unsigned short)indices[i + 1] * 8'),
            'previous-not-skipped': ('if (record == previous) {\n                break;',
                                     'if (record == previous) {\n                --i; break;'),
            'no-wildcard': ('hash == 0 || ', ''),
            'inverted-hash': ('*>(record) == hash', '*>(record) != hash'),
        }
        BUILD.mkdir(parents=True, exist_ok=True)
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source, name
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '0'],
                                    cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout)
            assert result.returncode and ('behavior mismatch' in result.stdout or 'UcError' in result.stdout), (name, result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    BUILD.mkdir(parents=True, exist_ok=True)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
