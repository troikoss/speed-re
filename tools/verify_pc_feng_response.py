#!/usr/bin/env python3
"""Compare original/rebuilt response conditional traversal, including record reads."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_EBX, UC_X86_REG_ESI,
                              UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_feng_response import link_response
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-response'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
RESPONSE, COMMANDS = ARENA + 0x100, ARENA + 0x1000
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngResponse.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    f = manifest['functions']['SkipConditionalBlock']
    address, size = int(f['address'], 16), f['size']
    assert hashlib.sha256(image[address-0x400000:address-0x400000+size]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    linked = link_response(args.compiler, args.source,
                           ROOT / 'src/Speed/Indep/Src/Frontend/FEngQueueState.cpp', BUILD)
    entry = next(a for n, a in linked['exports'].items() if n.startswith('?SkipConditionalBlock@FEResponse@'))
    state, machines = {}, []
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for start, body in linked['blocks']:
            uc.mem_write(start, body)
        ranges = linked['ranges'] if rebuilt else [(address, address + size)]
        def code(uc, pc, size, user, ranges=ranges):
            assert any(a <= pc < b for a, b in ranges), ('unexpected execution', hex(pc))
        def read(uc, access, location, size, value, user):
            if COMMANDS <= location < COMMANDS + 0x4000:
                state['reads'].append((location-COMMANDS, size))
        def write(uc, access, location, size, value, user):
            if ARENA <= location < ARENA + 0x8000:
                state['writes'].append((location, size, value))
        uc.hook_add(UC_HOOK_CODE, code)
        uc.hook_add(UC_HOOK_MEM_READ, read)
        uc.hook_add(UC_HOOK_MEM_WRITE, write)
        machines.append(uc)
    rng = random.Random(0x504490)
    # Each list has mapped extra records: the original may read index == count.
    cases = [(0, 0xffffffff, []), (2, 0, [0x300, 0]),
             (0, 0, [0]), (1, 0, [0]), (1, 1, [0, 0, 0]),
             (2, 0, [0x300, 0x500]), (2, 0, [0x300, 0x501]),
             (3, 0, [0x300, 0x300, 0]),
             (8, 0, [0x300, 0x300, 0x500, 0, 0x501, 0x500, 0, 0x501])]
    types = (0, 1, 2, 0x2ff, 0x300, 0x301, 0x302, 0x4ff, 0x500, 0x501, 0x502, 0xffffffff)
    for _ in range(8183):
        count = rng.randrange(0, 65)
        index = rng.randrange(count + 3)
        cases.append((count, index, [rng.choice(types) for _ in range(count + 5)]))
    for trial, (count, index, types_) in enumerate(cases):
        memory = bytearray(rng.randbytes(0x8000))
        struct.pack_into('<II', memory, RESPONSE-ARENA+0x10, count,
                         0x0bad0001 if index == (count-1) & 0xffffffff else COMMANDS)
        for i, type_ in enumerate(types_):
            struct.pack_into('<I', memory, COMMANDS-ARENA+i*12, type_)
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<II', STOP, index))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_ECX, RESPONSE)
            for i, register in enumerate(PRESERVED):
                uc.reg_write(register, 0x12345000+i)
            state.clear()
            state.update(reads=[], writes=[])
            uc.emu_start(entry if rebuilt else address, STOP, count=10000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK+8
            assert all(uc.reg_read(r) == 0x12345000+i for i, r in enumerate(PRESERVED))
            outputs.append((uc.reg_read(UC_X86_REG_EAX), bytes(uc.mem_read(ARENA, 0x8000)),
                            state['reads'], state['writes']))
        assert outputs[0] == outputs[1], ('state or read order mismatch', trial, count, index, outputs[0][0],
                                         outputs[1][0], outputs[0][2:], outputs[1][2:])
    report = {'compiler': args.compiler, 'comparisons': len(cases), 'fingerprints': manifest['functions'],
              'controlled_interfaces': [], 'checks': ['guarded memory', 'ordered command reads', 'no arena writes',
                                                        'stack cleanup', 'preserved registers'],
              'limitations': ['finite mapped record storage, including extra records beyond declared count',
                              'no arbitrary bad pointers or index/depth overflow coverage',
                              'caller response dispatch remains native', 'MSVC and byte matching pending']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'native-old-bound': ('index < count &&', 'index < count - 1 &&'),
            'skip-at-end-read': ('if (index == count - 1) return count;', 'if (index >= count - 1) return count;'),
            'else-closes-nested': ('if (depth == 1) depth = 0;', 'depth = 0;'),
            'ignore-inverted-condition': ('type <= 0x301', 'type < 0x301'),
            'omit-close-decrement': ('--depth;', 'depth = depth;'),
            'wrong-command-stride': ('unsigned int type = command[1].type;', 'unsigned int type = command[0].type;'),
        }
        report['rejected_mutations'] = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or read order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
