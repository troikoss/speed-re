#!/usr/bin/env python3
"""Compare original FEng language-hash setter and composed scroller label updates."""
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

BUILD = ROOT / 'build/analysis/feng-text'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OWNER, ICON1, ICON2 = [ARENA + x for x in (0x100, 0x400, 0x500)]
LABEL1, LABEL2, OBSERVER, VTABLE = [ARENA + x for x in (0x600, 0x700, 0x900, 0xa00)]
GLOBAL, CALLBACK = 0x838478, STOP + 0x800
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngText.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-text.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    functions = manifest['functions']
    for f in functions.values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/IconScrollerLabels.cpp'):
        obj = BUILD / (source.stem + '.obj')
        if args.compiler == 'msvc':
            command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc']
        else:
            command = ['clang-cl', '--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/Ob0', '/GS-', '/GR-', '/w']
        subprocess.run(command + ['/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {'?fengStringObserver838478@@3PAUFEngStringObserver@@A': GLOBAL,
                                   '?FngFindStringByHash@@YAPAXPBDI@Z': STOP + 0x100,
                                   '_cihash': STOP + 0x200})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith('?' + name + '@'))
               for name in functions}
    state, machines = {}, []
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
            if address == CALLBACK:
                sp = uc.reg_read(UC_X86_REG_ESP)
                ret, object_ = struct.unpack('<II', uc.mem_read(sp, 8))
                assert uc.reg_read(UC_X86_REG_ECX) == OBSERVER
                state['trace'].append((object_, bytes(uc.mem_read(ARENA, 0x1000))))
                state['calls'] += 1
                if state['mode'] & 1:
                    uc.mem_write(object_ + 0x1c, struct.pack('<I', state['flags']))
                if state['mode'] & 2:
                    uc.mem_write(object_ + 0x60, struct.pack('<I', state['hash']))
                if state['calls'] == 1:
                    uc.mem_write(OWNER + 0x58, struct.pack('<I', state['next_icon']))
                    uc.mem_write(OWNER + 0x138, struct.pack('<I', state['next_label']))
                    if state['disable']:
                        uc.mem_write(GLOBAL, bytes(4))
                uc.reg_write(UC_X86_REG_EAX, 0xaaaaaaaa)
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 8)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))

        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)
    rng = random.Random(0x50c900)
    counts = dict.fromkeys(functions, 0)

    def compare(name, memory, arguments, observer, **fixture):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(GLOBAL, struct.pack('<I', observer))
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_ECX, OWNER)
            for i, reg in enumerate(PRESERVED):
                uc.reg_write(reg, 0x12345678 + i)
            state.clear()
            state.update(fixture, trace=[], calls=0)
            uc.emu_start(entries[name] if rebuilt else int(functions[name]['address'], 16), STOP, count=2000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4
            assert all(uc.reg_read(r) == 0x12345678 + i for i, r in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0xe000)), bytes(uc.mem_read(GLOBAL, 4)), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, fixture['mode'])
        counts[name] += 1

    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x10000))
        def put(address, *values):
            struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        put(OBSERVER, VTABLE)
        put(VTABLE, CALLBACK)
        put(OWNER + 0x58, rng.choice((0, ICON1, ICON2)))
        put(OWNER + 0x134, rng.choice((0, LABEL1, LABEL2)), rng.choice((0, LABEL1, LABEL2)))
        hash_ = rng.getrandbits(32)
        put(LABEL1 + 0x60, hash_ if trial % 2 == 0 else hash_ ^ 0xffffffff)
        observer = 0 if trial % 3 == 0 else OBSERVER
        fixture = {'mode': trial % 4, 'flags': rng.getrandbits(32), 'hash': rng.getrandbits(32),
                   'next_icon': rng.choice((0, ICON1, ICON2)), 'next_label': rng.choice((0, LABEL1, LABEL2)),
                   'disable': trial % 5 == 0}
        compare('FngSetLabelString', memory, [0 if trial % 11 == 0 else LABEL1, hash_], observer, **fixture)
        compare('SelectedIconChanged', memory, [], observer, **fixture)
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': functions, 'controlled_interfaces': ['observer first virtual slot at *0x838478'],
              'composition': 'actual recovered setter executed by recovered label updater',
              'byte_matching': 'not established', 'limitations': ['finite synthetic callbacks; no recursive setter calls',
                              'localization and UTF-16 allocation remain native', 'MSVC pending with clang-cl']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'skip-unchanged-hash': ('        unsigned int flags = object->flags1C;',
                                    '        if (object->languageHash60 == hash) return;\n        unsigned int flags = object->flags1C;'),
            'wrong-hash-field': ('object->languageHash60 = hash;', 'object->texture24 = hash;'),
            'omit-change-dirty': ('flags | 0x400000', 'flags'),
            'keep-flag-two': ('object->flags1C & ~2u', 'object->flags1C'),
            'use-callback-flag': ('if (flags & 2)', 'if (object->flags1C & 2)'),
            'omit-observer': ('if (fengStringObserver838478)', 'if (0)'),
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
