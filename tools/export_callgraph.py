#!/usr/bin/env python3
"""Write build/ghidra/callgraph.jsonl (one {"a": start, "calls": [targets]} per function).

Stand-in for a Ghidra export: reads the function index from tools/recover_pc_index.py and
collects the direct `call rel32` targets that are themselves indexed function starts, using
Capstone over each function's byte range. Indirect calls are not followed (as in a plain
static call graph). Usage: export_callgraph.py SPEED2.EXE [--index build/analysis/function-index.json]
"""
import argparse
import json
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM

ROOT = Path(__file__).resolve().parents[1]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe', type=Path)
    ap.add_argument('--index', type=Path, default=ROOT / 'build/analysis/function-index.json')
    ap.add_argument('--output', type=Path, default=ROOT / 'build/ghidra/callgraph.jsonl')
    a = ap.parse_args()
    pe = pefile.PE(str(a.exe))
    image = pe.get_memory_mapped_image()
    base = pe.OPTIONAL_HEADER.ImageBase
    funcs = json.loads(a.index.read_text())['functions']
    starts = {int(f['start'], 16) for f in funcs}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    a.output.parent.mkdir(parents=True, exist_ok=True)
    with a.output.open('w') as out:
        for f in funcs:
            s = int(f['start'], 16)
            code = bytes(image[s - base:s - base + f['size']])
            calls = set()
            for ins in md.disasm(code, s):
                if ins.mnemonic == 'call' and ins.operands and ins.operands[0].type == X86_OP_IMM \
                        and ins.operands[0].imm in starts:
                    calls.add(ins.operands[0].imm)
            out.write(json.dumps({'a': s, 'calls': sorted(calls)}) + '\n')
    print(f'{len(funcs)} functions -> {a.output}')


if __name__ == '__main__':
    main()
