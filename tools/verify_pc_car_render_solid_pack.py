#!/usr/bin/env python3
"""Verify complete solid chunks, texture/profile slots and resource registration.

Executes recovered platform loading and relocation in the same chain. Only GPU,
texture lookup and cross-pack solid/owner lookup use controlled boundaries.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE
import verify_pc_car_render_solid_load as platform

SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderSolidPack.cpp'
BUILD=ROOT/'build/analysis/car-render-solid-pack'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-solid-pack.json'
ARENA=platform.ARENA
PACK, HELPER, SLOTS, RESOURCES, PROFILES, OTHER = [ARENA+i for i in (0x800,0x1000,0x2000,0x4000,0x5000,0x6000)]
GLOBAL=dict(ug2SolidOffline864F60=0x864f60,ug2SolidFlag82DAA8=0x82daa8,
    ug2SolidAuxBytes82D38C=0x82d38c,ug2SolidBytes82D390=0x82d390,
    ug2SolidLoads82D384=0x82d384,ug2SolidChunkBytes82D388=0x82d388,ug2SolidChanged82DAB0=0x82dab0,
    ug2SolidLoadedCount830DA0=0x830da0,ug2SolidBuckets830DA4=0x830da4,
    ug2LoadedSolids835794=0x835794,ug2FailedSolids835858=0x835858,
    ug2LightProfiles8358C0=0x8358c0,ug2LightDefaultPack7F399C=0x7f399c)
BOUNDARIES=dict(lookup=platform.STOP+0x200,loaded=platform.STOP+0x210,owner=platform.STOP+0x220)
ORIGINAL_BOUNDARIES=dict(lookup=0x4901d0,loaded=0x48df90,owner=0x48df40)
BINDINGS=dict(UG2SolidFindTexture=BOUNDARIES['lookup'],UG2SolidFindLoaded=BOUNDARIES['loaded'],
              UG2SolidFindOwner=BOUNDARIES['owner'])
ORIGINAL=dict(outer=0x493bb0,resolve=0x4907a0,find=0x488bc0,profile=0x431780,allowed=0x40cb20)
PREFIX=dict(outer='?CarRenderLoadSolidChunk@@',resolve='?ResolveTextures@CarRenderSlotTable@@',find='?CarRenderFindResource@@',
    profile='?CarRenderProfileFromLink@@',allowed='?CarRenderSolidLoadAllowed@@')
GCC=dict(outer='_Z23CarRenderLoadSolidChunk',resolve='_ZN18CarRenderSlotTable15ResolveTexturesEv',find='_Z21CarRenderFindResource',
    profile='_Z24CarRenderProfileFromLink',allowed='_Z25CarRenderSolidLoadAllowed')


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    bindings=dict(platform.GLOBALS,**platform.BINDINGS,**GLOBAL,**BINDINGS)
    sources=[source,platform.SOURCE]
    if gcc:linked=link_native(sources,BUILD,ROOT,bindings,GCC['resolve'])
    else:
        objects=[]
        for i,path in enumerate(sources):
            obj=BUILD/('pack-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c',
                '/Ox','/Ob0','/EHsc','/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
        linked=link_objects(objects,{'_'+k:v for k,v in bindings.items()},start=0x2400000)
    prefixes=dict(GCC if gcc else PREFIX,load=platform.GCC['load']if gcc else platform.PREFIX['load'])
    linked['entries']={k:next(v for n,v in linked['exports'].items()if n.startswith(p))for k,p in prefixes.items()}
    return linked


class Machine(platform.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries=dict(ORIGINAL,load=platform.ORIGINAL['load'])
        for kind,address in (BOUNDARIES if linked else ORIGINAL_BOUNDARIES).items():
            self.u.hook_add(UC_HOOK_CODE,self.boundary,user_data=kind,begin=address,end=address)
        self.u.hook_add(UC_HOOK_CODE,self.failure_fixture,begin=self.entries['load'],end=self.entries['load'])

    def run(self,kind,case):
        gcc=self.gcc
        try:
            # The profile converter is explicitly fastcall on both compilers:
            # its only argument is ECX, with no receiver on the stack.
            if kind=='profile':self.gcc=False
            result=super().run(kind,case)
        finally:self.gcc=gcc
        # ResolveTextures has no return value; EAX is caller-clobbered.
        return result[:3]+(None,result[4]) if kind=='resolve' else result

    def failure_fixture(self,u,address,size,data):
        if not self.case.get('force_failure'):return
        sp=u.reg_read(xc.UC_X86_REG_ESP)
        n=2 if self.gcc else 1
        args=struct.unpack('<'+'I'*n,bytes(u.mem_read(sp+4,4*n)))
        receiver,chunk=args if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),args[0])
        self.events.append(('platform-failure-fixture',receiver,chunk))
        ret=struct.unpack('<I',bytes(u.mem_read(sp,4)))[0]
        u.reg_write(xc.UC_X86_REG_ESP,sp+4+(0 if self.gcc else 4))
        u.reg_write(xc.UC_X86_REG_EAX,0);u.reg_write(xc.UC_X86_REG_EIP,ret)

    def boundary(self,u,address,size,kind):
        sp=u.reg_read(xc.UC_X86_REG_ESP)
        count=dict(lookup=3,loaded=2,owner=1)[kind]
        args=struct.unpack('<'+'I'*count,bytes(u.mem_read(sp+4,count*4)))
        if kind=='lookup':
            assert args[1:]==(1,0)
            result=ARENA+0x7000+(args[0]%8)*4
        elif kind=='loaded':
            assert args[1]==0
            result=self.case.get('other',0)
        else:result=PACK
        self.events.append((kind,args,result))
        ret=struct.unpack('<I',bytes(u.mem_read(sp,4)))[0]
        u.reg_write(xc.UC_X86_REG_ESP,sp+4);u.reg_write(xc.UC_X86_REG_EAX,result);u.reg_write(xc.UC_X86_REG_EIP,ret)


def enrich(case,index,rng):
    data=bytearray(case['data'])
    def put(a,fmt,*values):struct.pack_into('<'+fmt,data,a-ARENA,*values)
    g=case['globals']
    g[0x864f60]=struct.pack('<I',index%5==0)
    g[0x82daa8]=struct.pack('<I',index%3)
    for a in (0x82d384,0x82d388,0x82d38c,0x82d390,0x82dab0,0x830da0):
        g[a]=struct.pack('<I',rng.choice((0,0xffffffff,rng.getrandbits(32))))
    for head in (0x835794,0x835858):
        tail=ARENA+0x6800+(head%128)
        put(tail,'2I',head,head)
        g[head]=struct.pack('<2I',tail,tail)
    g[0x830da4]=rng.randbytes(8192)
    g[0x7f399c]=struct.pack('<I',ARENA+0x6a00)
    profile_count=index%5
    g[0x8358c0]=struct.pack('<2I',PROFILES+4 if profile_count else 0x8358c0,
                           PROFILES+(profile_count-1)*0x100+4 if profile_count else 0x8358c0)
    for i in range(4):
        put(PROFILES+i*0x100+4,'2I',PROFILES+(i+1)*0x100+4 if i+1<profile_count else 0x8358c0,
            PROFILES+(i-1)*0x100+4 if i else 0x8358c0)
        put(PROFILES+i*0x100+0xc,'I',i%3)
    keys=sorted(rng.choice((0,1,2,0x7fffffff,0x80000000,0xffffffff))for _ in range(32))
    for i,key in enumerate(keys):put(RESOURCES+i*8,'2I',key,rng.getrandbits(32))
    put(PACK+0xc,'i',rng.choice((-1,0,32,32)))
    put(PACK+0x74,'I',RESOURCES)
    put(HELPER+0x19,'b',index%256-128)
    put(HELPER+0x2c,'I',SLOTS)
    for i in range(128):put(SLOTS+i*8,'2I',rng.getrandbits(32),rng.getrandbits(32))
    case['other']=OTHER if index%4 else 0
    case['calls']['resolve']=HELPER,[]
    base=rng.choice((0,RESOURCES,RESOURCES))
    count=rng.choice((-2147483648,-1,0,1,2,32))
    offset,stride=rng.choice(((0,8),(4,8),(8,12),(8,8),(0,0)))
    case['calls']['find']=0,[rng.choice(keys+[0x12345678]),base,count&0xffffffff,offset,stride]
    case['calls']['profile']=rng.choice((0,1,3,4,0xffffffff,rng.getrandbits(32))),[]
    case['calls']['allowed']=0,[]
    case['data']=data
    return case,keys


def cases(n):
    rng=random.Random(0x493bb0)
    for index,original in enumerate(platform.synthetic_cases(n)):
        original_start=original['calls']['load'][1][0]-ARENA
        length=struct.unpack_from('<I',original['data'],original_start+4)[0]+8
        platform_bytes=original['data'][original_start:original_start+length]
        case,keys=enrich(original,index,rng)
        data=bytearray(rng.randbytes(0x20000));data[:0x8000]=case['data'][:0x8000]
        start=0x8000+(index%32)*4;current=start+8
        header=bytearray(rng.randbytes(0xc0))
        struct.pack_into('<H',header,0xe,rng.getrandbits(16))
        struct.pack_into('<I',header,0x10,rng.choice(keys+[0x12345678]))
        header[0x19]=(index%256)
        header[0x1a]=((index*7)%256)
        current,solid=platform.append_chunk(data,current,0x134011,header,16)
        slots=b''.join(struct.pack('<2I',rng.getrandbits(32),rng.getrandbits(32))for _ in range(128))
        current,_=platform.append_chunk(data,current,0x134012,slots)
        slots=b''.join(struct.pack('<2I',rng.randrange(6),rng.getrandbits(32))for _ in range(128))
        current,_=platform.append_chunk(data,current,0x134013,slots)
        current,_=platform.append_chunk(data,current,0x134017,rng.randbytes(12))
        for kind,length in ((0x134018,(index%16)*4),(0x134019,(index%17)*4),(0x13401b,(index%13)*4)):
            current,_=platform.append_chunk(data,current,kind,rng.randbytes(length))
        markers=(0,1,2,257)[index%4]
        current,_=platform.append_chunk(data,current,0x13401a,rng.randbytes(markers*0x50),16)
        # Preserve alignment residue when nesting an existing platform fixture.
        padding=(original_start-current-8)%128
        current,_=platform.append_chunk(data,current,0x12345678,rng.randbytes(padding))
        assert current%128==original_start%128
        data[current:current+len(platform_bytes)]=platform_bytes;current+=len(platform_bytes)
        struct.pack_into('<2I',data,start,0x80134010 if index%31 else 0x12345678,current-start-8)
        case['data']=bytes(data);case['alias']=0
        case['force_failure']=index%19==1
        case['calls']['outer']=0,[ARENA+start,PACK]
        yield case


def retail_cases(path):
    raw=path.read_bytes();rng=random.Random(0x493bb0)
    def walk(begin,end):
        while begin+8<=end:
            kind,length=struct.unpack_from('<2I',raw,begin);limit=begin+8+length
            assert limit<=end
            if kind==0x80134010:yield begin,limit
            elif kind&0x80000000:yield from walk(begin+8,limit)
            begin=limit
        assert begin==end
    for index,(begin,end)in enumerate(walk(0,len(raw))):
        case,_=enrich(platform.base_case(rng,index),index,rng)
        start=0x8000+begin%128
        data=bytearray(rng.randbytes(max(0x10000,start+end-begin+128)))
        assert len(data)<0x40000
        data[:0x8000]=case['data'][:0x8000]
        data[start:start+end-begin]=raw[begin:end]
        case['data']=bytes(data);case['calls']={'outer':(0,[ARENA+start,PACK])}
        # A matching context index is a fixture, not recovered pack-manager
        # metadata. Duplicate keys witness the original first-entry binding.
        data=bytearray(case['data'])
        child=begin+8
        while child<end:
            kind,length=struct.unpack_from('<2I',raw,child)
            if kind==0x134011:
                header=(child+23)&~15
                key=struct.unpack_from('<I',raw,header+0x10)[0]
                struct.pack_into('<i',data,PACK-ARENA+0xc,0 if index%7==0 else 2)
                for i in range(2):struct.pack_into('<2I',data,RESOURCES-ARENA+i*8,key,OTHER)
                break
            child+=length+8
        case['data']=bytes(data)
        case['retail_offset']=hex(begin)
        case['globals'][platform.RULES]=struct.pack('<12I',1,4,0,0x40,2,0,0x80,5,0,0,0,0)
        yield case


def verify(image,linked,gcc,source,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);totals,events=Counter(),Counter()
    for index,case in enumerate(source):
        for kind in kinds:
            try:x,y=a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError) as e:
                raise AssertionError(('solid pack mismatch',kind,index,case.get('retail_offset'),str(e)))from e
            if x!=y:
                offsets=[(hex(i),u,v)for i,(u,v)in enumerate(zip(x[0],y[0]))if u!=v][:8]
                event=[(i,u,v)for i,(u,v)in enumerate(zip(x[4],y[4]))if u!=v][:1]
                raise AssertionError(('solid pack mismatch',kind,index,case.get('retail_offset'),offsets,x[3],y[3],event))
            totals[kind]+=1;events.update(e[0]for e in x[4])
    return dict(comparisons=dict(totals),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true')
    p.add_argument('--cases',type=int,default=2048);p.add_argument('--retail',type=Path)
    p.add_argument('--kind',choices=['all']+list(ORIGINAL),default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    assert args.cases > 0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc);kinds=list(ORIGINAL)if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,cases(args.cases),kinds)
    if args.retail:report['retail']=dict(sha256=hashlib.sha256(args.retail.read_bytes()).hexdigest(),
        **verify(image,linked,args.gcc,retail_cases(args.retail),['outer']))
    report.update(failures=0,byte_identity='not established',compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',
        limitations=['GPU, texture and cross-pack lookups are controlled boundaries; no native activation or visual parity',
            'synthetic caller failures override the normally successful platform-loader return; retail runs execute it',
            'finite valid child-order fixtures, not exhaustive malformed or aliasing coverage'])
    if args.mutation_checks:
        mutations={
            'unsigned-texture-count':('resolve','i < textureCount19','i < (unsigned char)textureCount19'),
            'ignore-offline-mode':('resolve','ug2SolidOffline864F60 == 0','true'),
            'last-duplicate':('find','while (middle > 0 &&','while (false && middle > 0 &&'),
            'signed-resource-key':('find','if (hash < key)','if ((int)hash < (int)key)'),
            'skip-resource-width':('find',' && offset+4 <= stride',''),
            'wrong-header-loaded-byte':('outer','solid->loaded0D = 1;','solid->value0C = 1;'),
            'unsigned-light-count':('outer','int count = solid->count1A;','int count = (unsigned char)solid->count1A;'),
            'ignore-profile-hash':('outer','profile->hash0C == slot->key','profile->hash0C != slot->key'),
            'wrong-profile-link-offset':('outer','static_cast<unsigned char *>(link)-4','static_cast<unsigned char *>(link)-8'),
            'wrong-marker-stride':('outer','bytes/0x50','bytes/0x40'),
            'ignore-platform-failure':('outer','if (!solid->LoadPlatformChunk(child)) allowed = false;','solid->LoadPlatformChunk(child);'),
            'skip-other-flag':('outer','other->flags0E |= 0x100;','(void)0;'),
            'wrong-bucket-mask':('outer','solid->hash10 & 0x1fff','solid->hash10 & 0xfff'),
            'skip-payload-statistic':('outer','ug2SolidChunkBytes82D388 += chunk[1];','(void)0;'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','256']+(['--gcc']if args.gcc else[])
            result=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'solid pack mismatch'in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
