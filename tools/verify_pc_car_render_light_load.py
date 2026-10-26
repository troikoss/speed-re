#!/usr/bin/env python3
"""Original/rebuilt lighting chunk lifecycles and complete shared registry.

All recovered loader, list, defaults and registry bodies execute as machine code.
The retained bOMalloc executes the original already-recovered slot-pool body.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

WORLD = ROOT / 'src/Speed/Indep/Src/World'
SOURCES = [WORLD / name for name in ('CarRenderLightLoad.cpp', 'CarRenderLightLoadHelpers.cpp',
                                    'CarRenderLightSelect.cpp', 'CarRenderLightTree.cpp', 'TrackSectionResources.cpp')]
BUILD = ROOT / 'build/analysis/car-render-light-load'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-light-load.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x20000, 0x2100000, 0x2110000
REGISTRY = 0x883e70
GLOBALS = {'ug2LightSectors830D14': 0x830d14, 'ug2AdditionalLights835850': 0x835850,
           'ug2LightAnimations830AE8': 0x830ae8, 'ug2LightProfiles8358C0': 0x8358c0,
           'ug2LightStatePool8708B8': 0x8708b8, 'ug2LightDefaults7FF060': 0x7ff060,
           'ug2LightDefaultPack7F399C': 0x7f399c, 'ug2LightFallbackPack830BD8': 0x830bd8}
ORIGINAL = {'construct': 0x5e6360, 'acquire': 0x5db680, 'release': 0x5db6e0,
            'list': 0x598750, 'initialize': 0x57bcd0, 'load': 0x48e070, 'unload': 0x48e270,
            'append': 0x48e380, 'remove': 0x48e3b0, 'allocate': 0x5bb590,
            'defaults': 0x5bb5b0, 'noop': 0x4022c0}
ORIGINAL['select'] = 0x48e3d0
PREFIX = {'construct': '?Construct@TrackTopology@@', 'acquire': '?AcquireResource@TrackTopology@@',
          'release': '?ReleaseResource@TrackTopology@@', 'list': '?Construct@CarRenderLightList@@',
          'initialize': '?Initialise@CarRenderLightList@@', 'load': '?CarRenderLoadLightChunk@@',
          'unload': '?CarRenderUnloadLightChunk@@', 'append': '?CarRenderRegisterAdditionalLights@@',
          'remove': '?CarRenderRemoveLightSector@@', 'allocate': '?AllocateState@CarRenderLightPack@@',
          'defaults': '?ApplyDefaults@CarRenderLightPack@@', 'noop': '?LoadNoop@CarRenderLightTree@@'}
PREFIX['select'] = '?CarRenderSelectLights@@'
GCC = {'construct': '_ZN13TrackTopology9ConstructEv', 'acquire': '_ZN13TrackTopology15AcquireResourceEi',
       'release': '_ZN13TrackTopology15ReleaseResourceEi', 'list': '_ZN18CarRenderLightList9ConstructEv',
       'initialize': '_ZN18CarRenderLightList10InitialiseEv', 'load': '_Z23CarRenderLoadLightChunk',
       'unload': '_Z25CarRenderUnloadLightChunk', 'append': '_Z33CarRenderRegisterAdditionalLights',
       'remove': '_Z26CarRenderRemoveLightSector', 'allocate': '_ZN18CarRenderLightPack13AllocateStateEv',
       'defaults': '_ZN18CarRenderLightPack13ApplyDefaultsEv', 'noop': '_ZN18CarRenderLightTree8LoadNoopEv'}
GCC['select'] = '_Z21CarRenderSelectLights'
METHODS = {'construct': 0, 'acquire': 1, 'release': 1, 'list': 0, 'initialize': 0,
           'allocate': 0, 'defaults': 0, 'noop': 0}
HEADS = (0x830ae8, 0x830d14, 0x8358c0, 0x835850)
HASHES = (0x12c9453c, 0x3ed70c43, 0x471a1dca, 0x89f23a14, 0x89946400,
          0x8050ca3f, 0x8a616c4c, 0xd68ea0c1, 0xd6ec76d5, 0x2388dd82,
          0x23e6a3f4, 0x23e6b396, 0x6cf41096, 0x8062e2c0, 0x8cb61dcd,
          0x8cb694ed, 0x8cb95e6b, 0x8cbc02ec, 0x8cbe3497, 0xb3645f81,
          0xc9d2b801, 0x33a26cb6, 0xa6348ee3, 0x2c420d64, 0x12345678)


def build(replace, gcc):
    sources = [replace / p.name if replace and (replace / p.name).exists() else p for p in SOURCES]
    if gcc:
        linked = link_native(sources, BUILD, ROOT, dict(GLOBALS, trackTopology=REGISTRY,
                             _Z8bOMallocPv=0x440e50), '_ZN13TrackTopology9ConstructEv')
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        objects = []
        for i, path in enumerate(sources):
            obj = BUILD / ('load-' + str(i) + '.obj')
            subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                            '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                            '/Fo' + str(obj.relative_to(ROOT)), str(path.relative_to(ROOT))], cwd=ROOT, check=True)
            objects.append(obj)
        external = {'_' + k: v for k, v in GLOBALS.items()}
        external.update({'__fltused': 0, '?trackTopology@@3UTrackTopology@@A': REGISTRY})
        for obj in objects:
            for s in read_object(obj)['symbols'].values():
                if s['section'] == 0 and s['name'].startswith('?bOMalloc@@'):
                    external[s['name']] = 0x440e50
        linked = link_objects(objects, external, start=0x2400000)
    linked['entries'] = {kind: next(v for k, v in linked['exports'].items() if k.startswith(prefix))
                         for kind, prefix in (GCC if gcc else PREFIX).items()}
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False, arena_size=SIZE):
        self.size = arena_size
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, (arena_size + 4095) & ~4095)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for address, data in linked['blocks']: u.mem_write(address, data)
        self.entries = linked['entries'] if linked else ORIGINAL
        self.gcc = gcc
        for kind, address in self.entries.items():
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=kind, begin=address, end=address)
        u.hook_add(UC_HOOK_CODE, self.hook, user_data='pool', begin=0x440e50, end=0x440e50)

    def args(self, n):
        sp = self.u.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(sp + 4, n * 4)))

    def hook(self, u, address, size, kind):
        if kind in METHODS:
            if self.gcc: receiver, *args = self.args(METHODS[kind] + 1)
            else: receiver, args = u.reg_read(xc.UC_X86_REG_ECX), self.args(METHODS[kind])
            self.events.append((kind, receiver, tuple(args)))
        elif kind == 'pool': self.events.append((kind, self.args(1)))
        else: self.events.append((kind, self.args(2 if kind == 'select' else 1)))

    def reset(self, case):
        data, registry, defaults, chunks, section = case
        self.u.mem_write(ARENA, data)
        self.u.mem_write(REGISTRY, registry)
        self.u.mem_write(0x7ff060, defaults)
        self.u.mem_write(0x7f399c, struct.pack('<I', 0x830bd8))
        self.u.mem_write(0x8708b8, struct.pack('<I', ARENA + 0x1a000))

    def run(self, kind, receiver, args, cw):
        u = self.u
        self.events = []
        sp = STACK + 0x8000
        actual = ([receiver] + args) if self.gcc and kind in METHODS else args
        u.mem_write(sp, struct.pack('<' + 'I' * (len(actual) + 1), STOP, *actual))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        u.reg_write(xc.UC_X86_REG_ECX, receiver)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved): u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entries[kind], STOP, count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        pop = len(args) * 4 if kind in METHODS and not self.gcc else 0
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4 + pop
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        result = None if kind in ('release', 'defaults', 'initialize', 'allocate', 'noop') else u.reg_read(xc.UC_X86_REG_EAX)
        return (bytes(u.mem_read(ARENA, self.size)), bytes(u.mem_read(REGISTRY, 0x643c)),
                tuple(bytes(u.mem_read(h, 8)) for h in HEADS), bytes(u.mem_read(0x7f399c, 4)), result, self.events)


def cases(n):
    rng = random.Random(0x48e070)
    for index in range(n):
        data = bytearray(rng.randbytes(SIZE))
        def put(address, fmt, *values): struct.pack_into('<' + fmt, data, address - ARENA, *values)
        # Original slot-pool backend, with ample slots and expansion disabled.
        pool = ARENA + 0x1a000
        put(pool, '12I', 0, 0, 0, 0, ARENA + 0x1b000, 0, 0, 0, 0, 16, 16, 16)
        for i in range(16): put(ARENA + 0x1b000 + i * 16, 'I', ARENA + 0x1b000 + (i + 1) * 16 if i < 15 else 0)
        section = rng.choice((0, 2799, rng.randrange(2800)))
        chunks = []
        def child(address, key, payload):
            aligned = (address + 0x17) & ~15
            size = aligned - (address + 8) + len(payload)
            put(address, '2I', key, size)
            data[aligned - ARENA:aligned - ARENA + len(payload)] = payload
            return address + 8 + size
        primary = ARENA + 0x2000 + (index % 4) * 4
        offset = primary + 8
        payload = bytearray(rng.randbytes(0x40))
        struct.pack_into('<HBBI', payload, 8, 0 if index % 13 == 0 else 4, index % 3, 0, section)
        offset = child(offset, 0x135001, payload)
        if index & 1: offset = child(offset, 0x12345678, rng.randbytes(16))
        offset = child(offset, 0x135002, rng.randbytes(0x70))
        offset = child(offset, 0x135003, rng.randbytes(0x120))
        put(primary, '2I', 0x80135000, offset - primary - 8)
        chunks.append(primary)
        animation = ARENA + 0x4000 + (index % 4) * 4
        offset = animation + 8
        if index & 1: offset = child(offset, 0x12345678, rng.randbytes(16))
        payload = bytearray(rng.randbytes(0x60))
        struct.pack_into('<I', payload, 8, 0 if index % 11 == 0 else 3)
        count = index % 9
        struct.pack_into('<HBBI', payload, 0x50, count, index % 3, 0, section)
        offset = child(offset, 0x135101, payload)
        offset = child(offset, 0x135102, rng.randbytes(count * 0x30))
        put(animation, '2I', 0x80135100, offset - animation - 8)
        chunks.append(animation)
        profile = ARENA + 0x6000 + (index % 4) * 4
        put(profile, '2I', 0x135200, 0x94)
        put(profile + 8 + 0xc, '2I', HASHES[index % len(HASHES)], 0 if index % 7 == 0 else 3)
        put(profile + 8 + 0x80, 'f', rng.uniform(-100, 100))
        put(profile + 8 + 0x90, 'f', rng.uniform(-100, 100))
        chunks.append(profile)
        put(ARENA + 0x8000, '2I', 0x12345678, 0)
        chunks.append(ARENA + 0x8000)
        yield bytes(data), rng.randbytes(0x643c), rng.randbytes(40), chunks, section


def verify(image, linked, gcc, n):
    a, b = Machine(image), Machine(image, linked, gcc)
    totals, events = Counter(), Counter()
    def compare(kind, receiver, args, cw, index):
        x, y = a.run(kind, receiver, args, cw), b.run(kind, receiver, args, cw)
        if x != y:
            differences = []
            for part in range(2):
                offsets = sorted({i & ~3 for i, (v, w) in enumerate(zip(x[part], y[part])) if v != w})[:4]
                differences += [(part, hex(i), x[part][i:i + 4].hex(), y[part][i:i + 4].hex()) for i in offsets]
            raise AssertionError(('loader mismatch', index, kind, hex(cw), args, differences,
                                  x[4], y[4], x[5] if x[5] != y[5] else [], y[5] if x[5] != y[5] else []))
        totals[kind] += 1
        events.update(e[0] for e in x[5])
    for index, case in enumerate(cases(n)):
        for cw in (0x027f, 0x037f):
            a.reset(case); b.reset(case)
            compare('construct', REGISTRY, [], cw, index)
            for head in HEADS: compare('list', head, [], cw, index)
            section = case[4]
            for _ in range(3): compare('acquire', REGISTRY, [section], cw, index)
            for _ in range(4): compare('release', REGISTRY, [section], cw, index)
            compare('defaults', case[3][2] + 8, [], cw, index)
            for chunk in case[3]: compare('load', 0, [chunk], cw, index)
            for chunk in reversed(case[3]): compare('unload', 0, [chunk], cw, index)
            compare('append', 0, [0], cw, index)
            compare('remove', 0, [0], cw, index)
            for sector in (ARENA + 0x1000, ARENA + 0x1100): compare('append', 0, [sector], cw, index)
            for sector in (ARENA + 0x1000, ARENA + 0x1100): compare('remove', 0, [sector], cw, index)
            if index == 0:
                for section in range(512): compare('acquire', REGISTRY, [section], cw, index)
                for section in range(512): compare('release', REGISTRY, [section], cw, index)
    return dict(comparisons=dict(totals), calls=dict(events), failures=0)


def verify_retail(image, linked, gcc, path):
    raw = path.read_bytes()
    fixture, chunks, points = bytearray(0x2000), [], []
    counts = Counter()
    offset = 0
    while offset + 8 <= len(raw):
        key, size = struct.unpack_from('<II', raw, offset)
        end = offset + 8 + size
        assert end <= len(raw)
        if key in (0x80135000, 0x80135100):
            begin = ((len(fixture) + 15) & ~15) + (offset & 15)
            fixture.extend(bytes(begin - len(fixture)))
            fixture.extend(raw[offset:end])
            chunks.append(ARENA + begin)
            counts[hex(key)] += 1
            if key == 0x80135000:
                child = offset + 8
                while child + 8 <= end:
                    subkey, n = struct.unpack_from('<II', raw, child)
                    aligned = (child + 0x17) & ~15
                    if subkey == 0x135003:
                        for address in range(aligned, child + n + 8 - 0x5f, 0x60):
                            points.append(struct.unpack_from('<3f', raw, address + 0x10))
                    child += n + 8
        offset = end
    assert chunks and points
    a = Machine(image, arena_size=len(fixture))
    b = Machine(image, linked, gcc, arena_size=len(fixture))
    case = bytes(fixture), bytes(0x643c), bytes(40), chunks, 0
    comparisons, calls = Counter(), Counter()
    for cw in (0x027f, 0x037f):
        a.reset(case); b.reset(case)
        def compare(kind, receiver, args):
            x, y = a.run(kind, receiver, args, cw), b.run(kind, receiver, args, cw)
            assert x == y, ('retail loader mismatch', kind, hex(cw), args, x[4], y[4], x[5], y[5])
            comparisons[kind] += 1
            calls.update(e[0] for e in x[5])
        compare('construct', REGISTRY, [])
        for head in HEADS: compare('list', head, [])
        for chunk in chunks: compare('load', 0, [chunk])
        registry_count = struct.unpack('<I', bytes(a.u.mem_read(REGISTRY + 0x2c10, 4)))[0]
        queries = points[::max(1, len(points) // 128)]
        rng = random.Random(0x883e7048e3d0)
        queries += [(rng.uniform(-3000, 3000), rng.uniform(-3000, 3000), rng.uniform(-100, 100)) for _ in range(128)]
        for point in queries:
            for machine in (a, b): machine.u.mem_write(ARENA + 0x300, struct.pack('<3f', *point))
            compare('select', 0, [ARENA + 0x100, ARENA + 0x300])
        for chunk in reversed(chunks): compare('unload', 0, [chunk])
        assert struct.unpack('<I', bytes(a.u.mem_read(REGISTRY + 0x2c10, 4)))[0] == 0
    return dict(file=path.name, sha256=hashlib.sha256(raw).hexdigest(), chunks=dict(counts),
                registry_entries=registry_count, comparisons=dict(comparisons), calls=dict(calls), failures=0,
                limitations=['actual retail chunk data, constructed query positions; not an original streaming schedule'])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--cases', type=int, default=1024)
    p.add_argument('--replace', type=Path)
    p.add_argument('--mutation-checks', action='store_true')
    p.add_argument('--retail-track', type=Path)
    args = p.parse_args()
    raw = EXE.read_bytes(); image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[off:off + f['size']]).hexdigest() == f['sha256']
    linked = build(args.replace, args.gcc)
    report = verify(image, linked, args.gcc, args.cases)
    if args.retail_track:
        report['retail_track'] = verify_retail(image, linked, args.gcc, args.retail_track)
    report.update(compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0', byte_identity='not established',
                  limitations=['finite valid synthetic chunks/IDs and available registry slots',
                               'slot allocation executes original bOMalloc; no invalid-length safety claim',
                               'primary unloader requires a leading header; original malformed-input loop retained'])
    if args.mutation_checks:
        mutations = {
            'short-registry-table': ('TrackSectionResources.cpp', 'i < 2800', 'i < 2799'),
            'clear-registry-payloads': ('TrackSectionResources.cpp', 'resourceCount2C10 = 0;', 'resourceCount2C10 = 0; for (int j = 0; j < 512; ++j) storage2C14[j].valueC = 0;'),
            'short-free-record-clear': ('TrackSectionResources.cpp', 'i < 7', 'i < 6'),
            'early-final-release': ('TrackSectionResources.cpp', '--record->used.references0 == 0', '--record->used.references0 == 1'),
            'wrong-light-version': ('CarRenderLightLoad.cpp', 'sector->version08 != 4', 'sector->version08 != 3'),
            'wrong-tree-fixup': ('CarRenderLightLoad.cpp', '(tree) + 0x10)', '(tree) + 0x20)'),
            'skip-initial-empty-call': ('CarRenderLightLoad.cpp', 'if (fresh) tree->LoadNoop();', 'if (false && fresh) tree->LoadNoop();'),
            'wrong-animation-stride': ('CarRenderLightLoad.cpp', '(record) + 0x30)', '(record) + 0x20)'),
            'wrong-default-hash': ('CarRenderLightLoad.cpp', 'pack->hash0C == 0x2c420d64', 'pack->hash0C == 0x2c420d65'),
            'wrong-profile-default': ('CarRenderLightLoadHelpers.cpp', 'first = 0; second = 4;', 'first = 1; second = 4;'),
            'keep-unloaded-sector': ('CarRenderLightLoad.cpp', 'resources50[sector->section0C]->valueC = 0;', 'resources50[sector->section0C]->valueC = sector;'),
        }
        rejected = []
        for name, (filename, before, after) in mutations.items():
            source = (WORLD / filename).read_text(); assert before in source, name
            directory = BUILD / ('mutation-' + name); directory.mkdir(exist_ok=True)
            (directory / filename).write_text(source.replace(before, after))
            result = subprocess.run([sys.executable, __file__, '--replace', str(directory), '--cases', '64'] +
                                    (['--gcc'] if args.gcc else []), text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (directory / 'result.log').write_text(result.stdout)
            assert result.returncode and 'loader mismatch' in result.stdout, (name, result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + ('-retail' if args.retail_track else '')
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
