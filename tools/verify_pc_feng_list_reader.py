#!/usr/bin/env python3
"""Verify complete UG2 list-property dispatch with actual allocation, colour, cell and selection callees."""
import argparse, hashlib, json, os, random, struct, subprocess, sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-list-reader'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, SELF, AXES, ROWS, OLD, NEW_AXES, NEW, ALT, ALT_AXES, ALT_ROWS, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x1000, 0x1800, 0x2000, 0x4000, 0x5000, 0x8000, 0x9000, 0x9800, 0x1f000, 0x100000))
READER, COLOUR, INPUT, OLD_TEXT, PROPERTY, HEAP = (ARENA + n for n in (0x400, 0x600, 0x6000, 0x7000, 0xa000, 0x10000))
TAGS = (0x5443, 0x6343, 0x634c, 0x644c, 0x6943, 0x7243, 0x724c, 0x7343, 0x734c, 0x7443, 0x764c, 0x774c, 0, 0xffff, 0x644d, 0x7444)
TEB, GDT = 0x2120000, 0x2130000
PREFIXES = dict(ResizeColumns='?ResizeColumns@FEListObject@', ResizeRows='?ResizeRows@FEListObject@',
                ConstructCell='?FEngConstructCell@', ConstructArray='?FEngConstructArray@',
                ResetCells='?FEngGridResetCells@', ReleaseCells='?ReleaseCells@FEListObject@')
ARGS = dict(ResizeColumns=1, ResizeRows=1, ConstructCell=0, ConstructArray=4, ResetCells=2, ReleaseCells=0)
PREFIXES.update(SetCellType='?SetCellType@FEListObject@', SetCellColour='?SetCellColour@FEListObject@',
                SetCellPair4='?SetCellPair4@FEListObject@', SetCellTripleC='?SetCellTripleC@FEListObject@',
                SetCellFour20='?SetCellFour20@FEListObject@', SetCellText='?SetCellText@FEListObject@',
                Unpack='?Unpack@FEngColour@', Pack='?Pack@FEngColour@', ReadProperty='?ReadProperty@FEngListPropertyReader@',
                SetColumn='?SetColumn@FEListObject@', SetRow='?SetRow@FEListObject@', AdvanceCell='?AdvanceCell@FEListObject@', SetWrap='?SetWrap@FEListObject@')
ARGS.update(SetCellType=1, SetCellColour=1, SetCellPair4=1, SetCellTripleC=3, SetCellFour20=1, SetCellText=1,
            Unpack=1, Pack=0, ReadProperty=1, SetColumn=1, SetRow=1, AdvanceCell=0, SetWrap=1)
ROOTS = ('SetCellType', 'SetCellColour', 'SetCellPair4', 'SetCellTripleC', 'SetCellFour20', 'SetCellText', 'Unpack', 'ReadProperty')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngListReader.cpp')
    parser.add_argument('--cells-source', type=Path, default=FRONT / 'FEngListCells.cpp')
    parser.add_argument('--colour-source', type=Path, default=FRONT / 'FEngColour.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list-reader.json').read_text())
    manifest['functions'].update(json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list-storage.json').read_text())['functions'])
    dependency = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list.json').read_text())['functions']
    for name in ('SetColumn', 'SetRow', 'AdvanceCell', 'SetWrap'): manifest['functions'][name] = dependency[name]
    manifest['functions']['ResetCells'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid-storage.json').read_text())['functions']['ResetCells']
    manifest['functions']['ReleaseCells'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-typed-lifecycle.json').read_text())['functions']['ReleaseCells']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.cells_source, args.colour_source) + tuple(FRONT / (n + '.cpp') for n in ('FEngListStorage', 'FEngListState', 'FEngCellConstruct', 'FEngGridStorage', 'FEngTypedLifecycle', 'FEngLifecycle', 'FEngQueueNodes')):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for s in read_object(obj)['symbols'].values():
            if s['section'] != 0: continue
            if s['name'].startswith(('?FEngAllocateQueueNode@', '?FEngGridAllocateCells@')): externals[s['name']] = 0x575620
            elif s['name'].startswith(('?FEngFreeQueueNode@', '?FEngGridFreeCells@')): externals[s['name']] = 0x575630
            elif s['name'].startswith('?FEngObjectStateManager@'): externals[s['name']] = 0x863770
            elif s['name'].startswith('?FEngObjectDestructionObserver@'): externals[s['name']] = 0x838474
            elif s['name'].startswith('?fengStateKeys7992F4@'): externals[s['name']] = 0x7992f4
            elif s['name'].startswith('?FEngStateOffset@'): externals[s['name']] = 0x5046f0
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x18000))
    hits = dict.fromkeys(PREFIXES, 0)
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
        helpers = {a: n for n, a in (entries if rebuilt else original).items()}
        ranges = linked['ranges'] if rebuilt else [(a, a + manifest['functions'][n]['size']) for n, a in original.items()]
        ctor = entries['ConstructCell'] if rebuilt else original['ConstructCell']
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, ctor=ctor, original_run=not rebuilt):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                if original_run: hits[name] += 1
                argv = words(uc, sp + 4, ARGS[name])
                if name == 'ConstructArray':
                    assert argv[3] == ctor
                    argv = argv[:3] + ('ConstructCell',)
                this = None if name in ('ConstructArray', 'ResetCells') else uc.reg_read(xc.UC_X86_REG_ECX)
                if name == 'SetWrap': argv = (argv[0] & 255,)
                if name in ('Unpack', 'Pack') and STACK - 0x1000 <= this < STACK:
                    this = ('scratch-colour', words(uc, this, 4)) if name == 'Pack' else 'scratch-colour'
                if name in ('SetCellColour', 'SetCellPair4', 'SetCellFour20') and STACK - 0x1000 <= argv[0] < STACK:
                    argv = (('scratch-values', words(uc, argv[0], 2 if name == 'SetCellPair4' else 4)),)
                state['trace'].append((name, this, argv, snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                state['allocations'] += 1
                result = HEAP + state['allocations'] * 0x1000
                if mode & 1: put(uc, SELF + 0x90, ALT)
                if mode & 2: put(uc, READER + 0xc, 0xdeadbeef)
                if mode & 8: put(uc, state['cell'] + 0x20, OLD_TEXT + 0x80)
                if mode & 16 and state['root'] == 'ReadProperty' and state['tag'] == 0x644c: put(uc, PROPERTY + 8, 2)
            else:
                result = 0xfeedface
                if not state['freed']:
                    state['freed'] = True
                    if mode & 4:
                        put(uc, SELF + 0x90, ALT); put(uc, SELF + 0x88, 0); put(uc, SELF + 0x8c, 0)
                    if mode & 16: put(uc, SELF + 0x78, ALT_AXES); put(uc, SELF + 0x7c, ALT_ROWS)
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(ROOTS, 0); rng = random.Random(0x52b740); tags = dict.fromkeys(map(hex, TAGS), 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 17) % 32
        def putm(a, v): struct.pack_into('<I', memory, a - ARENA, v & 0xffffffff)
        columns, rows = 3, 2
        x, y = trial % columns, (trial // 3) % rows
        if trial % 64 == 0: columns, rows, x, y = 0x40000000, 4, 1, 4
        # Wrapped-index direct helper cases are separate from reader bounds/AdvanceCell.
        putm(SELF + 0x60, columns); putm(SELF + 0x64, rows)
        putm(SELF + 0x88, x); putm(SELF + 0x8c, y)
        putm(SELF + 0x78, AXES); putm(SELF + 0x7c, ROWS); putm(SELF + 0x90, OLD)
        cell = OLD + (((y * columns + x) & 0xffffffff) * 0x30 & 0xffffffff)
        for base in (OLD, ALT):
            for ci in range(12):
                putm(base + ci * 0x30 + 0x18, (trial // 5 + ci) % 4)
                putm(base + ci * 0x30 + 0x20, 0 if trial % 3 == 0 else OLD_TEXT)
        values = (0xffffffff, 0, 1, 255, 256, 0x80000000, 0x7fffffff)
        for i in range(4): putm(COLOUR + i * 4, values[(trial + i) % len(values)])
        text = [0x80 + i for i in range(trial % 17)] + [0]
        raw_text = struct.pack('<' + 'H' * len(text), *text)
        for a in (INPUT, OLD_TEXT): memory[a - ARENA:a - ARENA + len(raw_text)] = raw_text
        tag = TAGS[trial % len(TAGS)]
        putm(READER + 0xc, SELF)
        for offset in (0x24, 0x28): putm(READER + offset, 0xffffffff if trial % 3 == 0 else 0)
        putm(READER + 0x2c, (0xffffffff, 0, 1)[trial % 3])
        struct.pack_into('<HH', memory, PROPERTY - ARENA, tag, (0, 1, 0xffff)[trial % 3])
        floats = (0, 0x80000000, 0x3fa00000, 0x7fc00001, 0x7f800001, 0x7f800000, 0xff800000)
        for i in range(4): putm(PROPERTY + 4 + 4 * i, floats[(trial // 16 + i) % len(floats)] if tag in (0x734c, 0x764c) else rng.getrandbits(32))
        if tag == 0x644c: putm(PROPERTY + 4, (trial // 16) % 5); putm(PROPERTY + 8, (trial // 80) % 4)
        if tag == 0x7443: memory[PROPERTY + 4 - ARENA:PROPERTY + 4 - ARENA + len(raw_text)] = raw_text
        if tag == 0x5443: putm(PROPERTY + 4, (trial // 16) % 4)
        if tag not in TAGS[:12] and trial % 2: putm(READER + 0xc, 0)
        for name in ROOTS:
            fixture = bytearray(memory)
            if name == 'ReadProperty' and trial % 64 == 0:
                struct.pack_into('<4I', fixture, SELF + 0x60 - ARENA, 3, 2, 0, 0)
                struct.pack_into('<2I', fixture, SELF + 0x88 - ARENA, 0, 0)
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(fixture)); put(uc, TEB, 0xffffffff)
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_FPCW, 0x37f); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
                argument = dict(SetCellType=trial % 5, SetCellColour=COLOUR, SetCellPair4=INPUT,
                                SetCellFour20=INPUT, SetCellText=0 if trial % 7 == 0 else INPUT,
                                Unpack=(trial * 0x1234567) & 0xffffffff, ReadProperty=PROPERTY).get(name, 0)
                # Stable scalar inputs across machines; the random seed belongs to each trial.
                if name == 'SetCellPair4' and trial % 2: argument = cell
                if name == 'SetCellFour20' and trial % 2: argument = cell + 0x1c
                arguments = [argument] if name != 'SetCellTripleC' else [0x12345678, 0x80000000, trial]
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
                uc.reg_write(xc.UC_X86_REG_ECX, READER if name == 'ReadProperty' else COLOUR if name == 'Unpack' else SELF)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                state.clear(); state.update(trace=[], allocations=0, freed=False, mode=mode, cell=cell, root=name, tag=tag)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=200000)
                except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, hex(tag), str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(arguments) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                outputs.append((snapshot(uc), state['trace'], uc.reg_read(xc.UC_X86_REG_EAX) if name == 'Unpack' else None))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode, hex(tag),
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:12],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
            counts[name] += 1
        tags[hex(tag)] += 1
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), actual_entry_hits=hits, tags=tags, byte_matching=False)
    if args.mutation_checks:
        report['rejected_mutations'] = []
        mutations = {
            'reader-uses-length': ('    unsigned int tag = *property;', '    if (!property[1]) return; unsigned int tag = *property;'),
            'reader-skips-cell-advance': ('if (value2C) list->AdvanceCell();', 'if (false) list->AdvanceCell();'),
            'reader-wrong-column-reset': ('value24 = 0xffffffff;', 'value24 = 0;'),
            'reader-wrong-row-offset': ('&entry->value8 = payload[1];', '&entry->value4 = payload[1];'),
            'reader-Cr-wrong-triple': ('SetCellTripleC(0, 0, payload[0])', 'SetCellTripleC(payload[0], 0, 0)'),
            'reader-omits-Ls-float': ('list->value84 = second;', 'list->value84 = 0;'),
            'reader-Lw-inverted': ('SetWrap(payload[0] != 0)', 'SetWrap(payload[0] == 0)'),
        }
        for label, (before, after) in mutations.items():
            source = args.source.read_text(); assert before in source, label
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(source.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '128'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            report['rejected_mutations'].append(label)
        mutations = {
            'type-frees-unchanged': ('if (previous != value)', 'if (true)'),
            'type-frees-all-kinds': ('previous == 2 &&', 'true &&'),
            'type-omits-free': ('FEngFreeQueueNode(*(void **)(cell + 0x20));', '/* omitted */'),
            'colour-wrong-offset': ('+ offset) = colour;', '+ offset + 4) = colour;'),
            'pair-wrong-offset': ('+ offset + 4);', '+ offset + 8);'),
            'four-wrong-offset': ('+ offset + 0x20);', '+ offset + 0x24);'),
            'text-omits-free': ('FEngFreeQueueNode(*out);', '/* omitted */'),
            'text-omits-terminator': ('length * 2 + 2', 'length * 2'),
            'text-recomputes-cell': ('        *out = buffer;', '        out = (void **)((unsigned int)value90 + (value8C * count60 + value88) * 0x30 + 0x20); *out = buffer;'),
        }
        for label, (before, after) in mutations.items():
            source = args.cells_source.read_text(); assert before in source, label
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(source.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--cells-source', str(mutant), '--cases', '128'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            report['rejected_mutations'].append(label)
    if args.mutation_checks:
        source = args.colour_source.read_text()
        before = '    return this;'
        assert before in source
        mutant = BUILD / 'mutation-unpack-wrong-return.cpp'
        mutant.write_text(source.replace(before, '    return 0;', 1))
        run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--colour-source', str(mutant), '--cases', '128'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (BUILD / 'mutation-unpack-wrong-return.log').write_text(run.stdout)
        assert run.returncode and 'state or call order mismatch' in run.stdout, ('unpack wrong return', 'not a semantic rejection')
        report['rejected_mutations'].append('unpack-wrong-return')
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
