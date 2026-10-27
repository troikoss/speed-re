#!/usr/bin/env python3
"""Verify 0x5ed400's two virtual body queries and record-centre fallback."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE

SOURCE=ROOT/'src/Speed/Indep/Src/Physics/Collision/CollisionCentre.cpp'
BUILD=ROOT/'build/analysis/collision-centre'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/collision-centre.json'
ARENA,SIZE,STACK,STOP=0x2000000,0x10000,0x4800000,0x4810000
CAR,MOVER,OTHER,VTABLE,BODY,OTHER_BODY,RECORD,OTHER_RECORD=[ARENA+x for x in (0,0x1000,0x1100,0x1200,0x2000,0x3000,0x4000,0x4100)]
CALLBACK=STOP+0x1000


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    if gcc:
        linked=link_native(source,BUILD,ROOT,dict(UG2CollisionMoverGetBody=CALLBACK),'_Z18CarCollisionCentreP14MoverModeOwner',
            text_address=0x5000000,include_dirs=[SOURCE.parent])
        prefix='_Z18CarCollisionCentre'
    else:
        obj=BUILD/'centre.obj'
        subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
            '/Isrc/Speed/Indep/Src/Physics/Collision','/Fo'+str(obj.relative_to(ROOT)),str(source.relative_to(ROOT))],cwd=ROOT,check=True)
        linked=link_objects([obj],{'_UG2CollisionMoverGetBody':CALLBACK},start=0x5000000);prefix='?CarCollisionCentre@@'
    linked['entry']=next(a for n,a in linked['exports'].items()if n.startswith(prefix));return linked


class Machine:
    def __init__(self,image,linked=None):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32);self.rebuilt=bool(linked)
        for a,n in [(0,4096),(0x400000,(len(image)+4095)&~4095),(ARENA,SIZE),(STACK,0x20000)]:u.mem_map(a,n)
        u.mem_write(0x400000,image)
        self.entry=linked['entry']if linked else 0x5ed400
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        u.hook_add(UC_HOOK_CODE,self.callback,begin=CALLBACK,end=CALLBACK)
        if not linked:u.hook_add(UC_HOOK_CODE,self.null,begin=0x50d510,end=0x50d510)
    def put(self,a,*v):self.u.mem_write(a,struct.pack('<'+'I'*len(v),*v))
    def word(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
    def null(self,u,address,size,unused):self.events.append((MOVER,0)) # Original XOR EAX,EAX; RET executes.
    def callback(self,u,address,size,unused):
        sp=u.reg_read(xc.UC_X86_REG_ESP);mover=self.word(sp+4)if self.rebuilt else u.reg_read(xc.UC_X86_REG_ECX)
        assert mover in (MOVER,OTHER),('bad virtual receiver',hex(mover))
        if self.word(mover)==0x7a5bb0:result=0
        else:
            result=0 if not self.events and self.case['first_zero']else BODY if mover==MOVER else OTHER_BODY
            if not self.events:
                self.put(CAR+0x20,OTHER_RECORD)
                if self.case['swap']:self.put(CAR+0x34,OTHER)
        self.events.append((mover,result));pc=self.word(sp)
        u.reg_write(xc.UC_X86_REG_EAX,result);u.reg_write(xc.UC_X86_REG_ESP,sp+4);u.reg_write(xc.UC_X86_REG_EIP,pc)
    def run(self,c):
        self.case=c;self.events=[];u=self.u;u.mem_write(ARENA,c['data'])
        self.put(CAR+0x20,RECORD);self.put(CAR+0x34,0 if c['mode']==0 else MOVER)
        self.put(MOVER,0x7a5bb0 if c['mode']==1 else VTABLE);self.put(OTHER,VTABLE);self.put(VTABLE+12,CALLBACK)
        # Reproduce the stale adjacent resource-name bytes that exposed the
        # old nonvirtual +0x20 read past a 12-byte null mover.
        self.put(MOVER+0x20,0x5f494639);self.put(OTHER+0x20,0x5f494639)
        sp=STACK+0xf000;self.put(sp,STOP,CAR)
        regs=[xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP];seeds=[0x11223344,0x22334455,0x33445566,0x44556677]
        for r,v in zip(regs,seeds):u.reg_write(r,v)
        u.reg_write(xc.UC_X86_REG_ECX,CAR);u.reg_write(xc.UC_X86_REG_ESP,sp);u.reg_write(xc.UC_X86_REG_EFLAGS,0x202)
        u.reg_write(xc.UC_X86_REG_FPCW,0x27f);u.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
        u.emu_start(self.entry,STOP,count=1000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4
        assert [u.reg_read(r)for r in regs]==seeds
        assert u.reg_read(xc.UC_X86_REG_FPCW)==0x27f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        return bytes(u.mem_read(ARENA,SIZE)),u.reg_read(xc.UC_X86_REG_EAX),self.events


def verify(image,linked,n):
    a,b=Machine(image),Machine(image,linked);rng=random.Random(0x5ed400);counts={}
    for i in range(n):
        c=dict(data=rng.randbytes(SIZE),mode=i%4,first_zero=i%7==0,swap=bool(i&2))
        try:x,y=a.run(c),b.run(c)
        except (AssertionError,UcError)as e:raise AssertionError(('centre mismatch',i,str(e)))from e
        assert x==y,('centre mismatch',i,x[1:],y[1:])
        key=str(len(x[2]))+' virtual calls';counts[key]=counts.get(key,0)+1
    return dict(comparisons=n,virtual_call_counts=counts)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=4096)
    p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();m=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for f in m['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    report=verify(image,build(args.replace or SOURCE,args.gcc),args.cases)
    report.update(failures=0,byte_identity='not established',compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',
        limitations=['virtual callback effects are controlled except the real 0x50d510 null-return body',
        'finite pointer/record fixtures; full mover lifecycle and malformed pointers are not established'])
    if args.mutation_checks:
        mutations={
            'nonvirtual-body-read':('UG2CollisionMoverGetBody(car->mover34)','car->mover34->GetBody()'),
            'cache-mover':('    if (car->mover34 &&','    VehicleMover *saved=car->mover34;\n    if (saved &&'),
            'omit-second-call':('return &UG2CollisionMoverGetBody(car->mover34)->centre1F0;','return reinterpret_cast<WheelVector *>(reinterpret_cast<unsigned int>(car->mover34)+0x1f0);'),
            'wrong-centre-offset':('return &UG2CollisionMoverGetBody(car->mover34)->centre1F0;','return reinterpret_cast<WheelVector *>(reinterpret_cast<unsigned int>(UG2CollisionMoverGetBody(car->mover34))+0x1e0);'),
            'stale-fallback':('    if (car->mover34 &&','    CarPhysicsRecord *record=car->record20;\n    if (car->mover34 &&'),
        }
        rejected=[]
        for name,(before,after)in mutations.items():
            body=SOURCE.read_text();assert before in body;body=body.replace(before,after)
            if name=='cache-mover':body=body.replace('UG2CollisionMoverGetBody(car->mover34)','UG2CollisionMoverGetBody(saved)')
            if name=='stale-fallback':body=body.replace('reinterpret_cast<WheelVector *>(car->record20)','reinterpret_cast<WheelVector *>(record)')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--cases','128','--replace',str(path)]+(['--gcc']if args.gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'centre mismatch'in r.stdout,(name,r.stdout);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
