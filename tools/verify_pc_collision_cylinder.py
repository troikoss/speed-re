#!/usr/bin/env python3
"""Execute the original cylinder shape routines CollisionCylinder::Support 0x5a23d0..0x5a277d
and CollisionCylinder::GetVertex 0x5a2780..0x5a28f3 and the MSVC 7.1 (/Ox /Ob0) rebuild from
CollisionShape.cpp in Unicorn on random cylinders (all three axis rows, varied radius, half
height and centre), random poses with inverses and random, axis-aligned, zero, +-0.0, NaN and
ring-boundary directions, under 53- and 64-bit x87 precision, and compare the whole 0x30-byte
output vertex record, EAX and the callee-saved registers byte for byte.  GetVertex is run for
every index 0..127.

Only the two routines differ between the runs: WheelTransform 0x43c9c0 (-> 0x441000), the
float->int cast __ftol 0x75bed4 (compiled in-line by MSVC as a call to it) and the constants and
axis table at 0x784250, 0x7d7d3c, 0x7a2070, 0x7a2074 and 0x7a1930 are the ORIGINAL ones in both runs.
Instruction coverage of the original routines is reported.  Behavioural evidence only; nothing
here claims byte matching.  --mutation-checks recompiles deliberately broken sources and requires
each to be rejected."""
import argparse
import hashlib
import math
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EBX,
                               UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)
from probe_pc_race_reward import ROOT, EXE
from pc_coff import link_objects, read_object
from verify_pc_collision_gjk import rotation, pose

BUILD = ROOT/'build/analysis/collision-cylinder'
SOURCE = ROOT/'src/Speed/Indep/Src/Physics/Collision/CollisionShape.cpp'
SUPPORT = (0x5a23d0, 0x5a277d)
GETVERTEX = (0x5a2780, 0x5a28f3)
SYMBOLS = {'Support': '?Support@CollisionCylinder@@QAEPAUCollisionVertex@@PAUWheelVector@@PAU2@PAUWheelMatrix@@21@Z',
           'GetVertex': '?GetVertex@CollisionCylinder@@QAEXHPAUCollisionVertex@@@Z'}
EXTERNALS = {'?WheelTransform@@YAXPAUWheelVector@@PAUWheelMatrix@@0@Z': 0x43c9c0,
             '?collisionOne784250@@3MA': 0x784250, '?collisionZero7D7D3C@@3MA': 0x7d7d3c,
             '?collisionCylinderStep7A2070@@3MA': 0x7a2070, '?collisionCylinderStepScale7A2074@@3MA': 0x7a2074,
             '?collisionCylinderAxes@@3PAY02HA': 0x7a1930, '__ftol': 0x75bed4, '__ftol2': 0x75bed4, '__fltused': 0}
# Executed original callees (sanity guard only): WheelTransform, its body, __ftol.
CALLEES = [(0x43c9c0, 0x43c9c5), (0x441000, 0x441064), (0x75bed4, 0x75bf49)]
SHAPE, MATRIX, INVERSE, DIRECTION, OUTPUT, STACK, STOP, WRAPPER = (0x2000000, 0x2000100, 0x2000200, 0x2000300, 0x2000400,
                                                                   0x200f000, 0x2100000, 0x2101000)
FILL = b'\xa5'*0x30
TAIL = 0x1c0
MISSING = object()


def f32(x):
    try:
        return struct.unpack('<f', struct.pack('<f', x))[0]
    except OverflowError:
        return math.copysign(math.inf, x)


def shape_image(rng, axis, radius, half, centre):
    data = bytearray(0x40)
    struct.pack_into('<ii', data, 0x10, 5, 0)
    struct.pack_into('<ff', data, 0x18, half, radius)
    struct.pack_into('<3f', data, 0x20, *centre)
    struct.pack_into('<i', data, 0x30, axis)
    return bytes(data)


def build(source, name, includes=True):
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD/(name+'.obj')
    command = [str(ROOT/'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc',
               '/I', 'src/Speed/Indep/Src/Physics/Collision', '/Fo'+str(obj.relative_to(ROOT)), str(Path(source).resolve().relative_to(ROOT))]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        print(result.stdout, result.stderr)
        raise SystemExit('compile failed')
    undefined = {s['name'] for s in read_object(obj)['symbols'].values() if s['section'] == 0 and s['storage'] == 2}
    externals = dict(EXTERNALS)
    for i, symbol in enumerate(sorted(undefined-set(externals))):   # referenced only by the other shapes; never executed
        externals[symbol] = 0x7ff00000+i*16
    linked = link_objects([obj], externals)
    sizes = {s['name']: s['size'] for s in read_object(obj)['symbols'].values() if s['size']}
    return linked, sizes


def make_machine(image, base, linked, rebuilt, entries, coverage):
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, (len(image)+4095) & ~4095)
    uc.mem_write(base, image)
    uc.mem_map(0x2000000, 0x10000)
    uc.mem_map(0x2100000, 0x2000)
    uc.mem_map(linked['address'], linked['allocation'])
    for address, body in linked['blocks']:
        uc.mem_write(address, body)
    # Probe-only wrapper: thiscall with ECX = shape, arguments pushed by the caller (cdecl-style table).
    wrappers = {}
    cursor = WRAPPER
    for name in ('Support', 'GetVertex'):
        entry = entries[name]
        if name == 'Support':
            # push output, inverse, matrix, start(0), direction ; ecx = shape
            prefix = (b'\x68'+struct.pack('<I', OUTPUT)+b'\x68'+struct.pack('<I', INVERSE)+b'\x68'+struct.pack('<I', MATRIX)
                      + b'\x6a\x00'+b'\x68'+struct.pack('<I', DIRECTION)+b'\xb9'+struct.pack('<I', SHAPE))
        else:
            # push output, index (read from the probe slot at DIRECTION) ; ecx = shape
            prefix = (b'\x68'+struct.pack('<I', OUTPUT)+b'\xff\x35'+struct.pack('<I', DIRECTION)+b'\xb9'+struct.pack('<I', SHAPE))
        code = prefix+b'\xe8'+struct.pack('<i', entry-(cursor+len(prefix))-5)+b'\xc3'
        uc.mem_write(cursor, code)
        wrappers[name] = (cursor, len(code))
        cursor += 0x100
    ranges = list(CALLEES)+[(a, a+n) for a, n in wrappers.values()]
    ranges += linked['ranges'] if rebuilt else [SUPPORT, GETVERTEX]

    def guard(machine, address, size, user):
        if coverage is not None:
            coverage.add(address)
        if not any(start <= address < end for start, end in ranges):
            raise RuntimeError(f'Unexpected instruction at {address:#x}')
    uc.hook_add(UC_HOOK_CODE, guard)
    return uc, {name: a for name, (a, n) in wrappers.items()}


def run(uc, wrapper, shape, matrix, inverse, direction, control):
    uc.mem_write(SHAPE, shape)
    uc.mem_write(MATRIX, matrix)
    uc.mem_write(INVERSE, inverse)
    uc.mem_write(DIRECTION, direction)
    uc.mem_write(OUTPUT, FILL+b'\x5a'*16)
    uc.mem_write(STACK, struct.pack('<I', STOP))
    uc.reg_write(UC_X86_REG_ESP, STACK)
    uc.reg_write(UC_X86_REG_FPSW, 0)
    uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    uc.reg_write(UC_X86_REG_FPCW, control)
    saved = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
    for reg in saved:
        uc.reg_write(reg, 0x12345678)
    try:
        uc.emu_start(wrapper, STOP, count=20000)
    except UcError as error:
        return ('fault', str(error)+' eip '+hex(uc.reg_read(UC_X86_REG_EIP)))
    if uc.reg_read(UC_X86_REG_EIP) != STOP:
        return ('fault', 'instruction budget')
    assert all(uc.reg_read(reg) == 0x12345678 for reg in saved), 'callee-saved register clobbered'
    assert uc.reg_read(UC_X86_REG_ESP) == STACK+4, 'stack not balanced'
    assert (uc.reg_read(UC_X86_REG_FPSW) >> 11) & 7 == 0, 'x87 stack not balanced'
    return ('ok', bytes(uc.mem_read(OUTPUT, 0x40)), uc.reg_read(UC_X86_REG_EAX))


def special(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-30, -1e-30, 3e-8, 1e6, -1e6, 1e19, float('nan')))


def ring_direction(rng):
    """Local (a, b) pair on, next to or between ring samples, so the d1 == d2 comparison and the
    +-7 clamps are exercised; scaled by a random magnitude."""
    step = f32(0.08838834764831845)
    k = rng.randrange(-9, 10)
    offset = rng.choice((0.0, 0.5, 0.5, 0.499, 0.501, 0.25, -0.5, 0.0, 0.999, 1.0))
    s = max(-1.0, min(1.0, (k+offset)*step))
    c = math.sqrt(max(0.0, 1-s*s))*rng.choice((1, -1))
    scale = rng.choice((1.0, 1.0, 0.001, 37.5, 1e8))
    return [s*scale, c*scale] if rng.random() < 0.5 else [c*scale, s*scale]


AXES = ((1, 2, 0), (0, 2, 1), (0, 1, 2))   # original table 0x7a1930


def local_direction(rng, axis):
    mode = rng.choice((0, 1, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7))
    if mode == 0:
        return [rng.gauss(0, 1) for _ in range(3)]
    if mode == 1:
        v = [0.0, 0.0, 0.0]
        v[rng.randrange(3)] = rng.choice((1.0, -1.0, 2.5, -0.0))
        return [x if x != -0.0 else -0.0 for x in v]
    if mode == 2:
        return [special(rng) if rng.random() < 0.5 else rng.gauss(0, 1) for _ in range(3)]
    if mode == 3:   # exactly equal magnitudes on the ring axes (|a| == |b|)
        m = rng.choice((1.0, 0.3, 7.0))
        v = [0.0, 0.0, 0.0]
        v[AXES[axis][0]], v[AXES[axis][1]] = m*rng.choice((1, -1)), m*rng.choice((1, -1))
        v[AXES[axis][2]] = rng.gauss(0, 1)
        return v
    if mode == 4 and rng.random() < 0.5:
        # |a| and |b| a few float ulps apart.  The +-7 clamps (0x5a2491, 0x5a24a4, 0x5a25e4, 0x5a25f7) need the
        # float-rounded b/|v| * 11.3137 to reach 8.0; searching this neighbourhood (400k pairs) tops out at
        # 7.9999997, so they are defensive code and stay unreached.
        m = f32(rng.uniform(0.2, 9.0))
        bits = struct.unpack('<i', struct.pack('<f', m))[0]
        other = struct.unpack('<f', struct.pack('<i', bits+rng.choice((-3, -2, -1, 1, 2, 3))))[0]
        sa, sb = rng.choice((1, -1)), rng.choice((1, -1))
        v = [0.0, 0.0, 0.0]
        pair = (m*sa, other*sb) if rng.random() < 0.5 else (other*sa, m*sb)
        v[AXES[axis][0]], v[AXES[axis][1]] = pair
        v[AXES[axis][2]] = rng.gauss(0, 1)
        return v
    if mode == 4:
        return [0.0, 0.0, 0.0]
    if mode == 5:
        return [rng.choice((0.0, -0.0)), rng.choice((0.0, -0.0)), rng.choice((0.0, -0.0, 1.0, -1.0))]
    if mode == 6:   # near-boundary ring sample in an arbitrary pair of axes with a random height component
        pair = ring_direction(rng)
        v = [0.0, 0.0, 0.0]
        v[AXES[axis][0]], v[AXES[axis][1]] = pair
        v[AXES[axis][2]] = rng.choice((0.0, -0.0, 1.0, -1.0, rng.gauss(0, 1)))
        return v
    return [rng.gauss(0, 1)*rng.choice((1e-3, 1, 1e3)) for _ in range(3)]


def case(rng):
    axis = rng.randrange(3)
    radius = rng.choice((1.0, 0.5, rng.uniform(0.01, 5.0), rng.uniform(0.01, 5.0), 0.0))
    half = rng.choice((1.0, rng.uniform(0.01, 4.0), rng.uniform(0.01, 4.0), 0.0))
    centre = [rng.choice((0.0, rng.uniform(-3, 3))) for _ in range(3)]
    shape = shape_image(rng, axis, f32(radius), f32(half), [f32(c) for c in centre])
    if rng.random() < 0.5:
        # identity inverse: the local direction is the world direction, so the ring/axis
        # arrangement of `local_direction` reaches the routine as generated
        rot = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
    else:
        rot = rotation(rng)
    matrix, _ = pose(rot, [rng.uniform(-5, 5) for _ in range(3)])
    inv_rot = [[rot[c][r] for c in range(3)] for r in range(3)]
    _, inverse = pose(rot, [0.0, 0.0, 0.0])
    local = local_direction(rng, axis)
    # world direction whose inverse transform is `local` (row-vector: local = world * inv_rot)
    world = [sum(local[r]*rot[c][r] for r in range(3)) for c in range(3)] if rot != [[1, 0, 0], [0, 1, 0], [0, 0, 1]] else local
    if rng.random() < 0.15:
        inverse = bytes(struct.pack('<16f', *[f32(rng.uniform(-2, 2)) for _ in range(16)]))   # arbitrary inverse
        world = [rng.gauss(0, 1) for _ in range(3)]
    direction = struct.pack('<4f', *[f32(x) for x in world], 0.0)
    return shape, matrix, inverse, direction, axis


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=6000)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    pe = pefile.PE(data=EXE.read_bytes())
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    linked, sizes = build(args.source, 'CollisionShape')
    covered = set()
    entries_original = {'Support': SUPPORT[0], 'GetVertex': GETVERTEX[0]}
    entries_rebuilt = {name: linked['exports'][symbol] for name, symbol in SYMBOLS.items()}
    original, wrap_original = make_machine(image, base, linked, False, entries_original, covered)
    rebuilt, wrap_rebuilt = make_machine(image, base, linked, True, entries_rebuilt, None)
    rng = random.Random(0x5a23d0)
    failures, counts, both_fault = [], {'Support': 0, 'GetVertex': 0}, 0
    outcomes = {'quadrant': {}, 'cap': {'positive': 0, 'negative': 0}}
    axes_seen, controls_seen, indices_seen = set(), set(), set()
    exact_nan = 0

    def compare(label, a, b):
        nonlocal both_fault
        if a[0] == 'fault' and b[0] == 'fault':
            both_fault += 1
            return
        if a != b:
            failures.append((label, a if a[0] == 'fault' else (a[0], a[1][:0x20].hex(), hex(a[2])),
                             b if b[0] == 'fault' else (b[0], b[1][:0x20].hex(), hex(b[2]))))
            return
        counts[label[0]] += 1
        if a[1][:0x30] == FILL and label[0] == 'Support':
            failures.append((label, 'support wrote nothing'))

    for trial in range(args.cases):
        shape, matrix, inverse, direction, axis = case(rng)
        control = (0x027f, 0x037f)[trial & 1]
        axes_seen.add(axis)
        controls_seen.add(control)
        a = run(original, wrap_original['Support'], shape, matrix, inverse, direction, control)
        b = run(rebuilt, wrap_rebuilt['Support'], shape, matrix, inverse, direction, control)
        label = ('Support', trial, axis, hex(control), struct.unpack('<3f', direction[:12]))
        compare(label, a, b)
        if a[0] == 'ok':
            index = struct.unpack_from('<h', a[1], 0xc)[0]
            indices_seen.add(index)
            outcomes['cap']['positive' if index & 0x40 else 'negative'] += 1
            ring = index & 0x3f
            quadrant = 'q%d' % (0 if ring <= 0x10 else 1 if ring <= 0x20 else 2 if ring <= 0x30 else 3)
            outcomes['quadrant'][quadrant] = outcomes['quadrant'].get(quadrant, 0)+1
            if a[2] != OUTPUT:
                failures.append((label, 'EAX is not the output pointer', hex(a[2])))
    for trial in range(args.cases//10+1):
        shape, matrix, inverse, _, axis = case(rng)
        for index in range(128):
            for control in (0x027f, 0x037f):
                direction = struct.pack('<i', index)+bytes(12)
                a = run(original, wrap_original['GetVertex'], shape, matrix, inverse, direction, control)
                b = run(rebuilt, wrap_rebuilt['GetVertex'], shape, matrix, inverse, direction, control)
                compare(('GetVertex', trial, axis, hex(control), index), a, b)
                if a[0] == 'ok' and a[1][:0xc] != FILL[:0xc]:
                    failures.append((('GetVertex', trial, index), 'GetVertex wrote before +0xc'))
    # Instruction coverage of the original routines.
    code = image[SUPPORT[0]-base:GETVERTEX[1]-base]
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end in (SUPPORT, GETVERTEX):
        for insn in md.disasm(image[start-base:end-base], start):
            instructions[insn.address] = insn.mnemonic
    unreached = sorted(a for a in instructions if a not in covered)
    record = {name: dict(original_sha256=hashlib.sha256(image[lo-base:hi-base]).hexdigest(),
                         original_size=hi-lo, rebuilt_size=sizes[SYMBOLS[name]], byte_matching='not claimed')
              for name, (lo, hi) in (('Support', SUPPORT), ('GetVertex', GETVERTEX))}
    report = dict(compared=counts, both_fault=both_fault, failures=len(failures), x87_control_words=sorted(map(hex, controls_seen)),
                  axes=sorted(axes_seen), support_outcomes=outcomes, distinct_support_indices=len(indices_seen),
                  instructions_total=len(instructions), instructions_reached=len(instructions)-len(unreached),
                  unreached=[hex(a) for a in unreached], functions=record)
    for name, value in report.items():
        print(name, value)
    for failure in failures[:20]:
        print('FAIL', failure)
    status = 1 if failures else 0
    if args.mutation_checks and not failures:
        mutations = {
            'support-ring-compare-flip': ('if (cosine2 * fabs(local[a]) + sine2 * local[b] < cosine1 * fabs(local[a]) + sine1 * local[b])',
                                          'if (cosine2 * fabs(local[a]) + sine2 * local[b] > cosine1 * fabs(local[a]) + sine1 * local[b])'),
            'support-axis-tie': ('if (fabs(local[a]) > fabs(local[b])) {', 'if (fabs(local[a]) >= fabs(local[b])) {'),
            'support-cap-sign': ('if (local[c] > collisionZero7D7D3C) {', 'if (local[c] >= collisionZero7D7D3C) {'),
            'support-index': ('index = 0x18 - i;', 'index = 0x19 - i;'),
            'support-centre-order': ('point[0] = point[0] + centre20.x;', 'point[0] = point[0] - centre20.x;'),
            'support-output-w': ('output->point10[3] = 0.0f;\n    output->indexC = (short)index;', 'output->point10[3] = 1.0f;\n    output->indexC = (short)index;'),
            'getvertex-quadrant': ('float sine = (index - 0x28) * collisionCylinderStep7A2070;', 'float sine = (index - 0x27) * collisionCylinderStep7A2070;'),
            'getvertex-writes-index': ('    output->point10[3] = 0.0f;\n    float x = local[0]', '    output->point10[3] = 0.0f;\n    output->indexC = (short)index;\n    float x = local[0]'),
            'getvertex-cap': ('        local[row[2]] = halfHeight18;\n        index -= 0x40;', '        local[row[2]] = -halfHeight18;\n        index -= 0x40;'),
        }
        text = args.source.read_text()
        rejected = []
        for label, (before, after) in mutations.items():
            assert before in text, label
            mutant = BUILD/('mutation-'+label+'.cpp')
            mutant.write_text(text.replace(before, after, 1))
            result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '1500'], cwd=ROOT / 'tools',
                                    text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if result.returncode == 0:
                print('MUTATION SURVIVED', label)
                status = 1
            else:
                rejected.append(label)
        print('rejected_mutations', rejected)
    return status


if __name__ == '__main__':
    sys.exit(main())
