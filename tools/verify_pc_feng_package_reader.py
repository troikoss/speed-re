#!/usr/bin/env python3
"""Compare the FEng package reader chain with the original code under Unicorn.

Recovered routines (original address): package reader 0x5554a0, object reader 0x555140,
object clone 0x554300, group clone 0x5548a0, group append 0x517e90, package append 0x495610,
package-list insert 0x518280, package loader 0x555860 and the Init-script visitor 0x517ee0.

The original routine and the rebuilt object run on identical randomized arenas. Every other
original callee (readers, factory, pools, state manager, name hashing, script/track code,
visitors) is real original code in both runs. Controlled boundaries: the heap 0x575620/0x575630
(deterministic bump allocator), the unrecovered package driver 0x555800, and the observer
slots 0 and 0xc at queue +0x10c. The ordered trace of callee entries with their arguments and an
arena digest is compared together with the returned value, preserved registers, stack cleanup,
FS:[0], the arena, the heap, the writable image data and the null page."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import zlib
from concurrent.futures import ThreadPoolExecutor
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-package-reader'
FRONT = ROOT / 'src/Speed/Indep/Src/Frontend'
ARENA, ARENA_SIZE = 0x2000000, 0x60000
HEAP, HEAP_SIZE = 0x2100000, 0x400000
STACK, STOP, STUBS, TEB, GDT = 0x2800000, 0x2900000, 0x2910000, 0x2920000, 0x2930000
READER, PKG, QUEUE, OBSERVER, VTABLE = (ARENA + x for x in (0, 0x100, 0x1000, 0x6400, 0x6420))
TOP_START = 0x6600
DATA_BEGIN, DATA_END = 0x7e8000, 0x7e8000 + 0xd35f8
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
INIT = 0x1744b3
CHUNK_K, CHUNK_EA, CHUNK_BUTN, CHUNK_OD, CHUNK_MSGR, CHUNK_SCRP = 0xcc6a624f, 0xea624f46, 0x6e747542, 0x446a624f, 0x5267734d, 0x70726353
GLOBALS = {'FEngNextObjectGuid': 0x7f7cf8, 'FEngObjectStateManager': 0x863770, 'FEngResponseHeaderPool': 0x8637a0,
           'FEngScriptHeaderPool': 0x839c30}
# Writable image globals that callees may touch and the case initialises (address, size).
RESET_GLOBALS = ((0x7f7cf8, 4), (0x863770, 0x10), (0x8637a0, 0x10), (0x839c30, 0x10), (0x839bd0, 0x10), (0x838474, 8), (0x7f7cc4, 4))

# Recovered routines: name -> (symbol prefix of the rebuilt entry, stack arguments, stack bytes cleaned, uses ecx)
ROOTS = {'Read': ('?Read@FEngPackageReader@', 0), 'ReadObject': ('?ReadObject@FEngPackageReader@', 2),
         'CloneObject': ('?CloneConstruct@HudVisibilityObject@', 2), 'CloneGroup': ('?CloneConstruct@FEngGroupObject@', 3),
         'AppendChild': ('?AppendChild@FEngGroupObject@', 1), 'AppendObject': ('?AppendObject@FEPackage@', 1),
         'InsertByLevel': ('?InsertByLevel@FEngPackageList@', 1), 'Call555860': ('?Call555860@FEngQueue@', 3),
         'InitScriptVisit': ('?Visit@FEngInitScriptVisitor@', 1)}
ORIGINAL_ROOT = {'Read': 0x5554a0, 'ReadObject': 0x555140, 'CloneObject': 0x554300, 'CloneGroup': 0x5548a0, 'AppendChild': 0x517e90,
                 'AppendObject': 0x495610, 'InsertByLevel': 0x518280, 'Call555860': 0x555860, 'InitScriptVisit': 0x517ee0}
MANIFEST_NAMES = {'CloneObject': 'CloneObject'}
# Real original callees that are traced: name -> (address, stack arguments, ecx is meaningful)
CALLEES = {
    'Insert': (0x5053b0, 2, True), 'Remove': (0x505400, 1, True), 'SetName': (0x5176b0, 1, True),
    'FindGuid': (0x517fe0, 1, True), 'Resolve': (0x5186c0, 3, True), 'Create': (0x534df0, 1, True),
    'ListProp': (0x52b740, 1, True), 'StringProp': (0x534fc0, 1, True), 'GridProp': (0x535060, 1, True),
    'ScriptRead': (0x5515e0, 2, True), 'RespRead': (0x54d1d0, 3, True), 'ApplyTrack': (0x541f60, 4, False),
    'CalcOffsets': (0x504360, 0, True), 'InitialFill': (0x504c40, 0, True), 'InitScript': (0x517720, 0, True),
    'ReleaseState': (0x517520, 1, True), 'AcquireState': (0x52ace0, 1, True), 'RespAcquire': (0x54bdf0, 0, True),
    'ScriptAcquire': (0x54beb0, 0, True), 'Resize': (0x52ae00, 1, True), 'Assign': (0x52ad80, 1, True),
    'CopyConstruct': (0x551be0, 2, True), 'FindByName': (0x519e50, 2, True), 'VisitObjects': (0x504880, 1, True)}
EXTERNALS = {'?InitializeScript@HudVisibilityObject@': 'InitScript', '?Insert@FEngQueueList@': 'Insert',
             '?ReleaseState@FEngStatePoolManager@': 'ReleaseState', '?AcquireState@FEngStatePoolManager@': 'AcquireState',
             '?SetName@HudVisibilityObject@': 'SetName', '?Acquire@FEngResponsePool@': 'RespAcquire',
             '?ResizeCommands@FEResponse@': 'Resize', '?Assign@FEResponseCommand@': 'Assign',
             '?Acquire@FEngScriptPool@': 'ScriptAcquire', '?CopyConstruct@FEScript@': 'CopyConstruct',
             '?Resolve@FEngInheritanceContext@': 'Resolve', '?Create@FEngObjectFactoryContext@': 'Create',
             '?FindObjectByGuid@FEPackage@': 'FindGuid', '?ReadProperty@FEngGridPropertyReader@': 'GridProp',
             '?ReadProperty@FEngStringPropertyReader@': 'StringProp', '?ReadProperty@FEngListPropertyReader@': 'ListProp',
             '?Read@FEngResponseReader@': 'RespRead', '?Read@FEngScriptReader@': 'ScriptRead',
             '?CalculateOffsets@FEListObject@': 'CalcOffsets', '?InitialFill@FEGridObject@': 'InitialFill',
             '?FEngApplyScriptTrack@@': 'ApplyTrack', '?FindByName@FEngPackageList@': 'FindByName',
             '?VisitObjects@FEPackage@': 'VisitObjects', '?Remove@FEngQueueList@': 'Remove'}
ALLOC, FREE, DRIVER = 0x575620, 0x575630, 0x555800
SLOT_LOAD, SLOT_LOADED = STUBS + 0x00, STUBS + 0x10
SLOT_TRAP = STUBS + 0x20
# Dependency fingerprints: callee name -> (manifest, function key) checked when the manifest names the address.
DEPENDENCY_ADDRESSES = sorted({a for a, _, _ in CALLEES.values()} | {ALLOC, FREE})


def cihash(name):
    value = 0xffffffff
    for c in name: value = (value * 33 + (c - 32 if 97 <= c <= 122 else c)) & 0xffffffff
    return value


class Arena:
    """Randomized arena with a bump allocator for hand-built original structures."""
    def __init__(self, rng):
        self.rng = rng
        self.mem = bytearray(rng.randbytes(ARENA_SIZE))
        self.top = TOP_START

    def alloc(self, size, align=16):
        self.top = (self.top + align - 1) & ~(align - 1)
        address = ARENA + self.top
        self.mem[self.top:self.top + size] = bytes(size)
        self.top += size
        assert self.top < ARENA_SIZE - 0x1000, 'arena exhausted'
        return address

    def put(self, address, fmt, *values):
        fixed = [v & 0xffffffff if fmt[i] in 'I' else v for i, v in enumerate(values)] if len(fmt) == len(values) else values
        struct.pack_into('<' + fmt, self.mem, address - ARENA, *fixed)

    def get(self, address, fmt='I'):
        return struct.unpack_from('<' + fmt, self.mem, address - ARENA)[0]

    def string(self, text):
        address = self.alloc(len(text) + 1, 1)
        self.mem[address - ARENA:address - ARENA + len(text)] = text
        return address

    def blob(self, data, align=4):
        address = self.alloc(len(data), align)
        self.mem[address - ARENA:address - ARENA + len(data)] = data
        return address


NAMES = (b'Init', b'FadeIn', b'Loop', b'x', b'Longer_script_name', b'a b')
RESPONSE_TYPES = (0, 1, 2, 5, 0x100, 0x1ff, 0x203, 0x205, 0xffffffff)


def make_list(A, base, vtable=0x79a9ec, items=()):
    """FEngQueueList (value0, count4, first8, valueC) over doubly linked nodes (next at +4, previous at +8)."""
    A.put(base, 'IIII', vtable, len(items), items[0] if items else 0, items[-1] if items else 0)
    for i, node in enumerate(items):
        A.put(node + 4, 'II', items[i + 1] if i + 1 < len(items) else 0, items[i - 1] if i else 0)


def make_script(A, rng, hash_=None, chain=0):
    script = A.alloc(0x34)
    tracks = rng.choice((0, 0, 1, 2))
    tracks_ptr = 0
    if tracks:
        tracks_ptr = A.alloc(0x38 * tracks)
        for t in range(tracks):
            track = tracks_ptr + 0x38 * t
            A.mem[track - ARENA:track - ARENA + 0x28] = rng.randbytes(0x28)
            A.mem[track - ARENA + 1] = rng.choice((0, 4, 8, 12, 16))  # value1: state size in bytes
            A.mem[track - ARENA + 2] = rng.choice((2, 5, 0xff, 2, 7))   # value2: no key evaluation
            A.mem[track - ARENA + 7] = rng.randrange(0, 8)              # state7: state word index
            A.put(track + 0x28, 'IIII', 0x79aa30, 0, 0, 0)               # empty owned key reference
    events = rng.choice((0, 0, 1, 2))
    events_ptr = A.blob(rng.randbytes(12 * events)) if events else 0
    name = A.string(rng.choice(NAMES)) if rng.random() < 0.6 else 0
    A.put(script, 'IIIIIIIIIIII', 0x79ad28, 0, 0, rng.randrange(0, 3000), rng.randrange(0, 3000), rng.getrandbits(32),
          chain, tracks, tracks_ptr, events, events_ptr, name)
    A.put(script + 0x30, 'I', hash_ if hash_ is not None else rng.choice((INIT, 0x1234, 0x77, rng.getrandbits(32))))
    return script


def make_response(A, rng):
    response = A.alloc(0x18)
    count = rng.choice((0, 1, 2, 3))
    commands = 0
    if count:
        commands = A.alloc(12 * count + 4) + 4
        A.put(commands - 4, 'I', count)
        for c in range(count):
            A.put(commands + 12 * c, 'III', rng.choice(RESPONSE_TYPES), rng.getrandbits(32), rng.getrandbits(32))
    A.put(response, 'IIIIII', 0x79aa38, 0, 0, rng.choice((0x1111, 0x2222, rng.getrandbits(32))), count, commands)
    return response


def make_object(A, rng, vtable=0x79ba0c, type_=1, size=0x60, guid=None, scripts=True, responses=True, current=True):
    """Well-formed object prefix (0x5c bytes) with response and script lists."""
    obj = A.alloc(size)
    A.mem[obj - ARENA:obj - ARENA + size] = rng.randbytes(size)
    name = A.string(rng.choice(NAMES)) if rng.random() < 0.7 else 0
    A.put(obj, 'III', vtable, 0, 0)
    A.put(obj + 0xc, 'II', guid if guid is not None else rng.getrandbits(32), rng.getrandbits(32))
    A.put(obj + 0x14, 'I', name)
    A.put(obj + 0x18, 'II', type_, rng.getrandbits(32) & ~0x10000000)
    A.put(obj + 0x20, 'HHII', rng.getrandbits(16), rng.choice((0xffff, 3, 0x1234)), rng.getrandbits(32), rng.getrandbits(32))
    A.put(obj + 0x2c, 'II', rng.getrandbits(32) & 0x1fffff00, rng.choice((0, 4, 8, 16, 0x24, 0x40)))
    items = [make_response(A, rng) for _ in range(rng.choice((0, 0, 1, 2, 3)) if responses else 0)]
    make_list(A, obj + 0x34, items=items)
    items = []
    pool = []
    for i in range(rng.choice((0, 1, 2, 3)) if scripts else 0):
        script = make_script(A, rng)
        pool.append(script)
        items.append(script)
    for script in items:   # chain pointers reference sibling scripts
        if rng.random() < 0.4: A.put(script + 0x18, 'I', rng.choice(items))
        else: A.put(script + 0x18, 'I', 0)
    make_list(A, obj + 0x44, items=items)
    A.put(obj + 0x54, 'I', rng.choice(items) if items and current and rng.random() < 0.85 else 0)
    return obj


def make_group(A, rng, depth=1, guid=None, children=None):
    group = make_object(A, rng, 0x79b8e8, 5, 0x6c, guid)
    kids = []
    for _ in range(rng.choice((0, 1, 2, 3)) if children is None else children):
        if depth and rng.random() < 0.25: kids.append(make_group(A, rng, depth - 1))
        else: kids.append(make_object(A, rng, rng.choice((0x79ba0c, 0x79baf4)), rng.choice((1, 9, 10)), 0x60))
    make_list(A, group + 0x5c, items=kids)
    return group


def link_objects_chain(A, objects):
    for i, obj in enumerate(objects):
        A.put(obj + 4, 'II', objects[i + 1] if i + 1 < len(objects) else 0, objects[i - 1] if i else 0)


def make_package_names(A, rng, count, level=False):
    packages = []
    for i in range(count):
        package = A.alloc(0x98)
        A.mem[package - ARENA:package - ARENA + 0x98] = rng.randbytes(0x98)
        packages.append(package)
    return packages


def make_plain_package(A, rng, name=None, objects=None):
    package = A.alloc(0x98)
    A.mem[package - ARENA:package - ARENA + 0x98] = rng.randbytes(0x98)
    A.put(package + 4, 'II', 0, 0)
    if name is not None:
        A.put(package + 0xc, 'II', A.string(name), cihash(name))
        header = A.string(b'\x12\x34' + name)
        A.put(package + 0x38, 'I', header)
    objs = objects if objects is not None else []
    link_objects_chain(A, objs)
    A.put(package + 0x4c, 'I', objs[0] if objs else 0)
    make_list(A, package + 0x44, items=objs)
    A.put(package + 0x14, 'BB', rng.choice((0, 1)), 0)
    return package


def package_list(A, rng, base, packages):
    """FEngPackageList at base: value0, count4, first8, lastC; packages chained at +4/+8."""
    make_list(A, base, items=packages)


class Fault(Exception):
    pass


class Machine:
    """One Unicorn instance: the original image, plus the linked rebuilt object when rebuilt."""
    def __init__(self, image, linked, entries, visitor_vtable, rebuilt, state):
        self.rebuilt, self.state, self.visitor_vtable = rebuilt, state, visitor_vtable
        uc = self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        for base, size in ((0, 0x1000), (ARENA, ARENA_SIZE), (HEAP, HEAP_SIZE), (STACK - 0x40000, 0x41000), (STOP, 0x1000),
                           (STUBS, 0x1000), (TEB, 0x1000), (GDT, 0x1000)):
            uc.mem_map(base, size)
        def descriptor(base, limit, access, flags=0xc):
            return (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) | (((limit >> 16) & 15) << 48) | (flags << 52) | ((base >> 24) << 56)
        uc.mem_write(GDT + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93), descriptor(TEB, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, GDT, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16), (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)):
            uc.reg_write(reg, selector)
        if rebuilt:
            uc.mem_map(linked['address'], linked['allocation'])
            for address, body in linked['blocks']: uc.mem_write(address, body)
        self.pristine = bytes(uc.mem_read(DATA_BEGIN, DATA_END - DATA_BEGIN))
        self.names = {}   # hooked address -> name
        for name, (address, _, _) in CALLEES.items(): self.names[address] = name
        for name, address in ORIGINAL_ROOT.items(): self.names[address] = 'R:' + name
        if rebuilt:
            for name, (prefix, _) in ROOTS.items():
                self.names[next(a for s, a in entries.items() if s.startswith(prefix))] = 'R:' + name
        for address in (ALLOC, FREE, DRIVER, SLOT_LOAD, SLOT_LOADED): self.names[address] = {ALLOC: 'alloc', FREE: 'free', DRIVER: 'driver', SLOT_LOAD: 'load', SLOT_LOADED: 'loaded'}[address]
        for i in range(24): self.names[SLOT_TRAP + 4 * i] = 'trap%d' % i
        self.argc = {a: CALLEES[n][1] for a, n in self.names.items() if n in CALLEES}
        self.argc.update({a: ROOTS[n[2:]][1] for a, n in self.names.items() if n.startswith('R:')})
        self.has_this = {a: (CALLEES[n][2] if n in CALLEES else True) for a, n in self.names.items()}
        for address in self.names: uc.hook_add(UC_HOOK_CODE, self.on_code, begin=address, end=address)
        self.heap_top = HEAP
        self.heap_dirty = 0
        self.trace = []
        self.digest_limit = ARENA_SIZE

    # -- helpers
    def u32(self, address): return struct.unpack('<I', self.uc.mem_read(address, 4))[0]
    def put32(self, address, value): self.uc.mem_write(address, struct.pack('<I', value & 0xffffffff))

    def digest(self):
        uc = self.uc
        crc = zlib.crc32(bytes(uc.mem_read(ARENA, self.digest_limit)))
        crc = zlib.crc32(bytes(uc.mem_read(HEAP, max(self.heap_top - HEAP, 16))), crc)
        return crc

    def finish_stub(self, sp, cleanup, eax=None):
        uc = self.uc
        ret = self.u32(sp)
        if eax is not None: uc.reg_write(xc.UC_X86_REG_EAX, eax & 0xffffffff)
        uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
        uc.reg_write(xc.UC_X86_REG_ESP, sp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)

    def on_code(self, uc, address, size, user):
        name = self.names[address]
        sp = uc.reg_read(xc.UC_X86_REG_ESP); ecx = uc.reg_read(xc.UC_X86_REG_ECX)
        fx = self.state
        if name == 'alloc':
            request = self.u32(sp + 4)
            pointer = self.heap_top
            self.heap_top = (self.heap_top + request + 31) & ~15
            assert self.heap_top < HEAP + HEAP_SIZE, 'heap exhausted'
            uc.mem_write(pointer, b'\xcd' * request)
            self.trace.append(('alloc', request))
            return self.finish_stub(sp, 0, pointer)
        if name == 'free':
            self.trace.append(('free', self.u32(sp + 4)))
            return self.finish_stub(sp, 0, 0xfeedface)
        if name == 'driver':
            data, value = self.u32(sp + 4), self.u32(sp + 8)
            self.trace.append(('driver', ecx, data, value, self.digest()))
            return self.finish_stub(sp, 8, fx['loader']['package'])
        if name == 'load':
            package_name, block, free = self.u32(sp + 4), self.u32(sp + 8), self.u32(sp + 12)
            self.trace.append(('load', ecx, package_name, bytes(uc.mem_read(package_name, 40)), self.digest()))
            self.put32(block, fx['loader']['block']); uc.mem_write(free, bytes([fx['loader']['free']]))
            return self.finish_stub(sp, 12, fx['loader']['data'])
        if name == 'loaded':
            self.trace.append(('loaded', ecx, self.u32(sp + 4), self.digest()))
            return self.finish_stub(sp, 4, 0)
        if name.startswith('trap'):
            raise AssertionError(('unexpected observer slot', name))
        argc = self.argc[address]
        args = [self.u32(sp + 4 + 4 * i) for i in range(argc)]
        this = ecx if self.has_this[address] else None
        if name == 'ApplyTrack': args[1] &= 0xff
        elif name == 'VisitObjects' and self.u32(args[0]) in (0x79aaec, self.visitor_vtable): args[0] = 'visitor'
        elif name == 'R:InitScriptVisit' and STACK - 0x40000 <= ecx < STACK + 0x1000: this = 'visitor'
        # Temporaries (queries, visitors) live on the stack, whose depth differs between the two compilers.
        args = ['stack' if isinstance(a, int) and STACK - 0x40000 <= a < STACK + 0x1000 else a for a in args]
        if isinstance(this, int) and STACK - 0x40000 <= this < STACK + 0x1000: this = 'stack'
        self.trace.append((name, this, tuple(args), self.digest()))

    def reset(self, A, fx, limit):
        uc = self.uc
        self.state.clear(); self.state.update(fx)
        uc.mem_write(ARENA, bytes(A.mem))
        if self.heap_dirty > HEAP: uc.mem_write(HEAP, bytes(self.heap_dirty - HEAP))
        self.heap_top = HEAP; self.heap_dirty = HEAP
        uc.mem_write(0, bytes(0x1000))
        uc.mem_write(TEB, struct.pack('<I', 0xffffffff))
        for address, size in RESET_GLOBALS: uc.mem_write(address, bytes(size))
        self.put32(0x7f7cf8, fx['guid'])
        self.put32(0x7f7cc4, fx['default'])
        for i, slot in enumerate(range(0, 0x60, 4)):
            target = SLOT_TRAP + 4 * i
            if slot == 0: target = SLOT_LOAD
            elif slot == 0xc: target = SLOT_LOADED
            self.put32(VTABLE + slot, target)
        self.digest_limit = limit
        self.trace = []

    def call(self, address, ecx, args, count=4000000):
        uc = self.uc
        uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', a & 0xffffffff) for a in args))
        uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, ecx & 0xffffffff)
        uc.reg_write(xc.UC_X86_REG_EAX, 0x1badf00d); uc.reg_write(xc.UC_X86_REG_EDX, 0x2badf00d)
        for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
        try:
            uc.emu_start(address, STOP, count=count)
        except UcError as error:
            raise Fault(str(error))
        self.heap_dirty = max(self.heap_dirty, self.heap_top)
        if uc.reg_read(xc.UC_X86_REG_EIP) != STOP: raise Fault('nontermination')
        return uc.reg_read(xc.UC_X86_REG_EAX)

    def snapshot(self):
        uc = self.uc
        data = bytes(uc.mem_read(DATA_BEGIN, DATA_END - DATA_BEGIN))
        diffs = []
        for off in range(0, len(data), 256):
            if data[off:off + 256] != self.pristine[off:off + 256]:
                diffs.append((off, data[off:off + 256]))
                uc.mem_write(DATA_BEGIN + off, self.pristine[off:off + 256])
        return (bytes(uc.mem_read(ARENA, ARENA_SIZE)), bytes(uc.mem_read(HEAP, self.heap_top - HEAP)), tuple(diffs), bytes(uc.mem_read(0, 0x100)))


# ---------------------------------------------------------------------------------------------
# Fixtures

def new_fx(rng):
    return {'guid': rng.choice((0, 1, 0x100, 0xfffffffe, rng.getrandbits(32))), 'default': rng.getrandbits(32),
            'loader': {'data': 0, 'block': 0, 'free': 0, 'package': 0}}


def case_insert(rng):
    A = Arena(rng); base = QUEUE + 0xdc
    levels = (-3, -1, 0, 1, 2, 3, 5, 200, 0x7fffffff, rng.randrange(-10, 10))
    packages = [make_plain_package(A, rng) for _ in range(rng.choice((0, 1, 2, 3, 4)))]
    for p in packages: A.put(p + 0x18, 'i', rng.choice(levels))
    package_list(A, rng, base, packages)
    new = make_plain_package(A, rng); A.put(new + 0x18, 'i', rng.choice(levels))
    level = A.get(new + 0x18, 'i'); existing = [A.get(p + 0x18, 'i') for p in packages]
    tags = {'empty-list'} if not packages else {'tail-stays' if existing[-1] <= level else ('head' if all(e > level for e in existing) else 'middle')}
    return dict(A=A, this=base, args=[new], fx=new_fx(rng), tags=tags)


def case_append_object(rng):
    A = Arena(rng)
    package = make_plain_package(A, rng, objects=[make_object(A, rng) for _ in range(rng.choice((0, 1, 2, 3)))])
    new = make_object(A, rng) if rng.random() < 0.9 else 0
    return dict(A=A, this=package, args=[new], fx=new_fx(rng), tags={'null-object' if not new else 'object'})


def case_append_child(rng):
    A = Arena(rng)
    group = make_group(A, rng, 0, children=rng.choice((0, 1, 2, 3)))
    new = make_object(A, rng) if rng.random() < 0.9 else 0
    return dict(A=A, this=group, args=[new], fx=new_fx(rng), tags={'null-child' if not new else 'child'})


def case_visit(rng):
    A = Arena(rng)
    obj = make_object(A, rng, scripts=True)
    if rng.random() < 0.7:   # make sure an Init script exists
        init = make_script(A, rng, INIT)
        existing = []
        node = A.get(obj + 0x4c)
        while node: existing.append(node); node = A.get(node + 4)
        existing.insert(rng.randrange(len(existing) + 1), init)
        link_objects_chain(A, existing); make_list(A, obj + 0x44, items=existing)
    fake_visitor = A.alloc(16)
    return dict(A=A, this=fake_visitor, args=[obj], fx=new_fx(rng))


def case_clone_object(rng):
    A = Arena(rng)
    src = make_object(A, rng, vtable=rng.choice((0x79ba0c, 0x79b8e0)), type_=rng.choice((1, 2, 3, 7, 9, 10)),
                      current=rng.random() < 0.9)
    memory = A.alloc(0x68)
    A.mem[memory - ARENA:memory - ARENA + 0x68] = rng.randbytes(0x68)
    return dict(A=A, this=memory, args=[src, rng.choice((0, 1))], fx=new_fx(rng))


def case_clone_group(rng):
    A = Arena(rng)
    src = make_group(A, rng, 1)
    memory = A.alloc(0x6c)
    A.mem[memory - ARENA:memory - ARENA + 0x6c] = rng.randbytes(0x6c)
    return dict(A=A, this=memory, args=[src, rng.choice((0, 1, 1)), rng.choice((0, 1))], fx=new_fx(rng))


PATHS = (b'Screen.fng', b'dir/Screen.fng', b'dir\\Screen.fng', b'a/b\\c.fng', b'noext', b'', b'/', b'\\x', b'UPPER.FNG',
         b'a/', b'a\\', b'x' * 130, b'd/' + b'y' * 126, b'deep/er/path\\leaf.FNG')


def basename(name):
    cut = max(name.rfind(b'/'), name.rfind(b'\\'))
    return name[cut + 1:]


def case_load(rng):
    A = Arena(rng)
    fx = new_fx(rng)
    name = rng.choice(PATHS)
    # Name stored with an arbitrary byte in front, so the unterminated separator scan is observable.
    before = A.alloc(1, 1); prefix = rng.choice((b'/', b'\\', b'A', b'\0'))
    A.mem[before - ARENA] = prefix[0]
    name_ptr = A.string(name)
    assert name_ptr == before + 1
    cached_names = []
    mode = rng.choice(('full', 'base', 'base', 'miss', 'miss', 'prefixed', 'both'))
    if mode in ('full', 'both'): cached_names.append(name)
    if mode in ('base', 'both'): cached_names.append(basename(name))
    if mode == 'prefixed': cached_names.append(prefix + name)   # found by the unterminated scan
    cached_names.extend(rng.sample([b'Other.fng', b'zz', b'screen.FNG'], rng.randrange(0, 3)))
    rng.shuffle(cached_names)
    ec = []
    for text in cached_names:
        objects = []
        for _ in range(rng.choice((0, 1, 2))):
            objects.append(make_group(A, rng, 0, children=rng.choice((0, 1, 2))) if rng.random() < 0.3 else make_object(A, rng, scripts=True))
        init_missing = rng.random() < 0.15
        for obj in objects:
            if not init_missing and rng.random() < 0.8:
                init = make_script(A, rng, INIT)
                existing = []; node = A.get(obj + 0x4c)
                while node: existing.append(node); node = A.get(node + 4)
                existing.append(init); link_objects_chain(A, existing); make_list(A, obj + 0x44, items=existing)
        ec.append(make_plain_package(A, rng, text, objects))
    link_objects_chain(A, ec)
    package_list(A, rng, QUEUE + 0xec, ec)
    dc = [make_plain_package(A, rng) for _ in range(rng.choice((0, 1, 2, 3)))]
    for p in dc: A.put(p + 0x18, 'i', rng.choice((-1, 0, 1, 2, 3, 130, 255)))
    package_list(A, rng, QUEUE + 0xdc, dc)
    A.put(QUEUE, 'B', rng.choice((0, 1, 2)))
    observer = not (mode in ('full', 'both') and rng.random() < 0.2)
    A.put(QUEUE + 0x10c, 'I', OBSERVER if observer else 0)
    A.put(OBSERVER, 'I', VTABLE)
    new_package = make_plain_package(A, rng)
    fx['loader'] = dict(data=rng.choice((0, ARENA + 0x100, ARENA + 0x2000)), block=A.alloc(16),
                        free=rng.choice((0, 1, 0x80)), package=rng.choice((0, new_package, new_package)))
    level = rng.choice((0, 1, 2, 127, 128, 255, rng.randrange(256)))
    tags = {'cache-' + mode, 'observer-null' if not observer else 'observer', 'free-flag-%d' % (fx['loader']['free'] != 0),
            'loader-data-null' if not fx['loader']['data'] else 'loader-data', 'driver-null' if not fx['loader']['package'] else 'driver-package',
            'name-%d' % min(len(name), 3), 'level-high' if level > 127 else 'level-low'}
    return dict(A=A, this=QUEUE, args=[name_ptr, level, rng.choice((0, 1, 0xffffffff, rng.getrandbits(32)))], fx=fx, tags=tags)


def chunk(ident, payload):
    return struct.pack('<II', ident, len(payload)) + payload


def prop(ident, payload=b''):
    return struct.pack('<HH', ident, len(payload)) + payload


def words(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


class Context:
    """Reader, package, queue and the inheritance sources shared by ReadObject and Read fixtures."""
    def __init__(self, A, rng):
        self.A, self.rng = A, rng
        A.put(OBSERVER, 'I', VTABLE)
        A.put(QUEUE + 0x10c, 'I', OBSERVER)
        A.put(QUEUE, 'BBB', 1, 1, 0)
        names = (b'HudMain.fng', b'Source2.fng')
        self.sources = []
        self.guids = []
        packages = []
        guid = 0x1000
        for name in names:
            objects = []
            for _ in range(rng.choice((2, 3, 4))):
                guid += 1
                if rng.random() < 0.3: obj = make_group(A, rng, 1, guid)
                else: obj = make_object(A, rng, rng.choice((0x79ba0c, 0x79ba0c, 0x79baf4)), rng.choice((1, 9, 10)), 0x60, guid)
                objects.append(obj); self.guids.append((name, guid, obj))
            packages.append(make_plain_package(A, rng, name, objects))
        A.put(packages[0] + 4, 'I', packages[1])
        A.put(QUEUE + 0x104, 'I', packages[0])
        self.packages = packages
        # Inheritance records: (key, package name hash, guid). Valid ones, wrong package, wrong guid.
        self.records = []
        self.valid_keys, self.invalid_keys = [], []
        key = 0x5000
        for i in range(rng.choice((2, 4, 6))):
            key += rng.choice((1, 7))
            name, g, obj = rng.choice(self.guids)
            kind = rng.choice(('valid', 'valid', 'valid', 'package', 'guid'))
            package_hash = cihash(name) if kind != 'package' else cihash(name) ^ 0x55
            guid_ = g if kind != 'guid' else 0x9999
            self.records.append((key, package_hash, guid_))
            (self.valid_keys if kind == 'valid' else self.invalid_keys).append(key)
        self.missing_key = 0x4fff
        pkg = PKG
        A.mem[pkg - ARENA:pkg - ARENA + 0x98] = rng.randbytes(0x98)
        records = A.alloc(12 * len(self.records))
        for i, record in enumerate(self.records): A.put(records + 12 * i, 'III', *record)
        A.put(pkg + 0x84, 'II', len(self.records), records)
        focus = A.alloc(4 * 64)
        A.put(pkg + 0x8c, 'III', 0, focus, 64)
        make_list(A, pkg + 0x44)
        A.put(pkg + 0x4c, 'I', 0)
        A.put(pkg + 0x50, 'I', 0)
        # reader
        specs = A.alloc(8 * 4)
        for i in range(4): A.put(specs + 8 * i, 'II', rng.choice((1, 2, 4, 5, 6, 7, 9, 10, 11, 3)), rng.choice((0, 4, 8, 16, 0x24, 0x40)))
        group_sources = [o for n, g, o in self.guids if A.get(o + 0x18) == 5]
        stale = rng.choice([0, 0, make_object(A, rng, guid=0x1)] + group_sources)
        A.mem[READER - ARENA:READER - ARENA + 0x100] = rng.randbytes(0x100)
        A.put(READER, 'I', pkg)
        A.put(READER + 8, 'BBB', rng.choice((0, 0, 1)), rng.choice((0, 1, 1)), rng.choice((0, 1)))
        A.put(READER + 0xc, 'III', 0, stale, 0)
        A.put(READER + 0x18, 'B', rng.choice((0, 1)))
        A.put(READER + 0x30, 'III', rng.choice((0, 1, 2, 3, 4)), specs, OBSERVER)
        A.put(READER + 0x3c, 'I', QUEUE)
        A.put(READER + 0x48, 'II', rng.choice((0, 3, 70)), rng.choice((0, 0, 1, 5)))
        self.group_guids = []
        self.next_guid = 0x2000
        if rng.random() < 0.7:     # a group already in the package, reachable through 'PA'
            group = make_group(A, rng, 0, 0x3333, children=rng.choice((0, 1, 2)))
            A.put(pkg + 0x4c, 'I', group); make_list(A, pkg + 0x44, items=[group])
            self.group_guids.append(0x3333)

    def fresh_guid(self):
        self.next_guid += 1
        return self.next_guid


KNOWN_TYPES = (1, 2, 4, 5, 6, 7, 9, 10, 11, 3, 8, 12, 0xff)


def gen_object_props(rng, ctx, existing_group=None, alive0=False):
    """Property stream for one object chunk. info: alive (an object exists afterwards), failed (the
    reader returns 0), guid/type (the object's identity afterwards, when known)."""
    A = ctx.A
    out = bytearray()
    info = {'alive': alive0, 'ot': False, 'failed': False, 'guid': None, 'type': None, 'source': None}
    def unknown(): return prop(rng.randrange(0x100, 0x2000), rng.randbytes(rng.choice((0, 4, 8))))
    def maybe_pa():
        if existing_group is not None and rng.random() < 0.5: out.extend(prop(0x4150, words(existing_group)))
        elif rng.random() < 0.25: out.extend(prop(0x4150, words(rng.choice((0x1001, 0x77777)))))
    if rng.random() < 0.2: out.extend(unknown())
    maybe_pa()
    if not (alive0 and rng.random() < 0.6) and rng.random() < 0.97:
        type_ = rng.choice(KNOWN_TYPES + (5, 5, 5, 4, 6, 1, 2))
        out.extend(prop(0x744f, words(type_)))
        info.update(ot=True, type=type_, alive=type_ != 0, guid=None, source=None)
    steps = rng.randrange(0, 8) if info['alive'] else 0
    for _ in range(steps):
        kind = rng.choice(('OP', 'OP', 'On', 'Oh', 'SA', 'PA', 'If', 'type', 'unknown'))
        if kind == 'OP':
            flags = rng.getrandbits(32) & ~0x10100000 & 0x0fffffff
            if rng.random() < 0.25: flags |= 0x10000000
            guid = ctx.fresh_guid()
            inherit = rng.random() < 0.5
            if inherit:
                flags |= 0x100000
                guid = rng.choice(ctx.valid_keys * 2 + ctx.invalid_keys + [ctx.missing_key])
            out.extend(prop(0x504f, words(guid, rng.getrandbits(32), flags, rng.choice((0xffff, 3, 0x1234, rng.getrandbits(16))))))
            info['guid'] = guid
            if inherit:
                record = next((r for r in ctx.records if r[0] == guid), None)
                if record is None or guid not in ctx.valid_keys:
                    info['failed'] = True
                    break          # the reader returns 0 here; later properties are never read
                source = next(o for n, g, o in ctx.guids if g == record[2])
                info['source'] = source; info['type'] = A.get(source + 0x18)
        elif kind == 'On': out.extend(prop(0x6e4f, rng.choice((b'name\0', b'x\0\0\0', b'Longer name here\0'))))
        elif kind == 'Oh': out.extend(prop(0x684f, words(rng.getrandbits(32))))
        elif kind == 'SA': out.extend(prop(0x4153, words(*[rng.getrandbits(32) for _ in range(rng.randrange(0, 5))])))
        elif kind == 'PA': maybe_pa()
        elif kind == 'If': out.extend(prop(0x6649, words(rng.getrandbits(32))))
        elif kind == 'type':
            out.extend(rng.choice((prop(0x6a53, words(rng.getrandbits(32))), prop(0x6c53, words(rng.getrandbits(32))),
                                   prop(0x7753, words(rng.getrandbits(32))), prop(0x644c, words(rng.randrange(0, 4), rng.randrange(0, 4))),
                                   prop(0x744c, words(rng.getrandbits(32))), prop(0x764c, words(rng.getrandbits(32), rng.getrandbits(32))))))
        else: out.extend(unknown())
        if rng.random() < 0.15: out.extend(unknown())
    return bytes(out), info


def gen_msgr(rng):
    out = bytearray()
    for _ in range(rng.choice((1, 1, 2))):
        out.extend(prop(0x694d, words(rng.choice((0x1111, 0x2222, 0x3333, rng.getrandbits(32))))))
        count = rng.choice((0, 1, 2, 3))
        out.extend(prop(0x434d, words(count)))
        for _ in range(count):
            out.extend(prop(0x6952, words(rng.choice(RESPONSE_TYPES))))
            if rng.random() < 0.8: out.extend(prop(0x7452, words(rng.getrandbits(32))))
            if rng.random() < 0.8: out.extend(prop(0x7552, words(rng.getrandbits(32))))
    return bytes(out)


def gen_scrp(rng, existing):
    """Script tag stream limited to tags whose unchecked dereferences stay inside the fixture."""
    out = bytearray(); script = None; tracks = 0; next_track = 0; track = False
    existing = list(existing)
    for _ in range(rng.randrange(0, 9)):
        choices = ['Sn', 'Sh', 'unknown', 'SI']
        if script: choices += ['Sf', 'Sc', 'Sl', 'EV']
        if script and next_track < tracks: choices += ['FI', 'FI']
        if track: choices += ['Tt']
        tag = rng.choice(choices)
        if tag == 'Sn':
            name = rng.choice((b'', b'Init', b'Fade', bytes(rng.randrange(1, 255) for _ in range(rng.randrange(1, 9)))))
            out.extend(prop(0x6e53, name + b'\0' * (4 - len(name) % 4))); script = 'new'; tracks = next_track = 0; track = False
        elif tag == 'Sh':
            count = rng.randrange(0, 4); value = INIT if rng.random() < 0.45 else rng.getrandbits(32)
            out.extend(prop(0x6853, words(value, rng.randrange(-5, 3000), rng.getrandbits(32), count)))
            script = 'new'; tracks = count; next_track = 0; track = False
        elif tag == 'Sf': out.extend(prop(0x6653, words(rng.getrandbits(32))))
        elif tag == 'Sc': out.extend(prop(0x6353, words(rng.choice((INIT, 0x1234, rng.getrandbits(32))))))
        elif tag == 'Sl': out.extend(prop(0x6c53, words(rng.getrandbits(32))))
        elif tag == 'EV': out.extend(prop(0x5645, rng.randbytes(12 * rng.randrange(1, 4))))
        elif tag == 'SI':
            hit = bool(existing) and rng.random() < 0.8
            out.extend(prop(0x4953, words(rng.choice(existing) if hit else 0xdeadbeef)))
            if hit: script = 'selected'; tracks = next_track = 0; track = False   # FI would index its unknown tracks
            else: script = None; track = False
        elif tag == 'FI':
            out.extend(prop(0x4946, bytes((rng.randrange(8), rng.choice((4, 8, 12, 16)), rng.choice((2, 5, 7, 0xff, 2, 5, 0, 1, 3)), rng.randrange(128))) + rng.randbytes(4)))
            next_track += 1; track = True
        elif tag == 'Tt': out.extend(prop(0x7454, bytes((rng.choice((2, 5, 7, 0xff)),)) + b'\0\0\0'))
        else: out.extend(prop(rng.randrange(0x100, 0x1000), rng.randbytes(rng.randrange(0, 12))))
    return bytes(out)


def gen_list_chunk(rng, ctx):
    A = ctx.A
    children = []
    alive = False; failed = False; source = None; final = (None, None)
    for _ in range(rng.choice((0, 1, 1, 1, 1, 2))):
        group = rng.choice(ctx.group_guids) if ctx.group_guids and rng.random() < 0.6 else None
        props, info = gen_object_props(rng, ctx, group, alive)
        children.append(chunk(CHUNK_OD, props))
        if info['ot']: alive, source, final = info['alive'], info['source'], (info['type'], info['guid'])
        elif alive:
            if info['source'] is not None: source = info['source']
            if info['guid'] is not None: final = (info['type'] if info['type'] is not None else final[0], info['guid'])
        if info['failed']: failed = True; break
        if rng.random() < 0.2: children.append(chunk(rng.choice((0x41414141, 0x12345678, 0)), rng.randbytes(rng.choice((0, 3, 8)))))
    if alive and not failed:
        if final[0] == 5 and final[1] is not None and source is None: ctx.pending_groups.append(final[1])
        existing = []
        node = A.get(source + 0x4c) if source else 0
        while node: existing.append(A.get(node + 0x30)); node = A.get(node + 4)
        extras = []
        for _ in range(rng.choice((0, 0, 1, 2))): extras.append(chunk(CHUNK_MSGR, gen_msgr(rng)))
        for _ in range(rng.choice((0, 0, 1, 1, 2))): extras.append(chunk(CHUNK_SCRP, gen_scrp(rng, existing)))
        rng.shuffle(extras)
        children.extend(extras)
    return b''.join(children), failed


def gen_read_stream(rng, ctx):
    """Whole package stream. Returns (bytes, trailer) where trailer is appended after the stream."""
    ctx.pending_groups = []
    def filler(): return chunk(rng.choice((0x41414141, 0x12345678, 0x6e747543, 0)), rng.randbytes(rng.choice((0, 1, 3, 8, 13))))
    inner = []
    tags = set()
    for _ in range(rng.choice((0, 0, 1, 2))): inner.append(filler())
    for _ in range(rng.choice((0, 0, 1, 1, 2))):
        count = rng.choice((0, 1, 3, 8, 70))
        tags.add('butn-zero' if count == 0 else 'butn-nonzero')
        inner.append(chunk(CHUNK_BUTN, words(count, 0)[:rng.choice((4, 8))]))
    lists = rng.choice((0, 1, 1, 1, 2, 3))
    tags.add('lists-%d' % lists)
    failed = False
    for i in range(lists):
        payload, failed = gen_list_chunk(rng, ctx)
        inner.append(chunk(CHUNK_EA if i == 0 or rng.random() < 0.5 else rng.choice((0x41414141, CHUNK_EA)), payload))
        ctx.group_guids.extend(ctx.pending_groups); ctx.pending_groups = []
        if failed: break
    for _ in range(rng.choice((0, 0, 1))): inner.append(filler())
    body = b''.join(inner)
    k_chunk = chunk(CHUNK_K, body)
    top = []
    for _ in range(rng.choice((0, 0, 1, 2))): top.append(filler())
    mode = rng.random()
    if mode < 0.08:     # package chunk missing
        top.append(filler())
        trailer = rng.randbytes(8)
        tags.add('package-chunk-missing')
    elif mode < 0.14:   # found only through the end-of-chunks id check
        trailer = chunk(CHUNK_K, b'')
        tags.add('package-chunk-after-end')
    else:
        top.append(k_chunk)
        for _ in range(rng.choice((0, 0, 1))): top.append(filler())
        trailer = rng.randbytes(8)
    children = b''.join(top)
    if failed: tags.add('object-reader-failure')
    return struct.pack('<II', rng.getrandbits(32), len(children)) + children, trailer, tags


def case_read_object(rng):
    A = Arena(rng); ctx = Context(A, rng)
    pre = []
    alive0 = rng.random() < 0.4
    if alive0:
        type_ = rng.choice([t for t in KNOWN_TYPES if t != 0])
        pre = [('call', 0x534df0, READER, [type_]), ('store', READER + 0xc)]
    props, info = gen_object_props(rng, ctx, 0x3333 if ctx.group_guids and rng.random() < 0.7 else None, alive0)
    data = A.blob(props)
    return dict(A=A, this=READER, args=[data, len(props)], fx=new_fx(rng), pre=pre,
                tags={'preexisting-object' if alive0 else 'new-object'} | ({'failure'} if info['failed'] else set()))


def case_read(rng):
    A = Arena(rng); ctx = Context(A, rng)
    stream, trailer, tags = gen_read_stream(rng, ctx)
    data = A.blob(stream + trailer)
    A.put(READER + 4, 'I', data)
    return dict(A=A, this=READER, args=[], fx=new_fx(rng), tags=tags)


CASES = {'InsertByLevel': case_insert, 'AppendObject': case_append_object, 'AppendChild': case_append_child,
         'InitScriptVisit': case_visit, 'CloneObject': case_clone_object, 'CloneGroup': case_clone_group,
         'Call555860': case_load, 'ReadObject': case_read_object, 'Read': case_read}
RET_MASK = {'Read': 0xff, 'ReadObject': 0xff, 'InitScriptVisit': 0xff, 'CloneObject': 0xffffffff, 'CloneGroup': 0xffffffff,
            'Call555860': 0xffffffff, 'AppendChild': 0, 'AppendObject': 0, 'InsertByLevel': 0}


class FixtureError(Exception):
    pass


def compile_source(source, compiler, label):
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / (label + '.obj')
    if compiler == 'msvc':
        command = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe']
    else:
        command = ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin']
    done = subprocess.run(command + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend',
                                     os.path.relpath(source, ROOT)], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if done.returncode:
        raise SystemExit('compile failed:\n' + done.stdout)
    return obj


def check_dependencies(image):
    """Fingerprint every real original callee that appears in a recovered manifest."""
    found = {}
    for path in sorted((ROOT / 'config/SPEED2_EXE_1_2').glob('*.json')):
        try: data = json.loads(path.read_text())
        except ValueError: continue
        functions = data.get('functions', {}) if isinstance(data, dict) else {}
        entries = functions.items() if isinstance(functions, dict) else enumerate(functions)
        for key, entry in entries:
            if isinstance(entry, dict) and 'address' in entry and 'sha256' in entry:
                address = entry['address'] if isinstance(entry['address'], int) else int(entry['address'], 16)
                if address in DEPENDENCY_ADDRESSES and address not in found:
                    start = address - 0x400000
                    assert hashlib.sha256(image[start:start + entry['size']]).hexdigest() == entry['sha256'], (path.name, key)
                    found[address] = path.name
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='clang-cl')
    parser.add_argument('--source', type=Path, default=FRONT / 'FEngPackageReader.cpp')
    parser.add_argument('--cases', type=int, default=500, help='cases per routine')
    parser.add_argument('--roots', default=','.join(CASES))
    parser.add_argument('--seed', type=int, default=0x5554a0)
    parser.add_argument('--label', default='FEngPackageReader')
    parser.add_argument('--mutation-checks', action='store_true')
    parser.add_argument('--mutation-cases', type=int, default=150)
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-package-reader.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for entry in manifest['functions'].values():
        start = int(entry['address'], 16) - 0x400000
        assert hashlib.sha256(image[start:start + entry['size']]).hexdigest() == entry['sha256']
    dependencies = check_dependencies(image)
    obj = compile_source(args.source, args.compiler, args.label)
    # MSVC leaves the visitor base class's deleting destructor and vtable to another unit: bind the originals
    # (0x4ff210 and 0x79aa4c) and the derived one (0x5047c0). Neither runs here: the temporary visitor is destroyed inline.
    externals = {'??3@YAXPAXI@Z': STOP + 0x100, '??3@YAXPAX@Z': STOP + 0x100, '__purecall': STOP + 0x200,
                 '??_GFEngObjectVisitor@@UAEPAXI@Z': 0x4ff210, '??_GFEngInitScriptVisitor@@UAEPAXI@Z': 0x5047c0, '??_EFEngObjectVisitor@@UAEPAXI@Z': 0x4ff210, '??_EFEngInitScriptVisitor@@UAEPAXI@Z': 0x5047c0, '??_7FEngObjectVisitor@@6B@': 0x79aa4c}
    for symbol in read_object(obj)['symbols'].values():
        if symbol['section'] != 0 or symbol['storage'] != 2: continue
        name = symbol['name']
        if name in externals: continue
        if name.startswith('?FEngAllocateQueueNode@'): externals[name] = ALLOC
        elif name.startswith('?FEngFreeQueueNode@'): externals[name] = FREE
        elif name.startswith('?Call555800@FEngQueue@'): externals[name] = DRIVER
        elif any(name.startswith('?%s@@3' % g) for g in GLOBALS): externals[name] = next(a for g, a in GLOBALS.items() if name.startswith('?%s@@3' % g))
        else:
            match = [CALLEES[v][0] for k, v in EXTERNALS.items() if name.startswith(k)]
            assert len(match) == 1, ('unexpected external', name)
            externals[name] = match[0]
    linked = link_objects([obj], externals, start=0x3000000)
    entries = linked['exports']
    visitor_vtable = next((a for s, a in entries.items() if s.startswith('??_7FEngInitScriptVisitor@@')), None)
    assert visitor_vtable, 'visitor vtable symbol missing'
    state = {}
    machines = [Machine(image, linked, entries, visitor_vtable, rebuilt, state) for rebuilt in (False, True)]
    root_address = [dict(ORIGINAL_ROOT), {n: next(a for s, a in entries.items() if s.startswith(p)) for n, (p, _) in ROOTS.items()}]
    rng = random.Random(args.seed)
    counts, matched_faults, paths, tags, return_values = {}, {}, {}, {}, {}
    fixture_errors = {}
    fault_reasons = {}
    roots = [r for r in args.roots.split(',') if r]
    for name in roots:
        counts[name] = matched_faults[name] = fixture_errors[name] = 0
        argc = ROOTS[name][1]
        for trial in range(args.cases):
            case = CASES[name](rng)
            A, fx = case['A'], case['fx']
            limit = min(ARENA_SIZE, (A.top + 0x1000) & ~0xfff)
            results = []
            for index, machine in enumerate(machines):
                machine.reset(A, fx, limit)
                try:
                    for step in case.get('pre', ()):
                        if step[0] == 'call': machine.call(step[1], step[2], step[3])
                        else: machine.put32(step[1], machine.uc.reg_read(xc.UC_X86_REG_EAX))
                except (Fault, AssertionError) as error:
                    raise FixtureError(('fixture setup fault', name, trial, str(error)))
                machine.trace = []
                fault = eax = None
                try:
                    eax = machine.call(root_address[index][name], case['this'], case['args'])
                except Fault as error:
                    fault = str(error)
                except AssertionError as error:
                    fault = 'assertion ' + str(error)
                uc = machine.uc
                outcome = dict(fault=fault is not None, why=fault,
                               eax=None if eax is None else eax & RET_MASK[name],
                               esp=uc.reg_read(xc.UC_X86_REG_ESP), regs=tuple(uc.reg_read(r) for r in PRESERVED),
                               fs0=machine.u32(TEB), trace=list(machine.trace))
                outcome['snapshot'] = machine.snapshot()
                results.append(outcome)
            original, rebuilt = results
            if original['fault']:
                if not rebuilt['fault'] or original['trace'] != rebuilt['trace']:
                    raise AssertionError(('state or call order mismatch', 'fault behaviour', name, trial, original['why'], rebuilt['why'],
                                          [t[0] for t in original['trace']][-12:], [t[0] for t in rebuilt['trace']][-12:]))
                matched_faults[name] += 1
                fault_reasons.setdefault(name, []).append((trial, original['why'], [t[0] for t in original['trace']][-6:]))
                continue
            if rebuilt['fault']:
                raise AssertionError(('state or call order mismatch', 'rebuilt fault', name, trial, rebuilt['why'],
                                      [t[0] for t in original['trace']][-12:], [t[0] for t in rebuilt['trace']][-12:]))
            assert original['esp'] == STACK + 4 + 4 * argc, ('original contract', name, hex(original['esp']))
            assert original['regs'] == tuple(0x12345000 + i for i in range(4)) and original['fs0'] == 0xffffffff, ('original contract', name)
            problems = []
            for key in ('eax', 'esp', 'regs', 'fs0'):
                if original[key] != rebuilt[key]: problems.append((key, original[key], rebuilt[key]))
            if original['trace'] != rebuilt['trace']:
                at = next((i for i, (a, b) in enumerate(zip(original['trace'], rebuilt['trace'])) if a != b), min(len(original['trace']), len(rebuilt['trace'])))
                problems.append(('trace', at, [t[:-1] for t in original['trace'][at:at + 3]], [t[:-1] for t in rebuilt['trace'][at:at + 3]],
                                 len(original['trace']), len(rebuilt['trace'])))
            names = ('arena', 'heap', 'data', 'page0')
            for label, a, b in zip(names, original['snapshot'], rebuilt['snapshot']):
                if a != b:
                    if isinstance(a, bytes):
                        problems.append((label, [(hex(i), a[i:i + 4].hex(), b[i:i + 4].hex()) for i in range(0, min(len(a), len(b)), 4) if a[i:i + 4] != b[i:i + 4]][:8], len(a), len(b)))
                    else: problems.append((label, 'differs'))
            if problems:
                raise AssertionError(('state or call order mismatch', name, trial, problems))
            counts[name] += 1
            seen = set(t[0] for t in original['trace'])
            for item in seen: paths[(name, item)] = paths.get((name, item), 0) + 1
            derived = set(case.get('tags', ()))
            if any(original['snapshot'][3]): derived.add('null-pointer-store')
            derived.add('result-%d' % (original['eax'] != 0) if RET_MASK[name] == 0xff else 'result')
            for tag in derived: tags[(name, tag)] = tags.get((name, tag), 0) + 1
            value = original['eax'] if original['eax'] is not None and RET_MASK[name] == 0xff else None
            if value is not None: return_values[(name, value)] = return_values.get((name, value), 0) + 1
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), matched_faults=matched_faults, fault_reasons=fault_reasons,
                  paths={'%s:%s' % k: v for k, v in sorted(paths.items())}, tags={'%s:%s' % k: v for k, v in sorted(tags.items())},
                  return_values={'%s:%s' % k: v for k, v in sorted(return_values.items())},
                  dependencies_fingerprinted=sorted('%s=%s' % (hex(a), f) for a, f in dependencies.items()), fingerprints=manifest)
    if args.mutation_checks:
        report['rejected_mutations'] = run_mutations(args)
    BUILD.mkdir(parents=True, exist_ok=True)
    if args.label == 'FEngPackageReader': (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k not in ('fingerprints', 'paths')}, indent=2))
    print('trace/path hits:', json.dumps(report['paths']))


MUTATIONS = {
    # Package reader 0x5554a0
    'read-butn-count': [('            count48 = count;\n', '')],
    'read-butn-size': [('FEngAllocateQueueNode(count * 4)', 'FEngAllocateQueueNode(count * 8)')],
    'read-butn-free': [('            FEngFreeQueueNode(package->focusObjects90);\n', '')],
    'read-response-mode': [('->Read(child + 8, *(const unsigned int *)(child + 4), 0)', '->Read(child + 8, *(const unsigned int *)(child + 4), 1)')],
    'read-object-failure': [('if (!ReadObject((const unsigned short *)(child + 8), *(const unsigned int *)(child + 4))) return false;',
                             'ReadObject((const unsigned short *)(child + 8), *(const unsigned int *)(child + 4));')],
    'read-init-hash': [('while (init && init->hash30 != 0x1744b3) init = init->next4;', 'while (init && init->hash30 != 0x1744b4) init = init->next4;')],
    'read-apply-gate': [('            if (!value8) {', '            if (value8) {')],
    'read-group-append': [('            if (group14) ((FEngGroupObject *)group14)->AppendChild(objectC);', '            if (!group14) ((FEngGroupObject *)group14)->AppendChild(objectC);')],
    'read-type-setup': [('if (objectC->type18 == 4) ((FEListObject *)objectC)->CalculateOffsets();', 'if (objectC->type18 == 5) ((FEListObject *)objectC)->CalculateOffsets();')],
    'read-chain-link': [('while (target && target->hash30 != hash) target = target->next4;\n                    script->chain18 = target;',
                         'while (target && target->hash30 == hash) target = target->next4;\n                    script->chain18 = target;')],
    'read-list-reset': [('        objectC = 0;\n        group14 = 0;', '        objectC = 0;')],
    # Object reader 0x555140
    'object-parent-cache': [('if (!parent10 || parent10->guidC != payload[0]) parent10', 'if (!parent10) parent10')],
    'object-flags-merge': [('objectC->flags1C |= inherited1C->flags1C;', 'objectC->flags1C = inherited1C->flags1C;')],
    'object-resource-test': [('(short)0xffff', '(short)0xfffe')],
    'object-focus-index': [('                ++used4C;\n', '')],
    'object-group-clone-flag': [('group->CloneConstruct((FEngGroupObject *)inherited1C, 0, 1)', 'group->CloneConstruct((FEngGroupObject *)inherited1C, 1, 1)')],
    'object-failure-delete': [('                    if (objectC) ((FEngObjectVirtuals *)objectC)->Delete(1);\n                    objectC = 0;\n                    return 0;',
                               '                    objectC = 0;\n                    return 0;')],
    'object-state-count': [('unsigned int count = property[1] >> 2;', 'unsigned int count = property[1] >> 1;')],
    'object-name-gate': [('if (value9) objectC->SetName', 'if (!value9) objectC->SetName')],
    'object-script-hash': [('script->chain18 = (FEScript *)script->chain18->hash30;', 'script->chain18 = 0;')],
    # Clones 0x554300 / 0x5548a0
    'clone-guid-step': [('FEngNextObjectGuid = guid + 1;', 'FEngNextObjectGuid = guid + 2;')],
    'clone-zero-state': [('for (unsigned int count = stride >> 2; count; --count) *words++ = 0;', 'for (unsigned int count = stride >> 3; count; --count) *words++ = 0;')],
    'clone-flags': [('    flags1C = source->flags1C;\n    *(unsigned short', '    *(unsigned short')],
    'clone-message': [('        response->messageC = original->messageC;\n', '')],
    'clone-current-script': [('while (found && found->hash30 != hash) found = found->next4;', 'while (found && found->hash30 == hash) found = found->next4;')],
    'clone-chain-advance': [('        sourceScript = sourceScript->next4;\n        copy = copy->next4;', '        sourceScript = sourceScript->next4;')],
    'group-children-gate': [('    if (copyChildren) {', '    if (!copyChildren) {')],
    'group-vtable': [('*(unsigned int *)unidentified0 = 0x79b8e8;\n    children5C.value0', '*(unsigned int *)unidentified0 = 0x79b8e0;\n    children5C.value0')],
    'group-count': [('                ++children5C.count4;\n            }\n        }\n    }\n    return this;', '            }\n        }\n    }\n    return this;')],
    # List helpers 0x517e90 / 0x495610 / 0x518280
    'append-child-count': [('        ++children5C.count4;\n    }\n}\n\n// Original 0x554300', '    }\n}\n\n// Original 0x554300')],
    'append-object-cursor': [('list->Insert(list->valueC,', 'list->Insert(0,')],
    'insert-level-compare': [('if (cursor->value18 <= level) break;', 'if (cursor->value18 < level) break;')],
    # Package loader 0x555860 / visitor 0x517ee0
    'load-free-flag': [('        if (freeBlock) FEngFreeQueueNode(block);', '        if (!freeBlock) FEngFreeQueueNode(block);')],
    'load-reuse-flag': [('        package->value15 = 1;\n        package->VisitObjects(&visitor);', '        package->VisitObjects(&visitor);')],
    'load-level': [('package->value18 = level;', 'package->value18 = mask;')],
    'load-announce': [('    if (value10C) value10C->PackageLoaded(package);\n', '')],
    'load-unlink': [('        ((FEngQueueList *)&packagesEC)->Remove((FEngQueueNode *)package);\n    } else {', '    } else {')],
    'load-separator': [("while (c != '/' && c != '\\\\' && length > 0) {", "while (c != '/' && length > 0) {")],
    'load-start-adjust': [('        if (length != 0) ++start;', '        ++start;')],
    'visit-dirty-flags': [('object->flags1C |= 0x3c00000;', 'object->flags1C |= 0x3c00001;')],
    'visit-script-hash': [('while (script && script->hash30 != 0x1744b3) script = script->next4;', 'while (script && script->hash30 != 0x1744b2) script = script->next4;')],
}


def run_mutations(args):
    rejected = []
    text = args.source.read_text()
    def job(item):
        label, edits = item
        mutated = text
        for before, after in edits:
            assert before in mutated, (label, before)
            mutated = mutated.replace(before, after, 1)
        path = BUILD / ('mutation-' + label + '.cpp'); path.write_text(mutated)
        run = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(path), '--label', 'mutation-' + label,
                              '--cases', str(args.mutation_cases)], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (BUILD / ('mutation-' + label + '.log')).write_text(run.stdout)
        assert run.returncode and 'state or call order mismatch' in run.stdout, (label, 'not a semantic rejection', run.stdout[-1500:])
        return label
    with ThreadPoolExecutor(max_workers=4) as pool:
        for label in pool.map(job, MUTATIONS.items()): rejected.append(label)
    return rejected


if __name__ == '__main__': main()
