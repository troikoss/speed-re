#!/usr/bin/env python3
"""Execute UG2 UTF-16 storage and rebuilt callees together; control only heap boundaries."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile

import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-text-storage'
SOURCE = ROOT / 'src/Speed/Indep/Src/Frontend/FEngTextStorage.cpp'
ARENA, OBJECT, OLD, INPUT, NEW, ALT, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x1000, 0x2000, 0x3000, 0x4000, 0x7000, 0x10000))
PREFIXES = {'Copy': '?FEngCopyText@', 'Grow': '?Grow@FEngTextStorage@',
            'GetBuffer': '?GetBuffer@FEngTextStorage@', 'Assign': '?Assign@FEngTextStorage@',
            'Destruct': '?Destruct@FEngTextStorage@'}
ARGS = dict(Copy=2, Grow=1, GetBuffer=1, Assign=1, Destruct=0)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-text-storage.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'FEngTextStorage.obj'
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for symbol in read_object(obj)['symbols'].values():
        if symbol['section'] != 0:
            continue
        if symbol['name'].startswith('?FEngAllocateQueueNode@'):
            externals[symbol['name']] = 0x575620
        elif symbol['name'].startswith('?FEngFreeQueueNode@'):
            externals[symbol['name']] = 0x575630
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []

    def words(uc, a, count=1):
        return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))

    def put(uc, a, value):
        uc.mem_write(a, struct.pack('<I', value & 0xffffffff))

    def snapshot(uc):
        return bytes(uc.mem_read(ARENA, 0x5000))

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x8000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']:
            uc.mem_write(a, b)
        helpers = {a: n for n, a in (entries if rebuilt else original).items()}
        ranges = linked['ranges'] if rebuilt else [(a, a + manifest['functions'][n]['size']) for n, a in original.items()]

        def hook(uc, address, size, user, helpers=helpers, ranges=ranges):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, 0 if name == 'Copy' else uc.reg_read(xc.UC_X86_REG_ECX),
                                       words(uc, sp + 4, ARGS[name]), snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                # Real boundary callback: test fresh member loads after allocation.
                if state['mode'] & 1:
                    put(uc, OBJECT, ALT)
                if state['mode'] & 2:
                    put(uc, OBJECT + 4, 0x12345678)
                result = 0 if state['mode'] & 4 else NEW
            else:
                if state['mode'] & 8:
                    put(uc, OBJECT, ALT + 0x80)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result)
            uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc)
            uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4)
            uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    counts = dict.fromkeys(PREFIXES, 0)
    rng = random.Random(0x51de90)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x8000))
        def putm(a, value):
            struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def text(a, length):
            value = [rng.randrange(1, 65536) for _ in range(length)] + [0]
            memory[a - ARENA:a - ARENA + len(value) * 2] = struct.pack('<' + 'H' * len(value), *value)
        old_length = trial % 17
        source_length = (trial // 3) % 33
        for a, length in ((OLD, old_length), (INPUT, source_length), (ALT, trial % 7)):
            text(a, length)
        old = 0 if trial % 9 == 0 else OLD
        capacity = (0, old_length, old_length + 1, 128)[trial % 4]
        putm(OBJECT, old)
        putm(OBJECT + 4, capacity)
        request = (0, old_length, old_length + 1, 64, 0x80000001, 0xffffffff)[trial % 6]
        mode = (trial // 17) % 16
        for name in PREFIXES:
            if name == 'Copy':
                # Null destination must not touch even an invalid source; forward overlap only.
                pair = trial % 6
                argv = [(NEW, INPUT), (NEW, 0), (0, 0xdeadbeef), (INPUT, INPUT), (INPUT, INPUT + (2 if source_length else 0)), (NEW, INPUT)][pair]
            elif name == 'Assign':
                argv = [0 if trial % 5 == 0 else OLD if trial % 5 == 1 else INPUT]
            else:
                argv = [] if name == 'Destruct' else [request]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory))
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED):
                    uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                uc.reg_write(xc.UC_X86_REG_ECX, OBJECT)
                state.clear()
                state.update(trace=[], mode=mode)
                try:
                    uc.emu_start((entries if rebuilt else original)[name], STOP, count=20000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + (0 if name == 'Copy' else len(argv) * 4)
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                result = uc.reg_read(xc.UC_X86_REG_EAX) if name in ('GetBuffer', 'Assign') else None
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:10],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts,
                  byte_matching=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'grow-by-capacity': ('if (request > length)', 'if (request > capacity4)'),
            'omit-terminator-space': ('request * 2 + 2', 'request * 2'),
            'omit-capacity-write': ('capacity4 = request;', '/* omitted */'),
            'omit-old-copy': ('FEngCopyText(buffer, text0);', '/* omitted */'),
            'omit-old-free': ('FEngFreeQueueNode(text0);\n        text0 = buffer;', 'text0 = buffer;'),
            'get-buffer-always-grow': ('if (request > capacity4) Grow(request);', 'Grow(request);'),
            'assign-length-only': ('if (length + 1 > capacity4) Grow(length + 1);', 'if (length > capacity4) Grow(length);'),
            'copy-null-source-noop': ('if (destination) {', 'if (destination && source) {'),
            'destructor-clears-field': ('if (text0) FEngFreeQueueNode(text0);', 'if (text0) FEngFreeQueueNode(text0); text0 = 0;'),
        }
        rejected = []
        with tempfile.TemporaryDirectory(dir=BUILD) as directory:
            for label, (before, after) in mutations.items():
                assert before in source, label
                mutant = Path(directory) / (label + '.cpp')
                mutant.write_text(source.replace(before, after, 1))
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '128'],
                                     cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / (label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1200:])
                rejected.append(label)
        report['mutations_rejected'] = rejected
    (BUILD / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
