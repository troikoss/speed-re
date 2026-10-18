#!/usr/bin/env python3
"""Verify original list setup/tick helpers and composition with recovered navigation."""
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
import unicorn.x86_const as xc
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-list-state'
ARENA, OBJECT, XROWS, YROWS, STACK, STOP = 0x2000000, 0x2000100, 0x2001000, 0x2002000, 0x200f000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngListState.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngList.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32',
                     '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-ffp-eval-method=extended', '/clang:-mlong-double-80'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                 '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith('?' + name + '@FEListObject@'))
               for name in manifest['functions']}
    state, machines = {}, []
    markers = {0x504373: 'column-offsets', 0x504397: 'row-offsets', 0x504244: 'advance-column-wrap',
               0x504262: 'advance-row-wrap', 0x50418f: 'column-clamp', 0x5041bf: 'row-clamp',
               0x50427b: 'enable-wrap', 0x504284: 'disable-wrap', 0x517258: 'phase-lower-clamp',
               0x517271: 'phase-upper-clamp', 0x51727b: 'phase-reverse',
               0x51728f: 'motion-step', 0x517302: 'motion-finish-call'}
    coverage = dict.fromkeys(markers.values(), 0)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, data in linked['blocks']: uc.mem_write(address, data)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        finish = entries['FinishMotion'] if rebuilt else 0x5043b0
        def code(uc, address, size, user, ranges=ranges, finish=finish, original=not rebuilt):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
            if original and address in markers: coverage[markers[address]] += 1
            if address == finish: state['calls'].append(('FinishMotion', uc.reg_read(xc.UC_X86_REG_ECX)))
        def write(uc, access, address, size, value, user):
            # These nonvolatile stores can coalesce; every final byte is still checked.
            if ARENA <= address < ARENA + 0x4000 and address not in (OBJECT + 0x5c, OBJECT + 0x88, OBJECT + 0x8c, OBJECT + 0xa4):
                state['writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, code); uc.hook_add(UC_HOOK_MEM_WRITE, write)
        machines.append(uc)
    counts = dict.fromkeys(entries, 0)
    composed = 0
    def compare(memory, sequence, control, trial):
        traces = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); trace = []
            for name, arguments in sequence:
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *[x & 0xffffffff for x in arguments]))
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, OBJECT)
                uc.reg_write(xc.UC_X86_REG_FPCW, control); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
                for i, register in enumerate(PRESERVED): uc.reg_write(register, 0x12345000 + i)
                state.update(writes=[], calls=[])
                address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
                uc.emu_start(address, STOP, count=20000)
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(arguments)
                assert all(uc.reg_read(register) == 0x12345000 + i for i, register in enumerate(PRESERVED))
                trace.append((bytes(uc.mem_read(ARENA, 0x4000)), state['writes'], state['calls']))
            traces.append(trace)
        if traces[0] != traces[1]:
            for j, (a, b) in enumerate(zip(*traces)):
                if a != b:
                    differences = [(hex(i + ARENA), x, y) for i, (x, y) in enumerate(zip(a[0], b[0])) if x != y]
                    raise AssertionError(('state or call order mismatch', trial, sequence[j][0], differences[:25], a[1:], b[1:]))
        for name, _ in sequence: counts[name] += 1
    def bits(value): return struct.unpack('<I', struct.pack('<f', value))[0]
    rng = random.Random(0x504360)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x10000))
        def put(offset, value): struct.pack_into('<I', memory, OBJECT - ARENA + offset, value & 0xffffffff)
        def floating(offset, value): put(offset, bits(value))
        nx, ny = trial % 9, (trial // 9) % 9
        put(0x60, nx); put(0x64, ny); put(0x78, XROWS); put(0x7c, YROWS)
        put(0x5c, rng.getrandbits(32)); put(0x88, trial % 10); put(0x8c, (trial // 10) % 10)
        for rows in (XROWS, YROWS):
            for i in range(8):
                struct.pack_into('<fff', memory, rows - ARENA + i * 12,
                                 (i + 1) * .25 if trial < 2048 else rng.uniform(-1000, 1000), rng.uniform(-100, 100), rng.uniform(-100, 100))
        for offset in (0x68, 0x6c, 0x70, 0x74, 0x80, 0x84, 0x94, 0x98, 0x9c, 0xa0): floating(offset, rng.uniform(-10, 10))
        floating(0xa4, (trial % 33 - 8) * .0625); floating(0xa8, (trial % 19 - 9) * .0005)
        milliseconds = (0, .5, 1, 16, 33, 500, -1, 3000)[trial % 8]
        if trial % 97 == 0:
            floating(0xa4, 1); floating(0xa8, (1e-9, 1e-17)[trial % 2]); milliseconds = 1
        control = (0x27f, 0x37f)[trial % 2]
        for name, arguments in [('CalculateOffsets', []), ('FinishMotion', []), ('Tick', [bits(milliseconds)]),
                                ('SetColumn', [(0, 1, 9, 0xffffffff)[trial % 4]]), ('SetRow', [(0, 1, 9, 0xffffffff)[trial % 4]]),
                                ('AdvanceCell', []), ('SetWrap', [(0, 1, 2, 255, 256, 0xffffffff)[trial % 6]])]:
            compare(memory, [(name, arguments)], control, trial)
        put(0x60, max(nx, 1)); put(0x64, max(ny, 1)); put(0x88, 0); put(0x8c, 0)
        put(0x5c, 4); floating(0x70, 0); floating(0x74, 0); floating(0x94, 0); floating(0x98, 0)
        floating(0x80, .01); floating(0x84, .02); floating(0x68, 0); floating(0x6c, 0)
        sequence = [('CalculateOffsets', []), ('Navigate', [1, 1])] + [('Tick', [bits(16.0)])] * 8
        compare(memory, sequence, control, trial); composed += 1
    assert all(coverage.values()), coverage
    report = {'original_block_hits': coverage, 'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'composed_sequences': composed,
              'controlled_interfaces': [], 'x87_control_words': ['0x27f', '0x37f'],
              'limitations': ['finite acyclic mapped arrays and finite values', 'native parser/lifecycle not proved',
                              'flag/index/phase store coalescing checked through final bytes', 'MSVC and byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'omit-last-column-subtract': ('value : count60 - 1', 'value : count60'),
            'omit-last-row-subtract': ('value : count64 - 1', 'value : count64'),
            'wrong-empty-column': ('else value88 = 0;', 'else value88 = 1;'),
            'wrong-empty-row': ('else value8C = 0;', 'else value8C = 1;'),
            'skip-column-advance': ('++value88;', 'value88 = value88;'),
            'skip-row-advance': ('++value8C;', 'value8C = value8C;'),
            'off-by-one-row-wrap': ('value8C >= count64', 'value8C > count64'),
            'one-only-wrap': ('if (value) value5C', 'if (value == 1) value5C'),
            'wrong-offset-input': ('offset = offset + entry->value0;', 'offset = offset + entry->value8;'),
            'round-offset-accumulator': ('long double offset', 'volatile float offset'),
            'wrong-offset-field': ('entry->value4 =', 'entry->value8 ='),
            'skip-row-offsets': ('if (count64)', 'if (false)'),
            'wrong-finish-mask': ('flags & 0xffffff9d', 'flags & 0xfffffffd'),
            'omit-finish-zero-column': ('if (!value88)', 'if (false)'),
            'omit-finish-zero-row': ('if (!value8C)', 'if (false)'),
            'skip-phase-lower-clamp': ('phase < 0.0', 'phase < -1000.0'),
            'skip-phase-upper-clamp': ('phase > 1.0', 'phase > 1000.0'),
            'skip-phase-reversal': ('valueA8 = -valueA8;', 'valueA8 = valueA8;'),
            'round-phase-to-double': ('long double phase =', 'double phase ='),
            'round-phase-before-compare': ('long double phase =', 'float phase ='),
            'skip-motion': ('if (value5C & 2)', 'if (false)'),
            'omit-vertical-step-rounding': ('float verticalStep = (float)(dy * step);', 'double verticalStep = dy * step;'),
            'skip-finish-boundary': ('            FinishMotion();', '            value5C = value5C;'),
        }
        for name, (before, after) in mutations.items():
            assert before in source, name
            path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
