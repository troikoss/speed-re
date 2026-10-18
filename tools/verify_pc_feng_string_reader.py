#!/usr/bin/env python3
"""Compare the full UG2 string-property dispatch with actual label, hash and UTF-16 callees."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-string-reader'
SOURCE = ROOT / 'src/Speed/Indep/Src/Frontend/FEngStringReader.cpp'
ARENA, OBJECT, READER, OBS, VTABLE, OLD, LABEL, INPUT, PROPERTY, NEW, ALT, NEW_LABEL, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x300, 0x500, 0x540, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x4000, 0x5000, 0x7000, 0x10000))
OBSERVER = 0x838478
CALLBACK = STOP + 0x100
ROOTS = ('SetLabelHash', 'SetLabelBytes', 'ReadProperty')
PREFIXES = dict(SetLabelHash='?SetLabelHash@FEStringObject@', SetLabelBytes='?SetLabelBytes@FEStringObject@',
                ReadProperty='?ReadProperty@FEngStringPropertyReader@', Copy='?FEngCopyText@',
                Grow='?Grow@FEngTextStorage@', GetBuffer='?GetBuffer@FEngTextStorage@',
                Assign='?Assign@FEngTextStorage@', Destruct='?Destruct@FEngTextStorage@', FngHash='?FngHash@')
ARGS = dict.fromkeys(PREFIXES, 1)
ARGS.update(Copy=2, Destruct=0)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
TAGS = (0x4853, 0x4c53, 0x6253, 0x6a53, 0x6c53, 0x7453, 0x7753, 0, 0xffff,
        0x4852, 0x4854, 0x4c54, 0x6252, 0x6a54, 0x7452, 0x7754)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-string-reader.json').read_text())
    manifest['functions'].update(json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-text-storage.json').read_text())['functions'])
    manifest['functions']['FngHash'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid.json').read_text())['functions']['FngHash']
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        start = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngTextStorage.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngHash.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section'] != 0:
                continue
            name = symbol['name']
            if name.startswith('?FEngAllocateQueueNode@'):
                externals[name] = 0x575620
            elif name.startswith('?FEngFreeQueueNode@'):
                externals[name] = 0x575630
            elif name.startswith('?fengStringObserver838478@'):
                externals[name] = OBSERVER
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []

    def words(uc, a, count=1):
        return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))

    def put(uc, a, value):
        uc.mem_write(a, struct.pack('<I', value & 0xffffffff))

    def snapshot(uc):
        return bytes(uc.mem_read(ARENA, 0x6000)) + bytes(uc.mem_read(OBSERVER, 4))

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x8000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']:
            uc.mem_write(a, b)
        helpers = {a: n for n, a in (entries if rebuilt else original).items()}
        ranges = linked['ranges'] if rebuilt else [(a, a + manifest['functions'][n]['size']) for n, a in original.items()]

        def hook(uc, address, size, user, helpers=helpers, ranges=ranges):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, 0 if name in ('Copy', 'FngHash') else uc.reg_read(xc.UC_X86_REG_ECX),
                                       words(uc, sp + 4, ARGS[name]), snapshot(uc)))
                if name == 'Grow':
                    state['kind'] = 'text'
                elif name == 'SetLabelBytes':
                    state['kind'] = 'label'
            if address not in (0x575620, 0x575630, CALLBACK):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))
                return
            ret, argument = words(uc, sp, 2)
            mode = state['mode']
            if address == CALLBACK:
                state['trace'].append(('observe', uc.reg_read(xc.UC_X86_REG_ECX), argument, snapshot(uc)))
                assert argument == OBJECT
                if mode & 1:
                    put(uc, OBJECT + 0x1c, mode * 0x01010101)
                    put(uc, OBJECT + 0x60, 0xabcdef01)
                if mode & 2:
                    put(uc, OBSERVER, 0)
                cleanup = 8
                result = 0xfeedface
            else:
                state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
                cleanup = 4
                if address == 0x575620:
                    if mode & 4:
                        put(uc, OBJECT + 0x64, ALT)
                        put(uc, OBJECT + 0x68, 0x12345678)
                        put(uc, OBJECT + 0x5c, ALT + 0x100)
                    if mode & 8:
                        put(uc, OBSERVER, OBS)
                    result = NEW_LABEL if state['kind'] == 'label' else NEW
                    if state['kind'] == 'text' and mode & 16:
                        result = 0
                else:
                    if mode & 32:
                        put(uc, OBJECT + 0x1c, mode | 2)
                        put(uc, OBJECT + 0x5c, ALT + 0x100)
                        put(uc, OBSERVER, OBS)
                    result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result)
            uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc)
            uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + cleanup)
            uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    counts = dict.fromkeys(ROOTS, 0)
    tags = dict.fromkeys(map(hex, TAGS), 0)
    rng = random.Random(0x534fc0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x8000))
        def putm(a, value):
            struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def utf16(a, length):
            value = [rng.randrange(1, 65536) for _ in range(length)] + [0]
            memory[a - ARENA:a - ARENA + len(value) * 2] = struct.pack('<' + 'H' * len(value), *value)
        old_length = trial % 17
        utf16(OLD, old_length)
        utf16(ALT, trial % 7)
        value = (b'', b'Label', b'mIxEd-CASE', b'\x80\xffz')[trial % 4] + b'\0'
        for a in (INPUT, LABEL, ALT + 0x100):
            memory[a - ARENA:a - ARENA + len(value)] = value
        putm(OBJECT + 0x5c, 0 if trial % 5 == 0 else LABEL)
        putm(OBJECT + 0x64, 0 if trial % 9 == 0 else OLD)
        putm(OBJECT + 0x68, (0, old_length, old_length + 1, 128)[trial % 4])
        current_hash = rng.getrandbits(32)
        putm(OBJECT + 0x60, current_hash)
        putm(OBJECT + 0x1c, (0, 2, 0x400000, 0xffffffff, rng.getrandbits(32))[trial % 5])
        tag = TAGS[trial % len(TAGS)]
        # A zero/invalid declared length must not alter this method's dispatch.
        struct.pack_into('<HH', memory, PROPERTY - ARENA, tag, (0, 1, 0xffff)[trial % 3])
        scalar = (current_hash, 0, 0xffffffff, old_length, 64)[(trial // 3) % 5]
        if tag == 0x4c53:
            memory[PROPERTY + 4 - ARENA:PROPERTY + 4 - ARENA + len(value)] = value
        elif tag == 0x7453:
            utf16(PROPERTY + 4, (trial // 7) % 33)
        else:
            putm(PROPERTY + 4, scalar)
        putm(READER + 0xc, OBJECT)
        memory[READER + 9 - ARENA] = (0, 1, 0x80, 0xff)[(trial // 16) % 4]
        if tag not in TAGS[:7] or tag == 0x4c53 and not memory[READER + 9 - ARENA]:
            if trial % 2:
                putm(READER + 0xc, 0)
        putm(OBS, VTABLE)
        putm(VTABLE, CALLBACK)
        mode = (trial // 19) % 64
        for name in ROOTS:
            this = READER if name == 'ReadProperty' else OBJECT
            argument = PROPERTY if name == 'ReadProperty' else (current_hash if trial % 2 else scalar) if name == 'SetLabelHash' else (0 if trial % 5 == 0 else INPUT)
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory))
                put(uc, OBSERVER, OBS if trial % 3 else 0)
                uc.mem_write(STACK, struct.pack('<II', STOP, argument))
                for i, reg in enumerate(PRESERVED):
                    uc.reg_write(reg, 0x12345000 + i)
                uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                uc.reg_write(xc.UC_X86_REG_ECX, this)
                state.clear()
                state.update(trace=[], mode=mode, kind=None)
                try:
                    uc.emu_start((entries if rebuilt else original)[name], STOP, count=20000)
                except UcError as error:
                    raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 8
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                outputs.append((snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, hex(tag), mode,
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][0], outputs[1][0])) if a != b][:10],
                [x[:-1] for x in outputs[0][1]], [x[:-1] for x in outputs[1][1]])
            counts[name] += 1
        tags[hex(tag)] += 1
    report = dict(compiler=args.compiler, comparisons=sum(counts.values()), cases=counts, tags=tags,
                  byte_matching=False, controlled_boundaries=['allocation', 'free', 'string-observer slot0'])
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'hash-omits-notification': ('if (fengStringObserver838478) fengStringObserver838478->StringChanged(this);', '/* omitted */'),
            'hash-clears-bit2': ('if (languageHash60 != hash)', 'flags1C &= ~2u; if (languageHash60 != hash)'),
            'hash-omits-dirty': ('flags1C |= 0x400000;', '/* omitted */'),
            'label-omits-free': ('FEngFreeQueueNode(label);', '/* omitted */'),
            'label-omits-reset': ('label = 0;', '/* omitted */'),
            'label-omits-terminator': ('FEngAllocateQueueNode(length)', 'FEngAllocateQueueNode(length - 1)'),
            'label-hashes-source': ('languageHash60 = FngHash(label);', 'languageHash60 = FngHash(source);'),
            'reader-ignores-SL-gate': ('if (value9) object->SetLabelBytes', 'if (true) object->SetLabelBytes'),
            'reader-uses-length': ('unsigned int tag = *property;', 'if (!property[1]) return; unsigned int tag = *property;'),
            'reader-Sb-checks-capacity': ('->Grow(*payload);', '->GetBuffer(*payload);'),
            'reader-Sj-offset': ('object->justification6C = (int)*payload;', 'object->leading70 = (int)*payload;'),
            'reader-Sl-offset': ('object->leading70 = (int)*payload;', 'object->maxWidth74 = *payload;'),
            'reader-Sw-offset': ('object->maxWidth74 = *payload;', 'object->justification6C = (int)*payload;'),
            'reader-St-omits-Assign': ('((FEngTextStorage *)&object->text64)->Assign((const unsigned short *)payload);', '/* omitted */'),
        }
        rejected = []
        with tempfile.TemporaryDirectory(dir=BUILD) as directory:
            for label, (before, after) in mutations.items():
                assert before in source, label
                mutant = Path(directory) / (label + '.cpp')
                mutant.write_text(source.replace(before, after, 1))
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '256'],
                                     cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / (label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1600:])
                rejected.append(label)
        report['mutations_rejected'] = rejected
    (BUILD / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
