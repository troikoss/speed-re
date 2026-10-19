#!/usr/bin/env python3
"""Compare the drive camera's chase car frame under 53- and 64-bit x87 precision: the heading
follower steering 0x445c20 (SteerHeading), the chase car frame 0x445e00 (GetCarFrame), the
blocked expression 0x445ac0 (HumanBlocked) and the target setter 0x574cd0.

Two rebuilt objects are linked: CameraChase.cpp and CameraPush.cpp (the wheel queries 0x5ed290 and
0x5ed300 that 0x445ac0 calls, and the frame 0x401340 that 0x445e00 calls). WCollisionRay.cpp is
linked only to satisfy CameraPush.cpp's own symbols; no case reaches it. Original code runs for
the callees that are not recovered here or are recovered elsewhere: identity 0x43c780, normalise
0x43cf10, cross 0x43d1e0, scale-add 0x43d0c0, the target setter body 0x570ba0 and the heading
curve lookup 0x576790 (CameraPairCurve::Lookup is recovered in DriveCamera.cpp but is not linked
here). Vtable slot 0x24 (0x445a90) is a 6-byte fixture stub that returns a byte from the arena,
the same code in both runs, so the chase/non-chase choice is a direct input. The compared state
is the whole fixture arena (the output matrix, the heading follower's 0x90 bytes, the car and the
mover), return value where one exists, ESP, callee-saved registers, FPU control word and tag word,
the globals 0x86518c and 0x82a4e0, and the entry events (callee and arguments, pointers into the
stack by content) in call order."""

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
from verify_pc_camera_cockpit import SHARED

BUILD = ROOT / 'build/analysis/camera-chase'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x1800, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
SOURCES = {'chase': 'src/Speed/Indep/Src/Camera/CameraChase.cpp', 'push': 'src/Speed/Indep/Src/Camera/CameraPush.cpp',
           'ray': 'src/Speed/Indep/Src/World/WCollisionRay.cpp'}
# name: (rebuilt symbol prefix, callee-popped bytes, returns a bool in AL)
TARGETS = {'HumanBlocked': ('?HumanBlocked@DriveCameraMover@', 0, True), 'SetTargetPointer': ('?SetTarget@CameraCubicVector@@QAEXPBM', 4, False),
           'GetCarFrame': ('?GetCarFrame@DriveCameraMover@', 4, False), 'SteerHeading': ('?SteerHeading@DriveCameraMover@', 8, False)}
# Entry events: original address for the original run, rebuilt symbol prefix for the rebuilt run.
DEFINED_EVENTS = {'human': (0x445ac0, '?HumanBlocked@DriveCameraMover@'), 'inputs': (0x5ed290, '?HasWheelInputs@MoverModeOwner@'),
                  'targets': (0x5ed300, '?HasWheelTargets@MoverModeOwner@'), 'frame': (0x401340, '?WheelFrameAt@'),
                  'setptr': (0x574cd0, '?SetTarget@CameraCubicVector@@QAEXPBM')}
SHARED_EVENTS = {'identity': 0x43c780, 'normalize': 0x43cf10, 'cross': 0x43d1e0, 'scaleadd': 0x43d0c0, 'settarget': 0x570ba0, 'lookup': 0x576790}
# Stack argument words of each event (the callee-popped words only; thiscall `this` is ECX).
ARGS = {'human': 0, 'inputs': 0, 'targets': 0, 'frame': 3, 'setptr': 1, 'identity': 1, 'normalize': 2, 'cross': 3, 'scaleadd': 4,
        'settarget': 3, 'lookup': 1, 'slot24': 0}
THISCALL = ('human', 'inputs', 'targets', 'setptr', 'lookup', 'slot24', 'settarget')
CLOCK, CURVE = 0x86518c, 0x82a4e0
GLOBALS = ((CLOCK, 4), (CURVE, 8))
(MOVER, INPUT, HEAD, SETTING, OUT, VEC, POINTS, CAR) = (ARENA + x for x in (0x100, 0x200, 0x600, 0x700, 0x800, 0x900, 0x1700, 0x1000))
FLAG = ARENA + 0x80
EXTRA = ARENA + 0xa000
VTABLE, STUB = EXTRA, EXTRA + 0x100
DEFAULT_POINTS = 0x7ec420


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity
def s32(x): return (x + 0x80000000) % 0x100000000 - 0x80000000


def special(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-20, -1e-20, 1e-45, 1e30, 3e38, float('inf'), -float('inf'), float('nan'),
                       rng.uniform(-2, 2), rng.uniform(-500, 500)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: chase, push, ray)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-chase.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    # MSVC 7.1 rejects the __thiscall keyword (C4234). The only typedef'd calls (vtable slot 0x24 and the cockpit's slot 0x44)
    # take no stack arguments, where __fastcall (this in ECX, EDX unused) is the same call, so the keyword is defined away.
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/D__thiscall=__fastcall'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno'] + ['/clang:' + f for f in args.clang_flag])
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2']  + ['/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Camera', '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    parsed = [read_object(o) for o in objects]
    defined = {s['name'] for p in parsed for s in p['symbols'].values() if s['section'] > 0}
    externals = {}
    for p in parsed:
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for pre, a in SHARED.items() if s['name'].startswith(pre)]
            assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    rebuilt_events = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (_, p) in DEFINED_EVENTS.items()}
    original_events = {n: a for n, (a, _) in DEFINED_EVENTS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, events = [], [[], []]
    stack_range = (STACK - 0x1000, STACK + 0x100)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        vt = [0] * 0x14; vt[0x24 // 4] = STUB
        uc.mem_write(VTABLE, struct.pack('<20I', *vt))
        uc.mem_write(STUB, bytes([0xa0]) + struct.pack('<I', FLAG) + bytes([0xc3]))  # mov al, [FLAG]; ret
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def entry(uc, address, size, user, log=events[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            words = struct.unpack('<%dI' % ARGS[user], uc.mem_read(esp + 4, 4 * ARGS[user])); view = []
            for w in words:
                if ARENA <= w < ARENA + ARENA_SIZE: view.append(('arena', w, bytes(uc.mem_read(w, 12))))
                elif stack_range[0] <= w < stack_range[1]: view.append(('stack', bytes(uc.mem_read(w, 12))))
                else: view.append(w)
            log.append((user, (this if not (stack_range[0] <= this < stack_range[1]) else 'stack') if user in THISCALL else None, tuple(view)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        for name, address in (rebuilt_events if rebuilt else original_events).items():
            uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        for name, address in SHARED_EVENTS.items(): uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        uc.hook_add(UC_HOOK_CODE, entry, user_data='slot24', begin=STUB, end=STUB)
        machines.append(uc)
    rng = random.Random(0x445c20); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, statics, argv, precision, this):
        _, cleanup, returns = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            events[rebuilt].clear()
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
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not finish')
            value = (uc.reg_read(xc.UC_X86_REG_EAX) & 0xff) if returns else 0
            contract = (value, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW))
            outputs.append((contract, uc.reg_read(xc.UC_X86_REG_FPTAG), list(events[rebuilt]),
                            tuple(bytes(uc.mem_read(a, s)) for a, s in GLOBALS), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][0][1] == STACK + 4 + cleanup and outputs[0][0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            a, b = outputs
            diff = [(hex(ARENA + i), a[4][i:i + 4].hex(), b[4][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if a[4][i:i + 4] != b[4][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), a[0], b[0], a[1], b[1], [e[0] for e in a[2]], [e[0] for e in b[2]],
                                  next(((x, y) for x, y in zip(a[2], b[2]) if x != y), None), [(x.hex(), y.hex()) for x, y in zip(a[3], b[3]) if x != y], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def get(memory, address, count): return list(struct.unpack_from('<%df' % count, memory, address - ARENA))
    def note(path): paths[path] = paths.get(path, 0) + 1
    def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)

    def gen():
        return rng.choice((rng.gauss(0, 1), rng.uniform(-30, 30), rng.gauss(0, 1) * 1e-3)) if rng.random() < 0.88 else special(rng)

    def unit():
        v = [rng.gauss(0, 1) for _ in range(3)]; n = math.sqrt(sum(x * x for x in v)) or 1.0
        return [x / n for x in v]

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # --- mover, car, input ---
        pk(memory, MOVER, '<I', VTABLE); pk(memory, MOVER + 0x2c, '<I', CAR); pk(memory, MOVER + 0x40, '<I', HEAD)
        memory[MOVER + 0x52 - ARENA] = rng.choice((0, 1, 1, 2, 0x80)); memory[FLAG - ARENA] = rng.choice((0, 1, 1, 1, 0x80))
        clock = rng.choice((0, 5, 1000, 123456, 0x7fffffff, -0x80000000, rng.randrange(-0x80000000, 0x80000000)))
        delta = rng.choice((0, 1, -1, 100, 2000, 7999, 8000, 8001, 4000, 16000, 100000, -4000, -100000, 0x7fffffff, -0x80000000, rng.randrange(-20000, 20000), rng.randrange(-0x80000000, 0x80000000)))
        pk(memory, MOVER + 0x58, '<i', s32(clock - delta))
        pk(memory, CAR + 0x30, '<I', INPUT); pk(memory, CAR + 0x38, '<I', rng.choice((0, 1, 0x1234)))
        pk(memory, CAR + 0x424, '<i', rng.choice((-1, 0, 1, 2, 3, 5)))
        forward = unit(); row = forward + [0.0] if rng.random() < 0.7 else [gen(), gen(), gen(), gen()]
        put(memory, CAR + 0x90, row + [x for _ in range(3) for x in (unit() + [0.0])])
        put(memory, CAR + 0x50, [gen(), gen(), gen()])
        speed = rng.choice((0.0, -0.0, 3.0, 8.0, 10.0, 13.0, 20.0, rng.uniform(0, 30), rng.uniform(-5, 40), special(rng)))
        put(memory, CAR + 0x430, [speed])
        force = False
        kind = rng.choice(('back', 'forward', 'zero', 'tiny', 'random', 'random', 'special', 'side', 'cancel', 'cancel', 'underflow'))
        if kind == 'back': v = [-x * rng.uniform(0.1, 40) for x in forward]
        elif kind == 'forward': v = [x * rng.uniform(0.1, 40) for x in forward]
        elif kind == 'zero': v = [rng.choice((0.0, -0.0))] * 3
        elif kind == 'tiny': v = [rng.uniform(-1, 1) * 1e-30 for _ in range(3)]
        elif kind == 'special': v = [special(rng) for _ in range(3)]
        elif kind == 'side': v = [a * 5 for a in unit()]
        elif kind == 'cancel':  # two products that cancel and one small: the accumulation order decides the sign
            v = [1e20, -1e20, rng.choice((1.0, -1.0, 0.5, -0.5, 3.0))]; rng.shuffle(v); force = rng.random() < 0.8; put(memory, CAR + 0x90, [1.0, 1.0, 1.0, 0.0])
        elif kind == 'underflow':  # the dot product is nonzero in the x87 register but zero as a binary32
            v = [rng.choice((1e-30, -1e-30, 3e-25)) for _ in range(3)]; put(memory, CAR + 0x90, [rng.choice((-1e-20, 1e-20, -3e-21)) for _ in range(3)] + [0.0]); force = rng.random() < 0.8
        else: v = [rng.uniform(-30, 30) for _ in range(3)]
        put(memory, CAR + 0x70, v)
        put(memory, CAR + 0x3f8, [rng.choice((0.0, -0.0, -1.0, 0.25, special(rng))) for _ in range(4)])
        put(memory, INPUT + 0x1ec, [rng.choice((0.0, -0.0, -1.0, -1e-20, float('nan'), -float('inf'), special(rng), rng.uniform(0, 1), 1e-30)) for _ in range(8)])
        if rng.random() < 0.5: put(memory, INPUT + 0x1ec, [min(0.0, x) if x == x else x for x in get(memory, INPUT + 0x1ec, 8)])
        # --- heading follower: 3 channels of 0x30 bytes ---
        for c in range(3):
            base = HEAD + c * 0x30
            put(memory, base, [gen() for _ in range(4)])  # value, velocity, target, target velocity
            pk(memory, base + 0x2c, '<hh', rng.choice((0, 1, 2, 3, -1, 0x1234)), rng.choice((0, 1, 2)))
        if rng.random() < 0.3:  # a target equal to the value, so SetTarget does not restart the channel
            for c in range(3): memory[HEAD + c * 0x30 + 8 - ARENA:HEAD + c * 0x30 + 12 - ARENA] = memory[HEAD + c * 0x30 - ARENA:HEAD + c * 0x30 + 4 - ARENA]
        if rng.random() < 0.4: put(memory, HEAD, [rng.gauss(0, 1) for _ in range(1)])
        # --- setting, curve ---
        pk(memory, SETTING, '<hH', rng.choice((1, 0, 5, 1, 0, 5, 2, 3, 4, 6, -1, 0x7fff, 0x100)) if not force else rng.choice((1, 0, 5)), rng.randrange(65536))
        curve = struct.pack('<II', DEFAULT_POINTS, 2)
        if rng.random() < 0.4:
            xs = sorted(rng.uniform(0, 30) for _ in range(2)); ys = [rng.uniform(-1, 3), rng.uniform(-1, 3)]
            put(memory, POINTS, [xs[0], ys[0], xs[1], ys[1]]); curve = struct.pack('<II', POINTS, 2)
        statics = [struct.pack('<i', clock), curve]
        snap = rng.choice((0, 1)) if not force else 1
        # --- 0x445ac0 ---
        result = run('HumanBlocked', memory, statics, [], precision, MOVER)
        note('human-yes' if result[0][0] else 'human-no')
        if not memory[FLAG - ARENA]: note('human-slot-false')
        # --- 0x574cd0 ---
        put(memory, VEC, [gen(), gen(), gen()])
        run('SetTargetPointer', memory, statics, [VEC], precision, HEAD)
        # --- 0x445e00 ---
        result = run('GetCarFrame', memory, statics, [OUT], precision, MOVER)
        note('frame-chase' if memory[FLAG - ARENA] else 'frame-car')
        # --- 0x445c20 ---
        argument = snap | rng.choice((0, 0, 0x100, 0x5500))
        result = run('SteerHeading', memory, statics, [SETTING, argument], precision, MOVER)
        names = [e[0] for e in result[2]]
        t = struct.unpack_from('<h', memory, SETTING - ARENA)[0]
        note('steer-chase' if t in (1, 0, 5) else 'steer-other')
        note('steer-snap' if snap else 'steer-nosnap')
        if t in (1, 0, 5):
            note('steer-human-returned' if 'human' in names and 'lookup' not in names else 'steer-computed' if 'lookup' in names else 'steer-skipped-by-snap')
            if 'lookup' in names:
                elapsed = s32(clock - struct.unpack_from('<i', memory, MOVER + 0x58 - ARENA)[0]); ratio = 1.0 - elapsed / 8000.0
                note('steer-time-zero' if ratio <= 0 else 'steer-time-one' if ratio > 1 else 'steer-time-mid')
            note('steer-reflect' if names.count('scaleadd') == 2 else 'steer-no-reflect')
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED_EVENTS), cases=args.cases,
                  fixtures=['vtable slot 0x24 is a fixture stub (mov al, [arena flag]; ret) in both runs; the wheel queries and 0x401340 are rebuilt in the rebuilt run and original in the original run',
                            'the tick global 0x86518c and the heading curve 0x82a4e0 (default points 0x7ec420 or arena points) are written per case; the curve static initialiser 0x77be00 is not run'],
                  limitations=['finite and special generated values; exhaustive float coverage not established',
                               'signalling NaNs are not generated: the original copies some floats through integer registers and others through the x87',
                               'the heading curve lookup 0x576790, the setter body 0x570ba0 and the vector helpers run as original code in both runs',
                               'the snap reset block is compared as inline stores; CameraCubicVector::Snap in DriveCamera.cpp is not linked',
                               'the stub for slot 0x24 stands in for 0x445a90, which is compared by verify_pc_camera_cockpit.py',
                               'byte identity not established (compiler output differs from the original)'], fingerprints=manifest)
    if args.mutation_checks:
        Cs = 'chase'
        mutations = {
            'steer-type-5-dropped': (Cs, 'if (type == 1 || type == 0 || type == 5) {\n        if (!snap', 'if (type == 1 || type == 0) {\n        if (!snap'),
            'steer-type-0-dropped': (Cs, 'if (type == 1 || type == 0 || type == 5) {\n        if (!snap', 'if (type == 1 || type == 5) {\n        if (!snap'),
            'steer-human-ignored': (Cs, 'if (!snap && HumanBlocked()) {', 'if (0) {'),
            'steer-human-before-snap': (Cs, 'if (!snap && HumanBlocked()) {', 'if (HumanBlocked()) {'),
            'steer-dot-order': (Cs, 'velocity.z * row[2] + velocity.y * row[1] + velocity.x * row[0]', '(velocity.x * row[0] + velocity.y * row[1]) + velocity.z * row[2]'),
            'steer-reflect-always': (Cs, 'if (dot < fengZero7D7D3C) {', 'if (1) {'),
            'steer-reflect-le': (Cs, 'if (dot < fengZero7D7D3C) {', 'if (dot <= fengZero7D7D3C) {'),
            'steer-reflect-scale': (Cs, 'dot * cameraReflectScale7855D0', 'dot * -1.0f'),
            'steer-curve-speed': (Cs, 'Lookup(car2C->speed430)', 'Lookup(0.0f)'),
            'steer-scale-x-dropped': (Cs, '        velocity.x = velocity.x * s;\n', ''),
            'steer-scale-z-dropped': (Cs, '        velocity.z = velocity.z * s;\n', ''),
            'steer-blend': (Cs, 'fengOne784250 - s);', 's);'),
            'steer-ground-time': (Cs, 'cameraClock86518C - groundTime58', 'cameraClock86518C'),
            'steer-clamp-low': (Cs, '            t = fengZero7D7D3C;', '            t = 0.25f;'),
            'steer-clamp-high': (Cs, '} else if (fengOne784250 < t) {', '} else if (0) {'),
            'steer-z-not-scaled': (Cs, 'velocity.z = t * velocity.z;\n', ''),
            'steer-y-scaled': (Cs, 'velocity.z = t * velocity.z;', 'velocity.y = t * velocity.y;'),
            'steer-no-snap-return': (Cs, '        if (!snap) {\n            return;\n        }\n    } else {', '    } else {'),
            'steer-snap-velocity': (Cs, 'channels[i].velocity4 = channels[i].targetVelocityC;', 'channels[i].velocity4 = channels[i].velocity4;'),
            'steer-snap-state': (Cs, 'channels[i].state2C = 0;', 'channels[i].state2C = 2;'),
            'steer-other-target': (Cs, 'heading40->SetTarget(reinterpret_cast<float *>(&CarState(car2C)->matrix50));', 'heading40->SetTarget(reinterpret_cast<float *>(&CarState(car2C)->matrix50) + 4);'),
            'frame-slot-ignored': (Cs, 'if (reinterpret_cast<DriveCameraChaseQuery>(vtable0[0x24 / 4])(this)) {\n        WheelIdentity', 'if (0) {\n        WheelIdentity'),
            'frame-heading-y': (Cs, 'heading.y = heading40->channels0[1].value0;', 'heading.y = heading40->channels0[1].target8;'),
            'frame-cross-swapped': (Cs, 'WheelCross(row1, row2, row0);', 'WheelCross(row1, row0, row2);'),
            'frame-no-identity': (Cs, '        WheelIdentity(output);\n', ''),
            'frame-w': (Cs, 'output->elements[15] = 1.0f;\n    } else', 'output->elements[15] = 0.0f;\n    } else'),
            'frame-origin-z': (Cs, 'output->elements[14] = state->origin10.z;', 'output->elements[14] = state->origin10.y;'),
            'human-slot-skipped': (Cs, 'if (!reinterpret_cast<DriveCameraChaseQuery>(vtable0[0x24 / 4])(this)) {\n        return false;', 'if (0) {\n        return false;'),
            'human-mode-limit': (Cs, 'car->mode424 <= 2', 'car->mode424 < 2'),
            'human-targets-dropped': (Cs, '    if (car->HasWheelTargets()) return true;\n', ''),
            'setptr-order': (Cs, 'SetTarget(target[0], target[1], target[2]);', 'SetTarget(target[2], target[1], target[0]);'),
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
