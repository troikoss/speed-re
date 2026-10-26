#!/usr/bin/env python3
"""Compare UG2 texture records, pack lookup, registration and byte-range lifetime.

Run the recovered binary search and string helpers in the same chain. Formatting,
return-this, GPU upload/release, reference notifications and the missing-default
trap use controlled boundaries. No retail assets or disassembly are distributed.
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
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE
import verify_pc_car_render_solid_load as platform
import verify_pc_car_render_solid_pack as pack

SOURCE = ROOT/'src/Speed/Indep/Src/World/CarRenderTexture.cpp'
BUILD = ROOT/'build/analysis/car-render-texture'
MANIFEST = ROOT/'config/SPEED2_EXE_1_2/car-render-texture.json'
ARENA, SIZE, STACK, STOP = 0x2000000, 0x1000000, 0x3800000, 0x3810000
PACK, HEADER, INDEX, TEXTURES, STATES, PIXELS = [ARENA+x for x in (0x1000,0x2000,0x3000,0x5000,0x8000,0x10000)]
GLOBALS = dict(ug2TexturePacks830AF0=0x830af0, ug2TextureCount833780=0x833780,
    ug2TextureChanged82DAB4=0x82dab4, ug2TextureBuckets833784=0x833784,
    ug2TextureOverrideCount82DA80=0x82da80, ug2TextureOverrideKeys82D530=0x82d530,
    ug2TextureOverrideValues82D5C0=0x82d5c0, ug2TextureDefault82DB08=0x82db08,
    ug2TextureNull78DA04=0x78da04, ug2TextureLabel78D9F4=0x78d9f4, ug2TextureInventory78D9C0=0x78d9c0)
ORIGINAL_BOUNDARIES = dict(format=0x4400d0, end=0x502fd0, trap=0x43bd50, upload=0x5ce170,
    release=0x5caaa0, solids=0x4939b0, notify5c=0x5c0dc0, notify51=0x51bf70, notify61=0x611a10)
BOUNDARIES = {k:STOP+0x100+i*16 for i,k in enumerate(ORIGINAL_BOUNDARIES)}
C_BOUNDARIES = dict(UG2TextureMissingDefault='trap', UG2TextureUpload='upload', UG2TextureRelease='release',
    UG2TextureNotifySolids='solids', UG2TextureNotify5C0DC0='notify5c',
    UG2TextureNotify51BF70='notify51', UG2TextureNotify611A10='notify61')
METHODS = {'construct','destroy','register','register_array','unregister_array','find_loaded','attach','detach','set_state'}
RETURNS = {'construct','find_loaded','lookup','refresh','inventory'}
PREFIX = dict(construct='Construct@CarRenderTexturePack',destroy='Destroy@CarRenderTexturePack',
    register='Register@CarRenderTexturePack',register_array='RegisterArray@CarRenderTexturePack',
    unregister_array='UnregisterArray@CarRenderTexturePack',find_loaded='FindLoaded@CarRenderTexturePack',
    attach='Attach@CarRenderTexturePack',detach='Detach@CarRenderTexturePack',set_state='SetState@CarRenderTexture',
    lookup='_UG2SolidFindTexture',refresh='CarRenderRefreshTexture@@',inventory='_UG2TextureInventory',
    inventory_release='_UG2TextureInventoryRelease',load_slot='CarRenderLoadTextureSlot@@',
    unload_slot='CarRenderUnloadTextureSlot@@')
GCC = dict(construct='_ZN20CarRenderTexturePack9Construct',destroy='_ZN20CarRenderTexturePack7Destroy',
    register='_ZN20CarRenderTexturePack8RegisterE',register_array='_ZN20CarRenderTexturePack13RegisterArray',
    unregister_array='_ZN20CarRenderTexturePack15UnregisterArray',find_loaded='_ZN20CarRenderTexturePack10FindLoaded',
    attach='_ZN20CarRenderTexturePack6Attach',detach='_ZN20CarRenderTexturePack6Detach',
    set_state='_ZN16CarRenderTexture8SetState',lookup='UG2SolidFindTexture',refresh='_Z23CarRenderRefreshTexture',
    inventory='UG2TextureInventory',inventory_release='UG2TextureInventoryRelease',
    load_slot='_Z24CarRenderLoadTextureSlot',unload_slot='_Z26CarRenderUnloadTextureSlot')


def build(source, gcc):
    bindings = dict(platform.GLOBALS, **platform.BINDINGS, **pack.GLOBAL, **GLOBALS,
        UG2SolidFindLoaded=STOP+0x400, UG2SolidFindOwner=STOP+0x410)
    sources = [source,pack.SOURCE,platform.SOURCE,ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp']
    if gcc:
        bindings.update({k:BOUNDARIES[v] for k,v in C_BOUNDARIES.items()})
        bindings.update({'_Z8bSFormatPcPKcz':BOUNDARIES['format'], '_ZN18CarRenderLightList3EndEv':BOUNDARIES['end']})
        linked = link_native(sources,BUILD,ROOT,bindings,GCC['lookup'],text_address=0x4000000)
    else:
        BUILD.mkdir(parents=True,exist_ok=True)
        objects = []
        externals = {'_'+k:v for k,v in bindings.items()}
        for i,path in enumerate(sources):
            obj = BUILD/('texture-'+str(i)+'.obj')
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c',
                '/Ox','/Ob0','/EHsc','/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),
                str(path.relative_to(ROOT))],cwd=ROOT,check=True)
            objects.append(obj)
            for symbol in read_object(obj)['symbols'].values():
                if symbol['section'] != 0: continue
                name = symbol['name']
                if name.startswith('?bSFormat@@'): externals[name] = BOUNDARIES['format']
                if name.startswith('?End@CarRenderLightList@@'): externals[name] = BOUNDARIES['end']
                for function,kind in C_BOUNDARIES.items():
                    if name in ('_'+function,'@'+function+'@4'): externals[name] = BOUNDARIES[kind]
        linked = link_objects(objects,externals,start=0x4000000)
    linked['entries'] = {}
    for kind,needle in (GCC if gcc else PREFIX).items():
        hits = [(n,v) for n,v in linked['exports'].items() if needle in n]
        if kind in ('inventory','inventory_release'):
            hits = [(n,v) for n,v in hits if n == ('_' if not gcc else '')+GCC[kind]]
        assert len(hits) == 1,(kind,hits)
        linked['entries'][kind] = hits[0][1]
    return linked


class Machine:
    def __init__(self,image,manifest,linked=None,gcc=False):
        self.u = u = Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(0x400000,(len(image)+4095)&~4095); u.mem_write(0x400000,image)
        u.mem_map(ARENA,SIZE); u.mem_map(STACK,0x20000)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,body in linked['blocks']: u.mem_write(a,body)
        self.entries = linked['entries'] if linked else {k:int(f['address'],16) for k,f in manifest['functions'].items()}
        self.gcc = gcc
        for kind,address in (BOUNDARIES if linked else ORIGINAL_BOUNDARIES).items():
            u.hook_add(UC_HOOK_CODE,self.boundary,user_data=kind,begin=address,end=address)

    def word(self,a): return struct.unpack('<I',bytes(self.u.mem_read(a,4)))[0]
    def string(self,a):
        raw = bytearray()
        while self.u.mem_read(a+len(raw),1)[0]: raw += self.u.mem_read(a+len(raw),1)
        return bytes(raw)

    def boundary(self,u,address,size,kind):
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        result = pop = 0
        if kind == 'end': result = self.word(sp+4) if self.gcc else u.reg_read(xc.UC_X86_REG_ECX)
        elif kind == 'format':
            dest,fmt,arg = [self.word(sp+i) for i in (4,8,12)]
            assert self.string(fmt) == b'TexturePack,%s'
            body = b'TexturePack,'+self.string(arg)
            u.mem_write(dest,body+b'\0'); result = len(body)
            self.events.append((kind,self.string(fmt),self.string(arg)))
        elif kind in ('upload','release'):
            texture = u.reg_read(xc.UC_X86_REG_ECX)
            state = self.word(texture)
            self.events.append((kind,texture,bytes(u.mem_read(texture,0x7c)),bytes(u.mem_read(state,0x20))))
            u.mem_write(state+0x18,struct.pack('<I',0xc0010000+len(self.events) if kind == 'upload' else 0))
            if self.case.get('shorten_on_upload') and kind == 'upload':
                # Controlled callback mutation checks that count74 is re-read.
                u.mem_write(PACK+0x74,bytes(4))
        elif kind == 'trap': self.events.append((kind,))
        else:
            args = tuple(self.word(sp+i) for i in ((4,) if kind == 'notify61' else (4,8)))
            self.events.append((kind,*args))
        ret = self.word(sp)
        u.reg_write(xc.UC_X86_REG_ESP,sp+4+pop)
        u.reg_write(xc.UC_X86_REG_EAX,result); u.reg_write(xc.UC_X86_REG_EIP,ret)

    def run(self,kind,case):
        self.case, self.events = case, []
        u = self.u; u.mem_write(ARENA,bytes(case['data']))
        for a,body in case['globals'].items(): u.mem_write(a,body)
        receiver,args = case['calls'][kind]
        entry = self.entries.get(kind,STOP+0x1000)
        if kind == 'cycle':
            code = bytearray()
            for function,this,params,capture in case['steps']:
                words = [this]+params if self.gcc and function in METHODS else params
                for word in reversed(words): code += b'\x68'+struct.pack('<I',word&0xffffffff)
                if function in METHODS and not self.gcc: code += b'\xb9'+struct.pack('<I',this)
                code += b'\xe8'+struct.pack('<i',self.entries[function]-(entry+len(code)+5))
                if (self.gcc or function not in METHODS) and words:
                    code += b'\x81\xc4'+struct.pack('<I',len(words)*4)
                if capture: code += b'\xa3'+struct.pack('<I',capture)
            code += b'\xc3'; u.mem_write(entry,bytes(code))
            u.ctl_remove_cache(entry,entry+0x10000)
        actual = [receiver]+args if self.gcc and kind in METHODS else args
        sp = STACK+0x8000
        u.mem_write(sp,struct.pack('<'+'I'*(1+len(actual)),STOP,*(v&0xffffffff for v in actual)))
        u.reg_write(xc.UC_X86_REG_ESP,sp); u.reg_write(xc.UC_X86_REG_ECX,receiver)
        u.reg_write(xc.UC_X86_REG_EFLAGS,2)
        saved = (xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
        for i,r in enumerate(saved): u.reg_write(r,0xabcdef00+i)
        u.reg_write(xc.UC_X86_REG_FPCW,0x37f); u.reg_write(xc.UC_X86_REG_FPSW,0); u.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
        u.emu_start(entry,STOP,count=case.get('budget',3000000))
        assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
        assert u.reg_read(xc.UC_X86_REG_ESP) == sp+4+(4*len(args) if kind in METHODS and not self.gcc else 0)
        assert [u.reg_read(r) for r in saved] == [0xabcdef00+i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW) == 0x37f and u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        return (bytes(u.mem_read(ARENA,len(case['data']))),
            [bytes(u.mem_read(a,len(body))) for a,body in sorted(case['globals'].items())],
            u.reg_read(xc.UC_X86_REG_EAX) if kind in RETURNS else None,self.events)


def base_case(index,rng):
    data = bytearray(rng.randbytes(0x20000))
    def put(a,fmt,*v): struct.pack_into('<'+fmt,data,a-ARENA,*v)
    count = index%9
    keys = sorted(rng.choices([0,1,0x2001,0x80000001,0x66994f4a,0xffffffff,0x530b82b0+index//8%8],k=count))
    g = {0x833780:struct.pack('<I',rng.choice((0,1,0xffffffff))),0x82dab4:struct.pack('<I',rng.getrandbits(32)),
         0x833784:rng.randbytes(8192),0x82db08:struct.pack('<I',TEXTURES if index%3 else 0),
         0x82da80:struct.pack('<I',4),0x82d530:bytes(36*4),0x82d5c0:bytes(36*4)}
    # Three real pack contexts exercise order, missing headers, duplicate keys,
    # unloaded records and promotion from the middle and tail of the list.
    for p in range(3):
        desc,header,table = PACK+p*0x100,HEADER+p*0x100,INDEX+p*0x100
        put(desc,'2I',PACK+(p+1)*0x100 if p<2 else 0x830af0,PACK+(p-1)*0x100 if p else 0x830af0)
        put(desc+0x70,'Ii',header if (index+p)%7 else 0,count if index%11 else -1)
        put(header+0x74,'I',table if (index+p)%13 else 0)
        for j,key in enumerate(keys):
            texture,state = TEXTURES+(p*16+j)*0x7c,STATES+(p*16+j)*0x20
            put(table+j*8,'2I',key,texture if (index+p+j)%4 else 0)
            put(texture,'I',state); put(texture+0x24,'I',key)
            put(texture+0x2c,'I',rng.choice(keys+[0,0xdeadbeef]))
            offsets = (-2147483648,-1,0,1,16,31,32,63,64,127,128,0x7fffffff)
            put(texture+0x30,'iiII',rng.choice(offsets),rng.choice(offsets),rng.choice((0,16,256)),rng.choice((0,4,256)))
            put(texture+0x68,'4I',desc,0,PIXELS if (index+p+j)%3 else 0,PIXELS+0x100 if (index+p+j)%2 else 0)
    # First pack is always valid for range/register calls. Lookup still sees
    # missing header/index in the other two packs.
    put(PACK+0x70,'Ii',HEADER,count); put(HEADER+0x74,'I',INDEX)
    data[HEADER-ARENA+4:HEADER-ARENA+12] = b'fixture\0'
    data[HEADER-ARENA+0x20:HEADER-ARENA+0x2b] = b'CARS\\TEST\0\0'
    g[0x830af0] = struct.pack('<2I',PACK,PACK+0x200)
    if index%17 == 0: g[0x830af0] = struct.pack('<2I',0x830af0,0x830af0)
    query = rng.choice(keys+[0,2,0x66994f4a,0xffffffff])
    for j,key in enumerate([query,query,0x12345678,query]):
        body = bytearray(g[0x82d530]); struct.pack_into('<I',body,j*4,key); g[0x82d530] = bytes(body)
        body = bytearray(g[0x82d5c0]); struct.pack_into('<I',body,j*4,TEXTURES+0x7c if (index+j)%2 else 0); g[0x82d5c0] = bytes(body)
    # Deterministic bucket zero/one/255 cases supplement random collisions.
    if index%4 != 3:
        body = bytearray(g[0x833784]); body[query&8191] = (0,1,255)[index%4]; g[0x833784] = bytes(body)
    put(TEXTURES,'I',STATES)
    if not count: put(TEXTURES+0x24,'I',0x66994f4a)
    ranges = [(-2147483648,64),(-1,33),(0,32),(0,128),(16,48),(32,0),(64,-1),(0x7fffffff,2)]
    offset,length = ranges[index%len(ranges)]
    data[0x9000:0x9009] = b'explicit\0'; data[0x9100:0x9106] = b'path!\0'
    request,owner,decoded = [ARENA+x for x in (0xa000,0xa100,0xb000)]
    size = 0x400
    data[decoded-ARENA+size-0x9c:decoded-ARENA+size-0x20] = data[TEXTURES-ARENA:TEXTURES-ARENA+0x7c]
    put(request+0xc,'I',size); put(owner+0xbc,'I',HEADER); put(HEADER+0x70,'I',PACK)
    put(decoded+size-0x9c,'I',STATES)
    calls = dict(construct=(PACK,[HEADER if index%2 else 0,count,ARENA+0x9000 if index%3 else 0,ARENA+0x9100 if index%5 else 0]),
        destroy=(PACK,[]),register=(PACK,[TEXTURES,STATES+0x400,INDEX if index%3 else 0]),
        register_array=(PACK,[TEXTURES,STATES,count]),unregister_array=(PACK,[TEXTURES,STATES,count]),
        find_loaded=(PACK,[query]),lookup=(0,[query,index%3-1,index%2]),
        refresh=(0,[TEXTURES if index%3 else 0,query,PACK,(0,1,0x100,255)[index%4]]),
        attach=(PACK,[PIXELS,offset&0xffffffff,length&0xffffffff]),detach=(PACK,[offset&0xffffffff,length&0xffffffff]),
        set_state=(TEXTURES,[STATES+0x20 if index%2 else 0]),inventory=(0,[query]),inventory_release=(0,[query]),
        load_slot=(0,[decoded,request,owner]),unload_slot=(0,[decoded,request,owner]))
    return dict(data=data,globals=g,calls=calls,shorten_on_upload=index%29==0)


def cases(n):
    rng = random.Random(0x4901d0)
    for i in range(n): yield base_case(i,rng)


def retail_cases(path):
    raw = path.read_bytes(); found = []
    decompressor = None
    def children(start,end):
        while start < end:
            kind,size = struct.unpack_from('<2I',raw,start)
            assert start+8+size <= end
            yield kind,start+8,size
            start += size+8
        assert start == end
    for kind,start,size in children(0,len(raw)):
        if kind != 0xb3300000: continue
        for sub,base,length in children(start,start+size):
            if sub != 0xb3310000: continue
            fields = {key:(p,n) for key,p,n in children(base,base+length)}
            compressed = 0x33310003 in fields
            if not compressed and 0x33310004 not in fields: continue
            rng = random.Random(0x33310004)
            case = base_case(1,rng); data = bytearray(case['data'])
            header,hn = fields[0x33310001]; indices,in_ = fields[0x33310002]
            count = in_//8; assert in_%8 == 0
            # Keep complete serialized bytes and actual sort order. Only the
            # index/header pointers normally written by the loader are relocated.
            h,t,s,ix,out = [ARENA+x for x in (0x20000,0x21000,0x50000,0x60000,0x70000)]
            data.extend(bytes(0x80000-len(data)))
            copies = [(h,header,hn),(ix,indices,in_)]
            if not compressed:
                textures,tn = fields[0x33310004]; states,sn = fields[0x33310005]
                assert tn == count*0x7c and sn == count*0x20
                copies += [(t,textures,tn),(s,states,sn)]
            for address,offset,n in copies:
                data[address-ARENA:address-ARENA+n] = raw[offset:offset+n]
            struct.pack_into('<2I',data,h-ARENA+0x70,PACK,ix)
            keys = [struct.unpack_from('<I',data,ix-ARENA+i*8)[0] for i in range(count)]
            if compressed:
                from verify_pc_compression import Original
                if decompressor is None: decompressor = Original(EXE)
                slots,slot_bytes = fields[0x33310003]; assert slot_bytes == count*24
                owner = ARENA+0x40000; struct.pack_into('<I',data,owner-ARENA+0xbc,h)
                steps = [('construct',PACK,[h,count,0,0],0)]; unload = []
                for j in range(count):
                    key,offset,packed,size,_,_ = struct.unpack_from('<6I',raw,slots+j*24)
                    block = raw[offset:offset+packed]
                    address = {b'JDLZ':0x573700,b'HUFF':0x5744c0,b'RAWW':0x5744f0}[block[:4]]
                    result,decoded = decompressor.call(address,block,size); assert result == size
                    assert struct.unpack_from('<I',decoded,size-0x9c+0x24)[0] == key
                    request = ARENA+0x41000+j*16
                    struct.pack_into('<I',data,request-ARENA+0xc,size)
                    cursor = (len(data)+127)&~127
                    data.extend(bytes(cursor-len(data))+decoded)
                    params = [ARENA+cursor,request,owner]
                    steps.append(('load_slot',0,params,0)); unload.append(('unload_slot',0,params,0))
                for j,key in enumerate(keys): steps.append(('lookup',0,[key,0,0],out+j*4))
                steps += unload+[('destroy',PACK,[],0)]
                assert len(data) < SIZE
                case.update(data=data,calls={'cycle':(0,[])},steps=steps,shorten_on_upload=False,budget=50000000,
                            retail_count=count,retail_output=out,compressed=True)
                g = case['globals']; g[0x830af0] = struct.pack('<2I',0x830af0,0x830af0)
                g[0x833780] = bytes(4); g[0x833784] = bytes(8192); g[0x82db08] = bytes(4); g[0x82da80] = bytes(4)
                found.append(case)
                continue
            pixels = []
            for ck,p,n in children(start,start+size):
                if ck == 0xb3320000:
                    for cid,q,m in children(p,p+n):
                        if cid == 0x33320002:
                            aligned = (q+127)&~127
                            pixels = raw[aligned:q+m]
            assert pixels
            pixel_address = ARENA+0x80000
            data[pixel_address-ARENA:pixel_address-ARENA+len(pixels)] = pixels
            assert len(data) < SIZE
            steps = [('construct',PACK,[h,count,0,0],0),('register_array',PACK,[t,s,count],0),
                     ('attach',PACK,[pixel_address,0,len(pixels)],0)]
            for i,key in enumerate(keys): steps.append(('lookup',0,[key,0,0],out+i*4))
            steps += [('detach',PACK,[0,len(pixels)],0),('unregister_array',PACK,[t,s,count],0),('destroy',PACK,[],0)]
            case.update(data=data,calls={'cycle':(0,[])},steps=steps,shorten_on_upload=False,budget=10000000,
                        retail_count=count,retail_output=out)
            g = case['globals']; g[0x830af0] = struct.pack('<2I',0x830af0,0x830af0)
            g[0x833780] = bytes(4); g[0x833784] = bytes(8192); g[0x82db08] = bytes(4); g[0x82da80] = bytes(4)
            found.append(case)
    assert found,'No texture pack found'
    yield from found


def verify(image,manifest,linked,gcc,source,kinds):
    a,b = Machine(image,manifest),Machine(image,manifest,linked,gcc)
    totals,events = Counter(),Counter()
    for index,case in enumerate(source):
        for kind in kinds:
            try: x,y = a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError) as e:
                raise AssertionError(('texture mismatch',kind,index,str(e))) from e
            if x != y:
                detail = [(hex(i),u,v) for i,(u,v) in enumerate(zip(x[0],y[0])) if u!=v][:12]
                raise AssertionError(('texture mismatch',kind,index,detail,x[2],y[2],x[3][:2],y[3][:2]))
            if 'retail_count' in case:
                n = case['retail_count']; out = case['retail_output']-ARENA
                values = struct.unpack_from('<'+'I'*n,x[0],out)
                assert all(values),('retail textures not ready',values)
                assert a.word(0x833780) == 0 and bytes(a.u.mem_read(0x833784,8192)) == bytes(8192)
                assert a.word(0x82db08) == 0
                assert (a.word(0x830af0),a.word(0x830af4)) == (0x830af0,0x830af0)
            totals[kind] += 1; events.update(e[0] for e in x[3])
    return dict(comparisons=dict(totals),boundary_calls=dict(events))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--gcc',action='store_true'); p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--replace',type=Path); p.add_argument('--kind',default='all')
    p.add_argument('--retail',type=Path,action='append',default=[])
    p.add_argument('--mutation-checks',action='store_true')
    args = p.parse_args(); assert args.cases>0
    raw = EXE.read_bytes(); image = pefile.PE(data=raw).get_memory_mapped_image()
    manifest = json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    for f in manifest['functions'].values():
        a = int(f['address'],16)-0x400000
        assert hashlib.sha256(image[a:a+f['size']]).hexdigest() == f['sha256']
    linked = build(args.replace or SOURCE,args.gcc)
    kinds = list(manifest['functions']) if args.kind == 'all' else [args.kind]
    report = verify(image,manifest,linked,args.gcc,cases(args.cases),kinds)
    if args.retail:
        report['retail'] = []
        for path in args.retail:
            retail = list(retail_cases(path))
            report['retail'].append(dict(sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                textures=sum(c['retail_count'] for c in retail),
                **verify(image,manifest,linked,args.gcc,retail,['cycle'])))
    report.update(failures=0,compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',
        byte_identity='not established',limitations=[
            'controlled formatting, return-this, GPU and notification boundaries; trap resumed by fixture',
            'finite valid records and index/list layouts; no malformed chunk or allocation contract',
            'native record ownership and dispatcher activation remain separate work'])
    if args.mutation_checks:
        mutations = {
            'wrong-register-bucket':('register','++ug2TextureBuckets833784[texture->hash24 & 0x1fff];','--ug2TextureBuckets833784[texture->hash24 & 0x1fff];'),
            'wrong-default-key':('register','texture->hash24 == 0x66994f4a','texture->hash24 == 0x2c420d64'),
            'wrong-parent':('register','texture->parent6C = texture;','texture->parent6C = 0;'),
            'skip-state':('register','texture->SetState(state);','(void)0;'),
            'skip-pixel-clear':('register','texture->pixels70 = 0;','(void)0;'),
            'wrong-array-stride':('register_array','Register(textures+i,states+i,header70->indices74+i);','Register(textures+i,states,header70->indices74+i);'),
            'skip-unregister-default':('unregister_array','if (textures+i == ug2TextureDefault82DB08) ug2TextureDefault82DB08 = 0;','(void)0;'),
            'ignore-bucket':('find_loaded','if (ug2TextureBuckets833784[hash & 0x1fff])','if (true)'),
            'ignore-palette':('find_loaded','if (!texture->paletteBytes3C || texture->palette74)','if (true)'),
            'wrong-used-byte':('lookup','texture->used53 = 1;','texture->used53 = 2;'),
            'continue-null-override':('lookup','            break;\n        }','            continue;\n        }'),
            'ignore-raw-mode':('lookup','if (!rawMode) texture','if (true) texture'),
            'replace-current':('refresh','&& !pack->FindLoaded(hash)', '&& pack->FindLoaded(hash)'),
            'inclusive-range-end':('attach','value < end','value <= end'),
            'wrong-range-start':('attach','texture->pixelOffset30)-static_cast<unsigned int>(offset)','texture->pixelOffset30)-static_cast<unsigned int>(offset)+1'),
            'skip-notification':('detach','UG2TextureNotify5C0DC0(this,0);','(void)0;'),
            'wrong-special-hash':('detach','hash <= 0x530b82b5','hash <= 0x530b82b4'),
            'skip-pack-unlink':('destroy','Unlink(&link00);','(void)0;'),
            'wrong-slot-record':('load_slot','end-0x9c','end-0x98'),
            'wrong-slot-span':('load_slot','bytes = texture->paletteBytes3C+','bytes = 1+texture->paletteBytes3C+'),
            'skip-slot-unregister':('unload_slot','entry->texture04 = 0;\n        --ug2TextureCount833780;',
                                    'entry->texture04 = texture;\n        --ug2TextureCount833780;'),
        }
        rejected = []
        for name,(kind,before,after) in mutations.items():
            if kind not in kinds: continue
            body = SOURCE.read_text(); assert before in body,name
            path = BUILD/('mutation-'+name+'.cpp'); path.write_text(body.replace(before,after))
            command = [sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','256']+(['--gcc'] if args.gcc else [])
            result = subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'texture mismatch' in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations'] = rejected
    (BUILD/('verification-'+('gcc' if args.gcc else 'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__ == '__main__': main()
