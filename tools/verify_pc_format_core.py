#!/usr/bin/env python3
"""Execute original/rebuilt bWare format state machine, sink and recursive wrappers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG)
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/bware-format-core'
ARENA, STACK, STOP = 0x2000000, 0x200f000, 0x2100000
CONTEXT, BUFFER, FORMAT, ARGS, TEXT, VECTOR, COUNT_N = [ARENA + n for n in (0x100,0x200,0x1000,0x2000,0x3000,0x4000,0x5000)]
RING, INDEX = 0x828178, 0x828514
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
GLOBALS = {'bFormatParserTable786CF0':0x786cf0, 'bFormatNegativeInf7EB3D8':0x7eb3d8,
 'bFormatPositiveInf7EB3DC':0x7eb3dc, 'bFormatNaN7EB3E0':0x7eb3e0, 'bFormatNull7EB3E8':0x7eb3e8,
 'bFormatBadPointer7EB3EC':0x7eb3ec, 'bFormatDecimal7EB3F0':0x7eb3f0, 'bFormatSeparator7EB3F1':0x7eb3f1,
 'bFormatGrouping7EB3F2':0x7eb3f2, 'bFormatBadVector786DF4':0x786df4, 'bFormatVector2786E0C':0x786e0c,
 'bFormatVector3786E1C':0x786e1c, 'bFormatInvalidVector786E30':0x786e30, 'bFormatRing828178':RING,
 'bFormatRingIndex828514':INDEX, 'bFormatValue828150':0x828150, 'bFormatShift8284E4':0x8284e4,
 'bFormatScale828028':0x828028, 'bFormatTime828518':0x828518, 'bFormatCount82851C':0x82851c,
 'bFormatBadReadPointer':STOP+0x100, 'bFormatBadWritePointer':STOP+0x200, 'bFormatQueryCounter':STOP+0x300}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',choices=('msvc','clang-cl'),default='msvc')
    parser.add_argument('--source',type=Path,default=ROOT/'src/Speed/Indep/Libs/bWare/FormattingCore.cpp')
    parser.add_argument('--mutation-checks',action='store_true')
    args=parser.parse_args()
    manifest=json.loads((ROOT/'config/SPEED2_EXE_1_2/bware-format-core.json').read_text())
    raw=EXE.read_bytes();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();BUILD.mkdir(parents=True,exist_ok=True)
    for record in list(manifest['functions'].values())+list(manifest['tables'].values()):
        offset=int(record['address'],16)-0x400000
        assert hashlib.sha256(image[offset:offset+record['size']]).hexdigest()==record['sha256']
    objects=[]
    for source in (args.source,ROOT/'src/Speed/Indep/Libs/bWare/Formatting.cpp',ROOT/'src/Speed/Indep/Libs/bWare/FormattingSink.cpp'):
        obj=BUILD/(source.stem+'.obj')
        compiler=([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe'] if args.compiler=='msvc' else
                  ['clang-cl','--target=i686-pc-windows-msvc','/GS-','/GR-','/w','/arch:IA32','/clang:-fno-builtin','/clang:-fno-strict-aliasing'])
        subprocess.run(compiler+['/nologo','/c','/O2','/Ob0','/Fo'+str(obj.relative_to(ROOT)),
                                '/Isrc/Speed/Indep/Libs/bWare',os.path.relpath(source,ROOT)],cwd=ROOT,check=True)
        objects.append(obj)
    external={'__allshr':0x75c600,'__ftol2':0x75bed4,'__ftol':0x75bed4,'__fltused':ARENA+0x7800}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section']==0:
                for name,address in GLOBALS.items():
                    if symbol['name'].startswith('?'+name+'@'):external[symbol['name']]=address
    linked=link_objects(objects,external)
    entry=next(a for n,a in linked['exports'].items() if n.startswith('?bFormatCore@'))
    sink_entry=next(a for n,a in linked['exports'].items() if n.startswith('?bFormatWrite@'))
    state,machines={},[]
    def words(uc,address,count=1):return struct.unpack('<'+'I'*count,uc.mem_read(address,count*4))
    for rebuilt in (False,True):
        uc=Uc(UC_ARCH_X86,UC_MODE_32);uc.mem_map(0x400000,(len(image)+4095)&~4095);uc.mem_write(0x400000,image)
        for module in pe.DIRECTORY_ENTRY_IMPORT:
            for imp in module.imports:
                name=imp.name.decode() if imp.name else ''
                address={'IsBadReadPtr':STOP+0x100,'IsBadWritePtr':STOP+0x200,'QueryPerformanceCounter':STOP+0x300}.get(name)
                if address:uc.mem_write(imp.address,struct.pack('<I',address))
        uc.mem_map(ARENA,0x10000);uc.mem_map(STOP,0x1000);uc.mem_map(linked['address'],linked['allocation'])
        for address,body in linked['blocks']:uc.mem_write(address,body)
        ranges=(linked['ranges'] if rebuilt else [(0x43e950,0x43f5da),(0x43cb40,0x43cbc2),(0x43bd60,0x43bda0),
                  (0x4400d0,0x440100),(0x43fbc0,0x43fc35),(0x43e5c0,0x43e616)])+[(0x75bed4,0x75bf80),(0x75c600,0x75c621)]
        sink=sink_entry if rebuilt else 0x43cb40
        def hook(uc,address,size,user,ranges=ranges,sink=sink):
            sp=uc.reg_read(UC_X86_REG_ESP)
            if address==sink:
                _,ctx,text,length,count=words(uc,sp,5)
                state['trace'].append(('write',length,bytes(uc.mem_read(text,length)) if length else b''))
            if address in (STOP+0x100,STOP+0x200):
                ret,pointer,length=words(uc,sp,3);assert length==1
                result=1 if pointer in (0,0x0bad0001) else (state['policy']!=0 if address==STOP+0x100 else state['policy']==2)
                state['trace'].append(('read' if address==STOP+0x100 else 'write-probe',pointer,int(result)))
                cleanup=8
            elif address==STOP+0x300:
                ret,pointer=words(uc,sp,2);tick=1000000+state['clock'];state['clock']+=1
                uc.mem_write(pointer,struct.pack('<Q',tick));result=1;cleanup=4
                state['trace'].append(('counter',tick))
            else:
                assert any(a<=address<b for a,b in ranges),('unexpected execution',hex(address))
                return
            uc.reg_write(UC_X86_REG_EAX,int(result));uc.reg_write(UC_X86_REG_ECX,0xcccccccc);uc.reg_write(UC_X86_REG_EDX,0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP,sp+4+cleanup);uc.reg_write(UC_X86_REG_EIP,ret)
        def read(uc,access,address,size,value,user):
            if TEXT<=address<TEXT+7:state['trace'].append(('string-read',address-TEXT,size))
        uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_READ,read);machines.append(uc)
    rng=random.Random(0x43e950)
    def double(value):return list(struct.unpack('<II',struct.pack('<d',value)))
    cases=[]
    for f in (b'literal',b'%q',b'%%',b'trailing%',b'%s',b'%.0s',b'%.3s',b'%8.3s',b'%-8s',b'%c',b'abc%n!'):
        cases.append((f,[TEXT if f.endswith(b's') else COUNT_N if b'%n' in f else 65]))
    for f in (b'%d',b'%+06d',b'% d',b'%-6d!',b'%05d',b'%u',b'%#x',b'%#X',b'%#o',b'%#.0x',b'%p',b'%hd',b'%hu',b'%lu',b'%b',b'%$u',b'%$#x',b'%*.*d'):
        for v in (0,1,7,10,0x7fffffff,0x80000000,0xffffffff):cases.append((f,([8,3,v] if f==b'%*.*d' else [v])))
    for f in (b'%z',b'%.0z',b'%.1z',b'%.3z',b'%+012.4z'):
        for v in (0,1,0x8000,0x18000,0x7fffffff,0x80000000,0xffff8000):cases.append((f,[v]))
    for f in (b'%f',b'%.0f',b'%#.0f',b'%.2f',b'%+012.4f',b'%e',b'%E',b'%g',b'%G',b'%$f'):
        for v in (0.0,-0.0,0.125,1.25,1.999,9.999,123.5,-1234.5,1e-8,1e10,float('inf'),float('-inf'),float('nan')):cases.append((f,double(v)))
    for f in (b'%v2',b'%v3',b'A%v2B%n',b'%5.1v3',b'%v9'):
        cases.append((f,[VECTOR,COUNT_N]))
    cases.extend([(b'%s',[0]),(b'%s',[0x0bad0001]),(b'%v2',[0]),(b'%v3',[0x0bad0001])])
    for bits in (0x7ff0000000000001,0xfff0000000000001,0x7ff8000000001234,0xfff8000000001234):
        cases.append((b'%f',[bits&0xffffffff,bits>>32]))
    for value in (9.999,99.999,0.99999):cases.append((b'%.2e',double(value)))
    cases.extend([(b'A%nBC',[CONTEXT+4]),(b'%nZZZZtail',[FORMAT+2]),(b'abc%n%$u',[0x7eb3f0,123456])])
    for _ in range(768):
        flags=rng.choice((b'',b'#',b'+',b' ',b'-',b'0',b'$'))
        width=str(rng.randrange(0,18)).encode();precision=str(rng.randrange(0,7)).encode()
        kind=rng.choice((b'd',b'u',b'x',b'X',b'o',b'b',b'z',b'f',b'e',b'g',b's'))
        f=b'L%'+flags+width+b'.'+precision+kind+b'R'
        value=double(rng.choice((-1.0,1.0))*rng.randrange(1,1000000)/1000.0*10.0**rng.randrange(-8,9)) if kind in (b'f',b'e',b'g') else [TEXT] if kind==b's' else [rng.getrandbits(32)]
        cases.append((f,value))
    count=0
    for trial,(format_,arguments) in enumerate(cases):
        memory=bytearray(rng.randbytes(0x10000))
        struct.pack_into('<III',memory,CONTEXT-ARENA,BUFFER,0,0x12345678)
        memory[FORMAT-ARENA:FORMAT-ARENA+len(format_)+1]=format_+b'\0';memory[TEXT-ARENA:TEXT-ARENA+7]=b'abcdef\0'
        struct.pack_into('<'+'I'*len(arguments),memory,ARGS-ARENA,*arguments)
        struct.pack_into('<3f',memory,VECTOR-ARENA,1.25,-2.5,3.75)
        for variant in range(3):
            for precision in (0x27f,0x37f):
                outputs=[]
                for rebuilt,uc in zip((False,True),machines):
                    uc.mem_write(ARENA,bytes(memory));uc.mem_write(RING,bytes(128));uc.mem_write(INDEX,struct.pack('<III',trial%128,0,0))
                    uc.mem_write(CONTEXT,struct.pack('<III',0 if variant==1 else BUFFER,0 if variant<2 else (1,2,128,255)[trial%4],0x12345678))
                    uc.mem_write(0x828150,b'\x77');uc.mem_write(0x7eb3f0,image[0x3eb3f0:0x3eb3f4] if trial<321 else bytes((46,95,(0,1,3,255)[trial%4],0)))
                    uc.mem_write(0x8284e4,bytes(4));uc.mem_write(0x828028,struct.pack('<f',0.001))
                    uc.mem_write(STACK,struct.pack('<4I',STOP,CONTEXT,FORMAT,ARGS));uc.reg_write(UC_X86_REG_ESP,STACK)
                    uc.reg_write(UC_X86_REG_FPCW,precision);uc.reg_write(UC_X86_REG_FPSW,0);uc.reg_write(UC_X86_REG_FPTAG,0xffff)
                    for i,r in enumerate(PRESERVED):uc.reg_write(r,0x12345000+i)
                    state.clear();state.update(trace=[],clock=0,policy=trial%3)
                    uc.emu_start(entry if rebuilt else 0x43e950,STOP,count=500000)
                    assert uc.reg_read(UC_X86_REG_EIP)==STOP,(trial,rebuilt,format_,hex(uc.reg_read(UC_X86_REG_EIP)))
                    assert uc.reg_read(UC_X86_REG_ESP)==STACK+4
                    assert all(uc.reg_read(r)==0x12345000+i for i,r in enumerate(PRESERVED))
                    assert uc.reg_read(UC_X86_REG_FPCW)==precision and uc.reg_read(UC_X86_REG_FPSW)&0x3800==0
                    outputs.append((uc.reg_read(UC_X86_REG_EAX),bytes(uc.mem_read(ARENA,0x8000)),bytes(uc.mem_read(RING,128)),
                                    bytes(uc.mem_read(INDEX,12)),bytes(uc.mem_read(0x828150,1)),state['trace'],bytes(uc.mem_read(0x7eb3f0,4))))
                assert outputs[0]==outputs[1],('state or call order mismatch',trial,format_,arguments,hex(precision),
                    outputs[0][0],outputs[1][0],outputs[0][1][BUFFER-ARENA:BUFFER-ARENA+100],outputs[1][1][BUFFER-ARENA:BUFFER-ARENA+100],
                    outputs[0][5],outputs[1][5])
                count+=1
    report={'compiler':args.compiler,'comparisons':count,'fingerprints':manifest,'conversion_fixtures':len(cases),'sink_modes':['memory','count-only','ring'],'x87_control_words':['0x027f','0x037f'],'controlled_interfaces':['IsBadReadPtr','IsBadWritePtr','QueryPerformanceCounter'],
            'limitations': [
                'finite bounded conversions and mapped buffers; arbitrary widths, precisions and aliases unproved',
                'unsupported conversion-class paths with unset/prior locals are excluded',
                'original parser/string/locale data remain image bindings',
                'compiler-runtime float conversion and signed shift execute reference helpers',
                'Win32 probes and clock are controlled; native adapters are separate platform seams',
                'FP exception flags, legacy MSVC compilation and byte matching remain unverified']}
    if args.mutation_checks:
        source=args.source.read_text()
        mutations={
            'wrong-default-float-precision':('if (precision == 0xffffffff) precision = 3;', 'if (precision == 0xffffffff) precision = 6;'),
            'omit-alternate-flag':("case '#': flags |= 0x80; break;", "case '#': break;"),
            'count-vector-payload':('if (context->destination0) --context->destination0;', 'count += ignored;\n                        if (context->destination0) --context->destination0;'),
            'skip-invalid-vector-suffix':('if (!bFormatCheckPointer(vector)) text = (char *)bFormatBadVector786DF4;',
                                          'if (!bFormatCheckPointer(vector)) { text = (char *)bFormatBadVector786DF4; ++format; }'),
            'pointer-check-or':('bFormatBadReadPointer(pointer, 1) && bFormatBadWritePointer(pointer, 1)',
                                 'bFormatBadReadPointer(pointer, 1) || bFormatBadWritePointer(pointer, 1)'),
            'omit-string-extra-read':('do { char value = *p++; if (!value) break; if (!remaining--) break; } while (true);\n                    size = (unsigned int)(p - text - 1);',
                                      'while (remaining && *p) { ++p; --remaining; }\n                    size = (unsigned int)(p - text);'),
            'omit-octal-prefix':("case 'o': radix = 8; if (flags & 0x80) flags |= 0x200; break;", "case 'o': radix = 8; break;"),
            'skip-zero-fixed-rounding':('if (digit - 1 >= 5) {', 'if (requested && digit - 1 >= 5) {'),
        }
        report['rejected_mutations']=[]
        for name,(before,after) in mutations.items():
            assert before in source
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(source.replace(before,after))
            result=subprocess.run([os.sys.executable,str(Path(__file__).resolve()),'--compiler',args.compiler,'--source',str(path)],cwd=ROOT,capture_output=True,text=True)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode!=0 and 'state or call order mismatch' in result.stderr,name
            report['rejected_mutations'].append(name)
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
