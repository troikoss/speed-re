#!/usr/bin/env python3
"""Verify view/context binding, complete context flags, getters and instance gate.

The angle and Z-rotation helpers execute original code on both sides; the rebuilt
binding executes the recovered scalar matrix implementation. The instance gate
is a 20-byte fragment, not the complete scenery instance/LOD/queue routine.
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
from pc_coff import read_object,link_objects
from pc_native_elf import link_native
import verify_pc_car_render_view as core

ROOT,EXE=core.ROOT,core.EXE
SOURCE=core.WORLD/'CarRenderContext.cpp'
BUILD=ROOT/'build/analysis/car-render-context'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-context.json'
CONTEXT,INSTANCE=core.ARENA+0x9000,core.ARENA+0xa000
ORIGINAL=dict(bind=0x5c26e0,setup=0x4888a0,mode=0x5b98a0,zero=0x499db0)
GCC=dict(bind='_Z17CarRenderBindView',setup='_Z21CarRenderSetupContext',mode='UG2CarViewModeGlobal',zero='UG2CarLodZeroFlag',admit='_ZNK20CarRenderDrawContext14AdmitsInstanceE')
PREFIX=dict(bind='?CarRenderBindView@@',setup='?CarRenderSetupContext@@',mode='_UG2CarViewModeGlobal',zero='_UG2CarLodZeroFlag',admit='?AdmitsInstance@CarRenderDrawContext@@')
GLOBALS=dict(ug2RenderViewMode8707DC=0x8707dc,ug2RenderReverse89E7A4=0x89e7a4,ug2RenderLodZero836352=0x836352)
CACHES=[(0x870940,12),(0x87684c,8),(0x800534,4),(0x87092c,8)]
ACCEPT,REJECT=0x5d65d0,0x5d69a1
core.CDECL_RETURNS.update(('mode','zero','admit'))


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[source,core.MATRIX,core.PHYSICS/'WheelMath.cpp',core.PHYSICS/'BodyMatrixProduct.cpp']
    if gcc:
        bindings=dict(GLOBALS,ug2RenderLastTexture870940=0x870940,sqrt=core.STOP+0x100,sqrtl=core.STOP+0x120)
        bindings.update({'_Z13HudAngleUnitsff':0x43e6d0,'_Z13MakeRotationZP11WheelMatrixj':0x5bac90})
        linked=link_native(sources,BUILD,ROOT,bindings,'UG2CarViewModeGlobal')
    else:
        objects=[];bindings={'_'+k:v for k,v in GLOBALS.items()};bindings.update(__fltused=0,_ug2RenderLastTexture870940=0x870940)
        for i,p in enumerate(sources):
            obj=BUILD/('context-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
            for s in read_object(obj)['symbols'].values():
                if s['section']!=0:continue
                for n,a in [('HudAngleUnits',0x43e6d0),('MakeRotationZ',0x5bac90)]:
                    if s['name'].startswith('?'+n+'@@'):bindings[s['name']]=a
        linked=link_objects(objects,bindings,start=0x2400000)
    linked['entries']={k:next(a for n,a in linked['exports'].items()if n.startswith(p))for k,p in (GCC if gcc else PREFIX).items()}
    linked['multiply']=next(a for n,a in linked['exports'].items()if n.startswith('_Z24D3DXMatrixMultiplyScalar'if gcc else'?D3DXMatrixMultiplyScalar@@'))
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries=ORIGINAL
        self.u.hook_add(UC_HOOK_CODE,self.math,user_data='angle',begin=0x43e6d0,end=0x43e6d0)
        self.u.hook_add(UC_HOOK_CODE,self.math,user_data='rotation',begin=0x5bac90,end=0x5bac90)
        a=linked['multiply']if linked else 0x63cee1
        self.u.hook_add(UC_HOOK_CODE,self.math,user_data='multiply',begin=a,end=a)
        a=self.entries['bind'];self.u.hook_add(UC_HOOK_CODE,self.math,user_data='bind',begin=a,end=a)
        if not linked:self.u.hook_add(UC_HOOK_CODE,self.stop_fragment,begin=REJECT,end=REJECT)
    def args(self,n):return struct.unpack('<'+'I'*n,bytes(self.u.mem_read(self.u.reg_read(xc.UC_X86_REG_ESP)+4,n*4)))
    def math(self,u,address,size,kind):
        if kind=='angle':event=(kind,self.args(2))
        elif kind=='bind':event=(kind,self.args(2))
        elif kind=='rotation':event=(kind,self.args(2)[1]&0xffff)
        else:
            out,a,b=self.args(3);event=(kind,bytes(u.mem_read(a,64)),bytes(u.mem_read(b,64)),out==a,out==b)
        self.events.append(event)
    def stop_fragment(self,u,address,size,unused):u.emu_stop()
    def prepare(self,c):
        self.u.mem_write(core.ARENA,c['data']);self.events=[]
        self.put(0x8707dc,c['mode']);self.put(0x89e7a4,c['reverse']);self.u.mem_write(0x836352,bytes([c['zero']]))
        for (a,n),body in zip(CACHES,c['caches']):self.u.mem_write(a,body)
    def snapshot(self,result):
        return bytes(self.u.mem_read(core.ARENA,core.SIZE)),result,self.events,[bytes(self.u.mem_read(a,n))for a,n in [(0x8707dc,4),(0x89e7a4,4),(0x836352,1)]+CACHES]
    def run(self,kind,c,flags=None):
        self.prepare(c)
        if kind=='admit':self.put(c['context']+0x44,c['admit_flags'])
        if kind in ('bind','setup'):
            args=[core.VIEW,c['context']]+([c['flags']]if kind=='setup'else[])
            result=self.call(kind,args,cw=c['cw'])
        elif kind in ('mode','zero'):
            result=self.call(kind,cw=c['cw'])
            if kind=='zero':result&=255
        elif self.gcc or 'admit'in self.entries:
            result=self.call('admit',[flags],c['context'],c['cw'])&255
        else:
            struct_bytes=struct.pack('<H',flags);self.u.mem_write(INSTANCE+0x1a,struct_bytes)
            self.u.reg_write(xc.UC_X86_REG_EBX,c['context']);self.u.reg_write(xc.UC_X86_REG_ESI,INSTANCE)
            self.u.emu_start(0x5d65bc,ACCEPT,count=100)
            end=self.u.reg_read(xc.UC_X86_REG_EIP);assert end in (ACCEPT,REJECT)
            result=int(end==ACCEPT)
            # The instruction fragment reads this record; the standalone C++
            # fragment adapter takes its flags by value. Exclude fixture setup.
            self.u.mem_write(INSTANCE+0x1a,c['data'][INSTANCE+0x1a-core.ARENA:INSTANCE+0x1c-core.ARENA])
        return self.snapshot(result)


def cases(n):
    rng=random.Random(0x4888a0)
    for i,c in enumerate(core.cases(n)):
        data=bytearray(c['data'])
        data[core.CLIP-core.ARENA:core.CLIP-core.ARENA+64]=data[core.CAMERA-core.ARENA:core.CAMERA-core.ARENA+64]
        context=(CONTEXT,CONTEXT,CONTEXT,CONTEXT,core.VIEW-0x40,core.VIEW-0x3c,core.CLIP-0x40,core.CLIP-0x44)[i%8]
        flags=rng.getrandbits(32)&0xffffff00|(i&255)
        if context==core.CLIP-0x44:flags=0x3f000000|(i&255) # finite aliased heading input
        struct.pack_into('<I',data,context+0x44-core.ARENA,flags)
        if i%9==0:struct.pack_into('<3f',data,core.CLIP-core.ARENA,0,0,0)
        c.update(data=bytes(data),context=context,flags=(0,1,0x800,0x1000,0x1800,0x40,0xffffffff,rng.getrandbits(32))[i%8],
            mode=(0,1,2,3,4,5,-1,-0x80000000)[(i//8)%8],reverse=(0,1,2,-1,0x7fffffff)[(i//64)%5],zero=i%256)
        c['caches']=[rng.randbytes(n)for a,n in CACHES]
        c['admit_flags']=(rng.getrandbits(32)&0xffffff00)|(i&255)
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts,events,outcomes=Counter(),Counter(),Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            values=range(256)if kind=='admit'and i<256 else[]if kind=='admit'else[None]
            for low in values:
                flags=(i<<8)|low if low is not None else None
                try:x,y=a.run(kind,c,flags),b.run(kind,c,flags)
                except (AssertionError,UcError)as e:raise AssertionError(('context mismatch',kind,i,flags,str(e)))from e
                if x!=y:
                    offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:12]
                    event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[2],y[2]))if v!=w),None)
                    raise AssertionError(('context mismatch',kind,i,flags,offsets,x[1],y[1],event))
                counts[kind]+=1;events.update(e[0]for e in x[2])
                if kind=='admit':outcomes.update([bool(x[1])])
    return dict(comparisons=dict(counts),observed_calls=dict(events),admission_outcomes=dict(outcomes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=4096)
    p.add_argument('--kind',choices=['all',*ORIGINAL,'admit'],default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true')
    p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+list(manifest['fragments'].values()):
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc);kinds=[*ORIGINAL,'admit']if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,args.cases,kinds)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['heading and Z-rotation helper bodies execute original code on both sides; scalar matrix multiply executes recovered code in the rebuilt binding',
        'instance admission is only the conditional fragment; full scenery visibility/LOD/queues and render scheduling remain outside this verifier',
        'finite bounded records, aliasing and modes; native GPU/effect rendering and malformed/OOM/exception behavior are not established'])
    if args.native_dump:
        data=args.native_dump.read_bytes();assert len(data)==489 and data[:8]==b'UG2CTX01'
        view,clip,context=bytearray(data[8:120]),data[120:408],bytearray(data[408:480])
        mode,reverse=struct.unpack_from('<ii',data,480);zero=data[488]
        # The packed fixture has 8 + 0x70 + 0x120 + 0x48 + 4 + 4 + 1 bytes.
        # No pointer from the native process is dereferenced by the replay.
        index=struct.unpack_from('<i',view,4)[0]
        expected_view=0x832de0+index*0x70
        assert 0<=index<22 and struct.unpack_from('<I',context,0x40)[0]==expected_view
        assert struct.unpack_from('<I',view,0)[0]==0x873380+index*0x120
        c=next(cases(1));arena=bytearray(c['data'])
        struct.pack_into('<I',view,0,core.CLIP);struct.pack_into('<I',context,0x40,core.VIEW)
        before=bytearray(context);struct.pack_into('<2I',before,0x40,0xdeadbeef,0xa5a5a5a5)
        for a,body in [(core.VIEW,view),(core.CLIP,clip),(CONTEXT,before)]:arena[a-core.ARENA:a-core.ARENA+len(body)]=body
        c.update(data=bytes(arena),context=CONTEXT,flags=0,mode=mode,reverse=reverse,zero=zero,cw=0x027f)
        original=Machine(image);result=original.run('setup',c)
        actual=result[0][CONTEXT-core.ARENA:CONTEXT-core.ARENA+0x48]
        assert actual==context,('native context mismatch',[(hex(i),v,w)for i,(v,w)in enumerate(zip(actual,context))if v!=w])
        report['native_replay']=dict(bytes=0x48,view_index=index,flags=hex(struct.unpack_from('<I',context,0x44)[0]),
            mode=mode,reverse=reverse,lod_zero=zero,original_matches_native=True)
    if args.mutation_checks:
        mutations={
            'constant-view-mode':('mode','return ug2RenderViewMode8707DC','return 0'),
            'constant-lod-zero':('zero','return ug2RenderLodZero836352','return 0'),
            'wrong-heading-column':('bind','HudAngleUnits(matrix[2],matrix[0])','HudAngleUnits(matrix[1],matrix[0])'),
            'swap-heading-arguments':('bind','HudAngleUnits(matrix[2],matrix[0])','HudAngleUnits(matrix[0],matrix[2])'),
            'omit-local-identity':('bind','WheelIdentity(&identity);','WheelIdentity(&identity); identity.elements[0]=2;'),
            'omit-rotation-call':('bind','MakeRotationZ(&rotation,angle);','WheelIdentity(&rotation);'),
            'omit-discarded-product':('bind','D3DXMatrixMultiplyScalar(&identity,&identity,&rotation);','(void)0;'),
            'write-product-into-context':('bind','D3DXMatrixMultiplyScalar(&identity,','D3DXMatrixMultiplyScalar(reinterpret_cast<WheelMatrix *>(context),'),
            'bind-camera-as-view':('bind','context->view40=view;','context->view40=reinterpret_cast<CarRenderView *>(const_cast<CarRenderFrame *>(view->frame40));'),
            'invent-cache-reset':('bind','context->view40=view;','context->view40=view; ug2RenderLastTexture870940=0;'),
            'lose-input-flags':('setup','result=flags|0x10','result=0x10'),
            'wrong-forced-bit':('setup','result=flags|0x10','result=flags|0x20'),
            'missing-primary-view-bit':('setup','type==1 || type==2','type==1 && type==2'),
            'wrong-mode-class':('setup','mode==4 || mode==3','mode==2 || mode==3'),
            'wrong-forward-bit':('setup','Reverse89E7A4==0) result|=8','Reverse89E7A4==0) result|=4'),
            'wrong-reverse-bit':('setup','Reverse89E7A4==1) result|=4','Reverse89E7A4==1) result|=8'),
            'missing-reflection-bit':('setup','result&0x1800','result&0x800'),
            'missing-type-three-bit':('setup','type==3) result|=0x20','type==4) result|=0x20'),
            'write-flags-before-binding':('setup','CarRenderBindView(view,context);\n    context->flags44=result;','context->flags44=result;\n    CarRenderBindView(view,context);'),
            'omit-context-flags':('setup','context->flags44=result;','(void)0;'),
            'omit-instance-xor':('admit','flags^0x60','flags'),
            'wrong-instance-xor':('admit','flags^0x60','flags^0x20'),
            'only-low-nibble':('admit','static_cast<unsigned char>(flags44)','(flags44&15)'),
            'invert-instance-gate':('admit',')==0;',')!=0;'),
            'use-high-instance-bits':('admit','static_cast<unsigned char>(flags44)&static_cast<unsigned char>(flags^0x60)','flags44&(flags^0x60)'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            if name=='invent-cache-reset':body='#include "CarRenderMaterial.h"\n'+body
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout)
            assert r.returncode and 'context mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
