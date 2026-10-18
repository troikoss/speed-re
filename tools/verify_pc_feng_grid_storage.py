#!/usr/bin/env python3
"""Compare UG2 grid resizing with actual reset/setter callees and allocator seams."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-grid-storage'
ARENA, SELF, OLD, NEW, POOL, POOL_B, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x1000, 0x3000, 0x7000, 0x7800, 0xf000, 0x100000))
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'SetLogicalColumns': '?SetLogicalColumns@FEGridObject@', 'SetLogicalRows': '?SetLogicalRows@FEGridObject@',
            'ResetCells': '?FEngGridResetCells@', 'ResizeCells': '?ResizeCells@FEGridObject@'}
ARGS = {'SetLogicalColumns': 1, 'SetLogicalRows': 1, 'ResetCells': 2, 'ResizeCells': 2}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngGridStorage.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid-storage.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        if s['name'].startswith('?FEngGridAllocateCells@'): externals[s['name']] = 0x575620
        elif s['name'].startswith('?FEngGridFreeCells@'): externals[s['name']] = 0x575630
        else: raise AssertionError(('unexpected external', s['name']))
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    markers = {0x518bd9: 'column-positive-wrap', 0x518be8: 'column-negative-wrap', 0x518bf6: 'column-small-total',
               0x518c2c: 'row-positive-wrap', 0x518c3b: 'row-negative-wrap', 0x518c49: 'row-small-total',
               0x52bb80: 'partial-construction', 0x52bc6a: 'retained-row-copy',
               0x52bc86: 'retired-column-payload', 0x52bd16: 'retired-row-payload', 0x52bd52: 'old-buffer-free'}
    coverage = dict.fromkeys(markers.values(), 0); state = {}; machines = []
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, 4 * count))
    def put(uc, offset, value): uc.mem_write(SELF + offset, struct.pack('<I', value & 0xffffffff))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, None if name == 'ResetCells' else uc.reg_read(xc.UC_X86_REG_ECX),
                                       words(uc, sp + 4, ARGS[name]), bytes(uc.mem_read(ARENA, 0x9000))))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, bytes(uc.mem_read(ARENA, 0x9000))))
            if address == 0x575620:
                mode = state['mode']
                if mode & 1: put(uc, 0x68, words(uc, SELF + 0x68)[0] ^ 1)
                if mode & 2: put(uc, 0x60, 2)
                if mode & 4: put(uc, 0x64, 3)
                if mode & 8: put(uc, 0x7c, 7); put(uc, 0x80, 9)
                if mode & 16: put(uc, 0x8c, OLD + 0x800)
                if mode & 32: put(uc, 0x98, 180); put(uc, 0x9c, POOL_B)
                result = 0 if state['null_allocation'] else NEW
            else:
                assert argument in (OLD, 0), hex(argument)
                if state['mode'] & 64: put(uc, 0x68, words(uc, SELF + 0x68)[0] ^ 0x40)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(entries, 0)
    def compare(name, memory, arguments, mode=0, null_allocation=False):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *[a & 0xffffffff for a in arguments]))
            uc.reg_write(xc.UC_X86_REG_ECX, SELF); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, null_allocation=null_allocation)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                if not rebuilt: raise
                raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + (0 if name == 'ResetCells' else 4 * len(arguments))
            assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0x9000)), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, arguments, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:20],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
        counts[name] += 1
    rng = random.Random(0x52bb30)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x10000))
        def field(offset, value): struct.pack_into('<I', memory, SELF - ARENA + offset, value & 0xffffffff)
        old_columns, old_rows = 1 + trial % 6, 1 + (trial // 6) % 6
        columns, rows = 1 + (trial // 36) % 6, 1 + (trial // 216) % 6
        flags = (trial // 3) % 16; mode = (trial // 32) % 128
        field(0x60, old_columns); field(0x64, old_rows); field(0x68, flags)
        field(0x7c, rng.getrandbits(32)); field(0x80, rng.getrandbits(32))
        field(0x8c, OLD if trial % 11 else 0); field(0x98, 200); field(0x9c, POOL)
        for i in range(64):
            struct.pack_into('<I', memory, OLD - ARENA + i * 48 + 24, 2 if (i + trial) % 3 else 1)
        totals = (0, 1, 2, 3, 7, 0x7fffffff, 0x80000000, 0x80000001, 0xffffffff)
        total = totals[trial % len(totals)]
        # Centering zero over a one-cell axis divides by zero in the original;
        # leave that invalid input outside the completed-execution matrix.
        setter = bytearray(memory)
        if total == 0 and flags & 8:
            struct.pack_into('<I', setter, SELF - ARENA + 0x60, 2)
            struct.pack_into('<I', setter, SELF - ARENA + 0x64, 2)
        compare('SetLogicalColumns', setter, [total]); compare('SetLogicalRows', setter, [total])
        compare('ResetCells', memory, [0 if trial % 19 == 0 else NEW, 0 if trial % 19 == 0 else trial % 65])
        compare('ResizeCells', memory, [columns, rows], mode)
        if trial % 64 == 0:
            zero = bytearray(memory)
            struct.pack_into('<I', zero, SELF - ARENA + 0x68, flags & ~8)
            compare('ResizeCells', zero, [0, 0], mode & ~6, True)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'controlled_interfaces': ['allocation 0x575620', 'deallocation 0x575630'],
              'actual_callees': ['reset all cell words', 'logical column setter', 'logical row setter'],
              'limitations': ['finite mapped padded buffers; arbitrary aliases, allocation failure with nonzero count and overflow allocation sizes unverified',
                              'observed caller ignores setter EAX; void return contracts used', 'production grid reader/owner binding pending', 'MSVC 7.1 and byte matching pending'],
              'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'clamp-selection-field': [('value74 = selected;', 'value7C = selected; value74 = selected;')],
            'wrong-column-center': [('count60 >> 1', 'count60 >> 2')],
            'wrong-row-center': [('count64 >> 1', 'count64 >> 2')],
            'normalize-negative-center': [('value74 = total - (unsigned int)(((int)(0u - (unsigned int)origin)) % (int)total);', 'value74 = (total - (unsigned int)(((int)(0u - (unsigned int)origin)) % (int)total)) % total;')],
            'wrong-reset-word': [('cells->value0[0] = 0xffffffff;', 'cells->value0[0] = 0;')],
            'omit-reset-tail': [('cells->value0[11] = 0x3f800000;', 'cells->value0[11] = 0;')],
            'omit-partial-construction': [('cell->value0[9] = 0xffffffff;', 'cell->value0[9] = 0;')],
            'copy-retained-width': [('(count60 * 0x30) >> 2', '(retainedColumns * 0x30) >> 2')],
            'wrong-retired-type': [('cell->value0[6] == 2', 'cell->value0[6] == 1')],
            'wrong-payload-word': [('unsigned int payload = cell->value0[8];', 'unsigned int payload = cell->value0[9];')],
            'pool-push-instead-of-pop': [('--value98;', '++value98;')],
            'skip-old-free': [('FEngGridFreeCells(previousCells);', '(void)previousCells;')],
            'skip-ownership-gate': [('if (flags68 & 1)', 'if (true)')],
            'set-owned-too-early': [('    if (flags68 & 1) {', '    flags68 |= 1;\n    if (flags68 & 1) {')],
            'snapshot-reset-count': [('FEngGridResetCells(allocated, count60 * count64);', 'FEngGridResetCells(allocated, allocationCount);')],
            'snapshot-new-dimensions': [('SetLogicalColumns(count60);', 'SetLogicalColumns(columns);'), ('SetLogicalRows(count64);', 'SetLogicalRows(rows);')],
        }
        for name, edits in mutations.items():
            changed = source
            for before, after in edits:
                assert before in changed, name
                changed = changed.replace(before, after)
            path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(changed)
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler, '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
