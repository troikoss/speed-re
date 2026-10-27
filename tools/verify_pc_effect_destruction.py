#!/usr/bin/env python3
"""Compare linked UG2 effect destruction with retail x86, including nested cleanup.

Only allocation/free and COM Release are controlled. All cleanup bodies execute.
Fixtures retain unknown bytes and test type gates, reverse arrays/cookies, flags,
embedded receivers, linked lists and selected callback-induced field changes.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import random
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
SOURCE=LIB/'EffectDestruction.cpp'
BUILD=ROOT/'build/analysis/effect-destruction'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-destruction.json'
STOP,ARENA,HEAP=core.STOP,core.ARENA,core.HEAP
COM_RELEASE,CALLBACK,VTABLE=STOP+0x6000,STOP+0x6010,ARENA+0x3000
SPANS=dict(array=(0x64b5ff,44),empty=(0x6996f4,1),base=(0x663e90,53),parameter=(0x664efa,72),
    value=(0x663fcf,109),reference08=(0x65ee40,48),reference18=(0x65b7ad,67),record64=(0x663f15,57),
    group64=(0x66a419,41),record38=(0x665031,31),recordB4=(0x66a442,147),secondary=(0x65e4ff,47),
    temporaries=(0x65b7f0,214),effect=(0x66adec,473),base_delete=(0x65b6a8,33),parameter_delete=(0x664f42,33),
    value_delete=(0x66519e,33),link_delete=(0x6424e3,33),base_array=(0x661405,78),reference08_array=(0x66238c,78),
    reference18_array=(0x661453,78),parameter_array=(0x6651bf,78),record64_array=(0x664f63,78),
    group64_array=(0x66a4d5,78),record38_array=(0x665050,78),recordB4_array=(0x66a523,81),record20_array=(0x660f55,71))
FREE=dict(array='D3DXEffectDestroyArray',empty='D3DXDestroyEmpty',base='D3DXDestroyBase',parameter='D3DXDestroyParameter',
    value='D3DXDestroyValue',reference08='D3DXDestroyReference08',reference18='D3DXDestroyReference18',
    record64='D3DXDestroyRecord64',group64='D3DXDestroyGroup64',record38='D3DXDestroyRecord38',
    recordB4='D3DXDestroyRecordB4',secondary='D3DXDestroySecondary',temporaries='D3DXDestroyTemporaries',effect='D3DXDestroyEffect')
TYPES=dict(base=('D3DXEffectBaseRecord',0x28),parameter=('D3DXEffectParameter',0x50),value=('D3DXEffectValuePrefix',0x1c),
    link=('D3DXEffectPoolLink',0x14),reference08=('D3DXEffectReference08',8),reference18=('D3DXEffectReference18',0x18),
    record64=('D3DXEffectRecord64',0x64),group64=('D3DXEffectGroup64',0x64),record38=('D3DXEffectRecord38',0x38),
    recordB4=('D3DXEffectRecordB4',0xb4),record20=('D3DXEffectRecord20',0x20))
MEMBERS={k:(TYPES[k.rsplit('_',1)[0]][0],'DeleteArray'if k.endswith('_array')else'Delete')for k in SPANS if k not in FREE}
core.MEMBERS.update(MEMBERS);core.VOID.update(FREE)


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[source,LIB/'EffectObject.cpp',LIB/'EffectPool.cpp',core.SOURCE,
        ROOT/'src/Platform/Native/NativeFxDestroyDispatch.cpp',ROOT/'src/Platform/Native/NativeFxPoolDispatch.cpp',core.BRIDGE]
    bindings=dict(core.GLOBALS,**core.APIS,ug2FxPoolIid7B7410=0x7b7410,UG2FxMalloc=0x575620,UG2FxFree=0x575630,
        UG2FxComRelease=COM_RELEASE,abort=STOP+0x900)
    if gcc:
        linked=link_native(sources,BUILD,ROOT,bindings,FREE['array'],text_address=0x5000000,include_dirs=[LIB])
        prefixes=dict(FREE,**{k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()})
    else:
        objects=[]
        for i,p in enumerate(sources):
            obj=BUILD/('destruction-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/I'+str(LIB.relative_to(ROOT)),'/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()};bindings['_UG2FxComRelease@4']=COM_RELEASE
        for name,n,_ in core.API_SPECS:bindings['_UG2Fx'+name+'@'+str(n*4)]=core.APIS['UG2Fx'+name]
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:('@'+name+'@4'if k!='array'else'_'+name+'@16')for k,name in FREE.items()}
        prefixes.update({k:'?'+m+'@'+c+'@@'for k,(c,m)in MEMBERS.items()})
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for name,a in linked['exports'].items()if name==p or (k in MEMBERS and name.startswith(p))]
        assert len(hits)==1,(k,p,hits);linked['entries'][k]=hits[0]
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries={k:a for k,(a,n)in SPANS.items()}
        for address,kind in ((COM_RELEASE,'release'),(CALLBACK,'element')):
            self.u.hook_add(UC_HOOK_CODE,self.boundary,user_data=kind,begin=address,end=address)
    def state(self):return bytes(self.u.mem_read(ARENA,0x4000))+bytes(self.u.mem_read(HEAP,self.allocated))
    def boundary(self,u,address,size,kind):
        if kind=='element':
            obj=u.reg_read(xc.UC_X86_REG_ECX);self.events.append(('element',obj,self.state()))
            if self.case['callback']:
                # Change cookie storage: the iterator must retain its initial count.
                self.put(self.root-4,0xffffffff)
            self.ret();return
        obj,=self.args(1)
        self.events.append(('release',obj,self.state()))
        assert self.word(obj)==VTABLE
        self.put(obj+4,(self.word(obj+4)-1)&0xffffffff)
        if self.case['callback'] and self.release_patch:
            target,value=self.release_patch;self.put(target,value);self.release_patch=None
        self.ret(self.word(obj+4),4)
    def runtime(self,u,address,size,kind):
        arg,=self.args(1)
        # Controlled free may update a parent before the original rereads it.
        patch=None
        if kind=='free' and self.case['callback'] and self.free_patch and arg==self.free_patch[0]:
            patch=self.free_patch[1:];self.free_patch=None
        super().runtime(u,address,size,kind)
        if patch:self.put(*patch)
    def alloc(self,size):
        a=HEAP+self.allocated;self.allocated+=max(16,(size+15)&~15)
        assert self.allocated<core.HSIZE
        self.live[a]=size;return a
    def data(self):return self.alloc(1+self.rng.randrange(31))
    def com(self):
        a=self.alloc(8);self.put(a,VTABLE,0x100);return a
    def array(self,kind,count=None,deep=False):
        size=TYPES[kind][1];count=self.case['count']if count is None else count
        actual=count if count<8 else 0
        a=self.alloc(4+size*actual);self.put(a,count)
        for i in range(actual):self.fill(kind,a+4+i*size,deep)
        return a+4
    def base(self,p,deep):
        self.put(p+0x20,0,0)
        if deep:
            self.put(p+0x20,self.array('base',self.case['small'],False))
            tail=0
            for _ in range(self.case['small']):
                a=self.alloc(0x28);self.base(a,False);self.put(a+0x24,tail);tail=a
            self.put(p+0x24,tail)
    def link(self,p):
        sentinel=self.alloc(0x14)
        self.put(p+8,self.data(),sentinel,sentinel);self.put(sentinel+12,p,p)
    def fill(self,kind,p,deep=True):
        c=self.case;self.put(p,c['type'])
        if kind=='base':self.base(p,deep)
        elif kind=='parameter':
            self.base(p,deep);self.put(p+0x3c,0,0)
            if deep:
                tail=0
                for _ in range(c['small']):
                    a=self.alloc(0x50);self.fill('parameter',a,False);self.put(a+0x3c,tail);tail=a
                self.put(p+0x3c,tail)
                a=self.alloc(0x14);self.link(a);self.put(p+0x40,a)
        elif kind=='value':
            self.put(p+0xc,0);self.put(p+0x14,self.array('reference08',c['small'],False)if deep else 0,0)
            # Nonowning kinds retain a valid borrowed pointer, never freed.
            a=self.data();self.put(p+0xc,a)
            if deep:
                tail=0
                for _ in range(c['small']):
                    a=self.alloc(0x1c);self.fill('value',a,False);self.put(a+0x18,tail);tail=a
                self.put(p+0x18,tail)
        elif kind in('reference08','reference18'):
            self.put(p+4,self.com()if c['nonnull']else 0)
            if kind=='reference18':self.put(p+8,self.com()if c['second']else 0)
        elif kind=='record20':pass
        elif kind=='record64':
            # The original explicitly clears +28/+2c before any release. Use
            # poison there to detect omissions; base +0 is deliberately random.
            self.put(p+0x28,0xdead1234,0xdead5678)
            self.put(p+0x34,self.array('record20',c['small'],False),self.com()if c['nonnull']else 0)
        elif kind=='group64':self.put(p,self.data(),self.array('record64',c['small'],False))
        elif kind in('record38','recordB4'):
            self.base(p,deep);param=self.alloc(0x50);self.fill('parameter',param,deep)
            self.put(p+0x30,param)
            if kind=='recordB4':
                self.put(p+0x38,self.array('record38',c['small'],False));self.put(p+0x40,self.data())
                for off in(0x88,0x8c,0x90,0x94,0x98):self.put(p+off,self.com()if(c['nonnull']or off&4)else 0)
        elif kind=='link':self.link(p)
        elif kind=='secondary':self.put(p+8,self.com()if c['nonnull']else 0,self.com()if c['second']else 0)
        elif kind in('temporaries','effect'):
            # Explicitly clear only destructor-owned fields, leaving all others
            # random, including words which the constructor normally writes.
            for off in(0x0c,0x10,0x18,0x20,0x28,0x2c,0x38,0x9c,0xa4,0xb0,0xb4,0xbc,0xc0,0xc4,0xcc,0xd8,0xdc,0x100,0x104,0x108,0x11c):self.put(p+off,0)
            self.put(p+0xd0,0);self.put(p+0x120,0)
            self.put(p+0xa0,c['small']);self.put(p+0xac,c['stride'])
            for off in(0xb4,0xc0):
                if c['nonnull']or off==0xc0:
                    a=self.alloc(max(4,c['small']*c['stride']*4));self.put(p+off,a)
                    for j in range(c['small']*c['stride']):
                        d=self.data()if j&1 else 0;self.put(a+j*4,d)
                        if not self.free_patch and d and c['patch']=='stride':self.free_patch=(d,p+0xac,0)
            if kind=='effect':
                self.fill('value',p+0xe0,deep);self.fill('parameter',p+0x48,deep)
                if c['nonnull']:
                    a=self.alloc(0x28);self.fill('base',a,deep);self.put(p+0xdc,a)
                    a=self.alloc(4*c['small']or 4);self.put(p+0xd8,a);self.put(p+0xd0,c['small'])
                    for j in range(c['small']):
                        v=self.alloc(0x1c)if j&1 else 0
                        if v:self.fill('value',v,False)
                        self.put(a+j*4,v)
                    for off,t in((0x2c,'group64'),(0x9c,'parameter'),(0xa4,'recordB4'),(0x18,'reference18')):
                        self.put(p+off,self.array(t,c['small'],True))
                    for off in(0xbc,0xc4,0x20,0xb0,0xcc,0x108):self.put(p+off,self.data())
                    a=self.alloc(4*c['small']or 4);self.put(p+0x11c,a);self.put(p+0x120,c['small'])
                    for j in range(c['small']):self.put(a+j*4,self.data()if j&1 else 0)
                    for off in(0x100,0x104,0x28,0x38,0x0c,0x10):self.put(p+off,self.com())
                    # First Release changes a later reference. The original
                    # must reread it rather than caching all six references.
                    if c['patch']=='release':self.release_patch=(p+0x104,0)
    def run(self,kind,c):
        self.prepare(c);self.rng=random.Random(c['seed']);self.release_patch=self.free_patch=None
        self.put(VTABLE+8,COM_RELEASE)
        if kind=='array':
            self.root=self.alloc(4+32*8)+4;self.put(self.root-4,c['count'])
            result=self.call(kind,[self.root,c['stride'],c['count'],CALLBACK])
        else:
            basekind=kind.rsplit('_',1)[0]if kind in MEMBERS else kind
            if kind.endswith('_array')and c['flags']&2:self.root=self.array(basekind,deep=True)
            else:
                self.root=self.alloc(TYPES.get(basekind,('',0x130 if basekind in('effect','temporaries')else 16))[1])
                self.fill(basekind,self.root,True)
            result=self.call(kind,[c['flags']]if kind in MEMBERS else[],self.root)
        return self.snapshot(result)


def cases(n):
    rng=random.Random(0x66adec)
    types=list(range(19))+[0xffffffff,0x80000000,0x7fffffff]
    for i,c in enumerate(core.cases(n)):
        c.update(seed=rng.getrandbits(32),type=types[i%len(types)],small=(i//len(types))%4,
            count=(0,1,2,3,7,0x80000001,0xffffffff)[(i//6)%7],stride=(0,1,2,4,8,32)[i%6],
            flags=(0,1,2,3,0x80,0xff)[i%6],nonnull=bool((i//len(types))&1),second=bool((i//len(types))&2),
            callback=i%3==0,patch=('none','stride','release')[(i//3)%3])
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();coverage=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect destruction mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect destruction mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'event counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3])
            coverage.update([('type',hex(c['type'])),('count',hex(c['count'])),('flags',hex(c['flags'])),('callback',c['callback'],c['patch'])])
    return dict(comparisons=dict(counts),boundary_calls=dict(events),coverage={str(k):v for k,v in coverage.items()})


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512)
    p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        offset=int(f['address'],16)-0x400000;assert hashlib.sha256(image[offset:offset+f['size']]).hexdigest()==f['sha256']
    report=verify(image,build(args.replace or SOURCE,args.gcc),args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free and COM Release callbacks controlled; all 27 cleanup bodies execute recovered source',
        'bounded valid graphs, finite kind/flag/count combinations; callback changes cover selected later-field rereads',
        'high-bit counts whose signed count-1 guard skips iteration are tested; 2^31 iterations and malformed pointers are not',
        'no exception unwinding, GPU driver ownership, real-effect parsing or runtime setup established'])
    if args.mutation_checks:
        mutations={
            'forward-array':('array','address-=size; destroy(reinterpret_cast<void *>(address));','destroy(reinterpret_cast<void *>(address)); address-=size;'),
            'unsigned-count-guard':('array','static_cast<int>(remaining)>=0','count!=0'),
            'wrong-cookie-count':('base_array','sizeof(Type),*cookie,Destroy','sizeof(Type),0,Destroy'),
            'wrong-array-bit':('reference18_array','if (flags&2)','if (flags&4)'),
            'free-any-flag':('parameter_delete','if (flags&1)','if (flags)'),
            'wrong-base-array-stride':('base_array','sizeof(Type),*cookie','sizeof(Type)+4,*cookie'),
            'retain-base-array':('base','p->array20=0;','(void)0;'),
            'retain-child-link':('base','child->linked24=0;','(void)0;'),
            'parameter-base-plus8':('parameter','D3DXDestroyBase(p);','D3DXDestroyBase(reinterpret_cast<char *>(p)+8);'),
            'retain-pool-link':('parameter','p->value40=0;','(void)0;'),
            'free-all-value-kinds':('value','kind==0 || kind==2 || kind==4 || (kind>5 && kind<=10)','true'),
            'omit-value-child-detach':('value','child->value18=0;','(void)0;'),
            'omit-value-refs':('value','p->value14)->DeleteArray(3);','p->value14)->DeleteArray(2);'),
            'wrong-ref-kind-limit':('reference08','kind<=9 || kind==15','kind<=10 || kind==15'),
            'admit-negative-ref-type':('reference08','kind>4 &&','true &&'),
            'omit-resource-second':('reference18','if (p->value08) { UG2FxComRelease(p->value08); p->value08=0; }','(void)0;'),
            'record-base-plus0':('record64','D3DXDestroyBase(&p->value08);','D3DXDestroyBase(p);'),
            'omit-record-preclear':('record64','p->value08.array20=0;','(void)0;'),
            'omit-group-data':('group64','UG2FxFree(p->data00);','(void)0;'),
            'omit-pass-parameter':('record38','p->parameter30->Delete(1);','p->parameter30->Delete(0);'),
            'reverse-paired-release':('recordB4','if (slot[2]) { UG2FxComRelease(slot[2]); slot[2]=0; }\n        if (slot[0]) { UG2FxComRelease(slot[0]); slot[0]=0; }','if (slot[0]) { UG2FxComRelease(slot[0]); slot[0]=0; }\n        if (slot[2]) { UG2FxComRelease(slot[2]); slot[2]=0; }'),
            'wrong-secondary-offset':('secondary','unsigned int ref=p[3];','unsigned int ref=p[1];'),
            'erase-temp-scratch':('temporaries','p->valueC0=0;','p->valueC0=0; p->valueB8=0;'),
            'cache-temp-stride':('temporaries','j<p->valueAC','j<initialStride'),
            'retain-temp-array':('temporaries','p->valueB4=0;','(void)0;'),
            'clear-freed-bc':('effect','UG2FxFree(reinterpret_cast<void *>(p->valueBC));','UG2FxFree(reinterpret_cast<void *>(p->valueBC)); p->valueBC=0;'),
            'wrong-effect-embedded-value':('effect','D3DXDestroyValue(&p->valueE0);','D3DXDestroyValue(&p->value48);'),
            'omit-effect-secondary':('effect','D3DXDestroySecondary(&p->interface04);','(void)0;'),
        }
        rejected=[];kinds=set(SPANS)if args.kind=='all'else{args.kind}
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            if name=='cache-temp-stride':body=body.replace('if (p->valueB4) {','unsigned int initialStride=p->valueAC; if (p->valueB4) {')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','256']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect destruction mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
