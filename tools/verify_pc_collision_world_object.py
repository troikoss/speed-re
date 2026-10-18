#!/usr/bin/env python3
"""Execute the original world-object chain of UG2 PC v1.2 and the MSVC 7.1 (/Ox /Ob0) rebuild of
CollisionWorldObject.cpp in Unicorn on random fake worlds, and compare the complete seam call log, EAX,
the callee-saved registers, the stack pointer, the returned x87 value and the whole fake-world memory.

Routines verified (original address, rebuilt symbol): PromoteWorldObject 0x5b31f0, the promotion body
0x5b1390, BreakWorldObject 0x5ae000, the child attach 0x5adb20, the child update 0x5add50, the child
activation 0x5adf90, the registration 0x48bc50, the float RNG 0x487360, the placement 0x40f010 with its
recursive mark 0x6147e0, the entry numbering 0x5a8b70 with its list helper 0x502fd0, the shape bounds
0x592a10 and the placement seed accessor 0x611000.  They call each other as real machine code on both
sides (the original ones in the image, the rebuilt ones in the linked object), so the chain
Break -> create -> attach -> activate -> update/register and Promote -> promotion body -> attach is
exercised end to end.

Also real machine code (original bytes, mapped at the original address on both sides): the matrix copy
0x4012b0, the transform 0x43c9c0 (-> 0x441000), the name hash 0x43db80 and the CRT _ftol 0x75bed4 (the
rebuild calls __ftol2; it is bound to the same original routine).  They are verified elsewhere.

Seams, replaced identically on both sides by stubs that record their arguments (pointers to stack data
are logged by content, since the two frames differ) and return scripted values: the slot-pool test
0x43cd10, the collider constructor/destructor 0x5e0b40/0x5e0bf0, the pack update 0x5e68d0 (the stub
stores the scripted pack at collider +8, as the real one does), the area query 0x5e6fa0, the ground
query 0x5e7e70 (the stub loads the scripted height onto the x87 stack and writes the scripted
normal and found flag), the mover pose 0x59b9b0, the bounds refresh 0x5a9de0, the creation function
behind the pointer at 0x86af90, and the body's virtual slots 0 (deleting destructor), 8 (position),
0xc (velocity), 0x8c (world-object kind; a free function on the rebuilt side) and the slot 4 of the
object at +0x130.  Their own behaviour is therefore NOT covered.

Known, deliberate differences that this check cannot cover: the originals contain an SEH frame around the
local collider and 16-byte stack alignment, the rebuild does not; MSVC chooses other x87 instruction
sequences (e.g. FUCOMPP for FLD/FCOMP in 0x48bc50); the vectors copied in 0x5adb20 move y and z through
the x87 stack in the original, so signalling-NaN payloads in those inputs would be quietened there; the
random vectors therefore contain no signalling NaNs.  Behavioural evidence only; nothing here claims byte
matching.  --mutation-checks recompiles deliberately broken sources and requires each to be rejected."""
import argparse
import hashlib
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_GDTR, UC_X86_REG_CS, UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_FS,
                               UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EBX,
                               UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from probe_pc_race_reward import ROOT, EXE
from pc_coff import link_objects, read_object

BUILD = ROOT/'build/analysis/collision-world-object'
SOURCE = ROOT/'src/Speed/Indep/Src/Physics/Collision/CollisionWorldObject.cpp'
ROUTINES = {'Promote': (0x5b31f0, 0x5b3210), 'PromoteBody': (0x5b1390, 0x5b153c), 'Break': (0x5ae000, 0x5ae0e0),
            'Attach': (0x5adb20, 0x5add47), 'Update': (0x5add50, 0x5ade02), 'Activate': (0x5adf90, 0x5adfff),
            'Register': (0x48bc50, 0x48bd6c), 'Random': (0x487360, 0x48739f), 'Place': (0x40f010, 0x40f064),
            'Mark': (0x6147e0, 0x614810), 'Find': (0x5a8b70, 0x5a8bce), 'End': (0x502fd0, 0x502fd3),
            'Bounds': (0x592a10, 0x592a53), 'Seed': (0x611000, 0x611007)}
SYMBOLS = {'Promote': '?PromoteWorldObject@@YAPAVCollisionBody@@PAV1@@Z',
           'PromoteBody': '?PromoteWorldObject5B1390@@YAPAVCollisionBody@@PAV1@@Z',
           'Break': '?BreakWorldObject@@YAHPAVCollisionBody@@PAPAV1@@Z',
           'Attach': '?WorldObjectAttach5ADB20@@YAXPAVCollisionBody@@PAXPAUWheelVector@@2@Z',
           'Update': '?WorldObjectUpdate5ADD50@@YAXPAVCollisionBody@@@Z',
           'Activate': '?WorldObjectActivate5ADF90@@YAXPAVCollisionBody@@@Z',
           'Register': '?Register48BC50@WorldObject8357E0@@QAEXPAUWorldRegistrationNode@@@Z',
           'Random': '?RandomFloat487360@@YAMMPAI@Z',
           'Place': '?Place40F010@WorldObjectPlacement@@QAEXPAUWheelMatrix@@@Z',
           'Mark': '?Mark6147E0@WorldPlacementNode@@QAEXXZ',
           'Find': '?FindWorldObjectIndex5A8B70@@YAHPAX@Z',
           'End': '?End@WorldObjectBlock@@QAEPAU1@XZ',
           'Bounds': '?GetShapeBounds@DynamicCollisionBody@@QAEXPAUWheelVector@@0@Z',
           'Seed': '?GetSeed611000@WorldObjectPlacement@@QAEIXZ'}
# Routines returning void (or the x87 value): EAX is not part of their result.
VOID = {'Attach', 'Update', 'Activate', 'Register', 'Random', 'Place', 'Mark', 'Bounds'}
# Original routines run as real code on both sides (address ranges): name hash, matrix copy, transform thunk and body, _ftol.
REAL_CALLEES = [(0x43db80, 0x43dba1), (0x4012b0, 0x40133b), (0x43c9c0, 0x43c9c5), (0x441000, 0x441064), (0x75bed4, 0x75bf49)]

# Fake-world layout (BODY .. BODY+REGION is compared in full).
BODY, DESC, SOURCE_OBJ, PARENT, IDS, OUT, VTABLE = 0x2000000, 0x2000800, 0x2000900, 0x2000a00, 0x2000b00, 0x2000c00, 0x2000d00
CHILD, NODES = 0x2001000, 0x2006000
BLOCK0, BLOCK1, MISS = 0x2007400, 0x2007800, 0x2007c00
REG, REGDESC, EXIST = 0x2007d00, 0x2008b00, 0x2009200
MOVER, VT130, POS, VEL, PTREE, MATS = 0x2009300, 0x2009700, 0x2009740, 0x2009750, 0x2009800, 0x2009e00
XFORM, VECA, VECB = 0x200ac00, 0x200ac80, 0x200ad00
REGION = 0xb000
# Argument slots read by the probe wrappers (OUT+0x80..) and scripted seam values (OUT+0xc0..).
ARG_PROMOTE, ARG_REG, ARG_MATRIX, ARG_VECA, ARG_VECB, ARG_MARK, ARG_FIND, ARG_SCALE, ARG_STATE, ARG_CHILD = (OUT+0x80+4*i for i in range(10))
RANDOM_STATE, GROUND, RESULT, BOUNDS_MIN, BOUNDS_MAX = OUT+0x40, OUT+0xc0, OUT+0xd0, OUT+0xe0, OUT+0xf0
WORLD, STACK, STOP, WRAPPER = 0x8357e0, 0x200f000, 0x2100000, 0x2101000
GLOBAL_COUNTER, GLOBAL_CLOCK, GLOBAL_IDS, GLOBAL_CREATE = 0x86b378, 0x86518c, 0x86b2bc, 0x86af90
GLOBAL_POOL, GLOBAL_FLAG, GLOBAL_RANDOM, GLOBAL_BLOCKS, GLOBAL_BITMAP = 0x86b374, 0x8900f8, 0x7f3940, 0x86b4c0, 0x86ad78
# Watched image memory, compared after every run: counter, RNG state, bitmap, the world object.
WATCH = [(GLOBAL_COUNTER, 4), (GLOBAL_RANDOM, 4), (0x86ad70, 0x100), (WORLD, 0x60)]
SEAM_BASE = 0x2104000
TEB, GDT = 0x2106000, 0x2107000   # 0x5b1390 installs an SEH frame through fs:[0]
S_CREATE, S_DELETE, S_GETPOS, S_GETVEL, S_KIND_V, S_KIND_C, S_SLOT4 = (SEAM_BASE+0x10*i for i in range(7))
FLD_GROUND = b'\xd9\x05'+struct.pack('<I', GROUND)
# address: (name, stub code, argument spec).  Spec letters: c = ECX, i = stack int (stack addresses become
# 'stk'), v = pointed 3 floats, m = pointed 16 floats.  The same address is patched into the image on both
# sides: the originals call it, and the rebuild's externals are bound to it.
SEAMS = {0x43cd10: ('full', b'\xc3', 'i'),
         0x5e0b40: ('collider_new', b'\xc3', 'c'), 0x5e0bf0: ('collider_delete', b'\xc3', 'c'),
         0x5e68d0: ('update_pack', b'\xc2\x08\x00', 'cvi'), 0x5e6fa0: ('query_area', b'\xc2\x0c\x00', 'cviv'),
         0x5e7e70: ('ground', FLD_GROUND+b'\xc2\x10\x00', 'cviii'),
         0x59b9b0: ('set_pose', b'\xc2\x14\x00', 'cvmvvi'), 0x5a9de0: ('refresh', b'\xc3', 'c'),
         S_CREATE: ('create', b'\xc3', 'iii'), S_DELETE: ('delete', b'\xc2\x04\x00', 'ci'),
         S_GETPOS: ('position', b'\xc3', 'c'), S_GETVEL: ('velocity', b'\xc3', 'c'), S_KIND_V: ('kind', b'\xc3', 'c'),
         S_KIND_C: ('kind', b'\xc3', 'i'), S_SLOT4: ('slot4', b'\xc3', 'c')}
PREFIXES = {'?HudNameHash@@YAIPBDI@Z': 0x43db80, '?BodyMatrixCopy@@YAXPAUWheelMatrix@@0@Z': 0x4012b0,
            '?WheelTransform@@YAXPAUWheelVector@@PAUWheelMatrix@@0@Z': 0x43c9c0, '__ftol2': 0x75bed4,
            '?RefreshBounds@DynamicCollisionBody@@QAEXXZ': 0x5a9de0,
            '?SetPose@MoverBody@@QAEXPAUWheelVector@@PAUWheelMatrix@@001@Z': 0x59b9b0,
            '?UpdatePack@WCollider@@QAEXPAMM@Z': 0x5e68d0, '?QueryArea@WCollider@@QAEXPAMM0@Z': 0x5e6fa0,
            '?QueryGround@WCollider@@QAEMPAUWheelVector@@PAH0PAD@Z': 0x5e7e70,
            '?Construct@WCollider@@QAEXXZ': 0x5e0b40, '?Destruct@WCollider@@QAEXXZ': 0x5e0bf0,
            '?WorldObjectKind@@YAHPAVCollisionBody@@@Z': S_KIND_C, '?bIsSlotPoolFull@@YAHPAX@Z': 0x43cd10,
            '?worldObjectCreate86AF90@@': 0x86af90, '?worldObject8357E0@@': 0x8357e0,
            '?worldObjectActivations86B378@@': 0x86b378, '?collisionClock@@': 0x86518c,
            '?worldObjectChildName7A1EEC@@': 0x7a1eec, '?collisionIds86B2BC@@': 0x86b2bc,
            '?worldObjectPool86B374@@': 0x86b374, '?worldObjectBitmap86AD78@@': 0x86ad78,
            '?worldObjectUpdate8900F8@@': 0x8900f8, '?worldObjectRandomState7F3940@@': 0x7f3940,
            '?worldObjectBlocks86B4C0@@': 0x86b4c0, '?worldObjectZero7D7D3C@@': 0x7d7d3c,
            '?worldObjectSettle7844F4@@': 0x7844f4, '?worldObjectSpeedLimit7859D0@@': 0x7859d0,
            '?worldObjectRandomScale78D864@@': 0x78d864, '?worldObjectRandomRange786D84@@': 0x786d84, '__fltused': 0}


def build(source, name):
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD/(name+'.obj')
    command = [str(ROOT/'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc',
               '/I', 'src/Speed/Indep/Src/Physics/Collision', '/Fo'+str(obj.relative_to(ROOT)), str(Path(source).resolve().relative_to(ROOT))]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        print(result.stdout, result.stderr)
        raise SystemExit('compile failed')
    symbols = read_object(obj)['symbols'].values()
    externals = {}
    for symbol in symbols:
        if symbol['section'] == 0 and symbol['storage'] == 2:
            for prefix, address in PREFIXES.items():
                if symbol['name'] == prefix or (prefix.endswith('@@') and symbol['name'].startswith(prefix)):
                    externals[symbol['name']] = address
                    break
            else:
                raise SystemExit('unmapped external '+symbol['name'])
    linked = link_objects([obj], externals)
    sizes = {s['name']: s['size'] for s in symbols if s['size']}
    return linked, sizes


def imm(value):
    return struct.pack('<I', value)


def push_mem(address):
    return b'\xff\x35'+imm(address)


def mov_ecx_mem(address):
    return b'\x8b\x0d'+imm(address)


def wrapper_parts(name, rebuilt):
    """Probe-only wrapper around the call: (prefix, suffix).  The originals are thiscall (ECX = object, the
    callee pops its stack arguments); the rebuild takes the object as the first cdecl argument for the free
    functions (hand-off of the former thiscall routines) and is thiscall for the real members."""
    add_esp = lambda n: b'\x83\xc4'+bytes([n])
    if name == 'Promote':   # cdecl on both sides; the argument comes from a probe slot (may be 0)
        return push_mem(ARG_PROMOTE), add_esp(4)
    if name == 'PromoteBody':
        return (b'\x68'+imm(BODY), add_esp(4)) if rebuilt else (b'\xb9'+imm(BODY), b'')
    if name == 'Break':
        if rebuilt:
            return b'\x68'+imm(OUT)+b'\x68'+imm(BODY), add_esp(8)
        return b'\x68'+imm(OUT)+b'\xb9'+imm(BODY), b''
    if name == 'Attach':
        args = push_mem(ARG_VECB)+push_mem(ARG_VECA)+b'\x68'+imm(XFORM)
        if rebuilt:
            return args+push_mem(ARG_CHILD), add_esp(16)
        return args+mov_ecx_mem(ARG_CHILD), b''
    if name in ('Update', 'Activate'):
        return (push_mem(ARG_CHILD), add_esp(4)) if rebuilt else (mov_ecx_mem(ARG_CHILD), b'')
    if name == 'Register':
        return push_mem(ARG_REG)+b'\xb9'+imm(WORLD), b''
    if name == 'Random':    # cdecl (float scale, state), float result in ST0 stored to RESULT
        return push_mem(ARG_STATE)+push_mem(ARG_SCALE), add_esp(8)+b'\xd9\x1d'+imm(RESULT)
    if name == 'Place':
        return push_mem(ARG_MATRIX)+mov_ecx_mem(ARG_CHILD)+b'\x81\xc1'+imm(0x480), b''
    if name == 'Mark':
        return mov_ecx_mem(ARG_MARK), b''
    if name == 'Find':
        return push_mem(ARG_FIND), add_esp(4)
    if name == 'End':
        return mov_ecx_mem(ARG_CHILD), b''
    if name == 'Bounds':
        return b'\x68'+imm(BOUNDS_MAX)+b'\x68'+imm(BOUNDS_MIN)+mov_ecx_mem(ARG_CHILD), b''
    return b'\xb9'+imm(BODY+0x480), b''    # Seed


def make_machine(image, base, linked, rebuilt, entries, coverage, log):
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, (len(image)+4095) & ~4095)
    uc.mem_write(base, image)
    uc.mem_map(0x2000000, 0x10000)
    uc.mem_map(0x2100000, 0x8000)

    def descriptor(address, limit, access, flags=0xc):
        return ((limit & 0xffff) | ((address & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48)
                | (flags << 52) | ((address >> 24) << 56))
    uc.mem_write(GDT+8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
    uc.reg_write(UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
    for reg, selector in ((UC_X86_REG_CS, 8), (UC_X86_REG_SS, 16), (UC_X86_REG_DS, 16), (UC_X86_REG_ES, 16), (UC_X86_REG_FS, 24)):
        uc.reg_write(reg, selector)
    if rebuilt:
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
    for address, (name, code, spec) in SEAMS.items():
        uc.mem_write(address, code)
    wrappers, cursor = {}, WRAPPER
    for name in ROUTINES:
        prefix, tail = wrapper_parts(name, rebuilt)
        call = b'\xe8'+struct.pack('<i', entries[name]-(cursor+len(prefix))-5)
        code = prefix+call+tail+b'\xc3'
        uc.mem_write(cursor, code)
        wrappers[name] = (cursor, len(code))
        cursor += 0x80
    ranges = REAL_CALLEES+[(a, a+len(c)) for a, (n, c, s) in SEAMS.items()]+[(a, a+n) for a, n in wrappers.values()]
    ranges += linked['ranges'] if rebuilt else list(ROUTINES.values())
    state = dict(creates=0, case=None)

    def stack_arg(machine, index):
        esp = machine.reg_read(UC_X86_REG_ESP)
        return struct.unpack('<I', bytes(machine.mem_read(esp+4+4*index, 4)))[0]

    def describe(machine, value, letter):
        if letter == 'i':
            return 'stk' if STACK-0x4000 <= value <= STACK else value
        count = 3 if letter == 'v' else 16
        return struct.unpack('<%dI' % count, bytes(machine.mem_read(value, 4*count)))

    def guard(machine, address, size, user):
        if coverage is not None:
            coverage.add(address)
        if not any(start <= address < end for start, end in ranges):
            raise RuntimeError(f'Unexpected instruction at {address:#x}')
        if address in SEAMS:
            name, code, spec = SEAMS[address]
            case = state['case']
            ecx = machine.reg_read(UC_X86_REG_ECX)
            entry, index = [], 0
            for letter in spec:
                if letter == 'c':
                    entry.append('stk' if STACK-0x4000 <= ecx <= STACK else ecx)
                else:
                    entry.append(describe(machine, stack_arg(machine, index), letter))
                    index += 1
            log.append((name,)+tuple(entry))
            result = 0x7e57ab1e
            if name == 'full':
                result = case['full']
            elif name == 'kind':
                result = case['kind']
            elif name == 'create':
                result = CHILD+0x800*state['creates']
                state['creates'] += 1
            elif name == 'position':
                result = POS
            elif name == 'velocity':
                result = VEL
            elif name == 'update_pack':
                machine.mem_write(ecx+8, imm(case['pack']))
            elif name == 'ground':
                machine.mem_write(stack_arg(machine, 2), case['normal'])
                machine.mem_write(stack_arg(machine, 3), bytes([case['found']]))
            machine.reg_write(UC_X86_REG_EAX, result)
    uc.hook_add(UC_HOOK_CODE, guard)
    return uc, {n: a for n, (a, c) in wrappers.items()}, state


def setup(uc, case):
    uc.mem_write(BODY, case['memory'])
    for address, data in case['globals']:
        uc.mem_write(address, data)
    uc.mem_write(TEB, imm(0xffffffff))


def snapshot(uc):
    return (bytes(uc.mem_read(BODY, REGION)),)+tuple(bytes(uc.mem_read(a, n)) for a, n in WATCH+[(TEB, 4)])


def run(uc, wrapper, state, log, case):
    setup(uc, case)
    del log[:]
    state['case'] = case
    state['creates'] = 0
    uc.mem_write(STACK, struct.pack('<I', STOP))
    uc.reg_write(UC_X86_REG_ESP, STACK)
    saved = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
    for reg in saved:
        uc.reg_write(reg, 0x12345678)
    try:
        uc.emu_start(wrapper, STOP, count=400000)
    except UcError as error:
        return ('fault', str(error)+' eip '+hex(uc.reg_read(UC_X86_REG_EIP)))
    if uc.reg_read(UC_X86_REG_EIP) != STOP:
        return ('fault', 'instruction budget')
    assert all(uc.reg_read(reg) == 0x12345678 for reg in saved), 'callee-saved register clobbered'
    assert uc.reg_read(UC_X86_REG_ESP) == STACK+4, 'stack not balanced'
    return ('ok', snapshot(uc), tuple(log), uc.reg_read(UC_X86_REG_EAX))


def f32(rng, special=0.05):
    """A float: mostly moderate, sometimes zero, infinite or a quiet NaN."""
    if rng.random() < special:
        return rng.choice((0.0, -0.0, float('inf'), float('-inf'), float('nan'), 1e30, -1e30, 3.4e38, 1e-30))
    return rng.uniform(-60, 60)


def make_case(rng):
    mem = bytearray(rng.randbytes(REGION))

    def put(address, fmt, *values):
        struct.pack_into('<'+fmt, mem, address-BODY, *values)

    def floats(address, values):
        for i, value in enumerate(values):
            try:
                put(address+4*i, 'f', value)
            except OverflowError:
                put(address+4*i, 'f', float('inf') if value > 0 else float('-inf'))

    def random_floats(address, count, special=0.05):
        floats(address, [f32(rng, special) for _ in range(count)])

    count = rng.choice((9, 9, 1, 2, 3, 4, 5, 6, 7, 8, 0, -1, -5))
    next8 = rng.choice((0, 1, 0x1234, 0xffff, 0x10000, 0x12345, -1, rng.randrange(1 << 32)))
    seed = rng.choice((0xffffffff, 0, rng.randrange(1 << 32)))
    # Virtual tables: slot 0 destructor, 8 position, 0xc velocity, 0x8c kind; the object at +0x130 has slot 4.
    put(VTABLE, 'I', S_DELETE)
    put(VTABLE+8, 'I', S_GETPOS)
    put(VTABLE+0xc, 'I', S_GETVEL)
    put(VTABLE+0x8c, 'I', S_KIND_V)
    put(VT130+4, 'I', S_SLOT4)
    # The body: descriptor, placement, parent.
    put(BODY, 'I', VTABLE)
    put(BODY+0x148, 'I', DESC)
    put(DESC+0x8c, 'i', count)
    put(BODY+0x480+8, 'I', SOURCE_OBJ)
    put(SOURCE_OBJ+8, 'I', seed)
    put(BODY+0x480+0x60, 'I', PARENT if rng.random() < 0.5 else 0)
    put(IDS+8, 'I', next8 & 0xffffffff)
    random_floats(DESC+0xb8, 1)
    # World-object entry blocks (list head at 0x86b4c0) and the body's entry (+0x5c).
    nblocks = rng.choice((0, 1, 1, 2))
    blocks = [BLOCK0, BLOCK1][:nblocks]
    random_floats(BLOCK0, 0x400//4, 0.01)
    random_floats(BLOCK1, 0x400//4, 0.01)
    random_floats(MISS, 0x40, 0.01)
    block_info = []
    for i, block in enumerate(blocks):
        first = rng.choice((0, 0, 1, 7, 8, 9, 100, 1500, -3, -8))
        number = rng.choice((0, 1, 2, 3, 4, -1))
        put(block, 'I', blocks[i+1] if i+1 < nblocks else GLOBAL_BLOCKS)
        put(block+0xa, 'hh', first, number)
        block_info.append((block, first, number))
    entry = MISS+rng.choice((0, 8, 0x30))
    if block_info and rng.random() < 0.85:
        block, first, number = rng.choice(block_info)
        entry = block+0x10+0x70*rng.randrange(-1, max(number, 0)+2)+rng.choice((0, 0, 0, 0x6f, -0x20, 0x38))
    put(BODY+0x5c, 'I', entry)
    # A promotion whose lowering is exactly zero: the equality edge of the "> 0" test.
    edge = rng.random() < 0.08
    if edge:
        put(entry+0x68, 'f', 0.0)
        put(DESC+0xb8, 'f', 0.0)
    # Placement tree (parent chain of the children's placements): six nodes, node 0 is the root.
    parent_of = {}
    for i in range(1, 6):
        if rng.random() < 0.8:
            parent_of[i] = rng.randrange(i)
    for i in range(6):
        node = PTREE+0x100*i
        kids = [k for k in parent_of if parent_of[k] == i]
        put(node+8, 'I', PTREE+0x100*kids[0] if kids else node+8)
        for position, k in enumerate(kids):
            put(PTREE+0x100*k, 'I', PTREE+0x100*kids[position+1] if position+1 < len(kids) else node+8)
        put(node+0xe4, 'I', rng.choice((0, 1, 0, 2)))
        random_floats(node+0x50, 16, 0.02)
    # Registration nodes (27) and their descriptors; the children's +0x4fc lists point at them.
    for i in range(27):
        node, descriptor = REG+0x80*i, REGDESC+0x40*i
        put(node+0x10, 'i', rng.choice((0, 1, 2, 3, 4, -1, 1, 0)))
        put(node+0x14, 'f', f32(rng))
        put(node+0x1c, 'I', descriptor if rng.random() < 0.75 else 0)
        put(node+0x24, 'I', 0 if rng.random() < 0.5 else rng.randrange(1, 1 << 32))
        floats(descriptor+0x24, [rng.choice((0.0, -0.0, f32(rng, 0.1), f32(rng, 0.1))) if i % 2 else f32(rng, 0.1),
                                 f32(rng, 0.1), rng.choice((0.0, -0.0, f32(rng, 0.1), f32(rng, 0.1))), f32(rng, 0.1)])
    # Children: each with a mover, its object at +0x130, a descriptor, two node lists and a placement.
    for k in range(9):
        c = CHILD+0x800*k
        put(c, 'I', VTABLE)
        put(c+0x68, 'I', MOVER)
        put(c+0x130, 'I', VT130)
        put(c+0x148, 'I', DESC)
        put(c+0x480+0x60, 'I', rng.choice((0, 0, PTREE)) if rng.random() < 0.6 else PTREE+0x100*rng.randrange(6))
        random_floats(c+0x1c, 1, 0.1)
        random_floats(c+0xe0, 3)
        random_floats(c+0xf0, 3)
        random_floats(c+0x168, 1, 0.1)
        mem[c-BODY+0x505] = rng.choice((0, 1))
        mem[c-BODY+0x506] = rng.choice((0, 1, 2))
        mem[c-BODY+0x19] = rng.choice((0, 1, 0x80))
        put(c+0x15c, 'I', rng.choice((0, 0x7fffffff, 1, 0x7ffffffe, 0x80000000, rng.randrange(1 << 32))))
        for list_offset, node_base, target in ((0x4fc, NODES+0x200*k, REG), (0x4f4, NODES+0x200*k+0x100, MATS)):
            nodes = rng.choice((0, 0, 1, 2, 3))
            head = c+list_offset
            chain = [node_base+0x20*j for j in range(nodes)]
            put(head, 'II', chain[0] if chain else head, rng.randrange(1 << 32))
            for j, node in enumerate(chain):
                put(node, 'II', chain[j+1] if j+1 < len(chain) else head, rng.randrange(1 << 32))
                put(node+8, 'I', target+0x80*(3*k+j))
    if edge:
        put(CHILD+0xe8, 'f', 0.0)
    # Mover, positions and the transform for the attach.
    random_floats(MOVER+0x1f0, 3)
    random_floats(POS, 3)
    magnitude = rng.choice((0.0, 5.0, 100.0, 119.0, 121.0, 200.0, 1e30, float('inf'), float('nan')))
    direction = [rng.uniform(-1, 1) for _ in range(3)]
    norm = sum(d*d for d in direction) ** 0.5 or 1.0
    floats(VEL, [magnitude*d/norm for d in direction] if magnitude == magnitude and magnitude != float('inf') else [magnitude, 0.0, 0.0])
    random_floats(XFORM, 16, 0.02)
    random_floats(VECA, 3, 0.1)
    random_floats(VECB, 3, 0.1)
    # World list at 0x8357e0: empty or one existing node.
    existing = rng.random() < 0.5
    world = bytearray(rng.randbytes(0x60))
    sentinel = WORLD+8
    struct.pack_into('<II', world, 8, EXIST if existing else sentinel, EXIST if existing else sentinel)
    struct.pack_into('<i', world, 0x50, rng.choice((0, 1, 5, -1, rng.randrange(1 << 31))))
    put(EXIST, 'II', sentinel, sentinel)
    # Probe arguments and scripted seam values.
    child = rng.randrange(9)
    reg_index = rng.randrange(27)
    put(ARG_PROMOTE, 'I', rng.choice((BODY,) * 9 + (0,)))
    put(ARG_REG, 'I', REG+0x80*reg_index)
    put(ARG_MATRIX, 'I', rng.choice((XFORM, XFORM, 0)))
    put(ARG_VECA, 'I', rng.choice((VECA, 0)))
    put(ARG_VECB, 'I', rng.choice((VECB, 0)))
    put(ARG_MARK, 'I', PTREE+0x100*rng.randrange(6))
    put(ARG_FIND, 'I', entry)
    put(ARG_SCALE, 'f', rng.choice((0.0, 1.0, 2147483648.0, rng.uniform(-100, 100), float('nan'))))
    put(ARG_STATE, 'I', RANDOM_STATE)
    put(RANDOM_STATE, 'I', rng.choice((0, 1, 0x7fffffff, 0x80000000, rng.randrange(1 << 32))))
    put(ARG_CHILD, 'I', CHILD+0x800*child)
    floats(GROUND, [-0.05 if edge else f32(rng, 0.05)])
    normal = struct.pack('<3f', *[rng.uniform(-1, 1) for _ in range(3)])
    clock = rng.randrange(1 << 32)
    bitmap = rng.randbytes(0x100)
    globals_ = [(GLOBAL_COUNTER, imm(rng.choice((0, rng.randrange(1 << 32), 0x7fffffff)))), (GLOBAL_CLOCK, imm(clock)),
                (GLOBAL_IDS, imm(IDS)), (GLOBAL_CREATE, imm(S_CREATE)), (GLOBAL_POOL, imm(rng.randrange(1 << 32))),
                (GLOBAL_FLAG, imm(rng.choice((0, 1, 1, rng.randrange(1, 1 << 32))))),
                (GLOBAL_RANDOM, imm(rng.choice((0, 1, rng.randrange(1 << 32), 0x80000000)))),
                (GLOBAL_BLOCKS, imm(blocks[0] if blocks else GLOBAL_BLOCKS)),
                (0x86ad70, bitmap), (WORLD, bytes(world))]
    return dict(memory=bytes(mem), globals=globals_, count=count, next8=next8, full=0 if edge else rng.choice((0, 0, 0, 1)),
                kind=0 if edge else rng.choice((0, 0, 0, 1, 2, 0xffffffff)),
                pack=0xdeadbeef if edge else rng.choice((0, 0xdeadbeef, 0xdeadbeef)),
                found=1 if edge else rng.choice((0, 1, 1, 1, 2)), normal=normal, promote_arg=struct.unpack_from('<I', mem, ARG_PROMOTE-BODY)[0],
                child=child, reg_index=reg_index)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=3000)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    pe = pefile.PE(data=EXE.read_bytes())
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    linked, sizes = build(args.source, 'CollisionWorldObject')
    covered, logs = set(), ([], [])
    entries_rebuilt = {name: linked['exports'][symbol] for name, symbol in SYMBOLS.items()}
    original, wrap_original, state_original = make_machine(image, base, linked, False, {n: r[0] for n, r in ROUTINES.items()}, covered, logs[0])
    rebuilt, wrap_rebuilt, state_rebuilt = make_machine(image, base, linked, True, entries_rebuilt, None, logs[1])
    rng = random.Random(0x5ae000)
    failures, counts, both_fault = [], {name: 0 for name in ROUTINES}, 0
    seen = dict(creates=set(), deletes=0, registers=0, skipped_loops=0, full=0, kinds=set(), grounds=0, poses=0,
                queries=0, packs=0, hits=0, misses=0, sentinel_returns=0)
    for trial in range(args.cases):
        case = make_case(rng)
        for name in ROUTINES:
            if name == 'Break' and trial % 5 == 4:
                continue
            a = run(original, wrap_original[name], state_original, logs[0], case)
            b = run(rebuilt, wrap_rebuilt[name], state_rebuilt, logs[1], case)
            if name in VOID:
                a, b = [r[:3]+(0,) if r[0] == 'ok' else r for r in (a, b)]
            if a[0] == 'fault' and b[0] == 'fault':
                both_fault += 1
                continue
            if a != b:
                if a[0] == 'ok' and b[0] == 'ok':
                    detail = ('log' if a[2] != b[2] else 'eax' if a[3] != b[3] else 'memory')
                    if detail == 'memory':
                        detail += ' ' + str([i for i, (x, y) in enumerate(zip(a[1], b[1])) if x != y][:1])
                        if a[1][0] != b[1][0]:
                            offset = next(i for i, (x, y) in enumerate(zip(a[1][0], b[1][0])) if x != y)
                            detail += ' first diff at %#x' % (BODY+offset)
                    failures.append((name, trial, detail, a[2][:6] if detail == 'log' else '', b[2][:6] if detail == 'log' else '',
                                     hex(a[3]), hex(b[3])))
                else:
                    failures.append((name, trial, a if a[0] == 'fault' else (a[0], a[2], hex(a[3])),
                                     b if b[0] == 'fault' else (b[0], b[2], hex(b[3]))))
                continue
            counts[name] += 1
            log = a[2]
            if name == 'Break':
                seen['creates'].add(max(0, case['count']))
                seen['deletes'] += sum(1 for e in log if e[0] == 'delete')
                seen['registers'] += 0
                if case['count'] <= 0:
                    seen['skipped_loops'] += 1
                if a[3] != case['count'] & 0xffffffff:
                    failures.append((name, trial, 'returned count differs from the descriptor', a[3]))
                if not any(e[0] == 'delete' for e in log):
                    failures.append((name, trial, 'no deleting destructor call'))
            if name in ('Promote', 'PromoteBody'):
                full = any(e[0] == 'full' and e[1] for e in log) if name == 'PromoteBody' else None
                if name == 'Promote':
                    if (case['promote_arg'] != 0) != any(e[0] == 'delete' for e in log):
                        failures.append((name, trial, 'delete guard'))
                else:
                    if any(e[0] == 'full' for e in log) and case['full']:
                        seen['full'] += 1
                        if a[3] != 0 or len(log) != 1:
                            failures.append((name, trial, 'full pool does not return 0 at once', a[3]))
                    elif a[3] != CHILD:
                        failures.append((name, trial, 'does not return the created body', hex(a[3])))
                    for e in log:
                        if e[0] == 'kind':
                            seen['kinds'].add(case['kind'])
                        elif e[0] == 'ground':
                            seen['grounds'] += 1
                        elif e[0] == 'set_pose':
                            seen['poses'] += 1
                        elif e[0] == 'update_pack' and case['pack']:
                            seen['packs'] += 1
            if name == 'Update':
                seen['queries'] += sum(1 for e in log if e[0] == 'query_area')
            if name == 'Find':
                if a[3] == 0xffffffff:
                    seen['misses'] += 1
                else:
                    seen['hits'] += 1
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for name, (lo, hi) in ROUTINES.items():
        for insn in md.disasm(image[lo-base:hi-base], lo):
            instructions[insn.address] = insn.mnemonic
    unreached = sorted(a for a in instructions if a not in covered)
    record = {name: dict(original_sha256=hashlib.sha256(image[lo-base:hi-base]).hexdigest(), original_size=hi-lo,
                         rebuilt_size=sizes[SYMBOLS[name]], byte_matching='not claimed')
              for name, (lo, hi) in ROUTINES.items()}
    report = dict(compared=counts, cases=sum(counts.values()), both_fault=both_fault, failures=len(failures),
                  break_counts_seen=sorted(seen['creates']), break_with_no_loop=seen['skipped_loops'],
                  deleting_destructor_calls=seen['deletes'], promotion_full_pool=seen['full'],
                  promotion_kinds_seen=sorted(seen['kinds']), promotion_ground_queries=seen['grounds'],
                  promotion_poses=seen['poses'], update_area_queries=seen['queries'], find_hits=seen['hits'],
                  find_misses=seen['misses'],
                  instructions_total=len(instructions), instructions_reached=len(instructions)-len(unreached),
                  unreached=[hex(a) for a in unreached], functions=record)
    for name, value in report.items():
        print(name, value)
    for failure in failures[:5]:
        print('FAIL', str(failure)[:900])
    status = 1 if failures else 0
    if args.mutation_checks and not failures:
        mutations = {
            'child-digit': ("(char)(i + '1')", "(char)(i + '0')"),
            'transform-offset': ('parent + 0x50', 'parent + 0x40'),
            'placement-offset': ('bytes + 0x480', 'bytes + 0x484'),
            'stamp-sentinel': ('stamp == 0x7fffffff', 'stamp == 0x7ffffffe'),
            'stamp-zero': ('if (stamp == 0 ||', 'if (stamp == 1 ||'),
            'activation-counter': ('++worldObjectActivations86B378;', '--worldObjectActivations86B378;'),
            'registered-flag': ('bytes[0x505] = 1;', 'bytes[0x505] = 2;'),
            'entry-index': ('+ 0x90 + i * 4', '+ 0x94 + i * 4'),
            'id-truncation': ('(unsigned short)collisionIds86B2BC->next8', '(unsigned short)(collisionIds86B2BC->next8 + 1)'),
            'promote-delete': ('    if (body) {\n        delete body;\n    }', '    if (body && promoted) {\n        delete body;\n    }'),
            'promotion-lowering': ('+ worldObjectSettle7844F4;\n                if (lowered > worldObjectZero7D7D3C) {',
                                   '+ worldObjectSettle7844F4;\n                if (lowered >= worldObjectZero7D7D3C) {'),
            'promotion-descriptor-offset': ('+ 0xb8))', '+ 0xbc))'),
            'promotion-bitmap': ('index >> 3] |=', 'index >> 4] |='),
            'promotion-matrix-copy': ('entry + 0x30', 'entry + 0x34'),
            'promotion-index-store': ('+ 0x164) = index', '+ 0x168) = index'),
            'attach-pose-translation': ('position.z = rotated.z + translation.z;', 'position.z = rotated.z;'),
            'attach-orientation-row': ('orientation.elements[15] = 1.0f;', 'orientation.elements[15] = 0.0f;'),
            'attach-duration': ('dynamic->duration6C = 5000.0f;', 'dynamic->duration6C = 5001.0f;'),
            'attach-lowering': ('lowered.elements[14] - *reinterpret_cast<float *>(bytes + 0x168)',
                                'lowered.elements[14] + *reinterpret_cast<float *>(bytes + 0x168)'),
            'attach-second-list': ('int *second = reinterpret_cast<int *>(bytes + 0x4fc);', 'int *second = reinterpret_cast<int *>(bytes + 0x4f4);'),
            'attach-velocity-clear': ('dynamic->velocityC0.z = 0.0f;', ''),
            'update-clamp': ('speed = 6.0f;', 'speed = 5.0f;'),
            'update-radius': ('speed + radius', 'speed - radius'),
            'update-pack-radius': ('->x, 0.0f);', '->x, 1.0f);'),
            'register-front': ('sentinel->next0 = node;\n            front->previous4 = node;',
                               'sentinel->next0 = node;\n            front->previous4 = sentinel;'),
            'register-state-three': ('if (state != 3) {', 'if (state != 4) {'),
            'register-count': ('++count50;\n            return;', 'count50 += 2;\n            return;'),
            'register-fraction': ('float scale = second * first;', 'float scale = second + first;'),
            'register-clear': ('node->value18 = 0;', 'node->value18 = 1;'),
            'place-clear': ('value10 = 0;', 'value10 = 1;'),
            'place-mark': ('if (!parent->marked0E4) {', 'if (parent->marked0E4) {'),
            'mark-recursion': ('if (!child->marked0E4) {', 'if (child->marked0E4) {'),
            'find-divisor': ('/ 0x70 + first', '/ 0x71 + first'),
            'find-upper-bound': ('index < block->countC + first', 'index <= block->countC + first'),
            'random-mask': ('(value & 0x7fffffff) * worldObjectRandomScale78D864 * scale', '(value & 0x3fffffff) * worldObjectRandomScale78D864 * scale'),
            'bounds-max': ('maximum->x = maximumF0.x;', 'maximum->x = minimumE0.x;'),
        }
        text = args.source.read_text()
        rejected = []
        for label, (before, after) in mutations.items():
            assert before in text, label
            mutant = BUILD/('mutation-'+label+'.cpp')
            mutant.write_text(text.replace(before, after, 1))
            result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '600'], cwd=ROOT/'tools',
                                    text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if result.returncode == 0:
                print('MUTATION SURVIVED', label)
                status = 1
            else:
                rejected.append(label)
        print('rejected_mutations', len(rejected), 'of', len(mutations), rejected)
    return status


if __name__ == '__main__':
    sys.exit(main())
