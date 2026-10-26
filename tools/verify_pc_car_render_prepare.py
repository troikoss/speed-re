#!/usr/bin/env python3
"""Compare world render-record producer 0x48e6c0 with the PC v1.2 original.

Isolated mode executes original callees; --chain links the recovered record
foundations and coefficient transform. Exact fixture and ordered call-input checks.
GCC-to-original thiscall adapters change ABI only, never data or arithmetic.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

WORLD_SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecordPrepare.cpp'
FOUNDATION = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecord.cpp'
COEFFICIENTS = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecordTransform.cpp'
BUILD = ROOT / 'build/analysis/car-render-prepare'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-prepare.json'
ARENA, SIZE, STACK, STOP, BRIDGE = 0x2000000, 0x4000, 0x2100000, 0x2110000, 0x2300000
SRC = ARENA + 0x1000
GCC_ENTRY = '_Z27CarRenderPrepareWorldRecordP15CarRenderRecordS0_P11WheelMatrix'
PREFIX = {'copy': '?CopyFrom@CarRenderRecord@@', 'direction': '?CarRenderDirectionMatrix@@',
          'coefficients': '?TransformCoefficients@CarRenderRecord@@', 'matrix': '?BodyMatrixCopy@@', 'normalize': '?CarRenderNormalize4@@'}
GCC_NAMES = {'copy': '_ZN15CarRenderRecord8CopyFromEPS_',
             'direction': '_Z24CarRenderDirectionMatrixP11WheelMatrixS0_',
             'coefficients': '_ZN15CarRenderRecord21TransformCoefficientsEP11WheelMatrix',
             'normalize': '_Z19CarRenderNormalize4PfPKf'}
ORIG_CALLEES = {'copy': 0x48a3b0, 'direction': 0x5bb190, 'coefficients': 0x489ba0, 'normalize': 0x43d010}


def build(source, gcc, chain):
    sources = [source] + ([FOUNDATION, COEFFICIENTS] if chain else [])
    if gcc:
        bindings = {'_Z14BodyMatrixCopyP11WheelMatrixS0_': 0x4012b0,
                    'D3DXMatrixTranspose': 0x63d12c, 'sqrtl': STOP + 0x100, 'ug2WheelLodGate': 0x8654a4}
        if not chain:
            bindings.update({GCC_NAMES['copy']: BRIDGE, GCC_NAMES['direction']: 0x5bb190,
                             GCC_NAMES['coefficients']: BRIDGE + 0x40, GCC_NAMES['normalize']: 0x43d010})
        linked = link_native(sources, BUILD, ROOT, bindings, GCC_ENTRY)
        linked['entry'] = linked['exports'][GCC_ENTRY]
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        objects = []
        for i, path in enumerate(sources):
            obj = BUILD / ('prepare-' + str(i) + '.obj')
            subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                            '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                            '/Fo' + str(obj.relative_to(ROOT)), str(path.relative_to(ROOT))], cwd=ROOT, check=True)
            objects.append(obj)
        parsed = [read_object(obj) for obj in objects]
        defined = {s['name'] for o in parsed for s in o['symbols'].values() if s['section'] > 0}
        external = {'_D3DXMatrixTranspose@8': 0x63d12c, '__fltused': 0, '_ug2WheelLodGate': 0x8654a4}
        for o in parsed:
            for s in o['symbols'].values():
                name = s['name']
                if s['section'] == 0 and name not in defined:
                    for kind, prefix in PREFIX.items():
                        if name.startswith(prefix):
                            external[name] = 0x4012b0 if kind == 'matrix' else ORIG_CALLEES[kind]
        linked = link_objects(objects, external, start=0x2400000)
        linked['entry'] = next(v for k, v in linked['exports'].items()
                               if k.startswith('?CarRenderPrepareWorldRecord@@'))
    linked['callees'] = {}
    if chain:
        for kind, prefix in (GCC_NAMES if gcc else {k: v for k, v in PREFIX.items() if k != 'matrix'}).items():
            linked['callees'][kind] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False, chain=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_write(0x803f00, struct.pack('<I', 0x63d132))
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        u.mem_map(BRIDGE, 0x1000)
        for where, target in ((BRIDGE, 0x48a3b0), (BRIDGE + 0x40, 0x489ba0)):
            # cdecl(this, argument) -> thiscall(argument), preserving the cdecl caller stack.
            code = b'\x55\x89\xe5\xff\x75\x0c\x8b\x4d\x08\xe8' + struct.pack('<i', target - (where + 14))
            u.mem_write(where, code + b'\x89\xec\x5d\xc3')
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
        self.entry = linked['entry'] if linked else 0x48e6c0
        self.abi_gcc = gcc
        hooks = linked['callees'] if linked and chain else ORIG_CALLEES
        for kind, address in hooks.items():
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=kind, begin=address, end=address)
        u.hook_add(UC_HOOK_CODE, self.hook, user_data='unexpected', begin=STOP + 0x100, end=STOP + 0x100)
        self.native_methods = bool(linked and chain and gcc)

    def args(self, n):
        sp = self.u.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(sp + 4, n * 4)))

    def hook(self, u, address, size, kind):
        assert kind != 'unexpected', 'unexpected sqrtl fallback'
        if kind == 'direction':
            dst, src = self.args(2)
            self.events.append((kind, src, bytes(u.mem_read(src, 64))))
        else:
            if self.native_methods:
                dst, src = self.args(2)
            else:
                dst, src = u.reg_read(xc.UC_X86_REG_ECX), self.args(1)[0]
            if kind == 'copy':
                self.events.append((kind, dst, src, bytes(u.mem_read(src, 0x1c0))))
            else:
                self.events.append((kind, bytes(u.mem_read(dst, 0xa0)), bytes(u.mem_read(src, 64))))

    def run(self, data, dst, world, cw, arguments=None, expected_events=None):
        u = self.u
        self.events = []
        u.mem_write(ARENA, data)
        sp = STACK + 0x8000
        arguments = [dst, SRC, world] if arguments is None else arguments
        u.mem_write(sp, struct.pack('<' + 'I' * (len(arguments) + 1), STOP, *arguments))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entry, STOP, count=30000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        if expected_events is not False:
            expected_events = ['copy', 'direction', 'coefficients'] if expected_events is None else expected_events
            assert [e[0] for e in self.events] == expected_events
        return bytes(u.mem_read(ARENA, SIZE)), self.events


def cases(n):
    rng = random.Random(0x48e6c0)
    for i in range(n):
        data = bytearray(rng.randbytes(SIZE))
        for j in range(0x1c0 // 4):
            struct.pack_into('<f', data, SRC - ARENA + j * 4, rng.uniform(-3, 3))
        dst = SRC + rng.choice((-0x200, -64, -16, -4, 0, 4, 16, 64, 0x200))
        world = rng.choice((ARENA + 0x800, SRC, SRC + 0x40, dst, dst + 0x40))
        values = [rng.uniform(-3, 3) for _ in range(16)]
        if i % 8 == 0:
            values = [float(j in (0, 5, 10, 15)) for j in range(16)]
        elif i % 8 == 1:
            values = [0.0] * 16
        struct.pack_into('<16f', data, world - ARENA, *values)
        yield bytes(data), dst, world


def verify(image, linked, gcc, chain, n):
    a, b = Machine(image), Machine(image, linked, gcc, chain)
    count = 0
    for data, dst, world in cases(n):
        for cw in (0x027f, 0x037f):
            x, y = a.run(data, dst, world, cw), b.run(data, dst, world, cw)
            if x != y:
                offsets = sorted({j & ~3 for j, (u, v) in enumerate(zip(x[0], y[0])) if u != v})[:6]
                details = [(hex(j - (dst - ARENA)), struct.unpack_from('<f', x[0], j)[0],
                            struct.unpack_from('<f', y[0], j)[0]) for j in offsets]
                raise AssertionError(('prepare mismatch', count, hex(cw), hex(dst - SRC), hex(world - SRC),
                                      details, [i for i, (u, v) in enumerate(zip(x[1], y[1])) if u != v]))
            count += 1
    return count


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--chain', action='store_true')
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
    count = verify(image, build(args.replace or WORLD_SOURCE, args.gcc, args.chain), args.gcc, args.chain, args.cases)
    report = dict(comparisons=count, failures=0, byte_identity='not established', chain=args.chain,
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['finite bounded records, matrices and aliases; no exhaustive coverage',
                               'view-space producer and car-draw caller inputs are not covered',
                               'D3DX default scalar transpose selected; CPU dispatch not tested'])
    if args.mutation_checks:
        source = WORLD_SOURCE.read_text()
        mutations = {
            'omit-copy': ('output->CopyFrom(source);', ''),
            'wrong-arrangement': ('[component * 4 + column]', '[column * 4 + component]'),
            'wrong-coefficient-source': ('words[i] = source->value120[i];', 'words[i] = output->value120[i];'),
            'omit-transform': ('coefficients.TransformCoefficients(&direction);', ''),
            'short-coeff-copy': ('output->value120[i] = words[i];', 'if (i < 39) output->value120[i] = words[i];'),
            'wrong-source-basis': ('const float s0 = input[0]', 'const float s0 = input[1]'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after, 1))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '128'] +
                               (['--gcc'] if args.gcc else []) + (['--chain'] if args.chain else []),
                               text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and ('prepare mismatch' in r.stdout or 'self.events' in r.stdout), (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + ('-chain' if args.chain else '-isolated')
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
