#!/usr/bin/env python3
"""Compare UG2 state-pool allocation and concrete virtual destruction with real release callees."""
import argparse, hashlib, json, os, random, struct, subprocess
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-state-pool'
ARENA, CLASS, BLOCK, NEW_CLASS, NEW_BLOCK, STACK, STOP = (0x2000000 + x for x in (0, 0x1000, 0x3000, 0x6000, 0x7000, 0x1f000, 0x100000))
DATA, NEW_DATA, MANAGER, OBSERVER, TEB, GDT = 0x4000000, 0x4600000, 0x863770, 0x838474, 0x2120000, 0x2130000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'ConstructBlock': '?Construct@FEngStatePoolBlock@', 'AcquireSlot': '?AcquireSlot@FEngStatePoolBlock@',
            'AcquireBlock': '?Acquire@FEngStatePoolClass@', 'AcquireState': '?AcquireState@FEngStatePoolManager@',
            'DestructBlock': '?Destruct@FEngStatePoolBlock@', 'DestroyBlock': '?Destroy@FEngStatePoolBlock@',
            'DestructClass': '?Destruct@FEngStatePoolClass@', 'DestroyClass': '?Destroy@FEngStatePoolClass@',
            'ReleaseSlot': '?ReleaseSlot@FEngStatePoolClass@', 'ReleaseState': '?ReleaseState@FEngStatePoolManager@',
            'ClearOwned': '?ClearOwned@FEngQueueList@', 'Remove': '?Remove@FEngQueueList@'}
ARGS = dict.fromkeys(PREFIXES, 0)
ARGS.update(ConstructBlock=1, AcquireState=1, DestroyBlock=1, DestroyClass=1, ReleaseSlot=1, ReleaseState=1, Remove=1)
ROOTS = tuple(list(PREFIXES)[:8])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngStatePool.cpp')
    parser.add_argument('--mutation-checks', action='store_true'); args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-state-pool.json').read_text())
    lifecycle = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lifecycle.json').read_text())
    manifest['functions'].update(lifecycle['functions']); manifest['data'] = lifecycle['data']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(manifest['data'].values()) + list(manifest['tables'].values()):
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, ROOT / 'src/Speed/Indep/Src/Frontend/FEngLifecycle.cpp', ROOT / 'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp'):
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
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(prefix)) for n, prefix in PREFIXES.items()}
    markers = {0x50440e: 'full-block', 0x504420: 'skip-full-byte', 0x504445: 'skip-used-bit', 0x50445f: 'claim-slot',
               0x517434: 'new-block-allocation', 0x517363: 'block-data-allocation', 0x52ad0e: 'new-class-allocation',
               0x5173a0: 'block-self-free', 0x534ab0: 'class-self-free', 0x534aee: 'class-owned-block-clear'}
    coverage = dict.fromkeys(markers.values(), 0); machines = []; state = {}
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0x10000)) + bytes(uc.mem_read(MANAGER, 16))
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
                if state['pending_data']:
                    state['pending_data'] = False; result = NEW_DATA
                    if mode & 2:
                        uc.mem_write(state['block'] + 0xc, struct.pack('<HH', 3, 5))
                        put(uc, state['block'] + 0x10, 0xffffffff)
                    if mode & 4:
                        current = state['class']; put(uc, current + 0x14, BLOCK + 0x100); put(uc, current + 0x18, BLOCK + 0x100)
                        put(uc, current + 0x10, 1); put(uc, BLOCK + 0x108, 0)
                elif argument == 0x18:
                    result = NEW_BLOCK
                    if mode & 1: put(uc, state['class'] + 0x1c, 0x10003)
                else:
                    assert argument == 0x20
                    result = NEW_CLASS
                    if mode & 8:
                        put(uc, MANAGER + 4, 1); put(uc, MANAGER + 8, CLASS + 0x100); put(uc, MANAGER + 12, CLASS + 0x100)
                        put(uc, CLASS + 0x108, 0)
            else: result = 0xfeedface
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        def payload_write(uc, access, address, size, value, user):
            if DATA <= address < DATA + 0x1000000:
                state['payload_writes'].append((address, size, value))
        uc.hook_add(UC_HOOK_CODE, hook); uc.hook_add(UC_HOOK_MEM_WRITE, payload_write); machines.append(uc)
    counts = dict.fromkeys(ROOTS + ('AcquireReleaseSequence',), 0); rng = random.Random(0x52ace0)
    def compare(name, memory, manager, this, arguments, mode, release=False):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(MANAGER, manager); put(uc, OBSERVER, 0); put(uc, TEB, 0xffffffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, pending_data=False, block=BLOCK, payload_writes=[], **{'class': CLASS})
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
            if name in ('DestructBlock', 'DestructClass'): result = None
            if release: run('ReleaseState', MANAGER, [result])
            outputs.append((result, snapshot(uc), state['trace']))
        assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name, mode,
              [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:20],
              [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
        counts['AcquireReleaseSequence' if release else name] += 1
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 16) % 16
        def putm(a, value): struct.pack_into('<I', memory, a - ARENA, value & 0xffffffff)
        stride = (1, 2, 4, 8, 16, 32, 64, 127, 0xffff)[trial % 9]
        keys = (stride, stride | 0x10000)
        for ci in range(2):
            c = CLASS + ci * 0x100; first = BLOCK if ci == 0 else BLOCK + 0x200
            count = 0 if trial % 4 == 0 else 2 if ci == 0 else 1
            memory[c - ARENA:c - ARENA + 32] = struct.pack('<8I', 0x79b8dc, CLASS + 0x100 if ci == 0 else 0, CLASS if ci else 0,
                 0x79a9ec, count, first if count else 0, first + 0x100 if count == 2 else first if count else 0, keys[ci])
        for bi in range(3):
            b = BLOCK + bi * 0x100
            bitmap = rng.getrandbits(32) & ~(1 << ((trial // 3) % 32))
            if (bi == 0 and trial % 4 in (1, 3)) or (bi == 1 and trial % 4 == 3): bitmap = 0xffffffff
            used = bitmap.bit_count()
            if used != 32 and trial % 17 == 0: used = 65535
            if bi == 0 and trial % 23 == 0: used = 32
            memory[b - ARENA:b - ARENA + 24] = struct.pack('<IIIHHII', 0x79b13c,
                BLOCK + 0x100 if bi == 0 else 0, BLOCK if bi == 1 else 0, stride, used, bitmap, DATA + bi * 0x200000)
        manager = struct.pack('<4I', 0x79a9ec, 0 if trial % 5 == 0 else 2, 0 if trial % 5 == 0 else CLASS, 0 if trial % 5 == 0 else CLASS + 0x100)
        flags = (0, 1, 2, 0x100, 0x101, 0xffffffff)[trial % 6]
        key = (0, keys[0], keys[1], stride + 7)[(trial // 9) % 4]
        compare('ConstructBlock', memory, manager, BLOCK, [(rng.getrandbits(16) << 16) | stride], mode)
        compare('AcquireSlot', memory, manager, BLOCK, [], mode)
        compare('AcquireBlock', memory, manager, CLASS, [], mode)
        compare('AcquireState', memory, manager, MANAGER, [key], mode)
        compare('DestructBlock', memory, manager, BLOCK, [], mode)
        compare('DestroyBlock', memory, manager, BLOCK, [flags], mode)
        compare('DestructClass', memory, manager, CLASS, [], mode)
        compare('DestroyClass', memory, manager, CLASS, [flags], mode)
        if trial % 64 == 0:
            compare('AcquireState', memory, manager, MANAGER, [key], 0, release=True)
            empty_data = bytearray(memory)
            struct.pack_into('<I', empty_data, BLOCK + 0x14 - ARENA, 0)
            compare('DestructBlock', empty_data, manager, BLOCK, [], mode)
            compare('DestroyBlock', empty_data, manager, BLOCK, [flags], mode)
            compare('ConstructBlock', memory, manager, BLOCK, [0], mode)
            dirty = bytearray(memory)
            struct.pack_into('<I', dirty, CLASS - ARENA, 0xdeadbeef)
            compare('DestructClass', dirty, manager, CLASS, [], mode)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'actual_callees': ['block construction/claim/destruction', 'class acquire/destruction', 'owned list clear', 'slot/manager release', 'Remove'],
              'controlled_interfaces': ['allocation/free only; virtual slots relocated to actual rebuilt deleting destructors'],
              'limitations': ['normal return only, no SEH unwind/chain-observing callbacks', 'finite bitmap/list cases and bounded allocator mutations; invalid/OOM metadata and concurrency unverified',
                              'native production allocator/factory/owner integration and MSVC 7.1/byte matching pending'], 'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); report['rejected_mutations'] = []
        mutations = {
            'wrong-block-bytes': [('stride * 32', 'stride * 31')],
            'bitmap-clear-too-early': [('    data14 = (unsigned char *)FEngAllocateQueueNode((unsigned int)stride * 32);', '    *(unsigned int *)bitmap10 = 0;\n    data14 = (unsigned char *)FEngAllocateQueueNode((unsigned int)stride * 32);'), ('    *(unsigned int *)bitmap10 = 0;\n    return this;', '    return this;')],
            'zero-slot-payload': [('    *(unsigned int *)bitmap10 = 0;', '    *(unsigned int *)bitmap10 = 0;\n    if (data14) data14[0] = 0;')],
            'wrong-full-test': [('if (used == 32)', 'if (used >= 32)')],
            'wrong-first-free-bit': [('unsigned int slot = byte << 3;', 'unsigned int slot = (byte << 3) + 1;')],
            'wrong-claimed-bit': [('1u << (slot & 7));\n    return', '1u << ((slot + 1) & 7));\n    return')],
            'omit-used-increment': [('usedE = (unsigned short)(used + 1);', 'usedE = used;')],
            'wrong-slot-stride': [('(unsigned int)strideC * slot', '(unsigned int)strideC * (slot + 1)')],
            'skip-full-block-scan': [('block && block->usedE == 32', 'false')],
            'wrong-class-key': [('node->stride1C != stride', '(unsigned short)node->stride1C != (unsigned short)stride')],
            'skip-class-count': [('++classes0.count4;', '(void)classes0;')],
            'skip-block-count': [('++blocksC.count4;', '(void)blocksC;')],
            'skip-block-data-free': [('FEngFreeQueueNode(data14);', '(void)data14;')],
            'guard-null-data-free': [('FEngFreeQueueNode(data14);', 'if (data14) FEngFreeQueueNode(data14);')],
            'wrong-delete-flag': [('if (flags & 1)', 'if (flags & 2)')],
            'early-class-vtable-reset': [('void FEngStatePoolClass::Destruct() {', 'void FEngStatePoolClass::Destruct() {\n    vtable0 = (void *)0x79b8dc;')],
            'skip-owned-block-clear': [('blocksC.ClearOwned();', '(void)blocksC;')],
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
