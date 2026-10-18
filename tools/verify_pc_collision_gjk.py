#!/usr/bin/env python3
"""Execute the original GJK shape-pair distance 0x5b3390 (CollisionWitness::ComputeDistance,
the body spans 0x5b3390..0x5b3d20 including its two switch jump tables) and the MSVC 7.1
(/Ox /Ob0) rebuild in Unicorn on synthetic shape pairs, and compare the x87 return value,
every integer field and every simplex float.

Only the GJK body differs between the two runs.  Every callee of the rebuilt object links to
the ORIGINAL routine: hull/polygon/box/point support (0x5afba0, 0x5a2060, 0x5a2280, 0x5a2360),
vertex dispatch 0x5aad20 (through a probe-only cdecl->thiscall shim, see below),
WheelTransform 0x43c9c0 (-> 0x441000), WheelNormalize 0x43cf10 and the simplex
Distance/CopyFrom/Assign 0x595040/0x598130/0x598240.  Probe-only seams:
  * 0x43bdf0 (bGetTicker, a kernel32 call) is hooked to return a constant on both sides; the
    original bRandom 0x43c1c0 runs and its seed 0x7ea624 is reset before every run.
  * the rebuilt source calls CollisionShapeVertex as a cdecl free function while the original
    0x5aad20 takes the shape in ECX (ret 8); a 14-byte shim adapts the rebuilt call.  This
    is a recorded calling-convention difference of the recovered source, not a behaviour one.
Cylinder shapes (type 5: 0x5a23d0/0x5a2780) are unrecovered and never generated.
Behavioural evidence only; nothing here claims byte matching."""
import hashlib
import math
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EBX,
                               UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG,
                               UC_X86_REG_GDTR, UC_X86_REG_CS, UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_FS)
from probe_pc_race_reward import ROOT, EXE
from pc_coff import link_objects, read_object

BUILD = ROOT/'build/analysis/collision'
SOURCE = 'src/Speed/Indep/Src/Physics/Collision/CollisionWitness.cpp'
ORIGINAL = (0x5b3390, 0x5b3d20)
SYMBOL = '?ComputeDistance@CollisionWitness@@QAEMXZ'
SHIM = 0x2008000
SEED_ADDRESS, SEED_VALUE, TICKER, TICKER_VALUE = 0x7ea624, 0x2468ace1, 0x43bdf0, 0x1234
SUPPORT = {'?Support@CollisionHull@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z': 0x5afba0,
           '?Support@CollisionPolygon@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z': 0x5a2060,
           '?Support@CollisionBox@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z': 0x5a2280,
           '?Support@CollisionPoint@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z': 0x5a2360,
           '?Support@CollisionCylinder@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z': 0x5a23d0}
EXTERNALS = dict(SUPPORT)
EXTERNALS.update({'?CollisionShapeVertex@@YAPAUCollisionVertex@@PAUCollisionShape@@HPAU1@@Z': SHIM,
                  '?WheelTransform@@YAXPAUWheelVector@@PAUWheelMatrix@@0@Z': 0x43c9c0,
                  '?WheelNormalize@@YAXPAUWheelVector@@0@Z': 0x43cf10,
                  '?Distance@CollisionSimplex@@QAEMPAUWheelVector@@@Z': 0x595040,
                  '?CopyFrom@CollisionSimplex@@QAEXPAU1@@Z': 0x598130,
                  '?Assign@CollisionSimplex@@QAEXPAU1@@Z': 0x598240,
                  '?collisionIntersectDistance@@3MA': 0x7a192c, '__fltused': 0})
# Executed original ranges (sanity guard only).
CALLEES = [(0x5afba0, 0x5aff57), (0x5a2060, 0x5a2273), (0x5a2280, 0x5a2359), (0x5a2360, 0x5a23c5),
           (0x5aad20, 0x5aad78), (0x5a1970, 0x5a19ec), (0x5987d0, 0x598837), (0x5a2010, 0x5a205a),
           (0x43c9c0, 0x43c9c5), (0x441000, 0x441064), (0x43cf10, 0x43cf81), (0x43d1e0, 0x43d21d),
           (0x43d0c0, 0x43d0f3), (0x595040, 0x595040+0x30d9), (0x598130, 0x598130+0x10b),
           (0x598240, 0x598240+0x14f), (0x43c1c0, 0x43c1ef), (0x59fb50, 0x59fb6e), (TICKER, TICKER+1)]
WITNESS, SHAPE_A, SHAPE_B, OUTPUT, STACK, STOP, WRAPPER = (0x2000000, 0x2001000, 0x2002000, 0x2003000, 0x200f000, 0x2100000, 0x2101000)
TEB, GDT = 0x2120000, 0x2130000
WITNESS_SIZE, SHAPE_SIZE = 0x240, 0xa00
HULL_VERTICES, HULL_NEIGHBOURS = 0x100, 0x800
SIMPLEX = 0x40
BOX_TYPES = {'hull': 1, 'polygon': 2, 'box': 3, 'point': 4}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def close(x, y):
    return x == y or abs(x-y) <= 1e-4*max(1.0, abs(x), abs(y)) or (x != x and y != y)


# ---------------------------------------------------------------- geometry helpers
def rotation(rng, tilt=1.0):
    axis = [rng.gauss(0, 1) for _ in range(3)]
    norm = math.sqrt(sum(a*a for a in axis)) or 1.0
    x, y, z = (a/norm for a in axis)
    angle = rng.uniform(-math.pi, math.pi)*tilt
    c, s = math.cos(angle), math.sin(angle)
    t = 1-c
    return [[t*x*x+c, t*x*y+s*z, t*x*z-s*y], [t*x*y-s*z, t*y*y+c, t*y*z+s*x], [t*x*z+s*y, t*y*z-s*x, t*z*z+c]]


def pose(rotation_rows, translation):
    """Row-vector WheelMatrix (transform = v*M + translation) and its rotation inverse."""
    m = [0.0]*16
    inv = [0.0]*16
    for r in range(3):
        for c in range(3):
            m[r*4+c] = rotation_rows[r][c]
            inv[r*4+c] = rotation_rows[c][r]
    for k in range(3):
        m[12+k] = translation[k]
    m[15] = inv[15] = 1.0
    return struct.pack('<16f', *m), struct.pack('<16f', *inv)


def apply(rotation_rows, translation, p):
    return [sum(p[r]*rotation_rows[r][c] for r in range(3))+translation[c] for c in range(3)]


def vertex_record(index, point, stamp=0, count=0, start=0):
    """0x30-byte CollisionVertex: counts +8/+0xa, index +0xc, stamp +0xe, point +0x10, dot +0x20."""
    return struct.pack('<IIhhhh', 0, 0, count, start, index, stamp)+struct.pack('<4f', point[0], point[1], point[2], 0.0)+bytes(0x10)


# ---------------------------------------------------------------- shapes (layouts read by the original support routines)
def make_box(rng, scale):
    mn = [rng.uniform(-scale, 0) for _ in range(3)]
    mx = [rng.uniform(0.05, scale) for _ in range(3)]
    data = bytearray(SHAPE_SIZE)
    struct.pack_into('<ii', data, 0x10, 3, 8)
    centre = [(mn[k]+mx[k])*0.5 for k in range(3)]
    extent = [(mx[k]-mn[k])*0.5 for k in range(3)]
    struct.pack_into('<3f', data, 0x20, *centre)
    struct.pack_into('<3f', data, 0x30, *extent)
    return dict(kind='box', data=data, vertices=8, size=extent, indices=list(range(8)))


def make_point(rng, scale):
    data = bytearray(SHAPE_SIZE)
    struct.pack_into('<ii', data, 0x10, 4, 1)
    point = [rng.uniform(-scale, scale)*rng.choice((0, 0, 0.3, 1)) for _ in range(3)]
    index = rng.randrange(0, 12)
    struct.pack_into('<h', data, 0x2c, index)
    struct.pack_into('<3f', data, 0x30, *point)
    return dict(kind='point', data=data, vertices=1, size=[0.1, 0.1, 0.1], indices=[index])


def make_polygon(rng, scale):
    """Track polygon in world space (matrices play no part in its support function)."""
    data = bytearray(SHAPE_SIZE)
    count = rng.choice((3, 4))
    normal = [rng.gauss(0, 1)*0.3, rng.gauss(0, 1)*0.3, 1.0] if rng.random() < 0.6 else [rng.gauss(0, 1) for _ in range(3)]
    n = math.sqrt(sum(a*a for a in normal)) or 1.0
    normal = [a/n for a in normal]
    helper = [1, 0, 0] if abs(normal[0]) < 0.9 else [0, 1, 0]
    u = [normal[1]*helper[2]-normal[2]*helper[1], normal[2]*helper[0]-normal[0]*helper[2], normal[0]*helper[1]-normal[1]*helper[0]]
    nu = math.sqrt(sum(a*a for a in u)); u = [a/nu for a in u]
    w = [normal[1]*u[2]-normal[2]*u[1], normal[2]*u[0]-normal[0]*u[2], normal[0]*u[1]-normal[1]*u[0]]
    centre = [rng.uniform(-scale, scale) for _ in range(3)]
    size = rng.uniform(1.0, scale*2+1)
    angles = sorted(rng.uniform(0, 2*math.pi) for _ in range(count))
    if count == 4:
        angles = [0.3+rng.uniform(-.2, .2), 1.9+rng.uniform(-.2, .2), 3.4+rng.uniform(-.2, .2), 4.9+rng.uniform(-.2, .2)]
    front = [[centre[k]+size*(math.cos(a)*u[k]+math.sin(a)*w[k]) for k in range(3)] for a in angles]
    thickness = rng.choice((0.0, 0.0, rng.uniform(0.05, 2.0)))
    thick = rng.choice((0, 1))
    vertices = list(front)
    if thickness > 0:
        vertices += [[p[k]-thickness*normal[k] for k in range(3)] for p in front]
    struct.pack_into('<ii', data, 0x10, 2, len(vertices))
    for i, p in enumerate(vertices):
        data[0x20+i*0x30:0x20+(i+1)*0x30] = vertex_record(i, p)
    data[0x1d0] = thick
    struct.pack_into('<f', data, 0x1d4, thickness)
    return dict(kind='polygon', data=data, vertices=len(vertices), size=[size]*3, indices=list(range(len(vertices))),
                thickness=thickness, thick=thick)


def make_hull(rng, scale):
    data = bytearray(SHAPE_SIZE)
    count = rng.choice((6, 8, 12, 17, 20, 24))
    radii = [rng.uniform(0.4, scale) for _ in range(3)]
    points = []
    for _ in range(count):
        d = [rng.gauss(0, 1) for _ in range(3)]
        n = math.sqrt(sum(a*a for a in d)) or 1.0
        points.append([radii[k]*d[k]/n for k in range(3)])
    struct.pack_into('<ii', data, 0x10, 1, count)
    shifted = rng.choice((0, 1))
    data[0x3e] = shifted
    struct.pack_into('<3f', data, 0x40, *[rng.uniform(-0.5, 0.5) for _ in range(3)])
    neighbours, records = [], []
    for i, p in enumerate(points):
        order = sorted((j for j in range(count) if j != i), key=lambda j: sum((points[j][k]-p[k])**2 for k in range(3)))
        mine = order[:rng.choice((3, 4, 5))]
        records.append(vertex_record(i, p, stamp=rng.choice((0, 0, rng.randrange(-30000, 30000))), count=len(mine), start=len(neighbours)))
        neighbours += mine
    for i, record in enumerate(records):
        data[HULL_VERTICES+i*0x30:HULL_VERTICES+(i+1)*0x30] = record
    data[HULL_NEIGHBOURS:HULL_NEIGHBOURS+2*len(neighbours)] = struct.pack('<%dh' % len(neighbours), *neighbours)
    return dict(kind='hull', data=data, vertices=count, size=radii, indices=list(range(count)), hull=True)


MAKERS = {'hull': make_hull, 'polygon': make_polygon, 'box': make_box, 'point': make_point}


def relocate_hull(shape, base):
    if shape.get('hull'):
        struct.pack_into('<II', shape['data'], 0x50, base+HULL_NEIGHBOURS, base+HULL_VERTICES)


def build_pair(rng):
    kinds = rng.choice([('hull', 'polygon'), ('polygon', 'hull'), ('box', 'polygon'), ('polygon', 'box'), ('point', 'polygon'),
                        ('hull', 'hull'), ('hull', 'box'), ('box', 'hull'), ('box', 'box'), ('point', 'box'), ('box', 'point'),
                        ('hull', 'point'), ('point', 'hull'), ('point', 'point'), ('polygon', 'polygon'), ('hull', 'polygon'),
                        ('box', 'polygon'), ('hull', 'hull'), ('box', 'box')])
    scale = rng.choice((0.8, 2.0, 4.0))
    shapes = [MAKERS[k](rng, scale) for k in kinds]
    placement = rng.choice(('far', 'near', 'touch', 'inside', 'inside', 'near'))
    reach = sum(max(s['size']) for s in shapes)
    poses = []
    for i, s in enumerate(shapes):
        if s['kind'] == 'polygon':
            poses.append((rotation(rng, 0.0), [0.0, 0.0, 0.0]))
            continue
        distance = {'far': reach*rng.uniform(2.0, 4.0), 'near': reach*rng.uniform(0.7, 1.6), 'touch': reach*rng.uniform(0.9, 1.1),
                    'inside': reach*rng.uniform(0.0, 0.5)}[placement]
        d = [rng.gauss(0, 1) for _ in range(3)]
        n = math.sqrt(sum(a*a for a in d)) or 1.0
        scalar = distance if (i == 1 or any(x['kind'] == 'polygon' for x in shapes)) else 0.0
        t = [scalar*d[k]/n for k in range(3)]
        poses.append((rotation(rng), t))
    return shapes, poses, placement


# ---------------------------------------------------------------- witness image
def witness_image(shapes, poses, started, simplex_state=None, rng=None):
    w = bytearray(WITNESS_SIZE)
    mats = []
    for (rot, trans) in poses:
        mats.append(pose(rot, trans))
    w[0x120:0x160], w[0x160:0x1a0] = mats[0]
    w[0x1a0:0x1e0], w[0x1e0:0x220] = mats[1]
    struct.pack_into('<II', w, 0x224, SHAPE_A, SHAPE_B)
    if simplex_state is not None:
        w[0x40:0x40+0xdc] = simplex_state
    else:
        # uninitialised in the real game: finite junk in the vertex slots, zero elsewhere
        for off in range(0, 0x80, 4):
            struct.pack_into('<f', w, 0x40+off, rng.uniform(-9, 9) if off % 16 != 12 else 0.0)
    struct.pack_into('<i', w, 0x118, started)
    return w


def warm_state(rng, shapes):
    """Synthetic warm start: valid distinct vertex indices of each shape."""
    state = bytearray(0xdc)
    for slot, (offset_vertices, offset_count, offset_indices) in enumerate(((0x00, 0xc0, 0xb0), (0x40, 0xd4, 0xc4))):
        shape = shapes[slot]
        count = rng.randint(1, min(4, len(shape['indices'])))
        indices = rng.sample(shape['indices'], count)
        struct.pack_into('<i', state, offset_count, count)
        for i, index in enumerate(indices):
            struct.pack_into('<i', state, offset_indices+i*4, index)
        for i in range(4):
            struct.pack_into('<3f', state, offset_vertices+i*16, *[rng.uniform(-4, 4) for _ in range(3)])
    return state


def main():
    raw = EXE.read_bytes()
    pe = pefile.PE(data=raw)
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD/'CollisionWitness.obj'
    subprocess.run([str(ROOT/'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7',
                    '/EHsc', '/Fo'+str(obj.relative_to(ROOT)), SOURCE], cwd=ROOT, check=True, capture_output=True)
    undefined = {s['name'] for s in read_object(obj)['symbols'].values() if s['section'] == 0 and s['storage'] == 2}
    externals = dict(EXTERNALS)
    unused = sorted(undefined-set(externals))   # referenced only by SetPoses/Test/Separate/...; never executed here
    for i, name in enumerate(unused):
        externals[name] = 0x7ff00000+i*16
    linked = link_objects([obj], externals)
    sizes = {s['name']: s['size'] for s in read_object(obj)['symbols'].values() if s['size']}
    entry_rebuilt = linked['exports'][SYMBOL]
    record = dict(original=hex(ORIGINAL[0]), original_extent=hex(ORIGINAL[1]-ORIGINAL[0]),
                  original_sha256=hashlib.sha256(image[ORIGINAL[0]-base:ORIGINAL[1]-base]).hexdigest(),
                  rebuilt_size=sizes[SYMBOL], byte_matching='not claimed', unused_links=len(unused))

    trace = None

    def make_machine(rebuilt):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(base, (len(image)+4095) & ~4095)
        uc.mem_write(base, image)
        uc.mem_map(0x2000000, 0x10000)
        uc.mem_map(0x2100000, 0x2000)
        # hull support 0x5afba0 installs an SEH frame through fs:[0]
        uc.mem_map(TEB, 0x1000)
        uc.mem_map(GDT, 0x1000)

        def descriptor(address, limit, access, flags=0xc):
            return ((limit & 0xffff) | ((address & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48)
                    | (flags << 52) | ((address >> 24) << 56))
        uc.mem_write(GDT+8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((UC_X86_REG_CS, 8), (UC_X86_REG_SS, 16), (UC_X86_REG_DS, 16), (UC_X86_REG_ES, 16), (UC_X86_REG_FS, 24)):
            uc.reg_write(reg, selector)
        uc.mem_write(TEB, struct.pack('<I', 0xffffffff))
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
        entry = entry_rebuilt if rebuilt else ORIGINAL[0]
        wrapper = (b'\xe8'+struct.pack('<i', entry-WRAPPER-5)+b'\xd9\x1d'+struct.pack('<I', OUTPUT)+b'\xc3')
        uc.mem_write(WRAPPER, wrapper)
        # cdecl CollisionShapeVertex(shape, index, output) -> thiscall 0x5aad20 (ret 8), then cdecl return.
        shim = (b'\x8b\x4c\x24\x04' b'\xff\x74\x24\x0c' b'\xff\x74\x24\x0c' b'\xe8'+struct.pack('<i', 0x5aad20-(SHIM+12)-5)+b'\xc3')
        uc.mem_write(SHIM, shim)
        ranges = list(CALLEES)+[(WRAPPER, WRAPPER+len(wrapper)), (SHIM, SHIM+len(shim))]
        ranges += linked['ranges'] if rebuilt else [ORIGINAL]

        def guard(machine, address, size, user):
            if address == TICKER:
                sp = machine.reg_read(UC_X86_REG_ESP)
                machine.reg_write(UC_X86_REG_EAX, TICKER_VALUE)
                machine.reg_write(UC_X86_REG_EIP, struct.unpack('<I', machine.mem_read(sp, 4))[0])
                machine.reg_write(UC_X86_REG_ESP, sp+4)
                return
            if trace is not None and address in (0x5afba0, 0x5a2060, 0x5a2280, 0x5a2360, 0x595040, 0x598130, 0x598240):
                sp = machine.reg_read(UC_X86_REG_ESP)
                pointer = struct.unpack('<I', machine.mem_read(sp+4, 4))[0]
                entry = (hex(address), struct.unpack('<3f', machine.mem_read(pointer, 12)) if address in (0x5afba0, 0x5a2060, 0x5a2280, 0x5a2360) else None)
                if address == 0x595040:
                    simplex = bytes(machine.mem_read(WITNESS+0x40, 0xdc))
                    entry += (struct.unpack_from('<i', simplex, 0xc0)[0], struct.unpack_from('<4i', simplex, 0xb0),
                              struct.unpack_from('<i', simplex, 0xd4)[0], struct.unpack_from('<4i', simplex, 0xc4))
                trace.append(entry)
            if not any(start <= address < end for start, end in ranges):
                raise RuntimeError(f'Unexpected instruction at {address:#x}')
        uc.hook_add(UC_HOOK_CODE, guard)
        return uc
    machines = [make_machine(False), make_machine(True)]
    import os
    traced = int(os.environ['GJK_TRACE']) if 'GJK_TRACE' in os.environ else None

    def run(uc, shapes, w, control):
        for shape, address in zip(shapes, (SHAPE_A, SHAPE_B)):
            relocate_hull(shape, address)
            uc.mem_write(address, bytes(shape['data']))
        uc.mem_write(WITNESS, bytes(w))
        uc.mem_write(SEED_ADDRESS, struct.pack('<I', SEED_VALUE))
        uc.mem_write(OUTPUT, b'\xa5'*4)
        uc.mem_write(TEB, struct.pack('<I', 0xffffffff))
        uc.mem_write(STACK, struct.pack('<I', STOP))
        uc.reg_write(UC_X86_REG_ESP, STACK)
        uc.reg_write(UC_X86_REG_ECX, WITNESS)
        uc.reg_write(UC_X86_REG_FPSW, 0)
        uc.reg_write(UC_X86_REG_FPTAG, 0xffff)   # empty x87 stack (a faulted run may leave values)
        uc.reg_write(UC_X86_REG_FPCW, control)
        saved = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
        for reg in saved:
            uc.reg_write(reg, 0x12345678)
        try:
            uc.emu_start(WRAPPER, STOP, count=400000)
        except UcError as error:
            return ('fault', str(error)+' at eip '+hex(uc.reg_read(UC_X86_REG_EIP)), bytes(uc.mem_read(WITNESS, WITNESS_SIZE)))
        if uc.reg_read(UC_X86_REG_EIP) != STOP:
            return ('fault', 'instruction budget', bytes(uc.mem_read(WITNESS, WITNESS_SIZE)))
        assert all(uc.reg_read(reg) == 0x12345678 for reg in saved)
        assert uc.reg_read(UC_X86_REG_ESP) == STACK+4
        assert bytes(uc.mem_read(TEB, 4)) == b'\xff'*4, 'SEH chain not restored'
        assert (uc.reg_read(UC_X86_REG_FPSW) >> 11) & 7 == 0, 'x87 stack not balanced'
        return ('ok', dict(returned=bytes(uc.mem_read(OUTPUT, 4)), witness=bytes(uc.mem_read(WITNESS, WITNESS_SIZE)),
                           shapes=[bytes(uc.mem_read(a, SHAPE_SIZE)) for a in (SHAPE_A, SHAPE_B)]))

    def describe(witness):
        simplex = witness[0x40:0x40+0xdc]
        return dict(countA=struct.unpack_from('<i', simplex, 0xc0)[0], countB=struct.unpack_from('<i', simplex, 0xd4)[0],
                    idxA=struct.unpack_from('<4i', simplex, 0xb0), idxB=struct.unpack_from('<4i', simplex, 0xc4),
                    started=struct.unpack_from('<i', simplex, 0xd8)[0], normal=struct.unpack_from('<3f', simplex, 0xa0))

    def compare(first, second):
        ra, rb = struct.unpack('<f', first['returned'])[0], struct.unpack('<f', second['returned'])[0]
        if not close(ra, rb):
            return ('return', ra, rb)
        sa, sb = first['witness'], second['witness']
        for offset in range(0x40+0xb0, 0x40+0xdc, 4):
            if sa[offset:offset+4] != sb[offset:offset+4]:
                return ('integer', hex(offset-0x40), struct.unpack_from('<i', sa, offset)[0], struct.unpack_from('<i', sb, offset)[0],
                    (ra, describe(sa)), (rb, describe(sb)))
        for offset in range(0, 0xac, 4):
            if offset % 16 == 12:
                continue
            x, y = struct.unpack_from('<f', sa, 0x40+offset)[0], struct.unpack_from('<f', sb, 0x40+offset)[0]
            if not close(x, y):
                return ('float', hex(offset), x, y)
        for which, (x, y) in enumerate(zip(first['shapes'], second['shapes'])):
            if x != y:
                # hull vertices cache a per-call stamp and the dot product (+0x20) of the direction
                for offset in range(0, SHAPE_SIZE, 4):
                    if x[offset:offset+4] != y[offset:offset+4]:
                        fx, fy = struct.unpack_from('<f', x, offset)[0], struct.unpack_from('<f', y, offset)[0]
                        ix, iy = struct.unpack_from('<I', x, offset)[0], struct.unpack_from('<I', y, offset)[0]
                        # hull support leaves open-list links (+0/+4) to its own stack frame, whose depth differs
                        stack_link = (offset >= HULL_VERTICES and (offset-HULL_VERTICES) % 0x30 in (0, 4)
                                      and STACK-0x1000 <= ix <= STACK and STACK-0x1000 <= iy <= STACK)
                        dot = offset >= HULL_VERTICES and (offset-HULL_VERTICES) % 0x30 == 0x20 and close(fx, fy)
                        if not (stack_link or dot):
                            return ('shape memory', which, hex(offset), x[offset:offset+4].hex(), y[offset:offset+4].hex())
        return None

    rng = random.Random(0x5b3390)
    trials = int(sys.argv[1]) if len(sys.argv) > 1 else 1500
    stats = dict(cold=0, warm_chain=0, warm_synthetic=0, exact=0, both_fault=0, zero_return=0, positive_return=0, max_return_error=0.0)
    by_kind, by_control = {}, {}
    failures, faults = [], {}

    def check(label, shapes, w, control, kind):
        nonlocal trace
        results = []
        for uc in machines:
            trace = [] if traced == label[0] else None
            results.append(run(uc, shapes, w, control))
            if trace is not None:
                print(label, 'rebuilt' if uc is machines[1] else 'original')
                for entry in trace[:40]:
                    print('   ', entry)
        if results[0][0] == 'fault' and results[1][0] == 'fault':
            stats['both_fault'] += 1
            faults[(label[1], results[0][1])] = faults.get((label[1], results[0][1]), 0)+1
            return None
        if results[0][0] != results[1][0]:
            failures.append((label, 'fault mismatch', [r[0] if r[0] == 'fault' else 'ok' for r in results],
                             [describe(r[2] if r[0] == 'fault' else r[1]['witness']) for r in results]))
            return results[0][1] if results[0][0] == 'ok' else None
        a, b = results[0][1], results[1][1]
        stats[kind] += 1
        by_control[control] = by_control.get(control, 0)+1
        by_kind[label[1]] = by_kind.get(label[1], 0)+1
        if a == b:
            stats['exact'] += 1
        else:
            problem = compare(a, b)
            if problem:
                failures.append((label, problem))
        value = struct.unpack('<f', a['returned'])[0]
        other = struct.unpack('<f', b['returned'])[0]
        if value == value and other == other and value != other:
            stats['max_return_error'] = max(stats['max_return_error'], abs(value-other)/max(1.0, abs(value), abs(other)))
        stats['zero_return' if value == 0.0 else 'positive_return'] += 1
        return a

    for trial in range(trials):
        control = (0x027f, 0x037f)[trial & 1]
        shapes, poses, placement = build_pair(rng)
        names = shapes[0]['kind']+'/'+shapes[1]['kind']
        # cold start (started = 0)
        w = witness_image(shapes, poses, 0, rng=rng)
        first = check((trial, names, 'cold', placement, hex(control)), shapes, w, control, 'cold')
        # warm start chained from the original's own result, with the poses nudged
        state = first['witness'][0x40:0x40+0xdc] if first else None
        for step in range(2):
            if state is None or struct.unpack_from('<i', state, 0xd8)[0] == 0:
                break
            nudged = []
            for (rot, trans), shape in zip(poses, shapes):
                if shape['kind'] == 'polygon':
                    nudged.append((rot, trans))
                else:
                    nudged.append((rot, [t+rng.uniform(-0.15, 0.15) for t in trans]))
            poses = nudged
            w = witness_image(shapes, poses, 1, simplex_state=state)
            result = check((trial, names, f'warm-chain{step}', placement, hex(control)), shapes, w, control, 'warm_chain')
            state = result['witness'][0x40:0x40+0xdc] if result else None
        # synthetic warm start with valid but arbitrary indices
        w = witness_image(shapes, poses, 1, simplex_state=warm_state(rng, shapes))
        check((trial, names, 'warm-synthetic', placement, hex(control)), shapes, w, control, 'warm_synthetic')

    total = stats['cold']+stats['warm_chain']+stats['warm_synthetic']
    print({'function': record, 'trials': trials, 'compared': total, **stats, 'failures': len(failures),
           'control_words': by_control})
    print('pair kinds', dict(sorted(by_kind.items())))
    for fault, count in sorted(faults.items()):
        print('both-fault', fault, count)
    for failure in failures[:40]:
        print(failure)
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
