#!/usr/bin/env python3
"""Compare the recovered car draw pixel size and LOD selection with the original PC v1.2 code under emulation.

Routines (src/Speed/Indep/Src/World/CarRenderLod.cpp), original addresses in speed2.exe:
  * CarViewPixelRadius, 0x48acb0 (__thiscall on the view, RET 8): run inside the producer below.
  * CarLodPixelSize, 0x620c6b..0x620cc0: the pixel size, the view-mode scale and the LOD inputs (0x620cba exits when < 4).
  * CarLodSelect, 0x62107e..0x621230: the owner words, the car record search, the wheel flags, the body LOD and clamp,
    the forced zero, the clamps, the overrides, the exit when both are -1 and the arg8 bit 0x800.

Both sides run in Unicorn with the same fixture. The original runs the image bytes (the seams 0x5b98a0 and 0x499db0 and the
CRT __ftol2 run as image code); the rebuilt routines are compiled with MSVC 7.1 /Ox /Ob0 /EHsc and linked at a fixed address
against the same image addresses. Compared: the pixel and the LOD arguments (producer), and the frame cells, the arguments and
the exit of the selection. The float pixel stored by the original (frame+0x64) and the record index (frame+0x68) are compared
too, because later code of the car draw reads them. Byte identity of the rebuilt code is NOT claimed.

Cases are random with special values at the thresholds (pixel 4, 18, 20, 30, 40), the -1 sentinels and the overrides. The
mutation checks compile edited copies of CarRenderLod.cpp and require that the verifier rejects each one.
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
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_PROT_ALL
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/car-draw-lod'
SOURCE = 'src/Speed/Indep/Src/World/CarRenderLod.cpp'
IBASE = 0x400000
ARENA, ARENA_SIZE = 0x2000000, 0x40000
STK, STK_SIZE = 0x2100000, 0x20000
TRAMP, STOP = 0x2300000, 0x2310000
S_FRAME = STK + 0x6000               # ESP of the original car draw inside the fragments (frame base S)
EBP_ORIG = S_FRAME + 0xf00           # arguments of the car draw: EBP+8.. (EBP+0x24 arg8, 0x30 clamp, 0x34 lod)
RSTK = STK + 0x10000                 # stack of the rebuilt routine calls

PRODUCER_START, PRODUCER_END = 0x620c6b, 0x620cc0   # 0x620cc0 follows the exit check at 0x620cba
SELECT_START, SELECT_END = 0x62107e, 0x621230       # 0x621230 starts the call to 0x43c7c0
EXIT_DRAW = 0x630d1b                                 # the car draw epilogue both fragments may leave to
PIXEL_RADIUS = 0x48acb0

# Image globals read by the fragments (addresses are the original ones; the image holds their initial values).
G_GATE = 0x8654a4                    # ug2WheelLodGate
G_FLAG_E8 = 0x7fee4c                 # ug2CarLodFlag7FEE4C
G_OVERRIDE_LOD, G_OVERRIDE_CLAMP = 0x802728, 0x80272c
G_RECORDS, G_RECORD_COUNT = 0x8a1cd4, 0x8a1cd8
G_ZERO_FLAG = 0x836352               # byte read by 0x499db0
G_VIEW_MODE = 0x8707dc               # dword read by 0x5b98a0
TABLE_LOD = 0x7a66dc                 # 8 floats: pixel thresholds indexed by the body LOD
TABLE_PIXEL_LIMIT = 0x7a66fc         # 18.0
TABLE_SCALE = 0x7a6700               # 0.7
GETTER_VIEW_MODE, GETTER_ZERO_FLAG = 0x5b98a0, 0x499db0
FTOL2 = 0x75bed4

# Arena fixture (addresses are arena offsets + ARENA).
RI = ARENA + 0x0000                  # CarRenderWheelInfo (0x3000 bytes; flag at +0x2eb8)
CAR = ARENA + 0x3000                 # car object: byte +7 is compared with the record value
ROW = ARENA + 0x4000                 # car type row: dword +0x840 is compared with the record key
OWNER = ARENA + 0x5000               # owner (ri+0): dwords +0x424 and +0x958
VIEW = ARENA + 0x6000                # view object: focal at +0xc, frame pointer at +0x40
FRAMEV = ARENA + 0x6100              # view frame: origin at +0x40, depth axis at +0x50
POS = ARENA + 0x6200                 # position (three floats)
LODARGS = ARENA + 0x6300             # [0] = lod argument, [4] = clamp argument (producer)
INPUT = ARENA + 0x6400               # CarLodInputs (rebuilt selection): 15 dwords
RECS = ARENA + 0x7000                # car records, 0x40 bytes each
FRAME_LOD = {'ri': 0x58, 'owner': 0x18c, 'pixel': 0x60}   # frame cells the selection reads before writing
OUT_FRAME = {'searchIndex': 0x68, 'pixelFloat': 0x64, 'owner958': 0x180, 'ownerHas958': 0x14c, 'lod': 0x6c,
             'clamp': 0xf4, 'flagE8': 0xe8, 'flag1AC': 0x1ac}
INPUT_FIELDS = ['ri', 'pixel', 'lodIn', 'clampIn', 'arg8', 'searchIndex', 'pixelFloat', 'owner958', 'ownerHas958',
                'lod', 'clamp', 'flagE8', 'flag1AC', 'arg8Out', 'exitDraw']

STUBS = {}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def special_pixel(rng):
    return rng.choice((4, 5, 17, 18, 19, 20, 21, 29, 30, 31, 39, 40, 41, 100, 0x7fffffff, rng.randint(0, 200)))


class Machine:
    """One Unicorn run: the image, the arena and the stack. The original code runs as image bytes."""

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
        uc.mem_write(STOP, b'\x90')
        self.exited = False
        uc.hook_add(UC_HOOK_CODE, self._exit_hook, begin=EXIT_DRAW, end=EXIT_DRAW)

    def _exit_hook(self, uc, address, size, user):
        self.exited = True
        uc.emu_stop()


def write_fixture(m, case):
    """Writes the arena fixture and the image globals for one case (both sides get the same bytes)."""
    uc = m.uc
    ri = bytearray(0x3000)
    struct.pack_into('<I', ri, 0x0, case['owner'])
    struct.pack_into('<I', ri, 0x4, CAR)
    struct.pack_into('<I', ri, 0x8, ROW)
    ri[0x2eb8] = case['flag2eb8']
    uc.mem_write(RI, bytes(ri))
    car = bytearray(0x1000)
    car[7] = case['carByte'] & 0xff
    uc.mem_write(CAR, bytes(car))
    row = bytearray(0x1000)
    struct.pack_into('<I', row, 0x840, case['rowKey'])
    uc.mem_write(ROW, bytes(row))
    owner = bytearray(0x1000)
    struct.pack_into('<I', owner, 0x424, case['owner424'])
    struct.pack_into('<I', owner, 0x958, case['owner958'])
    uc.mem_write(OWNER, bytes(owner))
    recs = bytearray(0x40 * 8)
    for i, (key, value) in enumerate(case['records']):
        struct.pack_into('<II', recs, 0x40 * i, key & 0xffffffff, value & 0xffffffff)
    uc.mem_write(RECS, bytes(recs))
    uc.mem_write(G_RECORDS, struct.pack('<I', RECS))
    uc.mem_write(G_RECORD_COUNT, struct.pack('<i', case['count']))
    uc.mem_write(G_FLAG_E8, struct.pack('<i', case['flagE8Global']))
    uc.mem_write(G_OVERRIDE_LOD, struct.pack('<i', case['overrideLod']))
    uc.mem_write(G_OVERRIDE_CLAMP, struct.pack('<i', case['overrideClamp']))
    uc.mem_write(G_ZERO_FLAG, bytes([case['zeroFlag']]))
    uc.mem_write(G_VIEW_MODE, struct.pack('<i', case['viewMode']))
    uc.mem_write(G_GATE, struct.pack('<i', case['gate']))


def write_producer(m, case):
    uc = m.uc
    uc.mem_write(G_GATE, struct.pack('<i', case['gate']))
    uc.mem_write(G_VIEW_MODE, struct.pack('<i', case['viewMode']))
    view = bytearray(0x100)
    struct.pack_into('<f', view, 0xc, case['focal'])
    struct.pack_into('<I', view, 0x40, FRAMEV)
    uc.mem_write(VIEW, bytes(view))
    frame = bytearray(0x100)
    struct.pack_into('<3f', frame, 0x40, *case['origin'])
    struct.pack_into('<3f', frame, 0x50, *case['axis'])
    uc.mem_write(FRAMEV, bytes(frame))
    uc.mem_write(POS, struct.pack('<3f', *case['position']))
    uc.mem_write(LODARGS, struct.pack('<2i', case['lodArg'], case['clampArg']))


def set_original(m, case):
    uc = m.uc
    uc.reg_write(xc.UC_X86_REG_ESP, S_FRAME)
    uc.reg_write(xc.UC_X86_REG_EBP, EBP_ORIG)
    uc.reg_write(xc.UC_X86_REG_FPCW, case['fpcw'])
    uc.reg_write(xc.UC_X86_REG_EBX, 0)
    uc.reg_write(xc.UC_X86_REG_ESI, 0)
    uc.reg_write(xc.UC_X86_REG_EDI, 0)
    uc.reg_write(xc.UC_X86_REG_ECX, 0)
    uc.reg_write(xc.UC_X86_REG_EAX, 0)
    uc.reg_write(xc.UC_X86_REG_EDX, 0)


def run_original_producer(m, case):
    uc = m.uc
    write_producer(m, case)
    uc.mem_write(EBP_ORIG + 0x8, struct.pack('<I', VIEW))
    uc.mem_write(EBP_ORIG + 0x10, struct.pack('<I', POS))
    uc.mem_write(EBP_ORIG + 0x30, struct.pack('<i', case['clampArg']))
    uc.mem_write(EBP_ORIG + 0x34, struct.pack('<i', case['lodArg']))
    set_original(m, case)
    try:
        uc.emu_start(PRODUCER_START, PRODUCER_END, count=200000)
    except UcError as error:
        raise AssertionError(('original fixture fault', 'producer', str(error)))
    if m.exited:
        return (struct.unpack('<i', bytes(uc.mem_read(S_FRAME + 0x60, 4)))[0],
                struct.unpack('<i', bytes(uc.mem_read(EBP_ORIG + 0x34, 4)))[0],
                struct.unpack('<i', bytes(uc.mem_read(EBP_ORIG + 0x30, 4)))[0], True)
    return (struct.unpack('<i', bytes(uc.mem_read(S_FRAME + 0x60, 4)))[0],
            struct.unpack('<i', bytes(uc.mem_read(EBP_ORIG + 0x34, 4)))[0],
            struct.unpack('<i', bytes(uc.mem_read(EBP_ORIG + 0x30, 4)))[0], False)


def run_rebuilt_call(m, entry, args):
    """cdecl call of a rebuilt routine: the return address is STOP, the arguments follow it."""
    uc = m.uc
    esp = RSTK - 0x100
    uc.reg_write(xc.UC_X86_REG_ESP, esp)
    uc.reg_write(xc.UC_X86_REG_FPCW, args['fpcw'])
    uc.mem_write(esp, struct.pack('<I', STOP) + b''.join(struct.pack('<I', a & 0xffffffff) for a in args['words']))
    try:
        uc.emu_start(entry, STOP, count=200000)
    except UcError as error:
        raise AssertionError(('state or call order mismatch', 'rebuilt fault', str(error)))
    return uc.reg_read(xc.UC_X86_REG_EAX)


def run_original_select(m, case):
    uc = m.uc
    write_fixture(m, case)
    uc.mem_write(S_FRAME + FRAME_LOD['ri'], struct.pack('<I', RI))
    uc.mem_write(S_FRAME + FRAME_LOD['owner'], struct.pack('<I', case['owner']))
    uc.mem_write(S_FRAME + FRAME_LOD['pixel'], struct.pack('<i', case['pixel']))
    uc.mem_write(EBP_ORIG + 0x24, struct.pack('<I', case['arg8']))
    uc.mem_write(EBP_ORIG + 0x30, struct.pack('<i', case['clampIn']))
    uc.mem_write(EBP_ORIG + 0x34, struct.pack('<i', case['lodIn']))
    set_original(m, case)
    try:
        uc.emu_start(SELECT_START, SELECT_END, count=400000)
    except UcError as error:
        raise AssertionError(('original fixture fault', 'selection', str(error)))
    out = {}
    for name, off in OUT_FRAME.items():
        raw = bytes(uc.mem_read(S_FRAME + off, 4))
        out[name] = raw
    out['arg8Out'] = bytes(uc.mem_read(EBP_ORIG + 0x24, 4))
    out['exitDraw'] = m.exited
    return out


def compare_select(orig, rebuilt_in, case):
    """Compares the original frame cells with the fields of the rebuilt CarLodInputs (both as 32-bit words)."""
    rb = dict(zip(INPUT_FIELDS, rebuilt_in))
    checks = [
        ('searchIndex', orig['searchIndex'], rb['searchIndex'] & 0xffffffff),
        ('pixelFloat', orig['pixelFloat'], rb['pixelFloat'] & 0xffffffff),
        ('owner958', orig['owner958'], rb['owner958'] & 0xffffffff),
        ('ownerHas958', orig['ownerHas958'], rb['ownerHas958'] & 0xffffffff),
        ('lod', orig['lod'], rb['lod'] & 0xffffffff),
        ('clamp', orig['clamp'], rb['clamp'] & 0xffffffff),
        ('flagE8', orig['flagE8'], rb['flagE8'] & 0xffffffff),
        ('flag1AC', orig['flag1AC'], rb['flag1AC'] & 0xffffffff),
        ('arg8Out', orig['arg8Out'], rb['arg8Out'] & 0xffffffff),
    ]
    for name, original_bytes, value in checks:
        value_bytes = struct.pack('<I', value)    # raw words: the float pixel is compared by its bits
        if original_bytes != value_bytes:
            raise AssertionError(('state or call order mismatch', name, original_bytes.hex(), value_bytes.hex(), case['pixel'],
                                  case['lodIn'], case['clampIn']))
    if orig['exitDraw'] != bool(rb['exitDraw']):
        raise AssertionError(('state or call order mismatch', 'exit', orig['exitDraw'], rb['exitDraw'], case['pixel'],
                              case['lodIn'], case['clampIn']))


def random_producer_case(rng):
    case = {'fpcw': rng.choice((0x027f, 0x037f))}
    case['focal'] = rng.choice((rng.uniform(50.0, 2000.0), 300.0, 1.0, 0.0, -40.0))
    case['origin'] = [rng.uniform(-5, 5) for _ in range(3)]
    axis = [rng.uniform(-1, 1) for _ in range(3)]
    norm = max(1e-6, sum(a * a for a in axis) ** 0.5)
    case['axis'] = [f32(a / norm) for a in axis]
    if rng.random() < 0.6:
        # Positions along the axis at distances that give the behind, near and far cases of 0x48acb0.
        d = rng.choice((-5.0, -2.0, 0.0, 0.5, 2.0, 4.0, 5.0, 6.0, 9.0, 20.0, 60.0, 200.0, rng.uniform(4, 40),
                        rng.uniform(4, 40), rng.uniform(40, 150)))
        case['position'] = [f32(case['origin'][i] + case['axis'][i] * d + rng.uniform(-0.5, 0.5)) for i in range(3)]
    else:
        case['position'] = [f32(rng.uniform(-200, 200)) for _ in range(3)]
    case['gate'] = rng.choice((0, 0, 3, 6))
    case['viewMode'] = rng.choice((3, 3, 0, 1, 2))
    case['lodArg'] = rng.choice((0, 0, 1, 2, 3, -1, rng.randint(-2, 4)))
    case['clampArg'] = rng.choice((0, 0, 1, 2, 3, -1, rng.randint(-2, 4)))
    return case


def random_select_case(rng):
    case = {'fpcw': rng.choice((0x027f, 0x037f))}
    case['pixel'] = special_pixel(rng)
    case['lodIn'] = rng.choice((-1, 0, 0, 0, 1, 2, 3, 4))
    case['clampIn'] = rng.choice((-1, 0, 0, 1, 2, 3, 4))
    case['arg8'] = rng.choice((0x401, 0, 0x7fffffff, rng.getrandbits(32)))
    case['owner'] = rng.choice((0, OWNER, OWNER))
    case['owner424'] = rng.choice((0, 0, 1, 0x80000000))
    case['owner958'] = rng.choice((0, 0, 1, 0xffffffff, 0x12345678))
    case['flag2eb8'] = rng.choice((0, 1, 0x80, 1))
    case['carByte'] = rng.choice((0, 1, -1, 2, -3, rng.randint(-128, 127)))
    case['rowKey'] = rng.choice((0x12345678, 0xdeadbeef, 0x1, 0))
    case['count'] = rng.choice((0, 0, 1, 2, 3, 4))
    records = []
    for i in range(8):
        key = rng.choice((case['rowKey'], case['rowKey'], 0x55555555, 0))
        value = rng.choice((case['carByte'] & 0xffffffff, 0x7f, 0xffffffff, 0))
        records.append((key, value))
    case['records'] = records
    case['flagE8Global'] = rng.choice((1, 1, 0, 2, -1))
    case['overrideLod'] = rng.choice((-1, -1, -1, 0, 1, 2, 3, 4, -2))
    case['overrideClamp'] = rng.choice((-1, -1, -1, 0, 1, 2, 3, 4, -2))
    case['zeroFlag'] = rng.choice((0, 0, 0, 1))
    if rng.random() < 0.15:
        # The exit of the selection: both LOD values are -1 after the overrides (no record, no zero flag).
        case['lodIn'] = -1
        case['clampIn'] = -1
        case['overrideLod'] = -1
        case['overrideClamp'] = -1
        case['zeroFlag'] = 0
        case['pixel'] = rng.choice((5, 18, 20, 40, 150))
    case['viewMode'] = 0
    case['gate'] = 0
    return case


def check_manifest(image, manifest):
    for name, f in manifest['functions'].items():
        a = int(f['address'], 16) - IBASE
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256'], ('bytes changed', name)
    for addr, hexbytes in manifest['constants'].items():
        a = int(addr, 16) - IBASE
        assert image[a:a + len(hexbytes) // 2].hex() == hexbytes, ('constant changed', addr)


def build_rebuilt(source, out_name):
    obj = BUILD / out_name
    BUILD.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0',
                    '/EHsc', '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World', '/Fo' + str(obj.relative_to(ROOT)),
                    str(source.relative_to(ROOT) if source.is_relative_to(ROOT) else source)], cwd=ROOT, check=True)
    return obj


EXTERNALS = {
    'UG2CarViewModeGlobal': GETTER_VIEW_MODE, 'UG2CarLodZeroFlag': GETTER_ZERO_FLAG,
    'ug2WheelLodGate': G_GATE, 'ug2CarLodFlag7FEE4C': G_FLAG_E8,
    'ug2CarLodOverride802728': G_OVERRIDE_LOD, 'ug2CarLodOverride80272C': G_OVERRIDE_CLAMP,
    'ug2CarLodTable7A66DC': TABLE_LOD, 'ug2CarLodPixelLimit7A66FC': TABLE_PIXEL_LIMIT,
    'ug2CarLodScale7A6700': TABLE_SCALE, 'ug2CarLodRecords8A1CD4': G_RECORDS,
    'ug2CarLodRecordCount8A1CD8': G_RECORD_COUNT,
    '__ftol2': FTOL2, '__fltused': 0,
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
    for name in ('?CarLodSelect@@', '?CarLodPixelSize@@'):
        entries[name] = next(a for s, a in linked['exports'].items() if s.startswith(name))
    linked['entry_select'] = entries['?CarLodSelect@@']
    linked['entry_producer'] = entries['?CarLodPixelSize@@']
    linked['entry_radius'] = next(a for s, a in linked['exports'].items() if s.startswith('?CarViewPixelRadius@@'))
    return linked


def load_manifest():
    return json.loads((ROOT / 'config/SPEED2_EXE_1_2/car-draw-lod.json').read_text())


def run_cases(image, linked, n_producer, n_select, rng):
    counts = {'producer': 0, 'producer_exit': 0, 'producer_scaled': 0, 'radius_behind': 0, 'radius_near': 0,
              'radius_far': 0, 'select': 0, 'select_exit': 0, 'select_lod_ge1': 0, 'select_override': 0,
              'select_threshold_exact': 0}
    for _ in range(n_producer):
        case = random_producer_case(rng)
        orig = Machine(image)
        write_producer(orig, case)
        pixel_o, lod_o, clamp_o, exited_o = run_original_producer(orig, case)
        reb = Machine(image, linked)
        write_producer(reb, case)
        args = {'fpcw': case['fpcw'], 'words': [VIEW, POS, LODARGS, LODARGS + 4]}
        pixel_r = run_rebuilt_call(reb, linked['entry_producer'], args)
        pixel_r = struct.unpack('<i', struct.pack('<I', pixel_r & 0xffffffff))[0]
        lod_r = struct.unpack('<i', bytes(reb.uc.mem_read(LODARGS, 4)))[0]
        clamp_r = struct.unpack('<i', bytes(reb.uc.mem_read(LODARGS + 4, 4)))[0]
        if (pixel_o, lod_o, clamp_o) != (pixel_r, lod_r, clamp_r):
            raise AssertionError(('state or call order mismatch', 'producer', (pixel_o, lod_o, clamp_o),
                                  (pixel_r, lod_r, clamp_r), case))
        if exited_o != (pixel_o < 4):
            raise AssertionError(('state or call order mismatch', 'producer exit', exited_o, pixel_o))
        counts['producer'] += 1
        counts['producer_exit'] += 1 if exited_o else 0
        counts['producer_scaled'] += 1 if (case['viewMode'] == 3 and case['gate'] != 3) else 0
        # Classes of 0x48acb0 by the same tests: behind (depth < -3), near (distance - 3 <= 1.5), otherwise far.
        rel = [case['position'][i] - case['origin'][i] for i in range(3)]
        depth = sum(rel[i] * case['axis'][i] for i in range(3))
        dist = sum(v * v for v in rel) ** 0.5
        if -3.0 > depth:
            counts['radius_behind'] += 1
        elif dist - 3.0 <= 1.5:
            counts['radius_near'] += 1
        else:
            counts['radius_far'] += 1
    for _ in range(n_select):
        case = random_select_case(rng)
        orig = Machine(image)
        out = run_original_select(orig, case)
        reb = Machine(image, linked)
        write_fixture(reb, case)
        values = [RI, case['pixel'] & 0xffffffff, case['lodIn'] & 0xffffffff, case['clampIn'] & 0xffffffff,
                  case['arg8'], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        reb.uc.mem_write(INPUT, struct.pack('<15I', *[v & 0xffffffff for v in values]))
        run_rebuilt_call(reb, linked['entry_select'], {'fpcw': case['fpcw'], 'words': [INPUT]})
        fields = list(struct.unpack('<15I', bytes(reb.uc.mem_read(INPUT, 60))))
        rb = {name: fields[i] for i, name in enumerate(INPUT_FIELDS)}
        rebuilt_out = [rb[n] for n in INPUT_FIELDS]
        compare_select(out, rebuilt_out, case)
        counts['select'] += 1
        counts['select_exit'] += 1 if out['exitDraw'] else 0
        lod_value = struct.unpack('<i', out['lod'])[0]
        counts['select_lod_ge1'] += 1 if lod_value >= 1 else 0
        counts['select_override'] += 1 if (case['overrideLod'] != -1 or case['overrideClamp'] != -1) else 0
        counts['select_threshold_exact'] += 1 if case['pixel'] in (18, 20, 30, 40, 4) else 0
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--replace', help='compile this file instead of the recovered source')
    parser.add_argument('--producer-cases', type=int, default=512)
    parser.add_argument('--select-cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = load_manifest()
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    check_manifest(image, manifest)
    source = Path(args.replace) if args.replace else ROOT / SOURCE
    obj = build_rebuilt(source, 'replaced.obj' if args.replace else 'CarRenderLod.obj')
    linked = link_rebuilt(obj)
    rng = random.Random(0x620c6b)
    counts = run_cases(image, linked, args.producer_cases, args.select_cases, rng)
    report = dict(comparisons=counts, producer_cases=args.producer_cases, select_cases=args.select_cases,
                  compiler='MSVC 7.1 /Ox /Ob0 /EHsc',
                  compared=['pixel and LOD arguments (producer)', 'frame cells, arguments and exit (selection)'],
                  seams=['UG2CarViewModeGlobal 0x5b98a0 (image code)', 'UG2CarLodZeroFlag 0x499db0 (image code)',
                         '__ftol2 0x75bed4 (image code)'],
                  limitations=['random and special values; exhaustive float coverage not established',
                               'the table search reads the car records; the real records of the game are not in the fixture',
                               'the owner rules use the fixture owner; the game owner is not modelled',
                               'byte identity not established (compiler output differs from the original)'])
    if args.mutation_checks:
        text0 = (ROOT / SOURCE).read_text()
        rejected = []
        mutations = {
            'table-threshold-le': ('static_cast<double>(in->pixel) < static_cast<double>(ug2CarLodTable7A66DC[lod])',
                                   'static_cast<double>(in->pixel) <= static_cast<double>(ug2CarLodTable7A66DC[lod])'),
            'wheel-flag-le': ('static_cast<double>(in->pixel) < static_cast<double>(ug2CarLodPixelLimit7A66FC)',
                              'static_cast<double>(in->pixel) <= static_cast<double>(ug2CarLodPixelLimit7A66FC)'),
            'loop-float-compare': ('while (in->pixelFloat < ug2CarLodTable7A66DC[lod]);',
                                   'while (in->pixelFloat <= ug2CarLodTable7A66DC[lod]);'),
            'owner-rule-lod': ('lod = 2;', 'lod = 1;'),
            'arg8-threshold': ('if (lod >= 1) {\n        in->arg8Out |= 0x800u;', 'if (lod >= 2) {\n        in->arg8Out |= 0x800u;'),
            'exit-one-minus-one': ('if (lod == -1 && clamp == -1) {', 'if (lod == -1) {'),
            'override-swapped': ('        clamp = ug2CarLodOverride80272C;', '        clamp = ug2CarLodOverride802728;'),
            'zero-flag-clamp': ('        lod = 0;\n        clamp = 0;', '        lod = 0;'),
            'search-past-count': ('for (searchIndex = 0; searchIndex < count; ++searchIndex, record += 0x40) {',
                                  'for (searchIndex = 0; searchIndex <= count; ++searchIndex, record += 0x40) {'),
            'owner-has-958': ('ownerHas958 = owner958 != 0 ? 1u : 0u;', 'ownerHas958 = 1u;'),
            'scale-gate': ('ug2WheelLodGate != 3) {', 'ug2WheelLodGate != 6) {'),
            'scale-value': ('static_cast<double>(raw) * static_cast<double>(ug2CarLodScale7A6700)',
                            'static_cast<double>(raw) * 0.75'),
            'near-result': ('return static_cast<int>(2.0 * focal);', 'return static_cast<int>(focal);'),
            'behind-test': ('if (negRadius > depth) {', 'if (depth < 0.0) {'),
        }
        for label, (before, after) in mutations.items():
            if before not in text0:
                raise AssertionError(('mutation text missing', label))
            mutant = BUILD / ('mutation-' + label + '.cpp')
            mutant.write_text(text0.replace(before, after, 1).replace('#include "../Physics/', '#include "'))
            result = subprocess.run([sys.executable, __file__, '--replace', str(mutant), '--producer-cases', '128',
                                     '--select-cases', '512'], cwd=ROOT, text=True, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1200:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['comparisons'], indent=2))


if __name__ == '__main__':
    main()
