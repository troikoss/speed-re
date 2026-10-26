#!/usr/bin/env python3
"""Original/rebuilt texture reference refresh, dependency lists and cache clears.

Executes real texture lookup/binary search and pack-list cleanup. Node allocation,
GPU/formatting, return-this and missing-default trap are controlled boundaries.
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
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
from pc_coff import link_objects,read_object
from pc_native_elf import link_native
import verify_pc_car_render_texture as core
import verify_pc_car_render_solid_manager as manager

ROOT,EXE=core.ROOT,core.EXE
SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderTextureReferences.cpp'
BUILD=ROOT/'build/analysis/car-render-texture-references'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-texture-references.json'
ARENA,HEAP=core.ARENA,0x3200000
GLOBALS=dict(ug2TextureReferences830D08=0x830d08,ug2FontTextures863768=0x863768,ug2TextureCache89F050=0x89f050)
HOOKS={k:core.STOP+0x600+i*16 for i,k in enumerate(('new','free','ticker','release'))}
FUNCTIONS=dict(build='CarRenderBuildPackTextureReferences',offline='CarRenderDetachOfflineTextures',cache='CarRenderFindTextureCache')
PREFIX=dict(core.PREFIX,**{k:v+'@@'for k,v in FUNCTIONS.items()},prepend='Prepend@CarRenderPointerList',
    load='RefreshLoadedTextures@CarRenderSlotTable',unload='RefreshUnloadedTextures@CarRenderSlotTable',
    notify='_UG2TextureNotifySolids',references='_UG2TextureNotify5C0DC0',fonts='_UG2TextureNotify51BF70',invalidate='_UG2TextureNotify611A10')
GCC=dict(core.GCC,**{k:'_Z'+str(len(v))+v for k,v in FUNCTIONS.items()},prepend='_ZN20CarRenderPointerList7Prepend',
    load='_ZN18CarRenderSlotTable21RefreshLoadedTextures',unload='_ZN18CarRenderSlotTable23RefreshUnloadedTextures',
    notify='UG2TextureNotifySolids',references='UG2TextureNotify5C0DC0',fonts='UG2TextureNotify51BF70',invalidate='UG2TextureNotify611A10')
core.METHODS|={'prepend','load','unload'}
core.RETURNS|={'load','unload','cache'}
BOUNDARIES={k:core.BOUNDARIES[k]for k in ('format','end','trap','upload','release')}
ORIGINAL_BOUNDARIES={k:core.ORIGINAL_BOUNDARIES[k]for k in BOUNDARIES}


def build(source,gcc):
    bindings=dict(core.platform.GLOBALS,**core.platform.BINDINGS,**core.pack.GLOBAL,**core.GLOBALS,**manager.GLOBALS,**GLOBALS,
        UG2TextureMissingDefault=BOUNDARIES['trap'],UG2TextureUpload=BOUNDARIES['upload'],UG2TextureRelease=BOUNDARIES['release'],
        UG2SolidReadTicker=HOOKS['ticker'],UG2SolidReleaseIndexBuffer=HOOKS['release'])
    sources=[source,core.SOURCE,manager.SOURCE,core.pack.SOURCE,core.platform.SOURCE,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if gcc:
        bindings.update({'_Z8bSFormatPcPKcz':BOUNDARIES['format'],'_ZN18CarRenderLightList3EndEv':BOUNDARIES['end'],
            '_Z12bNewListNodev':HOOKS['new'],'_Z15bDeleteListNodeP14WCollisionLink':HOOKS['free']})
        linked=link_native(sources,BUILD,ROOT,bindings,'UG2TextureNotifySolids',text_address=0x5000000)
    else:
        BUILD.mkdir(parents=True,exist_ok=True);objects=[];external={'_'+k:v for k,v in bindings.items()}
        for i,path in enumerate(sources):
            obj=BUILD/('reference-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section']!=0:continue
                name=symbol['name']
                for prefix,address in [('?bNewListNode@@',HOOKS['new']),('?bDeleteListNode@@',HOOKS['free']),
                    ('?bSFormat@@',BOUNDARIES['format']),('?End@CarRenderLightList@@',BOUNDARIES['end'])]:
                    if name.startswith(prefix):external[name]=address
                for label,kind in [('UG2TextureUpload','upload'),('UG2TextureRelease','release')]:
                    if name=='@'+label+'@4':external[name]=BOUNDARIES[kind]
        linked=link_objects(objects,external,start=0x5000000)
    linked['entries']={}
    for kind,needle in (GCC if gcc else PREFIX).items():
        hits=[(n,v)for n,v in linked['exports'].items()if needle in n]
        if kind in ('inventory','inventory_release'):
            hits=[(n,v)for n,v in hits if n==(''if gcc else'_')+GCC[kind]]
        assert len(hits)==1,(kind,hits)
        linked['entries'][kind]=hits[0][1]
    return linked


class Machine(core.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        u.mem_map(ARENA,core.SIZE);u.mem_map(core.STACK,0x20000);u.mem_map(HEAP,0x100000)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,body in linked['blocks']:u.mem_write(a,body)
        both=json.loads(core.MANIFEST.read_text());both['functions'].update(manifest['functions'])
        self.entries=linked['entries']if linked else{k:int(f['address'],16)for k,f in both['functions'].items()}
        self.gcc,self.rebuilt=gcc,bool(linked)
        for kind,a in (BOUNDARIES if linked else ORIGINAL_BOUNDARIES).items():
            u.hook_add(UC_HOOK_CODE,self.boundary,user_data=kind,begin=a,end=a)
        for kind,a in (HOOKS if linked else dict(new=0x440ec0,free=0x440ef0,ticker=0x43bdf0,release=HOOKS['release'])).items():
            u.hook_add(UC_HOOK_CODE,self.node,user_data=kind,begin=a,end=a)
    def node(self,u,address,size,kind):
        sp=u.reg_read(xc.UC_X86_REG_ESP);result=0
        if kind=='new':
            result=HEAP+len(self.nodes)*16;self.nodes.append(result);u.mem_write(result,b'\x93'*16)
            self.events.append((kind,result))
        elif kind=='free':
            pointer=self.word(sp+4);self.events.append((kind,pointer,bytes(u.mem_read(pointer,12))))
            u.mem_write(pointer,struct.pack('<I',0xdeadbeef))
        else:raise AssertionError(('unexpected boundary',kind))
        ret=self.word(sp);u.reg_write(xc.UC_X86_REG_ESP,sp+4);u.reg_write(xc.UC_X86_REG_EAX,result);u.reg_write(xc.UC_X86_REG_EIP,ret)
    def run(self,kind,case):
        self.nodes=[]
        result=list(super().run(kind,case))
        if kind in ('load','unload'):result[2]&=255
        result.append([bytes(self.u.mem_read(a,16))for a in self.nodes])
        return tuple(result)


def cases(n):
    rng=random.Random(0x4939b0)
    for i in range(n):
        case=core.base_case(i,rng);data=bytearray(case['data'])+bytearray(rng.randbytes(0x40000));g=case['globals']
        def put(a,fmt,*v):struct.pack_into('<'+fmt,data,a-ARENA,*v)
        keys=[1,1,2,0x2001,0x66994f4a,0xffffffff]
        for p in range(3):
            desc,head,index=core.PACK+p*0x100,core.HEADER+p*0x100,core.INDEX+p*0x100
            put(desc+0x70,'Ii',head,6);put(head+0x74,'I',index)
            for j,key in enumerate(keys):
                texture=core.TEXTURES+(p*16+j)*0x7c
                put(index+j*8,'2I',key,texture if (i+p+j)%5 else 0)
                put(texture,'I',core.STATES+(p*16+j)*0x20);put(texture+0x24,'I',key)
                put(texture+0x3c,'I',4 if (i+p+j)%3==0 else 0)
                put(texture+0x68,'4I',desc,0,core.PIXELS if (i+p+j)%4 else 0,core.PIXELS if (i+p+j)%3 else 0)
        default=ARENA+0x18000
        put(default,'I',core.STATES);put(default+0x24,'I',0x66994f4a);put(default+0x68,'4I',core.PACK+0x200,0,core.PIXELS,0)
        g[0x82db08]=struct.pack('<I',default)
        g[0x864f60]=struct.pack('<i',(0,0,0,1,-1,4)[i%6])
        g[0x89f050]=rng.randbytes(6*0x1c8)
        packs=[ARENA+0x20000,ARENA+0x20100]
        g[0x830d1c]=struct.pack('<2I',packs[0],packs[-1])
        for p,pack_ in enumerate(packs):
            resources=ARENA+0x20800+p*0x100
            put(pack_,'2I',packs[p+1]if p+1<len(packs)else 0x830d1c,packs[p-1]if p else 0x830d1c)
            put(pack_+0xc,'i',(0,1,3,-1,3)[(i+p)%5]);put(pack_+0x74,'I',resources)
            put(pack_+0x7c,'2H',0xffff if i%3 else 2,1)
            for j in range(3):
                solid=ARENA+0x21000+(p*4+j)*0x100;slots=ARENA+0x23000+(p*4+j)*0x100
                put(resources+j*8,'2I',j,solid if (i+j)%7 else 0)
                put(solid+0x19,'b',(-128,-1,0,1,2,4)[(i+p+j)%6]);put(solid+0x2c,'I',slots)
                for k in range(4):
                    value=default if (i+j+k)%3 else core.TEXTURES+((p*16+k)%48)*0x7c
                    put(slots+k*8,'2I',keys[(j+k)%6],value)
            for off,values in [(0x80,[core.PACK,core.PACK+0x100]),(0x88,[ARENA+0x23000+p*0x400])]:
                head=pack_+off;nodes=[ARENA+0x27000+p*0x400+off*4+j*16 for j in range(len(values))]
                put(head,'2I',nodes[0],nodes[-1])
                for j,node in enumerate(nodes):put(node,'3I',nodes[j+1]if j+1<len(nodes)else head,nodes[j-1]if j else head,values[j])
        for head,base,font in [(0x830d08,ARENA+0x29000,False),(0x863768,ARENA+0x29400,True)]:
            count=i%5;g[head]=struct.pack('<2I',base if count else head,base+(count-1)*32 if count else head)
            for j in range(count):
                node=base+j*32;put(node,'2I',node+32 if j+1<count else head,node-32 if j else head)
                put(node+(8 if font else 0x14),'I',0 if (i+j)%3 else core.TEXTURES)
                put(node+(0x18 if font else 0xc),'I',keys[j%6])
        # A standalone pending pointer always names a real slot in a solid table.
        hash_=0x530b82af+i%9
        case.update(data=data,globals=g,calls=dict(prepend=(packs[0]+0x80,[core.PACK]),
            load=(ARENA+0x21000,[core.PACK]),unload=(ARENA+0x21000,[core.PACK]),build=(0,[packs[0]]),
            offline=(0,[core.PACK]),notify=(0,[core.PACK,(0,1,0x100,0x101,0xffffffff)[i%5]]),
            references=(0,[core.PACK,(0,1,0x100,0x101)[i%4]]),fonts=(0,[core.PACK,(0,1,0x100,0x101)[i%4]]),
            cache=(0,[hash_]),invalidate=(0,[hash_])))
        yield case


def verify(image,manifest,linked,gcc,fixtures,kinds):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);totals,events=Counter(),Counter()
    for i,case in enumerate(fixtures):
        for kind in kinds:
            try:x,y=a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError)as e:raise AssertionError(('texture reference mismatch',kind,i,str(e)))from e
            if x!=y:
                changes=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                raise AssertionError(('texture reference mismatch',kind,i,changes,x[2],y[2],x[3][:3],y[3][:3]))
            totals[kind]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(totals),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--replace',type=Path);p.add_argument('--kind',default='all')
    p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc)
    kinds=list(manifest['functions'])if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases),kinds)
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['controlled successful node allocation/free and return-this boundaries',
        'real texture lookup, duplicate search and pack cleanup execute in the chain',
        'finite record/list layouts; malformed/OOM behavior and rendering equivalence not established',
        'standalone/font record construction remains outside these prefix refresh checks']
    if args.mutation_checks:
        mutations={
            'wrong-node-value':('prepend','node->value08 = value;','node->value08 = 0;'),
            'wrong-used-byte':('load','found->used53 = 1;','found->used53 = 2;'),
            'skip-slot-update':('load','slot->texture04 = texture;','(void)0;'),
            'wrong-fallback-key':('unload','UG2SolidFindTexture(hash,0,0)','UG2SolidFindTexture(hash+1,0,0)'),
            'replace-ready-local':('unload','&& !pack->FindLoaded(hash)','&& pack->FindLoaded(hash)'),
            'dedup-requested-hash':('build','slot08->hash00 == texture->hash24','slot08->hash00 == slot->hash00'),
            'wrong-pending-count':('build','++pack->value7E;','++pack->value7C;'),
            'skip-list-clear':('build','CarRenderClearPackLists(pack);','(void)0;'),
            'skip-offline-displacement':('offline','+ug2SolidOffline864F60','+0'),
            'use-full-mode-word':('notify','static_cast<unsigned char>(mode)','mode'),
            'skip-pending-free':('notify','bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));','(void)0;'),
            'lose-next-snapshot':('notify','node = next;','node = node->next00;'),
            'skip-loaded-refresh':('notify','solidPack->resources74[i].solid04->RefreshLoadedTextures(pack);','(void)0;'),
            'skip-detach-rebuild':('notify','if (changed) CarRenderBuildPackTextureReferences(solidPack);','(void)0;'),
            'skip-standalone-refresh':('references','reference->texture14 = CarRenderRefreshTexture(reference->texture14,reference->hash0C,pack,static_cast<char>(mode));','(void)0;'),
            'wrong-font-key':('fonts','reference->hash18,pack','reference->value0C[0],pack'),
            'wrong-cache-slot':('cache','case 0x530b82b0: return &ug2TextureCache89F050[0];','case 0x530b82b0: return &ug2TextureCache89F050[1];'),
            'short-cache-clear':('invalidate','i<0x72','i<0x71'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            result=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'texture reference mismatch'in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
