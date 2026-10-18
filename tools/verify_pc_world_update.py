#!/usr/bin/env python3
"""Compare world update 0x609ce0, nearest-position query 0x5f1160 and draft sum 0x5ebd80
under 53- and 64-bit x87 precision.

bDistance 0x43ce10 and the list helper 0x502fd0 execute original code in both runs.
Every other callee is a traced stub patched at its original address, including the
recovered body advance 0x5f0f00, collision step 0x5ffa50 and the car pre/post updates
0x5fbbe0/0x604740 (of which only kinematics prefixes are recovered): this verifies the
world composition, not those callees. The recovered 0x5f0f00/0x5ffa50 are free cdecl
functions, so their rebuilt calls reach separate stubs whose traces are normalised to the
original thiscall form. Driver slot 0xc and input slot 4 are fixture vtable stubs."""

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

BUILD = ROOT / 'build/analysis/world-update'
ARENA, ARENA_SIZE, STACK, STOP, STUBS = 0x2000000, 0x8000, 0x2010000, 0x2100000, 0x2101000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
WORLD, CARS, PLAYERS, FLOW, RACE, SUB118, NODES, VT = (ARENA + x for x in (0x0, 0x1000, 0x6000, 0x6800, 0x6900, 0x6a00, 0x6b00, 0x7000))
CAR_SIZE, MAX_CARS = 0xa00, 8
TARGETS = {'Update': ('?Update@CarWorld@', 1, False), 'NearestPlayerDistance': ('?NearestPlayerDistance@CarWorld@', 1, True),
           'GetDraftSum': ('?GetDraftSum@MoverModeOwner@', 0, True)}
# Original address: (symbol prefix, stub kind, stack cleanup, trace ecx)
CALLEES = {0x5fa770: ('?Prepare5FA770@CarWorld@', 'void', 0, True), 0x5fc0e0: ('?UpdateCars5FC0E0@', 'void', 0, False),
           0x5f6cd0: ('?Update5F6CD0@MoverModeOwner@', 'void', 0, True), 0x4268a0: ('?Update4268A0@WorldObject81CBC0@', 'void', 0, True),
           0x614820: ('?Update614820@', 'void', 0, False), 0x42f910: ('?Update42F910@WorldSubsystem118@', 'void', 4, True),
           0x5ebc20: ('?GetDraft5EBC20@MoverModeOwner@', 'float', 4, True), 0x59e350: ('?Update59E350@', 'void', 0, False),
           0x5fbbe0: ('?SnapshotFrameKinematics@MoverModeOwner@', 'void', 4, True),
           0x604740: ('?UpdateFrameAcceleration@MoverModeOwner@', 'void', 4, True),
           0x5f5580: ('?Update5F5580@MoverModeOwner@', 'void', 0, True), 0x572e50: ('?UpdateRace572E50@', 'void-cdecl2', 0, False),
           0x5f6350: ('?Update5F6350@MoverModeOwner@', 'void', 0, True), 0x492e10: ('?Update492E10@WorldObject8357E0@', 'void', 4, True),
           0x5f5730: ('?UpdateEffects5F5730@MoverModeOwner@', 'void', 4, True), 0x605780: ('?Update605780@', 'void', 0, False),
           0x5e0a00: ('?Update5E0A00@', 'void', 0, False), 0x5e5430: ('?Update5E5430@', 'void', 0, False),
           0x43e650: ('?Sort43E650@CarWorld@', 'void', 4, True), 0x602650: ('?Update602650@PlayerRecord@', 'void', 0, True),
           0x43af20: ('?Update43AF20@WorldObject827B88@', 'void', 4, True), 0x638070: ('?Update638070@', 'void', 0, False),
           0x5e86e0: ('?Update5E86E0@WorldObject88EB00@', 'void', 0, True), 0x6076d0: ('?Update6076D0@RaceStatus890118@', 'void', 0, True),
           0x5f0f00: (None, 'void', 4, False), 0x5ffa50: (None, 'void', 4, True)}
ALT_ADVANCE, ALT_COLLIDE = STUBS + 0x100, STUBS + 0x140   # rebuilt cdecl forms of 0x5f0f00 / 0x5ffa50
SLOT_DRIVER, SLOT_INPUT = STUBS + 0x00, STUBS + 0x40
ARG_BYTES = {0x42f910: 4, 0x5ebc20: 4, 0x5fbbe0: 4, 0x604740: 4, 0x572e50: 8, 0x492e10: 4, 0x5f5730: 4, 0x43e650: 4,
             0x43af20: 4, 0x5f0f00: 4, 0x5ffa50: 4, SLOT_DRIVER: 4, SLOT_INPUT: 4, ALT_ADVANCE: 4, ALT_COLLIDE: 8}
SHARED = {'?carWorld890080@': 0x890080, '?engineSubstepDuration@': 0x7fc800, '?gameMode89008C@': 0x89008c,
          '?playerCount8900BC@': 0x8900bc, '?players8900B4@': 0x8900b4, '?gameFlow8900D8@': 0x8900d8,
          '?replayActive89CF69@': 0x89cf69, '?replayState8669F4@': 0x8669f4, '?replayState866A14@': 0x866a14,
          '?worldNearDistance7A548C@': 0x7a548c, '?worldEffectsDistance7A5490@': 0x7a5490, '?carDraftLimit7A55BC@': 0x7a55bc,
          '?worldObject81CBC0@': 0x81cbc0, '?worldObject8357E0@': 0x8357e0, '?worldObject827B88@': 0x827b88,
          '?worldObject88EB00@': 0x88eb00, '?raceStatus890118@': 0x890118, '?bDistance@': 0x43ce10,
          '?CompareWorldEntry5EA240@': 0x5ea240,
          '?AdvanceCollisionBodies@': ALT_ADVANCE, '?UpdateCollisionWorld@': ALT_COLLIDE, '__fltused': 0}
SHARED.update({p: a for a, (p, _, _, _) in CALLEES.items() if p})
RESULT_FLOAT = STUBS + 0x200
GLOBALS = ((0x7fc800, 4), (0x890080, 4), (0x89008c, 4), (0x8900b4, 8), (0x8900bc, 4), (0x8900d8, 4), (0x890118, 4),
           (0x89cf69, 1), (0x8669f4, 4), (0x866a14, 4))


def f32(x): return ctypes.c_float(x).value


def stub_code(kind, cleanup):
    ret = b'\xc2' + struct.pack('<H', cleanup) if cleanup else b'\xc3'
    if kind == 'float': return b'\xd9\x05' + struct.pack('<I', RESULT_FLOAT) + ret
    return ret


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Physics/WorldUpdate.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/world-update.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = bytearray(pefile.PE(data=raw).get_memory_mapped_image())
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for address, (_, kind, cleanup, _) in CALLEES.items():
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
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    recovered = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    names = {ALT_ADVANCE: 0x5f0f00, ALT_COLLIDE: 0x5ffa50}
    machines, traces, fixture = [], [[], []], {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x2000)
        uc.mem_write(SLOT_DRIVER, stub_code('void', 4)); uc.mem_write(SLOT_INPUT, stub_code('void', 4))
        uc.mem_write(ALT_ADVANCE, stub_code('void', 0)); uc.mem_write(ALT_COLLIDE, stub_code('void', 0))
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in recovered), ('state or call order mismatch', 'original reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            argv = bytes(uc.mem_read(esp + 4, ARG_BYTES.get(address, 0)))
            if address == ALT_COLLIDE: this, argv = struct.unpack('<I', argv[:4])[0], argv[4:]
            name = names.get(address, address)
            if address == 0x572e50 and fixture['flow']: uc.mem_write(FLOW, struct.pack('<i', 5))  # the original re-reads the flow state
            traced_this = this if address in (SLOT_DRIVER, SLOT_INPUT, ALT_COLLIDE) or (name in CALLEES and CALLEES[name][3]) else None
            trace.append((hex(name), traced_this, argv))
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in list(CALLEES) + [SLOT_DRIVER, SLOT_INPUT, ALT_ADVANCE, ALT_COLLIDE]:
            uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x609ce0); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, globals_, argv, precision, this):
        _, argc, floating = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for address, data in globals_: uc.mem_write(address, data)
            uc.mem_write(RESULT_FLOAT, struct.pack('<f', fixture['float']))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=5000000)
            except (UcError, AssertionError) as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            top = (uc.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7
            st0 = uc.reg_read(xc.UC_X86_REG_FP0 + top) if floating else None
            state = bytes(uc.mem_read(ARENA, ARENA_SIZE)) + b''.join(bytes(uc.mem_read(a, n)) for a, n in GLOBALS)
            outputs.append((uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), top, st0, list(traces[rebuilt]), state))
        assert outputs[0][0] == STACK + 4 + 4 * argc, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][5][i:i + 4].hex(), outputs[1][5][i:i + 4].hex()) for i in range(0, len(outputs[0][5]), 4) if outputs[0][5][i:i + 4] != outputs[1][5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), outputs[0][:4], outputs[1][:4],
                                  [t[0] for t in outputs[0][4]], [t[0] for t in outputs[1][4]],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][4], outputs[1][4])) if a != b), None), diff))
        counts[name] += 1
        return outputs[0]

    def note(path): paths[path] = paths.get(path, 0) + 1
    def put(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def fl(): return f32(rng.choice((0.0, 1.0, -1.0, 50.0, 99.0, 100.0, 149.0, 150.0, 200.0, float('nan'), rng.uniform(-300, 300))))

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        count10 = rng.choice((0, 1, 1, 2, 3, 4, 6, 8))
        count18 = rng.randint(0, count10); count14 = rng.randint(0, min(count18, 6))
        if rng.random() < 0.1: count14 = rng.randint(0, min(count10, 6)); count18 = rng.randint(count14, count10)
        put(memory, WORLD + 0x10, '<iii', count10, count14, count18)
        cars = [CARS + i * CAR_SIZE for i in range(MAX_CARS)]
        for i in range(0x19): put(memory, WORLD + 0x1c + i * 4, '<I', cars[i] if i < MAX_CARS else 0)
        put(memory, WORLD + 0x118, '<I', rng.choice((0, SUB118)))
        # World list at +0: a circular list (head is the sentinel) with a few nodes.
        nodes = [NODES + i * 0x10 for i in range(rng.randrange(0, 5))]
        chain = nodes + [WORLD]
        put(memory, WORLD, '<I', chain[0])
        for i, n in enumerate(nodes):
            put(memory, n, '<I', chain[i + 1]); put(memory, n + 0xa, '<h', rng.choice((0, 3, -1)))
        put(memory, VT + 0xc, '<I', SLOT_DRIVER); put(memory, VT + 0x104, '<I', SLOT_INPUT)
        for car in cars:
            put(memory, car + 0x14, '<II', ARENA + 0x6c00, rng.choice((0, RACE)))
            put(memory, car + 0x24, '<i', rng.choice((0, 1, 2, 3)))
            put(memory, car + 0x2c, '<II', ARENA + 0x6d00, ARENA + 0x6e00)
            for off in (0x60, 0x64, 0x68, 0x8fc, 0x4dc): put(memory, car + off, '<f', fl())
            put(memory, car + 0x970, '<i', rng.choice((0, 1, 5, -1)))
            memory[car + 0x550 - ARENA] = rng.choice((0, 1, 1)); memory[car + 0x998 - ARENA] = rng.choice((0, 0, 1))
            put(memory, car + 0x5cc, '<i', rng.choice((0, 1, 1)))
        put(memory, ARENA + 0x6c00 + 4, '<i', rng.choice((0, 1, 2, 3, 3)))
        put(memory, ARENA + 0x6d00, '<I', VT); put(memory, ARENA + 0x6e00, '<I', VT + 0x100)
        put(memory, RACE + 0x10, '<i', rng.randrange(4))
        for i in range(2):
            put(memory, PLAYERS + i * 0x250 + 4, '<I', cars[i]); put(memory, PLAYERS + i * 0x250 + 0x230, '<i', 7)
        put(memory, FLOW, '<i', rng.choice((0, 5, 6, 5)))
        globals_ = [(0x7fc800, struct.pack('<f', fl())), (0x890080, struct.pack('<I', WORLD)),
                    (0x89008c, struct.pack('<i', rng.choice((0, 0, 1, 2, 3)))), (0x8900b4, struct.pack('<II', PLAYERS, PLAYERS + 0x250)),
                    (0x8900bc, struct.pack('<i', rng.choice((0, 1, 2)))), (0x8900d8, struct.pack('<I', FLOW)),
                    (0x890118, struct.pack('<I', rng.choice((0, RACE)))), (0x89cf69, bytes([rng.choice((0, 0, 1))])),
                    (0x8669f4, struct.pack('<i', rng.choice((0, 0, 1, -1)))), (0x866a14, struct.pack('<i', rng.choice((0, 0, 1))))]
        fixture.update(flow=rng.random() < 0.5, float=f32(rng.choice((0.0, 0.1, 0.3, 0.5, 0.9, 1.0, float('nan'), rng.uniform(-1, 1)))))
        result = run('Update', memory, globals_, [struct.pack('<f', fl())], precision, WORLD)
        sites = {t[0] for t in result[4]}
        for site, label in (('0x5f5580', 'near-update'), ('0x5f5730', 'effects'), ('0x572e50', 'race-entry'), ('0x5f6350', 'leading-5f6350'),
                            ('0x42f910', 'subsystem118'), ('0x6076d0', 'race-status'), ('0x5ebc20', 'draft')):
            if site in sites: note(label)
        if result[5][0x6000 + 0x230:0x6000 + 0x234] != bytes(memory[0x6000 + 0x230:0x6000 + 0x234]): note('mode-change')
        point = ARENA + 0x6f00; put(memory, point, '<3f', fl(), fl(), fl())
        for i in range(6): put(memory, WORLD + 0x80 + i * 0x10, '<3f', fl(), fl(), fl())
        run('NearestPlayerDistance', memory, globals_, [point], precision, WORLD); note('nearest-%d' % min(count14, 5))
        run('GetDraftSum', memory, globals_, [], precision, cars[rng.randrange(MAX_CARS)])
    report = dict(comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=['bDistance 0x43ce10', 'list end 0x502fd0'],
                  traced_stubs=sorted(hex(a) for a in CALLEES) + ['driver slot 0xc', 'input slot 4'],
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'callees are stubs: 0x5f0f00/0x5ffa50 and the kinematics prefixes are verified separately',
                               'the recovered 0x5f0f00/0x5ffa50 are cdecl free functions; the original passes the world in ecx',
                               'byte identity not established'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'positions-fill': ('for (; i < 6; ++i) {', 'for (; i < 5; ++i) {'),
            'mode-value': ('if (mode == 1) player->value230 = 2;', 'if (mode == 1) player->value230 = 4;'),
            'driver-order': ('    for (i = 0; i < count18 - count14; ++i) {', '    for (i = 0; i < count18 - count14 - 1; ++i) {'),
            'countdown': ('if (car->countdown970) --car->countdown970;', '--car->countdown970;'),
            'collide-before-advance': ('    AdvanceCollisionBodies(duration);\n', ''),
            'near-limit': ('< worldNearDistance7A548C) car->Update5F5580();', '< worldEffectsDistance7A5490) car->Update5F5580();'),
            'flow-five': ('if (gameFlow8900D8->state0 != 5) car->Update5F6350();', 'car->Update5F6350();'),
            'effects-mode': ('car->context14->value4 != 3 &&', 'car->context14->value4 != 2 &&'),
            'nearest-players': ('count14 > 2) {', 'count14 > 1) {'),
            'nearest-five': ('if (count14 < 5) {', 'if (count14 < 6) {'),
            'draft-self': ('if (other != this) {', 'if (other) {'),
            'draft-limit': ('if (sum >= carDraftLimit7A55BC) return carDraftLimit7A55BC;\n', ''),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = args.source.parent / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            try:
                result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '1024'], cwd=ROOT, text=True,
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
