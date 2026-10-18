#!/usr/bin/env python3
"""Compare complete original response reader and its actual recovered dependency chain; control only heap calls."""
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

BUILD = ROOT / 'build/analysis/feng-response-reader'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, POOL, LIST, BLOCK, SECOND, NEW, COMMANDS, ALTERNATE, TEXT, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x200, 0x2000, 0x3000, 0x5000, 0x9000, 0x9400, 0xa000, 0x1f000, 0x100000))
CONTEXT, OBJECT, OTHER_OBJECT, PACKAGE, OTHER_PACKAGE, INPUT, HEAP = (ARENA + x for x in (0x400, 0x600, 0x800, 0xa00, 0xc00, 0xc000, 0xe000))
GLOBAL, TEB, GDT = 0x8637a0, 0x2120000, 0x2130000
PREFIXES = dict(Read='?Read@FEngResponseReader@', FindResponse='?FindResponseByMessage@HudVisibilityObject@', AppendResponse='?AppendResponse@FEPackage@', HeaderConstruct='?FEngConstructResponseHeader@', HeaderDestruct='?FEngDestructResponseHeader@',
    HeaderDestroy='?Destroy@FEResponse@', BlockConstruct='?Construct@FEngResponsePoolBlock@',
    BlockDestruct='?Destruct@FEngResponsePoolBlock@', BlockDestroy='?Destroy@FEngResponsePoolBlock@',
    Acquire='?Acquire@FEngResponsePool@', Release='?Release@FEngResponsePool@', Insert='?Insert@FEngQueueList@',
    InsertAtCursor='?InsertAtCursor@FEngQueueList@', BaseDestroy='?DestroyBase@FEngQueueNode@',
    ConstructArray='?FEngConstructArrayWithCleanup@', DestructArray='?FEngDestructArrayWithCleanup@',
    Construct='?FEngConstructResponseCommand@', Destruct='?FEngDestructResponseCommand@',
    Resize='?ResizeCommands@FEResponse@', SetString='?SetString@FEResponseCommand@', Assign='?Assign@FEResponseCommand@', Remove='?Remove@FEngQueueList@', ClearOwned='?ClearOwned@FEngQueueList@')
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(Read=3, FindResponse=1, AppendResponse=1, HeaderDestroy=1, BlockDestroy=1, Release=1, Insert=2, InsertAtCursor=1,
            BaseDestroy=1, Resize=1, SetString=1, Assign=1, ConstructArray=5, DestructArray=4, Remove=1)
ROOTS = ('Read', 'FindResponse', 'AppendResponse')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngResponseReader.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response-reader.json').read_text())
    for name in ('feng-response-array', 'feng-response-command', 'feng-response-pool'):
        manifest['functions'].update(json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + name + '.json')).read_text())['functions'])
    lifecycle = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())['functions']
    for name in ('Remove', 'ClearOwned'): manifest['functions'][name] = lifecycle[name]
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, FRONT / 'FEngResponsePool.cpp', FRONT / 'FEngResponseHeader.cpp', FRONT / 'FEngResponseArray.cpp', FRONT / 'FEngResponseArrayLifetime.cpp', FRONT / 'FEngResponseCommand.cpp', FRONT / 'FEngLifecycle.cpp', FRONT / 'FEngQueueNodes.cpp'):
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
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x18000)) + bytes(uc.mem_read(GLOBAL, 16))
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
            if mode & 1:
                put(uc, CONTEXT + 0xc, OTHER_OBJECT); put(uc, CONTEXT, OTHER_PACKAGE)
            if mode & 2: uc.mem_write(CONTEXT + 0x18, b'\x01')
            if mode & 4:
                uc.mem_write(INPUT + 2, struct.pack('<H', 4)); put(uc, INPUT + 4, 0x77777777)
            if address == 0x575620:
                if argument == 0x620: result = NEW
                else:
                    result = HEAP + state['allocations'] * 0x200
                    state['allocations'] += 1
            else: result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x54d1d0); counts = dict.fromkeys(ROOTS, 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        for base in (OBJECT, OTHER_OBJECT):
            for offset, value in ((0x34, 0x79a9ec), (0x38, 0), (0x3c, 0), (0x40, 0)): putm(base + offset, value)
        for base in (PACKAGE, OTHER_PACKAGE):
            for offset in (0x58, 0x5c, 0x60): putm(base + offset, 0)
        putm(CONTEXT, PACKAGE); putm(CONTEXT + 0xc, OBJECT)
        memory[CONTEXT - ARENA + 0x18] = (0, 1, 0x80, 0xff)[trial % 4]
        putm(BLOCK, 0x79bf50); putm(BLOCK + 4, 0); putm(BLOCK + 8, 0)
        for i in range(64):
            node = BLOCK + 12 + i * 24
            for offset, value in ((0, 0x79aa38), (4, 0xabadcafe), (8, 0xabadcafe), (12, (0x1234, 0x1234, 0x2222, 0x3333)[i % 4]), (16, 0), (20, 0)):
                putm(node + offset, value)
            if i < 4:
                array = COMMANDS + i * 0x100
                putm(array - 4, (trial + i) % 4)
                putm(node + 16, (trial + i) % 5); putm(node + 20, array if (trial + i) % 3 else 0)
                for j in range(4):
                    putm(array + j * 12, (0x200, 0x203, 0x204, 0)[j])
                    putm(array + j * 12 + 4, TEXT + j * 64)
                    text = b'old-' + bytes([65 + j]) + b'\0'
                    memory[TEXT - ARENA + j * 64:TEXT - ARENA + j * 64 + len(text)] = text
        headers = [BLOCK + 12 + i * 24 for i in range(4)]
        for first, base, count_offset, first_offset, last_offset in ((0, OBJECT, 0x38, 0x3c, 0x40), (2, PACKAGE, 0x58, 0x5c, 0x60)):
            putm(base + count_offset, 2); putm(base + first_offset, headers[first]); putm(base + last_offset, headers[first + 1])
            putm(headers[first] + 4, headers[first + 1]); putm(headers[first] + 8, 0)
            putm(headers[first + 1] + 4, 0); putm(headers[first + 1] + 8, headers[first])
        free_nodes = [] if trial % 5 == 0 else [BLOCK + 12 + i * 24 for i in range(4, 64)]
        putm(BLOCK + 0x60c, 0x79a9ec); putm(BLOCK + 0x610, len(free_nodes))
        putm(BLOCK + 0x614, free_nodes[0] if free_nodes else 0); putm(BLOCK + 0x618, free_nodes[-1] if free_nodes else 0)
        putm(BLOCK + 0x61c, 64 - len(free_nodes))
        for i, node in enumerate(free_nodes):
            putm(node + 4, free_nodes[i + 1] if i + 1 < len(free_nodes) else 0); putm(node + 8, free_nodes[i - 1] if i else 0)
        global_words = (0x79a9ec, 1, BLOCK, BLOCK)
        def word(value): return struct.pack('<I', value & 0xffffffff)
        def tag(key, value): return struct.pack('<HH', key, len(value)) + value
        # First Mi's extra payload is a valid nested MC. Heap callbacks can
        # shorten Mi's length, proving that the loop reloads length after calls.
        first_key = 0x1234 if trial % 3 else 0x9999
        data = tag(0x694d, word(first_key) + tag(0x434d, word(1)))
        for i, key in enumerate((first_key, 0x2222, first_key)):
            if i: data += tag(0x694d, word(key))
            if i == 2: data += tag(0x7452, word(0x12345678))
            data += tag(0x434d, word(2 + i))
            if i == 2: data += tag(0x7452, word(0x87654321))
            for j in range(2 + i):
                own = j % 2 == 0
                data += tag(0x6952, word(0x200 + (j % 3) if own else 0x203))
                if own:
                    data += tag(0x7352, bytes([0x80 + j, 65 + i]) + b'\0')
                    if j == 0: data += tag(0x7352, b'replace\0')
                else: data += tag(0x7552, word(0x11223344 + j))
                data += tag(0x7452, word(0xaabb0000 + j))
                data += tag(0x1, b'x')  # Odd-size unknown property.
        memory[INPUT - ARENA:INPUT - ARENA + len(data)] = data
        mode = (trial // 17) % 8
        package_mode = (0, 1, 0x100, 0x101, 0xff)[trial % 5]
        for name in ROOTS:
            this = CONTEXT if name == 'Read' else OBJECT if name == 'FindResponse' else PACKAGE
            if name == 'Read':
                argv = [INPUT, len(data), package_mode]
                if trial % 29 == 0: argv = [0xdeadbeef, 0, package_mode]
                if trial % 31 == 0: argv = [0xfffffff0, 32, package_mode]
            elif name == 'FindResponse': argv = [first_key]
            else: argv = [0 if trial % 4 == 0 else BLOCK + 12 + 63 * 24]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); uc.mem_write(GLOBAL, struct.pack('<4I', *global_words)); put(uc, TEB, 0xffffffff)
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear(); state.update(trace=[], mode=mode, allocations=0)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=200000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                result = uc.reg_read(xc.UC_X86_REG_EAX) & 255 if name == 'Read' else uc.reg_read(xc.UC_X86_REG_EAX) if name == 'FindResponse' else None
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, actual_entry_hits=hits,
                  byte_matching=False, exceptional_unwind_verified=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        mutations = {
            'lookup-use-guid': ('response->messageC != message', 'response->commandCount10 != message'),
            'lookup-skip-first': ('FEResponse *response = responses3C;', 'FEResponse *response = responses3C ? responses3C->next4 : 0;'),
            'append-wrong-count': ('++responseCount58;', '--responseCount58;'),
            'append-omit-tail': ('responseCursor60 = response;', '/* omitted */'),
            'reader-ignore-merge-flag': ('!packageMode && value18 &&', '!packageMode &&'),
            'reader-merge-package': ('!packageMode && value18 &&', 'value18 &&'),
            'reader-omit-inline-release': ('FEngFreeQueueNode(base);', '/* omitted */'),
            'reader-clear-only-count': ('response->commands14 = 0;\n                response->commandCount10 = 0;', 'response->commandCount10 = 0;'),
            'reader-omit-reset-index': ('index = 0xffffffff;\n        } else', '/* omitted */\n        } else'),
            'reader-clear-command-on-mi': ('            index = 0xffffffff;', '            command = 0;\n            index = 0xffffffff;'),
            'reader-clear-command-on-mc': ('            response->ResizeCommands(*payload);', '            response->ResizeCommands(*payload);\n            command = 0;'),
            'reader-wrong-cell-size': ('index * 12', 'index * 4'),
            'reader-omit-string': ('command->SetString((const char *)payload);', '/* omitted */'),
            'reader-wrong-target': ('command->target = *payload;', 'command->parameter = *payload;'),
            'reader-fresh-message': ('response->messageC = message;', 'response->messageC = *payload;'),
            'reader-cache-length': [('        unsigned int tag = property[0];', '        unsigned int length = property[1];\n        unsigned int tag = property[0];'), ('property + property[1] + 4', 'property + length + 4')],
            'reader-wrong-return': ('    return 1;', '    return 0;'),
        }
        rejected = []
        for label, edits in mutations.items():
            if isinstance(edits, tuple): edits = [edits]
            mutant_source = args.source.read_text()
            for before, after in edits:
                assert before in mutant_source, label; mutant_source = mutant_source.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(mutant_source)
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '128'],
                                 cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
