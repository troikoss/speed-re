#!/usr/bin/env python3
"""Compare the full UG2 grid property reader with its actual storage/pool/property callees."""
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

BUILD = ROOT / 'build/analysis/feng-grid-reader'
ARENA = 0x2000000
SELF, CELLS, NEW_DATA, NEW_TABLE, OLD_DATA, OLD_TABLE, EXTRA, PAIR, STACK, STOP = (ARENA + x for x in (0x100, 0x3000, 0x8000, 0xa000, 0xb000, 0xd000, 0xe000, 0xf000, 0x1f000, 0x100000))
READER, PROPERTY, NEW_CELLS = ARENA + 0x400, ARENA + 0x500, ARENA + 0x6000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ReadProperty': '?ReadProperty@FEngGridPropertyReader@', 'SetLogicalColumns': '?SetLogicalColumns@FEGridObject@', 'SetLogicalRows': '?SetLogicalRows@FEGridObject@', 'ResetCells': '?FEngGridResetCells@', 'ResizeCells': '?ResizeCells@FEGridObject@', 'ResizeStringPool': '?ResizeStringPool@FEGridObject@', 'SetCellValue0': '?SetCellValue0@FEGridObject@',
            'SetCellPair4': '?SetCellPair4@FEGridObject@', 'SetCellValue1C': '?SetCellValue1C@FEGridObject@'}
ARGS = {'ReadProperty': 1, 'SetLogicalColumns': 1, 'SetLogicalRows': 1, 'ResetCells': 2, 'ResizeCells': 2, 'ResizeStringPool': 2, 'SetCellValue0': 5, 'SetCellPair4': 5, 'SetCellValue1C': 5}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngGridReader.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid-reader.json').read_text())
    for dependency in ('feng-grid-cells.json', 'feng-grid-storage.json'):
        manifest['functions'].update(json.loads((ROOT / 'config/SPEED2_EXE_1_2' / dependency).read_text())['functions'])
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngGridCells.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngGridStorage.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin-memset'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for sym in read_object(obj)['symbols'].values():
            if sym['section'] != 0: continue
            if sym['name'].startswith('?FEngGridAllocateCells@'): externals[sym['name']] = 0x575620
            elif sym['name'].startswith('?FEngGridFreeCells@'): externals[sym['name']] = 0x575630
            elif sym['name'] == '__fltused': externals[sym['name']] = 0
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    markers = {0x5350a3: 'Cc', 0x5350b6: 'SI', 0x5350c9: 'LD', 0x5350e4: 'Lj',
               0x535158: 'Cs', 0x535125: 'LT', 0x535107: 'Lv'}
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
                arguments = list(words(uc, sp + 4, ARGS[name]))
                if name == 'ReadProperty':
                    state['tag'] = struct.unpack('<H', uc.mem_read(arguments[0], 2))[0]
                if name == 'SetCellPair4':
                    arguments[2] = ('local-pair', words(uc, arguments[2], 2))
                state['trace'].append((name, None if name == 'ResetCells' else uc.reg_read(xc.UC_X86_REG_ECX),
                                       arguments, bytes(uc.mem_read(ARENA, 0x18000))))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, bytes(uc.mem_read(ARENA, 0x18000))))
            if address == 0x575620:
                mode = state['mode']; number = state['allocations']; state['allocations'] += 1
                assert number < 2
                if number == 0:
                    if mode & 1: put(uc, 0xa0, EXTRA)
                    if mode & 16: put(uc, 0x98, 48 if state['tag'] == 0x444c else 1)
                else:
                    if mode & 2: put(uc, 0xa0, EXTRA)
                    if mode & 4: put(uc, 0x60, 2)
                    if mode & 8: put(uc, 0x8c, CELLS + 0x300)
                    if mode & 32: put(uc, 0x94, 99)
                result = NEW_CELLS if state['tag'] == 0x444c else (NEW_DATA if number == 0 else NEW_TABLE)
            else:
                assert argument in (OLD_DATA, OLD_TABLE, CELLS), hex(argument)
                if state['mode'] & 64:
                    put(uc, 0x9c, EXTRA + 0x1000); put(uc, 0xa0, EXTRA)
                result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = {'ReadProperty': 0}
    def compare(name, memory, arguments, mode=0):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *[a & 0xffffffff for a in arguments]))
            uc.reg_write(xc.UC_X86_REG_ECX, READER); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            control = (0x7f, 0x27f, 0x37f, 0x77f, 0xb7f, 0xf7f)[(trial // 16) % 6]
            uc.reg_write(xc.UC_X86_REG_FPCW, control); uc.reg_write(xc.UC_X86_REG_FPSW, 0)
            uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, allocations=0, tag=0)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                if not rebuilt: raise AssertionError(('original fixture fault', trial, hex(state['tag']), str(error))) from error
                raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(arguments)
            assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
            assert uc.reg_read(xc.UC_X86_REG_FPCW) == control
            assert uc.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
            assert not (uc.reg_read(xc.UC_X86_REG_FPSW) & 0x3800)
            outputs.append((bytes(uc.mem_read(ARENA, 0x18000)), state['trace'], uc.reg_read(xc.UC_X86_REG_FPSW)))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, arguments, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:20],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
        counts[name] += 1
    rng = random.Random(0x535060)
    tags = (0x444c, 0x4953, 0x6343, 0x7343, 0x6a4c, 0x744c, 0x764c, 0, 0x6a4d, 0xffff)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x20000))
        def field(offset, value): struct.pack_into('<I', memory, SELF - ARENA + offset, value & 0xffffffff)
        width, height = 1 + trial % 4, 1 + (trial // 4) % 4
        field(0x60, width); field(0x64, height); field(0x68, rng.getrandbits(32) & ~8)
        field(0x6c, width + 2); field(0x70, height + 1); field(0x74, trial % 7); field(0x78, trial % 5)
        field(0x7c, 0); field(0x80, 0); field(0x8c, CELLS)
        field(0x90, 64); field(0x94, 32); field(0x98, 40)
        field(0x9c, OLD_TABLE if trial % 4 in (0, 1) else 0)
        field(0xa0, OLD_DATA if trial % 4 in (0, 2) else 0)
        struct.pack_into('<I', memory, READER - ARENA + 0xc, SELF)
        for i in range(64):
            struct.pack_into('<I', memory, CELLS - ARENA + i * 48 + 24, (i + trial) % 3)
            pointer = 0 if (i + trial) % 5 == 0 else OLD_DATA + i * 64
            struct.pack_into('<I', memory, CELLS - ARENA + i * 48 + 32, pointer)
            text = [rng.randrange(1, 65536) for _ in range((i + trial) % 31)] + [0]
            struct.pack_into('<' + 'H' * len(text), memory, OLD_DATA - ARENA + i * 64, *text)
            struct.pack_into('<I', memory, OLD_TABLE - ARENA + i * 4, OLD_DATA + i * 64)
        tag = tags[trial % len(tags)]; one, two = rng.getrandbits(32), rng.getrandbits(32)
        if tag == 0x444c:
            one, two = 1 + (trial // 10) % 4, 1 + (trial // 40) % 4
            field(0x9c, OLD_TABLE) # Shrinking owned type-2 cells requires an existing pool table.
        elif tag == 0x4953: one, two = 17 + trial % 16, 1 + (trial // 17) % 9
        elif tag == 0x764c:
            values = (0, 0x80000000, 1, 0x80000001, 0x7f800000, 0xff800000, 0x7fc12345, 0x7f812345, 0xff812345)
            one, two = values[(trial // 10) % len(values)], values[(trial // 90) % len(values)]
        struct.pack_into('<HHII', memory, PROPERTY - ARENA, tag, rng.randrange(65536), one, two)
        compare('ReadProperty', memory, [PROPERTY], (trial // 16) % 128)
        if trial % 64 == 0:
            short = bytearray(memory)
            struct.pack_into('<I', short, READER - ARENA + 0xc, 0xdeadbeef)
            struct.pack_into('<H', short, 0x1fffe, 0xffff)
            compare('ReadProperty', short, [ARENA + 0x1fffe])
            # LT's first store must precede the potentially aliased payload read.
            alias = bytearray(memory)
            address = SELF + 0x64
            struct.pack_into('<HH', alias, address - ARENA, 0x744c, 4)
            compare('ReadProperty', alias, [address])
            # Lv preloads its second float before the first output store.
            alias = bytearray(memory); address = SELF + 0x7c
            struct.pack_into('<HHII', alias, address - ARENA, 0x764c, 8, 0x3f800000, 0x40000000)
            compare('ReadProperty', alias, [address])
            # Cs copies payload into a local pair before any cell writes.
            alias = bytearray(memory); address = CELLS - 4
            for offset, value in ((0x6c, width), (0x70, height), (0x74, 0), (0x78, 0)): struct.pack_into('<I', alias, SELF - ARENA + offset, value)
            struct.pack_into('<HHII', alias, address - ARENA, 0x7343, 8, 0x3f800000, 0x40000000)
            compare('ReadProperty', alias, [address])
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'controlled_interfaces': ['allocation 0x575620', 'deallocation 0x575630'],
              'limitations': ['finite reader properties with complete callees, selected aliases and padded cell/pool buffers; invalid allocations/arbitrary aliases unverified',
                              'void return contract; selected x87 control/status cases, not full exception/unwind or byte matching',
                              'production grid reader/owner binding pending', 'Clang disables invented memset libcall; MSVC 7.1 pending'],
              'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'wrong-LD-argument-order': [('ResizeCells(payload[0], payload[1])', 'ResizeCells(payload[1], payload[0])')],
            'wrong-SI-argument-order': [('ResizeStringPool(payload[0], payload[1])', 'ResizeStringPool(payload[1], payload[0])')],
            'wrong-Cc-default': [('SetCellValue0(0, 0, payload[0]', 'SetCellValue0(0, 0, payload[0] ^ 1')],
            'wrong-Lj-callee': [('SetCellValue1C(0, 0, payload[0]', 'SetCellValue0(0, 0, payload[0]')],
            'wrong-property-extent': [('grid->count60, grid->count64', 'grid->count64, grid->count60')],
            'LT-replaces-owned-bit': [('(payload[0] & 0xfffffffe) | owned', 'payload[0]')],
            'LT-skips-first-store': [('grid->flags68 = owned;', '(void)owned;')],
            'Cs-direct-payload-pointer': [('grid->SetCellPair4(0, 0, pair,', 'grid->SetCellPair4(0, 0, payload,')],
            'Cs-swapped-pair': [('pair[0] = payload[0]; pair[1] = payload[1];', 'pair[0] = payload[1]; pair[1] = payload[0];')],
            'Lv-wrong-second': [('grid->value88 = second;', 'grid->value88 = grid->value84;')],
            'Lv-early-first-store': [('float second = ((const float *)payload)[1];\n            grid->value84 = ((const float *)payload)[0];', 'grid->value84 = ((const float *)payload)[0];\n            float second = ((const float *)payload)[1];')],
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
