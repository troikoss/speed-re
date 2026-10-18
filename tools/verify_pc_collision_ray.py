#!/usr/bin/env python3
"""Compare the collision ray: 3D bounds helpers 0x43d540, 0x43d570, 0x43d610, 0x43d820,
face point test 0x5d35b0, segment/plane hit 0x5d36f0, face ray hit 0x5d37f0 and the
collider ray 0x5d39f0, under 53- and 64-bit x87 precision.

Recovered cross 0x43d1e0, scale-add 0x43d0c0 and set-length 0x43cf90 execute original
code in both runs; the rebuilt routines call each other."""


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

BUILD = ROOT / 'build/analysis/collision-ray'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x1000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
# name: (symbol prefix, stack argument count, callee cleanup bytes, returns AL/EAX mask)
TARGETS = {'Bounds3Set': ('?Bounds3Set@', 3, 0, 0), 'Bounds3Add': ('?Bounds3Add@', 3, 0, 0),
           'Bounds3AddSphere': ('?Bounds3AddSphere@', 4, 0, 0), 'Bounds3Overlap': ('?Bounds3Overlap@', 4, 0, 0xffffffff),
           'ContainsPoint': ('?ContainsPoint@WCollisionFace@', 2, 8, 0xff), 'SegmentHit': ('?SegmentHit@WCollisionFace@', 3, 12, 0xff),
           'RayHit': ('?RayHit@WCollisionFace@', 7, 28, 0xff), 'Ray': ('?Ray@WCollider@', 6, 24, 0xff)}
SHARED = {'?WheelCross@': 0x43d1e0, '?WheelScaleAdd@': 0x43d0c0, '?WheelSetLength@': 0x43cf90,
          '?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250, '?rayMinusOne7842A0@': 0x7842a0, '__fltused': 0}
COLLIDER, FACES, MIN, MAX, MIN2, MAX2, A, B, OUT, DIST = (ARENA + x for x in (0x100, 0x800, 0xc00, 0xc10, 0xc20, 0xc30, 0xc40, 0xc50, 0xc60, 0xc70))
FACE_COUNT = 6


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def value(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, -3e-7, 3.4028235e38, 1e30, float('inf'), float('-inf'),
                       float('nan'), rng.uniform(-2, 2), rng.uniform(-5000, 5000)))


def number(rng, special=0.1):
    return value(rng) if rng.random() < special else rng.uniform(-60, 60)


def sub(a, b): return [x - y for x, y in zip(a, b)]
def cross(a, b): return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]
def dot(a, b): return sum(x * y for x, y in zip(a, b))


def face(rng, special=0.05):
    """Planar triangle or quad with its unit normal (either winding) and plane distance."""
    import math
    centre = [rng.uniform(-30, 30) for _ in range(3)]
    u = [rng.gauss(0, 1) for _ in range(3)]
    if rng.random() < 0.4: u = [rng.uniform(-.3, .3), rng.uniform(-.3, .3), 1.0]  # mostly horizontal ground
    if rng.random() < 0.3: u = [rng.gauss(0, 1), rng.gauss(0, 1), rng.uniform(-.2, .2)]  # walls
    n = math.sqrt(dot(u, u)) or 1.0; u = [x / n for x in u]
    t = cross(u, [1, 0, 0] if abs(u[0]) < 0.9 else [0, 1, 0]); n = math.sqrt(dot(t, t)); t = [x / n for x in t]
    w = cross(u, t)
    count = rng.choice((3, 4))
    size = rng.uniform(1, 25)
    angles = sorted(rng.uniform(0, 2 * math.pi) for _ in range(count))
    if count == 4 and rng.random() < 0.5: angles = [0.3, 1.9, 3.4, 4.9]
    verts = [[centre[k] + size * (math.cos(a) * t[k] + math.sin(a) * w[k]) for k in range(3)] for a in angles]
    if rng.random() < 0.5: verts.reverse()
    verts = [[f32(x) for x in v] for v in verts]
    normal = cross(sub(verts[1], verts[0]), sub(verts[2], verts[0])); n = math.sqrt(dot(normal, normal)) or 1.0
    normal = [f32(x / n) for x in normal]
    if rng.random() < 0.15: normal = [-x for x in normal]
    if rng.random() < special: normal[rng.randrange(3)] = value(rng)
    return count, verts, normal, f32(dot(normal, verts[0]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/World/WCollisionRay.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/collision-ray.json').read_text())
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
    machines = []
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
        uc.hook_add(UC_HOOK_BLOCK, block); machines.append(uc)
    rng = random.Random(0x5d39f0); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, argv, precision, this=0):
        _, argc, cleanup, mask = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=500000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) & mask, uc.reg_read(xc.UC_X86_REG_ESP),
                            tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW), uc.reg_read(xc.UC_X86_REG_FPTAG),
                            bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][5][i:i + 4].hex(), outputs[1][5][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][5][i:i + 4] != outputs[1][5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), [x.hex() if isinstance(x, bytes) else hex(x) for x in argv], outputs[0][:5], outputs[1][:5], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def put_face(memory, address, spec, flags):
        count, verts, normal, distance = spec
        memory[address + 8 - ARENA:address + 12 - ARENA] = bytes((rng.randrange(256), count, flags, rng.randrange(256)))
        for i, v in enumerate(verts): put(memory, address + 0x30 + i * 0x10, v)
        put(memory, address + 0x70, normal); put(memory, address + 0x7c, [distance])
    def note(path): paths[path] = paths.get(path, 0) + 1
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # Bounds helpers with aliasing (point inside the bounds storage) and equal edges.
        for address in (MIN, MAX, MIN2, MAX2, A): put(memory, address, [number(rng) for _ in range(3)])
        point = rng.choice((A, A, A, MIN, MIN + 4, MAX + 4))
        run('Bounds3Set', memory, [MIN, MAX, point], precision)
        if rng.random() < 0.5:  # empty (inverted) or random bounds
            empty = rng.random() < 0.5
            put(memory, MIN, [3.4028235e38 if empty else number(rng) for _ in range(3)]); put(memory, MAX, [-3.4028235e38 if empty else number(rng) for _ in range(3)])
        run('Bounds3Add', memory, [MIN, MAX, rng.choice((A, A, MIN2, MAX, MIN + 4, MIN + 8))], precision)
        run('Bounds3AddSphere', memory, [MIN, MAX, rng.choice((A, A, MIN)), struct.pack('<f', f32(rng.choice((0.0, 1.0, rng.uniform(0, 20), number(rng)))))], precision)
        if rng.random() < 0.4:  # overlapping boxes touching on one axis
            lo = [rng.uniform(-50, 0) for _ in range(3)]; put(memory, MIN, lo); put(memory, MAX, [x + 30 for x in lo])
            put(memory, MIN2, [x + 10 for x in lo]); put(memory, MAX2, [x + 20 for x in lo])
            k = rng.randrange(3); src, dst = rng.choice(((MAX, MIN2), (MIN, MAX2)))
            memory[dst + 4 * k - ARENA:dst + 4 * k + 4 - ARENA] = memory[src + 4 * k - ARENA:src + 4 * k + 4 - ARENA]
        run('Bounds3Overlap', memory, [MIN, MAX, MIN2, MAX2], precision)
        # Face tests on random planar faces.
        spec = face(rng); flags = rng.choice((0, 2, 8, 10)); face0 = FACES
        put_face(memory, face0, spec, flags)
        count, verts, normal, distance = spec
        centre = [sum(v[k] for v in verts) / count for k in range(3)]
        p = [c + rng.uniform(-15, 15) for c in centre] if rng.random() < 0.8 else [number(rng) for _ in range(3)]
        put(memory, A, p)
        result = run('ContainsPoint', memory, [A, rng.choice((DIST, 0))], precision, face0); note('contains' if result[0] else 'outside')
        side = [normal[k] * rng.uniform(1, 30) for k in range(3)]
        target = [c + rng.uniform(-12, 12) for c in centre]
        start = [t + s for t, s in zip(target, side)]; end = [t - s * rng.uniform(0.2, 2) for t, s in zip(target, side)]
        if rng.random() < 0.15: end = list(start)
        if rng.random() < 0.1: end = [number(rng) for _ in range(3)]
        if rng.random() < 0.3: start, end = end, start
        put(memory, A, start); put(memory, B, end)
        result = run('SegmentHit', memory, [A, B, rng.choice((OUT, 0))], precision, face0); note('segment-hit' if result[0] else 'segment-miss')
        best = rng.choice((999999.0, rng.uniform(0, 60), 0.0))
        da, db = dot(sub(start, verts[0]), normal), dot(sub(end, verts[0]), normal)
        if rng.random() < 0.4 and da * db < 0:  # best within rounding of the hit length
            import math
            best = abs(da) / (abs(db) + abs(da)) * math.sqrt(dot(sub(end, start), sub(end, start)))
            best = rng.choice((f32(best), math.nextafter(f32(best), math.inf), math.nextafter(f32(best), -math.inf)))
        put(memory, DIST, [best])
        lo = [min(a, b) - rng.uniform(0, 2) for a, b in zip(start, end)]; hi = [max(a, b) + rng.uniform(0, 2) for a, b in zip(start, end)]
        if rng.random() < 0.2: lo = [x + 40 for x in lo]; hi = [x + 40 for x in hi]
        put(memory, MIN, lo); put(memory, MAX, hi)
        if rng.random() < 0.12:  # exact geometry: hit length equals best
            c, h = float(rng.randrange(-20, 20)), float(rng.randrange(1, 9))
            sign = rng.choice((1.0, -1.0)); quad = [[-4, -4, c], [4, -4, c], [4, 4, c], [-4, 4, c]]
            if sign < 0: quad.reverse()
            put_face(memory, face0, (4, quad, [0.0, 0.0, sign], sign * c), 0)
            start, end = [0.5, 0.25, c + h], [0.5, 0.25, c - h]
            if rng.random() < 0.5: start, end = end, start
            put(memory, A, start); put(memory, B, end); put(memory, DIST, [h])
            put(memory, MIN, [-10, -10, c - 20]); put(memory, MAX, [10, 10, c + 20])
        result = run('RayHit', memory, [DIST, A, B, MIN, MAX, rng.choice((0, 1)), rng.choice((OUT, 0))], precision, face0)
        note('rayhit' if result[0] else 'raymiss')
        # Collider ray across several faces.
        faces = rng.randrange(1, FACE_COUNT + 1)
        for i in range(FACE_COUNT):
            put_face(memory, FACES + i * 0x80, face(rng, 0.02), rng.choice((0, 2, 8, 10)))
            struct.pack_into('<I', memory, COLLIDER + 0x3c + i * 4 - ARENA, FACES + i * 0x80)
        struct.pack_into('<i', memory, COLLIDER + 0x2e0 - ARENA, faces)
        spec = face(rng, 0); put_face(memory, FACES, spec, rng.choice((0, 2, 8, 10)))
        count, verts, normal, distance = spec
        centre = [sum(v[k] for v in verts) / count for k in range(3)]
        side = [normal[k] * rng.uniform(1, 30) for k in range(3)]
        target = [c + rng.uniform(-8, 8) for c in centre]
        start = [t + s for t, s in zip(target, side)]; end = [t - s * rng.uniform(0.2, 2) for t, s in zip(target, side)]
        if rng.random() < 0.3: start, end = end, start
        if rng.random() < 0.15:  # far start, end just past the plane: the hit rounds onto the end
            start = [f32(t + n * rng.uniform(300, 800)) for t, n in zip(target, normal)]
            for scale in (1e-6, 3e-6, 1e-5, 3e-5, 1e-4):
                end = [f32(t - n * scale) for t, n in zip(target, normal)]
                if dot(sub(end, verts[0]), normal) < 0: break
        put(memory, A, start); put(memory, B, end)
        mode = rng.choice((0, 0, 1, 1, 2))
        result = run('Ray', memory, [mode, rng.choice((0, 1)), A, B, struct.pack('<f', f32(rng.choice((0.0, 0.7, rng.uniform(0, 3))))),
                                     struct.pack('<f', f32(rng.choice((1.0, 0.0, rng.uniform(-2, 2)))))], precision, COLLIDER)
        note('ray-mode%d-%s' % (mode, 'hit' if result[0] else 'miss'))
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'cross, scale-add and set-length bodies shared, verified elsewhere',
                               'faces have 3 or 4 vertices (0x5d35b0 reads uninitialised locals otherwise)',
                               'a nearest hit exactly equal to the segment length (strict test in 0x5d39f0) is not constructed',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'add-else-chain': ('    if (z < minimum[2]) minimum[2] = z;\n    if (x > maximum[0]) maximum[0] = x;', '    if (z < minimum[2]) minimum[2] = z;\n    else if (x > maximum[0]) maximum[0] = x;'),
            'add-reread': ('    if (y < minimum[1]) minimum[1] = y;\n    if (z < minimum[2]) minimum[2] = z;\n    if (x > maximum[0]) maximum[0] = x;', '    if (y < minimum[1]) minimum[1] = y;\n    if (z < minimum[2]) minimum[2] = z;\n    if (point[0] > maximum[0]) maximum[0] = point[0];'),
            'set-reread': ('    minimum[1] = y;\n    minimum[2] = z;\n    maximum[0] = x;', '    minimum[1] = y;\n    minimum[2] = z;\n    maximum[0] = point[0];'),
            'sphere-swap': ('    float highX = radius + center[0];', '    float highX = radius - center[0];'),
            'overlap-touch': ('    if (!(minimumB[0] < maximumA[0])) return 0;', '    if (!(minimumB[0] <= maximumA[0])) return 0;'),
            'contains-nan-fails': ('normal70.y * edges[i].y < fengZero7D7D3C) return false;', 'normal70.y * edges[i].y >= fengZero7D7D3C) {} else return false;'),
            'contains-quad-as-tri': ('if ((signed char)vertexCount9 == 4) {', 'if ((signed char)vertexCount9 == 5) {'),
            'contains-distance-sign': ('        *distance = -(relative[0][1]', '        *distance = (relative[0][1]'),
            'segment-touch': ('if (!(toDistance * fromDistance < fengZero7D7D3C)) return false;', 'if (toDistance * fromDistance > fengZero7D7D3C) return false;'),
            'segment-rounded-from': ('    float ax = from[0] - vertex[0];', '    volatile float ax = from[0] - vertex[0];'),
            'segment-extended-to': ('    volatile float bx = to[0] - vertex[0];', '    float bx = to[0] - vertex[0];'),
            'segment-unrounded-fraction-to': ('/ (fabs(storedTo) + fromAbsolute)', '/ (fabs(toDistance) + fromAbsolute)'),
            'rayhit-one-sided-ignored': ('if (!twoSided && !(dy', 'if (0 && !(dy'),
            'rayhit-rounded-length': ('    double length = sqrt(dy * dy + dx * dx + dz * dz);', '    double length = (float)sqrt(dy * dy + dx * dx + dz * dz);'),
            'rayhit-le-best': ('if (!(length < *best)) return false;', 'if (!(length <= *best)) return false;'),
            'rayhit-no-bounds': ('    if (!Bounds3Overlap(faceMinimum, faceMaximum, rayMinimum, rayMaximum)) return false;\n', ''),
            'ray-first-hit': ('if (face->RayHit(&best, from, to, rayMinimum, rayMaximum, twoSided, 0)) hitFace = face;', 'if (!hitFace && face->RayHit(&best, from, to, rayMinimum, rayMaximum, twoSided, 0)) hitFace = face;'),
            'ray-mode1-no-margin': ('WheelSetLength(&direction, &direction, best - margin);', 'WheelSetLength(&direction, &direction, best);'),
            'ray-lift-sign': ('float lift = (fengOne784250 - travel / length) * scale + direction.z;', 'float lift = (travel / length - fengOne784250) * scale + direction.z;'),
            'ray-flag8-swap': ('if (hitFace->flagsA & 8) to[2] = height - margin;', 'if (hitFace->flagsA & 4) to[2] = height - margin;'),
            'ray-mode2-writes': ('            return true;\n        }\n    }\n    return true;\n}', '            return true;\n        }\n    }\n    to[2] = best;\n    return true;\n}'),
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
