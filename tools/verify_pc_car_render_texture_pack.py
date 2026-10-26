#!/usr/bin/env python3
"""Original/rebuilt texture root/header callbacks, pixel queues and animations.

Executes the recovered texture record chain. Slot-pool allocation, formatting,
GPU and game reference notifications are explicit controlled boundaries.
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
from unicorn import UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
import verify_pc_car_render_texture as core

ROOT, EXE = core.ROOT, core.EXE
SOURCE = ROOT/'src/Speed/Indep/Src/World/CarRenderTexturePack.cpp'
BUILD = ROOT/'build/analysis/car-render-texture-pack'
MANIFEST = ROOT/'config/SPEED2_EXE_1_2/car-render-texture-pack.json'
HEAP = 0x3200000
ALLOC = {k:core.STOP+0x500+i*16 for i,k in enumerate(('new_pool','allocate','snapshot','free'))}
ORIGINAL_ALLOC = dict(new_pool=0x440df0,allocate=0x43f8f0,snapshot=0x440e50,free=0x43f920)
LIBRARY = dict(bNewSlotPool='new_pool',bSlotPoolAllocate='allocate',bOMalloc='snapshot',bFree='free')
GLOBALS = dict(ug2TexturePackPool82DB0C=0x82db0c,ug2TextureSnapshotPool870790=0x870790,
    ug2TextureCurrentPack82DB10=0x82db10,ug2TexturePixelQueue8358C8=0x8358c8,
    ug2TextureAnimations82DC00=0x82dc00,ug2TexturePoolName78D950=0x78d950)
FUNCTIONS = dict(init='CarRenderInitTexturePackPool',snapshot='CarRenderTextureSnapshot',
    free_snapshot='CarRenderFreeTextureSnapshot',load_header='CarRenderLoadTextureHeader',
    unload_header='CarRenderUnloadTextureHeader',queue='CarRenderQueueTexturePixels',
    find_pixels='CarRenderFindTexturePixels',load_root='CarRenderLoadTexturePack',unload_root='CarRenderUnloadTexturePack')
PREFIX = dict(core.PREFIX,**{k:v+'@@' for k,v in FUNCTIONS.items()})
GCC = dict(core.GCC,**{k:'_Z'+str(len(v))+v for k,v in FUNCTIONS.items()})
for k,m in [('anim_construct','Construct'),('anim_update','Update'),('anim_destroy','Destroy')]:
    PREFIX[k] = m+'@CarRenderTextureAnimation'
    GCC[k] = '_ZN25CarRenderTextureAnimation'+str(len(m))+m
core.METHODS |= {'anim_construct','anim_update','anim_destroy','free_snapshot'}
core.RETURNS |= {'snapshot','load_header','find_pixels','anim_construct','load_root','unload_root'}


def build(source,gcc):
    bindings = dict(core.platform.GLOBALS,**core.platform.BINDINGS,**core.pack.GLOBAL,**core.GLOBALS,**GLOBALS,
        UG2SolidFindLoaded=core.STOP+0x400,UG2SolidFindOwner=core.STOP+0x410)
    sources = [source,core.SOURCE,core.pack.SOURCE,core.platform.SOURCE,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if gcc:
        bindings.update({k:core.BOUNDARIES[v] for k,v in core.C_BOUNDARIES.items()})
        bindings.update({'_Z8bSFormatPcPKcz':core.BOUNDARIES['format'],'_ZN18CarRenderLightList3EndEv':core.BOUNDARIES['end'],
            '_Z12bNewSlotPooliiPKci':ALLOC['new_pool'],'_Z17bSlotPoolAllocatePv':ALLOC['allocate'],
            '_Z8bOMallocPv':ALLOC['snapshot'],'_Z5bFreePvS_':ALLOC['free']})
        linked = link_native(sources,BUILD,ROOT,bindings,GCC['load_root']+'Pj',text_address=0x4000000)
    else:
        BUILD.mkdir(parents=True,exist_ok=True); objects = []
        external = {'_'+k:v for k,v in bindings.items()}
        for i,path in enumerate(sources):
            obj = BUILD/('pack-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section'] != 0: continue
                name = symbol['name']
                for label,kind in LIBRARY.items():
                    if name.startswith('?'+label+'@@'): external[name] = ALLOC[kind]
                if name.startswith('?bSFormat@@'): external[name] = core.BOUNDARIES['format']
                if name.startswith('?End@CarRenderLightList@@'): external[name] = core.BOUNDARIES['end']
                for label,kind in core.C_BOUNDARIES.items():
                    if name in ('_'+label,'@'+label+'@4'): external[name] = core.BOUNDARIES[kind]
        linked = link_objects(objects,external,start=0x4000000)
    linked['entries'] = {}
    for kind,needle in (GCC if gcc else PREFIX).items():
        hits = [(n,v) for n,v in linked['exports'].items() if needle in n]
        if kind in ('inventory','inventory_release'):
            hits = [(n,v) for n,v in hits if n == ('_' if not gcc else '')+GCC[kind]]
        assert len(hits) == 1,(kind,hits)
        linked['entries'][kind] = hits[0][1]
    return linked


class Machine(core.Machine):
    def __init__(self,image,manifest,linked=None,gcc=False):
        both = json.loads(core.MANIFEST.read_text()); both['functions'].update(manifest['functions'])
        super().__init__(image,both,linked,gcc)
        self.u.mem_map(0,4096) # i386 FS exception-list slot for original/MSVC constructor guards
        self.u.mem_map(HEAP,0x200000)
        for kind,address in (ALLOC if linked else ORIGINAL_ALLOC).items():
            self.u.hook_add(UC_HOOK_CODE,self.allocate,user_data=kind,begin=address,end=address)

    def allocate(self,u,address,size,kind):
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        result = 0
        args = tuple(self.word(sp+4+i*4) for i in range(4 if kind=='new_pool' else 2 if kind=='free' else 1))
        if kind == 'new_pool':
            assert args[:2] == (0x7c,0x80) and args[3] == 0
            assert self.string(args[2]) == b'TexturePackSlotPool'
            result = core.ARENA+0xc00
        elif kind in ('allocate','snapshot'):
            result = HEAP+len(self.allocated)*0x100
            length = 0x7c if kind == 'allocate' else 4
            u.mem_write(result,b'\x93'*length); self.allocated.append((result,length))
        else:
            self.events.append((kind,args,bytes(u.mem_read(args[1],4))))
            u.mem_write(args[1],struct.pack('<I',0xdeadbeef))
        if kind != 'free': self.events.append((kind,args,result))
        ret = self.word(sp); u.reg_write(xc.UC_X86_REG_ESP,sp+4)
        u.reg_write(xc.UC_X86_REG_EAX,result); u.reg_write(xc.UC_X86_REG_EIP,ret)

    def run(self,kind,case):
        self.allocated = []; self.u.mem_write(0,struct.pack('<I',0xffffffff))
        if kind in ('root_cycle','header_cycle','animation_cycle'):
            case = dict(case,calls=dict(case['calls'],cycle=(0,[])),steps=case['drivers'][kind])
            kind = 'cycle'
        gcc = self.gcc
        if kind == 'free_snapshot': self.gcc = False # explicit stdcall on both compilers
        try: result = super().run(kind,case)
        finally: self.gcc = gcc
        assert self.word(0) == 0xffffffff
        return result+(tuple(bytes(self.u.mem_read(a,n)) for a,n in self.allocated),)


def cases(n):
    rng = random.Random(0x494ea0)
    for i in range(n):
        case = core.base_case(i,rng); data = bytearray(case['data'])+bytearray(0x20000)
        g = case['globals']
        for a in (0x8358c8,0x82dc00): g[a] = struct.pack('<2I',a,a)
        g[0x82db0c] = struct.pack('<I',core.ARENA+0xc00); g[0x870790] = struct.pack('<I',core.ARENA+0xc80)
        g[0x82db10] = struct.pack('<I',core.PACK)
        def put(a,fmt,*v): struct.pack_into('<'+fmt,data,a-core.ARENA,*v)
        def chunk(offset,kind,body):
            struct.pack_into('<2I',data,offset,kind,len(body)); data[offset+8:offset+8+len(body)] = body
            return offset+8+len(body)
        # A real serialized header layout, plus unknown child and trailing bytes
        # in record/state arrays to check the original count divisions.
        header = bytearray(rng.randbytes(0x7c)); header[4:9] = b'pack\0'; header[0x20:0x25] = b'path\0'
        struct.pack_into('<I',header,0x60,123)
        count = i%5
        start = 0x20000; cursor = start+8
        cursor = chunk(cursor,0x33310001,header); h = core.ARENA+start+16
        indices = b''.join(struct.pack('<2I',j+1,0) for j in range(count))
        cursor = chunk(cursor,0x33310002,indices)
        textures = bytearray(rng.randbytes(count*0x7c))
        for j in range(count):
            struct.pack_into('<I',textures,j*0x7c+0x24,j+1)
            struct.pack_into('<I',textures,j*0x7c+0x2c,0)
            struct.pack_into('<iiII',textures,j*0x7c+0x30,j*32,-1,32,0)
        cursor = chunk(cursor,0x33310004,textures+bytes(i%7))
        cursor = chunk(cursor,0x33310005,rng.randbytes(count*0x20)+bytes(i%11))
        cursor = chunk(cursor,0x33310003,rng.randbytes(count*24))
        cursor = chunk(cursor,0x12345678,rng.randbytes(i%4))
        struct.pack_into('<2I',data,start,0xb3310000,cursor-start-8)
        put(h+0x70,'I',core.PACK)
        # Pixel chunk: first child payload is 24 bytes, then padding and data.
        pixel = 0x24000
        put(core.ARENA+pixel,'2I',0xb3320000,0x180)
        put(core.ARENA+pixel+8,'2I',0x33320001,24)
        put(core.ARENA+pixel+0x1c,'I',123); put(core.ARENA+pixel+0x20,'I',i%2)
        put(core.ARENA+pixel+0x28,'2I',0,0x50)
        put(core.ARENA+pixel+0x80,'2I',0x33320002,0x100)
        queued = core.ARENA+0xd000
        put(queued,'4I',0x8358c8,0x8358c8,0,456)
        g[0x8358c8] = struct.pack('<2I',queued,queued)
        # Complete pack at an offset that exercises alignment of pixel payload.
        root = 0x28000+(i%32)*4; end = root+8
        data[end:end+cursor-start] = data[start:cursor]; end += cursor-start
        data[end:end+0x188] = data[pixel:pixel+0x188]; end += 0x188
        struct.pack_into('<2I',data,root,0xb3300000,end-root-8)
        bad_header=0x3c000
        data[bad_header:bad_header+cursor-start]=data[start:cursor]
        if i%7==0 and count:
            p=bad_header+8
            while p<bad_header+cursor-start:
                key,size=struct.unpack_from('<2I',data,p)
                if key==0x33310005: struct.pack_into('<I',data,p,0x12345678)
                p+=size+8
        # Standalone unload uses the existing descriptor, not an allocated one.
        put(h+0x70,'I',core.PACK)
        anim,entries,frames = [core.ARENA+x for x in (0x30000,0x31000,0x32000)]
        put(anim,'2I',0x82dc00,0x82dc00); g[0x82dc00] = struct.pack('<2I',anim,anim)
        put(anim+8,'5I',core.PACK,entries,frames,count,i)
        for j in range(count):
            put(entries+j*0x34+0x1c,'i',j%4)
            put(entries+j*0x34+0x28,'I',frames+j*0x100)
            for k in range(j%4): put(frames+j*0x100+k*16,'4I',k+1,core.TEXTURES if (i+k)%2 else 0,core.STATES,0)
        # Animation root container with the exact nested offsets the callback uses.
        ac = core.ARENA+0x35000
        put(ac,'4I',0xb0300100,0x800,0,0x20); put(ac+0x10,'3I',1 if i%3 else 0,anim,0)
        put(ac+0x34,'I',count*0x34)
        for j in range(count): put(ac+0x38+j*0x34+0x1c,'i',j%4)
        put(ac+0x3c+count*0x34,'I',0x100)
        for j in range(16): put(ac+0x40+count*0x34+j*16,'4I',j%3+1,0,0,0)
        calls = dict(init=(0,[]),snapshot=(core.TEXTURES,[]),free_snapshot=(0,[core.STATES]),
            load_header=(0,[core.ARENA+bad_header]),unload_header=(0,[core.ARENA+start]),
            queue=(0,[core.ARENA+pixel]),find_pixels=(0,[123 if i%2 else 456]),
            anim_construct=(anim,[core.PACK,entries,frames,count,i]),anim_update=(anim,[]),anim_destroy=(anim,[]),
            load_root=(0,[[ac,core.ARENA+root,core.ARENA+pixel,core.ARENA+0x1e000][i%4]]))
        put(core.ARENA+0x1e000,'2I',0x12345678,0)
        # Root unload must use a preceding successful root load. The direct
        # entry is covered via this driver rather than uninitialized records.
        case.update(data=data,calls=calls,shorten_on_upload=False,drivers={
            'header_cycle':[('load_header',0,[core.ARENA+start],0),('unload_header',0,[core.ARENA+start],0)],
            'root_cycle':[('load_root',0,[core.ARENA+root],0),('unload_root',0,[core.ARENA+root],0)],
            'animation_cycle':[('load_root',0,[ac],0),('unload_root',0,[ac],0)]})
        yield case


def retail_cases(path):
    from verify_pc_compression import Original
    raw = path.read_bytes(); decoder = None
    def children(start,end):
        while start<end:
            key,size = struct.unpack_from('<2I',raw,start)
            assert start+size+8<=end
            yield key,start,size+8
            start+=size+8
        assert start==end
    for key,start,length in children(0,len(raw)):
        if key!=0xb3300000: continue
        header_chunk = next(p for k,p,n in children(start+8,start+length) if k==0xb3310000)
        header_bytes = struct.unpack_from('<I',raw,header_chunk+4)[0]
        fields = {k:(p+8,n-8) for k,p,n in children(header_chunk+8,header_chunk+header_bytes+8)}
        indices,index_bytes = fields[0x33310002]; count = index_bytes//8
        header,_ = fields[0x33310001]
        case = core.base_case(1,random.Random(123)); data = bytearray(0x10000)
        root = core.ARENA+len(data); data.extend(raw[start:start+length])
        steps = [('load_root',0,[root],0)]; unload = []
        if 0x33310003 in fields:
            if decoder is None: decoder = Original(EXE)
            slots,slot_bytes = fields[0x33310003]; assert slot_bytes==count*24
            owner = core.ARENA+0xb000
            struct.pack_into('<I',data,owner-core.ARENA+0xbc,root+header-start)
            for j in range(count):
                hash_,offset,packed,size,_,_ = struct.unpack_from('<6I',raw,slots+j*24)
                block = raw[offset:offset+packed]
                address = {b'JDLZ':0x573700,b'HUFF':0x5744c0,b'RAWW':0x5744f0}[block[:4]]
                result,decoded = decoder.call(address,block,size); assert result==size
                assert struct.unpack_from('<I',decoded,size-0x9c+0x24)[0]==hash_
                request = core.ARENA+0x8000+j*16; struct.pack_into('<I',data,request-core.ARENA+0xc,size)
                cursor = (len(data)+127)&~127
                data.extend(bytes(cursor-len(data))+decoded)
                params = [core.ARENA+cursor,request,owner]
                steps.append(('load_slot',0,params,0)); unload.append(('unload_slot',0,params,0))
        for j in range(count):
            hash_ = struct.unpack_from('<I',raw,indices+j*8)[0]
            steps.append(('lookup',0,[hash_,0,0],core.ARENA+0x6000+j*4))
        steps+=unload+[('unload_root',0,[root],0)]
        assert len(data)<core.SIZE
        g = case['globals']
        for a in (0x830af0,0x8358c8,0x82dc00): g[a]=struct.pack('<2I',a,a)
        for a in (0x833780,0x82db08,0x82da80,0x82db10): g[a]=bytes(4)
        g[0x833784]=bytes(8192); g[0x82db0c]=struct.pack('<I',core.ARENA+0xc00)
        g[0x870790]=struct.pack('<I',core.ARENA+0xc80)
        case.update(data=data,globals=g,calls={'root_cycle':(0,[])},drivers={'root_cycle':steps},
                    shorten_on_upload=False,budget=50000000,retail_count=count)
        yield case


def verify(image,manifest,linked,gcc,source,kinds):
    a,b = Machine(image,manifest),Machine(image,manifest,linked,gcc)
    totals,events = Counter(),Counter()
    for i,case in enumerate(source):
        for kind in kinds:
            try: x,y = a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError) as e: raise AssertionError(('texture pack mismatch',kind,i,str(e))) from e
            if x != y:
                detail = [(hex(j),v,w) for j,(v,w) in enumerate(zip(x[0],y[0])) if v!=w][:8]
                raise AssertionError(('texture pack mismatch',kind,i,detail,x[2],y[2],x[3][:3],y[3][:3]))
            if 'retail_count' in case:
                assert all(struct.unpack_from('<'+'I'*case['retail_count'],x[0],0x6000))
                assert a.word(0x833780)==0 and a.word(0x82db08)==0
                assert bytes(a.u.mem_read(0x833784,8192))==bytes(8192)
                for head in (0x830af0,0x8358c8): assert (a.word(head),a.word(head+4))==(head,head)
            totals[kind]+=1; events.update(e[0] for e in x[3])
    return dict(comparisons=dict(totals),boundary_calls=dict(events))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc',action='store_true'); p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--kind',default='all'); p.add_argument('--replace',type=Path)
    p.add_argument('--retail',type=Path,action='append',default=[]); p.add_argument('--mutation-checks',action='store_true')
    args = p.parse_args(); assert args.cases>0
    raw = EXE.read_bytes(); image = pefile.PE(data=raw).get_memory_mapped_image(); manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'],16)-0x400000; assert hashlib.sha256(image[a:a+f['size']]).hexdigest() == f['sha256']
    linked = build(args.replace or SOURCE,args.gcc)
    kinds = ['init','snapshot','free_snapshot','load_header','unload_header','queue','find_pixels',
        'anim_construct','anim_update','anim_destroy','load_root','header_cycle','root_cycle','animation_cycle'] if args.kind=='all' else [args.kind]
    report = verify(image,manifest,linked,args.gcc,cases(args.cases),kinds)
    if args.retail:
        report['retail']=[]
        for path in args.retail:
            fixtures=list(retail_cases(path)); assert fixtures
            report['retail'].append(dict(sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                textures=sum(c['retail_count'] for c in fixtures),**verify(image,manifest,linked,args.gcc,fixtures,['root_cycle'])))
    report.update(failures=0,compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.mutation_checks:
        mutations={
            'wrong-pool-count':('init','bNewSlotPool(0x7c,0x80,','bNewSlotPool(0x7c,0x81,'),
            'wrong-snapshot-token':('snapshot','*snapshot = state->texture18;','*snapshot = 0;'),
            'wrong-snapshot-pool':('free_snapshot','bFree(ug2TextureSnapshotPool870790,snapshot);','bFree(ug2TexturePackPool82DB0C,snapshot);'),
            'skip-header-flag':('load_header','header->loaded6C = 1;','header->loaded6C = 2;'),
            'wrong-index-count':('load_header','indexCount = child[1] >> 3;','indexCount = child[1] >> 2;'),
            'ignore-count-mismatch':('load_header','if (count != stateCount) return 0;','(void)0;'),
            'skip-header-clear':('header_cycle','header->pack70 = 0;','(void)0;'),
            'wrong-pixel-pointer':('queue','base+bytes+chunk[3]+0x18','base+bytes+chunk[3]+0x1c'),
            'wrong-queue-key':('find_pixels','reinterpret_cast<unsigned int *>(node)[3] == key','reinterpret_cast<unsigned int *>(node)[2] == key'),
            'wrong-animation-stride':('anim_construct','<< 4','<< 3'),
            'wrong-ready':('anim_update','unsigned int ready = 1;','unsigned int ready = 0;'),
            'skip-animation-free':('anim_destroy','CarRenderFreeTextureSnapshot(frame->snapshot08);','(void)0;'),
            'wrong-attach-offset':('root_cycle','pack->Attach(data,0,','pack->Attach(data,1,'),
            'skip-pixel-unlink':('root_cycle','Unlink(queued);','(void)0;'),
            'skip-current-clear':('root_cycle','ug2TextureCurrentPack82DB10 = 0;','(void)0;'),
            'skip-animation-unlink':('animation_cycle','Unlink(&link00);','(void)0;'),
        }
        rejected=[]
        for name,(kind,before,after) in mutations.items():
            if kind not in kinds: continue
            body=SOURCE.read_text(); assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp'); path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc'] if args.gcc else [])
            result=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'texture pack mismatch' in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc' if args.gcc else 'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__': main()
