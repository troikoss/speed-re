#!/usr/bin/env python3
"""Compare original response-header pool lifecycle and actual nested callees; only heap calls are controlled."""
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

BUILD = ROOT / 'build/analysis/feng-response-pool'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, POOL, LIST, BLOCK, SECOND, NEW, COMMANDS, ALTERNATE, TEXT, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x200, 0x2000, 0x3000, 0x5000, 0x9000, 0x9400, 0xa000, 0x1f000, 0x100000))
GLOBAL, TEB, GDT = 0x8637a0, 0x2120000, 0x2130000
PREFIXES = dict(HeaderConstruct='?FEngConstructResponseHeader@', HeaderDestruct='?FEngDestructResponseHeader@',
    HeaderDestroy='?Destroy@FEResponse@', BlockConstruct='?Construct@FEngResponsePoolBlock@',
    BlockDestruct='?Destruct@FEngResponsePoolBlock@', BlockDestroy='?Destroy@FEngResponsePoolBlock@',
    Acquire='?Acquire@FEngResponsePool@', Release='?Release@FEngResponsePool@', Insert='?Insert@FEngQueueList@',
    InsertAtCursor='?InsertAtCursor@FEngQueueList@', BaseDestroy='?DestroyBase@FEngQueueNode@',
    ConstructArray='?FEngConstructArrayWithCleanup@', DestructArray='?FEngDestructArrayWithCleanup@',
    Construct='?FEngConstructResponseCommand@', Destruct='?FEngDestructResponseCommand@',
    Remove='?Remove@FEngQueueList@', ClearOwned='?ClearOwned@FEngQueueList@')
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(HeaderDestroy=1, BlockDestroy=1, Release=1, Insert=2, InsertAtCursor=1,
            BaseDestroy=1, ConstructArray=5, DestructArray=4, Remove=1)
ROOTS = tuple(PREFIXES)[:11]
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngResponsePool.cpp')
    parser.add_argument('--header-source', type=Path, default=FRONT / 'FEngResponseHeader.cpp')
    parser.add_argument('--cases', type=int, default=512)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response-pool.json').read_text())
    for name in ('feng-response-array', 'feng-response-command'):
        manifest['functions'].update(json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + name + '.json')).read_text())['functions'])
    lifecycle = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())['functions']
    for name in ('Remove', 'ClearOwned'): manifest['functions'][name] = lifecycle[name]
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.header_source, FRONT / 'FEngResponseArrayLifetime.cpp', FRONT / 'FEngResponseCommand.cpp', FRONT / 'FEngLifecycle.cpp', FRONT / 'FEngQueueNodes.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/EHsc', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {'?FEngResponseHeaderPool@@3UFEngResponsePool@@A': GLOBAL}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section'] != 0: continue
            if symbol['name'].startswith('?FEngAllocateQueueNode@'): externals[symbol['name']] = 0x575620
            elif symbol['name'].startswith('?FEngFreeQueueNode@'): externals[symbol['name']] = 0x575630
            elif symbol['name'].startswith('?FEngObjectStateManager@'): externals[symbol['name']] = 0x863770
            elif symbol['name'].startswith('?FEngObjectDestructionObserver@'): externals[symbol['name']] = 0x838474
            elif symbol['name'].startswith('?FEngGridFreeCells@'): externals[symbol['name']] = STOP + 0x900
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []; hits = dict.fromkeys(PREFIXES, 0)
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0xb000)) + bytes(uc.mem_read(GLOBAL, 16))
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
        for table, name in ((0x79a9e8, 'BaseDestroy'), (0x79aa38, 'HeaderDestroy'), (0x79bf50, 'BlockDestroy')):
            put(uc, table, functions[name])
        helpers = {a: n for n, a in functions.items()}
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        callback_names = {functions[n]: n for n in ('Construct', 'Destruct', 'HeaderConstruct', 'HeaderDestruct')}
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
            if address == 0x575620:
                if mode & 1: put(uc, POOL + 8, SECOND)
                if mode & 4: put(uc, POOL + 12, 0)
                result = NEW
            else:
                if mode & 2: put(uc, state['header'] + 0x14, ALTERNATE)
                if mode & 4: put(uc, state['header'] + 0xc, 0x11223344)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x54bdf0); counts = dict.fromkeys(ROOTS, 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        slot = trial % 64; header = BLOCK + 12 + slot * 24
        mode = (trial // 17) % 8
        def block(base, free_count, excluded):
            putm(base, 0x79bf50); putm(base + 4, 0); putm(base + 8, 0)
            for i in range(64):
                node = base + 12 + i * 24
                for offset, value in ((0, 0x79aa38), (4, 0xabadcafe), (8, 0xabadcafe), (12, trial + i), (16, 0), (20, 0)):
                    putm(node + offset, value)
            nodes = [base + 12 + i * 24 for i in range(64) if i != excluded][:free_count]
            putm(base + 0x60c, 0x79a9ec); putm(base + 0x610, len(nodes))
            putm(base + 0x614, nodes[0] if nodes else 0); putm(base + 0x618, nodes[-1] if nodes else 0)
            putm(base + 0x61c, 64 - len(nodes))
            for i, node in enumerate(nodes):
                putm(node + 4, nodes[i + 1] if i + 1 < len(nodes) else 0)
                putm(node + 8, nodes[i - 1] if i else 0)
        free_count = 63 if trial % 7 == 0 else trial % 5
        if trial % 11 == 0: free_count = 0
        block(BLOCK, free_count, slot); block(SECOND, trial % 4, -1)
        chained = trial % 3 == 0
        putm(BLOCK + 4, SECOND if chained else 0); putm(SECOND + 8, BLOCK if chained else 0)
        global_words = (0x79a9ec, 2 if chained else 1, BLOCK, SECOND if chained else BLOCK)
        for i, value in enumerate(global_words if trial % 5 else (0x79a9ec, 0, 0, 0)): putm(POOL + i * 4, value)
        for array in (COMMANDS, ALTERNATE):
            putm(array - 4, trial % 4)
            for i in range(4):
                putm(array + i * 12, (0x200, 0x203, 0x204, 0)[i])
                putm(array + i * 12 + 4, TEXT + i * 64)
                value = b'payload-' + str(i).encode() + b'\0'
                memory[TEXT - ARENA + i * 64:TEXT - ARENA + i * 64 + len(value)] = value
        putm(header + 0x10, trial % 6); putm(header + 0x14, COMMANDS if trial % 5 else 0)
        putm(LIST, 0x79a9ec); putm(LIST + 4, 0); putm(LIST + 8, 0); putm(LIST + 12, 0)
        other = BLOCK + 12 + ((slot + 1) % 64) * 24
        # Independent insertion root uses a one-node list; pool roots must retain
        # their own original links, so this root setup is applied per invocation.
        flags = (0, 1, 0x100, 0x101, 0xffffffff)[trial % 5]
        release_arg = header
        if trial % 11 == 0: release_arg = header + 1
        if trial % 13 == 0: release_arg = BLOCK + 0x60c
        if trial % 17 == 0: release_arg = BLOCK + 11
        if trial % 19 == 0: release_arg = 0
        for name in ROOTS:
            this = BLOCK if name.startswith('Block') else POOL if name in ('Acquire', 'Release') else LIST if name in ('Insert', 'InsertAtCursor') else header
            argv = [flags] if name in ('HeaderDestroy', 'BlockDestroy', 'BaseDestroy') else [release_arg] if name == 'Release' else [0 if trial % 3 == 0 else other, 0 if trial % 5 == 0 else header] if name == 'Insert' else [0 if trial % 5 == 0 else header] if name == 'InsertAtCursor' else []
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); uc.mem_write(GLOBAL, struct.pack('<4I', *global_words)); put(uc, TEB, 0xffffffff)
                if name in ('Insert', 'InsertAtCursor'):
                    put(uc, LIST + 4, 1); put(uc, LIST + 8, other); put(uc, LIST + 12, other)
                    put(uc, other + 4, 0); put(uc, other + 8, 0)
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear(); state.update(trace=[], mode=mode, header=header)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=100000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                result = uc.reg_read(xc.UC_X86_REG_EAX) if name in ('HeaderConstruct', 'HeaderDestroy', 'BlockConstruct', 'BlockDestroy', 'Acquire', 'BaseDestroy') else None
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'pool-constructor-count': ('count4 = 0;', 'count4 = 1;'),
            'pool-constructor-used': ('used61C = 0;', 'used61C = 1;'),
            'pool-constructor-slot-count': ('i < 64', 'i < 63'),
            'acquire-wrong-first': ('block->free60C.first8;', 'block->free60C.valueC;'),
            'acquire-omit-remove': ('if (header) block->free60C.Remove((FEngQueueNode *)header);', '/* omitted */'),
            'acquire-wrong-used': ('++block->used61C;', '--block->used61C;'),
            'acquire-cache-first': [('        block = (FEngResponsePoolBlock *)FEngAllocateQueueNode', '        FEngQueueNode *cached = blocks0.first8;\n        block = (FEngResponsePoolBlock *)FEngAllocateQueueNode'), ('(FEngResponsePoolBlock *)blocks0.first8;\n            block->next4', '(FEngResponsePoolBlock *)cached;\n            block->next4')],
            'release-omit-return': ('block->free60C.InsertAtCursor((FEngQueueNode *)header);', '/* omitted */'),
            'release-omit-used': ('--block->used61C;', '/* omitted */'),
            'release-closed-upper': ('address < (unsigned int)block + 0x60c', 'address <= (unsigned int)block + 0x60c'),
            'base-wrong-flags': ('if (flags & 1) FEngFreeQueueNode(this);', 'if (flags & 2) FEngFreeQueueNode(this);'),
            'block-omit-array-cleanup': ('FEngDestructArrayWithCleanup(headersC, 24, 64, FEngDestructResponseHeader);', '/* omitted */'),
            'insert-omit-count': ('        ++count4;', '        /* omitted */'),
        }
        header_mutations = {
            'header-constructor-wrong-type': ('0x79aa38;', '0x79a9e8;'),
            'header-cleanup-count-gate': ('if (commands)', 'if (commands && header->commandCount10)'),
            'header-cleanup-wrong-cookie': ('12, *base,', '12, header->commandCount10,'),
            'header-cleanup-omit-free': ('FEngFreeQueueNode(base);', '/* omitted */'),
            'header-destroy-omit-virtual': ('((FEngResponseDeletingDestructor *)*(void **)unidentified0)[0](this, 0);', '/* omitted */'),
            'header-destroy-omit-release': ('FEngResponseHeaderPool.Release(this);', '/* omitted */'),
        }
        rejected = []
        for owner, cases in ((args.source, mutations), (args.header_source, header_mutations)):
            for label, edits in cases.items():
                if isinstance(edits, tuple): edits = [edits]
                mutant_source = owner.read_text()
                for before, after in edits:
                    assert before in mutant_source, label; mutant_source = mutant_source.replace(before, after, 1)
                mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(mutant_source)
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler,
                                     '--header-source' if owner == args.header_source else '--source', str(mutant), '--cases', '64'],
                                     cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
                rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
