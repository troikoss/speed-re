#!/usr/bin/env python3
"""Compare the car wheel render-frame getters under 53- and 64-bit x87 precision:
MoverModeOwner::GetWheelRenderMatrices 0x5f6820 (tyre and brake frames) and
MoverModeOwner::GetWheelSpinMatrices 0x5ec7f0 (third frame set), callers 0x631250/0x6316d0.

WheelRenderMatrices.cpp is built with MSVC 7.1 /Ox /Ob0 and linked. Original code runs for the
callees: BodyMatrixCopy 0x4012b0, RotateMatrixY 0x5bada0, RotateMatrixZ 0x5bae60, the float to int
conversion 0x75bed4 and the scale constant 0x78423c. The D3DX multiply thunk 0x63cedb is a
traced stub (the same Python code in both runs). The compared state is the whole fixture arena
(car, record and the three output sets), ESP, callee-saved registers, FPU control and tag words
and the entry events (callee, arguments, pointers as arena offsets) in call order.
Byte identity of the rebuilt code is NOT claimed."""

import argparse
import ctypes
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/wheel-render'
SOURCE = 'src/Speed/Indep/Src/Physics/WheelRenderMatrices.cpp'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x4000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
CAR, RECORD, TIRES, BRAKES, SPIN = (ARENA + x for x in (0x1000, 0x2000, 0x3000, 0x3100, 0x3200))
D3DX = 0x63cedb
SHARED = {'?BodyMatrixCopy@@': 0x4012b0, '?RotateMatrixY@@': 0x5bada0, '?RotateMatrixZ@@': 0x5bae60, '?wheelRenderAngleScale@@': 0x78423c,
          '__ftol2': 0x75bed4, '__fltused': 0}
EVENTS = {'copy': 0x4012b0, 'roty': 0x5bada0, 'rotz': 0x5bae60}
ARGS = {'copy': 2, 'roty': 3, 'rotz': 3}
# name: (rebuilt symbol prefix, callee-popped bytes)
TARGETS = {'GetWheelRenderMatrices': ('?GetWheelRenderMatrices@MoverModeOwner@@', 12), 'GetWheelSpinMatrices': ('?GetWheelSpinMatrices@MoverModeOwner@@', 8)}


def f32(x): return ctypes.c_float(x).value


def special(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, -1e-20, 1e-45, 1e30, 3e38, float('inf'), -float('inf'), float('nan'),
                       rng.uniform(-2, 2), rng.uniform(-500, 500)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--replace', help='compile this file instead of the recovered source')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/wheel-render.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    source = Path(args.replace) if args.replace else ROOT / SOURCE
    obj = BUILD / ('replaced.obj' if args.replace else 'WheelRenderMatrices.obj')
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/EHsc',
                    '/Isrc/Speed/Indep/Src/Physics', '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT) if source.is_relative_to(ROOT) else source)],
                   cwd=ROOT, check=True)
    parsed = read_object(obj)
    defined = {s['name'] for s in parsed['symbols'].values() if s['section'] > 0}
    externals = {}
    for s in parsed['symbols'].values():
        if s['section'] != 0 or s['name'] in defined: continue
        match = [a for pre, a in SHARED.items() if s['name'].startswith(pre)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, events, traces = [], [[], []], [[], []]
    stack_range = (STACK - 0x1000, STACK + 0x100)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, ARENA_SIZE + 0x1000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        uc.mem_map(STACK - 0x2000, 0x3000)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
            out, left, right = struct.unpack('<3I', uc.mem_read(esp + 4, 12))
            l = struct.unpack('<16f', uc.mem_read(left, 64)); r = struct.unpack('<16f', uc.mem_read(right, 64))
            trace.append(('d3dx', bytes(uc.mem_read(left, 64)), bytes(uc.mem_read(right, 64))))
            uc.mem_write(out, struct.pack('<16f', *[f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4))) for i in range(4) for j in range(4)]))
            uc.reg_write(xc.UC_X86_REG_EAX, out); uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + 12); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        def entry(uc, address, size, user, log=events[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            words = struct.unpack('<%dI' % ARGS[user], uc.mem_read(esp + 4, 4 * ARGS[user]))
            view = []
            for i, w in enumerate(words):
                if ARENA <= w < ARENA + ARENA_SIZE: view.append(('arena', w - ARENA, bytes(uc.mem_read(w, 64)) if user != 'copy' or i == 1 else None))
                else: view.append(w)
            log.append((user, tuple(view)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        uc.hook_add(UC_HOOK_CODE, stub, begin=D3DX, end=D3DX)
        for name, address in EVENTS.items():
            uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x5f6820); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, argv, precision):
        _, cleanup = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            events[rebuilt].clear(); traces[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, CAR)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=400000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not finish')
            contract = (uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW))
            outputs.append((contract, uc.reg_read(xc.UC_X86_REG_FPTAG), list(events[rebuilt]), list(traces[rebuilt]), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][0][0] == STACK + 4 + cleanup and outputs[0][0][2] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            a, b = outputs
            diff = [(hex(ARENA + i), a[4][i:i + 4].hex(), b[4][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if a[4][i:i + 4] != b[4][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), a[0], b[0], a[1], b[1], [e[0] for e in a[2]], [e[0] for e in b[2]],
                                  next(((x, y) for x, y in zip(a[2], b[2]) if x != y), None), diff))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def note(path): paths[path] = paths.get(path, 0) + 1
    def gen():
        return rng.choice((rng.gauss(0, 1), rng.uniform(-30, 30), rng.gauss(0, 1) * 1e-3)) if rng.random() < 0.9 else special(rng)

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        view = rng.choice((0, 1, 2, 3, 4, 5, 7, 8, 31, 32, 33, rng.randrange(0, 256), rng.randrange(0, 8)))
        bit = (1 << (view & 31))
        rest = rng.random() < 0.5
        mask = rng.getrandbits(32)
        mask = (mask | bit) if rest else (mask & ~bit & 0xffffffff)
        if not rng.randrange(4): mask = bit if rest else 0
        pk(memory, CAR + 0x20, '<I', RECORD); pk(memory, CAR + 0x5b0, '<I', mask)
        # state at car +0x40: wheel frames F0/1F0, suspension 340, steering 3CE; spin angles at car +0x4e8
        put(memory, CAR + 0x130, [gen() for _ in range(32 * 4 // 2 * 2)])
        put(memory, CAR + 0x380, [rng.choice((0.0, gen(), rng.uniform(-0.2, 0.2))) for _ in range(4)])
        pk(memory, CAR + 0x40e, '<H', rng.choice((0, 1, 0x4000, 0x8000, 0xffff, rng.getrandbits(16))))
        put(memory, CAR + 0x4e8, [rng.choice((0.0, rng.uniform(-1000, 1000), rng.uniform(-5, 5), rng.uniform(-1e6, 1e6), special(rng))) for _ in range(4)])
        for w in range(4): put(memory, RECORD + 0x10 + w * 0x30, [rng.choice((gen(), rng.uniform(-1.2, 1.2))) for _ in range(3)])
        note('rest-pose' if mask & bit else 'live-frames')
        note('view-masked-high' if view >= 32 else 'view-low')
        run('GetWheelRenderMatrices', memory, [TIRES, BRAKES, view], precision)
        run('GetWheelSpinMatrices', memory, [SPIN, view], precision)
    report = dict(comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())), x87_precision=['53-bit', '64-bit'],
                  cases=args.cases, compiler='MSVC 7.1 /Ox /Ob0 /EHsc', shared_original_callees=sorted(EVENTS) + ['float to int 0x75bed4', 'scale 0x78423c'],
                  traced_stubs=['D3DX multiply thunk 0x63cedb'],
                  limitations=['finite and special generated values; exhaustive float coverage not established',
                               'signalling NaNs are not generated',
                               'rotation and copy callees run as original code in both runs; they are verified elsewhere',
                               'the arena is filled with random bytes; the real car layout around the accessed fields is not reproduced',
                               'byte identity not established (compiler output differs from the original)'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'copy-swapped': ('BodyMatrixCopy(&tires[wheel], &aux->wheelMatricesF0[wheel]);', 'BodyMatrixCopy(&tires[wheel], &aux->wheelMatrices1F0[wheel]);'),
            'brake-source': ('BodyMatrixCopy(&brakes[wheel], &aux->wheelMatrices1F0[wheel]);', 'BodyMatrixCopy(&brakes[wheel], &aux->wheelMatricesF0[wheel]);'),
            'rest-bit-inverted': ('if ((1 << view) & value5B0) {\n        for (int wheel = 0; wheel < 4; ++wheel) {\n            float *tire', 'if (!((1 << view) & value5B0)) {\n        for (int wheel = 0; wheel < 4; ++wheel) {\n            float *tire'),
            'rest-brake-position-y': ('brake[13] = position->y;', 'brake[13] = position->z;'),
            'rest-tire-w': ('tire[15] = 1.0f;', 'tire[15] = 0.0f;'),
            'spin-wheel-limit': ('if (wheel * 0x30 < 0x60) {', 'if (wheel * 0x30 < 0x90) {'),
            'spin-steer-dropped': ('RotateMatrixZ(m, m, State()->steering3CE);', ''),
            'spin-angle-source': ('value4E8[wheel] * wheelRenderAngleScale', 'value4F8[wheel] * wheelRenderAngleScale'),
            'spin-suspension-dropped': ('m->elements[14] = z + State()->suspension340[wheel];', 'm->elements[14] = z;'),
            'spin-rest-z': ('m[14] = position->z;\n            m[15] = 1.0f;\n        }\n        return;\n    }\n    for', 'm[14] = position->y;\n            m[15] = 1.0f;\n        }\n        return;\n    }\n    for'),
            'spin-translation-x': ('m->elements[12] = position->x;', 'm->elements[12] = position->y;'),
        }
        rejected = []
        text0 = (ROOT / SOURCE).read_text()
        for label, (before, after) in mutations.items():
            assert before in text0, label
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text0.replace(before, after, 1))
            result = subprocess.run([sys.executable, __file__, '--replace', str(mutant), '--cases', '256'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
