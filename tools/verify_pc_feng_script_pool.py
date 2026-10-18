#!/usr/bin/env python3
"""Compare original script-header pools and deleting cleanup with original execution; control only heap calls."""
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

BUILD = ROOT / 'build/analysis/feng-script-pool'
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
PREFIXES.update(ScriptBlockConstruct='?Construct@FEngScriptPoolBlock@', ScriptBlockDestruct='?Destruct@FEngScriptPoolBlock@', ScriptBlockDestroy='?Destroy@FEngScriptPoolBlock@', ScriptAcquire='?Acquire@FEngScriptPool@', ScriptRelease='?Release@FEngScriptPool@', ScriptDestroy='?Destroy@FEScript@')
ARGS.update(ScriptBlockConstruct=0, ScriptBlockDestruct=0, ScriptAcquire=0, ScriptBlockDestroy=1, ScriptRelease=1, ScriptDestroy=1)
ROOTS = tuple(n for n in PREFIXES if n.startswith('Script') and n not in ('ScriptConstruct', 'ScriptDestruct'))
POOL = 0x839c30
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngScriptPool.cpp')
    parser.add_argument('--pool-source', type=Path, default=FRONT / 'FEngScriptKeyPool.cpp')
    parser.add_argument('--cases', type=int, default=256)
    parser.add_argument('--roots', help='comma-separated roots (default: all)')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-script-tracks.json').read_text())
    for name in ('feng-script-pool', 'feng-response-array', 'feng-response-command', 'feng-response-pool'):
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
    for source in (args.source, FRONT / 'FEngScriptTrack.cpp', args.pool_source, FRONT / 'FEngResponsePool.cpp', FRONT / 'FEngResponseHeader.cpp', FRONT / 'FEngResponseArrayLifetime.cpp', FRONT / 'FEngResponseCommand.cpp', FRONT / 'FEngLifecycle.cpp', FRONT / 'FEngQueueNodes.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/EHsc', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {'?FEngResponseHeaderPool@@3UFEngResponsePool@@A': 0x8637a0, '?FEngScriptKeysPool@@3UFEngScriptKeyPool@@A': GLOBAL, '?FEngScriptHeaderPool@@3UFEngScriptPool@@A': POOL}
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
    state, machines = {}, []; hits = dict.fromkeys(PREFIXES, 0); rebindings = [0]
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x18000)) + bytes(uc.mem_read(GLOBAL, 16)) + bytes(uc.mem_read(POOL, 16))
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
        for table, name in ((0x79a9e8, 'BaseDestroy'), (0x79aa34, 'KeyDestroy'), (0x79b9ec, 'KeyBlockDestroy'), (0x79ad28, 'ScriptDestroy'), (0x79bf54, 'ScriptBlockDestroy')):
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
                if mode & 1: put(uc, POOL + 8, SECOND)
                result = NEW
            else:
                if mode & 2:
                    put(uc, BLOCK + 12 + 0x2c, NAME + 0x80)
                    put(uc, BLOCK + 12 + 0x28, EVENTS + 0x80)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    roots = tuple(args.roots.split(',')) if args.roots else ROOTS
    assert roots and all(n in ROOTS for n in roots)
    rng = random.Random(0x54beb0); counts = dict.fromkeys(roots, 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x30000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def block(base, used):
            putm(base, 0x79bf54); putm(base + 4, 0); putm(base + 8, 0)
            for i in range(32):
                h = base + 12 + i * 52
                putm(h, 0x79ad28)
                for offset in (0x1c, 0x20, 0x24, 0x28, 0x2c): putm(h + offset, 0)
                putm(h + 4, h + 52 if used <= i < 31 else 0)
                putm(h + 8, h - 52 if i > used else 0)
            putm(base + 0x68c, 0x79a9ec); putm(base + 0x690, 32 - used)
            putm(base + 0x694, base + 12 + used * 52 if used < 32 else 0)
            putm(base + 0x698, base + 12 + 31 * 52 if used < 32 else 0)
            putm(base + 0x69c, used)
        used = (1, 8, 31, 32)[trial % 4]
        block(BLOCK, used); block(SECOND, 31)
        chained = trial % 3 == 0
        putm(BLOCK + 4, SECOND if chained else 0); putm(SECOND + 8, BLOCK if chained else 0)
        pool_words = (0x79a9ec, 2 if chained else 1, BLOCK, SECOND if chained else BLOCK)
        header = BLOCK + 12
        putm(header + 0x2c, NAME if trial % 2 else 0)
        putm(header + 0x28, EVENTS if trial % 3 else 0)
        putm(header + 0x24, trial % 7)
        # Zero through three real constructed tracks; no owned key references.
        putm(OLD - 4, trial % 4)
        for i in range(3):
            t = OLD + i * 56
            putm(t + 8, 0x79aa34); putm(t + 0x28, 0x79aa30)
            memory[t - ARENA + 0x2c] = (0, 1, 0x80, 0xff)[(trial + i) % 4]
            putm(t + 0x30, 0); putm(t + 0x34, 0)
        putm(header + 0x1c, 7); putm(header + 0x20, OLD if trial % 3 else 0)
        mode = (trial // 17) % 4
        for name in roots:
            flags = (0, 1, 0x100, 0x101, 0xffffffff)[trial % 5]
            this = BLOCK if name.startswith('ScriptBlock') else POOL if name in ('ScriptAcquire', 'ScriptRelease') else header
            argv = [flags] if name in ('ScriptDestroy', 'ScriptBlockDestroy') else [header + (1 if trial % 11 == 0 else 0)] if name == 'ScriptRelease' else []
            if name == 'ScriptRelease' and trial % 7 == 0: argv = [BLOCK + 0x68c]
            if name == 'ScriptRelease' and trial % 13 == 0: argv = [0]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); uc.mem_write(POOL, struct.pack('<4I', *pool_words)); uc.mem_write(GLOBAL, struct.pack('<4I', 0x79a9ec, 0, 0, 0)); put(uc, TEB, 0xffffffff)
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear(); state.update(trace=[], mode=mode)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=200000)
                except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', trial, name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                result = None if name in ('ScriptBlockDestruct', 'ScriptRelease') else uc.reg_read(xc.UC_X86_REG_EAX)
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'pool-capacity': ('i < 32', 'i < 31', 'ScriptBlockConstruct'),
            'pool-size': ('FEngAllocateQueueNode(0x6a0)', 'FEngAllocateQueueNode(0x6a4)', 'ScriptAcquire'),
            'pool-take-last': ('block->free68C.first8;', 'block->free68C.valueC;', 'ScriptAcquire'),
            'pool-no-remove': ('if (header) block->free68C.Remove((FEngQueueNode *)header);', '/* omitted */', 'ScriptAcquire'),
            'pool-release-no-count': ('--block->used69C;', '/* omitted */', 'ScriptRelease'),
            'pool-release-closed': ('address < (unsigned int)block + 0x68c', 'address <= (unsigned int)block + 0x68c', 'ScriptRelease'),
            'pool-release-no-unlink': ('blocks0.Remove((FEngQueueNode *)block);', '/* omitted */', 'ScriptRelease'),
            'pool-destruct-no-array': ('FEngDestructArrayWithCleanup(headersC, 0x34, 32, FEngDestructScript);', '/* omitted */', 'ScriptBlockDestruct'),
            'pool-destruct-forward': ('FEngDestructArrayWithCleanup(headersC, 0x34, 32, FEngDestructScript);', 'for (unsigned int i = 0; i < 32; ++i) FEngDestructScript(headersC + i);', 'ScriptBlockDestruct'),
            'script-destroy-no-base': ('((FEngPoolDeletingDestructor *)vtable0)[0](this, 0);', '/* omitted */', 'ScriptDestroy'),
            'script-destroy-no-release': ('FEngScriptHeaderPool.Release(this);', '/* omitted */', 'ScriptDestroy'),
            'pool-destroy-wrong-flags': ('if (flags & 1) FEngFreeQueueNode(this);', 'if (flags) FEngFreeQueueNode(this);', 'ScriptBlockDestroy'),
        }
        rejected = []
        for label, (before, after, root) in mutations.items():
            text = args.source.read_text(); assert before in text, label
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '64', '--roots', root], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1200:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
