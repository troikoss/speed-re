#!/usr/bin/env python3
"""Compare UG2 script names and event storage with original execution; control only heap calls."""
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

BUILD = ROOT / 'build/analysis/feng-script-storage'
SOURCE = ROOT / 'src/Speed/Indep/Src/Frontend/FEngScriptStorage.cpp'
ARENA, OBJECT, OLD, INPUT, NEW, ALT, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x1000, 0x2000, 0x3000, 0x4000, 0x7000, 0x10000))
PREFIXES = dict(SetName='?SetName@FEScript@', ResizeEvents='?Resize@FEngScriptEventStorage@', FngHash='?FngHash@')
ARGS = dict(SetName=1, ResizeEvents=1, FngHash=1)
NAME_OLD, NAME_ALT = ARENA + 0x1800, ARENA + 0x4800
ROOTS = ('SetName', 'ResizeEvents')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-script-storage.json').read_text())
    manifest['functions']['FngHash'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid.json').read_text())['functions']['FngHash']
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngHash.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section'] != 0: continue
            if symbol['name'].startswith('?FEngAllocateQueueNode@'): externals[symbol['name']] = 0x575620
            elif symbol['name'].startswith('?FEngFreeQueueNode@'): externals[symbol['name']] = 0x575630
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []
    hits = dict.fromkeys(PREFIXES, 0)

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

        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original_run=not rebuilt):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                if original_run: hits[name] += 1
                state['trace'].append((name, None if name == 'FngHash' else uc.reg_read(xc.UC_X86_REG_ECX), words(uc, sp + 4, ARGS[name]), snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            mode = state['mode']
            if state['root'] == 'SetName':
                if mode & 1: put(uc, OBJECT + 0x2c, NAME_ALT)
                if mode & 2: put(uc, OBJECT + 0x30, 0x12345678)
                if mode & 4: put(uc, OBJECT + 0x14, 0xaabbccdd)
                if mode & 8 and uc.mem_read(INPUT, 1) != b'\0': uc.mem_write(INPUT, b'Q')
                result = NEW if address == 0x575620 else 0xfeedface
            else:
                if address == 0x575620:
                    if mode & 1: put(uc, OBJECT + 0x24, 2)
                    if mode & 2: put(uc, OBJECT + 0x28, ALT)
                    result = OBJECT + 0x24 if state['alias_descriptor'] else 0 if not argument and mode & 8 else OLD + 4 if mode & 8 and state['overlap'] else NEW
                else:
                    if mode & 4: put(uc, OBJECT + 0x28, ALT + 0x80)
                    result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result)
            uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc)
            uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4)
            uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    counts = dict.fromkeys(ROOTS, 0)
    rng = random.Random(0x519a10)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x8000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        for a, length in ((NAME_OLD, trial % 17), (INPUT, (trial // 3) % 33), (NAME_ALT, trial % 7)):
            value = bytes(rng.randrange(1, 256) for _ in range(length)) + b'\0'
            memory[a - ARENA:a - ARENA + len(value)] = value
        putm(OBJECT + 0x2c, NAME_OLD if trial % 3 else 0)
        old_count, request = trial % 7, (trial // 7) % 9
        mode = (trial // 97) % 16
        if trial % 23 == 0:
            old_count, request, mode = 0, 0x80000000, 8
        if trial % 29 == 0:
            old_count, request, mode = 3, 0x80000001, 0
        if trial % 31 == 0:
            old_count, request, mode = 0x80000000, 2, 0
        if trial % 37 == 0:
            old_count, request, mode = 0x80000001, 3, 0
        alias_descriptor = trial % 41 == 0
        if alias_descriptor:
            old_count, request, mode = 1, 3, 0
            putm(OLD, 3); putm(OLD + 4, ALT)
        putm(OBJECT + 0x24, old_count); putm(OBJECT + 0x28, OLD if old_count or trial % 3 or mode & 1 else 0)
        for name in ROOTS:
            argv = [0 if trial % 5 == 0 else INPUT] if name == 'SetName' else [request]
            this = OBJECT if name == 'SetName' else OBJECT + 0x24
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory))
                uc.mem_write(STACK, struct.pack('<II', STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear(); state.update(trace=[], mode=mode, root=name, overlap=bool(old_count), alias_descriptor=alias_descriptor)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=30000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 8
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                outputs.append((snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:12],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts,
                  actual_entry_hits=hits, byte_matching=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'name-omit-free': ('FEngFreeQueueNode(name2C);', '/* omitted */'),
            'name-omit-clear': ('name2C = 0;', '/* omitted */'),
            'name-wrong-hash-sentinel': ('hash30 = 0xffffffff;', 'hash30 = 0;'),
            'name-wrong-size': ('FEngAllocateQueueNode(length)', 'FEngAllocateQueueNode(length - 1)'),
            'name-omit-hash': ('hash30 = FngHash(name2C);', '/* omitted */'),
            'event-no-equal-fastpath': ('if (count != count0)', 'if (true)'),
            'event-wrong-byte-size': ('(unsigned int)count * 12;', '(unsigned int)count * 4;'),
            'event-unsigned-comparison': ('count < oldCount', '(unsigned int)count < (unsigned int)oldCount'),
            'event-no-copy': ('*destination++ = *source++', '*destination++ = 0'),
            'event-cache-postcopy-count': ('int freshCount = count0;', 'int freshCount = oldCount;'),
            'event-cache-free-pointer': [('        int oldCount = count0;', '        int oldCount = count0;\n        FEScriptEvent *cachedData = data4;'), ('        FEngFreeQueueNode(data4);', '        FEngFreeQueueNode(cachedData);')],
            'event-no-zero-tail': ('while (words--) *destination++ = 0;', '/* omitted */'),
            'event-omit-final-free': ('        FEngFreeQueueNode(data4);', '        /* omitted */'),
            'event-wrong-install-count': ('count0 = count;', 'count0 = 0;'),
            'event-cache-count-before-allocate': [('        unsigned int *buffer =', '        int cached = count0;\n        unsigned int *buffer ='), ('int oldCount = count0;', 'int oldCount = cached;')],
        }
        rejected = []
        with tempfile.TemporaryDirectory(dir=BUILD) as directory:
            for label, edits in mutations.items():
                if isinstance(edits, tuple): edits = [edits]
                mutant_source = source
                for before, after in edits:
                    assert before in mutant_source, label
                    mutant_source = mutant_source.replace(before, after, 1)
                mutant = Path(directory) / (label + '.cpp')
                mutant.write_text(mutant_source)
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '256'],
                                     cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / (label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1200:])
                rejected.append(label)
        report['mutations_rejected'] = rejected
    (BUILD / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
