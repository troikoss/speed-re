#!/usr/bin/env python3
"""Compare the camera's other-car push: nearby car search 0x44ac10, eye lift 0x44b040 and
the helpers 0x401250 (vector difference), 0x401340 (frame from rotation and position) and
0x441890 (car origin), under 53- and 64-bit x87 precision.

The normalize 0x43cf10, transform 0x441000, car velocity 0x4018a0, car body box 0x5ec250
and centre 0x5ec240 (rebuilt copies verified in camera-push), rigid inverse 0x5baf90 and
re-aim 0x444390 execute original code where the rebuilt object calls them externally.
Traced stubs: the race query 0x571250 and the D3DX multiply 0x63cedb. Vtable slot 0x44
(the mover's own car) is a fixture getter returning mover +0x2c in both runs."""

import argparse
import ctypes
import hashlib
import json
import math
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

BUILD = ROOT / 'build/analysis/camera-cars'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x3800, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
TARGETS = {'FindNearbyCar': ('?FindNearbyCar@CameraMoverBase@', 8, 0xffffffff), 'PushFromCars': ('?PushFromCars@CameraMoverBase@', 12, 0),
           'Difference': ('?Difference@WheelVector@', 8, 0xffffffff), 'WheelFrameAt': ('?WheelFrameAt@', 0, 0),
           'GetOrigin': ('?GetOrigin@MoverModeOwner@', 0, 0xffffffff)}
D3DX, RACE_QUERY = 0x63cedb, 0x571250
GLOBALS = ((0x82a580, 16), (0x82a590, 4), (0x7eb5c0, 4), (0x890080, 4))
SHARED = {'?Ray@WCollider@': 0x5d39f0, '?ContainsPoint@WCollisionFace@': 0x5d35b0, '?WheelScaleAdd@': 0x43d0c0,
          '?WheelTransform@': 0x441000, '?WheelNormalize@': 0x43cf10, '?bDistance@': 0x43ce10, '?CameraInvertRigid@': 0x5baf90,
          '?CameraReaim@': 0x444390, '?GetVelocity@MoverModeOwner@': 0x4018a0, '?Query571250@Object865480@': RACE_QUERY,
          '?object865480@': 0x865480, '?Query5EB520@CameraPlayer@': 0x5eb520, '?Construct@WCollider@': 0x5e0b40,
          '?QueryArea@WCollider@': 0x5e6fa0, '?Destruct@WCollider@': 0x5e0bf0, '?fengZero7D7D3C@': 0x7d7d3c,
          '?fengOne784250@': 0x784250, '?fengHalf784260@': 0x784260, '?cameraNearRadius787090@': 0x787090,
          '?cameraLookBackRadius787094@': 0x787094, '?cameraCarMargin78708C@': 0x78708c, '?cameraFaceDepth78709C@': 0x78709c,
          '?cameraRayMargin7870A0@': 0x7870a0, '?surfaceCameraSkip800EED@': 0x800eed, '?cameraCarHalf787088@': 0x787088,
          '?cameraCarSpeed7870C4@': 0x7870c4, '?cameraCarRange7870C8@': 0x7870c8, '?cameraCarHeading7870C0@': 0x7870c0,
          '?cameraLiftMargin784358@': 0x784358, '?carList890080@': 0x890080, '?cameraNearbyCarId7EB5C0@': 0x7eb5c0,
          '?cameraNearbyCar82A580@': 0x82a580, '?cameraNearbyGuard82A590@': 0x82a590, '__fltused': 0}
MOVER, VTABLE, GETTER, LIST, RACERS, VIEW, EYE, OUT, BODIES, CARS = (ARENA + x for x in (
    0x100, 0x180, 0x1f0, 0x200, 0x240, 0x300, 0x340, 0x360, 0x400, 0x1000))
CAR_COUNT, CAR_STRIDE, BODY_STRIDE = 6, 0x600, 0x180


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def sub(a, b): return [x - y for x, y in zip(a, b)]
def cross(a, b): return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]
def dot(a, b): return sum(x * y for x, y in zip(a, b))


def look_at(eye, target):
    f = sub(target, eye); n = math.sqrt(dot(f, f)) or 1.0; f = [x / n for x in f]
    r = cross([0, 0, -1], f); n = math.sqrt(dot(r, r)) or 1.0; r = [x / n for x in r]
    u = cross(f, r)
    m = [r[0], u[0], f[0], 0, r[1], u[1], f[1], 0, r[2], u[2], f[2], 0, 0, 0, 0, 1]
    m[12] = -dot(eye, r); m[13] = -dot(eye, u); m[14] = -dot(eye, f)
    return m


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Camera/CameraPush.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-cars.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Camera', '/Isrc/Speed/Indep/Src/Physics', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        match = [a for p, a in SHARED.items() if s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, traces, fixture = [], [[], []], {}
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
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
            if address == D3DX:
                out, left, right = struct.unpack('<3I', uc.mem_read(esp + 4, 12))
                l = struct.unpack('<16f', uc.mem_read(left, 64)); r = struct.unpack('<16f', uc.mem_read(right, 64))
                trace.append(('d3dx', bytes(uc.mem_read(left, 64)), bytes(uc.mem_read(right, 64))))
                uc.mem_write(out, struct.pack('<16f', *[f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4))) for i in range(4) for j in range(4)]))
                uc.reg_write(xc.UC_X86_REG_EAX, out); cleanup = 12
            else:
                trace.append(('race', this)); uc.reg_write(xc.UC_X86_REG_EAX, fixture['race']); cleanup = 0
            uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in (D3DX, RACE_QUERY): uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x44b040); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, statics, argv, precision, this):
        _, cleanup, mask = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for (address, size), data in zip(GLOBALS, statics): uc.mem_write(address, data)
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=4000000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not return')
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) & mask, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED),
                            uc.reg_read(xc.UC_X86_REG_FPCW), list(traces[rebuilt]), tuple(bytes(uc.mem_read(a, s)) for a, s in GLOBALS),
                            bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][6][i:i + 4].hex(), outputs[1][6][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][6][i:i + 4] != outputs[1][6][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), outputs[0][:4], outputs[1][:4],
                                  [t[0] for t in outputs[0][4]], [t[0] for t in outputs[1][4]],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][4], outputs[1][4])) if a != b), None),
                                  [(a.hex(), b.hex()) for a, b in zip(outputs[0][5], outputs[1][5]) if a != b], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def note(path): paths[path] = paths.get(path, 0) + 1

    def scene(memory, eye):
        """World list of up to six cars around `eye`, each with a body box, frame and velocity."""
        count = rng.choice((0, 1, 2, 3, 4, 5, 6, 6))
        struct.pack_into('<i', memory, LIST + 0x10 - ARENA, count)
        struct.pack_into('<2i', memory, RACERS - ARENA, 0, 3); struct.pack_into('<2i', memory, RACERS + 8 - ARENA, 0, rng.choice((1, 2, 4)))
        cars = []
        for i in range(CAR_COUNT):
            car, body = CARS + i * CAR_STRIDE, BODIES + i * BODY_STRIDE; cars.append(car)
            struct.pack_into('<I', memory, LIST + 0x1c + i * 4 - ARENA, car)
            struct.pack_into('<I', memory, car + 0x14 - ARENA, rng.choice((RACERS, RACERS, RACERS + 8)))
            memory[car + 0x550 - ARENA] = rng.choice((1, 1, 1, 0, 0x80))
            struct.pack_into('<I', memory, car + 0x5b4 - ARENA, rng.choice((0xffffffff, 0xffffffff, rng.getrandbits(32), 0)))
            struct.pack_into('<h', memory, car + 0xa - ARENA, rng.randrange(-3, 40))
            struct.pack_into('<I', memory, car + 0x3c - ARENA, body)
            reach = rng.choice((rng.uniform(0, 3), rng.uniform(2, 6), rng.uniform(6, 14), 0.0))
            angle = rng.uniform(-math.pi, math.pi)
            position = [eye[0] + reach * math.cos(angle), eye[1] + reach * math.sin(angle), eye[2] + rng.choice((rng.uniform(-2.5, 1.5), 0.0, rng.uniform(-12, 12)))]
            put(memory, car + 0x50, position)
            put(memory, car + 0x70, rng.choice(([rng.uniform(-40, 40) for _ in range(3)], [rng.uniform(-3, 3) for _ in range(3)], [0, 0, 0])))
            yaw, pitch = rng.uniform(-math.pi, math.pi), rng.choice((0.0, rng.uniform(-0.4, 0.4)))
            c, s, cp, sp = math.cos(yaw), math.sin(yaw), math.cos(pitch), math.sin(pitch)
            put(memory, car + 0x90, [c * cp, s * cp, sp, 0, -s, c, 0, 0, -c * sp, -s * sp, cp, 0, position[0], position[1], position[2], 1])
            centre = [rng.uniform(-.5, .5), rng.uniform(-.3, .3), rng.uniform(0, .8)]
            half = [rng.uniform(.8, 2.5), rng.uniform(.6, 1.2), rng.uniform(.3, .9)]
            put(memory, body + 0x110, centre); put(memory, body + 0xe0, [x - h for x, h in zip(centre, half)])
            put(memory, body + 0xf0, [x + h for x, h in zip(centre, half)])
        return cars

    def statics():
        return (bytes(rng.randbytes(16)), struct.pack('<I', rng.choice((0, 1, 0xfffffffe, rng.getrandbits(32)))),
                struct.pack('<i', rng.choice((-1, 0, 3, rng.randrange(-3, 40)))), struct.pack('<I', LIST))
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # Mover whose vtable slot 0x44 returns its own car at +0x2c.
        memory[GETTER - ARENA:GETTER + 4 - ARENA] = bytes.fromhex('8b412cc3')
        vtable = [0] * 0x14; vtable[0x44 // 4] = GETTER
        struct.pack_into('<20I', memory, VTABLE - ARENA, *vtable); struct.pack_into('<I', memory, MOVER - ARENA, VTABLE)
        struct.pack_into('<I', memory, MOVER + 0x10 - ARENA, rng.choice((0, 1, 2, 3, 31)))
        eye = [rng.uniform(-50, 50), rng.uniform(-50, 50), rng.uniform(-5, 5)]
        cars = scene(memory, eye)
        own = rng.choice(cars + [0, 0])
        struct.pack_into('<I', memory, MOVER + 0x2c - ARENA, own)
        fixture['race'] = rng.choice((0, 0, 1, 0x100))
        racers = rng.choice((0, 1, 0x100))
        if rng.random() < 0.15:
            # Eye exactly at a car's lowered box top: equality decides the height test.
            car = rng.choice(cars); low = struct.unpack_from('<3f', memory, car + 0x50 - ARENA); eye = [low[0] + 0.3, low[1] - 0.2, low[2]]
        put(memory, EYE, eye)
        result = run('FindNearbyCar', memory, statics(), [EYE, racers], precision, MOVER)
        note('nearby-found' if result[0] else 'nearby-none')
        # Lift a camera looking at the player's car over the nearby cars.
        target = rng.choice(cars)
        put(memory, VIEW, look_at(eye, [c + rng.uniform(-1, 1) for c in struct.unpack_from('<3f', memory, target + 0x50 - ARENA)]))
        before = bytes(memory[VIEW - ARENA:VIEW + 64 - ARENA])
        result = run('PushFromCars', memory, statics(), [VIEW, target, racers], precision, MOVER)
        note('lift-moved' if result[6][VIEW - ARENA:VIEW + 64 - ARENA] != before else 'lift-unchanged')
        note('lift-reset' if result[5][2] == struct.pack('<i', -1) else 'lift-kept')
        # Helpers.
        put(memory, OUT, [rng.uniform(-100, 100) for _ in range(3)])
        run('Difference', memory, statics(), [OUT + 0x10, OUT], precision, rng.choice(cars))
        run('Difference', memory, statics(), [OUT, OUT], precision, OUT)  # aliasing output and other
        run('WheelFrameAt', memory, statics(), [VIEW, rng.choice(cars) + 0x90, rng.choice((EYE, VIEW + 0x30))], precision, 0)
        run('GetOrigin', memory, statics(), [], precision, rng.choice(cars))
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  traced_stubs=['race query 0x571250', 'D3DX multiply 0x63cedb', 'vtable slot 0x44 fixture getter (+0x2c)'],
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'car positions, frames and boxes are finite; signalling-NaN copies through 0x401340 are not exercised',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'cars-own-kept': ('        if (car == own) continue;\n', ''),
            'cars-racers-ignored': ('if (racersOnly && *reinterpret_cast', 'if (0 && *reinterpret_cast'),
            'cars-inactive-kept': ('        if (!bytes[0x550]) continue;\n', ''),
            'cars-view-mask': ('& (1 << view10)', '& (2 << view10)'),
            'cars-range-unsquared': ('cameraCarRange7870C8 * cameraCarRange7870C8', 'cameraCarRange7870C8'),
            'cars-no-race-query': ('if (own && !object865480.Query571250())', 'if (own)'),
            'cars-speed-unsquared': ('cameraCarSpeed7870C4 * cameraCarSpeed7870C4 <', 'cameraCarSpeed7870C4 <'),
            'cars-no-separation-test': ('                        < cameraCarHeading7870C0) continue;', '                        < -2.0f) continue;'),
            'cars-separation-reversed': ('reinterpret_cast<WheelVector *>(position)->Difference(&separation,\n                                                                          reinterpret_cast<WheelVector *>(own->GetOrigin()));',
                                         'reinterpret_cast<WheelVector *>(own->GetOrigin())->Difference(&separation,\n                                                                          reinterpret_cast<WheelVector *>(position));'),
            'cars-sphere-no-margin': ('sqrt(ez * ez + ey * ey + ex * ex) + cameraCarMargin78708C;', 'sqrt(ez * ez + ey * ey + ex * ex);'),
            'cars-single-corner': ('point.z = (corner & 1) ? first[2] : second[2];', 'point.z = first[2];'),
            'cars-bottom-no-margin': ('float bottom = low - cameraCarHalf787088;', 'float bottom = low;'),
            'cars-no-id': ('        if (cameraNearbyCarId7EB5C0 != id) cameraNearbyCarId7EB5C0 = id;\n', ''),
            'cars-result-radius': ('        result[3] = radius2;\n', '        result[3] = (float)reach;\n'),
            'cars-guard-bit': ('cameraNearbyGuard82A590 |= 1;', 'cameraNearbyGuard82A590 |= 2;'),
            'lift-no-margin': ('sqrt(sphere[3] + cameraLiftMargin784358 -', 'sqrt(sphere[3] -'),
            'lift-once': ('            sphere = FindNearbyCar(eye, racersOnly);\n        } while', '            sphere = 0;\n        } while'),
            'lift-no-projection': ('    projected[0] = (float)(storedX * inverse);\n', '    projected[0] = 0.0f;\n'),
            'lift-keeps-id': ('    if (!found) cameraNearbyCarId7EB5C0 = -1;\n', ''),
            'lift-rounded-y': ('    float y = m[9] * origin[2]', '    volatile float y = m[9] * origin[2]'),
            'difference-order': ('    float dy = y - other->y;', '    float dy = other->y - y;'),
            'frame-w-zero': ('    o[15] = 1.0f;', '    o[15] = 0.0f;'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '1024'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
