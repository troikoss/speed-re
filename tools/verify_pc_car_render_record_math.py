#!/usr/bin/env python3
"""Compare render-record foundations 0x48a3b0, 0x43d010, 0x5bb190 with PC v1.2.

Whole fixture, integer returns, ABI preservation and matrix-call order are checked.
Original matrix-copy and default D3DX transpose machine code run on both sides.
These are foundation tests, not coverage of the two render-record producer callers.
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

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderRecord.cpp'
BUILD = ROOT / 'build/analysis/car-render-record-math'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-record-math.json'
ARENA, STACK, STOP = 0x2000000, 0x2100000, 0x2110000
SIZE = 0x4000
ORIGINAL = {'copy': 0x48a3b0, 'normalize': 0x43d010, 'direction': 0x5bb190}
MSVC = {'copy': '?CopyFrom@CarRenderRecord@@', 'normalize': '?CarRenderNormalize4@@',
        'direction': '?CarRenderDirectionMatrix@@'}
GCC = {'copy': '_ZN15CarRenderRecord8CopyFromEPS_', 'normalize': '_Z19CarRenderNormalize4PfPKf',
       'direction': '_Z24CarRenderDirectionMatrixP11WheelMatrixS0_'}


def build(source, gcc):
    if gcc:
        bindings = {'_Z14BodyMatrixCopyP11WheelMatrixS0_': 0x4012b0,
                    'D3DXMatrixTranspose': 0x63d12c, 'sqrtl': STOP + 0x100}
        linked = link_native(source, BUILD, ROOT, bindings, GCC['normalize'])
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        obj = BUILD / 'record.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))],
                       cwd=ROOT, check=True)
        external = {'_D3DXMatrixTranspose@8': 0x63d12c, '__fltused': 0}
        for s in read_object(obj)['symbols'].values():
            if s['section'] == 0 and s['name'].startswith('?BodyMatrixCopy@@'):
                external[s['name']] = 0x4012b0
        linked = link_objects([obj], external, start=0x2400000)
    linked['entries'] = {}
    for kind, prefix in (GCC if gcc else MSVC).items():
        linked['entries'][kind] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for a, b in linked['blocks']:
                u.mem_write(a, b)
        self.linked, self.gcc = linked, gcc
        # Select the actual default D3DX transpose body, bypassing CPU dispatch only.
        u.mem_write(0x803f00, struct.pack('<I', 0x63d132))
        for address in (0x4012b0, 0x63d12c, STOP + 0x100):
            u.hook_add(UC_HOOK_CODE, self.hook, begin=address, end=address)

    def hook(self, u, address, size, user):
        assert address != STOP + 0x100, 'unexpected sqrtl fallback'
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        self.events.append((address, *struct.unpack('<II', bytes(u.mem_read(sp + 4, 8)))))

    def run(self, kind, data, dst, src, cw):
        u = self.u
        self.events = []
        u.mem_write(ARENA, data)
        sp = STACK + 0x8000
        if kind == 'copy' and not self.gcc:
            args = [src]
            u.reg_write(xc.UC_X86_REG_ECX, dst)
            cleanup = 8
        else:
            args = [dst, src]
            cleanup = 4
        u.mem_write(sp, struct.pack('<' + 'I' * (len(args) + 1), STOP, *args))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.linked['entries'][kind] if self.linked else ORIGINAL[kind], STOP, count=10000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + cleanup
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        ret = u.reg_read(xc.UC_X86_REG_EAX) if kind != 'normalize' else None
        return ret, bytes(u.mem_read(ARENA, SIZE)), self.events


def cases(n):
    rng = random.Random(0x48a3b0)
    # Copy fixtures include arbitrary binary32 payloads, including NaNs, and overlap.
    for displacement in (-0x200, -64, -16, -4, 0, 4, 16, 64, 0x200):
        for _ in range(max(1, n // 9)):
            data = rng.randbytes(SIZE)
            yield 'copy', data, ARENA + 0x800 + displacement, ARENA + 0x800
    for _ in range(n):
        data = bytearray(rng.randbytes(SIZE))
        for i in range(16):
            struct.pack_into('<f', data, 0x800 + 4 * i, rng.uniform(-100, 100))
        yield 'direction', bytes(data), ARENA + rng.choice((0x800, 0x1000)), ARENA + 0x800
    vectors = [(0, 0, 0, 0), (0, 0, 0, 2), (1, 2, 3, 4), (1e-30, -1e-30, 0, 1e-30),
               (1e30, -1e30, 1e30, -1e30), (-0.0, -0.0, -0.0, -0.0)]
    vectors += [tuple(rng.uniform(-100, 100) for _ in range(4)) for _ in range(n)]
    for v in vectors:
        for displacement in (-4, 0, 4, 8, 16, 0x100):
            data = bytearray(rng.randbytes(SIZE))
            struct.pack_into('<4f', data, 0x800, *v)
            yield 'normalize', bytes(data), ARENA + 0x800 + displacement, ARENA + 0x800


def verify(image, linked, gcc, n):
    orig, reb = Machine(image), Machine(image, linked, gcc)
    counts = dict.fromkeys(ORIGINAL, 0)
    for kind, data, dst, src in cases(n):
        for cw in (0x027f, 0x037f):
            a, b = orig.run(kind, data, dst, src, cw), reb.run(kind, data, dst, src, cw)
            if a != b:
                diffs = [hex(i) for i, (x, y) in enumerate(zip(a[1], b[1])) if x != y][:8]
                raise AssertionError(('behavior mismatch', kind, cw, dst - src, a[0], b[0], diffs, a[2], b[2]))
            counts[kind] += 1
    return counts


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--replace', type=Path)
    p.add_argument('--cases', type=int, default=256)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    counts = verify(image, build(args.replace or SOURCE, args.gcc), args.gcc, args.cases)
    report = dict(comparisons=counts, failures=0, byte_identity='not established',
                  compiler='GCC i386 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['foundation helpers only, producer callers remain open',
                               'finite normalisation inputs; bounded copy alias and NaN fixtures',
                               'D3DX default scalar transpose selected; CPU dispatch not tested'])
    if args.mutation_checks:
        source = SOURCE.read_text()
        mutations = {
            'omit-w-length': ('+ w * w;', ';'),
            'normal-zero': ('output[0] = 1.0f;', 'output[0] = 0.0f;'),
            'normal-write-w': ('output[2] = savedZ * inverse;', 'output[2] = savedZ * inverse; output[3] = 0.0f;'),
            'length-float-spill': ('const long double length =', 'volatile float length ='),
            'direction-15-one': ('output->elements[15] = 0.0f;', 'output->elements[15] = 1.0f;'),
            'direction-keep-12': ('output->elements[12] = 0.0f;', ''),
            'copy-omit-last': ('group < 10', 'group < 9'),
            'copy-whole-snapshot': ('BodyMatrixCopy(&matrix00, &source->matrix00);',
                'CarRenderRecord snapshot = *source; source = &snapshot; BodyMatrixCopy(&matrix00, &source->matrix00);'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            r = subprocess.run([sys.executable, __file__, '--replace', str(path), '--cases', '32'] +
                               (['--gcc'] if args.gcc else []), text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(r.stdout)
            assert r.returncode and ('behavior mismatch' in r.stdout or 'Unexpected' in r.stdout), (name, r.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD / ('verification-gcc.json' if args.gcc else 'verification.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
