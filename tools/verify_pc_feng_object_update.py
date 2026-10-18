#!/usr/bin/env python3
"""Compare FEng script playback: per-object update 0x54ce60, script apply 0x541db0,
event firing 0x534c70, group recursion 0x504910 and type-6 tick 0x504dc0.

Track evaluators, queue posts, focus, GUID lookup, script initialization, list
tick, the observer's game-message slot and the type-6 callback are traced,
controlled boundaries (their bodies are verified by other tools); each records
its receiver, arguments and an arena digest at the call."""
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

BUILD = ROOT / 'build/analysis/feng-object-update'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x8000, 0x2018000, 0x2100000
STUB_OBSERVER, STUB_CALLBACK = STOP + 0x100, STOP + 0x200
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
TARGETS = {'UpdateObject': ('?UpdateObject@FEPackage@', 2, 8), 'ApplyScript': ('?ApplyScript@FEPackage@', 2, 8),
           'FireScriptEvents': ('?FireScriptEvents@FEPackage@', 5, 0x14), 'UpdateChildren': ('?UpdateChildren@FEPackage@', 2, 8)}
# name: (address, symbol prefix, stack arguments, callee cleanup, thiscall)
STUBS = {'UpdateTrack': (0x5354f0, '?FEngUpdateTrack@', 3, False, False), 'EvaluateTrack': (0x52bdc0, '?FEngEvaluateTrack@', 3, False, False),
         'StepTrack': (0x519d40, '?FEngStepTrack@', 3, False, False), 'Post': (0x51aa20, '?Post@FEngQueue@', 5, True, True),
         'SetFocus': (0x52b3a0, '?SetFocus@FEPackage@', 3, True, True), 'FindObjectByGuid': (0x517fe0, '?FindObjectByGuid@FEPackage@', 1, True, True),
         'InitializeScript': (0x517720, '?InitializeScript@HudVisibilityObject@', 0, True, True), 'ListTick': (0x517230, '?Tick@FEListObject@', 1, True, True),
         'GameMessage': (STUB_OBSERVER, None, 4, True, True), 'Callback': (STUB_CALLBACK, None, 1, False, False)}
VALUES = {'?fengFlagMask7F7C8C@': 0x7f7c8c, '?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250, '__fltused': 0}
INTERNAL = {'Type6Tick': '?FEngTickType6@'}


class Layout:
    def __init__(self, rng, memory):
        self.rng, self.memory, self.cursor, self.guids = rng, memory, ARENA + 0x400, {}

    def alloc(self, size):
        address = self.cursor; self.cursor += (size + 15) & ~15
        assert self.cursor < ARENA + ARENA_SIZE - 0x400, 'fixture capacity'
        return address

    def put(self, address, value, fmt='<I'):
        struct.pack_into(fmt, self.memory, address - ARENA, value & 0xffffffff if fmt == '<I' else value)

    def script(self, chained=True):
        rng = self.rng; s = self.alloc(0x40)
        length = rng.choice((0, 1, 100, 500, 1000, -5, rng.randrange(1, 4000)))
        self.put(s + 0xc, length); self.put(s + 0x10, rng.choice((0, 1, length, length + 1, length - 1, rng.randrange(-20, 5000))))
        self.put(s + 0x14, rng.getrandbits(32)); self.put(s + 0x18, self.script(False) if chained and rng.random() < 0.25 else 0)
        count = rng.choice((0, 1, 2, 3, 5))
        tracks = self.alloc(0x38 * max(count, 1)) if count or rng.random() < 0.5 else 0
        self.put(s + 0x1c, count | (rng.choice((0, 0, 0x100, 0x7f00)) << 8 if count else 0)); self.put(s + 0x20, tracks)
        for i in range(count):
            t = tracks + i * 0x38
            self.memory[t - ARENA:t - ARENA + 4] = bytes((rng.randrange(7), rng.choice((4, 8, 12, 16)), rng.choice((0, 1, 3, 2, 4)), rng.randrange(256)))
            self.put(t + 4, (rng.choice((0, 0, 1, 7, 0x10, 0x14, 0xfc)) << 24) | rng.getrandbits(24))
        if tracks and count == 0: self.put(tracks + 4, rng.choice((0, 0x01000000)) | rng.getrandbits(24))
        events = rng.choice((0, 0, 1, 2, 4))
        table = self.alloc(12 * max(events, 1))
        self.put(s + 0x24, events); self.put(s + 0x28, table)
        times = sorted(rng.choice((0, length, length + 1, rng.randrange(-5, 5000))) for _ in range(events))
        for i, time in enumerate(times):
            message = rng.choice((0x1b3909aa, 0x12345678, rng.getrandbits(32)))
            target = rng.choice((0, 0xffffffff, 0xfffffffc, 0xfffffffb, 0xfffffffa, 0xfffffffe, 0xfffffffd, rng.randrange(1, 64)))
            self.put(table + i * 12, message); self.put(table + i * 12 + 4, target); self.put(table + i * 12 + 8, time)
        return s

    def object(self, depth=0):
        rng = self.rng; o = self.alloc(0xc0)
        type_ = rng.choice((1, 2, 3, 4, 6, 7) + ((5, 5) if depth < 2 else ()))
        self.put(o + 0x18, type_); self.put(o + 0x1c, rng.getrandbits(32) & ~0 if rng.random() < 0.5 else rng.choice((0, 0x400000, 0x1c00000, 0x800000)))
        state = self.alloc(0x80) + 0x20
        self.put(o + 0x2c, state); self.put(state + 0xc, rng.choice((0, 0, 0xff, rng.getrandbits(32))))
        self.put(o + 0x54, self.script())
        self.put(o + 0x5c, rng.getrandbits(32))
        if type_ == 6:
            self.put(o + 0xa4, rng.choice((0.0, 0.5, 0.999, 1.0, -0.1)), '<f'); self.put(o + 0xa8, rng.choice((0.001, -0.002, 0.0, 1.5, -0.5)), '<f')
            self.put(o + 0xbc, rng.choice((0, STUB_CALLBACK)))
        children = []
        if type_ == 5:
            children = [self.object(depth + 1) for _ in range(rng.choice((0, 1, 2)))]
            for a, b in zip(children, children[1:] + [0]): self.put(a + 4, b)
        self.put(o + 0x64, children[0] if children else 0)
        return o


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngObjectUpdate.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-object-update.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-builtin'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        match = [a for p, a in VALUES.items() if s['name'].startswith(p)] + [a for a, p, _, _, _ in STUBS.values() if p and s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    entries['Type6Tick'] = next(a for s, a in linked['exports'].items() if s.startswith(INTERNAL['Type6Tick']))
    original = {n: int(f['address'], 16) for n, f in manifest['functions'].items()}
    target_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    stubs = {a: (n, argc, cleanup, thiscall) for n, (a, _, argc, cleanup, thiscall) in STUBS.items()}
    state, machines, hits = {}, [], {}
    def digest(uc): return hashlib.sha256(bytes(uc.mem_read(ARENA, ARENA_SIZE))).hexdigest()[:16]
    def put(uc, a, v): uc.mem_write(a, struct.pack('<I', v & 0xffffffff))
    def word(uc, a): return struct.unpack('<I', uc.mem_read(a, 4))[0]
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        entry_names = {(entries if rebuilt else original)[n]: n for n in ('UpdateObject', 'ApplyScript', 'FireScriptEvents', 'UpdateChildren', 'Type6Tick')}
        def hook(uc, address, size, user, rebuilt=rebuilt, entry_names=entry_names):
            if address in entry_names:
                sp = uc.reg_read(xc.UC_X86_REG_ESP)
                state['trace'].append(('enter', entry_names[address], uc.reg_read(xc.UC_X86_REG_ECX), word(uc, sp + 4), digest(uc)))
            if address not in stubs:
                if rebuilt: assert not any(a <= address < b for a, b in target_ranges), ('state or call order mismatch', 'original target reached', hex(address))
                else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
                return
            name, argc, cleanup, thiscall = stubs[address]; sp = uc.reg_read(xc.UC_X86_REG_ESP)
            if not rebuilt: hits[name] = hits.get(name, 0) + 1
            this = uc.reg_read(xc.UC_X86_REG_ECX) if thiscall else None
            argv = struct.unpack('<%dI' % argc, uc.mem_read(sp + 4, 4 * argc)) if argc else ()
            state['trace'].append((name, this, argv, digest(uc)))
            result = 0
            if name in ('UpdateTrack', 'EvaluateTrack', 'StepTrack'):
                track, time, out = argv
                put(uc, out, hash((name, track, time)) & 0xffffffff)
                if name == 'UpdateTrack' and (track ^ time) & 0x10:  # deterministic keyless marking
                    flags = uc.mem_read(track + 3, 1)[0]; uc.mem_write(track + 3, bytes((flags | 0x80,)))
            elif name == 'FindObjectByGuid':
                result = 0 if argv[0] % 5 == 0 else ARENA + 0x100 + (argv[0] & 0x3f) * 4
            ret = word(uc, sp)
            uc.reg_write(xc.UC_X86_REG_EAX, result); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4 + (4 * argc if cleanup else 0)); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    rng = random.Random(0x54ce60); counts = dict.fromkeys(TARGETS, 0)

    def run(name, memory, this, argv, precision):
        _, argc, cleanup = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(0x7f7c8c, struct.pack('<I', state['mask']))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state['trace'] = []
            try: uc.emu_start(entries[name] if rebuilt else original[name], STOP, count=500000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            outputs.append((uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW),
                            bytes(uc.mem_read(ARENA, ARENA_SIZE)), state['trace']))
        assert outputs[0][0] == STACK + 4 + cleanup, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][3][i:i + 4].hex(), outputs[1][3][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][3][i:i + 4] != outputs[1][3][i:i + 4]][:8]
            t0, t1 = [x[:4] for x in outputs[0][4]], [x[:4] for x in outputs[1][4]]
            first = next((i for i, (a, b) in enumerate(zip(t0, t1)) if a != b), min(len(t0), len(t1)))
            raise AssertionError(('state or call order mismatch', name, outputs[0][:3], outputs[1][:3], diff, first, t0[first:first + 3], t1[first:first + 3]))
        counts[name] += 1

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        state['mask'] = rng.choice((0xffffffff, 0))  # 0x558c00 stores 0 while queue byte +0x5250 is set
        memory = bytearray(rng.randbytes(ARENA_SIZE)); layout = Layout(rng, memory)
        package, queue, observer = ARENA + 0x40, ARENA + 0x300, ARENA + 0x380
        memory[package + 0x14 - ARENA] = rng.choice((0, 1, 1, 2, 0xff))
        layout.put(package + 0x30, queue); layout.put(package + 0x34, rng.choice((0, 16, 33, -40, 1000, rng.randrange(-100, 3000))))
        layout.put(queue + 0x10c, observer); layout.put(observer, observer + 0x40); layout.put(observer + 0x40 + 0x18, STUB_OBSERVER)
        obj = layout.object()
        script = word_at = struct.unpack_from('<I', memory, obj + 0x54 - ARENA)[0]
        delta = rng.choice((0, 16, 33, -7, 500))
        run('UpdateObject', memory, package, [obj, delta], precision)
        run('ApplyScript', memory, package, [obj, script], precision)
        frm = rng.choice((0, -1, 5, 100, rng.randrange(-10, 5000))); to = rng.choice((frm, frm + 1, frm - 1, 0, struct.unpack_from('<i', memory, script + 0xc - ARENA)[0], rng.randrange(-10, 5000)))
        run('FireScriptEvents', memory, package, [queue, rng.choice((obj, 0)), script, frm, to], precision)
        if struct.unpack_from('<I', memory, obj + 0x18 - ARENA)[0] == 5: run('UpdateChildren', memory, package, [obj, delta], precision)
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), boundary_hits=hits,
                  x87_precision=['53-bit', '64-bit'],
                  limitations=['controlled evaluator/post/focus/lookup/init/tick/observer boundaries; their bodies verified separately',
                               'fixtures avoid the original\'s null current-script and null chained-script dereferences',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'apply-skip-colour-first': ('        if (track && !(*(unsigned int *)track->unidentified4 & 0xff000000)) {\n            if ((signed char)', '        if (0) {\n            if ((signed char)'),
            'apply-ignore-alpha': ('        if (*(int *)((char *)object->data2C + 0xc) && (count = *(unsigned char *)&script->trackCount1C) != 0) {\n            unsigned int remaining = count;\n            do {\n                if', '        if ((count = *(unsigned char *)&script->trackCount1C) != 0) {\n            unsigned int remaining = count;\n            do {\n                if'),
            'apply-full-count': ('(count = *(unsigned char *)&script->trackCount1C) != 0) {\n            unsigned int remaining = count;\n            do {\n                if', '(count = (unsigned char)(script->trackCount1C ? script->trackCount1C : 0)) != 0 && script->trackCount1C) {\n            unsigned int remaining = script->trackCount1C;\n            do {\n                if'),
            'apply-mask-after-update': ('                if (track->state7 < 0x10) finished &= track->flags3;\n                FEngUpdateTrack(track, time, state + track->state7);', '                FEngUpdateTrack(track, time, state + track->state7);\n                if (track->state7 < 0x10) finished &= track->flags3;'),
            'apply-no-running-flag': ('            else object->flags1C |= 0x800000;', '            else object->flags1C |= 0;'),
            'events-signed-time': ('while ((unsigned int)event[index].time < (unsigned int)from)', 'while (event[index].time < from)'),
            'events-no-length-extend': ('    if (to == script->lengthC) ++to;\n', ''),
            'events-focus-no-relookup': ('SetFocus(queue, FindObjectByGuid(event->target), 1);', 'SetFocus(queue, found, 1);'),
            'events-post-without-sender': ('            else if (object) queue->Post(', '            else queue->Post('),
            'events-observer-mask': ('queue->value10C->GameMessage(event->message, object, 0, this);', 'queue->value10C->GameMessage(event->message, object, 1, this);'),
            'update-no-clamp': ('    if (now < 0) script->time10 = 0;\n', ''),
            'update-chain-no-carry': ('            script->time10 = carry;', '            script->time10 = 0;'),
            'update-once-length': ('            script->time10 = script->lengthC + 1;', '            script->time10 = script->lengthC;'),
            'update-loop-no-init': ('                if (script->eventCount24) FireScriptEvents(queue30, object, script, 0, wrapped);\n            }\n            object->InitializeScript();', '                if (script->eventCount24) FireScriptEvents(queue30, object, script, 0, wrapped);\n            }'),
            'update-pingpong-single': ('script->time10 = now % (length * 2);', 'script->time10 = now % length;'),
            'update-always-apply': ('    if (!value14 || old != script->time10 || old != script->lengthC + 1 || (object->flags1C & 0x400000))', '    if (1)'),
            'update-list-delta-package': ('((FEListObject *)object)->Tick((float)delta);', '((FEListObject *)object)->Tick((float)delta34);'),
            'update-type7-unconditional': ('case 7: if (value14) *(int *)', 'case 7: if (1) *(int *)'),
            'update-final-keep-400000': ('    flags = object->flags1C & (fengFlagMask7F7C8C | 0xffbfffff);', '    flags = object->flags1C;'),
            'children-no-propagate': ('        child->flags1C |= group->flags1C & 0x3c00000;\n', ''),
            'type6-no-reflect-high': ('    } else if (value > fengOne784250) {', '    } else if (0) {'),
        }
        rejected, survivors = [], []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '2048'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            (rejected if result.returncode and 'state or call order mismatch' in result.stdout else survivors).append(label)
        report['rejected_mutations'] = rejected
        assert not survivors, ('surviving mutations', survivors)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
