#!/usr/bin/env python3
"""Execute the original contact impulse 0x5af0e0 (CollisionContact::ApplyImpulse, 2274 bytes)
and the MSVC 7.1 (/Ox /Ob0) rebuild from CollisionResponse.cpp in Unicorn on scripted contacts,
and compare the complete call log plus the resulting memory.

Probe-only fixtures (nothing here is game code):
  * Bodies are fake objects whose vtable slots are tiny x86 stubs (`fld [ecx+X]; ret N`, ...)
    that return values scripted in the body memory.  A code hook at each stub entry logs the
    slot and its arguments by value (impulse and arm vectors for +0x4c/+0x50, the other body for
    +0x70/+0x74) and refreshes the per-call scripted result (velocities per iteration, shares).
    Slots used by 0x5af0e0: +0x08 position, +0x3c velocities, +0x4c impulse share, +0x50 apply,
    +0x5c inverse mass, +0x60 inverse inertia, +0x70 restitution, +0x74 friction; any other slot
    faults.  The stubs never mutate the fake world, so iterations see the scripted velocities.
  * ContactFrame 0x594000, CreateEvent 0x5a9b10 and the event accumulate 0x5a9620 are HOOKED
    identically on both sides (scripted contact point/normal; CreateEvent installs the fake event;
    accumulate just logs point, impulse and sliding speed).  The rebuilt object is linked to those
    original addresses (its own Accumulate entry is hooked to the same handler).  They are not
    executed, so their semantics are NOT covered here.
  * Vector helpers cross 0x43d1e0, scale-add 0x43d0c0 and transform 0x43c9c0 (-> 0x441000) run
    the original code on both sides.  CombineFriction 0x5941c0 runs the original on the original
    side and the rebuilt static helper on the rebuilt side; both call the +0x74 stub, so the call
    log checks the order of the friction queries too.  Restitution, the wall-polygon test and the
    arm flattening are inlined in the original and static helpers in the rebuilt source.
Behavioural evidence on finite scripted cases only; nothing claims byte matching."""
import hashlib
import math
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_ECX, UC_X86_REG_EBX, UC_X86_REG_ESI,
                               UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from probe_pc_race_reward import ROOT, EXE
from pc_coff import link_objects, read_object

BUILD = ROOT/'build/analysis/collision'
SOURCE = 'src/Speed/Indep/Src/Physics/Collision/CollisionResponse.cpp'
ORIGINAL = (0x5af0e0, 0x5af9c2)
SYMBOL = '?ApplyImpulse@CollisionContact@@QAEXXZ'
ACCUMULATE_SYMBOL = '?Accumulate@CollisionEvent@@QAEXPAUWheelVector@@0M@Z'
CONTACT_FRAME, CREATE_EVENT, ACCUMULATE = 0x594000, 0x5a9b10, 0x5a9620
EXTERNALS = {'?WheelTransform@@YAXPAUWheelVector@@PAUWheelMatrix@@0@Z': 0x43c9c0,
             '?WheelScaleAdd@@YAXPAUWheelVector@@00M@Z': 0x43d0c0,
             '?WheelCross@@YAXPAUWheelVector@@00@Z': 0x43d1e0,
             '?CreateEvent@CollisionBodyWitness@@QAEXPAUWheelVector@@@Z': CREATE_EVENT,
             '?ContactFrame@CollisionContact@@QAEXPAUWheelVector@@0@Z': CONTACT_FRAME,
             '?collisionWallNormalLimit@@3MA': 0x784358, '?collisionRestingScale@@3MA': 0x7a18a8,
             '?collisionWallFrictionZ@@3MA': 0x7a18a4, '?collisionFrictionMinimumSpeed@@3MA': 0x7845a0,
             '?collisionFrictionRampSpeed@@3MA': 0x7fbf30, '?collisionUnbreakable@@3MA': 0x7a24fc,
             '?collisionMinimumImpulseMass@@3MA': 0x7fbf24, '__fltused': 0}
UNBREAKABLE, RAMP_SPEED, MINIMUM_SPEED, WALL_LIMIT = -57575.0, 1.0, 1e-5, 0.1   # 0x7a24fc 0x7fbf30 0x7845a0 0x784358
CALLEES = [(0x5941c0, 0x5941c0+89), (0x43d1e0, 0x43d21d), (0x43d0c0, 0x43d0f3), (0x43c9c0, 0x43c9c5), (0x441000, 0x441064)]

CONTACT, WITNESS, OWNER, EVENT = 0x2000000, 0x2001000, 0x2002000, 0x2003000
BODY = {'A': 0x2010000, 'B': 0x2011000}
CAR = {'A': 0x2020000, 'B': 0x2021000}
FACE = {'A': 0x2030000, 'B': 0x2031000}
VTABLE, STUBS, TRAP = 0x2040000, 0x2041000, 0x2050000
STACK, STOP = 0x20ff000, 0x2100000
WORLD_BASE, WORLD_SIZE, DATA_SIZE = 0x2000000, 0x100000, 0x40000   # DATA_SIZE: fake world compared after each run
SLOTS = {0x08: 'position', 0x3c: 'velocities', 0x4c: 'share', 0x50: 'apply', 0x5c: 'mass', 0x60: 'inertia',
         0x70: 'restitution', 0x74: 'friction'}
# body probe layout
RET, VELOCITY, ANGULAR, POSITION, INERTIA = 0x400, 0x600, 0x610, 0x620, 0x640


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def close(x, y):
    return x == y or abs(x-y) <= 1e-4*max(1.0, abs(x), abs(y)) or (x != x and y != y)


def stub_code(slot):
    load = lambda offset: b'\x8b\x81'+struct.pack('<I', offset)       # mov eax,[ecx+disp32]
    if slot == 0x08 or slot == 0x60:
        return load(RET+slot)+b'\xc3'
    if slot == 0x3c:
        code = b'\x8b\x54\x24\x04'
        for k in range(3):
            code += b'\x8b\x81'+struct.pack('<I', VELOCITY+4*k)+b'\x89'+(b'\x02' if k == 0 else bytes([0x42, 4*k]))
        code += b'\x8b\x54\x24\x08'
        for k in range(3):
            code += b'\x8b\x81'+struct.pack('<I', ANGULAR+4*k)+b'\x89'+(b'\x02' if k == 0 else bytes([0x42, 4*k]))
        return code+b'\xc2\x08\x00'
    if slot in (0x4c, 0x5c, 0x70, 0x74):
        return b'\xd9\x81'+struct.pack('<I', RET+slot)+{0x4c: b'\xc2\x08\x00', 0x5c: b'\xc3', 0x70: b'\xc2\x04\x00', 0x74: b'\xc2\x04\x00'}[slot]
    if slot == 0x50:
        return b'\xc2\x08\x00'
    raise ValueError(slot)


# ---------------------------------------------------------------- scenario construction
def unit(v):
    n = math.sqrt(sum(a*a for a in v)) or 1.0
    return [a/n for a in v]


def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]


def add(a, b): return [x+y for x, y in zip(a, b)]
def sub(a, b): return [x-y for x, y in zip(a, b)]
def scale(a, s): return [x*s for x in a]
def dot(a, b): return sum(x*y for x, y in zip(a, b))


def random_inertia(rng):
    m = [0.0]*16
    basis = [[rng.uniform(-1, 1) for _ in range(3)] for _ in range(3)]
    weights = [rng.uniform(0.0, 2.0) for _ in range(3)]
    for i in range(3):
        for j in range(3):
            m[i*4+j] = sum(weights[k]*basis[k][i]*basis[k][j] for k in range(3))
    m[15] = 1.0
    return m


def make_case(rng):
    kinds = rng.choice([(0, 3), (3, 0), (0, 3), (0, 0), (0, 0), (0, 1), (1, 0), (1, 3), (0, 3), (3, 0)])
    wall = rng.random() < 0.4
    case = dict(kinds=kinds)
    case['normal'] = unit([rng.gauss(0, 1) for _ in range(3)] if rng.random() < 0.5 else [rng.gauss(0, 0.3), rng.gauss(0, 0.3), 1.0])
    case['point'] = [rng.uniform(-30, 30) for _ in range(3)]
    case['bodies'] = {}
    for label, kind in zip('AB', kinds):
        position = add(case['point'], [rng.uniform(-2.5, 2.5) for _ in range(3)])
        if kind == 3:
            normal_z = rng.uniform(-0.09, 0.09) if wall else rng.choice((rng.uniform(-1, 1), 1.0, -1.0, 0.1, -0.1, 0.0999999))
            case['bodies'][label] = dict(kind=kind, position=position, mass=0.0, inertia=[0.0]*16, face_z=normal_z,
                                         restitution=rng.choice((0.0, 0.2, 0.5, 0.9)), friction=rng.choice((0.0, 0.3, 0.8, 1.2)),
                                         wall_contacts=0)
        else:
            case['bodies'][label] = dict(kind=kind, position=position, mass=rng.choice((rng.uniform(0.0004, 0.002), rng.uniform(0.0004, 0.002), 0.0)),
                                         inertia=random_inertia(rng) if rng.random() < 0.9 else [0.0]*16,
                                         restitution=rng.choice((0.0, 0.1, 0.3, 0.5, 0.9)), friction=rng.choice((0.0, 0.3, 0.8, 1.2, 0.5)),
                                         wall_contacts=rng.choice((0, 0, 1, 3)), mode=rng.choice((4, 4, 0, 1, 3)),
                                         up=unit([rng.gauss(0, 0.3), rng.gauss(0, 0.3), 1.0]) if rng.random() < 0.7 else unit([rng.gauss(0, 1) for _ in range(3)]))
    if case['bodies']['A']['mass'] == 0.0 and case['bodies']['B']['mass'] == 0.0:
        case['bodies'][rng.choice('AB')]['mass'] = rng.uniform(0.0004, 0.002)
    if rng.random() < 0.15:   # equal values exercise the tie in the restitution / friction selection
        case['bodies']['B']['restitution'] = case['bodies']['A']['restitution']
        case['bodies']['B']['friction'] = case['bodies']['A']['friction']
    case['preexisting_event'] = rng.random() < 0.5
    case['event_value10'] = rng.choice((0, 1, 1))
    case['owner'] = 'witness' if rng.random() < 0.85 else 'swapped'
    case['control'] = rng.choice((0x027f, 0x037f))
    # iterations
    iterations = rng.choice((1, 1, 1, 2, 3, 4))
    ending = rng.choice(('separating', 'share1', 'single', 'unbreakable_a', 'unbreakable_b', 'separating', 'share1', 'single_b'))
    case['iterations'], case['ending'] = iterations, ending
    case['velocities'] = {'A': [], 'B': []}
    case['shares'] = {'A': [], 'B': []}
    for step in range(iterations):
        last = step == iterations-1
        desired_approach = -rng.choice((0.05, 0.5, 2.0, 8.0, 30.0))
        if last and ending == 'separating' and iterations > 1 or (last and iterations == 1 and rng.random() < 0.2):
            desired_approach = rng.choice((0.0, 0.0, 0.5, 4.0))
        sliding = rng.choice((0.0, 1e-7, 1e-5, 2e-5, 0.3, 0.9999, 1.0, 1.0001, 3.0, 25.0))
        tangent = unit(cross(case['normal'], [rng.gauss(0, 1) for _ in range(3)]))
        arms = {l: sub(case['point'], case['bodies'][l]['position']) for l in 'AB'}
        angular = {l: ([rng.gauss(0, 1) for _ in range(3)] if rng.random() < 0.7 else [0.0]*3) for l in 'AB'}
        velocity_a = [rng.gauss(0, 6) for _ in range(3)]
        relative = add(scale(case['normal'], desired_approach), scale(tangent, sliding))
        point_a = add(cross(angular['A'], arms['A']), velocity_a)
        velocity_b = sub(add(point_a, relative), cross(angular['B'], arms['B']))
        if desired_approach == 0.0:   # exactly zero approach speed: equal velocities, no spin (no rounding knife edge)
            angular = {'A': [0.0]*3, 'B': [0.0]*3}
            velocity_b = list(velocity_a)
        case['velocities']['A'].append((velocity_a, angular['A']))
        case['velocities']['B'].append((velocity_b, angular['B']))
        # shares (Slot4C): iterations continue only when both are positive and the smaller is not 1
        if not last:
            shares = (rng.choice((0.5, 0.25, 0.9, 0.7)), rng.choice((0.5, 1.0, 0.8, 0.3)))
            if min(shares) == 1.0:
                shares = (0.5, 1.0)
        else:
            shares = {'separating': (rng.choice((0.5, 1.0)), rng.choice((0.5, 1.0))), 'share1': (1.0, rng.choice((1.0, 2.0))),
                      'single': (-rng.choice((0.5, 1.0, 2.0)), rng.choice((0.5, 1.0))), 'single_b': (rng.choice((0.5, 1.0)), -rng.choice((0.5, 1.0, 3.0))),
                      'unbreakable_a': (UNBREAKABLE, 0.5), 'unbreakable_b': (0.5, UNBREAKABLE)}[ending]
            if ending == 'share1' and shares[0] != shares[1]:
                shares = (rng.choice(((1.0, 1.0), (1.0, 2.0), (2.0, 1.0)))[0], 1.0) if rng.random() < 0.5 else (2.0, 1.0)
        case['shares']['A'].append(shares[0])
        case['shares']['B'].append(shares[1])
    # any further call: separating velocities and share 1.0
    case['velocities']['A'].append(([0.0]*3, [0.0]*3))
    case['velocities']['B'].append((scale(case['normal'], 1.0), [0.0]*3))
    case['shares']['A'].append(1.0)
    case['shares']['B'].append(1.0)
    if rng.random() < 0.4:   # some corner cases: negative shares in the middle
        case['shares']['A'][0], case['shares']['B'][0] = rng.choice(((-0.5, 0.7), (0.5, -2.0), (-1.0, -1.0), (0.0, 0.5), (0.5, 0.0)))
    return case


def classify(case):
    tags = [f'kinds{case["kinds"]}', f'iter{case["iterations"]}', case['ending']]
    if any(b.get('face_z') is not None and abs(b['face_z']) < WALL_LIMIT for b in case['bodies'].values()):
        tags.append('wall')
    if case['event_value10'] and any(b['kind'] == 0 and b['mode'] == 4 for b in case['bodies'].values()):
        tags.append('flatten')
    return tags


# ---------------------------------------------------------------- world image
def write_world(uc, case):
    uc.mem_write(WORLD_BASE, bytes(DATA_SIZE))
    for label in 'AB':
        body = case['bodies'][label]
        base = BODY[label]
        uc.mem_write(base, struct.pack('<I', VTABLE))
        uc.mem_write(base+0x11, bytes([body['kind']]))
        uc.mem_write(base+0x120, struct.pack('<i', body['wall_contacts']))
        uc.mem_write(base+POSITION, struct.pack('<3f', *body['position']))
        uc.mem_write(base+INERTIA, struct.pack('<16f', *body['inertia']))
        uc.mem_write(base+RET+0x08, struct.pack('<I', base+POSITION))
        uc.mem_write(base+RET+0x60, struct.pack('<I', base+INERTIA))
        uc.mem_write(base+RET+0x5c, struct.pack('<f', body['mass']))
        uc.mem_write(base+RET+0x70, struct.pack('<f', body['restitution']))
        uc.mem_write(base+RET+0x74, struct.pack('<f', body['friction']))
        if body['kind'] == 0:
            uc.mem_write(base+0x130, struct.pack('<I', CAR[label]))
            uc.mem_write(CAR[label]+0x424, struct.pack('<i', body['mode']))
            uc.mem_write(CAR[label]+0xb0, struct.pack('<3f', *body['up']))
        if body['kind'] == 3:
            uc.mem_write(base+0x60, struct.pack('<I', FACE[label]))
            uc.mem_write(FACE[label]+0x78, struct.pack('<f', body['face_z']))
    uc.mem_write(CONTACT+0xc, struct.pack('<III', BODY['A'], BODY['B'], WITNESS))
    for witness, (first, second) in ((WITNESS, ('A', 'B')), (OWNER, ('B', 'A'))):
        uc.mem_write(witness+0x238, struct.pack('<II', BODY[first], BODY[second]))
    uc.mem_write(WITNESS+0x244, struct.pack('<I', EVENT if case['preexisting_event'] else 0))
    uc.mem_write(EVENT+0x10, struct.pack('<i', case['event_value10']))
    uc.mem_write(EVENT+0x64, struct.pack('<I', WITNESS if case['owner'] == 'witness' else OWNER))


def main():
    raw = EXE.read_bytes()
    pe = pefile.PE(data=raw)
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD/'CollisionResponse.obj'
    subprocess.run([str(ROOT/'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7',
                    '/EHsc', '/Fo'+str(obj.relative_to(ROOT)), SOURCE], cwd=ROOT, check=True, capture_output=True)
    undefined = {s['name'] for s in read_object(obj)['symbols'].values() if s['section'] == 0 and s['storage'] == 2}
    externals = dict(EXTERNALS)
    unused = sorted(undefined-set(externals))
    for i, name in enumerate(unused):   # CollisionEffects, collisionClock: only reachable from the hooked Accumulate
        externals[name] = 0x7ff00000+i*16
    linked = link_objects([obj], externals)
    sizes = {s['name']: s['size'] for s in read_object(obj)['symbols'].values() if s['size']}
    record = dict(original=hex(ORIGINAL[0]), original_size=ORIGINAL[1]-ORIGINAL[0],
                  original_sha256=hashlib.sha256(image[ORIGINAL[0]-base:ORIGINAL[1]-base]).hexdigest(),
                  rebuilt_size=sizes[SYMBOL], byte_matching='not claimed', unlinked_externals=unused)

    state = dict(log=[], counters={}, case=None)

    def vector(machine, pointer):
        return struct.unpack('<3f', machine.mem_read(pointer, 12))

    def label_of(address):
        return 'A' if address == BODY['A'] else 'B' if address == BODY['B'] else hex(address)

    def stub_entry(machine, slot):
        sp = machine.reg_read(UC_X86_REG_ESP)
        ecx = machine.reg_read(UC_X86_REG_ECX)
        label = label_of(ecx)
        words = struct.unpack('<2I', machine.mem_read(sp+4, 8))
        key = (label, slot)
        n = state['counters'][key] = state['counters'].get(key, -1)+1
        case = state['case']
        entry = [SLOTS[slot], label]
        if slot == 0x3c:
            seq = case['velocities'][label]
            velocity, angular = seq[min(n, len(seq)-1)]
            machine.mem_write(ecx+VELOCITY, struct.pack('<3f', *velocity))
            machine.mem_write(ecx+ANGULAR, struct.pack('<3f', *angular))
            entry += [struct.unpack('<3f', struct.pack('<3f', *velocity)), struct.unpack('<3f', struct.pack('<3f', *angular))]
        elif slot == 0x4c:
            seq = case['shares'][label]
            share = seq[min(n, len(seq)-1)]
            machine.mem_write(ecx+RET+0x4c, struct.pack('<f', share))
            entry += [vector(machine, words[0]), vector(machine, words[1]), f32(share)]
        elif slot == 0x50:
            entry += [vector(machine, words[0]), vector(machine, words[1])]
        elif slot in (0x70, 0x74):
            entry += [label_of(words[0])]
        state['log'].append(tuple(entry))

    def ret_thiscall(machine, argument_bytes):
        sp = machine.reg_read(UC_X86_REG_ESP)
        machine.reg_write(UC_X86_REG_EIP, struct.unpack('<I', machine.mem_read(sp, 4))[0])
        machine.reg_write(UC_X86_REG_ESP, sp+4+argument_bytes)

    def contact_frame(machine):
        sp = machine.reg_read(UC_X86_REG_ESP)
        point_pointer, normal_pointer = struct.unpack('<2I', machine.mem_read(sp+4, 8))
        machine.mem_write(point_pointer, struct.pack('<3f', *state['case']['point']))
        machine.mem_write(normal_pointer, struct.pack('<3f', *state['case']['normal']))
        state['log'].append(('ContactFrame', machine.reg_read(UC_X86_REG_ECX) == CONTACT))
        ret_thiscall(machine, 8)

    def create_event(machine):
        sp = machine.reg_read(UC_X86_REG_ESP)
        witness = machine.reg_read(UC_X86_REG_ECX)
        point_pointer = struct.unpack('<I', machine.mem_read(sp+4, 4))[0]
        machine.mem_write(witness+0x244, struct.pack('<I', EVENT))
        state['log'].append(('CreateEvent', witness == WITNESS, vector(machine, point_pointer)))
        ret_thiscall(machine, 4)

    def accumulate(machine):
        sp = machine.reg_read(UC_X86_REG_ESP)
        point_pointer, impulse_pointer, speed = struct.unpack('<2If', machine.mem_read(sp+4, 12))
        state['log'].append(('Accumulate', machine.reg_read(UC_X86_REG_ECX) == EVENT, vector(machine, point_pointer),
                             vector(machine, impulse_pointer), speed))
        ret_thiscall(machine, 12)

    def make_machine(rebuilt):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(base, (len(image)+4095) & ~4095)
        uc.mem_write(base, image)
        uc.mem_map(WORLD_BASE, WORLD_SIZE)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
        for slot in range(0, 0x90, 4):
            uc.mem_write(VTABLE+slot, struct.pack('<I', STUBS+slot*0x80 if slot in SLOTS else TRAP))
        for slot in SLOTS:
            uc.mem_write(STUBS+slot*0x80, stub_code(slot))
        entry = linked['exports'][SYMBOL] if rebuilt else ORIGINAL[0]
        hooks = {CONTACT_FRAME: contact_frame, CREATE_EVENT: create_event,
                 linked['exports'][ACCUMULATE_SYMBOL] if rebuilt else ACCUMULATE: accumulate}
        ranges = list(CALLEES)+[(STUBS, STUBS+0x4000)]+[(address, address+1) for address in hooks]
        ranges += linked['ranges'] if rebuilt else [ORIGINAL]

        def guard(machine, address, size, user):
            handler = hooks.get(address)
            if handler:
                handler(machine)
                return
            if STUBS <= address < STUBS+0x4000 and (address-STUBS) % 0x80 == 0:
                stub_entry(machine, (address-STUBS)//0x80)
                return
            if not any(start <= address < end for start, end in ranges):
                raise RuntimeError(f'Unexpected instruction at {address:#x}')
        uc.hook_add(UC_HOOK_CODE, guard)
        return uc, entry
    machines = [make_machine(False), make_machine(True)]

    def run(uc, entry, case):
        write_world(uc, case)
        state.update(log=[], counters={}, case=case)
        uc.mem_write(STACK, struct.pack('<I', STOP))
        uc.reg_write(UC_X86_REG_ESP, STACK)
        uc.reg_write(UC_X86_REG_ECX, CONTACT)
        uc.reg_write(UC_X86_REG_FPSW, 0)
        uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
        uc.reg_write(UC_X86_REG_FPCW, case['control'])
        saved = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
        for reg in saved:
            uc.reg_write(reg, 0x12345678)
        try:
            uc.emu_start(entry, STOP, count=200000)
        except UcError as error:
            return ('fault', f'{error} at eip {uc.reg_read(UC_X86_REG_EIP):#x}', state['log'])
        if uc.reg_read(UC_X86_REG_EIP) != STOP:
            return ('fault', 'instruction budget', state['log'])
        assert all(uc.reg_read(reg) == 0x12345678 for reg in saved)
        assert uc.reg_read(UC_X86_REG_ESP) == STACK+4
        assert (uc.reg_read(UC_X86_REG_FPSW) >> 11) & 7 == 0, 'x87 stack not balanced'
        return ('ok', list(state['log']), bytes(uc.mem_read(WORLD_BASE, DATA_SIZE)))

    def deviation(x, y):
        """Largest relative difference of a log entry; vectors are scaled by their largest component,
        because a component that is a cancellation residual carries the absolute rounding error of the
        whole vector."""
        if isinstance(x, float):
            return 0.0 if x == y or (x != x and y != y) else abs(x-y)/max(1.0, abs(x), abs(y))
        if isinstance(x, tuple):
            if len(x) != len(y):
                return math.inf
            if x and all(isinstance(v, float) for v in x):
                scale = max([1.0]+[abs(v) for v in x]+[abs(v) for v in y])
                return max(0.0 if v == w or (v != v and w != w) else abs(v-w)/scale for v, w in zip(x, y))
            return max([deviation(v, w) for v, w in zip(x, y)]+[0.0])
        return 0.0 if x == y else math.inf

    rng = random.Random(0x5af0e0)
    trials = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
    stats = dict(cases=0, exact_log=0, bit_identical=0, early_return=0, failures=0, max_log_error=0.0)
    tag_counts, length_counts, failures = {}, {}, []
    slot_counts = {}
    for trial in range(trials):
        case = make_case(rng)
        results = [run(uc, entry, case) for uc, entry in machines]
        stats['cases'] += 1
        for tag in classify(case):
            tag_counts[tag] = tag_counts.get(tag, 0)+1
        problem = None
        if results[0][0] != 'ok' or results[1][0] != 'ok':
            problem = ('fault', [(r[0], r[1]) if r[0] == 'fault' else 'ok' for r in results])
        else:
            first, second = results[0][1], results[1][1]
            if len(first) != len(second):
                problem = ('log length', len(first), len(second), first, second)
            else:
                for i, (x, y) in enumerate(zip(first, second)):
                    error = deviation(x, y)
                    if error > 1e-4:
                        problem = ('log entry', i, x, y)
                        break
                    stats['max_log_error'] = max(stats['max_log_error'], error)
            if problem is None and results[0][2] != results[1][2]:
                problem = ('memory',)
            if problem is None:
                if first == second:
                    stats['exact_log'] += 1
                    stats['bit_identical'] += results[0][2] == results[1][2]
                length_counts[len(first)] = length_counts.get(len(first), 0)+1
                if not any(e[0] == 'share' for e in first):
                    stats['early_return'] += 1
                for e in first:
                    slot_counts[e[0]] = slot_counts.get(e[0], 0)+1
        if problem:
            stats['failures'] += 1
            failures.append((trial, classify(case), hex(case['control']), problem))
    print({'function': record, **stats, 'calls_by_kind': dict(sorted(slot_counts.items())),
           'log_length_histogram': dict(sorted(length_counts.items()))})
    print('scenario tags', dict(sorted(tag_counts.items())))
    for failure in failures[:25]:
        print(failure)
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
