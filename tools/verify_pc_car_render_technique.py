#!/usr/bin/env python3
"""Compare UG2 technique construction, parameter discovery, setup and ramp.

D3D/D3DX and successful allocation/library map operations are controlled runtime
boundaries. Recovered engine calls and real string/hash bodies execute in chain.
No native effect implementation or shader/rendering equivalence is implied.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE

SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderTechnique.cpp'
BUILD=ROOT/'build/analysis/car-render-technique'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-technique.json'
ARENA,SIZE,HEAP,STACK,STOP=0x2000000,0x40000,0x3200000,0x4800000,0x4810000
TECH,PARAM,READY,SEM,NODE,PIXEL=[ARENA+x for x in (0,0x1000,0x3000,0x1c000,0x20000,0x25000)]
DEVICE,EFFECT,SYSTEM,POOL,TEXTURE=[ARENA+x for x in (0x28000,0x28200,0x28400,0x28600,0x28800)]
GLOBALS=dict(ug2SolidDevice870974=0x870974,ug2SolidTechniques86EB28=0x86eb28,
    ug2TechniqueCaps86E5C8=0x86e5c8,ug2TechniqueAdapter86C120=0x86c120,
    ug2TechniqueSystem870970=0x870970,ug2TechniqueAdapter8709AC=0x8709ac,ug2TechniquePool870914=0x870914,
    ug2TechniqueFilter870D18=0x870d18,ug2TechniqueMaximum876530=0x876530,ug2TechniqueSkip870908=0x870908,
    ug2TechniqueReady87090C=0x87090c,ug2TechniqueRecipes7FF7D0=0x7ff7d0,
    ug2TechniqueTypes800494=0x800494,ug2TechniqueUsages8004B4=0x8004b4,
    ug2TechniqueSizes8004E4=0x8004e4,ug2TechniqueIndices8004D8=0x8004d8,
    ug2TechniqueAttempts800BF4=0x800bf4,ug2TechniqueBlend86EAD8=0x86ead8,
    ug2TechniqueRamp86BD18=0x86bd18,ug2TechniqueShader7A4584=0x7a4584)
PREFIX=dict(construct='?Construct@CarRenderTechnique@@',bind='?Bind@CarRenderTechniqueParameters@@',
    cache='?CacheParameters@CarRenderTechnique@@',setup='?Setup@CarRenderTechnique@@',
    mag='?CarRenderTechniqueMagFilter@@',ramp='?CarRenderCreateTechniqueRamp@@',initialise='?CarRenderInitialiseTechniques@@')
GCC=dict(construct='_ZN18CarRenderTechnique9ConstructEv',bind='_ZN28CarRenderTechniqueParameters4BindEjj',
    cache='_ZN18CarRenderTechnique15CacheParametersEv',setup='_ZN18CarRenderTechnique5SetupEi',
    mag='_Z27CarRenderTechniqueMagFilterv',ramp='_Z28CarRenderCreateTechniqueRampv',initialise='_Z29CarRenderInitialiseTechniquesv')
# (source adapter, argument count, original D3D method byte offset, owner)
APIS=[('UG2TechniqueGetCaps',2,0x1c,DEVICE),('UG2TechniqueGetAdapter',4,0x14,SYSTEM),
    ('UG2TechniqueRelease',1,8,None),('UG2TechniqueCreatePool',1,None,None),
    ('UG2TechniqueCreateDeclaration',3,0x158,DEVICE),('UG2TechniqueCreateEffect',9,None,None),
    ('UG2EffectGetDescription',2,0xc,EFFECT),('UG2EffectGetParameter',3,0x20,EFFECT),
    ('UG2EffectGetParameterDescription',3,0x10,EFFECT),('UG2EffectGetTechnique',2,0x30,EFFECT),
    ('UG2EffectGetAnnotation',3,0x4c,EFFECT),('UG2EffectGetInt',3,0x6c,EFFECT),
    ('UG2EffectSetInt',3,0x68,EFFECT),('UG2EffectSetIntArray',4,0x70,EFFECT),
    ('UG2EffectValidate',2,0xf8,EFFECT),('UG2EffectSetTechnique',2,0xf0,EFFECT),
    ('UG2EffectGetPass',3,0x38,EFFECT),('UG2EffectSetFloat',3,0x78,EFFECT),
    ('UG2TextureCreate',9,0x5c,DEVICE),('UG2TextureLock',5,0x4c,TEXTURE),('UG2TextureUnlock',2,0x50,TEXTURE)]
API={name:STOP+0x1000+i*16 for i,(name,*_)in enumerate(APIS)}
LIB=dict(malloc=0x575620,map=0x5d0bb0,insert=0x5d0cf0,reset=0x5d0f20)
LIB_GCC=dict(map='_ZN21CarRenderTechniqueMap9Construct',insert='_ZN21CarRenderTechniqueMap6Insert',
    reset='_ZN28CarRenderTechniqueParameters5ResetEv')
LIB_PREFIX=dict(map='?Construct@CarRenderTechniqueMap@@',insert='?Insert@CarRenderTechniqueMap@@',reset='?Reset@CarRenderTechniqueParameters@@')
NAMES={int(i):s for i,s in re.findall(r'\{(\d+),"([^"]+)"\}',SOURCE.read_text())}
REGIONS=[(0x86e5c8,0x130),(0x86c120,0x450),(0x870970,8),(0x8709ac,4),(0x870914,4),
    (0x870d18,4),(0x876530,4),(0x870908,5),(0x86eb28,23*4),(0x86ead8,20),(0x86bd18,4),(0x800bf4,4)]


def build(source,gcc):
    bindings=dict(GLOBALS,**API,UG2TextureMalloc=LIB['malloc'],UG2TextureFree=0x575630,sqrtl=STOP+0x900)
    sources=[source,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if gcc:
        # Resolve only the three explicitly unrecovered library operations.
        subprocess.run(['g++','-std=c++11','-m32','-O1','-fno-pie','-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),
            '-Isrc/Speed/Indep/Src/World','-c',str(source),'-o',str(BUILD/'names.o')],cwd=ROOT,check=True)
        for line in subprocess.check_output(['nm','-u',str(BUILD/'names.o')],text=True).splitlines():
            for k,p in LIB_GCC.items():
                if p in line:bindings[line.split()[-1]]=LIB[k]
        linked=link_native(sources,BUILD,ROOT,bindings,GCC['construct'],text_address=0x5000000)
    else:
        objects=[];bindings={'_'+k:v for k,v in bindings.items()};bindings.update(__fltused=0,__ftol2=0x75bed4)
        for i,path in enumerate(sources):
            obj=BUILD/('technique-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
            for s in read_object(obj)['symbols'].values():
                for k,p in LIB_PREFIX.items():
                    if s['section']==0 and s['name'].startswith(p):bindings[s['name']]=LIB[k]
        linked=link_objects(objects,bindings,start=0x5000000)
    linked['entries']={}
    for k,p in (GCC if gcc else PREFIX).items():
        hits=[v for s,v in linked['exports'].items()if s.startswith(p)];assert len(hits)==1,(k,p,hits)
        linked['entries'][k]=hits[0]
    return linked


def hash_(s):
    h=0xffffffff
    for c in s.encode():h=(h*33+c)&0xffffffff
    return h


class Machine:
    def __init__(self,image,manifest,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(0,0x1000);u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        u.mem_map(ARENA,SIZE);u.mem_map(HEAP,0x100000);u.mem_map(STACK,0x20000)
        self.gcc,self.rebuilt=gcc,bool(linked)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        self.entries=linked['entries']if linked else{k:int(f['address'],16)for k,f in manifest['functions'].items()}
        for k,a in LIB.items():u.hook_add(UC_HOOK_CODE,self.library,user_data=k,begin=a,end=a)
        for name,n,offset,owner in APIS:
            a=API[name]if linked or offset is not None else(0x640aac if name.endswith('CreatePool')else 0x640c02)
            u.hook_add(UC_HOOK_CODE,self.api,user_data=(name,n),begin=a,end=a)
        u.hook_add(UC_HOOK_CODE,self.unexpected,begin=STOP+0x900,end=STOP+0x900)
    def unexpected(self,*args):raise AssertionError('unexpected sqrtl fallback')
    def word(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
    def put(self,a,*v):self.u.mem_write(a,struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v)))
    def args(self,n):return struct.unpack('<'+'I'*n,self.u.mem_read(self.u.reg_read(xc.UC_X86_REG_ESP)+4,n*4))
    def member(self,n):
        if self.gcc:
            a=self.args(n+1);return a[0],a[1:]
        return self.u.reg_read(xc.UC_X86_REG_ECX),self.args(n)
    def ret(self,result=0,pop=0):
        u=self.u;sp=u.reg_read(xc.UC_X86_REG_ESP);pc=self.word(sp)
        u.reg_write(xc.UC_X86_REG_EAX,result&0xffffffff);u.reg_write(xc.UC_X86_REG_ESP,sp+4+pop);u.reg_write(xc.UC_X86_REG_EIP,pc)
    def string(self,a):
        if not a:return None
        b=bytearray()
        while self.u.mem_read(a+len(b),1)!=b'\0':b+=self.u.mem_read(a+len(b),1)
        return bytes(b)
    def library(self,u,address,size,k):
        pop=0;result=0
        if k=='malloc':
            n,=self.args(1);result=HEAP+self.allocated;self.allocated+=(n+15)&~15
            self.events.append((k,n,result));u.mem_write(result,b'\x93'*n)
        else:
            receiver,a=self.member(2 if k in ('map','insert')else 0)
            if k=='map':
                assert a[0]==a[1] # opaque empty library tags; deliberately do not read padding.
                u.mem_write(receiver,b'\xa7'*0x28);result=receiver
                self.events.append((k,receiver))
            elif k=='reset':
                self.events.append((k,receiver));u.mem_write(receiver+0x960,b'\xa7'*0x28)
                if self.case['mutate']:
                    self.put(0x86c548,self.case['fresh_vendor'])
            else:
                out,pair=a;h=self.word(pair);assert self.word(pair+4)==0
                key=(receiver,h)
                inserted=key not in self.nodes
                if inserted:self.nodes[key]=NODE+len(self.nodes)*16;self.put(self.nodes[key],0,0,h,0)
                node=self.nodes[key];self.put(out,node);u.mem_write(out+4,bytes([inserted]))
                # The engine static slot must be written before entering the map.
                self.events.append((k,receiver,h,bytes(u.mem_read(receiver-0x960,0x960))))
                result=out
            if not self.gcc:pop=len(a)*4
        self.ret(result,pop)
    def api(self,u,address,size,info):
        name,n=info;a=self.args(n);short=name.removeprefix('UG2Technique').removeprefix('UG2Effect').removeprefix('UG2Texture')
        c=self.case;result=c['hresult']
        # COM/stdcall boundaries pop all arguments; rebuilt adapters use cdecl.
        pop=0 if self.rebuilt else n*4
        event=(short,*a)
        if name=='UG2TechniqueGetCaps':
            assert a==(DEVICE,0x86e5c8)
            flags=c['caps'][min(self.caps_calls,1)];self.caps_calls+=1
            self.put(a[1]+0x40,flags);self.put(a[1]+0x6c,c['anisotropy']);event=(short,flags)
        elif name=='UG2TechniqueGetAdapter':
            assert a==(SYSTEM,c['adapter'],2,0x86c120);self.put(a[3]+0x428,c['vendor'],c['device']);event=(short,c['adapter'],2)
        elif name=='UG2TechniqueRelease':
            assert a[0]in(EFFECT,POOL);result=7
        elif name=='UG2TechniqueCreatePool':
            assert a==(0x870914,);event=(short,self.word(a[0]));self.put(a[0],POOL)
        elif name=='UG2TechniqueCreateDeclaration':
            assert a[0]==DEVICE
            declaration=bytearray()
            for i in range(16):
                b=bytes(u.mem_read(a[1]+i*8,8));declaration+=b
                if struct.unpack_from('<H',b)[0]==0xff:break
            else:raise AssertionError('missing declaration sentinel')
            self.current=a[2]-0x24;event=(short,bytes(declaration),bytes(u.mem_read(self.current,0x28)))
            self.put(a[2],0x12345678)
        elif name=='UG2TechniqueCreateEffect':
            assert a[0:2]==(DEVICE,0)and a[3:6]==(0,0,0)and a[6]==POOL
            event=(short,self.string(a[2]),a[6]);self.put(a[7],EFFECT);self.put(a[8],0xabcdef01)
        elif name=='UG2EffectGetDescription':
            assert a[0]==EFFECT;self.put(a[1],SEM,c['parameter_count'],0xabcd,0x9876);event=(short,c['parameter_count'])
        elif name=='UG2EffectGetParameter':
            assert a[0:2]==(EFFECT,0);result=0x1000+a[2]*4
        elif name=='UG2EffectGetParameterDescription':
            assert a[0]==EFFECT;i=(a[1]-0x1000)//4
            semantic=c['semantics'][i];self.put(a[2],SEM+((i+1)%60)*32,SEM+semantic*32 if semantic is not None else 0)
            u.mem_write(a[2]+8,b'\x87'*36);event=(short,a[1],semantic)
        elif name=='UG2EffectGetTechnique':
            assert a[0]==EFFECT;result=0 if a[1]in c['missing']else 0x5000+a[1]*4
        elif name=='UG2EffectGetAnnotation':
            assert a[0]==EFFECT and self.string(a[2])==b'shader'
            i=(a[1]-0x5000)//4;result=0x7000+i*4 if i%4!=3 else 0;event=(short,a[1],b'shader')
        elif name=='UG2EffectGetInt':
            assert a[0]==EFFECT;i=(a[1]-0x7000)//4;self.put(a[2],c['shaders'][i%len(c['shaders'])]);event=(short,a[1])
        elif name=='UG2EffectSetIntArray':
            assert a[0]==EFFECT and a[3]==5;event=(short,a[1],bytes(u.mem_read(a[2],20)))
        elif name=='UG2EffectValidate':
            assert a[0]==EFFECT;result=0 if (a[1]-0x5000)//4==c['accept']else 0x80004005
        elif name=='UG2EffectSetTechnique':
            assert a[0]==EFFECT
            if c['change_type']:self.put(self.current+4,9 if self.word(self.current+4)!=9 else 0)
        elif name=='UG2EffectGetPass':
            assert a[0]==EFFECT;result=0x9000+a[2]*4 if a[2]<c['passes']else 0
        elif name in ('UG2EffectSetInt','UG2EffectSetFloat'):assert a[0]==EFFECT
        elif name=='UG2TextureCreate':
            assert a==(DEVICE,64,1,0,0,21,1,0x86bd18,0);self.put(a[7],TEXTURE)
        elif name=='UG2TextureLock':
            assert a[0:2]==(TEXTURE,0)and a[3:]==(0,0);self.put(a[2],c['pitch'],PIXEL);event=(short,TEXTURE,0,c['pitch'])
        elif name=='UG2TextureUnlock':assert a==(TEXTURE,0)
        else:raise AssertionError(name)
        self.events.append(event);self.ret(result,pop)
    def run(self,kind,c):
        self.case=c;self.events=[];self.nodes={};self.allocated=0;self.caps_calls=0;self.current=TECH
        u=self.u;u.mem_write(ARENA,c['data']);u.mem_write(HEAP,b'\x93'*0x20000)
        for a,b in c['globals'].items():u.mem_write(a,b)
        self.put(0,0xffffffff)
        if kind=='cache':self.put(TECH+0x20,EFFECT)
        if kind=='mag':self.put(0x870d18,c['mag_filter'])
        for token in (DEVICE,EFFECT,SYSTEM,POOL,TEXTURE):self.put(token,token+0x100)
        for name,n,offset,owner in APIS:
            if offset is not None:
                for token in ([owner]if owner else[EFFECT,POOL]):self.put(token+0x100+offset,API[name])
        args=[];receiver=TECH
        if kind=='bind':receiver=PARAM;args=[c['hash'],c['handle']]
        elif kind=='setup':args=[c['index']]
        if kind in ('construct','bind','cache','setup'):
            if self.gcc:args=[receiver]+args
        sp=STACK+0x10000-0x100;self.put(sp,STOP,*args)
        for r,v in [(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),
            (xc.UC_X86_REG_EBX,0x1248),(xc.UC_X86_REG_ESI,0x1357),(xc.UC_X86_REG_EDI,0x2468),(xc.UC_X86_REG_EBP,0x3579),
            (xc.UC_X86_REG_EFLAGS,2),(xc.UC_X86_REG_FPCW,c['cw']),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        try:u.emu_start(self.entries[kind],STOP,count=5000000)
        except UcError as e:raise AssertionError((str(e),hex(u.reg_read(xc.UC_X86_REG_EIP)),self.events[-3:]))from e
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,(kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        pop=8 if kind=='bind'else 4 if kind=='setup'else 0
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+(0 if self.gcc else pop)
        assert self.word(0)==0xffffffff
        for r,v in [(xc.UC_X86_REG_EBX,0x1248),(xc.UC_X86_REG_ESI,0x1357),(xc.UC_X86_REG_EDI,0x2468),(xc.UC_X86_REG_EBP,0x3579)]:assert u.reg_read(r)==v
        assert u.reg_read(xc.UC_X86_REG_FPCW)==c['cw']
        result=u.reg_read(xc.UC_X86_REG_EAX)if kind in ('construct','mag')else None
        return (bytes(u.mem_read(ARENA,SIZE)),bytes(u.mem_read(HEAP,self.allocated)),
            tuple(bytes(u.mem_read(a,n))for a,n in REGIONS),self.events,result)


def cases(n,image):
    rng=random.Random(0x5d21f0)
    recipes=bytes(image[0x7ff7d0-0x400000:0x800504-0x400000])
    for i in range(n):
        data=bytearray(rng.randbytes(SIZE));g={a:bytes(image[a-0x400000:a-0x400000+n])for a,n in REGIONS}
        def put(a,*v):struct.pack_into('<'+'I'*len(v),data,a-ARENA,*(x&0xffffffff for x in v))
        def glob(a,*v):g[a]=struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v))
        # Names/semantics include deliberately different display names and unknown semantic.
        for j in range(61):
            s=(NAMES[j]if j<60 else 'NOT_A_PARAMETER').encode()+b'\0';data[SEM-ARENA+j*32:SEM-ARENA+j*32+len(s)]=s
        for p in [PARAM]+[ARENA+0x4000+j*0x1000 for j in range(23)]:
            for j in range(60):
                s=NAMES[j].encode()+b'\0';data[p-ARENA+j*40:p-ARENA+j*40+len(s)]=s
                put(p+j*40+0x20,hash_(NAMES[j]),(0xa000+j*4)if(i+j)%4 else 0)
            if i%3==0:put(p+59*40+0x20,hash_(NAMES[0])) # duplicate: first slot wins
        for j,t in enumerate([TECH]+[READY+j*0x28 for j in range(23)]):
            p=PARAM if j==0 else ARENA+0x4000+(j-1)*0x1000
            put(t+0x1c,p,EFFECT if i%2 else 0,0xabcde)
        glob(0x86eb28,*[READY+j*0x28 for j in range(23)])
        glob(0x870974,DEVICE);glob(0x870970,SYSTEM);glob(0x870914,POOL)
        glob(0x8709ac,i%4);glob(0x800bf4,(-1,0,1,3,4,8,20)[i%7])
        glob(0x870908,(0,1,-1,2)[i%4]);g[0x87090c]=bytes([i%2])
        glob(0x870d18,(0,1,2)[i%3]);glob(0x86c548,(0x1039,0x8086,0x102b,0x10de,0)[i%5],0x2572 if i%2 else 0)
        glob(0x86e608,(0,0x400,0x02020200,0x02020600,0x04000000,0xffffffff)[i%6]);glob(0x86e634,i%17)
        g[0x7ff7d0]=recipes
        # Bounded declaration permutations, including uint stride wrap and ushort truncation.
        if i%4==0:
            rows=bytearray(recipes);idx=i%23;count=(0,1,7,15)[(i//4)%4]
            for j in range(count):struct.pack_into('<2I',rows,idx*0x8c+j*8,(i+j)%8,(i+j)%9)
            struct.pack_into('<2I',rows,idx*0x8c+count*8,8,0);g[0x7ff7d0]=bytes(rows)
            g[0x8004e4]=struct.pack('<8I',0xffffffff,0x10001,12,16,4,0,0,0)
        c=dict(data=bytes(data),globals=g,index=i%23,hash=hash_(NAMES[(0,59,3,28)[i%4]])if i%5 else 0xdeadbeef,
            handle=(0,1,0x87654321,0xffffffff)[i%4],parameter_count=(0,1,7,60,65)[i%5],
            semantics=[None if(i+j)%7==0 else 60 if(i+j)%11==0 else (i+j)%60 for j in range(65)],
            missing=set(j for j in range(21)if(i+j)%5==0),shaders=[(-1,0,1,2)[(i+j)%4]for j in range(4)],accept=(-1,0,1,2,3,7,19)[(i//7)%7],
            passes=(0,1,2,15,16,17,24)[i%7],mutate=i%3==0,fresh_vendor=(0x1039,0x102b,0x8086)[i%3],
            change_type=i%6==0,vendor=(0x1039,0x8086,0x102b,0x10de,0)[i%5],device=0x2572 if i%2 else 0,
            caps=((0,0x400,0x02020200,0x02020600,0x02000000,0x200,0x20000,0x02000200,0x02020000,0x20200,0xffffffff)[i%11],(0x400,0x04000000,0,0xffffffff)[i%4]),
            mag_filter=(-0x80000000,-1,0,1,2,3,0x7fffffff)[i%7],anisotropy=i%17,adapter=i%4,hresult=(0,0x80004005)[i%2],pitch=(256,512,-256)[i%3],cw=(0x027f,0x037f)[i%2])
        yield c


def verify(image,manifest,linked,gcc,fixtures,kinds,root_cases):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);counts,events=Counter(),Counter()
    for i,c in enumerate(fixtures):
        for k in kinds:
            if k=='initialise'and i>=root_cases:continue
            try:x,y=a.run(k,c),b.run(k,c)
            except (UcError,AssertionError)as e:raise AssertionError(('technique mismatch',k,i,str(e)))from e
            if x!=y:
                changes=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('technique mismatch',k,i,changes,event,len(x[3]),len(y[3]),x[4],y[4],
                    [j for j,(v,w)in enumerate(zip(x[2],y[2]))if v!=w],x[1][:32],y[1][:32]))
            counts[k]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--root-cases',type=int,default=128);p.add_argument('--kind',default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args();assert args.cases>0 and args.root_cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);linked=build(args.replace or SOURCE,args.gcc)
    kinds=list(manifest['functions'])if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases,image),kinds,args.root_cases)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['D3D/D3DX and successful allocation are controlled runtime boundaries; HRESULT failures still write valid outputs',
        'library map construction/insertion/reset are controlled boundaries, including opaque empty allocator/comparator tags',
        'real string copy/hash and recovered engine calls execute; normal-return ABI/FS state checked, no exception/OOM equivalence',
        'finite legal recipes/semantics/capabilities; minification selector outside 0..2 with a live handle is undefined and excluded',
        'no native effect backend, shader execution or rendering equivalence established']
    if args.mutation_checks:
        mutations={
            'wrong-semantic':('cache','cshash(parameterDescription.semantic04)','cshash(parameterDescription.name00)'),
            'skip-static-handle':('bind','slots00[i].handle24 = handle;','(void)0;'),
            'last-duplicate':('bind','for (int i=0;i<60;++i)','for (int i=59;i>=0;--i)'),
            'skip-map-value':('bind','map960.Insert(&result,&pair)->node00->value0C = handle;','map960.Insert(&result,&pair);'),
            'wrong-name':('construct','"ENVMAPANGE"','"ENVMAPRANGE"'),
            'wrong-mag-capability':('mag','&0x04000000','&0x02000000'),
            'wrong-declaration-index':('setup','element.index07 = ug2TechniqueIndices8004D8[usage];','element.index07 = 0;'),
            'wrong-stride':('setup','stride += ug2TechniqueSizes8004E4[type];','stride += 4;'),
            'wrong-shader-skip':('setup','shader == 1','shader != 0'),
            'wrong-snapshot-vendor':('setup','vendor == 0x1039 ||','ReadWord(ug2TechniqueAdapter86C120,0x428) == 0x1039 ||'),
            'wrong-pass-limit':('setup','passes<16','passes<15'),
            'wrong-focal-test':('setup','if (type04 == 9)','if (index == 9)'),
            'wrong-ramp-byte':('ramp','pixels[i] = 0;','pixels[i] = 1;'),
            'wrong-ramp-range':('ramp','i<48','i<47'),
            'wrong-class-token':('initialise','case 5: token = 0x7a4560;','case 5: token = 0x7a4578;'),
            'wrong-cap-clamp':('initialise','maximum = 1;','maximum = 0;'),
            'skip-ready-gate':('initialise','if (!ug2TechniqueReady87090C)','if (true)'),
        }
        rejected=[]
        for name,(k,before,after)in mutations.items():
            if k not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',k,'--cases','128','--root-cases','16']+(['--gcc']if args.gcc else[])
            r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(r.stdout)
            assert r.returncode and 'technique mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
