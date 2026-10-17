#!/usr/bin/env python3
"""Compare original/rebuilt archive search, including ordered intrusive-link writes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_EBX, UC_X86_REG_ESI,
                              UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-archive'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
ARCHIVE = ARENA + 0x100
NODES = [ARENA + 0x200 + i * 0x100 for i in range(5)]
RECORDS = [ARENA + 0x1000 + i * 0x100 for i in range(5)]
DATA = [ARENA + 0x1800 + i * 0x100 for i in range(5)]
NAME = ARENA + 0x3000
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngArchive.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-archive.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Libs/bWare/Strings.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe']
                    if args.compiler == 'msvc' else ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {'_strlen': STOP + 0x100})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix)) for name, prefix in
               [('FindNode', '?FindNode@FEngArchive@'), ('cihash', '_cihash'), ('strlenWithNullcheck', '_strlenWithNullcheck')]}
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
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n
                   for n in ('cihash', 'strlenWithNullcheck')}
        def code(uc, address, size, user, ranges=ranges, helpers=helpers):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
            if address in helpers:
                argument = struct.unpack('<I', uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4, 4))[0]
                state['calls'].append((helpers[address], argument))
        def write(uc, access, address, size, value, user):
            if ARENA <= address < ARENA + 0x8000:
                state['writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, code)
        uc.hook_add(UC_HOOK_MEM_WRITE, write)
        machines.append(uc)

    rng = random.Random(0x51bb70)
    count = 0
    def hash_name(value):
        result = 0xffffffff
        for c in value:
            result = (result * 33 + (c - 32 if 97 <= c <= 122 else c)) & 0xffffffff
        return result
    texts = (b'UI_A.fng', b'ui_a.FNG', b'path\\UI_A.fng', b'\\UI_A.fng', b'path/UI_A.fng',
             b'', b'path\\', b'\\', b'a\\b\\UI_A.fng', b'B!.fng', b'AB.fng', b'\x1fA!', b'\xffAZ')
    assert hash_name(texts[9]) == hash_name(texts[10])
    assert hash_name(texts[11]) == 0
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x10000))
        def put(address, *values):
            struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        text = texts[trial % len(texts)]
        name = 0 if trial % 19 == 0 else NAME
        memory[NAME - ARENA:NAME - ARENA + len(text) + 1] = text + b'\0'
        length = trial % 6
        links = [ARCHIVE + 4] + [node + 4 for node in NODES[:length]]
        for i, link in enumerate(links):
            put(link, links[(i + 1) % len(links)], links[(i - 1) % len(links)])
        for i, node in enumerate(NODES):
            put(node + 0xc, DATA[i], RECORDS[i])
            kind = (0x30210, 0x30203, 0x12345)[(trial + i) % 3]
            put(RECORDS[i], kind, rng.getrandbits(32), hash_name(texts[(trial + i) % len(texts)]))
            put(DATA[i], 0xe76e4546 if trial % 7 != i else 0, rng.getrandbits(32),
                0x64486b50 if trial % 11 != i else 0, rng.getrandbits(32),
                (0x1ffff, 0x20000, 0x20001, 0xffffffff)[(trial + i) % 4])
            value = texts[(trial + i) % len(texts)]
            memory[DATA[i] + 0x28 - ARENA:DATA[i] + 0x28 - ARENA + len(value) + 1] = value + b'\0'
        # Dedicated duplicates and collision hits exercise first-hit priority and repeated relocation.
        if trial % 8 == 1:
            for i in range(5):
                put(RECORDS[i], 0x30210, 0, hash_name(b'UI_A.fng'))
            text = b'UI_A.fng'
            memory[NAME - ARENA:NAME - ARENA + len(text) + 1] = text + b'\0'
        if trial % 8 == 2:
            put(RECORDS[0], 0x30210, 0, hash_name(b'B!.fng'))
            memory[NAME - ARENA:NAME - ARENA + 8] = b'AB.fng\0\0'
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            state.clear()
            state.update(calls=[], writes=[])
            results = []
            for repeat in range(2):
                uc.mem_write(STACK, struct.pack('<II', STOP, name))
                uc.reg_write(UC_X86_REG_ESP, STACK)
                uc.reg_write(UC_X86_REG_ECX, ARCHIVE)
                for i, reg in enumerate(PRESERVED):
                    uc.reg_write(reg, 0x12345000 + i)
                uc.emu_start(entries['FindNode'] if rebuilt else 0x51bb70, STOP, count=20000)
                assert uc.reg_read(UC_X86_REG_EIP) == STOP
                assert uc.reg_read(UC_X86_REG_ESP) == STACK + 8
                assert all(uc.reg_read(r) == 0x12345000 + i for i, r in enumerate(PRESERVED))
                results.append(uc.reg_read(UC_X86_REG_EAX))
            outputs.append((results, bytes(uc.mem_read(ARENA, 0x8000)), state['calls'], state['writes']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial, outputs[0][0], outputs[1][0],
                                         outputs[0][2:], outputs[1][2:])
        count += 2
    report = {'compiler': args.compiler, 'comparisons': count, 'fingerprints': manifest['functions'],
              'controlled_interfaces': [], 'composition': 'actual cihash and nullable string length',
              'checks': ['guarded memory', 'ordered link writes', 'helper call trace', 'repeat lookup after relocation',
                         'stack cleanup', 'preserved registers'],
              'limitations': ['finite valid circular lists', 'no malformed pointers or cycles beyond the sentinel',
                              'no allocation/loading recovery', 'MSVC pending with clang-cl', 'byte matching not established']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'omit-relocation': ('            root4.next0 = link;', '            root4.next0 = head;'),
            'wrong-magic': ('data[0] == 0xe76e4546', 'data[0] == 0xe76e4547'),
            'wrong-version-boundary': ('data[4] >= 0x20000', 'data[4] > 0x20000'),
            'skip-zero-candidate': ('if (candidate == hash)', 'if (candidate && candidate == hash)'),
            'ignore-basename': ('unsigned int hash = cihash(component);', 'unsigned int hash = cihash(name);'),
            'fold-leading-backslash': ('unsigned int hash = cihash(component);', "if (component && *component == '\\\\') ++component;\n    unsigned int hash = cihash(component);"),
        }
        report['rejected_mutations'] = []
        for name_, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name_ + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name_ + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name_
            report['rejected_mutations'].append(name_)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
