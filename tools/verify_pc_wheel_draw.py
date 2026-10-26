#!/usr/bin/env python3
"""Compare the recovered car wheel placement with the original PC v1.2 code under emulation.

Two routines are checked:
  * CarRenderWheelLayout, original 0x61fe10 (__thiscall on the CarRenderInfo, no stack arguments):
    the layout fields ri+0x400..0x43c and ri+0x420..0x42c, with the real SPINNEROFFSET lookup 0x60fef0.
  * CarWheelDraw, original 0x62700e..0x62df66 (the wheel block of the car draw 0x620ae0): the setup of
    the per-wheel sets, the spin pre-rotation, the solid links, the eight draws and the tail.

WheelDraw sources (src/Speed/Indep/Src/World/CarRenderWheels.cpp) are built with MSVC 7.1 /Ox /Ob0 /EHsc
and linked at a fixed address. Both sides run in Unicorn with the same fixture (arena, frame inputs,
globals) and the same stubs for the unrecovered callees: the solid draw 0x5c5930, visibility 0x48f660,
solid slot 0x48d860, helper 0x60d700 (float result), helper 0x620600, and the allocators 0x401e80/0x401e30.
The rebuilt routine calls its callees through the same addresses, so the stubs record identical events.
Compared: the ordered callee events (arguments normalised: arena pointers as offsets, stack pointers as a
marker) and the whole arena at the end. Reads of unprepared frame cells in the original are failures.
The BodyMatrixCopy, RotateMatrixX/Z, D3DX multiply (traced stub, same in both runs), __ftol2 and
SpinTerm are original code in both runs. Byte identity of the rebuilt code is NOT claimed.
"""

import argparse
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path

import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_PROT_ALL
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/wheel-draw'
SOURCE = 'src/Speed/Indep/Src/World/CarRenderWheels.cpp'
IBASE = 0x400000
ARENA, ARENA_SIZE = 0x2000000, 0x40000
STK, STK_SIZE = 0x2100000, 0x20000
TRAMP, STOP = 0x2300000, 0x2310000
S_FRAME = STK + 0x6000               # ESP of the original block at 0x62700e (frame base S)
EBP_ORIG = S_FRAME + 0xf00           # arguments of 0x620ae0 at EBP+8..
RSTK = STK + 0x10000                 # rebuilt routine stack top
ENTRY_BLOCK, EXIT_BLOCK = 0x62700e, 0x62df66
ENTRY_LAYOUT = 0x61fe10
GLOBAL_TABLE = 0x8a1cc8              # bWare-style global used by 0x60fef0

# Arena fixture (addresses are arena offsets + ARENA).
RI = ARENA + 0x0000                  # CarRenderInfo (0x3000 bytes: wheel presence bytes at +0x2dd8)
CAR = ARENA + 0x3000                 # ri+4 car object
ROW = ARENA + 0x4000                 # CarTypeInfo row (ri+8)
OWNER = ARENA + 0x5000               # ri+0 owner (presence rule bytes +0x3f4..+0x3f7)
VIEW = ARENA + 0x6000                # view object (arg1)
TYRE = ARENA + 0x6200                # arg5 (four frames)
BRAKE = ARENA + 0x6300               # arg6
THIRD = ARENA + 0x6400               # arg7
BODY = ARENA + 0x6500                # EBX body matrix
SOLIDS = ARENA + 0x7000              # dummy solid objects
INPUT = ARENA + 0x7800               # CarWheelDrawInputs for the rebuilt routine
SETDUMP = ARENA + 0xa000             # verification output of the rebuilt routine (16 matrices)
RENDER_POSITION, SOURCE_RECORD = ARENA + 0xb000, ARENA + 0xb100
PARTS = ARENA + 0x7900               # rim parts for the layout check
GTAB = ARENA + 0x9000                # global lookup table for 0x60fef0 (layout check)
POOL = ARENA + 0x10000
POOL_SIZE = 0x8000

# Globals of the executable read by the block (their image values are zero or constants).
G_WIDTH_FRONT, G_WIDTH_REAR = 0x7a668c, 0x7a6690
G_LOD_GATE, G_LOD_THRESHOLD = 0x8654a4, 0x8a1c5c
D3DX = 0x63cedb
ARG_VISIBLE_GLOBAL = 0x871b50

# Image constants (checked against the executable bytes by the manifest).
IMAGE_CONSTANTS = {0x7d7d3c: 0x00000000, 0x8026c4: 0x3e3c6a7f, 0x8026c0: 0x3f000000, 0x784260: 0x3f000000,
                   0x784250: 0x3f800000, 0x7a6750: 0x00000000, 0x7a6754: 0x40e00000, 0x7a6758: 0x3cf5c28f,
                   0x784238: 0x47800000}

# Input struct layout (CarWheelDrawInputs, 32-bit): 15 dwords.
INPUT_FIELDS = ['ri', 'view', 'flagsArg8', 'body', 'tyre', 'brake', 'third', 'lod6C', 'slotD8',
                'lodF4', 'flagsCC', 'flags1AC', 'setDump', 'renderPosition', 'sourceRecord']
# Frame cells the original reads before writing, filled from the inputs (S-relative offsets).
FRAME_INPUTS = {0x58: 'ri', 0x6c: 'lod6C', 0xd8: 'slotD8', 0xf4: 'lodF4', 0xcc: 'flagsCC',
                0x1ac: 'flags1AC'}

STUB_ADDRESSES = {0x5c5930: 'DrawSolid', 0x48f660: 'Visible', 0x48d860: 'SetSlot', 0x60d700: 'Fn60d700',
                  0x620600: 'Fn620600', 0x401e80: 'Alloc4', 0x401e30: 'Alloc700', D3DX: 'D3DX'}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def special_float(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-30, -1e-30, 2.5, -3.25, rng.uniform(-2, 2)))


def randf(rng, scale=1.5):
    r = rng.random()
    if r < 0.05:
        return special_float(rng)
    return f32(rng.uniform(-scale, scale))


def randbits(rng, bits=0):
    return rng.getrandbits(32)


class Machine:
    """One Unicorn run of either the original block or the rebuilt routine."""

    def __init__(self, image, linked=None):
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc = self.uc
        uc.mem_map(IBASE, (len(image) + 0xfff) & ~0xfff, UC_PROT_ALL)
        uc.mem_write(IBASE, image)
        uc.mem_map(ARENA, ARENA_SIZE, UC_PROT_ALL)
        uc.mem_map(STK, STK_SIZE, UC_PROT_ALL)
        uc.mem_map(TRAMP & ~0xfff, 0x2000, UC_PROT_ALL)
        uc.mem_map(STOP & ~0xfff, 0x1000, UC_PROT_ALL)
        if linked is not None:
            uc.mem_map(linked['address'], linked['allocation'], UC_PROT_ALL)
            for a, b in linked['blocks']:
                uc.mem_write(a, b)
        self.events = []
        self.unprepared = []
        self.prepared = set()
        self.written = set()
        self.fn_values = []
        self.alloc_next = POOL
        self.count = 0
        uc.mem_write(TRAMP, bytes([0xd9, 0x05]) + struct.pack('<I', TRAMP + 0x100) + bytes([0xff, 0x25]) +
                     struct.pack('<I', TRAMP + 0x104))
        uc.mem_write(STOP, b'\x90')
        uc.hook_add(UC_HOOK_MEM_READ, self._on_read, begin=S_FRAME - 0x100, end=S_FRAME + 0xf00)
        uc.hook_add(UC_HOOK_MEM_WRITE, self._on_write, begin=S_FRAME - 0x100, end=S_FRAME + 0xf00)
        for addr in STUB_ADDRESSES:
            uc.hook_add(UC_HOOK_CODE, self._stub, user_data=addr, begin=addr, end=addr)

    def _on_read(self, uc, access, address, size, value, user):
        if S_FRAME <= address < S_FRAME + 0xf00 and address not in self.written and address not in self.prepared:
            self.unprepared.append((hex(uc.reg_read(xc.UC_X86_REG_EIP)), address - S_FRAME))

    def _on_write(self, uc, access, address, size, value, user):
        if S_FRAME <= address < S_FRAME + 0xf00:
            self.written.add(address)

    def _args(self, uc, n):
        esp = uc.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<%dI' % n, bytes(uc.mem_read(esp + 4, 4 * n)))

    def _ret(self, uc, pop):
        esp = uc.reg_read(xc.UC_X86_REG_ESP)
        ret = struct.unpack('<I', bytes(uc.mem_read(esp, 4)))[0]
        uc.reg_write(xc.UC_X86_REG_ESP, esp + pop)
        return ret

    def _stub(self, uc, address, size, name):
        name = STUB_ADDRESSES[name]
        if name == 'DrawSolid':          # __thiscall, RET 0x14: ECX = view, five arguments
            args = self._args(uc, 5)
            self.events.append(('DrawSolid', uc.reg_read(xc.UC_X86_REG_ECX), args))
            uc.reg_write(xc.UC_X86_REG_EIP, self._ret(uc, 4 + 0x14))
        elif name == 'Visible':          # cdecl, six arguments; the result is not used by the block
            args = self._args(uc, 6)
            assert args[3] in (S_FRAME + 0x790, RENDER_POSITION), ('state or call order mismatch', 'position pointer')
            assert args[5] in (S_FRAME + 0xb20, SOURCE_RECORD), ('state or call order mismatch', 'source pointer')
            self.events.append(('Visible', args, bytes(uc.mem_read(args[3], 16)), bytes(uc.mem_read(args[5], 0x1c0))))
            uc.reg_write(xc.UC_X86_REG_EAX, 1)
            uc.reg_write(xc.UC_X86_REG_EIP, self._ret(uc, 4))
        elif name == 'SetSlot':          # __thiscall, RET 8: ECX = solid, key and value
            args = self._args(uc, 2)
            self.events.append(('SetSlot', uc.reg_read(xc.UC_X86_REG_ECX), args))
            uc.reg_write(xc.UC_X86_REG_EIP, self._ret(uc, 4 + 8))
        elif name == 'Fn60d700':         # cdecl: float result in ST0 (returned through a trampoline)
            args = self._args(uc, 2)
            value = self.fn_values[self.count % len(self.fn_values)] if self.fn_values else 0.0
            self.count += 1
            self.events.append(('Fn60d700', args[0], args[1], None))
            uc.mem_write(TRAMP + 0x100, struct.pack('<f', value))
            uc.mem_write(TRAMP + 0x104, struct.pack('<I', self._ret(uc, 4)))
            uc.reg_write(xc.UC_X86_REG_EIP, TRAMP)
        elif name == 'Fn620600':         # __thiscall on ri, RET 8: arguments view and world
            args = self._args(uc, 2)
            self.events.append(('Fn620600', uc.reg_read(xc.UC_X86_REG_ECX), args))
            uc.reg_write(xc.UC_X86_REG_EIP, self._ret(uc, 4 + 8))
        elif name in ('Alloc4', 'Alloc700'):   # cdecl, one argument: bump pool allocation
            args = self._args(uc, 1)
            size = 0x100 if name == 'Alloc4' else 0x700
            p = self.alloc_next
            self.alloc_next += (size + 15) & ~15
            self.events.append((name, args[0], p - ARENA))
            uc.reg_write(xc.UC_X86_REG_EAX, p)
            uc.reg_write(xc.UC_X86_REG_EIP, self._ret(uc, 4))
        elif name == 'D3DX':            # the D3DX multiply thunk: traced stub (same in both runs)
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            ret, out, left, right = struct.unpack('<4I', bytes(uc.mem_read(esp, 16)))
            l = struct.unpack('<16f', bytes(uc.mem_read(left, 64)))
            r = struct.unpack('<16f', bytes(uc.mem_read(right, 64)))
            res = struct.pack('<16f', *[f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4)))
                                       for i in range(4) for j in range(4)])
            uc.mem_write(out, res)
            self.events.append(('D3DX', 'stk' if STK <= out < STK + STK_SIZE else out - ARENA))
            uc.reg_write(xc.UC_X86_REG_EAX, out)
            uc.reg_write(xc.UC_X86_REG_ESP, esp + 16)
            uc.reg_write(xc.UC_X86_REG_EIP, ret)


def norm(v):
    if ARENA <= v < ARENA + ARENA_SIZE:
        return ('arena', v - ARENA)
    if STK <= v < STK + STK_SIZE:
        return ('stack',)
    return v


def normalize_events(events):
    out = []
    for e in events:
        if e[0] == 'D3DX':
            out.append(e)
        elif e[0] == 'Visible':
            args = tuple(('position',) if i == 3 else ('source',) if i == 5 else norm(a) for i, a in enumerate(e[1]))
            out.append(('Visible', args, e[2], e[3]))
        elif e[0] == 'Fn60d700':
            out.append(('Fn60d700', norm(e[1]), norm(e[2])))
        elif e[0] in ('Alloc4', 'Alloc700'):
            out.append(e)
        elif e[0] in ('DrawSolid', 'SetSlot', 'Fn620600'):
            out.append((e[0], norm(e[1]), tuple(norm(a) for a in e[2])))
        else:
            out.append(e)
    return out


def write_block_inputs(m, case):
    uc = m.uc
    uc.mem_write(INPUT, struct.pack('<15I', *[case[f] & 0xffffffff for f in INPUT_FIELDS]))


def setup_original_block(m, case, body_ptr):
    uc = m.uc
    S = S_FRAME
    for off, field in FRAME_INPUTS.items():
        uc.mem_write(S + off, struct.pack('<I', case[field] & 0xffffffff))
        m.prepared.add(S + off)
    for offset, source, size in ((0x790, RENDER_POSITION, 16), (0xb20, SOURCE_RECORD, 0x1c0)):
        uc.mem_write(S + offset, bytes(uc.mem_read(source, size)))
        m.prepared.update(range(S + offset, S + offset + size, 4))
    args = {0x08: case['view'], 0x0c: 0x7f3f00, 0x10: body_ptr, 0x14: 0, 0x18: case['tyre'],
            0x1c: case['brake'], 0x20: case['third'], 0x24: case['flagsArg8']}
    for off, v in args.items():
        uc.mem_write(EBP_ORIG + off, struct.pack('<I', v & 0xffffffff))
    uc.reg_write(xc.UC_X86_REG_ESP, S)
    uc.reg_write(xc.UC_X86_REG_EBP, EBP_ORIG)
    uc.reg_write(xc.UC_X86_REG_EBX, body_ptr)
    uc.reg_write(xc.UC_X86_REG_ESI, case['ri'])
    uc.reg_write(xc.UC_X86_REG_EDI, case['third'])
    uc.reg_write(xc.UC_X86_REG_ECX, case['ri'])
    uc.reg_write(xc.UC_X86_REG_FPCW, case['fpcw'])


def build_fixture(case, rng):
    """Returns a dict of byte blobs at arena addresses and the case description."""
    blobs = {}
    ri = bytearray(0x3000)
    struct.pack_into('<I', ri, 0x0, case['owner'])
    struct.pack_into('<I', ri, 0x4, CAR)
    struct.pack_into('<I', ri, 0x8, ROW)
    struct.pack_into('<8f', ri, 0x400, *case['pivot400'])
    struct.pack_into('<4f', ri, 0x420, *case['scale420'])
    struct.pack_into('<f', ri, 0x430, case['width430'])
    struct.pack_into('<f', ri, 0x434, case['width434'])
    struct.pack_into('<8f', ri, 0x438, *case['offset438'])
    struct.pack_into('<8I', ri, 0x1b48, *case['solid1'])
    struct.pack_into('<8I', ri, 0x1b68, *case['solid2'])
    ri[0x2d78 - 0x0] = case['present'][0]
    ri[0x2d98] = case['present'][1]
    ri[0x2db8] = case['present'][2]
    ri[0x2dd8] = case['present'][3]
    struct.pack_into('<I', ri, 0x390, case['v0'])
    struct.pack_into('<I', ri, 0x3ac, case['v1'])
    blobs[RI] = bytes(ri)
    car = bytearray(0x1000)
    car[0x855] = case['gate']
    car[0x10 + 0x353] = case['spin353']
    car[0x10 + 0x354] = case['spin354']
    blobs[CAR] = bytes(car)
    owner = bytearray(0x1000)
    for w in range(4):
        owner[0x3f4 + w] = case['owner_bytes'][w]
    blobs[OWNER] = bytes(owner)
    row = bytearray(0x900)
    for w in range(4):
        for i, v in enumerate(case['row_wheel'][w]):
            struct.pack_into('<f', row, 0x120 + 0x30 * w + i * 4, v)
    blobs[ROW] = bytes(row)
    blobs[TYRE] = b''.join(struct.pack('<16f', *m) for m in case['tyre_m'])
    blobs[BRAKE] = b''.join(struct.pack('<16f', *m) for m in case['brake_m'])
    blobs[THIRD] = b''.join(struct.pack('<16f', *m) for m in case['third_m'])
    blobs[BODY] = struct.pack('<16f', *case['body_m'])
    blobs[VIEW] = bytes(rng.getrandbits(8) for _ in range(0x100))
    blobs[SOLIDS] = bytes(0x400)
    blobs[RENDER_POSITION] = struct.pack('<4f', randf(rng, 30), randf(rng, 30), randf(rng, 30), 1.0)
    blobs[SOURCE_RECORD] = struct.pack('<112f', *[randf(rng) for _ in range(112)])
    return blobs


def random_case(rng, layout=False):
    case = {}
    case['owner'] = rng.choice((0, OWNER, OWNER)) if rng.random() < 0.9 else 0
    case['owner_bytes'] = [rng.choice((0, 1, 1, 2)) for _ in range(4)]
    case['pivot400'] = [randf(rng) for _ in range(8)]
    case['scale420'] = [rng.choice((0.0, randf(rng), randf(rng))) for _ in range(4)]
    case['width430'] = rng.choice((0.0, randf(rng, 0.4), randf(rng, 0.4)))
    case['width434'] = rng.choice((0.0, randf(rng, 0.4), randf(rng, 0.4)))
    case['offset438'] = [randf(rng) for _ in range(8)]
    # Cancellation cases: the pivot near F*W = B*s'*W, so C - T is small and the float rounding of T shows.
    beta = 1.0 / float(f32(0.184))
    for w in range(4):
        if rng.random() < 0.5:
            s_w = case['scale420'][w]
            sp = f32(0.184) if s_w == 0.0 else s_w
            width = case['width430'] if w < 2 else case['width434']
            if width != 0.0:
                target = f32(beta * float(sp) * float(width)) * (1.0 + rng.uniform(-1e-7, 1e-7))
                case['pivot400'][2 * w] = f32(target)
                case['pivot400'][2 * w + 1] = f32(target)
    solid_choices = [0] + [SOLIDS + 0x40 * i for i in range(8)]
    case['solid1'] = [rng.choice(solid_choices) if rng.random() < 0.8 else 0 for _ in range(8)]
    case['solid2'] = [rng.choice(solid_choices) if rng.random() < 0.8 else 0 for _ in range(8)]
    case['present'] = [rng.choice((0, 1, 1, 1)) for _ in range(4)]
    case['v0'] = rng.choice((0, 0, 0x1234, SOLIDS + 0x40))
    case['v1'] = rng.choice((0, 0, 0x2222))
    case['gate'] = 1 if rng.random() < 0.9 else 0
    case['spin353'] = rng.choice((0, 1))
    case['spin354'] = rng.choice((0, 1, 0))
    case['row_wheel'] = [[randf(rng, 0.6) for _ in range(9)] for _ in range(4)]
    case['tyre_m'] = [[randf(rng) for _ in range(16)] for _ in range(4)]
    case['brake_m'] = [[randf(rng) for _ in range(16)] for _ in range(4)]
    case['third_m'] = [[randf(rng) for _ in range(16)] for _ in range(4)]
    case['body_m'] = [randf(rng) for _ in range(16)]
    case['view'] = VIEW
    case['ri'] = RI
    case['body'] = BODY
    case['tyre'] = TYRE
    case['brake'] = BRAKE
    case['third'] = THIRD
    case['lod6C'] = rng.choice((0, 0, 1, 1, 2, 3))
    case['slotD8'] = rng.choice((0, 1))
    case['lodF4'] = rng.choice((0, 0, 1, 2, 3))
    case['flagsCC'] = rng.choice((0, 0x1, 0x400, rng.getrandbits(16)))
    case['flags1AC'] = rng.choice((0, 0x2, 0x800, rng.getrandbits(16)))
    case['flagsArg8'] = rng.choice((0x401, 0, rng.getrandbits(16)))
    case['setDump'] = SETDUMP
    case['renderPosition'] = RENDER_POSITION
    case['sourceRecord'] = SOURCE_RECORD
    case['fpcw'] = rng.choice((0x027f, 0x037f))
    case['fn_values'] = [randf(rng, 1.5) for _ in range(12)]
    case['lod_gate'] = rng.choice((0, 3, 6, 6, 2))
    case['lod_threshold'] = randf(rng, 1.0)
    case['override_front'] = 0.0 if rng.random() < 0.8 else randf(rng, 0.5)
    case['override_rear'] = 0.0 if rng.random() < 0.8 else randf(rng, 0.5)
    return case


def write_case_memory(m, blobs, case, body):
    uc = m.uc
    for addr, data in blobs.items():
        uc.mem_write(addr, data)
    uc.mem_write(OWNER, blobs[OWNER])
    uc.mem_write(G_LOD_GATE, struct.pack('<I', case['lod_gate']))
    uc.mem_write(G_LOD_THRESHOLD, struct.pack('<f', case['lod_threshold']))
    uc.mem_write(G_WIDTH_FRONT, struct.pack('<f', case['override_front']))
    uc.mem_write(G_WIDTH_REAR, struct.pack('<f', case['override_rear']))
    uc.mem_write(ARG_VISIBLE_GLOBAL, struct.pack('<16f', *case['body_m']))
    m.fn_values = case['fn_values']


def run_block(m, case):
    m.events = []
    m.unprepared = []
    m.prepared = set()
    m.written = set()
    m.count = 0
    m.alloc_next = POOL
    uc = m.uc
    uc.mem_write(POOL, b'\x00' * POOL_SIZE)
    write_block_inputs(m, case)
    setup_original_block(m, case, BODY)
    try:
        uc.emu_start(ENTRY_BLOCK, EXIT_BLOCK, count=3000000)
    except UcError as error:
        raise AssertionError(('state or call order mismatch', 'original fixture fault', str(error)))
    return m


def run_rebuilt_block(m, case, entry):
    m.events = []
    m.count = 0
    m.alloc_next = POOL
    uc = m.uc
    uc.mem_write(POOL, b'\x00' * POOL_SIZE)
    write_block_inputs(m, case)
    esp = RSTK - 0x100
    uc.reg_write(xc.UC_X86_REG_ESP, esp)
    uc.reg_write(xc.UC_X86_REG_FPCW, case['fpcw'])
    uc.mem_write(esp, struct.pack('<2I', STOP, INPUT))
    try:
        uc.emu_start(entry, STOP, count=3000000)
    except UcError as error:
        raise AssertionError(('state or call order mismatch', 'rebuilt fault', str(error)))
    return m


def compare_block(orig, rebuilt, case):
    if orig.unprepared:
        raise AssertionError(('original fixture fault', 'read of an unprepared frame cell', orig.unprepared[:4]))
    # Sets of the original (frame cells) against the rebuilt dump: set0 0x200, set1 0x3d0, set2 0x4d0, set3 0x5d0.
    frame_sets = []
    for base in (0x200, 0x3d0, 0x4d0, 0x5d0):
        for w in range(4):
            frame_sets.append(bytes(orig.uc.mem_read(S_FRAME + base + 0x40 * w, 64)))
    dumped = [bytes(rebuilt.uc.mem_read(SETDUMP + 64 * i, 64)) for i in range(16)]
    for i, (a, b) in enumerate(zip(frame_sets, dumped)):
        if a != b:
            raise AssertionError(('state or call order mismatch', 'set', ('set0', 'set1', 'set2', 'set3')[i // 4], i % 4,
                                  struct.unpack('<16f', a), struct.unpack('<16f', b)))
    eo = normalize_events(orig.events)
    er = normalize_events(rebuilt.events)
    if eo != er:
        for i, (a, b) in enumerate(zip(eo, er)):
            if a != b:
                raise AssertionError(('state or call order mismatch', 'event', i, a, b))
        raise AssertionError(('state or call order mismatch', 'event count', len(eo), len(er)))
    # The set dump is verification output of the rebuilt routine only; the sets were compared above.
    ao = bytes(orig.uc.mem_read(ARENA, ARENA_SIZE))
    ar = bytearray(rebuilt.uc.mem_read(ARENA, ARENA_SIZE))
    ar[SETDUMP - ARENA:SETDUMP - ARENA + 0x400] = ao[SETDUMP - ARENA:SETDUMP - ARENA + 0x400]
    ar = bytes(ar)
    if ao != ar:
        diff = [i for i in range(0, ARENA_SIZE, 4) if ao[i:i + 4] != ar[i:i + 4]][:6]
        raise AssertionError(('state or call order mismatch', 'arena', [hex(ARENA + d) for d in diff]))
    return len(eo)


def layout_fixture(rng):
    """0x61fe10: row, car parts and the global lookup table for 0x60fef0."""
    case = {'row': [[randf(rng, 0.8) for _ in range(9)] for _ in range(4)]}
    case['parts'] = []
    case['car604'] = rng.choice((0, PARTS, PARTS + 0x100))
    case['car608'] = rng.choice((0, PARTS + 0x200, PARTS))
    case['flagsA'] = rng.getrandbits(8)
    case['flagsB'] = rng.getrandbits(8)
    return case


def spinner_hash():
    h = 0xffffffff
    for c in b'SPINNEROFFSET':
        h = (h * 33 + c) & 0xffffffff
    return h


SLOT_WORDS = (0, 4, 8, 12, 0xffff)   # table offsets (2 * slot) that do not overlap


def write_layout_memory(uc, case, rng):
    row = bytearray(0x900)
    for w in range(4):
        for i, v in enumerate(case['row'][w]):
            struct.pack_into('<f', row, 0x120 + 0x30 * w + i * 4, v)
    uc.mem_write(ROW, bytes(row))
    ri = bytearray(0x3000)
    struct.pack_into('<I', ri, 0x4, CAR)
    struct.pack_into('<I', ri, 0x8, ROW)
    uc.mem_write(RI, bytes(ri))
    car = bytearray(0x1000)
    struct.pack_into('<I', car, 0x604, case['car604'])
    struct.pack_into('<I', car, 0x608, case['car608'])
    uc.mem_write(CAR, bytes(car))
    for base, flag in ((PARTS, case['flagsA']), (PARTS + 0x100, case['flagsB']), (PARTS + 0x200, case['flagsA'] ^ 0x80)):
        part = bytearray(0x40)
        struct.pack_into('<H', part, 0xa, rng.choice(SLOT_WORDS))
        part[5] = flag
        uc.mem_write(base, bytes(part))
    # global lookup: G+0x14 -> index table (16-bit count, then 16-bit record indices at 2 * slot word),
    # G+0x1c -> 8-byte records (u32 hash, float value)
    tbl = bytearray(0x400)
    for slot in SLOT_WORDS[:-1]:
        cnt = rng.randint(0, 2)
        struct.pack_into('<h', tbl, slot, cnt)
        for i in range(cnt):
            struct.pack_into('<h', tbl, slot + 2 + 2 * i, rng.randint(0, 15))
    recs = bytearray(0x200)
    hsh = spinner_hash()
    for r in range(16):
        h = hsh if r % 3 == 0 else (0x11111111 * (r + 1)) & 0xffffffff
        struct.pack_into('<I', recs, 8 * r, h)
        struct.pack_into('<f', recs, 8 * r + 4, randf(rng, 1.0))
    uc.mem_write(GTAB + 0x100, bytes(tbl))
    uc.mem_write(GTAB + 0x300, bytes(recs))
    g = bytearray(0x40)
    struct.pack_into('<I', g, 0x14, GTAB + 0x100)
    struct.pack_into('<I', g, 0x1c, GTAB + 0x300)
    uc.mem_write(GTAB, bytes(g))
    uc.mem_write(GLOBAL_TABLE, struct.pack('<I', GTAB))


def run_layout(m, entry, layout_case, rng, is_original):
    uc = m.uc
    stack = STK + 0x9000
    uc.reg_write(xc.UC_X86_REG_ESP, stack)
    if is_original:
        uc.mem_write(stack, struct.pack('<I', STOP))
        uc.reg_write(xc.UC_X86_REG_ECX, RI)
        uc.emu_start(entry, STOP, count=200000)
    else:
        uc.mem_write(stack, struct.pack('<2I', STOP, RI))
        uc.emu_start(entry, STOP, count=200000)


def check_layout(image, linked_layout, n_cases, rng):
    counts = 0
    for i in range(n_cases):
        lc = layout_fixture(rng)
        ms = []
        for is_original in (True, False):
            m = Machine(image, linked_layout if not is_original else None)
            write_layout_memory(m.uc, lc, random.Random(0x61fe10 + i))
            entry = ENTRY_LAYOUT if is_original else linked_layout['entry']
            run_layout(m, entry, lc, rng, is_original)
            ms.append(m)
        a = bytes(ms[0].uc.mem_read(RI, 0x3000))
        b = bytes(ms[1].uc.mem_read(RI, 0x3000))
        if a != b:
            diff = [hex(RI + k) for k in range(0, 0x3000, 4) if a[k:k + 4] != b[k:k + 4]][:6]
            raise AssertionError(('state or call order mismatch', 'layout', i, diff))
        counts += 1
    return counts


def load_manifest():
    return json.loads((ROOT / 'config/SPEED2_EXE_1_2/wheel-draw.json').read_text())


def build_rebuilt(source, out_name, mutated=False):
    obj = BUILD / out_name
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0',
                    '/EHsc', '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World', '/Fo' + str(obj.relative_to(ROOT)),
                    str(source.relative_to(ROOT) if source.is_relative_to(ROOT) else source)], cwd=ROOT, check=True)
    return obj


EXTERNALS = {
    '?BodyMatrixCopy@@': 0x4012b0, '?RotateMatrixX@@': 0x5bacf0, '?RotateMatrixZ@@': 0x5bae60,
    '?DrawSolid@CarRenderView@@': 0x5c5930, '?SetSlot@CarRenderSolid@@': 0x48d860,
    '?Fn620600@CarRenderWheelInfo@@': 0x620600, '?FindRecord@CarRenderPart@@': 0x60fef0,
    '__ftol2': 0x75bed4, '__fltused': 0,
    'UG2WheelVisible': 0x48f660, 'UG2WheelFn60d700': 0x60d700,
    'UG2WheelAllocMatrices': 0x401e80, 'UG2WheelAllocRecords': 0x401e30,
    'ug2WheelWidthOverrideFront': G_WIDTH_FRONT, 'ug2WheelWidthOverrideRear': G_WIDTH_REAR,
    'ug2WheelOneRuntime': 0x784250, 'ug2WheelBetaRuntime': 0x8026c4,
    'ug2WheelLodGate': G_LOD_GATE, 'ug2WheelLodThreshold': G_LOD_THRESHOLD,
    'ug2WheelReflectionBasis871B50': ARG_VISIBLE_GLOBAL,
}


def link_rebuilt(obj):
    parsed = read_object(obj)
    defined = {s['name'] for s in parsed['symbols'].values() if s['section'] > 0}
    externals = {}
    for s in parsed['symbols'].values():
        if s['section'] != 0 or s['name'] in defined or s['name'] == '':
            continue
        name = s['name']
        key = name[1:] if name.startswith('_') and not name.startswith('__') else name
        match = None
        for pre, addr in EXTERNALS.items():
            if key == pre or key.startswith(pre) or name.startswith(pre):
                match = addr
                break
        if match is None:
            raise AssertionError(('unexpected external', name))
        externals[name] = match
    linked = link_objects([obj], externals)
    entries = {}
    for name in ('?CarWheelDraw@@', '?CarRenderWheelLayout@@'):
        entries[name] = next(a for s, a in linked['exports'].items() if s.startswith(name))
    linked['entry_block'] = entries['?CarWheelDraw@@']
    linked['entry_layout'] = entries['?CarRenderWheelLayout@@']
    return linked


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--replace', help='compile this file instead of the recovered source')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--layout-cases', type=int, default=512)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = load_manifest()
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - IBASE
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256'], f
    for addr, val in IMAGE_CONSTANTS.items():
        assert struct.unpack('<I', image[addr - IBASE:addr - IBASE + 4])[0] == val, hex(addr)
    BUILD.mkdir(parents=True, exist_ok=True)
    source = Path(args.replace) if args.replace else ROOT / SOURCE
    obj = build_rebuilt(source, 'replaced.obj' if args.replace else 'CarRenderWheels.obj')
    linked = link_rebuilt(obj)
    rng = random.Random(0x62700e)
    layout_count = check_layout(image, dict(linked, entry=linked['entry_layout']), args.layout_cases, rng)
    counts = {'layout_0x61fe10': layout_count, 'block_0x62700e': 0, 'events': 0}
    unprepared = 0
    for i in range(args.cases):
        case = random_case(rng)
        orig = Machine(image)
        blobs = build_fixture(case, random.Random(i))
        write_case_memory(orig, blobs, case, body=None)
        run_block(orig, case)
        reb = Machine(image, linked)
        write_case_memory(reb, blobs, case, body=None)
        run_rebuilt_block(reb, case, linked['entry_block'])
        counts['events'] += compare_block(orig, reb, case)
        counts['block_0x62700e'] += 1
        for e in orig.events:
            key = 'event_' + e[0]
            counts[key] = counts.get(key, 0) + 1
        if case['lod6C'] <= 1 and case['spin353'] | case['spin354']:
            counts['case_spin_nonzero'] = counts.get('case_spin_nonzero', 0) + 1
        if min(case['lodF4'], 2) == 0 and case['lod_gate'] == 6:
            counts['case_lod6_at_clamp0'] = counts.get('case_lod6_at_clamp0', 0) + 1
        if case['lod_gate'] == 3 and case['lod6C'] <= 2:
            counts['case_lod3_tail'] = counts.get('case_lod3_tail', 0) + 1
    report = dict(comparisons=counts, cases=args.cases, layout_cases=args.layout_cases,
                  compiler='MSVC 7.1 /Ox /Ob0 /EHsc', compared=['callee events (normalised)', 'position/source buffer contents', 'whole arena bytes'],
                  seams=sorted(STUB_ADDRESSES.values()),
                  limitations=['random and special values; exhaustive float coverage not established',
                               'LOD/gate globals and width overrides are fixture values, not game runtime values',
                               'the body matrix (EBX) is an explicit input (a prologue pool block in the original)',
                               'SpinTerm constants give spin values 0, 637, 1274 only (image constants)',
                               'byte identity not established (compiler output differs from the original)'])
    if args.mutation_checks:
        text0 = (ROOT / SOURCE).read_text()
        rejected = []
        mutations = {
            'wrong-position-buffer': ('const_cast<float *>(in->renderPosition)', 'in->body'),
            'wrong-source-buffer': ('in->view, in->sourceRecord', 'in->view, reinterpret_cast<CarRenderRecord *>(const_cast<float *>(in->renderPosition))'),
            'tyre-flag-cc-dropped': ('const unsigned int flagsTyre = in->flagsCC | in->flags1AC | in->flagsArg8;',
                                     'const unsigned int flagsTyre = in->flags1AC | in->flagsArg8;'),
            'spin-sign-wheel1': ('SpunFrame(&rot, &in->tyre[1], -spin, spin)', 'SpunFrame(&rot, &in->tyre[1], spin, spin)'),
            'spin-translation-not-restored': ('    copy->elements[12] = tx;\n', ''),
            'stage-result-rounding': ('volatile float stored = static_cast<float>(f * static_cast<double>(width));',
                                      'volatile double stored = f * static_cast<double>(width);'),
            'lod-threshold-direction': ('UG2WheelFn60d700(a1 + 2, in->view) < ug2WheelLodThreshold',
                                        'UG2WheelFn60d700(a1 + 2, in->view) > ug2WheelLodThreshold'),
            'tail-gate-value': ('if (ug2WheelLodGate == 3) {', 'if (ug2WheelLodGate == 6) {'),
            'rear-brake-solid': ('in->view->DrawSolid(solid2[1], a2 + 3, a4 + 0x540, flagsBrake, 0);',
                                 'in->view->DrawSolid(solid2[0], a2 + 3, a4 + 0x540, flagsBrake, 0);'),
            'brake-zero-element': ('set0[w].elements[13] = static_cast<float>(-e);', 'set0[w].elements[13] = static_cast<float>(e);'),
            'pivot-column': ('ri->pivot400[2 * w + k]', 'ri->pivot400[2 * w]'),
            'block1-copy-source': ('BodyMatrixCopy(world, &t);\n}\n\n// Block 2 (', 'BodyMatrixCopy(world, &m);\n}\n\n// Block 2 ('),
            'solid-link-key': ('solid1[j]->SetSlot(0x12049251u, v);', 'solid1[j]->SetSlot(0x12049252u, v);'),
            'layout-rear-flag': ('const unsigned int rearFlag = partB ? (partB[5] >> 5) : 0u;',
                                 'const unsigned int rearFlag = partA ? (partA[5] >> 5) : 0u;'),
        }
        for label, (before, after) in mutations.items():
            if before not in text0:
                raise AssertionError(('mutation text missing', label))
            mutant = BUILD / ('mutation-' + label + '.cpp')
            mutant.write_text(text0.replace(before, after, 1).replace('#include "../Physics/', '#include "'))
            result = subprocess.run([sys.executable, __file__, '--replace', str(mutant), '--cases', '128',
                                     '--layout-cases', '64'], cwd=ROOT, text=True, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1200:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['comparisons'], indent=2))


if __name__ == '__main__':
    main()
