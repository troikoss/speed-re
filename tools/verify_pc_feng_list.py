#!/usr/bin/env python3
"""Compare complete type-4 list navigation against original UG2 x86 execution."""
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
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-list'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
OBJECT, XROWS, YROWS = ARENA + 0x100, ARENA + 0x1000, ARENA + 0x2000
ADDRESS, SIZE = 0x516e90, 927
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngList.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-list.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    assert hashlib.sha256(image[ADDRESS - 0x400000:ADDRESS - 0x400000 + SIZE]).hexdigest() == manifest['functions']['Navigate']['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-ffp-eval-method=extended'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                             '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    linked = link_objects([obj], {})
    rebuilt_entry = next(a for n, a in linked['exports'].items() if n.startswith('?Navigate@FEListObject@'))
    state, machines = {}, []
    markers = {0x516eb0: 'x-busy', 0x516ebd: 'y-busy', 0x516efa: 'x-upper-clamp',
               0x516f01: 'x-lower-clamp', 0x516fa2: 'y-upper-clamp', 0x516fa9: 'y-lower-clamp',
               0x5170b8: 'x-negative-wrap', 0x51710f: 'x-positive-wrap',
               0x51715e: 'y-negative-wrap', 0x5171bb: 'y-positive-wrap',
               0x516f54: 'x-viewport-adjust', 0x516fff: 'y-viewport-adjust',
               0x51714f: 'x-wrap-flag', 0x5171fe: 'y-wrap-flag',
               0x51705c: 'settle', 0x517209: 'normalize'}
    coverage = dict.fromkeys(markers.values(), 0)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, data in linked['blocks']: uc.mem_write(address, data)
        ranges = linked['ranges'] if rebuilt else [(ADDRESS, ADDRESS + SIZE)]
        def hook(uc, address, size, user, ranges=ranges, original=not rebuilt):
            assert any(start <= address < end for start, end in ranges), ('unexpected execution', hex(address))
            if original and address in markers: coverage[markers[address]] += 1
        def write(uc, access, address, size, value, user):
            if ARENA <= address < ARENA + 0xe000 and address != OBJECT + 0x5c:
                state['writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, hook)
        uc.hook_add(UC_HOOK_MEM_WRITE, write)
        machines.append(uc)
    rng = random.Random(ADDRESS)
    comparisons = 0
    for trial in range(16896):
        memory = bytearray(rng.randbytes(0x10000))
        def put(offset, value): struct.pack_into('<I', memory, OBJECT - ARENA + offset, value & 0xffffffff)
        def floating(offset, value): struct.pack_into('<f', memory, OBJECT - ARENA + offset, value)
        nx, ny = 1 + trial % 8, 1 + (trial // 8) % 8
        put(0x5c, ((trial // 64) % 128) | (rng.getrandbits(32) & 0xffffff80))
        put(0x60, nx); put(0x64, ny); put(0x78, XROWS); put(0x7c, YROWS)
        ix, iy = (trial // 5) % nx, (trial // 7) % ny
        put(0x88, ix); put(0x8c, iy)
        # Exact binary fractions cover wrap, clamp, viewport and settling paths.
        for i in range(8):
            for rows in (XROWS, YROWS):
                struct.pack_into('<fff', memory, rows - ARENA + i * 12, (i + 1) * .25 if trial < 8192 else rng.uniform(-100, 100), i * 2.0 if trial < 8192 else rng.uniform(-100, 100), rng.uniform(-100, 100))
        for offset in (0x68, 0x6c): floating(offset, (0, .25, 1, 8)[trial % 4])
        for offset in (0x70, 0x74, 0x94, 0x98): floating(offset, (0, .03125, .5, 2)[(trial // 9 + offset) % 4] if trial < 8192 else rng.uniform(-100, 100))
        x = (-1, 0, 1, 2)[(trial // 17) % 4]
        y = (-1, 0, 1, 2)[(trial // 23) % 4]
        if trial >= 16384:
            # Original unordered normalization branch, infinities and tiny deltas.
            put(0x5c, 4)
            x, y = 0, 1
            put(0x94, (0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000, 0, 0x80000000, 1, 0x80000001)[trial % 8])
            floating(0x70, 0)
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<III', STOP, x & 0xffffffff, y & 0xffffffff))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            uc.reg_write(xc.UC_X86_REG_ECX, OBJECT)
            uc.reg_write(xc.UC_X86_REG_FPCW, (0x27f, 0x37f)[trial % 2])
            uc.reg_write(xc.UC_X86_REG_FPSW, 0)
            uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, register in enumerate(PRESERVED): uc.reg_write(register, 0x12345000 + i)
            state['writes'] = []
            uc.emu_start(rebuilt_entry if rebuilt else ADDRESS, STOP, count=20000)
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('nontermination', trial)
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 12
            assert all(uc.reg_read(register) == 0x12345000 + i for i, register in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA, 0xe000)), state['writes']))
        if outputs[0] != outputs[1]:
            differences = [(hex(i + ARENA), a, b) for i, (a,b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b]
            raise AssertionError(('state or write mismatch', trial, x, y, differences[:25], outputs[0][1], outputs[1][1]))
        comparisons += 1
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': comparisons, 'original_block_hits': coverage,
              'controlled_interfaces': [], 'x87_control_words': ['0x27f', '0x37f'], 'limitations': ['finite valid nonempty axis arrays; x/y deltas -1,0,1,2; finite floats and selected exceptional values', 'compiler may coalesce nonvolatile flag stores; compared through final state', 'MSVC and byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text()
        report['rejected_mutations'] = []
        mutations = {
            'ignore-busy-x': ('if (flags & 0x20) x = 0;', 'if (false) x = 0;'),
            'ignore-busy-y': ('if (flags & 0x40) y = 0;', 'if (false) y = 0;'),
            'wrong-x-index': ('value88 = selected;', 'value88 = 0;'),
            'wrong-y-index': ('value8C = selected;', 'value8C = 0;'),
            'fix-unsigned-negative-wrap': ('value88 = selected % count;', 'value88 = (selected + count) % count;'),
            'wrong-wrap-y-flag': ('value5C = flags | 8;', 'value5C = flags | 0x10;'),
            'wrong-x-viewport': ('remaining < value68', 'remaining > value68'),
            'wrong-y-viewport': ('remaining < value6C', 'remaining > value6C'),
            'omit-x-busy': ('value5C |= 0x20;', 'value5C |= 0;'),
            'omit-y-busy': ('value5C |= 0x40;', 'value5C |= 0;'),
            'omit-dirty': ('value5C |= 2;', 'value5C |= 0;'),
            'reverse-dx': ('value94 - value70', 'value70 - value94'),
            'reverse-dy': ('value98 - value74', 'value74 - value98'),
            'wrong-normalization': ('1.0f / length', '1.0f / (length * length)'),
            'wrong-threshold': ('length < 0.1f', 'length < 1.0f'),
            'wrong-settle-flags': ('flags & 0xffffff9d', 'flags & 0xfffffffd'),
            'skip-zero-index-reset': ('if (!value88)', 'if (false)'),
            'wrong-unordered-branch': ('!(length < 0.1f)', 'length >= 0.1f'),
            'round-normalization-to-float': ('double length = sqrt(', 'float length = (float)sqrt('),
        }
        for name, (before, after) in mutations.items():
            assert before in source, name
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or write mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
