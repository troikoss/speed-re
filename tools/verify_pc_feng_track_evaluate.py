#!/usr/bin/env python3
"""Compare FEng track evaluators 0x52bdc0, 0x519d40, 0x5354f0 and 0x541f60.

Both runs execute the same original key lookups, interpolators, quaternion
multiply/assign and reference resolve (themselves verified by
verify_pc_feng_interpolation.py and verify_pc_hud_freeroam.py); every call into
them is traced with its stack arguments. Page zero is mapped with fixture bytes
so the original's null-key read in ping-pong mode is compared, not avoided."""
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
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-track-evaluate'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x4000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
TARGETS = {'Evaluate': ('?FEngEvaluateTrack@', 3), 'Step': ('?FEngStepTrack@', 3),
           'Update': ('?FEngUpdateTrack@', 3), 'ApplyScriptTrack': ('?FEngApplyScriptTrack@', 4)}
# Shared original callees: (symbol prefix, address, traced stack argument count)
SHARED = {'FindKey': ('?FindKey@FEScriptTrack@', 0x5052b0, 1), 'FindKeyOrBase': ('?FindKeyOrBase@FEScriptTrack@', 0x505260, 1),
          'FirstKey': ('?FirstKey@FEScriptTrack@', 0x4fe8d0, 0), 'Resolve': ('?Resolve@FEngReference@', 0x4fe890, 0),
          'LerpInteger': ('?FEngLerpInteger@', 0x5050d0, 5), 'LerpVector2': ('?FEngLerpVector2@', 0x505110, 5),
          'LerpVector3': ('?HudLerpVector3@', 0x505150, 5), 'Slerp': ('?FEngSlerpQuaternion@', 0x519b00, 5),
          'LerpColour': ('?HudLerpColour@', 0x5051a0, 5), 'Multiply': ('?Multiply@FEngQuaternion@', 0x4fe6b0, 2),
          'Assign': ('?Assign@FEngQuaternion@', 0x4955f0, 1),
          'Evaluate': ('?FEngEvaluateTrack@', 0x52bdc0, 3), 'Step': ('?FEngStepTrack@', 0x519d40, 3)}
VALUES = {'?fengZero7D7D3C@': 0x7d7d3c, '?fengOne784250@': 0x784250, '?fengEpsilon7845A0@': 0x7845a0, '__fltused': 0}
TRACK, KEYS, OUT, SCRIPT, OBJECT, STATE = (ARENA + x for x in (0x100, 0x400, 0x1000, 0x1100, 0x1200, 0x1400))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngTrackEvaluate.cpp')
    parser.add_argument('--cases', type=int, default=8192)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    config = ROOT / 'config/SPEED2_EXE_1_2'
    manifest = json.loads((config / 'feng-track-evaluate.json').read_text())
    shared_manifest = json.loads((config / 'feng-interpolation.json').read_text())['functions']
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values()) + list(shared_manifest.values()):
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
        match = [a for p, a in VALUES.items() if s['name'].startswith(p)] + [a for p, a, _ in SHARED.values() if s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _) in TARGETS.items()}
    original = {n: int(f['address'], 16) for n, f in manifest['functions'].items()}
    target_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    traced = {a: (n, argc) for n, (_, a, argc) in SHARED.items() if n not in TARGETS}
    rebuilt_internal = {}  # rebuilt evaluator entries traced as their original names
    for name in ('Evaluate', 'Step'): rebuilt_internal[entries[name]] = (name, 3)
    state, machines = {}, []
    hits = {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0, 0x1000)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        calls = dict(traced)
        if rebuilt: calls.update(rebuilt_internal)
        else: calls.update({original[n]: (n, 3) for n in ('Evaluate', 'Step')})
        def block(uc, address, size, user, rebuilt=rebuilt, calls=calls):
            if address in calls:
                name, argc = calls[address]; sp = uc.reg_read(xc.UC_X86_REG_ESP)
                if not rebuilt: hits[name] = hits.get(name, 0) + 1
                # Stack temporaries differ by frame layout; compare them as a token.
                norm = lambda v: 'stack' if STACK - 0x8000 <= v < STACK + 0x100 else v
                state['trace'].append((name, norm(uc.reg_read(xc.UC_X86_REG_ECX)) if name in ('FindKey', 'FindKeyOrBase', 'FirstKey', 'Resolve', 'Multiply', 'Assign') else None,
                                       tuple(norm(v) for v in struct.unpack('<%dI' % argc, uc.mem_read(sp + 4, 4 * argc))) if argc else ()))
            if rebuilt: assert not any(a <= address < b for a, b in target_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        uc.hook_add(UC_HOOK_BLOCK, block); machines.append(uc)
    rng = random.Random(0x52bdc0); counts = dict.fromkeys(TARGETS, 0); modes = {}

    def run(name, memory, page0, argv, precision):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(0, page0); uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, 0xcafe0000)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            state['trace'] = []
            try: uc.emu_start(entries[name] if rebuilt else original[name], STOP, count=200000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            outputs.append((uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW),
                            uc.reg_read(xc.UC_X86_REG_FPTAG), bytes(uc.mem_read(ARENA, ARENA_SIZE)), bytes(uc.mem_read(0, 0x40)), state['trace']))
        assert outputs[0][0] == STACK + 4 and outputs[0][2] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][4][i:i + 4].hex(), outputs[1][4][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][4][i:i + 4] != outputs[1][4][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), [hex(x & 0xffffffff) for x in argv], outputs[0][:4], outputs[1][:4], diff, outputs[0][6], outputs[1][6]))
        counts[name] += 1

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE)); page0 = rng.randbytes(0x1000)
        put = lambda a, v, fmt='<I': struct.pack_into(fmt, memory, a - ARENA, v & 0xffffffff if fmt == '<I' else v)
        type_ = rng.choice((1, 2, 3, 4, 5, 6, 6, 4, 0, 7))
        interp = rng.choice((1, 1, 1, 3, 0, 0, 2))
        cycle = rng.choice((0, 0, 1, 1, 2, 3)) | rng.choice((0, 0, 0x80))
        size = rng.choice((4, 8, 12, 16, 13, 0))
        duration = rng.choice((0, 100, 1000, 2500, rng.randrange(-50, 5000), 0xffffff))
        memory[TRACK - ARENA:TRACK - ARENA + 4] = bytes((type_, size, interp, cycle))
        put(TRACK + 4, (rng.getrandbits(8) << 24) | (duration & 0xffffff))
        count = rng.choice((0, 1, 2, 3, 5, rng.randrange(1, 9)))
        keys = [KEYS + i * 0x40 for i in range(count)]
        times = sorted(rng.choice((0, 1, 50, duration, duration - 1, rng.randrange(-20, 5000))) for _ in range(count))
        if rng.random() < 0.1: rng.shuffle(times)
        for i, key in enumerate(keys):
            put(key + 4, keys[i + 1] if i + 1 < count else 0); put(key + 8, keys[i - 1] if i else 0); put(key + 0xc, times[i] & 0xffffffff)
            for j in range(4):
                if type_ in (1, 6): put(key + 0x10 + 4 * j, rng.choice((0, 255, rng.randrange(-300, 300), rng.getrandbits(32))))
                else: put(key + 0x10 + 4 * j, rng.uniform(-500, 500), '<f')
        for j in range(4):
            if type_ in (1, 6): put(TRACK + 0x18 + 4 * j, rng.choice((0, 7, rng.randrange(-300, 300))))
            else: put(TRACK + 0x18 + 4 * j, rng.uniform(-50, 50), '<f')
        if type_ == 5 and rng.random() < 0.7:  # unit quaternions for the slerp path
            for address in [TRACK + 0x18] + [k + 0x10 for k in keys]:
                v = [rng.gauss(0, 1) for _ in range(4)]; n = sum(x * x for x in v) ** 0.5
                struct.pack_into('<4f', memory, address - ARENA, *(x / n for x in v))
        # Inline base key links and time: zero, or real keys for the step evaluator.
        put(TRACK + 0xc, rng.choice((0, keys[0] if keys else 0))); put(TRACK + 0x10, rng.choice((0, keys[-1] if keys else 0)))
        put(TRACK + 0x14, rng.choice((0, -1, 10, 2000)))
        flag = rng.choice((0, 0, 0, 1))
        if flag:
            put(TRACK + 0x200 + 4, 0); put(TRACK + 0x200 + 8, keys[0] if keys else 0); pointer = TRACK + 0x200
        else: pointer = keys[0] if keys else 0
        memory[TRACK + 0x2c - ARENA] = flag; put(TRACK + 0x30, pointer)
        time = rng.choice((0, -1, -5, 1, duration, duration + 1, 2 * duration, rng.randrange(-100, 6000)) + tuple(times) + tuple(t + 1 for t in times))
        if trial % 20 == 0:  # scalar cancellation: x + lower + base differs from x + base + lower
            memory[TRACK - ARENA:TRACK - ARENA + 4] = bytes((2, 4, 1, 0)); put(TRACK + 0x2c - ARENA + ARENA, 0)
            memory[TRACK + 0x2c - ARENA] = 0; put(TRACK + 0x30, KEYS)
            put(KEYS + 4, KEYS + 0x40); put(KEYS + 8, 0); put(KEYS + 0xc, 0); put(KEYS + 0x10, 1.0, '<f')
            put(KEYS + 0x44, 0); put(KEYS + 0x48, KEYS); put(KEYS + 0x4c, 2); put(KEYS + 0x50, 1.0 - 2.0 ** 71, '<f')
            put(TRACK + 0x18, 2.0 ** 70, '<f'); time = 1; interp, cycle = 1, 0
        out = rng.choice((OUT, OUT, OUT, TRACK + 0x18))  # aliasing the base pins read order
        modes[(interp, cycle & 0x7f)] = modes.get((interp, cycle & 0x7f), 0) + 1
        if interp in (1, 3) and not cycle & 0x80: run('Evaluate', memory, page0, [TRACK, time, out], precision)
        if interp == 0 and not cycle & 0x80: run('Step', memory, page0, [TRACK, time, out], precision)
        run('Update', memory, page0, [TRACK, time, out], precision)
        # Script/object wrapper: track index, state offset (+7) into object +0x2c.
        index = rng.randrange(0, 3)
        put(SCRIPT + 0x20, TRACK - index * 0x38); memory[TRACK + 7 - ARENA] = rng.randrange(-8, 12) & 0xff
        put(OBJECT + 0x2c, STATE)
        if not cycle & 0x80 or True:
            run('ApplyScriptTrack', memory, page0, [SCRIPT, index | (rng.getrandbits(24) << 8), time, OBJECT], precision)
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), shared_callee_hits=hits,
                  mode_counts={'%d/%d' % k: v for k, v in sorted(modes.items())}, x87_precision=['53-bit', '64-bit'],
                  limitations=['finite generated tracks; callee bodies shared and verified separately',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'loop-wrap-no-duration': ('(unsigned int)TrackDuration(track) + (unsigned int)upper->timeC', '(unsigned int)upper->timeC'),
            'loop-equal-upper': ('                } else if (upperTime == time) {\n                    lower = upper;', '                } else if (upperTime == time) {\n                    lower = upper->previous8 ? upper->previous8 : upper;'),
            'loop-first-base-time': ('lower = track->FindKeyOrBase(duration);', 'lower = track->FindKeyOrBase(time);'),
            'pingpong-no-mirror': ('if (time > duration) mirrored', 'if (0) mirrored'),
            'zero-fraction-upper': ('fraction == fengZero7D7D3C ? (const unsigned int *)((char *)lower + 0x10)', 'fraction == fengZero7D7D3C ? (const unsigned int *)upper->value10'),
            'scalar-lerp-order': ('*(float *)out = (*(float *)b - l) * fraction + l + *(const float *)base;', '*(float *)out = (*(float *)b - l) * fraction + *(const float *)base + l;'),
            'colour-constant-float': ('    case 6: {\n        unsigned int *destination = (unsigned int *)out;\n        destination[2] = value[2] + base[2];', '    case 6: {\n        unsigned int *destination = (unsigned int *)out;\n        ((float *)destination)[2] = ((const float *)value)[2] + ((const float *)base)[2];'),
            'base-copy-type5-short': ('        case 5: case 6:\n            destination[0] = base[0]; destination[3] = base[3];', '        case 5: case 6:\n            destination[0] = base[0];'),
            'step-strict-duration': ('if (time <= duration) {', 'if (time < duration) {'),
            'step-ignore-previous': ('if (key->previous8 && key->timeC > time) {', 'if (0) {'),
            'step-size-plus-four': ('unsigned int size = track->value1;', 'unsigned int size = track->value1 + 4;'),
            'update-ignore-skip': ('if ((signed char)track->flags3 < 0) return;', ''),
            'update-no-mark': ('if (!count) track->flags3 |= 0x80;', ''),
            'update-interp3-step': ('    case 1: case 3: FEngEvaluateTrack(track, time, out); break;\n    }\n    int count', '    case 1: FEngEvaluateTrack(track, time, out); break;\n    case 3: FEngStepTrack(track, time, out); break;\n    }\n    int count'),
            'apply-unsigned-state': ('FEngEvaluateTrack(track, time, object->data2C + track->state7);', 'FEngEvaluateTrack(track, time, object->data2C + (unsigned char)track->state7);'),
            'duration-unsigned': ('return (int)(*(const unsigned int *)track->unidentified4 << 8) >> 8;', 'return (int)(*(const unsigned int *)track->unidentified4 & 0xffffff);'),
        }
        rejected, survivors = [], []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '4096'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            if result.returncode and 'state or call order mismatch' in result.stdout: rejected.append(label)
            else: survivors.append(label)
        report['rejected_mutations'] = rejected
        assert not survivors, ('surviving mutations', survivors)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
