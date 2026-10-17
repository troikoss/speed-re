#!/usr/bin/env python3
"""Differential execution of FEng script selection, track activation and real arithmetic helpers."""
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
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_ESP, UC_X86_REG_EIP,
                              UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
                              UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-script'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OBJECT, SCRIPT, TRACKS, STATE, KEY, REF, NAME = [ARENA + x for x in (0x100, 0x300, 0x1000, 0x3000, 0x5000, 0x6000, 0x7000)]
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
PREFIXES = {'Resolve': '?Resolve@FEngReference@', 'Multiply': '?Multiply@FEngQuaternion@',
            'Subtract': '?Subtract@FEngInteger4@', 'InitializeScript': '?InitializeScript@HudVisibilityObject@',
            'SetScript': '?SetScript@HudVisibilityObject@', 'FngSetObjectScript': '?FngSetObjectScript@', 'cihash': '_cihash'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngScript.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-script.json').read_text())
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
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {'_strlen': STOP + 0x100})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix)) for name, prefix in PREFIXES.items()}
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
        helper = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in PREFIXES}
        def code(uc, address, size, user, ranges=ranges, helper=helper):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
            if address in helper:
                name = helper[address]
                this = uc.reg_read(UC_X86_REG_ECX)
                sp = uc.reg_read(UC_X86_REG_ESP)
                if name in ('Multiply', 'Subtract'):
                    other = struct.unpack('<I', uc.mem_read(sp + 8, 4))[0]
                    state['trace'].append((name, bytes(uc.mem_read(this, 16)), bytes(uc.mem_read(other, 16))))
                elif name == 'InitializeScript':
                    state['trace'].append((name, this, bytes(uc.mem_read(this + 0x54, 4))))
                elif name == 'Resolve':
                    state['trace'].append((name, this))
                elif name == 'cihash':
                    state['trace'].append((name, bytes(uc.mem_read(sp + 4, 4))))
        uc.hook_add(UC_HOOK_CODE, code)
        machines.append(uc)
    counts = dict.fromkeys(PREFIXES, 0)
    def compare(name, memory, arguments, this=0, cleanup=0, result=False, precision=0x27f):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(UC_X86_REG_ECX, this)
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_FPCW, precision)
            uc.reg_write(UC_X86_REG_FPSW, 0)
            uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
            for i, r in enumerate(PRESERVED):
                uc.reg_write(r, 0x12345000 + i)
            state['trace'] = []
            uc.emu_start(entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16), STOP, count=40000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4 + cleanup
            assert all(uc.reg_read(r) == 0x12345000 + i for i, r in enumerate(PRESERVED))
            assert uc.reg_read(UC_X86_REG_FPCW) == precision
            assert uc.reg_read(UC_X86_REG_FPSW) & 0x3800 == 0
            outputs.append((uc.reg_read(UC_X86_REG_EAX) if result else None,
                            bytes(uc.mem_read(ARENA, 0x8000)), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, trial, hex(precision),
                                         [(i, a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                                         outputs[0][2], outputs[1][2])
        counts[name] += 1
    rng = random.Random(0x517720)
    for trial in range(1024):
        memory = bytearray(rng.randbytes(0x10000))
        def put(address, *values):
            struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        def floats(address):
            values = [rng.getrandbits(1) << 31 | rng.randrange(115, 137) << 23 | rng.getrandbits(23) for _ in range(4)]
            put(address, *values)
        for address in range(STATE - 0x200, STATE + 0x200, 16): floats(address)
        count = trial % 9
        for i in range(8):
            track, key = TRACKS + i * 0x38, KEY + i * 0x80
            memory[track - ARENA] = (trial + i) % 8
            memory[track + 2 - ARENA] = (trial + i) % 6
            memory[track + 7 - ARENA] = (i * 4 - 12) & 255
            floats(track + 0x18)
            memory[track + 0x2c - ARENA] = 0 if (trial + i) % 3 else 0x80
            put(track + 0x30, 0 if (trial + i) % 11 == 0 else (REF if (trial + i) % 3 == 0 else key))
            if (trial + i) % 3 == 0:
                # References require a valid receiver even if the resolved head is null.
                put(track + 0x30, REF)
        put(REF + 4, 2, REF + 0x20)
        put(REF + 0x24, 0, 0 if trial % 7 == 0 else KEY)
        for i in range(3):
            put(SCRIPT + i * 0x80 + 4, SCRIPT + (i + 1) * 0x80 if i < trial % 4 - 1 else 0)
            put(SCRIPT + i * 0x80 + 0x1c, count, TRACKS)
            put(SCRIPT + i * 0x80 + 0x30, (0xffffffff, 0x4a8b4, 0x4a8b4)[i])
        put(OBJECT + 0x2c, STATE)
        put(OBJECT + 0x4c, SCRIPT if trial % 4 else 0)
        put(OBJECT + 0x54, SCRIPT + (trial % 3) * 0x80)
        memory[NAME - ARENA:NAME - ARENA + 5] = b'go\0\0\0'
        # Use the actual hash for two duplicate script names, including mixed-case lookup.
        put(SCRIPT + 0xb0, 0x535); put(SCRIPT + 0x130, 0x535)  # hash of GO from seed -1
        alias_current = trial % 23 == 6
        if alias_current:
            put(OBJECT + 0x54, SCRIPT + 0x80)
            for n in range(3): put(SCRIPT + n * 0x80 + 0x1c, 1, TRACKS)
            memory[TRACKS - ARENA] = 1
            memory[TRACKS + 2 - ARENA] = 3
            memory[TRACKS + 7 - ARENA] = 0
            memory[TRACKS + 0x2c - ARENA] = 0
            put(TRACKS + 0x18, 0)
            put(STATE, SCRIPT + 0x100)
            put(TRACKS + 0x30, OBJECT + 0x44)  # key+0x10 aliases currentScript54
        integer_memory = bytearray(memory)
        integers = [rng.getrandbits(32) for _ in range(8)]
        if trial % 16 == 0: integers = [0, 0xffffffff, 0x80000000, 0x7fffffff] * 2
        struct.pack_into('<8I', integer_memory, STATE - ARENA, *integers)
        for precision in (0x27f, 0x37f):
            compare('InitializeScript', memory, [], OBJECT, precision=precision)
            compare('SetScript', memory, [0 if trial % 5 == 0 else SCRIPT], OBJECT, 4, precision=precision)
            compare('FngSetObjectScript', memory, [0 if trial % 13 == 0 else OBJECT,
                    0 if alias_current or trial % 3 == 0 else NAME, (0, 1, 2, 128, 255)[trial % 5]], precision=precision)
            compare('Resolve', memory, [], REF, result=True, precision=precision)
            out = (STATE, STATE + 16, STATE + 32)[trial % 3]
            compare('Multiply', memory, [out, STATE + 16], STATE, 8, True, precision)
            compare('Subtract', integer_memory, [out, STATE + 16], STATE, 8, True, precision)
            compare('cihash', memory, [NAME], result=True, precision=precision)
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': manifest['functions'], 'controlled_interfaces': [], 'x87_control_words': ['0x027f', '0x037f'],
              'limitations': ['finite normal floating inputs; non-finite/subnormal behavior excluded', 'valid acyclic script/reference chains',
                              'no allocation/playback recovery', 'MSVC pending with clang-cl', 'byte matching not established']}
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'keep-track-flag': ('track->flags3 &= 0x7f;', 'track->flags3 |= 0x80;'),
            'skip-mode-four': ('track->value2 == 3 || track->value2 == 4', 'track->value2 == 3'),
            'unsigned-state-offset': ('data2C + track->state7', 'data2C + (unsigned char)track->state7'),
            'skip-reference': ('((FEngReference *)track->value30)->Resolve()', 'track->value30'),
            'skip-time-reset': ('object->currentScript54->time10 = 0;', ';'),
            'reset-original-pointer': ('object->currentScript54->time10 = 0;', 'script->time10 = 0;'),
            'restart-same-unconditionally': ('if (restart) script->time10 = 0;', 'script->time10 = 0;'),
            'keep-null-selection': ('currentScript54 = script;\n    if (script)', 'if (script) currentScript54 = script;\n    if (script)'),
            'hash-once': ('for (FEScript *script = object->scripts4C; script; script = script->next4) {\n        if (script->hash30 == cihash(name))',
                          'unsigned int hash = cihash(name);\n    for (FEScript *script = object->scripts4C; script; script = script->next4) {\n        if (script->hash30 == hash)'),
        }
        report['rejected_mutations'] = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and ('state or call order mismatch' in result.stderr or 'Invalid memory' in result.stderr), name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
