#!/usr/bin/env python3
"""Compare UG2 string-pool migration and inline rectangular cell setters."""
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

BUILD = ROOT / 'build/analysis/feng-grid-cells'
ARENA = 0x2000000
SELF, CELLS, NEW_DATA, NEW_TABLE, OLD_DATA, OLD_TABLE, EXTRA, PAIR, STACK, STOP = (ARENA + x for x in (0x100, 0x3000, 0x8000, 0xa000, 0xb000, 0xd000, 0xe000, 0xf000, 0x1f000, 0x100000))
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ResizeStringPool': '?ResizeStringPool@FEGridObject@', 'SetCellValue0': '?SetCellValue0@FEGridObject@',
            'SetCellPair4': '?SetCellPair4@FEGridObject@', 'SetCellValue1C': '?SetCellValue1C@FEGridObject@'}
ARGS = {'ResizeStringPool': 2, 'SetCellValue0': 5, 'SetCellPair4': 5, 'SetCellValue1C': 5}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngGridCells.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid-cells.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin-memset'])
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
    markers = {0x518cea: 'pool-zero-words', 0x518cf1: 'pool-zero-tail', 0x518d10: 'pool-slot-table',
               0x518d80: 'type2-migration', 0x518db9: 'utf16-copy', 0x518dcb: 'utf16-termination',
               0x518df7: 'old-data-free', 0x518e10: 'old-table-free'}
    coverage = dict.fromkeys(markers.values(), 0); state = {}; machines = []
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, 4 * count))
    def put(uc, offset, value): uc.mem_write(SELF + offset, struct.pack('<I', value & 0xffffffff))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, uc.reg_read(xc.UC_X86_REG_ECX),
                                       words(uc, sp + 4, ARGS[name]), bytes(uc.mem_read(ARENA, 0x18000))))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, bytes(uc.mem_read(ARENA, 0x18000))))
            if address == 0x575620:
                mode = state['mode']; number = state['allocations']; state['allocations'] += 1
                assert number < 2
                if number == 0:
                    if mode & 1: put(uc, 0xa0, EXTRA)
                    if mode & 16: put(uc, 0x98, 1)
                else:
                    if mode & 2: put(uc, 0xa0, EXTRA)
                    if mode & 4: put(uc, 0x60, 2)
                    if mode & 8: put(uc, 0x8c, CELLS + 0x300)
                    if mode & 32: put(uc, 0x94, 99)
                result = NEW_DATA if number == 0 else NEW_TABLE
            else:
                assert argument in (OLD_DATA, OLD_TABLE), hex(argument)
                if state['mode'] & 64:
                    put(uc, 0x9c, EXTRA + 0x1000); put(uc, 0xa0, EXTRA)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(entries, 0)
    def compare(name, memory, arguments, mode=0):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *[a & 0xffffffff for a in arguments]))
            uc.reg_write(xc.UC_X86_REG_ECX, SELF); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, allocations=0)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                if not rebuilt: raise
                raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(arguments)
            assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0x18000)), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, arguments, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:20],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
        counts[name] += 1
    rng = random.Random(0x518c60)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x20000))
        def field(offset, value): struct.pack_into('<I', memory, SELF - ARENA + offset, value & 0xffffffff)
        width, height = 1 + trial % 4, 1 + (trial // 4) % 4
        field(0x60, width); field(0x64, height); field(0x68, rng.getrandbits(32))
        field(0x6c, (0, 1, width, width + 3)[(trial // 16) % 4])
        field(0x70, (0, 1, height, height + 2)[(trial // 64) % 4])
        origins = (0, 1, 7, -1, -9, 0x80000000, 0x7fffffff)
        field(0x74, origins[(trial // 256) % len(origins)])
        field(0x78, origins[(trial // 512) % len(origins)])
        field(0x8c, CELLS); field(0x90, 64); field(0x94, 32); field(0x98, 40)
        field(0x9c, OLD_TABLE if trial % 4 in (0, 1) else 0)
        field(0xa0, OLD_DATA if trial % 4 in (0, 2) else 0)
        for i in range(64):
            struct.pack_into('<I', memory, CELLS - ARENA + i * 48 + 24, (i + trial) % 3)
            pointer = 0 if (i + trial) % 5 == 0 else OLD_DATA + i * 64
            struct.pack_into('<I', memory, CELLS - ARENA + i * 48 + 32, pointer)
            text = [rng.randrange(1, 65536) for _ in range((i + trial) % 31)] + [0]
            struct.pack_into('<' + 'H' * len(text), memory, OLD_DATA - ARENA + i * 64, *text)
            struct.pack_into('<I', memory, OLD_TABLE - ARENA + i * 4, OLD_DATA + i * 64)
        coordinates = (0, 1, 4, 9, -1, -7, 0x80000000, 0x7fffffff, 0xfffffffe)
        x, y = coordinates[trial % len(coordinates)], coordinates[(trial // len(coordinates)) % len(coordinates)]
        columns, rows = (0, 1, 2, 4)[(trial // 7) % 4], (0, 1, 2, 3)[(trial // 29) % 4]
        value = rng.getrandbits(32)
        compare('SetCellValue0', memory, [x, y, value, columns, rows])
        compare('SetCellPair4', memory, [x, y, PAIR if columns and rows else 0, columns, rows])
        compare('SetCellValue1C', memory, [x, y, value, columns, rows])
        if trial % 32 == 0:
            alias = bytearray(memory)
            for offset, value_ in ((0x6c, width), (0x70, height), (0x74, 0), (0x78, 0)):
                struct.pack_into('<I', alias, SELF - ARENA + offset, value_)
            compare('SetCellPair4', alias, [0, 0, CELLS, 1, 1])
        count = 17 + trial % 16 if trial % 17 else 0
        capacity = 1 + (trial // 17) % 9 if trial % 19 else 0
        compare('ResizeStringPool', memory, [count, capacity], (trial // 32) % 128)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'controlled_interfaces': ['allocation 0x575620', 'deallocation 0x575630'],
              'limitations': ['finite padded buffers and bounded pool capacities; allocation failure/overflow and arbitrary aliases unverified',
                              'void return contracts; no per-instruction memory access ordering or byte matching claim',
                              'production grid reader/owner binding pending', 'Clang disables invented memset libcall; MSVC 7.1 pending'],
              'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'pool-no-clear-count': [('    value98 = 0;', '    value98 = 1;')],
            'pool-wrong-character-size': [('count * capacity * 2', 'count * capacity')],
            'pool-wrong-slot-stride': [('offset += capacity * 2;', 'offset += capacity;')],
            'pool-omit-zero-tail': [('while (remainder) { *tail++ = 0; --remainder; }', '(void)remainder;')],
            'pool-wrong-migration-type': [('cell->value0[6] == 2', 'cell->value0[6] == 1')],
            'pool-wrong-used-count': [('++value98;', '--value98;')],
            'pool-wrong-copy-bound': [('capacity - 1 == copied', 'capacity == copied')],
            'pool-no-payload-rebind': [('cell->value0[8] = (unsigned int)output;', '(void)cell;')],
            'pool-skip-data-free': [('if (previousData) FEngGridFreeCells((void *)previousData);', '(void)previousData;')],
            'pool-skip-table-free': [('if (previousTable) FEngGridFreeCells(previousTable);', '(void)previousTable;')],
            'cell-wrong-value-word': [('cell->value0[0] = value;', 'cell->value0[3] = value;')],
            'cell-wrong-1c-word': [('cell->value0[7] = value;', 'cell->value0[8] = value;')],
            'cell-unsigned-coordinate': [('if (coordinate >= logical)', 'if ((unsigned int)coordinate >= (unsigned int)logical)')],
            'cell-ignore-origin': [('(unsigned int)coordinate - value74', '(unsigned int)coordinate')],
            'cell-wrong-row-origin': [('(unsigned int)coordinate - value78', '(unsigned int)coordinate - value74')],
            'cell-no-logical-negative-adjust': [('if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);', 'if (false) difference = (int)((unsigned int)difference + (unsigned int)logical);')],
            'cell-wrong-row-stride': [('(unsigned int)physicalColumns * (unsigned int)projectedRow', '(unsigned int)physicalRows * (unsigned int)projectedRow')],
            'pair-preload-second-word': [('cell->value0[1] = pair[0];\n            cell->value0[2] = pair[1];', 'unsigned int second = pair[1]; cell->value0[1] = pair[0];\n            cell->value0[2] = second;')],
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
