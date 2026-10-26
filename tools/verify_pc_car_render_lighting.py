#!/usr/bin/env python3
"""Exact-output checks for five lighting-record foundations in UG2 PC v1.2.

The positive/negative pair runs its accumulator as machine code on both sides.
Vector calls use original trig unless --native-trig selects the actual CMake
native i386 object. No float tolerance is used; full fixture and call inputs match.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import re
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderLighting.cpp'
BUILD = ROOT / 'build/analysis/car-render-lighting'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-lighting.json'
ARENA, SIZE, RECORD, STACK, STOP = 0x2000000, 0x4000, 0x2001000, 0x2100000, 0x2110000
ORIGINAL = {'clear': 0x48ab10, 'vector': 0x48a310, 'accumulate': 0x4895e0,
            'pair': 0x489af0, 'normalize': 0x489ed0}
MSVC = {'clear': '?CarRenderClearRecord@@', 'vector': '?CarRenderLightVector@@',
        'accumulate': '?AccumulateLight@CarRenderRecord@@', 'pair': '?AddLightPair@CarRenderRecord@@',
        'normalize': '?NormalizeLightCoefficients@CarRenderRecord@@'}
GCC = {'clear': '_Z20CarRenderClearRecord', 'vector': '_Z20CarRenderLightVector',
       'accumulate': '_ZN15CarRenderRecord15AccumulateLight', 'pair': '_ZN15CarRenderRecord12AddLightPair',
       'normalize': '_ZN15CarRenderRecord26NormalizeLightCoefficients'}
GCC_TRIG = '_Z15WheelSineCosinePfS_t'
TRIG_OBJECT = ROOT / 'build/native/CMakeFiles/ug2recovered.dir/src/Speed/Indep/Src/Physics/PhysicsMoverController.cpp.o'


def fbits(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def build(source, gcc, native_trig):
    if gcc:
        bindings = {'sqrtl': STOP + 0x100}
        extra = []
        if native_trig:
            assert TRIG_OBJECT.exists(), 'build the native ug2recovered target first'
            names = {'engineAngleUnit', 'engineHalfPi', 'enginePi', 'engineTwoPi', 'engineThreeHalfPi',
                     'engineSineC3', 'engineSineC5', 'engineSineC7', 'engineSineC9',
                     'engineCosineC4', 'engineCosineC6', 'engineCosineC8', 'engineCosineC10'}
            for name, address in re.findall(r'^(\w+)\s*=\s*(0x[0-9a-f]+);',
                                           (ROOT / 'src/Platform/Native/OriginalSymbols.ld').read_text(), re.M):
                if name in names:
                    bindings[name] = int(address, 16)
            assert names <= bindings.keys()
            extra = [TRIG_OBJECT]
        else:
            bindings[GCC_TRIG] = 0x43c390
        linked = link_native(source, BUILD, ROOT, bindings, '_Z20CarRenderLightVectorPffff',
                             extra_objects=extra, gc_sections=native_trig)
        linked['trig'] = linked['exports'][GCC_TRIG] if native_trig else 0x43c390
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        obj = BUILD / 'lighting.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))], cwd=ROOT, check=True)
        external = {'__fltused': 0, '__ftol2': 0x75bed4}
        for s in read_object(obj)['symbols'].values():
            if s['section'] == 0 and s['name'].startswith('?WheelSineCosine@@'):
                external[s['name']] = 0x43c390
        linked = link_objects([obj], external, start=0x2400000)
        linked['trig'] = 0x43c390
    linked['entries'] = {}
    for kind, prefix in (GCC if gcc else MSVC).items():
        linked['entries'][kind] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
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
        self.linked, self.gcc = linked, gcc
        accu = linked['entries']['accumulate'] if linked else ORIGINAL['accumulate']
        trig = linked['trig'] if linked else 0x43c390
        for label, address in (('accumulate', accu), ('trig', trig), ('unexpected', STOP + 0x100)):
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=label, begin=address, end=address)

    def args(self, n):
        sp = self.u.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(sp + 4, n * 4)))

    def hook(self, u, address, size, label):
        assert label != 'unexpected', 'unexpected sqrtl fallback'
        if label == 'trig':
            sine, cosine, angle = self.args(3)
            self.events.append((label, sine == cosine, angle & 0xffff))
        else:
            if self.gcc:
                receiver, color, *args = self.args(6)
            else:
                receiver = u.reg_read(xc.UC_X86_REG_ECX)
                color, *args = self.args(5)
            self.events.append((label, receiver, bytes(u.mem_read(color, 16)), tuple(args)))

    def run(self, kind, data, args, cw):
        u = self.u
        self.events = []
        u.mem_write(ARENA, data)
        sp = STACK + 0x8000
        methods = kind in ('accumulate', 'pair', 'normalize')
        args = ([RECORD] + args) if self.gcc and methods else args
        u.mem_write(sp, struct.pack('<' + 'I' * (len(args) + 1), STOP, *args))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        u.reg_write(xc.UC_X86_REG_ECX, RECORD)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.linked['entries'][kind] if self.linked else ORIGINAL[kind], STOP, count=50000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        pop = 0 if self.gcc or not methods else {'accumulate': 20, 'pair': 8, 'normalize': 16}[kind]
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4 + pop
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        expected = {'vector': {'trig': 2}, 'accumulate': {'accumulate': 1}, 'pair': {'accumulate': 2}}
        assert Counter(e[0] for e in self.events) == Counter(expected.get(kind, {}))
        return bytes(u.mem_read(ARENA, SIZE)), self.events


def cases(kind, n, exhaustive=False):
    rng = random.Random(ORIGINAL[kind])
    angles = (-32000.0, -720.0, -360.0, -90.0, -1.0, -0.001, -0.0, 0.0, 0.001, 1.0,
              45.0, 90.0, 179.999, 180.0, 270.0, 359.999, 360.0, 720.0, 32000.0)
    for index in range(n):
        data = bytearray(rng.randbytes(SIZE))
        for i in range(0x1c0 // 4):
            value = rng.uniform(-2, 2) * rng.choice((1.0, 1.0, 1e-10, 1e5))
            if index % 8 == 0:
                value = 0.0
            struct.pack_into('<f', data, RECORD - ARENA + i * 4, value)
        color = rng.choice((ARENA + 0x900, RECORD, RECORD + 4, RECORD + 16, RECORD + 0x90))
        direction = rng.choice((ARENA + 0x800, RECORD, RECORD + 4, color))
        struct.pack_into('<4f', data, color - ARENA, *[rng.uniform(-2, 2) for _ in range(4)])
        vector = [rng.uniform(-2, 2) for _ in range(3)]
        if index % 7 == 0:
            vector = [0.0, -0.0, 1.0]
        struct.pack_into('<3f', data, direction - ARENA, *vector)
        if kind == 'clear':
            args = [RECORD]
        elif kind == 'vector':
            azimuth, polar = rng.choice(angles), rng.choice(angles)
            if index >= len(angles):
                azimuth, polar = rng.uniform(-32000, 32000), rng.uniform(-32000, 32000)
            args = [RECORD, fbits(azimuth), fbits(polar), fbits(rng.choice((0.0, -0.0, 1.0, -1.0, rng.uniform(-10, 10))))]
        elif kind == 'accumulate':
            args = [color, fbits(rng.uniform(-2, 2)), *map(fbits, vector)]
        elif kind == 'pair':
            args = [direction, color]
        else:
            args = [rng.choice((0, 1, -1, 2)) & 0xffffffff, fbits(rng.choice((0.0, -1.0, 1.0, 0.3, 10.0))),
                    rng.choice((0, 1, -1, 2)) & 0xffffffff, fbits(rng.choice((0.0, -2.0, 1.0, 0.1, 20.0)))]
        yield bytes(data), args
    if exhaustive and kind == 'vector':
        data = rng.randbytes(SIZE)
        for units in range(65536):
            # Exact binary32 degrees for each quantized angle; polar visits the
            # same full domain with a fixed offset. This is one context per angle,
            # not every combination of azimuth, polar angle and radius.
            azimuth = units * (360.0 / 65536.0)
            polar = ((units + 0x1234) & 0xffff) * (360.0 / 65536.0)
            yield data, [RECORD, fbits(azimuth), fbits(polar), fbits(-1.0 if units & 1 else 1.0)]


def verify(image, linked, gcc, kinds, n, exhaustive=False):
    a, b = Machine(image), Machine(image, linked, gcc)
    counts = Counter()
    for kind in kinds:
        for data, args in cases(kind, n, exhaustive):
            for cw in (0x027f, 0x037f):
                x, y = a.run(kind, data, args, cw), b.run(kind, data, args, cw)
                if x != y:
                    offsets = sorted({j & ~3 for j, (u, v) in enumerate(zip(x[0], y[0])) if u != v})[:6]
                    details = [(hex(j - (RECORD - ARENA)), struct.unpack_from('<f', x[0], j)[0],
                                struct.unpack_from('<f', y[0], j)[0]) for j in offsets]
                    raise AssertionError(('lighting mismatch', kind, counts[kind], hex(cw), args, details,
                                          [i for i, (u, v) in enumerate(zip(x[1], y[1])) if u != v]))
                counts[kind] += 1
    return dict(counts)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--native-trig', action='store_true')
    p.add_argument('--kind', choices=['all'] + list(ORIGINAL), default='all')
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=4096)
    p.add_argument('--mutation-checks', action='store_true')
    p.add_argument('--exhaustive-angles', action='store_true', help='add one context for each 16-bit angle')
    args = p.parse_args()
    assert args.cases > 0
    assert not args.native_trig or args.gcc
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[off:off + f['size']]).hexdigest() == f['sha256']
    for address, value in manifest['constants'].items():
        off = int(address, 16) - 0x400000
        assert image[off:off + len(value) // 2].hex() == value
    linked = build(args.replace or SOURCE, args.gcc, args.native_trig)
    kinds = list(ORIGINAL) if args.kind == 'all' else [args.kind]
    assert not args.exhaustive_angles or 'vector' in kinds
    counts = verify(image, linked, args.gcc, kinds, args.cases, args.exhaustive_angles)
    report = dict(comparisons=counts, failures=0, byte_identity='not established', native_trig=args.native_trig,
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['finite bounded inputs, angles within the signed conversion range',
                               'source-record builder and light-list selection are not covered'])
    if args.exhaustive_angles:
        report['complete_16bit_angle_contexts'] = 65536
    if args.native_trig:
        report['native_trig_object_sha256'] = hashlib.sha256(TRIG_OBJECT.read_bytes()).hexdigest()
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'short-clear': ('clear', 'i < 0x70', 'i < 0x6f'),
            'wrong-polar-axis': ('vector', 'static_cast<long double>(cosPolar) * radius', 'static_cast<long double>(sinPolar) * radius'),
            'wrong-angle-divisor': ('vector', ') / 360;', ') / 180;'),
            'omit-linear-spill': ('accumulate', 'volatile float linearX', 'const long double linearX'),
            'wrong-constant-term': ('accumulate', 'constant * light78D930 -', 'constant * light78D930 +'),
            'wrong-opposite-color': ('pair', 'light78D948 = -0.05f;', 'light78D948 = 0.05f;'),
            'wrong-opposite-x': ('pair', '1.0f, -x, -y, -z', '1.0f, x, -y, -z'),
            'wrong-max-selection': ('normalize', 'stored > maximum', 'stored < maximum'),
            'omit-fourth-root-spill': ('normalize', 'volatile float length =', 'const long double length ='),
            'wrong-fourth-target': ('normalize', 'static_cast<long double>(fourthTarget) / length', 'static_cast<long double>(firstThreeTarget) / length'),
        }
        rejected = []
        for name, (kind, before, after) in mutations.items():
            if kind not in kinds:
                continue
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--kind', kind, '--cases', '256'] +
                               (['--gcc'] if args.gcc else []) + (['--native-trig'] if args.native_trig else []),
                               text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and 'lighting mismatch' in r.stdout, (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc-native-trig' if args.native_trig else 'gcc' if args.gcc else 'msvc') + '-' + args.kind
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
