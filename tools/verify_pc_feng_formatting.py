#!/usr/bin/env python3
"""Compare recovered GUI formatting wrappers/profiling with the original formatter core controlled."""
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
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-formatting'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
BUFFER, FORMAT, ARGS, FLOAT_RESULT = ARENA + 0x100, ARENA + 0x800, ARENA + 0x1000, ARENA + 0x7f00
SHIFT, SCALE, TOTAL, COUNT = 0x8284e4, 0x828028, 0x828518, 0x82851c
CLOCK, CORE, CAPTURE = STOP + 0x100, 0x43e950, STOP + 0x200
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngFormatting.cpp')
    parser.add_argument('--profile-source', type=Path, default=ROOT / 'src/Speed/Indep/Libs/bWare/Formatting.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-formatting.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    im = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(im[off:off + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, args.profile_source):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Libs/bWare', '/Isrc/Speed/Indep/Src/Frontend',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    globals_ = {'bFormatQueryCounter': CLOCK, 'bFormatCore': CORE, 'bFormatShift8284E4': SHIFT,
                'bFormatScale828028': SCALE, 'bFormatTime828518': TOTAL, 'bFormatCount82851C': COUNT}
    external = {'__allshr': 0x75c600, '__fltused': ARENA + 0x7800}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section'] == 0:
                for name, address in globals_.items():
                    if symbol['name'].startswith('?' + name + '@'): external[symbol['name']] = address
    linked = link_objects(objects, external)
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix)) for name, prefix in
               [('Elapsed', '?bFormatElapsed@'), ('VFormat', '?bVFormat@'), ('FormatHash', '?FngFormatHash@'), ('FormatBuffer', '?FngFormatBuffer@')]}
    state, machines = {}, []
    def words(uc, address, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(address, count * 4))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(im) + 4095) & ~4095)
        uc.mem_write(0x400000, im)
        uc.mem_write(0x78311c, struct.pack('<I', CLOCK))
        uc.mem_map(ARENA, 0x10000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_write(CAPTURE, b'\xdb\x3d' + struct.pack('<I', FLOAT_RESULT))  # fstp m80 after Elapsed return
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']: uc.mem_write(address, body)
        ranges = linked['ranges'] + [(0x75c600, 0x75c621)] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        def hook(uc, address, size, user, ranges=ranges):
            if address == CAPTURE:
                assert state['elapsed']
                return
            sp = uc.reg_read(UC_X86_REG_ESP)
            if address == CLOCK:
                ret, output = words(uc, sp, 2)
                index = state['clock_index']; state['clock_index'] += 1
                value = state['counters'][index]
                state['trace'].append(('clock', index, value))
                uc.mem_write(output, struct.pack('<Q', value))
                if index == 1 and state['clock_mutation']:
                    uc.mem_write(TOTAL, struct.pack('<f', -3.5))
                    uc.mem_write(SHIFT, struct.pack('<I', state['final_shift']))
                result, cleanup = 1, 4
            elif address == CORE:
                ret, context, fmt, arguments = words(uc, sp, 4)
                destination = words(uc, context)[0]
                assert bytes(uc.mem_read(context + 4, 1)) == b'\0' and words(uc, context + 8)[0] == 0, 'formatter context initialization mismatch'
                # Stack-local destination/context addresses differ by compiler; compare their used contents.
                state['trace'].append(('core', fmt, words(uc, arguments, 6), words(uc, SHIFT),
                                       bytes(uc.mem_read(TOTAL, 8))))
                if destination: uc.mem_write(destination, state['text'] + b'\0')
                uc.mem_write(context, struct.pack('<I', destination + len(state['text']) if destination else 0))
                if state['core_mutation']:
                    uc.mem_write(TOTAL, struct.pack('<f', 7.25))
                    uc.mem_write(COUNT, struct.pack('<I', 0xffffffff))
                    uc.mem_write(SCALE, struct.pack('<f', 0.125))
                result, cleanup = state['core_result'], 0
            else:
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            uc.reg_write(UC_X86_REG_EAX, result & 0xffffffff)
            uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
            uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP, sp + 4 + cleanup)
            uc.reg_write(UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)
    rng = random.Random(0x43fbc0)
    counts = dict.fromkeys(entries, 0)
    for trial in range(1024):
        text = rng.randbytes(trial % 256).replace(b'\0', b'a')
        counters = [rng.getrandbits(64), rng.getrandbits(64)]
        if trial % 4 == 0: counters[1] = (counters[0] + trial * 12345) & 0xffffffffffffffff
        shift = (0, 1, 7, 16, 31, 32, 63, 64, 255, 256, 0xffffffff)[trial % 11]
        fixture = dict(text=text, counters=counters, core_result=rng.getrandbits(32), core_mutation=trial % 7 == 0,
                       clock_mutation=trial % 5 == 0, final_shift=(trial * 17) & 255)
        memory = bytearray(rng.randbytes(0x10000))
        memory[FORMAT - ARENA:FORMAT - ARENA + 5] = b'%s%d\0'
        values = [rng.getrandbits(32) for _ in range(6)]
        struct.pack_into('<6I', memory, ARGS - ARENA, *values)
        for precision in (0x27f, 0x37f):
            for name, arguments in [('Elapsed', [rng.getrandbits(32)]), ('VFormat', [BUFFER, FORMAT, ARGS]),
                                    ('FormatHash', [FORMAT] + values), ('FormatBuffer', [BUFFER, trial % 8, FORMAT] + values)]:
                outputs = []
                for rebuilt, uc in zip((False, True), machines):
                    uc.mem_write(ARENA, bytes(memory))
                    uc.mem_write(SHIFT, struct.pack('<I', shift))
                    uc.mem_write(SCALE, struct.pack('<f', 0.001953125 if trial % 2 else 0.00314159))
                    uc.mem_write(TOTAL, struct.pack('<f', 1.125))
                    uc.mem_write(COUNT, struct.pack('<I', trial))
                    uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
                    uc.reg_write(UC_X86_REG_ESP, STACK)
                    uc.reg_write(UC_X86_REG_FPCW, precision)
                    uc.reg_write(UC_X86_REG_FPSW, 0)
                    uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
                    for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                    state.clear(); state.update(fixture, elapsed=name == 'Elapsed', trace=[], clock_index=0)
                    uc.emu_start(entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16),
                                 STOP, count=20000)
                    if name == 'Elapsed': uc.emu_start(CAPTURE, CAPTURE + 6, count=10)
                    assert uc.reg_read(UC_X86_REG_EIP) == (CAPTURE + 6 if name == 'Elapsed' else STOP), (name, trial, rebuilt, hex(uc.reg_read(UC_X86_REG_EIP)), hex(uc.reg_read(UC_X86_REG_ESP)))
                    assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4
                    assert all(uc.reg_read(r) == 0x12345000 + i for i, r in enumerate(PRESERVED))
                    assert uc.reg_read(UC_X86_REG_FPCW) == precision and uc.reg_read(UC_X86_REG_FPSW) & 0x3800 == 0
                    outputs.append((None if name == 'Elapsed' else uc.reg_read(UC_X86_REG_EAX), bytes(uc.mem_read(ARENA, 0x8000)),
                                    bytes(uc.mem_read(TOTAL, 8)), words(uc, SHIFT), bytes(uc.mem_read(SCALE, 4)), state['trace']))
                assert outputs[0] == outputs[1], ('state or call order mismatch', name, trial, hex(precision),
                    [(hex(ARENA+i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                    outputs[0][2:], outputs[1][2:])
                counts[name] += 1
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': manifest['functions'], 'controlled_interfaces': ['QueryPerformanceCounter IAT 0x78311c', 'formatter core 0x43e950'],
              'x87_control_words': ['0x027f', '0x037f'], 'limitations': ['finite well-terminated formatter output <=255 bytes',
                 'clock failure excluded', 'core formatting not recovered', 'MSVC pending with clang-cl', 'byte matching not established']}
    if args.mutation_checks:
        wrappers, profile = args.source.read_text(), args.profile_source.read_text()
        mutations = {
            'wrong-hash-seed': (False, 'unsigned int hash = 0xffffffff;', 'unsigned int hash = 0;'),
            'skip-case-fold': (False, "if ((signed char)c >= 'a' && (signed char)c <= 'z')", 'if (false)'),
            'respect-capacity-zero': (False, 'int FngFormatBuffer(char *buffer, unsigned int capacity, const char *format, ...) {',
                                     'int FngFormatBuffer(char *buffer, unsigned int capacity, const char *format, ...) {\n    if (!capacity) return 0;'),
            'shift-varargs': (False, 'bVFormat(buffer, format, arguments);', 'bVFormat(buffer, format, (const char *)arguments + 4);'),
            'omit-profile-count': (True, '++bFormatCount82851C;', ';'),
            'omit-delta-clamp': (True, 'if ((int)delta < 0) delta = 0;', ';'),
            'unsigned-scale-factor': (True, '(double)(int)(1u << (shift & 31))', '(double)(1u << (shift & 31))'),
            'mask-shift-instead-of-saturating': (True, 'unsigned char bits = (unsigned char)shift;', 'unsigned char bits = (unsigned char)(shift & 63);'),
            'cache-total-before-clock': (True, 'double elapsed = bFormatElapsed(start);\n    bFormatTime828518 = (float)(elapsed + bFormatTime828518);',
                                        'double previous = bFormatTime828518;\n    double elapsed = bFormatElapsed(start);\n    bFormatTime828518 = (float)(elapsed + previous);'),
            'wrong-context-field': (True, 'context.value8 = 0;', 'context.value8 = 1;'),
        }
        report['rejected_mutations'] = []
        for name, (is_profile, before, after) in mutations.items():
            source = profile if is_profile else wrappers
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(args.source if is_profile else path),
                                     '--profile-source', str(path if is_profile else args.profile_source)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and ('state or call order mismatch' in result.stderr or
                                              'formatter context initialization mismatch' in result.stderr), name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
