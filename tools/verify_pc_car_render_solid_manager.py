#!/usr/bin/env python3
"""Compare original pack lifecycle, real resource lookups and reference movement.

Includes complete retail root load/unload with its actual resource-index context.
GPU APIs, node-pool allocation/free, clock acquisition, texture lookup and the
already recovered return-this leaf are the explicit platform/library boundaries.
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
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
from pc_coff import link_objects,read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE
import verify_pc_car_render_solid_load as platform
import verify_pc_car_render_solid_pack as pack

# Analysis-only larger address map: the complete retail file and staged GPU
# copies must not overlap code, the stack, GPU object tokens or allocated nodes.
platform.SIZE=0x2000000
platform.STACK,platform.STOP=0x4800000,0x4810000
platform.CALLS={k:platform.STOP+0x100+i*16 for i,k in enumerate(('vertex','index','lock','unlock'))}
platform.BINDINGS=dict(UG2SolidCreateVertexBuffer=platform.CALLS['vertex'],UG2SolidCreateIndexBuffer=platform.CALLS['index'],
    UG2SolidLockBuffer=platform.CALLS['lock'],UG2SolidUnlockBuffer=platform.CALLS['unlock'])
SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderSolidManager.cpp'
BUILD=ROOT/'build/analysis/car-render-solid-manager'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-solid-manager.json'
ARENA=platform.ARENA
GLOBALS=dict(ug2SolidPacks830D1C=0x830d1c,ug2UnresolvedSolids82DBC0=0x82dbc0,ug2OfflineSolids832DA4=0x832da4,
    ug2QueuedSolids872B44=0x872b44,ug2VertexReleases870D28=0x870d28,ug2SolidPackCount82D380=0x82d380,
    ug2SolidTimerShift8284E4=0x8284e4,ug2SolidTimerScale828028=0x828028,ug2SolidLookupTime82DAC0=0x82dac0,
    ug2SolidResolveMax82DA8C=0x82da8c,ug2SolidResolveSum82DA94=0x82da94,ug2SolidDetachMax82DA90=0x82da90,
    ug2SolidDetachSum82DA98=0x82da98,ug2SolidRefTime82DA9C=0x82da9c,ug2SolidResolveCount82DAA0=0x82daa0,
    ug2SolidDetachCount82DAA4=0x82daa4)
BOUNDARY={k:platform.STOP+0x200+i*16 for i,k in enumerate(('lookup','ticker','release','new','free','end'))}
ORIGINAL_BOUNDARY=dict(lookup=0x4901d0,ticker=0x43bdf0,new=0x440ec0,free=0x440ef0,end=0x502fd0)
NAMES=dict(load_root='CarRenderLoadSolidPack',unload_root='CarRenderUnloadSolidPack',load_header='CarRenderLoadPackHeader',
    unload_header='CarRenderUnloadPackHeader',clear_header='CarRenderClearPackLists',find_loaded='UG2SolidFindLoaded',
    find_owner='UG2SolidFindOwner',resolve='CarRenderResolveSolidReferences',detach='CarRenderDetachSolidReferences',
    unload_solid='CarRenderUnloadSolidChunk',walk_platform='CarRenderWalkPackPlatform',elapsed='CarRenderSolidElapsed')
VOID={'clear_header','resolve','detach','unload_solid','defer'}


def build(source,gcc):
    bindings=dict(platform.GLOBALS,**platform.BINDINGS,**pack.GLOBAL,**GLOBALS,
        UG2SolidFindTexture=BOUNDARY['lookup'],UG2SolidReadTicker=BOUNDARY['ticker'],UG2SolidReleaseIndexBuffer=BOUNDARY['release'])
    sources=[source,pack.SOURCE,platform.SOURCE]
    if gcc:
        bindings.update({'_Z12bNewListNodev':BOUNDARY['new'],'_Z15bDeleteListNodeP14WCollisionLink':BOUNDARY['free'],
                         '_ZN18CarRenderLightList3EndEv':BOUNDARY['end']})
        linked=link_native(sources,BUILD,ROOT,bindings,'_Z22CarRenderLoadSolidPackPj',text_address=0x5000000)
    else:
        BUILD.mkdir(parents=True,exist_ok=True);objects=[]
        externals={'_'+k:v for k,v in bindings.items()}
        for i,path in enumerate(sources):
            obj=BUILD/('manager-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section']!=0:continue
                for prefix,kind in [('?bNewListNode@@','new'),('?bDeleteListNode@@','free'),('?End@CarRenderLightList@@','end')]:
                    if symbol['name'].startswith(prefix):externals[symbol['name']]=BOUNDARY[kind]
        linked=link_objects(objects,externals,start=0x5000000)
    linked['entries']={k:next(v for n,v in linked['exports'].items()if name in n and not n.startswith('ug2'))for k,name in NAMES.items()}
    for kind,needle in [('defer','CarRenderDeferredBufferList3Add'if gcc else'?Add@CarRenderDeferredBufferList@@'),
                        ('unload_platform','CarRenderSlotTable19UnloadPlatformChunk'if gcc else'?UnloadPlatformChunk@CarRenderSlotTable@@')]:
        linked['entries'][kind]=next(v for n,v in linked['exports'].items()if needle in n)
    return linked


class Machine(platform.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries={k:int(f['address'],16)for k,f in manifest['functions'].items()}
        for kind,a in (BOUNDARY if linked else dict(ORIGINAL_BOUNDARY,release=BOUNDARY['release'])).items():
            self.u.hook_add(UC_HOOK_CODE,self.boundary,user_data=kind,begin=a,end=a)
        self.u.mem_write(platform.BUFFER_TABLE+8,struct.pack('<I',BOUNDARY['release']))
        for label,offset,functions in [('cycle',0x1000,['load_root','unload_root']),
                                       ('header_cycle',0x1100,['load_header','unload_header'])]:
            driver=platform.STOP+offset;code=bytearray()
            for kind in functions:
                code+=bytes.fromhex('ff742404')
                code+=b'\xe8'+struct.pack('<i',self.entries[kind]-(driver+len(code)+5))+bytes.fromhex('83c404')
            code+=b'\xc3';self.u.mem_write(driver,bytes(code));self.entries[label]=driver

    def boundary(self,u,address,size,kind):
        sp=u.reg_read(xc.UC_X86_REG_ESP);pop=0;result=0
        if kind=='end':
            result=struct.unpack('<I',bytes(u.mem_read(sp+4,4)))[0]if self.gcc else u.reg_read(xc.UC_X86_REG_ECX)
        elif kind=='ticker':
            result=(self.case.get('ticker',0xfffffff0)+self.ticks*self.case.get('tick_step',23))&0xffffffff
            self.ticks+=1;self.events.append((kind,result))
        elif kind=='new':
            result=ARENA+0xe00000+len(self.nodes)*16;self.nodes.append(result)
            u.mem_write(result,b'\x93'*16);self.events.append((kind,result))
        elif kind=='free':
            node=struct.unpack('<I',bytes(u.mem_read(sp+4,4)))[0]
            self.events.append((kind,node,bytes(u.mem_read(node,12))));u.mem_write(node,struct.pack('<I',0xdeadbeef))
        elif kind=='lookup':
            args=struct.unpack('<3I',bytes(u.mem_read(sp+4,12)));assert args[1:]==(1,0)
            result=ARENA+0x7000+args[0]%8*4;self.events.append((kind,args,result))
        else:
            token=struct.unpack('<I',bytes(u.mem_read(sp+4,4)))[0]
            self.events.append((kind,token));result=0
            if not self.rebuilt:pop=4
        ret=struct.unpack('<I',bytes(u.mem_read(sp,4)))[0]
        u.reg_write(xc.UC_X86_REG_ESP,sp+4+pop);u.reg_write(xc.UC_X86_REG_EAX,result);u.reg_write(xc.UC_X86_REG_EIP,ret)

    def run(self,kind,case):
        self.ticks=0;self.nodes=[]
        case=dict(case);data=bytearray(case['data'])
        struct.pack_into('<I',data,platform.BUFFER_TABLE-ARENA+8,BOUNDARY['release'])
        case['data']=bytes(data)
        if kind=='elapsed':
            u=self.u;u.mem_write(ARENA,case['data'])
            for a,body in case['globals'].items():u.mem_write(a,body)
            sp=platform.STACK+0x8000;args=case['calls'][kind][1]
            u.mem_write(sp,struct.pack('<3I',platform.STOP,*args));u.reg_write(xc.UC_X86_REG_ESP,sp)
            saved=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
            for i,reg in enumerate(saved):u.reg_write(reg,0xabcdef00+i)
            u.reg_write(xc.UC_X86_REG_FPCW,case['cw']);u.reg_write(xc.UC_X86_REG_FPSW,0);u.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
            u.emu_start(self.entries[kind],platform.STOP,count=1000)
            assert u.reg_read(xc.UC_X86_REG_EIP)==platform.STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4
            assert [u.reg_read(reg)for reg in saved]==[0xabcdef00+i for i in range(4)]
            assert u.reg_read(xc.UC_X86_REG_FPCW)==case['cw']
            top=(u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7;assert top==7
            return u.reg_read(xc.UC_X86_REG_FP0+top)
        result=super().run(kind,case)
        result=list(result)
        if kind in VOID:result[3]=None
        result[2].extend(bytes(self.u.mem_read(a,16))for a in self.nodes)
        return tuple(result)


def base_case(index,rng):
    case,keys=pack.enrich(platform.base_case(rng,index),index,rng)
    data=bytearray(rng.randbytes(0x30000));data[:len(case['data'])]=case['data'];case['data']=data
    g=case['globals']
    for a in [0x830d1c,0x82dbc0,0x832da4,0x872b44,0x870d28]:g[a]=struct.pack('<2I',a,a)
    for a in [0x82d380,0x82daa0,0x82daa4]:g[a]=struct.pack('<I',rng.choice((0,0xffffffff)))
    for a in [0x82dac0,0x82da8c,0x82da94,0x82da90,0x82da98,0x82da9c]:g[a]=struct.pack('<f',rng.uniform(-100,100))
    g[0x8284e4]=struct.pack('<I',rng.choice((0,1,8,15,31,32,255)))
    g[0x828028]=struct.pack('<f',rng.choice((0.0,1e-6,0.1,-2.5,1.0)))
    g[0x864f60]=struct.pack('<I',0);g[0x830da4]=bytes([1])*8192
    for solid in [pack.HELPER,pack.OTHER]:
        struct.pack_into('<2I',data,solid-ARENA+0x88,solid+0x88,solid+0x88)
        struct.pack_into('<H',data,solid-ARENA+0xe,0)
    for i in range(3):
        p=ARENA+0x9000+i*0x100;resources=ARENA+0xa000+i*0x100
        struct.pack_into('<4I',data,p-ARENA,ARENA+0x9000+(i+1)*0x100 if i<2 else 0x830d1c,
                         ARENA+0x9000+(i-1)*0x100 if i else 0x830d1c,0,3)
        struct.pack_into('<I',data,p-ARENA+0x74,resources)
        for j in range(3):struct.pack_into('<2I',data,resources-ARENA+j*8,j+1,pack.OTHER if (index+i+j)%3 else 0)
        for offset in [0x80,0x88]:struct.pack_into('<2I',data,p-ARENA+offset,p+offset,p+offset)
    for offset,base,count in [(0x80,ARENA+0xb300,index%4),(0x88,ARENA+0xb400,index//4%4)]:
        head=ARENA+0x9000+offset
        struct.pack_into('<2I',data,head-ARENA,base if count else head,base+(count-1)*16 if count else head)
        for j in range(count):struct.pack_into('<3I',data,base-ARENA+j*16,
            base+(j+1)*16 if j+1<count else head,base+(j-1)*16 if j else head,rng.getrandbits(32))
    g[0x830d1c]=struct.pack('<2I',ARENA+0x9000,ARENA+0x9200)
    query=index%5
    case['calls']=dict(find_loaded=(0,[query,0 if index%2 else ARENA+0x9100]),find_owner=(0,[query]),
        resolve=(0,[ARENA+0x9100]),detach=(0,[pack.HELPER]),clear_header=(0,[ARENA+0x9000]),
        elapsed=(0,[rng.getrandbits(32),rng.getrandbits(32)]),defer=(0x870d28,[rng.getrandbits(32)]))
    dummy=ARENA+0x1200
    struct.pack_into('<I',data,platform.TABLE-ARENA,dummy)
    struct.pack_into('<2I',data,dummy-ARENA,0x872ed0,0x872ed0)
    g[0x872ed0]=struct.pack('<2I',dummy,dummy)
    struct.pack_into('<I',data,dummy-ARENA+0x20,platform.OBJECTS if index%2 else 0)
    struct.pack_into('<I',data,dummy-ARENA+0x2c,123 if index%3 else 0)
    struct.pack_into('<I',data,platform.OBJECTS-ARENA,platform.BUFFER_TABLE)
    count=index%5;base=ARENA+0xb500;head=0x872b44
    g[head]=struct.pack('<2I',base if count else head,base+(count-1)*16 if count else head)
    for j in range(count):struct.pack_into('<3I',data,base-ARENA+j*16,
        base+(j+1)*16 if j+1<count else head,base+(j-1)*16 if j else head,dummy if j%2 else 0)
    case['calls']['unload_platform']=platform.TABLE,[0xdead0000]
    # Both unresolved and attached lists exercise real resource lookups.
    for head,count,base,owner in [(0x82dbc0,index%5,ARENA+0xb000,0),(pack.HELPER+0x88,index%4,ARENA+0xb100,pack.HELPER)]:
        first,last=(base,base+(count-1)*16)if count else(head,head)
        if head<ARENA:g[head]=struct.pack('<2I',first,last)
        else:struct.pack_into('<2I',data,head-ARENA,first,last)
        for j in range(count):struct.pack_into('<4I',data,base-ARENA+j*16,
            base+(j+1)*16 if j+1<count else head,base+(j-1)*16 if j else head,j%3+1,owner)
    # Offline variants exercise encoded pointer detachment and offline relinking
    # separately, without inventing an online traversal of encoded pointers.
    if index%7==0:
        g[0x864f60]=struct.pack('<I',4)
        head=0x832da4;node=ARENA+0xb200
        g[head]=struct.pack('<2I',node,node)
        struct.pack_into('<4I',data,node-ARENA,head,head,1,pack.OTHER)
    case.update(cw=0x27f if index%2 else 0x37f,objects_base=ARENA+0xc00000,buffer_base=ARENA+0x1000000,
                ticker=rng.getrandbits(32),tick_step=rng.choice((0,1,23,0x80000000,0xffffffff)))
    return case


def cases(n):
    rng=random.Random(0x494060)
    for index,outer in enumerate(pack.cases(n)):
        case=base_case(index,rng);data=case['data'];g=case['globals']
        # A valid root uses a real index in its own header, not the standalone
        # lookup fixture above. Root-only cases keep unrelated references empty.
        begin=outer['calls']['outer'][1][0]-ARENA
        length=struct.unpack_from('<I',outer['data'],begin+4)[0]+8
        solid_bytes=bytearray(outer['data'][begin:begin+length])
        def prepare_platform(b,e):
            found=False
            while b+8<=e:
                kind,size=struct.unpack_from('<2I',solid_bytes,b);limit=b+8+size
                if kind==0x134900:
                    p=((begin+b+23)&~15)-begin
                    struct.pack_into('<I',solid_bytes,p+0x20,0)
                    struct.pack_into('<I',solid_bytes,p+0x2c,0)
                    found=True
                elif kind==0x80134100:
                    if not prepare_platform(b+8,limit):struct.pack_into('<I',solid_bytes,b,0x12345678)
                b=limit
            return found
        prepare_platform(8,len(solid_bytes))
        start=0x10000;cursor=start+8
        header=bytearray(rng.randbytes(0x90));struct.pack_into('<i',header,0xc,1)
        header_start=cursor;cursor+=8
        cursor,_=platform.append_chunk(data,cursor,0x134002,header)
        # Header hash is read from the first actual solid child.
        child=begin+8;key=None
        while child<begin+length:
            kind,size=struct.unpack_from('<2I',outer['data'],child)
            if kind==0x134011:key=struct.unpack_from('<I',outer['data'],((child+23)&~15)+0x10)[0];break
            child+=size+8
        assert key is not None
        cursor,_=platform.append_chunk(data,cursor,0x134003,struct.pack('<2I',key,0))
        cursor,_=platform.append_chunk(data,cursor,0x134004,rng.randbytes(24))
        struct.pack_into('<2I',data,header_start,0x80134001,cursor-header_start-8)
        case['calls']['load_header']=0,[ARENA+header_start]
        case['calls']['header_cycle']=0,[ARENA+header_start]
        case['calls']['walk_platform']=0,[ARENA+header_start]
        padding=(begin-cursor-8)%128;cursor,_=platform.append_chunk(data,cursor,0x12345678,rng.randbytes(padding))
        assert cursor%128==begin%128
        data[cursor:cursor+length]=solid_bytes;cursor+=length
        # The outer fixture occasionally uses an unrelated ID for early return;
        # restore the valid solid ID when embedding it into a root lifecycle.
        struct.pack_into('<I',data,cursor-length,0x80134010)
        if index%4==1:
            # A second real header/solid pair exercises finalization between
            # contexts, duplicate hashes, and lookups after the first unregister.
            second=cursor;cursor+=8
            cursor,_=platform.append_chunk(data,cursor,0x134002,header)
            cursor,_=platform.append_chunk(data,cursor,0x134003,struct.pack('<2I',key,0))
            cursor,_=platform.append_chunk(data,cursor,0x134004,rng.randbytes(24))
            struct.pack_into('<2I',data,second,0x80134001,cursor-second-8)
            padding=(begin-cursor-8)%128;cursor,_=platform.append_chunk(data,cursor,0x12345678,rng.randbytes(padding))
            data[cursor:cursor+length]=solid_bytes
            struct.pack_into('<I',data,cursor,0x80134010);cursor+=length
        struct.pack_into('<2I',data,start,0x80134000 if index%31 else 0x12345678,cursor-start-8)
        case['calls']['load_root']=(0,[ARENA+start]);case['calls']['cycle']=(0,[ARENA+start])
        case['data']=bytes(data);case['root_globals']=dict(g)
        for a in [0x830d1c,0x82dbc0,0x832da4]:case['root_globals'][a]=struct.pack('<2I',a,a)
        case['root_globals'][0x830da4]=bytes(8192);case['root_globals'][0x864f60]=struct.pack('<I',0)
        yield case


def retail_cases(path):
    raw=path.read_bytes();rng=random.Random(612);case=base_case(1,rng)
    data=bytearray(rng.randbytes(0x10000+len(raw)+128));data[:0x10000]=case['data'][:0x10000]
    data[0x10000:0x10000+len(raw)]=raw;case['data']=bytes(data)
    case['calls']={'load_root':(0,[ARENA+0x10000]),'cycle':(0,[ARENA+0x10000])}
    for a in [0x830d1c,0x82dbc0,0x832da4]:case['globals'][a]=struct.pack('<2I',a,a)
    case['globals'][0x830da4]=bytes(8192);case['globals'][0x864f60]=struct.pack('<I',0)
    case['instruction_budget']=30000000
    yield case


def verify(image,manifest,linked,gcc,source,kinds):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);totals,events=Counter(),Counter()
    for index,case in enumerate(source):
        for kind in kinds:
            g=case['globals']
            if kind in ['load_root','cycle'] and 'root_globals'in case:case['globals']=case['root_globals']
            try:x,y=a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError)as e:raise AssertionError(('solid manager mismatch',kind,index,str(e)))from e
            finally:case['globals']=g
            if x!=y:
                detail=[]
                if kind!='elapsed':
                    detail=[(hex(i),u,v)for i,(u,v)in enumerate(zip(x[0],y[0]))if u!=v][:8]
                    detail += [(i,u,v)for i,(u,v)in enumerate(zip(x[4],y[4]))if u!=v][:1]
                raise AssertionError(('solid manager mismatch',kind,index,detail,x[3]if kind!='elapsed'else x,
                                      y[3]if kind!='elapsed'else y))
            totals[kind]+=1
            if kind!='elapsed':events.update(e[0]for e in x[4])
    return dict(comparisons=dict(totals),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--retail',type=Path);p.add_argument('--replace',type=Path);p.add_argument('--kind',default='all')
    p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc)
    kinds=['find_loaded','find_owner','resolve','detach','clear_header','elapsed','defer','unload_platform',
           'load_header','header_cycle','walk_platform','load_root','cycle']if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases),kinds)
    if args.retail:report['retail']=dict(sha256=hashlib.sha256(args.retail.read_bytes()).hexdigest(),
        **verify(image,manifest,linked,args.gcc,retail_cases(args.retail),['load_root','cycle']))
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['controlled GPU/texture/node-pool/clock and return-this boundaries',
            'finite valid list and chunk orders; deferred vertex release is queued, not drained',
            'native activation and render consumption not established by this verifier'])
    if args.mutation_checks:
        mutations={
            'ignore-bucket-miss':('load_root','if (!ug2SolidBuckets830DA4[hash & 0x1fff])','if (false)'),
            'skip-pack-promotion':('find_loaded','Unlink(&pack->link00);\n                CarRenderQueueLink *first',
                                  '(void)0;\n                CarRenderQueueLink *first'),
            'wrong-owner-key':('find_owner','CarRenderFindResource(hash,pack->resources74','CarRenderFindResource(hash+1,pack->resources74'),
            'skip-reference-lookup':('resolve','if (reference->hash08) {','if (false) {'),
            'wrong-reference-target':('resolve','reference->solid0C = solid;','reference->solid0C = 0;'),
            'wrong-offline-encoding':('detach','reinterpret_cast<unsigned char *>(solid)+ug2SolidOffline864F60',
                                     'reinterpret_cast<unsigned char *>(solid)+0'),
            'wrong-deferred-token':('defer','node->buffer08 = buffer;','node->buffer08 = buffer+1;'),
            'skip-index-release':('unload_platform','UG2SolidReleaseIndexBuffer(platform->indexBuffer20);','(void)0;'),
            'skip-platform-clear':('unload_platform','platform00 = 0;','(void)0;'),
            'wrong-header-flag':('load_header','pack->value72 = 1;','pack->value72 = 2;'),
            'wrong-stream-pointer':('load_header','pack->stream78 = child+2;','pack->stream78 = child+3;'),
            'skip-finalization':('load_root','if (pack) CarRenderResolveSolidReferences(pack);','(void)0;'),
            'skip-node-free':('clear_header','bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));','(void)0;'),
            'wrong-clear-count':('clear_header','pack->value7C = 0; pack->value7E = 0;','pack->value7C = 1; pack->value7E = 0;'),
            'wrong-unload-total':('cycle','--ug2SolidLoads82D384;','++ug2SolidLoads82D384;'),
            'wrong-clock-sign':('elapsed','static_cast<int>(end-start)','static_cast<int>(start-end)'),
            'unsigned-clock-multiplier':('elapsed','int multiplier = static_cast<int>','unsigned int multiplier = static_cast<unsigned int>'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            command=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            result=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'solid manager mismatch'in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
