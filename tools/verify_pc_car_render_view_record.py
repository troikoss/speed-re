#!/usr/bin/env python3
"""Exact-output verification of 0x48a6a0 and the 0x48f660 preparation wrapper.

Uses the producer harness for ABI and original callee execution. --chain links
all recovered preparation/foundation/coefficient bodies; D3DX remains original.
Native activation and the car-draw upstream inputs are outside this verifier.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
import verify_pc_car_render_prepare as w
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/car-render-view-record'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-view-record.json'
SOURCE = w.WORLD_SOURCE
ROOTS = {'view': (0x48a6a0, '?CarRenderPrepareViewRecord@@', '_Z26CarRenderPrepareViewRecord'),
         'wrapper': (0x48f660, '?CarRenderPrepareRecord@@', '_Z22CarRenderPrepareRecord')}


def build(source, gcc, chain, kind):
    w.BUILD = BUILD
    linked = w.build(source, gcc, chain)
    prefix = ROOTS[kind][2 if gcc else 1]
    linked['entry'] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
    return linked


class Machine(w.Machine):
    def __init__(self, image, linked=None, gcc=False, chain=False, kind='view'):
        super().__init__(image, linked, gcc, chain)
        if linked is None:
            self.entry = ROOTS[kind][0]
        self.kind = kind
        self.u.hook_add(UC_HOOK_CODE, self.hook, user_data='matrix-copy', begin=0x4012b0, end=0x4012b0)
        if kind == 'wrapper':
            for label, orig, msvc, gccpre in (
                    ('world-producer', 0x48e6c0, '?CarRenderPrepareWorldRecord@@', '_Z27CarRenderPrepareWorldRecord'),
                    ('view-producer', 0x48a6a0, '?CarRenderPrepareViewRecord@@', '_Z26CarRenderPrepareViewRecord')):
                address = next(v for k, v in linked['exports'].items() if k.startswith(gccpre if gcc else msvc)) if linked else orig
                self.u.hook_add(UC_HOOK_CODE, self.hook, user_data=label, begin=address, end=address)

    def hook(self, u, address, size, kind):
        if kind == 'normalize':
            dst, src = self.args(2)
            self.events.append((kind, dst == src, bytes(u.mem_read(src, 16))))
        elif kind == 'matrix-copy':
            dst, src = self.args(2)
            self.events.append((kind, dst if w.ARENA <= dst < w.ARENA + w.SIZE else 'local', bytes(u.mem_read(src, 64))))
        elif kind == 'world-producer':
            self.events.append((kind, self.args(3)))
        elif kind == 'view-producer':
            self.events.append((kind, self.args(4)))
        else:
            super().hook(u, address, size, kind)

    def run_case(self, case, cw):
        self.u.mem_write(0x8654a4, struct.pack('<i', case['gate']))
        dst, world, matrixArg3, position = (case[k] for k in ('dst', 'world', 'matrixArg3', 'position'))
        if self.kind == 'view':
            arguments = [dst, world, matrixArg3, position]
        else:
            arguments = [dst, world, matrixArg3, position, 0xdead0000, w.SRC]
        result = super().run(case['data'], dst, world, cw, arguments, False)
        assert self.u.reg_read(xc.UC_X86_REG_EAX) == 1, 'record return mismatch'
        assert bytes(self.u.mem_read(0x8654a4, 4)) == struct.pack('<i', case['gate'])
        expected = {'direction': 1 + (self.kind == 'wrapper')}
        if position:
            expected['normalize'] = 1
        copies = int(bool(position and matrixArg3)) + 4 * (self.kind == 'wrapper')
        if copies:
            expected['matrix-copy'] = copies
        if self.kind == 'wrapper':
            expected.update({'copy': 1, 'coefficients': 1, 'world-producer': 1, 'view-producer': 1})
        assert Counter(e[0] for e in self.events) == Counter(expected)
        return result


def f32(bits):
    return struct.unpack('<f', struct.pack('<I', bits))[0]


def cases(n):
    rng = random.Random(0x48a6a0)
    near = [0.0, -0.0, 1e-10, 1.0, 2.78, 8.0, 9.0]
    for v in (2.78, 8.0):
        bits = struct.unpack('<I', struct.pack('<f', v))[0]
        near += [f32(bits - 1), f32(bits), f32(bits + 1)]
    for index in range(n):
        data = bytearray(w.SIZE)
        for j in range(w.SIZE // 4):
            struct.pack_into('<f', data, j * 4, rng.uniform(-3, 3))
        dst = w.SRC + rng.choice((-0x200, -16, -4, 0, 4, 16, 0x200))
        world = rng.choice((w.ARENA + 0x800, w.SRC, dst, dst + 0x40, dst + 0xc0))
        matrixArg3 = rng.choice((0, w.ARENA + 0x900, world, dst, dst + 0xe0))
        position = rng.choice((0, w.ARENA + 0x600, world + 0x30, dst + 0xc0))
        gate = rng.choice((0, 1, 3, 3, 6))
        if index < len(near) * 8:
            world, matrixArg3 = w.ARENA + 0x800, (w.ARENA + 0x900 if index & 1 else 0)
            position = w.ARENA + 0x600 if index & 2 else 0
            gate = 3 if index & 4 else 0
            identity = [float(j in (0, 5, 10, 15)) for j in range(16)]
            struct.pack_into('<16f', data, world - w.ARENA, *identity)
            struct.pack_into('<16f', data, 0x900, *identity)
            struct.pack_into('<3f', data, 0x600, near[index // 8], 0.0, 0.0)
        if index == len(near) * 8:
            # Product-order discriminator: huge + 2 - huge + 1 differs from
            # huge + 1 - huge + 2 under 53-bit x87 precision.
            world, matrixArg3, position = w.ARENA + 0x800, w.ARENA + 0x900, w.ARENA + 0x600
            left, right = [0.0] * 16, [0.0] * 16
            left[:4] = [2.0 ** 30, 1.0, -(2.0 ** 30), 2.0]
            right[0], right[4], right[8], right[12] = 2.0 ** 30, 1.0, 2.0 ** 30, 1.0
            struct.pack_into('<16f', data, 0x800, *left)
            struct.pack_into('<16f', data, 0x900, *right)
            struct.pack_into('<3f', data, 0x600, 1.0, 0.0, 0.0)
        yield dict(data=bytes(data), dst=dst, world=world, matrixArg3=matrixArg3, position=position, gate=gate)


def verify(image, linked, gcc, chain, kind, n):
    a, b = Machine(image, kind=kind), Machine(image, linked, gcc, chain, kind)
    counts = Counter()
    for case in cases(n):
        for cw in (0x027f, 0x037f):
            x, y = a.run_case(case, cw), b.run_case(case, cw)
            if x != y:
                offsets = sorted({j & ~3 for j, (u, v) in enumerate(zip(x[0], y[0])) if u != v})[:6]
                details = [(hex(j - (case['dst'] - w.ARENA)), struct.unpack_from('<f', x[0], j)[0],
                            struct.unpack_from('<f', y[0], j)[0]) for j in offsets]
                event_diff = [i for i, (u, v) in enumerate(zip(x[1], y[1])) if u != v]
                raise AssertionError(('view record mismatch', kind, counts['total'], hex(cw),
                                      {k: hex(v) if isinstance(v, int) else '-' for k, v in case.items() if k != 'data'},
                                      details, event_diff))
            counts['total'] += 1
            counts['position'] += int(bool(case['position']))
            counts['matrix'] += int(bool(case['position'] and case['matrixArg3']))
            counts['gate3'] += int(case['gate'] == 3)
    return dict(counts)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--chain', action='store_true')
    p.add_argument('--kind', choices=ROOTS, default='view')
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=4096)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for address, data in manifest['constants'].items():
        off = int(address, 16) - 0x400000
        assert image[off:off + len(data) // 2].hex() == data
    linked = build(args.replace or SOURCE, args.gcc, args.chain, args.kind)
    counts = verify(image, linked, args.gcc, args.chain, args.kind, args.cases)
    report = dict(comparisons=counts, failures=0, chain=args.chain, kind=args.kind, byte_identity='not established',
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['finite bounded matrices, records and aliases; no exhaustive coverage',
                               'car-draw upstream inputs and native activation are not covered',
                               'D3DX default scalar transpose selected; CPU dispatch not tested'])
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'wrong-clamp-gate': ('ug2WheelLodGate == 3', 'ug2WheelLodGate == 6'),
            'wrong-upper-bound': ('maximum78D794 = 8.0f;', 'maximum78D794 = 7.0f;'),
            'wrong-lower-bound': ('minimum78D788 = 2.78f;', 'minimum78D788 = 2.0f;'),
            'omit-normalize': ('CarRenderNormalize4(point, point);', ''),
            'wrong-position-w': ('output->valueC0[3] = 1.0f;', 'output->valueC0[3] = 0.0f;'),
            'matrix-without-position': ('if (matrixArg3 && position)', 'if (matrixArg3)'),
            'wrong-product-order': ('static_cast<long double>(world->elements[3]) * matrixArg3->elements[12]) +\n             static_cast<long double>(world->elements[2]) * matrixArg3->elements[8]) +\n            static_cast<long double>(world->elements[1]) * matrixArg3->elements[4];',
                                    'static_cast<long double>(world->elements[1]) * matrixArg3->elements[4]) +\n             static_cast<long double>(world->elements[2]) * matrixArg3->elements[8]) +\n            static_cast<long double>(world->elements[3]) * matrixArg3->elements[12];'),
            'keep-matrix-w': ('dst[15] = 0;', 'dst[15] = 0x3f800000;'),
        }
        if args.kind == 'wrapper':
            mutations.update({
                'omit-world-producer': ('CarRenderPrepareWorldRecord(output, source, world);', ''),
                'omit-view-producer': ('CarRenderPrepareViewRecord(output, world, matrixArg3, position);', ''),
                'wrong-wrapper-return': ('CarRenderPrepareViewRecord(output, world, matrixArg3, position);\n    return 1;',
                                         'CarRenderPrepareViewRecord(output, world, matrixArg3, position);\n    return 0;'),
            })
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after, 1))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '256', '--kind', args.kind] +
                               (['--gcc'] if args.gcc else []) + (['--chain'] if args.chain else []),
                               text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and ('view record mismatch' in r.stdout or 'self.events' in r.stdout or 'record return mismatch' in r.stdout), (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + ('-chain' if args.chain else '-isolated') + '-' + args.kind
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
