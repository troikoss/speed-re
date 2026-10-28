#!/usr/bin/env python3
"""Compare the default scalar D3DX transpose, including x87 payload conversion.

CPU dispatch and optimized library variants are not represented. Every aligned
partial overlap is exercised; complete arena bytes, return, ABI and x87 status
are compared against the retail PC v1.2 body.
"""
import argparse
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
import unicorn.x86_const as xc
from pc_coff import read_object, link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/Physics/D3DXMatrixScalar.cpp'
BUILD = ROOT / 'build/analysis/d3dx-transpose'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/d3dx-transpose.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x2000, 0x2100000, 0x2110000
ENTRY = 0x63d132


def build(source, gcc):
    BUILD.mkdir(parents=True, exist_ok=True)
    if gcc:
        linked = link_native(source, BUILD, ROOT, {}, '_Z25D3DXMatrixTransposeScalarP11WheelMatrixPKS_',
                             include_dirs=[SOURCE.parent])
    else:
        obj = BUILD / 'matrix.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/Physics',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))], cwd=ROOT, check=True)
        linked = link_objects([obj], {'__fltused': 0}, start=0x2400000)
    prefix = '_Z25D3DXMatrixTransposeScalar' if gcc else '?D3DXMatrixTransposeScalar@@'
    linked['entry'] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
    return linked


class Machine:
    def __init__(self, image, linked=None):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        self.entry = ENTRY
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
            self.entry = linked['entry']

    def run(self, data, displacement, cw):
        u = self.u
        src, dst, sp = ARENA + 0x800, ARENA + 0x800 + displacement, STACK + 0x8000
        u.mem_write(ARENA, data)
        u.mem_write(sp, struct.pack('<III', STOP, dst, src))
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        for r, v in ((xc.UC_X86_REG_ESP, sp), (xc.UC_X86_REG_FPCW, cw),
                     (xc.UC_X86_REG_FPSW, 0), (xc.UC_X86_REG_FPTAG, 0xffff)):
            u.reg_write(r, v)
        u.emu_start(self.entry, STOP, count=10000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 12
        assert all(u.reg_read(r) == 0xabcdef00 + i for i, r in enumerate(saved))
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        assert u.reg_read(xc.UC_X86_REG_EAX) == dst
        result = bytes(u.mem_read(ARENA, SIZE))
        restored = bytearray(result)
        off = dst - ARENA
        restored[off:off + 64] = data[off:off + 64]
        assert restored == data, 'writes outside output'
        return result, u.reg_read(xc.UC_X86_REG_FPSW) & 0x7f


def cases(n):
    rng = random.Random(ENTRY)
    displacements = list(range(-60, 64, 4)) + [-256, -64, 64, 256]
    payloads = [0, 0x80000000, 1, 0x80000001, 0x007fffff, 0x00800000,
                0x7f7fffff, 0xff7fffff, 0x7f800000, 0xff800000,
                0x7f800001, 0xff800001, 0x7fbfffff, 0x7fc00001, 0xffc12345]
    for displacement in displacements:
        for trial in range(n):
            data = bytearray(rng.randbytes(SIZE))
            if trial < len(payloads):
                struct.pack_into('<16I', data, 0x800, *([payloads[trial]] * 16))
            elif trial < len(payloads) + 16:
                # Move one signaling NaN through each source position separately.
                struct.pack_into('<16f', data, 0x800, *range(16))
                struct.pack_into('<I', data, 0x800 + 4 * (trial - len(payloads)), 0x7f800123)
            for precision in (0x027f, 0x037f):
                for rounding in (0, 0x400, 0x800, 0xc00):
                    yield bytes(data), displacement, precision | rounding


def verify(image, linked, n):
    original, rebuilt = Machine(image), Machine(image, linked)
    count = 0
    for data, displacement, cw in cases(n):
        a, b = original.run(data, displacement, cw), rebuilt.run(data, displacement, cw)
        if a != b:
            diffs = [(hex(i), a[0][i:i+4].hex(), b[0][i:i+4].hex()) for i in range(0, SIZE, 4)
                     if a[0][i:i+4] != b[0][i:i+4]][:8]
            raise AssertionError(('behavior mismatch', displacement, hex(cw), diffs, a[1], b[1]))
        count += 1
    return count


def native_check(source):
    exe = BUILD / 'transpose-native'
    subprocess.run(['g++', '-std=c++11', '-m32', '-O1', '-fno-pie', '-no-pie', '-mfpmath=387',
                    '-fno-strict-aliasing', '-fno-fast-math', '-ffp-contract=off',
                    '-include', str(ROOT / 'src/Platform/Native/NativeCompat.h'),
                    '-I' + str(SOURCE.parent), str(source),
                    str(ROOT / 'src/Platform/Native/D3DXTransposeNativeTest.cpp'), '-lm', '-o', str(exe)],
                   cwd=ROOT, check=True)
    run = subprocess.run([str(exe)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    assert run.returncode == 0, ('native behavior mismatch', run.stdout)
    return run.stdout.strip()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=128)
    p.add_argument('--mutation-checks', action='store_true')
    p.add_argument('--native-checks', action='store_true', help='Run GCC i386 hardware payload checks')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    assert hashlib.sha256(image[ENTRY - 0x400000:ENTRY - 0x400000 + 113]).hexdigest() == manifest['sha256']
    report = dict(comparisons=verify(image, build(args.replace or SOURCE, args.gcc), args.cases),
                  failures=0, byte_identity='not established',
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  checks=['whole arena bytes and write bounds', 'return, stack, preserved registers',
                          'x87 control, status exception bits and empty stack',
                          'all DWORD aligned overlaps, IEEE payloads and random bits',
                          'two x87 precisions and four rounding modes'],
                  limitations=['finite cases, masked x87 exceptions', 'default scalar body only; CPU dispatch excluded',
                               'Unicorn does not quiet signaling NaNs or raise x87 invalid for these loads/stores'])
    if args.native_checks:
        assert args.gcc, '--native-checks validates the GCC i386 source build'
        report['native_hardware'] = native_check(args.replace or SOURCE)
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'omit-last-pair': ('output->elements[14]=saved;', ''),
            'raw-first-pair': ('volatile long double saved=input->elements[1];\n    volatile long double other=input->elements[4];\n    output->elements[1]=other;\n    output->elements[4]=saved;',
                               'volatile long double saved,other; unsigned int first=from[1],second=from[4]; to[1]=second; to[4]=first;'),
            'raw-second-direction': ('saved=input->elements[2];\n    to[2]=word;\n    output->elements[8]=saved;',
                                     'unsigned int second=from[2]; to[2]=word; to[8]=second;'),
            'convert-raw-direction': ('to[2]=word;', 'other=input->elements[8]; output->elements[2]=other;'),
            'snapshot-overlaps': ('unsigned int *to=', 'WheelMatrix snapshot=*input; input=&snapshot; unsigned int *to='),
            'reverse-diagonals': ('to[0]=from[0];\n        to[5]=from[5];\n        to[10]=from[10];\n        to[15]=from[15];',
                                  'to[15]=from[15];\n        to[10]=from[10];\n        to[5]=from[5];\n        to[0]=from[0];'),
        }
        if not args.native_checks:
            for name in ('raw-first-pair', 'raw-second-direction', 'convert-raw-direction'):
                del mutations[name]
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + ('gcc-' if args.gcc else 'msvc-') + name + '.cpp')
            path.write_text(source.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '32'] +
                                 (['--gcc'] if args.gcc else []) + (['--native-checks'] if args.native_checks else []),
                                 text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + ('gcc-' if args.gcc else 'msvc-') + name + '.log')).write_text(run.stdout)
            assert run.returncode and 'behavior mismatch' in run.stdout, (name, run.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD / ('verification-gcc.json' if args.gcc else 'verification-msvc.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
