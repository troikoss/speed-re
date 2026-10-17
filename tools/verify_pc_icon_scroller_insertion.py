#!/usr/bin/env python3
"""Compare original/rebuilt icon construction, insertion and texture dispatch."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_FPCW,
                              UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/icon-scroller-insertion'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OWNER, SCROLLER, ICON, IMAGE, TAIL, MANAGER = [ARENA + x for x in (0x100, 0x14c, 0x400, 0x600, 0x900, 0xb00)]
NAME, PREFIX, VTABLE, DELETE = [ARENA + x for x in (0xc00, 0xc80, 0xd00, 0xe00)]
GLOBAL = 0x8384c4
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
INTERFACES = {
    'FngFormatBuffer': 0x50c960, 'FngFindNamedImage': 0x4953e0,
    'FEngGetSize': 0x51d730, 'FEngGetColour': 0x495570, 'SetFocusByHash': 0x52ce40,
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/IconScrollerInsertion.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/icon-scroller-insertion.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    functions = manifest['functions']
    for f in functions.values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'Insertion.obj'
    if args.compiler == 'msvc':
        command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc']
    else:
        command = ['clang-cl', '--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/Ob0', '/GS-', '/GR-', '/w', '/arch:IA32']
    subprocess.run(command + ['/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend',
                              os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    external = {
        '?FngFormatBuffer@@YAHPADIPBDZZ': INTERFACES['FngFormatBuffer'],
        '?FngFindNamedImage@@YAPAUHudVisibilityObject@@PBD0@Z': INTERFACES['FngFindNamedImage'],
        '?FEngGetSize@@YAXPAUHudVisibilityObject@@PAM1@Z': INTERFACES['FEngGetSize'],
        '?FEngGetColour@@YAIPAUHudVisibilityObject@@@Z': INTERFACES['FEngGetColour'],
        '?SetFocusByHash@FEngManager@@QAEXPBDI@Z': INTERFACES['SetFocusByHash'],
        '?fengManager8384C4@@3PAUFEngManager@@A': GLOBAL,
        '__purecall': STOP + 0x100, '__fltused': STOP + 0x200,
        '??1IconScrollerIcon@@UAE@XZ': 0x527770, '??3@YAXPAX@Z': 0x575630,
    }
    linked = link_objects([obj], external)
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix))
               for name, prefix in [('Constructor', '??0IconScrollerIcon@'), ('InsertIcon', '?InsertIcon@'), ('AddIcon', '?AddIcon@')]}
    rebuilt_vtable = linked['exports']['??_7IconScrollerIcon@@6B@']
    state, machines = {}, []

    def words(uc, address, count):
        return struct.unpack('<' + 'I' * count, uc.mem_read(address, count * 4))

    def string(uc, address):
        return bytes(uc.mem_read(address, 128)).split(b'\0', 1)[0].decode('ascii')

    def snapshot(uc):
        return tuple(bytes(uc.mem_read(a, n)) for a, n in ((SCROLLER, 0xe8), (ICON, 0x48), (IMAGE, 0x80), (TAIL, 0x80)))

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in functions.values()]

        def hook(uc, address, size, user, ranges=ranges):
            if address not in (*INTERFACES.values(), DELETE):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret = words(uc, sp, 1)[0]
            cleanup, result = 0, 0
            if address == INTERFACES['FngFormatBuffer']:
                buffer, limit, fmt, prefix, index = words(uc, sp + 4, 5)
                assert limit == 32 and string(uc, fmt) == '%s%d'
                signed = index if index < 0x80000000 else index - 0x100000000
                text = string(uc, prefix) + str(signed)
                uc.mem_write(buffer, text.encode() + b'\0')
                state['trace'].append(('format', limit, string(uc, fmt), prefix, index, snapshot(uc)))
                result = len(text)
            elif address == INTERFACES['FngFindNamedImage']:
                package, name = words(uc, sp + 4, 2)
                state['trace'].append(('lookup', package, string(uc, name), snapshot(uc)))
                result = IMAGE if state['found'] else 0
            elif address == INTERFACES['FEngGetSize']:
                object_, width, height = words(uc, sp + 4, 3)
                dest = (width, height) if ARENA <= width < ARENA + 0xe000 else 'scratch'
                state['trace'].append(('size', object_, dest, snapshot(uc)))
                size_ = state['sizes'][state['size_index']]
                state['size_index'] += 1
                uc.mem_write(width, struct.pack('<f', size_[0]))
                uc.mem_write(height, struct.pack('<f', size_[1]))
            elif address == INTERFACES['FEngGetColour']:
                state['trace'].append(('colour', words(uc, sp + 4, 1), snapshot(uc)))
                result = state['colour']
            elif address == INTERFACES['SetFocusByHash']:
                state['trace'].append(('focus', uc.reg_read(UC_X86_REG_ECX), words(uc, sp + 4, 2), snapshot(uc)))
                cleanup = 8
            else:
                state['trace'].append(('delete', uc.reg_read(UC_X86_REG_ECX), words(uc, sp + 4, 1), snapshot(uc)))
                cleanup = 4
            uc.reg_write(UC_X86_REG_EAX, result)
            uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
            uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP, sp + 4 + cleanup)
            uc.reg_write(UC_X86_REG_EIP, ret)

        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    rng = random.Random(0x543c10)
    counts = dict.fromkeys(functions, 0)

    def compare(name, memory, arguments, manager, precision, **fixture):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(GLOBAL, struct.pack('<I', manager))
            uc.mem_write(VTABLE + 8, struct.pack('<I', entries['InsertIcon'] if rebuilt else 0x543c10))
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_ECX, ICON if name == 'Constructor' else OWNER if name == 'AddIcon' else SCROLLER)
            uc.reg_write(UC_X86_REG_FPCW, precision)
            for i, reg in enumerate(PRESERVED):
                uc.reg_write(reg, 0x12345678 + i)
            state.clear()
            state.update(fixture, trace=[], size_index=0)
            uc.emu_start(entries[name] if rebuilt else int(functions[name]['address'], 16), STOP, count=2000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4 + len(arguments) * 4
            assert all(uc.reg_read(r) == 0x12345678 + i for i, r in enumerate(PRESERVED))
            if name == 'Constructor':
                assert words(uc, ICON, 1)[0] == (rebuilt_vtable if rebuilt else 0x79b4d8)
                uc.mem_write(ICON, struct.pack('<I', 0x79b4d8))
            # The harness-installed virtual insertion target differs between machines.
            uc.mem_write(VTABLE + 8, bytes(4))
            outputs.append((bytes(uc.mem_read(ARENA, 0xe000)), state['trace'],
                            uc.reg_read(UC_X86_REG_EAX) if name != 'AddIcon' else None))
        if outputs[0] != outputs[1]:
            differences = [hex(ARENA + i) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:20]
            traces = [[(row[0], row[1:-1]) for row in output[1]] for output in outputs]
            raise AssertionError(('state or call order mismatch', name, precision, differences,
                                  [output[2] for output in outputs], traces))
        counts[name] += 1

    for trial in range(1000):
        memory = bytearray(rng.randbytes(0x10000))
        put = lambda address, *values: struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        put(ICON, VTABLE + 0x20)
        put(VTABLE + 0x20, DELETE)
        put(SCROLLER, VTABLE)
        nonempty = bool(trial % 2)
        put(SCROLLER + 4, TAIL + 4 if nonempty else SCROLLER + 4, TAIL + 4 if nonempty else SCROLLER + 4)
        put(TAIL + 4, SCROLLER + 4, SCROLLER + 4)
        put(TAIL + 0x10, 0 if trial % 13 == 0 else IMAGE)
        if trial % 2 == 0:
            put(IMAGE + 0x24, struct.unpack_from('<I', memory, ICON - ARENA + 0xc)[0])
        put(SCROLLER + 0xc, TAIL if nonempty and trial % 3 == 0 else 0)
        put(SCROLLER + 0x1c, NAME, PREFIX)
        put(SCROLLER + 0x28, (0, 1, 9, 0x7fffffff, 0xffffffff)[trial % 5])
        memory[NAME - ARENA:NAME - ARENA + 13] = b'UI_Pause.fng\0'
        memory[PREFIX - ARENA:PREFIX - ARENA + 8] = b'option_\0'
        manager = MANAGER if trial % 4 else 0
        spacing = rng.choice((0.0, 40.0, -40.0, 1.0 / 1024))
        offset = rng.choice((0.0, 100.5, -1000.0, 16777216.0))
        struct.pack_into('<f', memory, SCROLLER - ARENA + 0x24, spacing)
        struct.pack_into('<f', memory, SCROLLER - ARENA + 0xe0, offset)
        fixture = {'found': trial % 5 != 0, 'colour': rng.getrandbits(32),
                   'sizes': [(rng.choice((0.0, 0.125, 20.5, 90.0)), float(i + 1)) for i in range(3)]}
        if trial % 11 == 1:
            # An early binary32 spill of spacing + offset loses the final unit.
            struct.pack_into('<f', memory, SCROLLER - ARENA + 0x24, 16777216.0)
            struct.pack_into('<f', memory, SCROLLER - ARENA + 0xe0, 1.0)
            fixture['sizes'][2] = (-16777216.0, 5.0)
        for precision in (0x27f, 0x37f):
            compare('Constructor', memory, [rng.getrandbits(32) for _ in range(3)], manager, precision, **fixture)
            compare('InsertIcon', memory, [0 if trial % 7 == 0 else ICON], manager, precision, **fixture)
            compare('AddIcon', memory, [ICON], manager, precision, **fixture)
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': functions, 'controlled_interfaces': list(INTERFACES) + ['virtual scalar deletion'],
              'byte_matching': 'not established', 'constructor_vtable': 'compiler table normalized to 0x79b4d8',
              'limitations': ['finite valid-list fixtures', 'formatting, lookup, size, colour, focus and deletion controlled',
                              'null icon excluded for AddIcon; original wrapper dereferences it',
                              'controlled deletion retains storage; post-deletion lifetime is not verified',
                              'legacy compiler pending when using clang-cl']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'clear-untouched-links': ('    value2C = value2C_;', '    next4 = previous8 = 0;\n    value2C = value2C_;'),
            'wrong-number': ('option28 + 1u', 'option28 + 2u'),
            'wrong-offset': ('icon->offset14 = runningE0;', 'icon->offset14 = spacing24;'),
            'keep-original-colour': ('icon->colour1C = 0xffffffff;', '/* mutation: retain sampled colour */'),
            'wrong-dirty-bit': ('object->flags1C |= 0x400000;', 'object->flags1C |= 0x200000;'),
            'early-float-spill': ('    runningE0 = (float)(((double)spacing24 + runningE0) + width);',
                                  '    volatile float sum = spacing24 + runningE0;\n    runningE0 = sum + width;'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert source.count(before) == 1
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
