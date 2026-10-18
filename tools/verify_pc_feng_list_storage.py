#!/usr/bin/env python3
"""Verify UG2 list axis/cell resizing with actual constructor iteration, reset and release callees."""
import argparse, hashlib, json, os, random, struct, subprocess, sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-list-storage'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, SELF, AXES, ROWS, OLD, NEW_AXES, NEW, ALT, ALT_AXES, ALT_ROWS, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x1000, 0x1800, 0x2000, 0x4000, 0x5000, 0x8000, 0x9000, 0x9800, 0x1f000, 0x100000))
TEB, GDT = 0x2120000, 0x2130000
PREFIXES = dict(ResizeColumns='?ResizeColumns@FEListObject@', ResizeRows='?ResizeRows@FEListObject@',
                ConstructCell='?FEngConstructCell@', ConstructArray='?FEngConstructArray@',
                ResetCells='?FEngGridResetCells@', ReleaseCells='?ReleaseCells@FEListObject@')
ARGS = dict(ResizeColumns=1, ResizeRows=1, ConstructCell=0, ConstructArray=4, ResetCells=2, ReleaseCells=0)
ROOTS = ('ResizeColumns', 'ResizeRows', 'ConstructCell', 'ConstructArray')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngListStorage.cpp')
    parser.add_argument('--constructor-source', type=Path, default=FRONT / 'FEngCellConstruct.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list-storage.json').read_text())
    manifest['functions']['ResetCells'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid-storage.json').read_text())['functions']['ResetCells']
    manifest['functions']['ReleaseCells'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-typed-lifecycle.json').read_text())['functions']['ReleaseCells']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.constructor_source) + tuple(FRONT / (n + '.cpp') for n in ('FEngGridStorage', 'FEngTypedLifecycle', 'FEngLifecycle', 'FEngQueueNodes')):
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
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0xb000))
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
                state['trace'].append((name, this, argv, snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                state['allocations'] += 1
                if state['allocations'] == 1:
                    result = NEW_AXES
                    if mode & 1: put(uc, SELF + 0x78, ALT_AXES); put(uc, SELF + 0x7c, ALT_ROWS)
                    if mode & 2: put(uc, SELF + 0x60, 2); put(uc, SELF + 0x64, 2)
                else:
                    result = NEW
                    if mode & 8: put(uc, SELF + 0x60, 1); put(uc, SELF + 0x64, 1)
            else:
                result = 0xfeedface
                if not state['freed']:
                    state['freed'] = True
                    if mode & 4: put(uc, SELF + 0x90, ALT)
                    if mode & 16: put(uc, SELF + 0x78, ALT_AXES); put(uc, SELF + 0x7c, ALT_ROWS)
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(ROOTS, 0); rng = random.Random(0x52a910)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 17) % 32
        def putm(a, v): struct.pack_into('<I', memory, a - ARENA, v & 0xffffffff)
        old_columns, old_rows = trial % 7, (trial // 7) % 6
        request = (trial // 3) % 9
        if trial % 64 == 0: old_columns = old_rows = 0; request = 0x40000000; mode = 0
        putm(SELF + 0x60, old_columns); putm(SELF + 0x64, old_rows)
        putm(SELF + 0x78, AXES); putm(SELF + 0x7c, ROWS); putm(SELF + 0x90, 0 if trial % 5 == 0 else OLD)
        for name in ROOTS:
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); put(uc, TEB, 0xffffffff)
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                arguments = [] if name == 'ConstructCell' else [request]
                if name == 'ConstructArray': arguments = [NEW, (0, 0x30, 0x34, 0x40)[trial % 4], (0, 1, 2, 7, 0x80000001, 0xffffffff)[trial % 6], (entries if rebuilt else original)['ConstructCell']]
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
                uc.reg_write(xc.UC_X86_REG_ECX, NEW if name == 'ConstructCell' else SELF); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                state.clear(); state.update(trace=[], allocations=0, freed=False, mode=mode)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=200000)
                except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(arguments) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) if name == 'ConstructCell' else None, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), actual_entry_hits=hits, byte_matching=False)
    if args.mutation_checks:
        report['rejected_mutations'] = []
        mutations = {
            'columns-copy-cells-not-bytes': ('unsigned int remaining = retained >> 2;', 'unsigned int remaining = (retained * 0x30) >> 2;'),
            'rows-copy-cells-not-bytes': ('unsigned int copiedBytes = count60 * retained;', 'unsigned int copiedBytes = count60 * retained * 0x30;'),
            'omit-axis-zero': ('while (words--) *zero++ = 0;', '/* omitted */'),
            'omit-old-axis-free': ('FEngFreeQueueNode(value78);', '/* omitted */'),
            'omit-cell-construction': ('if (cells) FEngConstructArray(cells, 0x30, allocationCount, FEngConstructCell);', '/* omitted */'),
            'omit-cell-reset': ('FEngGridResetCells(cells, allocationCount);', '/* omitted */'),
            'zero-axis-release-first': ('    if (!count) {', '    if (!count) { ReleaseCells();'),
            'wrong-row-count-store': ('    count64 = count;', '    count60 = count;'),
        }
        for label, (before, after) in mutations.items():
            source = args.source.read_text(); assert before in source
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(source.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '256'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            report['rejected_mutations'].append(label)
    if args.mutation_checks:
        mutations = {
            'constructor-clears-unwritten-word': ('    cell->value0[9] = 0xffffffff;', '    cell->value0[9] = 0xffffffff; cell->value0[7] = 0;'),
            'constructor-wrong-sentinel': ('    cell->value0[9] = 0xffffffff;', '    cell->value0[9] = 0;'),
            'constructor-wrong-return': ('    return value;', '    return 0;'),
            'iterator-unsigned-count': ('if ((int)(count - 1) >= 0)', 'if (count)'),
            'iterator-wrong-stride': ('+ stride);', '+ stride + 4);'),
        }
        for label, (before, after) in mutations.items():
            source = args.constructor_source.read_text(); assert before in source
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(source.replace(before, after, 1))
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--constructor-source', str(mutant), '--cases', '128'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            report['rejected_mutations'].append(label)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
