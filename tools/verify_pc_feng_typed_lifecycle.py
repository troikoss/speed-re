#!/usr/bin/env python3
"""Compare original image/group/list cleanup with actual base, pool, queue and storage callees."""
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
BUILD = ROOT / 'build/analysis/feng-typed-lifecycle'
ARENA, SELF, LIST, OBS, VT, EXTRA, CLASS, BLOCK, DATA, STACK, STOP = (0x2000000 + x for x in (0, 0x100, 0x400, 0x500, 0x600, 0x3000, 0x4000, 0x5000, 0x8000, 0x1f000, 0x100000))
MANAGER, OBSERVER, TEB, GDT = 0x863770, 0x838474, 0x2120000, 0x2130000
DELETE, DELETE_ALT, NOTICE = STOP + 0x200, STOP + 0x240, STOP + 0x100
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ClearOwned': '?ClearOwned@FEngQueueList@', 'ReleaseSlot': '?ReleaseSlot@FEngStatePoolClass@',
            'ReleaseState': '?ReleaseState@FEngStatePoolManager@', 'DestructBase': '?DestructBase@HudVisibilityObject@',
            'DestructGrid': '?Destruct@FEGridObject@', 'Remove': '?Remove@FEngQueueList@'}
PREFIXES.update(DestructImage='?DestructImage@FEngLeafObject@', DestroyImage='?DestroyImage@FEngLeafObject@',
                DestructGroup='?Destruct@FEngGroupObject@', DestroyGroup='?Destroy@FEngGroupObject@',
                DestructList='?Destruct@FEListObject@', DestroyList='?Destroy@FEListObject@',
                ReleaseStorage='?ReleaseStorage@FEListObject@', ReleaseCells='?ReleaseCells@FEListObject@', DestructBlock='?Destruct@FEngStatePoolBlock@',
                DestroyBlock='?Destroy@FEngStatePoolBlock@', DestructClass='?Destruct@FEngStatePoolClass@',
                DestroyClass='?Destroy@FEngStatePoolClass@')
ARGS = {'ClearOwned': 0, 'ReleaseSlot': 1, 'ReleaseState': 1, 'DestructBase': 0, 'DestructGrid': 0, 'Remove': 1}
ARGS.update(DestructImage=0, DestroyImage=1, DestructGroup=0, DestroyGroup=1, DestructList=0, DestroyList=1, ReleaseStorage=0, ReleaseCells=0, DestructBlock=0, DestroyBlock=1, DestructClass=0, DestroyClass=1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngTypedLifecycle.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    parser.add_argument('--cases', type=int, default=2048)
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-typed-lifecycle.json').read_text())
    manifest['data'] = {}
    for dependency in ('feng-lifecycle', 'feng-state-pool'):
        dep = json.loads((ROOT / ('config/SPEED2_EXE_1_2/' + dependency + '.json')).read_text())
        manifest['functions'].update(dep['functions']); manifest['data'].update(dep.get('data', {}))
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()) + list(manifest['tables'].values()):
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source,) + tuple(ROOT / ('src/Speed/Indep/Src/Frontend/' + n + '.cpp') for n in ('FEngLifecycle', 'FEngQueueNodes', 'FEngStatePool')):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
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
    markers = {0x532666: 'image-base-jump', 0x534c1e: 'group-child-clear', 0x534a7b: 'list-storage-release', 0x516e07: 'storage-cells-jump', 0x504319: 'cell-owned-free', 0x50433e: 'cells-free', 0x534de0: 'image-delete', 0x534be0: 'group-delete', 0x541da0: 'list-delete'}
    coverage = dict.fromkeys(markers.values(), 0)
    helper_hits = dict.fromkeys(PREFIXES, 0)
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
        if rebuilt:
            put(uc, 0x79b13c, entries['DestroyBlock']); put(uc, 0x79b8dc, entries['DestroyClass'])
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            if address in helpers:
                name = helpers[address]
                if original: helper_hits[name] += 1
                state['trace'].append((name, this, words(uc, sp + 4, ARGS[name]), snapshot(uc)))
                if name == 'ClearOwned': state['list'] = this
            if address not in (0x575630, DELETE, DELETE_ALT, NOTICE):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('free' if address == 0x575630 else 'notice' if address == NOTICE else 'delete', address, this if address != 0x575630 else None, argument, snapshot(uc)))
            if address == 0x575630:
                if not state['freed']:
                    state['freed'] = True
                    if mode & 1: put(uc, SELF + 0x90, ARENA + 0x6800)
                    if mode & 2: put(uc, SELF + 0x64, 0)
                    if mode & 32: put(uc, SELF + 0x7c, ARENA + 0x17100)
                    if mode & 4: put(uc, OBSERVER, 0)
                    if mode & 8: put(uc, OBSERVER, OBS)
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
    counts = dict.fromkeys(('DestructImage', 'DestroyImage', 'DestructGroup', 'DestroyGroup', 'DestructList', 'DestroyList', 'ReleaseStorage', 'ReleaseCells'), 0)
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
            result = uc.reg_read(xc.UC_X86_REG_EAX) if name.startswith('Destroy') else None
            outputs.append((result, snapshot(uc), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
              [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:20],
              [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
        counts[name] += 1
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 8) % 256
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        def node_list(address, start, count):
            memory[address - ARENA:address - ARENA + 16] = struct.pack('<IIII', 0, count, start if count else 0, start + (count - 1) * 0x40 if count else 0)
            for i in range(count): memory[start - ARENA + i * 0x40:start - ARENA + i * 0x40 + 12] = struct.pack('<III', VT, start + (i + 1) * 0x40 if i + 1 < count else 0, start + (i - 1) * 0x40 if i else 0)
        node_list(SELF + 0x44, ARENA + 0x1000, trial % 4)
        node_list(SELF + 0x34, ARENA + 0x2000, (trial // 4) % 4)
        putm(VT, DELETE); putm(VT + 0x10, DELETE_ALT); putm(VT + 0x20, NOTICE); putm(OBS, VT + 0x20)
        for ci in range(2):
            c = CLASS + ci * 0x100; b = BLOCK + ci * 0x100; stride = 1 + (trial // 16) % 64
            memory[c - ARENA:c - ARENA + 32] = struct.pack('<8I', 0x79b8dc, CLASS + 0x100 if ci == 0 else 0, CLASS if ci else 0, 0, 1, b, b, stride)
            used = (0, 1, 2, 32, 65535)[(trial // 7) % 5]
            memory[b - ARENA:b - ARENA + 24] = struct.pack('<IIIHH4sI', 0x79b13c, 0, 0, stride, used, rng.randbytes(4), DATA + ci * 0x4000)
        stride = 1 + (trial // 16) % 64; slot = (trial // 3) % 32
        pointer = DATA + 0x4000 + slot * stride + trial % stride
        manager = struct.pack('<4I', 0, 2, CLASS, CLASS + 0x100)
        putm(SELF + 0x2c, 0 if trial % 11 == 0 else pointer)
        for offset in (0x14, 0x8c, 0xa0, 0x9c): putm(SELF + offset, ARENA + 0x15000 + offset if (trial >> (offset % 7)) & 1 else 0)
        observer = OBS if trial % 3 else 0
        putm(SELF + 0x5c, (rng.getrandbits(32) & ~1) | ((trial // 8) % 2))
        columns = (0, 1, 2, 3, 0x10000, 1, 2, 0xffffffff)[trial % 8]
        rows = (3, 0, 2, 3, 0x10000, 1, 3, 0)[trial % 8]
        putm(SELF + 0x60, columns); putm(SELF + 0x64, rows)
        putm(SELF + 0x78, 0 if trial % 5 == 0 else ARENA + 0x14000)
        putm(SELF + 0x7c, 0 if trial % 7 == 0 else ARENA + 0x14100)
        putm(SELF + 0x90, 0 if not (columns * rows & 0xffffffff) and trial % 3 else ARENA + 0x6000)
        for base in (ARENA + 0x6000, ARENA + 0x6800):
            for ci in range(12):
                cell = base + ci * 0x30
                putm(cell + 0x18, (0, 1, 2, 3, 2, 2)[(trial + ci) % 6])
                putm(cell + 0x20, 0 if (trial + ci) % 4 == 0 else ARENA + 0x18000 + ci * 0x20)
        typed = memory
        memory = bytearray(memory)
        node_list(SELF + 0x5c, ARENA + 0x2800, (trial // 16) % 4)
        group = memory
        memory = typed
        flags = (0, 1, 2, 3, 0x100, 0x101, 0x80000000, 0xffffffff)[trial % 8]
        for name in counts:
            compare(name, group if 'Group' in name else typed, manager, observer, SELF,
                    [flags] if name.startswith('Destroy') else [], mode)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'actual_callees': {name: hits for name, hits in helper_hits.items() if hits},
              'controlled_interfaces': ['free 0x575630', 'observer virtual slot0', 'synthetic script/response node deleting destructor slot0'],
              'limitations': ['normal return only; no SEH unwind/exception callback coverage', 'finite lists/pools and bounded callback mutations, no arbitrary aliases/concurrency or callback inspection of SEH frames',
                              'native state/script/response ownership still requires adapter; MSVC 7.1/byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'image-wrong-vtable': [('= 0x79ba0c;', '= 0x79bae4;')],
            'image-omit-base': [('    ((HudVisibilityObject *)this)->DestructBase();', '    /* omitted */')],
            'group-omit-clear': [('    children5C.ClearOwned();', '    /* omitted */')],
            'group-rewrite-own-vtable': [('    children5C.value0 = 0x79a9ec;', '    *(unsigned int *)unidentified0 = 0x79b8e8;\n    children5C.value0 = 0x79a9ec;')],
            'list-wrong-vtable': [('= 0x79bae4;', '= 0x79ba0c;')],
            'storage-ignore-owned-flag': [('if (value5C & 1)', 'if (true)')],
            'storage-omit-flag-clear': [('        value5C &= ~1u;', '        /* omitted */')],
            'storage-cells-first': [('        value5C &= ~1u;', '        ReleaseCells();\n        value5C &= ~1u;')],
            'storage-omit-column-reset': [('            count60 = 0;', '            /* omitted */')],
            'storage-omit-row-free': [('            FEngFreeQueueNode(value7C);', '            /* omitted */')],
            'cells-wrong-type': [('== 2 &&', '== 1 &&')],
            'cells-zero-product-free': [('    if (count) {', '    if (!count) { FEngFreeQueueNode(value90); value90 = 0; }\n    if (count) {')],
            'cells-cache-base': [('        unsigned int offset = 0;', '        void *cached = value90;\n        unsigned int offset = 0;'), ('(unsigned char *)value90 + offset', '(unsigned char *)cached + offset')],
            'cells-omit-pointer-reset': [('        value90 = 0;', '        /* omitted */')],
            'wrong-delete-flag': [('if (flags & 1)', 'if (flags & 2)')],
            'wrong-return': [('    return this;', '    return 0;')],
        }
        for name, edits in mutations.items():
            changed = source
            for before, after in edits:
                assert before in changed, name
                changed = changed.replace(before, after)
            path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(changed)
            result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler, '--source', str(path), '--cases', '256'], cwd=ROOT, capture_output=True, text=True)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
            report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
