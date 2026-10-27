#!/usr/bin/env python3
"""Verify effect setup and recursive reference adjustment/resolution against UG2.

Allocation and the three unrecovered preparation/evaluation routines are controlled.
All setup, recursive references, buffer access and allocator/free order execute.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
import verify_pc_effect_input as core

ROOT,EXE=core.ROOT,core.EXE
LIB=ROOT/'src/Speed/Indep/Libs/D3DX'
SOURCE=LIB/'EffectSetup.cpp';BUILD=ROOT/'build/analysis/effect-setup'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-setup.json'
STOP,ARENA,HEAP=core.STOP,core.ARENA,core.HEAP
EFFECT=ARENA+0x1000
SPANS=dict(setup=(0x66afe6,1082),adjust=(0x65ae25,236),resolve=(0x66aa2f,488))
MEMBERS={k:('D3DXEffectObject',v)for k,v in dict(setup='Setup',adjust='AdjustReferences',resolve='ResolveReferences').items()}
core.MEMBERS.update(MEMBERS)
BOUNDARIES=dict(prepareA=(0x666df7,1,'UG2FxPrepareGroupA'),prepareB=(0x667d51,1,'UG2FxPrepareGroupB'),evaluate=(0x663a32,2,'UG2FxEvaluateRecord'))
SHIMS={k:STOP+0x7000+i*16 for i,k in enumerate(BOUNDARIES)}


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[source,LIB/'EffectObject.cpp',LIB/'EffectPool.cpp',LIB/'EffectDestruction.cpp',core.SOURCE,core.BRIDGE,
        ROOT/'src/Platform/Native/NativeFxPoolDispatch.cpp',ROOT/'src/Platform/Native/NativeFxEffectDispatch.cpp',
        ROOT/'src/Platform/Native/NativeFxDestroyDispatch.cpp',ROOT/'src/Platform/Native/NativeFxComDispatch.cpp']
    bindings=dict(core.GLOBALS,**core.APIS,ug2FxPoolIid7B7410=0x7b7410,ug2FxIndexBias7A1E58=0x7a1e58,
        UG2FxMalloc=0x575620,UG2FxFree=0x575630,abort=STOP+0x900)
    bindings.update({name:SHIMS[k]for k,(a,n,name)in BOUNDARIES.items()})
    if gcc:
        linked=link_native(sources,BUILD,ROOT,bindings,'_ZN16D3DXEffectObject5SetupEv',text_address=0x5000000,include_dirs=[LIB])
        prefixes={k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()}
    else:
        objects=[]
        for i,p in enumerate(sources):
            obj=BUILD/('setup-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/I'+str(LIB.relative_to(ROOT)),'/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()};bindings.update(__fltused=0,__ftol2=0x75dd28)
        for name,n,_ in core.API_SPECS:bindings['_UG2Fx'+name+'@'+str(n*4)]=core.APIS['UG2Fx'+name]
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:'?'+m+'@'+c+'@@'for k,(c,m)in MEMBERS.items()}
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for name,a in linked['exports'].items()if name.startswith(p)]
        assert len(hits)==1,(k,p,hits);linked['entries'][k]=hits[0]
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.rebuilt=bool(linked)
        if not linked:self.entries={k:a for k,(a,n)in SPANS.items()}
        for k,(a,n,name)in BOUNDARIES.items():
            address=SHIMS[k]if linked else a
            self.u.hook_add(UC_HOOK_CODE,self.boundary,user_data=k,begin=address,end=address)
    def call(self,kind,args=(),receiver=0):
        u=self.u;params=([receiver]+list(args))if self.gcc else list(args)
        sp=core.STACK+0xf000;self.put(sp,STOP,*params)
        regs=[xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP]
        seeds=[0x11223344,0x22334455,0x33445566,0x44556677]
        for r,v in zip(regs,seeds):u.reg_write(r,v)
        for r,v in [(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,0x202),
            (xc.UC_X86_REG_FPCW,self.control_word),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+(0 if self.gcc else len(args)*4),('stack',kind)
        assert [u.reg_read(r)for r in regs]==seeds,('registers',kind)
        assert u.reg_read(xc.UC_X86_REG_FPCW)==self.control_word and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        assert self.word(0)==0x12345678
        return u.reg_read(xc.UC_X86_REG_EAX)
    def half(self,a):return struct.unpack('<H',self.u.mem_read(a,2))[0]
    def put_half(self,a,v):self.u.mem_write(a,struct.pack('<H',v&0xffff))
    def state(self):return bytes(self.u.mem_read(ARENA,0x2000))+bytes(self.u.mem_read(HEAP,self.allocated))
    def boundary(self,u,address,size,kind):
        n=BOUNDARIES[kind][1]
        if self.rebuilt:obj,*a=self.args(n+1);pop=0
        else:obj=u.reg_read(xc.UC_X86_REG_ECX);a=self.args(n);pop=n*4
        self.events.append((kind,obj,tuple(a),self.state()))
        if self.case['callback']:
            group=self.word(obj+0x2c)+a[0]*0x64
            # Mutation is limited to an unknown word and a later classification
            # flag; owned pointers/ring links remain valid during callbacks.
            self.put(group+0x10,self.case['mark'])
            self.put(group+8,self.word(group+8)^8)
        self.ret(self.case[kind],pop)
    def alloc(self,n):
        a=HEAP+self.allocated;self.allocated+=max(16,(n+15)&~15)
        assert self.allocated<core.HSIZE;self.live[a]=n;return a
    def fill(self,c):
        self.u.mem_write(EFFECT,c['effect'])
        groupCount=7;groups=self.alloc(groupCount*0x64);self.put(EFFECT+0x2c,groups)
        self.put(EFFECT+0x118,c['flag']);self.put(EFFECT+0x40,c['word40'])
        self.put(EFFECT+0xa0,c['rows']);self.put(EFFECT+0xac,c['parameters'])
        self.put(EFFECT+0xb4,0,c['scratch'],0,0)
        techniques=self.alloc(max(4,c['rows']*0xb4));self.put(EFFECT+0xa4,techniques)
        for row in range(c['rows']):
            count=c['passes'];passes=self.alloc(max(4,count*0x38));self.put(techniques+row*0xb4+0x34,count,passes)
            for j in range(count):self.put_half(passes+j*0x38+0x34,(row+j)%2)
        for g in range(groupCount):
            group=groups+g*0x64;data=self.alloc(0x100);records=self.alloc(2*0x64)
            self.put(group,data,records,c['group_flags'][g]);self.put(group+0x14,0)
            selectors=c['selectors']if g<2 else(0xffff,0xffff)
            self.put_half(group+0x18,selectors[0]);self.put_half(group+0x1a,selectors[1]);self.put_half(group+0x34,c['records']if g<2 else 0)
            self.put_half(group+0x3a,c['all_counts'][g]);self.put_half(group+0x3c,c['direct_counts'][g])
            for r in range(2):
                record=records+r*0x64
                self.put_half(data+r*2,r)
                self.put_half(data+0x40+r*4,(3+r*2)if g<2 and c['linked']else 0)
                self.put_half(data+0x42+r*4,c['selected'])
                metadata=self.alloc(0x14);self.put(metadata+0x10,c['elements'])
                self.put(record,c['dirty'],records,0,metadata)
                self.put(record+0x48,0x40+r*4);self.put(record+0x58,(0x0a000000,0x0b000000)[r]if c['special_types']else 0x01020304)
                buffer=self.alloc(16);description=self.alloc(16);self.put(buffer,0x7b711c,1,16,description)
                self.put(description+0xc,0xc0+r*8);self.put(record+0x38,buffer)
                self.u.mem_write(data+0xc0+r*8,struct.pack('<d',c['index_value']))
        table=self.alloc(max(4,c['parameters']*4));self.put(EFFECT+0xb0,table)
        for p in range(c['parameters']):
            parameter=self.alloc(0x50);self.put(table+p*4,parameter)
            count=c['uses'];nodes=[self.alloc(0x20)for _ in range(count)]
            self.put(parameter+0x34,nodes[0]if nodes else 0)
            for j,node in enumerate(nodes):
                self.put_half(node+0x14,(p+j)%groupCount)
                self.put(node+0x18,nodes[j-1],nodes[(j+1)%count])
    def run(self,kind,c):
        self.prepare(c);self.fill(dict(c,selected=0)if kind=='adjust'and c['selected']>=max(1,c['elements'])else c)
        if kind=='adjust':a=[c['group'],c['direct_delta'],c['all_delta']]
        elif kind=='resolve':a=[c['group'],c['direct']]
        else:a=[]
        return self.snapshot(self.call(kind,a,EFFECT))


def cases(n):
    rng=random.Random(0x66afe6)
    counters=(0,1,2,0x7fff,0x8000,0xfffe,0xffff)
    deltas=(0,1,-1,0x7fff,0x8000,0xffff,0x10000,0x10001,0x80000000,0xffffffff)
    values=(-1.001,-1.0,-0.001,0.0,0.998,0.999,1.0,1.999,2.0,65534.999,65535.0,65536.0,65536.999,4294967296.0,281474976710656.0)
    for i,c in enumerate(core.cases(n)):
        c.update(effect=rng.randbytes(0x130),flag=rng.getrandbits(32),word40=rng.getrandbits(32),scratch=rng.getrandbits(32),
            rows=(0,1,2,3)[i%4],parameters=(0,1,2,5)[(i//4)%4],passes=(0,1,2,3)[(i//16)%4],uses=(0,1,2,5,9,17)[(i//3)%6],
            group_flags=[rng.getrandbits(32)for _ in range(7)],all_counts=[counters[(i+j)%7]for j in range(7)],
            direct_counts=[counters[(i//7+j)%7]for j in range(7)],selectors=((0xffff,0xffff),(0,0xffff),(0xffff,1),(0,1))[i%4],
            records=(0,1,2)[(i//2)%3],elements=(0,1,2)[(i//6)%3],linked=bool((i//18)&1),dirty=0 if(i//36)%3==0 else 1,
            selected=(0,1,2,0xffff)[(i//9)%4],index_value=values[i%len(values)],special_types=bool((i//5)&1),
            prepareA=(0,7,0x80004005)[(i//11)%3],prepareB=(0,9,0x8007000e)[(i//33)%3],evaluate=(0,3,0x8876086c)[(i//17)%3],
            group=i%2,direct_delta=deltas[i%len(deltas)],all_delta=deltas[(i//len(deltas))%len(deltas)],direct=(0,1,-1)[i%3],
            callback=i%5==0,mark=rng.getrandbits(32))
        # Counts up to two and child groups 2..5 keep recursive graphs acyclic.
        c['selected']=(i//9)%max(1,c['elements'])
        if i%23==0:c.update(selectors=(0xffff,0xffff),selected=0xffff,index_value=65535.0,dirty=1)
        # Broad failure injection spans initial grids, list growth and outputs.
        c['fail_allocation']=0 if i%3 else 1+(i//3)%28
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();profiles=Counter()
    fixtures=[(i,c,cw)for i,c in enumerate(cases(n))for cw in(0x27f,0x37f)]
    for i,c,cw in fixtures:
        a.control_word=b.control_word=cw
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect setup mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:3],w[:3])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect setup mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'event counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[hex(x[2])]+=1
            profiles.update([('x87_control_word',hex(cw)),('rows',c['rows']),('parameters',c['parameters']),('uses',c['uses']),('failure',c['fail_allocation']),('linked',c['linked'])])
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),coverage={str(k):v for k,v in profiles.items()})


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        offset=int(f['address'],16)-0x400000;assert hashlib.sha256(image[offset:offset+f['size']]).hexdigest()==f['sha256']
    report=verify(image,build(args.replace or SOURCE,args.gcc),args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free and shader preparation A/B and record evaluation are controlled boundaries',
        'finite bounded valid acyclic group graphs and circular use rings; unknown bytes and partial OOM state compared',
        'floating selector conversions use finite signed-64-bit-range values; exceptions and NaN/out-of-range conversion not established',
        'real effect deserialization, shader preparation/evaluation and GPU/native draw activation remain open'])
    if args.mutation_checks:
        mutations={
            'drop-byte-flag':('setup','[1]|=8;','[1]|=0;'),
            'retain-setup-flag':('setup','[1]&=0xf7;','[1]&=0xff;'),
            'wide-row-sentinel':('setup','*reinterpret_cast<unsigned short *>(&value40)=0xffff;','value40=0xffffffff;'),
            'clear-row-on-initial-oom':('setup','goto finish;','*reinterpret_cast<unsigned short *>(&value40)=0xffff; goto finish;'),
            'wrong-byte-allocation':('setup','UG2FxMalloc(valueAC)','UG2FxMalloc(valueAC*4)'),
            'zero-grid-before-populating':('setup','unsigned int row=*reinterpret_cast<unsigned short *>(&value40);','for (unsigned int z=0;z<valueA0*valueAC;++z) reinterpret_cast<unsigned int *>(valueB4)[z]=0; unsigned int row=*reinterpret_cast<unsigned short *>(&value40);'),
            'wrong-row-stride':('setup','valueB4+valueAC*row*4','valueB4+row*4'),
            'omit-increment-before-resolve':('setup','record->value34),0,1);','record->value34),0,0);'),
            'skip-resolution':('setup','ResolveReferences(static_cast<unsigned short>(record->value34),0);','(void)0;'),
            'reverse-classification':('setup','!(group->value08&8)','(group->value08&8)'),
            'keep-inactive-use':('setup','if (group->value3A)','if (true)'),
            'wrong-growth':('setup','firstCapacity*2+1','firstCapacity+1'),
            'omit-list-copy':('setup','grown[i]=first[i];','grown[i]=0;'),
            'omit-output-zero-tail':('setup','output[i]=0;','(void)0;'),
            'omit-second-terminator':('setup','reinterpret_cast<unsigned int>(*slot)+bytes)=0;','reinterpret_cast<unsigned int>(*slot)+bytes)=1;'),
            'omit-row-decrement':('setup','record->value34),0,-1);','record->value34),0,0);'),
            'wrong-temp-free-order':('setup','UG2FxFree(first);UG2FxFree(second);','UG2FxFree(second);UG2FxFree(first);'),
            'signed-counter':('adjust','static_cast<int>(p->value3C)+direct>=0','static_cast<short>(p->value3C)+direct>=0'),
            'full-width-delta':('adjust','short direct=static_cast<short>(directDelta),all=static_cast<short>(allDelta);','int direct=directDelta; short all=static_cast<short>(allDelta);'),
            'wrap-negative-counter':('adjust','static_cast<int>(p->value3A)+all>=0','true'),
            'omit-element-default':('adjust','(count ? count : 1)','count'),
            'wrong-selected-reference':('adjust','(reference[1]-1)+','(reference[1])+'),
            'omit-all-recursion':('adjust','if (all) AdjustReferences','if (false) AdjustReferences'),
            'wrong-prepare-tag':('resolve','==0x0a000000','==0x0b000000'),
            'ignore-prepare-failure':('resolve','result=UG2FxPrepareGroupA(this,group); if (result<0) return result;','result=UG2FxPrepareGroupA(this,group);'),
            'omit-dirty-clear':('resolve','current->value00=current->value04;','(void)0;'),
            'wrong-index-bias':('resolve','+ug2FxIndexBias7A1E58','+0.0'),
            'strict-index-equality':('resolve','if (count<=index)','if (count<index)'),
            'validate-unchanged-index':('resolve','if (reference[1]!=index)','if (true)'),
            'ignore-evaluate-failure':('resolve','result=UG2FxEvaluateRecord(this,group,indices[i]); if (result<0) return result;','result=UG2FxEvaluateRecord(this,group,indices[i]);'),
            'omit-resolve-default':('resolve','if (!count) count=1;','(void)0;'),
        }
        rejected=[];kinds=set(SPANS)if args.kind=='all'else{args.kind}
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect setup mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
