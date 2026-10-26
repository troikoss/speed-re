#!/usr/bin/env python3
"""Compare recovered solid dispatch, texture swaps and primitive queues with UG2.

Runs the complete recovered chain on both compilers. Bounds classification,
texture lookup and successful pool allocation are explicit external boundaries;
their arguments and order are compared along with all fixture/list bytes.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderSolid.cpp'
BUILD = ROOT / 'build/analysis/car-render-solid'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-solid.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x10000, 0x2100000, 0x2110000
SOLID, TABLE, PLATFORM, VIEW, WORLD, MODE = [ARENA + o for o in (0, 0x100, 0x200, 0x300, 0x400, 0x480)]
TECHNIQUES, PRIMITIVES, TEXTURES, STATES = [ARENA + o for o in (0x500, 0x1000, 0x2000, 0x3000)]
SLOTS, LIGHTS, OVERRIDES, REMAP = [ARENA + o for o in (0x4000, 0x4800, 0x5000, 0x5800)]
GROUPS, COMMANDS = ARENA + 0x6000, ARENA + 0x8000
POOL_GROUP, POOL_COMMAND = ARENA + 0xff00, ARENA + 0xff04
ORIGINAL = dict(apply=0x4905a0, restore=0x488e40, default=0x48ac50,
                mode=0x60fdf0, dispatch=0x5c5930, queue=0x5ceb30)
PREFIX = dict(apply='?ApplyTextures@CarRenderSolid@@', restore='?RestoreTextures@CarRenderSolid@@',
              default='?CarRenderDefaultLightPack@@', mode='?CarRenderSpecialMode@@',
              dispatch='?DrawSolid@CarRenderView@@', queue='?CarRenderQueuePrimitive@@')
GCC = dict(apply='_ZN14CarRenderSolid13ApplyTexturesEv', restore='_ZN14CarRenderSolid15RestoreTexturesEv',
           default='_Z25CarRenderDefaultLightPackv', mode='_Z20CarRenderSpecialModev',
           dispatch='_ZN13CarRenderView9DrawSolidE', queue='_Z23CarRenderQueuePrimitive')
GLOBALS = dict(ug2SolidTechniques86EB28=0x86eb28, ug2SolidRemap7FF090=0x7ff090,
               ug2SolidRemap7FF0B4=0x7ff0b4, ug2SolidRemap7FF0D8=0x7ff0d8,
               ug2SolidFlag870CC4=0x870cc4, ug2SolidFlag870CD4=0x870cd4, ug2SolidFlag870D14=0x870d14,
               ug2SolidDebug8707CD=0x8707cd, ug2SolidViewMode870848=0x870848,
               ug2SolidSkipTexture870A18=0x870a18, ug2SolidMode8A1C8C=0x8a1c8c,
               ug2SolidGroupPool870918=0x870918, ug2SolidCommandPool87091C=0x87091c,
               ug2SolidGroups870D30=0x870d30, ug2SolidGroups8712F0=0x8712f0,
               ug2SolidGroupCount87094C=0x87094c, ug2LightDefaultPack7F399C=0x7f399c)
BOUNDARIES = dict(bounds=0x5bc300, lookup=0x4901d0, allocate=0x440e50)
REGIONS = ((0x86eb28, 23 * 4), (0x8707c0, 0x140), (0x870900, 0x130),
           (0x870d30, 0x1800), (0x8a1c8c, 4), (0x7f399c, 4))


def build(source, gcc):
    if gcc:
        bindings = dict(GLOBALS, UG2SolidFindTexture=BOUNDARIES['lookup'])
        bindings.update({'_ZN13CarRenderView10TestBoundsEPKfS1_P11WheelMatrix': BOUNDARIES['bounds'],
                         '_Z8bOMallocPv': BOUNDARIES['allocate']})
        linked = link_native(source, BUILD, ROOT, bindings, GCC['apply'])
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        obj = BUILD / 'solid.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))], cwd=ROOT, check=True)
        bindings = {'_' + k: v for k, v in GLOBALS.items()}
        bindings['_UG2SolidFindTexture'] = BOUNDARIES['lookup']
        for symbol in read_object(obj)['symbols'].values():
            for prefix, address in (('?TestBounds@CarRenderView@@', BOUNDARIES['bounds']),
                                    ('?bOMalloc@@', BOUNDARIES['allocate'])):
                if symbol['section'] == 0 and symbol['name'].startswith(prefix):
                    bindings[symbol['name']] = address
        linked = link_objects([obj], bindings, start=0x2400000)
    linked['entries'] = {k: next(v for name, v in linked['exports'].items() if name.startswith(prefix))
                         for k, prefix in (GCC if gcc else PREFIX).items()}
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for address, data in linked['blocks']: u.mem_write(address, data)
        self.entries = linked['entries'] if linked else ORIGINAL
        self.gcc = gcc
        self.initial = {a: image[a - 0x400000:a - 0x400000 + n] for a, n in REGIONS}
        for kind, address in BOUNDARIES.items():
            u.hook_add(UC_HOOK_CODE, self.boundary, user_data=kind, begin=address, end=address)
        for kind, address in self.entries.items():
            # GCC folds these tiny read-only helpers into their caller. Their
            # standalone return values are checked; trace observable boundaries.
            if kind in ('mode', 'default'): continue
            u.hook_add(UC_HOOK_CODE, self.observe, user_data=kind, begin=address, end=address)

    def args(self, n):
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(self.u.reg_read(xc.UC_X86_REG_ESP) + 4, n * 4)))

    def member(self, n):
        if self.gcc:
            receiver, *args = self.args(n + 1)
            return receiver, tuple(args)
        return self.u.reg_read(xc.UC_X86_REG_ECX), self.args(n)

    def observe(self, u, address, size, kind):
        if kind in ('apply', 'restore', 'dispatch'):
            receiver, args = self.member(5 if kind == 'dispatch' else 0)
            self.events.append((kind, receiver, args))
        else:
            args = self.args(9 if kind == 'queue' else 0)
            self.events.append((kind, args))
        if kind == 'queue':
            self.events.append(('queue-records', bytes(u.mem_read(args[0], 0x3c)),
                                bytes(u.mem_read(args[4], 0x57)),
                                bytes(u.mem_read(SLOTS, 0x800))))

    def boundary(self, u, address, size, kind):
        pop = 0
        if kind == 'bounds':
            receiver, args = self.member(3)
            assert receiver == VIEW
            minimum, maximum, world = args
            assert maximum == minimum + 0x10 and world == WORLD
            result = struct.unpack('<I', bytes(u.mem_read(minimum + 4, 4)))[0]
            self.events.append((kind, receiver, args))
            if not self.gcc: pop = 12
        elif kind == 'lookup':
            args = self.args(3)
            assert args[1:] == (1, 0)
            result = TEXTURES + (args[0] % 8) * 0x100
            self.events.append((kind, args))
        else:
            pool, = self.args(1)
            assert pool in (POOL_GROUP, POOL_COMMAND), hex(pool)
            count = self.allocations[pool]
            self.allocations[pool] += 1
            result = (GROUPS + count * 0x20) if pool == POOL_GROUP else (COMMANDS + count * 0x2c)
            assert result + 0x2c < ARENA + 0xff00
            self.events.append((kind, pool, result))
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        ret = struct.unpack('<I', bytes(u.mem_read(sp, 4)))[0]
        u.reg_write(xc.UC_X86_REG_EAX, result)
        u.reg_write(xc.UC_X86_REG_ESP, sp + 4 + pop)
        u.reg_write(xc.UC_X86_REG_EIP, ret)

    def run(self, kind, case):
        data, globals_, heads, calls = case
        u = self.u
        self.events = []
        self.allocations = Counter()
        u.mem_write(ARENA, data)
        for address, body in self.initial.items(): u.mem_write(address, body)
        for address, body in heads.items(): u.mem_write(address, body)
        for address, body in globals_.items(): u.mem_write(address, body)
        returns = []
        for receiver, args in calls[kind]:
            sp = STACK + 0x8000
            actual = ([receiver] + list(args)) if self.gcc and receiver else args
            u.mem_write(sp, struct.pack('<' + 'I' * (len(actual) + 1), STOP, *actual))
            u.reg_write(xc.UC_X86_REG_ESP, sp)
            u.reg_write(xc.UC_X86_REG_ECX, receiver)
            saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
            for i, reg in enumerate(saved): u.reg_write(reg, 0xabcdef00 + i)
            u.reg_write(xc.UC_X86_REG_FPCW, 0x037f)
            u.reg_write(xc.UC_X86_REG_FPSW, 0)
            u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            u.emu_start(self.entries[kind], STOP, count=100000)
            assert u.reg_read(xc.UC_X86_REG_EIP) == STOP, 'instruction budget'
            assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4 + (len(args) * 4 if receiver and not self.gcc else 0)
            assert [u.reg_read(reg) for reg in saved] == [0xabcdef00 + i for i in range(4)]
            assert u.reg_read(xc.UC_X86_REG_FPCW) == 0x037f
            assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
            if kind in ('default', 'mode'):
                result = u.reg_read(xc.UC_X86_REG_EAX)
                returns.append(result & 255 if kind == 'mode' else result)
        regions = sorted(set(REGIONS) | {(a, 8) for a in heads})
        return (bytes(u.mem_read(ARENA, SIZE)), [bytes(u.mem_read(a, n)) for a, n in regions],
                returns, self.events)


def cases(n):
    rng = random.Random(0x5c5930)
    for index in range(n):
        data = bytearray(rng.randbytes(SIZE))
        def put(a, fmt, *values): struct.pack_into('<' + fmt, data, a - ARENA, *values)
        def global_(a, fmt, *values): globals_[a] = struct.pack('<' + fmt, *values)
        globals_, heads = {}, {}
        # The three original remap arrays have nine elements. All 23 technique
        # IDs are also covered using the caller-provided map with remapping off.
        custom = index % 4 == 0
        base_type = index % (23 if custom else 9)
        put(SOLID + 8, '3Ii', rng.choice((0xbe43edbb, 0, rng.getrandbits(32))),
            0 if index % 29 == 0 else TABLE, 0 if index % 7 == 0 else OVERRIDES,
            rng.choice((-3, 0, 1, 2, 4, 8)))
        put(TABLE, 'I', PLATFORM)
        put(TABLE + 0x19, 'b', index % 256 - 128)
        put(TABLE + 0x2c, 'I', SLOTS)
        put(TABLE + 0x3c, 'I', LIGHTS)
        put(PLATFORM + 0x10, 'iI', rng.choice((-2, 0, 1, 4, 16, 32)), PRIMITIVES)
        put(PLATFORM + 0x38, '2I', REMAP if index % 3 else 0, TECHNIQUES + base_type * 0x20)
        put(VIEW + 0x54, 'I', 0 if index % 11 == 0 else ARENA + 0xe000)
        view_mode = rng.choice((0, 0, 0, 1, 2, 0xffffffff))
        put(MODE, 'I', view_mode)
        global_(0x870848, 'I', MODE)
        global_(0x8707cd, 'B', rng.choice((0, 1, 1, 255)))
        global_(0x870cc4, 'I', 0 if custom else rng.choice((0, 1, 2)))
        global_(0x870cd4, 'I', 1 if custom else rng.choice((0, 1, 2)))
        global_(0x870d14, 'I', rng.choice((0, 1, 2)))
        global_(0x8a1c8c, 'I', rng.choice((0, 3, 4, 4, 5, 0xffffffff)))
        global_(0x870a18, 'I', rng.choice((0x1234, 0x6d57415, 0xffffffff)))
        global_(0x870918, 'I', POOL_GROUP)
        global_(0x87091c, 'I', POOL_COMMAND)
        global_(0x87094c, 'I', rng.choice((0, 0xffffffff, rng.getrandbits(32))))
        global_(0x7f399c, 'I', rng.choice((0, ARENA + 0xe400)))
        global_(0x86eb28, '23I', *(TECHNIQUES + i * 0x20 for i in range(23)))
        for i in range(23):
            put(TECHNIQUES + i * 0x20 + 4, 'I', i)
            put(TECHNIQUES + i * 0x20 + 0x10, 'I', rng.choice((0, 1, 1, 0xffffffff)))
            put(REMAP + i * 4, 'i', rng.choice((-1, i, rng.randrange(23))))
        for i in range(32):
            primitive = PRIMITIVES + i * 0x3c
            put(primitive + 4, 'I', rng.choice((0, 0, 1, 2, 0x100, 0xffffffff)))
            put(primitive + 0xc, 'I', rng.choice((0, 1, 3, 600, 0xffffffff)))
            put(primitive + 0x1c, 'B', rng.randrange(256))
            put(primitive + 0x20, 'B', rng.choice((0, 1, 7, 0x80, 0xfe, 0xff)))
            put(primitive + 0x38, 'I', rng.choice((0, 1, 0x100, 0xffffffff)))
        for i in range(8):
            texture, state = TEXTURES + i * 0x100, STATES + i * 0x100
            # Two texture records can share the cached GPU state object.
            put(texture, 'I', STATES if index % 5 == 0 else state)
            put(texture + 0x24, 'I', rng.choice((0x1234, 0x6d57415, 0xbd3d01a8, 0xffffffff, 0x4567)))
            # -1 in the first bank aliases dispatch configuration globals;
            # -2 and -128 test signed indexing without corrupting that state.
            priority = rng.choice((-128, -2, 0, 1, 7, 127)) if index % 8 == 0 else rng.randrange(8)
            bank = rng.choice((0, 1, 2, 255))
            put(texture + 0x51, 'b', priority)
            put(texture + 0x54, 'B', bank)
            put(texture + 0x56, 'B', rng.choice((0, 1, 2, 2, 3, 255)))
            put(state + 0x1c, 'I', 0)
            # Each potential destination list is initialized, including signed
            # priority extremes; the actual retail producer range is unproven.
            for t in range(23):
                head = (0x870d30 if bank == 0 else 0x8712f0) + (priority * 23 + t) * 8
                heads[head] = struct.pack('<2I', head, head)
            put(OVERRIDES + i * 0x10, '4I', rng.randrange(4), rng.choice((0, 0, 1, rng.getrandbits(32))),
                rng.choice((0xffffffff, texture, TEXTURES)),
                rng.choice((0, 0, SLOTS + (i % 8) * 8 + 4)))
        for i in range(256):
            put(SLOTS + i * 8, '2I', rng.randrange(4), TEXTURES + (i % 8) * 0x100)
            put(LIGHTS + i * 8, '2I', rng.getrandbits(32), ARENA + 0xc000 + (i % 8) * 0x100)
        flags = rng.choice((0, 4, 0x200, 0x204, 0x401, rng.getrandbits(32)))
        record, zero = rng.choice((0, 0, ARENA + 0xe200)), rng.getrandbits(32)
        calls = dict(apply=[(SOLID, [])], restore=[(SOLID, [])], default=[(0, [])], mode=[(0, [])],
                     dispatch=[(VIEW, [SOLID, WORLD, record, flags, zero])], queue=[])
        # Applying textures requires a nonnull table even when it returns early.
        # Separate fixtures cover the dispatcher's null-table early return.
        calls['apply'] = [(SOLID + 0x40, [])]
        data[0x40:0x58] = data[0:0x18]
        put(SOLID + 0x4c, 'I', TABLE)
        for i in range(12):
            calls['queue'].append((0, [PRIMITIVES + i * 0x3c, TABLE, rng.getrandbits(32),
                TECHNIQUES + rng.choice((0, 0, 1, 5, 22)) * 0x20, TEXTURES + (i % 3) * 0x100,
                WORLD, record, ARENA + 0xc000, zero]))
        if index < 128:
            # Deliberate debug-remap matrix: preserve the pre-remap type while
            # varying the selected type and material. Random draws alone can
            # miss this branch behind the enable, culling and texture gates.
            initial, selected = 2 + index % 2, (index // 2) % 9
            put(SOLID + 8, '3Ii', 0, TABLE, 0, 0)
            put(PLATFORM + 0x10, 'iI', 8, PRIMITIVES)
            put(PLATFORM + 0x38, '2I', REMAP, TECHNIQUES + initial * 0x20)
            put(REMAP + initial * 4, 'i', selected)
            put(MODE, 'I', 0)
            global_(0x8707cd, 'B', 1)
            global_(0x870cc4, 'I', 0)
            global_(0x870cd4, 'I', 1)
            global_(0x870a18, 'I', 0x9999)
            for t in range(23): put(TECHNIQUES + t * 0x20 + 0x10, 'I', 1)
            for i in range(8):
                put(PRIMITIVES + i * 0x3c + 0x1c, 'B', i)
                put(TEXTURES + i * 0x100 + 0x24, 'I', 0x6d57415 if i == 7 else 0x4567)
                put(TEXTURES + i * 0x100 + 0x56, 'B', ((index // 18) + i) % 4)
            calls['dispatch'] = [(VIEW, [SOLID, WORLD, 0, 4, zero])]
        yield bytes(data), globals_, heads, calls


def verify(image, linked, gcc, n, kinds):
    machines = Machine(image), Machine(image, linked, gcc)
    totals, events = Counter(), Counter()
    for index, case in enumerate(cases(n)):
        for kind in kinds:
            try:
                a, b = [m.run(kind, case) for m in machines]
            except UcError as error:
                raise AssertionError(('solid mismatch', kind, index, 'emulation failure', str(error))) from error
            if a != b:
                offsets = [(hex(i), x, y) for i, (x, y) in enumerate(zip(a[0], b[0])) if x != y][:8]
                changed_events = [(i, x, y) for i, (x, y) in enumerate(zip(a[3], b[3])) if x != y][:1]
                raise AssertionError(('solid mismatch', kind, index, offsets, a[2], b[2],
                                      len(a[3]), len(b[3]), changed_events))
            totals[kind] += 1
            if kind in ('dispatch', 'queue'): events.update((kind + ':' + e[0]) for e in a[3])
    return dict(comparisons=dict(totals), observed_events=dict(events))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--cases', type=int, default=2048)
    p.add_argument('--kind', choices=['all'] + list(ORIGINAL), default='all')
    p.add_argument('--replace', type=Path)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[off:off + f['size']]).hexdigest() == f['sha256']
    linked = build(args.replace or SOURCE, args.gcc)
    kinds = list(ORIGINAL) if args.kind == 'all' else [args.kind]
    report = verify(image, linked, args.gcc, args.cases, kinds)
    report.update(failures=0, byte_identity='not established',
                  compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['bounded synthetic records; no native GL activation or retail dispatch capture',
                               'bounds, texture lookup and successful allocation are external boundaries',
                               'GPU consumer, original solid/table construction and technique setup remain open'])
    if args.mutation_checks:
        mutations = {
            'skip-texture-lookup': ('apply', 'if (overrides[i].value08 ==', 'if (false && overrides[i].value08 =='),
            'unsigned-texture-count': ('apply', 'int textureCount = table->textureCount19;',
                                       'int textureCount = (unsigned char)table->textureCount19;'),
            'override-zero-hash': ('apply', 'entry->hash04 != 0 && ', ''),
            'omit-restore-value': ('restore', 'entry->value08 = old;', 'entry->value08 = 0;'),
            'wrong-special-mode': ('mode', 'ug2SolidMode8A1C8C == 4', 'ug2SolidMode8A1C8C == 3'),
            'ignore-initial-technique': ('dispatch', 'int initialType = technique->type04;', 'int initialType = 0;'),
            'retain-remap': ('dispatch', 'platform->remap38 = 0;', '(void)0;'),
            'wrong-cull-flag': ('dispatch', '(flags & 4)', '(flags & 8)'),
            'skip-texture-filter': ('dispatch', 'if (texture->hash24 == ug2SolidSkipTexture870A18)', 'if (false)'),
            'wrong-extra-order': ('dispatch', 'candidate->type04 == 5 && !record', 'candidate->type04 == 22 && !record'),
            'wrong-special-texture': ('dispatch', 'texture->hash24 == 0xbd3d01a8', 'texture->hash24 == 0xbd3d01a9'),
            'ignore-group-technique': ('queue', 'state->group1C->technique14 != technique', 'false'),
            'unsigned-priority': ('queue', 'texture->priority51 * 23', '(unsigned char)texture->priority51 * 23'),
            'wrong-queue-bank': ('queue', 'texture->value54 == 0', 'texture->value54 != 0'),
            'wrong-index-total': ('queue', 'group->indexCount0C += primitive->indexCount0C;', '++group->indexCount0C;'),
        }
        rejected = []
        for name, (kind, before, after) in mutations.items():
            if kind not in kinds: continue
            body = SOURCE.read_text()
            assert before in body, name
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(body.replace(before, after))
            command = [sys.executable, __file__, '--replace', str(path), '--kind', kind, '--cases', '512']
            if args.gcc: command.append('--gcc')
            result = subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout)
            assert result.returncode and 'solid mismatch' in result.stdout, (name, result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + '-' + args.kind
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
