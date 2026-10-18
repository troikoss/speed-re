#!/usr/bin/env python3
"""Compare full UG2 grid initial fill, navigation, gates, wrap and hash with callback mutations."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-grid'
ARENA, SELF, CELLS_A, CELLS_B, STRING, STACK, STOP = 0x2000000, 0x2000100, 0x2001000, 0x2003000, 0x2008000, 0x200f000, 0x2100000
OWNERS, VTABLES = (ARENA + 0x400, ARENA + 0x420), (ARENA + 0x500, ARENA + 0x580)
MESSAGES, REFILLS, CUSTOMS = (STOP + 0x180, STOP + 0x1c0), (STOP + 0x4c0, STOP + 0x500), (STOP + 0x600, STOP + 0x640)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PREFIXES = {'WrapIndex': '?FEngGridWrapIndex@', 'FngHash': '?FngHash@', 'CanShift': '?CanShift@FEGridObject@',
            'MoveSelection': '?MoveSelection@FEGridObject@', 'ShiftAxis': '?ShiftAxis@FEGridObject@', 'Navigate': '?Navigate@FEGridObject@', 'InitialFill': '?InitialFill@FEGridObject@'}
ARGUMENTS = {'WrapIndex': 2, 'FngHash': 1, 'CanShift': 5, 'MoveSelection': 5, 'ShiftAxis': 6, 'Navigate': 2, 'InitialFill': 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngGrid.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    parser.add_argument('--hash-source', type=Path, default=ROOT / 'src/Speed/Indep/Src/Frontend/FEngHash.cpp')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/feng-grid.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        offset = int(f['address'], 16) - 0x400000
        assert hashlib.sha256(image[offset:offset + f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True); objects = []
    for source in (args.source, args.hash_source):
        obj = BUILD / (source.stem + '.obj')
        compiler = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if args.compiler == 'msvc' else
                    ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                 '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    linked = link_objects(objects, {})
    entries = {name: next(a for n, a in linked['exports'].items() if n.startswith(prefix)) for name, prefix in PREFIXES.items()}
    markers = {0x504cca: 'initial-custom', 0x504d58: 'initial-owner', 0x519079: 'positive-horizontal-custom', 0x51913a: 'positive-horizontal-owner',
               0x5191fe: 'negative-horizontal-custom', 0x5192c1: 'negative-horizontal-owner',
               0x5193d5: 'positive-vertical-custom', 0x5194b6: 'positive-vertical-owner',
               0x51959d: 'negative-vertical-custom', 0x519686: 'negative-vertical-owner',
               0x518ea8: 'full-window-message', 0x518ee7: 'boundary-message'}
    coverage = dict.fromkeys(markers.values(), 0)
    state, machines = {}, []
    def words(uc, address, count=1): return struct.unpack('<' + 'I' * count, uc.mem_read(address, 4 * count))
    def put(uc, offset, value): uc.mem_write(SELF + offset, struct.pack('<I', value & 0xffffffff))
    def mutate(uc, message=False):
        mode = state['mode']
        if message:
            if mode & 8: put(uc, 0x5c, OWNERS[1])
            if mode & 1: put(uc, 0x68, words(uc, SELF + 0x68)[0] ^ 0xe)
            return
        if state['mutated']: return
        state['mutated'] = True
        if mode & 1:
            put(uc, 0x74, words(uc, SELF + 0x74)[0] + 2)
            put(uc, 0x78, words(uc, SELF + 0x78)[0] + 3)
        if mode & 2:
            put(uc, 0x8c, CELLS_B if words(uc, SELF + 0x8c)[0] == CELLS_A else CELLS_A)
        if mode & 4: put(uc, 0x64 if state['axis'] else 0x60, 1)
        if mode & 8:
            put(uc, 0xc0, CUSTOMS[1]); put(uc, 0xc4, ARENA + 0x904); put(uc, 0x5c, OWNERS[1])
        if mode & 16:
            offset = 0x60 if state['axis'] else 0x64
            previous = words(uc, SELF + offset)[0]
            put(uc, offset, 2 if previous == 1 else previous - 1)
        if mode & 32:
            address = words(uc, SELF + 0x8c)[0]
            uc.mem_write(address + 0x20, struct.pack('<I', 0x12345678))
        if mode & 64:
            put(uc, 0x6c, words(uc, SELF + 0x6c)[0] + 1)
            put(uc, 0x70, words(uc, SELF + 0x70)[0] + 2)
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(0x400000, (len(image) + 4095) & ~4095); uc.mem_write(0x400000, image)
        uc.mem_map(ARENA, 0x10000); uc.mem_map(STOP, 0x1000)
        uc.mem_map(linked['address'], linked['allocation'])
        for address, data in linked['blocks']: uc.mem_write(address, data)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'], 16), int(f['address'], 16) + f['size']) for f in manifest['functions'].values()]
        helpers = {entries[n] if rebuilt else int(manifest['functions'][n]['address'], 16): n for n in entries}
        def hook(uc, address, size, user, ranges=ranges, helpers=helpers, original=not rebuilt):
            if original and address in markers: coverage[markers[address]] += 1
            sp = uc.reg_read(xc.UC_X86_REG_ESP); this = uc.reg_read(xc.UC_X86_REG_ECX)
            if address in helpers:
                name = helpers[address]; arguments = words(uc, sp + 4, ARGUMENTS[name])
                state['trace'].append((name, None if name in ('WrapIndex', 'FngHash') else this, arguments))
                if name == 'ShiftAxis': state['axis'] = arguments[5] & 255
            if address not in MESSAGES + REFILLS + CUSTOMS:
                assert any(a <= address < b for a, b in ranges), ('unexpected execution', hex(address)); return
            if address in MESSAGES:
                ret, *arguments = words(uc, sp, 5); cleanup = 16
                assert this in OWNERS
                state['trace'].append(('message', address, this, arguments, bytes(uc.mem_read(ARENA, 0x9000))))
                mutate(uc, True)
            elif address in REFILLS:
                ret, grid, x, y = words(uc, sp, 4); cleanup = 12
                assert this in OWNERS and grid == SELF
                state['trace'].append(('owner-refill', address, this, x, y, bytes(uc.mem_read(ARENA, 0x9000))))
                mutate(uc)
            else:
                ret, context, grid, x, y = words(uc, sp, 5); cleanup = 0
                assert grid == SELF
                state['trace'].append(('custom-refill', address, context, x, y, bytes(uc.mem_read(ARENA, 0x9000))))
                mutate(uc)
            uc.reg_write(xc.UC_X86_REG_EAX, 0xa5a500ff); uc.reg_write(xc.UC_X86_REG_ECX, 0xcccccccc); uc.reg_write(xc.UC_X86_REG_EDX, 0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP, sp + 4 + cleanup); uc.reg_write(xc.UC_X86_REG_EIP, ret)
        uc.hook_add(UC_HOOK_CODE, hook); machines.append(uc)
    counts = dict.fromkeys(entries, 0)
    def compare(name, memory, arguments, mode, trial, this=SELF):
        outputs = []
        for rebuilt, uc in zip((False, True), machines):
            uc.mem_write(ARENA, bytes(memory)); uc.mem_write(STACK, struct.pack('<I', STOP) + struct.pack('<' + 'I' * len(arguments), *[x & 0xffffffff for x in arguments]))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            for i, register in enumerate(PRESERVED): uc.reg_write(register, 0x12345000 + i)
            state.clear(); state.update(trace=[], mode=mode, mutated=False, axis=0)
            address = entries[name] if rebuilt else int(manifest['functions'][name]['address'], 16)
            try:
                uc.emu_start(address, STOP, count=100000)
            except UcError as error:
                if not rebuilt: raise
                raise AssertionError(('state or call order mismatch', trial, name, 'rebuilt execution fault', str(error))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP, (trial, name, 'nontermination')
            assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + (0 if name in ('WrapIndex', 'FngHash') else 4 * len(arguments))
            assert all(uc.reg_read(register) == 0x12345000 + i for i, register in enumerate(PRESERVED))
            result = uc.reg_read(xc.UC_X86_REG_EAX)
            if name in ('Navigate', 'InitialFill'): result = None
            elif name not in ('WrapIndex', 'FngHash'): result &= 255
            outputs.append((result, bytes(uc.mem_read(ARENA, 0x9000)), state['trace']))
        if outputs[0] != outputs[1]:
            differences = [(hex(i + ARENA), a, b) for i, (a, b) in enumerate(zip(outputs[0][1], outputs[1][1])) if a != b]
            trace_a = [x[:-1] if isinstance(x[-1], bytes) else x for x in outputs[0][2]]
            trace_b = [x[:-1] if isinstance(x[-1], bytes) else x for x in outputs[1][2]]
            raise AssertionError(('state or call order mismatch', trial, name, mode, differences[:25], outputs[0][0], outputs[1][0], trace_a, trace_b))
        counts[name] += 1
    rng = random.Random(0x518f20)
    deltas = (-1, 0, 1, 2, -3, 0x80000000, 0x7fffffff)
    for trial in range(4096):
        memory = bytearray(rng.randbytes(0x10000))
        def field(offset, value): struct.pack_into('<I', memory, SELF - ARENA + offset, value & 0xffffffff)
        width, height = 1 + trial % 4, 1 + (trial // 4) % 4
        tx, ty = width + 1 + (trial // 16) % 4, height + 1 + (trial // 64) % 4
        pattern = (trial // 256) % 4
        if pattern == 1: tx, ty = width, height
        elif pattern == 2: tx, ty = max(1, width - 1), max(1, height - 1)
        elif pattern == 3: tx, ty = 1, 1
        flags = (trial // 4) % 16
        mode = (trial // 16) % 128
        interface = (trial // 7) % 3
        owner = 0 if interface == 2 and not (flags & 2) and (not (flags & 4) or (tx > width and ty > height)) else OWNERS[0]
        field(0x5c, owner); field(0x60, width); field(0x64, height); field(0x68, flags)
        field(0x6c, tx); field(0x70, ty)
        for offset, total in ((0x74, tx), (0x78, ty), (0x7c, tx), (0x80, ty)): field(offset, rng.randrange(total))
        field(0x8c, CELLS_A); field(0xc0, CUSTOMS[0] if interface == 0 else 0); field(0xc4, ARENA + 0x900)
        for owner_, vtable, message, refill in zip(OWNERS, VTABLES, MESSAGES, REFILLS):
            struct.pack_into('<I', memory, owner_ - ARENA, vtable)
            struct.pack_into('<I', memory, vtable - ARENA + 0x18, message)
            struct.pack_into('<I', memory, vtable - ARENA + 0x4c, refill)
        text = bytes(rng.randrange(1, 256) for _ in range(trial % 97)) + b'\0'
        memory[STRING - ARENA:STRING - ARENA + len(text)] = text
        delta = deltas[(trial // 3) % len(deltas)]
        horizontal = (0, 1, 2, 255, 256)[trial % 5]
        selection = SELF + 0x74 if horizontal & 255 else SELF + 0x78
        offset = selection if trial % 11 == 0 else (SELF + 0x7c if horizontal & 255 else SELF + 0x80)
        total, visible = (tx, width) if horizontal & 255 else (ty, height)
        wrap_values = (-12, -6, -3, -2, -1, 0, 1, 2, 3, 6, 0x80000000, 0x7fffffff)
        compare('WrapIndex', memory, [wrap_values[trial % len(wrap_values)], 1 + (trial // len(wrap_values)) % 7], 0, trial)
        compare('FngHash', memory, [0 if trial % 9 == 0 else STRING], 0, trial)
        sv, ov = struct.unpack_from('<I', memory, selection - ARENA)[0], struct.unpack_from('<I', memory, offset - ARENA)[0]
        compare('CanShift', memory, [delta, sv, ov, total, visible], mode, trial)
        if owner and trial % 17 == 0:
            special = bytearray(memory)
            struct.pack_into('<I', special, SELF - ARENA + 0x68, flags | 4)
            compare('CanShift', special, [delta, sv, ov, visible, visible], mode, trial)
        compare('MoveSelection', memory, [delta, selection, offset, total, visible], 0, trial)
        compare('ShiftAxis', memory, [delta, selection, offset, total, visible, horizontal], mode, trial)
        compare('Navigate', memory, [delta, deltas[(trial // 13) % len(deltas)]], mode, trial)
        compare('InitialFill', memory, [], mode, trial)
        # Initial coordinates are passed raw, then incremented/wrapped inline;
        # callback changes must not change cached extents or reset column origin.
        fill = bytearray(memory)
        origins = (-12, -6, -3, -2, -1, 0, 1, 2, 0x80000000, 0x7fffffff)
        struct.pack_into('<I', fill, SELF - ARENA + 0x74, origins[trial % len(origins)] & 0xffffffff)
        struct.pack_into('<I', fill, SELF - ARENA + 0x78, origins[(trial // len(origins)) % len(origins)] & 0xffffffff)
        compare('InitialFill', fill, [], mode, trial)
        if trial % 64 == 0:
            for zero in (0x6c, 0x70, 0x64, 0x60):
                empty = bytearray(fill)
                struct.pack_into('<I', empty, SELF - ARENA + zero, 0)
                # No callback may be accessed for an empty dimension.
                struct.pack_into('<I', empty, SELF - ARENA + 0xc0, 0xdeadbeef)
                struct.pack_into('<I', empty, SELF - ARENA + 0x5c, 0xdeadbeef)
                if zero == 0x60:
                    # Prefix ends at an unmapped page: the final zero guard
                    # must return before touching callback/context +c0/+c4.
                    guarded = ARENA + 0xff80
                    empty[0xff80:] = empty[SELF - ARENA:SELF - ARENA + 0x80]
                    compare('InitialFill', empty, [], mode, trial, this=guarded)
                else:
                    compare('InitialFill', empty, [], mode, trial)
            # Unsigned min comparisons: large logical counts stay bounded by
            # small physical dimensions; use nonnegative origins/divisors.
            large = bytearray(memory)
            for offset in (0x6c, 0x70): struct.pack_into('<I', large, SELF - ARENA + offset, 0x80000001)
            for offset in (0x74, 0x78): struct.pack_into('<I', large, SELF - ARENA + offset, 0)
            compare('InitialFill', large, [], 0, trial)
        if trial % 64 == 0:
            compare('ShiftAxis', memory, [0, 0, 0, 0, 0, 0], mode, trial)
            special = bytearray(memory)
            for offset in (0x68, 0x74, 0x7c): struct.pack_into('<I', special, SELF - ARENA + offset, 0)
            compare('ShiftAxis', special, [-1, SELF + 0x74, SELF + 0x7c, 0, 0, 1], mode, trial)
    assert all(coverage.values()), coverage
    report = {'compiler': args.compiler, 'comparisons': counts, 'total': sum(counts.values()), 'original_block_hits': coverage,
              'actual_callees': ['name hash', 'index wrap', 'both gates', 'full axis shift', 'initial grid fill'],
              'controlled_interfaces': ['owner +0x18 messages', 'owner +0x4c refill', 'cdecl refill callback'],
              'limitations': ['finite mapped physical grids/logical totals, initial-fill empty guards and selected signed/unsigned extrema', 'no exception/unwind or arbitrary pointer aliases',
                              'callback snapshots/final bytes rather than byte matching or per-DWORD store order', 'native grid reader/owner binding not proved', 'MSVC and byte matching pending'],
              'fingerprints': manifest}
    if args.mutation_checks:
        source = args.source.read_text(); hash_source = args.hash_source.read_text()
        report['rejected_mutations'] = []
        start = '    if (!delta) return 0;'
        mutations = {
            'fill-normalize-first-column': [('int firstColumn = (int)value74;', 'int firstColumn = (int)value74 % value6C;')],
            'fill-live-column-origin': [('int column = firstColumn;', 'int column = (int)value74;')],
            'fill-live-row-origin': [('int column = firstColumn;', 'row = (int)value78; int column = firstColumn;')],
            'fill-live-column-extent': [('unsigned int remaining = columns;', 'unsigned int remaining = count60;')],
            'fill-live-row-extent': [('} while (--rows);', '} while (--rows && rows < count64);')],
            'fill-extra-wrap-call': [('if (next >= 0) column = next % total;', 'if (next >= 0) column = FEngGridWrapIndex(next, total);')],
            'fill-cache-owner': [('void FEGridObject::InitialFill() {', 'void FEGridObject::InitialFill() { FEGridOwner *fillOwner = owner5C;'), ('owner5C->RefillGrid(this, column, row);', 'fillOwner->RefillGrid(this, column, row);')],
            'fill-cache-custom': [('void FEGridObject::InitialFill() {', 'void FEGridObject::InitialFill() { FEGridRefill fillCallback = callbackC0; void *fillContext = contextC4;'), ('callbackC0(contextC4, this, column, row);', 'fillCallback(fillContext, this, column, row);')],
            'fill-ignore-logical-row-min': [('if (logicalRows < rows) rows = logicalRows;', 'if (false) rows = logicalRows;')],
            'fill-ignore-empty-column': [('if (!physicalColumns) return;', 'if (false) return;')],
            'normalize-negative-wrap': [('return (int)((unsigned int)total - (unsigned int)(magnitude % total));', 'return (int)((unsigned int)total - (unsigned int)(magnitude % total)) % total;')],
            'wrong-singleton-negative-wrap': [('if (total > 1) {', 'if (total >= 1) {')],
            'omit-full-window-message': [('owner5C->GameMessage(FngHash((const char *)0x79b14c), this, 0xff, 0);', 'owner5C->GameMessage(FngHash((const char *)0x79b140), this, 0xff, 0);')],
            'rejects-return-success': [('            return 0;', '            return 1;')],
            'wrong-gate-flags': [('if (!(flags & 2)) return 1;', 'if (!(flags & 4)) return 1;')],
            'wrong-gate-limit': [('(unsigned int)total - (unsigned int)visible', '(unsigned int)total + (unsigned int)visible')],
            'ignore-move-both': [('if (flags & 8)', 'if (false)')],
            'require-both-window-flags': [('if (!(flags & 4) || !(flags & 2))', 'if (!(flags & 4) && !(flags & 2))')],
            'wrong-negative-move-comparison': [('if (delta < 0)', 'if (delta <= 0)')],
            'ignore-visibility-distance': [('if (visible <= distance)', 'if (false)')],
            'drop-zero-delta-guard': [(start, '    if (false) return 0;')],
            'horizontal-one-only': [('if (horizontal)', 'if (horizontal == 1)')],
            'wrong-saved-payload-word': [('value0[8]', 'value0[9]')],
            'wrong-positive-copy-neighbor': [('cells8C[index] = cells8C[index + 1];', 'cells8C[index] = cells8C[index - 1];')],
            'wrong-negative-copy-neighbor': [('cells8C[index] = cells8C[index - 1];', 'cells8C[index] = cells8C[index + 1];')],
            'fresh-positive-horizontal-copy-count': [('while (i < copies)', 'while (i < count60 - 1)')],
            'fresh-positive-vertical-copy-count': [('if (rows != 1)', 'if (count64 != 1)'), ('while (i < rows - 1)', 'while (i < count64 - 1)')],
            'cache-horizontal-loop-height': [(start, '    unsigned int rowsSnapshot = count64;\n' + start), ('row < count64', 'row < rowsSnapshot')],
            'cache-vertical-loop-width': [(start, '    unsigned int columnsSnapshot = count60;\n' + start), ('column < count60', 'column < columnsSnapshot')],
            'cache-refill-pointer-context': [(start, '    FEGridRefill refillSnapshot = callbackC0; void *contextSnapshot = contextC4;\n' + start), ('callbackC0(contextC4, this,', 'refillSnapshot(contextSnapshot, this,')],
            'cache-refill-owner': [(start, '    FEGridOwner *ownerSnapshot = owner5C;\n' + start), ('owner5C->RefillGrid(', 'ownerSnapshot->RefillGrid(')],
            'cache-cell-buffer': [(start, '    FEGridCell *cellsSnapshot = cells8C;\n' + start), ('cells8C[', 'cellsSnapshot['), ('(unsigned char *)cells8C', '(unsigned char *)cellsSnapshot')],
            'extra-inline-wrap-boundary': [('else y %= rows;', 'else y = FEngGridWrapIndex(y, rows);')],
            'wrong-negative-vertical-owner-coordinate': [('owner5C->RefillGrid(this, (int)value74, FEngGridWrapIndex((int)(value78 + i), value70));', 'owner5C->RefillGrid(this, (int)(value74 + column), FEngGridWrapIndex((int)(value78 + i), value70));')],
            'cache-second-axis-inputs': [('    ShiftAxis(x, &value74, &value7C, value6C, count60, 1);', '    int rowsSnapshot = value70; unsigned int countSnapshot = count64;\n    ShiftAxis(x, &value74, &value7C, value6C, count60, 1);'), ('ShiftAxis(y, &value78, &value80, value70, count64, 0);', 'ShiftAxis(y, &value78, &value80, rowsSnapshot, countSnapshot, 0);')],
        }
        hash_mutations = {
            'hash-initial-zero': [('unsigned int hash = 0xffffffff;', 'unsigned int hash = 0;')],
            'hash-null-zero': [('    if (name) {', '    if (!name) return 0;\n    if (name) {')],
            'hash-case-sensitive': [("if (c > '`' && c < '{')", 'if (false)')],
            'hash-signed-byte': [('hash = hash * 33 + c;', 'hash = hash * 33 + (signed char)c;')],
            'hash-multiplier-31': [('hash * 33', 'hash * 31')],
        }
        for target, variants, original_source in (('grid', mutations, source), ('hash', hash_mutations, hash_source)):
            for name, edits in variants.items():
                # Older navigation mutations belong to the original six-function
                # unit, not the new fill body with similar callback expressions.
                prefix = ''
                mutated = original_source
                if target == 'grid' and not name.startswith('fill-'):
                    split = original_source.index('// Original 0x504790.')
                    prefix, mutated = original_source[:split], original_source[split:]
                for before, after in edits:
                    assert before in mutated, name
                    mutated = mutated.replace(before, after)
                path = BUILD / ('mutation-' + name + '.cpp'); path.write_text(prefix + mutated)
                command = [os.sys.executable, str(Path(__file__).resolve()), '--compiler', args.compiler,
                           '--source', str(path if target == 'grid' else args.source),
                           '--hash-source', str(path if target == 'hash' else args.hash_source)]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
                (BUILD / ('mutation-' + name + '.log')).write_text(result.stdout + result.stderr)
                assert result.returncode != 0 and 'state or call order mismatch' in result.stderr, name
                report['rejected_mutations'].append(name)
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps(report, indent=2))


if __name__ == '__main__': main()
