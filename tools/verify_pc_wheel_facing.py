#!/usr/bin/env python3
"""Verify wheel facing helper 0x60d700, including its raw x87 return.

Runs original and MSVC 7.1 or native i386 GCC code with identical finite fixtures and both
x87 precision settings. BodyMatrixCopy executes original code on both sides.
No numeric tolerance: the significand/exponent in ST0 must match exactly.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/wheel-facing'
SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarWheelFacing.cpp'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/wheel-facing.json'
ARENA, WORLD, VIEW, FRAME = 0x2000000, 0x2000000, 0x2000100, 0x2000200
STACK, STOP = 0x2100000, 0x2110000
GLOBALS = {'_ug2WheelFacingEnabled802914': 0x802914, '_ug2WheelFacingX8A1C54': 0x8a1c54,
           '_ug2WheelFacingY802910': 0x802910, '_ug2WheelFacingZ8A1C58': 0x8a1c58,
           '_ug2WheelOneRuntime': 0x784250}


def compile_link(source, gcc=False):
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'facing.obj'
    if gcc:
        elf = BUILD / 'facing.elf.o'
        executable = BUILD / 'facing.elf'
        subprocess.run(['g++', '-m32', '-O1', '-fno-pie', '-mfpmath=387',
                        '-fno-stack-protector', '-fno-strict-aliasing', '-fno-fast-math',
                        '-ffp-contract=off', '-Isrc/Speed/Indep/Src/World',
                        '-c', str(source), '-o', str(elf)], cwd=ROOT, check=True)
        bindings = {k.lstrip('_'): v for k, v in GLOBALS.items()}
        bindings['_Z14BodyMatrixCopyP11WheelMatrixS0_'] = 0x4012b0
        subprocess.run(['ld', '-m', 'elf_i386', '-Ttext=0x2400000',
                        '--entry=UG2WheelFn60d700'] +
                       ['--defsym=' + k + '=' + hex(v) for k, v in bindings.items()] +
                       [str(elf), '-o', str(executable)], check=True)
        raw = executable.read_bytes()
        phoff = struct.unpack_from('<I', raw, 28)[0]
        phsize, phcount = struct.unpack_from('<HH', raw, 42)
        blocks = []
        for i in range(phcount):
            kind, off, addr, _, filesz, memsz, _, _ = struct.unpack_from('<8I', raw, phoff + i * phsize)
            if kind == 1 and memsz:
                blocks.append((addr, raw[off:off + filesz] + bytes(memsz - filesz)))
        low = min(a for a, _ in blocks) & ~4095
        high = (max(a + len(b) for a, b in blocks) + 4095) & ~4095
        return {'entry': struct.unpack_from('<I', raw, 24)[0], 'blocks': blocks,
                'address': low, 'allocation': high - low}
    else:
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c',
                        '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)),
                        str(source.relative_to(ROOT) if source.is_relative_to(ROOT) else source)],
                       cwd=ROOT, check=True)
    # Symbol spelling comes from the unchanged original matrix-copy declaration.
    external = dict(GLOBALS)
    for s in read_object(obj)['symbols'].values():
        if s['section'] == 0 and ('BodyMatrixCopy' in s['name']):
            external[s['name']] = 0x4012b0
    linked = link_objects([obj], external, start=0x2400000)
    linked['entry'] = linked['exports']['_UG2WheelFn60d700']
    return linked


class Machine:
    def __init__(self, image, linked=None):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, 0x10000)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
        self.entry = linked['entry'] if linked else 0x60d700
        u.hook_add(UC_HOOK_CODE, self.copy_hook, begin=0x4012b0, end=0x4012b0)

    def copy_hook(self, u, address, size, user):
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        dest, src = struct.unpack('<II', bytes(u.mem_read(sp + 4, 8)))
        assert src == WORLD
        self.copies += 1
        # Leave the original copy running; the source and call order are checked.
        assert STACK <= dest < STACK + 0x10000

    def run(self, case, cw):
        u = self.u
        self.copies = 0
        arena = bytearray(0x10000)
        struct.pack_into('<16f', arena, WORLD - ARENA, *case['world'])
        struct.pack_into('<I', arena, VIEW - ARENA + 0x40, FRAME)
        struct.pack_into('<3f', arena, FRAME - ARENA + 0x50, *case['axis'])
        u.mem_write(ARENA, bytes(arena))
        u.mem_write(0x802914, struct.pack('<I', case['enabled']))
        for a, v in zip((0x8a1c54, 0x802910, 0x8a1c58), case['normal']):
            u.mem_write(a, struct.pack('<f', v))
        sp = STACK + 0x8000
        # Disabled path must return one without reading either argument.
        world, view = (WORLD, VIEW) if case['enabled'] else (0xdead0000, 0xdead1000)
        u.mem_write(sp, struct.pack('<III', STOP, world, view))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entry, STOP, count=10000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert bytes(u.mem_read(ARENA, len(arena))) == bytes(arena)
        assert self.copies == int(bool(case['enabled']))
        top = (u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7
        assert top == 7, 'expected one x87 result'
        return u.reg_read(xc.UC_X86_REG_FP0 + top)


def cases(n):
    rng = random.Random(0x60d700)
    identity = [float(i in (0, 5, 10, 15)) for i in range(16)]
    for enabled in (0, 1, 2, 0xffffffff):
        for axis in ((0, 0, 0), (1, 0, 0), (0, 1, 0), (0, 0, 1), (-1, -1, -1)):
            for normal in ((0, 0, 0), (1, 0, 0), (0, 1, 0), (0, 0, 1), (0.1, -0.2, 0.3)):
                yield {'enabled': enabled, 'world': identity, 'axis': axis, 'normal': normal}
    for _ in range(n):
        def rf():
            return rng.uniform(-1, 1) * rng.choice((1e-20, 1e-5, 1, 1e5, 1e10))
        yield {'enabled': rng.choice((0, 1, 1, 1, 0xffffffff)),
               'world': [rf() for _ in range(16)], 'axis': [rf() for _ in range(3)],
               'normal': [rf() for _ in range(3)]}


def verify(image, linked, n):
    original, rebuilt = Machine(image), Machine(image, linked)
    count = 0
    for case in cases(n):
        for cw in (0x027f, 0x037f):
            a, b = original.run(case, cw), rebuilt.run(case, cw)
            assert a == b, ('x87 mismatch', count, hex(cw), a, b, case)
            count += 1
    return count


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=4096)
    p.add_argument('--mutation-checks', action='store_true')
    p.add_argument('--gcc', action='store_true', help='verify native i386 GCC -O1 code')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for a, value in manifest['constants'].items():
        off = int(a, 16) - 0x400000
        assert image[off:off + len(value) // 2].hex() == value
    count = verify(image, compile_link(args.replace or SOURCE, args.gcc), args.cases)
    report = {'comparisons': count, 'failures': 0, 'raw_x87_exact': True,
              'byte_identity': 'not established', 'compiler': 'GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
              'limitations': ['finite synthetic matrices/vectors; no exhaustive coverage',
                              'normal uses original image defaults; direct xrefs show only reads; indirect ownership and native gate scheduling remain open']}
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'disabled-zero': ('return ug2WheelOneRuntime;', 'return 0.0;'),
            'enabled-inverted': ('if (!ug2WheelFacingEnabled802914)', 'if (ug2WheelFacingEnabled802914)'),
            'wrong-axis': ('axis50[2]', 'axis50[1]'),
            'wrong-normal': ('normalZ = ug2WheelFacingZ8A1C58', 'normalZ = ug2WheelFacingX8A1C54'),
            'missing-x-spill': ('volatile float x', 'const long double x'),
            'missing-z-spill': ('volatile float z', 'const long double z'),
            'extra-y-spill': ('const long double y', 'volatile float y'),
            'return-float': ('return (static_cast<long double>(z) * axisZ + y * axisY) + static_cast<long double>(x) * axisX;',
                             'volatile float result = (static_cast<long double>(z) * axisZ + y * axisY) + static_cast<long double>(x) * axisX; return result;'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after, 1))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '256'] + (['--gcc'] if args.gcc else []),
                               text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and ('x87 mismatch' in r.stdout or 'self.copies' in r.stdout or 'UcError' in r.stdout), (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD / ('verification-gcc.json' if args.gcc else 'verification.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
