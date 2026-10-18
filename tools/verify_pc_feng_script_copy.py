#!/usr/bin/env python3
"""Compare original complete script copy construction, including actual owned/borrowed tracks and events; control only heap calls."""
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

BUILD = ROOT / 'build/analysis/feng-script-copy'
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
    ScriptConstruct='?FEngConstructScript@', ScriptDestruct='?FEngDestructScript@', TrackAssign='?Assign@FEScriptTrack@',
    KeyDestruct='?DestructMessage@FEngQueueNode@', ConstructArray='?FEngConstructArrayWithCleanup@',
    DestructArray='?FEngDestructArrayWithCleanup@', Remove='?Remove@FEngQueueList@',
    Insert='?Insert@FEngQueueList@', InsertAtCursor='?InsertAtCursor@FEngQueueList@',
    BaseDestroy='?DestroyBase@FEngQueueNode@', ClearOwned='?ClearOwned@FEngQueueList@')
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(KeyDestroy=1, KeyBlockDestroy=1, KeyRelease=1, ReferenceRemove=1, SetTracks=1, TrackAssign=1,
            ConstructArray=5, DestructArray=4, Remove=1, Insert=2, InsertAtCursor=1, BaseDestroy=1)
PREFIXES.update(ScriptCopy='?CopyConstruct@FEScript@', SetName='?SetName@FEScript@', ResizeEvents='?Resize@FEngScriptEventStorage@', FngHash='?FngHash@')
ARGS.update(ScriptCopy=2, SetName=1, ResizeEvents=1, FngHash=1)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngScriptCopy.cpp')
    parser.add_argument('--pool-source', type=Path, default=FRONT / 'FEngScriptKeyPool.cpp')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-script-tracks.json').read_text())
    for name in ('feng-script-copy', 'feng-script-storage', 'feng-response-array', 'feng-response-command', 'feng-response-pool'):
        manifest['functions'].update(json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + name + '.json')).read_text())['functions'])
    manifest['functions']['FngHash'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid.json').read_text())['functions']['FngHash']
    lifecycle = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())['functions']
    for name in ('Remove', 'ClearOwned'): manifest['functions'][name] = lifecycle[name]
    manifest['functions']['KeyDestruct'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-queue-nodes.json').read_text())['functions']['DestructMessage']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, FRONT / 'FEngScriptTrack.cpp', FRONT / 'FEngScriptStorage.cpp', FRONT / 'FEngHash.cpp', args.pool_source, FRONT / 'FEngResponsePool.cpp', FRONT / 'FEngResponseHeader.cpp', FRONT / 'FEngResponseArrayLifetime.cpp', FRONT / 'FEngResponseCommand.cpp', FRONT / 'FEngLifecycle.cpp', FRONT / 'FEngQueueNodes.cpp'):
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
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x28000)) + bytes(uc.mem_read(GLOBAL, 16))
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
                argv = words(uc, sp + 4, ARGS[name]); this = None if name == 'FngHash' else uc.reg_read(xc.UC_X86_REG_ECX)
                if name.endswith('Array'):
                    this = None
                    argv = argv[:3] + tuple(callback_names.get(a, hex(a)) for a in argv[3:])
                state['trace'].append((name, this, argv, snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected original execution' if original_run else 'state or call order mismatch', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                allocation = state['allocations']; state['allocations'] += 1
                result = ARENA + 0x10000 + allocation * 0x3000
                assert argument < 0x3000
                # Field rebinding at the name/track/event allocator boundaries.
                if mode & 1:
                    put(uc, ARENA + 0x800 + 0x30, 0x87654321)
                    put(uc, ARENA + 0x800 + 0x10, 0x12345678)
                if mode & 2 and argument == 12 * state['event_count']:
                    put(uc, ARENA + 0x800 + 0x28, EVENTS + 0x100)
                if mode & 4:
                    put(uc, TRACK + 0x14, 0xa1b2c3d4)
                    put(uc, BLOCK + 12 + 12, 0x11223344)
                if mode & 8 and argument == state['track_count'] * 56 + 4:
                    put(uc, SCRIPT + 0x1c, max(0, state['track_count'] - 1))
            else:
                if mode & 16: put(uc, ARENA + 0x800 + 0x28, EVENTS + 0x100)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x551be0)
    source_script = ARENA + 0x800
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x30000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        tracks = trial % 4
        events = (trial // 4) % 5
        name = (None, b'', b'Init', b'Paused\x80', b'back')[trial % 5]
        if name is not None: memory[NAME - ARENA:NAME - ARENA + len(name) + 1] = name + b'\0'
        putm(source_script + 0x1c, tracks); putm(source_script + 0x20, TRACK if tracks else 0)
        putm(source_script + 0x24, events); putm(source_script + 0x28, EVENTS if events else 0)
        putm(source_script + 0x2c, NAME if name is not None else 0)
        # A coherent preexisting pool contains source-owned keys and free slots.
        putm(BLOCK, 0x79b9ec); putm(BLOCK + 4, 0); putm(BLOCK + 8, 0)
        putm(BLOCK + 0x200c, 0x79a9ec); putm(BLOCK + 0x2010, 248)
        putm(BLOCK + 0x2014, BLOCK + 12 + 8 * 32); putm(BLOCK + 0x2018, BLOCK + 12 + 255 * 32)
        putm(BLOCK + 0x201c, 8)
        for i in range(256):
            node = BLOCK + 12 + i * 32
            putm(node, 0x79aa34)
            putm(node + 4, node + 32 if 8 <= i < 255 else 0)
            putm(node + 8, node - 32 if i > 8 else 0)
        for i in range(tracks):
            track = TRACK + i * 56
            flag = (0, 1, 0x80, 0xff)[(trial // 16 + i) % 4]
            memory[track - ARENA + 0x2c] = flag
            first, last = BLOCK + 12 + i * 64, BLOCK + 44 + i * 64
            putm(first + 4, last); putm(first + 8, 0); putm(last + 4, 0); putm(last + 8, first)
            target = first if not flag else REFERENCE
            putm(track + 0x30, target if trial % 7 else 0); putm(track + 0x34, last)
        shallow = (0, 1, 0x80, 0xff)[(trial // 64) % 4]
        mode = (trial // 17) % 32
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(GLOBAL, struct.pack('<4I', 0x79a9ec, 1, BLOCK, BLOCK)); put(uc, TEB, 0xffffffff)
            uc.mem_write(STACK, struct.pack('<3I', STOP, SCRIPT if trial % 23 == 0 else source_script, shallow | ((trial & 0xffffff) << 8)))
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, SCRIPT)
            state.clear(); state.update(trace=[], mode=mode, allocations=0, event_count=events, track_count=tracks)
            try: uc.emu_start((entries if rebuilt else original)['ScriptCopy'], STOP, count=200000)
            except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', trial, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 12
            assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
            assert words(uc, TEB)[0] == 0xffffffff
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX), snapshot(uc), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial,
            [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
            [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
    report = dict(compiler=args.compiler, comparisons=args.cases, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'copy-self-fastpath': ('    next4 = (FEScript *)0xabadcafe;', '    if (other == this) return this;\n    next4 = (FEScript *)0xabadcafe;'),
            'copy-call-constructor': ('    next4 = (FEScript *)0xabadcafe;', '    FEngConstructScript(this);\n    next4 = (FEScript *)0xabadcafe;'),
            'copy-omit-name': ('    SetName(other->name2C);', '    /* omitted */'),
            'copy-keep-name-hash': ('    hash30 = other->hash30;', '    /* omitted */'),
            'copy-chain': ('    flags14 = other->flags14;', '    chain18 = other->chain18;\n    flags14 = other->flags14;'),
            'copy-shallow-test-bit': ('    if (shallow)', '    if (shallow & 1)'),
            'copy-shallow-inline-vtable': ('            destination[1] = source[1];', '            destination[0] = source[0];\n            destination[1] = source[1];'),
            'copy-shallow-omit-links': ('            destination[1] = source[1];', '            /* omitted */'),
            'copy-shallow-direct-head': ('void *target = (char *)other->tracks20 + offset + 0x28;', 'void *target = ((FEScriptTrack *)((char *)other->tracks20 + offset))->value30;'),
            'copy-shallow-omit-clear': ('if (!reference->value4) reference->ClearOwned();', '/* omitted */'),
            'copy-deep-omit-assign': ('((FEScriptTrack *)((char *)tracks20 + offset))->Assign((const FEScriptTrack *)((const char *)other->tracks20 + offset));', '/* omitted */'),
            'copy-events-omit-resize': ('((FEngScriptEventStorage *)&eventCount24)->Resize(other->eventCount24);', '/* omitted */'),
            'copy-cache-time-before-name': [('    next4 = (FEScript *)0xabadcafe;', '    int earlyTime = other->time10;\n    next4 = (FEScript *)0xabadcafe;'), ('    time10 = other->time10;', '    time10 = earlyTime;')],
            'copy-cache-event-data-before-resize': [('    ((FEngScriptEventStorage *)&eventCount24)->Resize(other->eventCount24);', '    const void *earlyEvents = other->events28;\n    ((FEngScriptEventStorage *)&eventCount24)->Resize(other->eventCount24);'), ('    const unsigned int *source = (const unsigned int *)other->events28;', '    const unsigned int *source = (const unsigned int *)earlyEvents;')],
            'copy-use-source-track-count': ('while (index < (unsigned int)trackCount1C)', 'while (index < (unsigned int)other->trackCount1C)'),
            'copy-events-short': ('unsigned int bytes = (unsigned int)eventCount24 * 12;', 'unsigned int bytes = (unsigned int)eventCount24 * 8;'),
        }
        rejected = []
        for label, edits in mutations.items():
            if isinstance(edits, tuple): edits = [edits]
            text = args.source.read_text()
            for before, after in edits:
                assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '256'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1200:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
