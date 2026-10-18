#!/usr/bin/env python3
"""Compare FEng frame manager 0x558c00 and its pad/mouse latches 0x504fa0/0x50b0c0.

The latches run as original code under the original frame manager and as rebuilt code
under the rebuilt one. Pad dispatch 0x54d4b0, mouse dispatch 0x54e020, message delivery
0x541fc0, operation consumer 0x5559c0 and object update 0x54ce60 are traced stubs at their
original addresses (delivery and operations are recovered and verified separately); the
dispatch stub sets consumed bits and the delivery/operation stubs set the message-list
head, from a sequence shared by both runs. Observer slots 0x40/0x44/0x5c/0x60 are fixture
vtable stubs."""

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
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-frame'
ARENA, ARENA_SIZE, STACK, STOP, STUBS = 0x2000000, 0x8000, 0x2010000, 0x2100000, 0x2101000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
QUEUE, PADS, PACKAGES, OBJECTS, VT, PACKET = (ARENA + x for x in (0x0, 0x5400, 0x5800, 0x5c00, 0x6000, 0x6100))
TARGETS = {'Update': ('?Update@FEngQueue@', 2), 'PadLatch': ('?Latch@FEngPadState@', 2),
           'MouseLatch': ('?Latch@FEngMouseState@', 2)}
# Original address: (symbol prefix, stack cleanup)
CALLEES = {0x54d4b0: ('?DispatchPad@FEngQueue@', 4), 0x54e020: ('?DispatchMouse@FEngQueue@', 4),
           0x541fc0: ('?DeliverMessages@FEngQueue@', 0), 0x5559c0: ('?ConsumeOperations@FEngQueue@', 0),
           0x54ce60: ('?UpdateObject@FEPackage@', 8)}
SLOTS = {0x40: (STUBS + 0x00, 4), 0x44: (STUBS + 0x20, 4), 0x5c: (STUBS + 0x40, 0), 0x60: (STUBS + 0x60, 0)}
SHARED = {'?fengFlagMask7F7C8C@': 0x7f7c8c}
SHARED.update({p: a for a, (p, _) in CALLEES.items()})


def stub_code(cleanup):
    return b'\xc2' + struct.pack('<H', cleanup) if cleanup else b'\xc3'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngFrame.cpp')
    parser.add_argument('--cases', type=int, default=2048)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-frame.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = bytearray(pefile.PE(data=raw).get_memory_mapped_image())
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    for address, (_, cleanup) in CALLEES.items():
        code = stub_code(cleanup); image[address - 0x400000:address - 0x400000 + len(code)] = code
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/O2', '/Ob0',
                    '/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)],
                   cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0 or s['storage'] != 2: continue
        match = [a for p, a in SHARED.items() if s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _) in TARGETS.items()}
    recovered = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines, traces, fixture = [], [[], []], {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, bytes(image))
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x2000)
        for slot, (address, cleanup) in SLOTS.items(): uc.mem_write(address, stub_code(cleanup) if slot != 0x40 else b'\xa1' + struct.pack('<I', STUBS + 0x200) + stub_code(cleanup))
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in recovered), ('state or call order mismatch', 'original reached', hex(address))
            elif fixture.get('target') == 'Update':
                assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, trace=traces[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            cleanup = CALLEES[address][1] if address in CALLEES else next(c for a, c in SLOTS.values() if a == address)
            argv = bytes(uc.mem_read(esp + 4, cleanup))
            if address == SLOTS[0x40][0]: argv = argv[:1]  # only the pad byte is defined
            extra = ()
            if address == 0x54ce60:
                extra = (bytes(uc.mem_read(this + 0x30, 8)), bytes(uc.mem_read(0x7f7c8c, 4)))
            elif address in (0x541fc0, 0x5559c0):
                extra = (bytes(uc.mem_read(QUEUE + 0x5250, 1)), bytes(uc.mem_read(QUEUE + 0x411c, 4)))
                uc.mem_write(QUEUE + 0x411c, struct.pack('<I', fixture['heads'].pop() if fixture['heads'] else 0))
            elif address == 0x54d4b0:
                uc.mem_write(QUEUE + 0x34, struct.pack('<I', fixture['consumed'].pop() if fixture['consumed'] else 0))
            elif address == SLOTS[0x40][0]:
                uc.mem_write(STUBS + 0x200, struct.pack('<I', fixture['buttons'].pop() if fixture['buttons'] else 0))
            elif address == SLOTS[0x44][0]:
                packet = struct.unpack('<I', argv)[0]
                uc.mem_write(packet, fixture['packets'].pop() if fixture['packets'] else bytes(8)); argv = b''
            trace.append((hex(address), this, argv) + extra)
        uc.hook_add(UC_HOOK_BLOCK, block)
        for address in list(CALLEES) + [a for a, _ in SLOTS.values()]:
            uc.hook_add(UC_HOOK_CODE, stub, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x558c00); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def run(name, memory, argv, this, sequences):
        argc = TARGETS[name][1]
        outputs = []
        fixture['target'] = name
        for rebuilt, uc in zip((False, True), machines):
            traces[rebuilt].clear()
            fixture.update({k: list(v) for k, v in sequences.items()})
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(0x7f7c8c, struct.pack('<I', 0x5a5a5a5a))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=2000000)
            except (UcError, AssertionError) as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            state = bytes(uc.mem_read(ARENA, ARENA_SIZE)) + bytes(uc.mem_read(0x7f7c8c, 4))
            outputs.append((uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), list(traces[rebuilt]), state))
        assert outputs[0][0] == STACK + 4 + 4 * argc, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][3][i:i + 4].hex(), outputs[1][3][i:i + 4].hex()) for i in range(0, len(outputs[0][3]), 4) if outputs[0][3][i:i + 4] != outputs[1][3][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, outputs[0][:2], outputs[1][:2],
                                  [t[0] for t in outputs[0][2]][:40], [t[0] for t in outputs[1][2]][:40],
                                  next(((i, a, b) for i, (a, b) in enumerate(zip(outputs[0][2], outputs[1][2])) if a != b), None), diff))
        counts[name] += 1
        return outputs[0]

    def note(path): paths[path] = paths.get(path, 0) + 1
    def put(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def bits(): return rng.choice((0, 0xffffffff, 1, 0x80000000, rng.getrandbits(32), rng.getrandbits(32) & rng.getrandbits(32)))

    for trial in range(args.cases):
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # Queue flags +0/+1/+2 and +0x5251; packages chained from +0xe4 with objects from +0x4c.
        memory[0] = rng.choice((0, 1, 1, 1, 2)); memory[1] = rng.choice((0, 1, 1)); memory[2] = rng.choice((0, 1))
        memory[0x5250] = rng.choice((0, 0, 1)); memory[0x5251] = rng.choice((0, 1))
        pads = rng.choice((0, 1, 2, 4))
        put(memory, QUEUE + 8, '<I', PADS); put(memory, QUEUE + 0xd4, '<I', pads); put(memory, QUEUE + 0x10c, '<I', ARENA + 0x6200); put(memory, ARENA + 0x6200, '<I', VT)
        for i in range(19): put(memory, QUEUE + 0x38 + 4 * i, '<I', rng.choice((0, 5, 20, 100, 0xffffffff, rng.getrandbits(32))))
        for slot, (address, _) in SLOTS.items(): put(memory, VT + slot, '<I', address)
        packages = [PACKAGES + i * 0x98 for i in range(rng.choice((0, 1, 2, 3, 4)))]
        put(memory, QUEUE + 0xe4, '<I', packages[0] if packages else 0)
        for i, package in enumerate(packages):
            put(memory, package + 4, '<I', packages[i + 1] if i + 1 < len(packages) else 0)
            objects = [OBJECTS + (i * 4 + j) * 0x40 for j in range(rng.choice((0, 1, 2, 3)))]
            put(memory, package + 0x4c, '<I', objects[0] if objects else 0)
            for j, obj in enumerate(objects): put(memory, obj + 4, '<I', objects[j + 1] if j + 1 < len(objects) else 0)
        for i in range(4):
            for j in range(32): put(memory, PADS + i * 0x88 + 8 + 4 * j, '<I', rng.choice((0, 3, 50, 0xfffffff0, rng.getrandbits(32))))
            put(memory, PADS + i * 0x88, '<II', bits(), bits())
        delta = rng.choice((0, 1, 19, 20, 21, 40, 45, 100, -5, rng.randrange(0, 250)))
        unlimited = rng.choice((0, 0, 1, 0x100))
        sequences = dict(heads=[rng.choice((0, 0, ARENA + 0x7000)) for _ in range(40)], consumed=[bits() for _ in range(8)],
                         buttons=[bits() for _ in range(8)],
                         packets=[struct.pack('<4h', *(rng.choice((0, 1, -1, 300, rng.randrange(-32768, 32768))) for _ in range(4))) for _ in range(2)])
        result = run('Update', memory, [delta, unlimited], QUEUE, sequences)
        sites = [t[0] for t in result[2]]
        if memory[0] and memory[1]: note('input')
        if '0x54e020' in sites: note('mouse-dispatch')
        if sites.count('0x541fc0') > sites.count('0x5559c0') and '0x5559c0' in sites: note('second-drain')
        if sites.count('0x5559c0') > 1: note('multi-step')
        if not memory[0]: note('unstepped')
        # Latches alone.
        pad = PADS + rng.randrange(4) * 0x88
        run('PadLatch', memory, [bits(), delta], pad, {}); note('pad-latch')
        put(memory, PACKET, '<4h', *(rng.choice((0, 1, -1, 7, rng.randrange(-32768, 32768))) for _ in range(4)))
        mouse = QUEUE + 0xc
        if rng.random() < 0.3: memory[0xc:0x14] = memory[PACKET - ARENA:PACKET - ARENA + 2] + b'\0\0' + memory[PACKET - ARENA + 2:PACKET - ARENA + 4] + b'\0\0'
        run('MouseLatch', memory, [PACKET, delta], mouse, {}); note('mouse-latch')
    report = dict(comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  traced_stubs=sorted(hex(a) for a in CALLEES) + ['observer slots 0x40/0x44/0x5c/0x60'],
                  limitations=['finite generated values; exhaustive coverage not established',
                               'pad/mouse dispatch 0x54d4b0/0x54e020 are not recovered; delivery, operations and object update are verified separately',
                               'byte identity not established'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'step-limit': ('remaining > 20) step = 20;', 'remaining > 21) step = 21;'),
            'unlimited': ('if (!unlimited && remaining > 20)', 'if (remaining > 20)'),
            'mask': ('value5250 ? 0 : 0xffffffff', 'value5250 ? 0xffffffff : 0'),
            'second-drain': ('if (messages4114.first8) DeliverMessages();', 'DeliverMessages();'),
            'consume-order': ('                DeliverMessages();\n                ConsumeOperations();', '                ConsumeOperations();\n                DeliverMessages();'),
            'consumed-clear': ('                *consumed = 0;\n', ''),
            'consumed-saturate': ('if (*hold > amount) *hold -= amount;\n                                else *hold = 0;', '*hold -= amount;'),
            'pad-hold': ('if (previous0 & bit) *hold += delta;', 'if (current4 & bit) *hold += delta;'),
            'mouse-moved': ('packet->x0 != x0 || packet->y2 != y4', 'packet->x0 != x0'),
            'mouse-buttons': ('for (int i = 0; i < 3; ++i, ++hold) {', 'for (int i = 0; i < 2; ++i, ++hold) {'),
            'copy-30': ('            value2C = value30;\n', ''),
            'mouse-dispatch': ('if (mouse2) DispatchMouse(package);', 'DispatchMouse(package);'),
            'end-frame': ('done:\n    if (value5251) value10C->EndFrame();', 'done:\n    ;'),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = args.source.parent / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            try:
                result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '1024'], cwd=ROOT, text=True,
                                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            finally:
                mutant.unlink()
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
