#!/usr/bin/env python3
"""Original/rebuilt platform-solid loading and primitive relocation checks.

GPU calls are typed boundaries. Compare their arguments/order, complete retained
chunk bytes, intrusive registration lists, GPU buffer bytes and return values.
--retail additionally executes the player's real GEOMETRY.BIN platform chunks.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE = ROOT / 'src/Speed/Indep/Src/World/CarRenderSolidLoad.cpp'
BUILD = ROOT / 'build/analysis/car-render-solid-load'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-solid-load.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x200000, 0x2300000, 0x2310000
TABLE, MODE, DEVICE, VTABLE, BUFFER_TABLE = [ARENA + o for o in (0, 0x100, 0x200, 0x300, 0x400)]
OBJECTS, TECHNIQUES, PLATFORM, PRIMITIVES = [ARENA + o for o in (0x500, 0x600, 0x1000, 0x2000)]
HEAD, RULES, TECHNIQUE_TABLE = 0x872ed0, 0x800464, 0x86eb28
ORIGINAL = dict(index=0x5bc6b0, fixup=0x5c58a0, load=0x5cc280)
PREFIX = dict(index='?CarRenderTechniqueIndex@@', fixup='?FixupPrimitives@CarRenderPlatformSolid@@',
              load='?LoadPlatformChunk@CarRenderSlotTable@@')
GCC = dict(index='_Z23CarRenderTechniqueIndexj', fixup='_ZN22CarRenderPlatformSolid15FixupPrimitivesEj',
           load='_ZN18CarRenderSlotTable17LoadPlatformChunkEPj')
GLOBALS = dict(ug2SolidRules800464=RULES, ug2SolidTechniques86EB28=TECHNIQUE_TABLE,
               ug2PlatformSolids872ED0=HEAD, ug2SolidLoadMode8900D8=0x8900d8, ug2SolidDevice870974=0x870974)
CALLS = dict(vertex=STOP+0x100, index=STOP+0x110, lock=STOP+0x120, unlock=STOP+0x130)
BINDINGS = dict(UG2SolidCreateVertexBuffer=CALLS['vertex'], UG2SolidCreateIndexBuffer=CALLS['index'],
                UG2SolidLockBuffer=CALLS['lock'], UG2SolidUnlockBuffer=CALLS['unlock'])


def build(source, gcc):
    BUILD.mkdir(parents=True, exist_ok=True)
    if gcc:
        linked = link_native(source, BUILD, ROOT, dict(GLOBALS, **BINDINGS), GCC['index'])
    else:
        obj = BUILD / 'load.obj'
        subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                        '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                        '/Fo' + str(obj.relative_to(ROOT)), str(source.relative_to(ROOT))], cwd=ROOT, check=True)
        linked = link_objects([obj], {'_' + k: v for k, v in dict(GLOBALS, **BINDINGS).items()}, start=0x2400000)
    linked['entries'] = {kind: next(v for name, v in linked['exports'].items() if name.startswith(prefix))
                         for kind, prefix in (GCC if gcc else PREFIX).items()}
    return linked


class Machine:
    def __init__(self, image, linked=None, gcc=False):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image)+4095)&~4095)
        u.mem_write(0x400000, image)
        u.mem_map(ARENA, SIZE)
        u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for address, body in linked['blocks']: u.mem_write(address, body)
        self.entries = linked['entries'] if linked else ORIGINAL
        self.rebuilt, self.gcc = bool(linked), gcc
        for kind, address in CALLS.items():
            u.hook_add(UC_HOOK_CODE, self.gpu, user_data=kind, begin=address, end=address)

    def gpu(self, u, address, size, kind):
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        count = 7 if kind in ('vertex', 'index') else 5 if kind == 'lock' else 1
        args = struct.unpack('<'+'I'*count, bytes(u.mem_read(sp+4, count*4)))
        if kind in ('vertex', 'index'):
            device, length, usage, fmt, pool, output, shared = args
            assert device == DEVICE and pool == 1 and shared == 0
            assert (usage, fmt) == ((8, 0) if kind == 'vertex' else (0, 0x65))
            assert length <= 0x60000
            token = OBJECTS + len(self.buffers)*4
            u.mem_write(token, struct.pack('<I', BUFFER_TABLE))
            target = self.next_buffer
            self.next_buffer = (target+length+0x11f)&~255
            assert self.next_buffer < ARENA+SIZE
            # Only a final vertex payload is used for forward-overlap fixtures;
            # its writes cannot alter a subsequent child header.
            alias = self.case.get('alias', 0) if kind == 'vertex' else 0
            if alias:
                target = self.case['vertex_payload'] + (4 if alias == 1 else 0)
            else:
                u.mem_write(target-16, b'\xa5'*(length+32))
            self.buffers[token] = dict(kind=kind, target=target, length=length, alias=alias)
            u.mem_write(output, struct.pack('<I', token))
            self.events.append((kind, args[:5], output, shared, token))
        elif kind == 'lock':
            token, offset, length, output, flags = args
            assert (offset, length, flags) == (0, 0, 0)
            u.mem_write(output, struct.pack('<I', self.buffers[token]['target']))
            self.events.append((kind, token, offset, length, flags))
        else:
            token, = args
            buffer = self.buffers[token]
            self.events.append((kind, token, bytes(u.mem_read(buffer['target'], buffer['length']))))
        ret = struct.unpack('<I', bytes(u.mem_read(sp, 4)))[0]
        # Typed adapters are cdecl; the original D3D methods are stdcall.
        u.reg_write(xc.UC_X86_REG_ESP, sp+4+(0 if self.rebuilt else count*4))
        u.reg_write(xc.UC_X86_REG_EAX, self.case.get('hresult', 0))
        u.reg_write(xc.UC_X86_REG_EIP, ret)

    def run(self, kind, case):
        u = self.u
        self.case, self.events, self.buffers = case, [], {}
        self.next_buffer = ARENA + 0x40000
        u.mem_write(ARENA, case['data'])
        for address, body in case['globals'].items(): u.mem_write(address, body)
        u.mem_write(VTABLE+0x68, struct.pack('<2I', CALLS['vertex'], CALLS['index']))
        u.mem_write(BUFFER_TABLE+0x2c, struct.pack('<2I', CALLS['lock'], CALLS['unlock']))
        u.mem_write(DEVICE, struct.pack('<I', VTABLE))
        receiver, args = case['calls'][kind]
        actual = [receiver]+args if self.gcc and receiver else args
        sp = STACK + 0x8000
        u.mem_write(sp, struct.pack('<'+'I'*(len(actual)+1), STOP, *actual))
        u.reg_write(xc.UC_X86_REG_ESP, sp)
        u.reg_write(xc.UC_X86_REG_ECX, receiver)
        u.reg_write(xc.UC_X86_REG_EFLAGS, 2)  # Original REP copy requires DF clear.
        saved = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
        for i, reg in enumerate(saved): u.reg_write(reg, 0xabcdef00+i)
        u.reg_write(xc.UC_X86_REG_FPCW, 0x037f)
        u.reg_write(xc.UC_X86_REG_FPSW, 0)
        u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        u.emu_start(self.entries[kind], STOP, count=2000000)
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp+4+(len(args)*4 if receiver and not self.gcc else 0)
        assert [u.reg_read(reg) for reg in saved] == [0xabcdef00+i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == 0x037f and u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        buffers = [bytes(u.mem_read(b['target']-16, b['length']+32)) for b in self.buffers.values()]
        globals_ = [bytes(u.mem_read(a, len(body))) for a, body in sorted(case['globals'].items())]
        return bytes(u.mem_read(ARENA, len(case['data']))), globals_, buffers, u.reg_read(xc.UC_X86_REG_EAX), self.events


def base_case(rng, index):
    data = bytearray(rng.randbytes(0x10000))
    def put(address, fmt, *values): struct.pack_into('<'+fmt, data, address-ARENA, *values)
    put(MODE, 'I', index%6)
    globals_ = {HEAD: struct.pack('<2I', HEAD, HEAD), 0x8900d8: struct.pack('<I', 0 if index%7 == 0 else MODE),
                0x870974: struct.pack('<I', DEVICE),
                TECHNIQUE_TABLE-4: struct.pack('<24I', *(TECHNIQUES+i*0x10 for i in range(24))),
                RULES: struct.pack('<12I', 1,4,0, 0x40,2,0, 0x80,5,0, 0,0,0)}
    flags = rng.getrandbits(32) if index%2 else index%256
    # Synthetic rule changes cover early zero-mask termination and no match;
    # retail checks below always use the unmodified image rule table.
    if index%13 == 0:
        globals_[RULES] = struct.pack('<12I', 1,4,0, 2,2,0, 4,5,0, 8,0,0)
        flags = 0
    elif index%17 == 0:
        globals_[RULES] = struct.pack('<12I', 0,7,0, 0x40,2,0, 0x80,5,0, 0,0,0)
    put(PLATFORM+0xc, 'IiII', flags, rng.choice((-2147483648,-1,0,1,2,32)), PRIMITIVES,
        rng.choice((0,0xffffffff,0x7fffffff,ARENA+0x5000)))
    for i in range(32): put(PRIMITIVES+i*0x3c+0x34, 'I', rng.choice((0,1,0x7fffffff,0xffffffff,rng.getrandbits(32))))
    return dict(data=data, globals=globals_, calls={'index':(0,[flags]), 'fixup':(PLATFORM,[rng.getrandbits(32)])})


def append_chunk(data, offset, kind, payload, alignment=1):
    begin = (offset+8+alignment-1)&~(alignment-1)
    end = begin+len(payload)
    struct.pack_into('<2I', data, offset, kind, end-offset-8)
    data[begin:end] = payload
    return end, ARENA+begin


def synthetic_cases(n):
    rng = random.Random(0x5cc280)
    for index in range(n):
        case = base_case(rng, index)
        data = case['data']
        start = 0x8000 + (index%32)*4
        current = start+8
        if index%19 == 0:
            # Empty and unknown-only containers do not dereference platform00.
            if index%38: current, _ = append_chunk(data, current, 0x12345678, rng.randbytes(12))
        else:
            header = bytearray(rng.randbytes(0x44))
            flags = rng.choice((0,1,0x40,0x80,0xc1,rng.getrandbits(32)))
            amount = rng.choice((-3,0,1,2,16))
            vertex_count = rng.choice((0,1,6,0xffffffff))
            struct.pack_into('<Ii', header, 0xc, flags, amount)
            struct.pack_into('<I', header, 0x34, vertex_count)
            current, _ = append_chunk(data, current, 0x134900, header, 16)
            current, _ = append_chunk(data, current, 0x22222222, rng.randbytes((index%4)*4))
            primitives = rng.randbytes(max(amount,1)*0x3c)
            current, _ = append_chunk(data, current, 0x134b02, primitives, 16)
            # Index payloads exercise zero length and both word-copy remainders.
            current, _ = append_chunk(data, current, 0x134b03, rng.randbytes((index%17)*2), 16)
            if index%3:
                current, vertex = append_chunk(data, current, 0x134b01, rng.randbytes(index%137), 128)
                case['vertex_payload'] = vertex
                case['alias'] = 1 if index%11 == 0 else 2 if index%13 == 0 else 0
            case['hresult'] = 0 if index%5 else 0x8876086c
        struct.pack_into('<2I', data, start, 0x80134100, current-start-8)
        case['calls']['load'] = TABLE, [ARENA+start]
        case['data'] = bytes(data)
        yield case


def retail_cases(path):
    raw = path.read_bytes()
    def walk(begin, end):
        while begin+8 <= end:
            kind, size = struct.unpack_from('<2I', raw, begin)
            limit = begin+8+size
            assert limit <= end
            if kind == 0x80134100: yield begin, limit
            elif kind & 0x80000000: yield from walk(begin+8, limit)
            begin = limit
        assert begin == end
    rng = random.Random(0x134900)
    for index, (begin, end) in enumerate(walk(0,len(raw))):
        case = base_case(rng, index)
        # Preserve the retail file's absolute 128-byte alignment residue.
        start = 0x1000+(begin%128)
        size = start+end-begin+128
        assert size < 0x40000
        data = bytearray(rng.randbytes(max(size,0x10000)))
        data[:0x1000] = case['data'][:0x1000]
        data[start:start+end-begin] = raw[begin:end]
        case['data'] = bytes(data)
        case['globals'][RULES] = struct.pack('<12I', 1,4,0, 0x40,2,0, 0x80,5,0, 0,0,0)
        case['calls'] = {'load': (TABLE,[ARENA+start])}
        case['retail_offset'] = hex(begin)
        yield case


def verify(image, linked, gcc, cases, kinds):
    a, b = Machine(image), Machine(image, linked, gcc)
    totals, events = Counter(), Counter()
    for index, case in enumerate(cases):
        for kind in kinds:
            try: x,y = a.run(kind,case),b.run(kind,case)
            except UcError as error:
                raise AssertionError(('solid load mismatch',kind,index,case.get('retail_offset'),str(error))) from error
            if x != y:
                offsets = [(hex(i),u,v)for i,(u,v)in enumerate(zip(x[0],y[0]))if u!=v][:8]
                event = [(i,u,v)for i,(u,v)in enumerate(zip(x[4],y[4]))if u!=v][:1]
                raise AssertionError(('solid load mismatch',kind,index,case.get('retail_offset'),offsets,x[3],y[3],event))
            totals[kind]+=1
            events.update(e[0] for e in x[4])
    return dict(comparisons=dict(totals),gpu_calls=dict(events))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc',action='store_true')
    p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--retail',type=Path)
    p.add_argument('--kind',choices=['all']+list(ORIGINAL),default='all')
    p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args()
    assert args.cases > 0
    raw=EXE.read_bytes(); image=pefile.PE(data=raw).get_memory_mapped_image()
    manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000
        assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc)
    kinds=list(ORIGINAL)if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,synthetic_cases(args.cases),kinds)
    if args.retail:
        report['retail'] = dict(path=args.retail.name,sha256=hashlib.sha256(args.retail.read_bytes()).hexdigest(),
                                **verify(image,linked,args.gcc,retail_cases(args.retail),['load']))
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',
        byte_identity='not established',limitations=['GPU creation/lock/unlock use controlled typed boundaries, not real D3D',
            'native activation, full pack manager, texture/technique objects and GPU consumer remain open',
            'finite valid child-order fixtures; malformed chunk repair and allocation failure are not claimed'])
    if args.mutation_checks:
        mutations={
            'last-rule-wins':('index','return rule->technique04;','return ug2SolidRules800464[3].technique04;'),
            'ignore-zero-mask':('index',' || rule->mask00 == 0',''),
            'wrong-index-scale':('fixup','primitives14[i].value34 * 2','primitives14[i].value34 * 4'),
            'wrong-relocation-sign':('fixup',' * 2 - offset +',' * 2 + offset +'),
            'skip-technique-store':('fixup','technique3C = ug2SolidTechniques86EB28[static_cast<int>(index)];','(void)0;'),
            'unsigned-primitive-count':('fixup','i < primitiveCount10','(unsigned int)i < (unsigned int)primitiveCount10'),
            'wrong-header-alignment':('load','AlignedPayload(child, 16)','AlignedPayload(child, 8)'),
            'wrong-vertex-alignment':('load','AlignedPayload(child, 128)','AlignedPayload(child, 16)'),
            'skip-index-fixup':('load','current->FixupPrimitives(0);','(void)0;'),
            'wrong-mode-gate':('load','*ug2SolidLoadMode8900D8 == 4','*ug2SolidLoadMode8900D8 == 3'),
            'skip-vertex-zero-gate':('load','current->vertexCount34 != 0','true'),
            'drop-copy-remainder':('load','bytes &= 3;','bytes = 0;'),
            'reverse-word-copy':('load','*reinterpret_cast<unsigned int *>(output) = *reinterpret_cast<const unsigned int *>(input);',
                '*reinterpret_cast<unsigned int *>(output) = *reinterpret_cast<const unsigned int *>(input + words * 4);'),
            'omit-registration-tail':('load','ug2PlatformSolids872ED0.previous04 = &current->link00;','(void)0;'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','256']
            if args.gcc:cmd.append('--gcc')
            result=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'solid load mismatch'in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    label=('gcc'if args.gcc else'msvc')+'-'+args.kind
    (BUILD/('verification-'+label+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
