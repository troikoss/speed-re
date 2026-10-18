#!/usr/bin/env python3
"""Compare original/rebuilt response-command ownership; only heap calls are controlled."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys

import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-response-command'
SOURCE = ROOT / 'src/Speed/Indep/Src/Frontend/FEngResponseCommand.cpp'
ARENA, SELF, SOURCE_COMMAND, OLD, INPUT, ALT, NEW, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x200, 0x1000, 0x2000, 0x3000, 0x4140, 0x7000, 0x10000))
PREFIXES = dict(Construct='?FEngConstructResponseCommand@', Destruct='?FEngDestructResponseCommand@',
                SetString='?SetString@FEResponseCommand@', Assign='?Assign@FEResponseCommand@')
ARGS = dict(Construct=0, Destruct=0, SetString=1, Assign=1)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-response-command.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'FEngResponseCommand.obj'
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for symbol in read_object(obj)['symbols'].values():
        if symbol['section'] != 0: continue
        if symbol['name'].startswith('?FEngAllocateQueueNode@'): externals[symbol['name']] = 0x575620
        elif symbol['name'].startswith('?FEngFreeQueueNode@'): externals[symbol['name']] = 0x575630
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []
    hits = dict.fromkeys(PREFIXES, 0)

    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x5000))

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x8000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        helpers = {a: n for n, a in (entries if rebuilt else original).items()}
        ranges = linked['ranges'] if rebuilt else [(a, a + manifest['functions'][n]['size']) for n, a in original.items()]

        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original_run=not rebuilt):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                if original_run: hits[name] += 1
                state['trace'].append((name, uc.reg_read(xc.UC_X86_REG_ECX), words(uc, sp + 4, ARGS[name]), snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            ret, argument = words(uc, sp, 2)
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            mode = state['mode']
            if mode & 1: put(uc, SELF + 4, ALT)
            if mode & 2: put(uc, SELF, 0x203 if address == 0x575620 else 0x204)
            if mode & 4:
                put(uc, SOURCE_COMMAND + 4, ALT)
                put(uc, SOURCE_COMMAND + 8, 0xaabbccdd)
            if mode & 8 and uc.mem_read(INPUT, 1) != b'\0': uc.mem_write(INPUT, b'Q')
            uc.reg_write(xc.UC_X86_REG_EAX, NEW if address == 0x575620 else 0xfeedface)
            uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)

    rng = random.Random(0x52ad80)
    types = (0, 1, 0x1ff, 0x200, 0x201, 0x202, 0x203, 0x204, 0x205, 0xffffffff)
    counts = dict.fromkeys(PREFIXES, 0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x8000))
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        for a, length in ((OLD, trial % 17), (INPUT, (trial // 3) % 33), (ALT, trial % 7)):
            value = bytes(rng.randrange(1, 256) for _ in range(length)) + b'\0'
            memory[a - ARENA:a - ARENA + len(value)] = value
        putm(SELF, types[trial % len(types)]); putm(SELF + 4, 0 if trial % 9 == 0 else OLD)
        putm(SOURCE_COMMAND, types[(trial // len(types)) % len(types)])
        putm(SOURCE_COMMAND + 4, 0 if trial % 7 == 0 else INPUT)
        mode = (trial // 101) % 16
        for name in PREFIXES:
            argv = [] if not ARGS[name] else [SELF + 4 if trial % 13 == 0 else 0 if trial % 5 == 0 else INPUT] if name == 'SetString' else [SELF if trial % 11 == 0 else SOURCE_COMMAND]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory))
                uc.mem_write(STACK, struct.pack('<' + 'I' * (len(argv) + 1), STOP, *argv))
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, SELF)
                state.clear(); state.update(trace=[], mode=mode)
                try: uc.emu_start((entries if rebuilt else original)[name], STOP, count=20000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, mode, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + len(argv) * 4
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                result = uc.reg_read(xc.UC_X86_REG_EAX) if name in ('Construct', 'Assign') else None
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts,
                  actual_entry_hits=hits, byte_matching=False, controlled_boundaries=['0x575620 allocation', '0x575630 free'])
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'ctor-wrong-target': ('command->target = 0;', 'command->target = 1;'),
            'ctor-wrong-return': ('return command;', 'return 0;'),
            'ownership-includes-203': (' && value != 0x203', ''),
            'ownership-omits-204': ('value <= 0x204', 'value < 0x204'),
            'destruct-omit-free': ('FEngFreeQueueNode((void *)command->parameter);', '/* omitted */'),
            'destruct-omit-clear': ('command->parameter = 0;\n}', '/* omitted */\n}'),
            'string-omit-clear': ('    parameter = 0;\n    if (text)', '    if (text)'),
            'string-wrong-size': ('FEngAllocateQueueNode(length)', 'FEngAllocateQueueNode(length - 1)'),
            'string-store-before-copy': ('        char *destination = buffer;', '        parameter = (unsigned int)buffer;\n        char *destination = buffer;'),
            'assign-omit-initial-clear': ('    parameter = 0;\n    value = source->type;', '    value = source->type;'),
            'assign-shallow-string': ('        SetString((const char *)source->parameter);', '        parameter = source->parameter;'),
            'assign-cache-target': [('    unsigned int value = type;', '    unsigned int cachedTarget = source->target;\n    unsigned int value = type;'), ('target = source->target;', 'target = cachedTarget;')],
        }
        for label, edits in mutations.items():
            if isinstance(edits, tuple): edits = [edits]
            mutant_source = source
            for before, after in edits:
                # Scope assignment-only mutation to its definition.
                if label == 'assign-cache-target':
                    prefix, body = mutant_source.split('FEResponseCommand *FEResponseCommand::Assign', 1)
                    assert before in body; mutant_source = prefix + 'FEResponseCommand *FEResponseCommand::Assign' + body.replace(before, after)
                else:
                    assert before in mutant_source; mutant_source = mutant_source.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(mutant_source)
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '512'],
                                 cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
        report['rejected_mutations'] = list(mutations)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
