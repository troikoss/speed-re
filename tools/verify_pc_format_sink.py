#!/usr/bin/env python3
"""Compare original/rebuilt formatter output and alias-sensitive ring/count ordering."""
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
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EBX,
                              UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/bware-format-sink'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
CONTEXT, SOURCE, BUFFER, COUNT = [ARENA + n for n in (0x100, 0x200, 0x800, 0x1000)]
RING, INDEX = 0x828178, 0x828514
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Libs/bWare/FormattingSink.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/bware-format-sink.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    im = pefile.PE(data=raw).get_memory_mapped_image()
    offset = int(manifest['address'], 16) - 0x400000
    assert hashlib.sha256(im[offset:offset + manifest['size']]).hexdigest() == manifest['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'sink.obj'
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/clang:-fno-builtin', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Libs/bWare', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    external = {}
    for symbol in read_object(obj)['symbols'].values():
        if symbol['name'].startswith('?bFormatRingIndex828514@'): external[symbol['name']] = INDEX
        if symbol['name'].startswith('?bFormatRing828178@'): external[symbol['name']] = RING
    linked = link_objects([obj], external)
    entry = next(a for n, a in linked['exports'].items() if n.startswith('?bFormatWrite@'))
    state, machines = {}, []
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(im) + 4095) & ~4095); uc.mem_write(0x400000, im)
        uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']: uc.mem_write(address, body)
        ranges = linked['ranges'] if rebuilt else [(0x43cb40, 0x43cbc2)]
        def code(uc, address, size, user, ranges=ranges):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
        def write(uc, access, address, size, value, user):
            if ARENA <= address < ARENA + 0x8000 or RING <= address < RING + 128 or address == INDEX:
                state['writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, code); uc.hook_add(UC_HOOK_MEM_WRITE, write)
        machines.append(uc)
    rng = random.Random(0x43cb40)
    counts = dict.fromkeys(('memory', 'ring', 'count-only', 'forward-overlap', 'reverse-overlap',
                           'count-alias-destination', 'count-alias-mode-memory', 'count-alias-mode-ring',
                           'count-alias-index', 'context-in-ring', 'source-alias-count'), 0)
    for trial in range(4096):
        memory, ring = bytearray(rng.randbytes(0x10000)), bytearray(rng.randbytes(128))
        index = (0, 1, 126, 127, trial % 128)[trial % 5]
        length = (0, 1, 2, 127, 128, 129, 255, 300)[trial % 8]
        case = list(counts)[trial % len(counts)]
        context, source, destination, count = CONTEXT, SOURCE, BUFFER, COUNT
        mode = (1, 2, 128, 255)[trial % 4] if case == 'ring' else 0
        def put(address, *values): struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        put(COUNT, rng.getrandbits(32))
        if case == 'count-only': destination = 0
        elif case == 'forward-overlap': destination = SOURCE + 1
        elif case == 'reverse-overlap': destination = SOURCE - 1
        elif case == 'count-alias-destination': destination, count = BUFFER - length, CONTEXT
        elif case == 'count-alias-mode-memory': count = CONTEXT + 4
        elif case == 'count-alias-mode-ring': mode, count = 254, CONTEXT + 4
        elif case == 'count-alias-index': mode, count = 1, INDEX
        elif case == 'context-in-ring':
            context, mode, index = RING + 64, 1, 68
            memory[SOURCE - ARENA] = 0  # first ring byte turns mode off; later bytes go to memory
        elif case == 'source-alias-count': source, count = CONTEXT + 4, CONTEXT + 4
        if context == CONTEXT:
            put(context, destination, 0xffffff00 | mode if case == 'count-alias-mode-ring' else mode, 0x12345678)
        else:
            struct.pack_into('<III', ring, 64, destination, mode, 0x12345678)
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(RING, bytes(ring))
            uc.mem_write(INDEX, struct.pack('<I', index))
            uc.mem_write(STACK, struct.pack('<5I', STOP, context, source, length, count))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            for i, r in enumerate(PRESERVED): uc.reg_write(r, 0x12345000 + i)
            state['writes'] = []
            uc.emu_start(entry if rebuilt else 0x43cb40, STOP, count=30000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4
            assert all(uc.reg_read(r) == 0x12345000 + i for i, r in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0x8000)), bytes(uc.mem_read(RING, 128)),
                            bytes(uc.mem_read(INDEX, 4)), state['writes']))
        assert outputs[0] == outputs[1], ('state or write order mismatch', case, trial, length,
                                        outputs[0][3][:12], outputs[1][3][:12])
        counts[case] += 1
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'fingerprint': manifest,
              'controlled_interfaces': [], 'checks': ['guarded memory', 'ring/index', 'ordered writes', 'stack cleanup', 'preserved registers'],
              'limitations': ['finite bounded nonnegative lengths', 'valid mapped pointers and ring index',
                              'selected aliases only', 'MSVC pending with clang-cl', 'byte matching not established']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'skip-count-with-null-output': ('*count += length;', 'if (context->destination0) *count += length;'),
            'wrong-ring-mask': ('(index + 1) & 0x7f', '(index + 1) & 0x3f'),
            'update-ring-index-each-byte': ('index = (index + 1) & 0x7f;', 'index = (index + 1) & 0x7f; bFormatRingIndex828514 = index;'),
            'skip-mode-reload': ('unsigned char mode = context->value4;', 'unsigned char mode = 1;'),
            'count-after-memory-copy': ('*count += length;', 'unsigned int pending = length;'),
            'advance-pointer-before-store': ('*context->destination0 = value;\n                context->destination0 = context->destination0 + 1;',
                                             'context->destination0 = context->destination0 + 1;\n                *(context->destination0 - 1) = value;'),
        }
        report['rejected_mutations'] = []
        for name, (before, after) in mutations.items():
            assert before in source
            changed = source.replace(before, after)
            if name == 'count-after-memory-copy':
                changed = changed.replace('    } else if (length) {', '        *count += pending;\n    } else if (length) {')
            path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(changed)
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or write order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
