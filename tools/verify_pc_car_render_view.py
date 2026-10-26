#!/usr/bin/env python3
"""Verify original view construction, projection/planes and their scalar dependencies.

Executes the recovered scalar matrix library and unspilled angle helpers, then
checks the plane producer through the recovered bounds consumer. The optimized
D3DX variants and full renderer scheduler are outside this verifier.
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
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UcError
import unicorn.x86_const as xc
from pc_coff import read_object,link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE

BUILD=ROOT/'build/analysis/car-render-view'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-view.json'
WORLD=ROOT/'src/Speed/Indep/Src/World'
PHYSICS=WORLD/'../Physics'
SOURCE=WORLD/'CarRenderView.cpp'
ANGLE=PHYSICS/'EngineAngleMath.cpp'
MATRIX=PHYSICS/'D3DXMatrixScalar.cpp'
SOURCES=[SOURCE,ANGLE,MATRIX,PHYSICS/'WheelMath.cpp',PHYSICS/'BodyMatrixProduct.cpp',WORLD/'CarRenderBounds.cpp']
ORIGINAL=dict(construct=0x491940,storage=0x5bc5b0,update=0x5c7ec0,sine=0x43c2b0,cosine=0x43c370,multiply=0x63cee1,bounds=0x5bc300)
NAMES=dict(construct=('CarRenderView','Construct'),storage=(None,'CarRenderClipDataByIndex'),update=('CarRenderClipData','Update'),
    sine=(None,'EngineSineUnitsExtended'),cosine=(None,'EngineCosineUnitsExtended'),multiply=(None,'D3DXMatrixMultiplyScalar'),bounds=('CarRenderView','TestBounds'))
PREFIX={k:'?'+n+'@'+c+'@@'if c else'?'+n+'@@'for k,(c,n)in NAMES.items()}
GCC={k:'_ZN'+str(len(c))+c+str(len(n))+n if c else'_Z'+str(len(n))+n for k,(c,n)in NAMES.items()}
ARENA,SIZE,STACK,STOP=0x2000000,0x10000,0x2100000,0x2120000
CAMERA,PORT,VIEW,CLIP,WORLD_MATRIX,LEFT,RIGHT,OUTPUT=[ARENA+x for x in (0x1000,0x2000,0x3000,0x4000,0x6000,0x7000,0x7100,0x7200)]
MINIMUM,MAXIMUM,RAW_RETURN=ARENA+0x8000,ARENA+0x8010,ARENA+0xf000
REGIONS=[(0x832de0,22*0x70),(0x873380,22*0x120)]
CDECL_RETURNS={'construct','storage','multiply','bounds'}
SYMBOLS={n:int(a,16)for n,a in re.findall(r'^(\w+) = (0x[0-9a-f]+);',(ROOT/'src/Platform/Native/OriginalSymbols.ld').read_text(),re.M)}
GLOBALS=dict(ug2RenderViews832DE0=0x832de0,ug2RenderClipData873380=0x873380)
FLOAT_GLOBALS={n:SYMBOLS[n]for n in ('engineAngleUnit','engineThreeHalfPi','engineTwoPi','engineHalfPi','enginePi',
    'engineSineC3','engineSineC5','engineSineC7','engineSineC9','cameraAspectScaleX7A27DC','cameraAspectScaleY7A27E0','cameraAspectDivisor7A27D8','fengZero7D7D3C')}


def build(gcc,source=SOURCE,angle=ANGLE,matrix=MATRIX):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[source,angle,matrix]+SOURCES[3:]
    if gcc:
        bindings=dict(GLOBALS,**FLOAT_GLOBALS,sqrt=STOP+0x100,sqrtl=STOP+0x120)
        # Bounds declares the shared zero with C linkage; GCC global names agree.
        linked=link_native(sources,BUILD,ROOT,bindings,'_Z24CarRenderClipDataByIndexi')
    else:
        objects=[];bindings={'_'+k:v for k,v in GLOBALS.items()}
        bindings.update({'?'+k+'@@3MA':v for k,v in FLOAT_GLOBALS.items()});bindings.update(_fengZero7D7D3C=0x7d7d3c,__fltused=0,__ftol2=0x75bed4)
        for i,p in enumerate(sources):
            obj=BUILD/('view-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        linked=link_objects(objects,bindings,start=0x2400000)
    linked['entries']={k:next(a for n,a in linked['exports'].items()if n.startswith(p))for k,p in (GCC if gcc else PREFIX).items()}
    return linked


class Machine:
    def __init__(self,image,linked=None,gcc=False):
        u=self.u=Uc(UC_ARCH_X86,UC_MODE_32);self.gcc=gcc
        u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        u.mem_map(0,0x1000);u.mem_map(ARENA,SIZE);u.mem_map(STACK,0x30000)
        self.entries=linked['entries']if linked else ORIGINAL
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        # Select the scalar original library, rather than its CPU initializer.
        self.put(0x803e74,0x63cee1)
        # Runtime libm fallbacks perform the same single FSQRT operation. They
        # accept each compiler's actual argument width and leave an x87 return.
        u.mem_write(STOP+0x100,b'\xdd\x44\x24\x04\xd9\xfa\xc3')
        u.mem_write(STOP+0x120,b'\xdb\x6c\x24\x04\xd9\xfa\xc3')
        self.initial=[(a,image[a-0x400000:a-0x400000+n])for a,n in REGIONS]
    def put(self,a,*v):self.u.mem_write(a,struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v)))
    def call(self,kind,args=(),receiver=0,cw=0x037f):
        u=self.u;sp=STACK+0x20000-0x100
        values=[receiver]+list(args)if receiver and self.gcc else args
        self.put(sp,STOP,*values)
        saved=[(r,0xabcdef00+i)for i,r in enumerate((xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP))]
        for r,v in saved+[(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_FPCW,cw),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP
        pop=len(args)*4 if kind=='multiply' or(receiver and not self.gcc)else 0
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop,(kind,hex(u.reg_read(xc.UC_X86_REG_ESP)),hex(sp),pop)
        assert all(u.reg_read(r)==v for r,v in saved),kind
        assert u.reg_read(xc.UC_X86_REG_FPCW)==cw
        if kind in ('sine','cosine'):
            u.mem_write(STOP,b'\xdb\x3d'+struct.pack('<I',RAW_RETURN))
            u.emu_start(STOP,STOP+6);u.mem_write(STOP,b'\x90'*6)
            assert u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
            return bytes(u.mem_read(RAW_RETURN,10))
        assert u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        return u.reg_read(xc.UC_X86_REG_EAX)if kind in CDECL_RETURNS else None
    def run(self,kind,c):
        u=self.u;u.mem_write(ARENA,c['data']);self.put(0,0)
        for a,b in self.initial:u.mem_write(a,b)
        if kind=='construct':u.mem_write(0x832de0+c['index']*0x70,c['data'][:0x70])
        cw=c['cw']
        if kind=='construct':result=self.call(kind,receiver=0x832de0+c['index']*0x70,cw=cw)
        elif kind=='storage':result=self.call(kind,[c['index']],cw=cw)
        elif kind=='multiply':result=self.call(kind,[c['output'],LEFT,RIGHT],cw=cw)
        else:
            result=self.call('update',[VIEW,*c['overrides'],c['unused']],CLIP,cw)
            if kind=='chain':result=self.call('bounds',[MINIMUM,MAXIMUM,c['world']],VIEW,cw)
        return bytes(u.mem_read(ARENA,SIZE)),[bytes(u.mem_read(a,n))for a,n in REGIONS],result,bytes(u.mem_read(0,4))


def cases(n):
    rng=random.Random(0x5c7ec0)
    def bits(v):return struct.unpack('<I',struct.pack('<f',v))[0]
    for i in range(n):
        data=bytearray(rng.randbytes(SIZE))
        def put(a,fmt,*v):struct.pack_into('<'+fmt,data,a-ARENA,*v)
        def floats(a,v):put(a,'f'*len(v),*v)
        for a in (CAMERA,LEFT,RIGHT,WORLD_MATRIX):floats(a,[rng.uniform(-4,4)for _ in range(16)])
        # Camera matrices include translation and nonzero fourth-column entries,
        # which the producer must clear after its original raw matrix copy.
        put(CAMERA+0xbc,'2fH',(0.1,0.5,1.0,3.5)[i%4],(1000,10000,17000)[i%3],(0,1,2,0x4000,0x8000,0xffff,0x2400,0x3000)[i%8])
        put(PORT+0x14,'2i',(640,800,1024,1920)[i%4],(480,600,768,1080)[(i//4)%4])
        put(VIEW,'Ii',CLIP,(0,1,2,3,4,5,6,9,10,16,21)[i%11])
        put(VIEW+0x40,'I',0 if i%19==0 else CAMERA);put(VIEW+0x58,'I',PORT)
        floats(MINIMUM,[-1,-2,-3]);floats(MAXIMUM,[1,2,3])
        yield dict(data=bytes(data),index=i%22,cw=(0x027f,0x037f)[(i//22)%2],
            overrides=[bits((0,0.25,2.5)[(i//8)%3]),bits((0,0,1.25)[(i//24)%3])],unused=rng.getrandbits(32),
            world=0 if i%3==0 else WORLD_MATRIX,
            output=(OUTPUT,LEFT,RIGHT,LEFT+4,RIGHT+4,LEFT-4,RIGHT-4)[i%7])


def verify(image,linked,gcc,n,kinds,angles):
    a,b=Machine(image),Machine(image,linked,gcc);counts,classes=Counter(),Counter()
    for i,c in enumerate(cases(n)):
        for k in kinds:
            try:x,y=a.run(k,c),b.run(k,c)
            except (AssertionError,UcError)as e:raise AssertionError(('view mismatch',k,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:12]
                raise AssertionError(('view mismatch',k,i,hex(c['cw']),offsets,x[2:],y[2:]))
            counts[k]+=1
            if k=='chain':classes.update([x[2]])
    for cw in (0x027f,0x037f):
        for i in range(angles):
            for k in ('sine','cosine'):
                units=i*65536//angles
                x,y=a.call(k,[units],cw=cw),b.call(k,[units],cw=cw)
                assert x==y,('view mismatch',k,units,hex(cw),x.hex(),y.hex())
                counts[k]+=1
    return dict(comparisons=dict(counts),classifications=dict(classes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--angles',type=int,default=65536);p.add_argument('--kind',choices=['all','construct','storage','update','multiply','chain'],default='all')
    p.add_argument('--replace',type=Path);p.add_argument('--replace-angle',type=Path);p.add_argument('--replace-matrix',type=Path)
    p.add_argument('--native-dump',type=Path)
    p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.gcc,args.replace or SOURCE,args.replace_angle or ANGLE,args.replace_matrix or MATRIX)
    kinds=['construct','storage','update','multiply','chain']if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,args.cases,kinds,args.angles)
    if args.native_dump:
        dump=args.native_dump.read_bytes();assert len(dump)==644 and dump[:8]==b'UG2VIEW1'
        camera,port,view,clip=dump[8:216],dump[216:244],bytearray(dump[244:356]),dump[356:644]
        index=struct.unpack_from('<i',view,4)[0]
        storage=struct.unpack_from('<I',view,0)[0]
        assert 0<=index<22 and storage==0x873380+index*0x120,('native view storage',index,hex(storage))
        focal=bytes(view[0xc:0x10])
        c=next(cases(1));data=bytearray(c['data'])
        struct.pack_into('<I',view,0,CLIP);struct.pack_into('<I',view,0x40,CAMERA);struct.pack_into('<I',view,0x58,PORT)
        for a,b in ((CAMERA,camera),(PORT,port),(VIEW,view),(CLIP,b'\x5a'*0x120)):data[a-ARENA:a-ARENA+len(b)]=b
        c.update(data=bytes(data),overrides=[0,0],unused=0,cw=0x027f)
        original=Machine(image);original.run('update',c)
        actual=bytes(original.u.mem_read(CLIP,0x120));actual_focal=bytes(original.u.mem_read(VIEW+0xc,4))
        assert actual==clip and actual_focal==focal,('native view mismatch',[(hex(i),v,w)for i,(v,w)in enumerate(zip(actual,clip))if v!=w][:12],actual_focal.hex(),focal.hex())
        report['native_replay']=dict(bytes=0x120+4,focal=struct.unpack('<f',focal)[0],fov=struct.unpack_from('<H',camera,0xc4)[0],
            view_index=index,storage_address=hex(storage),clip_sha256=hashlib.sha256(clip).hexdigest(),original_matches_native=True)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['scalar D3DX implementation only; CPU dispatch/optimized library variants and full renderer scheduling remain outside this verifier',
        'finite bounded camera/view/matrix records; native GPU/effect rendering and malformed/OOM/exception behavior are not established'])
    if args.mutation_checks:
        mutations={
            'wrong-view-index-stride':('construct','view','))/0x70;', '))/0x60;'),
            'missing-list-reset':('construct','view','link44.next00=link44.previous04=&link44;','link44.next00=link44.previous04=0;'),
            'wrong-constructor-scale':('construct','view','value18=1.0f;','value18=0;'),
            'incomplete-record-clear':('construct','view','i<0x70/4','i<0x68/4'),
            'wrong-storage-index':('storage','view','ug2RenderClipData873380+index','ug2RenderClipData873380+index+1'),
            'clear-combined-on-null-camera':('update','view','WheelIdentity(&matrices00[1]);\n        return;','WheelIdentity(&matrices00[1]); WheelIdentity(&matrices00[2]);\n        return;'),
            'missing-projection-reset':('update','view','    WheelIdentity(&matrices00[1]);\n    float *projection','    (void)0;\n    float *projection'),
            'wrong-zero-fov-focal':('update','view','tangent==0.0f ? 10.0f','tangent==0.0f ? 0.0f'),
            'reverse-viewport-aspect':('update','view','view->viewport58->height18)/view->viewport58->width14','view->viewport58->width14)/view->viewport58->height18'),
            'ignore-main-view-far-distance':('update','view','view->type04==1 ? 15000.0f : camera->farC0','camera->farC0'),
            'second-override-as-far':('update','view','if (secondOverride!=0.0f) nearValue=secondOverride;','if (secondOverride!=0.0f) farValue=secondOverride;'),
            'ignore-first-override':('update','view','if (firstOverride!=0.0f) nearValue=firstOverride;','(void)0;'),
            'omit-type-four-scaling':('update','view','type==1 || type==4 || type==2 || type==5','type==1 || type==2 || type==5'),
            'omit-fixed-aspect-types':('update','view','if (type==9 || type==10)','if (false)'),
            'wrong-horizontal-scale':('update','view','scaleX=cameraAspectScaleX7A27DC;','scaleX=2.0f*cameraAspectScaleX7A27DC;'),
            'round-odd-fov-up':('update','view','camera->fovC4>>1','(camera->fovC4+1)>>1'),
            'spill-cosine-return':('update','view','long double cosine=','volatile float cosine='),
            'omit-sine-spill':('update','view','volatile float sine=','long double sine='),
            'omit-depth-spill':('update','view','volatile float depthScale=','long double depthScale='),
            'retain-camera-fourth-column':('update','view','matrices00[0].elements[3]=matrices00[0].elements[7]=matrices00[0].elements[11]=0;','(void)0;'),
            'reverse-view-projection-product':('update','view','&matrices00[0],&matrices00[1]','&matrices00[1],&matrices00[0]'),
            'wrong-near-plane-column':('update','view','combined)[row+2]','combined)[row+3]'),
            'wrong-far-plane-sign':('update','view','else plane[j]=static_cast<long double>(combined[row+3])-combined[row+2];','else plane[j]=static_cast<long double>(combined[row+3])+combined[row+2];'),
            'leave-plane-distance-unnormalized':('update','view','plane[3]=inverse*plane[3];','(void)0;'),
            'wrong-normal-length':('update','view','(x*x+y*y)+z*z','(x*x+y*y)-z*z'),
            'spill-plane-reciprocal':('update','view','long double inverse=1.0f/length;','volatile float inverse=1.0f/length;'),
            'always-snapshot-matrix':('multiply','matrix','(output==left || output==right)','(true)'),
            'ignore-left-matrix-alias':('multiply','matrix','(output==left || output==right)','(output==right)'),
            'wrong-matrix-component':('multiply','matrix','static_cast<long double>(a[3])','static_cast<long double>(a[2])'),
            'mask-half-angle-domain':('update','angle','units&0xffffu','units&0x7fffu'),
            'wrong-polynomial-coefficient':('update','angle','x5*engineSineC5','x5*engineSineC7'),
            'wrong-quarter-turn':('update','angle','units+0x4000u','units+0x8000u'),
            'spill-angle-radians':('update','angle','long double x=','volatile float x='),
            'omit-ninth-order-term':('update','angle','+x7*x2*engineSineC9',''),
        }
        rejected=[];paths=dict(view=SOURCE,angle=ANGLE,matrix=MATRIX)
        for name,(kind,unit,before,after)in mutations.items():
            if kind not in kinds:continue
            body=paths[unit].read_text();assert before in body,name
            body=body.replace(before,after)
            for header in ('EngineAngleMath.h','D3DXMatrixScalar.h'):body=body.replace('"'+header+'"','"../Physics/'+header+'"')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            flag=dict(view='--replace',angle='--replace-angle',matrix='--replace-matrix')[unit]
            r=subprocess.run([sys.executable,__file__,flag,str(path),'--kind',kind,'--cases','256','--angles','512'if unit=='angle'else'0']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout)
            assert r.returncode and 'view mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
