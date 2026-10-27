#!/usr/bin/env python3
"""Compare UG2's linked D3DX resource and byte-buffer input layer.

Win32 resource calls and allocation are controlled. Virtual buffer calls execute
recovered bodies through a native class-token bridge. No effect deserializer or
shader execution is modeled. Retail resource payloads come from the player EXE.
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
from pc_coff import link_objects
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT,EXE

SOURCE=ROOT/'src/Speed/Indep/Libs/D3DX/EffectInput.cpp'
BRIDGE=ROOT/'src/Platform/Native/NativeFxBufferDispatch.cpp'
BUILD=ROOT/'build/analysis/effect-input'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-input.json'
ARENA,SIZE,HEAP,HSIZE,STACK,STOP=0x2000000,0x10000,0x3200000,0x40000,0x4800000,0x4810000
RESOURCE,BUFFER,OUT,IID,NAME,BLOB=[ARENA+x for x in (0,0x100,0x300,0x400,0x600,0x8000)]
SPANS=dict(resource_construct=(0x670f13,13),resource_destroy=(0x670f20,14),clear=(0x670f2e,33),load=(0x670f4f,144),
    buffer_construct=(0x670e8e,24),buffer_destroy=(0x670da2,16),allocate=(0x670db2,44),delete=(0x670e24,33),
    query=(0x670e45,73),addref=(0x670dde,18),release=(0x670df0,37),pointer=(0x6954f7,15),length=(0x670e15,15),
    create=(0x670ea6,98),forward=(0x670f08,11))
MEMBERS=dict(resource_construct=('D3DXEffectResource','Construct'),resource_destroy=('D3DXEffectResource','Destroy'),
    clear=('D3DXEffectResource','Clear'),load=('D3DXEffectResource','Load'),buffer_construct=('D3DXEffectBuffer','Construct'),
    buffer_destroy=('D3DXEffectBuffer','Destroy'),allocate=('D3DXEffectBuffer','Allocate'),delete=('D3DXEffectBuffer','Delete'))
FREE=dict(query='D3DXEffectBufferQuery',addref='D3DXEffectBufferAddRef',release='D3DXEffectBufferRelease',
    pointer='D3DXEffectBufferPointer',length='D3DXEffectBufferLength',create='D3DXEffectBufferCreate',forward='D3DXEffectBufferCreateForward')
VOID={'resource_construct','resource_destroy','buffer_destroy'}
API_SPECS=[('DeleteObject',1,0x783058),('GetLastError',0,0x783114),('LockResource',1,0x7831dc),
    ('LoadResource',2,0x7831e0),('SizeofResource',2,0x7831e4),('FindResourceA',3,0x7831e8),('FindResourceW',3,0x7831ec)]
APIS={'UG2Fx'+name:STOP+0x2000+i*16 for i,(name,*_)in enumerate(API_SPECS)}
GLOBALS=dict(ug2FxIUnknown7D9904=0x7d9904,ug2FxBufferIid7B7430=0x7b7430)


def build(source,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    bindings=dict(GLOBALS,**APIS,UG2FxMalloc=0x575620,UG2FxFree=0x575630,abort=STOP+0x900)
    if gcc:
        linked=link_native([source,BRIDGE],BUILD,ROOT,bindings,'D3DXEffectBufferQuery',text_address=0x5000000,include_dirs=[SOURCE.parent])
        prefixes={k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()}
        prefixes.update(FREE)
    else:
        objects=[]
        for i,p in enumerate([source,BRIDGE]):
            obj=BUILD/('effect-input-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/Isrc/Speed/Indep/Libs/D3DX','/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()}
        for name,n,_ in API_SPECS:bindings['_UG2Fx'+name+'@'+str(n*4)]=APIS['UG2Fx'+name]
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:'?'+m+'@'+c+'@@'for k,(c,m)in MEMBERS.items()}
        prefixes.update({k:'_'+n+'@'for k,n in FREE.items()})
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for n,a in linked['exports'].items()if n==p or n.startswith(p if k in MEMBERS or not gcc else p+'@')]
        # ELF stdcall names have no @ suffix; the exact-name arm handles them.
        assert len(hits)==1,(k,p,hits);linked['entries'][k]=hits[0]
    return linked


class Machine:
    def __init__(self,image,linked=None,gcc=False):
        self.u=u=Uc(UC_ARCH_X86,UC_MODE_32);self.gcc=gcc
        u.mem_map(0,4096);u.mem_map(0x400000,(len(image)+4095)&~4095);u.mem_write(0x400000,image)
        for a,n in [(ARENA,SIZE),(HEAP,HSIZE),(STACK,0x20000)]:u.mem_map(a,n)
        if linked:
            u.mem_map(linked['address'],linked['allocation'])
            for a,b in linked['blocks']:u.mem_write(a,b)
        self.entries=linked['entries']if linked else{k:a for k,(a,n)in SPANS.items()}
        for a,k in [(0x575620,'malloc'),(0x575630,'free'),(STOP+0x900,'abort')]:u.hook_add(UC_HOOK_CODE,self.runtime,user_data=k,begin=a,end=a)
        for name,n,iat in API_SPECS:
            a=APIS['UG2Fx'+name];self.put(iat,a);u.hook_add(UC_HOOK_CODE,self.api,user_data=(name,n),begin=a,end=a)
    def put(self,a,*v):self.u.mem_write(a,struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v)))
    def word(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
    def args(self,n):return struct.unpack('<'+'I'*n,self.u.mem_read(self.u.reg_read(xc.UC_X86_REG_ESP)+4,n*4))
    def ret(self,result=0,pop=0):
        sp=self.u.reg_read(xc.UC_X86_REG_ESP);pc=self.word(sp)
        self.u.reg_write(xc.UC_X86_REG_EAX,result&0xffffffff);self.u.reg_write(xc.UC_X86_REG_ESP,sp+4+pop);self.u.reg_write(xc.UC_X86_REG_EIP,pc)
    def state(self):return bytes(self.u.mem_read(RESOURCE,0x110))+bytes(self.u.mem_read(OUT,0x120))+bytes(self.u.mem_read(HEAP,self.allocated))
    def runtime(self,u,address,size,kind):
        assert kind!='abort','unknown native buffer class'
        arg,=self.args(1);state=self.state()
        if kind=='malloc':
            self.requests+=1;result=0
            if self.requests!=self.case['fail_allocation']:
                result=HEAP+self.allocated;self.allocated+=max(16,(arg+15)&~15)
                assert self.allocated<=HSIZE;self.live[result]=arg
            self.events.append((kind,arg,result,state));self.ret(result)
        else:
            assert not arg or arg in self.live,('invalid free',hex(arg),self.live)
            body=bytes(u.mem_read(arg,self.live.pop(arg)))if arg else b''
            self.events.append((kind,arg,body,state));self.ret()
    def api(self,u,address,size,info):
        name,n=info;a=self.args(n);c=self.case;result=0
        self.events.append((name,a,bytes(u.mem_read(RESOURCE,12))))
        if name.startswith('FindResource'):
            result=(0x12300002 if c['bitmap_found']else 0)if a[2]==2 else (0x1230000a if c['raw_found']else 0)
        elif name=='SizeofResource':result=0 if c['fail_stage']=='size'else len(c['payload'])
        elif name=='LoadResource':result=0 if c['fail_stage']=='load'else 0x23400000
        elif name=='LockResource':result=0 if c['fail_stage']=='lock'else BLOB
        elif name=='GetLastError':result=0xffffffff
        else:
            result=c['delete_result']
            if c['callback']:self.put(RESOURCE,0x76543210,0x87654321,0x98765432)
        self.ret(result,n*4)
    def prepare(self,c):
        self.case=c;self.u.mem_write(ARENA,c['arena']);self.u.mem_write(HEAP,c['heap']);self.put(0,0x12345678)
        self.u.mem_write(BLOB,c['payload']);self.events=[];self.requests=0;self.allocated=0;self.live={}
        self.put(RESOURCE,c['old_handle'],0x13572468,0x24681357)
        self.put(BUFFER,0x7b711c,c['refs'],c['old_size'],0)
        guid=bytes(self.u.mem_read(c['guid'],16))if c['guid']else c['bad_guid']
        self.u.mem_write(IID,guid)
    def call(self,kind,args=(),receiver=0):
        u=self.u;member=kind in MEMBERS;params=([receiver]+list(args))if self.gcc and member else list(args)
        sp=STACK+0xf000;self.put(sp,STOP,*params)
        regs=[xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP];seeds=[0x11223344,0x22334455,0x33445566,0x44556677]
        for r,v in zip(regs,seeds):u.reg_write(r,v)
        for r,v in [(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,0x202),
            (xc.UC_X86_REG_FPCW,0x27f),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        pop=0 if self.gcc and member else len(args)*4
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop,('stack',kind)
        assert [u.reg_read(r)for r in regs]==seeds,('registers',kind)
        assert u.reg_read(xc.UC_X86_REG_FPCW)==0x27f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        assert self.word(0)==0x12345678
        return None if kind in VOID else u.reg_read(xc.UC_X86_REG_EAX)
    def snapshot(self,result):return bytes(self.u.mem_read(ARENA,SIZE)),bytes(self.u.mem_read(HEAP,HSIZE)),result,self.events,self.live,self.allocated
    def run(self,kind,c):
        self.prepare(c)
        if kind in ('buffer_destroy','allocate','delete','query','release','pointer','length','addref'):
            self.put(BUFFER+12,HEAP);self.live[HEAP]=32;self.allocated=32;self.live[BUFFER]=16
        if kind=='load':result=self.call(kind,[c['module'],NAME,c['bitmap_first'],c['wide']],RESOURCE)
        elif kind in ('resource_construct','resource_destroy','clear'):result=self.call(kind,receiver=RESOURCE)
        elif kind in ('buffer_construct','buffer_destroy'):result=self.call(kind,receiver=BUFFER)
        elif kind=='allocate':result=self.call(kind,[c['length']],BUFFER)
        elif kind=='delete':result=self.call(kind,[c['flags']],BUFFER)
        elif kind=='query':result=self.call(kind,[BUFFER,IID,(OUT,BUFFER+4,IID)[c['alias']]])
        elif kind in ('create','forward'):result=self.call(kind,[c['length'],0 if c['null_output']else OUT])
        else:result=self.call(kind,[BUFFER])
        return self.snapshot(result)
    def chain(self,c):
        self.prepare(c);self.call('resource_construct',receiver=RESOURCE)
        result=self.call('load',[c['module'],NAME,c['bitmap_first'],c['wide']],RESOURCE)
        if result==0:
            result=self.call('forward',[self.word(RESOURCE+8),OUT])
            if result==0:
                buffer=self.word(OUT);data=self.call('pointer',[buffer]);length=self.call('length',[buffer])
                assert length==len(c['payload']);self.u.mem_write(data,bytes(self.u.mem_read(self.word(RESOURCE+4),length)))
                assert bytes(self.u.mem_read(data,length))==c['payload']
                self.call('addref',[buffer]);self.call('release',[buffer]);self.call('release',[buffer])
        self.call('resource_destroy',receiver=RESOURCE);assert not self.live,('chain leak',self.live)
        return self.snapshot(result)


def cases(n):
    rng=random.Random(0x670f4f)
    for i in range(n):
        payload=rng.randbytes((0,1,3,4,7,64,255,1024)[i%8]);bad=bytearray(struct.pack('<4I',0,0,0,0))
        struct.pack_into('<I',bad,(i%4)*4,1<<(i%32))
        yield dict(arena=rng.randbytes(SIZE),heap=rng.randbytes(HSIZE),payload=payload,
            bitmap_first=(0,1,-1)[i%3],wide=(0,1,-1)[(i//3)%3],bitmap_found=bool(i&1),raw_found=i%5!=0,
            old_handle=0 if i%3==0 else 0x34500000,module=(0,0x12345678)[i%2],
            fail_stage=('none','size','load','lock')[(i//5)%4],callback=i%7==0,delete_result=i&1,
            fail_allocation=(0,1,2)[(i//4)%3],refs=(0,1,2,0xffffffff,0x80000000)[i%5],old_size=rng.getrandbits(32),
            guid=(0x7d9904,0x7b7430,0)[i%3],bad_guid=bytes(bad),flags=(0,1,2,3,0x80,0xff)[i%6],
            alias=(i//3)%3,length=(0,1,3,4,16,255,1024,18032)[i%8],null_output=i%11==0)


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();outcomes=Counter()
    fixtures=[(k,c)for c in cases(n)for k in kinds]
    if retail:
        for name,payload in retail:
            for wide in (0,1):
                c=next(cases(1));c.update(payload=payload,wide=wide,raw_found=True,bitmap_first=1,bitmap_found=False,
                    module=0,fail_stage='none',fail_allocation=0,callback=False)
                fixtures.append(('chain',c))
    for i,(kind,c)in enumerate(fixtures):
        try:x=a.chain(c)if kind=='chain'else a.run(kind,c);y=b.chain(c)if kind=='chain'else b.run(kind,c)
        except (AssertionError,UcError)as e:raise AssertionError(('effect input mismatch',kind,i,str(e)))from e
        if x!=y:
            offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
            event=next(((j,v[:3],w[:3])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
            raise AssertionError(('effect input mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event))
        counts[kind]+=1
        if x[2]is not None:outcomes[hex(x[2])]+=1
    return dict(comparisons=dict(counts),returns=dict(outcomes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512)
    p.add_argument('--kind',choices=['all',*SPANS,'chain'],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert args.cases>0 and hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    retail=[]
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id!=10:continue
        for entry in t.directory.entries:
            d=entry.directory.entries[0].data.struct;retail.append((str(entry.name),pe.get_data(d.OffsetToData,d.Size)))
    kinds=[*SPANS,'chain']if args.kind=='all'else[args.kind]
    report=verify(image,build(args.replace or SOURCE,args.gcc),args.gcc,args.cases,kinds,retail if args.kind=='all'else[])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        retail_resources=[dict(name=n,bytes=len(b),sha256=hashlib.sha256(b).hexdigest())for n,b in retail],
        limitations=['Win32 calls and allocation outcomes are controlled; native API coverage is separately bounded',
        'finite legal records/aliases and forced allocation failures; arbitrary malformed pointers and exceptions not established',
        'effect factory, deserialization, shader execution and rendering equivalence remain open'])
    if args.native_dump:
        dump=args.native_dump.read_bytes();assert dump[:8]==b'UG2FXIN1';pos=8;seen=set();total=0;resources=dict(retail)
        while pos<len(dump):
            index,wide,name_size,size=struct.unpack_from('<4I',dump,pos);pos+=16
            name=dump[pos:pos+name_size].decode('ascii');pos+=name_size
            body=dump[pos:pos+size];pos+=size
            assert 0<=index<23 and wide in (0,1) and (index,wide)not in seen
            pointer=struct.unpack_from('<I',image,0x7ff7d0-0x400000+index*0x8c+0x84)[0]-0x400000
            recipe=image[pointer:image.index(0,pointer)].decode('ascii')
            assert name==recipe and len(body)==size and body==resources[name],('native effect input mismatch',index,wide,name)
            seen.add((index,wide));total+=size
        assert pos==len(dump) and len(seen)==46
        report['native_replay']=dict(recipes=23,cycles=len(seen),payload_bytes=total,all_bytes_match=True)
    if args.mutation_checks:
        mutations={
            'omit-resource-clear':('resource_construct','handle00=0; data04=0; size08=0;','handle00=0; data04=0;'),
            'clear-on-destroy':('resource_destroy','void D3DXEffectResource::Destroy() { if (handle00) UG2FxDeleteObject(handle00); }','void D3DXEffectResource::Destroy() { Clear(); }'),
            'clear-before-delete':('clear','if (handle00) UG2FxDeleteObject(handle00);\n    handle00=0;','handle00=0; if (handle00) UG2FxDeleteObject(handle00);\n    handle00=0;'),
            'ignore-wide':('load','wide ? UG2FxFindResourceW : UG2FxFindResourceA','UG2FxFindResourceA'),
            'ignore-bitmap':('load','if (bitmapFirst) info=find(module,name,2);','(void)0;'),
            'omit-raw-fallback':('load','if (!info) info=find(module,name,10);','(void)0;'),
            'erase-stale-fields':('load','if (handle00) Clear();','Clear();'),
            'skip-size-zero':('load','if (size08) {','if (true) {'),
            'wrong-load-error':('load','0x88760b59','0x80004005'),
            'omit-error-query':('load','UG2FxGetLastError();','(void)0;'),
            'wrong-initial-refs':('buffer_construct','refs04=1;','refs04=0;'),
            'clear-dangling-buffer':('buffer_destroy','UG2FxFree(data0C); }','UG2FxFree(data0C); data0C=0; }'),
            'size-before-allocation':('allocate','data0C=static_cast<unsigned char *>(UG2FxMalloc(size));','size08=size; data0C=static_cast<unsigned char *>(UG2FxMalloc(size));'),
            'free-on-any-flag':('delete','if (flags&1)','if (flags)'),
            'omit-output-clear':('query','*output=0;','(void)0;'),
            'ignore-buffer-iid':('query','iid[i]==ug2FxBufferIid7B7430[i]','false'),
            'short-iid':('query','while (i<4 &&','while (i<3 &&'),
            'omit-query-addref':('query','UG2FxBufferAddRefVirtual(buffer);','(void)0;'),
            'wrong-addref-return':('addref','return ++buffer->refs04;','return buffer->refs04++;'),
            'release-on-zero':('release','if (old==1)','if (old==0)'),
            'wrong-pointer':('pointer','return buffer->data0C;','return reinterpret_cast<unsigned char *>(buffer);'),
            'wrong-length':('length','return buffer->size08;','return buffer->size08/4;'),
            'preclear-factory-output':('create','if (!output) return','if (output) *output=0; if (!output) return'),
            'leak-failed-factory':('create','UG2FxBufferDeleteVirtual(buffer,1); return result;','return result;'),
            'wrong-forward-size':('forward','return D3DXEffectBufferCreate(size,output);','return D3DXEffectBufferCreate(size+1,output);'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect input mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
