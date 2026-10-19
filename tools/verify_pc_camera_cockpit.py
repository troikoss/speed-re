#!/usr/bin/env python3
"""Compare the drive camera's cockpit blocked tests under 53- and 64-bit x87 precision: the
cockpit car test 0x445fc0, the world car box test 0x5f14f0, the bounds containment 0x43d7a0,
the wheel input/target queries 0x5ed290/0x5ed300, the player car getter 0x5eac70, drive camera
vtable slot 0x24 0x445a90 and the start of Update 0x453570..0x4536f4 (original run from the entry
to 0x4536f4, rebuilt as DriveCameraMover::SelectUpdateType; the compared value is EBX against EAX).

Five rebuilt objects are linked: CameraCockpit.cpp, CameraChase.cpp (HumanBlocked 0x445ac0 only), CameraPush.cpp (queries 0x5ed290/0x5ed300 and
the frame 0x401340), WCollisionRay.cpp (0x43d7a0 and the collider ray 0x5d39f0 with its face test)
and nothing else. Original code runs for the rigid inverse 0x5baf90, transform 0x441000, scale-add
0x43d0c0, cross, set-length, ReadSetting 0x445eb0 (a reduced rebuild lives in DriveCamera.cpp),
the human input slot 0x54 (0x410470) and the car getter slot 0x44 (0x44ba70). Traced stub: the
D3DX multiply 0x63cedb. The rebuilt call to CameraSettingsTable::FindOrDefault (the platform
`settings` pointer and `viewState` instead of the original's 0x5ecca0 on the car and the global
0x8707dc) goes to a 38-byte shim that stores its viewState argument to 0x8707dc and runs original
0x441140 twice as 0x5ecca0 does; the global is compared after every run."""

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

BUILD = ROOT / 'build/analysis/camera-cockpit'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x6800, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
SOURCES = {'cockpit': 'src/Speed/Indep/Src/Camera/CameraCockpit.cpp', 'push': 'src/Speed/Indep/Src/Camera/CameraPush.cpp',
           'ray': 'src/Speed/Indep/Src/World/WCollisionRay.cpp', 'chase': 'src/Speed/Indep/Src/Camera/CameraChase.cpp'}
# name: (rebuilt symbol prefix, callee-popped bytes, return mask)
TARGETS = {'Bounds3Contains': ('?Bounds3Contains@', 0, 0xffffffff), 'AnyCarContains': ('?AnyCarContains@CameraCarList@', 12, 0xff),
           'CarsNearCockpit': ('?CarsNearCockpit@DriveCameraMover@', 0, 0xff), 'UsesChaseSetting': ('?UsesChaseSetting@DriveCameraMover@', 0, 0xff),
           'GetPlayerCar': ('?GetCar@CameraPlayer@', 0, 0xffffffff), 'HasWheelInputs': ('?HasWheelInputs@MoverModeOwner@', 0, 0xff),
           'HasWheelTargets': ('?HasWheelTargets@MoverModeOwner@', 0, 0xff), 'SelectUpdateType': ('?SelectUpdateType@DriveCameraMover@', 0, 0xffffffff)}
PREFIX_END = 0x4536f4
# Entry events recorded in both runs (original address, rebuilt symbol prefix): the call order is compared.
EVENTS = {'chase': (0x445a90, '?UsesChaseSetting@DriveCameraMover@'), 'cars_near': (0x445fc0, '?CarsNearCockpit@DriveCameraMover@'),
          'any_car': (0x5f14f0, '?AnyCarContains@CameraCarList@'), 'has_inputs': (0x5ed290, '?HasWheelInputs@MoverModeOwner@'),
          'has_targets': (0x5ed300, '?HasWheelTargets@MoverModeOwner@'), 'ray': (0x5d39f0, '?Ray@WCollider@'),
          'player_car': (0x5eac70, '?GetCar@CameraPlayer@'), 'contains': (0x43d7a0, '?Bounds3Contains@'),
          'slot54': (0x410470, None), 'frame': (0x401340, '?WheelFrameAt@')}
D3DX, FIND, READ = 0x63cedb, 0x441140, 0x445eb0
GLOBALS = ((0x890080, 4), (0x890118, 4), (0x8707dc, 4), (0x89e7e2, 1), (0x890100, 4))
SHARED = {'?ReadSetting@DriveCameraMover@': READ, '?WheelScaleAdd@': 0x43d0c0, '?WheelTransform@': 0x441000, '?CameraInvertRigid@': 0x5baf90,
          '?WheelCross@': 0x43d1e0, '?WheelSetLength@': 0x43cf90, '?WheelNormalize@': 0x43cf10, '?bDistance@': 0x43ce10,
          '?CameraReaim@': 0x444390, '?GetVelocity@MoverModeOwner@': 0x4018a0, '?Query571250@Object865480@': 0x571250,
          '?object865480@': 0x865480, '?Construct@WCollider@': 0x5e0b40, '?QueryArea@WCollider@': 0x5e6fa0,
          '?Destruct@WCollider@': 0x5e0bf0, '?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250, '?fengHalf784260@': 0x784260,
          '?rayMinusOne7842A0@': 0x7842a0, '?cameraNearRadius787090@': 0x787090, '?cameraLookBackRadius787094@': 0x787094,
          '?cameraCarMargin78708C@': 0x78708c, '?cameraFaceDepth78709C@': 0x78709c, '?cameraRayMargin7870A0@': 0x7870a0,
          '?surfaceCameraSkip800EED@': 0x800eed, '?cameraCarHalf787088@': 0x787088, '?cameraCarSpeed7870C4@': 0x7870c4,
          '?cameraCarRange7870C8@': 0x7870c8, '?cameraCarHeading7870C0@': 0x7870c0, '?cameraLiftMargin784358@': 0x784358,
          '?carList890080@': 0x890080, '?cameraNearbyCarId7EB5C0@': 0x7eb5c0, '?cameraNearbyCar82A580@': 0x82a580,
          '?cameraNearbyGuard82A590@': 0x82a590, '?cameraCockpitReach7843E8@': 0x7843e8, '?cameraBlockedRayLength78715C@': 0x78715c,
          '?cameraPlayerWorld890118@': 0x890118, '__fltused': 0,
          # CameraChase.cpp (HumanBlocked 0x445ac0 lives there); the other routines in that object are not run here.
          '?SetTarget@CameraCubicVector@': 0x570ba0, '?Lookup@CameraPairCurve@': 0x576790, '?WheelIdentity@': 0x43c780,
          '?cameraHeadingCurve82A4E0@': 0x82a4e0, '?cameraReflectScale7855D0@': 0x7855d0, '?cameraHeadingDecay787158@': 0x787158,
          '?cameraClock86518C@': 0x86518c, '?cameraClockScale784268@': 0x784268}
(MOVER, PLAYER, WORLD, HOLDER, LIST, SCRATCH, RECORD, FACES, BODIES, CARS, EXTRA) = (ARENA + x for x in (
    0x100, 0xb00, 0xe00, 0x1a00, 0x1a40, 0x1b00, 0x1c00, 0x2400, 0x2800, 0x3100, 0xa000))
INPUTS, INPUT_STRIDE, INPUT_COUNT = ARENA + 0x200, 0x300, 3
TABLE = RECORD + 0x720
VTABLE_MOVER, VTABLE_INPUT, SHIM = EXTRA, EXTRA + 0x100, EXTRA + 0x200
CAR_COUNT, CAR_STRIDE, BODY_STRIDE, FACE_COUNT = 6, 0x900, 0x180, 8
POINT, BMIN, BMAX = SCRATCH, SCRATCH + 0x10, SCRATCH + 0x20


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity
def sub(a, b): return [x - y for x, y in zip(a, b)]
def cross(a, b): return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]
def dot(a, b): return sum(x * y for x, y in zip(a, b))


def special(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-20, -1e-20, 1e30, float('inf'), -float('inf'), float('nan'), rng.uniform(-2, 2), rng.uniform(-500, 500)))


def face(rng, near, jitter=2):
    centre = [c + rng.uniform(-jitter, jitter) for c in near]
    u = rng.choice(([rng.gauss(0, 1) for _ in range(3)], [rng.uniform(-.3, .3), rng.uniform(-.3, .3), 1.0], [rng.gauss(0, 1), rng.gauss(0, 1), rng.uniform(-.2, .2)]))
    n = math.sqrt(dot(u, u)) or 1.0; u = [x / n for x in u]
    t = cross(u, [1, 0, 0] if abs(u[0]) < 0.9 else [0, 1, 0]); n = math.sqrt(dot(t, t)); t = [x / n for x in t]
    w = cross(u, t); count = rng.choice((3, 4)); size = rng.uniform(0.5, 6)
    angles = [0.3, 1.9, 3.4, 4.9] if count == 4 else sorted(rng.uniform(0, 2 * math.pi) for _ in range(3))
    verts = [[f32(centre[k] + size * (math.cos(a) * t[k] + math.sin(a) * w[k])) for k in range(3)] for a in angles]
    if rng.random() < 0.5: verts.reverse()
    normal = cross(sub(verts[1], verts[0]), sub(verts[2], verts[0])); n = math.sqrt(dot(normal, normal)) or 1.0
    normal = [f32(x / n) for x in normal]
    if rng.random() < 0.05: normal[rng.randrange(3)] = rng.choice((float('nan'), float('inf')))
    return count, verts, normal, f32(dot(normal, verts[0]))


def transform(m, v):
    """Row-vector transform of 0x441000: v * rotation rows + translation row."""
    return [f32(sum(v[k] * m[k * 4 + j] for k in range(3)) + m[12 + j]) for j in range(3)]


def shim_bytes():
    code = bytearray([0x56, 0x89, 0xce, 0x8b, 0x44, 0x24, 0x0c, 0xa3]) + struct.pack('<I', 0x8707dc)
    code += bytes([0xff, 0x74, 0x24, 0x08, 0xe8]); code += struct.pack('<i', FIND - (SHIM + len(code) + 4))
    code += bytes([0x85, 0xc0, 0x75, 0x09, 0x89, 0xf1, 0x6a, 0x00, 0xe8]); code += struct.pack('<i', FIND - (SHIM + len(code) + 4))
    code += bytes([0x5e, 0xc2, 0x08, 0x00])
    return bytes(code)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: cockpit, push, ray)')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-cockpit.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno'])
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Camera', '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    parsed = [read_object(o) for o in objects]
    defined = {s['name'] for p in parsed for s in p['symbols'].values() if s['section'] > 0}
    externals = {}
    for p in parsed:
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            if s['name'].startswith('?FindOrDefault@CameraSettingsTable@'): externals[s['name']] = SHIM; continue
            match = [a for pre, a in SHARED.items() if s['name'].startswith(pre)]
            assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    rebuilt_events = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (_, p) in EVENTS.items() if p}
    original_events = {n: a for n, (a, _) in EVENTS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, traces, events = [], [[], []], [[], []]
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        # Mover vtable: slot 0x24 is the chase query, 0x44 the car getter 0x44ba70. Human input vtable: slot 0x54 is 0x410470.
        vt = [0] * 0x14; vt[0x24 // 4] = rebuilt and entries['UsesChaseSetting'] or 0x445a90; vt[0x44 // 4] = 0x44ba70
        uc.mem_write(VTABLE_MOVER, struct.pack('<20I', *vt))
        iv = [0] * 0x24; iv[0x54 // 4] = 0x410470
        uc.mem_write(VTABLE_INPUT, struct.pack('<36I', *iv))
        uc.mem_write(SHIM, shim_bytes())
        def block(uc, address, size, user, rebuilt=rebuilt):
            # Rebuilt code must not reach original targets; the original must not reach rebuilt code.
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
            out, left, right = struct.unpack('<3I', uc.mem_read(esp + 4, 12))
            l = struct.unpack('<16f', uc.mem_read(left, 64)); r = struct.unpack('<16f', uc.mem_read(right, 64))
            trace.append(('d3dx', bytes(uc.mem_read(left, 64)), bytes(uc.mem_read(right, 64))))
            uc.mem_write(out, struct.pack('<16f', *[f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4))) for i in range(4) for j in range(4)]))
            uc.reg_write(xc.UC_X86_REG_EAX, out); uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + 12); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        def entry(uc, address, size, user, log=events[rebuilt]):
            # Entry events with the arguments the callee sees, so call boundaries are compared.
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            words = struct.unpack('<7I', uc.mem_read(esp + 4, 28)); mem = lambda a, n: bytes(uc.mem_read(a, n))
            if user == 'ray': log.append((user, this, words[0], words[1] & 0xff, mem(words[2], 12), mem(words[3], 12), words[4], words[5]))
            elif user == 'any_car': log.append((user, mem(words[0], 12), words[1], words[2]))
            elif user == 'contains': log.append((user, mem(words[0], 12), mem(words[1], 12), mem(words[2], 12), words[3]))
            elif user in ('has_inputs', 'has_targets', 'cars_near', 'chase', 'player_car'): log.append((user, this))
            else: log.append((user,))
        uc.hook_add(UC_HOOK_BLOCK, block)
        uc.hook_add(UC_HOOK_CODE, stub, begin=D3DX, end=D3DX)
        for name, address in (rebuilt_events if rebuilt else original_events).items():
            uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        if rebuilt: uc.hook_add(UC_HOOK_CODE, entry, user_data='slot54', begin=0x410470, end=0x410470)  # the same original code in both runs
        machines.append(uc)
    rng = random.Random(0x445fc0); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, statics, argv, precision, this):
        _, cleanup, mask = TARGETS[name]
        prefix = name == 'SelectUpdateType'
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear(); events[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for (address, size), data in zip(GLOBALS, statics): uc.mem_write(address, data)
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            until = PREFIX_END if prefix and not rebuilt else STOP
            try: uc.emu_start(address, until, count=4000000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == until, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not finish')
            value = (uc.reg_read(xc.UC_X86_REG_EBX) if prefix and not rebuilt else uc.reg_read(xc.UC_X86_REG_EAX)) & mask
            if prefix: contract = (value, uc.reg_read(xc.UC_X86_REG_FPCW))
            else: contract = (value, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW))
            outputs.append((contract, uc.reg_read(xc.UC_X86_REG_FPTAG), list(traces[rebuilt]), list(events[rebuilt]),
                            tuple(bytes(uc.mem_read(a, s)) for a, s in GLOBALS), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        if not prefix: assert outputs[0][0][1] == STACK + 4 + cleanup and outputs[0][0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            a, b = outputs
            diff = [(hex(ARENA + i), a[5][i:i + 4].hex(), b[5][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if a[5][i:i + 4] != b[5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), a[0], b[0], a[1], b[1], [e[0] for e in a[3]], [e[0] for e in b[3]], next(((x, y) for x, y in zip(a[3], b[3]) if x != y), None),
                                  [(x.hex(), y.hex()) for x, y in zip(a[4], b[4]) if x != y], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def get(memory, address, count): return list(struct.unpack_from('<%df' % count, memory, address - ARENA))
    def note(path): paths[path] = paths.get(path, 0) + 1
    def put_face(memory, address, spec, flags):
        count, verts, normal, distance = spec
        memory[address + 8 - ARENA:address + 12 - ARENA] = bytes((rng.randrange(256), count, flags, rng.randrange(0, 24)))
        for i, v in enumerate(verts): put(memory, address + 0x30 + i * 0x10, v)
        put(memory, address + 0x70, normal); put(memory, address + 0x7c, [distance])

    def make_table(memory):
        """Both settings tables always hold a type 0 entry, so FindOrDefault never fails."""
        tables = []
        for count_offset, entries_offset in ((0, 0x10), (4, 0x70)):
            types = [0] + [t for t in (2, 4) if rng.random() < 0.8] + [rng.choice((1, 3, 5, 6)) for _ in range(rng.randrange(0, 3))]
            rng.shuffle(types); tables.append(types)
            struct.pack_into('<i', memory, TABLE + count_offset - ARENA, len(types))
            for i, t in enumerate(types):
                struct.pack_into('<hH', memory, TABLE + entries_offset + i * 16 - ARENA, t, rng.randrange(0, 65536))
                put(memory, TABLE + entries_offset + i * 16 + 4, [rng.choice((rng.uniform(-3, 3), rng.uniform(-0.5, 2), special(rng))) if rng.random() < 0.9 else special(rng) for _ in range(3)])
        return tables

    def chase_type(tables, view_state, key):
        types = tables[1] if view_state == 3 else tables[0]
        return 0 if key not in types else key  # FindOrDefault falls back to type 0 whose entry type is 0

    def scene(memory):
        """Cars with frames, boxes, inputs and colliders; the mover's tables and statics."""
        view_state = rng.choice((0, 1, 2, 3, 3))
        tables = make_table(memory)
        struct.pack_into('<I', memory, WORLD + 0xbd0 - ARENA, HOLDER)
        cars = [CARS + i * CAR_STRIDE for i in range(CAR_COUNT)]
        for i in range(INPUT_COUNT):
            inp = INPUTS + i * INPUT_STRIDE
            struct.pack_into('<I', memory, inp - ARENA, VTABLE_INPUT); memory[inp + 0x2fb - ARENA] = rng.choice((0, 0, 0, 1, 0x80))
            def wheel():
                return rng.choice((0.0, -0.0, -1.0, -1e-20, float('nan'), -float('inf'), special(rng), rng.uniform(0, 1), 1e-30, float('inf')))
            put(memory, inp + 0x1ec, [wheel() for _ in range(8)])
            if rng.random() < 0.5: put(memory, inp + 0x1ec, [min(0.0, v) if v == v else v for v in get(memory, inp + 0x1ec, 8)])
        for i, car in enumerate(cars):
            body = BODIES + i * BODY_STRIDE
            struct.pack_into('<I', memory, car + 0xc - ARENA, RECORD)
            struct.pack_into('<I', memory, car + 0x30 - ARENA, INPUTS + rng.randrange(INPUT_COUNT) * INPUT_STRIDE)
            struct.pack_into('<I', memory, car + 0x38 - ARENA, rng.choice((0, 1, 0x1234)))
            struct.pack_into('<I', memory, car + 0x3c - ARENA, body)
            struct.pack_into('<i', memory, car + 0x424 - ARENA, rng.choice((-1, 0, 1, 2, 3, 5)))
            memory[car + 0x550 - ARENA] = rng.choice((1, 1, 1, 0, 0x80))
            put(memory, car + 0x3f8, [rng.choice((0.0, -0.0, -1.0, 0.25, special(rng), float('nan'))) for _ in range(4)])
            yaw, pitch = rng.uniform(-math.pi, math.pi), rng.choice((0.0, rng.uniform(-0.4, 0.4)))
            c, s, cp, sp = math.cos(yaw), math.sin(yaw), math.cos(pitch), math.sin(pitch)
            position = [rng.uniform(-6, 6), rng.uniform(-6, 6), rng.uniform(-1, 1)]
            put(memory, car + 0x50, position)
            put(memory, car + 0x60, [rng.choice((rng.uniform(-1, 1), special(rng))) if rng.random() < 0.9 else special(rng) for _ in range(3)])
            put(memory, car + 0x90, [c * cp, s * cp, sp, 0, -s, c, 0, 0, -c * sp, -s * sp, cp, 0, position[0], position[1], position[2], 1])
            centre = [rng.uniform(-.5, .5), rng.uniform(-.3, .3), rng.uniform(0, .8)]
            half = [rng.uniform(.8, 2.5), rng.uniform(.6, 1.2), rng.uniform(.3, .9)]
            put(memory, body + 0x110, centre); put(memory, body + 0xe0, [x - h for x, h in zip(centre, half)])
            put(memory, body + 0xf0, [x + h for x, h in zip(centre, half)])
            # Collider area: a few faces around the car's camera point.
            struct.pack_into('<I', memory, car + 0x5c4 + 8 - ARENA, rng.choice((1, 1, 0)))
            struct.pack_into('<i', memory, car + 0x5c4 + 0x2e0 - ARENA, rng.randrange(0, FACE_COUNT + 1))
            for k in range(FACE_COUNT): struct.pack_into('<I', memory, car + 0x5c4 + 0x3c + k * 4 - ARENA, FACES + k * 0x80)
        for k in range(FACE_COUNT): put_face(memory, FACES + k * 0x80, face(rng, [rng.uniform(-6, 6), rng.uniform(-6, 6), rng.uniform(-1, 3)]), rng.choice((0, 2, 8, 10)))
        count = rng.choice((0, 1, 2, 3, 4, 5, 6, 6))
        struct.pack_into('<i', memory, LIST + 0x10 - ARENA, count)
        for i, car in enumerate(cars): struct.pack_into('<I', memory, LIST + 0x1c + i * 4 - ARENA, car)
        # Player: car +4, optional override through the holder at *0x890118 +0xbd0.
        struct.pack_into('<I', memory, PLAYER + 4 - ARENA, rng.choice(cars)); memory[PLAYER + 0x24a - ARENA] = rng.choice((0, 0, 1, 0x80))
        memory[PLAYER + 0x2b - ARENA] = rng.choice((0, 1, 0x80))
        struct.pack_into('<I', memory, HOLDER - ARENA, rng.choice(cars))
        statics = [struct.pack('<I', LIST), struct.pack('<I', WORLD), struct.pack('<i', view_state), bytes((rng.choice((0, 0, 0, 1)),)),
                   struct.pack('<I', rng.choice((0, 0, 0, 1)))]
        return cars, tables, view_state, statics

    def mover(memory, cars, tables, view_state, own, kind):
        struct.pack_into('<I', memory, MOVER - ARENA, VTABLE_MOVER)
        struct.pack_into('<I', memory, MOVER + 0x20 - ARENA, rng.choice((PLAYER, 0)))
        struct.pack_into('<I', memory, MOVER + 0x2c - ARENA, own)
        struct.pack_into('<i', memory, MOVER + 0x48 - ARENA, kind)
        struct.pack_into('<i', memory, MOVER + 0x4c - ARENA, rng.choice((2, 4, 2, 4, 3, 0, 1, 5, 9)))
        memory[MOVER + 0x51 - ARENA] = rng.choice((0, 1, 2)); memory[MOVER + 0x52 - ARENA] = rng.choice((0, 1, 2, 0x80))
        memory[MOVER + 0x53 - ARENA] = rng.randrange(256); memory[MOVER + 0x54 - ARENA] = rng.choice((0, 1, 0x55))
        struct.pack_into('<II', memory, MOVER + 0x60 - ARENA, TABLE, view_state)  # platform tail read by the rebuilt code

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        cars, tables, view_state, statics = scene(memory)
        # --- bounds containment 0x43d7a0 ---
        lo = [rng.choice((-1.0, 0.0, -0.0, 1.0, rng.uniform(-5, 5), special(rng))) for _ in range(3)]
        hi = [rng.choice((1.0, 2.0, lo[i], rng.uniform(-5, 5), special(rng))) if rng.random() < 0.8 else lo[i] + rng.uniform(0, 3) for i in range(3)]
        margin = rng.choice((0.5, 0.0, -0.5, 1.0, rng.uniform(0, 2), special(rng)))
        pt = []
        for i in range(3):
            pt.append(rng.choice((lo[i] - margin, f32(hi[i] + margin), lo[i], hi[i], (lo[i] + hi[i]) / 2, lo[i] - margin - 1e-6, f32(hi[i] + margin) + 1e-6, special(rng),
                                  rng.uniform(-6, 6))))
        put(memory, BMIN, lo); put(memory, BMAX, hi); put(memory, POINT, pt)
        result = run('Bounds3Contains', memory, statics, [BMIN, BMAX, POINT, struct.unpack('<I', struct.pack('<f', f32(margin)))[0]], precision, 0)
        note('contains-yes' if result[0][0] else 'contains-no')
        # --- wheel queries ---
        for car in rng.sample(cars, 3):
            result = run('HasWheelInputs', memory, statics, [], precision, car); note('inputs-yes' if result[0][0] else 'inputs-no')
            result = run('HasWheelTargets', memory, statics, [], precision, car); note('targets-yes' if result[0][0] else 'targets-no')
        # --- player car 0x5eac70, including a null holder ---
        pmem = bytearray(memory)
        if rng.random() < 0.3: struct.pack_into('<I', pmem, WORLD + 0xbd0 - ARENA, 0)
        if rng.random() < 0.2: struct.pack_into('<I', pmem, PLAYER + 4 - ARENA, 0)
        result = run('GetPlayerCar', pmem, statics, [], precision, PLAYER)
        note('player-override' if pmem[PLAYER + 0x24a - ARENA] and struct.unpack_from('<I', pmem, WORLD + 0xbd0 - ARENA)[0] else 'player-own')
        # --- world car box test 0x5f14f0 ---
        count = struct.unpack_from('<i', memory, LIST + 0x10 - ARENA)[0]
        target = rng.choice(cars)
        mat = get(memory, target + 0x90, 16); body = BODIES + cars.index(target) * BODY_STRIDE
        centre = get(memory, body + 0x110, 3); box_lo = get(memory, body + 0xe0, 3); box_hi = get(memory, body + 0xf0, 3)
        margin = rng.choice((0.5, 0.5, 0.0, rng.uniform(0, 2), rng.uniform(-1, 1), float('nan'), float('inf'), special(rng)))
        local = [centre[i] + rng.uniform(-1, 1) * ((box_hi[i] - box_lo[i]) / 2 + (abs(margin) if margin == margin and margin != float('inf') else 0.5) * rng.choice((0, 1, 1.4, 3))) for i in range(3)]
        point = transform(mat, local)
        if rng.random() < 0.1: point[rng.randrange(3)] = special(rng)
        put(memory, POINT, point)
        skip = rng.choice(cars + [0, ARENA, target])
        result = run('AnyCarContains', memory, statics, [POINT, struct.unpack('<I', struct.pack('<f', f32(margin)))[0], skip], precision, rng.choice((LIST, 0, 0x7654321)))
        note('anycar-yes' if result[0][0] else 'anycar-no')
        # --- slot 0x24 and the cockpit car test ---
        kind = rng.choice((2, 4, 0, 1, 3, 5))
        own = rng.choice(cars)
        mover(memory, cars, tables, view_state, own, kind)
        result = run('UsesChaseSetting', memory, statics, [], precision, MOVER); note('chase-yes' if result[0][0] else 'chase-no')
        if rng.random() < 0.5:
            # Put another car on the cockpit point so the test can hold.
            others = [c for c in cars if c != own]
            if others:
                mat = get(memory, own + 0x90, 16)
                table_entries = TABLE + (0x70 if view_state == 3 else 0x10)
                n = struct.unpack_from('<i', memory, TABLE + (4 if view_state == 3 else 0) - ARENA)[0]
                typed = [struct.unpack_from('<hHfff', memory, table_entries + i * 16 - ARENA) for i in range(n)]
                found = [t for t in typed if t[0] == kind] or [t for t in typed if t[0] == 0]
                offset = [found[0][2], found[0][4], found[0][3]]  # forward, lateral, height
                cockpit = transform(mat, offset)
                other = rng.choice(others)
                if all(v == v and abs(v) < 1e6 for v in cockpit):
                    omat = get(memory, other + 0x90, 16)
                    put(memory, other + 0x50, [cockpit[i] + rng.uniform(-1.2, 1.2) for i in range(3)])
                    for j in range(3): omat[12 + j] = get(memory, other + 0x50, 3)[j]
                    put(memory, other + 0x90, omat)
        result = run('CarsNearCockpit', memory, statics, [], precision, MOVER); note('cockpit-yes' if result[0][0] else 'cockpit-no')
        # --- start of Update 0x453570..0x4536f4 ---
        pm = bytearray(memory)
        previous = struct.unpack_from('<i', pm, MOVER + 0x4c - ARENA)[0]
        player = struct.unpack_from('<I', pm, MOVER + 0x20 - ARENA)[0]
        effective_own = own
        if player:
            holder_used = pm[PLAYER + 0x24a - ARENA]
            effective_own = struct.unpack_from('<I', pm, HOLDER - ARENA)[0] if holder_used else struct.unpack_from('<I', pm, PLAYER + 4 - ARENA)[0]
        ci = cars.index(effective_own)
        if rng.random() < 0.15: struct.pack_into('<I', pm, effective_own + 0x3c - ARENA, 0)  # no body: the point stays car +0x60..
        if effective_own in cars[:count] and struct.unpack_from('<I', pm, effective_own + 0x3c - ARENA)[0] == 0:
            pass  # the own car is skipped by the list walk before its body is read
        # Faces across the ray from the camera point along the car's +0xb0 row.
        if rng.random() < 0.6:
            mat = get(pm, effective_own + 0x90, 16); body_ptr = struct.unpack_from('<I', pm, effective_own + 0x3c - ARENA)[0]
            if body_ptr:
                origin = get(pm, effective_own + 0x50, 3); world = transform(mat, get(pm, body_ptr + 0x110, 3))
                ray_start = [world[i] + origin[i] for i in range(3)]
            else: ray_start = get(pm, effective_own + 0x60, 3)
            if all(v == v and abs(v) < 1e6 for v in ray_start + mat[8:11]):
                for k in range(FACE_COUNT):
                    if rng.random() < 0.5: put_face(pm, FACES + k * 0x80, face(rng, [ray_start[i] + mat[8 + i] * rng.uniform(0.1, 1.9) for i in range(3)], 0.1), rng.choice((0, 2, 8, 10)))
                struct.pack_into('<i', pm, effective_own + 0x5c4 + 0x2e0 - ARENA, rng.randrange(1, FACE_COUNT + 1))
        # A null input object is only safe when slot 0x24 is false (the wheel queries dereference it).
        chase = chase_type(tables, view_state, previous) in (0, 1, 5)
        if rng.random() < 0.2 and not chase: struct.pack_into('<I', pm, effective_own + 0x30 - ARENA, 0)
        statics_p = statics
        result = run('SelectUpdateType', pm, statics_p, [], precision, MOVER)
        kind_now = struct.unpack_from('<i', pm, MOVER + 0x48 - ARENA)[0]
        if kind_now in (2, 4):
            note('prefix-blocked' if result[0][0] == 0 else 'prefix-clear')
            if any(e[0] == 'ray' for e in result[3]): note('prefix-ray-hit' if result[0][0] == 0 else 'prefix-ray-miss')
            else: note('prefix-no-ray')
            note('prefix-cars-near-call' if any(e[0] == 'cars_near' for e in result[3]) else 'prefix-cars-near-skipped')
            note('prefix-human-inputs' if any(e[0] == 'has_inputs' for e in result[3]) else 'prefix-human-off')
        else: note('prefix-other-type')
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED), cases=args.cases,
                  traced_stubs=['D3DX multiply 0x63cedb', 'FindOrDefault shim (stores viewState to 0x8707dc, runs original 0x441140 as 0x5ecca0 does)'],
                  fixtures=['vtable slot 0x24 is original 0x445a90 in the original run and the rebuilt query in the rebuilt run; slot 0x44 is original 0x44ba70',
                            'input object vtable slot 0x54 is original 0x410470 in both runs'],
                  limitations=['finite and special generated values; exhaustive float coverage not established',
                               'signalling-NaN copies through the x87 (car +0x60..+0x68 y and z, body box) are not exercised',
                               'the prefix is compared from 0x453570 to 0x4536f4 as a region, not a function: the original frame and registers differ and are not compared',
                               'ReadSetting runs as original 0x445eb0; the reduced rebuild in DriveCamera.cpp is not compared',
                               'the settings lookup goes through a shim; the rebuilt table pointer/viewState platform fields are fixtures',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        C, P, R = 'cockpit', 'push', 'ray'
        mutations = {
            'player-override-ignored': (C, 'if (overridden) {', 'if (0) {'),
            'player-holder-unchecked': (C, '        if (holder) {', '        if (1) {'),
            'cars-skip-ignored': (C, '        if (car == skip) continue;\n', ''),
            'cars-inactive-kept': (C, '        if (!bytes[0x550]) continue;\n', ''),
            'cars-reach-no-margin': (C, 'margin + cameraCockpitReach7843E8', 'cameraCockpitReach7843E8'),
            'cars-no-invert': (C, '        CameraInvertRigid(&frame, &frame);\n', ''),
            'cars-margin-dropped': (C, 'Bounds3Contains(minimum, maximum, &local.x, margin)', 'Bounds3Contains(minimum, maximum, &local.x, 0.0f)'),
            'cars-box-swapped': (C, 'minimum[0] = body[0xe0 / 4];', 'minimum[0] = body[0xf0 / 4];'),
            'chase-types': (C, 'return type == 1 || type == 0 || type == 5;', 'return type == 1 || type == 0;'),
            'chase-type48': (C, 'FindOrDefault(previousType4C, viewState);\n    short type', 'FindOrDefault(type48, viewState);\n    short type'),
            'cockpit-previous-type': (C, 'FindOrDefault(type48, viewState);\n    float offset', 'FindOrDefault(previousType4C, viewState);\n    float offset'),
            'cockpit-margin': (C, 'AnyCarContains(offset, cameraCarHalf787088, own)', 'AnyCarContains(offset, 0.25f, own)'),
            'cockpit-no-transform': (C, '    WheelTransform(reinterpret_cast<WheelVector *>(offset), &frame, reinterpret_cast<WheelVector *>(offset));\n', ''),
            'human-slot-skipped': ('chase', 'if (!reinterpret_cast<DriveCameraChaseQuery>(vtable0[0x24 / 4])(this)) {', 'if (0) {'),
            'human-mode-limit': ('chase', 'car->mode424 <= 2', 'car->mode424 < 2'),
            'human-targets-dropped': ('chase', '    if (car->HasWheelTargets()) return true;\n', ''),
            'prefix-active54': (C, '    active54 = 0;\n    if (player20)', '    if (player20)'),
            'prefix-player-car': (C, '        car2C = player20->GetCar();', '        ;'),
            'prefix-hood-only': (C, 'type == DRIVE_CAMERA_HOOD || type == DRIVE_CAMERA_BUMPER', 'type == DRIVE_CAMERA_HOOD'),
            'prefix-input-slot': (C, 'if (input && input->GetWheelMode54()) {', 'if (0) {'),
            'prefix-cars-skipped': (C, '} else if (humanBlocked || carsNear) {', '} else if (humanBlocked) {'),
            'prefix-human-skipped': (C, '} else if (humanBlocked || carsNear) {', '} else if (carsNear) {'),
            'prefix-origin-y': (C, 'point[1] = point[1] + origin[1];', 'point[1] = point[1];'),
            'prefix-ray-length': (C, 'cameraBlockedRayLength78715C);', '1.0f);'),
            'prefix-ray-one-sided': (C, 'Ray(1, true, point', 'Ray(1, false, point'),
            'prefix-ray-margin': (C, '0.5f, 1.0f);', '0.25f, 1.0f);'),
            'prefix-snap-blocked': (C, 'snap52 = snap52 | (previousType4C != 0);', 'snap52 = snap52 | (previousType4C == 0);'),
            'prefix-snap-clear': (C, 'snap52 = snap52 | (previousType4C == 0);', 'snap52 = snap52 | (previousType4C != 0);'),
            'prefix-type-zero': (C, '            type = 0;', '            type = 1;'),
            'prefix-body-unchecked': (C, 'if (car->collisionBody3C) {', 'if (1) {'),
            'inputs-three': (P, 'for (int i = 0; i < 4; ++i) {\n        if (input30->wheelInputs1EC[i] > fengZero7D7D3C) {', 'for (int i = 0; i < 3; ++i) {\n        if (input30->wheelInputs1EC[i] > fengZero7D7D3C) {'),
            'inputs-at-least': (P, 'wheelInputs1EC[i] > fengZero7D7D3C', 'wheelInputs1EC[i] >= fengZero7D7D3C'),
            'targets-car-skipped': (P, '        if (own[i] > fengZero7D7D3C) {', '        if (0) {'),
            'targets-offset': (P, ' + 0x3f8);', ' + 0x3f4);'),
            'contains-margin-low': (R, 'minimum[0] - margin > point[0]', 'minimum[0] > point[0]'),
            'contains-high-inclusive': (R, 'margin + maximum[2] < point[2]', 'margin + maximum[2] <= point[2]'),
            'contains-nan-rejects': (R, 'if (minimum[1] - margin > point[1]) return 0;', 'if (!(minimum[1] - margin <= point[1])) return 0;'),
        }
        rejected = []
        for label, (key, before, after) in mutations.items():
            text = (ROOT / SOURCES[key]).read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--replace', key + '=' + str(mutant), '--cases', '512'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
