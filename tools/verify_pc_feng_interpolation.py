#!/usr/bin/env python3
"""Compare FEng track interpolation primitives: key lookups, integer/vector2 lerps,
quaternion helpers and slerp 0x519b00, under 53- and 64-bit x87 precision.

Recovered trig (0x43c350/0x43c490), quaternion multiply 0x4fe6b0, reference resolve
0x4fe890 and the CRT conversion 0x75bed4 execute original code in both runs."""
import argparse
import hashlib
import json
import math
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

BUILD = ROOT / 'build/analysis/feng-interpolation'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x4000, 0x2010000, 0x2100000
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
# name: (symbol prefix, stack argument count, callee cleanup bytes, thiscall)
TARGETS = {'FindKey': ('?FindKey@FEScriptTrack@', 1, 4, True), 'FindKeyOrBase': ('?FindKeyOrBase@FEScriptTrack@', 1, 4, True),
           'FirstKey': ('?FirstKey@FEScriptTrack@', 0, 0, True), 'Assign': ('?Assign@FEngQuaternion@', 1, 4, True),
           'LerpInteger': ('?FEngLerpInteger@', 5, 0, False), 'LerpVector2': ('?FEngLerpVector2@', 5, 0, False),
           'Subtract': ('?FEngQuaternionSubtract@', 3, 0, False), 'Scale': ('?FEngQuaternionScale@', 3, 0, False),
           'Add': ('?FEngQuaternionAdd@', 3, 0, False), 'Normalize': ('?FEngQuaternionNormalize@', 1, 0, False),
           'Slerp': ('?FEngSlerpQuaternion@', 5, 0, False)}
SHARED = {'?EngineSine@': 0x43c350, '?HudArcSineUnits@': 0x43c490, '?Multiply@FEngQuaternion@': 0x4fe6b0,
          '?Resolve@FEngReference@': 0x4fe890, '__ftol2': 0x75bed4, '?fengZero7D7D3C@': 0x7d7d3c,
          '?fengSlerpLimit79B158@': 0x79b158, '?fengAngleUnit784240@': 0x784240, '?fengOne784250@': 0x784250,
          '?fengHalf784260@': 0x784260, '?fengEpsilon7845A0@': 0x7845a0, '__fltused': 0}
RETURNS_POINTER = {'FindKey', 'FindKeyOrBase', 'FirstKey', 'Assign', 'Subtract', 'Scale', 'Add'}


def f32(x): return struct.pack('<f', x)
def bits(x): return struct.unpack('<I', struct.pack('<f', x))[0]


def special(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, 1e-20, -3e-7, 65504.0, rng.uniform(-2, 2), rng.uniform(-1000, 1000),
                       float('inf'), float('nan'), 1.0000001, 0.99999994))


def unit(rng):
    v = [rng.gauss(0, 1) for _ in range(4)]; n = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / n for x in v]


def near(rng, q, scale):
    v = [x + rng.gauss(0, scale) for x in q]; n = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / n for x in v]


def straddle(rng):
    """Float quaternions whose exact dot and float-rounded dot fall on opposite sides of 0.999f."""
    from fractions import Fraction
    limit = Fraction(struct.unpack('<f', struct.pack('<f', 0.999))[0])
    r = lambda x: struct.unpack('<f', struct.pack('<f', x))[0]
    import math as m
    for _ in range(20):
        qa = [r(x) for x in unit(rng)]
        perp = [rng.gauss(0, 1) for _ in range(4)]
        d = sum(x * y for x, y in zip(perp, qa)); perp = [p - d * x for p, x in zip(perp, qa)]
        n = m.sqrt(sum(p * p for p in perp)); perp = [p / n for p in perp]
        c = float(limit); sn = m.sqrt(1 - c * c)
        qb = [r(c * x + sn * p) for x, p in zip(qa, perp)]  # dot within a few float ulps of the limit
        k = max(range(4), key=lambda i: abs(qa[i]))
        for _ in range(300):  # walk qb[k] one ulp at a time across the threshold
            exact = sum(Fraction(x) * Fraction(y) for x, y in zip(qa, qb))
            if (exact < limit) != (Fraction(r(float(exact))) < limit): return qa, qb
            up = (exact < limit) == (qa[k] > 0)
            qb[k] = r(m.nextafter(qb[k], m.inf if up else -m.inf))
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngInterpolation.cpp')
    parser.add_argument('--cases', type=int, default=4096)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-interpolation.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); obj = BUILD / (args.source.stem + '.obj')
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
    subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                              '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(args.source, ROOT)], cwd=ROOT, check=True)
    externals = {}
    for s in read_object(obj)['symbols'].values():
        if s['section'] != 0: continue
        match = [a for p, a in SHARED.items() if s['name'].startswith(p)]
        assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects([obj], externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    machines = []
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            # Rebuilt code must not reach original targets; the original must not reach rebuilt code.
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        uc.hook_add(UC_HOOK_BLOCK, block); machines.append(uc)
    rng = random.Random(0x519b00); counts = dict.fromkeys(TARGETS, 0); paths = {'slerp': 0, 'lerp': 0}
    A, B, BASE, OUT, TRACK = (ARENA + x for x in (0x100, 0x200, 0x300, 0x400, 0x800))

    def run(name, memory, argv, this=0, precision=0x027f):
        _, argc, cleanup, thiscall = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(x if isinstance(x, bytes) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX) if name in RETURNS_POINTER else 0, uc.reg_read(xc.UC_X86_REG_ESP),
                            tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW), uc.reg_read(xc.UC_X86_REG_FPTAG),
                            bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][1] == STACK + 4 + cleanup and outputs[0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            diff = [(hex(ARENA + i), outputs[0][5][i:i + 4].hex(), outputs[1][5][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if outputs[0][5][i:i + 4] != outputs[1][5][i:i + 4]][:8]
            raise AssertionError(('state or call order mismatch', name, hex(precision), [x.hex() if isinstance(x, bytes) else hex(x) for x in argv], outputs[0][:5], outputs[1][:5], diff))
        counts[name] += 1
        return outputs[0]

    def quat(memory, address, q): struct.pack_into('<4f', memory, address - ARENA, *q)
    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        memory = bytearray(rng.randbytes(ARENA_SIZE))
        # Key lookups over an owned or borrowed (reference-chain) list.
        count = rng.randrange(0, 6); keys = [TRACK + 0x200 + i * 0x20 for i in range(count)]
        times = sorted(rng.randrange(-5, 3000) for _ in range(count))
        if rng.random() < 0.2 and count > 1: times[1] = times[0]
        for i, key in enumerate(keys):
            struct.pack_into('<IIi', memory, key + 4 - ARENA, keys[i + 1] if i + 1 < count else 0, keys[i - 1] if i else 0, times[i])
        head = keys[0] if keys else 0
        flag = rng.choice((0, 0, 1, 0x80))
        if flag:
            ref1, ref2 = TRACK + 0x100, TRACK + 0x140
            depth = rng.choice((1, 2))
            struct.pack_into('<BxxxI', memory, ref1 + 4 - ARENA, 1 if depth == 2 else 0, ref2 if depth == 2 else head)
            struct.pack_into('<BxxxI', memory, ref2 + 4 - ARENA, 0, head)
            pointer = ref1
        else: pointer = head
        memory[TRACK + 0x2c - ARENA] = flag; struct.pack_into('<I', memory, TRACK + 0x30 - ARENA, pointer)
        time = rng.choice((-2, -1, 0, rng.randrange(-10, 3100)) + tuple(times))
        for name in ('FindKey', 'FindKeyOrBase'): run(name, memory, [time], TRACK, precision)
        run('FirstKey', memory, [], TRACK, precision)
        # Integer and vector lerps.
        fraction = rng.choice((0.0, 1.0, 0.5, -0.25, 2.0, rng.random(), rng.uniform(-3, 3), 1e-9))
        a, b = (rng.choice((0, 1, -1, 255, -256, 0x7fffffff, -0x80000000, rng.getrandbits(32) - 2 ** 31, rng.randrange(-300, 300))) for _ in range(2))
        struct.pack_into('<i', memory, BASE - ARENA, rng.choice((0, 7, -9, rng.getrandbits(32) - 2 ** 31)))
        run('LerpInteger', memory, [a, b, f32(fraction), BASE, OUT], 0, precision)
        for address in (A, B, BASE): struct.pack_into('<4f', memory, address - ARENA, *(special(rng) if rng.random() < 0.2 else rng.uniform(-500, 500) for _ in range(4)))
        lerp_fraction = fraction
        if rng.random() < 0.25:  # cancellation exposes the per-component addition order
            big, small = rng.choice((2.0 ** 70, 2.0 ** 40, 1e30)), rng.choice((1.0, 3.0, 0.5))
            lerp_fraction = 1.0
            struct.pack_into('<4f', memory, A - ARENA, small, small, 0, 0)
            struct.pack_into('<4f', memory, BASE - ARENA, big, big, 0, 0)
            struct.pack_into('<4f', memory, B - ARENA, small - big, small - big, 0, 0)
        run('LerpVector2', memory, [A, B, f32(lerp_fraction), BASE, OUT], 0, precision)
        for address in (A, B, BASE): struct.pack_into('<4f', memory, address - ARENA, *(special(rng) if rng.random() < 0.2 else rng.uniform(-500, 500) for _ in range(4)))
        out = rng.choice((OUT, OUT, A, B))  # aliasing pins read-before-write
        run('Subtract', memory, [out, A, B], 0, precision); run('Add', memory, [out, A, B], 0, precision)
        run('Scale', memory, [rng.choice((OUT, A)), A, f32(special(rng) if rng.random() < 0.3 else fraction)], 0, precision)
        run('Assign', memory, [B], rng.choice((OUT, A)), precision)
        q = rng.choice((unit(rng), [0, 0, 0, 0], [1e-6, 0, 0, 0], [3, 4, 0, 0], [special(rng) for _ in range(4)]))
        quat(memory, A, q); run('Normalize', memory, [A], 0, precision)
        # Slerp: random, near-equal (lerp path), opposite-sign and non-unit inputs.
        qa = unit(rng); mode = rng.random()
        if trial % 8 == 0:
            pair = straddle(rng)
            if pair:
                quat(memory, A, pair[0]); quat(memory, B, pair[1]); quat(memory, BASE, unit(rng)); paths['straddle'] = paths.get('straddle', 0) + 1
                run('Slerp', memory, [A, B, f32(rng.random()), BASE, OUT], 0, precision)
        qb = (near(rng, qa, 0.01) if mode < 0.3 else [-x for x in near(rng, qa, rng.choice((0.001, 0.2)))] if mode < 0.45 else
              [x * rng.uniform(0.5, 2) for x in unit(rng)] if mode < 0.55 else unit(rng))
        quat(memory, A, qa); quat(memory, B, qb); quat(memory, BASE, unit(rng) if rng.random() < 0.8 else [0, 0, 0, 1])
        dot = sum(x * y for x, y in zip(qa, qb)); paths['lerp' if abs(dot) >= 0.999 else 'slerp'] += 1
        run('Slerp', memory, [A, B, f32(rng.choice((0.0, 1.0, 0.5, rng.random(), rng.uniform(-0.5, 1.5)))), BASE, OUT], 0, precision)
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), slerp_paths=paths,
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(SHARED),
                  limitations=['finite generated values; exhaustive float coverage not established',
                               'trig, multiply, resolve and CRT conversion bodies shared, verified elsewhere',
                               'MSVC 7.1 and byte identity pending'], fingerprints=manifest)
    if args.mutation_checks:
        mutations = {
            'findkey-time-le': ('    while (key->next4 && key->timeC < time) key = key->next4;\n    return key;\n}\n\n// Original 0x505260', '    while (key->next4 && key->timeC <= time) key = key->next4;\n    return key;\n}\n\n// Original 0x505260'),
            'base-threshold-zero': ('if (time <= -1) return', 'if (time < -1) return'),
            'firstkey-ignore-flag': ('// Original 0x4fe8d0.\nFEngScriptKey *FEScriptTrack::FirstKey() {\n    return (FEngScriptKey *)(value2C ?', '// Original 0x4fe8d0.\nFEngScriptKey *FEScriptTrack::FirstKey() {\n    return (FEngScriptKey *)(0 ?'),
            'lerp-int-32bit-convert': ('(unsigned int)(long long)((double)delta', '(unsigned int)(int)((double)delta'),
            'lerp-int-omit-half': ('(long long)((double)delta * fraction + fengHalf784260)', '(long long)((double)delta * fraction)'),
            'lerp2-order': ('    out[1] = (b[1] - a[1]) * fraction + base[1] + a[1];', '    out[1] = (b[1] - a[1]) * fraction + a[1] + base[1];'),
            'subtract-swap': ('float x = a->value0[0] - b->value0[0], y', 'float x = b->value0[0] - a->value0[0], y'),
            'normalize-rounded-length': ('double inverse = fengOne784250 / sqrt(x * x + y * y + z * z + w * w);', 'double inverse = fengOne784250 / (float)sqrt((float)(x * x + y * y) + (float)(z * z + w * w));'),
            'normalize-ge-epsilon': ('if (inverse > fengEpsilon7845A0)', 'if (inverse >= fengEpsilon7845A0 || inverse != inverse)'),
            'slerp-rounded-dot': ('    float cosine = (float)dot;\n    if (dot < fengZero7D7D3C) {', '    dot = (float)dot;\n    float cosine = (float)dot;\n    if (dot < fengZero7D7D3C) {'),
            'slerp-lerp-uses-flip': ('FEngQuaternionSubtract(&difference, b, a)', 'FEngQuaternionSubtract(&difference, &q, a)'),
            'slerp-rounded-u0': ('double u0 = (double)r0 + s->value0[0];', 'volatile float u0 = r0 + s->value0[0];'),
            'slerp-swap-sines': ('float sineU = EngineSine((fengOne784250 - fraction) * theta);', 'float sineU = EngineSine(theta * fraction); sineT = EngineSine((fengOne784250 - fraction) * theta);'),
            'slerp-multiply-order': ('*out = *result.Multiply(&product, base);', 'FEngQuaternion baseCopy = *base; *out = *baseCopy.Multiply(&product, &result);'),
            'slerp-no-normalize': ('        FEngQuaternionNormalize(&sum);\n', ''),
        }
        rejected = []
        for label, (before, after) in mutations.items():
            text = args.source.read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--source', str(mutant), '--cases', '2048'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            assert result.returncode and 'state or call order mismatch' in result.stdout, (label, result.stdout[-1500:])
            rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
