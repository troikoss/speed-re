#!/usr/bin/env python3
"""Execute original response resize/CRT iterators and rebuilt normal paths; control only heap calls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-response-array'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, SELF, WORK, OLD, ALT, NEW, TEXT, STRINGS, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x1000, 0x2000, 0x3000, 0x5000, 0x6000, 0x8000, 0x1f000, 0x100000))
TEB, GDT = 0x2120000, 0x2130000
PREFIXES = dict(Resize='?ResizeCommands@FEResponse@', ConstructArray='?FEngConstructArrayWithCleanup@',
                DestructArray='?FEngDestructArrayWithCleanup@', Construct='?FEngConstructResponseCommand@',
                Destruct='?FEngDestructResponseCommand@', SetString='?SetString@FEResponseCommand@', Assign='?Assign@FEResponseCommand@')
ARGS = dict(Resize=1, ConstructArray=5, DestructArray=4, Construct=0, Destruct=0, SetString=1, Assign=1)
ROOTS = ('Resize', 'ConstructArray', 'DestructArray')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngResponseArray.cpp')
    parser.add_argument('--lifetime-source', type=Path, default=FRONT / 'FEngResponseArrayLifetime.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response-array.json').read_text())
    manifest['functions'].update(json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response-command.json').read_text())['functions'])
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.lifetime_source, FRONT / 'FEngResponseCommand.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/EHsc', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
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
    state, machines = {}, []; hits = dict.fromkeys(PREFIXES, 0)
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0xc000))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000); uc.mem_map(TEB, 0x1000); uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)): uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        functions = entries if rebuilt else original
        helpers = {a: n for n, a in functions.items()}
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        callback_names = {functions[n]: n for n in ('Construct', 'Destruct')}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, callback_names=callback_names, original_run=not rebuilt):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                if original_run: hits[name] += 1
                argv = words(uc, sp + 4, ARGS[name]); this = uc.reg_read(xc.UC_X86_REG_ECX)
                if name.endswith('Array'):
                    this = None
                    argv = argv[:3] + tuple(callback_names.get(a, hex(a)) for a in argv[3:])
                state['trace'].append((name, this, argv, snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if mode & 1: put(uc, SELF + 0x14, ALT)
            if mode & 2: put(uc, SELF + 0x10, 2)
            if mode & 4:
                put(uc, OLD + 8, 0xaabbccdd); put(uc, ALT + 8, 0x11223344)
            if address == 0x575620:
                index = state['allocations']; state['allocations'] += 1
                result = (0 if mode == 8 else NEW) if index == 0 else STRINGS + index * 0x100
            else: result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x52ae00); counts = dict.fromkeys(ROOTS, 0)
    types = (0, 0x1ff, 0x200, 0x201, 0x202, 0x203, 0x204, 0x205, 0xffffffff)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        mode = (trial // 101) % 8
        old_count = trial % 7; request = (trial // 7) % 7
        if trial % 19 == 0: old_count = 0; mode = 8
        putm(SELF + 0x10, old_count); putm(SELF + 0x14, OLD)
        for array, cookie in ((OLD, (trial // 3) % 7), (ALT, (trial // 5) % 7)):
            putm(array - 4, cookie)
            for i in range(7):
                putm(array + i * 12, types[(trial + i) % len(types)])
                putm(array + i * 12 + 4, 0 if (trial + i) % 5 == 0 else TEXT + i * 64)
        for i in range(7):
            data = bytes(rng.randrange(1, 256) for _ in range((trial + i) % 33)) + b'\0'
            memory[TEXT - ARENA + i * 64:TEXT - ARENA + i * 64 + len(data)] = data
        for i in range(-8, 17):
            putm(WORK + i * 12, types[(trial + i) % len(types)])
            putm(WORK + i * 12 + 4, TEXT + (i % 7) * 64)
        # Null old storage only with a zero logical count, avoiding invalid copy sources.
        if not old_count and trial % 2 == 0 and mode in (0, 8): putm(SELF + 0x14, 0)
        stride = (0, 12, 24, 36, 0xfffffff4)[trial % 5]
        iterator_count = (trial // 5) % 7
        if trial % 17 == 0: iterator_count = 0xffffffff
        if trial % 23 == 0: iterator_count = 0x80000001
        for name in ROOTS:
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                functions = entries if rebuilt else original
                argv = [request] if name == 'Resize' else [WORK, stride, iterator_count, functions['Construct'], functions['Destruct']] if name == 'ConstructArray' else [WORK, stride, iterator_count, functions['Destruct']]
                uc.mem_write(ARENA, bytes(memory)); put(uc, TEB, 0xffffffff)
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, SELF)
                state.clear(); state.update(trace=[], mode=mode, allocations=0)
                try: uc.emu_start(functions[name], STOP, count=100000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                outputs.append((snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:12],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'constructor-unsigned-count': ('(int)i < (int)count', 'i < count'),
            'constructor-wrong-stride': ('memory + stride', 'memory + stride + 4'),
            'destructor-forward': ('memory - stride', 'memory + stride'),
            'destructor-wrong-count': ('stride * count', 'stride * (count + 1)'),
            'resize-omit-equal-fastpath': ('if (count != commandCount10)', 'if (true)'),
            'resize-wrong-allocation': ('count * 12 + 4', 'count * 12'),
            'resize-wrong-cookie': ('*allocation = count;', '*allocation = count + 1;'),
            'resize-no-construct': ('FEngConstructArrayWithCleanup(buffer, 12, count,\n                    (FEngLifetimeConstructor)FEngConstructResponseCommand, (FEngLifetimeDestructor)FEngDestructResponseCommand);', '/* omitted */'),
            'resize-wrong-min': ('if (count < retained) retained = count;', '/* omitted */'),
            'resize-cache-old-before-allocate': [('            unsigned int *allocation =', '            FEResponseCommand *cached = commands14;\n            unsigned int *allocation ='), ('(unsigned int)commands14 + offset', '(unsigned int)cached + offset')],
            'resize-shallow-copy': (')->Assign(\n                    (FEResponseCommand *)((unsigned int)commands14 + offset));', ')[0] = *(FEResponseCommand *)((unsigned int)commands14 + offset);'),
            'resize-use-logical-dtor-count': ('FEngDestructArrayWithCleanup(old, 12, *base,', 'FEngDestructArrayWithCleanup(old, 12, commandCount10,'),
            'resize-omit-free': ('FEngFreeQueueNode(base);', '/* omitted */'),
            'resize-omit-install': ('commands14 = buffer;', '/* omitted */'),
        }
        for label, edits in mutations.items():
            if isinstance(edits, tuple): edits = [edits]
            owner = args.lifetime_source if label.startswith(('constructor-', 'destructor-')) else args.source
            mutant_source = owner.read_text()
            for before, after in edits:
                assert before in mutant_source; mutant_source = mutant_source.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(mutant_source)
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, ('--lifetime-source' if owner == args.lifetime_source else '--source'), str(mutant), '--cases', '256'],
                                 cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
        report['rejected_mutations'] = list(mutations)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
