#!/usr/bin/env python3
"""Compare the drive camera's shake and the Camera finalisation under 53- and 64-bit x87 precision:
Camera::Finalise 0x447280, the noise apply 0x441340 with its helpers 0x441270, 0x4411b0 and
0x4412e0, the rotation 0x5bacf0 and the D3DX transpose wrapper 0x5baf80, the drive camera shake
0x452ed0 with the sample 0x446d00, the blend 0x5766e0, the scale-add 0x43d100 and the car query
0x5ecdb0, and the view shake 0x572a50 with 0x572860 and 0x5722a0.

Two rebuilt objects are linked: Camera.cpp and CameraShake.cpp. Original code runs for the callees
recovered elsewhere or not at all: identity 0x43c780, normalise 0x43cf10, matrix copy 0x4012b0, the
quantised sine and cosine 0x43c2b0/0x43c370, the arcsine 0x43c490, the Y and Z rotations
0x5bada0/0x5bae60, the nitrous stage 0x59fa30 (with 0x5925b0, 0x592500, 0x592480 and the replay
query 0x5f00f0) and _ftol 0x75bed4. Traced stubs (the same Python code in both runs, argument
contents logged): the statically linked D3DX multiply 0x63cedb and transpose 0x63d12c, which are
reached through run time function pointers the harness cannot load. The compared state is the whole
fixture arena, the camera related globals, the return value (ST0 for the float functions), ESP,
callee-saved registers, FPU control word and tag word, and the entry events (callee and arguments,
pointers by content) in call order."""

import argparse
import ctypes
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
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE
from verify_pc_camera_cockpit import SHARED as COCKPIT_SHARED

BUILD = ROOT / 'build/analysis/camera-shake'
ARENA, ARENA_SIZE, STACK, STOP = 0x2000000, 0x2000, 0x2010000, 0x2100000
STACK_RANGE = (STACK - 0x1000, STACK + 0x100)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
SOURCES = {'camera': 'src/Speed/Indep/Src/Camera/Camera.cpp', 'shake': 'src/Speed/Indep/Src/Camera/CameraShake.cpp'}
# name: (rebuilt symbol prefix, callee-popped bytes, return kind)
TARGETS = {'Finalise': ('?Finalise@Camera@@', 8, None), 'ApplyNoise': ('?ApplyNoise@Camera@@', 12, None),
           'NoiseValue': ('?CameraNoiseValue@@', 0, 'st0'), 'NoiseSum': ('?CameraNoiseSum@@', 0, 'st0'),
           'GetFieldAngle': ('?GetFieldAngle@Camera@@', 4, 'ax'), 'RotateX': ('?RotateMatrixX@@', 0, None),
           'Transpose': ('?CameraTranspose@@', 0, 'eax'), 'Shake': ('?Shake@DriveCameraMover@@', 16, None),
           'CurveSample': ('?Sample@CameraShakeCurve@@', 8, None), 'Blend4': ('?CameraBlend4@@', 16, None),
           'ScaleAdd4': ('?CameraScaleAdd4@@', 0, None), 'NitrousQuery': ('?GetCameraNitrousIndex@MoverModeOwner@@', 0, 'eax'),
           'ViewLookup': ('?Lookup@CameraViewShakeCurve@@', 4, 'st0'), 'ViewEvaluate': ('?Evaluate@CameraViewShakeState@@', 4, None),
           'ViewShake': ('?CameraViewShake@@', 0, None)}
# Entry events: name: (original address, rebuilt symbol prefix or None for shared original code, `this` kind, stack arguments).
# Argument kinds: 'w' raw word, 'h' low 16 bits (the angle units: every callee masks them, and 0x43c490 leaves the upper
# half of EAX as it found it), 'o' output pointer (arena address or `stack`), ('p', n) input pointer by content.
EVENTS = {
    'noise_sum': (0x441270, '?CameraNoiseSum@@', None, ['w']), 'noise_value': (0x4411b0, '?CameraNoiseValue@@', None, ['w']),
    'field_angle': (0x4412e0, '?GetFieldAngle@Camera@@', 'this', ['w']), 'rotx': (0x5bacf0, '?RotateMatrixX@@', None, ['o', ('p', 64), 'h']),
    'transpose': (0x5baf80, '?CameraTranspose@@', None, ['o', ('p', 64)]),
    'nitrous_q': (0x5ecdb0, '?GetCameraNitrousIndex@MoverModeOwner@@', 'this', []),
    'curve_sample': (0x446d00, '?Sample@CameraShakeCurve@@', ('p', 0x14), ['o', 'w']),
    'blend4': (0x5766e0, '?CameraBlend4@@', None, ['o', ('p', 16), ('p', 16), 'w']),
    'scaleadd4': (0x43d100, '?CameraScaleAdd4@@', None, ['o', ('p', 16), ('p', 16), 'w']),
    'apply_noise': (0x441340, '?ApplyNoise@Camera@@', 'this', [('p', 64), 'w', 'w']),
    'view_lookup': (0x5722a0, '?Lookup@CameraViewShakeCurve@@', ('p', 8), ['w']),
    'view_eval': (0x572860, '?Evaluate@CameraViewShakeState@@', ('p', 0x20), ['o']),
    'identity': (0x43c780, None, None, ['o']), 'normalize': (0x43cf10, None, None, ['o', ('p', 12)]),
    'copy': (0x4012b0, None, None, ['o', ('p', 64)]), 'sine': (0x43c2b0, None, None, ['h']), 'cosine': (0x43c370, None, None, ['h']),
    'arcsine': (0x43c490, None, None, ['w']), 'roty': (0x5bada0, None, None, ['o', ('p', 64), 'h']),
    'rotz': (0x5bae60, None, None, ['o', ('p', 64), 'h']), 'nitrous': (0x59fa30, None, 'this', [])}
D3DX_MUL, D3DX_T = 0x63cedb, 0x63d12c
# clang-cl turns the 0xd0-byte state copies into calls of memcpy; MSVC uses rep movsd. A cdecl rep movsb stub stands in for the CRT.
MEMCPY = 0x2300000
MEMCPY_CODE = bytes.fromhex('5756' '8b7c240c' '8b742410' '8b4c2414' '89f8' 'f3a4' '5e5fc3')
SHARED = dict(COCKPIT_SHARED)
SHARED.update({'?BodyMatrixCopy@': 0x4012b0, '?EngineCosineUnits@': 0x43c370, '?EngineSineUnits@': 0x43c2b0, '?HudArcSineUnits@': 0x43c490,
               '?RotateMatrixY@': 0x5bada0, '?RotateMatrixZ@': 0x5bae60, '_D3DXMatrixMultiply@12': D3DX_MUL, '_D3DXMatrixTranspose@8': D3DX_T,
               '__fltused': 0, '_memcpy': MEMCPY, '__ftol': 0x75bed4, '?GetNitrousIndex@DrivetrainAuxiliary@': 0x59fa30,
               '?roadSurfaces@': 0x800ee0, '?cameraFinaliseDisabled828640@': 0x828640, '?cameraDebugEnabled82A480@': 0x82a480,
               '?cameraDebugSuppressed865094@': 0x865094, '?cameraDebugActive82A4F8@': 0x82a4f8, '?cameraDebugMatrix82A490@': 0x82a490,
               '?cameraDebugScale78425C@': 0x78425c, '?cameraNoiseFalloff786F6C@': 0x786f6c, '?cameraNoiseScale787580@': 0x787580,
               '?cameraReferenceFov7EC1F0@': 0x7ec1f0, '?cameraTurnUnits784238@': 0x784238, '?cameraBumpGain7870BC@': 0x7870bc,
               '?cameraBumpQuarter784D28@': 0x784d28, '?cameraNitrousAdd7870B0@': 0x7870b0, '?cameraNitrousScale7870B4@': 0x7870b4,
               '?cameraShakeKeyCount7844F8@': 0x7844f8, '?cameraShakeKeys7EC430@': 0x7ec430, '?cameraShakeNoise7EC480@': 0x7ec480,
               '?cameraShakeNoise7EC490@': 0x7ec490, '?cameraShakeNoise7EC4A0@': 0x7ec4a0, '?cameraShakeNoise7EC4B0@': 0x7ec4b0,
               '?cameraShakeSpacing784258@': 0x784258, '?cameraSpeedLimit7A5660@': 0x7a5660, '?cameraSpeedScale78426C@': 0x78426c,
               '?cameraThrottleLimit78435C@': 0x78435c, '?cameraViewShake865360@': 0x865360, '?cameraViewShakeAngle7FB740@': 0x7fb740,
               '?cameraViewShakeAngle7FB744@': 0x7fb744, '?cameraViewShakeAngle7FB748@': 0x7fb748})
# Compared globals; each case writes the listed ones (the rest keep the image values).
CAMERA_G, DISABLED, DEBUG_ON, DEBUG_OFF, DEBUG_ACTIVE, DEBUG_MATRIX = 0, 0x828640, 0x82a480, 0x865094, 0x82a4f8, 0x82a490
GLOBALS = ((0x828640, 4), (0x82a480, 4), (0x865094, 4), (0x82a4f8, 4), (0x82a490, 64), (0x86518c, 4), (0x8651ac, 4), (0x890080, 4),
           (0x865360, 0x40), (0x89cf48, 0x20))
(CAMERA, MATRIX, OUT, CURVE, KEYS, POINTS, VEC_A, VEC_B, VEC_O, MOVER, CARLIST, PHYS, INPUT, CAR) = (ARENA + x for x in (
    0x100, 0x500, 0x540, 0x600, 0x740, 0x900, 0xa00, 0xa10, 0xa20, 0xb00, 0xc00, 0xe00, 0x1000, 0x1400))


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity
def s32(x): return (x + 0x80000000) % 0x100000000 - 0x80000000


IN_RANGE = False  # set by --in-range (default for clang-cl): keep float to int conversions inside the int range


def lim(rng, v):
    """MSVC converts through the CRT _ftol (low 32 bits of the 64-bit result); clang-cl saturates to 0x80000000, so the out of range
    behaviour differs between the compilers and is only generated for MSVC."""
    if not IN_RANGE or v == 0 or (v == v and 1e-3 <= abs(v) <= 1e4): return v
    return rng.uniform(0.01, 3.0)


def special(rng):
    return lim(rng, special_values(rng))


def special_values(rng):
    return rng.choice((0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e-20, -1e-20, 1e-45, 1e30, 3e38, float('inf'), -float('inf'), float('nan'),
                       rng.uniform(-2, 2), rng.uniform(-500, 500)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: camera, shake)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--in-range', action='store_true', help='keep float to int conversions inside the int range (default with clang-cl)')
    parser.add_argument('--targets', default='', help='comma separated subset of targets')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    global IN_RANGE
    IN_RANGE = args.in_range or args.compiler == 'clang-cl'
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/camera-shake.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[a:a + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-fno-inline'] + ['/clang:' + f for f in args.clang_flag])
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)), '/Isrc/Speed/Indep/Src/Camera',
                                   '/Isrc/Speed/Indep/Src/Physics', '/Isrc/Speed/Indep/Src/World', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    parsed = [read_object(o) for o in objects]
    defined = {s['name'] for p in parsed for s in p['symbols'].values() if s['section'] > 0}
    externals = {}
    for p in parsed:
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for pre, a in SHARED.items() if s['name'].startswith(pre)]
            assert len(match) == 1, ('unexpected external', s['name']); externals[s['name']] = match[0]
    linked = link_objects(objects, externals)
    entries = {n: next(a for s, a in linked['exports'].items() if s.startswith(p)) for n, (p, _, _) in TARGETS.items()}
    original_ranges = [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
    defaults = {a: bytes(image[a - 0x400000:a - 0x400000 + n]) for a, n in GLOBALS}
    selected = [t for t in args.targets.split(',') if t] or list(TARGETS)
    machines, events = [], [[], []]

    def view(uc, w, size):
        if ARENA <= w < ARENA + ARENA_SIZE: return ('arena', w, bytes(uc.mem_read(w, size)) if size else None)
        if STACK_RANGE[0] <= w < STACK_RANGE[1]: return ('stack', bytes(uc.mem_read(w, size)) if size else None)
        return w
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x20000); uc.mem_map(STOP, 0x1000); uc.mem_map(0, 0x1000)
        uc.mem_map(linked['address'], linked['allocation']); uc.mem_map(MEMCPY, 0x1000); uc.mem_write(MEMCPY, MEMCPY_CODE)
        for a, b in linked['blocks']: uc.mem_write(a, b)
        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt: assert not any(a <= address < b for a, b in original_ranges), ('state or call order mismatch', 'original target reached', hex(address))
            else: assert not any(a <= address < b for a, b in linked['ranges']), ('unexpected execution', hex(address))
        def stub(uc, address, size, user, log=events[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
            if address == D3DX_MUL:
                out, left, right = struct.unpack('<3I', uc.mem_read(esp + 4, 12)); pop = 12
                l = struct.unpack('<16f', uc.mem_read(left, 64)); r = struct.unpack('<16f', uc.mem_read(right, 64))
                log.append(('d3dx_mul', bytes(uc.mem_read(left, 64)), bytes(uc.mem_read(right, 64))))
                uc.mem_write(out, struct.pack('<16f', *[f32(sum(l[i * 4 + k] * r[k * 4 + j] for k in range(4))) for i in range(4) for j in range(4)]))
            else:
                out, source = struct.unpack('<2I', uc.mem_read(esp + 4, 8)); pop = 8
                data = bytes(uc.mem_read(source, 64)); log.append(('d3dx_t', data))
                uc.mem_write(out, b''.join(data[(j * 4 + i) * 4:(j * 4 + i) * 4 + 4] for i in range(4) for j in range(4)))
            uc.reg_write(xc.UC_X86_REG_EAX, out); uc.reg_write(xc.UC_X86_REG_ESP, esp + 4 + pop); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        def entry(uc, address, size, user, log=events[rebuilt]):
            esp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            _, _, this_kind, spec = EVENTS[user]
            words = struct.unpack('<%dI' % len(spec), uc.mem_read(esp + 4, 4 * len(spec))); seen = []
            for kind, w in zip(spec, words):
                if kind == 'w': seen.append(w)
                elif kind == 'h': seen.append(w & 0xffff)
                elif kind == 'o': seen.append(('arena', w) if ARENA <= w < ARENA + ARENA_SIZE else 'stack' if STACK_RANGE[0] <= w < STACK_RANGE[1] else w)
                else: seen.append(view(uc, w, kind[1]))
            if this_kind is None: head = None
            elif this_kind == 'this': head = view(uc, this, 0)
            else: head = view(uc, this, this_kind[1])
            log.append((user, head, tuple(seen)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        uc.hook_add(UC_HOOK_CODE, stub, begin=D3DX_MUL, end=D3DX_MUL); uc.hook_add(UC_HOOK_CODE, stub, begin=D3DX_T, end=D3DX_T)
        for name, (address, prefix, _, _) in EVENTS.items():
            if prefix is not None and rebuilt: address = next(a for s, a in linked['exports'].items() if s.startswith(prefix))
            uc.hook_add(UC_HOOK_CODE, entry, user_data=name, begin=address, end=address)
        machines.append(uc)
    rng = random.Random(0x447280); counts = dict.fromkeys(TARGETS, 0); paths = {}

    def get_floats(memory, address, count): return list(struct.unpack_from('<%df' % count, memory, address - ARENA))

    def run(name, memory, statics, argv, precision, this):
        _, cleanup, returns = TARGETS[name]
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            events[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            for address, data in defaults.items(): uc.mem_write(address, statics.get(address, data))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<f', f32(x)) if isinstance(x, float) else struct.pack('<I', x & 0xffffffff) for x in argv))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this); uc.reg_write(xc.UC_X86_REG_EDX, 0x5a5a5a5a)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0); uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for i, reg in enumerate(PRESERVED): uc.reg_write(reg, 0x12345000 + i)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try: uc.emu_start(address, STOP, count=4000000)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not finish')
            value = None
            if returns == 'st0': value = uc.reg_read(xc.UC_X86_REG_FP0 + ((uc.reg_read(xc.UC_X86_REG_FPSW) >> 11) & 7))  # ST(0) is physical register TOP
            elif returns == 'ax': value = uc.reg_read(xc.UC_X86_REG_EAX) & 0xffff
            elif returns == 'eax': value = uc.reg_read(xc.UC_X86_REG_EAX)
            contract = (value, uc.reg_read(xc.UC_X86_REG_ESP), tuple(uc.reg_read(r) for r in PRESERVED), uc.reg_read(xc.UC_X86_REG_FPCW))
            outputs.append((contract, uc.reg_read(xc.UC_X86_REG_FPTAG), list(events[rebuilt]),
                            tuple(bytes(uc.mem_read(a, n)) for a, n in GLOBALS), bytes(uc.mem_read(ARENA, ARENA_SIZE))))
        assert outputs[0][0][1] == STACK + 4 + cleanup and outputs[0][0][3] == precision, ('original contract', name)
        if outputs[0] != outputs[1]:
            a, b = outputs
            diff = [(hex(ARENA + i), a[4][i:i + 4].hex(), b[4][i:i + 4].hex()) for i in range(0, ARENA_SIZE, 4) if a[4][i:i + 4] != b[4][i:i + 4]][:8]
            if os.environ.get('UG2_VERIFY_DEBUG'):
                print('matrix', get_floats(memory, MATRIX, 16)); print('statics', {hex(k): struct.unpack('<%df' % (len(v) // 4), v) for k, v in statics.items() if len(v) <= 0x40})
                k = next((i for i, (x, y) in enumerate(zip(a[2], b[2])) if x != y), 0)
                for i in range(max(0, k - 12), k + 3): print(i, a[2][i][:1] + ((a[2][i][2],) if a[2][i][0] in ('sine', 'arcsine', 'field_angle', 'cosine') else ()), '|', b[2][i][:1] + ((b[2][i][2],) if b[2][i][0] in ('sine', 'arcsine', 'field_angle', 'cosine') else ()))
            raise AssertionError(('state or call order mismatch', name, hex(precision), argv, [struct.unpack_from('<4f', memory, CAMERA + 0x70 + 0x10 * i - ARENA) for i in range(4)], a[0], b[0], a[1], b[1], [e[0] for e in a[2]], [e[0] for e in b[2]],
                                  next(((i, x, y) for i, (x, y) in enumerate(zip(a[2], b[2])) if x != y), None), [(hex(GLOBALS[i][0]), x.hex(), y.hex()) for i, (x, y) in enumerate(zip(a[3], b[3])) if x != y], diff))
        if os.environ.get('UG2_VERIFY_DEBUG') and TARGETS[name][2] == 'st0': print(name, argv, outputs[0][0][0], outputs[1][0][0], hex(precision))
        counts[name] += 1
        return outputs[0]

    def put(memory, address, values): struct.pack_into('<%df' % len(values), memory, address - ARENA, *(f32(v) for v in values))
    def get(memory, address, count): return list(struct.unpack_from('<%df' % count, memory, address - ARENA))
    def note(path): paths[path] = paths.get(path, 0) + 1
    def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)
    def gen(): return rng.choice((rng.gauss(0, 1), rng.uniform(-30, 30), rng.gauss(0, 1) * 1e-3)) if rng.random() < 0.9 else special(rng)
    def small(): return rng.uniform(0, 12) if rng.random() < 0.9 else special(rng)

    def unit():
        v = [rng.gauss(0, 1) for _ in range(3)]; n = math.sqrt(sum(x * x for x in v)) or 1.0
        return [x / n for x in v]

    def matrix():
        kind = rng.choice(('rigid', 'rigid', 'random', 'special'))
        if kind == 'random': return [gen() for _ in range(16)]
        if kind == 'special': return [special(rng) for _ in range(16)]
        f = unit(); t = [rng.gauss(0, 1) for _ in range(3)]
        r = [t[1] * f[2] - t[2] * f[1], t[2] * f[0] - t[0] * f[2], t[0] * f[1] - t[1] * f[0]]; n = math.sqrt(sum(x * x for x in r)) or 1.0; r = [x / n for x in r]
        d = [f[1] * r[2] - f[2] * r[1], f[2] * r[0] - f[0] * r[2], f[0] * r[1] - f[1] * r[0]]
        return [r[0], d[0], f[0], 0.0, r[1], d[1], f[1], 0.0, r[2], d[2], f[2], 0.0] + [rng.uniform(-3000, 3000) for _ in range(3)] + [1.0]

    def camera_state(memory, base, fields):
        put(memory, base, matrix()); put(memory, base + 0x40, [gen() for _ in range(3)]); put(memory, base + 0x50, [gen() for _ in range(3)])
        put(memory, base + 0x60, [gen() for _ in range(3)]); put(memory, base + 0x70, [fields() for _ in range(16)])
        put(memory, base + 0xb0, [gen() for _ in range(5)]); pk(memory, base + 0xc4, '<H', rng.choice((0, 1, 10922, 13018, 0x8000, 0xffff, rng.randrange(65536))))

    def rand_camera(memory, fields=gen):
        camera_state(memory, CAMERA, fields); camera_state(memory, CAMERA + 0xd0, fields)
        put(memory, CAMERA + 0x1e0, [gen() for _ in range(0x44)])
        pk(memory, CAMERA + 0x270, '<B', rng.choice((0, 0, 0, 1, 0x80)))
        pk(memory, CAMERA + 0x274, '<f', f32(gen()))

    def fixture_car(memory, with_input=True):
        pk(memory, CAR + 0x10, '<I', rng.choice((0, PHYS, PHYS)))
        pk(memory, CAR + 0x30, '<I', INPUT)
        put(memory, PHYS + 0x20, [rng.choice((100.0, 25.0, -5.0, 3.0, rng.uniform(1, 200)) if IN_RANGE else (100.0, 25.0, 0.0, -0.0, -5.0, 1e-30, 3.0, float('nan'), gen(), rng.uniform(1, 200)))])
        put(memory, INPUT + 0x20c, [rng.choice((0.0, -0.0, 0.0005, -0.0005, 0.001, 0.0010000000474974513, 0.5, -0.5, 1.0, float('nan'), float('inf'), gen()))])
        put(memory, CAR + 0x430, [rng.choice((0.0, 1.0, 5.0, 2.2, 2.23, rng.uniform(0, 30), rng.uniform(-10, 100), special(rng)))])
        for i in range(4): pk(memory, CAR + 0x394 + 4 * i, '<i', rng.choice((0, 1, 2, 5, 13, rng.randrange(0, 30), rng.randrange(0, 4)) if IN_RANGE else (0, 1, 2, 5, 13, rng.randrange(-2, 60), rng.randrange(0, 4))))  # the table 0x800ee0 holds about 30 real surfaces
        for off in (0x454, 0x45c, 0x460, 0x458):
            pk(memory, CAR + off, '<i', rng.choice((0, 0x7fffffff, rng.randrange(1, 20000), rng.randrange(-5000, 5000), 4000, 40000)))

    def clocks():
        return {0x86518c: struct.pack('<i', rng.choice((0, 5, 4000, 123456, 0x7fffffff, -0x80000000, rng.randrange(-0x80000000, 0x80000000), rng.randrange(0, 200000)))),
                0x8651ac: struct.pack('<i', rng.choice((0, 5, 4000, 123456, rng.randrange(0, 200000))))}

    for trial in range(args.cases):
        precision = rng.choice((0x027f, 0x037f))
        for name in selected:
            memory = bytearray(rng.randbytes(ARENA_SIZE)); statics = {}; argv = []; this = 0
            if name == 'Finalise':
                rand_camera(memory); put(memory, MATRIX, matrix()); this = CAMERA
                dt = rng.choice((1 / 60, 0.001, 0.05, 1.0, 0.0, -0.0, -1.0, lim(rng, 1e-30), lim(rng, 3e38), lim(rng, float('nan')), lim(rng, float('inf')), rng.uniform(1e-3, 0.1)))
                argv = [MATRIX, dt]
                if rng.random() < 0.15: put(memory, CAMERA + 0x70, [special(rng) for _ in range(16)])
                disabled = rng.choice((0, 0, 0, 0, 0, 0, 0, 1, 0x80000000, -1)); on, off, active = rng.choice((0, 1, 1)), rng.choice((0, 0, 1)), rng.choice((0, 1, 7))
                statics = {DISABLED: struct.pack('<I', disabled & 0xffffffff), DEBUG_ON: struct.pack('<i', on), DEBUG_OFF: struct.pack('<i', off), DEBUG_ACTIVE: struct.pack('<i', active),
                           DEBUG_MATRIX: struct.pack('<16f', *[f32(x) for x in matrix()])}
                result = run(name, memory, statics, argv, precision, this)
                note('finalise-disabled' if disabled else 'finalise-debug' if on and not off else 'finalise-matrix')
                note('finalise-reset' if memory[CAMERA + 0x270 - ARENA] else 'finalise-no-reset')
                note('finalise-step-positive' if dt == dt and dt > 0 else 'finalise-step-nonpositive')
                continue
            if name == 'ApplyNoise':
                rand_camera(memory, fields=small); put(memory, MATRIX, matrix()); this = CAMERA
                time = rng.choice((0.0, 1.0, 12.5, 100.0, 0.0002, rng.uniform(0, 2000), lim(rng, rng.uniform(0, 1e6)), special(rng))); scale = rng.choice((1.0, 1.0, 0.5, rng.uniform(0, 2) if IN_RANGE else gen()))
                argv = [MATRIX, time, scale]
                if rng.random() < 0.2: put(memory, CAMERA + 0x80, [rng.choice((0.0, 1.0, 5.0, 90.0, 360.0, special(rng))) for _ in range(12)])
                run(name, memory, statics, argv, precision, this); note('noise-apply')
                continue
            if name in ('NoiseValue', 'NoiseSum'):
                x = rng.choice((rng.uniform(-100, 100), rng.uniform(-3, 3), lim(rng, rng.uniform(-1e6, 1e6)), float(rng.randrange(-50, 50)), rng.uniform(0, 12), lim(rng, 2.0 ** 31), lim(rng, -2.0 ** 31), lim(rng, 3e9), lim(rng, -3e9), special(rng)))
                run(name, memory, statics, [x], precision, 0); note('noise-' + name[5:].lower())
                continue
            if name == 'GetFieldAngle':
                this = CAMERA; pk(memory, CAMERA + 0xc4, '<H', rng.choice((0, 1, 10922, 13018, 0x8000, 0xffff, rng.randrange(65536))))
                angle = rng.choice((0, 1, -1, 0x4000, 0x8000, 0x10000, 100, -100, 30, rng.randrange(-70000, 70000), rng.randrange(-0x80000000, 0x80000000)))
                run(name, memory, statics, [angle], precision, this)
                continue
            if name == 'RotateX':
                put(memory, MATRIX, matrix()); angle = rng.choice((0, 1, 0x4000, 0x8000, 0xffff, rng.randrange(0x10000), rng.randrange(0x100000000)))
                run(name, memory, statics, [rng.choice((OUT, MATRIX)), MATRIX, angle], precision, 0)
                continue
            if name == 'Transpose':
                put(memory, MATRIX, matrix()); run(name, memory, statics, [rng.choice((OUT, MATRIX)), MATRIX], precision, 0)
                continue
            if name in ('Blend4', 'ScaleAdd4'):
                put(memory, VEC_A, [gen() for _ in range(4)]); put(memory, VEC_B, [gen() for _ in range(4)]); put(memory, VEC_O, [gen() for _ in range(4)])
                t = rng.choice((0.0, 1.0, 0.5, gen(), rng.uniform(0, 1)))
                run(name, memory, statics, [VEC_O, VEC_A, VEC_B, t] if name == 'Blend4' else [rng.choice((VEC_O, VEC_A)), VEC_A, VEC_B, t], precision, 0)
                continue
            if name == 'CurveSample':
                count = rng.choice((0, 1, 2, 3, 5, 5, 5, 6, 8)); put(memory, KEYS, [gen() for _ in range(32)]); pk(memory, CURVE, '<i', count)
                put(memory, CURVE + 4, [rng.choice((0.0, 0.0, 1.0, -2.0, gen())), rng.choice((100.0, 4.0, gen())), rng.choice((0.04, 0.16, 1.0, -1.0, 0.0, gen()))])
                pk(memory, CURVE + 0x10, '<I', KEYS)
                x = rng.choice((rng.uniform(-10, 120), rng.uniform(0, 100), rng.uniform(0, 30), 0.0, 100.0, 25.0, special(rng)))
                run(name, memory, statics, [VEC_O, x], precision, CURVE); note('curve-sample')
                continue
            if name == 'NitrousQuery':
                fixture_car(memory); statics = clocks()
                result = run(name, memory, statics, [], precision, CAR); note('nitrous-query-%d' % result[0][0]); continue
            if name == 'Shake':
                rand_camera(memory); fixture_car(memory); statics = clocks(); put(memory, MATRIX, matrix())
                pk(memory, MOVER + 0x1c, '<I', CAMERA)
                run(name, memory, statics, [MATRIX, CAR, rng.choice((1.0, 0.5, 0.0, rng.uniform(0, 2) if IN_RANGE else gen())), rng.choice((1.0, 0.8, rng.uniform(0, 2) if IN_RANGE else gen()))], precision, MOVER); note('shake')
                continue
            if name in ('ViewLookup', 'ViewEvaluate', 'ViewShake'):
                count = rng.choice((1, 2, 3, 4, 5, 8) if IN_RANGE else (0, 1, 2, 3, 4, 5, 8))  # count 0 reads the arena bytes before the points
                if IN_RANGE:  # well spaced points: a near zero span would make huge values and out of range float to int conversions
                    put(memory, POINTS, [x for i in range(count) for x in (-3.0 + 1.1 * i + rng.uniform(0, 0.4), rng.uniform(-3, 3))])
                else:
                    put(memory, POINTS, [x for p in sorted(rng.uniform(-3, 6) for _ in range(count)) for x in (p, gen())] if rng.random() < 0.9 else [gen() for _ in range(2 * count)])
                    if rng.random() < 0.1: put(memory, POINTS, [rng.choice((0.0, 1.0, 2.0)) for _ in range(2 * count)])
                pk(memory, CURVE, '<II', POINTS, count)
                state = struct.pack('<3f4xI', *[f32(rng.uniform(-3, 3) if IN_RANGE else gen()) for _ in range(3)], rng.choice((0, CURVE, CURVE))) + struct.pack('<f4xf', f32(rng.choice((0.0, 1.0, 2.0, 3.0, 0.5, 1.5, rng.uniform(-3, 6), special(rng)))), f32(rng.choice((0.0, 1.0, 2.0, 3.0, 0.5, 0.25, rng.uniform(0, 6)))))
                if name == 'ViewLookup':
                    x = rng.choice((rng.uniform(-4, 7), 0.0, 1.0, 2.0, special(rng)))
                    if count and rng.random() < 0.3: x = get(memory, POINTS, 2 * count)[rng.randrange(2 * count) & ~1]
                    run(name, memory, statics, [x], precision, CURVE); note('view-lookup-%d' % min(count, 3))
                elif name == 'ViewEvaluate':
                    memory[OUT - ARENA:OUT - ARENA + 0x20] = state; run(name, memory, statics, [VEC_O], precision, OUT); note('view-evaluate')
                else:
                    carlist = rng.choice((0.0, 0.5, 1.0, 1.0, 1.5, 2.0, 100.0, -1.0, float('nan')))
                    put(memory, CARLIST + 0x108, [carlist]); put(memory, MATRIX, matrix())
                    statics = {0x890080: struct.pack('<I', CARLIST), 0x865360: state + struct.pack('<3f4xI', *[f32(rng.uniform(-3, 3) if IN_RANGE else gen()) for _ in range(3)], rng.choice((0, CURVE))) + struct.pack('<f4xf', f32(rng.choice((0.0, 1.0, 2.0, rng.uniform(0, 6)))), f32(rng.choice((0.0, 1.0, 2.0, 0.5))))}
                    view_id = rng.choice((0, 1, 1, 2, 2, 3, 3, 4, -1, 5)); run(name, memory, statics, [view_id, MATRIX], precision, 0)
                    note('viewshake-gate-closed' if carlist < 1.0 else 'viewshake-view-%d' % view_id)
                continue
    report = dict(compiler=args.compiler, comparisons=counts, total=sum(counts.values()), paths=dict(sorted(paths.items())),
                  x87_precision=['53-bit', '64-bit'], shared_original_callees=sorted(n for n, e in EVENTS.items() if e[1] is None) + ['_ftol 0x75bed4'],
                  cases=args.cases, traced_stubs=['D3DX multiply 0x63cedb (row-vector product, binary32 rounding)', 'D3DX transpose 0x63d12c'],
                  fixtures=['car and camera fields are written with finite and special binary32 values; the camera/car/mover/curve/state objects live in a random-filled arena that is compared whole',
                            'the road surface table 0x800ee0 and the nitrous thresholds are the image values; the replay recorder 0x89cf48 keeps its image state'],
                  limitations=['finite and special generated values; exhaustive float coverage not established',
                               'signalling NaNs and NaN payloads are not generated (canonical quiet NaNs only)',
                               'the D3DX routines run as Python stubs: the real implementation selected at run time is not executed, and the transpose is inferred from the use',
                               'the replay recorder state read by the nitrous timer 0x5f00f0 is not varied',
                               'byte identity not established (compiler output differs from the original)'], fingerprints=manifest)
    if args.mutation_checks:
        C, S = 'camera', 'shake'
        mutations = {
            'noise-value-hash-constant': (C, '0x3d73u + 0xc0ae5u) * first', '0x3d73u + 0xc0ae6u) * first'),
            'noise-value-second-hash': (C, '(second * second) * 0x3d73u', '(second * first) * 0x3d73u'),
            'noise-value-fraction': (C, '* (1.0f - fraction)\n         +', '* fraction\n         +'),
            'noise-value-floor': (C, 'lower = lower - 1.0f;', 'lower = lower;'),
            'noise-sum-octaves': (C, 'int octave = 6;', 'int octave = 5;'),
            'noise-sum-frequency': (C, 'frequency = frequency + frequency;', 'frequency = frequency + 1.0f;'),
            'noise-sum-falloff': (C, 'amplitude = cameraNoiseFalloff786F6C * amplitude;', 'amplitude = amplitude;'),
            'field-angle-reference': (C, 'cameraReferenceFov7EC1F0 >> 1', 'cameraReferenceFov7EC1F0'),
            'field-angle-fov': (C, 'EngineSineUnits(current0.fovC4 >> 1)', 'EngineSineUnits(current0.fovC4)'),
            'rotx-sine-sign': (C, 'm[9] = -sine;', 'm[9] = sine;'),
            'rotx-cosine': (C, 'm[10] = cosine;', 'm[10] = sine;'),
            'apply-w0-order': (C, 'float w0 = (m0 * amplitudeA0[0] + v0) * scale;', 'float w0 = (m0 * amplitudeA0[0] + v0) + scale;'),
            'apply-time-scale': (C, 'float q2 = scale90[2] * time;', 'float q2 = scale90[2];'),
            'apply-translation': (C, 'rotation.elements[13] = w1;', 'rotation.elements[13] = w0;'),
            'apply-angle-divisor': (C, '(int)(w2 * cameraTurnUnits784238) / 360', '(int)(w2 * cameraTurnUnits784238) / 361'),
            'apply-rotation-order': (C, 'RotateMatrixX(&rotation, &rotation, GetFieldAngle(angleX));\n    int angleY', 'int angleY'),
            'apply-product-order': (C, 'o[5] = r[1] * i[4] + r[13] * i[7] + r[5] * i[5] + r[9] * i[6];', 'o[5] = r[1] * i[4] + r[13] * i[7] + r[5] * i[5] + r[9] * i[5];'),
            'finalise-disabled-ignored': (C, 'if (cameraFinaliseDisabled828640) {\n        return;\n    }', ''),
            'finalise-debug-scale': (C, 'debug.elements[14] = debug.elements[14] * cameraDebugScale78425C;', 'debug.elements[14] = debug.elements[14];'),
            'finalise-debug-gate': (C, 'cameraDebugEnabled82A480 && !cameraDebugSuppressed865094', 'cameraDebugEnabled82A480 || !cameraDebugSuppressed865094'),
            'finalise-debug-flag': (C, '            cameraDebugActive82A4F8 = 0;', '            cameraDebugActive82A4F8 = 1;'),
            'finalise-step-store': (C, '    step274 = step;\n', ''),
            'finalise-position-sign': (C, 'current0.position40.y = -y;', 'current0.position40.y = y;'),
            'finalise-position-order': (C, 'float z = t[2] * c[12] + t[6] * c[13] + t[10] * c[14] + t[14];', 'float z = t[2] * c[12] + t[6] * c[13] + t[10] * c[14];'),
            'finalise-target-rounding': (C, 'volatile float targetX = ', 'float targetX = '),
            'finalise-reset-step': (C, 'step274 = 1.0f;', 'step274 = step;'),
            'finalise-reset-clear': (C, '        reset270 = 0;\n', ''),
            'finalise-step-gate': (C, 'if (!(step274 > fengZero7D7D3C)) {', 'if (!(step274 >= fengZero7D7D3C)) {'),
            'finalise-velocity-order': (C, 'v[0] = current0.position40.x - previousD0.position40.x;', 'v[0] = previousD0.position40.x - current0.position40.x;'),
            'finalise-velocity-scale': (C, '    v[2] = inverse * v[2];\n    v = &velocity200.x;', '    v = &velocity200.x;'),
            'finalise-noise-group': (C, 'w = noiseVelocity240;\n    w[0] = current0.noiseA0[0] - previousD0.noiseA0[0];', 'w = noiseVelocity240;\n    w[0] = current0.noise90[0] - previousD0.noiseA0[0];'),
            'finalise-zero-store': (C, '    zero20C = 0.0f;\n', ''),
            'curve-index-clamp': (S, 'if (count - 1 <= index) {', 'if (count <= index) {'),
            'curve-negative': (S, 'if (index < 0) {', 'if (index < -1) {'),
            'curve-fraction': (S, 'position - floorValue', 'position'),
            'curve-start': (S, '(x - start4) * inverseSpacingC', 'x * inverseSpacingC'),
            'blend-weights': (S, 'CameraScaleAdd4(output, output, second, 1.0f - t);', 'CameraScaleAdd4(output, output, second, t);'),
            'blend-w': (S, 'output[3] = a3 * t;', 'output[3] = a2 * t;'),
            'scaleadd-w': (S, 'float w = scale * direction[3] + base[3];', 'float w = scale * direction[3] + base[2];'),
            'nitrous-query-throttle': (S, 'magnitude <= cameraThrottleLimit78435C', 'magnitude < cameraThrottleLimit78435C'),
            'nitrous-query-speed': (S, 'scaledSpeed < cameraSpeedLimit7A5660', 'speed430 < cameraSpeedLimit7A5660'),
            'nitrous-query-abs': (S, 'float magnitude = throttle < 0.0f ? -throttle : throttle;', 'float magnitude = throttle;'),
            'shake-spacing-default': (S, 'float spacing = cameraShakeSpacing784258;', 'float spacing = 50.0f;'),
            'shake-spacing-info': (S, 'spacing = car->physicsInfo10->parameters10.value10;', 'spacing = car->physicsInfo10->parameters10.valueC;'),
            'shake-surface-order': (S, '(surface0->bumpFrequency18 + surface1->bumpFrequency18) + surface2->bumpFrequency18', '(surface0->bumpFrequency18 + surface1->bumpFrequency18) + surface1->bumpFrequency18'),
            'shake-amplitude-gain': (S, 'cameraBumpGain7870BC *', 'cameraBumpQuarter784D28 *'),
            'shake-amplitude-rounding': (S, 'volatile float amplitude01', 'float amplitude01'),
            'shake-nitrous-gate': (S, '!car->GetCameraNitrousIndex()', 'car->GetCameraNitrousIndex()'),
            'shake-nitrous-add': (S, 'speedShake = cameraNitrousAdd7870B0 + speedShake;', 'speedShake = cameraNitrousAdd7870B0;'),
            'shake-nitrous-scale': (S, 'lifted = cameraNitrousScale7870B4 * lifted;', 'lifted = cameraNitrousScale7870B4 + lifted;'),
            'shake-held-order': (S, 'float held = sample[1] * amplitude * secondScale;', 'float held = sample[1] * amplitude;'),
            'shake-table-80': (S, 'b[0] = cameraShakeNoise7EC490[0] * speedShake;', 'b[0] = cameraShakeNoise7EC490[0] * lifted;'),
            'shake-table-a0': (S, 'd[3] = cameraShakeNoise7EC4B0[3] * held;', 'd[3] = cameraShakeNoise7EC4B0[3] * frequencyShake;'),
            'shake-time': (S, '(float)cameraClock86518C * cameraClockScale784268, 1.0f', '(float)cameraClock86518C * cameraClockScale784268, 2.0f'),
            'shake-sample-speed': (S, 'curve.Sample(sample, car->speed430);', 'curve.Sample(sample, 0.0f);'),
            'view-lookup-fallback': (S, 'return (input - p[i * 2]) * fengZero7D7D3C + p[i * 2 + 1];', 'return p[i * 2 + 1];'),
            'view-lookup-low': (S, 'if (input < p[0]) {', 'if (input <= p[0]) {'),
            'view-lookup-high': (S, 'if (p[count * 2 - 2] <= input) {', 'if (p[count * 2 - 2] < input) {'),
            'view-lookup-slope': (S, '(p[i * 2 + 3] - p[i * 2 + 1]) / (p[i * 2 + 2] - p[i * 2])', '(p[i * 2 + 3] - p[i * 2 + 1]) / (p[i * 2 + 3] - p[i * 2])'),
            'view-evaluate-parity': (S, '!((int)(value1C * time14) & 1)', '((int)(value1C * time14) & 1)'),
            'view-evaluate-curve': (S, '&& curve10) {', '|| curve10) {'),
            'view-evaluate-y': (S, 'output[1] = vector0[1] * value;', 'output[1] = vector0[0] * value;'),
            'view-shake-gate-value': (S, '< fengOne784250) {\n        return;', '<= fengOne784250) {\n        return;'),
            'view-shake-slot': (S, '} else if (view == 2) {\n        slot = 1;', '} else if (view == 2) {\n        slot = 0;'),
            'view-shake-view3': (S, 'if (view == 1 || view == 3) {', 'if (view == 1) {'),
            'view-shake-x-order': (S, 'float x = v[2] * m[8] +', 'float x = v[2] * m[9] +'),
            'view-shake-zero-product': (S, 'm[14] * fengZero7D7D3C;', '0.0f;'),
            'view-shake-angle-x': (S, '(t[13] + t[14]) * cameraViewShakeAngle7FB740', '(t[13] - t[14]) * cameraViewShakeAngle7FB740'),
            'view-shake-angle-z': (S, '(t[12] + t[13]) * cameraViewShakeAngle7FB748', '(t[12] + t[13]) * cameraViewShakeAngle7FB744'),
            'view-shake-product': (S, 'o[9] = r[9] * m[10] + r[13] * m[11] + r[1] * m[8] + r[5] * m[9];', 'o[9] = r[9] * m[10] + r[13] * m[11] + r[1] * m[8] + r[5] * m[10];'),
        }
        rejected = []
        for label, (key, before, after) in mutations.items():
            text = (ROOT / SOURCES[key]).read_text(); assert before in text, label; text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--replace', key + '=' + str(mutant), '--cases', '384'], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            if not (result.returncode and 'state or call order mismatch' in result.stdout): report.setdefault('survived_mutations', []).append(label)
            else: rejected.append(label)
        report['rejected_mutations'] = rejected
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))


if __name__ == '__main__': main()
