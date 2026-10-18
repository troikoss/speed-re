#!/usr/bin/env python3
"""Compare the complete FEng script reader 0x5515e0 over generated Scrp tag streams.

Leaf lookups and script construction execute original code in both runs; pools,
name/track/event storage, assignment, heap and CRT array iterators are
controlled, traced boundaries whose bodies are verified by other tools."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-script-reader'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, ARENA_SIZE = 0x2000000, 0x40000
READER, OBJECT, EXISTING, REGISTRY, DESCRIPTORS, PROPERTIES = (ARENA + x for x in (0, 0x100, 0x1000, 0x2000, 0x2100, 0x3000))
DATA, HEADERS, KEYS, FRESH = (ARENA + x for x in (0x8000, 0x10000, 0x18000, 0x20000))
STACK, STOP, TEB, GDT = 0x2080000, 0x2100000, 0x2140000, 0x2150000
HEADER_POOL, KEY_POOL = 0x839c30, 0x839bd0
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
LEAVES = {'FindScriptByHash': ('feng-response.json', '?FindScriptByHash@HudVisibilityObject@'),
          'ScriptConstruct': ('feng-script-tracks.json', '?FEngConstructScript@'),
          'FindTrack': ('feng-track-lookup.json', '?FindTrack@FEScript@'),
          'FindDescriptor': ('feng-track-lookup.json', '?Find@FEngTypeRegistry@'),
          'GetProperty': ('feng-track-lookup.json', '?GetProperty@FEngTypeDescriptor@')}
# name: (address, symbol prefix, stack arguments, callee cleanup)
STUBS = {'HeaderAcquire': (0x54beb0, '?Acquire@FEngScriptPool@', 0, True),
         'KeyAcquire': (0x53e650, '?Acquire@FEngScriptKeyPool@', 0, True),
         'SetName': (0x518730, '?SetName@FEScript@', 1, True),
         'SetTracks': (0x535210, '?SetTracks@FEScript@', 1, True),
         'TrackAssign': (0x54d350, '?Assign@FEScriptTrack@', 1, True),
         'ResizeEvents': (0x519a10, '?Resize@FEngScriptEventStorage@', 1, True),
         'Allocate': (0x575620, '?FEngAllocateQueueNode@', 1, False),
         'Free': (0x575630, '?FEngFreeQueueNode@', 1, False),
         'ConstructArray': (0x75c461, '?FEngConstructArrayWithCleanup@', 5, True),
         'DestructArray': (0x75c401, '?FEngDestructArrayWithCleanup@', 4, True)}
VALUES = {'?FEngScriptHeaderPool@': HEADER_POOL, '?FEngScriptKeysPool@': KEY_POOL,
          '?FEngConstructScriptTrack@': 0x525e80, '?FEngDestructScriptTrack@': 0x52b960}
INIT = 0x1744b3
PREKEYS = ARENA + 0x6000


def reference_state(rng, put, track, slot):
    """Constructed track key reference +0x28: empty, owned 1-2 node list, or borrowed flag."""
    choice = rng.random()
    if choice < 0.5: flag, head, tail = 0, 0, 0
    elif choice < 0.8:
        head = PREKEYS + slot * 0x40; tail = head + 0x20 if rng.random() < 0.5 else head
        put(head + 4, tail if tail != head else 0); put(head + 8, 0)
        if tail != head: put(tail + 4, 0); put(tail + 8, head)
        flag = 0
    else: flag, head, tail = rng.choice((1, 0x80, 0xff)), rng.getrandbits(32), rng.getrandbits(32)
    put(track + 0x28, 0x79aa30); put(track + 0x2c, flag); put(track + 0x30, head); put(track + 0x34, tail)
    # Value sizes stay within the 16-byte key value; larger sizes overrun +0x28 in the original too.
    put(track, (rng.getrandbits(32) & 0xffff00ff) | rng.choice((0, 4, 8, 12, 16, 3, 13)) << 8)


class Stream:
    """Generate tag streams that stay within the original's unchecked dereferences."""
    def __init__(self, rng, existing):
        self.rng, self.existing = rng, existing
        self.script = None    # None, 'new' (no tracks yet) or 'tracks'
        self.current = None   # index of an existing (object-list) script made current by SI
        self.track = False
        self.out = bytearray()

    def emit(self, tag, payload):
        self.out += struct.pack('<HH', tag, len(payload)) + payload

    def step(self):
        rng = self.rng; choices = ['Sn', 'Sh', 'Sf', 'Ti', 'Tt', 'Ta', 'unknown']
        if self.script: choices += ['Sc', 'Sl', 'EV', 'SI']
        if self.script == 'tracks': choices += ['FI'] * 4
        if self.track: choices += ['To', 'Tb', 'Kd', 'Kd']
        tag = rng.choice(choices); words = lambda *v: struct.pack('<%dI' % len(v), *[x & 0xffffffff for x in v])
        if tag == 'Sn':
            name = rng.choice((b'', b'Init', b'FadeIn', bytes(rng.randrange(1, 255) for _ in range(rng.randrange(1, 9)))))
            self.emit(0x6e53, name + b'\0' * (4 - len(name) % 4)); self.script = 'new'; self.current = None
        elif tag == 'Sh':
            count = rng.randrange(0, 7); value = INIT if rng.random() < 0.3 else rng.getrandbits(32)
            self.emit(0x6853, words(value, rng.randrange(-5, 3000), rng.getrandbits(32), count))
            if self.current is not None: self.existing[self.current] = value  # Sh rewrites an SI-selected script's hash
            self.script = 'tracks' if count else 'new'  # SetTracks(0) leaves no table for FI
        elif tag == 'Sf': self.emit(0x6653, words(rng.getrandbits(32)))
        elif tag == 'Sc': self.emit(0x6353, words(rng.getrandbits(32)))
        elif tag == 'Sl': self.emit(0x6c53, words(rng.getrandbits(32)))
        elif tag == 'Ti':
            self.emit(0x6954, struct.pack('<H', rng.randrange(11) if rng.random() < 0.9 else rng.randrange(11, 20)) + rng.randbytes(rng.choice((0, 2))))
            if self.script: self.script, self.track = 'tracks', True
        elif tag in ('Tt', 'Ta'):
            if self.script and not self.track: return
            self.emit(0x7454 if tag == 'Tt' else 0x6154, rng.randbytes(rng.choice((1, 4))))
        elif tag == 'EV':
            self.emit(0x5645, rng.randbytes(12 * rng.randrange(1, 5) + rng.choice((0, 0, 4, 11))))
        elif tag == 'SI':
            hit = bool(self.existing) and rng.random() < 0.8
            value = rng.choice(self.existing) if hit else 0xdeadbeef  # never an existing hash
            self.emit(0x4953, words(value))
            self.current = self.existing.index(value) if hit else None  # first match, as 0x5046b0
            self.script, self.track = ('tracks', False) if hit else (None, False)
        elif tag == 'FI':
            self.emit(0x4946, bytes((rng.randrange(256), rng.choice((4, 8, 12, 16, rng.randrange(17))), rng.randrange(256), rng.randrange(256))) + rng.randbytes(4))
            self.track = True
        elif tag == 'To': self.emit(0x6f54, rng.randbytes(rng.choice((1, 4))))
        elif tag == 'Tb': self.emit(0x6254, rng.randbytes(rng.choice((4, 8, 20, 68))))
        elif tag == 'Kd': self.emit(0x644b, rng.randbytes(rng.choice((0, 4, 8, 20, 40, rng.randrange(0, 160)))))
        else:
            value = rng.choice((0x4954, 0x6954 + 1, 0x7453, rng.getrandbits(16)))
            if value in (0x6653, 0x6154, 0x4946, 0x4953, 0x5645, 0x6254, 0x6353, 0x644b, 0x6e53, 0x6853, 0x6954, 0x6c53, 0x6f54, 0x7454): value = 0x1234
            self.emit(value, rng.randbytes(rng.randrange(0, 12)))

    def build(self):
        for _ in range(self.rng.randrange(1, 14)): self.step()
        if not self.script: self.emit(0x6853, struct.pack('<4I', INIT, 100, 0, 1))
        return bytes(self.out)


def fixture(rng):
    memory = bytearray(rng.randbytes(ARENA_SIZE))
    put = lambda a, v: struct.pack_into('<I', memory, a - ARENA, v & 0xffffffff)
    type18 = rng.choice((1, 2, 5))
    memory[READER - ARENA + 0xa] = rng.choice((0, 1, 0x80))
    put(READER + 0xc, OBJECT); memory[READER - ARENA + 0x18] = rng.choice((0, 0, 1, 0xff))
    put(READER + 0x3c, REGISTRY - 0x4134)
    put(OBJECT + 0x18, type18)
    hashes = []
    count = rng.randrange(0, 4)
    for i in range(count):
        header = EXISTING + i * 0x80; tracks = EXISTING + 0x400 + i * 0x400; n = rng.randrange(1, 5)
        hashes.append(rng.choice((INIT, rng.getrandbits(32))))
        put(header + 4, EXISTING + (i + 1) * 0x80 if i + 1 < count else 0)
        put(header + 8, EXISTING + (i - 1) * 0x80 if i else 0)
        put(header + 0x1c, n); put(header + 0x20, tracks if n else 0); put(tracks - 4, n); put(header + 0x30, hashes[-1])
        for k in range(n): reference_state(rng, put, tracks + k * 0x38, i * 8 + k)
    put(OBJECT + 0x48, count); put(OBJECT + 0x4c, EXISTING if count else 0)
    put(OBJECT + 0x50, EXISTING + (count - 1) * 0x80 if count else 0)
    # Registry: the object's type is always present; properties cover selectors 0..23.
    types = [rng.choice((0, 3, 4, 6)) for _ in range(3)] + [type18]; rng.shuffle(types)
    put(REGISTRY + 8, DESCRIPTORS)
    for i, value in enumerate(types):
        descriptor = DESCRIPTORS + i * 0x40
        put(descriptor + 4, descriptor + 0x40 if i + 1 < len(types) else 0); put(descriptor + 0x24, value)
        put(descriptor + 0x1c, PROPERTIES + i * 0x400)
        for j in range(24):
            prop = PROPERTIES + i * 0x400 + j * 0x20
            put(prop + 4, prop + 0x20 if j < 23 else 0)
            put(prop + 0x18, rng.getrandbits(32) & ~0xff | rng.choice((4, 8, 12, 16, rng.randrange(17))))
            put(prop + 0x1c, rng.choice((0, 4, 0x10, 0x1c, 0x28, 0x38, 0x44, 0x54, -4, rng.randrange(-0x200, 0x200))))
    stream = Stream(rng, list(hashes)).build()
    memory[DATA - ARENA:DATA - ARENA + len(stream)] = stream
    return memory, len(stream)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngScriptReader.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    config = ROOT / 'config/SPEED2_EXE_1_2'
    manifest = json.loads((config / 'feng-script-reader.json').read_text())
    functions = dict(manifest['functions'])
    for name, (file, _) in LEAVES.items(): functions[name] = json.loads((config / file).read_text())['functions'][name]
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in functions.values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    prefixes = dict(VALUES); prefixes.update({p: int(functions[n]['address'], 16) for n, (_, p) in LEAVES.items()})
    prefixes.update({p: a for a, p, _, _ in STUBS.values()})
    externals = {}
    for symbol in read_object(obj)['symbols'].values():
        if symbol['section'] != 0: continue
        match = [a for p, a in prefixes.items() if symbol['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', symbol['name']); externals[symbol['name']] = match[0]
    linked = link_objects([obj], externals)
    entry = next(a for s, a in linked['exports'].items() if s.startswith('?Read@FEngScriptReader@'))
    leaf_ranges = [(int(functions[n]['address'], 16), int(functions[n]['address'], 16) + functions[n]['size']) for n in LEAVES]
    stubs = {a: (n, argc, cleanup) for n, (a, _, argc, cleanup) in STUBS.items()}
    state, machines, hits = {}, [], dict.fromkeys(list(STUBS) + list(LEAVES), 0)
    leaf_names = {int(functions[n]['address'], 16): n for n in LEAVES}
    def words(uc, a, count=1): return struct.unpack('<%dI' % count, uc.mem_read(a, count * 4))
    def put(uc, a, value): uc.mem_write(a, struct.pack('<I', value & 0xffffffff))
    def digest(uc): return hashlib.sha256(bytes(uc.mem_read(ARENA, ARENA_SIZE))).hexdigest()
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x60000); uc.mem_map(STACK - 0x8000, 0x9000); uc.mem_map(STOP, 0x1000); uc.mem_map(TEB, 0x1000); uc.mem_map(GDT, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)): uc.reg_write(reg, selector)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        ranges = (linked['ranges'] if rebuilt else [(0x5515e0, 0x5515e0 + 1521)]) + leaf_ranges
        def hook(uc, address, size, user, ranges=ranges, original_run=not rebuilt):
            if address in leaf_names:
                if original_run: hits[leaf_names[address]] += 1
                name = leaf_names[address]
                argument = None if name == 'ScriptConstruct' else words(uc, uc.reg_read(xc.UC_X86_REG_ESP) + 4)[0]
                state['trace'].append((name, uc.reg_read(xc.UC_X86_REG_ECX), argument, digest(uc)))
            if address not in stubs:
                assert any(a <= address < b for a, b in ranges), ('unexpected original execution' if original_run else 'state or call order mismatch', hex(address)); return
            name, argc, cleanup = stubs[address]; sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if original_run: hits[name] += 1
            ecx = uc.reg_read(xc.UC_X86_REG_ECX); argv = words(uc, sp + 4, argc) if argc else ()
            if name in ('Allocate', 'Free', 'ConstructArray', 'DestructArray'): ecx = None
            state['trace'].append((name, ecx, argv, digest(uc)))
            result = 0xa5a5a5a5
            def fresh(size):
                region = FRESH + state['fresh'] * 0x1000; state['fresh'] += 1
                assert size <= 0x1000 and region + 0x1000 <= ARENA + ARENA_SIZE, 'fixture capacity'
                return region
            if name == 'HeaderAcquire':
                assert ecx == HEADER_POOL; result = HEADERS + state['headers'] * 0x40; state['headers'] += 1
            elif name == 'KeyAcquire':
                assert ecx == KEY_POOL; result = KEYS + state['keys'] * 0x20; state['keys'] += 1
                assert result < FRESH, 'fixture capacity'
            elif name == 'SetName':
                text = bytes(uc.mem_read(argv[0], 64)).split(b'\0')[0]
                put(uc, ecx + 0x2c, 0x5e700000 + len(text)); put(uc, ecx + 0x30, int.from_bytes(hashlib.sha256(text).digest()[:4], 'little'))
            elif name == 'SetTracks':
                region = fresh(0) + 4; put(uc, region - 4, argv[0]); put(uc, ecx + 0x1c, argv[0]); put(uc, ecx + 0x20, region if argv[0] else 0)
                local = random.Random(state['seed'] * 64 + state['fresh'])
                for k in range(min(argv[0], 64)): reference_state(local, lambda a, v: put(uc, a, v), region + k * 0x38, 32 + (state['fresh'] % 16) * 8 + k % 8)
            elif name == 'TrackAssign':
                uc.mem_write(ecx, bytes(uc.mem_read(argv[0], 0x38)))
            elif name == 'ResizeEvents':
                put(uc, ecx, argv[0]); put(uc, ecx + 4, fresh(argv[0] * 12))
            elif name == 'Allocate':
                result = fresh(argv[0])
            elif name == 'ConstructArray':
                assert argv[3:] == (0x525e80, 0x52b960), ('state or call order mismatch', 'array callbacks')
                for k in range(argv[2]):  # constructed empty reference, as 0x525e80
                    for offset, value in ((0x28, 0x79aa30), (0x2c, 0), (0x30, 0), (0x34, 0)): put(uc, argv[0] + k * 0x38 + offset, value)
            ret = words(uc, sp)[0]
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4 + (4 * argc if cleanup else 0)); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x5515e0); tags = {}
    for trial in range(args.cases):
        memory, size = fixture(rng)
        cursor = 0; stream = memory[DATA - ARENA:DATA - ARENA + size]
        while cursor + 4 <= size:
            tag, length = struct.unpack_from('<HH', stream, cursor); tags[tag] = tags.get(tag, 0) + 1; cursor += 4 + length
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); put(uc, TEB, 0xfeedf00d)
            uc.mem_write(STACK, struct.pack('<3I', STOP, DATA, size))
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, READER)
            state.clear(); state.update(trace=[], headers=0, keys=0, fresh=0, seed=trial)
            try: uc.emu_start(entry if rebuilt else 0x5515e0, STOP, count=500000)
            except UcError as error: raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', trial, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch', trial, 'nontermination')
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) & 0xff, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED),
                            words(uc, TEB)[0], bytes(uc.mem_read(ARENA, ARENA_SIZE)), state['trace']))
        assert outputs[0][:4] == (1, STACK + 12, tuple(0x12345000 + i for i in range(4)), 0xfeedf00d), ('original contract', trial)
        if outputs[0] != outputs[1]:
            diff = [hex(ARENA + i) for i, (a, b) in enumerate(zip(outputs[0][4], outputs[1][4])) if a != b][:12]
            raise AssertionError(('state or call order mismatch', trial, outputs[0][:4], outputs[1][:4], diff,
                                  [x[:3] for x in outputs[0][5]], [x[:3] for x in outputs[1][5]]))
    report = dict(compiler=args.compiler, comparisons=args.cases, tag_counts={struct.pack('<H', k).decode('latin1'): v for k, v in sorted(tags.items()) if v > 50},
                  boundary_hits=hits, executed_original_leaves=list(LEAVES), controlled_boundaries={n: hex(a) for n, (a, _, _, _) in STUBS.items()},
                  byte_matching=False, exceptional_unwind_verified=False,
                  limitations=['generated streams avoid the original\'s unchecked null script/track, empty-EV and missing-descriptor faults; SI\'s track reset is observable only through such faults',
                               'pool, name, SetTracks, Assign, Resize, heap and CRT iterator bodies controlled here; verified by their own tools',
                               'SEH frame registration compared only as restored FS:[0]', 'MSVC 7.1 and byte identity pending'],
                  fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'sf-unchecked': ('if (script) script->flags14 = payload[0];', 'script->flags14 = payload[0];'),
            'fi-duration-16bit': ('*bits ^= (payload[1] ^ *bits) & 0xffffff;', '*bits ^= (payload[1] ^ *bits) & 0xffff;'),
            'ev-short-copy': ('event->time = (int)source[2];', 'event->time = 0;'),
            'tb-omit-time': ('track->key8.timeC = (int)payload[0];', '/* omitted */'),
            'kd-keep-borrowed': ('            if (keys->value4) {', '            if (0) {'),
            'kd-pool-first-key': ('                if (index) {\n                    key = FEngScriptKeysPool.Acquire();', '                if (1) {\n                    key = FEngScriptKeysPool.Acquire();'),
            'kd-append-head': ('                    if (tail) {', '                    if (0) {'),
            'kd-while-loop': ('            unsigned int index = 0;\n            do {', '            unsigned int index = 0;\n            if (count) do {'),
            'sn-keep-index': ('            if (value0A) script->SetName((const char *)payload);\n            trackIndex = -1;', '            if (value0A) script->SetName((const char *)payload);'),
            'sn-always-name': ('if (value0A) script->SetName', 'script->SetName'),
            'sh-reset-existing': ('            script->hash30 = payload[0];\n            script->lengthC', '            trackIndex = -1;\n            script->hash30 = payload[0];\n            script->lengthC'),
            'sh-omit-prezero': ('                header->trackCount1C = 0;\n', ''),
            'ti-insert-at-end': ('(!property || (int)source->state7 < property->offset1C / 4)', '(!property || 1)'),
            'ti-unsigned-offset': ('property->offset1C / 4)', '(int)((unsigned int)property->offset1C >> 2))'),
            'ti-flags-one': ('destination->flags3 = 0;', 'destination->flags3 = 1;'),
            'ti-keep-found-track': ('                track = script->FindTrack((int)selector);\n                if (!track) {', '                FEScriptTrack *found = script->FindTrack((int)selector);\n                if (!found) {'),
            'ti-omit-free': ('                        FEngFreeQueueNode(base);\n', ''),
            'link-append-head': ('            if (last) {', '            if (0) {'),
            'link-ignore-flag': ('    if (!value18) {', '    if (1) {'),
            'link-omit-count': ('            ++scripts->count4;\n', ''),
            'init-wrong-hash': ('== 0x1744b3', '== 0x1744b4'),
            'tt-unchecked': ('if (script) track->value2', 'track->value2'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '1024'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
            assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
