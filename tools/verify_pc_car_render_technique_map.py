#!/usr/bin/env python3
"""Verify UG2 parameter-map lifetime, linear hashing and bucket vectors.

The allocator is controlled. Rebuilt cycles execute recovered hash insertion and
all its map/list/vector callees. Oversized paths stop at the original exception
construction boundaries, without claiming Windows unwinding.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE
import verify_pc_car_render_technique as technique

SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderTechniqueMap.cpp'
BUILD=ROOT/'build/analysis/car-render-technique-map'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-technique-map.json'
ARENA,SIZE,HEAP,HEAP_SIZE,STACK,STOP=0x2000000,0x10000,0x3200000,0x40000,0x4800000,0x4810000
PARAM,MAP,VECTOR,LIST,DATA,OUT,VALUE,TAG,PAIR=[ARENA+x for x in (0,0x960,0x3000,0x3100,0x4000,0x5000,0x5100,0x5200,0x5300)]
# Contiguous instruction spans, including inline Windows EH landing pads.
SPANS=dict(construct=(0x5d0bb0,147),destroy=(0x5d0660,84),reset=(0x5d0f20,86),
    sentinel=(0x5ce070,26),fill=(0x5ce0f0,35),copy=(0x5ca630,37),range=(0x5ca610,30),
    backward=(0x5ce0c0,40),vcopy=(0x5cf190,40),vfill=(0x5cfbc0,39),
    initialise=(0x5d0250,95),assign=(0x5d0c50,77),insert=(0x5d0890,669),erase=(0x5cf140,75),
    node=(0x5ce090,45),grow=(0x5d06c0,171),verase=(0x5d02d0,53),resize=(0x5d0b30,120),hash_insert=(0x5d0cf0,560))
MEMBERS=dict(construct=('CarRenderTechniqueMap','Construct'),destroy=('CarRenderTechniqueMap','Destroy'),
    reset=('CarRenderTechniqueParameters','Reset'),erase=('CarRenderTechniqueMapList','Erase'),
    node=('CarRenderTechniqueMapList','CreateNode'),grow=('CarRenderTechniqueMapList','Grow'),
    hash_insert=('CarRenderTechniqueMap','Insert'))
MEMBERS.update({k:('CarRenderTechniqueMapVector',v)for k,v in dict(vcopy='Copy',vfill='Fill',initialise='Initialise',assign='Assign',insert='Insert',verase='Erase',resize='Resize').items()})
MEMBERS['parameter_bind']=('CarRenderTechniqueParameters','Bind')
FREE=dict(sentinel='CarRenderTechniqueMapSentinel',fill='CarRenderTechniqueMapFill',copy='CarRenderTechniqueMapCopy',
    range='CarRenderTechniqueMapFillRange',backward='CarRenderTechniqueMapCopyBackward')
RETURNS={'construct','sentinel','copy','vcopy','vfill','node','hash_insert'}
ORIGINAL={k:a for k,(a,n)in SPANS.items()};ORIGINAL['parameter_bind']=0x5d1b70
HASH_BRANCHES={0x5d0d0a:'split',0x5d0d25:'grow-buckets',0x5d0d55:'double-mask',0x5d0d8a:'keep-low',0x5d0dbe:'check-terminal-high',
    0x5d0d91:'move-high',0x5d0dc4:'nonterminal-high',0x5d0e1a:'splice',0x5d0e71:'fold-bucket',
    0x5d0ea3:'new-node',0x5d0f0c:'duplicate'}


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    bindings=dict(UG2TextureMalloc=0x575620,UG2TextureFree=0x575630,UG2TechniqueVectorTooLong=0x5d07e0,UG2TechniqueListTooLong=STOP+0x100)
    bindings.update(technique.GLOBALS);bindings.update(technique.API);bindings['sqrtl']=STOP+0x900
    sources=[source,technique.SOURCE,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if gcc:
        linked=link_native(sources,BUILD,ROOT,bindings,'_Z29CarRenderTechniqueMapSentinelv',text_address=0x5000000)
        prefixes={k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()}
        prefixes.update({k:'_Z'+str(len(n))+n for k,n in FREE.items()})
    else:
        objects=[]
        for i,path in enumerate(sources):
            obj=BUILD/('technique-map-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()};bindings.update(__fltused=0,__ftol2=0x75bed4)
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:'?'+m+'@'+c+'@@'for k,(c,m)in MEMBERS.items()}
        prefixes.update({k:'?'+n+'@@'for k,n in FREE.items()})
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for n,a in linked['exports'].items()if n.startswith(p)]
        assert len(hits)==1,(k,p,hits)
        linked['entries'][k]=hits[0]
    return linked


class Machine:
    def __init__(self,image,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(0,4096);u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        for a,n in [(ARENA,SIZE),(HEAP,HEAP_SIZE),(STACK,0x20000)]:u.mem_map(a,n)
        self.gcc,self.linked=gcc,linked
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        self.entries=linked['entries']if linked else ORIGINAL
        for a,k in [(0x575620,'malloc'),(0x575630,'free'),(0x5d07e0,'too-long'),(0x5d06ec,'list-too-long'),(STOP+0x100,'list-too-long')]:
            u.hook_add(UC_HOOK_CODE,self.runtime,user_data=k,begin=a,end=a)
        if not linked:
            for a,k in HASH_BRANCHES.items():u.hook_add(UC_HOOK_CODE,self.branch,user_data=k,begin=a,end=a)
    def branch(self,u,address,size,kind):
        if kind=='check-terminal-high':
            if u.reg_read(xc.UC_X86_REG_EAX)!=self.word(u.reg_read(xc.UC_X86_REG_ESI)+8):return
            kind='terminal-high'
        self.branches[kind]+=1
    def word(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
    def put(self,a,*values):self.u.mem_write(a,struct.pack('<'+'I'*len(values),*(v&0xffffffff for v in values)))
    def ret(self,result=0):
        sp=self.u.reg_read(xc.UC_X86_REG_ESP);pc=self.word(sp)
        self.u.reg_write(xc.UC_X86_REG_EAX,result);self.u.reg_write(xc.UC_X86_REG_ESP,sp+4);self.u.reg_write(xc.UC_X86_REG_EIP,pc)
    def runtime(self,u,address,size,kind):
        if kind in ('too-long','list-too-long'):self.events.append((kind,));self.thrown=True;u.emu_stop();return
        arg=self.word(u.reg_read(xc.UC_X86_REG_ESP)+4)
        # Capture caller-visible object/list state at every allocation boundary.
        state=bytes(u.mem_read(PARAM,0x988))+bytes(u.mem_read(VECTOR,0x120))+bytes(u.mem_read(HEAP,self.allocated))
        if kind=='malloc':
            result=HEAP+self.allocated;self.allocated+=(arg+15)&~15
            assert self.allocated<=HEAP_SIZE,('heap limit',arg)
            self.live[result]=arg;self.events.append((kind,arg,result,state));self.ret(result)
        else:
            assert not arg or arg in self.live,('invalid free',hex(arg),self.live)
            body=bytes(u.mem_read(arg,self.live.pop(arg)))if arg else b''
            self.events.append((kind,arg,body,state));self.ret()
    def prepare(self,c):
        self.u.mem_write(ARENA,c['arena']);self.u.mem_write(HEAP,c['heap']);self.put(0,0x12345678)
        self.events=[];self.allocated=0;self.live={};self.thrown=False;self.branches=Counter();self.hash_results=Counter()
    def call(self,kind,args=(),receiver=0,original=False):
        u=self.u;gcc=self.gcc and not original
        member=kind in MEMBERS
        params=([receiver]+list(args))if gcc and member else list(args)
        sp=STACK+0xf000;self.put(sp,STOP,*params)
        u.reg_write(xc.UC_X86_REG_ESP,sp);u.reg_write(xc.UC_X86_REG_ECX,receiver)
        regs=[xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP]
        seeds=[0x19876543,0x23456789,0x38765432,0x456789ab]
        for r,v in zip(regs,seeds):u.reg_write(r,v)
        u.reg_write(xc.UC_X86_REG_EFLAGS,0x202);u.reg_write(xc.UC_X86_REG_FPCW,0x27f);u.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
        if kind=='hash_insert':old_count,old_buckets=self.word(receiver+0xc),self.word(receiver+0x24)
        pc=ORIGINAL[kind]if original else self.entries[kind]
        u.emu_start(pc,STOP,count=1000000)
        if self.thrown:return None
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('did not return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        pop=0 if gcc or not member else len(args)*4
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop,('stack',kind)
        assert [u.reg_read(r)for r in regs]==seeds,('callee saved',kind)
        assert u.reg_read(xc.UC_X86_REG_FPCW)==0x27f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff,('x87',kind)
        assert self.word(0)==0x12345678,('exception chain',kind)
        if kind=='hash_insert':
            # Explicit calls (including seeders); nested parameter binding is
            # counted separately by the original-instruction branch hooks.
            inserted=self.u.mem_read(args[0]+4,1)[0]
            self.hash_results['inserted'if inserted else'duplicate']+=1
            if not inserted and self.word(receiver+0x24)>old_buckets:self.hash_results['duplicate-with-split']+=1
            assert self.word(receiver+0xc)==old_count+inserted,('hash insertion count',)
        return u.reg_read(xc.UC_X86_REG_EAX)if kind in RETURNS else None
    def allocate(self,n):
        a=HEAP+self.allocated;self.allocated+=(n+15)&~15;self.live[a]=n;return a
    def list(self,count):
        head=self.allocate(16);nodes=[self.allocate(16)for _ in range(count)];ring=[head]+nodes
        for i,a in enumerate(ring):self.put(a,ring[(i+1)%len(ring)],ring[i-1])
        self.put(LIST+4,head,count);self.put(MAP+8,head,count)
        return head,nodes
    def vector(self,c):
        n,capacity=c['size'],c['capacity']
        storage=self.allocate(capacity*4)if capacity else 0
        self.put(VECTOR+4,storage,storage+n*4,storage+capacity*4)
        if storage:self.u.mem_write(storage,c['words'][:capacity*4])
        return storage
    def snapshot(self,result):
        return (bytes(self.u.mem_read(ARENA,SIZE)),bytes(self.u.mem_read(HEAP,HEAP_SIZE)),result,
            self.events,self.live,self.thrown,self.allocated)
    def run(self,kind,c):
        self.prepare(c);n=c['size'];position=c['position']
        if kind=='construct':result=self.call(kind,[TAG,TAG+1],MAP)
        elif kind=='sentinel':result=self.call(kind)
        elif kind=='node':result=self.call(kind,[DATA,OUT,HEAP if c['alias']else PAIR],LIST)
        elif kind=='grow':
            self.put(LIST+8,c['list_size']);result=self.call(kind,[c['count']],LIST)
        elif kind=='hash_insert':
            self.call('construct',[TAG,TAG+1],MAP,original=True)
            # Leave the final lookup to the candidate body, before duplicate
            # tail calls can perform a pending split in the original seeder.
            for key,value in c['pairs'][0][:-2]:
                self.put(PAIR,key,value);self.call(kind,[OUT,PAIR],MAP,original=True)
            self.put(PAIR,c['key'],c['handle']);result=self.call(kind,[OUT,PAIR],MAP)
            self.assert_map()
        elif kind in ('erase','destroy','reset'):
            if kind!='erase'and c['null_head']:
                head,nodes=0,[];self.put(LIST+4,0,0);self.put(MAP+8,0,0)
            else:head,nodes=self.list(n)
            if kind=='erase':
                ring=[head]+nodes
                # Some ranges cross the sentinel; the sentinel must be skipped.
                start=position%len(ring);length=c['count']%len(ring)
                result=self.call(kind,[OUT,ring[start],ring[(start+length)%len(ring)]],LIST)
            else:
                capacity=c['capacity'];storage=self.allocate(capacity*4)if capacity else 0
                self.put(MAP+0x14,storage,storage+capacity*4,storage+capacity*4)
                if storage:self.u.mem_write(storage,c['words'][:capacity*4])
                result=self.call(kind,receiver=MAP if kind=='destroy'else PARAM)
        elif kind in ('insert','assign','initialise','vcopy','vfill','verase','resize'):
            storage=self.vector(c)
            value=(VECTOR+8 if c['alias_index']==3 else storage+(0,n//2,max(0,n-1))[c['alias_index']]*4)if c['alias']and n else VALUE
            if kind=='insert':result=self.call(kind,[storage+position*4,c['count'],value],VECTOR)
            elif kind in ('assign','initialise'):result=self.call(kind,[c['count'],value],VECTOR)
            elif kind=='resize':result=self.call(kind,[c['count'],self.word(value)],VECTOR)
            elif kind=='verase':result=self.call(kind,[VECTOR+8 if c['alias']else OUT,storage+position*4,storage+min(n,position+c['count'])*4],VECTOR)
            elif kind=='vcopy':result=self.call(kind,[DATA,DATA+n*4,c['output']],VECTOR)
            else:result=self.call(kind,[c['output'],c['count'],c['value']],VECTOR)
        elif kind=='fill':result=self.call(kind,[c['output'],c['count'],c['value'],VECTOR,0xa5a5a5a5])
        elif kind=='copy':result=self.call(kind,[DATA,DATA+n*4,c['output'],VECTOR,0xdeadbeef])
        elif kind=='range':result=self.call(kind,[DATA,DATA+c['count']*4,c['value']])
        else:result=self.call(kind,[OUT,DATA,DATA+n*4,c['output']+n*4])
        return self.snapshot(result)
    def cycle(self,c):
        self.prepare(c);self.call('construct',[TAG,TAG+1],MAP)
        for repeat in range(3):
            for key,value in c['pairs'][repeat]:
                self.put(PAIR,key,value);self.call('hash_insert',[OUT,PAIR],MAP)
                self.assert_map()
                self.events.append(('hash-result',key,value,bytes(self.u.mem_read(OUT,8)),bytes(self.u.mem_read(MAP,40)),bytes(self.u.mem_read(HEAP,self.allocated))))
            self.call('reset',receiver=PARAM)
            assert self.word(MAP+0xc)==0 and self.word(MAP+0x20)==self.word(MAP+0x24)==1
            assert self.word(MAP+0x18)-self.word(MAP+0x14)==36
            assert all(self.word(self.word(MAP+0x14)+4*j)==self.word(MAP+8)for j in range(9))
        self.call('destroy',receiver=MAP);assert not self.live,('leaked map allocations',self.live)
        self.call('destroy',receiver=MAP) # Original tolerates an already-cleared map.
        assert bytes(self.u.mem_read(PARAM,0x960))==c['arena'][:0x960],('reset cleared static slots',)
        return self.snapshot(None)
    def bind_chain(self,c):
        self.prepare(c)
        for i in range(60):
            h=technique.hash_(technique.NAMES[0 if c['alias']and i==59 else i])
            self.put(PARAM+i*40+0x20,h,self.word(PARAM+i*40+0x24))
        self.call('construct',[TAG,TAG+1],MAP)
        for repeat in range(2):
            for i in range(128):
                h=(0xdeadbeef if i%13==0 else technique.hash_(technique.NAMES[(i*7+repeat*11)%60]))
                handle=(c['handle']+i*0x12345+repeat)&0xffffffff
                self.call('parameter_bind',[h,handle],PARAM)
                self.assert_map()
                self.events.append(('parameter-bind',h,handle,bytes(self.u.mem_read(PARAM,0x988)),bytes(self.u.mem_read(HEAP,self.allocated))))
            slots=bytes(self.u.mem_read(PARAM,0x960));self.call('reset',receiver=PARAM)
            assert slots==bytes(self.u.mem_read(PARAM,0x960)),('reset cleared slots',)
        self.call('destroy',receiver=MAP);assert not self.live,('bind-chain leaked allocations',)
        return self.snapshot(None)
    def assert_map(self):
        head=self.word(MAP+8);node=self.word(head);count=0;previous=head
        while node!=head:
            assert self.word(node+4)==previous;previous,node=node,self.word(node);count+=1
            assert count<=self.word(MAP+0xc)
        assert count==self.word(MAP+0xc) and self.word(head+4)==previous
        # Every bucket is a contiguous unsigned-key-sorted range in the ring.
        mask,buckets=self.word(MAP+0x20),self.word(MAP+0x24);base=self.word(MAP+0x14)
        assert self.word(base+buckets*4)==head
        all_nodes=[]
        for bucket in range(buckets):
            node,end=self.word(base+bucket*4),self.word(base+(bucket+1)*4);last_key=-1
            while node!=end:
                key=self.word(node+8);index=key&mask
                if index>=buckets:index-=1+(mask>>1)
                assert index==bucket and key>=last_key,('bucket invariant',bucket,key,last_key,index)
                all_nodes.append(node);last_key=key;node=self.word(node)
                assert len(all_nodes)<=count
        assert len(all_nodes)==count and len(set(all_nodes))==count


def cases(n):
    rng=random.Random(0x5d0bb0)
    for i in range(n):
        size=(0,1,2,3,4,8,9,16,31,60)[i%10];capacity=size+(0,1,4,9,24)[(i//10)%5]
        count=(0,1,2,4,9,16)[(i//3)%6]
        # Initialise assumes that its backing allocation already fits the count.
        pairs=[]
        for repeat in range(3):
            length=(i*7+repeat*13)%97;mode=i%8
            keys=[j if mode==0 else length-j if mode==1 else 0xffffffff if mode==2 else
                (j*64+(i&3))if mode==3 else (0xffffffff-j)if mode==4 else (0x80000000^(j*33))if mode==5 else
                ((j%7)if j%3==0 else rng.getrandbits(32))for j in range(length)]
            pairs.append([(key,rng.getrandbits(32))for key in keys]+([(keys[0],rng.getrandbits(32))]*2 if keys else []))
        if i%16==0:pairs[0]=[(key,rng.getrandbits(32))for key in (0,2,4,7,0,0)]
        yield dict(arena=rng.randbytes(SIZE),heap=rng.randbytes(HEAP_SIZE),size=size,capacity=max(capacity,16),
            position=(0,size//2,size)[(i//2)%3],count=count,alias=bool(i&1),alias_index=(i//3)%4,null_head=i%31==0,words=rng.randbytes(512),
            output=(OUT,DATA,DATA+4,DATA-4)[(i//6)%4],value=(VALUE,DATA,DATA+4)[(i//4)%3],
            key=(0,1,0x7fffffff,0x80000000,0xffffffff,rng.getrandbits(32))[i%6],handle=rng.getrandbits(32),
            list_size=(0,1,0x1ffffffe,0x1fffffff,60)[i%5],
            pairs=pairs)


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();branches=Counter();hash_branches=Counter();hash_results=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            # Include empty vectors, growth and both in-place insertion branches.
            test=dict(c)
            if kind in ('insert','assign')and i%11==0:test.update(size=0,capacity=0,position=0,alias=False)
            if kind=='insert'and i%29==0:test.update(size=1,capacity=16,position=0,count=0x3fffffff,alias=False)
            try:
                x=a.cycle(test)if kind=='cycle'else a.bind_chain(test)if kind=='bind-chain'else a.run(kind,test)
                y=b.cycle(test)if kind=='cycle'else b.bind_chain(test)if kind=='bind-chain'else b.run(kind,test)
            except (AssertionError,UcError)as e:raise AssertionError(('map mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:3],w[:3])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('map mismatch',kind,i,offsets,'result',x[2],y[2],'events',len(x[3]),len(y[3]),event))
            counts[kind]+=1
            hash_branches.update(a.branches);hash_results.update(a.hash_results)
            if kind=='insert':
                category='too-long'if x[5]else'zero'if not test['count']else'grow'if test['size']+test['count']>test['capacity']else'in-place-long-tail'if test['size']-test['position']>=test['count']else'in-place-short-tail'
                branches[category]+=1
    return dict(comparisons=dict(counts),vector_insert_branches=dict(branches),original_hash_branches=dict(hash_branches),hash_results=dict(hash_results))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512)
    p.add_argument('--kind',choices=['all',*SPANS,'cycle','bind-chain'],default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for kind,f in dict(manifest['functions'],**manifest.get('linked_functions',{})).items():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256'],kind
    kinds=[*SPANS,'cycle','bind-chain']if args.kind=='all'else[args.kind]
    report=verify(image,build(args.replace or SOURCE,args.gcc),args.gcc,args.cases,kinds)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['successful bounded allocation; malformed containers, allocator failure and Windows unwinding not established',
        'oversized list/vector compares only exception-construction handoffs, not Windows exception construction',
        'native GPU effect setup remains inactive; these library bodies do not establish rendered equivalence'])
    if args.mutation_checks:
        mutations={
            'wrong-tag-byte':('construct','value00[0] = *tagA;','value00[1] = *tagA;'),
            'clear-opaque-word':('construct','count0C = 0;','count0C = 0; value10=0;'),
            'wrong-initial-mask':('construct','mask20 = 1;','mask20 = 3;'),
            'missing-sentinel-backlink':('sentinel','if (previous) *reinterpret_cast<CarRenderTechniqueMapNode **>(previous) = node;','(void)previous;'),
            'incomplete-fill':('fill','for (;count;--count,++output) if (output) *output = *value;','if (count) --count; for (;count;--count,++output) if (output) *output = *value;'),
            'backward-forward-copy':('copy','for (;first!=last;++first,++output) if (output) *output = *first;','CarRenderTechniqueMapNode **end=output+(last-first); output=end; while (first!=last) *--end=*--last;'),
            'wrong-fill-range':('range','for (;first!=last;++first) *first = *value;','for (;first!=last;++first) *first = 0;'),
            'wrong-backward-result':('backward','*result = output;','*result = output+1;'),
            'wrong-wrapper-copy-return':('vcopy','return CarRenderTechniqueMapCopy(first,last,output,this,reinterpret_cast<unsigned int>(output));','return CarRenderTechniqueMapCopy(first,last,output,this,reinterpret_cast<unsigned int>(output))+1;'),
            'wrong-wrapper-fill-return':('vfill','return output+count;','return output;'),
            'wrong-initial-end':('initialise','end08 = output+count;','end08 = output;'),
            'wrong-growth-factor':('insert','capacity += capacity>>1;','capacity += capacity;'),
            'uncaptured-insert-value':('insert','CarRenderTechniqueMapFillRange(position,position+count,&saved);','CarRenderTechniqueMapFillRange(position,position+count,value);'),
            'wrong-short-tail-fill':('insert','count-static_cast<unsigned int>(end08-position)','count'),
            'skip-oversize-check':('insert','if (0x3fffffff-size<count)','if (false)'),
            'do-not-clear-vector':('assign','if (begin04!=end08) end08 = begin04;','(void)0;'),
            'omit-list-count':('erase','--count08;','(void)0;'),
            'omit-list-backlink':('erase','node->next00->previous04 = node->previous04;','(void)0;'),
            'omit-head-free':('destroy','UG2TextureFree(head08);','(void)0;'),
            'free-list-before-vector':('destroy','if (begin14) UG2TextureFree(begin14);','reinterpret_cast<CarRenderTechniqueMapList *>(value00+4)->Erase(&head08,head08 ? head08->next00 : 0,head08); if (begin14) UG2TextureFree(begin14);'),
            'clear-cached-handles':('reset','CarRenderTechniqueMapNode *head = map960.head08;','slots00[0].handle24=0; CarRenderTechniqueMapNode *head = map960.head08;'),
            'retain-reset-mask':('reset','map960.mask20 = 1;','(void)0;'),
            'wrong-node-key':('node','node->key08 = pair->hash00;','node->key08 = pair->value04;'),
            'wrong-node-value':('node','node->value0C = pair->value04;','node->value0C = pair->hash00;'),
            'early-node-pair-copy':('node','CarRenderTechniqueMapNode *node = static_cast<CarRenderTechniqueMapNode *>(UG2TextureMalloc(0x10));\n    if (node) {','CarRenderTechniqueMapPair saved=*pair; pair=&saved; CarRenderTechniqueMapNode *node = static_cast<CarRenderTechniqueMapNode *>(UG2TextureMalloc(0x10));\n    if (node) {'),
            'wrong-list-limit':('grow','0x1fffffff-count08<count','0x3fffffff-count08<count'),
            'reject-list-limit-equality':('grow','0x1fffffff-count08<count','0x1fffffff-count08<=count'),
            'wrong-list-growth':('grow','count08 += count;','count08 += count+1;'),
            'wrong-vector-erase-end':('verase','end08 = output;','end08 = oldEnd;'),
            'wrong-vector-erase-result':('verase','*result = first;','*result = first+1;'),
            'wrong-vector-growth-fill':('resize','Insert(end08,count-size,&value);','value=0; Insert(end08,count-size,&value);'),
            'wrong-vector-shrink-range':('resize','Erase(&result,begin04+count,end08);','Erase(&result,begin04+count+1,end08);'),
            'late-bucket-split':('hash_insert','bucket<=(count0C>>2)','bucket<(count0C>>2)'),
            'wrong-load-factor':('hash_insert','count0C>>2','count0C>>3'),
            'wrong-bucket-mask':('hash_insert','mask20 = mask20*2+1;','mask20 = mask20*2+3;'),
            'wrong-bucket-vector-growth':('hash_insert','Resize(size*2-1,head08);','Resize(size*2+1,head08);'),
            'wrong-split-bucket':('hash_insert','bucket = buckets24-(mask20>>1)-1;','bucket = buckets24-(mask20>>1);'),
            'missing-upper-boundary':('hash_insert','begin14[upper] = node;','begin14[upper] = head08;'),
            'missing-lower-boundary':('hash_insert','begin14[lower] = begin14[lower]->next00;','(void)0;'),
            'return-from-terminal-split':('hash_insert','if (next==head08) break;','if (next==head08) { result->node00=node; result->inserted04=0; return result; }'),
            'wrong-splice-backlink':('hash_insert','following->previous04 = node->previous04;','following->previous04 = head;'),
            'omit-bucket-increment':('hash_insert','++buckets24;','(void)0;'),
            'wrong-fold-boundary':('hash_insert','if (buckets24<=bucket)','if (buckets24<bucket)'),
            'signed-key-order':('hash_insert','if (node->key08<=key)','if (static_cast<int>(node->key08)<=static_cast<int>(key))'),
            'overwrite-duplicate-value':('hash_insert','result->inserted04 = 0;','result->inserted04 = 0; node->value0C=pair->value04;'),
            'grow-before-allocation':('hash_insert','CarRenderTechniqueMapNode *created = list->CreateNode(node,node->previous04,pair);\n    list->Grow(1);','list->Grow(1); CarRenderTechniqueMapNode *created = list->CreateNode(node,node->previous04,pair);'),
            'clear-result-padding':('hash_insert','result->inserted04 = 1;','result->inserted04 = 1; result->pad05[0]=0;'),
            'wrong-result-node':('hash_insert','result->node00 = inserted;','result->node00 = node;'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout)
            assert r.returncode and 'map mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
