#!/usr/bin/env python3
"""Compare original script tracks, key references, pool lifecycle and normal script destruction; control only heap calls."""
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

BUILD = ROOT / 'build/analysis/feng-script-tracks'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, SCRIPT, REFERENCE, TRACK, BLOCK, SECOND, NEW, OLD, FRESH, NAME, EVENTS, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x500, 0x1000, 0x2000, 0x5000, 0x8000, 0xc000, 0xe000, 0xf000, 0xf400, 0x2f000, 0x100000))
GLOBAL, TEB, GDT = 0x839bd0, 0x2140000, 0x2150000
PREFIXES = dict(KeyConstruct='?FEngConstructScriptKey@', KeyDestroy='?Destroy@FEngScriptKey@',
    KeyBlockConstruct='?Construct@FEngScriptKeyPoolBlock@', KeyBlockDestruct='?Destruct@FEngScriptKeyPoolBlock@',
    KeyBlockDestroy='?Destroy@FEngScriptKeyPoolBlock@', KeyAcquire='?Acquire@FEngScriptKeyPool@',
    KeyRelease='?Release@FEngScriptKeyPool@', ReferenceRemove='?Remove@FEngReference@',
    ReferenceClear='?ClearOwned@FEngReference@', TrackConstruct='?FEngConstructScriptTrack@',
    TrackDestruct='?FEngDestructScriptTrack@', SetTracks='?SetTracks@FEScript@',
    ScriptConstruct='?FEngConstructScript@', ScriptDestruct='?FEngDestructScript@',
    KeyDestruct='?DestructMessage@FEngQueueNode@', ConstructArray='?FEngConstructArrayWithCleanup@',
    DestructArray='?FEngDestructArrayWithCleanup@', Remove='?Remove@FEngQueueList@',
    Insert='?Insert@FEngQueueList@', InsertAtCursor='?InsertAtCursor@FEngQueueList@',
    BaseDestroy='?DestroyBase@FEngQueueNode@', ClearOwned='?ClearOwned@FEngQueueList@')
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(KeyDestroy=1, KeyBlockDestroy=1, KeyRelease=1, ReferenceRemove=1, SetTracks=1,
            ConstructArray=5, DestructArray=4, Remove=1, Insert=2, InsertAtCursor=1, BaseDestroy=1)
ROOTS = tuple(PREFIXES)[:14]
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngScriptTrack.cpp')
    parser.add_argument('--pool-source', type=Path, default=FRONT / 'FEngScriptKeyPool.cpp')
    parser.add_argument('--cases', type=int, default=256)
    parser.add_argument('--roots', help='comma-separated roots (default: all)')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-script-tracks.json').read_text())
    for name in ('feng-response-array', 'feng-response-command', 'feng-response-pool'):
        manifest['functions'].update(json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + name + '.json')).read_text())['functions'])
    lifecycle = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())['functions']
    for name in ('Remove', 'ClearOwned'): manifest['functions'][name] = lifecycle[name]
    manifest['functions']['KeyDestruct'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-queue-nodes.json').read_text())['functions']['DestructMessage']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.pool_source, FRONT / 'FEngResponsePool.cpp', FRONT / 'FEngResponseHeader.cpp', FRONT / 'FEngResponseArrayLifetime.cpp', FRONT / 'FEngResponseCommand.cpp', FRONT / 'FEngLifecycle.cpp', FRONT / 'FEngQueueNodes.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/EHsc', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {'?FEngResponseHeaderPool@@3UFEngResponsePool@@A': 0x8637a0, '?FEngScriptKeysPool@@3UFEngScriptKeyPool@@A': GLOBAL}
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
    key_callback = next(a for symbol, a in linked['exports'].items() if symbol.startswith('?FEngDestructScriptKey@'))
    state, machines = {}, []; hits = dict.fromkeys(PREFIXES, 0)
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x18000)) + bytes(uc.mem_read(GLOBAL, 16))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x30000); uc.mem_map(STOP, 0x1000); uc.mem_map(TEB, 0x1000); uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)): uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        functions = entries if rebuilt else original
        for table, name in ((0x79a9e8, 'BaseDestroy'), (0x79aa34, 'KeyDestroy'), (0x79b9ec, 'KeyBlockDestroy')):
            put(uc, table, functions[name])
        helpers = {a: n for n, a in functions.items()}
        if rebuilt: helpers[key_callback] = 'KeyDestruct'
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        callback_names = {functions[n]: n for n in ('KeyConstruct', 'KeyDestruct', 'TrackConstruct', 'TrackDestruct', 'ScriptConstruct', 'ScriptDestruct')}
        if rebuilt: callback_names[key_callback] = 'KeyDestruct'
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
                assert any(a <= address < b for a, b in ranges), ('unexpected original execution' if original_run else 'state or call order mismatch', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                if mode & 1: put(uc, SCRIPT + 0x1c, 0x12345678)
                if mode & 2: put(uc, GLOBAL + 8, SECOND)
                result = NEW if argument == 0x2020 else FRESH
            else:
                if mode & 1:
                    put(uc, SCRIPT + 0x2c, NAME + 0x80); put(uc, SCRIPT + 0x28, EVENTS + 0x80)
                if mode & 2: put(uc, SCRIPT + 0x20, FRESH + 0x400)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    roots = tuple(args.roots.split(',')) if args.roots else ROOTS
    assert roots and all(name in ROOTS for name in roots)
    rng = random.Random(0x535210); counts = dict.fromkeys(roots, 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x30000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def init_block(base, free_start):
            putm(base, 0x79b9ec); putm(base + 4, 0); putm(base + 8, 0)
            for i in range(256):
                node = base + 12 + i * 32
                putm(node, 0x79aa34); putm(node + 4, 0xabadcafe); putm(node + 8, 0xabadcafe)
            nodes = [base + 12 + i * 32 for i in range(free_start, 256)]
            putm(base + 0x200c, 0x79a9ec); putm(base + 0x2010, len(nodes))
            putm(base + 0x2014, nodes[0] if nodes else 0); putm(base + 0x2018, nodes[-1] if nodes else 0)
            putm(base + 0x201c, free_start)
            for i, node in enumerate(nodes):
                putm(node + 4, nodes[i + 1] if i + 1 < len(nodes) else 0); putm(node + 8, nodes[i - 1] if i else 0)
        init_block(BLOCK, 256 if trial % 5 == 0 else 8); init_block(SECOND, 255)
        chained = trial % 3 == 0
        putm(BLOCK + 4, SECOND if chained else 0); putm(SECOND + 8, BLOCK if chained else 0)
        global_words = (0x79a9ec, 2 if chained else 1, BLOCK, SECOND if chained else BLOCK)
        nodes = [BLOCK + 12 + i * 32 for i in range(8)]
        def reference(base, pair):
            putm(base, 0x79aa30); memory[base - ARENA + 4] = 0
            putm(base + 8, nodes[pair]); putm(base + 12, nodes[pair + 1])
            putm(nodes[pair] + 4, nodes[pair + 1]); putm(nodes[pair] + 8, 0)
            putm(nodes[pair + 1] + 4, 0); putm(nodes[pair + 1] + 8, nodes[pair])
        reference(REFERENCE, 0)
        memory[REFERENCE - ARENA + 4] = (0, 1, 0x80, 0xff)[trial % 4]
        reference(TRACK + 0x28, 0)
        memory[TRACK - ARENA + 0x2c] = (0, 1, 0x80, 0xff)[trial % 4]
        if trial % 4: putm(TRACK + 0x30, REFERENCE)
        cookie = trial % 4; putm(OLD - 4, cookie)
        for i in range(3):
            reference(OLD + i * 56 + 0x28, (i + 1) * 2)
            memory[OLD - ARENA + i * 56 + 0x2c] = (trial + i) % 2
            if (trial + i) % 2: putm(OLD + i * 56 + 0x30, REFERENCE)
        putm(SCRIPT + 0x1c, (trial // 4) % 5); putm(SCRIPT + 0x20, OLD if trial % 3 else 0)
        putm(SCRIPT + 0x2c, NAME if trial % 4 else 0)
        putm(SCRIPT + 0x28, EVENTS if trial % 2 else 0)
        mode = (trial // 17) % 4
        for name in roots:
            this = BLOCK if name.startswith('KeyBlock') else GLOBAL if name in ('KeyAcquire', 'KeyRelease') else REFERENCE if name.startswith('Reference') else TRACK if name.startswith('Track') else nodes[0] if name.startswith('Key') else SCRIPT
            flags = (0, 1, 0x100, 0x101, 0xffffffff)[trial % 5]
            argv = [flags] if name in ('KeyDestroy', 'KeyBlockDestroy') else [trial % 5] if name == 'SetTracks' else [0 if trial % 4 == 0 else nodes[0]] if name == 'ReferenceRemove' else [nodes[0] + (1 if trial % 7 == 0 else 0)] if name == 'KeyRelease' else []
            if name == 'KeyRelease' and trial % 11 == 0: argv = [BLOCK + 0x200c]
            if name == 'KeyRelease' and trial % 13 == 0: argv = [0]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); uc.mem_write(GLOBAL, struct.pack('<4I', *global_words)); put(uc, TEB, 0xffffffff)
                if name in ('ReferenceClear', 'TrackDestruct', 'SetTracks', 'ScriptDestruct', 'KeyDestroy', 'KeyRelease'):
                    # Use coherent nonempty pool metadata; last-block release is
                    # separately exercised with a complete free list below.
                    put(uc, BLOCK + 0x201c, 8)
                if name in ('KeyRelease', 'KeyDestroy') and trial % 5 == 1:
                    put(uc, BLOCK + 0x201c, 1); put(uc, BLOCK + 0x2010, 255)
                    put(uc, BLOCK + 0x2014, nodes[1]); put(uc, BLOCK + 0x2018, BLOCK + 12 + 255 * 32)
                    for i in range(1, 256):
                        node = BLOCK + 12 + i * 32
                        put(uc, node + 4, node + 32 if i < 255 else 0); put(uc, node + 8, node - 32 if i > 1 else 0)
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear(); state.update(trace=[], mode=mode)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=200000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', trial, name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                result = uc.reg_read(xc.UC_X86_REG_EAX) if name in ('KeyConstruct', 'KeyDestroy', 'KeyBlockConstruct', 'KeyBlockDestroy', 'KeyAcquire', 'TrackConstruct', 'ScriptConstruct') else None
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'key-destroy-omit-base': ('((FEngKeyDeletingDestructor *)vtable0)[0](this, 0);', '/* omitted */'),
            'key-destroy-omit-release': ('FEngScriptKeysPool.Release(this);', '/* omitted */'),
            'key-constructor-wrong-vtable': ('key->vtable0 = (void *)0x79aa34;', 'key->vtable0 = (void *)0x79a9e8;'),
            'reference-remove-clears-flag': ('    if (key) {\n        if (key == value8)', '    value4 = 0;\n    if (key) {\n        if (key == value8)'),
            'reference-omit-first-remove': ('if (key) Remove(key);', '/* omitted */'),
            'track-clear-borrowed': ('if (!reference)', 'if (true)'),
            'track-skip-owned-clear': ('((FEngReference *)track->unidentified28)->ClearOwned();', '/* omitted */'),
            'track-ctor-overwrite-baseline': ('    track->value2C = 0;', '    track->base18[0] = 0;\n    track->value2C = 0;'),
            'tracks-equal-fastpath': ('    void *old = tracks20;', '    if (count == (unsigned int)trackCount1C) return;\n    void *old = tracks20;'),
            'tracks-use-logical-count': ('old, 0x38, *base,', 'old, 0x38, (unsigned int)trackCount1C,'),
            'tracks-wrong-size': ('count * 0x38 + 4', 'count * 0x38'),
            'tracks-no-construct': ('FEngConstructArrayWithCleanup(buffer, 0x38, count, FEngConstructScriptTrack, FEngDestructScriptTrack);', '/* omitted */'),
            'script-ctor-write-time': ('    script->trackCount1C = 0;', '    script->time10 = 0;\n    script->trackCount1C = 0;'),
            'script-dtor-clear-count': ('    script->tracks20 = 0;', '    script->trackCount1C = 0;\n    script->tracks20 = 0;'),
            'script-dtor-skip-null-event-free': ('    FEngFreeQueueNode(script->events28);', '    if (script->events28) FEngFreeQueueNode(script->events28);'),
        }
        pool_mutations = {
            'pool-wrong-capacity': ('i < 256', 'i < 255'),
            'pool-wrong-size': ('FEngAllocateQueueNode(0x2020)', 'FEngAllocateQueueNode(0x2024)'),
            'pool-take-last': ('block->free200C.first8;', 'block->free200C.valueC;'),
            'pool-no-remove': ('if (header) block->free200C.Remove((FEngQueueNode *)header);', '/* omitted */'),
            'pool-release-omit-count': ('--block->used201C;', '/* omitted */'),
            'pool-release-closed-range': ('address < (unsigned int)block + 0x200c', 'address <= (unsigned int)block + 0x200c'),
        }
        rejected = []
        for owner, cases in ((args.source, mutations), (args.pool_source, pool_mutations)):
            for label, edits in cases.items():
                if isinstance(edits, tuple): edits = [edits]
                mutant_source = owner.read_text()
                for before, after in edits:
                    assert before in mutant_source, label; mutant_source = mutant_source.replace(before, after, 1)
                mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(mutant_source)
                selected = 'KeyDestroy' if label.startswith('key-destroy-') else 'KeyConstruct' if label.startswith('key-constructor-') else 'ReferenceRemove' if label.startswith('reference-remove-') else 'ReferenceClear' if label.startswith('reference-') else 'TrackConstruct' if label.startswith('track-ctor-') else 'TrackDestruct' if label.startswith('track-') else 'SetTracks' if label.startswith('tracks-') else 'ScriptConstruct' if label.startswith('script-ctor-') else 'ScriptDestruct' if label.startswith('script-') else 'KeyBlockConstruct' if label == 'pool-wrong-capacity' else 'KeyRelease' if label.startswith('pool-release-') else 'KeyAcquire'
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler,
                                     '--pool-source' if owner == args.pool_source else '--source', str(mutant), '--cases', '64', '--roots', selected],
                                     cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
                rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
