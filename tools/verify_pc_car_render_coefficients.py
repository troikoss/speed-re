#!/usr/bin/env python3
"""Compare the four-lane coefficient transform 0x489ba0 with original PC v1.2.

All fixture bytes are compared exactly; no float tolerance. Both x87 precision
settings and matrix/record aliases are exercised. This does not verify its caller.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecordTransform.cpp'
BUILD = ROOT / 'build/analysis/car-render-coefficients'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-coefficients.json'
ARENA, STACK, STOP = 0x2000000, 0x2100000, 0x2110000
SIZE, RECORD = 0x4000, ARENA + 0x1000
GCC_SYMBOL = '_ZN15CarRenderRecord21TransformCoefficientsEP11WheelMatrix'


def build(source, gcc):
    if gcc:
        linked = link_native(source, BUILD, ROOT, {}, GCC_SYMBOL)
        linked['entry'] = linked['exports'][GCC_SYMBOL]
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        obj = BUILD / 'coefficients.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))],
                       cwd=ROOT, check=True)
        linked = link_objects([obj], {'__fltused': 0}, start=0x2400000)
        linked['entry'] = next(v for k, v in linked['exports'].items()
                               if k.startswith('?TransformCoefficients@CarRenderRecord@@'))
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
        self.entry = linked['entry'] if linked else 0x489ba0
        self.gcc = gcc

    def run(self, data, matrix, cw):
        u = self.u
        u.mem_write(ARENA, data)
        sp = STACK + 0x8000
        args = [RECORD, matrix] if self.gcc else [matrix]
        u.mem_write(sp, struct.pack('<' + 'I' * (len(args) + 1), STOP, *args))
        u.reg_write(xc.UC_X86_REG_ECX, RECORD)
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entry, STOP, count=10000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + (4 if self.gcc else 8)
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        return bytes(u.mem_read(ARENA, SIZE))


def cases(n):
    rng = random.Random(0x489ba0)
    for i in range(n):
        data = bytearray(rng.randbytes(SIZE))
        for j in range(0x1c0 // 4):
            v = rng.uniform(-2, 2) * rng.choice((1, 1e-10, 1e5))
            struct.pack_into('<f', data, RECORD - ARENA + j * 4, v)
        matrix = ARENA + rng.choice((0x800, 0x800, 0x1000, 0x1004, 0x1040, 0x1080))
        values = [rng.uniform(-2, 2) for _ in range(16)]
        if i % 8 == 0:
            values = [float(k in (0, 5, 10, 15)) for k in range(16)]
        elif i % 8 == 1:
            values = [0.0] * 16
        elif i % 8 == 2:
            values = [rng.choice((-1.0, -0.0, 0.0, 1.0)) for _ in range(16)]
        struct.pack_into('<16f', data, matrix - ARENA, *values)
        yield bytes(data), matrix


def verify(image, linked, gcc, n, measure=False):
    original, rebuilt = Machine(image), Machine(image, linked, gcc)
    counts = 0
    stats = {'mismatched_cases': 0, 'mismatched_words': 0, 'max_ulp': 0,
             'signed_zero_words': 0, 'max_absolute_error': 0.0, 'worst_ulp': None, 'by_precision': {'0x27f': 0, '0x37f': 0}}
    for data, matrix in cases(n):
        for cw in (0x027f, 0x037f):
            a, b = original.run(data, matrix, cw), rebuilt.run(data, matrix, cw)
            if a != b:
                offsets = [i for i in range(SIZE) if a[i] != b[i]]
                words = sorted({i & ~3 for i in offsets})[:8]
                details = [(hex(i - (RECORD - ARENA)), struct.unpack_from('<f', a, i)[0],
                            struct.unpack_from('<f', b, i)[0]) for i in words]
                if stats['mismatched_cases'] == 0:
                    (BUILD / 'failure-input.bin').write_bytes(data)
                    (BUILD / 'failure.json').write_text(json.dumps(dict(case=counts, cw=hex(cw),
                        matrix=hex(matrix), details=details), indent=2) + '\n')
                if not measure:
                    raise AssertionError(('coefficient mismatch', counts, hex(cw), hex(matrix - RECORD), details))
                assert all(RECORD - ARENA <= off < RECORD - ARENA + 0xa0 for off in offsets)
                stats['mismatched_cases'] += 1
                stats['by_precision'][hex(cw)] += 1
                for off in {i & ~3 for i in offsets}:
                    x, y = struct.unpack_from('<I', a, off)[0], struct.unpack_from('<I', b, off)[0]
                    assert (x & 0x7f800000) != 0x7f800000 and (y & 0x7f800000) != 0x7f800000
                    stats['mismatched_words'] += 1
                    if (x & 0x7fffffff) == 0 and (y & 0x7fffffff) == 0:
                        stats['signed_zero_words'] += 1
                    def ordered(v):
                        return 0x80000000 - (v & 0x7fffffff) if v & 0x80000000 else 0x80000000 + v
                    distance = abs(ordered(x) - ordered(y))
                    fx, fy = struct.unpack('<f', struct.pack('<I', x))[0], struct.unpack('<f', struct.pack('<I', y))[0]
                    stats['max_absolute_error'] = max(stats['max_absolute_error'], abs(fx - fy))
                    if distance > stats['max_ulp']:
                        stats['max_ulp'] = distance
                        stats['worst_ulp'] = dict(case=counts, cw=hex(cw), offset=hex(off - (RECORD - ARENA)),
                                                 original=fx, rebuilt=fy)

            counts += 1
    return (counts, stats) if measure else counts


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=4096)
    p.add_argument('--mutation-checks', action='store_true')
    p.add_argument('--measure', action='store_true', help='measure all numeric differences; still exit nonzero on any mismatch')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    result = verify(image, build(args.replace or SOURCE, args.gcc), args.gcc, args.cases, args.measure)
    count, differences = result if args.measure else (result, {})
    report = dict(comparisons=count, failures=0, byte_identity='not established',
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['finite synthetic coefficients and matrices, bounded aliases',
                               'producer caller not covered'])
    if differences:
        report['numeric_differences'] = differences
        report['failures'] = differences['mismatched_cases']
    if args.mutation_checks:
        assert not args.measure
        source = SOURCE.read_text()
        mutations = {
            'three-lanes': ('lane < 4', 'lane < 3'),
            'wrong-matrix-axis': ('u1 = matrix->elements[4]', 'u1 = matrix->elements[5]'),
            'omit-float-gv1': ('volatile float gv1', 'const long double gv1'),
            'omit-float-fw0': ('volatile float fw0', 'const long double fw0'),
            'omit-float-ew1': ('volatile float ew1Stored', 'const long double ew1Stored'),
            'wrong-double-cross': ('xy += term + term;', 'xy += term;'),
            'wrong-linear-term': ('static_cast<long double>(j) * u2', 'static_cast<long double>(j) * u1'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after, 1))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '256'] +
                               (['--gcc'] if args.gcc else []), text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and 'coefficient mismatch' in r.stdout, (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD / (('measurement-gcc.json' if args.gcc else 'measurement.json') if args.measure else ('verification-gcc.json' if args.gcc else 'verification.json'))).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    if report['failures']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
