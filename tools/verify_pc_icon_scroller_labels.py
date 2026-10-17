#!/usr/bin/env python3
"""Compare UG2 scroller label updates and named lookup with recovered hashing."""
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
                              UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EBX,
                              UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/icon-scroller-labels'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OWNER, ICON1, ICON2 = [ARENA + x for x in (0x100, 0x400, 0x500)]
LABELS = [ARENA + x for x in (0x600, 0x700, 0x800)]
NAME = ARENA + 0xa00
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/IconScrollerLabels.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/icon-scroller-labels.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    functions = manifest['functions']
    for f in functions.values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Libs/bWare/Strings.cpp'):
        obj = BUILD / (source.stem + '.obj')
        if args.compiler == 'msvc':
            command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc']
        else:
            command = ['clang-cl', '--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/Ob0', '/GS-', '/GR-', '/w']
        subprocess.run(command + ['/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {'?FngFindStringByHash@@YAPAXPBDI@Z': 0x537a10,
                                   '?FngSetLabelString@@YAXPAXI@Z': 0x50c900,
                                   '_strlen': STOP + 0x100})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith('?' + name + '@'))
               for name in ('SelectedIconChanged', 'FngFindNamedString')}
    entries['cihash'] = linked['exports']['_cihash']
    state, machines = {}, []

    def snapshot(uc):
        return bytes(uc.mem_read(ARENA, 0x1000))

    def words(uc, address, count):
        return struct.unpack('<' + 'I' * count, uc.mem_read(address, 4 * count))

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
        hash_entry = entries['cihash'] if rebuilt else 0x505450

        def hook(uc, address, size, user, ranges=ranges, hash_entry=hash_entry):
            sp = uc.reg_read(UC_X86_REG_ESP)
            if address == hash_entry:
                state['trace'].append(('hash', words(uc, sp + 4, 1), snapshot(uc)))
            if address in (0x537a10, 0x50c900):
                ret, first, second = words(uc, sp, 3)
                if address == 0x537a10:
                    state['trace'].append(('lookup', first, second, snapshot(uc)))
                    result = state['found']
                else:
                    state['trace'].append(('label', first, second, snapshot(uc)))
                    if first:
                        uc.mem_write(first + 0x20, struct.pack('<I', second))
                    state['label_calls'] += 1
                    if state['label_calls'] == 1:
                        # Reentrant setter may change both current icon and the next label.
                        uc.mem_write(OWNER + 0x58, struct.pack('<I', state['next_icon']))
                        uc.mem_write(OWNER + 0x138, struct.pack('<I', state['next_label']))
                    result = 0x1234
                uc.reg_write(UC_X86_REG_EAX, result)
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 4)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))

        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    rng = random.Random(0x50da50)
    counts = dict.fromkeys(functions, 0)

    def compare(name, memory, arguments, **fixture):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_ECX, OWNER)
            for i, reg in enumerate(PRESERVED):
                uc.reg_write(reg, 0x12345678 + i)
            state.clear()
            state.update(fixture, trace=[], label_calls=0)
            uc.emu_start(entries[name] if rebuilt else int(functions[name]['address'], 16), STOP, count=10000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4
            assert all(uc.reg_read(r) == 0x12345678 + i for i, r in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0xe000)), state['trace'],
                            uc.reg_read(UC_X86_REG_EAX) if name != 'SelectedIconChanged' else None))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name)
        counts[name] += 1

    names = (b'', b'ICON_TITLE', b'icon_title', b'OPTION_1', b'\x80az\xff', b'a' * 127)
    for trial in range(2000):
        memory = bytearray(rng.randbytes(0x10000))
        selected = rng.choice((0, ICON1, ICON2))
        struct.pack_into('<I', memory, OWNER - ARENA + 0x58, selected)
        struct.pack_into('<II', memory, OWNER - ARENA + 0x134, rng.choice([0] + LABELS), rng.choice([0] + LABELS))
        name = names[trial % len(names)]
        memory[NAME - ARENA:NAME - ARENA + len(name) + 1] = name + b'\0'
        fixture = {'found': rng.choice([0] + LABELS), 'next_icon': rng.choice((0, ICON1, ICON2)),
                   'next_label': rng.choice([0] + LABELS)}
        compare('SelectedIconChanged', memory, [], **fixture)
        name_ptr = 0 if trial % 7 == 0 else NAME
        compare('FngFindNamedString', memory, [0 if trial % 3 == 0 else ARENA + 0xc00, name_ptr], **fixture)
        compare('cihash', memory, [name_ptr], **fixture)
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': functions, 'controlled_interfaces': ['typed string lookup 0x537a10', 'label setter 0x50c900'],
              'byte_matching': 'not established', 'hashing': 'actual recovered cihash executed',
              'limitations': ['finite synthetic fixtures', 'package traversal/localization/allocation not verified',
                              'MSVC validation pending when using clang-cl']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'omit-description': ('    FngSetLabelString(description138, icon ? icon->value30 : 0);', ''),
            'wrong-description-hash': ('icon ? icon->value30 : 0', 'icon ? icon->value2C : 0'),
            'cache-current-icon': ('    icon = reinterpret_cast<IconScroller4C *>(scroller4C)->currentC;', ''),
            'wrong-bound-label': ('FngSetLabelString(title134,', 'FngSetLabelString(description138,'),
            'hash-package': ('cihash(name)', 'cihash(package)'),
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
