#!/usr/bin/env python3
"""Record bounded original formatter conversions; pointer validation alone is controlled.

This is original-only investigation, not original/rebuilt verification.
"""
import hashlib
import json
import struct
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP,
                              UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from probe_pc_race_reward import ROOT, EXE

ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
CONTEXT, BUFFER, FORMAT, ARGS, TEXT = [ARENA + n for n in (0x100, 0x200, 0x1000, 0x2000, 0x3000)]
BAD = 0x0bad0001


def main():
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == 'f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7'
    image = pefile.PE(data=raw).get_memory_mapped_image()
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
    uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
    state = {}
    def hook(uc, address, size, user):
        if address == 0x43bd60:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret, pointer = struct.unpack('<II', uc.mem_read(sp, 8))
            valid = pointer != BAD
            state['validation'].append((hex(pointer), valid))
            uc.reg_write(UC_X86_REG_EAX, int(valid))
            uc.reg_write(UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP, sp + 4); uc.reg_write(UC_X86_REG_EIP, ret)
        elif not (0x43e950 <= address < 0x43f5da or 0x43cb40 <= address < 0x43cbc2 or
                  0x75bed4 <= address < 0x75bf80):
            state['unresolved'] = hex(address); uc.emu_stop()
    uc.hook_add(UC_HOOK_CODE, hook)
    def double(value): return list(struct.unpack('<II', struct.pack('<d', value)))
    cases = [(b'literal', []), (b'%s', [TEXT]), (b'%.0s', [TEXT]), (b'%.3s', [TEXT]),
             (b'%s', [0]), (b'%s', [BAD]), (b'%#x', [0x123]), (b'%#o', [9]), (b'%#.0x', [0]),
             (b'%05d', [0xfffffffe]), (b'%+06d', [7]), (b'% d', [7]), (b'%-6d!', [7]),
             (b'%*.*d', [6, 3, 7]), (b'%*d', [0xfffffffa, 7]), (b'%b', [10]),
             (b'%p', [0x1234abcd]), (b'%hd', [0xffff]), (b'%hu', [0x1234ffff]),
             (b'%lu', [0x12345678]), (b'%u', [0xffffffff]), (b'%z', [0x18000]),
             (b'%.0z', [0x18000]), (b'%.3z', [0xffff8000]), (b'%f', double(1.25)),
             (b'%.2f', double(1.999)), (b'%e', double(0.125)), (b'%g', double(123.5)),
             (b'%f', double(float('inf'))), (b'%f', double(float('-inf'))),
             (b'%f', double(float('nan'))), (b'abc%n!', [ARENA + 0x4000]),
             (b'%q', []), (b'%%', []), (b'trailing%', [])]
    reports = []
    for precision in (0x27f, 0x37f):
        for format_, arguments in cases:
            uc.mem_write(ARENA, bytes(0x10000))
            uc.mem_write(CONTEXT, struct.pack('<III', BUFFER, 0, 0))
            uc.mem_write(FORMAT, format_ + b'\0'); uc.mem_write(TEXT, b'abcdef\0')
            if arguments: uc.mem_write(ARGS, struct.pack('<' + 'I' * len(arguments), *arguments))
            uc.mem_write(STACK, struct.pack('<4I', STOP, CONTEXT, FORMAT, ARGS))
            uc.reg_write(UC_X86_REG_ESP, STACK)
            uc.reg_write(UC_X86_REG_FPCW, precision); uc.reg_write(UC_X86_REG_FPSW, 0); uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
            # Reset mutable grouping bytes from the reference between probes.
            uc.mem_write(0x7eb3f0, image[0x3eb3f0:0x3eb3f4])
            state.clear(); state.update(validation=[])
            error = None
            try: uc.emu_start(0x43e950, STOP, count=200000)
            except UcError as exc: error = str(exc)
            complete = uc.reg_read(UC_X86_REG_EIP) == STOP and error is None
            data = bytes(uc.mem_read(BUFFER, 512)).split(b'\0', 1)[0]
            reports.append({'format': format_.decode('ascii'), 'arguments': [hex(a) for a in arguments],
                            'x87': hex(precision), 'complete': complete,
                            'result': uc.reg_read(UC_X86_REG_EAX) if complete else None,
                            'output_latin1': data.decode('latin1'), 'validation': state['validation'],
                            'count_n': struct.unpack('<I', uc.mem_read(ARENA + 0x4000, 4))[0],
                            'unresolved': state.get('unresolved'), 'error': error})
    output = ROOT / 'build/analysis/bware-format-sink/core-probe.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({'executable_sha256': hashlib.sha256(raw).hexdigest(),
                                 'controlled_interfaces': ['pointer validation 0x43bd60'], 'cases': reports}, indent=2) + '\n')
    for report in reports[:len(cases)]:
        print(report['format'], repr(report['output_latin1']), report['result'], report['unresolved'] or report['error'] or '')
    print('Original-only report:', output)


if __name__ == '__main__': main()
