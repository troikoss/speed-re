#!/usr/bin/env python3
"""Verify texture/material binding and record/light upload, plus the full consumer.

All direct engine bind/upload helpers execute. GPU/effect APIs and texture
lookup remain controlled. Device-light comparisons include every written word
and exclude indeterminate original fields; no invented zero stores are used.
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
import verify_pc_car_render_uploads as uploads

core=uploads.core
ROOT,EXE=core.ROOT,core.EXE
SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderMaterial.cpp'
BUILD=ROOT/'build/analysis/car-render-material'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-material.json'
ARENA,STOP=core.ARENA,core.STOP
COLOUR,TRANSFORM=ARENA+0x3d000,ARENA+0x3d100
GLOBALS=dict(ug2RenderLastTexture870940=0x870940,ug2RenderLastTextureTechnique870944=0x870944,
    ug2RenderTextureChanges872F10=0x872f10,ug2RenderUvDirty870948=0x870948,ug2RenderUvEnable870CE8=0x870ce8,
    ug2RenderClock86518C=0x86518c,ug2RenderUvOverride870924=0x870924,ug2RenderUvOverride870928=0x870928,
    ug2RenderUvMode800504=0x800504,ug2RenderUvPeriod800508=0x800508,ug2RenderFlag870CB8=0x870cb8,
    ug2RenderRecordScale80050C=0x80050c,ug2RenderDiffuseScale800510=0x800510,ug2RenderSpecularScale800514=0x800514,
    ug2RenderPower80051C=0x80051c,ug2RenderShaderPower800B94=0x800b94)
APIS=[('UG2RenderSetTexture',3,0x104,'device'),('UG2RenderSetLight',3,0xcc,'device'),
    ('UG2RenderLightEnable',3,0xd4,'device'),('UG2RenderSetMaterial',2,0xc4,'device'),
    ('UG2EffectSetVectorArray',4,0x90,'effect'),('UG2EffectSetIntArray',4,0x70,'effect')]
for i,item in enumerate(APIS):core.APIS.append(item);core.API[item[0]]=STOP+0x4000+i*16
FUNCTIONS=dict(bind='CarRenderBindTexture',record='CarRenderUploadRecord',light='CarRenderUploadLightPack',
    alpha='CarRenderSetAlphaTest',blend='CarRenderSetBlend',wave='CarRenderTextureWave',clamp='CarRenderClampColour',transform='CarRenderTransform4')
PREFIX={k:'?'+name+'@@'for k,name in FUNCTIONS.items()};PREFIX.update(scroll_u='?ScrollU@CarRenderTexture@@',scroll_v='?ScrollV@CarRenderTexture@@')
GCC={k:'_Z'+str(len(name))+name for k,name in FUNCTIONS.items()};GCC.update(scroll_u='_ZN16CarRenderTexture7ScrollUEf',scroll_v='_ZN16CarRenderTexture7ScrollVEf')
core.MEMBERS.update(('scroll_u','scroll_v'));core.FP_RETURNS.update(('wave','scroll_u','scroll_v'));core.STDCALL.add('wave')
COVERED={'bind','record','light'}
REGIONS=[(0x870940,12),(0x86ead8,20),(0x872f10,4)]
LIGHT_WORDS=(0,1,2,3,5,6,7,9,10,11,12,16,17,18)


def build(source,gcc):
    uploads.BUILD=BUILD
    linked=uploads.build(uploads.UPLOADS,uploads.RUNTIME,uploads.BRIDGE,gcc,more_sources=[source],
        more_globals=GLOBALS,more_supplied=[FUNCTIONS[k]for k in COVERED])
    for k,p in (GCC if gcc else PREFIX).items():
        hits=[a for s,a in linked['exports'].items()if s.startswith(p)];assert len(hits)==1,(k,p,hits)
        linked['entries'][k]=hits[0]
    return linked


class Machine(uploads.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        super().__init__(image,manifest,linked,gcc)
        for k in ('alpha','blend','wave','clamp','transform','scroll_u','scroll_v'):
            a=self.entries[k];self.u.hook_add(UC_HOOK_CODE,self.material_observe,user_data=k,begin=a,end=a)
        if linked:
            for k in COVERED:
                a=self.entries[k];self.u.hook_add(UC_HOOK_CODE,self.material_observe,user_data=k,begin=a,end=a)
    def material_observe(self,u,address,size,k):
        if k in ('scroll_u','scroll_v'):
            t,a=self.member(1);event=(k,t,a)
        else:
            n=dict(alpha=2,blend=3,wave=4,clamp=1,transform=3,bind=1,record=2,light=2)[k]
            a=self.args(n)
            if k in ('wave','clamp','transform'):return # internal inlining is not an observable API boundary
            event=(k,a,self.word(0x870910))
        self.events.append(event)
    def helper(self,u,address,size,info):
        if info[0]in COVERED:self.material_observe(u,address,size,info[0])
        else:super().helper(u,address,size,info)
    def api(self,u,address,size,info):
        name,n=info
        if name=='UG2EffectSetVector'and self.kind=='light':
            self.vectors+=1
            if self.vectors==2 and self.case['change_type']:self.put(core.TECH+4,5)
            if self.vectors==1 and self.case['change_pack']:self.put(core.LIGHT+0x58,0x40800000)
        if name not in {x[0]for x in APIS}:return super().api(u,address,size,info)
        a=self.args(n);event=(name,*a)
        if name.startswith('UG2Render'):assert a[0]==core.DEVICE
        else:assert core.EFFECT<=a[0]<core.EFFECT+23*0x200
        if name=='UG2RenderSetLight':
            event=(name,a[0],a[1],tuple(self.word(a[2]+j*4)for j in LIGHT_WORDS))
        elif name=='UG2RenderSetMaterial':event=(name,a[0],bytes(u.mem_read(a[1],0x44)))
        elif name=='UG2EffectSetVectorArray':
            assert a[3]==10;event=(name,a[0],a[1],bytes(u.mem_read(a[2],a[3]*16)),a[3])
        elif name=='UG2EffectSetIntArray':
            assert a[3]==5;event=(name,a[0],a[1],bytes(u.mem_read(a[2],20)),a[3])
        event+=(tuple(self.word(a)for a in (0x870940,0x870944,0x870948)),)
        self.events.append(event);self.ret(self.case['hresult'],0 if self.rebuilt else n*4)
    def prepare(self,kind,c):
        super().prepare(kind,c);self.kind=kind;self.vectors=0
        if kind in ('bind','record','light','alpha','blend'):
            self.put(core.TECH+4,c['render_type']);self.u.mem_write(core.TECH+0x18,bytes([c['shader']]))
        if kind in ('alpha','blend')or(kind=='bind'and c['null_bind']):self.put(0x870910,0 if c['immediate']else core.TECH)
        if kind=='bind':
            self.put(0x870940,core.TEXTURE if c['cache_texture']else 0)
            self.put(0x870944,self.word(0x870910)if c['cache_technique']else core.TECH+0x28)
            if c['null_bind']:self.put(core.STATE+0x18,0)
    def snapshot(self,result):
        value=list(super().snapshot(result));value.append(tuple(bytes(self.u.mem_read(a,n))for a,n in REGIONS));return tuple(value)
    def run(self,kind,c):
        if kind=='consume':return super().run(kind,c)
        self.reset(kind,c)
        if kind=='bind':
            result=self.call(kind,[core.TEXTURE])
            if c['repeat_bind']:self.call(kind,[core.TEXTURE])
        elif kind=='record':result=self.call(kind,[core.RECORD,core.WORLD])
        elif kind=='light':result=self.call(kind,[core.LIGHT,c['flags']])
        elif kind=='alpha':result=self.call(kind,c['alpha'])
        elif kind=='blend':result=self.call(kind,c['blend'])
        elif kind=='wave':result=self.call(kind,c['wave'])
        elif kind=='clamp':result=self.call(kind,[COLOUR])
        elif kind=='transform':result=self.call(kind,[c['transform_output'],core.WORLD,core.RECORD])
        elif kind in ('scroll_u','scroll_v'):result=self.call(kind,[c['wave'][0]],core.TEXTURE)
        else:raise AssertionError(kind)
        return self.snapshot(result)


def cases(n,image):
    rng=random.Random(0x5bcc30)
    def bits(x):return struct.unpack('<I',struct.pack('<f',x))[0]
    for i,c in enumerate(uploads.cases(n,image)):
        data=bytearray(c['data']);g=c['globals']
        def put(a,*v):struct.pack_into('<'+'I'*len(v),data,a-ARENA,*(x&0xffffffff for x in v))
        def floats(a,v):struct.pack_into('<'+'f'*len(v),data,a-ARENA,*v)
        def glob(a,*v):g[a]=struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v))
        for j in range(16):
            t=core.TEXTURE+j*0x80
            struct.pack_into('<2h',data,t-ARENA+0x44,(1,4,32,64,128)[(i+j)%5],(2,8,16,64)[(i+j)%4])
            data[t-ARENA+0x52]=(0,1,2,3,4,128,255)[(i+j)%7]
            data[t-ARENA+0x55]=(0,1,2,255)[(i+j)%4];data[t-ARENA+0x56]=(0,1,2,3,4,5,6,255)[(i+j)%8]
            struct.pack_into('<5h',data,t-ARENA+0x58,*[(-512,-1,0,1,128,256,32767)[(i+j+k)%7]for k in range(5)])
            floats(core.RECORD+j*0x200,[rng.uniform(-2,2)for _ in range(0x1c0//4)])
            floats(core.LIGHT+j*0x100+0x30,[rng.uniform(-1,2)for _ in range(0x70//4)])
        # Meaningful directional enable thresholds, without inventing light disables.
        if i%5==0:
            floats(core.WORLD,[1 if k%5==0 else 0 for k in range(16)])
            floats(core.RECORD+0x40,[0,0,0,0,0,0,0,0,0,0,0,0])
            floats(core.RECORD+0x40,[(0,0.00009999,0.0001,0.00010001,-0.0001)[(i//5)%5],0,0,0])
        glob(0x870940,0);glob(0x870944,0);glob(0x870948,(0,1,5)[i%3]);glob(0x872f10,0xffffffff if i%7==0 else i)
        glob(0x870ce8,(0,1,-1)[(i//7)%3]);glob(0x86518c,(0,1,-1,99999,0x7fffffff,-0x80000000)[i%6])
        g[0x870924]=struct.pack('<2f',(0,1,-1,13.25)[(i//21)%4],(0,0,3.5,-2)[(i//84)%4])
        glob(0x800504,(0,1,2,-1,3)[(i//336)%5]);g[0x800508]=struct.pack('<f',(0.25,0.5,1,-1)[i%4])
        glob(0x870cb8,(0,1,-1)[(i//224)%3])
        for a,values in [(0x80050c,(0.25,1,-0.5)),(0x800510,(0.5,1,2)),(0x800514,(0.4,0,-1)),(0x80051c,(4,0,-2)),(0x800b94,(1.1,2,0))]:g[a]=struct.pack('<f',values[i%3])
        pattern=(0,0x80000000,0x3f000000,0x3f800000,0xbf800000,0x40000000,0x7f800000,0xff800000,0x7fc12345,0x7f812345)
        put(COLOUR,*[pattern[(i+j)%len(pattern)]for j in range(4)])
        phase=(0,-0.5,0.5,1024.25,536870.9,1e9)[i%6]
        freq=(0,0.25,-0.5,0.1,31.999)[(i//6)%5]
        mode=(0,1,2,-1,128)[(i//30)%5];period=(0.25,0.7,-1,0)[(i//150)%4]
        c.update(data=bytes(data),globals=g,render_type=(0,4,5,-1,5,4,20)[i%7],shader=(0,1,2,255)[(i//7)%4],
            flags=(0,0x80,0x100,0x180,0x101,0xffffffff,0x10000,0x1ff)[(i//28)%8],
            immediate=(i//4)%2==0,null_bind=i%11==0,cache_texture=i%3==0,cache_technique=(i//3)%3==0,repeat_bind=i%2==0,
            alpha=((0,1,0xffffffff)[i%3],(0,95,0x12345678)[(i//3)%3]),
            blend=((0,1,7)[i%3],(0,5,7,0xffffffff)[(i//3)%4],(2,6,8,0x12345678)[(i//12)%4]),
            wave=[bits(phase),bits(freq),mode&0xffffffff,bits(period)],
            transform_output=(TRANSFORM,core.RECORD,core.RECORD+4,core.WORLD,core.WORLD+4)[i%5],
            change_type=i%13==0,change_pack=i%17==0)
        yield c


def verify(image,manifest,linked,gcc,fixtures,kinds,chain_cases):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);counts,events=Counter(),Counter()
    for i,c in enumerate(fixtures):
        for k in kinds:
            if k=='consume'and i>=chain_cases:continue
            try:x,y=a.run(k,c),b.run(k,c)
            except AssertionError as e:raise AssertionError(('material mismatch',k,i,str(e)))from e
            if x!=y:
                changes=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[2],y[2]))if v!=w),None)
                raise AssertionError(('material mismatch',k,i,changes,event,len(x[2]),len(y[2]),
                    [j for j,(v,w)in enumerate(zip(x[1],y[1]))if v!=w],x[4]!=y[4],x[5]!=y[5]))
            counts[k]+=1;events.update(e[0]for e in x[2])
    return dict(comparisons=dict(counts),observed_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--chain-cases',type=int,default=512);p.add_argument('--kind',default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args();assert args.cases>0 and args.chain_cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();own=json.loads(MANIFEST.read_text())
    manifest=json.loads(core.MANIFEST.read_text());manifest['functions'].update(json.loads(uploads.MANIFEST.read_text())['functions']);manifest['functions'].update(own['functions'])
    assert hashlib.sha256(raw).hexdigest()==own['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);linked=build(args.replace or SOURCE,args.gcc)
    kinds=list(own['functions'])+['consume']if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases,image),kinds,args.chain_cases)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['GPU/effect APIs, texture lookup and library map destruction are controlled boundaries',
        'all direct consumer bind/upload helpers and concrete technique methods execute in the linked chain',
        'every written device-light word is compared; its unwritten fields are indeterminate and excluded',
        'original scalar D3DX matrix library executes on both sides; native GPU execution and rendering equivalence remain unproven']
    if args.mutation_checks:
        mutations={
            'ignore-texture-technique-cache':('bind','ug2RenderLastTextureTechnique870944 == ug2RenderTechnique870910','true'),
            'skip-zero-token-return':('bind','if (!token) return;','(void)0;'),
            'alpha-byte-as-boolean':('bind','texture->value55 == 1','texture->value55 != 0'),
            'wrong-additive-destination':('bind','case 2: destination = 2;','case 2: destination = 6;'),
            'wrong-car-cull':('bind','ug2SolidDevice870974,22,2','ug2SolidDevice870974,22,1'),
            'opaque-depth-write-off':('bind','ug2SolidDevice870974,14,1','ug2SolidDevice870974,14,0'),
            'skip-uv-handle-gate':('bind','if (ug2RenderTechnique870910->parameters1C->slots00[55].handle24)','if (true)'),
            'wrong-clock-scale':('bind','clockScale784268 = 0.00025f','clockScale784268 = 0.0003f'),
            'wrong-static-uv-offset':('bind','texture->value5E)*-0.0009765625f','texture->value5E)*0.0009765625f'),
            'wrong-override-width':('bind','ug2RenderUvOverride870924/static_cast<float>(texture->width44)','ug2RenderUvOverride870924/static_cast<float>(texture->height46)'),
            'wrong-uv-matrix-row':('bind','matrix.elements[8] = offset[0];','matrix.elements[12] = offset[0];'),
            'skip-uv-reset':('bind','if (!texture->value52) ug2RenderUvDirty870948 = 0;','(void)0;'),
            'wrong-immediate-alpha-reference':('alpha','ug2SolidDevice870974,24,reference','ug2SolidDevice870974,24,enabled'),
            'wrong-effect-blend-destination':('blend','ug2TechniqueBlend86EAD8[4] = destination;','ug2TechniqueBlend86EAD8[3] = destination;'),
            'wrong-immediate-blend-destination':('blend','ug2SolidDevice870974,20,destination','ug2SolidDevice870974,20,source'),
            'wrong-quantization-mode':('wave','if (mode == 2)','if (mode == 1)'),
            'floor-negative-phase':('wave','return scaled-whole;','if (scaled<whole) --whole; return scaled-whole;'),
            'unsigned-whole-part':('wave','int whole = static_cast<int>(static_cast<unsigned int>(static_cast<MaterialInteger>(scaled)));','unsigned int whole = static_cast<unsigned int>(static_cast<MaterialInteger>(scaled));'),
            'wrong-scroll-u-field':('scroll_u','static_cast<float>(value5A)','static_cast<float>(value5C)'),
            'wrong-scroll-v-scale':('scroll_v','frequency = static_cast<float>(value5C)*0.0009765625f','frequency = static_cast<float>(value5C)*0.001953125f'),
            'generic-nan-clamp':('clamp','float upper = value[i]<1.0f ? value[i] : 1.0f;','float upper = value[i]; if (upper>1.0f) upper = 1.0f;'),
            'skip-lower-clamp':('clamp','value[i] = upper<=0.0f ? 0.0f : upper;','value[i] = upper;'),
            'transform-before-snapshot':('transform','float y = ((static_cast<long double>(m[13])','output[0] = x; float y = ((static_cast<long double>(m[13])'),
            'wrong-transform-column':('transform','m[14])*input[3]','m[13])*input[3]'),
            'drop-world-bone-record':('record','else if (type == 5 || type == 4)','else if (type == 5)'),
            'wrong-direction-transpose':('record','input[0x10+j*4+i]','input[0x10+i*4+j]'),
            'translate-light-directions':('record','directions[i][3] = 0;','directions[i][3] = 1;'),
            'wrong-record-packed-channel':('record','red = PackChannel(colour[0])','red = PackChannel(colour[1])'),
            'wrong-direction-threshold':('record','directionThreshold7843EC = 0.0001f','directionThreshold7843EC = 0.0002f'),
            'invent-light-disable':('record','UG2RenderLightEnable(ug2SolidDevice870974,i,1);','UG2RenderLightEnable(ug2SolidDevice870974,i,1); else UG2RenderLightEnable(ug2SolidDevice870974,i,0);'),
            'wrong-harmonic-count':('record','handle24,input+0x48,10','handle24,input+0x48,9'),
            'wrong-environment-gate':('light','!(flags&0x100)','!(flags&0x200)'),
            'ignore-signed-low-byte':('light','static_cast<signed char>(flags)>=0','true'),
            'reverse-material-range':('light','p->value44[i])*p->value40-a[i]','p->value44[i])*p->value40+a[i]'),
            'wrong-half-average':('light','ar[i])*0.5f+a[i]','ar[i])*0.25f+a[i]'),
            'wrong-fixed-power-minimum':('light','powerMinimum784358 = 0.1f','powerMinimum784358 = 0.2f'),
            'conventional-alpha-average':('light','ar[3])*128.0f','ar[3])*127.5f'),
            'cache-type-across-vector-calls':('light','if (ug2RenderTechnique870910->type04 == 5) {','if (type == 5) {'),
            'wrong-shader-power':('light','ug2RenderShaderPower800B94*p->value58','ug2RenderPower80051C*p->value58'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            limit=1024 if kind in ('bind','light')else 256
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases',str(limit)]+(['--gcc']if args.gcc else[])
            r=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.log')).write_text(r.stdout)
            assert r.returncode and 'material mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
