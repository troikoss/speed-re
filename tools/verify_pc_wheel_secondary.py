#!/usr/bin/env python3
"""Compare original secondary wheel draw, quad initialization and attribute helpers.

Only texture lookup and quad packing/render submission are controlled. Original
Y rotation/angle/scalar D3DX execute on the reference; their recovered bodies
execute on the rebuilt side. Rebuilt attribute
access executes recovered FindRecord; quad padding is explicitly excluded.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
import verify_pc_car_render_view as scalar
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import read_object,link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE
WORLD_DIR=ROOT/'src/Speed/Indep/Src/World';SOURCE=WORLD_DIR/'CarWheelSecondary.cpp'
BUILD=ROOT/'build/analysis/wheel-secondary';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/wheel-secondary.json'
ARENA,SIZE,STACK,STOP=0x2000000,0x10000,0x2100000,0x2120000
RI,CAR,PART,TABLE,INDICES,RECORDS,VIEW,WORLD,QUAD,TEXTURE=[ARENA+x for x in(0,0x3000,0x4000,0x5000,0x5100,0x5200,0x6000,0x6100,0x6200,0x6300)]
ORIGINAL=dict(secondary=0x620600,attribute=0x61b500,rim=0x4fe120,initialize=0x5bb1c0,wrapper=0x5c7e80,flags=0x622eee,rotate=0x5bada0)
PREFIX=dict(secondary='?Fn620600@CarRenderWheelInfo@@',attribute='?AttributeRaw@CarRenderPart@@',rim='?RimValue@CarRenderPart@@',initialize='?Initialize@CarRenderQuad@@',wrapper='?DrawQuad@CarRenderView@@',flags='?CarWheelCallerFlags@@',rotate='?RotateMatrixY@@')
GCC=dict(secondary='_ZN18CarRenderWheelInfo8Fn620600',attribute='_ZN13CarRenderPart12AttributeRaw',rim='_ZN13CarRenderPart8RimValue',initialize='_ZN13CarRenderQuad10Initialize',wrapper='_ZN13CarRenderView8DrawQuad',flags='_Z19CarWheelCallerFlags',rotate='_Z13RotateMatrixY')
GLOBALS=dict(ug2CarPartTable8A1CC8=0x8a1cc8,ug2WheelSecondaryScale7A889C=0x7a889c,ug2WheelSecondaryScale7A6728=0x7a6728,ug2WheelSecondaryY7A672C=0x7a672c,ug2WheelSecondaryTexture7A88A0=0x7a88a0)
CDECL={'flags','rotate'}
QUAD_FIELDS=tuple(j for j in range(0x71)if j not in sum((list(range(i*16+12,i*16+16))for i in range(4)),[]))

def build(gcc,source):
    BUILD.mkdir(parents=True,exist_ok=True);sources=[source,WORLD_DIR/'CarRenderRecords.cpp',ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp',scalar.PHYSICS/'WheelRotateY.cpp',scalar.ANGLE,scalar.MATRIX,ROOT/'src/Platform/Native/NativeMatrixDispatch.cpp']
    if gcc:
        bindings=dict(GLOBALS,**scalar.FLOAT_GLOBALS,UG2SolidFindTexture=0x4901d0,_Z19CarRenderSubmitQuadP13CarRenderQuadP16CarRenderTextureP11WheelMatrixj=0x5c5580)
        linked=link_native(sources,BUILD,ROOT,bindings,'_ZN18CarRenderWheelInfo8Fn620600EP13CarRenderViewP11WheelMatrix')
    else:
        bindings={'_'+k:v for k,v in GLOBALS.items()};bindings.update({'?'+k+'@@3MA':v for k,v in scalar.FLOAT_GLOBALS.items()});bindings.update(__fltused=0,_UG2SolidFindTexture=0x4901d0);objects=[]
        for i,p in enumerate(sources):
            obj=BUILD/('secondary-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc','/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section']!=0:continue
                for name,address in(('CarRenderSubmitQuad',0x5c5580),):
                    if symbol['name'].startswith('?'+name+'@@'):bindings[symbol['name']]=address
        linked=link_objects(objects,bindings,start=0x2400000)
    linked['entries']={k:next(a for n,a in linked['exports'].items()if n.startswith(p))for k,p in(GCC if gcc else PREFIX).items()}
    linked['rotation']=next(a for n,a in linked['exports'].items()if n.startswith('_Z13RotateMatrixY'if gcc else'?RotateMatrixY@@'))
    return linked

def masked_quad(data):return bytes(data[j]for j in QUAD_FIELDS)

class Machine:
    def __init__(self,image,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image);u.mem_map(ARENA,SIZE);u.mem_map(STACK,0x30000)
        self.linked=linked;self.gcc=gcc;self.entries=linked['entries']if linked else ORIGINAL
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        u.hook_add(UC_HOOK_CODE,self.texture,begin=0x4901d0,end=0x4901d0)
        u.hook_add(UC_HOOK_CODE,self.submit,begin=0x5c5580,end=0x5c5580)
        rotation=linked['rotation']if linked else 0x5bada0
        u.hook_add(UC_HOOK_CODE,self.rotation,begin=rotation,end=rotation)
        u.hook_add(UC_HOOK_CODE,self.stop_flags,begin=0x622f0d,end=0x622f0d)
        self.put(0x803e74,0x63cee1)
    def put(self,a,*values):self.u.mem_write(a,struct.pack('<'+'I'*len(values),*(v&0xffffffff for v in values)))
    def args(self,n):return struct.unpack('<'+'I'*n,self.u.mem_read(self.u.reg_read(xc.UC_X86_REG_ESP)+4,n*4))
    def ret(self,value=0):
        sp=self.u.reg_read(xc.UC_X86_REG_ESP);pc,=struct.unpack('<I',self.u.mem_read(sp,4));self.u.reg_write(xc.UC_X86_REG_EAX,value);self.u.reg_write(xc.UC_X86_REG_ESP,sp+4);self.u.reg_write(xc.UC_X86_REG_EIP,pc)
    def texture(self,u,a,s,d):self.events.append(('texture',self.args(3)));self.ret(TEXTURE if self.case['texture']else 0)
    def submit(self,u,a,s,d):
        quad,texture,world,fourth=self.args(4);self.events.append(('submit',masked_quad(bytes(u.mem_read(quad,0x74))),texture,world,fourth));self.ret()
    def rotation(self,u,a,s,d):
        out,matrix,angle=self.args(3);self.events.append(('rotation',angle,out==matrix,bytes(u.mem_read(matrix,64))))
    def stop_flags(self,u,a,s,d):u.emu_stop()
    def run(self,kind,c):
        u=self.u;self.case=c;self.events=[];u.mem_write(ARENA,c['data']);self.put(0x8a1cc8,TABLE)
        sp=STACK+0x10000;regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=[0x12345678+j for j in range(4)]
        for reg,value in zip(regs,saved):u.reg_write(reg,value)
        for reg,value in((xc.UC_X86_REG_FPCW,c['control']),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff),(xc.UC_X86_REG_MXCSR,0x1f80)):u.reg_write(reg,value)
        receiver=dict(secondary=RI,attribute=PART,rim=PART,initialize=QUAD,wrapper=VIEW).get(kind,0)
        args=dict(secondary=[VIEW,WORLD],attribute=[c['key'],c['fallback']],rim=[],initialize=[],wrapper=[QUAD,TEXTURE,WORLD,c['fourth'],c['fifth']],flags=[c['gate']],rotate=[WORLD if c['alias']else QUAD,WORLD,c['angle']])[kind]
        if self.gcc and kind not in CDECL:args=[receiver,*args]
        self.put(sp,STOP,*args);u.reg_write(xc.UC_X86_REG_ESP,sp);u.reg_write(xc.UC_X86_REG_ECX,receiver)
        if kind=='flags'and not self.linked:
            u.reg_write(xc.UC_X86_REG_ESI,c['gate']);u.reg_write(xc.UC_X86_REG_EAX,0x76543210);u.emu_start(ORIGINAL[kind],STOP,count=30);assert u.reg_read(xc.UC_X86_REG_EIP)==0x622f0d
            result=struct.unpack('<I',u.mem_read(sp+0xcc,4))[0]
        else:
            u.emu_start(self.entries[kind],STOP,count=50000);assert u.reg_read(xc.UC_X86_REG_EIP)==STOP
            assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+(0 if self.gcc or kind in CDECL else len(args)*4)
            assert[u.reg_read(r)for r in regs]==saved
            result=u.reg_read(xc.UC_X86_REG_EAX)if kind in('attribute','rim','flags')else None
        assert u.reg_read(xc.UC_X86_REG_FPCW)==c['control']and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        return(result,bytes(u.mem_read(ARENA,SIZE)),self.events)

def cases(n):
    rng=random.Random(0x620600)
    for i in range(n):
        data=bytearray(rng.randbytes(SIZE));put=lambda a,*v:struct.pack_into('<'+'I'*len(v),data,a-ARENA,*(x&0xffffffff for x in v))
        put(RI+4,CAR);put(CAR+0x604,PART if i<3072 or i%5 else 0);struct.pack_into('<H',data,PART+0xa-ARENA,0xffff if i>=3072 and i%13==0 else 0)
        put(TABLE+0x14,INDICES);put(TABLE+0x1c,RECORDS);struct.pack_into('<5h',data,INDICES-ARENA,4,0,1,2,3)
        payload=(rng.getrandbits(24)<<8)|(i&255);put(RECORDS,0x12345678,0xaabbccdd,0xeb0101e2,payload,0xeb0101e2,~payload,0x87654321,0x7ff01234)
        if i>=3072 and i%11==0:put(RECORDS+8,0xabcdef01,payload);put(RECORDS+16,0xabcdef02,~payload)
        struct.pack_into('<16f',data,WORLD-ARENA,*(rng.uniform(-8192,8192)for _ in range(16)))
        yield dict(data=bytes(data),angle=(0,0x1999,0x4000,0x8000,0xffff,rng.getrandbits(32))[i%6],alias=i%2,key=(0,0x12345678,0xeb0101e2,0x87654321,0x11111111)[i%5],fallback=rng.getrandbits(32),texture=i%3!=0,fourth=rng.getrandbits(32),fifth=rng.getrandbits(32),gate=(0,3,5,6,7,0xffffffff,0x80000000,rng.getrandbits(32))[i%8],control=(0x7f,0x27f,0x37f)[i//256%3]|((i//768)%4)*0x400)

def native_row(machine,c,i):
    machine.run('secondary',c);event=next(e for e in machine.events if e[0]=='submit');data=c['data']
    part=struct.unpack_from('<I',data,CAR+0x604-ARENA)[0];index=struct.unpack_from('<H',data,PART+0xa-ARENA)[0]
    key,payload=struct.unpack_from('<2I',data,RECORDS+8-ARENA)
    mode=0 if not part else 2 if index==0xffff else 3 if key!=0xeb0101e2 else 1
    flags=machine.run('flags',c)[0]
    return struct.pack('<7I',i,c['control'],mode,payload,c['gate'],int(c['texture']),flags)+event[1]+bytes(3)

def mutation_checks(gcc):
    mutations={
        'signed-rim':('static_cast<signed char>(part->RimValue())','static_cast<unsigned char>(part->RimValue())'),
        'absent-rim-default':(': 17.0f;',': 0.0f;'),
        'rim-attribute-key':('AttributeRaw(0xeb0101e2,0)','AttributeRaw(0xeb0101e3,0)'),
        'attribute-fallback':('return record ? record[1] : fallback','return record ? record[1] : 0'),
        'secondary-angle':('RotateMatrixY(&rotation,&rotation,0x1999)','RotateMatrixY(&rotation,&rotation,0x2000)'),
        'radius-spill':('volatile float positive=size;','volatile float positive=scaledRim;'),
        'corner-order':('quad.position00[2][0]=x2','quad.position00[2][0]=x3'),
        'quad-colour':('colour60[i]=0x80808080','colour60[i]=0xffffffff'),
        'wrapper-fourth':('CarRenderSubmitQuad(quad,texture,world,fourth)','CarRenderSubmitQuad(quad,texture,world,0)'),
        'caller-flag-gate':('gate==6 ? 0x8000','gate==3 ? 0x8000'),
    }
    rejected=[]
    for name,(before,after)in mutations.items():
        content=SOURCE.read_text();assert before in content
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        run=subprocess.run([sys.executable,__file__,'--cases','4096','--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'wheel secondary mismatch'in run.stdout,(name,run.returncode,run.stdout[-3000:]);rejected.append(name)
    return rejected

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=4096);p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();m=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for row in[*m['functions'].values(),*m['data'].values(),m['flag_fragment']]:
        a=int(row['address'],16)-0x400000;assert hashlib.sha256(image[a:a+row['size']]).hexdigest()==row['sha256']
    linked=build(args.gcc,args.replace or SOURCE);a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    for i,c in enumerate(cases(args.cases)):
        for kind in ORIGINAL:
            x,y=a.run(kind,c),b.run(kind,c);assert x==y,('wheel secondary mismatch',kind,i,c['control'],[(u[0],v[0])for u,v in zip(x[2],y[2])if u!=v],x[0],y[0]);counts[kind]+=1
    if args.write_native_fixtures:
        fixtures=b'UG2WHQS1'+struct.pack('<I',args.cases)
        for i,c in enumerate(cases(args.cases)):fixtures+=native_row(a,c,i)
        args.write_native_fixtures.write_bytes(fixtures)
    report=dict(comparisons=dict(counts),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',boundary='texture lookup and quad packing/submission controlled; recovered rotation, angle and scalar D3DX execute on rebuilt side',byte_identity='not established')
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
