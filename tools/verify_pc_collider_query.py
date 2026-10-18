#!/usr/bin/env python3
"""Compare the collider query 0x5e6fa0 and its 2D bounds helpers 0x43d220, 0x43d2f0,
0x43d470 and 0x43d090, under 53- and 64-bit x87 precision.

WCollider::UpdatePack 0x5e68d0 and RefreshArea 0x5e5890 run as traced stubs in both
runs; their arguments and the collider bounds at each call must match."""

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

BUILD = ROOT / 'build/analysis/collider-query'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x1000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
# name: (symbol prefix, stack argument count, callee cleanup bytes, returns EAX)
TARGETS = {'Bounds2Reset': ('?Bounds2Reset@', 2, 0, False), 'Bounds2Expand': ('?Bounds2Expand@', 4, 0, False),
           'Bounds2Contains': ('?Bounds2Contains@', 4, 0, True), 'ScaleAdd2': ('?ScaleAdd2@', 4, 0, False),
           'QueryArea': ('?QueryArea@WCollider@', 3, 12, False)}
# Collider methods reached from 0x5e6fa0 run as traced stubs in both runs: name, address, cleanup.
STUBS = {0x5e68d0: ('UpdatePack', 8), 0x5e5890: ('RefreshArea', 0)}
SHARED = {'?UpdatePack@WCollider@': 0x5e68d0, '?RefreshArea@WCollider@': 0x5e5890, '?collisionAreaHalf@': 0x784260,
          '?colliderQuerySpan7A5338@': 0x7a5338, '?colliderQueryDecay785510@': 0x785510, '__fltused': 0}
COLLIDER, MIN, MAX, POINT, VELOCITY, OUT = (ARENA + x for x in (0x100, 0x500, 0x510, 0x520, 0x530, 0x540))


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def value(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, -3e-7, 3.4028235e38, -3.4028235e38, 1e30, float('inf'), float('-inf'),
                       float('nan'), rng.uniform(-2, 2), rng.uniform(-5000, 5000)))


def number(rng, special=0.15):
    return value(rng) if rng.random() < special else rng.uniform(-4000, 4000)


def straddle(rng, limit=28.799999237060547):
    """Radius and velocity whose swept area diagonal and its binary32-rounded value fall on
    opposite sides of the 0x7a5338 limit (53-bit precision model, point at the origin)."""
    import math
    s15 = f32(0.15)
    for _ in range(1000):
        r = f32(rng.uniform(0.5, 6)); R = f32(r + 0.5)
        vy = f32(rng.uniform(0, 40)); dy = -R - f32(f32(s15 * vy) + R)
        if limit * limit <= dy * dy: continue
        vx = f32((math.sqrt(limit * limit - dy * dy) - R - R) / s15)
        for k in range(-3, 4):
            v = f32(vx + k * vx * 2 ** -23)
            dx = -R - f32(f32(s15 * v) + R); total = dx * dx + dy * dy
            if (math.sqrt(total) < limit) != (f32(math.sqrt(f32(total))) < limit): return r, v, vy
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/World/WColliderQuery.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/collider-query.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/World', '/Isrc/Speed/Indep/Src/Physics', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
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
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            name, cleanup = STUBS[address]
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
            trace.append((name, uc.reg_read(xc.UC_X86_REG_ECX), bytes(uc.mem_read(esp + 4, cleanup)), bytes(uc.mem_read(COLLIDER + 0x1c, 0x28))))
            uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in STUBS: uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x5e6fa0); counts = dict.fromkeys(TARGETS, 0); paths = {'inside': 0, 'no-velocity': 0, 'swept': 0}

    def run(name, memory, argv, precision, this=0):
        _, argc, cleanup, returns = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=200000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) if returns else 0, uc.reg_read(xc.UC_X86_REG_ESP),
                            tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW), list(traces[rebuilt]),
                            bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][5][i:i + 4].hex(), outputs[1][5][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][5][i:i + 4] != outputs[1][5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), [x.hex() if isinstance(x, bytes) else hex(x) for x in argv], outputs[0][:4], outputs[1][:4],
                                  [t[:3] for t in outputs[0][4]], [t[:3] for t in outputs[1][4]], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        put(memory, MIN, [number(rng) for _ in range(2)]); put(memory, MAX, [number(rng) for _ in range(2)])
        put(memory, POINT, [number(rng) for _ in range(2)])
        radius = struct.pack('<f', rng.choice((0.0, -0.0, 1.0, rng.uniform(0, 50), rng.uniform(0, 50), number(rng, 0.3))))
        run('Bounds2Reset', memory, [MIN, MAX], precision)
        bounds = (MIN, MAX) if rng.random() < 0.8 else (MIN, MIN)
        point = rng.choice((POINT, POINT, POINT, MIN, MAX))  # aliasing pins read-before-write
        run('Bounds2Expand', memory, [bounds[0], bounds[1], point, radius], precision)
        put(memory, OUT, [number(rng) for _ in range(2)]); put(memory, OUT + 8, [number(rng) for _ in range(2)])
        if rng.random() < 0.3:  # equal edges exercise the inclusive tests
            struct.pack_into('<4f', memory, OUT - ARENA, *struct.unpack('<4f', memory[MIN - ARENA:MIN - ARENA + 8] + memory[MAX - ARENA:MAX - ARENA + 8]))
        run('Bounds2Contains', memory, [MIN, MAX, OUT, OUT + 8], precision)
        put(memory, VELOCITY, [number(rng) for _ in range(2)])
        run('ScaleAdd2', memory, [rng.choice((OUT, POINT, POINT + 4, VELOCITY + 4)), POINT, VELOCITY, struct.pack('<f', number(rng, 0.3))], precision)
        # Collider query: area +0x2c/+0x34 either already covering the sphere or not.
        cx, cy = rng.uniform(-4000, 4000), rng.uniform(-4000, 4000)
        r = rng.choice((0.0, 1.0, 2.5, rng.uniform(0, 30), number(rng, 0.2)))
        put(memory, POINT, [cx, cy, rng.uniform(-50, 50)])
        span = rng.choice((r + 0.1, r + 5, rng.uniform(0, 200), number(rng, 0.2)))
        put(memory, COLLIDER + 0x2c, [cx - span, cy - span * rng.uniform(.5, 1.5)]); put(memory, COLLIDER + 0x34, [cx + span, cy + span])
        speed = rng.choice((0.0, 5.0, 40.0, 90.0, 300.0, rng.uniform(-200, 200), number(rng, 0.2)))
        put(memory, VELOCITY, [speed * rng.uniform(-1, 1), speed * rng.uniform(-1, 1), 0])
        velocity = 0 if rng.random() < 0.3 else VELOCITY
        if trial % 8 == 0:
            case = straddle(rng)
            if case:
                r, vx, vy = case; precision = 0x027f; velocity = VELOCITY; paths['straddle'] = paths.get('straddle', 0) + 1
                put(memory, POINT, [0.0, 0.0, 0.0]); put(memory, VELOCITY, [vx, vy, 0.0])
                put(memory, COLLIDER + 0x2c, [1e6, 1e6]); put(memory, COLLIDER + 0x34, [1e6 + 1, 1e6 + 1])
        result = run('QueryArea', memory, [POINT, struct.pack('<f', f32(r)), velocity], precision, COLLIDER)
        refreshes = [t for t in result[4] if t[0] == 'RefreshArea']
        paths['inside' if not refreshes else 'no-velocity' if not velocity else 'swept'] += 1
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), query_paths=paths,
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  traced_stubs=['WCollider::UpdatePack 0x5e68d0 (this, point, radius; bounds at call)',
                                'WCollider::RefreshArea 0x5e5890 (this; bounds at call)'],
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'UpdatePack/RefreshArea bodies recovered and verified separately; stubbed here',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'reset-swap': ('    minimum[0] = 3.40282347e+38f;', '    minimum[0] = -3.40282347e+38f;'),
            'expand-nan-keeps-old': ('float minX = minimum[0] < lowX ? minimum[0] : lowX;', 'float minX = lowX < minimum[0] ? lowX : minimum[0];'),
            'expand-early-store': ('    float lowY = point[1] - radius;\n    float minY', '    minimum[0] = minX;\n    float lowY = point[1] - radius;\n    float minY'),
            'contains-strict': ('if (!(innerMinimum[0] >= outerMinimum[0])) return 0;', 'if (!(innerMinimum[0] > outerMinimum[0])) return 0;'),
            'contains-nan-passes': ('if (!(innerMaximum[1] <= outerMaximum[1])) return 0;', 'if (innerMaximum[1] > outerMaximum[1]) return 0;'),
            'scaleadd-early-x': ('    float y = scale * direction[1] + base[1];\n    float x = scale * direction[0] + base[0];\n    output[0] = x;', '    float x = scale * direction[0] + base[0];\n    output[0] = x;\n    float y = scale * direction[1] + base[1];'),
            'query-no-update': ('    UpdatePack(point, 0.0f);\n', ''),
            'query-no-half': ('point, radius + collisionAreaHalf);', 'point, radius);'),
            'query-swept-half-late': ('    radius = radius + collisionAreaHalf;\n', ''),
            'query-scale': ('    float scale = 0.15f;', '    float scale = 0.25f;'),
            'query-rounded-span': ('if (sqrt(dy * dy + dx * dx) < colliderQuerySpan7A5338) break;', 'if ((float)sqrt((float)(dy * dy + dx * dx)) < colliderQuerySpan7A5338) break;'),
            'query-zero-at-2': ('if (attempt == 3) scale = 0.0f;', 'if (attempt == 2) scale = 0.0f;'),
            'query-four-tries': ('if (++attempt >= 5) break;', 'if (++attempt >= 4) break;'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '2048'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
