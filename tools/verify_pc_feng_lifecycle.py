#!/usr/bin/env python3
"""Compare normal-return FEng cleanup with actual pool release and list unlink callees."""
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
BUILD = ROOT / 'build/analysis/feng-lifecycle'
ARENA, SELF, LIST, OBS, VT, EXTRA, CLASS, BLOCK, DATA, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x400, 0x500, 0x600, 0x3000, 0x4000, 0x5000, 0x8000, 0x1f000, 0x100000))
MANAGER, OBSERVER, TEB, GDT = 0x863770, 0x838474, 0x2120000, 0x2130000
DELETE, DELETE_ALT, NOTICE = STOP + 0x200, STOP + 0x240, STOP + 0x100
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ClearOwned': '?ClearOwned@FEngQueueList@', 'ReleaseSlot': '?ReleaseSlot@FEngStatePoolClass@',
            'ReleaseState': '?ReleaseState@FEngStatePoolManager@', 'DestructBase': '?DestructBase@HudVisibilityObject@',
            'DestructGrid': '?Destruct@FEGridObject@', 'Remove': '?Remove@FEngQueueList@'}
ARGS = {'ClearOwned': 0, 'ReleaseSlot': 1, 'ReleaseState': 1, 'DestructBase': 0, 'DestructGrid': 0, 'Remove': 1}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngLifecycle.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()):
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for s in read_object(obj)['symbols'].values():
            if s['section'] != 0: continue
            if s['name'].startswith(('?FEngFreeQueueNode@', '?FEngGridFreeCells@')): externals[s['name']] = 0x575630
            elif s['name'].startswith('?FEngAllocateQueueNode@'): externals[s['name']] = 0x575620
            elif s['name'].startswith('?FEngObjectDestructionObserver@'): externals[s['name']] = OBSERVER
            elif s['name'].startswith('?FEngObjectStateManager@'): externals[s['name']] = MANAGER
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    state, machines = {}, []
    def words(uc, a, n=1): return struct.unpack('<' + 'I' * n, uc.mem_read(a, n * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x10000)) + bytes(uc.mem_read(MANAGER, 16)) + bytes(uc.mem_read(OBSERVER, 4))
    markers = {0x51167c: 'first-remove-call', 0x5116a1: 'later-inline-unlink', 0x5174cb: 'pool-miss',
               0x5174db: 'pool-slot-divide', 0x517513: 'block-delete', 0x51755c: 'class-delete',
               0x52afc8: 'observer', 0x52afe0: 'payload-free', 0x52bad6: 'cells-free', 0x52bae9: 'pool-data-free', 0x52bafc: 'pool-table-free'}
    coverage = dict.fromkeys(markers.values(), 0)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000); uc.mem_map(TEB, 0x1000); uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)): uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, this, words(uc, sp + 4, ARGS[name]), snapshot(uc)))
                if name == 'ClearOwned': state['list'] = this
            if address not in (0x575630, DELETE, DELETE_ALT, NOTICE):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('free' if address == 0x575630 else 'notice' if address == NOTICE else 'delete', address, this if address != 0x575630 else None, argument, snapshot(uc)))
            if address == 0x575630:
                if not state['freed']:
                    state['freed'] = True
                    if mode & 1: put(uc, SELF + 0xa0, ARENA + 0x17000)
                    if mode & 2: put(uc, SELF + 0x9c, ARENA + 0x17100)
                    if mode & 4: put(uc, OBSERVER, 0)
                cleanup = 0
            elif address == NOTICE:
                assert this == OBS and argument == SELF
                if mode & 16: put(uc, SELF + 0x2c, 0)
                if mode & 32: put(uc, SELF + 0x14, ARENA + 0x17200)
                if mode & 64:
                    for offset in (0x34, 0x44):
                        for field in (4, 8, 12): put(uc, SELF + offset + field, 0)
                cleanup = 4
            else:
                assert argument == 1
                if BLOCK <= this < BLOCK + 0x400:
                    if mode & 8: put(uc, CLASS + 0x100 + 0x10, 1)
                    if mode & 32: put(uc, SELF + 0x14, ARENA + 0x17300)
                if ARENA + 0x1000 <= this < ARENA + 0x3100:
                    current = state['list']
                    if mode & 1 and not state['inserted']:
                        state['inserted'] = True
                        first = words(uc, current + 8)[0]
                        uc.mem_write(EXTRA, struct.pack('<III', VT + 0x10, first, 0))
                        if first: put(uc, first + 8, EXTRA)
                        put(uc, current + 8, EXTRA)
                        if not words(uc, current + 12)[0]: put(uc, current + 12, EXTRA)
                        put(uc, current + 4, words(uc, current + 4)[0] + 1)
                    if mode & 128:
                        first = words(uc, current + 8)[0]
                        if first: put(uc, first, VT + 0x10)
                cleanup = 4
            uc.reg_write(xc.UC_X86_REG_EAX, 0xa5a500ff); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(('ClearOwned', 'ReleaseSlot', 'ReleaseState', 'DestructBase', 'DestructGrid'), 0)
    rng = random.Random(0x52baa0)
    def compare(name, memory, manager, observer, this, arguments, mode):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(MANAGER, manager); put(uc, OBSERVER, observer); put(uc, TEB, 0xffffffff)
            uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.reg_write(xc.UC_X86_REG_ECX, this); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, freed=False, inserted=False, list=LIST)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                if not rebuilt: raise AssertionError(('original fixture fault', trial, name, str(error))) from error
                raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(arguments)
            assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
            assert words(uc, TEB)[0] == 0xffffffff
            result = uc.reg_read(xc.UC_X86_REG_EAX) & 255 if name == 'ReleaseSlot' else None
            outputs.append((result, snapshot(uc), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
              [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:20],
              [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
        counts[name] += 1
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 16) % 256
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def node_list(address, start, count):
            memory[address - ARENA:address - ARENA + 16] = struct.pack('<IIII', 0, count, start if count else 0, start + (count - 1) * 0x40 if count else 0)
            for i in range(count): memory[start - ARENA + i * 0x40:start - ARENA + i * 0x40 + 12] = struct.pack('<III', VT, start + (i + 1) * 0x40 if i + 1 < count else 0, start + (i - 1) * 0x40 if i else 0)
        node_list(LIST, ARENA + 0x1000, trial % 4)
        node_list(SELF + 0x44, ARENA + 0x1000, trial % 4)
        node_list(SELF + 0x34, ARENA + 0x2000, (trial // 4) % 4)
        putm(VT, DELETE); putm(VT + 0x10, DELETE_ALT); putm(VT + 0x20, NOTICE); putm(OBS, VT + 0x20)
        for ci in range(2):
            c = CLASS + ci * 0x100; b = BLOCK + ci * 0x100; stride = 1 + (trial // 16) % 64
            memory[c - ARENA:c - ARENA + 32] = struct.pack('<8I', VT, CLASS + 0x100 if ci == 0 else 0, CLASS if ci else 0, 0, 1, b, b, stride)
            used = (0, 1, 2, 32, 65535)[(trial // 7) % 5]
            memory[b - ARENA:b - ARENA + 24] = struct.pack('<IIIHH4sI', VT, 0, 0, stride, used, rng.randbytes(4), DATA + ci * 0x4000)
        stride = 1 + (trial // 16) % 64; slot = (trial // 3) % 32
        pointer = DATA + 0x4000 + slot * stride + trial % stride
        manager = struct.pack('<4I', 0, 2, CLASS, CLASS + 0x100)
        putm(SELF + 0x2c, 0 if trial % 11 == 0 else pointer)
        for offset in (0x14, 0x8c, 0xa0, 0x9c): putm(SELF + offset, ARENA + 0x15000 + offset if (trial >> (offset % 7)) & 1 else 0)
        observer = OBS if trial % 3 else 0
        compare('ClearOwned', memory, manager, observer, LIST, [], mode)
        compare('ReleaseSlot', memory, manager, observer, CLASS + (trial % 2) * 0x100, [pointer], mode)
        compare('ReleaseState', memory, manager, observer, MANAGER, [0 if trial % 11 == 0 else pointer], mode)
        compare('DestructBase', memory, manager, observer, SELF, [], mode)
        compare('DestructGrid', memory, manager, observer, SELF, [], mode)
        if trial % 64 == 0:
            for edge in (DATA + 0x4000 - 1, DATA + 0x4000 + stride * 32):
                compare('ReleaseSlot', memory, manager, observer, CLASS + 0x100, [edge], mode)
            empty = bytearray(memory)
            struct.pack_into('<3I', empty, CLASS + 0x110 - ARENA, 0, 0, 0)
            compare('ReleaseSlot', empty, manager, observer, CLASS + 0x100, [pointer], mode)
            wrapped = bytearray(memory)
            struct.pack_into('<I', wrapped, BLOCK + 0x114 - ARENA, 0xfffffff0)
            compare('ReleaseSlot', wrapped, manager, observer, CLASS + 0x100, [0xfffffff1], mode)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'actual_callees': ['Remove', 'ReleaseSlot', 'ReleaseState', 'ClearOwned', 'DestructBase'],
              'controlled_interfaces': ['free 0x575630', 'observer virtual slot0', 'node deleting destructor slot0'],
              'limitations': ['normal return only; no SEH unwind/exception callback coverage', 'finite lists/pools and bounded callback mutations, no arbitrary aliases/concurrency',
                              'native production lifecycle/owner binding and MSVC 7.1/byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'clear-extra-remove-call': [('                first8 = node->next4;', '                Remove(node);\n                first8 = node->next4;')],
            'clear-omit-node-delete': [('if (node) ((FEngDeletingDestructor *)node->vtable0)[0](node, 1);', 'if (false) ((FEngDeletingDestructor *)node->vtable0)[0](node, 1);')],
            'clear-skip-reinserted-head': [('            node = first8;', '            node = 0;')],
            'pool-wrong-range-end': [('block->strideC * 32', 'block->strideC * 31')],
            'pool-inclusive-end': [('address < base + (unsigned int)block->strideC * 32', 'address <= base + (unsigned int)block->strideC * 32')],
            'pool-nonwrapping-range': [('address < base + (unsigned int)block->strideC * 32', '(unsigned long long)address < (unsigned long long)base + (unsigned int)block->strideC * 32')],
            'pool-wrong-slot-divisor': [('/ block->strideC;', '/ (block->strideC + 1);')],
            'pool-wrong-bitmap-bit': [('1u << (slot & 7)', '1u << ((slot + 1) & 7)')],
            'pool-increment-used': [('--block->usedE;', '++block->usedE;')],
            'pool-skip-block-delete': [('((FEngDeletingDestructor *)block->vtable0)[0](block, 1);', '(void)block;')],
            'manager-skip-class-delete': [('((FEngDeletingDestructor *)node->vtable0)[0](node, 1);\n        }', '(void)node;\n        }')],
            'base-skip-observer': [('if (FEngObjectDestructionObserver)', 'if (false)')],
            'base-wrong-list-order': [('this + 0x44', 'this + 0x34')],
            'base-omit-final-vtable': [('= 0x79a9e8;', '= 0x79b8e0;')],
            'grid-skip-cells-free': [('if (cells8C) FEngGridFreeCells(cells8C);', '(void)cells8C;')],
            'grid-cache-pool-pointers': [('    if (cells8C) FEngGridFreeCells(cells8C);', '    unsigned int data = valueA0; void *table = value9C;\n    if (cells8C) FEngGridFreeCells(cells8C);'), ('if (valueA0) FEngGridFreeCells((void *)valueA0);', 'if (data) FEngGridFreeCells((void *)data);'), ('if (value9C) FEngGridFreeCells(value9C);', 'if (table) FEngGridFreeCells(table);')],
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
