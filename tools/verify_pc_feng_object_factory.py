#!/usr/bin/env python3
"""Compare all UG2 object factory branches with actual constructors and state-pool callees."""
import argparse, hashlib, json, os, random, struct, subprocess
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-object-factory'
ARENA, CLASS, BLOCK, NEW_CLASS, NEW_BLOCK, STACK, STOP = (0x2000000 + x for x in (0, 0x1000, 0x3000, 0x6000, 0x7000, 0x1f000, 0x100000))
DATA, NEW_DATA, MANAGER, OBSERVER, TEB, GDT = 0x4000000, 0x4600000, 0x863770, 0x838474, 0x2120000, 0x2130000
OBJECT, CONTEXT, SPECS, SPECS_ALT, COUNTER, DEFAULT = ARENA + 0x800, ARENA + 0x400, ARENA + 0x8000, ARENA + 0x8100, 0x7f7cf8, 0x7f7cc4
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ConstructBlock': '?Construct@FEngStatePoolBlock@', 'AcquireSlot': '?AcquireSlot@FEngStatePoolBlock@',
            'AcquireBlock': '?Acquire@FEngStatePoolClass@', 'AcquireState': '?AcquireState@FEngStatePoolManager@',
            'DestructBlock': '?Destruct@FEngStatePoolBlock@', 'DestroyBlock': '?Destroy@FEngStatePoolBlock@',
            'DestructClass': '?Destruct@FEngStatePoolClass@', 'DestroyClass': '?Destroy@FEngStatePoolClass@',
            'ReleaseSlot': '?ReleaseSlot@FEngStatePoolClass@', 'ReleaseState': '?ReleaseState@FEngStatePoolManager@',
            'ClearOwned': '?ClearOwned@FEngQueueList@', 'Remove': '?Remove@FEngQueueList@'}
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(ConstructBlock=1, AcquireState=1, DestroyBlock=1, DestroyClass=1, ReleaseSlot=1, ReleaseState=1, Remove=1)
PREFIXES.update(ConstructImage='?ConstructImage@FEngLeafObject@', ConstructString='?Construct@FEStringObject@',
                ConstructList='?Construct@FEListObject@', ConstructGroup='?Construct@FEngGroupObject@',
                ConstructType7='?ConstructType7@FEngLeafObject@', ConstructType9='?ConstructType9@FEngLeafObject@',
                ConstructType10='?ConstructType10@FEngLeafObject@', ConstructType11='?Construct@FEngType11Object@',
                ConstructText='?Construct@FEngTextStorage@', Create='?Create@FEngObjectFactoryContext@',
                ConstructBase='?ConstructBase@HudVisibilityObject@', ConstructGrid='?Construct@FEGridObject@')
ARGS.update(dict.fromkeys(('ConstructImage', 'ConstructString', 'ConstructList', 'ConstructGroup', 'ConstructType7',
                          'ConstructType9', 'ConstructType10', 'ConstructType11', 'ConstructText', 'ConstructBase', 'ConstructGrid'), 0))
ARGS['Create'] = 1
ROOTS = ('ConstructImage', 'ConstructString', 'ConstructList', 'ConstructGroup', 'ConstructType7', 'ConstructType9', 'ConstructType10', 'ConstructType11', 'ConstructText', 'Create')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngObjectFactory.cpp')
    parser.add_argument('--constructor-source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngObjectConstructors.cpp')
    parser.add_argument('--mutation-checks', action='store_true'); args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-object-factory.json').read_text())
    manifest['data'] = {}
    for dependency in ('feng-construct.json', 'feng-state-pool.json', 'feng-lifecycle.json'):
        dep = json.loads((ROOT / 'config/SPEED2_EXE_1_2' / dependency).read_text())
        manifest['functions'].update(dep['functions']); manifest['data'].update(dep.get('data', {})); manifest['tables'].update(dep.get('tables', {}))
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()) + list(manifest['tables'].values()):
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.constructor_source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngConstruct.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngStatePool.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngLifecycle.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {}
    for obj in objects:
        for sym in read_object(obj)['symbols'].values():
            if sym['section'] != 0: continue
            if sym['name'].startswith('?FEngAllocateQueueNode@'): externals[sym['name']] = 0x575620
            elif sym['name'].startswith(('?FEngFreeQueueNode@', '?FEngGridFreeCells@')): externals[sym['name']] = 0x575630
            elif sym['name'].startswith('?FEngObjectStateManager@'): externals[sym['name']] = MANAGER
            elif sym['name'].startswith('?FEngObjectDestructionObserver@'): externals[sym['name']] = OBSERVER
            elif sym['name'].startswith('?FEngNextObjectGuid@'): externals[sym['name']] = COUNTER
            elif sym['name'].startswith('?FEngValue7F7CC4@'): externals[sym['name']] = DEFAULT
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    markers = {int(manifest['functions'][n]['address'], 16): n for n in ROOTS}
    coverage = dict.fromkeys(markers.values(), 0); machines = []; state = {}
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x10000)) + bytes(uc.mem_read(MANAGER, 16)) + bytes(uc.mem_read(COUNTER, 4)) + bytes(uc.mem_read(DEFAULT, 4))
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000); uc.mem_map(DATA, 0x1000000)
        uc.mem_map(TEB, 0x1000); uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)): uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        if rebuilt:
            # Relocate only dispatch slots to the actual rebuilt virtual methods.
            put(uc, 0x79b13c, entries['DestroyBlock']); put(uc, 0x79b8dc, entries['DestroyClass'])
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            if address in helpers:
                name = helpers[address]
                state['trace'].append((name, this, words(uc, sp + 4, ARGS[name]), snapshot(uc)))
                if name == 'ConstructBlock': state['pending_data'] = True; state['block'] = this
                if name in ('AcquireBlock', 'ReleaseSlot'): state['class'] = this
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if address == 0x575620:
                if state['root'] == 'Create' and not state['object_allocated']:
                    state['object_allocated'] = True; result = OBJECT
                    if mode & 1: put(uc, CONTEXT + 0x30, 3); put(uc, CONTEXT + 0x34, SPECS_ALT)
                    if mode & 2: put(uc, CONTEXT + 0x38, ARENA + 0x9200)
                    if mode & 4: put(uc, COUNTER, 0xffffffff)
                    if mode & 32:
                        uc.mem_write(OBJECT + 0x22, b'\x34\x12'); put(uc, OBJECT + 0x54, 0xcafebabe)
                elif state['pending_data']:
                    state['pending_data'] = False; result = NEW_DATA
                elif argument == 0x18:
                    result = NEW_BLOCK
                    if mode & 8: put(uc, CONTEXT + 0x34, SPECS_ALT)
                else:
                    assert argument == 0x20
                    result = NEW_CLASS
                    if mode & 8: put(uc, CONTEXT + 0x34, SPECS_ALT)
                if state['root'] == 'Create' and mode & 16 and result != OBJECT:
                    put(uc, OBJECT + 0x2c, DATA + 0x1234)
            else: result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        def payload_write(uc, access, address, size, value, user):
            if DATA <= address < DATA + 0x1000000:
                state['payload_writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, hook); uc.hook_add(UC_HOOK_MEM_WRITE, payload_write); machines.append(uc)
    counts = dict.fromkeys(ROOTS, 0); rng = random.Random(0x534df0)
    def compare(name, memory, manager, this, arguments, mode):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(MANAGER, manager); put(uc, OBSERVER, 0); put(uc, TEB, 0xffffffff)
            put(uc, COUNTER, (0, 1, 0xffffffff, 0xfffffffe, trial)[trial % 5]); put(uc, DEFAULT, (trial * 0x1234567) & 0xffffffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], root=name, object_allocated=False, mode=mode, pending_data=False, block=BLOCK, payload_writes=[], **{'class': CLASS})
            def run(target, self_, argv):
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(argv), *[a & 0xffffffff for a in argv]))
                uc.reg_write(xc.UC_X86_REG_ECX, self_); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                address = entries[target] if rebuilt else int(manifest['functions'][target]['address'], 16)
                try: uc.emu_start(address, STOP, count=100000)
                except UcError as error:
                    if not rebuilt: raise AssertionError(('original fixture fault', trial, name, str(error))) from error
                    raise AssertionError(('state or call order mismatch', name, 'rebuilt fault', str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (name, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(argv)
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                assert not state['payload_writes'], ('state or call order mismatch', name, 'invented slot-payload writes', state['payload_writes'][:5])
                return uc.reg_read(xc.UC_X86_REG_EAX)
            result = run(name, this, arguments)
            outputs.append((result, snapshot(uc), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
              [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:20],
              [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
        counts[name] += 1
    types = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0xffffffff, 0x7fffffff, 0x80000000)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 16) % 64; type_ = types[trial % len(types)]
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        for ci in range(2):
            c = CLASS + ci * 0x100; b = BLOCK + ci * 0x100; stride = (64, 80)[ci]
            count = 0 if trial % 4 == 0 else 1
            memory[c - ARENA:c - ARENA + 32] = struct.pack('<8I', 0x79b8dc, CLASS + 0x100 if ci == 0 else 0, CLASS if ci else 0,
                0x79a9ec, count, b if count else 0, b if count else 0, stride)
            bitmap = 0xffffffff if trial % 3 == 0 else rng.getrandbits(32) & ~(1 << (trial % 32))
            memory[b - ARENA:b - ARENA + 24] = struct.pack('<IIIHHII', 0x79b13c, 0, 0, stride, bitmap.bit_count(), bitmap, DATA + ci * 0x200000)
        manager = struct.pack('<4I', 0x79a9ec, 0 if trial % 5 == 0 else 2, 0 if trial % 5 == 0 else CLASS, 0 if trial % 5 == 0 else CLASS + 0x100)
        count = trial % 7
        putm(CONTEXT + 0x30, count); putm(CONTEXT + 0x34, SPECS if count else 0); putm(CONTEXT + 0x38, ARENA + 0x9000)
        for table, bias in ((SPECS, 0), (SPECS_ALT, 1)):
            for i in range(8):
                key = type_ if i % 3 == 0 else types[(trial + i) % len(types)]
                stride = (0, 16, 64, 80, 0x10040)[(trial // 7 + i + bias) % 5]
                struct.pack_into('<II', memory, table + i * 8 - ARENA, key, stride)
        if trial < 2048:
            for name in ROOTS[:-1]: compare(name, memory, manager, OBJECT, [], 0)
        compare('Create', memory, manager, CONTEXT, [type_], mode)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'actual_callees': ['all constructor branches including base/grid and nested text storage', 'state manager release/acquire and pool constructors/claim'],
              'controlled_interfaces': ['allocation/free only; concrete state-pool virtual slots relocated to actual rebuilt wrappers'],
              'limitations': ['normal return only; compiler SEH unwind and chain-observing callbacks unverified', 'finite dirty-prefix/spec-table/allocator cases; invalid/OOM/concurrency/arbitrary aliases unverified',
                              'native wrapper allocation/state/reader integration remains a platform seam', 'MSVC 7.1 and byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        report['rejected_mutations'] = []
        factory_mutations = {
            'wrong-image-size': [('FEngAllocateQueueNode(0x60)', 'FEngAllocateQueueNode(0x64)')],
            'wrong-grid-owner': [('owner5C = owner38;', 'owner5C = 0;')],
            'wrong-default-type': [('object->type18 = type;', 'object->type18 = 3;')],
            'last-spec-match': [('stride = specs[i].stride4; break;', 'stride = specs[i].stride4;')],
            'skip-state-release': [('FEngObjectStateManager.ReleaseState(object->data2C);', '(void)object;')],
            'skip-state-allocation': [('void *state = FEngObjectStateManager.AcquireState(stride);', 'void *state = 0;')],
            'wrong-object-state-key': [('object->unidentified30 = stride;', 'object->unidentified30 = 0;')],
            'wrong-zero-type': [('case 0: return 0;', 'case 0: type = 3;')],
        }
        constructor_mutations = {
            'wrong-string-language-sentinel': [('languageHash60 = 0xffffffff;', 'languageHash60 = 0;')],
            'skip-nested-text-constructor': [('((FEngTextStorage *)&text64)->Construct();', 'text64 = 0;')],
            'wrong-list-default-flags': [('value5C = 4;', 'value5C = 0;')],
            'wrong-list-default-motion': [('value80 = 1;', 'value80 = 0;')],
            'wrong-group-list-vtable': [('children5C.value0 = 0x79a9ec;', 'children5C.value0 = 0;')],
            'wrong-type7-vtable': [('= 0x79b8f0;', '= 0x79b8fc;')],
            'wrong-type11-tail-write': [('*(unsigned int *)unidentified0 = 0x79b8fc;', '*(unsigned int *)unidentified0 = 0x79b8fc;\n    *(unsigned int *)(unidentified0 + 0x5c) = 0;')],
        }
        for kind, variants, base in (('factory', factory_mutations, args.source), ('constructors', constructor_mutations, args.constructor_source)):
            for name, edits in variants.items():
                changed = base.read_text()
                for before, after in edits:
                    assert before in changed, name
                    changed = changed.replace(before, after)
                path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(changed)
                result = subprocess.run([os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                    '--source', str(path if kind == 'factory' else args.source), '--constructor-source', str(path if kind == 'constructors' else args.constructor_source)],
                    cwd=ROOT, capture_output=True, text=True)
                (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
                assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
                report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
