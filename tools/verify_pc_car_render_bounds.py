#!/usr/bin/env python3
"""Compare original AABB transformation, six-plane classification and solid dispatch.

The recovered bounds bodies execute inside the dispatcher. Texture lookup and
successful pool allocation remain controlled; native clip-plane production and
rendering are not established by these bounded i386 fixtures.
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
from unicorn import UcError,UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
import verify_pc_car_render_solid as core

ROOT,EXE=core.ROOT,core.EXE
SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderBounds.cpp'
BUILD=ROOT/'build/analysis/car-render-bounds'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-bounds.json'
CLIP,MINIMUM,MAXIMUM=core.ARENA+0xe800,core.ARENA+0xe600,core.ARENA+0xe610
ORIGINAL=dict(core.ORIGINAL,bounds=0x5bc300,transform=0x5bc210)
PREFIX=dict(core.PREFIX,bounds='?TestBounds@CarRenderView@@',transform='?CarRenderTransformBounds@@')
GCC=dict(core.GCC,bounds='_ZN13CarRenderView10TestBoundsEPKfS1_P11WheelMatrix',transform='_Z24CarRenderTransformBounds')


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    bindings=dict(core.GLOBALS,UG2SolidFindTexture=core.BOUNDARIES['lookup'],fengZero7D7D3C=0x7d7d3c)
    if gcc:
        bindings['_Z8bOMallocPv']=core.BOUNDARIES['allocate']
        linked=link_native([source,core.SOURCE],BUILD,ROOT,bindings,GCC['bounds'])
    else:
        objects=[];bindings={'_'+k:v for k,v in bindings.items()};bindings.update(__fltused=0)
        for i,path in enumerate((source,core.SOURCE)):
            obj=BUILD/('bounds-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            for s in read_object(obj)['symbols'].values():
                if s['section']==0 and s['name'].startswith('?bOMalloc@@'):bindings[s['name']]=core.BOUNDARIES['allocate']
        linked=link_objects(objects,bindings,start=0x2400000)
    linked['entries']={k:next(a for name,a in linked['exports'].items()if name.startswith(p))for k,p in (GCC if gcc else PREFIX).items()}
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries=ORIGINAL
        self.coverage=Counter()
        if not linked:
            for a in (0x5bc3c0,0x5bc438):self.u.hook_add(UC_HOOK_CODE,self.cover,begin=a,end=a)
        # Base hooks already observe the original bounds entry. Other standalone
        # bodies have no observable call trace, allowing ordinary GCC inlining.
    def cover(self,u,address,size,unused):
        self.coverage[('test-'if address==0x5bc3c0 else'outside-')+str(u.reg_read(xc.UC_X86_REG_EDX))]+=1
    def boundary(self,u,address,size,kind):
        if kind=='bounds':
            receiver,args=self.member(3);self.events.append((kind,receiver,args));return
        return super().boundary(u,address,size,kind)
    def observe(self,u,address,size,kind):
        if kind=='bounds':
            if self.gcc or address!=0x5bc300:
                receiver,args=self.member(3);self.events.append((kind,receiver,args))
        elif kind!='transform':return super().observe(u,address,size,kind)
    def run(self,kind,case,cw):
        data,globals_,heads,calls=case;u=self.u;self.events=[];self.allocations=Counter()
        u.mem_write(core.ARENA,data)
        for address,body in self.initial.items():u.mem_write(address,body)
        for address,body in heads.items():u.mem_write(address,body)
        for address,body in globals_.items():u.mem_write(address,body)
        returns=[]
        for receiver,args in calls[kind]:
            sp=core.STACK+0x8000;actual=[receiver]+list(args)if self.gcc and receiver else args
            u.mem_write(sp,struct.pack('<'+'I'*(len(actual)+1),core.STOP,*actual))
            saved=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
            for i,r in enumerate(saved):u.reg_write(r,0xabcdef00+i)
            for r,v in [(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_FPCW,cw),
                        (xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
            u.emu_start(self.entries[kind],core.STOP,count=100000)
            assert u.reg_read(xc.UC_X86_REG_EIP)==core.STOP
            assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+(len(args)*4 if receiver and not self.gcc else 0)
            assert [u.reg_read(r)for r in saved]==[0xabcdef00+i for i in range(4)]
            assert u.reg_read(xc.UC_X86_REG_FPCW)==cw and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
            if kind=='bounds':returns.append(u.reg_read(xc.UC_X86_REG_EAX))
        regions=sorted(set(core.REGIONS)|{(a,8)for a in heads}|{(0x7d7d3c,4)})
        return bytes(u.mem_read(core.ARENA,core.SIZE)),[bytes(u.mem_read(a,n))for a,n in regions],returns,self.events


def cases(n):
    rng=random.Random(0x5bc300)
    for i,(data,globals_,heads,calls)in enumerate(core.cases(n)):
        data=bytearray(data)
        def put(a,fmt,*v):struct.pack_into('<'+fmt,data,a-core.ARENA,*v)
        def floats(a,v):put(a,'f'*len(v),*v)
        matrix=[rng.uniform(-3,3)for _ in range(16)]
        matrix[12:15]=[rng.uniform(-20,20)for _ in range(3)]
        for j in (3,7,11):matrix[j]=0
        matrix[15]=1
        low=[rng.uniform(-10,0)for _ in range(3)];high=[rng.uniform(0,10)for _ in range(3)]
        if i%13==0:low,high=high,low
        if i%5==0:matrix=[float(j%5==0)for j in range(16)]
        if i%8==0:
            low=[-1,-2,-3];high=[1,2,3];matrix=[float(j%5==0)for j in range(16)]
        if i%64==1:
            low=[1,1,1];high=[1+2**-23]*3;matrix=[float(j%5==0)for j in range(16)]
        floats(core.WORLD,matrix);floats(MINIMUM,low);floats(MAXIMUM,high)
        put(core.VIEW,'I',CLIP)
        for j in range(6):
            p=[0.0]*4;p[j//2]=1.0 if j%2==0 else -1.0;p[3]=1000
            floats(CLIP+0xc0+j*16,p)
        target=(i//8)%6 if i%8==0 else (i//64)%6 if i%64==1 else i%6
        plane=[rng.uniform(-2,2)for _ in range(4)]
        if i%4!=3:
            # Axis-aligned touching, outside and intersecting tests, at every
            # plane index. Non-unit normals and boundary ULPs are also sampled.
            plane=[0.0]*4;plane[target//2]=(1.0,-1.0,0.5,2.0)[(i//6)%4]
            plane[3]=(-20,-10,-1,0,1,10,20)[(i//24)%7]
        if i%8==0:
            plane=[0.0]*4;plane[target//2]=1
            radius=high[target//2]
            distance=radius if (i//144)%2 else -radius
            # Exactly on either plane-support boundary, or its adjacent binary32 ULP.
            word=struct.unpack('<I',struct.pack('<f',distance))[0]+(-1,0,1)[(i//48)%3]
            plane[3]=struct.unpack('<f',struct.pack('<I',word))[0]
        if i%64==1:plane=[0.0]*4;plane[target//2]=1;plane[3]=-1
        floats(CLIP+0xc0+target*16,plane)
        if i%11==0 and i%8 and i%64!=1:
            # Classification preserves unordered comparisons; no invented NaN reject.
            put(CLIP+0xc0+target*16+(i//11%4)*4,'I',(0x7fc12345,0xffc54321,0x7f800000,0xff800000)[(i//44)%4])
        globals_[0x7d7d3c]=struct.pack('<f',0 if i%17 or i%8==0 or i%64==1 else (0.25 if i%2 else -0.25))
        for j in range(32):
            a=core.PRIMITIVES+j*0x3c
            lo=[rng.uniform(-50,0)for _ in range(3)];hi=[rng.uniform(0,50)for _ in range(3)]
            floats(a,lo);floats(a+0x10,hi)
        world=0 if i%3==0 else core.WORLD
        calls['bounds']=[(core.VIEW,[MINIMUM,MAXIMUM,world])]
        # Overlap exercises the original final store order; all matrix and bound
        # inputs are consumed before those stores.
        alias=i%6
        a,b=(MINIMUM,MAXIMUM)if alias==0 else (MINIMUM,MINIMUM)if alias==1 else (MINIMUM,MINIMUM+4)if alias==2 else (core.WORLD,MAXIMUM)if alias==3 else (MINIMUM,core.WORLD+4)if alias==4 else (core.WORLD+0x30,core.WORLD+0x34)
        calls['transform']=[(0,[core.WORLD,a,b])]
        # Independent blocks cover every plane, either support boundary and
        # its adjacent ULPs under both x87 precision-control settings.
        yield (bytes(data),globals_,heads,calls),(0x027f,0x037f)[(i//288)%2]


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts,outcomes,events=Counter(),Counter(),Counter()
    for i,(case,cw)in enumerate(cases(n)):
        for kind in kinds:
            try:x,y=a.run(kind,case,cw),b.run(kind,case,cw)
            except (AssertionError,UcError)as e:raise AssertionError(('bounds mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:12]
                ev=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('bounds mismatch',kind,i,hex(cw),offsets,x[2],y[2],ev,len(x[3]),len(y[3])))
            counts[kind]+=1
            if kind=='bounds':outcomes.update(x[2])
            events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),classifications=dict(outcomes),observed_calls=dict(events),original_plane_coverage=dict(a.coverage))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=4096)
    p.add_argument('--kind',choices=['all','bounds','transform','dispatch'],default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc);kinds=['bounds','transform','dispatch']if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,args.cases,kinds)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['bounded synthetic boxes, matrices and clip planes; native plane production and rendering remain open',
                     'texture lookup and successful queue pool allocations are controlled in the real dispatcher chain'])
    if args.mutation_checks:
        mutations={
            'ignore-world-matrix':('bounds','if (world) CarRenderTransformBounds(world,low,high);','(void)0;'),
            'skip-sixth-plane':('bounds','i<6','i<5'),
            'outside-touch-as-outside':('bounds','distance+radius<fengZero7D7D3C','distance+radius<=fengZero7D7D3C'),
            'inside-touch-as-intersecting':('bounds','distance-radius<fengZero7D7D3C','distance-radius<=fengZero7D7D3C'),
            'collapse-inside-result':('bounds','intersects ? 1 : 2','intersects ? 1 : 1'),
            'ignore-intersection':('bounds','intersects=true;','intersects=false;'),
            'ignore-radius':('bounds','distance+radius<fengZero7D7D3C','distance<fengZero7D7D3C'),
            'signed-normal-radius':('bounds','x=fabs(static_cast<double>(p[0]))','x=p[0]'),
            'drop-plane-offset':('bounds',')+p[3];',');'),
            'reverse-extent':('bounds','static_cast<long double>(high[i])-center[i]','static_cast<long double>(center[i])-high[i]'),
            'repeat-first-plane':('bounds','clip00->planesC0[i]','clip00->planesC0[0]'),
            'wrong-center-scale':('bounds','low[i])*0.5f','low[i])*0.25f'),
            'omit-float-center-spills':('bounds','volatile float center[3],extent[3];','long double center[3]; volatile float extent[3];'),
            'wrong-world-translation':('transform','minX=m[12]','minX=m[13]'),
            'ignore-max-translation':('transform','maxX=m[12],maxY=m[13],maxZ=m[14]','maxX=0,maxY=0,maxZ=0'),
            'omit-high-product-spill':('transform','volatile float high','long double high'),
            'reverse-product-selection':('transform','if (low<high)','if (low>high)'),
            'drop-third-axis':('transform','i<3','i<2'),
            'transpose-matrix-column':('transform','minimum[i])*m[i*4+1]','minimum[i])*m[i+4]'),
            'store-before-all-inputs':('transform','for (int i=0;i<3;++i) {','minimum[0]=minX; for (int i=0;i<3;++i) {'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (path.with_suffix('.log')).write_text(r.stdout)
            assert r.returncode and 'bounds mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
