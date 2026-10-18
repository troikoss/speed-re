#!/usr/bin/env python3
"""Verify original/rebuilt FEng tree, package-chain and typed object lookup."""
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
                              UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
                              UC_X86_REG_GDTR, UC_X86_REG_CS, UC_X86_REG_SS,
                              UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_FS,
                              UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-lookup'
ARENA, STACK, STOP, TIB, GDT = 0x2000000, 0x200f000, 0x2100000, 0x2120000, 0x2130000
PACKAGES = [ARENA + x for x in (0x100, 0x200, 0x300)]
QUEUE, MANAGER, QUERY, VISITOR, VTABLE = [ARENA + x for x in (0x600, 0x800, 0x1200, 0x1300, 0x1400)]
NAMES = [ARENA + 0x3000 + i * 0x100 for i in range(10)]
NODES = [[ARENA + 0x4000 + p * 0x1000 + i * 0x100 for i in range(6)] for p in range(3)]
GLOBAL, MODE, CALLBACK, ARCHIVE = 0x8384c4, 0x8384d0, STOP + 0x800, 0x8637b0
ARCHIVE_NODE = ARENA + 0x1800
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
PREFIXES = {
    'QueryVisit': '?Visit@FEngHashQuery@', 'VisitChildren': '?VisitChildren@FEPackage@',
    'VisitObjects': '?VisitObjects@FEPackage@', 'PackageFindObject': '?FindObject@FEPackage@',
    'CompareNames': '?FEngComparePackageNames@', 'FindByName': '?FindByName@FEngPackageList@',
    'QueueFindObject': '?FindObject@FEngQueue@', 'FngFindObject5379C0': '?FngFindObject5379C0@',
    'FngFindStringByHash': '?FngFindStringByHash@', 'FngFindImageByHash': '?FngFindImageByHash@',
    'cihash': '_cihash', 'FngFindPackage': '?FngFindPackage@',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngLookup.cpp')
    parser.add_argument('--locator-source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngLocator.cpp')
    parser.add_argument('--script-lookup-source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngScriptLookup.cpp')
    parser.add_argument('--compose-formatting', action='store_true', help='Execute formatting wrappers/profile with formatter core and clock controlled')
    parser.add_argument('--compose-scripts', action='store_true', help='Execute recovered script wrappers and activation through actual archive lookup')
    parser.add_argument('--compose-archive', action='store_true', help='Execute recovered archive search instead of controlling it')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    if args.compose_formatting: args.compose_scripts = True
    if args.compose_scripts: args.compose_archive = True
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lookup.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    functions = dict(manifest['functions'])
    if args.compose_archive:
        archive_manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-archive.json').read_text())
        assert archive_manifest['executable_sha256'] == manifest['executable_sha256']
        functions.update(archive_manifest['functions'])
    if args.compose_scripts:
        for name in ('feng-script', 'feng-script-lookup'):
            extra = json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + name + '.json')).read_text())
            assert extra['executable_sha256'] == manifest['executable_sha256']
            functions.update(extra['functions'])
    if args.compose_formatting:
        extra = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-formatting.json').read_text())
        assert extra['executable_sha256'] == manifest['executable_sha256']
        functions.update(extra['functions'])
    for f in functions.values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    objects = []
    sources = [args.source, args.locator_source, ROOT / 'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if args.compose_archive:
        sources.append(ROOT / 'src/Speed/Indep/Src/Frontend/FEngArchive.cpp')
    if args.compose_scripts:
        sources.extend([ROOT / 'src/Speed/Indep/Src/Frontend/FEngScript.cpp', args.script_lookup_source])
    if args.compose_formatting:
        sources.extend([ROOT / 'src/Speed/Indep/Src/Frontend/FEngFormatting.cpp',
                        ROOT / 'src/Speed/Indep/Libs/bWare/Formatting.cpp'])
    for source in sources:
        obj = BUILD / (source.stem + '.obj')
        if args.compiler == 'msvc':
            command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc']
        else:
            command = ['clang-cl', '--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/Ob0', '/GS-', '/GR-', '/w', '/EHsc', '/clang:-fno-builtin', '/arch:IA32']
        subprocess.run(command + ['/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend',
                                  os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    external = {'?FindNode@FEngArchive@@QAEPAUFEngPackageNode@@PBD@Z': 0x51bb70,
                '?fengArchive8637B0@@3UFEngArchive@@A': ARCHIVE,
                '?fengValue8384D0@@3IA': MODE,
                '?fengManager8384C4@@3PAUFEngManager@@A': GLOBAL,
                '_strlen': STOP + 0x100, '??3@YAXPAX@Z': STOP + 0x200,
                '??3@YAXPAXI@Z': STOP + 0x300, '__purecall': STOP + 0x400,
                '___CxxFrameHandler3': STOP + 0x500, '___CxxFrameHandler': STOP + 0x500}
    if args.compose_scripts:
        external['?FngFormatHash@@YAIPBDZZ'] = 0x50b9c0
    if args.compose_archive:
        del external['?FindNode@FEngArchive@@QAEPAUFEngPackageNode@@PBD@Z']
    if args.compose_formatting:
        del external['?FngFormatHash@@YAIPBDZZ']
        external['__allshr'] = 0x75c600
        external['__fltused'] = ARENA + 0x7800
        formatting_globals = {'bFormatQueryCounter': STOP + 0xa00, 'bFormatCore': 0x43e950,
                              'bFormatShift8284E4': 0x8284e4, 'bFormatScale828028': 0x828028,
                              'bFormatTime828518': 0x828518, 'bFormatCount82851C': 0x82851c}
        for obj in objects:
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section'] == 0:
                    for name, address in formatting_globals.items():
                        if symbol['name'].startswith('?' + name + '@'): external[symbol['name']] = address
    linked = link_objects(objects, external)
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix))
               for name, prefix in PREFIXES.items()}
    if args.compose_archive:
        entries['FindNode'] = next(a for n, a in linked['exports'].items() if n.startswith('?FindNode@FEngArchive@'))
    if args.compose_scripts:
        for name in ('FngSetScript', 'FngSetScriptHash', 'FngScriptActive', 'FngScriptRunning'):
            entries[name] = next(a for n, a in linked['exports'].items() if n.startswith('?' + name + '@'))
    state, machines = {}, []

    def words(uc, address, count=1):
        return struct.unpack('<' + 'I' * count, uc.mem_read(address, count * 4))

    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095)
        uc.mem_write(0x400000, image)
        if args.compose_formatting: uc.mem_write(0x78311c, struct.pack('<I', STOP + 0xa00))
        uc.mem_map(ARENA, 0x10000)
        uc.mem_map(STOP, 0x1000)
        uc.mem_map(TIB, 0x1000)
        uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return ((limit & 0xffff) | (base & 0xffffff) << 16 | access << 40 | (limit >> 16 & 15) << 48
                    | flags << 52 | (base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93),
                                         descriptor(TIB, 0xfff, 0x93, 4)))
        uc.reg_write(UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((UC_X86_REG_CS, 8), (UC_X86_REG_SS, 16), (UC_X86_REG_DS, 16),
                              (UC_X86_REG_ES, 16), (UC_X86_REG_FS, 24)):
            uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in functions.values()]
        query_entry = entries['QueryVisit'] if rebuilt else 0x5048d0
        hash_entry = entries['cihash'] if rebuilt else 0x505450
        compare_entry = entries['CompareNames'] if rebuilt else 0x505310
        list_entry = entries['FindByName'] if rebuilt else 0x519e50
        locator_entry = entries['FngFindPackage'] if rebuilt else 0x52cef0

        script_calls = {}
        if args.compose_scripts:
            for name, original, prefix in (
                ('name-select', 0x51cf70, '?FngSetObjectScript@'),
                ('hash-select', 0x51cfe0, '?FngSetObjectScriptHash@'),
                ('initialize', 0x517720, '?InitializeScript@HudVisibilityObject@')):
                entry = next(a for n, a in linked['exports'].items() if n.startswith(prefix)) if rebuilt else original
                script_calls[entry] = name
        def hook(uc, address, size, user, ranges=ranges, query_entry=query_entry,
                 hash_entry=hash_entry, compare_entry=compare_entry, list_entry=list_entry, locator_entry=locator_entry, script_calls=script_calls):
            sp = uc.reg_read(UC_X86_REG_ESP)
            if address in script_calls:
                method = script_calls[address]
                if method == 'initialize':
                    this = uc.reg_read(UC_X86_REG_ECX)
                    state['trace'].append((method, this, words(uc, this + 0x54)[0]))
                else:
                    obj, script, restart = words(uc, sp + 4, 3)
                    state['trace'].append((method, obj, script, restart & 255))
            if address == locator_entry:
                state['trace'].append(('package', words(uc, sp + 4)[0]))
            if address == list_entry:
                # The callee ignores this; Clang may omit its load even with /Ob0.
                state['trace'].append(('package-list', words(uc, sp + 4, 2)))
            if address == query_entry:
                this = uc.reg_read(UC_X86_REG_ECX)
                state['trace'].append(('query', words(uc, this + 4, 2), words(uc, sp + 4)[0]))
            if address == hash_entry:
                state['trace'].append(('hash', words(uc, sp + 4)[0]))
            if address == compare_entry:
                state['trace'].append(('name-compare', words(uc, sp + 4, 2)))
            if address == STOP + 0xa00 and args.compose_formatting:
                ret, destination = words(uc, sp, 2)
                tick = 1000000 + state['clock_index']
                state['clock_index'] += 1
                state['trace'].append(('counter', tick))
                uc.mem_write(destination, struct.pack('<Q', tick))
                uc.reg_write(UC_X86_REG_EAX, 1)
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 8)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            if address == 0x43e950 and args.compose_formatting:
                ret, context, argument, raw_arguments = words(uc, sp, 4)
                assert bytes(uc.mem_read(context + 4, 1)) == b'\0' and words(uc, context + 8)[0] == 0
                state['trace'].append(('format-core', argument))
                text_ = (b'go', b'', b'notfound')[state['format_variant']]
                destination = words(uc, context)[0]
                uc.mem_write(destination, text_ + b'\0')
                uc.mem_write(context, struct.pack('<I', destination + len(text_)))
                if state['format_mutation']:
                    for nodes in NODES:
                        for node in nodes: uc.mem_write(node + 0x54, bytes(4))
                uc.reg_write(UC_X86_REG_EAX, len(text_))
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 4)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            if address == 0x50b9c0 and args.compose_scripts and not args.compose_formatting:
                ret, argument = words(uc, sp, 2)
                state['trace'].append(('format-hash', argument))
                if state['format_mutation']:
                    for nodes in NODES:
                        for node in nodes: uc.mem_write(node + 0x54, bytes(4))
                uc.reg_write(UC_X86_REG_EAX, state['format_hash'])
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 4)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            if address == CALLBACK or (address == 0x51bb70 and not args.compose_archive):
                ret, argument = words(uc, sp, 2)
                if address == 0x51bb70:
                    assert uc.reg_read(UC_X86_REG_ECX) == ARCHIVE
                    state['trace'].append(('archive', argument))
                    result, cleanup = state['archive_node'], 4
                    if state['disable_manager']:
                        uc.mem_write(GLOBAL, bytes(4))
                else:
                    assert uc.reg_read(UC_X86_REG_ECX) == VISITOR
                    state['trace'].append(('visit', argument, words(uc, argument + 0x10, 3)))
                    state['visits'] += 1
                    if state['visits'] == 1:
                        if state['mutation'] & 1:
                            uc.mem_write(argument + 4, bytes(4))
                        if state['mutation'] & 2:
                            uc.mem_write(argument + 0x18, struct.pack('<I', 1))
                    result = 0 if state['stop_after'] == state['visits'] else state['true_byte']
                    result |= 0xa5a50000
                    cleanup = 4
                uc.reg_write(UC_X86_REG_EAX, result)
                uc.reg_write(UC_X86_REG_ECX, 0xcccccccc)
                uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
                uc.reg_write(UC_X86_REG_ESP, sp + 4 + cleanup)
                uc.reg_write(UC_X86_REG_EIP, ret)
                return
            assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address))

        uc.hook_add(UC_HOOK_CODE, hook)
        machines.append(uc)

    rng = random.Random(0x537a10)
    counts = dict.fromkeys(entries, 0)

    def compare(name, memory, arguments, this=0, cleanup=0, byte_result=False, manager=0, mode=0, void_result=False, **fixture):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(GLOBAL, struct.pack('<I', manager))
            uc.mem_write(MODE, struct.pack('<I', mode))
            if args.compose_formatting:
                uc.mem_write(0x8284e4, bytes(4))
                uc.mem_write(0x828028, struct.pack('<f', 0.001))
                uc.mem_write(0x828518, bytes(8))
                uc.reg_write(UC_X86_REG_FPCW, 0x27f)
                uc.reg_write(UC_X86_REG_FPSW, 0)
                uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
            if args.compose_archive:
                head = ARCHIVE_NODE + 4 if fixture['archive_node'] else ARCHIVE + 4
                uc.mem_write(ARCHIVE + 4, struct.pack('<II', head, head))
            uc.mem_write(TIB, struct.pack('<I', 0x12345678))
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_ECX, this)
            for i, reg in enumerate(PRESERVED):
                uc.reg_write(reg, 0x12345000 + i)
            state.clear()
            state.update(fixture, trace=[], visits=0, clock_index=0)
            uc.emu_start(entries[name] if rebuilt else int(functions[name]['address'], 16), STOP, count=20000)
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == STACK + 4 + cleanup
            assert all(uc.reg_read(r) == 0x12345000 + i for i, r in enumerate(PRESERVED))
            assert words(uc, TIB)[0] == 0x12345678
            if args.compose_formatting:
                assert uc.reg_read(UC_X86_REG_FPCW) == 0x27f
                assert uc.reg_read(UC_X86_REG_FPSW) & 0x3800 == 0
            value = uc.reg_read(UC_X86_REG_EAX)
            outputs.append((None if void_result else value & 255 if byte_result else value, bytes(uc.mem_read(ARENA, 0xe000)),
                            words(uc, GLOBAL), words(uc, MODE), state['trace'],
                            bytes(uc.mem_read(ARCHIVE, 12)) if args.compose_archive else b'',
                            bytes(uc.mem_read(0x828518, 8)) if args.compose_formatting else b''))
        assert outputs[0] == outputs[1], ('state or call order mismatch', name, outputs[0][0], outputs[1][0],
                                         outputs[0][4], outputs[1][4])
        counts[name] += 1

    def hash_name(value):
        result = 0xffffffff
        for c in value:
            result = (result * 33 + (c - 32 if 97 <= c <= 122 else c)) & 0xffffffff
        return result

    for trial in range(1024):
        memory = bytearray(rng.randbytes(0x10000))
        def put(address, *values):
            struct.pack_into('<' + 'I' * len(values), memory, address - ARENA, *values)
        strings = (b'UI_A.fng', b'ui_b.FNG', b'COLLISION', b'missing', b'\xffaz', b'\x80AZ', b'B!.fng', b'AB.fng', b'UI_B.fng', b'')
        for pointer, text in zip(NAMES, strings):
            memory[pointer - ARENA:pointer - ARENA + len(text) + 1] = text + b'\0'
        length = trial % 4
        for p, package in enumerate(PACKAGES):
            put(package + 4, PACKAGES[p + 1] if p + 1 < length else 0)
            name = 0 if p == 2 and trial % 2 == 0 else NAMES[p]
            name_index = p
            if p == 2 and trial % 11 == 3:
                name_index, name = 0, NAMES[0]
            if trial % 7 == 1 and p < 2:
                name_index = 6 + p
                name = NAMES[name_index]
            put(package + 0xc, name, hash_name(strings[name_index]))
            nodes = NODES[p]
            for n, object_ in enumerate(nodes):
                put(object_ + 4, 0)
                put(object_ + 0x10, rng.randrange(4))
                put(object_ + 0x18, (5, rng.choice((1, 2, 7)), 1, 5, 2, 1)[n])
                put(object_ + 0x64, 0)
            put(nodes[0] + 4, nodes[1])
            put(nodes[0] + 0x64, nodes[2])
            put(nodes[2] + 4, nodes[3])
            put(nodes[3] + 0x64, nodes[4])
            put(nodes[4] + 4, nodes[5])
            put(package + 0x4c, 0 if trial % 9 == p else nodes[0])
        if args.compose_scripts:
            memory[0x2a00:0x2a03] = b'go\0'
            limits = ((0, 0), (1, 0), (0xffffffff, 0), (0x80000000, 0x7fffffff),
                      (0x7fffffff, 0x80000000), (100, 100), (101, 100))
            for p, nodes in enumerate(NODES):
                for n, node in enumerate(nodes):
                    index = p * 6 + n
                    base = ARENA + 0x8000 + index * 0x100
                    track = ARENA + 0xa000 + index * 0x80
                    state_pointer = ARENA + 0xc000 + index * 0x40
                    put(node + 0x2c, state_pointer)
                    put(node + 0x4c, 0 if (trial + index) % 7 == 0 else base)
                    put(node + 0x54, 0 if (trial + index) % 5 == 0 else base + 0x40)
                    for j in range(3):
                        script = base + j * 0x40
                        put(script + 4, script + 0x40 if j + 1 < (trial + index) % 4 else 0)
                        time, duration = limits[(trial + index + j) % len(limits)]
                        put(script + 0xc, duration, time)
                        put(script + 0x1c, trial % 2, track)
                        put(script + 0x30, 0xffffffff if j == 0 else 0x535)
                    memory[track - ARENA] = 1
                    memory[track + 2 - ARENA] = 1  # real initialization clears flags; no float operation
                    memory[track + 7 - ARENA] = 0
        put(QUEUE + 0xe4, PACKAGES[0] if length else 0)
        put(QUEUE + 0xf4, 0 if trial % 4 == 1 else PACKAGES[trial % 3])
        put(ARCHIVE_NODE + 0x18, rng.choice((0, *PACKAGES)))
        if args.compose_archive:
            put(ARCHIVE_NODE + 4, ARCHIVE + 4, ARCHIVE + 4)
            put(ARCHIVE_NODE + 0x10, ARENA + 0x1900)
            put(ARENA + 0x1900, 0x30210, 0, hash_name(strings[trial % 10]))
        put(MANAGER + 8, QUEUE)
        put(VISITOR, VTABLE)
        put(VTABLE, STOP + 0x600, CALLBACK)
        hash_ = rng.randrange(6)
        put(QUERY, VTABLE, hash_, NODES[1][0])
        name = 0 if trial % 3 == 0 else NAMES[trial % 4]
        if trial % 11 == 3:
            name = NAMES[0]
        elif trial % 7 == 1:
            assert hash_name(strings[6]) == hash_name(strings[7])
            name = NAMES[7]
        elif trial % 13 == 2:
            name = NAMES[8]
        elif trial % 17 == 4:
            name = NAMES[9]
        fixture = {'archive_node': rng.choice((0, ARCHIVE_NODE)), 'mode': (0, 0, 1, 0x80000000, 0xffffffff)[trial // 5 % 5], 'disable_manager': trial % 11 == 0,
                   'format_variant': trial % 3, 'format_mutation': trial % 3 == 0, 'format_hash': (0x535, 0xffffffff, 0xdeadbeef)[trial % 3],
                   'mutation': trial % 4, 'stop_after': trial % 8, 'true_byte': (1, 2, 128, 255)[trial % 4]}
        manager = 0 if trial % 5 == 0 else MANAGER
        compare('QueryVisit', memory, [NODES[0][trial % 6]], QUERY, 4, True, manager, **fixture)
        compare('VisitChildren', memory, [NODES[0][0], VISITOR], PACKAGES[0], 8, True, manager, **fixture)
        compare('VisitObjects', memory, [VISITOR], PACKAGES[0], 4, True, manager, **fixture)
        compare('PackageFindObject', memory, [hash_], PACKAGES[0], 4, manager=manager, **fixture)
        compare_pair = [NAMES[1], NAMES[8]] if trial % 13 == 2 else [NAMES[trial % 10], NAMES[(trial + 1) % 10]]
        compare('CompareNames', memory, compare_pair, manager=manager, **fixture)
        compare('FindByName', memory, [name, PACKAGES[0] if length else 0], QUEUE + 0xdc, 8, manager=manager, **fixture)
        compare('QueueFindObject', memory, [hash_, name], QUEUE, 8, manager=manager, **fixture)
        for function in ('FngFindObject5379C0', 'FngFindStringByHash', 'FngFindImageByHash'):
            compare(function, memory, [name, hash_], manager=manager, **fixture)
        compare('cihash', memory, [name], manager=manager, **fixture)
        compare('FngFindPackage', memory, [name], manager=manager, **fixture)
        if args.compose_archive:
            compare('FindNode', memory, [name], ARCHIVE, 4, manager=manager, **fixture)
        if args.compose_scripts:
            script_name = 0 if trial % 7 == 0 else ARENA + 0x2a00
            script_hash = (0x535, 0xffffffff, 0xdeadbeef)[trial % 3]
            restart = (0, 1, 2, 128, 255, 0x100, 0x10001, 0xffffffff)[trial % 8]
            wrapper_name, wrapper_hash, wrapper_manager, wrapper_fixture = name, hash_, manager, fixture
            if restart == 0x100:
                # Break fixture correlations: a high-byte-only restart must reach an unchanged selection.
                wrapper_name, wrapper_manager = NAMES[0], MANAGER
                wrapper_fixture = dict(fixture, mode=0, disable_manager=False)
                wrapper_hash = struct.unpack_from('<I', memory, NODES[0][0] + 0x10 - ARENA)[0]
                put(PACKAGES[0] + 0xc, NAMES[0], hash_name(strings[0]))
                put(PACKAGES[0] + 0x4c, NODES[0][0])
                put(QUEUE + 0xe4, PACKAGES[0])
                put(NODES[0][0] + 0x4c, ARENA + 0x8000)
                put(NODES[0][0] + 0x54, ARENA + 0x8040)
                put(ARENA + 0x8004, ARENA + 0x8040)
                put(ARENA + 0x8044, 0)
                put(ARENA + 0x804c, 100, 100)
                script_name, script_hash = ARENA + 0x2a00, 0x535
            compare('FngSetScript', memory, [wrapper_name, wrapper_hash, script_name, restart], void_result=True, manager=wrapper_manager, **wrapper_fixture)
            compare('FngSetScriptHash', memory, [wrapper_name, wrapper_hash, script_hash, restart], void_result=True, manager=wrapper_manager, **wrapper_fixture)
            compare('FngScriptActive', memory, [wrapper_name, wrapper_hash, script_name], byte_result=True, manager=wrapper_manager, **wrapper_fixture)
            compare('FngScriptRunning', memory, [wrapper_name, wrapper_hash, script_hash], byte_result=True, manager=wrapper_manager, **wrapper_fixture)
        if trial % 16 == 0:
            put(MANAGER + 8, 0)
            compare('FngFindPackage', memory, [name], manager=MANAGER, **dict(fixture, mode=0))
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()),
              'fingerprints': functions, 'controlled_interfaces': (['visitor callback in direct walk fixtures'] if args.compose_archive else
                  ['FEngArchive::FindNode 0x51bb70', 'visitor callback in direct walk fixtures']),
              'composition': 'real tree/hash query, package locator, both live lists, typed wrappers, hashing and name comparison',
              'archive_composed': args.compose_archive, 'scripts_composed': args.compose_scripts, 'formatting_composed': args.compose_formatting,
              'seh': 'normal-return FS chain restoration verified; exceptions/unwind excluded', 'byte_matching': 'not established',
              'limitations': ['finite acyclic trees/package chains', 'valid non-null queue behind non-null manager',
                              'null requested name with a non-null empty-name package is excluded',
                              'native package membership and resource loading remain seams', 'MSVC pending with clang-cl']}
    if args.compose_formatting:
        report['controlled_interfaces'].extend(['formatter core 0x43e950', 'QueryPerformanceCounter IAT 0x78311c'])
    elif args.compose_scripts: report['controlled_interfaces'].append('FngFormatHash 0x50b9c0')
    if args.mutation_checks:
        source = args.source.read_text()
        mutations = {
            'continue-after-match': ('        return false;\n    }\n    return true;', '        return true;\n    }\n    return true;'),
            'skip-root-recursion': ('object->type18 == 5 && !VisitChildren(object, visitor)', 'object->type18 == 9 && !VisitChildren(object, visitor)'),
            'wrong-image-type': ('object && object->type18 == 1 ? object : 0', 'object && object->type18 == 2 ? object : 0'),
            'ignore-name-collision': ('hash == package->nameHash10 && !FEngComparePackageNames(package->nameC, name)', 'hash == package->nameHash10'),
            'skip-manager-fallback': ('if (!object && fengManager8384C4)', 'if (false)'),
            'retry-wrong-string-type': (
                'if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);\n    return object && object->type18 == 2',
                'if (fengManager8384C4 && (!object || object->type18 != 2)) object = fengManager8384C4->queue8->FindObject(hash, name);\n    return object && object->type18 == 2'),
        }
        rejected = []
        for name, (before, after) in mutations.items():
            assert before in source
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(source.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(path), '--locator-source', str(args.locator_source)] + (['--compose-formatting'] if args.compose_formatting else ['--compose-scripts'] if args.compose_scripts else ['--compose-archive'] if args.compose_archive else []), cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            rejected.append(name)
        locator = args.locator_source.read_text()
        locator_mutations = {
            'ignore-mode': ('!fengValue8384D0 && fengManager8384C4', 'fengManager8384C4'),
            'skip-secondary-list': ('return queue->packagesEC.FindByName(name, queue->packagesEC.first8);', 'return 0;'),
            'prefer-secondary-list': ('FEPackage *package = queue->packagesDC.FindByName(name, queue->packagesDC.first8);',
                                      'FEPackage *package = queue->packagesEC.FindByName(name, queue->packagesEC.first8);'),
            'archive-on-live-miss': ('return queue->packagesEC.FindByName(name, queue->packagesEC.first8);',
                                     'FEPackage *secondary = queue->packagesEC.FindByName(name, queue->packagesEC.first8);\n        if (secondary) return secondary;'),
        }
        for name, (before, after) in locator_mutations.items():
            assert before in locator
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(locator.replace(before, after))
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                     '--source', str(args.source), '--locator-source', str(path)] + (['--compose-formatting'] if args.compose_formatting else ['--compose-scripts'] if args.compose_scripts else ['--compose-archive'] if args.compose_archive else []), cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            rejected.append(name)
        if args.compose_scripts:
            wrappers = args.script_lookup_source.read_text()
            wrapper_mutations = {
                'skip-script-fallback': ('if (!object && fengManager8384C4)', 'if (false)'),
                'exclusive-script-finish': ('->time10 <= object->currentScript54->lengthC', '->time10 < object->currentScript54->lengthC'),
                'unsigned-script-time': ('object->currentScript54->time10 <= object->currentScript54->lengthC',
                                         '(unsigned int)object->currentScript54->time10 <= (unsigned int)object->currentScript54->lengthC'),
                'skip-format-on-miss': ('unsigned int scriptHash = FngFormatHash(script);',
                                        'unsigned int scriptHash = object ? FngFormatHash(script) : 0;'),
                'cache-selection-before-format': (
                    'unsigned int scriptHash = FngFormatHash(script);\n    return object && object->currentScript54 && object->currentScript54->hash30 == scriptHash &&\n           object->currentScript54->time10 <= object->currentScript54->lengthC;',
                    'FEScript *selected = object ? object->currentScript54 : 0;\n    unsigned int scriptHash = FngFormatHash(script);\n    return selected && selected->hash30 == scriptHash && selected->time10 <= selected->lengthC;'),
                'skip-null-selector': ('FngSetObjectScriptHash(object, script, (unsigned char)restart);',
                                       'if (object) FngSetObjectScriptHash(object, script, (unsigned char)restart);'),
                'full-word-restart': ('(unsigned char)restart', '(restart ? 1 : 0)'),
            }
            for name, (before, after) in wrapper_mutations.items():
                assert before in wrappers
                path = BUILD / ('mutation-' + name + '.cpp')
                path.write_text(wrappers.replace(before, after))
                result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                                         '--compose-formatting' if args.compose_formatting else '--compose-scripts', '--script-lookup-source', str(path)], cwd=ROOT, capture_output=True, text=True)
                (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
                assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
                rejected.append(name)
        report['rejected_mutations'] = rejected
    report_name = 'formatting-composition.json' if args.compose_formatting else 'script-composition.json' if args.compose_scripts else 'archive-composition.json' if args.compose_archive else 'verification.json'
    (BUILD / report_name).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
