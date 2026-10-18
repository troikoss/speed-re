#!/usr/bin/env python3
"""Compare the camera view-matrix helpers: look-at 0x5ba9a0, rigid inverse 0x5baf90 and
re-aim 0x444390, under 53- and 64-bit x87 precision.

Recovered normalize 0x43cf10, cross 0x43d1e0 and matrix copy 0x4012b0 execute original
code in both runs. The statically linked D3DX multiply 0x63cedb dispatches through a
runtime-patched pointer (0x803e74); both runs reach a traced stub whose arguments and
input matrices must match, and which writes the binary32-rounded row-vector product."""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/camera-matrix'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x1000, 0x2010000, 0x2100000
D3DX = 0x63cedb
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
# name: (symbol prefix, stack argument count, callee cleanup bytes, returns EAX)
TARGETS = {'LookAt': ('?CameraLookAt@', 4, 0, False), 'InvertRigid': ('?CameraInvertRigid@', 2, 0, True),
           'Reaim': ('?CameraReaim@', 4, 16, False)}
SHARED = {'?WheelNormalize@': 0x43cf10, '?WheelCross@': 0x43d1e0, '?BodyMatrixCopy@': 0x4012b0,
          '_D3DXMatrixMultiply@12': D3DX, '?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250,
          '__fltused': 0}
VIEW, OUT, EYE, TARGET, UP, OFFSET = (ARENA + x for x in (0x100, 0x200, 0x300, 0x320, 0x340, 0x360))


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def value(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, -3e-7, 1e30, float('inf'), float('nan'),
                       rng.uniform(-2, 2), rng.uniform(-5000, 5000), rng.uniform(-5000, 5000)))


def vector(rng, special=0.15):
    if rng.random() < special: return [value(rng) for _ in range(3)]
    if rng.random() < 0.2:  # axis-aligned vectors keep signed zeros in the zero-weighted sums
        v = [0.0, 0.0, 0.0]; v[rng.randrange(3)] = rng.choice((1.0, -1.0, rng.uniform(-50, 50))); return v
    return [rng.uniform(-3000, 3000) for _ in range(3)]


def rigid(rng):
    import math
    a, b, c = (rng.uniform(-math.pi, math.pi) for _ in range(3))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    r = [[cb * cc, cb * sc, -sb], [sa * sb * cc - ca * sc, sa * sb * sc + ca * cc, sa * cb],
         [ca * sb * cc + sa * sc, ca * sb * sc - sa * cc, ca * cb]]
    t = vector(rng, 0)
    return [r[0][0], r[0][1], r[0][2], 0, r[1][0], r[1][1], r[1][2], 0, r[2][0], r[2][1], r[2][2], 0, t[0], t[1], t[2], 1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Camera/CameraMatrix.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-matrix.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Camera', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        match = [a for p, a in SHARED.items() if s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, traces = [], [[], []]
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            # Rebuilt code must not reach original targets; the original must not reach rebuilt code.
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def d3dx(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            ret, out, left, right = struct.unpack('<4I', uc.mem_read(esp, 16))
            l = struct.unpack('<16f', uc.mem_read(left, 64)); r = struct.unpack('<16f', uc.mem_read(right, 64))
            trace.append((out - (STACK if out >= STACK - 0x1000 and out < STACK else 0), bytes(uc.mem_read(left, 64)), bytes(uc.mem_read(right, 64)),
                          left == out or right == out))
            p = [f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4))) for i in range(4) for j in range(4)]
            uc.mem_write(out, struct.pack('<16f', *p))
            uc.reg_write(xc.UC_X86_REG_EAX, out); uc.reg_write(xc.UC_X86_REG_ESP, esp + 16); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_BLOCK, block); uc.hook_add(UC_HOOK_CODE, d3dx, begin=D3DX, end=D3DX); machines.append(uc)
    rng = random.Random(0x5ba9a0); counts = dict.fromkeys(TARGETS, 0)

    def run(name, memory, argv, precision):
        _, argc, cleanup, returns = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=200000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            # D3DX outputs on the stack are compared by their (normalised) inputs, not addresses.
            trace = [(t[1], t[2], t[3]) for t in traces[rebuilt]]
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) if returns else 0, uc.reg_read(xc.UC_X86_REG_ESP),
                            tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW), trace,
                            bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][5][i:i + 4].hex(), outputs[1][5][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][5][i:i + 4] != outputs[1][5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), [hex(x) for x in argv], outputs[0][:4], outputs[1][:4],
                                  len(outputs[0][4]), len(outputs[1][4]), outputs[0][4] == outputs[1][4], diff))
        counts[name] += 1

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *values)
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        eye, target, up = vector(rng), vector(rng), vector(rng)
        mode = rng.random()
        if mode < 0.1: target = list(eye)                                   # zero forward
        elif mode < 0.2: up = [x - y for x, y in zip(target, eye)]          # up parallel to forward
        elif mode < 0.4: up = rng.choice(([0, 0, 1], [0, 0, -1], [0, 1, 0], [rng.uniform(-.2, .2), rng.uniform(-.2, .2), 1]))
        put(memory, EYE, eye); put(memory, TARGET, target); put(memory, UP, up)
        out = rng.choice((OUT, OUT, OUT, EYE, TARGET, UP))  # aliasing pins the write/read order
        run('LookAt', memory, [out, EYE, TARGET, UP], precision)
        put(memory, VIEW, rigid(rng) if rng.random() < 0.7 else [value(rng) for _ in range(16)])
        run('InvertRigid', memory, [rng.choice((OUT, OUT, VIEW)), VIEW], precision)
        put(memory, VIEW, rigid(rng) if rng.random() < 0.85 else [value(rng) for _ in range(16)])
        put(memory, EYE, vector(rng, 0.05)); put(memory, TARGET, vector(rng, 0.05))
        put(memory, OFFSET, [rng.choice((0.0, -0.0, rng.uniform(-1, 1), rng.uniform(-1, 1), value(rng))) for _ in range(2)])
        run('Reaim', memory, [VIEW, EYE, TARGET, OFFSET], precision)
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  traced_stubs=['D3DX matrix multiply 0x63cedb (row-vector product, binary32 rounding)'],
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'normalize, cross and matrix copy bodies shared, verified elsewhere',
                               'D3DX implementation selected at run time is not executed',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'lookat-cross-order': ('WheelCross(&right, &down, &forward);', 'WheelCross(&right, &forward, &down);'),
            'lookat-skip-second-normalize': ('    WheelNormalize(&down, &down);\n', ''),
            'lookat-fold-zero': ('p[0] = right.x + right.y * zero + right.z * zero;', 'p[0] = right.x;'),
            'lookat-fold-one': ('p[15] = ez * zero + ey * zero + ex * zero + fengOne784250;', 'p[15] = 1.0f;'),
            'lookat-translation-order': ('p[12] = right.z * ez + ey * right.y + ex * right.x;', 'p[12] = ex * right.x + ey * right.y + right.z * ez;'),
            'lookat-late-rotation': ('    volatile float ex = -eye->x, ey = -eye->y, ez = -eye->z;\n', None),
            'invert-swap': ('D3DXMatrixMultiply(output, &translation, &rotation);', 'D3DXMatrixMultiply(output, &rotation, &translation);'),
            'invert-no-transpose': ('    r[1] = m[4];\n    r[2] = m[8];\n    r[4] = m[1];', '    r[1] = m[1];\n    r[2] = m[8];\n    r[4] = m[4];'),
            'reaim-extended-x': ('    volatile float x = l[8]', '    float x = l[8]'),
            'reaim-rounded-y': ('    float y = l[5]', '    volatile float y = l[5]'),
            'reaim-swap-offset': ('    x = x - depth * offset[0];\n    y = y - depth * offset[1];', '    x = x - depth * offset[1];\n    y = y - depth * offset[0];'),
            'reaim-own-up': ('    down.x = -view->elements[1];', '    down.x = -view->elements[0];'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label
            if label == 'lookat-late-rotation':
                # Read the eye before the rotation is written (observable when output aliases eye).
                text = text.replace(before, '', 1)
                text = text.replace('    float *m = output->elements;\n', before + '    float *m = output->elements;\n', 1)
            else: text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '2048'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
