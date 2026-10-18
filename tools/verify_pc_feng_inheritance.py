#!/usr/bin/env python3
"""Compare UG2 owned names and inheritance resolution with actual package/GUID tree callees."""
import argparse, hashlib, json, os, random, struct, subprocess, sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
BUILD = ROOT / 'build/analysis/feng-inheritance'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, SELF, CONTEXT, OUT_OBJECT, OUT_PACKAGE, QUEUE, SOURCE_PACKAGE, RECORDS, OLD_NAME, INPUT, ALT_NAME, NEW_NAME, STACK, STOP = (
    0x2000000 + x for x in (0, 0x100, 0x600, 0x800, 0x804, 0x1000, 0x2000, 0x3000, 0x9000, 0x9100, 0x9200, 0xa000, 0x1f000, 0x100000))
PACKAGES = (ARENA + 0x2400, ARENA + 0x2600)
TEB, GDT = 0x2120000, 0x2130000
PREFIXES = dict(SetName='?SetName@HudVisibilityObject@', FindInheritance='?FindInheritance@FEPackage@',
                FindByEncodedNameHash='?FindByEncodedNameHash@FEngQueue@', Resolve='?Resolve@FEngInheritanceContext@',
                FngHash='?FngHash@', PackageFindGuid='?FindObjectByGuid@FEPackage@', VisitObjects='?VisitObjects@FEPackage@',
                VisitChildren='?VisitChildren@FEPackage@', GuidQueryVisit='?Visit@FEngGuidQuery@')
ARGS = dict.fromkeys(PREFIXES, 1); ARGS.update(Resolve=3, VisitChildren=2)
ROOTS = ('SetName', 'FindInheritance', 'FindByEncodedNameHash', 'Resolve')
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngInheritance.cpp')
    parser.add_argument('--name-source', type=Path, default=FRONT / 'FEngObjectName.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true'); args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-inheritance.json').read_text())
    dep = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-lookup.json').read_text())['functions']
    for name in ('PackageFindGuid', 'VisitObjects', 'VisitChildren', 'GuidQueryVisit'): manifest['functions'][name] = dep[name]
    manifest['functions']['FngHash'] = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid.json').read_text())['functions']['FngHash']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.name_source, FRONT / 'FEngLookup.cpp', FRONT / 'FEngHash.cpp'):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/EHsc', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                  '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    externals = {'_cihash': 0x505450, '?FngFindPackage@@YAPAUFEPackage@@PBD@Z': 0x52cef0,
                 '?fengManager8384C4@@3PAUFEngManager@@A': 0x8384c4,
                 '??3@YAXPAX@Z': STOP + 0x200, '??3@YAXPAXI@Z': STOP + 0x300,
                 '__purecall': STOP + 0x400, '___CxxFrameHandler3': STOP + 0x500,
                 '___CxxFrameHandler': STOP + 0x500}
    for obj in objects:
        for s in read_object(obj)['symbols'].values():
            if s['section'] != 0: continue
            if s['name'].startswith('?FEngAllocateQueueNode@'): externals[s['name']] = 0x575620
            elif s['name'].startswith('?FEngFreeQueueNode@'): externals[s['name']] = 0x575630
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, p in PREFIXES.items()}
    original = {n: int(manifest['functions'][n]['address'], 16) for n in PREFIXES}
    state, machines = {}, []
    def words(uc, a, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def snapshot(uc): return bytes(uc.mem_read(ARENA, 0xc000))
    hits = dict.fromkeys(PREFIXES, 0)
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
        helpers = {a: n for n, a in (entries if rebuilt else original).items()}
        ranges = linked['ranges'] if rebuilt else [(a, a + manifest['functions'][n]['size']) for n, a in original.items()]
        def query(uc, address): return ('guid-query', words(uc, address + 4, 2))
        def hook(uc, address, size, user, helpers=helpers, ranges=ranges, original_run=not rebuilt):
            sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if address in helpers:
                name = helpers[address]
                if original_run: hits[name] += 1
                argv = words(uc, sp + 4, ARGS[name]); this = uc.reg_read(xc.UC_X86_REG_ECX)
                if name == 'FngHash': this = None
                if name == 'VisitObjects': argv = (query(uc, argv[0]),)
                if name == 'VisitChildren': argv = (argv[0], query(uc, argv[1]))
                if name == 'GuidQueryVisit': this = query(uc, this)
                state['trace'].append((name, this, argv, snapshot(uc)))
            if address not in (0x575620, 0x575630):
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            ret, argument = words(uc, sp, 2); mode = state['mode']
            state['trace'].append(('allocate' if address == 0x575620 else 'free', argument, snapshot(uc)))
            if mode & 1: put(uc, SELF + 0x14, ALT_NAME)
            if mode & 2: put(uc, SELF + 0x10, 0x12345678)
            if mode & 4: put(uc, SELF + 0x1c, mode | 2)
            if mode & 8 and uc.mem_read(INPUT, 1) != b'\0': uc.mem_write(INPUT, b'Q')
            uc.reg_write(xc.UC_X86_REG_EAX, NEW_NAME if address == 0x575620 else 0xfeedface)
            uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    def hash_(name):
        value = 0xffffffff
        for c in name: value = (value * 33 + (c - 32 if 97 <= c <= 122 else c)) & 0xffffffff
        return value
    counts = dict.fromkeys(ROOTS, 0); rng = random.Random(0x5186c0)
    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(0x20000)); mode = (trial // 17) % 16
        def putm(a, v): struct.pack_into('<I', memory, a - ARENA, v & 0xffffffff)
        names = (b'!', b'a', b'Screen.fng', b'\x80\xffz', b'', b'MiXeD')
        name = names[trial % len(names)]
        for a, data in ((INPUT, name + b'\0'), (OLD_NAME, b'old-name\0'), (ALT_NAME, b'other\0')):
            memory[a - ARENA:a - ARENA + len(data)] = data
        putm(SELF + 0x14, 0 if trial % 3 == 0 else OLD_NAME)
        putm(CONTEXT, SOURCE_PACKAGE); putm(CONTEXT + 0x3c, QUEUE)
        putm(QUEUE + 0x104, 0 if trial % 13 == 0 else PACKAGES[0])
        requested_hash = hash_(name)
        for pi, package in enumerate(PACKAGES):
            encoded = b'\x12\x34' + (name if pi == 0 or trial % 3 == 0 else names[(trial + 1) % len(names)]) + b'\0'
            header = ARENA + 0xb000 + pi * 0x100
            memory[header - ARENA:header - ARENA + len(encoded)] = encoded
            putm(package + 0x38, 0xfffffffe if trial % 11 == 0 else header)
            putm(package + 4, PACKAGES[1] if pi == 0 else 0)
            putm(package + 0xc, 0xdeadbeef); putm(package + 0x10, requested_hash ^ 1)
            base = ARENA + 0x4000 + pi * 0x1000
            nodes = [base + i * 0x100 for i in range(5)]
            for i, node in enumerate(nodes):
                putm(node + 4, 0); putm(node + 0x18, 1); putm(node + 0x64, 0)
                putm(node + 0xc, (0x11, 0x22, 42, 42, package)[i])
            putm(nodes[0] + 0x18, 5); putm(nodes[0] + 0x64, nodes[1]); putm(nodes[0] + 4, nodes[3])
            putm(nodes[1] + 0x18, 5); putm(nodes[1] + 0x64, nodes[2]); putm(nodes[3] + 4, nodes[4])
            putm(package + 0x4c, 0 if trial % 17 == 0 else nodes[0])
        key = (0, 1, 42, 0xffffffff, 0x80000000)[trial % 5]
        record_count = trial % 7
        putm(SOURCE_PACKAGE + 0x84, record_count); putm(SOURCE_PACKAGE + 0x88, RECORDS)
        guid = (0, 42, 0x11, 0x22, 999, PACKAGES[0], PACKAGES[1])[trial % 7]
        for i in range(7):
            putm(RECORDS + i * 12, key if i % 3 == 0 else key ^ (i + 1))
            putm(RECORDS + i * 12 + 4, requested_hash if i % 2 == 0 else requested_hash ^ 1)
            putm(RECORDS + i * 12 + 8, guid if i % 2 == 0 else 42)
        out_object, out_package = OUT_OBJECT, OUT_PACKAGE
        alias = (trial // 7) % 5
        if alias == 1: out_package = out_object
        if alias == 2: out_package = RECORDS + 8
        if alias == 3: out_object = SOURCE_PACKAGE + 0x84
        if alias == 4: out_object = RECORDS + 4
        for name_ in ROOTS:
            arguments = [0 if trial % 7 == 0 else INPUT] if name_ == 'SetName' else [key] if name_ == 'FindInheritance' else [requested_hash] if name_ == 'FindByEncodedNameHash' else [key, out_object, out_package]
            this = {'SetName': SELF, 'FindInheritance': SOURCE_PACKAGE, 'FindByEncodedNameHash': QUEUE, 'Resolve': CONTEXT}[name_]
            outputs = []
            for rebuilt, uc in zip((False, True), machines):
                uc.mem_write(ARENA, bytes(memory)); put(uc, TEB, 0xffffffff)
                for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
                uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *arguments))
                uc.reg_write(xc.UC_X86_REG_ECX, this); uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                state.clear(); state.update(trace=[], mode=mode)
                try: uc.emu_start((entries if rebuilt else original)[name_], STOP, count=100000)
                except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, name_, mode, alias, str(error))) from error
                assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', name_, 'nontermination')
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + 4 * len(arguments)
                assert all(uc.reg_read(reg) == 0x12345000 + i for i, reg in enumerate(PRESERVED))
                assert words(uc, TEB)[0] == 0xffffffff
                result = uc.reg_read(xc.UC_X86_REG_EAX) if name_ != 'SetName' else None
                if name_ == 'Resolve': result &= 255
                outputs.append((result, snapshot(uc), state['trace']))
            assert outputs[0] == outputs[1], ('state or call order mismatch', trial, name_, mode, alias, outputs[0][0], outputs[1][0],
                [(hex(ARENA + i), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b][:12],
                [x[:-1] for x in outputs[0][2]], [x[:-1] for x in outputs[1][2]])
            counts[name_] += 1
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), actual_entry_hits=hits, byte_matching=False)
    if args.mutation_checks:
        report['rejected_mutations'] = []
        mutations = {
            'record-wrong-key': ('records[i].key0 == key', 'records[i].guid8 == key'),
            'record-skip-first': ('unsigned int i = 0;', 'unsigned int i = 1;'),
            'package-wrong-chain': ('FEPackage *package = first104;', 'FEPackage *package = packagesDC.first8;'),
            'package-use-cached-hash': ('if (value == hash)', 'if (package->nameHash10 == hash)'),
            'package-wrong-offset': ('package->value38 + 2', 'package->value38 + 1'),
            'resolve-omit-object-clear': ('    *object = 0;', '    /* omitted */'),
            'resolve-omit-package-store': ('        *package = found;', '        /* omitted */'),
            'resolve-cache-guid': [('        FEPackage *found =', '        unsigned int cached = record->guid8;\n        FEPackage *found ='), ('FindObjectByGuid(record->guid8)', 'FindObjectByGuid(cached)')],
        }
        name_mutations = {
            'name-omit-free': ('FEngFreeQueueNode(name14);', '/* omitted */'),
            'name-omit-reset': ('name14 = 0;', '/* omitted */'),
            'name-wrong-sentinel': ('hash10 = 0xffffffff;', 'hash10 = 0;'),
            'name-wrong-size': ('FEngAllocateQueueNode(length)', 'FEngAllocateQueueNode(length - 1)'),
            'name-dirties-flags': ('    hash10 = 0xffffffff;', '    flags1C |= 0x400000;\n    hash10 = 0xffffffff;'),
        }
        for owner, cases in ((args.source, mutations), (args.name_source, name_mutations)):
            for label, edits in cases.items():
                if isinstance(edits, tuple): edits = [edits]
                source = owner.read_text()
                for before, after in edits:
                    assert before in source, label; source = source.replace(before, after, 1)
                mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(source)
                option = '--source' if owner == args.source else '--name-source'
                run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, option, str(mutant), '--cases', '128'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
                assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
                report['rejected_mutations'].append(label)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report))


if __name__ == '__main__': main()
