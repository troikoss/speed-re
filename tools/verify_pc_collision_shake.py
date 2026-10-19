#!/usr/bin/env python3
"""Compare the collision camera shake under 53- and 64-bit x87 precision: the view shake state
start 0x572810 (CameraViewShakeState::Start), the tick 0x5728c0, the reset 0x5729e0, the collision
producer 0x572e50, the player gate 0x5ea7c0 (PlayerRecord::QueryDriverCamera) and the option
mapping 0x502500 (DriveCameraTypeForOption, recovered in DriveCamera.cpp).

Two rebuilt objects are linked: CameraCollisionShake.cpp and DriveCamera.cpp. Original code runs for
the callees recovered elsewhere: the shake curve lookup 0x5722a0 and the length rescale 0x43cf90. The
other externals of DriveCamera.cpp (camera and engine globals and functions, the allocation operators)
are linked to a trap address that fails the comparison if it is ever executed; 0x502500 calls none of
them. The compared state is the whole fixture arena, the shake, player and option globals, the return
value (AL for the gate, EAX for the mapping), ESP, callee-saved registers, FPU control word and tag
word, and the entry events (callee and arguments, pointers by content) in call order. The fixtures put
the magnitude exactly at 250 and 600 and set a constant curve equal to the amplitude 0.08 at 250.
The driver camera branch of the gate is never taken by the original: 0x502500 never returns type 3."""

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

BUILD = ROOT / 'build/analysis/collision-shake'
ARENA, ARENA_SIZE, STACK, STOP, TRAP = 0x2000000, 0x4000, 0x2010000, 0x2100000, 0x2310000
STACK_RANGE = (STACK - 0x1000, STACK + 0x100)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
SOURCES = {'collision': 'src/Speed/Indep/Src/Camera/CameraCollisionShake.cpp', 'drive': 'src/Speed/Indep/Src/Camera/DriveCamera.cpp'}
# name: (rebuilt symbol prefix, callee-popped bytes, return kind)
TARGETS = {'Start': ('?Start@CameraViewShakeState@', 12, None), 'Tick': ('?CameraViewShakeTick@', 0, None),
           'Reset': ('?CameraViewShakeReset@', 0, None), 'Collision': ('?CameraCollisionShake@', 0, None),
           'DriverCamera': ('?QueryDriverCamera@PlayerRecord@', 0, 'al'),
           'DriveCameraType': ('?DriveCameraTypeForOption@', 0, 'eax')}
# Entry events: name: (original address, rebuilt symbol prefix or None for shared original code, `this` kind, stack arguments).
# Argument kinds: 'w' raw word, 'o' output pointer (arena address or `stack`), ('p', n) input pointer by content.
EVENTS = {
    'start': (0x572810, '?Start@CameraViewShakeState@', 'this', [('p', 12), 'w', 'w']),
    'lookup': (0x5722a0, None, ('p', 8), ['w']),
    'set_length': (0x43cf90, None, None, ['o', ('p', 12), 'w']),
    'driver_camera': (0x5ea7c0, '?QueryDriverCamera@PlayerRecord@', 'this', []),
    'drive_type': (0x502500, '?DriveCameraTypeForOption@', None, ['w']),
}
# Retail globals the rebuilt code reads or writes, by MSVC symbol prefix.
SHARED = {'?Lookup@CameraViewShakeCurve@': 0x5722a0, '?WheelSetLength@': 0x43cf90, '__fltused': 0,
          '?fengZero7D7D3C@': 0x7d7d3c, '?frameDuration865154@': 0x865154, '?cameraNegativeOne7842A0@': 0x7842a0,
          '?cameraViewShakeRequest8650C4@': 0x8650c4, '?cameraViewShakeStart7FB730@': 0x7fb730,
          '?cameraViewShakeCurve7FB7A0@': 0x7fb7a0, '?cameraViewShakeDuration79DC64@': 0x79dc64,
          '?cameraViewShakeFrequency79DC68@': 0x79dc68, '?cameraCollisionMagnitudeLow79DCD4@': 0x79dcd4,
          '?cameraCollisionMagnitudeHigh79DCD8@': 0x79dcd8, '?cameraCollisionAmplitudeLow79DCDC@': 0x79dcdc,
          '?cameraCollisionAmplitudeHigh79DCE0@': 0x79dce0, '?cameraCollisionDurationLow79DCE4@': 0x79dce4,
          '?cameraCollisionDurationHigh79DCE8@': 0x79dce8, '?cameraCollisionScaleX7FB750@': 0x7fb750,
          '?cameraCollisionScaleY7FB754@': 0x7fb754, '?cameraCollisionScaleZ7FB758@': 0x7fb758,
          '?cameraCollisionAmplitudeScale7850EC@': 0x7850ec, '?cameraCollisionDurationScale78A0EC@': 0x78a0ec,
          '?cameraViewShake865360@': 0x865360, '?players8900AC@': 0x8900ac, '?playerSettings83AA64@': 0x83aa64}
# Compared globals: the shake requests, the states, the frame time, the player pointers and the option rows.
GLOBALS = ((0x8650c4, 4), (0x865360, 0x40), (0x865154, 4), (0x8900ac, 8), (0x83aa64, 0x50))
CURVE_IMAGE = 0x7fb7a0
# Fixture arena layout.
(PLAYER0, PLAYER1, CAMERA0, CAMERA1, ACC, STATE, CURVE_A, CURVE_B, POINTS, POINTS2, CONST, CONST_OBJ) = (
    ARENA + x for x in (0x100, 0x400, 0x800, 0x840, 0x900, 0xa00, 0xb00, 0xb10, 0xc00, 0xc80, 0xd00, 0xd20))


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: collision, drive)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--targets', default='', help='comma separated subset of targets')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/collision-shake.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-fno-inline'] + ['/clang:' + f for f in args.clang_flag])
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Camera',
                                   '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append((key, obj))
    parsed = {key: read_object(obj) for key, obj in objects}
    defined = {s['name'] for p in parsed.values() for s in p['symbols'].values() if s['section'] > 0}
    externals, trapped = {}, set()
    for key, p in parsed.items():
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for pre, a in SHARED.items() if s['name'].startswith(pre)]
            if len(match) == 1:
                externals[s['name']] = match[0]
            else:
                assert key == 'drive' and not match, ('unexpected external', key, s['name'])
                externals[s['name']] = TRAP; trapped.add(s['name'])
    linked = link_objects([obj for _, obj in objects], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    defaults = {a: bytes(image[a - 0x400000:a - 0x400000 + n]) for a, n in GLOBALS}
    selected = [t for t in args.targets.split(',') if t] or list(TARGETS)
    machines, events = [], [[], []]

    def view(uc, w, size):
        if ARENA <= w < ARENA + ARENA_SIZE: return ('arena', w, bytes(uc.mem_read(w, size)) if size else None)
        if STACK_RANGE[0] <= w < STACK_RANGE[1]: return ('stack', bytes(uc.mem_read(w, size)) if size else None)
        return w
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000); uc.mem_map(0, 0x1000)
        uc.mem_map(linked['address'], linked['allocation']); uc.mem_map(TRAP, 0x1000)
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def trap(uc, address, size, user):
            raise AssertionError(('state or call order mismatch', 'unlinked callee reached', hex(address)))
        def entry(uc, address, size, user, log=events[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            _, _, this_kind, spec = EVENTS[user]
            words = struct.unpack('<%dI' % len(spec), uc.mem_read(esp + 4, 4 * len(spec))) if spec else ()
            seen = []
            for kind, w in zip(spec, words):
                if kind == 'w': seen.append(w)
                elif kind == 'o': seen.append(('arena', w) if ARENA <= w < ARENA + ARENA_SIZE else 'stack' if STACK_RANGE[0] <= w < STACK_RANGE[1] else w)
                else: seen.append(view(uc, w, kind[1]))
            if this_kind is None: head = None
            elif this_kind == 'this': head = view(uc, this, 0)
            else: head = view(uc, this, this_kind[1])
            log.append((user, head, tuple(seen)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        uc.hook_add(UC_HOOK_CODE, trap, begin=TRAP, end=TRAP)
        for name, (address, prefix, _, _) in EVENTS.items():
            if prefix is not None and rebuilt: address = next(a for s, a in linked['exports'].items() if s.startswith(prefix))
            uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x572810); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, statics, argv, precision, this):
        _, cleanup, returns = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            events[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for address, data in defaults.items(): uc.mem_write(address, statics.get(address, data))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<f', f32(x)) if isinstance(x, float) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this); uc.reg_write(xc.UC_X86_REG_EDX, 0x5a5a5a5a)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=4000000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not finish')
            value = None
            if returns == 'al': value = uc.reg_read(xc.UC_X86_REG_EAX) & 0xff
            elif returns == 'eax': value = uc.reg_read(xc.UC_X86_REG_EAX)
            contract = (value, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW))
            outputs.append((contract, uc.reg_read(xc.UC_X86_REG_FPTAG), list(events[rebuilt]),
                            tuple(bytes(uc.mem_read(a, n)) for a, n in GLOBALS), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][0][1] == STACK + 4 + cleanup and outputs[0][0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            a, b = outputs
            diff = [(hex(ARENA + i), a[4][i:i + 4].hex(), b[4][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if a[4][i:i + 4] != b[4][i:i + 4]][:8]
            k = next((i for i, (x, y) in enumerate(zip(a[2], b[2])) if x != y), None)
            raise AssertionError(('state or call order mismatch', name, hex(precision), argv, a[0], b[0], a[1], b[1],
                                  [e[0] for e in a[2]], [e[0] for e in b[2]], None if k is None else (k, a[2][k], b[2][k] if k < len(b[2]) else None),
                                  [(hex(GLOBALS[i][0]), x.hex(), y.hex()) for i, (x, y) in enumerate(zip(a[3], b[3])) if x != y], diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def note(path): paths[path] = paths.get(path, 0) + 1

    def special(): return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-20, -1e-20, 1e-45, 1e30, 3e38, float('inf'), -float('inf'), float('nan'),
                                      rng.uniform(-2, 2), rng.uniform(-500, 500)))
    def gen(): return rng.choice((rng.gauss(0, 1), rng.uniform(-30, 30), rng.gauss(0, 1) * 1e-3)) if rng.random() < 0.9 else special()

    def players(memory, statics):
        for base, camera in ((PLAYER0, CAMERA0), (PLAYER1, CAMERA1)):
            pk(memory, base + 0x10, '<i', rng.choice((0, 0, 1)))
            pk(memory, base + 0x58, '<i', rng.choice((7, 7, 0, 1, 3, 6, 8)))
            pk(memory, base + 0x238, '<I', rng.choice((camera, camera, camera, 0)))
            pk(memory, camera + 0xc, '<i', rng.choice((1, 1, 1, 0, 2, 4)))
            pk(memory, camera + 0x14, '<i', rng.choice((1, 1, 0, 2, -1)))
        statics[0x8900ac] = struct.pack('<II', PLAYER0, PLAYER1)
        row = bytearray(image[0x83aa64 - 0x400000:0x83aa64 - 0x400000 + 0x50])
        for k in range(2): struct.pack_into('<i', row, 0x28 * k + 0x10, rng.choice((0, 1, 2, 3, 4, 5, 6, -1, 0)))
        statics[0x83aa64] = bytes(row)

    def curves(memory, constant):
        # CURVE_A and CURVE_B: random keys (sorted x, arbitrary y) or specials; CONST: two keys of `constant`.
        for obj, points in ((CURVE_A, POINTS), (CURVE_B, POINTS2)):
            count = rng.choice((0, 1, 2, 3, 5, 5, 5, 8))
            xs = sorted(rng.uniform(-0.2, 1.4) for _ in range(count)) if rng.random() < 0.85 else [gen() for _ in range(count)]
            put(memory, points, [x for i in range(count) for x in (xs[i], gen())])
            pk(memory, obj, '<II', points, count)
        put(memory, CONST, [0.0, constant, 1.0, constant])
        pk(memory, CONST_OBJ, '<II', CONST, 2)

    def states(view_slot, constant):
        data = bytearray()
        for slot in range(2):
            vector = [gen() for _ in range(3)]
            if slot == view_slot and rng.random() < 0.5:
                curve, time = CONST_OBJ, rng.choice((0.0, 0.25, rng.uniform(0, 1.0), 0.999999, special()))
            else:
                curve = rng.choice((0, 0, CURVE_IMAGE, CURVE_A, CURVE_B, CONST_OBJ))
                time = rng.choice((rng.uniform(-0.1, 1.3), rng.uniform(0, 1.0), 0.0, 1.0, 0.5, special()))
            duration = rng.choice((0.6, 1.0, 0.7, 0.3, rng.uniform(0.05, 2.0), 0.0, special()))
            frequency = rng.choice((17.0, 17.0, 0.0, gen()))
            data += struct.pack('<3f', *[f32(v) for v in vector]) + bytes(rng.randbytes(4))
            data += struct.pack('<I', curve) + struct.pack('<fff', f32(time), f32(duration), f32(frequency))
        assert len(data) == 0x40
        return bytes(data)

    def frame_time():
        return rng.choice((1 / 60, 0.001, 0.0, 0.05, -1 / 60, 0.2, gen(), special()))

    def expected_amplitude(magnitude):
        # 0.08 + (0.16 - 0.08) * t with t = (m - 250) / 350 clamped to 600, in binary32.
        clamped = min(magnitude, 600.0)
        t = f32((clamped - 250.0) / 350.0)
        return f32(f32(f32(0.16) - f32(0.08)) * t + f32(0.08))

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        for name in selected:
            memory = bytearray(rng.randbytes(ARENA_SIZE)); statics = {}; argv = []; this = 0
            if name == 'Start':
                put(memory, ACC, [gen() for _ in range(3)])
                argv = [ACC, rng.choice((0.6, 0.3, 1.0, 0.0, gen(), special())), rng.choice((17.0, 0.0, gen(), special()))]
                run(name, memory, statics, argv, precision, STATE); note('start-vector-%s' % ('finite' if f32(argv[1]) == argv[1] else 'special'))
                continue
            if name == 'Tick':
                curves(memory, rng.choice((0.0, 0.5, 0.08, gen(), special())))
                statics = {0x8650c4: struct.pack('<i', rng.choice((0, 1, 1))), 0x865154: struct.pack('<f', f32(frame_time())),
                           0x865360: states(rng.choice((0, 1)), 0.0)}
                result = run(name, memory, statics, [], precision, 0)
                before = [struct.unpack_from('<I', statics[0x865360], 0x20 * slot + 0x10)[0] for slot in (0, 1)]
                after = [struct.unpack_from('<I', result[3][1], 0x20 * slot + 0x10)[0] for slot in (0, 1)]
                note('tick-dropped-curve' if any(b and not a for b, a in zip(before, after)) else 'tick-kept-curves')
                note('tick-request' if statics[0x8650c4] != struct.pack('<i', 0) else 'tick-no-request')
                continue
            if name == 'Reset':
                statics = {0x865360: bytes(rng.randbytes(0x40))}
                run(name, memory, statics, [], precision, 0); note('reset')
                continue
            if name == 'Collision':
                view_index = rng.choice((0, 1))
                players(memory, statics)
                mode = rng.random()
                if mode < 0.35:
                    magnitude = rng.choice((0.0, 100.0, 249.5, 250.0, 250.0, 250.00002, 251.0, 300.0, 450.0, 599.0, 600.0, 600.0, 600.5,
                                            700.0, 1e4, 3e38, float('inf'), float('nan'), rng.uniform(200, 700)))
                    acceleration = [0.0, 0.0, magnitude / 2.0]
                    constant = f32(0.08) if magnitude == 250.0 else expected_amplitude(magnitude) if magnitude == magnitude else 0.0
                    if rng.random() < 0.3: constant = rng.choice((0.0, f32(0.08 * (1 + 2 ** -23)), gen()))
                elif mode < 0.6:
                    scale = rng.choice((1.0, 10.0, 100.0))
                    acceleration = [gen() * scale for _ in range(3)]
                    constant = rng.choice((0.0, 0.08, gen(), special()))
                elif mode < 0.85:
                    # A direction with a magnitude between 240 and 620 that is not a binary32 value, so
                    # the unrounded magnitude and its rounding reach the amplitude and duration.
                    direction = [rng.gauss(0, 1) for _ in range(3)]
                    norm = (sum(x * x for x in direction) ** 0.5) or 1.0
                    magnitude = rng.uniform(240.0, 620.0)
                    acceleration = [direction[0] / norm * magnitude / 0.7, direction[1] / norm * magnitude / 0.5,
                                    direction[2] / norm * magnitude / 2.0]
                    constant = rng.choice((expected_amplitude(magnitude), 0.0, 0.08, gen()))
                else:
                    acceleration = [special() for _ in range(3)]
                    constant = special()
                put(memory, ACC, acceleration)
                curves(memory, constant)
                statics[0x865360] = states(view_index, constant)
                result = run(name, memory, statics, [view_index, ACC], precision, 0)
                note('collision-view-%d' % view_index); note('collision-%s' % ('magnitude' if mode < 0.35 else 'general' if mode < 0.6 else 'directional' if mode < 0.85 else 'special'))
                note('collision-shake-started' if any(e[0] == 'start' for e in result[2]) else 'collision-no-shake')
                note('collision-curve-lookup' if any(e[0] == 'lookup' for e in result[2]) else 'collision-no-lookup')
                if mode < 0.35 and magnitude in (250.0, 600.0): note('collision-magnitude-%d' % magnitude)
                continue
            if name == 'DriverCamera':
                players(memory, statics)
                this = rng.choice((PLAYER0, PLAYER1))
                result = run(name, memory, statics, [], precision, this); note('driver-camera-%d' % result[0][0])
                continue
            if name == 'DriveCameraType':
                option = rng.choice((-1, 0, 1, 2, 3, 4, 5, 6, 7, -2, 0x7fffffff, -0x80000000, rng.randrange(-100, 100)))
                result = run(name, memory, statics, [option], precision, 0); note('drive-type-%d' % result[0][0])
                continue
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], cases=args.cases,
                  shared_original_callees=['view shake curve lookup 0x5722a0', 'length rescale 0x43cf90'],
                  trapped_externals=sorted(trapped),
                  fixtures=['players, camera movers, option rows, curves and shake states are written per case; the arena is random-filled and compared whole',
                            'magnitudes include exactly 250 and 600 (the acceleration is on the z axis, so the magnitude is exact), NaN and infinity',
                            'the constant curve equals the amplitude 0.08 at magnitude 250, so the amplitude test is an equality'],
                  limitations=['finite and special generated values; exhaustive float coverage not established',
                               'the driver camera branch of the gate is unreachable: no option maps to type 3, so the gate is false for every input',
                               'signalling NaNs and NaN payloads are not generated (canonical quiet NaNs only)',
                               'the stack alignment (AND ESP,-16) of 0x572e50 is not reproduced; the compiled frame is compared by content, not address',
                               'byte identity not established (compiler output differs from the original)'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'start-time': ('collision', '    time14 = 0.0f;\n    duration18 = duration;', '    time14 = 0.5f;\n    duration18 = duration;'),
            'start-y-source': ('collision', '    vector0[1] = y;', '    vector0[1] = vector[0];'),
            'start-frequency': ('collision', '    value1C = frequency;', '    value1C = duration;'),
            'start-curve': ('collision', '    curve10 = &cameraViewShakeCurve7FB7A0;', '    curve10 = 0;'),
            'tick-flag-kept': ('collision', '        cameraViewShakeRequest8650C4 = 0;\n', ''),
            'tick-z-constant': ('collision', 'state->vector0[2] = cameraViewShakeStart7FB730[2]', 'state->vector0[2] = cameraViewShakeStart7FB730[1]'),
            'tick-duration': ('collision', 'state->duration18 = cameraViewShakeDuration79DC64;', 'state->duration18 = cameraViewShakeFrequency79DC68;'),
            'tick-slot1': ('collision', '    AdvanceViewShake(&cameraViewShake865360[1]);', ''),
            'advance-divide': ('collision', 'float time = frameDuration865154 / state->duration18 + state->time14;', 'float time = frameDuration865154 * state->duration18 + state->time14;'),
            'advance-store': ('collision', '    state->time14 = time;\n', ''),
            'advance-first-key': ('collision', 'if (time < points[0] || time >= points[curve->count4 * 2 - 2]) {', 'if (time <= points[0] || time >= points[curve->count4 * 2 - 2]) {'),
            'advance-last-key': ('collision', 'if (time < points[0] || time >= points[curve->count4 * 2 - 2]) {', 'if (time < points[0] || time > points[curve->count4 * 2 - 2]) {'),
            'advance-nan': ('collision', 'if (time < points[0] || time >= points[curve->count4 * 2 - 2]) {', 'if (time < points[0] || !(time < points[curve->count4 * 2 - 2])) {'),
            'reset-duration': ('collision', '        state->duration18 = 1.0f;', '        state->duration18 = 0.0f;'),
            'reset-frequency': ('collision', '        state->time14 = 0.0f;\n        state->duration18 = 1.0f;\n        state->value1C = cameraViewShakeFrequency79DC68;', '        state->time14 = 0.0f;\n        state->duration18 = 1.0f;\n        state->value1C = 0.0f;'),
            'reset-padding': ('collision', '        state->curve10 = 0;\n        state->time14 = 0.0f;\n        state->duration18 = 1.0f;', '        state->curve10 = 0;\n        state->unidentifiedC[0] = 0;\n        state->time14 = 0.0f;\n        state->duration18 = 1.0f;'),
            'reset-slot': ('collision', 'for (int slot = 0; slot < 2; ++slot) {', 'for (int slot = 0; slot < 1; ++slot) {'),
            'collision-driver-gate': ('collision', '    if (player->QueryDriverCamera()) {', '    if (0) {'),
            'collision-player-view': ('collision', '    PlayerRecord *player = players8900AC[view];', '    PlayerRecord *player = players8900AC[0];'),
            'collision-scale-x': ('collision', 'scaled[0] = cameraCollisionScaleX7FB750 * acceleration->x;', 'scaled[0] = cameraCollisionScaleY7FB754 * acceleration->x;'),
            'collision-z-factor': ('collision', '(cameraCollisionScaleZ7FB758 * acceleration->z) * scaled[2]', 'acceleration->z * scaled[2]'),
            'collision-sum-order': ('collision', 'scaled[0] * scaled[0] +', 'scaled[1] * scaled[1] +'),
            'collision-low-equal': ('collision', '    if (magnitude < cameraCollisionMagnitudeLow79DCD4) {', '    if (magnitude <= cameraCollisionMagnitudeLow79DCD4) {'),
            'collision-high-clamp': ('collision', '        magnitude = cameraCollisionMagnitudeHigh79DCD8;', '        magnitude = cameraCollisionMagnitudeHigh79DCD8 + 1.0f;'),
            'collision-t-numerator': ('collision', 'volatile float t = (magnitude - cameraCollisionMagnitudeLow79DCD4) /', 'volatile float t = (magnitude) /'),
            'collision-amplitude-square': ('collision', '(cameraCollisionAmplitudeHigh79DCE0 - cameraCollisionAmplitudeLow79DCDC) * t +', '(cameraCollisionAmplitudeHigh79DCE0 - cameraCollisionAmplitudeLow79DCDC) * t * t +'),
            'collision-current-default': ('collision', '    float current = fengZero7D7D3C;', '    float current = 0.5f;'),
            'collision-equal-amplitude': ('collision', '    if (!(amplitude > current)) {', '    if (!(amplitude >= current)) {'),
            'collision-state58': ('collision', '    if (player && player->state58 == 7) {', '    if (player && player->state58 == 6) {'),
            'collision-amplitude-scale': ('collision', 'amplitude = amplitude * cameraCollisionAmplitudeScale7850EC;', 'amplitude = amplitude * cameraCollisionDurationScale78A0EC;'),
            'collision-duration-scale': ('collision', 'duration = duration * cameraCollisionDurationScale78A0EC;', 'duration = duration * cameraCollisionAmplitudeScale7850EC;'),
            'collision-length-output': ('collision', '    WheelSetLength(&shaken, (WheelVector *)scaled, amplitude);', '    WheelSetLength(&shaken, (WheelVector *)scaled, duration);'),
            'collision-start-frequency': ('collision', 'state->Start(&shaken.x, duration, cameraViewShakeFrequency79DC68);', 'state->Start(&shaken.x, duration, cameraViewShakeDuration79DC64);'),
            'driver-kind': ('collision', '    if (!camera || camera->kindC != 1 || camera->active14 == 0) {', '    if (!camera || camera->kindC != 2 || camera->active14 == 0) {'),
            'driver-active': ('collision', '    if (!camera || camera->kindC != 1 || camera->active14 == 0) {', '    if (!camera || camera->kindC != 1 || camera->active14 == 1) {'),
            'driver-type': ('collision', '    return DriveCameraTypeForOption(playerSettings83AA64[index10].value10) == DRIVE_CAMERA_DRIVER;', '    return DriveCameraTypeForOption(playerSettings83AA64[index10].value10) == DRIVE_CAMERA_BUMPER;'),
            'driver-row': ('collision', '    return DriveCameraTypeForOption(playerSettings83AA64[index10].value10) == DRIVE_CAMERA_DRIVER;', '    return DriveCameraTypeForOption(playerSettings83AA64[0].value10) == DRIVE_CAMERA_DRIVER;'),
            'mapping-bumper': ('drive', '    case 0:\n        return DRIVE_CAMERA_BUMPER;', '    case 0:\n        return DRIVE_CAMERA_HOOD;'),
            'mapping-hood': ('drive', '    case 1:\n        return DRIVE_CAMERA_HOOD;', '    case 1:\n        return DRIVE_CAMERA_FAR;'),
            'mapping-close': ('drive', '    case 2:\n        return DRIVE_CAMERA_CLOSE;', '    case 2:\n        return DRIVE_CAMERA_FAR;'),
            'mapping-super-far': ('drive', '    case 4:\n        return DRIVE_CAMERA_SUPER_FAR;', '    case 3:\n        return DRIVE_CAMERA_SUPER_FAR;'),
            'mapping-default': ('drive', '    default:\n        return DRIVE_CAMERA_FAR;', '    default:\n        return DRIVE_CAMERA_DRIVER;'),
        }
        rejected = []
        for label, (key, before, after) in mutations.items():
            text = (ROOT / SOURCES[key]).read_text(); assert text.count(before) >= 1, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--replace', key + '=' + str(mutant), '--cases', '384'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            if not (result.returncode and 'state or call order mismatch' in result.stdout): report.setdefault('survived_mutations', []).append(label)
            else: rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
