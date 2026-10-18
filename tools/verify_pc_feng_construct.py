#!/usr/bin/env python3
"""Compare common/grid constructors, untouched bytes, GUID side effects and base calls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-construct'
ARENA, SELF, STACK, STOP, GLOBALS = 0x2000000, 0x2000100, 0x2003000, 0x2100000, 0x7f7b00
COUNTER, DEFAULT = 0x7f7cf8, 0x7f7cc4
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ConstructBase': '?ConstructBase@HudVisibilityObject@', 'ConstructGrid': '?Construct@FEGridObject@'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngConstruct.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-construct.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()):
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        if s['name'].startswith('?FEngNextObjectGuid@'): externals[s['name']] = COUNTER
        elif s['name'].startswith('?FEngValue7F7CC4@'): externals[s['name']] = DEFAULT
        else: raise AssertionError(('unexpected external', s['name']))
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    state, machines = {}, []
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x4000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
            if address in helpers:
                state['trace'].append((helpers[address], uc.reg_read(xc.UC_X86_REG_ECX),
                                       bytes(uc.mem_read(ARENA, 0x1000)), bytes(uc.mem_read(GLOBALS, 0x400))))
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x52af20); counts = dict.fromkeys(entries, 0)
    for trial in range(4096):
        memory, globals_ = rng.randbytes(0x4000), bytearray(rng.randbytes(0x400))
        initial = (0, 1, 0xffffffff, 0xfffffffe, rng.getrandbits(32))[trial % 5]
        struct.pack_into('<I', globals_, COUNTER - GLOBALS, initial)
        struct.pack_into('<I', globals_, DEFAULT - GLOBALS, rng.getrandbits(32))
        # Selected aliases pin increment/write and late default-read boundaries;
        # arbitrary aliases and concurrent global mutation are not covered.
        this = (SELF, COUNTER - 0xc, DEFAULT - 0x5c, DEFAULT - 0xc0)[trial % 4]
        for name in entries:
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, memory); uc.mem_write(GLOBALS, bytes(globals_))
                uc.mem_write(STACK, struct.pack('<I', STOP)); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                uc.reg_write(xc.UC_X86_REG_ECX, this)
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                state.clear(); state.update(trace=[])
                address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
                try: uc.emu_start(address, STOP, count=10000)
                except UcError as error:
                    if not rebuilt: raise
                    raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                outputs.append((uc.reg_read(xc.UC_X86_REG_EAX), bytes(uc.mem_read(ARENA, 0x1000)),
                                bytes(uc.mem_read(GLOBALS, 0x400)), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, hex(this))
            counts[name] += 1
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'actual_callees': ['common base constructor'], 'controlled_interfaces': [],
              'limitations': ['finite dirty-prefix/global cases and selected aliases, no arbitrary aliases/concurrency',
                              'retail vtable/default words retained as tokens, virtual dispatch/lifecycle not established',
                              'byte matching and MSVC 7.1 pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'wrong-base-vtable': [('= 0x79b8e0;', '= 0x79b904;')],
            'zero-list-sentinels': [('(HudVisibilityObject *)0xabadcafe', '(HudVisibilityObject *)0')],
            'omit-counter-increment': [('FEngNextObjectGuid = guid + 1;', 'FEngNextObjectGuid = guid;')],
            'store-incremented-guid': [('guidC = guid;', 'guidC = guid + 1;')],
            'zero-untouched-script': [('    hash10 = 0;', '    currentScript54 = 0;\n    hash10 = 0;')],
            'zero-resource-halfword': [('*(unsigned short *)unidentified20 = 0;', '*(unsigned int *)unidentified20 = 0;')],
            'wrong-inner-list-vtable': [('= 0x79a9ec;', '= 0;')],
            'omit-base-call': [('((HudVisibilityObject *)this)->ConstructBase();', '(void)this;')],
            'wrong-grid-flags': [('flags68 = 2;', 'flags68 = 0;')],
            'wrong-grid-type': [('= 6;', '= 4;')],
            'wrong-grid-phase': [('= 0xbab60b61;', '= 0;')],
            'wrong-grid-colour-word': [('= 0xff;', '= 0xffffffff;')],
            'zero-grid-default': [('unsigned int valueBC = FEngValue7F7CC4;', 'unsigned int valueBC = 0;')],
            'early-grid-default': [('    ((HudVisibilityObject *)this)->ConstructBase();', '    unsigned int earlyBC = FEngValue7F7CC4;\n    ((HudVisibilityObject *)this)->ConstructBase();'), ('unsigned int valueBC = FEngValue7F7CC4;', 'unsigned int valueBC = earlyBC;')],
        }
        for name, edits in mutations.items():
            changed = source
            for before, after in edits:
                assert before in changed, name
                changed = changed.replace(before, after)
            path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(changed)
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler, '--source', str(path)], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
