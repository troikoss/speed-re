#!/usr/bin/env python3
"""Compare the script reader's Ti track search, type-descriptor lookup and property indexer."""
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
BUILD = ROOT / 'build/analysis/feng-track-lookup'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x4000, 0x2008000, 0x2100000
KEYS = 0x7992f4
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'FindTrack': '?FindTrack@FEScript@', 'FindDescriptor': '?Find@FEngTypeRegistry@',
            'GetProperty': '?GetProperty@FEngTypeDescriptor@'}


def track_case(rng, keys):
    memory = bytearray(rng.randbytes(ARENA_SIZE))
    if rng.random() < 0.25:  # Controlled table values pin the signed byte compare.
        keys = [rng.randrange(-128, 128) for _ in keys]
    count = rng.choice((0, 1, 2, 3, rng.randrange(4, 64)))
    tracks = ARENA + 0x100
    struct.pack_into('<iI', memory, 0x1c, count, tracks)
    pool = keys + [rng.randrange(-128, 128) for _ in range(3)]
    for i in range(count):
        # Signed bytes above 0x7f must not match positive keys; duplicates pin first-match order.
        memory[0x100 + i * 0x38 + 7] = rng.choice(pool) & 0xff
    selector = rng.randrange(11) if rng.random() < 0.9 else rng.randrange(11, 16)
    if rng.random() < 0.05:
        # Negative counts compare unsigned in the original; a guaranteed early match bounds the walk.
        selector = rng.randrange(11); hit = rng.randrange(64)
        struct.pack_into('<i', memory, 0x1c, -rng.randrange(1, 100))
        for i in range(hit + 1):
            memory[0x100 + i * 0x38 + 7] = (keys[selector] if i == hit else keys[selector] + 1) & 0xff
    return memory, ARENA, selector, keys


def list_case(rng, key_offset, link_offset, head_offset):
    memory = bytearray(rng.randbytes(ARENA_SIZE))
    count = rng.choice((0, 1, 2, rng.randrange(3, 40)))
    nodes = rng.sample(range(0x100, ARENA_SIZE - 0x40, 0x40), count)
    for i, node in enumerate(nodes):
        struct.pack_into('<I', memory, node + link_offset, ARENA + nodes[i + 1] if i + 1 < count else 0)
    struct.pack_into('<I', memory, head_offset, ARENA + nodes[0] if nodes else 0)
    if key_offset is None:
        return memory, ARENA, rng.choice((-1, 0, 1, count - 1, count, count + 1, rng.randrange(-5, 50))), None
    types = [rng.choice((0, 1, 2, 3, 5, -1, rng.getrandbits(32) - 2**31)) for _ in nodes]
    for node, value in zip(nodes, types):
        struct.pack_into('<i', memory, node + key_offset, value)
    wanted = rng.choice(types) if types and rng.random() < 0.7 else rng.choice((0, 1, 7, -1))
    return memory, ARENA, wanted, None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngTrackLookup.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-track-lookup.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()):
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    keys = list(struct.unpack('<11i', image[KEYS - 0x400000:KEYS - 0x400000 + 44]))
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        if s['name'].startswith('?fengStateKeys7992F4@'): externals[s['name']] = KEYS
        else: raise AssertionError(('unexpected external', s['name']))
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    machines = []
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        def hook(uc, address, size, user, ranges=ranges):
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    makers = {'FindTrack': lambda rng: track_case(rng, keys),
              'FindDescriptor': lambda rng: list_case(rng, 0x24, 4, 8),
              'GetProperty': lambda rng: list_case(rng, None, 4, 0x1c)}
    rng = random.Random(0x504a00); counts = dict.fromkeys(entries, 0); hits = dict.fromkeys(entries, 0)
    for trial in range(4096):
        for name in entries:
            memory, this, argument, table = makers[name](rng)
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory))
                uc.mem_write(KEYS, struct.pack('<11i', *(table or keys)))
                uc.mem_write(STACK, struct.pack('<Ii', STOP, argument)); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                uc.reg_write(xc.UC_X86_REG_ECX, this)
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
                try: uc.emu_start(address, STOP, count=20000)
                except UcError as error:
                    if not rebuilt: raise
                    raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
                outputs.append((uc.reg_read(xc.UC_X86_REG_EAX), uc.reg_read(xc.UC_X86_REG_ESP),
                                tuple(uc.reg_read(reg) for reg in PRESERVED), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
            assert outputs[0][1] == STACK + 8, ('original stdcall cleanup', name)
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name)
            counts[name] += 1; hits[name] += outputs[0][0] != 0
    report = {'compiler': args.compiler, 'comparisons': counts, 'nonnull_results': hits, 'total': sum(counts.values()),
              'actual_callees': [], 'controlled_interfaces': [],
              'limitations': ['finite acyclic lists and in-arena track tables; cycles and invalid pointers not covered; negative counts only with an in-arena match',
                              '25% of track cases substitute controlled key-table values; selectors 11..15 read adjacent retail data as the original does; registry producer unrecovered',
                              'byte matching and MSVC 7.1 pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'unsigned-state-byte': [('(int)track->state7 == key', '(int)(unsigned char)track->state7 == key')],
            'last-track-match': [('if ((int)track->state7 == key) return track;', 'if ((int)track->state7 == key) found = track;'),
                                 ('    unsigned int index = 0;', '    unsigned int index = 0;\n    FEScriptTrack *found = 0;'),
                                 ('    return 0;\n}\n\n// Original 0x505480', '    return found;\n}\n\n// Original 0x505480')],
            'signed-count': [('index < (unsigned int)trackCount1C', '(int)index < trackCount1C')],
            'selector-not-mapped': [('int key = fengStateKeys7992F4[selector];', 'int key = selector;')],
            'descriptor-skip-head': [('FEngTypeDescriptor *descriptor = first8;', 'FEngTypeDescriptor *descriptor = first8 ? first8->next4 : 0;')],
            'property-one-based': [('int position = 0;', 'int position = 1;')],
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
