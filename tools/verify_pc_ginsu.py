#!/usr/bin/env python3
"""Behavioural verifier for the recovered Ginsu granular synthesizer of EA SPEED (UG2 PC v1.2).

The recovered sources src/Speed/Indep/Src/EAXSound/Ginsu/GinsuData.cpp and GinsuSynth.cpp are compiled with
MSVC 7.1 (wibo) and linked as analysis objects. Each routine of config/SPEED2_EXE_1_2/ginsu.json runs on two
Unicorn machines with the same fixture: the retail image at the manifest address, and the linked objects at
the exported symbol. The snd library is a seam (SND below) with two kinds of entry:

- stateful entry points (critical section, SNDPKTPLAY_overhead/create/start/submit/stop/destroy) are events on
  both sides. The Unicorn copy of the retail image has their call targets patched to RET, and the linked objects
  call one-byte RET stubs. Each call is logged in call order with its arguments (structs and the packet record by
  content). The return value (EAX) comes from a per-trial script, so the success and error paths are both run.
- pure leaves (SND_attrsetdef, SNDplaysetdef) keep no state and only fill their argument: both sides run the retail
  bytes and log the call. The native replacements are checked separately (see docs/pc-ginsu.md).

Compared per case: the whole fixture arena (object, sample buffers and gin tables), the return value (AL or
EAX, or the x87 ST0 for float routines), ESP, EBX/ESI/EDI/EBP, the FXSAVE control word, the logical x87 stack
(abridged tag, TOP and the ST(i) registers of the FXSAVE image) and the ordered event list.

Documented limitations are listed in the report. Byte identity is not claimed."""

import argparse
import ctypes
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
from probe_pc_race_reward import ROOT, EXE, GAME_DIR

BUILD = ROOT / 'build/analysis/ginsu'
SOURCE_DIR = 'src/Speed/Indep/Src/EAXSound/Ginsu'
SOURCES = {'data': SOURCE_DIR + '/GinsuData.cpp', 'synth': SOURCE_DIR + '/GinsuSynth.cpp'}
SEED = 0x47494e53
IMAGE_BASE = 0x400000
ARENA, ARENA_SIZE = 0x2000000, 0x40000
OBJ, SYN, SAMPLES, DEST, GIN = ARENA + 0x0000, ARENA + 0x0100, ARENA + 0x0400, ARENA + 0x1800, ARENA + 0x2000
PKT1, PKT2 = ARENA + 0x3c000, ARENA + 0x3c800  # the two packet buffers of StartSynthesis (2 KB each)
STACK_BASE, STACK = 0x2100000, 0x2110000
STACK_ZERO = STACK - 0x8000
STACK_RANGE = (STACK_ZERO, STACK + 0x100)
STOP, FXAREA, STUBS = 0x2120000, 0x2121000, 0x2122000
FXSAVE_AT_STOP = b'\x0f\xae\x05' + struct.pack('<I', FXAREA)
PRECISIONS = (0x027f, 0x037f)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PRESERVED_VALUES = tuple(0x12345000 + i for i in range(4))
MAX_INSTRUCTIONS = 4000000
VTABLE = 0x789ec8
GIN_MAGIC = b'Gnsu2'

# name: (retail address, rebuilt symbol prefix, callee-popped bytes, return kind, `this` kind, argument kinds)
# return kinds: 'al' (bool), 'eax' (int), 'st0' (float left in ST0), None. argument kinds: 'w' raw word,
# 'p' pointer (identity: arena address or the stack), ('c', n) pointer whose n bytes are logged by value.
ROUTINES = {
    'Construct': (0x466750, '??0GinsuSynthData@', 0, None, 'arena', []),
    'BindToData': (0x466770, '?BindToData@GinsuSynthData@', 4, 'al', 'arena', ['p']),
    'FrequencyToSample': (0x466840, '?FrequencyToSample@GinsuSynthData@', 4, 'eax', 'arena', ['w']),
    'CycleToSample': (0x466910, '?CycleToSample@GinsuSynthData@', 4, 'eax', 'arena', ['w']),
    'CyclePeriod': (0x4669d0, '?CyclePeriod@GinsuSynthData@', 4, 'st0', 'arena', ['w']),
    'SampleToCycle': (0x466af0, '?SampleToCycle@GinsuSynthData@', 4, 'st0', 'arena', ['w']),
    'DecodeBlock': (0x466590, '?DecodeBlock@GinsuSynthData@', 4, None, 'arena', ['w']),
    'GetSamples': (0x466c60, '?GetSamples@GinsuSynthData@', 12, 'al', 'arena', ['w', 'w', 'p']),
    'CeilToInt': (0x466560, '?GinsuCeilToInt@', 0, 'eax', None, ['w']),
    'SetSynthData': (0x467160, '?SetSynthData@GinsuSynthesis@', 4, 'al', 'arena', ['p']),
    'UpdateFrequency': (0x467230, '?UpdateFrequency@GinsuSynthesis@', 8, 'al', 'arena', ['w', 'w']),
    'HandlePacketRelease': (0x466d30, '?HandlePacketRelease@GinsuSynthesis@', 4, None, 'arena', ['p']),
    'PacketReleaseCallback': (0x4777f0, '?PacketReleaseCallback@GinsuSynthesis@', 0, None, None, ['p', 'p']),
    'SynthesisConstruct': (0x477800, '??0GinsuSynthesis@', 8, None, 'arena', ['p', 'w']),
    'SynthesisDestruct': (0x477870, '??1GinsuSynthesis@', 0, None, 'arena', []),
    'StartSynthesis': (0x4778c0, '?StartSynthesis@GinsuSynthesis@', 4, 'eax', 'arena', ['w']),
}
# snd seam: key: (retail address, rebuilt symbol prefix, argument kinds, logged name, mode).
# mode 'stub': an event with a scripted EAX (see RETURNS); 'retail': a pure leaf run as retail bytes on both sides.
# 'f' is a function pointer, logged by the name of the routine it names on that side.
SND = {
    'Lock': (0x721f99, '?SNDSYS_entercritical@', [], 'SNDSYS_entercritical', 'stub'),
    'Unlock': (0x721fa7, '?SNDSYS_leavecritical@', [], 'SNDSYS_leavecritical', 'stub'),
    'Overhead': (0x723124, '?SNDPKTPLAY_overhead@', ['w'], 'SNDPKTPLAY_overhead', 'stub'),
    'Create': (0x723138, '?SNDPKTPLAY_create@', ['f', 'w', 'p', 'p', 'w'], 'SNDPKTPLAY_create', 'stub'),
    'Start': (0x7231d4, '?SNDPKTPLAY_start@', ['w', ('c', 4), ('c', 0x68), ('c', 0x18)], 'SNDPKTPLAY_start', 'stub'),
    'Submit': (0x723515, '?SNDPKTPLAY_submit@', ['w', ('c', 16)], 'SNDPKTPLAY_submit', 'stub'),
    'Stop': (0x72387b, '?SNDPKTPLAY_stop@', ['w'], 'SNDPKTPLAY_stop', 'stub'),
    'Destroy': (0x72366a, '?SNDPKTPLAY_destroy@', ['w'], 'SNDPKTPLAY_destroy', 'stub'),
    'SetAttr': (0x72423e, '?SND_attrsetdef@', ['p'], 'SND_attrsetdef', 'retail'),
    'SetPlay': (0x721a6b, '?SNDplaysetdef@', ['p'], 'SNDplaysetdef', 'retail'),
}
STUB = {'Lock': STUBS, 'Unlock': STUBS + 0x10, 'Overhead': STUBS + 0x20, 'Create': STUBS + 0x30,
        'Start': STUBS + 0x40, 'Submit': STUBS + 0x50, 'Stop': STUBS + 0x60, 'Destroy': STUBS + 0x70}
# Retail address of each pure leaf (its bytes run on both sides): SND_attrsetdef 0x72423e..0x72428b and
# SNDplaysetdef 0x721a6b..0x721aae (both end in RET and call nothing).
PURE_RANGES = ((0x72423e, 0x72428b), (0x721a6b, 0x721aae))
# Retail globals read by the recovered code, by MSVC symbol prefix; __fltused is a null symbol.
SHARED = {'?ginsuZero7D7D3C@': 0x7d7d3c, '?ginsuOne784250@': 0x784250, '?ginsuHalf784260@': 0x784260,
          '?ginsuThousand784550@': 0x784550, '?ginsuRateScaleA788890@': 0x788890,
          '?ginsuRateScaleB788894@': 0x788894, '?ginsuRateScaleC788898@': 0x788898,
          '?ginsuFilterA7ED92C@': 0x7ed92c, '?ginsuFilterB7ED93C@': 0x7ed93c,
          '?ginsuStepTable7ED950@': 0x7ed950, '__fltused': 0}
# Retail addresses of the E branch (fz1 == fz2) and of the SampleToCycle found path (0x466c2f).
E_BRANCH_HOOK, FOUND_HOOK = 0x466e98, 0x466c2f
TARGETS = ('Construct', 'BindToData', 'FrequencyToSample', 'CycleToSample', 'CyclePeriod', 'SampleToCycle',
           'DecodeBlock', 'GetSamples', 'SetSynthData', 'UpdateFrequency', 'HandlePacketRelease', 'CeilToInt',
           'PacketReleaseCallback', 'SynthesisConstruct', 'SynthesisDestruct', 'StartSynthesis')
LIMITATIONS = [
    'the uninitialised packet words are read without being written by the original (the frame word bit 31, the SNDPACKET ssf and sizeofsamples words, and the fields at 0x3c and 0x44 of the HandlePacketRelease record); the stack is zeroed before each run so both sides read zero',
    'status word condition bits and exception flags are not compared (only the control word and the logical stack)',
    'NaN is generated only for CyclePeriod and CeilToInt inputs; a NaN CyclePeriod result compares as NaN class (sign and payload not compared)',
    'finite values only for integers within |x| < 2^28 and overlap within [0, 40]',
    'the arena is compared byte for byte; the stack is compared through the events, the return values and the x87 image only',
    'the MSVC 7.1 build passes no /GS flag: this cl.exe reads /GS- as /GS (security cookies appear), and the default compile has none',
    'the stateful snd entry points are scripted: their retail bodies do not run, so the player behaviour is not compared here (the native player is checked separately); the scripted EAX is the only result they give',
    'SND_attrsetdef and SNDplaysetdef run as retail bytes on both sides, so the struct contents passed to SNDPKTPLAY_start are the retail ones by construction',
]


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def fw(x): return struct.unpack('<I', struct.pack('<f', f32(x)))[0]


def next_f32(x, up):
    bits = struct.unpack('<I', struct.pack('<f', x))[0]
    if x == 0.0:
        bits = 1 if up else 0x80000001
    else:
        bits = (bits + 1 if (x > 0) == up else bits - 1) & 0xffffffff
    return struct.unpack('<f', struct.pack('<I', bits))[0]


def ext_is_nan(value):
    mantissa, sign_exponent = struct.unpack_from('<QH', value, 0)
    return (sign_exponent & 0x7fff) == 0x7fff and (mantissa & 0x7fffffffffffffff) != 0


def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)


# Named arena regions for mismatch reports: object header, decoded samples, object block number, synth state,
# the packet sample buffers, the GetSamples destination, the packet region of StartSynthesis and the gin tables.
ARENA_REGIONS = ((OBJ, OBJ + 0x2c, 'object.header'), (OBJ + 0x2c, OBJ + 0xac, 'object.sample2C'),
                 (OBJ + 0xac, OBJ + 0xb0, 'object.currentBlockAC'), (SYN, SYN + 0x44, 'synth'),
                 (SAMPLES, SAMPLES + 0x1000, 'samples'), (DEST, DEST + 0x100, 'dest'), (PKT1, PKT2 + 0x800, 'packets'),
                 (GIN, ARENA + ARENA_SIZE, 'gin'))


def arena_regions(diff):
    counts = {}
    for offset in diff:
        address = ARENA + offset
        region = next((name for lo, hi, name in ARENA_REGIONS if lo <= address < hi), 'other')
        counts[region] = counts.get(region, 0) + 1
    return counts


def parse_gin(raw):
    if raw[:5] != GIN_MAGIC: raise ValueError('not a Gnsu2 file')
    seg, cyc, cnt, rate = struct.unpack_from('<iiii', raw, 0x10)
    freq = list(struct.unpack_from('<%di' % (seg + 1), raw, 0x20))
    cycs = list(struct.unpack_from('<%di' % (cyc + 1), raw, 0x20 + 4 * (seg + 1)))
    minimum = cnt
    for i in range(cyc): minimum = min(minimum, cycs[i + 1] - cycs[i])
    return dict(raw=raw, seg=seg, cyc=cyc, cnt=cnt, rate=rate, minf=struct.unpack_from('<f', raw, 8)[0],
                maxf=struct.unpack_from('<f', raw, 0xc)[0], freq=freq, cycs=cycs, minimum=minimum,
                nblocks=(cnt + 31) // 32)


def write_object(memory, gin, rng, rate=None, current=None):
    """The GinsuSynthData fields as BindToData sets them (object at OBJ, tables at GIN)."""
    seg, cyc, nblocks = gin['seg'], gin['cyc'], gin['nblocks']
    freq_pos = GIN + 0x20
    cyc_pos = freq_pos + 4 * (seg + 1)
    sample_data = cyc_pos + 4 * (cyc + 1)
    pk(memory, OBJ + 0x00, '<I', VTABLE)
    pk(memory, OBJ + 0x04, '<f', f32(gin['minf']))
    pk(memory, OBJ + 0x08, '<f', f32(gin['maxf']))
    pk(memory, OBJ + 0x0c, '<i', seg)
    pk(memory, OBJ + 0x10, '<i', cyc)
    pk(memory, OBJ + 0x14, '<i', gin['cnt'])
    pk(memory, OBJ + 0x18, '<i', gin['rate'] if rate is None else rate)
    pk(memory, OBJ + 0x1c, '<I', freq_pos)
    pk(memory, OBJ + 0x20, '<I', cyc_pos)
    pk(memory, OBJ + 0x24, '<I', sample_data)
    pk(memory, OBJ + 0x28, '<f', f32(gin['minimum']))
    for i in range(32): pk(memory, OBJ + 0x2c + 4 * i, '<f', rng.uniform(-40000.0, 40000.0))
    pk(memory, OBJ + 0xac, '<i', rng.choice((-1, rng.randrange(nblocks))) if current is None else current)


def write_gin(memory, raw):
    memory[GIN - ARENA:GIN - ARENA + len(raw)] = raw


# GinsuSynthesis fields (object at SYN, 0x44 bytes). Pointers and the vtable are unsigned, counts signed.
SYNTH_FIELDS = {'vtable0': (0x00, '<I'), 'packetData4': (0x04, '<I'), 'packetData8': (0x08, '<I'),
                'maxPacketSizeC': (0x0c, '<i'), 'noJumpSize10': (0x10, '<i'), 'packetSize14': (0x14, '<i'),
                'overlapSize18': (0x18, '<i'), 'packetHandle1C': (0x1c, '<I'), 'sndHandle': (0x20, '<i'),
                'sampleRate24': (0x24, '<i'), 'synthData28': (0x28, '<I'), 'playbackPos2C': (0x2c, '<i'),
                'currentPos30': (0x30, '<i'), 'targetPos34': (0x34, '<i'), 'packetCountdown38': (0x38, '<i'),
                'noJumpRemaining3C': (0x3c, '<i'), 'currentCycle40': (0x40, '<f')}


def write_synth(memory, **fields):
    for name, value in fields.items():
        offset, fmt = SYNTH_FIELDS[name]
        if fmt == '<f': value = f32(value)
        elif fmt == '<I': value &= 0xffffffff
        pk(memory, SYN + offset, fmt, value)


def scale_sizes(rate, scales):
    """Packet sizes of SetSynthData: round(float32(rate * scale)), round-half-even as FISTP."""
    a_scale, b_scale, c_scale = scales
    return tuple(round(f32(float(rate) * s)) for s in (a_scale, b_scale, c_scale))


def pointer_view(uc, w, size):
    if ARENA <= w < ARENA + ARENA_SIZE:
        return ('arena', w, bytes(uc.mem_read(w, size)) if size else None)
    if STACK_RANGE[0] <= w < STACK_RANGE[1]:
        return ('stack', bytes(uc.mem_read(w, size)) if size else None)
    return ('raw', w)


def argument_view(uc, w, kind, labels):
    if kind == 'w': return w
    if kind == 'f': return ('callback', labels.get(w, ('raw', w)))
    if isinstance(kind, tuple): return pointer_view(uc, w, kind[1])
    return pointer_view(uc, w, 0)


def entry_hook(log, name, this_kind, kinds, labels, scripted=None, key=None):
    """Logs one entry. For a stateful snd entry, scripted[key] (when present) becomes EAX before the RET stub."""
    def hook(uc, address, size, user):
        esp = uc.reg_read(xc.UC_X86_REG_ESP)
        this = pointer_view(uc, uc.reg_read(xc.UC_X86_REG_ECX), 0) if this_kind else None
        words = struct.unpack('<%dI' % len(kinds), bytes(uc.mem_read(esp + 4, 4 * len(kinds)))) if kinds else ()
        log.append((name, this, tuple(argument_view(uc, w, k, labels) for w, k in zip(words, kinds))))
        if scripted is not None and key in scripted:
            uc.reg_write(xc.UC_X86_REG_EAX, scripted[key] & 0xffffffff)
    return hook


def fx_state(uc):
    raw = bytes(uc.mem_read(FXAREA, 512))
    cw, sw = struct.unpack_from('<HH', raw, 0)
    tag, top = raw[4], (sw >> 11) & 7
    depth = bin(tag).count('1')
    if {j for j in range(8) if tag >> j & 1} != {(top + i) & 7 for i in range(depth)}:
        raise AssertionError(('fxsave layout', hex(tag), top))
    return dict(cw=cw, top=top, tag=tag, depth=depth, st=[raw[32 + 16 * i:42 + 16 * i] for i in range(depth)])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: data, synth)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--targets', default='', help='comma separated subset of targets')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/ginsu.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    assert set(manifest['functions']) == set(ROUTINES), 'manifest names'
    for name, f in manifest['functions'].items():
        address, size = int(f['address'], 16), f['size']
        assert address == ROUTINES[name][0], ('manifest address', name)
        assert hashlib.sha256(image[address - IMAGE_BASE:address - IMAGE_BASE + size]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)

    # Compile both sources. The MSVC 7.1 build omits /GS-: this cl.exe reads it as /GS (cookies on) and warns.
    if args.compiler == 'msvc':
        compiler = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe']
    else:
        compiler = ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32',
                    '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-fno-inline'] + ['/clang:' + f for f in args.clang_flag]
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)), '/I' + SOURCE_DIR,
                                   os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append((key, obj))
    parsed = {key: read_object(obj) for key, obj in objects}
    defined = {s['name'] for p in parsed.values() for s in p['symbols'].values() if s['section'] > 0}
    # Each snd entry resolves to its stub (stateful) or to its retail bytes (pure leaf).
    snd_link = {key: (STUB[key] if spec[4] == 'stub' else spec[0]) for key, spec in SND.items()}
    externals = {}
    for key, p in parsed.items():
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for prefix, a in SHARED.items() if s['name'].startswith(prefix)]
            snd = [snd_link[k] for k, spec in SND.items() if s['name'].startswith(spec[1])]
            if len(match) + len(snd) != 1:
                raise AssertionError(('unresolved external', key, s['name']))
            externals[s['name']] = match[0] if match else snd[0]
    linked = link_objects([obj for _, obj in objects], externals)
    exports = linked['exports']

    def entry_of(prefix):
        found = {a for s, a in exports.items() if s.startswith(prefix) and s not in externals}
        assert len(found) == 1, ('entry symbol', prefix, len(found))
        return found.pop()
    rebuilt_entry = {name: entry_of(spec[1]) for name, spec in ROUTINES.items()}
    rebuilt_entry.update(snd_link)
    retail_entry = {name: spec[0] for name, spec in ROUTINES.items()}
    retail_entry.update({key: spec[0] for key, spec in SND.items()})
    original_ranges = [(spec[0], spec[0] + manifest['functions'][name]['size']) for name, spec in ROUTINES.items()]
    linked_ranges = linked['ranges']
    stub_retail = [spec[0] for spec in SND.values() if spec[4] == 'stub']
    original_allowed = original_ranges + [(a, a + 1) for a in stub_retail] + list(PURE_RANGES) + [(STOP, STOP + 7)]
    rebuilt_allowed = linked_ranges + [(STUBS, STUBS + 0x80), (STOP, STOP + 7)] + list(PURE_RANGES)
    inside = lambda ranges, a: any(lo <= a < hi for lo, hi in ranges)

    # Gin files: sorted by name, five fixed positions and the last one.
    gin_dir = GAME_DIR / 'SOUND/ENGINE'
    gin_names = sorted(p.name for p in gin_dir.glob('GIN_*.gin'))
    picks = sorted({0, len(gin_names) // 5, 2 * len(gin_names) // 5, 3 * len(gin_names) // 5, 4 * len(gin_names) // 5, len(gin_names) - 1})
    gins = [parse_gin((gin_dir / gin_names[i]).read_bytes()) for i in picks]

    events = {False: [], True: []}
    counters = {False: {}, True: {}}
    returns = {}  # scripted EAX of the stateful snd entries for the current run
    labels = {False: {retail_entry['PacketReleaseCallback']: 'PacketReleaseCallback'},
              True: {rebuilt_entry['PacketReleaseCallback']: 'PacketReleaseCallback'}}
    machines = {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(IMAGE_BASE, (len(image) + 4095) & ~4095); uc.mem_write(IMAGE_BASE, bytes(image))
        uc.mem_map(ARENA, ARENA_SIZE)
        uc.mem_map(STACK_BASE, STACK + 0x1000 - STACK_BASE)
        uc.mem_map(STOP, 0x1000); uc.mem_write(STOP, FXSAVE_AT_STOP)
        uc.mem_map(FXAREA, 0x1000)
        if rebuilt:
            uc.mem_map(linked['address'], max(0x1000, linked['allocation']))
            for address, body in linked['blocks']: uc.mem_write(address, body)
            uc.mem_map(STUBS, 0x1000)
            for address in STUB.values(): uc.mem_write(address, b'\xc3')
        else:
            for spec in SND.values():
                if spec[4] == 'stub': uc.mem_write(spec[0], b'\xc3')

        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt:
                if inside(original_ranges, address):
                    raise AssertionError(('state or call order mismatch', 'original target reached', hex(address)))
                if not inside(rebuilt_allowed, address):
                    raise AssertionError(('state or call order mismatch', 'unexpected execution', hex(address)))
            else:
                if inside(linked_ranges, address):
                    raise AssertionError(('unexpected execution', 'linked code reached from the original', hex(address)))
                if not inside(original_allowed, address):
                    raise AssertionError(('unexpected execution', 'original code outside the recovered routines', hex(address)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        for name, spec in ROUTINES.items():
            address = rebuilt_entry[name] if rebuilt else spec[0]
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], name, spec[4], spec[5], labels[rebuilt]), begin=address, end=address)
        for key, spec in SND.items():
            address = rebuilt_entry[key] if rebuilt else spec[0]
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], spec[3], None, spec[2], labels[rebuilt], returns, key),
                        begin=address, end=address)
        if not rebuilt:
            def count(name, rebuilt=rebuilt):
                return lambda uc, address, size, user: counters[rebuilt].__setitem__(name, counters[rebuilt].get(name, 0) + 1)
            uc.hook_add(UC_HOOK_CODE, count('E'), begin=E_BRANCH_HOOK, end=E_BRANCH_HOOK)
            uc.hook_add(UC_HOOK_CODE, count('found'), begin=FOUND_HOOK, end=FOUND_HOOK)
        machines[rebuilt] = uc

    def run(name, memory, words, precision, this):
        """Run one routine on both machines and return the two snapshots."""
        spec = ROUTINES[name]
        snapshots = {}
        for rebuilt in (False, True):
            uc = machines[rebuilt]
            events[rebuilt].clear(); counters[rebuilt].clear()
            uc.mem_write(ARENA, bytes(memory))
            uc.mem_write(STACK_ZERO, bytes(STACK + 0x100 - STACK_ZERO))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_EDX, 0x5a5a5a5a); uc.reg_write(xc.UC_X86_REG_EAX, 0)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0)
            uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for reg, value in zip(PRESERVED, PRESERVED_VALUES): uc.reg_write(reg, value)
            try:
                uc.emu_start(rebuilt_entry[name] if rebuilt else retail_entry[name], STOP + 7, count=MAX_INSTRUCTIONS)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP + 7, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not return')
            snapshots[rebuilt] = dict(events=list(events[rebuilt]), arena=bytes(uc.mem_read(ARENA, ARENA_SIZE)),
                                      esp=uc.reg_read(xc.UC_X86_REG_ESP), eax=uc.reg_read(xc.UC_X86_REG_EAX),
                                      regs=tuple(uc.reg_read(r) for r in PRESERVED), fx=fx_state(uc),
                                      counters=dict(counters[rebuilt]))
        return snapshots[False], snapshots[True]

    def check(name, precision, a, b):
        """Compare the two snapshots. Every category must match; a failure lists each category that differs."""
        spec = ROUTINES[name]
        cleanup, kind = spec[2], spec[3]
        if a['esp'] != STACK + 4 + cleanup or a['fx']['cw'] != precision:
            raise AssertionError(('original contract', name, hex(a['esp']), hex(a['fx']['cw'])))

        # Every category is evaluated, so a failure lists each one that differs.
        problems = []
        if a['events'] != b['events']:
            k = next((i for i, (x, y) in enumerate(zip(a['events'], b['events'])) if x != y), min(len(a['events']), len(b['events'])))
            problems.append(('event list', k, len(a['events']), len(b['events']), a['events'][k:k + 1], b['events'][k:k + 1]))
        if a['arena'] != b['arena']:
            diff = [i for i in range(ARENA_SIZE) if a['arena'][i] != b['arena'][i]]
            problems.append(('arena', len(diff), hex(ARENA + diff[0]), arena_regions(diff),
                             a['arena'][diff[0]:diff[0] + 4].hex(), b['arena'][diff[0]:diff[0] + 4].hex()))
        if b['esp'] != a['esp']: problems.append(('esp', hex(a['esp']), hex(b['esp'])))
        if a['regs'] != b['regs']: problems.append(('preserved registers', a['regs'], b['regs']))
        if b['fx']['cw'] != precision: problems.append(('rebuilt control word', hex(b['fx']['cw'])))
        if kind == 'al' and (a['eax'] & 0xff) != (b['eax'] & 0xff): problems.append(('AL', a['eax'] & 0xff, b['eax'] & 0xff))
        if kind == 'eax' and a['eax'] != b['eax']: problems.append(('EAX', hex(a['eax']), hex(b['eax'])))
        fa, fb = a['fx'], b['fx']
        expected = 1 if kind == 'st0' else 0
        if fb['depth'] != expected: problems.append(('rebuilt x87 depth', fb['depth'], expected))
        if fa['depth'] != fb['depth']: problems.append(('original x87 depth', fa['depth'], fb['depth']))
        for i in range(min(fa['depth'], fb['depth'])):
            x, y = fa['st'][i], fb['st'][i]
            if x == y or (name == 'CyclePeriod' and ext_is_nan(x) and ext_is_nan(y)): continue
            problems.append(('x87 ST(%d)' % i, x.hex(), y.hex()))
        if problems:
            raise AssertionError(('state or call order mismatch', name, hex(precision)) + tuple(problems))

    rng = random.Random(SEED)
    selected = [t for t in args.targets.split(',') if t] or list(TARGETS)
    counts = dict.fromkeys(TARGETS, 0)
    paths = {}
    failures = []
    attempts = dict.fromkeys(TARGETS, 0)

    def record_failure(name, trial, precision, error):
        failures.append((name, trial, precision, str(error.args)))

    def note(path): paths[path] = paths.get(path, 0) + 1

    def random_memory(): return bytearray(rng.randbytes(ARENA_SIZE))

    def cycle_input(gin):
        k = rng.random()
        if k < 0.5: return f32(rng.uniform(-1.0, gin['cyc'] + 1.0))
        if k < 0.65: return float(rng.randint(0, gin['cyc'] + 1))
        return rng.choice((-0.0, float('inf'), -float('inf'), 1e30, rng.uniform(-1.0, gin['cyc'] + 1.0)))

    def frequency_input(gin):
        lo, hi = gin['minf'], gin['maxf']; span = hi - lo
        k = rng.random()
        if k < 0.35: return f32(rng.uniform(lo - 0.2 * span, hi + 0.2 * span))
        if k < 0.7:
            return rng.choice((lo, hi, next_f32(lo, True), next_f32(lo, False), next_f32(hi, True), next_f32(hi, False),
                               f32(f32(rng.randint(0, gin['seg']) * f32(span / gin['seg'])) + lo),
                               -0.0, float('inf'), -float('inf'), 1e30, -1e30))
        return f32(rng.uniform(lo - 0.2 * span, hi + 0.2 * span))

    def sample_input(gin):
        cycs = gin['cycs']
        k = rng.random()
        if k < 0.3: return rng.randint(cycs[0] - 3, cycs[-1] + 3)
        if k < 0.6:
            i = rng.randrange(len(cycs))
            return cycs[i] + rng.choice((0, 0, 1, -1))
        if k < 0.75:
            i = rng.randrange(len(cycs) - 1)
            return (cycs[i] + cycs[i + 1]) // 2
        return rng.randint(-2 ** 20, 2 ** 28 - 1)

    def create_result():
        return rng.choice((rng.randrange(0, 8), rng.randrange(0, 8), -9, -10, -6, rng.randint(-2 ** 31, -1),
                           rng.randint(8, 2 ** 31 - 1)))

    scales = tuple(struct.unpack_from('<f', image, a - IMAGE_BASE)[0] for a in (0x788890, 0x788894, 0x788898))

    for trial in range(args.cases):
        precision = rng.choice(PRECISIONS)
        gin = gins[rng.randrange(len(gins))]
        for name in selected:
            memory = random_memory()
            returns.clear()
            if name == 'Construct':
                words, this = [], OBJ
                note('construct')
            elif name == 'BindToData':
                kind = rng.random()
                if kind < 0.4:
                    write_gin(memory, gin['raw']); note('bind-real')
                elif kind < 0.6:
                    data = bytearray(gin['raw'])
                    k = rng.randrange(5)
                    data[k] = (data[k] + rng.randrange(1, 256)) & 0xff
                    write_gin(memory, bytes(data)); note('bind-bad-magic-%d' % k)
                else:
                    seg, cyc, cnt = rng.randint(0, 40), rng.randint(0, 300), rng.randint(0, 5000)
                    header = bytearray(0x20)
                    header[0:5] = GIN_MAGIC
                    struct.pack_into('<ffiiii', header, 8, f32(rng.uniform(-1e6, 1e6)), f32(rng.uniform(-1e6, 1e6)),
                                     seg, cyc, cnt, rng.getrandbits(32) - 2 ** 31)
                    tables = struct.pack('<%di' % (seg + 1 + cyc + 1), *[rng.randint(-2 ** 28 + 1, 2 ** 28 - 1) for _ in range(seg + 1 + cyc + 1)])
                    write_gin(memory, bytes(header) + tables); note('bind-random-header')
                words, this = [GIN], OBJ
            elif name in ('FrequencyToSample', 'CycleToSample', 'CyclePeriod', 'SampleToCycle', 'DecodeBlock', 'GetSamples'):
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                this = OBJ
                if name == 'FrequencyToSample':
                    words = [fw(frequency_input(gin))]
                elif name == 'CycleToSample':
                    words = [fw(cycle_input(gin))]
                elif name == 'CyclePeriod':
                    if rng.random() < 0.08:
                        value = float('nan')
                    else:
                        value = cycle_input(gin) if rng.random() < 0.8 else rng.uniform(-2.0, gin['cyc'] + 2.0)
                    words = [fw(value)]
                elif name == 'SampleToCycle':
                    words = [sample_input(gin) & 0xffffffff]
                elif name == 'DecodeBlock':
                    k = rng.random()
                    block = 0 if k < 0.1 else gin['nblocks'] - 1 if k < 0.2 else rng.randrange(gin['nblocks'])
                    words = [block]
                    pk(memory, OBJ + 0xac, '<i', rng.choice((-1, rng.randrange(gin['nblocks']))))
                else:
                    cnt, nb = gin['cnt'], gin['nblocks']
                    k = rng.random()
                    if k < 0.2:
                        start, count = rng.randint(-3, cnt + 3), rng.randint(-2, 70)
                    elif k < 0.4:
                        start, count = 32 * rng.randint(-1, nb + 1) + rng.choice((-1, 0, 1)), rng.randint(-2, 70)
                    elif k < 0.6:
                        start = rng.randint(cnt - 69, cnt + 3); count = cnt - start + 1
                    elif k < 0.8:
                        start = rng.randint(cnt - 70, cnt + 2); count = cnt - start
                    else:
                        start, count = rng.randint(-3, cnt + 3), rng.randint(-2, 70)
                    if rng.random() < 0.5 and 0 <= start < cnt:
                        pk(memory, OBJ + 0xac, '<i', start >> 5)  # stale block: the decode buffer is used as is
                    words = [start & 0xffffffff, count & 0xffffffff, DEST]
            elif name == 'SetSynthData':
                kind = rng.random()
                rate = gin['rate']
                if kind < 0.3:
                    rate = rng.choice((0, 22050, 44100, 48000, rng.randint(1, 2 ** 20)))
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng, rate=rate)
                if rate:
                    sr24 = rng.choice((0, rate, rate + 1000, rng.randint(1, 2 ** 20)))
                else:
                    sr24 = rng.choice((0, 22050, 44100))
                if rate and rng.random() < 0.3:
                    a_size, b_size, c_size = scale_sizes(rate, scales)
                    max_size = b_size + c_size
                    max_size += rng.choice((0, 0, -1, 1)) if rng.random() < 0.5 else 0
                    max_size = max(0, min(max_size, 4096))
                    note('set-boundary')
                else:
                    max_size = rng.randint(0, 4096)
                write_synth(memory, maxPacketSizeC=max_size, sampleRate24=sr24,
                            synthData28=0 if rng.random() < 0.5 else OBJ)
                words, this = [OBJ], SYN
            elif name == 'UpdateFrequency':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, synthData28=OBJ)
                target = frequency_input(gin)
                latency = rng.choice((0.0, 1e-30, 0.5, 1.0, 1e6, rng.uniform(0.0, 2.0), -0.5, -1e6, -rng.uniform(0.0, 2.0)))
                words, this = [fw(target), fw(latency)], SYN
            elif name in ('HandlePacketRelease', 'PacketReleaseCallback'):
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                cnt = gin['cnt']
                max_size = rng.randint(16, 1024)
                packet = rng.randint(1, max_size)
                countdown = rng.choice((0, 1, 2, 3, 5, 10, rng.randint(1, 40)))
                write_synth(memory, maxPacketSizeC=max_size, packetSize14=packet, overlapSize18=rng.randint(0, 40),
                            noJumpSize10=rng.randint(0, 600), noJumpRemaining3C=rng.randint(-100, 700),
                            playbackPos2C=rng.randint(0, cnt - 1), currentPos30=rng.randint(0, cnt - 1),
                            targetPos34=rng.randint(0, cnt - 1), packetCountdown38=countdown,
                            currentCycle40=f32(rng.uniform(0.0, gin['cyc'])), packetHandle1C=rng.getrandbits(32),
                            sampleRate24=rng.choice((0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=OBJ)
                # The callback receives the samples and the GinsuSynthesis; the routine itself is the same as above.
                words = [SAMPLES] if name == 'HandlePacketRelease' else [SAMPLES, SYN]
                this = SYN
            elif name == 'SynthesisConstruct':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                overhead = rng.choice((0x19c, 0x19c, 0x19c, 0x9c + 32 * rng.randrange(0, 40), rng.randrange(0, 0x2000)))
                returns['Overhead'] = overhead
                returns['Create'] = create_result()
                k = rng.random()
                if k < 0.5:
                    size = overhead + 8 * rng.randint(36, 44) + rng.choice((-8, -4, -1, 0, 0, 1, 4, 7, 8))
                    note('ctor-boundary')
                elif k < 0.8:
                    size = rng.randint(0, 0x30000)
                else:
                    size = overhead - rng.randint(1, 0x100)
                    note('ctor-size-below-overhead')
                words, this = [PKT1, size & 0xffffffff], SYN
            elif name == 'SynthesisDestruct':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, packetHandle1C=rng.choice((-1, rng.randrange(0, 8), rng.randint(-2 ** 31, 2 ** 31 - 1))),
                            sampleRate24=rng.choice((0, 0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=rng.choice((0, OBJ)))
                words, this = [], SYN
            elif name == 'StartSynthesis':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                cnt = gin['cnt']
                write_synth(memory, packetData4=PKT1, packetData8=PKT2,
                            packetHandle1C=rng.choice((-1, rng.randrange(0, 8), rng.randrange(0, 8), rng.randint(-2 ** 31, 2 ** 31 - 1))),
                            sndHandle=rng.randint(-2 ** 31, 2 ** 31 - 1),
                            sampleRate24=rng.choice((0, 0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=rng.choice((OBJ, OBJ, OBJ, 0)),
                            noJumpSize10=rng.randint(0, 600), packetSize14=rng.randint(0, 1024),
                            overlapSize18=rng.randint(0, 40), maxPacketSizeC=rng.randint(0, 4096),
                            playbackPos2C=rng.randint(0, cnt - 1), currentPos30=rng.randint(0, cnt - 1),
                            targetPos34=rng.randint(0, cnt - 1), packetCountdown38=rng.randint(0, 40),
                            noJumpRemaining3C=rng.randint(-100, 700), currentCycle40=f32(rng.uniform(0.0, gin['cyc'])))
                returns['Start'] = rng.choice((rng.randrange(0, 8), rng.randrange(0, 8), -9, -10, -6, -13,
                                               rng.randint(-2 ** 31, -1)))
                words, this = [fw(frequency_input(gin))], SYN
            elif name == 'CeilToInt':
                k = rng.random()
                value = rng.choice((rng.uniform(-1000.0, 1000.0), float(rng.randint(-1000, 1000)), rng.uniform(-1000.0, 1000.0) + 0.5,
                                    -0.0, float('nan'), float('inf'), -float('inf'), 1e30, -1e30, 0.5, -0.5, 1e-40)) if k < 0.95 else float('nan')
                words, this = [fw(value)], 0

            attempts[name] += 1
            try:
                a, b = run(name, memory, words, precision, this)
            except AssertionError as error:
                record_failure(name, trial, precision, error)
                continue
            names = [e[0] for e in a['events']]
            # The original's run defines the coverage, whatever the comparison says.
            if name == 'HandlePacketRelease':
                if 'CyclePeriod' not in names: note('hpr-J')
                else: note('hpr-N-E' if a['counters'].get('E') else 'hpr-N-D')
                if 'CeilToInt' in names: note('hpr-E1')
                gets = names.count('GetSamples')
                if 'CycleToSample' in names:
                    note('hpr-k-nonzero')
                    note('hpr-crossfade-rem-positive' if gets == 4 else 'hpr-crossfade-rem-nonpositive')
            if name == 'SampleToCycle': note('sts-found' if a['counters'].get('found') else 'sts-boundary')
            if name == 'CyclePeriod' and a['fx']['depth'] and ext_is_nan(a['fx']['st'][0]): note('cycleperiod-nan-result')
            if name == 'GetSamples': note('gs-ok' if (a['eax'] & 0xff) else 'gs-rejected')
            if name == 'SetSynthData': note('set-accepted' if (a['eax'] & 0xff) else 'set-rejected')
            if name == 'BindToData': note('bind-ok' if (a['eax'] & 0xff) else 'bind-rejected')
            if name == 'SynthesisConstruct': note('ctor-create' if 'SNDPKTPLAY_create' in names else 'ctor-no-create')
            if name == 'SynthesisDestruct': note('dtor-stop' if 'SNDPKTPLAY_stop' in names else 'dtor-plain')
            if name == 'StartSynthesis':
                note('start-refused' if a['eax'] & 0x80000000 else 'start-started')
                note('start-stop-first' if 'SNDPKTPLAY_stop' in names else 'start-no-stop')
            try:
                check(name, precision, a, b)
            except AssertionError as error:
                record_failure(name, trial, precision, error)
                continue
            counts[name] += 1
    report_counts = dict(counts)
    total = sum(counts.values())
    failed_counts = {name: sum(1 for f in failures if f[0] == name) for name in selected}
    report = dict(compiler=args.compiler, attempts=attempts, comparisons=report_counts, total=total, paths=dict(sorted(paths.items())),
                  failures=failed_counts, failure_total=len(failures),
                  first_failures={name: 'trial %d precision %#x: %s' % next((f[1], f[2], f[3][:700]) for f in failures if f[0] == name)
                                  for name in selected if failed_counts[name]},
                  x87_precision=['53-bit (0x027f)', '64-bit (0x037f)'], cases=args.cases, gin_files=[gin_names[i] for i in picks],
                  limitations=LIMITATIONS, fingerprints=manifest)
    if args.mutation_checks:
        if failures:
            raise SystemExit('mutation checks need a passing baseline: %d failures in the unmutated run' % len(failures))
        mutations = {
            'freq-min-strict': ('data', 'if (frequency <= minFrequency4) {', 'if (frequency < minFrequency4) {'),
            'cycle-max-strict': ('data', 'if ((float)cycleCount10 <= cycle) {', 'if ((float)cycleCount10 < cycle) {'),
            'decode-byte1-low': ('data', 'return (c2 * a + l) + c1 * ((c1 * a + h) + c2 * b);',
                                 'return (c1 * a + l) + c1 * ((c1 * a + h) + c2 * b);'),
            'decode-groups': ('data', 'group < 3', 'group < 2'),
            'getsamples-pos': ('data', 'pos == 32', 'pos == 31'),
            'getsamples-end': ('data', 'start + count - 1 >= sampleCount14', 'start + count - 1 > sampleCount14'),
            'cycleperiod-clamp': ('data', 'cycle = 0.0f;', ''),
            'sample-cand-equal': ('data', 'if (cand > lo) {', 'if (cand >= lo) {'),
            'sample-ceil-floor': ('data', 'const int cand = idx - GinsuCeilOf(q);', 'const int cand = idx - GinsuFloorOf(q);'),
            'cycle-volatile': ('data', 'volatile float v = (float)(cyclePos20[idx + 1]', 'float v = (float)(cyclePos20[idx + 1]'),
            'sample-saturation': ('data', 'return GinsuSignMask(value) ? (short)-32768 : (short)32767;', 'return GinsuSignMask(value) ? (short)32767 : (short)-32768;'),
            'minperiod-plus-one': ('data', 'minPeriod28 = (float)minimum;', 'minPeriod28 = (float)(minimum + 1);'),
            'handle-z-order': ('synth', 'k = (z1 < z2) ? GinsuCeilOf(z1) : GinsuFloorOf(z1);', 'k = (z1 <= z2) ? GinsuCeilOf(z1) : GinsuFloorOf(z1);'),
            'handle-z1-zero': ('synth', 'k = (z1 < 0.0f) ? GinsuCeilToInt(z1) : GinsuFloorOf(z1);', 'k = (z1 <= 0.0f) ? GinsuCeilToInt(z1) : GinsuFloorOf(z1);'),
            'handle-key-bit': ('synth', 'key ^= (key ^ (unsigned int)spliceCount) & 0x7fffffffu;', 'key = (unsigned int)spliceCount;'),
            'handle-countdown-gt2': ('synth', 'if (packetCountdown38 > 1) {', 'if (packetCountdown38 > 2) {'),
            'update-countdown-round': ('synth', 'packetCountdown38 = r + GinsuSignMask(d) + 1;', 'packetCountdown38 = r + GinsuSignMask(d) + 0;'),
            'set-size-equal': ('synth', 'if (c + b > maxPacketSizeC) {', 'if (c + b >= maxPacketSizeC) {'),
            'handle-nojump-overlap': ('synth', 'noJumpRemaining3C = noJumpSize10 - overlapSize18;', 'noJumpRemaining3C = noJumpSize10;'),
            'handle-multiplier': ('synth', 'multiplier = (k != 0) ? (float)qRounded : ginsuOne784250;', 'multiplier = (float)qRounded;'),
            'handle-wp-volatile': ('synth', 'volatile float wp = (ginsuRateScaleA788890 / ginsuRateScaleB788894) * y1;', 'float wp = (ginsuRateScaleA788890 / ginsuRateScaleB788894) * y1;'),
            'handle-u-volatile': ('synth', 'volatile float u = (float)k + data->SampleToCycle(playbackPos2C);', 'float u = (float)k + data->SampleToCycle(playbackPos2C);'),
            'callback-this': ('synth', 'GinsuSynthesis *object = static_cast<GinsuSynthesis *>(clientdata);', 'GinsuSynthesis *object = static_cast<GinsuSynthesis *>(samples);'),
            'ctor-threshold': ('synth', 'if (maxPacketSizeC >= 0x50) {', 'if (maxPacketSizeC > 0x50) {'),
            'ctor-shift': ('synth', 'const unsigned int n = (unsigned int)(size - overhead) >> 3;', 'const unsigned int n = (unsigned int)(size - overhead) >> 2;'),
            'ctor-buffer2': ('synth', 'packetData[1] = reinterpret_cast<short *>(base + 4 * n);', 'packetData[1] = reinterpret_cast<short *>(base + 2 * n);'),
            'ctor-memory': ('synth', 'base + 8 * n, overhead);', 'base + 8 * n + 4, overhead);'),
            'ctor-handle-init': ('synth', '    packetHandle1C = -1;\n    sampleRate24 = 0;', '    packetHandle1C = 0;\n    sampleRate24 = 0;'),
            'dtor-vtable': ('synth', '    SNDPKTPLAY_destroy(packetHandle1C);\n    vtable0 = (void *)0x789ce4;', '    SNDPKTPLAY_destroy(packetHandle1C);'),
            'start-handle-check': ('synth', 'if (packetHandle1C < 0 || synthData28 == 0) {', 'if (packetHandle1C <= 0 || synthData28 == 0) {'),
            'start-samplerep': ('synth', 'format.samplerep = 8;', 'format.samplerep = 7;'),
            'start-rendermode': ('synth', 'attr.rendermode = 4;', 'attr.rendermode = 3;'),
            'start-volume': ('synth', 'options.vol = 0;', 'options.vol = 0x7f;'),
            'start-zero-length': ('synth', 'GinsuZeroBytes(packetData[1], (unsigned int)packetSize14 * 2u);', 'GinsuZeroBytes(packetData[1], (unsigned int)packetSize14 * 2u + 1u);'),
            'start-rate-store': ('synth', '        sampleRate24 = synthData28->sampleRate18;', '        sampleRate24 = 0;'),
            'start-second-buffer': ('synth', 'packet.psamples[0] = packetData[1];', 'packet.psamples[0] = packetData[0];'),
            'start-unlock': ('synth', '    SNDSYS_leavecritical();\n    return sndHandle;', '    return sndHandle;'),
            'start-return': ('synth', 'return sndHandle;', 'return 0;'),
            'start-key-bit': ('synth', 'key ^= (key ^ (unsigned int)packetSize14) & 0x7fffffffu;', 'key = (unsigned int)packetSize14;'),
        }
        rejected, survived, no_mismatch_text = [], [], []
        for label, (key, before, after) in mutations.items():
            text = (ROOT / SOURCES[key]).read_text(); assert text.count(before) == 1, (label, text.count(before))
            text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--replace', key + '=' + str(mutant),
                                     '--cases', str(args.cases)],
                                    cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            if result.returncode:
                rejected.append(label)
                if 'state or call order mismatch' not in result.stdout: no_mismatch_text.append(label)
            else:
                survived.append(label)
        report['mutation_checks'] = dict(rejected=rejected, survived=survived, rejected_without_mismatch_text=no_mismatch_text,
                                         total=len(mutations))
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))
    if failures:
        sys.exit(1)


if __name__ == '__main__':
    main()
