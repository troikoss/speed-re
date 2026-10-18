#!/usr/bin/env python3
"""Compare car update 0x6012b0 and its leaves (contact state 0x5fc050, opponent gap
0x5f6740, gear 0x5ebe00, smoothing 0x5ecac0, below-ground check 0x5ecba0, effect
emission 0x61fb00, upgrade level 0x5995e0, tick conversion 0x4ae7b0) under 53- and
64-bit x87 precision.

_ftol 0x75bed4 and the pair curve 0x576770/0x576790 execute original code in both
runs. Unrecovered callees are traced stubs patched at their original addresses
(reset paths 0x6009a0, 0x409770, 0x409960, 0x40f690, 0x401b20, 0x403db0, 0x5e2da0,
0x5fbed0; ground queries 0x5e7e70/0x5e7e80; effect spawn 0x615cc0) or fixture vtable
slots (mover +4, replay +0x48, participant +0x1c)."""

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

BUILD = ROOT / 'build/analysis/car-update'
ARENA, ARENA_SIZE, STACK, STOP, STUBS = 0x2000000, 0x4000, 0x2010000, 0x2100000, 0x2101000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
CAR, PHYS, CONTEXT, RACE, OBJ, OTHER_OBJ, PART, MOVER, BODY, REPLAY, EFFECTS, ROUTE, DIR, SLOT0, SLOT1, OTHER_CAR, VT = (
    ARENA + x for x in (0x0, 0xa00, 0x1000, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00, 0x2100, 0x2200, 0x2300,
                        0x2400, 0x2500, 0x2510, 0x2600, 0x3000))
# name: (symbol prefix, argument count, ST(0) result, eax mask)
TARGETS = {'Update': ('?Update@MoverModeOwner@', 1, True, 0), 'UpdateContactState': ('?UpdateContactState@', 0, False, 0),
           'UpdateOpponentGap': ('?UpdateOpponentGap@', 0, False, 0), 'UpdateGear': ('?UpdateGear@', 1, False, 0),
           'UpdateSmoothing': ('?UpdateSmoothing@', 1, False, 0), 'CheckBelowGround': ('?CheckBelowGround@', 0, False, 0),
           'EmitEffect': ('?EmitEffect@', 4, False, 0), 'GetLevel': ('?GetLevel@PhysicsUpgradeSelection@', 1, False, 0xffffffff),
           'TicksConstruct': ('?Construct@CarTicks@', 1, False, 0xffffffff),
           'NotifyContact': ('?NotifyContact@MoverModeOwner@', 1, False, 0)}
# Patched original-address stubs: address -> (kind, stack cleanup).
STUB_CODE = {0x6009a0: ('void', 0), 0x409770: ('void', 4), 0x409960: ('void', 4), 0x40f690: ('placement', 0),
             0x401b20: ('direction', 8), 0x403db0: ('void-cdecl', 0), 0x5e2da0: ('float', 4), 0x5fbed0: ('void', 0x20),
             0x5e72e0: ('ground', 0x10), 0x5e7e80: ('int', 4), 0x615cc0: ('void', 0x1c), 0x5f46a0: ('void', 4)}
SLOT_STUBS = {'mover+4': (STUBS + 0x00, 'float', 4), 'replay+48': (STUBS + 0x40, 'void', 0xc), 'participant+1c': (STUBS + 0x80, 'int', 0),
              'participant+38': (STUBS + 0xc0, 'void', 8)}
SHARED = {'?drivetrainClock@': 0x86518c, '?drivetrainClockScale@': 0x784268, '?hudZero7D7D3C@': 0x7d7d3c,
          '?hudOne784250@': 0x784250, '?carContactHold7A565C@': 0x7a565c, '?carSmoothingStep7A5654@': 0x7a5654,
          '?carSmoothingRate7A5658@': 0x7a5658, '?carResetSpeedLimit8021CC@': 0x8021cc,
          '?carResetExtension8021D0@': 0x8021d0, '?carGapScale88F2B0@': 0x88f2b0,
          '?carResetDirectionFlag88F2BE@': 0x88f2be, '?gameFlag89E7D8@': 0x89e7d8, '?gameFlag890108@': 0x890108,
          '?carGapEnabled89E7F8@': 0x89e7f8, '?playerCount8900BC@': 0x8900bc, '?hudPlayerSlots8900AC@': 0x8900ac,
          '?raceTuningData81CB60@': 0x81cb60, '?collisionEventOwner890080@': 0x890080,
          '?Construct@DriveFeedbackCurve@': 0x576770, '?Lookup@DriveFeedbackCurve@': 0x576790,
          '?QueryGround@WCollider@': 0x5e7e70, '?HasGround@WCollider@': 0x5e7e80, '?Spawn@CarEffectEmitters@': 0x615cc0,
          '?Reset6009A0@MoverRaceEntry@': 0x6009a0, '?Remove409770@CarResetHandler@': 0x409770,
          '?Restore409960@CarResetHandler@': 0x409960, '?GetResetPlacement@RaceParticipant@': 0x40f690,
          '?GetDirection@ParticipantRoute@': 0x401b20, '?ReleaseResetId403DB0@': 0x403db0,
          '?QueryDistance@ParticipantRoute@': 0x5e2da0, '?Reset5FBED0@MoverModeOwner@': 0x5fbed0, '?SwitchMover@MoverModeOwner@': 0x5f46a0,
          '__ftol': 0x75bed4, '__ftol2': 0x75bed4, '__fltused': 0}
INTERNAL = ('?UpdateContactState@', '?UpdateOpponentGap@', '?UpdateGear@', '?UpdateSmoothing@', '?CheckBelowGround@',
            '?EmitEffect@', '?GetLevel@', '?Construct@CarTicks@')
ARG_BYTES = {STUBS + 0x00: 4, STUBS + 0x40: 0xc, 0x409770: 4, 0x409960: 4, 0x403db0: 4, 0x5e2da0: 4, 0x5fbed0: 0x20,
             0x5e7e80: 4, 0x615cc0: 0x1c, 0x5f46a0: 4, STUBS + 0xc0: 8}
RESULT_FLOAT, RESULT_INT, GROUND_FOUND = STUBS + 0x200, STUBS + 0x204, STUBS + 0x208


def f32(x): return ctypes.c_float(x).value


def stub_code(kind, cleanup):
    ret = b'\xc2' + struct.pack('<H', cleanup) if cleanup else b'\xc3'
    if kind in ('float', 'ground'): return b'\xd9\x05' + struct.pack('<I', RESULT_FLOAT) + ret
    if kind == 'int': return b'\xa1' + struct.pack('<I', RESULT_INT) + ret
    if kind == 'placement': return b'\xb8' + struct.pack('<I', EFFECTS) + ret
    if kind == 'direction': return b'\xb8' + struct.pack('<I', DIR) + ret
    return ret


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Physics/CarUpdate.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/car-update.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = bytearray(pefile.PE(data=raw).get_memory_mapped_image())
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for address, (kind, cleanup) in STUB_CODE.items():
        code = stub_code(kind, cleanup); image[address - 0x400000:address - 0x400000 + len(code)] = code
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7',
                    '/EHsc', '/Fo' + str(obj.relative_to(ROOT)), os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0 or s['storage'] != 2: continue
        match = [SHARED[s['name']]] if s['name'] in SHARED else [a for p, a in SHARED.items() if p.startswith('?') and s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _, _) in TARGETS.items()}
    recovered = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    stub_sites = set(STUB_CODE) | {a for a, _, _ in SLOT_STUBS.values()}
    machines, traces, fixture = [], [[], []], {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x2000)
        for _, (address, kind, cleanup) in SLOT_STUBS.items(): uc.mem_write(address, stub_code(kind, cleanup))
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in recovered), ('state or call order mismatch', 'original reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            argv = bytes(uc.mem_read(esp + 4, 0x20))
            if address == 0x5e72e0:
                found = struct.unpack('<I', argv[12:16])[0]
                if found: uc.mem_write(found, bytes([fixture['found']]))
                argv = argv[:12]  # the found flag is a caller local
            elif address == 0x401b20: argv = argv[4:8]  # the output is a caller local
            else: argv = argv[:ARG_BYTES.get(address, 0)]
            trace.append((hex(address), this, argv))
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in stub_sites:
            target = 0x5e72e0 if address == 0x5e7e70 else address
            uc.hook_add(UC_HOOK_CODE, stub, begin=target, end=target)
        machines.append(uc)
    rng = random.Random(0x6012b0); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, globals_, argv, precision, this):
        _, argc, floating, mask = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for address, data in globals_: uc.mem_write(address, data)
            uc.mem_write(RESULT_FLOAT, struct.pack('<fi', fixture['float'], fixture['int']))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=2000000)
            except (UcError, AssertionError) as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            top = (uc.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7
            st0 = uc.reg_read(xc.UC_X86_REG_FP0 + top) if floating else None
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) & mask, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED),
                            top, st0, list(traces[rebuilt]), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + 4 * argc, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][6][i:i + 4].hex(), outputs[1][6][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][6][i:i + 4] != outputs[1][6][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), outputs[0][:5], outputs[1][:5],
                                  [t[0] for t in outputs[0][5]], [t[0] for t in outputs[1][5]],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][5], outputs[1][5])) if a != b), None), diff))
        counts[name] += 1
        return outputs[0]

    def note(path): paths[path] = paths.get(path, 0) + 1
    def r32(memory, address): return struct.unpack_from('<I', memory, address - ARENA)[0]
    def put(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def fl(): return f32(rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 8.0, 2.0, 2.5, float('nan'), rng.uniform(-2, 2), rng.uniform(-50, 50), rng.uniform(0, 12))))

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        clock = rng.choice((0, 1000, 800, 1200, 4000, rng.randrange(0, 1 << 20), rng.randrange(-(1 << 30), 1 << 30)))
        # Car object and its pointers.
        put(memory, CAR + 0x10, '<IIII', PHYS, CONTEXT, RACE, OBJ)
        put(memory, CAR + 0x28, '<iII', rng.choice((0, 1, 1, 2, 3)), PART, 0)
        put(memory, CAR + 0x34, '<IiI', MOVER, rng.choice((0, 1, 2, 3, 7)), BODY)
        put(memory, MOVER, '<I', VT); put(memory, VT + 4, '<I', SLOT_STUBS['mover+4'][0])
        put(memory, REPLAY, '<I', VT + 0x100); put(memory, VT + 0x100 + 0x48, '<I', SLOT_STUBS['replay+48'][0])
        # The original reset path dereferences the participant unconditionally.
        put(memory, PART, '<I', VT + 0x200); put(memory, VT + 0x200 + 0x1c, '<I', SLOT_STUBS['participant+1c'][0])
        put(memory, VT + 0x200 + 0x38, '<I', SLOT_STUBS['participant+38'][0])
        put(memory, PART + 0xc, '<I', rng.choice((ROUTE, ROUTE, 0))); memory[PART + 0x10 - ARENA] = rng.choice((0, 0, 1))
        put(memory, DIR, '<ff', fl(), fl()); put(memory, EFFECTS + 0x70, '<I', rng.choice((0, 2, 3, 0xffffffff)))
        put(memory, CONTEXT + 4, '<i', rng.choice((0, 1, 1, 2, 3, 3, 4, 5))); memory[CONTEXT + 0x900 - ARENA] = rng.choice((0, 0, 1))
        put(memory, RACE + 0x10, '<i', rng.choice((0, 1)))
        put(memory, OBJ + 0x14, '<f', fl()); put(memory, OTHER_OBJ + 0x14, '<f', fl())
        put(memory, OTHER_CAR + 0x1c, '<I', OTHER_OBJ); put(memory, SLOT0 + 4, '<I', OTHER_CAR); put(memory, SLOT1 + 4, '<I', OTHER_CAR)
        memory[BODY + 0x18 - ARENA] = rng.choice((0, 0, 1, 3, 0x80))
        put(memory, BODY + 0x13c, '<i', rng.choice((0, 0x7fffffff, clock, clock - rng.randrange(0, 4000), clock - rng.randrange(0, 1000))))
        put(memory, CAR + 0x424, '<i', rng.choice((0, 4, 4, 1)))
        put(memory, CAR + 0x42c, '<f', fl()); put(memory, CAR + 0x434, '<i', rng.choice((0, 1, 2, 3, 4, 5)))
        put(memory, CAR + 0x448, '<i', rng.choice((0, 1))); put(memory, CAR + 0x4d0, '<i', rng.choice((0, 1, 2, 3, 4, 5)))
        put(memory, CAR + 0x4d4, '<i', rng.choice((0, 1)))
        put(memory, CAR + 0x54c, '<f', rng.choice((0.0, f32(clock * 0.00025), f32(clock * 0.00025 - rng.uniform(0, 0.3)), fl(),
                                                   f32(f32(clock * f32(0.00025)) - f32(0.1)), f32(clock * f32(0.00025) - f32(0.1)))))
        put(memory, CAR + 0x554, '<I', rng.choice((EFFECTS, 0))); put(memory, CAR + 0x5b0, '<i', rng.choice((0, 0, 1)))
        for off in (0x60, 0x64, 0x68, 0x90, 0x94, 0x900, 0x904) + tuple(range(0x330, 0x340, 4)) + tuple(range(0x4e8, 0x508, 4)):
            put(memory, CAR + off, '<f', fl())
        put(memory, CAR + 0x968, '<I', rng.choice((REPLAY, 0))); put(memory, CAR + 0x96c, '<i', rng.choice((0, 0, 1)))
        put(memory, CAR + 0x974, '<i', rng.choice((0, 0x7fffffff, clock - rng.randrange(0, 12000), clock - rng.randrange(0, 4000))))
        put(memory, CAR + 0x978, '<f', rng.choice((f32(rng.uniform(0, 3)), 2.0, 0.5, fl())))
        put(memory, CAR + 0x9d8, '<i', rng.choice((clock, clock - rng.randrange(0, 4000), rng.getrandbits(31))))
        for i in range(10): put(memory, PHYS + 0x2f0 + i * 4, '<f', rng.choice((0.0, 1.0, 1.5, 2.0, 3.0, 2.49, fl())))
        put(memory, PHYS + 0x328, '<i', rng.choice((0, 1)))
        # Curve table at race tuning +0xa6c and the world's reset handler.
        put(memory, ARENA + 0x2700 + 0xa6c, '<8f', -10.0, -1.0, -1.0, 0.0, 1.0, 0.0, 10.0, 1.0)
        put(memory, ARENA + 0x3800 + 0x118, '<I', ARENA + 0x3900)
        globals_ = [(0x86518c, struct.pack('<i', clock)), (0x88f2b0, struct.pack('<f', fl())),
                    (0x88f2be, bytes([rng.choice((0, 0, 1))])), (0x89e7d8, bytes([rng.choice((0, 0, 1))])),
                    (0x890108, struct.pack('<i', rng.choice((0, 0, 1)))), (0x89e7f8, struct.pack('<i', rng.choice((0, 1)))),
                    (0x8900bc, struct.pack('<i', rng.choice((1, 2, 2)))), (0x8900ac, struct.pack('<II', SLOT0, SLOT1)),
                    (0x81cb60, struct.pack('<I', ARENA + 0x2700)), (0x890080, struct.pack('<I', ARENA + 0x3800))]
        fixture.update(float=fl(), int=rng.choice((0, 1, -1, 5)), found=rng.choice((0, 1, 1)))
        state38 = r32(memory, CAR + 0x38)
        result = run('Update', memory, globals_, [struct.pack('<f', fl())], precision, CAR)
        note('update-reset' if r32(memory, CAR + 0x96c) else 'update')
        if result[6][0x974:0x978] != bytes(memory[0x974:0x978]): note('update-reset-extended')
        for site, label in (('0x6009a0', 'reset-race-entry'), ('0x409770', 'reset-mode3'), ('0x409960', 'reset-mode3-restore'),
                            ('0x5fbed0', 'reset-placement'), ('0x5e2da0', 'reset-distance')):
            if any(t[0] == site for t in result[5]): note(label)
        run('UpdateContactState', memory, globals_, [], precision, CAR); note('contact-%d' % min(state38, 3))
        result = run('UpdateOpponentGap', memory, globals_, [], precision, CAR)
        if result[6][0x468:0x46c] != bytes(memory[0x468:0x46c]): note('opponent-gap')
        result = run('UpdateGear', memory, globals_, [rng.choice((0, 1, 2, 3, 4, 5))], precision, CAR)
        if any(t[0] == '0x615cc0' for t in result[5]): note('gear-effect')
        run('UpdateSmoothing', memory, globals_, [struct.pack('<f', fl())], precision, CAR)
        result = run('CheckBelowGround', memory, globals_, [], precision, CAR)
        if result[6][0x96c:0x970] != bytes(memory[0x96c:0x970]): note('below-ground')
        value = rng.choice((0, 0x10000, 0x30005, 0x80000, 0x90000, -0x10000, rng.getrandbits(31)))
        target = rng.choice((0, ARENA + 0x2800))
        run('EmitEffect', memory, globals_, [rng.choice((0, 3, 4, 8, 9, 10, 1)), struct.pack('<f', fl()), value, target], precision, CAR)
        run('GetLevel', memory, globals_, [rng.randrange(10)], precision, PHYS + 0x2f0)
        put(memory, CAR + 0x2c, '<I', rng.choice((PART, 0))); put(memory, CAR + 0x18, '<I', rng.choice((RACE, 0)))
        result = run('NotifyContact', memory, globals_, [OTHER_CAR], precision, CAR)
        note('contact-' + ','.join(t[0] for t in result[5]) if result[5] else 'contact-none')
        run('TicksConstruct', memory, globals_, [struct.pack('<f', fl())], precision, ARENA + 0x2900)
    report = dict(comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=['_ftol 0x75bed4', 'pair curve 0x576770/0x576790'],
                  traced_stubs=sorted(hex(a) for a in STUB_CODE) + sorted(SLOT_STUBS),
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'reset-path callees are stubs: the reset itself is not recovered',
                               'byte identity: 0x4ae7b0, 0x5995e0 and 0x5ecac0 only'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'contact-hold-strict': ('now - contactTime54C >= carContactHold7A565C', 'now - contactTime54C > carContactHold7A565C'),
            'gap-swapped': ('own * scale - opponent * scale', 'opponent * scale - own * scale'),
            'gear-no-mover-type': ('if (moverType28 == 1 || (float)', 'if ((float)'),
            'gear-level-half': ('level += 0.5f', 'level += 1.0f'),
            'smoothing-order': ('value4E8[i] += value4F8[i] * duration;', 'value4E8[i] += target[i] * duration;'),
            'below-margin': ('position20.z + 0.25f', 'position20.z'),
            'effect-high-limit': ('high <= 8', 'high < 8'),
            'update-no-check': ('    if (!context14->flag900) CheckBelowGround();\n', ''),
            'update-extension': ('since >= hudZero7D7D3C', 'since > hudZero7D7D3C'),
            'update-heading': ('heading = -1.0f;\n                }\n            }', 'heading = 1.0f;\n                }\n            }'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = args.source.parent / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            try:
                result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '2048'], cwd=ROOT, text=True,
                                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            finally:
                mutant.unlink()
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
