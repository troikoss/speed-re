#!/usr/bin/env python3
"""Bounded UG2 original/rebuilt scroller input and deferred-message comparison."""
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

BUILD = ROOT / 'build/analysis/icon-scroller-input'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OBJECT, PACKAGE, NAME = ARENA + 0x100, ARENA + 0x400, ARENA + 0x800
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/IconScrollerInput.cpp')
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/icon-scroller-input.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    functions = manifest['functions']
    for entry in functions.values():
        offset = int(entry['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + entry['size']]).hexdigest() == entry['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngDiagnostic.cpp'):
        obj = BUILD / (source.stem + '.obj')
        if args.compiler == 'msvc':
            command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe',
                       '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc']
        else:
            command = ['clang-cl', '--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/Ob0', '/GS-', '/w']
        subprocess.run(command + ['/Fo' + str(obj.relative_to(ROOT)),
                                 '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {'?FngFindPackage@@YAPAUFEPackage@@PBD@Z': 0x52cef0})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith('?' + name + '@'))
               for name in functions}
    state, machines = {}, []

    def snapshot(uc):
        return bytes(uc.mem_read(OBJECT, 0x150)), bytes(uc.mem_read(PACKAGE, 0x98))

    def string(uc, address):
        return bytes(uc.mem_read(address, 128)).split(b'\0', 1)[0].decode('ascii')

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
        diagnostic = entries['FngScrollerDiagnostic'] if rebuilt else 0x50d510

        def hook(uc, address, size, user, ranges=ranges, diagnostic=diagnostic):
            sp = uc.reg_read(UC_X86_REG_ESP)
            if address == 0x52cef0:
                ret, name = struct.unpack('<II', uc.mem_read(sp, 8))
                state['trace'].append(('lookup', name, snapshot(uc)))
                uc.reg_write(UC_X86_REG_EAX, PACKAGE)
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 4)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            if address == diagnostic:
                words = struct.unpack('<6I', uc.mem_read(sp, 24))
                state['trace'].append(('diagnostic', string(uc, words[1]), words[2:], snapshot(uc)))
                # Execute the actual recovered/original no-op, including its register behavior.
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))

        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    rng = random.Random(0x52fbf0)
    counts = dict.fromkeys(functions, 0)

    def compare(sequence, memory):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            state['trace'] = []
            checkpoints = []
            for name, arguments in sequence:
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
                uc.reg_write(UC_X86_REG_ESP, STACK)
                uc.reg_write(UC_X86_REG_ECX, OBJECT)
                for index, reg in enumerate(PRESERVED):
                    uc.reg_write(reg, 0x12345678 + index)
                uc.emu_start(entries[name] if rebuilt else int(functions[name]['address'], 16), STOP, count=1000)
                assert uc.reg_read(UC_X86_REG_EIP) == STOP
                cleanup = 16 if name == 'Value50DA20' else 0
                assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4 + cleanup
                assert all(uc.reg_read(reg) == 0x12345678 + i for i, reg in enumerate(PRESERVED))
                checkpoints.append(bytes(uc.mem_read(ARENA, 0xe000)))
                if name == 'FngScrollerDiagnostic':
                    assert uc.reg_read(UC_X86_REG_EAX) == 0
            outputs.append((checkpoints, list(state['trace'])))
        assert outputs[0] == outputs[1], ('state or call order mismatch', [n for n, _ in sequence])
        for name, _ in sequence:
            counts[name] += 1

    for trial in range(1200):
        memory = bytearray(rng.randbytes(0x10000))
        struct.pack_into('<I', memory, OBJECT - ARENA + 4, NAME)
        memory[NAME - ARENA:NAME - ARENA + 12] = b'UI_Pause.fng'
        memory[NAME - ARENA + 12] = 0
        memory[OBJECT - ARENA + 0x14c] = (0, 1, 2, 0x80, 0xff)[trial % 5]
        arguments = [rng.getrandbits(32) for _ in range(4)]
        for name, values in (('Value50DA20', arguments), ('StopInput', []), ('StartInput', [])):
            compare([(name, values)], memory)
        compare([('StopInput', []), ('Value50DA20', arguments), ('StopInput', []),
                 ('StartInput', []), ('StartInput', [])], memory)
        # Diagnostic is cdecl; the arguments are deliberately retained by the caller.
        compare([('FngScrollerDiagnostic', [NAME, 0, 1, 0xffffffff, 0x80000000])], memory)
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': functions, 'controlled_interfaces': ['package lookup 0x52cef0 (valid package)'],
              'byte_matching': 'not established',
              'limitations': ['finite synthetic fixtures', 'package lookup and full message dispatch remain native',
                              'missing package crashes in original and is excluded']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'wrong-stop-mask': ('package->inputMask24 = 0;', 'package->inputMask28 = 0;'),
            'wrong-saved-mask': ('mask = package->inputMask20;', 'mask = package->inputMask24;'),
            'canonical-stopped-only': ('if (inputStopped14C)', 'if (inputStopped14C == 1)'),
            'wrong-message-argument': ('value148 = value10;', 'value148 = valueC;'),
            'wrong-stopped-byte': ('inputStopped14C = 1;', 'inputStopped14C = 2;'),
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
