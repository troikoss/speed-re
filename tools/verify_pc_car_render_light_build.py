#!/usr/bin/env python3
"""Compare source-record builder 0x48ed60 with original PC v1.2 machine code.

The sector-light selector is a controlled boundary on both sides, including
caller count-clamp probes beyond its usual ten-light limit. Other callees execute
original code unless --chain links the recovered lighting foundations. All fixture
bytes, ordered helper inputs, return values and preserved ABI state must match.
"""
import argparse
from collections import Counter
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
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderLightBuild.cpp'
LIGHTING = ROOT / 'src/Speed/Indep/Src/World/CarRenderLighting.cpp'
BUILD = ROOT / 'build/analysis/car-render-light-build'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-light-build.json'
ARENA, SIZE, STACK, STOP, BRIDGE = 0x2000000, 0x6000, 0x2100000, 0x2110000, 0x2300000
OUTPUT, PROFILE, POSITION = ARENA + 0x400, ARENA + 0x1000, ARENA + 0x1200
MATRIX4, MATRIX5, VIEW, FRAME, LIGHTS = [ARENA + i for i in (0x1300, 0x1400, 0x1500, 0x1600, 0x2000)]
ORIGINAL = {'direction': 0x5bb190, 'vector': 0x48a310, 'sine': 0x43c350,
            'normalize': 0x43cf10, 'pair': 0x489af0, 'coefficients': 0x489ed0, 'select': 0x48e3d0}
GCC = {'direction': '_Z24CarRenderDirectionMatrixP11WheelMatrixS0_',
       'vector': '_Z20CarRenderLightVectorPffff', 'sine': '_Z10EngineSinef',
       'normalize': '_Z14WheelNormalizeP11WheelVectorS0_',
       'pair': '_ZN15CarRenderRecord12AddLightPairEPKfS1_',
       'coefficients': '_ZN15CarRenderRecord26NormalizeLightCoefficientsEifif',
       'select': '_Z21CarRenderSelectLightsP23CarRenderLightSelectionPKf'}
PREFIX = {'direction': '?CarRenderDirectionMatrix@@', 'vector': '?CarRenderLightVector@@',
          'sine': '?EngineSine@@', 'normalize': '?WheelNormalize@@',
          'pair': '?AddLightPair@CarRenderRecord@@',
          'coefficients': '?NormalizeLightCoefficients@CarRenderRecord@@', 'select': '?CarRenderSelectLights@@'}
GLOBALS = {'ug2LightNormalize82DAE4': 0x82dae4, 'ug2LightTarget7F39B8': 0x7f39b8,
           'ug2LightNormalize7F39BC': 0x7f39bc, 'ug2LightTarget7F39C0': 0x7f39c0}
ENTRY = '_Z25CarRenderBuildLightRecordP15CarRenderRecordPK21CarRenderLightProfilePKfP11WheelMatrixS7_PK13CarRenderView'


def build(source, gcc, chain):
    sources = [source] + ([LIGHTING] if chain else [])
    if gcc:
        bindings = dict(GLOBALS, sqrtl=STOP + 0x100)
        bindings['_Z15WheelSineCosinePfS_t'] = 0x43c390
        for kind, name in GCC.items():
            if not chain or kind not in ('pair', 'coefficients', 'vector'):
                bindings[name] = BRIDGE + (0 if kind == 'pair' else 0x40) if kind in ('pair', 'coefficients') else ORIGINAL[kind]
        linked = link_native(sources, BUILD, ROOT, bindings, ENTRY)
        linked['entry'] = linked['exports'][ENTRY]
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        objects = []
        for i, path in enumerate(sources):
            obj = BUILD / ('builder-' + str(i) + '.obj')
            subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                            '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                            '/Fo' + str(obj.relative_to(ROOT)), str(path.relative_to(ROOT))], cwd=ROOT, check=True)
            objects.append(obj)
        parsed = [read_object(p) for p in objects]
        defined = {s['name'] for o in parsed for s in o['symbols'].values() if s['section'] > 0}
        external = {'_' + k: v for k, v in GLOBALS.items()}
        external.update({'__fltused': 0, '__ftol2': 0x75bed4})
        for o in parsed:
            for s in o['symbols'].values():
                name = s['name']
                if s['section'] == 0 and name not in defined:
                    for kind, prefix in PREFIX.items():
                        if name.startswith(prefix):
                            external[name] = ORIGINAL[kind]
                    if name.startswith('?WheelSineCosine@@'):
                        external[name] = 0x43c390
        linked = link_objects(objects, external, start=0x2400000)
        linked['entry'] = next(v for k, v in linked['exports'].items() if k.startswith('?CarRenderBuildLightRecord@@'))
    linked['hooks'] = dict(ORIGINAL)
    if chain:
        for kind in ('pair', 'coefficients', 'vector'):
            linked['hooks'][kind] = next(v for k, v in linked['exports'].items()
                                         if k.startswith((GCC if gcc else PREFIX)[kind]))
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False, chain=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        # Original default scalar D3DX transpose implementation.
        u.mem_write(0x803f00, struct.pack('<I', 0x63d132))
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        u.mem_map(BRIDGE, 0x1000)
        for offset, target, nargs in ((0, ORIGINAL['pair'], 2), (0x40, ORIGINAL['coefficients'], 4)):
            where = BRIDGE + offset
            code = b'\x55\x89\xe5'
            for i in range(nargs, 0, -1):
                code += b'\xff\x75' + bytes([8 + i * 4])
            code += b'\x8b\x4d\x08\xe8'
            code += struct.pack('<i', target - (where + len(code) + 4))
            u.mem_write(where, code + b'\x89\xec\x5d\xc3')
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
        self.entry = linked['entry'] if linked else 0x48ed60
        self.native_methods = bool(linked and gcc and chain)
        for kind, address in (linked['hooks'] if linked else ORIGINAL).items():
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=kind, begin=address, end=address)
        u.hook_add(UC_HOOK_CODE, self.hook, user_data='unexpected', begin=STOP + 0x100, end=STOP + 0x100)

    def args(self, n):
        sp = self.u.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(sp + 4, n * 4)))

    def hook(self, u, address, size, kind):
        assert kind != 'unexpected', 'unexpected sqrtl fallback'
        if kind == 'select':
            output, position = self.args(2)
            self.events.append((kind, position, bytes(u.mem_read(position, 12))))
            u.mem_write(output, self.selection)
            sp = u.reg_read(xc.UC_X86_REG_ESP)
            ret = struct.unpack('<I', bytes(u.mem_read(sp, 4)))[0]
            u.reg_write(xc.UC_X86_REG_ESP, sp + 4)
            u.reg_write(xc.UC_X86_REG_EAX, self.count & 0xffffffff)
            u.reg_write(xc.UC_X86_REG_EIP, ret)
        elif kind == 'direction':
            output, matrix = self.args(2)
            self.events.append((kind, matrix, bytes(u.mem_read(matrix, 64))))
        elif kind == 'vector':
            output, *args = self.args(4)
            self.events.append((kind, tuple(args)))
        elif kind == 'sine':
            self.events.append((kind, self.args(1)))
        elif kind == 'normalize':
            output, vector = self.args(2)
            self.events.append((kind, output == vector, bytes(u.mem_read(vector, 12))))
        else:
            nargs = 2 if kind == 'pair' else 4
            if self.native_methods:
                receiver, *args = self.args(nargs + 1)
            else:
                receiver, args = u.reg_read(xc.UC_X86_REG_ECX), self.args(nargs)
            prefix = bytes(u.mem_read(receiver, 160))
            values = tuple(bytes(u.mem_read(p, 12 if i == 0 else 16)) for i, p in enumerate(args)) if kind == 'pair' else tuple(args)
            self.events.append((kind, prefix, values))

    def run(self, case, cw):
        data, args, self.selection, self.count, globals_ = case
        u = self.u
        self.events = []
        u.mem_write(ARENA, data)
        for address, value in globals_.items():
            u.mem_write(address, struct.pack('<I', value))
        sp = STACK + 0x8000
        u.mem_write(sp, struct.pack('<7I', STOP, *args))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entry, STOP, count=300000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        assert u.reg_read(xc.UC_X86_REG_EAX) == (1 if args[0] else 0xffffffff)
        return bytes(u.mem_read(ARENA, SIZE)), self.events


def fbits(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]


def cases(n):
    rng = random.Random(0x48ed60)
    for index in range(n):
        data = bytearray(rng.randbytes(SIZE))
        def put(address, fmt, *args):
            struct.pack_into('<' + fmt, data, address - ARENA, *args)
        for matrix in (MATRIX4, MATRIX5, FRAME):
            values = [rng.uniform(-2, 2) for _ in range(16)]
            if index % 7 == 0:
                values = [1.0 if i % 5 == 0 else 0.0 for i in range(16)]
            put(matrix, '16f', *values)
        for i in range(3):
            flag = (0, 1, 0x80000000, 0xffffffff)[(index + i) % 4]
            put(PROFILE + 0x20 + i * 0x1c, 'I6f', flag, rng.uniform(-720, 720), rng.uniform(-360, 360),
                rng.uniform(-2, 2), rng.uniform(-2, 2), rng.uniform(-2, 2), rng.uniform(-2, 2))
        position = [rng.uniform(-100, 100) for _ in range(3)]
        put(POSITION, '3f', *position)
        put(VIEW + 0x40, 'I', 0 if index % 3 == 0 else FRAME)
        selection = bytearray(rng.randbytes(0xd0))
        count = (-3, -1, 0, 1, 2, 3, 9, 10, 15, 16, 17, 19, 24)[index % 13]
        for i in range(24):
            light = LIGHTS + 0x40 * i
            struct.pack_into('<I', selection, i * 4, light)
            struct.pack_into('<f', selection, 0x60 + i * 4, rng.choice((0.0, -1.0, 1.0, rng.uniform(-2, 2))))
            put(light + 4, 'B', rng.choice((0, 1, 2, 3, 3, 4)))
            put(light + 0xc, 'I', rng.getrandbits(32))
            coordinates = position if index % 8 == 0 else [rng.uniform(-100, 100) for _ in range(3)]
            put(light + 0x10, '3f', *coordinates)
            put(light + 0x20, '3f', *[rng.uniform(-2, 2) for _ in range(3)])
            put(light + 0x34, 'f', rng.choice((0.0, 3.141592653589793, -3.141592653589793, rng.uniform(-6, 6))))
        # Output aliases are safe only if all required inputs were read before stores.
        output = rng.choice((OUTPUT, OUTPUT, PROFILE, POSITION, MATRIX4, FRAME, LIGHTS, VIEW))
        if index % 31 == 0:
            output = 0
        args = [output, PROFILE, POSITION, MATRIX4 if index & 1 else 0, MATRIX5, VIEW if index & 2 else 0]
        if not output:
            args = [0] * 6  # Early return must not touch any of the other arguments.
        globals_ = {0x82dae4: rng.choice((0, 1, 0xffffffff)), 0x7f39b8: fbits(rng.choice((0.0, 1.0, -1.0, 0.3))),
                    0x7f39bc: rng.choice((0, 1, 2)), 0x7f39c0: fbits(rng.choice((0.0, 0.8, 1.0, -0.5)))}
        yield bytes(data), args, bytes(selection), count, globals_


def verify(image, linked, gcc, chain, n):
    a, b = Machine(image), Machine(image, linked, gcc, chain)
    totals = Counter()
    for index, case in enumerate(cases(n)):
        for cw in (0x027f, 0x037f):
            x, y = a.run(case, cw), b.run(case, cw)
            if x != y:
                offsets = sorted({j & ~3 for j, (u, v) in enumerate(zip(x[0], y[0])) if u != v})[:6]
                details = [(hex(j), x[0][j:j + 4].hex(), y[0][j:j + 4].hex()) for j in offsets]
                events = [(j, u, v) for j, (u, v) in enumerate(zip(x[1], y[1])) if u != v][:1]
                raise AssertionError(('builder mismatch', index, hex(cw), case[1], case[3], details, events))
            totals['comparisons'] += 1
            if cw == 0x027f:
                totals.update(e[0] for e in x[1])
    return dict(totals)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--chain', action='store_true')
    p.add_argument('--cases', type=int, default=2048)
    p.add_argument('--replace', type=Path)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    assert args.cases > 0
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
    linked = build(args.replace or SOURCE, args.gcc, args.chain)
    report = dict(results=verify(image, linked, args.gcc, args.chain, args.cases), failures=0,
                  compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  recovered_lighting_chain=args.chain, byte_identity='not established',
                  limitations=['sector-light selector supplied by a controlled fixture; not recovered here',
                               'direction transpose, sine, three-component normalization and trig execute original code',
                               'finite bounded inputs; native source-record activation remains open'])
    if args.mutation_checks:
        mutations = {
            'omit-view-override': ('if (view && view->frame40)', 'if (false && view && view->frame40)'),
            'float-profile-flag': ('if (light.flag00)', 'if (*reinterpret_cast<const float *>(&light.flag00) != 0.0f)'),
            'wrong-polar-offset': ('light784E8C = 90.0f', 'light784E8C = 180.0f'),
            'wrong-count-clamp': ('count = 15;', 'count = 16;'),
            'wrong-cone-kind': ('light->kind04 == 3', 'light->kind04 == 2'),
            'wrong-cone-comparison': ('dot > threshold', 'dot < threshold'),
            'round-cone-dot': ('const long double dot =', 'volatile float dot ='),
            'omit-extra-transform': ('if (matrixArg4)', 'if (false && matrixArg4)'),
            'wrong-luma-channel': ('colors[i][2]) * light78D9B8', 'colors[i][1]) * light78D9B8'),
            'transpose-output-directions': ('elements[lane * 4 + i]', 'elements[i * 4 + lane]'),
            'short-coefficient-copy': ('output->value120[i] = words[i];', 'if (i < 39) output->value120[i] = words[i];'),
        }
        source = SOURCE.read_text()
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            command = [sys.executable, __file__, '--replace', str(path), '--cases', '256']
            command += ['--gcc'] if args.gcc else []
            command += ['--chain'] if args.chain else []
            result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout)
            assert result.returncode and 'builder mismatch' in result.stdout, (name, result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + ('-chain' if args.chain else '-isolated')
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
