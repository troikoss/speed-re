#!/usr/bin/env python3
"""Execute the original world-object routines PromoteWorldObject 0x5b31f0, BreakWorldObject 0x5ae000,
the child activation 0x5adf90 and the placement seed accessor 0x611000 and the MSVC 7.1 (/Ox /Ob0)
rebuild from CollisionWorldObject.cpp in Unicorn on random world objects, and compare the complete
call log, EAX, the callee-saved registers, the stack pointer and the whole fake-world memory.

What is real and what is hooked.  The four routines above, and the hash 0x43db80 (called as the
original on both sides; it is recovered and verified as HudNameHash in verify_pc_hud_minimap.py, so
it is not re-verified here), run as machine code.  The unrecovered callees are seams, replaced
identically on both sides by `ret` stubs that record their arguments (thiscall registers on the
original side, the cdecl hand-off on the rebuilt side, normalised to the same tuple) and return
scripted values: 0x5b1390 (promotion), 0x5adb20 (child attach), 0x5add50 (child update), 0x48bc50
(child registration), the creation function behind the pointer at 0x86af90 and the world object's
deleting destructor (vtable slot 0).  Their own behaviour is therefore NOT covered.  Behavioural
evidence only; nothing here claims byte matching.  --mutation-checks recompiles deliberately broken
sources and requires each to be rejected."""
import argparse
import hashlib
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EBX,
                               UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from probe_pc_race_reward import ROOT, EXE
from pc_coff import link_objects, read_object

BUILD = ROOT/'build/analysis/collision-world-object'
SOURCE = ROOT/'src/Speed/Indep/Src/Physics/Collision/CollisionWorldObject.cpp'
ROUTINES = {'Promote': (0x5b31f0, 0x5b3210), 'Break': (0x5ae000, 0x5ae0e0), 'Activate': (0x5adf90, 0x5adfff),
            'Seed': (0x611000, 0x611007)}
SYMBOLS = {'Promote': '?PromoteWorldObject@@YAPAVCollisionBody@@PAV1@@Z',
           'Break': '?BreakWorldObject@@YAHPAVCollisionBody@@PAPAV1@@Z',
           'Activate': '?WorldObjectActivate5ADF90@@YAXPAVCollisionBody@@@Z',
           'Seed': '?GetSeed611000@WorldObjectPlacement@@QAEIXZ'}
HASH = (0x43db80, 0x43dba1)
# Seams: original address, patched stub (`ret n`), rebuilt stub address, name.
ORIGINAL_SEAMS = {0x5b1390: ('promote', b'\xc3'), 0x5adb20: ('attach', b'\xc2\x0c\x00'), 0x5add50: ('update', b'\xc3'),
                  0x48bc50: ('register', b'\xc2\x04\x00')}
S_PROMOTE, S_ATTACH, S_UPDATE, S_REGISTER, S_CREATE, S_DELETE = (0x2102000+0x10*i for i in range(6))
REBUILT_SEAMS = {S_PROMOTE: ('promote', b'\xc3'), S_ATTACH: ('attach', b'\xc3'), S_UPDATE: ('update', b'\xc3'),
                 S_REGISTER: ('register', b'\xc3')}
COMMON_SEAMS = {S_CREATE: ('create', b'\xc3'), S_DELETE: ('delete', b'\xc2\x04\x00')}
PREFIXES = {'?HudNameHash@@YAIPBDI@Z': 0x43db80,
            '?PromoteWorldObject5B1390@@': S_PROMOTE, '?WorldObjectAttach5ADB20@@': S_ATTACH,
            '?WorldObjectUpdate5ADD50@@': S_UPDATE, '?WorldObjectRegister48BC50@@': S_REGISTER,
            '?worldObjectCreate86AF90@@': 0x86af90, '?worldObject8357E0@@': 0x8357e0,
            '?worldObjectActivations86B378@@': 0x86b378, '?collisionClock@@': 0x86518c,
            '?worldObjectChildName7A1EEC@@': 0x7a1eec, '?collisionIds86B2BC@@': 0x86b2bc, '__fltused': 0}
BODY, DESC, SOURCE_OBJ, PARENT, IDS, OUT = 0x2000000, 0x2000800, 0x2000900, 0x2000a00, 0x2000b00, 0x2000c00
CHILD, NODES, REGION = 0x2001000, 0x2006000, 0x8000
WORLD, STACK, STOP, WRAPPER = 0x8357e0, 0x200f000, 0x2100000, 0x2101000
VTABLE = 0x2000d00
GLOBAL_COUNTER, GLOBAL_CLOCK, GLOBAL_IDS, GLOBAL_CREATE = 0x86b378, 0x86518c, 0x86b2bc, 0x86af90


def build(source, name):
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD/(name+'.obj')
    command = [str(ROOT/'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo', '/c', '/Ox', '/Ob0', '/Z7', '/EHsc',
               '/I', 'src/Speed/Indep/Src/Physics/Collision', '/Fo'+str(obj.relative_to(ROOT)), str(Path(source).resolve().relative_to(ROOT))]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        print(result.stdout, result.stderr)
        raise SystemExit('compile failed')
    symbols = read_object(obj)['symbols'].values()
    externals = {}
    for symbol in symbols:
        if symbol['section'] == 0 and symbol['storage'] == 2:
            for prefix, address in PREFIXES.items():
                if symbol['name'] == prefix or (prefix.endswith('@@') and symbol['name'].startswith(prefix)):
                    externals[symbol['name']] = address
                    break
            else:
                raise SystemExit('unmapped external '+symbol['name'])
    linked = link_objects([obj], externals)
    sizes = {s['name']: s['size'] for s in symbols if s['size']}
    return linked, sizes


def wrapper_parts(name, rebuilt):
    """Probe-only wrapper around the call: (prefix, suffix).  The originals are thiscall (ECX = object,
    BreakWorldObject pops its argument); the rebuild takes the object as the first cdecl argument,
    except the placement accessor, a real member function (thiscall)."""
    if name == 'Break':
        if rebuilt:
            return b'\x68'+struct.pack('<I', OUT)+b'\x68'+struct.pack('<I', BODY), b'\x83\xc4\x08'
        return b'\x68'+struct.pack('<I', OUT)+b'\xb9'+struct.pack('<I', BODY), b''
    if name == 'Promote':   # cdecl on both sides; the argument comes from a probe slot (may be 0)
        return b'\xff\x35'+struct.pack('<I', OUT+0x80), b'\x83\xc4\x04'
    if name == 'Activate':
        if rebuilt:
            return b'\x68'+struct.pack('<I', CHILD), b'\x83\xc4\x04'
        return b'\xb9'+struct.pack('<I', CHILD), b''
    return b'\xb9'+struct.pack('<I', BODY+0x480), b''


def make_machine(image, base, linked, rebuilt, entries, coverage, log):
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, (len(image)+4095) & ~4095)
    uc.mem_write(base, image)
    uc.mem_map(0x2000000, 0x10000)
    uc.mem_map(0x2100000, 0x4000)
    if rebuilt:
        uc.mem_map(linked['address'], linked['allocation'])
        for address, body in linked['blocks']:
            uc.mem_write(address, body)
    seams = dict(COMMON_SEAMS)
    if rebuilt:
        seams.update(REBUILT_SEAMS)
    else:
        seams.update({a: (n, c) for a, (n, c) in ORIGINAL_SEAMS.items()})
    for address, (name, code) in seams.items():
        uc.mem_write(address, code)
    wrappers, cursor = {}, WRAPPER
    for name in ROUTINES:
        prefix, tail = wrapper_parts(name, rebuilt)
        call = b'\xe8'+struct.pack('<i', entries[name]-(cursor+len(prefix))-5)
        code = prefix+call+tail+b'\xc3'
        uc.mem_write(cursor, code)
        wrappers[name] = (cursor, len(code))
        cursor += 0x100
    ranges = [HASH]+[(a, a+len(c)) for a, (n, c) in seams.items()]+[(a, a+n) for a, n in wrappers.values()]
    ranges += linked['ranges'] if rebuilt else list(ROUTINES.values())
    state = dict(promoted=0, creates=0)

    def guard(machine, address, size, user):
        if coverage is not None:
            coverage.add(address)
        if not any(start <= address < end for start, end in ranges):
            raise RuntimeError(f'Unexpected instruction at {address:#x}')
        if address in seams:
            name = seams[address][0]
            ecx = machine.reg_read(UC_X86_REG_ECX)
            esp = machine.reg_read(UC_X86_REG_ESP)
            stack = [struct.unpack('<I', bytes(machine.mem_read(esp+4+4*i, 4)))[0] for i in range(4)]
            # Normalised argument tuples: thiscall registers on the original side, cdecl stack on the rebuilt side.
            if name == 'create':
                entry = tuple(stack[:3])
            elif name == 'delete':
                entry = (ecx, stack[0])
            elif rebuilt:
                entry = tuple(stack[:{'promote': 1, 'attach': 4, 'update': 1, 'register': 2}[name]])
            else:
                entry = (ecx,)+tuple(stack[:{'promote': 0, 'attach': 3, 'update': 0, 'register': 1}[name]])
            log.append((name,)+entry)
            if name == 'promote':
                machine.reg_write(UC_X86_REG_EAX, state['promoted'])
            elif name == 'create':
                machine.reg_write(UC_X86_REG_EAX, CHILD+0x800*state['creates'])
                state['creates'] += 1
            else:
                machine.reg_write(UC_X86_REG_EAX, 0x7e57ab1e)
    uc.hook_add(UC_HOOK_CODE, guard)
    return uc, {n: a for n, (a, c) in wrappers.items()}, state


def setup(uc, case):
    uc.mem_write(BODY, case['memory'])
    uc.mem_write(GLOBAL_COUNTER, struct.pack('<I', case['counter']))
    uc.mem_write(GLOBAL_CLOCK, struct.pack('<I', case['clock']))
    uc.mem_write(GLOBAL_IDS, struct.pack('<I', IDS))
    uc.mem_write(GLOBAL_CREATE, struct.pack('<I', S_CREATE))
    uc.mem_write(OUT+0x80, struct.pack('<I', case['promote_arg']))


def run(uc, wrapper, state, log, case):
    setup(uc, case)
    del log[:]
    state['promoted'] = case['promoted']
    state['creates'] = 0
    uc.mem_write(STACK, struct.pack('<I', STOP))
    uc.reg_write(UC_X86_REG_ESP, STACK)
    saved = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
    for reg in saved:
        uc.reg_write(reg, 0x12345678)
    try:
        uc.emu_start(wrapper, STOP, count=20000)
    except UcError as error:
        return ('fault', str(error)+' eip '+hex(uc.reg_read(UC_X86_REG_EIP)))
    if uc.reg_read(UC_X86_REG_EIP) != STOP:
        return ('fault', 'instruction budget')
    assert all(uc.reg_read(reg) == 0x12345678 for reg in saved), 'callee-saved register clobbered'
    assert uc.reg_read(UC_X86_REG_ESP) == STACK+4, 'stack not balanced'
    return ('ok', bytes(uc.mem_read(BODY, REGION)), bytes(uc.mem_read(GLOBAL_COUNTER, 4)), tuple(log),
            uc.reg_read(UC_X86_REG_EAX))


def make_case(rng):
    mem = bytearray(rng.randbytes(REGION))
    count = rng.choice((9, 9, 1, 2, 3, 4, 5, 6, 7, 8, 0, -1, -5))
    next8 = rng.choice((0, 1, 0x1234, 0xffff, 0x10000, 0x12345, -1, rng.randrange(1 << 32)))
    seed = rng.choice((0xffffffff, 0, rng.randrange(1 << 32)))
    struct.pack_into('<I', mem, 0, VTABLE)
    struct.pack_into('<I', mem, 0x148, DESC-BODY+BODY)
    struct.pack_into('<i', mem, 0x8c+DESC-BODY, count)
    struct.pack_into('<I', mem, 0x480+8, SOURCE_OBJ)
    struct.pack_into('<I', mem, SOURCE_OBJ-BODY+8, seed)
    struct.pack_into('<I', mem, 0x480+0x60, PARENT if rng.random() < 0.5 else 0)
    struct.pack_into('<I', mem, VTABLE-BODY, S_DELETE)
    struct.pack_into('<I', mem, IDS-BODY+8, next8 & 0xffffffff)
    for k in range(9):
        base = CHILD-BODY+0x800*k
        mem[base+0x505] = rng.choice((0, 1))
        mem[base+0x506] = rng.choice((0, 1, 2))
        mem[base+0x19] = rng.choice((0, 1, 0x80))
        struct.pack_into('<I', mem, base+0x15c, rng.choice((0, 0x7fffffff, 1, 0x7ffffffe, 0x80000000, rng.randrange(1 << 32))))
        nodes = rng.choice((0, 0, 1, 2, 3))
        head = CHILD+0x800*k+0x4fc
        chain = [NODES+0x100*k+0x20*j for j in range(nodes)]
        struct.pack_into('<II', mem, base+0x4fc, chain[0] if chain else head, rng.randrange(1 << 32))
        for j, node in enumerate(chain):
            struct.pack_into('<II', mem, node-BODY, chain[j+1] if j+1 < len(chain) else head, rng.randrange(1 << 32))
    return dict(memory=bytes(mem), counter=rng.choice((0, rng.randrange(1 << 32), 0x7fffffff)), clock=rng.randrange(1 << 32),
                promoted=rng.choice((0, BODY+0x1000, rng.randrange(1 << 32))), count=count, next8=next8,
                promote_arg=rng.choice((BODY, BODY, 0)))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--cases', type=int, default=3000)
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    pe = pefile.PE(data=EXE.read_bytes())
    image, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    linked, sizes = build(args.source, 'CollisionWorldObject')
    covered, logs = set(), ([], [])
    entries_rebuilt = {name: linked['exports'][symbol] for name, symbol in SYMBOLS.items()}
    original, wrap_original, state_original = make_machine(image, base, linked, False, {n: r[0] for n, r in ROUTINES.items()}, covered, logs[0])
    rebuilt, wrap_rebuilt, state_rebuilt = make_machine(image, base, linked, True, entries_rebuilt, None, logs[1])
    rng = random.Random(0x5ae000)
    failures, counts, both_fault = [], {name: 0 for name in ROUTINES}, 0
    seen = dict(creates=set(), deletes=0, registers=0, parents=set(), skipped_loops=0)
    for trial in range(args.cases):
        case = make_case(rng)
        for name in ROUTINES:
            if name == 'Break' and trial % 5 == 4:
                continue
            if name == 'Promote' and trial % 3:
                continue
            a = run(original, wrap_original[name], state_original, logs[0], case)
            b = run(rebuilt, wrap_rebuilt[name], state_rebuilt, logs[1], case)
            if a[0] == 'fault' and b[0] == 'fault':
                both_fault += 1
                continue
            if a != b:
                failures.append((name, trial, a if a[0] == 'fault' else (a[0], a[3], hex(a[4])),
                                 b if b[0] == 'fault' else (b[0], b[3], hex(b[4]))))
                continue
            counts[name] += 1
            if name == 'Break':
                seen['creates'].add(max(0, case['count']))
                seen['deletes'] += sum(1 for e in a[3] if e[0] == 'delete')
                seen['registers'] += sum(1 for e in a[3] if e[0] == 'register')
                if case['count'] <= 0:
                    seen['skipped_loops'] += 1
                if a[4] != case['count'] & 0xffffffff:
                    failures.append((name, trial, 'returned count differs from the descriptor', a[4]))
                if not any(e[0] == 'delete' for e in a[3]):
                    failures.append((name, trial, 'no deleting destructor call'))
            if name == 'Promote':
                if a[4] != case['promoted']:
                    failures.append((name, trial, 'does not return the seam result', hex(a[4])))
                if (case['promote_arg'] != 0) != any(e[0] == 'delete' for e in a[3]):
                    failures.append((name, trial, 'delete guard'))
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for name, (lo, hi) in ROUTINES.items():
        for insn in md.disasm(image[lo-base:hi-base], lo):
            instructions[insn.address] = insn.mnemonic
    unreached = sorted(a for a in instructions if a not in covered)
    record = {name: dict(original_sha256=hashlib.sha256(image[lo-base:hi-base]).hexdigest(), original_size=hi-lo,
                         rebuilt_size=sizes[SYMBOLS[name]], byte_matching='not claimed')
              for name, (lo, hi) in ROUTINES.items()}
    report = dict(compared=counts, cases=sum(counts.values()), both_fault=both_fault, failures=len(failures),
                  break_counts_seen=sorted(seen['creates']), break_with_no_loop=seen['skipped_loops'],
                  deleting_destructor_calls=seen['deletes'], child_registrations=seen['registers'],
                  instructions_total=len(instructions), instructions_reached=len(instructions)-len(unreached),
                  unreached=[hex(a) for a in unreached], functions=record)
    for name, value in report.items():
        print(name, value)
    for failure in failures[:5]:
        print('FAIL', str(failure)[:600])
    status = 1 if failures else 0
    if args.mutation_checks and not failures:
        mutations = {
            'child-digit': ("(char)(i + '1')", "(char)(i + '0')"),
            'transform-offset': ('parent + 0x50', 'parent + 0x40'),
            'placement-offset': ('bytes + 0x480', 'bytes + 0x484'),
            'stamp-sentinel': ('stamp == 0x7fffffff', 'stamp == 0x7ffffffe'),
            'stamp-zero': ('if (stamp == 0 ||', 'if (stamp == 1 ||'),
            'activation-counter': ('++worldObjectActivations86B378;', '--worldObjectActivations86B378;'),
            'registered-flag': ('bytes[0x505] = 1;', 'bytes[0x505] = 2;'),
            'entry-index': ('+ 0x90 + i * 4', '+ 0x94 + i * 4'),
            'id-truncation': ('(unsigned short)collisionIds86B2BC->next8', '(unsigned short)(collisionIds86B2BC->next8 + 1)'),
            'promote-delete': ('    if (body) {\n        delete body;\n    }', '    if (body && promoted) {\n        delete body;\n    }'),
        }
        text = args.source.read_text()
        rejected = []
        for label, (before, after) in mutations.items():
            assert before in text, label
            mutant = BUILD/('mutation-'+label+'.cpp')
            mutant.write_text(text.replace(before, after, 1))
            result = subprocess.run([sys.executable, __file__, '--source', str(mutant), '--cases', '600'], cwd=ROOT/'tools',
                                    text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if result.returncode == 0:
                print('MUTATION SURVIVED', label)
                status = 1
            else:
                rejected.append(label)
        print('rejected_mutations', len(rejected), 'of', len(mutations), rejected)
    return status


if __name__ == '__main__':
    sys.exit(main())
