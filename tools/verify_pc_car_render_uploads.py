#!/usr/bin/env python3
"""Compare world/bone/headlight uploads, technique methods and their consumer chain.

Original scalar D3DX matrix code executes on both sides. Recovered identity,
rigid inverse, hash, view/sky, queues and pools execute in the rebuilt chain.
Only GPU APIs, texture lookup, three pending engine bodies and map destruction
remain controlled boundaries; no effect/shader/GPU equivalence is claimed.
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
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
import verify_pc_car_render_consume as core

ROOT,EXE=core.ROOT,core.EXE
WORLD=ROOT/'src/Speed/Indep/Src/World'
UPLOADS=WORLD/'CarRenderUploads.cpp'
RUNTIME=WORLD/'CarRenderTechniqueRuntime.cpp'
BRIDGE=ROOT/'src/Platform/Native/NativeTechniqueDispatch.cpp'
BUILD=ROOT/'build/analysis/car-render-uploads'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-uploads.json'
ARENA,STOP=core.ARENA,core.STOP
OWNER,VALUE,CONTEXT,VIEW_MATRICES=[ARENA+x for x in (0x24000,0x24800,0x24c00,0x36000)]
GLOBALS=dict(ug2RenderLastWorld876850=0x876850,ug2RenderLastTechnique87684C=0x87684c,ug2RenderLastView800534=0x800534,
    ug2RenderWorldCalls872EF4=0x872ef4,ug2RenderWorldChanges872EF8=0x872ef8,ug2RenderViewChanges872EFC=0x872efc,
    ug2RenderIdentityCalls872F00=0x872f00,ug2RenderIdentity872E90=0x872e90,
    ug2RenderHeadlightBias80052C=0x80052c,ug2RenderHeadlightBias800530=0x800530,ug2RenderClipScale79BFC0=0x79bfc0,
    ug2RenderHeadlightContext8900B4=0x8900b4,ug2RenderTexture870894=0x870894,
    ug2RenderTexture870858=0x870858,ug2RenderTexture87088C=0x87088c,ug2RenderMode8654A4=0x8654a4,
    ug2RenderColour800520=0x800520,ug2RenderReflectionMinimum7A27E4=0x7a27e4,ug2RenderHeadlights7A3658=0x7a3658)
EXTRA_APIS=[('UG2RenderSetTransform',3,0xb0,'device'),('UG2RenderSetTextureStageState',4,0x10c,'device'),
    ('UG2EffectSetMatrix',3,0x98,'effect'),('UG2EffectSetMatrixArray',4,0xa0,'effect'),
    ('UG2EffectSetVector',3,0x88,'effect'),('UG2TechniqueRelease',1,8,'effect')]
for i,entry in enumerate(EXTRA_APIS):
    core.APIS.append(entry);core.API[entry[0]]=STOP+0x3000+i*16
METHODS=dict(begin_headlight=('CarRenderTechnique','BeginHeadlight'),begin_reflection=('CarRenderTechnique','BeginReflection'),
    begin_combined=('CarRenderTechnique','BeginHeadlightReflection'),begin_car=('CarRenderTechnique','BeginCar'),
    begin_filter=('CarRenderTechnique','BeginFilter'),empty=('CarRenderTechnique','End'),destroy=('CarRenderTechnique','Destroy'),
    delete=('CarRenderTechnique','Delete'),delete_base=('CarRenderTechnique','DeleteBase'),parameters_destroy=('CarRenderTechniqueParameters','Destroy'))
FUNCTIONS=dict(world='CarRenderUploadWorld',bones='CarRenderUploadBones',headlights='CarRenderUploadHeadlights',colour='CarRenderHeadlightColour')
PREFIX={k:'?'+method+'@'+cls+'@@'for k,(cls,method)in METHODS.items()}
PREFIX.update({k:'?'+name+'@@'for k,name in FUNCTIONS.items()})
GCC={k:'_ZN'+str(len(cls))+cls+str(len(method))+method for k,(cls,method)in METHODS.items()}
GCC.update({k:'_Z'+str(len(name))+name for k,name in FUNCTIONS.items()})
core.MEMBERS.update(METHODS);core.RETURNS.update(('colour','delete','delete_base'))
REGIONS=[(0x87684c,8),(0x800534,4),(0x832de0,24*0x70)]
COVERED={'world','bones','headlights'}
TOKENS=[0x7a4578,0x7a453c,0x7a4548,0x7a4554,0x7a4578,0x7a4560,0x7a4578,0x7a4578,0x7a4578,0x7a4578,
    0x7a453c,0x7a4548,0x7a4554,0x7a4578,0x7a453c,0x7a456c,0x7a456c,0x7a4578,0x7a456c,0x7a4578,0x7a4578,0x7a4578,0x7a4578]


def build(uploads,runtime,bridge,gcc):
    core.BUILD=BUILD
    extra=[uploads,runtime,bridge,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp',
        WORLD/'../Physics/WheelMath.cpp',WORLD/'../Camera/CameraMatrix.cpp']
    native=dict(D3DXMatrixMultiply=0x63cedb,D3DXMatrixTranspose=0x63d12c,fengZero7D7D3C=0x7d7d3c,fengOne784250=0x784250,
        sqrtl=STOP+0x900,sqrt=STOP+0x900,abort=STOP+0x900,UG2TextureFree=0x575630)
    gcc_extra={'_ZN21CarRenderTechniqueMap7DestroyEv':0x5d0660}
    msvc_extra={'?Destroy@CarRenderTechniqueMap@@QAEXXZ':0x5d0660,'_D3DXMatrixMultiply@12':0x63cedb,'_D3DXMatrixTranspose@8':0x63d12c,
        '?fengZero7D7D3C@@3MA':0x7d7d3c,'?fengOne784250@@3MA':0x784250}
    cpp=dict(BodyMatrixCopy=0x4012b0,WheelSineCosine=0x43c390,EngineSine=0x43c350)
    linked=core.build(core.SOURCE,gcc,extra_sources=extra,extra_globals=dict(GLOBALS,**native),
        supplied=[FUNCTIONS[k]for k in COVERED]+['UG2RenderTechniqueBegin','UG2RenderTechniqueEnd'],
        gcc_bindings=gcc_extra,msvc_bindings=msvc_extra,extra_cpp=cpp)
    for k,p in (GCC if gcc else PREFIX).items():
        hits=[a for s,a in linked['exports'].items()if s.startswith(p)];assert len(hits)==1,(k,p,hits)
        linked['entries'][k]=hits[0]
    return linked


class Machine(core.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        super().__init__(image,manifest,linked,gcc)
        u=self.u
        for kind in METHODS:
            if kind=='parameters_destroy':continue # GCC folds this pure forwarding wrapper.
            a=self.entries[kind];u.hook_add(UC_HOOK_CODE,self.observe,user_data=kind,begin=a,end=a)
        if linked:
            for k in COVERED:
                a=self.entries[k];u.hook_add(UC_HOOK_CODE,self.observe,user_data=k,begin=a,end=a)
        a=self.entries['colour'];u.hook_add(UC_HOOK_CODE,self.observe,user_data='colour',begin=a,end=a)
        for a,k in [(0x575630,'heap-free'),(0x5d0660,'map-destroy'),(0x63cedb,'multiply'),(STOP+0x900,'unexpected')]:
            u.hook_add(UC_HOOK_CODE,self.library,user_data=k,begin=a,end=a)
    def helper(self,u,address,size,info):
        if info[0]in COVERED:self.observe(u,address,size,info[0])
        else:super().helper(u,address,size,info)
    def observe(self,u,address,size,k):
        if k in COVERED:self.events.append((k,self.args(2),self.word(0x870910)))
        elif k=='colour':self.events.append((k,self.args(1)))
        else:
            receiver,args=self.member(1 if k in ('delete','delete_base')else 0)
            if args:args=(args[0]&255,)
            self.events.append((k,receiver,args,self.word(0x870910)))
    def library(self,u,address,size,k):
        if k=='multiply':
            out,a,b=self.args(3)
            self.events.append((k,bytes(u.mem_read(a,64)),bytes(u.mem_read(b,64)),out==a,out==b))
            return # actual original D3DX scalar body executes
        if k=='unexpected':raise AssertionError('unexpected abort/sqrt fallback')
        if k=='map-destroy':
            receiver,args=self.member(0);self.events.append((k,receiver,bytes(u.mem_read(receiver,0x28))))
            u.mem_write(receiver,b'\x5a'*0x28)
        else:
            pointer,=self.args(1)
            assert pointer==core.TECH or pointer==core.PARAM,(k,hex(pointer))
            n=0x28 if pointer==core.TECH else 0x988
            self.events.append((k,pointer,bytes(u.mem_read(pointer,n))));u.mem_write(pointer,b'\xdd'*n)
        self.ret()
    def lookup(self,u,address,size,unused):
        a=self.args(3);assert a[1:]in ((0,0),(1,0))
        self.events.append(('lookup',a));self.ret(core.TEXTURE+15*0x80)
    def api(self,u,address,size,info):
        name,n=info
        if name not in {x[0]for x in EXTRA_APIS}:return super().api(u,address,size,info)
        a=self.args(n)
        if name in ('UG2RenderSetTransform','UG2RenderSetTextureStageState'):assert a[0]==core.DEVICE
        else:assert core.EFFECT<=a[0]<core.EFFECT+23*0x200 and (a[0]-core.EFFECT)%0x200==0
        event=(name,*a)
        if name=='UG2RenderSetTransform':
            event=(name,a[0],a[1],bytes(u.mem_read(a[2],64)))
            if a[1]==0x100 and self.case['flip_world']:
                t=self.word(0x870910);u.mem_write(t+0x18,b'\x01')
        elif name=='UG2EffectSetMatrix':event=(name,a[0],a[1],bytes(u.mem_read(a[2],64)))
        elif name=='UG2EffectSetMatrixArray':
            assert a[3]==16;event=(name,a[0],a[1],bytes(u.mem_read(a[2],a[3]*64)),a[3])
        elif name=='UG2EffectSetVector':event=(name,a[0],a[1],bytes(u.mem_read(a[2],16)))
        elif name=='UG2TechniqueRelease':event=(name,a[0],bytes(u.mem_read(core.TECH,0x28)))
        event+=(tuple(self.word(a)for a in (0x87684c,0x876850,0x800534,0x872ef4,0x872ef8,0x872efc)),)
        self.events.append(event);self.ret(self.case['hresult'],0 if self.rebuilt else n*4)
    def prepare(self,kind,c):
        for j,t in enumerate(TOKENS):self.put(core.TECH+j*0x28,t)
        self.put(0x803e74,0x63cee1);self.put(0x803f00,0x63d132)
        if kind!='consume':self.put(0x870910,core.TECH)
        # Direct calls exercise a receiver distinct from the current technique.
        if kind in ('destroy','delete','delete_base'):
            self.put(core.TECH+0x1c,core.PARAM if c['parameters_present']else 0,
                core.EFFECT if c['effect_present']else 0)
    def snapshot(self,result):
        value=list(super().snapshot(result));value.append(tuple(bytes(self.u.mem_read(a,n))for a,n in REGIONS));return tuple(value)
    def run(self,kind,c):
        if kind=='consume':return super().run(kind,c)
        self.reset(kind,c)
        receiver=core.TECH+0x28 if c['other_receiver']else core.TECH
        if kind in ('destroy','delete','delete_base'):receiver=core.TECH
        if kind in ('delete','delete_base'):result=self.call(kind,[c['delete_flags']],receiver)
        elif kind in METHODS:result=self.call(kind,receiver=core.PARAM if kind=='parameters_destroy'else receiver)
        elif kind=='world':
            world=0x872e90 if c['identity_world']else core.WORLD
            result=self.call(kind,[world,c['view_index']])
            if c['repeat_world']:self.call(kind,[world,c['view_index']])
        elif kind=='bones':result=self.call(kind,[core.BONES,core.PRIM])
        elif kind=='headlights':result=self.call(kind,[core.WORLD,0x832de0+9*0x70])
        elif kind=='colour':result=self.call(kind,[c['colour_owner']])
        else:raise AssertionError(kind)
        return self.snapshot(result)


def cases(n,image):
    rng=random.Random(0x5bdde0)
    for i,c in enumerate(core.cases(n,image)):
        data=bytearray(c['data']);g=c['globals']
        def put(a,*v):struct.pack_into('<'+'I'*len(v),data,a-ARENA,*(x&0xffffffff for x in v))
        def glob(a,*v):g[a]=struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v))
        def floats(a,values):struct.pack_into('<'+'f'*len(values),data,a-ARENA,*values)
        for j in range(16):floats(core.WORLD+j*64,[rng.uniform(-8,8)for _ in range(16)])
        for j in range(24):
            glob(0x832de0+j*0x70,VIEW_MATRICES+j*0xc0)
            floats(VIEW_MATRICES+j*0xc0,[rng.uniform(-4,4)for _ in range(48)])
        for j in range(23):
            p=core.PARAM+j*0xa00
            for k in range(60):
                if (i+j+k)%7==0:put(p+k*40+0x24,0)
            data[core.TECH+j*0x28-ARENA+0x18]=(i+j)%3
        glob(0x87684c,core.TECH if i%3 else core.TECH+0x28,core.WORLD if i%4 else 0x872e90)
        glob(0x800534,(-1,0,1,9,10)[i%5]);glob(0x86bd18,0 if i%17==0 else 0x5678)
        glob(0x870894,0 if i%11==0 else 0x12340000+i);glob(0x870858,0 if i%13==0 else 0xabc00000+i);glob(0x87088c,0 if i%23==0 else 0xdef00000+i)
        put(core.STATE+15*0x20+0x18,0 if i%19==0 else 0x900f)
        glob(0x8654a4,(0,5,6,7,-1)[i%5])
        g[0x800520]=struct.pack('<3f',*((-1,0,0.1,0.5,0.7,0.8,0.9,1,1.1,256.5)[(i+j)%10]for j in range(3)))
        g[0x80052c]=struct.pack('<2f',(-1,0,0.25,0.5,1)[i%5],(0,0.5,2)[i%3])
        g[0x79bfc0]=struct.pack('<f',(0.005,0.5,-1,0.125)[i%4]);g[0x7a27e4]=struct.pack('<f',(0.1,0.25,-2)[i%3])
        g[0x872e90]=struct.pack('<16f',*[1.0 if j%5==0 else 0.0 for j in range(16)])
        put(OWNER+0x554,VALUE if i%3 else 0);put(VALUE+0x380,(0,0xffffffff,0xff0080ff,0x12345678,0x80112233)[i%5])
        put(CONTEXT,0,OWNER if (i//3)%3 else 0);glob(0x8900b4,CONTEXT if (i//9)%4 else 0)
        c.update(data=bytes(data),globals=g,view_index=(0,1,9,10)[i%4],identity_world=i%7==0,repeat_world=i%2==0,
            other_receiver=(i//3)%2!=0,flip_world=i%8==0,parameters_present=(i//32)%3!=0,effect_present=(i//8)%4!=0,
            delete_flags=(0,1,2,3,0x100,0x101,0xff,0xffffffff)[i%8],colour_owner=OWNER if i%4 else 0)
        yield c


def verify(image,manifest,linked,gcc,fixtures,kinds,chain_cases):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);counts,events=Counter(),Counter()
    for i,c in enumerate(fixtures):
        for k in kinds:
            if k=='consume'and i>=chain_cases:continue
            try:x,y=a.run(k,c),b.run(k,c)
            except AssertionError as e:raise AssertionError(('upload mismatch',k,i,str(e)))from e
            if x!=y:
                changes=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[2],y[2]))if v!=w),None)
                raise AssertionError(('upload mismatch',k,i,changes,event,len(x[2]),len(y[2]),x[3],y[3],
                    [j for j,(v,w)in enumerate(zip(x[1],y[1]))if v!=w],x[4]!=y[4]))
            counts[k]+=1;events.update(e[0]for e in x[2])
    return dict(comparisons=dict(counts),observed_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--chain-cases',type=int,default=512);p.add_argument('--kind',default='all')
    p.add_argument('--replace-uploads',type=Path);p.add_argument('--replace-runtime',type=Path);p.add_argument('--replace-bridge',type=Path)
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args();assert args.cases>0 and args.chain_cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();own=json.loads(MANIFEST.read_text())
    manifest=json.loads(core.MANIFEST.read_text());manifest['functions'].update(own['functions'])
    assert hashlib.sha256(raw).hexdigest()==own['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True)
    linked=build(args.replace_uploads or UPLOADS,args.replace_runtime or RUNTIME,args.replace_bridge or BRIDGE,args.gcc)
    kinds=list(own['functions'])+['consume']if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases,image),kinds,args.chain_cases)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['D3D/D3DX effect/device calls, texture lookup and library map destruction are controlled boundaries',
        'three pending engine bind/record/light bodies remain controlled in the consumer chain',
        'original scalar D3DX matrix library executes on both sides; recovered identity, inverse, hash, queue/pool/view code executes',
        'finite valid records/matrices and successful cleanup; no native consumer activation, OOM/exception or shader/rendering equivalence']
    if args.mutation_checks:
        mutations={
            'ignore-technique-cache':('world','uploads','ug2RenderTechnique870910 == ug2RenderLastTechnique87684C','true'),
            'wrong-projection-binding':('world','uploads','ug2SolidDevice870974,3,matrices+1','ug2SolidDevice870974,2,matrices+1'),
            'skip-world-view-upload':('world','uploads','if (ug2RenderTechnique870910->parameters1C->slots00[58].handle24)','if (false)'),
            'skip-shader-byte-recheck':('world','uploads','if (!ug2RenderTechnique870910->value18) return;','return;'),
            'wrong-wvp-input':('world','uploads','projection = matrices+2','projection = matrices+1'),
            'wrong-identity-counter':('world','uploads','world == &ug2RenderIdentity872E90','world != &ug2RenderIdentity872E90'),
            'wrong-palette-index':('bones','uploads','primitive->boneIndices21[i]*16','primitive->boneIndices21[0]*16'),
            'short-palette':('bones','uploads','for (int i=0;i<16;++i)','for (int i=0;i<15;++i)'),
            'wrong-palette-count':('bones','uploads','handle24,palette,16','handle24,palette,15'),
            'wrong-headlight-product-order':('headlights','uploads','&projected,world,projection','&projected,projection,world'),
            'lose-product-alias':('headlights','uploads','D3DXMatrixMultiply(&projected,&projected,&bias);','WheelMatrix copy = projected; D3DXMatrixMultiply(&projected,&copy,&bias);'),
            'wrong-clip-column':('headlights','uploads','cameraProjection.elements[3]*','cameraProjection.elements[2]*'),
            'wrong-colour-scale':('headlights','uploads','0.0078125f','0.003921569f'),
            'missing-colour-alpha-zero':('headlights','uploads','vector[3] = 0.0f;','vector[3] = 1.0f;'),
            'skip-colour-lookup':('colour','uploads','return object->value554->value380;','return 0;'),
            'wrong-receiver-gate':('begin_headlight','runtime','if (!value18)','if (!ug2RenderTechnique870910->value18)'),
            'wrong-reflection-column':('begin_reflection','runtime','matrix.elements[2] = matrix.elements[3];','matrix.elements[2] = matrix.elements[0];'),
            'skip-shader-rebinding':('begin_combined','runtime','} else {\n        BindHeadlightMaps();','} else {\n        (void)0;'),
            'wrong-car-map-gate':('begin_car','runtime','ug2RenderMode8654A4 == 6','ug2RenderMode8654A4 == 5'),
            'short-stage-loop':('begin_filter','runtime','stage<4','stage<3'),
            'wrong-base-identity':('destroy','runtime','value00 = 0x7a41b0;','value00 = 0x7a4578;'),
            'skip-container-destroy':('destroy','runtime','parameters->map960.Destroy();','(void)0;'),
            'clear-dangling-pointer':('destroy','runtime','UG2TextureFree(parameters);','UG2TextureFree(parameters); parameters1C = 0;'),
            'double-effect-release':('destroy','runtime','UG2TechniqueRelease(effect20);','UG2TechniqueRelease(effect20); UG2TechniqueRelease(effect20);'),
            'skip-delete-without-effect':('delete','runtime','Destroy();\n    if (flags&1) UG2TextureFree(this);','unsigned int previous = effect20; Destroy();\n    if (previous && (flags&1)) UG2TextureFree(this);'),
            'gate-base-delete-on-cleared-effect':('delete_base','runtime','if (flags&1) UG2TextureFree(this);','if (effect20 && (flags&1)) UG2TextureFree(this);'),
            'wrong-delete-bit':('delete','runtime','flags&1','flags&2'),
            'wrong-base-delete-bit':('delete_base','runtime','flags&1','flags&2'),
            'wrong-map-receiver':('parameters_destroy','runtime','{ map960.Destroy(); }','{ reinterpret_cast<CarRenderTechniqueMap *>(this)->Destroy(); }'),
            'wrong-class-begin':('consume','bridge','case 0x7a4554: technique->BeginHeadlightReflection();','case 0x7a4554: technique->BeginReflection();'),
            'wrong-empty-dispatch':('consume','bridge','technique->End(); return;','technique->BeginFilter(); return;'),
        }
        sources=dict(uploads=UPLOADS,runtime=RUNTIME,bridge=BRIDGE);rejected=[]
        for name,(kind,unit,before,after)in mutations.items():
            if kind not in kinds:continue
            body=sources[unit].read_text();assert before in body,name
            body=body.replace(before,after)
            if unit=='bridge':body=body.replace('#include "../../Speed/Indep/Src/World/CarRenderConsume.h"','#include "CarRenderConsume.h"')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            cmd=[sys.executable,__file__,'--replace-'+unit,str(path),'--kind',kind,'--cases','128','--chain-cases','128']+(['--gcc']if args.gcc else[])
            r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.log')).write_text(r.stdout)
            assert r.returncode and 'upload mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
