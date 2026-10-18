#!/usr/bin/env python3
"""Compare the camera mover pushes: drive camera slot 0x30 0x445a60, face push 0x444190,
world push 0x444530, the player query 0x5eb520 and the car body accessors 0x5ec250/0x5ec240, under 53- and 64-bit
x87 precision.

The collider ray 0x5d39f0, face test 0x5d35b0, scale-add 0x43d0c0, transform 0x441000,
distance 0x43ce10, rigid inverse 0x5baf90 and re-aim 0x444390 execute original code in
both runs (verified separately). Traced stubs: the collider
constructor 0x5e0b40 (which installs the fixture's face area), area query 0x5e6fa0,
destructor 0x5e0bf0 and the D3DX multiply 0x63cedb."""



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

BUILD = ROOT / 'build/analysis/camera-push'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x3000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
TARGETS = {'NearRadius': ('?NearRadius@CameraMoverBase@', 0, 0, 0), 'PushFromFaces': ('?PushFromFaces@CameraMoverBase@', 4, 16, 0xff),
           'PushFromWorld': ('?PushFromWorld@CameraMoverBase@', 3, 12, 0xff), 'GetBodyBox': ('?GetBodyBox@MoverModeOwner@', 2, 8, 0),
           'GetBodyCentre': ('?GetBodyCentre@MoverModeOwner@', 0, 0, 0xffffffff),
           'PlayerQuery': ('?Query5EB520@CameraPlayer@', 0, 0, 0xff)}
D3DX, CONSTRUCT, QUERY, DESTRUCT = 0x63cedb, 0x5e0b40, 0x5e6fa0, 0x5e0bf0
SHARED = {'?Ray@WCollider@': 0x5d39f0, '?ContainsPoint@WCollisionFace@': 0x5d35b0, '?WheelScaleAdd@': 0x43d0c0,
          '?WheelTransform@': 0x441000, '?bDistance@': 0x43ce10, '?CameraInvertRigid@': 0x5baf90, '?CameraReaim@': 0x444390,
          '?Construct@WCollider@': CONSTRUCT, '?QueryArea@WCollider@': QUERY,
          '?Destruct@WCollider@': DESTRUCT, '?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250, '?fengHalf784260@': 0x784260,
          '?cameraNearRadius787090@': 0x787090, '?cameraLookBackRadius787094@': 0x787094, '?cameraCarMargin78708C@': 0x78708c,
          '?cameraFaceDepth78709C@': 0x78709c, '?cameraRayMargin7870A0@': 0x7870a0, '?surfaceCameraSkip800EED@': 0x800eed, '__fltused': 0,
          # Externals of the other-car push in the same file (verified by verify_pc_camera_cars.py).
          '?WheelNormalize@': 0x43cf10, '?GetVelocity@MoverModeOwner@': 0x4018a0, '?Query571250@Object865480@': 0x571250,
          '?object865480@': 0x865480, '?cameraCarHalf787088@': 0x787088, '?cameraCarSpeed7870C4@': 0x7870c4,
          '?cameraCarRange7870C8@': 0x7870c8, '?cameraCarHeading7870C0@': 0x7870c0, '?cameraLiftMargin784358@': 0x784358,
          '?carList890080@': 0x890080, '?cameraNearbyCarId7EB5C0@': 0x7eb5c0, '?cameraNearbyCar82A580@': 0x82a580,
          '?cameraNearbyGuard82A590@': 0x82a590}
MOVER, VTABLE, PLAYER, COLLIDER, FACES, CAR, BODY, VIEW, FROM, POINT, OUT, FOCUS = (ARENA + x for x in (
    0x100, 0x180, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x2200, 0x2300, 0x2310, 0x2320, 0x2340))
FACE_COUNT = 8


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def value(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, 1e30, float('inf'), float('nan'), rng.uniform(-2, 2), rng.uniform(-500, 500)))


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


def face(rng, centre_near=None):
    import math
    centre = [rng.uniform(-6, 6) for _ in range(3)] if centre_near is None else [c + rng.uniform(-3, 3) for c in centre_near]
    u = rng.choice(([rng.gauss(0, 1) for _ in range(3)], [rng.uniform(-.3, .3), rng.uniform(-.3, .3), 1.0],
                    [rng.gauss(0, 1), rng.gauss(0, 1), rng.uniform(-.2, .2)]))
    n = math.sqrt(dot(u, u)) or 1.0; u = [x / n for x in u]
    t = cross(u, [1, 0, 0] if abs(u[0]) < 0.9 else [0, 1, 0]); n = math.sqrt(dot(t, t)); t = [x / n for x in t]
    w = cross(u, t); count = rng.choice((3, 4)); size = rng.uniform(2, 15)
    angles = [0.3, 1.9, 3.4, 4.9] if count == 4 else sorted(rng.uniform(0, 2 * math.pi) for _ in range(3))
    verts = [[f32(centre[k] + size * (math.cos(a) * t[k] + math.sin(a) * w[k])) for k in range(3)] for a in angles]
    if rng.random() < 0.5: verts.reverse()
    normal = cross(sub(verts[1], verts[0]), sub(verts[2], verts[0])); n = math.sqrt(dot(normal, normal)) or 1.0
    normal = [f32(x / n) for x in normal]
    if rng.random() < 0.06: normal[rng.randrange(3)] = rng.choice((float('nan'), float('inf')))  # NaN dots pass 0x5d35b0
    return count, verts, normal, f32(dot(normal, verts[0]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Camera/CameraPush.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-push.json').read_text())
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
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _, _) in TARGETS.items()}
    # Slot 0x30 (0x445a60, with its player query 0x5eb520) is reached through the fixture vtable in both runs.
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for n, f in manifest['functions'].items() if n not in ('NearRadius', 'PlayerQuery')]
    machines, traces, fixture = [], [[], []], {}

    def collider_state(uc, this):
        return bytes(uc.mem_read(this + 0x1c, 0x20)) + bytes(uc.mem_read(this + 0x2f0, 0x14))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0, 0x1000)  # FS:[0] exception list of the original's SEH frame
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
            elif address == CONSTRUCT:
                # The fixture's face area stands in for the collider the query would fill.
                state = bytearray(0x314)
                struct.pack_into('<I', state, 8, fixture['pack'])
                struct.pack_into('<i', state, 0x2e0, fixture['faces'])
                for i in range(fixture['faces']): struct.pack_into('<I', state, 0x3c + i * 4, FACES + i * 0x80)
                uc.mem_write(this, bytes(state)); trace.append(('construct',)); cleanup = 0
            elif address == QUERY:
                point, radius, velocity = struct.unpack('<IfI', uc.mem_read(esp + 4, 12))
                trace.append(('query', bytes(uc.mem_read(point, 12)), struct.pack('<f', radius), velocity, collider_state(uc, this))); cleanup = 12
            else:
                trace.append(('destruct', collider_state(uc, this))); cleanup = 0
            uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in (D3DX, CONSTRUCT, QUERY, DESTRUCT): uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x444530); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, argv, precision, this):
        _, argc, cleanup, mask = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(0, bytes(0x1000)); uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=2000000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            st0 = None
            if name == 'NearRadius':
                top = (uc.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7  # ST(0) is physical register TOP
                st0 = uc.reg_read(xc.UC_X86_REG_FP0 + top)
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) & mask, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED),
                            uc.reg_read(xc.UC_X86_REG_FPCW), st0, list(traces[rebuilt]), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][6][i:i + 4].hex(), outputs[1][6][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][6][i:i + 4] != outputs[1][6][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), outputs[0][:5], outputs[1][:5],
                                  [t[0] for t in outputs[0][5]], [t[0] for t in outputs[1][5]],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][5], outputs[1][5])) if a != b), None), diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def put_face(memory, address, spec, flags):
        count, verts, normal, distance = spec
        memory[address + 8 - ARENA:address + 12 - ARENA] = bytes((rng.randrange(256), count, flags, rng.randrange(0, 24)))
        for i, v in enumerate(verts): put(memory, address + 0x30 + i * 0x10, v)
        put(memory, address + 0x70, normal); put(memory, address + 0x7c, [distance])
    def note(path): paths[path] = paths.get(path, 0) + 1
    def look_at(eye, target):
        import math
        f = sub(target, eye); n = math.sqrt(dot(f, f)) or 1.0; f = [x / n for x in f]
        r = cross([0, 0, -1], f); n = math.sqrt(dot(r, r)) or 1.0; r = [x / n for x in r]
        u = cross(f, r)
        m = [r[0], u[0], f[0], 0, r[1], u[1], f[1], 0, r[2], u[2], f[2], 0, 0, 0, 0, 1]
        m[12] = -dot(eye, r); m[13] = -dot(eye, u); m[14] = -dot(eye, f)
        return m
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # Mover with a vtable whose slot 0x30 is the original drive camera radius.
        vtable = [0] * 0x14; vtable[0x30 // 4] = 0x445a60
        struct.pack_into('<20I', memory, VTABLE - ARENA, *vtable); struct.pack_into('<I', memory, MOVER - ARENA, VTABLE)
        memory[MOVER + 0x51 - ARENA] = rng.choice((0, 1, 2))
        struct.pack_into('<I', memory, MOVER + 0x20 - ARENA, rng.choice((PLAYER, PLAYER, 0)))
        memory[PLAYER + 0x2b - ARENA] = rng.choice((0, 0, 1, 0x80))
        result = run('NearRadius', memory, [], precision, MOVER); note('radius')
        run('PlayerQuery', memory, [], precision, PLAYER)
        # Face area around the focus/camera region.
        centre = [rng.uniform(-3, 3) for _ in range(3)]
        faces = rng.randrange(0, FACE_COUNT + 1)
        for i in range(FACE_COUNT): put_face(memory, FACES + i * 0x80, face(rng, centre), rng.choice((0, 2, 8, 10)))
        struct.pack_into('<I', memory, COLLIDER + 8 - ARENA, rng.choice((1, 1, 1, 0)))
        struct.pack_into('<i', memory, COLLIDER + 0x2e0 - ARENA, faces)
        for i in range(FACE_COUNT): struct.pack_into('<I', memory, COLLIDER + 0x3c + i * 4 - ARENA, FACES + i * 0x80)
        start = [c + rng.uniform(-10, 10) for c in centre]; point = [c + rng.uniform(-4, 4) for c in centre]
        put(memory, FROM, start); put(memory, POINT, point)
        result = run('PushFromFaces', memory, [COLLIDER, FROM, POINT, OUT], precision, MOVER)
        note('faces-hit' if result[0] else 'faces-clear')
        if struct.unpack('<3f', bytes(memory[OUT - ARENA:OUT - ARENA + 12])) != struct.unpack('<3f', bytes(result[6][OUT - ARENA:OUT - ARENA + 12])): note('faces-moved')
        # Car, body and view for the world push.
        struct.pack_into('<I', memory, CAR + 0x3c - ARENA, rng.choice((BODY, BODY, 0)))
        carpos = [rng.uniform(-3, 3) for _ in range(3)]; put(memory, CAR + 0x50, carpos)
        import math
        yaw = rng.uniform(-math.pi, math.pi)
        put(memory, CAR + 0x90, [math.cos(yaw), math.sin(yaw), 0, 0, -math.sin(yaw), math.cos(yaw), 0, 0, 0, 0, 1, 0, carpos[0], carpos[1], carpos[2], 1])
        bodycentre = [rng.uniform(-.5, .5), rng.uniform(-.5, .5), rng.uniform(0, .8)]
        half = [rng.uniform(.8, 2.5), rng.uniform(.6, 1.2), rng.uniform(.4, .9)]
        put(memory, BODY + 0x110, bodycentre); put(memory, BODY + 0xe0, [c - h for c, h in zip(bodycentre, half)])
        put(memory, BODY + 0xf0, [c + h for c, h in zip(bodycentre, half)])
        groups = rng.choice((0, 1, 3, 4, -1))
        struct.pack_into('<i4I', memory, CAR + 0x5c4 + 0x2f0 - ARENA, groups, *(rng.getrandbits(32) for _ in range(4)))
        run('GetBodyBox', memory, [FROM, OUT], precision, CAR); run('GetBodyCentre', memory, [], precision, CAR)
        distance = rng.choice((rng.uniform(0.5, 3), rng.uniform(3, 8), rng.uniform(0.1, 1)))
        angle = rng.uniform(-math.pi, math.pi)
        eye = [carpos[0] + distance * math.cos(angle), carpos[1] + distance * math.sin(angle), carpos[2] + rng.uniform(-0.5, 2.5)]
        put(memory, VIEW, look_at(eye, [c + rng.uniform(-.3, .3) for c in carpos]))
        fixture['pack'] = rng.choice((1, 1, 1, 0)); fixture['faces'] = rng.randrange(0, FACE_COUNT + 1)
        for i in range(FACE_COUNT): put_face(memory, FACES + i * 0x80, face(rng, eye if rng.random() < 0.6 else carpos), rng.choice((0, 2, 8, 10)))
        focus = rng.choice((0, 0, FOCUS)); put(memory, FOCUS, [c + rng.uniform(-1, 1) for c in carpos])
        car = CAR if focus == 0 or rng.random() < 0.7 else 0
        result = run('PushFromWorld', memory, [VIEW, car, focus], precision, MOVER)
        note('world-hit' if result[0] else 'world-clear')
        if result[6][VIEW - ARENA + 0x30:VIEW - ARENA + 0x40] != bytes(memory[VIEW - ARENA + 0x30:VIEW - ARENA + 0x40]): note('world-view-moved')
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  traced_stubs=['collider constructor 0x5e0b40 (installs the fixture face area)',
                                'area query 0x5e6fa0', 'collider destructor 0x5e0bf0', 'D3DX multiply 0x63cedb'],
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'collider construction/query/destruction stubbed; their bodies are verified separately',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'radius-ignore-player': ('if (lookBack51 && player20 && player20->Query5EB520())', 'if (lookBack51 && player20)'),
            'faces-no-pack-writes': ('    if (!collider->pack8) return false;\n', '    if (!collider->pack8) { offset[0] = 0.0f; return false; }\n'),
            'faces-one-sided': ('collider->Ray(1, true, from, end', 'collider->Ray(1, false, from, end'),
            'faces-mode0': ('collider->Ray(1, true,', 'collider->Ray(0, true,'),
            'faces-skip-surface': ('            if (surfaceCameraSkip800EED[face->surfaceB * 0x98]) continue;\n', ''),
            'faces-fixed-limit': ('float limit = (face->flagsA & 2) ?', 'float limit = (face->flagsA & 4) ?'),
            'faces-depth-weight': ('WheelScaleAdd(&sum, &sum, &face->normal70, limit - depth);', 'WheelScaleAdd(&sum, &sum, &face->normal70, depth);'),
            'faces-sum-not-mean': ('(float)(fengOne784250 / count)', '1.0f'),
            'faces-two-passes': ('for (int pass = 0; pass < 3; ++pass)', 'for (int pass = 0; pass < 2; ++pass)'),
            'faces-w-kept': ('    offset[3] = 0.0f;\n', ''),
            'world-focus-origin': ('    } else if (car->collisionBody3C) {', '    } else if (0) {'),
            'world-middle-eye': ('WheelScaleAdd(&middle, &middle, reinterpret_cast<WheelVector *>(focus), 0.5f);', 'WheelScaleAdd(&middle, &middle, reinterpret_cast<WheelVector *>(focus), 0.0f);'),
            'world-radius-no-one': ('+ fengOne784250) * fengHalf784260);', ') * fengHalf784260);'),
            'world-no-groups': ('        collider.groupCount2F0 = count;\n', ''),
            'world-extended-x': ('    volatile float x = focus[1]', '    float x = focus[1]'),
            'world-rounded-y': ('    float y = focus[1]', '    volatile float y = focus[1]'),
            'world-no-push': ('    eye[2] = eye[2] + push[2];\n', ''),
            'world-sphere-margin': ('+ cameraCarMargin78708C;', ';'),
            'world-sphere-centre-z': ('eye[2] = (float)(sqrt(reach * reach - (storedX2 + storedY2)) + focus[2]);', 'eye[2] = (float)(sqrt(reach * reach - (storedX2 + storedY2)) + eye[2]);'),
            'world-rounded-reach': ('        double reach = sqrt(', '        float reach = (float)sqrt('),
            'world-target-eye': ('CameraReaim(view, reinterpret_cast<WheelVector *>(eye), reinterpret_cast<WheelVector *>(focus), projected);', 'CameraReaim(view, reinterpret_cast<WheelVector *>(eye), &middle, projected);'),
            'player-query-inverted': ('return reinterpret_cast<char *>(this)[0x2b] == 0;', 'return reinterpret_cast<char *>(this)[0x2b] != 0;'),
            'box-swap': ('    float *a = reinterpret_cast<float *>(body + 0xe0);', '    float *a = reinterpret_cast<float *>(body + 0xf0);'),
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
