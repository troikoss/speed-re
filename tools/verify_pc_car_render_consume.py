#!/usr/bin/env python3
"""Verify the complete original queued solid consumer and recovered queue/pool chain.

Rendering APIs, technique virtual calls and the six pending upload/bind bodies
are controlled boundaries. Real queueing, view/sky getters and slot return run.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import random
import re
import struct
import subprocess
import sys
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
from pc_coff import link_objects,read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE
import verify_pc_car_render_technique as technique
import verify_pc_car_render_solid as solid

SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderConsume.cpp'
BUILD=ROOT/'build/analysis/car-render-consume'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-consume.json'
ARENA,SIZE,STACK,STOP=0x2000000,0x40000,0x4800000,0x4810000
TECH,PARAM,PLATFORM,TABLE,PRIM,TEXTURE,STATE,GPOOL,CPOOL,IPOOL,VERTEX,WORLD,MODE,RECORD,LIGHT,BONES,DEVICE,EFFECT,BUFFER,UPLOAD=[ARENA+x for x in
    (0,0x1000,0x10000,0x11000,0x12000,0x13000,0x14000,0x15000,0x16000,0x18000,0x1a000,0x1b000,0x1b800,0x1c000,0x1e000,0x1f000,0x28000,0x28200,0x2d000,0x30000)]
GLOBALS=dict(technique.GLOBALS,**solid.GLOBALS,ug2RenderTechnique870910=0x870910,
    ug2RenderVertex87092C=0x87092c,ug2RenderIndex870930=0x870930,ug2RenderImmediatePool870954=0x870954,
    ug2RenderFrames872EDC=0x872edc,ug2RenderGroups872ED8=0x872ed8,ug2RenderIndexed872EE0=0x872ee0,
    ug2RenderImmediate872EE4=0x872ee4,ug2RenderVertexChanges872EE8=0x872ee8,ug2RenderIndexChanges872EEC=0x872eec,
    ug2RenderTriangles872EF0=0x872ef0,ug2RenderTechniques872F04=0x872f04,ug2RenderViews832DE0=0x832de0,
    ug2RenderSkyHashes8A07A8=0x8a07a8,ug2RenderSkyIndex8A1C94=0x8a1c94,ug2RenderSkyTime8A1C9C=0x8a1c9c,ug2RenderSkyPeriod802954=0x802954)
PREFIX=dict(consume='?CarRenderConsumeCommands@@',clear='?Clear@CarRenderCommandList@@',
    group_clear='?ClearCommands@CarRenderTextureGroup@@',view='?CarRenderViewByIndex@@',sky='?CarRenderSkyIndex@@',
    free='?Free@bSlotPool@@',fraction='?CarRenderSkyFraction@@',queue='?CarRenderQueuePrimitive@@')
GCC=dict(consume='_Z24CarRenderConsumeCommandsv',clear='_ZN20CarRenderCommandList5ClearEv',
    group_clear='_ZN21CarRenderTextureGroup13ClearCommandsEv',view='_Z20CarRenderViewByIndexi',sky='_Z17CarRenderSkyIndexv',
    free='_ZN9bSlotPool4FreeEPv',fraction='_Z20CarRenderSkyFractionv',queue=solid.GCC['queue'])
# Every original D3D/D3DX argument is on the stack; effect objects have individual tokens.
APIS=[('UG2RenderSetDeclaration',2,0x15c,'device'),('UG2RenderSetStream',5,0x190,'device'),
    ('UG2RenderSetIndices',2,0x1a0,'device'),('UG2RenderDrawIndexed',7,0x148,'device'),
    ('UG2RenderDrawImmediate',5,0x14c,'device'),('UG2RenderSetState',3,0xe4,'device'),
    ('UG2EffectBegin',3,0x104,'effect'),('UG2EffectCommit',1,0x10c,'effect'),('UG2EffectBeginPass',2,0x108,'effect'),
    ('UG2EffectEndPass',1,0x110,'effect'),('UG2EffectEnd',1,0x114,'effect'),
    ('UG2EffectSetTexture',3,0xd0,'effect'),('UG2EffectSetFloat',3,0x78,'effect'),('UG2EffectSetInt',3,0x68,'effect'),
    ('UG2SolidCreateVertexBuffer',7,0x68,'device'),('UG2SolidLockBuffer',5,0x2c,'buffer'),('UG2SolidUnlockBuffer',1,0x30,'buffer')]
API={name:STOP+0x1000+i*16 for i,(name,*_)in enumerate(APIS)}
HELPERS=dict(bind=('CarRenderBindTexture',0x5bcc30,1),world=('CarRenderUploadWorld',0x5bdde0,2),
    headlights=('CarRenderUploadHeadlights',0x5bdaa0,2),record=('CarRenderUploadRecord',0x5bd140,2),
    light=('CarRenderUploadLightPack',0x5bd4a0,2),bones=('CarRenderUploadBones',0x5bdcd0,2))
VIRTUAL=dict(begin=STOP+0x2000,end=STOP+0x2010)
REGIONS=[(0x870d30,0xb80),(0x86eb28,92),(0x870910,4),(0x870918,8),(0x87092c,8),(0x87094c,12),(0x872ed8,0x30)]


def build(source,gcc,pool_source=None):
    symbols={name:int(a,16)for name,a in re.findall(r'^(\w+) = (0x[0-9a-f]+);',(ROOT/'src/Platform/Native/OriginalSymbols.ld').read_text(),re.M)}
    bindings=dict(GLOBALS,**API,UG2RenderTechniqueBegin=VIRTUAL['begin'],UG2RenderTechniqueEnd=VIRTUAL['end'],
        UG2SolidFindTexture=0x4901d0)
    # These symbols are only used by other bWare pool operations or dispatcher paths.
    cpp=dict(bMalloc=0x440390,bFreeMemory=0x440540)
    cpp.update({name:address for name,address,n in HELPERS.values()})
    sources=[source,solid.SOURCE,pool_source or ROOT/'src/Speed/Indep/Libs/bWare/bSlotPool.cpp']
    if gcc:
        bindings.update({k:symbols[k]for k in ('slotPoolRegistry828530','nodeSlotPool8284F0','nodeSlotPoolPendingDelete8284F4','nodeSlotPoolName786F40')})
        bindings['_ZN13CarRenderView10TestBoundsEPKfS1_P11WheelMatrix']=0x5bc300
        for i,path in enumerate(sources):
            obj=BUILD/('names-'+str(i)+'.o')
            subprocess.run(['g++','-std=c++11','-m32','-O1','-fno-pie','-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),
                '-Isrc/Speed/Indep/Src/World','-c',str(path),'-o',str(obj)],cwd=ROOT,check=True)
            for line in subprocess.check_output(['nm','-u',str(obj)],text=True).splitlines():
                name=line.split()[-1]
                for k,v in cpp.items():
                    if name.startswith('_Z'+str(len(k))+k):bindings[name]=v
        linked=link_native(sources,BUILD,ROOT,bindings,GCC['consume'],text_address=0x5000000)
    else:
        objects=[];bindings={'_'+k:v for k,v in bindings.items()};bindings.update(__fltused=0,__ftol2=0x75bed4)
        for i,path in enumerate(sources):
            obj=BUILD/('consume-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            for s in read_object(obj)['symbols'].values():
                if s['section']!=0:continue
                name=s['name'];m=re.match(r'^\?(\w+)@@',name)
                if m and m[1]in symbols:bindings[name]=symbols[m[1]]
                for k,v in cpp.items():
                    if name.startswith('?'+k+'@@'):bindings[name]=v
                if name.startswith('?TestBounds@CarRenderView@@'):bindings[name]=0x5bc300
        linked=link_objects(objects,bindings,start=0x5000000)
    linked['entries']={}
    for k,p in (GCC if gcc else PREFIX).items():
        hits=[a for s,a in linked['exports'].items()if s.startswith(p)];assert len(hits)==1,(k,p,hits)
        linked['entries'][k]=hits[0]
    return linked


class Machine(technique.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32);self.gcc,self.rebuilt=gcc,bool(linked)
        u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        u.mem_map(ARENA,SIZE);u.mem_map(STACK,0x20000)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        self.entries=linked['entries']if linked else{k:int(f['address'],16)for k,f in manifest['functions'].items()}
        self.entries['queue']=linked['entries']['queue']if linked else 0x5ceb30
        for name,n,offset,owner in APIS:u.hook_add(UC_HOOK_CODE,self.api,user_data=(name,n),begin=API[name],end=API[name])
        for kind,(name,a,n)in HELPERS.items():u.hook_add(UC_HOOK_CODE,self.helper,user_data=(kind,n),begin=a,end=a)
        for kind,a in VIRTUAL.items():u.hook_add(UC_HOOK_CODE,self.virtual,user_data=kind,begin=a,end=a)
        u.hook_add(UC_HOOK_CODE,self.lookup,begin=0x4901d0,end=0x4901d0)
        for a in [0x440390,0x440540,0x5bc300]:u.hook_add(UC_HOOK_CODE,self.unexpected,begin=a,end=a)
    def unexpected(self,*args):raise AssertionError('unexpected allocation/dispatch boundary')
    def virtual(self,u,address,size,kind):
        if self.rebuilt:t,=self.args(1)
        else:t=u.reg_read(xc.UC_X86_REG_ECX)
        self.events.append((kind,t,self.word(0x870910),bytes(u.mem_read(t,0x28))))
        self.ret()
    def helper(self,u,address,size,info):
        k,n=info;a=self.args(n)
        event=(k,a,self.word(0x870910))
        if k=='bind':event+=(self.word(a[0])+0x1c,)
        self.events.append(event);self.ret()
    def lookup(self,u,address,size,unused):
        a=self.args(3);assert a[1:]==(0,0)
        self.events.append(('lookup',a));self.ret(TEXTURE+15*0x80)
    def api(self,u,address,size,info):
        name,n=info;a=self.args(n);c=self.case;event=(name,*a)
        if name in ('UG2EffectBegin','UG2EffectCommit','UG2EffectBeginPass','UG2EffectEndPass','UG2EffectEnd',
            'UG2EffectSetTexture','UG2EffectSetFloat','UG2EffectSetInt'):
            assert EFFECT<=a[0]<EFFECT+23*0x200 and (a[0]-EFFECT)%0x200==0
        elif 'Buffer'not in name:assert a[0]==DEVICE
        if name=='UG2EffectBegin':
            assert a[2]==0;self.put(a[1],c['effect_passes']);event=(name,a[0],c['effect_passes'],a[2])
        elif name=='UG2SolidCreateVertexBuffer':
            j=(a[5]-PLATFORM-0x2c)//0x50;assert a[0]==DEVICE and a[2:5]==(8,0,1)and a[6]==0
            assert 0<=j<16;self.put(a[5],BUFFER+j*0x80)
            event=(name,a[1],a[2:5],j,bytes(u.mem_read(PLATFORM+j*0x50,0x44)))
        elif name=='UG2SolidLockBuffer':
            assert a[1:3]==(0,0)and a[4]==0;j=(a[0]-BUFFER)//0x80;assert 0<=j<16
            target=VERTEX+j*128+self.case['overlap']if self.case['overlap']else UPLOAD+j*0x100
            self.put(a[3],target);event=(name,a[0],a[1:3],a[4])
        elif name=='UG2SolidUnlockBuffer':
            j=(a[0]-BUFFER)//0x80;assert 0<=j<16;event=(name,a[0],bytes(u.mem_read(UPLOAD+j*0x100,0x100)))
        elif name=='UG2RenderDrawImmediate':
            assert a[1]==5 and a[4]==0x18;event=(name,*a,bytes(u.mem_read(a[3],max(0,min(4,a[2]+2))*24)))
        self.events.append(event);self.ret(c['hresult'],0 if self.rebuilt else n*4)
    def call(self,kind,args=(),receiver=0):
        u=self.u;member=kind in ('clear','group_clear','free')
        values=[receiver]+list(args)if member and self.gcc else list(args)
        sp=STACK+0x10000-0x100;self.put(sp,STOP,*values)
        saved=[(xc.UC_X86_REG_EBX,0x1248),(xc.UC_X86_REG_ESI,0x1357),(xc.UC_X86_REG_EDI,0x2468),(xc.UC_X86_REG_EBP,0x3579)]
        for r,v in saved+[(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,2),
            (xc.UC_X86_REG_FPCW,self.case['cw']),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        try:u.emu_start(self.entries[kind],STOP,count=3000000)
        except UcError as e:raise AssertionError((str(e),hex(u.reg_read(xc.UC_X86_REG_EIP)),self.events[-3:]))from e
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+(len(args)*4 if member and not self.gcc else 0)
        for r,v in saved:assert u.reg_read(r)==v,(kind,r)
        assert u.reg_read(xc.UC_X86_REG_FPCW)==self.case['cw']
        if kind=='fraction':
            # Capture the actual x87 return, before any host/float conversion.
            u.mem_write(STOP,b'\xdb\x3d'+struct.pack('<I',ARENA+0x3e000))
            u.emu_start(STOP,STOP+6)
        return u.reg_read(xc.UC_X86_REG_EAX)if kind in ('view','sky')else None
    def run(self,kind,c):
        self.case=c;self.events=[];u=self.u;u.mem_write(ARENA,c['data'])
        for a,b in c['globals'].items():u.mem_write(a,b)
        if kind=='fraction':
            self.put(0x8a1c9c,c['fraction'][0]);self.put(0x802954,c['fraction'][1])
        for owner in [DEVICE]+[EFFECT+i*0x200 for i in range(23)]+[BUFFER+i*0x80 for i in range(16)]:
            self.put(owner,owner+(0x40 if owner>=BUFFER else 0x100))
        for name,n,offset,owner in APIS:
            tokens=[DEVICE]if owner=='device'else[EFFECT+i*0x200 for i in range(23)]if owner=='effect'else[BUFFER+i*0x80 for i in range(16)]
            for token in tokens:self.put(self.word(token)+offset,API[name])
        for i in range(23):
            token=TECH+i*0x28;self.put(token,ARENA+0x3f000);self.put(ARENA+0x3f004,VIRTUAL['begin'],VIRTUAL['end'])
        if kind=='consume':
            if c['queue']:
                for a in c['queue']:self.call('queue',a)
                self.events.append(('queued-state',bytes(u.mem_read(ARENA,SIZE)),tuple(bytes(u.mem_read(a,n))for a,n in REGIONS)))
            result=self.call(kind)
        elif kind in ('clear','group_clear'):result=self.call(kind,receiver=c[kind])
        elif kind=='free':result=self.call(kind,[c['free']],IPOOL)
        elif kind=='view':result=self.call(kind,[c['view']])
        else:result=self.call(kind)
        return(bytes(u.mem_read(ARENA,SIZE)),tuple(bytes(u.mem_read(a,n))for a,n in REGIONS),self.events,result)


def cases(n,image):
    rng=random.Random(0x5cf420)
    for i in range(n):
        data=bytearray(rng.randbytes(SIZE));g={a:bytes(image[a-0x400000:a-0x400000+n])for a,n in REGIONS}
        def put(a,*v):struct.pack_into('<'+'I'*len(v),data,a-ARENA,*(x&0xffffffff for x in v))
        def glob(a,*v):g[a]=struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v))
        for j in range(368):glob(0x870d30+j*8,0x870d30+j*8,0x870d30+j*8)
        for j in range(23):
            t,p=TECH+j*0x28,PARAM+j*0xa00
            put(t+4,j,20+j*4,0x123,0xabcd,(-1,0,1,2,4)[(i+j)%5]);data[t-ARENA+0x18]=(i+j)%3
            put(t+0x1c,p,EFFECT+j*0x200,0x123400+j)
            for k in range(60):put(p+k*40+0x24,0x8000+k*4)
            put(p+0x59c,0 if(i+j)%3 else 0x123456)
        glob(0x86eb28,*[TECH+j*0x28 for j in range(23)])
        glob(0x870974,DEVICE);glob(0x870910,0);glob(0x870918,GPOOL,CPOOL);glob(0x870954,IPOOL)
        glob(0x870848,MODE);put(MODE,i%3,(0,1,9,10)[i%4]);g[0x8707cd]=bytes([(i//3)%2])
        glob(0x87092c,0 if i%3 else BUFFER,0 if i%3 else 0x1230)
        glob(0x87094c,0xfffffffe if i%7==0 else 30)
        for a in range(0x872ed8,0x872f08,4):glob(a,0xfffffffe if i%7==0 else i)
        glob(0x8a07a8,*[0x111100+j for j in range(8)]);glob(0x8a1c94,i%8)
        g[0x8a1c9c]=struct.pack('<f',(-7.5,-1,0,0.1,0.5,1,255.99)[i%7]);g[0x802954]=struct.pack('<f',(1,3,4,17)[i%4])
        for j in range(16):
            p,t,tex,s=PLATFORM+j*0x50,TABLE+j*0x80,TEXTURE+j*0x80,STATE+j*0x20
            put(t,p);put(p+0x18,0x30000000+j*128);put(p+0x20,0x1230+j%3)
            put(p+0x28,VERTEX+j*128,0 if(i+j)%3==0 else BUFFER+(j%3)*0x80,(0,1,2,3,4,7,64,127)[(i+j)%8],(0,1,3,17,0xfffe,0xffff,0x10000,0xffffffff)[(i+j)%8])
            put(PRIM+j*0x40+0xc,(0,1,2,3,17,0xffffffff)[(i+j)%6])
            put(PRIM+j*0x40+0x34,0x30000000+j*128+(-7,-4,-1,0,1,4,18)[(i+j)%7])
            put(tex,s);put(tex+0x24,0x111100+(i+j)%8);put(s+0x18,0x9000+j,0)
            data[tex-ARENA+0x51]=(i+j)%8;data[tex-ARENA+0x54]=(i+j)%2
        groups=[];commands=[];immediates=[];headgroups={}
        for j in range(i%5):
            slot=((i*7+(0 if i%6==0 and j==2 else j*137))%368);idx=slot%23;group=GPOOL+0x30+j*0x20;head=0x870d30+slot*8
            groups.append(group);headgroups.setdefault(head,[]).append(group)
            put(group+8,0x111,0x222,TEXTURE+j*0x80,TECH+idx*0x28)
            put(STATE+j*0x20+0x1c,group);cs=[]
            for k in range((i+j)%5):
                cmd=CPOOL+0x30+len(commands)*0x2c;commands.append(cmd);cs.append(cmd);p=(j*4+k)%16
                primitive=PRIM+p*0x40;imm=(i+j+k)%3==0
                if imm:
                    primitive=IPOOL+0x30+len(immediates)*0x64;immediates.append(primitive);put(primitive,(0,1,2,3,4)[(i+k)%5])
                selected=(idx+(1 if(i+k)%7==0 else 0))%23
                put(cmd+8,primitive,(1,0xffffffff)[i%2]if imm else 0,TABLE+p*0x80,
                    (0,1,0x100,0xffffffff)[(i+k)%4],TECH+selected*0x28,
                    RECORD+p*0x200 if(i+k)%2 else 0,LIGHT+p*0x100 if(i+k)%3 else 0,WORLD+p*64,BONES if(i+k)%2 else 0)
            head_=group+0x18;put(head_,cs[0]if cs else head_,cs[-1]if cs else head_)
            for k,cmd in enumerate(cs):put(cmd,cs[k+1]if k+1<len(cs)else head_,cs[k-1]if k else head_)
        for head,gs in headgroups.items():
            glob(head,gs[0],gs[-1])
            for j,group in enumerate(gs):put(group,gs[j+1]if j+1<len(gs)else head,gs[j-1]if j else head)
        # Spare commands exercise both direct and group clear with a nonempty list.
        clear=ARENA+0x3c000;gc=ARENA+0x3c100;nodes=[CPOOL+0x30+(len(commands)+j)*0x2c for j in range(i%4)]
        for head_ in [clear,gc+0x18]:put(head_,nodes[0]if nodes else head_,nodes[-1]if nodes else head_)
        # Separate fixtures use the same nodes with the proper sentinel patched in run().
        for j,node in enumerate(nodes):put(node,nodes[j+1]if j+1<len(nodes)else clear,nodes[j-1]if j else clear)
        for pool,stride,count,used in [(GPOOL,0x20,32,len(groups)),(CPOOL,0x2c,128,len(commands)+len(nodes)),(IPOOL,0x64,32,len(immediates))]:
            put(pool+0x10,pool+0x30+used*stride,0,used,used)
            put(pool+0x24,count,stride,count)
            for j in range(used,count):put(pool+0x30+j*stride,pool+0x30+(j+1)*stride if j+1<count else 0)
        queue=[]
        if i%2:
            for j in range(3):
                p=(i+j)%16;idx=(i+j*5)%23
                queue.append([PRIM+p*0x40,TABLE+p*0x80,1 if j%2 else 0,TECH+idx*0x28,TEXTURE+(10+j)*0x80,WORLD+p*64,RECORD+p*0x200,LIGHT+p*0x100,BONES])
            queue.append(list(queue[0])) # same cached group
            changed=list(queue[1]);changed[3]=TECH+((i+6)%23)*0x28;queue.append(changed) # new technique group
        yield dict(data=bytes(data),globals=g,overlap=(0,0,1,2,3)[i%5],queue=queue,clear=clear,group_clear=gc,clear_nodes=nodes,
            fraction=((0,0x80000000,0x3f800000,0x7f7fffff,0x800000,1,0x7f800000,0xff800000,0x7fc12345,0x7f812345)[i%10],
                (0,0x3f800000,0x40400000,0x7f7fffff,1,0x7fc54321)[(i//10)%6]),
            free=IPOOL+0x30+31*0x64,view=(-1,0,1,9,23)[i%5],cw=(0x027f,0x037f)[(i//60)%2],hresult=(0,0x80004005)[i%2],effect_passes=(0,1,16,0xffffffff)[i%4])


def verify(image,manifest,linked,gcc,fixtures,kinds):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);counts,events=Counter(),Counter()
    for i,c in enumerate(fixtures):
        for k in kinds:
            if k=='group_clear':
                c=dict(c);data=bytearray(c['data']);head=c['group_clear']+0x18
                for j,node in enumerate(c['clear_nodes']):
                    struct.pack_into('<2I',data,node-ARENA,c['clear_nodes'][j+1]if j+1<len(c['clear_nodes'])else head,c['clear_nodes'][j-1]if j else head)
                c['data']=bytes(data)
            try:x,y=a.run(k,c),b.run(k,c)
            except (UcError,AssertionError)as e:raise AssertionError(('consumer mismatch',k,i,str(e)))from e
            if x!=y:
                changes=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[2],y[2]))if v!=w),None)
                raise AssertionError(('consumer mismatch',k,i,changes,event,len(x[2]),len(y[2]),x[3],y[3],
                    [j for j,(v,w)in enumerate(zip(x[1],y[1]))if v!=w]))
            counts[k]+=1;events.update(e[0]for e in x[2])
    return dict(comparisons=dict(counts),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--kind',default='all');p.add_argument('--replace',type=Path);p.add_argument('--replace-pool',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);linked=build(args.replace or SOURCE,args.gcc,args.replace_pool)
    kinds=list(manifest['functions'])if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases,image),kinds)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['controlled D3D/D3DX, technique Begin/End, lookup and six bind/upload boundaries',
        'real recovered queue/pool/free/getter/clear routines execute in chain, including sky fraction through the consumer',
        'finite writable record/graph/buffer fixtures, controlled draw results and successful allocations; no native GPU activation or rendering equivalence']
    if args.mutation_checks:
        mutations={
            'skip-last-row':('consume','row<16','row<15'),
            'skip-last-technique':('consume','index<23','index<22'),
            'skip-detach-command':('consume','Unlink(&command->link00);','(void)0;'),
            'skip-world-upload':('consume','CarRenderUploadWorld(command->world24,ug2SolidViewMode870848[1]);','(void)0;'),
            'wrong-headlight-handle':('consume','slots00[35].handle24','slots00[34].handle24'),
            'ignore-bone-flag':('consume','command->value28 && (command->flags14&1)','command->value28'),
            'wrong-upload-byte-count':('consume','words=solid->vertexBytes30>>2','words=0'),
            'skip-stream-cache':('consume','solid->vertexBuffer2C != ug2RenderVertex87092C','true'),
            'skip-index-cache':('consume','solid->indexBuffer20 != ug2RenderIndex870930','true'),
            'wrong-vertex-limit':('consume','solid->vertexCount34<0xffff','solid->vertexCount34<=0xffff'),
            'wrong-pass-override':('consume','count = 1;','count = 2;'),
            'wrong-sky-predecessor':('consume','? sky-2 : sky-1','? sky-1 : sky-1'),
            'wrong-sky-alpha':('consume','alpha<<24','alpha<<16'),
            'wrong-start-index':('consume','solid->indexAddress18)>>1','solid->indexAddress18)>>2'),
            'wrong-counter-input':('consume','ug2RenderTriangles872EF0 += solid->vertexCount34/3;','ug2RenderTriangles872EF0 += primitive->indexCount0C/3;'),
            'skip-immediate-state-restore':('consume','UG2RenderSetState(ug2SolidDevice870974,14,1);','(void)0;'),
            'skip-cached-group-clear':('consume','group->texture10->state00->group1C = 0;','(void)0;'),
            'skip-context-clear':('consume','ug2RenderTechnique870910 = 0;','(void)0;'),
            'skip-group-count-clear':('consume','ug2SolidGroupCount87094C = 0;','(void)0;'),
            'skip-clear-unlink':('clear','Unlink(command);','(void)0;'),
            'wrong-view-stride':('view','ug2RenderViews832DE0+index','ug2RenderViews832DE0+index*2'),
            'wrong-sky-ratio':('fraction','/ug2RenderSkyPeriod802954','*ug2RenderSkyPeriod802954'),
        }
        rejected=[]
        for name,(k,before,after)in mutations.items():
            if k not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',k,'--cases','256']+(['--gcc']if args.gcc else[])
            r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(r.stdout)
            assert r.returncode and 'consumer mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        if 'free'in kinds:
            name='wrong-member-pool-count';body=(ROOT/'src/Speed/Indep/Libs/bWare/bSlotPool.cpp').read_text()
            before='--used18;';assert before in body
            path=BUILD/('mutation-'+name+'.cpp')
            body=body.replace('#include "bSlotPool.h"','#include "../../../src/Speed/Indep/Libs/bWare/bSlotPool.h"')
            body=body.replace('#include "../../Src/World/WCollision.h"','#include "../../../src/Speed/Indep/Src/World/WCollision.h"')
            path.write_text(body.replace(before,'++used18;'))
            cmd=[sys.executable,__file__,'--replace-pool',str(path),'--kind','free','--cases','8']+(['--gcc']if args.gcc else[])
            r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(r.stdout)
            assert r.returncode and 'consumer mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
