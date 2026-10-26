#!/usr/bin/env python3
"""Exact original/rebuilt light-tree, admission and sector-selector checks.

All recovered dependencies run as machine code, with real intrusive lists and
recursive tree records. No selector, admission, traversal or arithmetic stubs.
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
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderLightSelect.cpp'
TREE = ROOT / 'src/Speed/Indep/Src/World/CarRenderLightTree.cpp'
BUILD = ROOT / 'build/analysis/car-render-light-select'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-light-select.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x12000, 0x2100000, 0x2110000
OUTPUT, POSITION, STATE, LIGHT = [ARENA + i for i in (0x100, 0x300, 0x600, 0x900)]
SECTORS, EXTRA_SECTORS, TREES, NODES, LIGHTS, LINKS = [ARENA + i for i in (0x1000, 0x1800, 0x2000, 0x3000, 0x6000, 0xc000)]
HEAD, EXTRA_HEAD = 0x830d14, 0x835850
ORIGINAL = {'end': 0x502fd0, 'contains': 0x5746e0, 'descend': 0x574730,
            'find': 0x5747e0, 'admit': 0x4893a0, 'select': 0x48e3d0}
GCC = {'end': '_ZN18CarRenderLightList3EndEv', 'contains': '_ZN22CarRenderLightTreeNode8ContainsEfff',
       'descend': '_ZN18CarRenderLightTree7DescendEP22CarRenderLightTreeNodefff',
       'find': '_ZN18CarRenderLightTree4FindEfff',
       'admit': '_Z19CarRenderAdmitLightP23CarRenderLightSelectionP25CarRenderLightSelectStateP14CarRenderLightfff',
       'select': '_Z21CarRenderSelectLightsP23CarRenderLightSelectionPKf'}
PREFIX = {'end': '?End@CarRenderLightList@@', 'contains': '?Contains@CarRenderLightTreeNode@@',
          'descend': '?Descend@CarRenderLightTree@@', 'find': '?Find@CarRenderLightTree@@',
          'admit': '?CarRenderAdmitLight@@', 'select': '?CarRenderSelectLights@@'}


def build(source, tree, gcc):
    if gcc:
        linked = link_native([source, tree], BUILD, ROOT,
                             {'ug2LightSectors830D14': HEAD, 'ug2AdditionalLights835850': EXTRA_HEAD}, GCC['select'])
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        objects = []
        for i, path in enumerate((source, tree)):
            obj = BUILD / ('select-' + str(i) + '.obj')
            subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                            '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                            '/Fo' + str(obj.relative_to(ROOT)), str(path.relative_to(ROOT))], cwd=ROOT, check=True)
            objects.append(obj)
        linked = link_objects(objects, {'__fltused': 0, '_ug2LightSectors830D14': HEAD,
                                        '_ug2AdditionalLights835850': EXTRA_HEAD}, start=0x2400000)
    linked['entries'] = {}
    for kind, prefix in (GCC if gcc else PREFIX).items():
        linked['entries'][kind] = next(v for k, v in linked['exports'].items() if k.startswith(prefix))
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
            for address, data in linked['blocks']:
                u.mem_write(address, data)
        self.entries = linked['entries'] if linked else ORIGINAL
        self.gcc = gcc
        for kind, address in self.entries.items():
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=kind, begin=address, end=address)

    def args(self, n):
        sp = self.u.reg_read(xc.UC_X86_REG_ESP)
        return struct.unpack('<' + 'I' * n, bytes(self.u.mem_read(sp + 4, n * 4)))

    def hook(self, u, address, size, kind):
        if kind == 'admit':
            output, state, light, *xyz = self.args(6)
            nearest, current, end, weight, distance, _, count = struct.unpack('<7I', bytes(u.mem_read(state + 0xb0, 28)))
            used = current - (state + 0x10)
            assert 0 <= used <= 160
            self.events.append((kind, output, light, tuple(xyz),
                                bytes(u.mem_read(light, 0x60)), bytes(u.mem_read(nearest, 16)),
                                bytes(u.mem_read(state + 0x10, used)), used, end - (state + 0x10), weight, distance, count))
        elif kind == 'select':
            self.events.append((kind, self.args(2)))
        else:
            n = {'end': 0, 'contains': 3, 'descend': 4, 'find': 3}[kind]
            if self.gcc:
                receiver, *args = self.args(n + 1)
            else:
                receiver, args = u.reg_read(xc.UC_X86_REG_ECX), self.args(n)
            self.events.append((kind, receiver, tuple(args)))

    def run(self, kind, case, cw):
        data, heads, arguments = case
        receiver, args = arguments[kind]
        u = self.u
        self.events = []
        u.mem_write(ARENA, data)
        for address, body in heads.items():
            u.mem_write(address, body)
        sp = STACK + 0x8000
        actual = ([receiver] + args) if self.gcc and receiver else args
        u.mem_write(sp, struct.pack('<' + 'I' * (len(actual) + 1), STOP, *actual))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        u.reg_write(xc.UC_X86_REG_ECX, receiver)
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, r in enumerate(saved):
            u.reg_write(r, 0xabcdef00 + i)
        u.reg_write(xc.UC_X86_REG_FPCW, cw)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entries[kind], STOP, count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp + 4 + (len(args) * 4 if receiver and not self.gcc else 0)
        assert [u.reg_read(r) for r in saved] == [0xabcdef00 + i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        result = u.reg_read(xc.UC_X86_REG_EAX) if kind != 'admit' else None
        return bytes(u.mem_read(ARENA, SIZE)), [bytes(u.mem_read(h, 8)) for h in (HEAD, EXTRA_HEAD)], result, self.events


def fbits(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]


def cases(n):
    rng = random.Random(0x48e3d0)
    for index in range(n):
        data = bytearray(rng.randbytes(SIZE))
        def put(address, fmt, *values):
            struct.pack_into('<' + fmt, data, address - ARENA, *values)
        xyz = [rng.uniform(-8, 8) for _ in range(3)]
        if index % 8 == 0:
            xyz = [0.0, 0.0, 0.0]
        put(POSITION, '3f', *xyz)
        # Each tree has recursive branches, overlapping leaves and a signed
        # negative child index. Strict boundary/outside probes select different paths.
        for s in range(4):
            base = NODES + s * 0x400 + 0x30
            put(TREES + s * 4, 'I', 0 if index % 11 == s else base)
            for j in range(-1, 7):
                center = [0.0, 0.0, 0.0]
                extents = [100.0, 100.0, 100.0] if j == 0 else [rng.choice((2.0, 8.0, 10.0, 20.0)) for _ in range(3)]
                children = [1, 2, -1] if j == 0 else [3, 4] if j == 1 else [rng.randint(-3, 6) for _ in range(10)]
                count = len(children) if j in (0, 1) else -len(children)
                if index % 9 == 0 and j == 0:
                    count = 0
                put(base + j * 0x30, '3fHh3f10h', *center, 0, count, *extents, *(children + [0] * (10 - len(children))))
            light_base = LIGHTS + s * 0x600 + 4 * 0x60
            put(SECTORS + s * 0x100 + 0x10, 'I', TREES + s * 4)
            put(SECTORS + s * 0x100 + 0x18, 'I', light_base)
            for j in range(-3, 7):
                address = light_base + j * 0x60
                kind = rng.choice((0, 0, 2, 3, 1, 4))
                if index % 5 == 1: kind = 0
                if index % 5 == 2: kind = rng.choice((2, 3))
                put(address + 4, '4B', kind, rng.choice((0, 1, 2, 2)), 0, rng.choice((0, 1, 1, 1, 2)))
                put(address + 0xc, 'I', rng.getrandbits(32))
                pos = xyz if index % 6 == 0 else [v + rng.uniform(-10, 10) for v in xyz]
                put(address + 0x10, '4f', *pos, rng.choice((0.0, 1.0, 10.0, 100.0, -20.0)))
                put(address + 0x2c, '3f', rng.uniform(-2, 2), rng.choice((0.0, 2.0, 10.0, -4.0, rng.uniform(0.1, 5.0))), 0.5)
        heads = {}
        for head, sector_base, count in ((HEAD, SECTORS, index % 5), (EXTRA_HEAD, EXTRA_SECTORS, index % 3)):
            heads[head] = struct.pack('<2I', sector_base if count else head, sector_base + (count - 1) * 0x100 if count else head)
            for s in range(count):
                address = sector_base + s * 0x100
                put(address, '2I', address + 0x100 if s + 1 < count else head, address - 0x100 if s else head)
                if head == EXTRA_HEAD:
                    link_base = LINKS + s * 0x2000
                    amount = index % 31
                    put(address + 0x28, 'I', 0 if index % 7 == 0 else 1)
                    put(address + 0x2c, '2I', link_base if amount else (0 if index % 2 else address + 0x2c), 0)
                    for j in range(amount):
                        link = link_base + j * 0x68
                        put(link, '2I', link + 0x68 if j + 1 < amount else (0 if index % 2 else address + 0x2c), 0)
                        source = LIGHTS + (j % 10 + 1) * 0x60
                        data[link + 8 - ARENA:link + 8 - ARENA + 0x60] = data[source - ARENA:source - ARENA + 0x60]
        # Standalone admission starts from a fully initialized, sometimes full
        # state, and includes exact radius/equality probes at simple magnitudes.
        source = LIGHTS + (index % 10 + 1) * 0x60
        data[LIGHT - ARENA:LIGHT - ARENA + 0x60] = data[source - ARENA:source - ARENA + 0x60]
        if index % 4 == 0:
            put(LIGHT + 4, '4B', (0, 2, 3, 4)[(index // 4) % 4], 2, 0, 1)
            put(LIGHT + 0x10, '4f', 0.0, 0.0, 0.0, 10.0)
            put(LIGHT + 0x30, 'f', 2.0)
            xyz = [rng.choice((0.0, 2.0, 9.999999, 10.0, 10.000001)), 0.0, 0.0]
        elif index % 4 == 1:
            # A fractional inner radius exercises its unrounded square in the
            # transition shell, rather than only exact small integer squares.
            inner = rng.uniform(2.0, 9.0)
            put(LIGHT + 4, '4B', 2, 0, 0, 1)
            put(LIGHT + 0x10, '4f', 0.0, 0.0, 0.0, 10.0)
            put(LIGHT + 0x30, 'f', inner)
            xyz = [rng.uniform(inner, 10.0), 0.0, 0.0]
        for j in range(44):
            put(STATE + j * 4, 'f', rng.uniform(-2, 2))
        used = index % 11
        put(STATE + 0xb0, '3I2fIi', STATE, STATE + 0x10 + used * 16, STATE + 0xb0,
            rng.choice((0.0, 0.5, 1.0, 2.0)), rng.choice((0.0, 1.0, 1e30)), rng.getrandbits(32), index % 26)
        bits = [fbits(v) for v in xyz]
        node = NODES + 0x30
        put(TREES + 0x10, 'I', node)  # Direct descent always has a valid node array.
        if index % 8 == 0:
            # Test root strict edge, signed zero and immediately adjacent floats.
            bits = [fbits(rng.choice((100.0, -100.0, 99.99999, 100.00001, -0.0))), 0, 0]
        arguments = {'end': (HEAD, []), 'contains': (node, bits), 'descend': (TREES + 0x10, [node] + bits),
                     'find': (TREES, bits), 'admit': (0, [OUTPUT, STATE, LIGHT] + [fbits(v) for v in xyz]),
                     'select': (0, [OUTPUT, POSITION])}
        yield bytes(data), heads, arguments


def verify(image, linked, gcc, kinds, n):
    a, b = Machine(image), Machine(image, linked, gcc)
    totals, calls = Counter(), Counter()
    for index, case in enumerate(cases(n)):
        for kind in kinds:
            for cw in (0x027f, 0x037f):
                x, y = a.run(kind, case, cw), b.run(kind, case, cw)
                if x != y:
                    offsets = sorted({j & ~3 for j, (u, v) in enumerate(zip(x[0], y[0])) if u != v})[:6]
                    details = [(hex(j), x[0][j:j + 4].hex(), y[0][j:j + 4].hex()) for j in offsets]
                    events = [(j, u, v) for j, (u, v) in enumerate(zip(x[3], y[3])) if u != v][:1]
                    raise AssertionError(('selection mismatch', kind, index, hex(cw), details, x[2], y[2], len(x[3]), len(y[3]), events))
                totals[kind] += 1
                if kind == 'select': calls.update(e[0] for e in x[3])
    return dict(comparisons=dict(totals), selector_calls=dict(calls))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc', action='store_true')
    p.add_argument('--cases', type=int, default=2048)
    p.add_argument('--kind', choices=['all'] + list(ORIGINAL), default='all')
    p.add_argument('--replace', type=Path)
    p.add_argument('--replace-tree', type=Path)
    p.add_argument('--mutation-checks', action='store_true')
    args = p.parse_args()
    assert args.cases > 0
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        off = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[off:off + f['size']]).hexdigest() == f['sha256']
    linked = build(args.replace or SOURCE, args.replace_tree or TREE, args.gcc)
    kinds = list(ORIGINAL) if args.kind == 'all' else [args.kind]
    report = verify(image, linked, args.gcc, kinds, args.cases)
    report.update(failures=0, byte_identity='not established',
                  compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
                  limitations=['finite valid list/tree fixtures; not exhaustive or a retail traversal capture',
                               'native light-list/tree population and source-record activation remain open'])
    if args.mutation_checks:
        mutations = {
            'inclusive-bounds': (TREE, 'find', '<= LIGHT_ABS', '< LIGHT_ABS'),
            'skip-tree-descent': (TREE, 'find', 'return Descend(root, x, y, z);', 'return root;'),
            'reverse-child-search': (TREE, 'find', 'node->indices1C[i]', 'node->indices1C[count - 1 - i]'),
            'wrong-active-byte': (SOURCE, 'admit', 'light->active07 != 1', 'light->active07 == 1'),
            'inclusive-radius': (SOURCE, 'admit', 'distance < outer', 'distance <= outer'),
            'round-inner-square': (SOURCE, 'admit', 'const long double inner =', 'volatile float inner ='),
            'wrong-ambient-mode': (SOURCE, 'select', 'mode == 2', 'mode == 1'),
            'wrong-dynamic-limit': (SOURCE, 'admit', 'state->countC8 < 24', 'state->countC8 < 10'),
            'wrong-sector-limit': (SOURCE, 'select', 'state.countC8 < 10', 'state.countC8 < 9'),
            'skip-additional-lists': (SOURCE, 'select', 'if (sector->flag28)', 'if (false && sector->flag28)'),
            'wrong-ambient-scale': (SOURCE, 'select', 'scale = static_cast<long double>(light784250) / state.weightBC;', 'scale = 1.0f;'),
            'round-ambient-sums': (SOURCE, 'select', 'long double r =', 'volatile float r ='),
        }
        rejected = []
        for name, (source, kind, before, after) in mutations.items():
            if kind not in kinds: continue
            body = source.read_text()
            assert before in body
            path = BUILD / ('mutation-' + name + '.cpp')
            path.write_text(body.replace(before, after))
            command = [sys.executable, __file__, '--replace-tree' if source == TREE else '--replace', str(path),
                       '--kind', kind, '--cases', '512'] + (['--gcc'] if args.gcc else [])
            result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout)
            assert result.returncode and 'selection mismatch' in result.stdout, (name, result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    label = ('gcc' if args.gcc else 'msvc') + '-' + args.kind
    (BUILD / ('verification-' + label + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
