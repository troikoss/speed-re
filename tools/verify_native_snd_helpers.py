#!/usr/bin/env python3
"""Byte check of the native SND_attrsetdef and SNDplaysetdef against the retail routines (speed2.exe).

The two snd helpers are pure: each only writes its argument. The retail bytes at 0x72423e (SND_attrsetdef, 0x68-byte
SNDSAMPLEATTR) and 0x721a6b (SNDplaysetdef, 0x18-byte SNDPLAYOPTS) run in Unicorn on random initial contents; the
native functions in src/Platform/Native/NativeAudio.cpp run on the same contents through tools/verify_native_snd_helpers.cpp.
Every byte of the result must match, including the bytes the retail routine leaves untouched.
"""
import argparse
import os
from pathlib import Path
import random
import struct
import subprocess
import sys

import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32
import unicorn.x86_const as xc
from probe_pc_race_reward import ROOT, EXE

IMAGE_BASE = 0x400000
ARG = 0x2000000
STACK_TOP = 0x2110000
STOP = 0x2120000
HELPERS = {'SND_attrsetdef': (0x72423e, 0x68), 'SNDplaysetdef': (0x721a6b, 0x18)}
BUILD = ROOT / 'build/analysis/ginsu'


def retail_run(image, entry, initial):
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(IMAGE_BASE, (len(image) + 4095) & ~4095)
    uc.mem_write(IMAGE_BASE, bytes(image))
    uc.mem_map(ARG, 0x1000)
    uc.mem_write(ARG, initial)
    uc.mem_map(STACK_TOP - 0x10000, 0x10000)
    uc.mem_map(STOP, 0x1000)
    uc.mem_write(STOP, b'\x90')
    uc.mem_write(STACK_TOP - 0x100, struct.pack('<II', STOP, ARG))
    uc.reg_write(xc.UC_X86_REG_ESP, STACK_TOP - 0x100)
    try:
        uc.emu_start(entry, STOP, count=10000)
    except UcError as error:
        raise SystemExit('retail helper faulted at %#x: %s' % (entry, error))
    assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP
    return bytes(uc.mem_read(ARG, len(initial)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--cases', type=int, default=256)
    parser.add_argument('--seed', type=int, default=0x534e44)
    args = parser.parse_args()
    raw = EXE.read_bytes()
    image = pefile.PE(data=raw).get_memory_mapped_image()
    BUILD.mkdir(parents=True, exist_ok=True)
    driver = BUILD / 'native_snd_helpers'
    # The vendored include directory holds the 32-bit SDL2 configuration header, as in the CMake native build.
    subprocess.run(['g++', '-m32', '-O1', '-Wall', '-isystem', str(ROOT / 'src/Platform/Native/include'), '-o', str(driver),
                    str(ROOT / 'tools/verify_native_snd_helpers.cpp'), str(ROOT / 'src/Platform/Native/NativeAudio.cpp'),
                    '-L/usr/lib/i386-linux-gnu', '-l:libSDL2-2.0.so.0'], cwd=ROOT, check=True)

    rng = random.Random(args.seed)
    cases = []
    for _ in range(args.cases):
        attr = bytes(rng.randrange(256) for _ in range(HELPERS['SND_attrsetdef'][1]))
        play = bytes(rng.randrange(256) for _ in range(HELPERS['SNDplaysetdef'][1]))
        expected = (retail_run(image, HELPERS['SND_attrsetdef'][0], attr), retail_run(image, HELPERS['SNDplaysetdef'][0], play))
        cases.append((attr, play, expected))

    text = ''.join('%s %s\n' % (attr.hex(), play.hex()) for attr, play, _ in cases)
    result = subprocess.run([str(driver)], input=text, text=True, capture_output=True, cwd=ROOT, timeout=120)
    if result.returncode != 0:
        raise SystemExit('native driver failed: %s' % result.stderr)
    lines = result.stdout.splitlines()
    assert len(lines) == len(cases), (len(lines), len(cases))
    mismatches = []
    changed_attr = changed_play = 0
    for index, ((attr, play, (want_attr, want_play)), line) in enumerate(zip(cases, lines)):
        got_attr, got_play = (bytes.fromhex(part) for part in line.split())
        changed_attr += sum(1 for a, b in zip(attr, want_attr) if a != b)
        changed_play += sum(1 for a, b in zip(play, want_play) if a != b)
        if got_attr != want_attr:
            mismatches.append(('SND_attrsetdef', index, [i for i in range(len(want_attr)) if got_attr[i] != want_attr[i]][:8]))
        if got_play != want_play:
            mismatches.append(('SNDplaysetdef', index, [i for i in range(len(want_play)) if got_play[i] != want_play[i]][:8]))
    print('cases %d: SND_attrsetdef and SNDplaysetdef compared byte for byte' % len(cases))
    print('retail writes changed %d attribute bytes and %d playset bytes across the cases' % (changed_attr, changed_play))
    print('mismatches: %d' % len(mismatches))
    for item in mismatches[:10]:
        print('  %s case %d differs at offsets %s' % item)
    if mismatches:
        sys.exit(1)
    print('PASS')


if __name__ == '__main__':
    main()
