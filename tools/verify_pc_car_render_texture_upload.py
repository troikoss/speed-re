#!/usr/bin/env python3
"""Compare original/rebuilt texture upload bytes and handle retirement.

Typed GPU, heap and already-recovered deferred-list/node-pool boundaries.
Compare complete records, lists, allocation guards, mip bytes and call order.
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
from probe_pc_race_reward import ROOT,EXE

SOURCE=ROOT/'src/Speed/Indep/Src/World/CarRenderTextureUpload.cpp'
BUILD=ROOT/'build/analysis/car-render-texture-upload'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-texture-upload.json'
ARENA,SIZE,STACK,STOP=0x2000000,0x2000000,0x4800000,0x4810000
TEXTURE,STATE,DEVICE,VTABLE,TOKEN,PIXELS,PALETTE=[ARENA+x for x in (0x1000,0x2000,0x4000,0x4100,0x5000,0x10000,0xb000)]
GLOBALS=dict(ug2TextureSkip870794=0x870794,ug2TextureDisabled87079C=0x87079c,ug2TextureUploads864F50=0x864f50,
    ug2TextureStates872B4C=0x872b4c,ug2TextureReleases8763B0=0x8763b0,ug2SolidDevice870974=0x870974)
CALLS={k:STOP+0x100+i*16 for i,k in enumerate(('create','lock','unlock','gpu_release','malloc','free','defer','node_free'))}
BINDINGS=dict(UG2TextureCreate=CALLS['create'],UG2TextureLock=CALLS['lock'],UG2TextureUnlock=CALLS['unlock'],
    UG2TextureReleaseGpu=CALLS['gpu_release'],UG2TextureMalloc=CALLS['malloc'],UG2TextureFree=CALLS['free'])


def build(source,gcc):
    bindings=dict(GLOBALS,**BINDINGS)
    if gcc:
        bindings.update({'_ZN27CarRenderDeferredBufferList3AddEj':CALLS['defer'],
                         '_Z15bDeleteListNodeP14WCollisionLink':CALLS['node_free']})
        linked=link_native(source,BUILD,ROOT,bindings,'UG2TextureUpload',text_address=0x5000000)
    else:
        BUILD.mkdir(parents=True,exist_ok=True);obj=BUILD/'upload.obj'
        subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
            '/Isrc/Speed/Indep/Src/World','/Fo'+str(obj.relative_to(ROOT)),str(source.relative_to(ROOT))],cwd=ROOT,check=True)
        external={'_'+k:v for k,v in bindings.items()}
        for symbol in read_object(obj)['symbols'].values():
            name=symbol['name']
            if symbol['section']!=0:continue
            if name.startswith('?Add@CarRenderDeferredBufferList@@'):external[name]=CALLS['defer']
            if name.startswith('?bDeleteListNode@@'):external[name]=CALLS['node_free']
        linked=link_objects([obj],external,start=0x5000000)
    prefixes=dict(upload='UG2TextureUpload' if gcc else '@UG2TextureUpload@4',
        release='UG2TextureRelease' if gcc else '@UG2TextureRelease@4',
        flush='_Z24CarRenderReleaseTexturesv' if gcc else '?CarRenderReleaseTextures@@',
        retire='_Z23CarRenderRetireTexturesv' if gcc else '?CarRenderRetireTextures@@')
    linked['entries']={k:next(v for n,v in linked['exports'].items() if n.startswith(p)) for k,p in prefixes.items()}
    return linked


class Machine:
    def __init__(self,image,manifest,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32)
        u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        u.mem_map(ARENA,SIZE);u.mem_map(STACK,0x20000)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,body in linked['blocks']:u.mem_write(a,body)
        self.entries=linked['entries'] if linked else {k:int(f['address'],16)for k,f in manifest['functions'].items()}
        self.gcc,self.rebuilt=gcc,bool(linked)
        boundaries=CALLS if linked else dict(create=CALLS['create'],lock=CALLS['lock'],unlock=CALLS['unlock'],
            gpu_release=CALLS['gpu_release'],malloc=0x575620,free=0x575630,defer=0x479130,node_free=0x440ef0)
        for k,a in boundaries.items():u.hook_add(UC_HOOK_CODE,self.boundary,user_data=k,begin=a,end=a)
    def word(self,a):return struct.unpack('<I',bytes(self.u.mem_read(a,4)))[0]
    def put(self,a,*values):self.u.mem_write(a,struct.pack('<'+'I'*len(values),*(v&0xffffffff for v in values)))
    def allocate(self,bytes_):
        a=self.heap;self.heap=(a+bytes_+63)&~31
        assert self.heap<ARENA+SIZE
        self.u.mem_write(a,b'\xa5'*(bytes_+32));return a+16
    def boundary(self,u,address,size,kind):
        sp=u.reg_read(xc.UC_X86_REG_ESP);pop=result=0
        counts=dict(create=9,lock=5,unlock=2,gpu_release=1,malloc=1,free=1,node_free=1)
        n=counts.get(kind,2 if self.gcc else 1)
        args=struct.unpack('<'+'I'*n,bytes(u.mem_read(sp+4,n*4)))
        if kind=='defer':
            head,token=args if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),args[0])
            node=ARENA+0xa000+len(self.nodes)*16;self.nodes.append(node)
            first,last=self.word(head),self.word(head+4)
            self.put(last,node);self.put(head+4,node);self.put(node,head,last,token)
            self.events.append((kind,head,token,node))
            if not self.gcc:pop=4
        elif kind=='create':
            assert args[0]==DEVICE and args[4]==0 and args[6]==1 and args[8]==0
            token=TOKEN if self.case.get('create_success',True) else 0
            self.put(args[7],token);self.events.append((kind,args[:7],args[7],args[8],token))
            result=self.case.get('hresult',0)
            if not self.rebuilt:pop=36
        elif kind=='lock':
            token,level,out,rectangle,flags=args;assert not rectangle and not flags
            capacity=max(64,self.case['capacity'] >> (min(level,15)*2))
            dest=self.allocate(capacity)
            if self.case.get('alias') and level==0:dest=PIXELS+4
            self.locks.append((dest,capacity));self.put(out,self.case.get('pitch',123),dest)
            self.events.append((kind,token,level,rectangle,flags,dest))
            if self.case.get('replace_token') and level==0:self.put(STATE+0x18,TOKEN+0x100)
            result=self.case.get('hresult',0)
            if not self.rebuilt:pop=20
        elif kind=='unlock':
            self.events.append((kind,*args))
            result=self.case.get('hresult',0)
            if not self.rebuilt:pop=8
        elif kind=='gpu_release':
            token=args[0];remaining=self.references.setdefault(token,self.case.get('references',1))
            result=max(0,remaining-1);self.references[token]=result
            self.events.append((kind,token,result))
            if not self.rebuilt:pop=4
        elif kind=='malloc':
            result=self.allocate(args[0]);self.allocations.append((result,args[0]))
            self.events.append((kind,args[0],result))
        elif kind=='free':
            self.events.append((kind,args[0]));self.frees.append(args[0])
        else:
            self.events.append((kind,args[0],bytes(u.mem_read(args[0],12))))
            self.put(args[0],0xdeadbeef)
        ret=self.word(sp);u.reg_write(xc.UC_X86_REG_ESP,sp+4+pop)
        u.reg_write(xc.UC_X86_REG_EAX,result);u.reg_write(xc.UC_X86_REG_EIP,ret)
    def run(self,kind,case):
        self.case=case;self.events=[];self.allocations=[];self.locks=[];self.nodes=[];self.frees=[];self.references={}
        self.heap=(ARENA+len(case['data'])+0x1fff)&~4095
        u=self.u;u.mem_write(ARENA,bytes(case['data']))
        for a,body in case['globals'].items():u.mem_write(a,body)
        self.put(DEVICE,VTABLE);self.put(VTABLE+0x5c,CALLS['create'])
        self.put(VTABLE+0x4c,CALLS['lock'],CALLS['unlock']);self.put(VTABLE+8,CALLS['gpu_release'])
        for token in (TOKEN,TOKEN+0x100,TOKEN+0x200,TOKEN+0x300):self.put(token,VTABLE)
        entry=self.entries.get(kind,STOP+0x1000)
        if kind=='cycle':
            code=bytearray()
            for f in ('upload','release','flush'):
                code+=b'\xb9'+struct.pack('<I',TEXTURE)
                code+=b'\xe8'+struct.pack('<i',self.entries[f]-(entry+len(code)+5))
            code+=b'\xc3';u.mem_write(entry,bytes(code));u.ctl_remove_cache(entry,entry+256)
        sp=STACK+0x8000;self.put(sp,STOP)
        u.reg_write(xc.UC_X86_REG_ESP,sp);u.reg_write(xc.UC_X86_REG_ECX,TEXTURE)
        saved=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
        for i,r in enumerate(saved):u.reg_write(r,0xabcdef00+i)
        u.reg_write(xc.UC_X86_REG_EFLAGS,2);u.reg_write(xc.UC_X86_REG_FPCW,0x37f)
        u.reg_write(xc.UC_X86_REG_FPSW,0);u.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
        u.emu_start(entry,STOP,count=case.get('budget',3000000))
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4
        assert [u.reg_read(r)for r in saved]==[0xabcdef00+i for i in range(4)]
        assert u.reg_read(xc.UC_X86_REG_FPCW)==0x37f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        return (bytes(u.mem_read(ARENA,len(case['data']))),
            [bytes(u.mem_read(a,len(body)))for a,body in sorted(case['globals'].items())],
            [bytes(u.mem_read(a-16,n+32))for a,n in self.allocations+self.locks],
            [bytes(u.mem_read(a,12))for a in self.nodes],self.events,self.frees,self.references)


def cases(n):
    rng=random.Random(0x5ce170)
    for i in range(n):
        data=bytearray(rng.randbytes(0x20000))
        def put(a,fmt,*v):struct.pack_into('<'+fmt,data,a-ARENA,*v)
        put(TEXTURE,'I',STATE)
        format_=rng.choice((21,41,0x31545844,0x33545844,0x35545844))
        put(STATE+0x14,'2I',format_,TOKEN+0x100 if i%3 else 0)
        put(TEXTURE+0x38,'IIIhh',512,1024,rng.choice((0,1,3,4,31,32,63)),rng.choice((-2,0,1,4,8)),rng.choice((-2,0,1,4,8)))
        put(TEXTURE+0x4e,'b',rng.choice((-3,0,1,2,3,4,6,17,33)))
        put(TEXTURE+0x70,'2I',PIXELS if i%11 else 0,PALETTE)
        for k in range(256):put(PALETTE+k*4,'I',rng.getrandbits(32))
        states=[STATE+0x100,STATE+0x200][:i%3]
        if i%3:states.insert(0,STATE)
        for j,state in enumerate(states):
            put(state,'2I',states[j+1] if j+1<len(states) else 0x872b4c,states[j-1] if j else 0x872b4c)
            if state!=STATE:put(state+0x18,'I',TOKEN+0x200)
        if STATE not in states:put(STATE,'2I',STATE,STATE)
        nodes=[ARENA+0x9000+j*16 for j in range(i%6)]
        for j,node in enumerate(nodes):put(node,'3I',nodes[j+1] if j+1<len(nodes) else 0x8763b0,
            nodes[j-1] if j else 0x8763b0,TOKEN+0x300)
        g={0x870794:struct.pack('<i',rng.choice((-2,-1,0,1,2,3,4,32))),0x87079c:bytes([i%7==0]),
            0x864f50:struct.pack('<I',rng.choice((0,0xffffffff))),0x870974:struct.pack('<I',DEVICE),
            0x872b4c:struct.pack('<2I',states[0] if states else 0x872b4c,states[-1] if states else 0x872b4c),
            0x8763b0:struct.pack('<2I',nodes[0] if nodes else 0x8763b0,nodes[-1] if nodes else 0x8763b0)}
        yield dict(data=data,globals=g,capacity=4096,create_success=i%5!=0,references=1+i%4,
            alias=i%17==0 and format_!=41,replace_token=i%13==0,hresult=i%3-1,pitch=-123 if i%2 else 64)


def retail_cases(path):
    from verify_pc_compression import Original
    raw=path.read_bytes();decoder=None
    def children(start,end):
        while start<end:
            key,size=struct.unpack_from('<2I',raw,start);assert start+size+8<=end
            yield key,start+8,size
            start+=size+8
        assert start==end
    def fixture(record,state,pixels,palette):
        case=next(cases(1));data=bytearray(0x10000+len(pixels)+64)
        data[TEXTURE-ARENA:TEXTURE-ARENA+0x7c]=record
        data[STATE-ARENA:STATE-ARENA+0x20]=state
        data[PIXELS-ARENA:PIXELS-ARENA+len(pixels)]=pixels
        if palette:data[PALETTE-ARENA:PALETTE-ARENA+len(palette)]=palette
        struct.pack_into('<I',data,TEXTURE-ARENA,STATE)
        struct.pack_into('<2I',data,TEXTURE-ARENA+0x70,PIXELS,PALETTE if palette else 0)
        struct.pack_into('<2I',data,STATE-ARENA,STATE,STATE);struct.pack_into('<I',data,STATE-ARENA+0x18,0)
        format_=struct.unpack_from('<I',state,0x14)[0]
        capacity=struct.unpack_from('<I',record,0x40)[0]*(4 if format_==41 else 1)+64
        case.update(data=data,capacity=capacity,alias=False,replace_token=False,create_success=True,references=1,
                    hresult=0,budget=100000000,format=format_)
        g=case['globals'];g[0x870794]=bytes(4);g[0x87079c]=bytes(1);g[0x864f50]=bytes(4)
        for head in (0x872b4c,0x8763b0):g[head]=struct.pack('<2I',head,head)
        return case
    for key,start,size in children(0,len(raw)):
        if key!=0xb3300000:continue
        top=list(children(start,start+size))
        header=next((p,n)for k,p,n in top if k==0xb3310000)
        fields={k:(p,n)for k,p,n in children(header[0],sum(header))}
        if 0x33310003 in fields:
            if decoder is None:decoder=Original(EXE)
            slots,slot_bytes=fields[0x33310003]
            for q in range(slots,slots+slot_bytes,24):
                hash_,offset,packed,size,_,_=struct.unpack_from('<6I',raw,q)
                block=raw[offset:offset+packed]
                addr={b'JDLZ':0x573700,b'HUFF':0x5744c0,b'RAWW':0x5744f0}[block[:4]]
                n,decoded=decoder.call(addr,block,size);assert n==size
                record=decoded[size-0x9c:size-0x20];state=decoded[size-0x20:size]
                assert struct.unpack_from('<I',record,0x24)[0]==hash_
                length=struct.unpack_from('<I',record,0x38)[0]
                yield fixture(record,state,decoded[:length],b'')
        else:
            records,count_bytes=fields[0x33310004];states,state_bytes=fields[0x33310005]
            assert count_bytes//0x7c==state_bytes//32
            pixel_chunk=next((p,n)for k,p,n in top if k==0xb3320000)
            begin,length=next((p,n)for k,p,n in children(pixel_chunk[0],sum(pixel_chunk))if k==0x33320002)
            pixels=raw[(begin+127)&~127:begin+length]
            for i in range(count_bytes//0x7c):
                record=raw[records+i*0x7c:records+(i+1)*0x7c];state=raw[states+i*32:states+(i+1)*32]
                offset,pal,bytes_,palette_bytes=struct.unpack_from('<4I',record,0x30)
                yield fixture(record,state,pixels[offset:offset+bytes_],pixels[pal:pal+1024]if palette_bytes else b'')


def verify(image,manifest,linked,gcc,fixtures,kinds):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);totals,events,formats=Counter(),Counter(),Counter()
    for i,case in enumerate(fixtures):
        for kind in kinds:
            try:x,y=a.run(kind,case),b.run(kind,case)
            except (UcError,AssertionError)as e:raise AssertionError(('texture upload mismatch',kind,i,str(e)))from e
            if x!=y:
                differences=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:8]
                raise AssertionError(('texture upload mismatch',kind,i,differences,x[4][:4],y[4][:4]))
            totals[kind]+=1;events.update(e[0]for e in x[4])
        if 'format'in case:formats[hex(case['format'])]+=1
    return dict(comparisons=dict(totals),boundary_calls=dict(events),formats=dict(formats))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048)
    p.add_argument('--kind',default='all');p.add_argument('--replace',type=Path)
    p.add_argument('--retail',type=Path,action='append',default=[]);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc)
    kinds=['upload','release','flush','retire','cycle']if args.kind=='all'else[args.kind]
    report=verify(image,manifest,linked,args.gcc,cases(args.cases),kinds)
    if args.retail:
        report['retail']=[]
        for path in args.retail:
            report['retail'].append(dict(sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                **verify(image,manifest,linked,args.gcc,retail_cases(path),['cycle'])))
    report.update(failures=0,compiler='GCC i386 C++11 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    report['limitations']=['controlled GPU, heap and already-recovered deferred enqueue/node-pool boundaries',
        'finite bounded copies and valid list layouts; failed GPU locks with unwritten outputs not established',
        'original paletted conversion allocations are retained on disabled/failed-create paths',
        'native staging storage is not a D3D or rendering equivalence claim']
    if args.mutation_checks:
        mutations={
            'wrong-palette-value':('upload','palette[*source++]','palette[0]'),
            'skip-compression-byte':('upload','texture->compression4A = 0x20;','(void)0;'),
            'skip-upload-count':('upload','++ug2TextureUploads864F50;','(void)0;'),
            'wrong-mip-divisor':('upload','ug2TextureSkip870794)*2','ug2TextureSkip870794)*4'),
            'skip-mip-prefix':('upload','position += levelBytes;','position += 0;'),
            'wrong-mip-copy-shift':('upload','shift+=2','shift+=1'),
            'skip-copy-tail':('upload','(static_cast<unsigned int>(bytes)&3)','0u'),
            'skip-expanded-free':('upload','if (expanded) UG2TextureFree(start);','(void)0;'),
            'skip-release-clear':('release','state->texture18 = 0;','(void)0;'),
            'skip-release-unlink':('release','Unlink(&state->link00);','(void)0;'),
            'wrong-retire-limit':('retire','i<3 &&','i<2 &&'),
            'ignore-disable-retire':('retire','if (!ug2TextureDisabled87079C)','if (true)'),
            'single-release':('flush','while (UG2TextureReleaseGpu(token)) {}','UG2TextureReleaseGpu(token);'),
            'skip-node-free':('flush','bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));','(void)0;'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(body.replace(before,after))
            cmd=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            result=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout)
            assert result.returncode and 'texture upload mismatch'in result.stdout,(name,result.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
