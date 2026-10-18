#!/usr/bin/env python3
"""Compare participant setup 0x6065b0, car creation 0x606480, car constructor 0x604260,
physics binding 0x601110, car list rebuild 0x5f0d20, physics setup copy 0x413440 and the
leaves 0x4133d0, 0x59b060, 0x5b5350, 0x639ce0 and 0x5a93a0 under 53- and 64-bit x87
precision.

Every unrecovered callee is a traced stub patched at its original address; recovered
routines in the list call each other, so the nested runs compare whole chains. Stack
pointers passed to stubs are compared by their contents. 0x606480 receives its grid entry
in ecx and its record in edx; the rebuilt function takes all four arguments on the stack.
The rebuilt CarNullMover vtable is normalised to 0x7a5bb0."""

import argparse
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

BUILD = ROOT / 'build/analysis/car-construction'
ARENA, ARENA_SIZE = 0x2000000, 0x40000
STACK_BASE, STACK, STOP, TIB, GDT = 0x2040000, 0x204f000, 0x2100000, 0x2120000, 0x2130000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
WORLD, RECORDS, SETUP, CAR, CARS, VT, TABLE, MISC, HEAP = (ARENA + x for x in (
    0x0, 0x1000, 0x6000, 0x7000, 0x8000, 0xf000, 0xd000, 0xf800, 0x10000))
HEAP_END = ARENA + 0x3c000
RECORD_SIZE, MAX_RECORDS = 0x940, 8
STUB_DTOR = 0x2101000
NULL_MOVER_VTABLE = 0x7a5bb0
# Original address: (rebuilt symbol, stack cleanup, argument bytes, traced this, result)
CALLEES = {
    0x575620: ('?AllocateCarMemory@', 0, 4, False, 'alloc'),
    0x575630: ('?FreeCarMemory@', 0, 4, False, None),
    0x440e50: ('?bOMalloc@', 0, 4, False, 'slot'),
    0x5eb5d0: ('?GetStartingPositions@', 0, 8, False, 'grid'),
    0x5e02b0: ('?RegisterUpdate5E02B0@', 0, 8, False, 'value'),
    0x5f4400: ('?SelectMover5F4400@', 0, 4, False, 'value'),
    0x5da2f0: ('?Construct@WRoadNav@@QAEPAU1@XZ', 0, 0, True, 'this'),
    0x5ded90: ('?Construct@WRoadNav@@QAEPAU1@PAUStartingPosition@@@Z', 4, 4, True, 'nav'),
    0x5da3d0: ('?SetPosition@WRoadNav@@QAEXPAUWheelVector@@@Z', 4, 4, True, None),
    0x5da410: ('?SetPosition@WRoadNav@@QAEXPAUWheelVector@@MH@Z', 0xc, 0xc, True, None),
    0x5e0b40: ('?Construct@WCollider@', 0, 0, True, None),
    0x59f880: ('?Reset@DrivetrainAuxiliary@', 0, 0, True, None),
    0x5b4fc0: ('?Refresh@PhysicsParameters@', 0xc, 0xc, True, None),
    0x6399d0: ('?SetupParts@PhysicsInfo@', 8, 8, True, None),
    0x5eb930: ('??0CarDriver@', 4, 4, True, 'this'),
    0x63b590: ('??0CarTracker914@', 0, 0, True, 'this'),
    0x60bcf0: ('?Bind@CarTracker914@', 0x10, 0x10, True, None),
    0x5aec20: ('??0CarCollisionBody@', 8, 8, True, 'this'),
    0x63a440: ('??1CarFeedback958@', 0, 0, True, None),
    0x63a360: ('??0CarFeedback958@', 4, 4, True, 'this'),
    0x5fcb00: ('??0CarRecorder918@', 4, 4, True, 'this'),
    0x4690e0: ('?CreateCarSound@Audio82B884@', 4, 4, True, 'value'),
    0x63a610: ('?DestroyRender@MoverModeOwner@', 0, 0, True, None),
    0x63a5a0: ('?CreateRender@MoverModeOwner@', 4, 4, True, None),
    0x5f5010: ('?PlaceOnGround@MoverModeOwner@', 0, 0, True, None),
    0x5f46a0: ('?SwitchMover@MoverModeOwner@', 4, 4, True, None),
    0x5eba30: ('?SetParticipantKind@MoverModeOwner@', 4, 4, True, None),
    0x5f6e20: ('?Prepare5F6E20@MoverModeOwner@', 0, 0, True, None),
    0x5f54a0: ('?LoadColliderPacks@MoverModeOwner@', 0, 0, True, None),
    0x42f7a0: ('?CreateEntry42F7A0@WorldSubsystem118@', 4, 4, True, 'entry'),
    STUB_DTOR: (None, 4, 4, True, None),
}
SHARED = {'?carWorld890080@': 0x890080, '?participantRecords89E810@': 0x89e810, '?participantCount89E814@': 0x89e814,
          '?raceValue89E7B8@': 0x89e7b8, '?raceType89E7B0@': 0x89e7b0, '?carTypeInfoTable@': 0x8a1ccc,
          '?humanCarType8021B0@': 0x8021b0, '?humanSelection8900E8@': 0x8900e8,
          '?carCollisionBodyPool86AD60@': 0x86ad60, '?carNullMoverPool86B304@': 0x86b304, '?drivetrainClock@': 0x86518c,
          '?resetDurationDefault@': 0x7a55c4, '?replayActive89CF69@': 0x89cf69, '?replayState8669F4@': 0x8669f4,
          '?replayState866A14@': 0x866a14, '?audioEnabled7FAAA8@': 0x7faaa8, '?audio82B884@': 0x82b884,
          '?physicsCurveSpacing7850B4@': 0x7850b4, '?carTable89CCF8@': 0x89ccf8, '?carMoverTypes89CD50@': 0x89cd50,
          '?carMovers89CDA8@': 0x89cda8, '?carKinds89CE00@': 0x89ce00, '?carParticipants89CE58@': 0x89ce58,
          '?carInputs89CEB0@': 0x89ceb0, '?EmptyRoutine4022C0@': 0x4022c0, '?Destroy@WRoadNav@': 0x4022c0,
          '??3@YAXPAX@Z': 0x575630, '?Slot00@CarNullMover@': STUB_DTOR + 0x40,
          '?collisionIds86B2BC@': 0x86b2bc, '___CxxFrameHandler': 0x75bfd2, '__except_list': 0, '__fltused': 0}
SHARED.update({p: a for a, (p, _, _, _, _) in CALLEES.items() if p})
TARGETS = {  # name: (rebuilt symbol, original address key, convention, stack argument count, returns)
    'SetupParticipants': ('?SetupParticipants@', 'cdecl', 0, False),
    'CreateParticipantCar': ('?CreateParticipantCar@', 'register', 4, False),
    'Construct': ('?Construct@MoverModeOwner@', 'this', 4, True),
    'SetPhysics': ('?SetPhysics@MoverModeOwner@', 'this', 1, False),
    'RebuildCarList': ('?RebuildCarList@CarWorld@', 'this', 0, False),
    'SetupCopy': ('??4CarPhysicsSetup@', 'this', 1, True),
    'CurveCopy': ('?Copy@PhysicsCurveTable@', 'this', 1, False),
    'TuningBind': ('?Bind@PhysicsTuning@', 'this', 3, False),
    'GetRecord': ('?GetRecord@PhysicsParameters@', 'this', 1, True),
    'InfoRefresh': ('?Refresh@PhysicsInfo@', 'this', 4, False),
    'StateConstruct': ('?Construct@DrivetrainAuxiliary@', 'this', 2, True),
    'NextCollisionId': ('?CurrentCollisionId5930A0@', 'cdecl', 0, True),
}
GLOBALS = ((0x890080, 4), (0x89e810, 8), (0x89e7b0, 4), (0x89e7b8, 4), (0x8a1ccc, 4), (0x8021b0, 4), (0x8900e8, 4),
           (0x86ad60, 4), (0x86b304, 4), (0x86518c, 4), (0x89cf69, 1), (0x8669f4, 4), (0x866a14, 4), (0x7faaa8, 4),
           (0x82b884, 4), (0x86b2bc, 4), (0x89ccf8, 0x210), (TIB, 4))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Physics/CarConstruction.cpp')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/car-construction.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = bytearray(pefile.PE(data=raw).get_memory_mapped_image())
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for address, (_, cleanup, _, _, _) in CALLEES.items():
        if address >= 0x1000000: continue
        code = b'\xc2' + struct.pack('<H', cleanup) if cleanup else b'\xc3'
        image[address - 0x400000:address - 0x400000 + len(code)] = code
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7',
                    '/EHsc', '/Fo' + str(obj.relative_to(ROOT)), os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0 or s['storage'] != 2: continue
        match = [SHARED[s['name']]] if s['name'] in SHARED else [a for p, a in SHARED.items() if p.startswith('?') and s['name'].startswith(p)]
        assert len(set(match)) == 1, ('unexpected external', s['name'], match); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _, _) in TARGETS.items()}
    rebuilt_vtable = linked['exports']['??_7CarNullMover@@6B@']
    recovered = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, traces, fixture = [], [[], []], {}

    def stack_tag(uc, value):
        if STACK_BASE <= value < STACK + 0x100: return ('stack', bytes(uc.mem_read(value, 0xc)))
        return value

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        uc.mem_map(ARENA, ARENA_SIZE); uc.mem_map(STACK_BASE, 0x10000); uc.mem_map(STOP, 0x2000)
        uc.mem_write(STUB_DTOR, b'\xc2\x04\x00'); uc.mem_write(STUB_DTOR + 0x40, b'\xc3')
        uc.mem_map(TIB, 0x1000); uc.mem_map(GDT, 0x1000)

        def descriptor(base, limit, access, flags=0xc):
            return ((limit & 0xffff) | (base & 0xffffff) << 16 | access << 40 | (limit >> 16 & 0xf) << 48
                    | flags << 52 | (base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93),
                                          descriptor(TIB, 0xfff, 0x93, 0x4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for register, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16),
                                   (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)):
            uc.reg_write(register, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)

        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in recovered), ('state or call order mismatch', 'original reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))

        def stub(uc, address, size, user, trace=traces[rebuilt]):
            _, _, argc, traced_this, result = CALLEES[address]
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            argv = tuple(stack_tag(uc, v) for v in struct.unpack('<%dI' % (argc // 4), bytes(uc.mem_read(esp + 4, argc))))
            if address == 0x6399d0: argv = (argv[0] & 0xff,) + argv[1:]  # bool: only al is set
            trace.append((hex(address), stack_tag(uc, this) if traced_this else None, argv))
            if result == 'alloc':
                pointer = fixture['heap'][0]; fixture['heap'][0] += (argv[0] + 0x3f) & ~0x3f
                assert fixture['heap'][0] <= HEAP_END; uc.reg_write(xc.UC_X86_REG_EAX, pointer)
            elif result == 'slot':
                pointer = fixture['heap'][0] if fixture['rng'].random() < 0.8 else 0
                if pointer: fixture['heap'][0] += 0x40
                uc.reg_write(xc.UC_X86_REG_EAX, pointer)
            elif result == 'grid':
                uc.mem_write(struct.unpack('<I', bytes(uc.mem_read(esp + 8, 4)))[0], fixture['grid'])
            elif result == 'value':
                uc.reg_write(xc.UC_X86_REG_EAX, fixture['rng'].getrandbits(32) if address != 0x4690e0 else fixture['rng'].choice((0, MISC + 0x100)))
            elif result == 'this':
                uc.reg_write(xc.UC_X86_REG_EAX, this)
            elif result == 'nav':
                uc.mem_write(this + 0x24, struct.pack('<f', fixture['navigatorDistance'])); uc.reg_write(xc.UC_X86_REG_EAX, this)
            elif result == 'entry':
                if fixture['rng'].random() < 0.7:
                    pointer = fixture['heap'][0]; fixture['heap'][0] += 0x40
                    uc.mem_write(pointer + 0x14, struct.pack('<I', RECORDS + RECORD_SIZE * fixture['rng'].randrange(MAX_RECORDS)))
                else: pointer = 0
                uc.reg_write(xc.UC_X86_REG_EAX, pointer)
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in CALLEES: uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)

    rng = random.Random(0x604260); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, globals_, argv, precision, this=0, edx=0):
        _, convention, argc, returns = TARGETS[name]
        outputs = []
        seed = rng.getrandbits(32)
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            fixture['rng'] = random.Random(seed); fixture['heap'] = [HEAP]
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(STACK_BASE, b'\xcd' * 0x10000)
            uc.mem_write(TIB, struct.pack('<I', 0xffffffff))
            for address, data in globals_: uc.mem_write(address, data)
            stack_args = argv if rebuilt or convention != 'register' else argv[2:]
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in stack_args))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            if convention == 'register' and not rebuilt:
                uc.reg_write(xc.UC_X86_REG_ECX, argv[0]); uc.reg_write(xc.UC_X86_REG_EDX, argv[1])
            else:
                uc.reg_write(xc.UC_X86_REG_ECX, this); uc.reg_write(xc.UC_X86_REG_EDX, edx)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=5000000)
            except (UcError, AssertionError) as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            state = bytearray(uc.mem_read(ARENA, ARENA_SIZE)) + b''.join(bytes(uc.mem_read(a, n)) for a, n in GLOBALS)
            if rebuilt:
                for i in range(0, len(state) - 3, 4):
                    if state[i:i + 4] == struct.pack('<I', rebuilt_vtable): state[i:i + 4] = struct.pack('<I', NULL_MOVER_VTABLE)
            outputs.append((uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED),
                            uc.reg_read(xc.UC_X86_REG_EAX) & (0xffff if name == 'NextCollisionId' else 0xffffffff) if returns else None, list(traces[rebuilt]), bytes(state)))
        expected = STACK + 4 + (4 * argc if convention == 'this' else 0)
        assert outputs[0][0] == expected, ('original contract', name, hex(outputs[0][0]), hex(expected))
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][4][i:i + 4].hex(), outputs[1][4][i:i + 4].hex()) for i in range(0, len(outputs[0][4]), 4) if outputs[0][4][i:i + 4] != outputs[1][4][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), outputs[0][:3], outputs[1][:3],
                                  [t[0] for t in outputs[0][3]], [t[0] for t in outputs[1][3]],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][3], outputs[1][3])) if a != b), None), diff))
        counts[name] += 1
        return outputs[0]

    def note(path): paths[path] = paths.get(path, 0) + 1
    def put(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def fl(): return rng.choice((0.0, 1.0, -1.0, -0.0, 16.0, 250.5, float('nan'), rng.uniform(-500, 500)))

    def make_record(memory, address, kind=None):
        memory[address - ARENA] = rng.choice((0, 0, 0xff, 1)); memory[address - ARENA + 1] = rng.randrange(256)
        memory[address - ARENA + 3] = rng.choice((1, 2, 3, 4, 5, 6, 7, 0))
        put(memory, address + 4, '<i', rng.choice((0, 1, 1, 2, 3)) if kind is None else kind)
        put(memory, address + 0x10, '<i', rng.choice((0, 1, 2, 3, -1)))          # setup car type
        put(memory, address + 0x20, '<i', rng.choice((0, 1, 2, 3, -1)))          # parameters car type
        memory[address - ARENA + 0x24] = rng.choice((0xff, 0, 2))               # class index
        put(memory, address + 0x46c + 0x104, '<ff', fl(), fl())                # curve start/end
        memory[address - ARENA + 0x900] = rng.choice((0, 0, 0, 1)); memory[address - ARENA + 0x91c] = rng.choice((0, 1))
        put(memory, address + 0x920, '<fff', fl(), fl(), fl())

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        count = rng.randrange(MAX_RECORDS + 1)
        for i in range(MAX_RECORDS): make_record(memory, RECORDS + i * RECORD_SIZE)
        make_record(memory, SETUP - 0x10)
        for row in range(-1, 4): put(memory, TABLE + row * 0x890 + 0x844, '<i', rng.choice((0, 1)))
        # World list: empty, or a few existing cars each with a participant record.
        cars = [CARS + i * 0xa00 for i in range(rng.randrange(4))]
        chain = [WORLD] + cars + [WORLD]
        for i, node in enumerate(cars):
            put(memory, node, '<II', chain[i + 2], chain[i])
            put(memory, node + 0x14, '<I', RECORDS + RECORD_SIZE * rng.randrange(MAX_RECORDS))
        put(memory, WORLD, '<II', chain[1], chain[-2])
        put(memory, WORLD + 0x118, '<I', MISC)
        put(memory, VT, '<I', STUB_DTOR)
        fixture['grid'] = bytes(rng.randbytes(0xc0))
        fixture['navigatorDistance'] = fl()
        globals_ = [(0x890080, struct.pack('<I', WORLD)), (0x89e810, struct.pack('<Ii', RECORDS, count)),
                    (0x89e7b0, struct.pack('<i', rng.choice((0, 1)))), (0x89e7b8, struct.pack('<f', fl())),
                    (0x8a1ccc, struct.pack('<I', TABLE)), (0x86ad60, struct.pack('<I', MISC + 0x200)),
                    (0x86b304, struct.pack('<I', MISC + 0x300)), (0x86518c, struct.pack('<i', rng.getrandbits(31))),
                    (0x89cf69, bytes([rng.choice((0, 0, 1))])), (0x8669f4, struct.pack('<i', rng.choice((0, 0, 1, -1)))),
                    (0x866a14, struct.pack('<i', rng.choice((0, 0, 1)))), (0x7faaa8, struct.pack('<i', rng.choice((0, 1)))),
                    (0x82b884, struct.pack('<I', MISC + 0x400)), (0x86b2bc, struct.pack('<I', MISC + 0xa00))]
        result = run('SetupParticipants', memory, globals_, [], precision)
        sites = [t[0] for t in result[3]]
        note('created-%d' % sites.count('0x5eba30'))
        if '0x42f7a0' in sites: note('subsystem-entries')
        if '0x5fcb00' in sites: note('recorder')
        if '0x4690e0' in sites: note('car-sound')
        if '0x5f6e20' in sites: note('prepare-5f6e20')
        record = RECORDS + RECORD_SIZE * rng.randrange(MAX_RECORDS)
        source = rng.choice((0, CARS + 0x40))
        run('CreateParticipantCar', memory, globals_, [MISC + 0x500, record, rng.choice((0, 1, 2, 3)), source], precision)
        put(memory, MISC + 0x600, '<fff', fl(), fl(), fl())
        distance = fl(); note('distance-negative' if distance < 0 else 'distance-other')
        run('Construct', memory, globals_, [record, rng.choice((0, MISC + 0x600)), rng.getrandbits(16), struct.pack('<f', distance)], precision, CAR)
        put(memory, CAR + 0x14, '<I', record); put(memory, CAR + 0x28, '<i', rng.choice((0, 1, 2)))
        put(memory, CAR + 0x3c, '<I', rng.choice((0, MISC + 0x700))); put(memory, MISC + 0x700, '<I', VT)
        put(memory, CAR + 0x958, '<I', rng.choice((0, MISC + 0x780))); put(memory, CAR + 0x554, '<I', rng.choice((0, 1)))
        run('SetPhysics', memory, globals_, [rng.choice((SETUP, record + 0x10))], precision, CAR)
        run('RebuildCarList', memory, globals_, [], precision, WORLD)
        run('SetupCopy', memory, globals_, [SETUP], precision, record + 0x10)
        run('CurveCopy', memory, globals_, [SETUP + 0x45c + 0x10], precision, record + 0x46c)
        run('TuningBind', memory, globals_, [rng.getrandbits(32), rng.getrandbits(32), rng.getrandbits(32)], precision, record + 0x384)
        run('GetRecord', memory, globals_, [rng.choice((0, 1, 0x100))], precision, record + 0x20)
        run('InfoRefresh', memory, globals_, [rng.choice((0, 1, 2)), 1, rng.choice((0, 1)), rng.choice((0, 1, 0x100))], precision, record + 0x10)
        result = run('NextCollisionId', memory, globals_, [], precision)
        run('StateConstruct', memory, globals_, [MISC + 0x800, MISC + 0x900], precision, CAR + 0x40)
    report = dict(comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], traced_stubs=sorted(hex(a) for a in CALLEES if a < 0x1000000) + ['collision body deleting destructor slot 0'],
                  limitations=['finite generated values; exhaustive coverage not established',
                               'callees are stubs; 0x5eb5d0, 0x5da2f0, 0x5e0b40, 0x59f880 and 0x5b4fc0 are verified separately',
                               '0x606480 passes its first two arguments in ecx/edx; the rebuilt function uses the stack',
                               'stack pointers passed to stubs are compared by their first 12 bytes',
                               'byte identity not established'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'grid-order': ('for (int index = 1; index <= 6; ++index, ++start) {', 'for (int index = 1; index < 6; ++index, ++start) {'),
            'skip-state': ('if (record->state0 > 0 ||', 'if (record->state0 >= 0 ||'),
            'human-kind': ('raceValue89E7B8 > 0.0f ? 3 : 1', 'raceValue89E7B8 >= 0.0f ? 3 : 1'),
            'flag900-kind': ('CreateParticipantCar(start, record, 0, 0);', 'CreateParticipantCar(start, record, 1, 0);'),
            'kind3-mover': ('        if (!raceType89E7B0) car->Prepare5F6E20();\n', ''),
            'navigator-sign': ('if (distance < 0.0f) Navigator()', 'if (distance <= 0.0f) Navigator()'),
            'kind3-value': ('if (context14->value4 == 3) value910 = 16.0f;', 'if (context14->value4 == 2) value910 = 16.0f;'),
            'reset-default': ('resetDuration978 = resetDurationDefault;', 'resetDuration978 = 0.0f;'),
            'recorder-gate': ('replayState8669F4 > 0 ||', 'replayState8669F4 >= 0 ||'),
            'human-globals': ('if (context14->value4 == 1 && !context14->flag900) {', 'if (context14->value4 == 1) {'),
            'mover-order': ('    SwitchMover(0);\n    SwitchMover(type);', '    SwitchMover(type);\n    SwitchMover(0);'),
            'feedback-row': ('valueC + 0x844) == 0', 'valueC + 0x840) == 0'),
            'list-kind-order': ('if (car->context14->value4 == 2) cars1C', 'if (car->context14->value4 == 3) cars1C'),
            'curve-count': ('for (int i = 0; i < 0x3f; ++i) samples[i]', 'for (int i = 0; i < 0x40; ++i) samples[i]'),
            'tuning-bind': ('parameters10.tuning374.Bind(parameters10.value0,', 'parameters10.tuning374.Bind(source.value0,'),
            'record-compose': ('if (index4 == -1 && compose)', 'if (compose)'),
            'state-flags': ('renderFlags3B4[i] = 1;', 'renderFlags3B4[i] = 0;'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = args.source.parent / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            try:
                result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '512'], cwd=ROOT, text=True,
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
