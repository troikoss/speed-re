#!/usr/bin/env python3
"""Verify complete UG2 deserialization and original debug/registry dependencies.

All parser, relocation and shared-pool bodies execute. Allocation and platform
loader/registry/GPU services are controlled. Native replay disables GPU creation
through the original effect flag, without fabricating a successful GPU object.
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
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_effect_shared_pool as sp

md=sp.md;ROOT,EXE,LIB=sp.ROOT,sp.EXE,sp.LIB
SOURCE=LIB/'EffectDeserialize.cpp';DEBUG=LIB/'EffectDebug.cpp';BUILD=ROOT/'build/analysis/effect-deserialize';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-deserialize.json'
ARENA,HEAP,STOP,EFFECT=sp.ARENA,sp.HEAP,sp.STOP,sp.EFFECT
POSITION,OUTPUT=ARENA+0x3000,ARENA+0x3100
SPANS=dict(deserialize=(0x66f69c,4015),registry=(0x641539,95),mute=(0x671aed,213))
GLOBALS=dict(ug2FxDebugRelease8A5CF8=0x8a5cf8,ug2FxDebugRuntime8A5CF4=0x8a5cf4,ug2FxDoNotMute80A170=0x80a170,
    ug2FxD3D9Name7B72A4=0x7b72a4,ug2FxD3D9DebugName7B7288=0x7b7288,ug2FxDebugMuteName7B7294=0x7b7294,ug2FxDoNotMuteName7B7278=0x7b7278,ug2FxRegistryPath7AB494=0x7ab494,
    ug2FxGetProcAddressImport783194=0x783194,ug2FxLoadLibraryImport783198=0x783198)
APIS=dict(module=('UG2FxGetModuleHandleA',1,0x783138),load=('UG2FxLoadLibraryA',1,0x783198),proc=('UG2FxGetProcAddress',2,0x783194),
    open=('UG2FxRegOpenKeyA',3,0x78300c),query=('UG2FxRegQueryValueExA',6,0x783004),close=('UG2FxRegCloseKey',1,0x783014))
TOKENS={k:STOP+0x9000+i*0x10 for i,k in enumerate(APIS)};LOAD_B,PROC_B,CALLBACK_A,CALLBACK_B,GPU_V,GPU_P=[STOP+i for i in(0x9100,0x9110,0x9200,0x9210,0x9300,0x9310)]
md.SOURCES+=[SOURCE,DEBUG,LIB/'EffectShaderResources.cpp',ROOT/'src/Platform/Native/NativeFxShaderDispatch.cpp',ROOT/'src/Platform/Native/NativeFxDeserializeDispatch.cpp',ROOT/'src/Platform/Native/NativeFxDebugPlatform.cpp']
md.BUILD=BUILD;md.core.GLOBALS.update(GLOBALS);md.MEMBERS.update(deserialize=('D3DXEffectObject','Deserialize'))
md.FREE.update(registry='D3DXEffectRegistryValue',mute='D3DXEffectDebugMute');md.STDCALL.update(registry=16,mute=4);md.VOID.add('mute')
md.core.HSIZE=0x200000


def pack(words):return sp.v.hc.cp.pack(words)
def synthetic(c):
    blob=bytearray(0x1000)
    for off,name in((0x40,b'p0'),(0x50,b'T0'),(0x60,b'a'),(0x70,b'm')):
        struct.pack_into('<I',blob,off,len(name)+1);blob[off+4:off+5+len(name)]=name+b'\0'
    family=c['family'];shape=c['shape'];elements=c['elements'];rows=c['rows'];columns=c['columns'];kind=3
    if family==1:kind,shape=4,4
    elif family==2:kind,shape=(15 if c['pixel']else 16),4
    elif family==3:kind,shape=0,5
    desc=[kind,shape,0x40,0,elements]
    if shape<4:desc += [rows,columns]
    elif shape==5:desc += [2,3,0,0x70,0,0,2,1,1,2,0x70,0,0,2,3]
    struct.pack_into('<'+'I'*len(desc),blob,0x100,*desc)
    struct.pack_into('<7I',blob,0x200,3,0,0x60,0,0,1,1)
    for i in range(128):struct.pack_into('<I',blob,0x400+i*4,0x3f000000+i if i%3 else 0)
    if shape==4:
        for i in range(elements or 1):struct.pack_into('<I',blob,0x400+i*4,i+1)
    if c['invalid_type']:struct.pack_into('<I',blob,0x104,6)
    struct.pack_into('<I',blob,0,c['blob_marker'])
    roots=1;techniques=0 if c['zero_techniques']else 1;passes=c['passes']
    groups=(elements or 1)if family==2 else 0;groups+=passes+4
    references=(elements or 1)+1 if shape==4 else 1
    annotations=[0x200,0x700]*c['annotations'];words=[0x100,0x400,c['parameter_flags'],c['annotations']]+annotations
    if techniques:
        words += [0x50,c['tech_annotations'],passes]+[0x200,0x700]*c['tech_annotations']
        for p in range(passes):words += [0,c['pass_annotations'],1 if c['pass_record']else 0]+[0x200,0x700]*c['pass_annotations']+([172,0,0x100,0x400]if c['pass_record']else[])
    symbol=c.get('symbol_payload',0)
    if symbol:
        # One pass shader record points at the root shader through a named
        # symbol, exposing the nonzero-owner sampler packing path.
        pass_words=[0,c['pass_annotations'],1]+[0x200,0x700]*c['pass_annotations']+[147 if c['pixel']else 146,0,0x100,0x800]
        root_words=[0x100,0x400,c['parameter_flags'],c['annotations']]+annotations
        words=root_words+[0x50,c['tech_annotations'],1]+[0x200,0x700]*c['tech_annotations']+pass_words
    resource=[]
    if family in(1,2):
        text=b'hello\0'if family==1 else pack([0xffff0200,0xffff])
        for i in range(elements or 1):resource+= [i+1,len(text)]+list(struct.unpack('<'+'I'*((len(text)+3)//4),text+bytes((-len(text))&3)))
    words += [elements or 1 if family in(1,2) else 0,1 if symbol else 0]+resource
    if symbol:
        name=b'p0\0';words += [0,0,0xffffffff,0,symbol]+([len(name)]if symbol==1 else[4+len(name),len(name)])+[int.from_bytes(name,'little')]
    header=[c['magic'],len(blob)];counts=[roots,techniques,groups,references]
    return pack(header)+bytes(blob)+pack(counts+words)


def cases(n):
    rng=random.Random(0x66f69c);base=next(sp.cases(1))
    for i in range(n):
        c=dict(base);c.update(family=i%4,shape=(i//4)%4,elements=(0,1,2,3)[(i//7)%4],rows=1+(i//11)%4,columns=1+(i//13)%4,pixel=(i//4)%2,
            annotations=i%3,tech_annotations=(i//5)%2,pass_annotations=(i//9)%2,passes=(i//6)%3,pass_record=i%5==0,
            parameter_flags=1 if i%5==0 else 0,pool=i%3==0,flags=(0x200,0x300,0,0x100,0xdead0200)[(i//7)%5],zero_techniques=i%31==0,
            invalid_type=i%37==0,blob_marker=1 if i%41==0 else 0,magic=0 if i%43==0 else 0xfeff0901,
            null_device=i%47==0,null_buffer=i%53==0,start=(0,0,1,2)[(i//17)%4],limit_mode=(i//19)%7,callback_kind='pointer'if i%2 else'length',
            callback_ordinal=(i//23)%12+1,cursor_change=1 if i%29==0 else 0,fail_allocation=(0,0,0,1,2,3,4,7,10,15,20,30)[(i//2)%12],
            setting=(-1,0,1,2,0x80000000)[i%5],cached_exports=(0,0,1,2,3)[(i//3)%5],mute_value=(0,1,0xffffffff)[i%3],repeat=i%7==0,
            modules=(i//5)%4,load_fail=(i//11)%4,exports=(i//13)%4,open_result=(0,2)[i%2],query_result=(0,2)[(i//3)%2],
            registry_type=(4,1,0xffffffff)[(i//5)%3],registry_value=(0,1,2,0xffffffff)[(i//7)%4],registry_bytes=(0,1,4,8)[i%4],query_write=i%3!=0,
            close_result=i%5,close_mutates=i%17==0,callback_mutates=i%11==0,switch_module=i%19==0,switch_proc=i%23==0,
            effect=rng.randbytes(0x130),arena=rng.randbytes(md.core.SIZE),heap=rng.randbytes(md.core.HSIZE))
        c['payload']=synthetic(c);yield c


class Machine(sp.Machine):
    instruction_limit=8000000
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.is_linked=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for kind,(name,n,iat)in APIS.items():
            a=linked['exports'][name if gcc else'_'+name+('@'+str(n*4)if kind not in('load','proc')else'')]if linked else TOKENS[kind]
            self.u.hook_add(UC_HOOK_CODE,self.debug_api,user_data=kind,begin=a,end=a);self.put(iat,TOKENS[kind])
        if linked:
            a=linked['exports']['UG2FxCallDebugMute'if gcc else'_UG2FxCallDebugMute'];self.u.hook_add(UC_HOOK_CODE,self.debug_callback,user_data=True,begin=a,end=a)
        else:
            for a,kind in((LOAD_B,'load'),(PROC_B,'proc')):self.u.hook_add(UC_HOOK_CODE,self.debug_api,user_data=kind,begin=a,end=a)
            for a in(CALLBACK_A,CALLBACK_B):self.u.hook_add(UC_HOOK_CODE,self.debug_callback,user_data=False,begin=a,end=a)
        a=linked['entries']['array']if linked else 0x401650;self.u.hook_add(UC_HOOK_CODE,self.array_entry,begin=a,end=a)
        for kind,original,name in(('vertex',GPU_V,'UG2FxCreateVertexShader'),('pixel',GPU_P,'UG2FxCreatePixelShader')):
            a=linked['exports'][name if gcc else'_'+name+'@16']if linked else original
            self.u.hook_add(UC_HOOK_CODE,self.gpu,user_data=kind,begin=a,end=a)
        for key,original,n in(('validate',0x66548b,3),('relocate',0x6614a1,4),('bind',0x64259c,2)):
            a=linked['entries'][key]if linked else original
            self.u.hook_add(UC_HOOK_CODE,self.body_entry,user_data=(key,n),begin=a,end=a)
        self.put(0x7b711c+0x16c,GPU_V);self.put(0x7b711c+0x1a8,GPU_P)
    def body_entry(self,u,address,size,data):
        key,n=data
        args=self.args(n+1)if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),*self.args(n))
        if key=='validate':args=(args[0],self.word(args[1]),self.word(args[2]),args[3])
        elif key=='relocate':args=(args[0],args[1],self.word(args[2]),self.word(args[3]),self.word(args[4]))
        self.events.append((key,args,self.state()))
    def extra(self):return b''.join(bytes(self.u.mem_read(a,4))for a in(0x8a5cf8,0x8a5cf4,0x80a170,0x783194,0x783198))
    def state(self):return hashlib.sha256(super().state()+self.extra()).digest()
    def snapshot(self,result):
        x=super().snapshot(result);return x+(self.extra(),)
    def array_entry(self,u,address,size,user):self.events.append(('array',self.args(4)[:3],self.state()))
    def getter(self,u,address,size,kind):
        super().getter(u,address,size,kind)
        if self.active_kind=='deserialize'and self.case['cursor_change']and kind==self.case['callback_kind']and self.getters[kind]==self.case['callback_ordinal']:
            self.put(POSITION,self.word(POSITION)+self.case['cursor_change'])
    def debug_api(self,u,address,size,kind):
        c=self.case;name,n,_=APIS[kind];bridge=self.is_linked and kind in('load','proc')
        args=self.args(n+(1 if bridge else 0));dispatch=args[0]if bridge else address;args=args[1:]if bridge else args
        result=0
        if kind=='module':
            index=0 if args[0]==0x7b72a4 else 1;result=0x11110001+index if c['modules']&(1<<index)else 0
            if c['switch_module']and not self.debug_calls['module']:self.put(0x783198,LOAD_B);self.put(0x783194,PROC_B)
            event=(kind,args)
        elif kind=='load':
            index=0 if args[0]==0x7b72a4 else 1;result=0 if c['load_fail']&(1<<index)else 0x11110001+index;event=(kind,dispatch,args)
        elif kind=='proc':
            index=args[0]-0x11110001;result=(CALLBACK_A,CALLBACK_B)[index]if c['exports']&(1<<index)else 0;event=(kind,dispatch,args)
            if c['switch_proc']and not self.debug_calls['proc']:self.put(0x783198,LOAD_B);self.put(0x783194,PROC_B)
        elif kind=='open':
            event=(kind,args[:2],self.word(args[2]));result=c['open_result']
            if not result:self.put(args[2],0x12340000)
        elif kind=='query':
            key,name,reserved,type_pointer,data,bytes_pointer=args
            event=(kind,(key,name,reserved,data),self.word(type_pointer),self.word(bytes_pointer));self.registry_data=data;self.registry_type_pointer=type_pointer
            self.put(type_pointer,c['registry_type']);self.put(bytes_pointer,c['registry_bytes'])
            if c['query_write']:self.put(data,c['registry_value'])
            result=c['query_result']
        else:
            event=(kind,args);result=c['close_result']
            if c['close_mutates']:self.put(self.registry_data,0x55);self.put(self.registry_type_pointer,4)
        self.events.append((*event,self.state()));self.debug_calls[kind]+=1;self.ret(result,0 if bridge else n*4)
    def debug_callback(self,u,address,size,bridge):
        if bridge:callback,mute=self.args(2)
        else:callback=address;mute,=self.args(1)
        self.events.append(('debug-callback',(callback,mute),self.state()))
        if self.case['callback_mutates']and callback==CALLBACK_A:self.put(0x8a5cf4,0);self.put(0x80a170,0x1234)
        self.ret(0x12345678)
    def gpu(self,u,address,size,kind):
        args=self.args(4 if self.is_linked else 3);dispatch,device,code,output=args if self.is_linked else(0x7b711c,*args)
        self.events.append(('gpu',(kind,dispatch,device,code,output),self.state()));self.put(output,0);self.ret(0x80004005,16 if self.is_linked else 12)
    def setup_debug(self,c):
        self.debug_calls=Counter();self.put(0x8a5cf8,CALLBACK_A if c['cached_exports']&1 else 0);self.put(0x8a5cf4,CALLBACK_B if c['cached_exports']&2 else 0);self.put(0x80a170,c['setting'])
        self.put(0x783198,TOKENS['load']);self.put(0x783194,TOKENS['proc'])
    def setup_deserialize(self,c,payload):
        self.active_kind='deserialize';self.prepare(c);self.getters=Counter();self.position=POSITION;self.setup_debug(dict(c,modules=0,open_result=2,cached_exports=0,setting=-1))
        self.case=dict(c,modules=0,open_result=2,cached_exports=0,setting=-1)
        self.crt_native=True;self.put(0x819d24,0x819cd0);self.put(sp.v.hc.THREAD+0x64,0x819cd0)
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT);self.put(EFFECT+0x98,0,0,0,0,7,7);self.put(EFFECT+0x14,0);self.put(EFFECT+0x1c,0);self.put(EFFECT+0x118,c['flags'])
        self.device=self.buffer(8);self.serialized=self.alloc(16);size=max(0x1000,len(payload)*4+64);stream=self.alloc(size);self.u.mem_write(stream,bytes(size));offset=c['start']*4;self.u.mem_write(stream+offset,payload)
        limit=len(payload)+offset if not c['limit_mode']else(0,1,2,4,8,len(payload)//4-1)[c['limit_mode']-1]
        self.put(self.serialized,0x7b711c,1,limit,stream);self.put(POSITION,c['start'])
        self.pool=self.alloc(0x20)if c['pool']else 0
        if self.pool:self.call('pool_construct',receiver=self.pool)
        self.requests=0;self.events=[]
    def run(self,kind,c,payload=None):
        if kind=='deserialize':
            self.setup_deserialize(c,payload or c['payload'])
            result=self.call(kind,[0 if c['null_device']else self.device,self.pool,0 if c['null_buffer']else self.serialized,POSITION],EFFECT)
        else:
            self.active_kind=kind;self.prepare(c);self.getters=Counter();self.setup_debug(c);self.put(OUTPUT,0x12345678)
            if kind=='registry':result=self.call(kind,[4,0x7b7278,OUTPUT,c['registry_bytes']])
            else:
                result=self.call(kind,[c['mute_value']])
                if c['repeat']:result=self.call(kind,[c['mute_value']^1])
        return self.snapshot(result)


def valid_case(index=0):
    c=next(cases(1));c.update(family=index%4,shape=(index//4)%4,elements=(0,1,2,3)[(index//7)%4],rows=1+(index//11)%4,columns=1+(index//13)%4,pixel=(index//4)%2,annotations=index%3,tech_annotations=(index//5)%2,pass_annotations=(index//9)%2,passes=(index//6)%3,pass_record=index%5==0)
    c.update(null_device=False,null_buffer=False,flags=0x200,start=0,limit_mode=0,fail_allocation=0,cursor_change=0,
        zero_techniques=False,invalid_type=False,blob_marker=0,magic=0xfeff0901,parameter_flags=1,pool=False)
    c['symbol_payload']=(1 if index%16==6 else 2)if index%8==6 else 0
    c['payload']=synthetic(c);return c


def native_cases(retail):
    for i in range(64):
        c=valid_case(i);yield c['payload'],i%2
    for row in retail:
        for pooled in(0,1):yield row['payload'],pooled


def digest(data):
    value=2166136261
    for byte in data:value=((value^byte)*16777619)&0xffffffff
    return value


def capture_effect(m,e,words):
    w=m.word;blob=w(e+0x20);mat=((blob+w(e+0x1c))&~31)-blob+32
    def fields(at,offsets):words.extend(w(at+o)for o in offsets)
    def hash_at(at,count):return digest(bytes(m.u.mem_read(at,count)))
    fields(e,(0x14,0x1c,0x34,0x3c,0x40,0x44,0x98,0xa0,0xa8,0xac,0x10c,0x110,0x118,0x124,0x128,0x12c))
    words.extend((m.half(e+0x30),hash_at(blob,w(e+0x1c)),int((blob+mat)%32==0)))
    at=blob+mat
    for size in(w(e+0x124),w(e+0x128),w(e+0x12c)):
        words.append(hash_at(at,size));at+=size
    for i in range(w(e+0x10c)):words.append(w(w(e+0x108)+i*4))
    table=w(e+0xb0);pool=w(e+0x38)
    for i in range(w(e+0xa8)):
        p=w(table+i*4);owner=w(table+w(p+0x14)*4);shared=bool(w(owner+0x40));base=w(pool+0x10)if shared else blob
        words.extend((int(w(p)==base+w(owner+0x30)+w(p+0x1c)),int(w(p+4)==base+w(owner+0x2c)+w(p+0x18))))
        if not i:continue
        fields(p,(8,0xc,0x10,0x14,0x18,0x1c,0x28,0x2c,0x38))
        offset=w(p+0x30);offset=offset if shared or offset<w(e+0x1c)else 0x80000000+offset-mat
        words.extend((offset,int(shared),w(p+0x44)))
    for i in range(w(e+0x14)):
        ref=w(e+0x18)+i*0x18;kind=w(ref);words.extend((kind,w(ref+0xc),w(ref+0x14)&3,(w(ref+0x10)&0xffff)if kind in(15,16)and w(ref+8)else 0))
        for slot in(4,8):
            b=w(ref+slot);words.extend((int(bool(b)),w(b+4)if b else 0,w(b+8)if b else 0,hash_at(w(b+0xc),w(b+8))if b else 0))
    for i in range(m.half(e+0x30)):
        g=w(e+0x2c)+i*0x64;words.append(w(g+8)&31);fields(g,(0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x34,0x38,0x3c))
        words.append(hash_at(w(g),w(g+0xc)))
        for j in range(m.half(g+0x36)):
            r=w(g+4)+j*0x64;fields(r,(0,4,0x10,0x18,0x1c,0x20,0x24))
            words.append(0 if w(r+0x58)in(0x09000000,0x0a000000,0x0b000000)else w(r+0x48));fields(r,(0x50,0x54,0x58,0x5c))
            words.extend((w(r+0x30)&31,m.half(r+0x4e),int(bool(w(r+0x3c))),int(bool(w(r+0x40))),int(bool(w(r+0x44)))))
            b=w(r+0x38);words.append(int(bool(b)))
            if b:
                data=w(b+0xc);words.append(w(b+8));fields(data,(0,4,8,12));words.append(hash_at(data+32,w(b+8)-32))
    for i in range(w(e+0xa0)):
        t=w(e+0xa4)+i*0xb4;fields(t,(0x28,0x2c,0x34));words.append(m.half(t+0x3c));fields(t,(0x88,0x8c,0x90,0x94,0x98))
        for j in range(w(t+0x34)):
            p=w(t+0x38)+j*0x38;fields(p,(0x28,0x2c));words.append(m.half(p+0x34))


def native_graph(m,payload,pooled):
    c=valid_case();c.update(flags=0x300 if pooled else 0x200,modules=0,open_result=2,cached_exports=0,setting=-1)
    m.case=c;m.active_kind='native';m.prepare(c);m.getters=Counter();m.setup_debug(c);m.crt_native=True;m.put(0x819d24,0x819cd0);m.put(sp.v.hc.THREAD+0x64,0x819cd0)
    device=m.buffer(8);pool=m.alloc(0x20)if pooled else 0
    if pool:m.call('pool_construct',receiver=pool)
    graphs=[];words=[]
    for index in range(2 if pooled else 1):
        e=EFFECT+index*0x200;m.u.mem_write(e,bytes([0xa5])*0x130);m.call('effect_ctor',receiver=e);m.put(e+0x98,0,0,0,0,7,7);m.put(e+0x14,0);m.put(e+0x1c,0);m.put(e+0x118,c['flags'])
        b=m.buffer(len(payload));m.u.mem_write(m.word(b+0xc),payload);m.put(POSITION,0);result=m.call('deserialize',[device,pool,b,POSITION],e)
        assert result==0,('effect deserialize mismatch','native-result',index,result)
        words.extend((result,m.word(POSITION),m.word(device+4),m.word(b+4)));graphs.append((e,b))
    for e,b in graphs:capture_effect(m,e,words)
    words.extend((m.word(0x8a5cf8),m.word(0x8a5cf4),m.word(0x80a170)))
    if pool:
        words.extend((m.word(pool+4),m.word(pool+0x14),m.word(pool+0x18),digest(bytes(m.u.mem_read(m.word(pool+0x10),m.word(pool+0x14))))))
    for e,b in reversed(graphs):m.call('destroy',receiver=e);m.call('buffer_release',[b])
    words.append(m.word(device+4));m.call('buffer_release',[device])
    if pool:words.append(m.word(pool+4));m.call('pool_release',[pool])
    assert not m.live,('effect deserialize mismatch','native-cleanup',m.live)
    return [x&0xffffffff for x in words]


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();returns=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect deserialize mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12];event=next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect deserialize mismatch',kind,i,diffs,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;returns[str((kind,x[2]))]+=1;events.update(e[0]for e in x[3])
    for i in range(8 if 'mute'in kinds else 0):
        c=next(cases(1));c.update(modules=3,load_fail=0,cached_exports=0,setting=0,exports=3,repeat=bool(i&4),switch_proc=bool(i&1),switch_module=bool(i&2),callback_mutates=bool(i&4))
        x=a.run('mute',c);y=b.run('mute',c)
        assert x==y,('effect deserialize mismatch','import-cache',i,x[2],y[2])
        counts['import_cache']+=1;events.update(e[0]for e in x[3])
    for i in range(64 if 'deserialize'in kinds else 0):
        c=valid_case(i);c.update(flags=(0x200,0x300,0,0x100)[(i//4)%4],pool=i%3==0)
        try:x=a.run('deserialize',c);y=b.run('deserialize',c)
        except(AssertionError,UcError)as e:raise AssertionError(('effect deserialize mismatch','valid',i,str(e)))from e
        assert x==y,('effect deserialize mismatch','valid',i,x[2],y[2],next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None))
        counts['valid']+=1;returns[str(('valid',x[2]))]+=1;events.update(e[0]for e in x[3])
    for row in retail if 'deserialize'in kinds else():
        c=next(cases(1));c.update(null_device=False,null_buffer=False,flags=0x200,start=0,limit_mode=0,fail_allocation=0,cursor_change=0,pool=False)
        try:x=a.run('deserialize',c,row['payload']);y=b.run('deserialize',c,row['payload'])
        except(AssertionError,UcError)as e:raise AssertionError(('effect deserialize mismatch','retail',row['recipe'],str(e)))from e
        if x!=y:
            diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12];event=next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
            raise AssertionError(('effect deserialize mismatch','retail',row['recipe'],diffs,x[2],y[2],event,len(x[3]),len(y[3])))
        counts['retail']+=1;returns[str(('retail',x[2]))]+=1
    for i in range(16 if 'deserialize'in kinds else 0):
        c=valid_case();c.update(cursor_change=1,callback_kind='pointer'if i%2 else'length',callback_ordinal=i//2+1)
        try:x=a.run('deserialize',c);y=b.run('deserialize',c)
        except(AssertionError,UcError)as e:raise AssertionError(('effect deserialize mismatch','cursor',i,str(e)))from e
        assert x==y,('effect deserialize mismatch','cursor',i,x[2],y[2])
        counts['cursor']+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),returns=dict(returns),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--replace-debug',type=Path);p.add_argument('--no-retail',action='store_true');p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    replace={}
    if args.replace:replace[SOURCE.name]=args.replace
    if args.replace_debug:replace[DEBUG.name]=args.replace_debug
    linked=md.build(replace,args.gcc);retail=[]if args.no_retail else sp.v.hc.cp.rd.retail_inputs(pe)
    if args.write_native_fixtures:
        rows=list(native_cases(retail));content=b'UG2FXDF1'+pack([len(rows)])
        for payload,pooled in rows:content+=pack([pooled,len(payload)])+payload
        args.write_native_fixtures.write_bytes(content)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind],retail);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.kind=='all' and not args.replace and not args.replace_debug:
        a,b=Machine(image),Machine(image,linked,args.gcc);native=[]
        for i,(payload,pooled)in enumerate(native_cases(retail)):
            try:x=native_graph(a,payload,pooled);y=native_graph(b,payload,pooled)
            except(AssertionError,UcError)as e:raise AssertionError(('effect deserialize mismatch','native',i,str(e)))from e
            assert x==y and a.snapshot(tuple(x))==b.snapshot(tuple(y)),('effect deserialize mismatch','native-state',i,x,y)
            native.append(x)
        report['native_chains']=len(native)
        if args.native_dump:
            raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXDD1';at=8
            for i,expected in enumerate(native):
                row,count=struct.unpack_from('<2I',raw,at);at+=8;values=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
                assert row==i and values==expected,('native effect deserialize mismatch',i,[(j,a,b)for j,(a,b)in enumerate(zip(expected,values))if a!=b][:20],len(expected),len(values))
            assert at==len(raw);report['native_replay']=len(native)

    if args.mutation_checks:
        mutations={
            'final-count-on-failure':('deserialize',SOURCE,'valueAC=valueA8;return result;','return result;'),
            'null-device-gate':('deserialize',SOURCE,'if (!device || !buffer)','if (!buffer)'),
            'temporary-zero-width':('deserialize',SOURCE,'i<7;++i','i<6;++i'),
            'pool-retain':('deserialize',SOURCE,'if (pool) UG2FxComAddRef','if (false) UG2FxComAddRef'),
            'input-buffer-retain':('deserialize',SOURCE,'value28=reinterpret_cast<unsigned int>(buffer);UG2FxComAddRef(reinterpret_cast<unsigned int>(buffer));','value28=reinterpret_cast<unsigned int>(buffer);'),
            'magic-cached-cursor':('deserialize',SOURCE,'FX_DESERIAL_CACHED(magic);','FX_DESERIAL_WORD(magic);'),
            'generic-cursor-reload':('deserialize',SOURCE,'destination=*reinterpret_cast<unsigned int *>(data+*position*4);','destination=*reinterpret_cast<unsigned int *>(data+(*position-1)*4);'),
            'blob-byte-count':('deserialize',SOURCE,'unsigned int bytes=value1C;','unsigned int bytes=value1C-1;'),
            'zero-technique-result':('deserialize',SOURCE,'if (!valueA0) goto invalid;','if (!valueA0) goto oom;'),
            'grown-tail-zero':('deserialize',SOURCE,'for (i=0;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(zero)[i]=0;','for (i=1;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(zero)[i]=0;'),
            'matrix-next-alignment':('deserialize',SOURCE,'reinterpret_cast<unsigned int>(grown)+0x20;','reinterpret_cast<unsigned int>(grown);'),
            'parameter-id-origin':('deserialize',SOURCE,'unsigned int identifier=1;','unsigned int identifier=0;'),
            'root-group-word':('deserialize',SOURCE,'parameter->value44=value30;parameter->value46=0;','parameter->value44=0;parameter->value46=0;'),
            'root-array-fallback':('deserialize',SOURCE,'if (!parameter->value4C) parameter->value4C=1;','if (!parameter->value4C) parameter->value4C=0;'),
            'annotation-validation':('deserialize',SOURCE,'result=ValidateType(&type,&data,index);if (result<0) goto finish; \\\n','result=0; \\\n'),
            'root-validation':('deserialize',SOURCE,'parameter->value44=value30;parameter->value46=0;\n            result=ValidateType(&type,&data,index);if (result<0) goto finish;','parameter->value44=value30;parameter->value46=0;\n            result=0;'),
            'payload-resource-loop':('deserialize',SOURCE,'entry<resourceEntries','entry<0'),
            'payload-compiled-routing':('deserialize',SOURCE,'result=ReadCompiledRecord(group,recordIndex,buffer,position);','result=ReadSymbolRecord(group,recordIndex,buffer,position);'),
            'payload-shader-selector':('deserialize',SOURCE,'else if (!selector) result=ReadShaderRecord','else if (selector) result=ReadShaderRecord'),
            'pool-parameter-flag':('deserialize',SOURCE,'FX_DESERIAL_OWNER(parameter)->value28&1','FX_DESERIAL_OWNER(parameter)->value28&2'),
            'group-storage-allocation':('deserialize',SOURCE,'result=group->AllocateData();if (result<0) goto finish;','result=0;'),
            'group-zero-width':('deserialize',SOURCE,'for (i=0;i<(count>>2);++i) reinterpret_cast<unsigned int *>(data)[i]=0;','for (i=1;i<(count>>2);++i) reinterpret_cast<unsigned int *>(data)[i]=0;'),
            'sampler-ordinal-origin':('deserialize',SOURCE,'unsigned int ordinal=0;','unsigned int ordinal=1;'),
            'packed-sampler-base':('deserialize',SOURCE,'storage[0]=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14]->value44+1;','storage[0]=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14]->value44;'),
            'effect-ready-flag':('deserialize',SOURCE,'value118|=0x40;','value118|=0x20;'),
            'selected-group-high-word':('deserialize',SOURCE,'reinterpret_cast<unsigned short *>(&value40)[1]=0;','(void)0;'),
            'root-dirty':('deserialize',SOURCE,'result=MarkParameterDirty(parameter);','result=0;'),
            'final-type-rebase':('deserialize',SOURCE,'record->value04=record->value18+owner->value2C+FX_DESERIAL_BASE(owner);','(void)0;'),
            'registry-close-on-query-error':('registry',DEBUG,'UG2FxRegCloseKey(key);','if (!result) UG2FxRegCloseKey(key);'),
            'registry-ignore-type':('registry',DEBUG,'!result && actualType==type','!result'),
            'registry-initial-type':('registry',DEBUG,'unsigned int actualType=reinterpret_cast<unsigned int>(data);','unsigned int actualType=type;'),
            'registry-root-key':('registry',DEBUG,'0x80000002,ug2FxRegistryPath7AB494','0x80000001,ug2FxRegistryPath7AB494'),
            'mute-probe-gate':('mute',DEBUG,'!ug2FxDebugRelease8A5CF8 && !ug2FxDebugRuntime8A5CF4','!ug2FxDebugRelease8A5CF8 || !ug2FxDebugRuntime8A5CF4'),
            'mute-existing-module':('mute',DEBUG,'if (module) {','if (!module) {'),
            'mute-import-cache':('mute',DEBUG,'UG2FxLoadLibraryA(loadDispatch,ug2FxD3D9DebugName7B7288)','UG2FxLoadLibraryA(ug2FxLoadLibraryImport783198,ug2FxD3D9DebugName7B7288)'),
            'mute-setting-cache':('mute',DEBUG,'if (ug2FxDoNotMute80A170==-1)','if (true)'),
            'mute-boolean-normalization':('mute',DEBUG,'if (ug2FxDoNotMute80A170) ug2FxDoNotMute80A170=1;','(void)0;'),
            'mute-disabled-gate':('mute',DEBUG,'if (ug2FxDoNotMute80A170) return;','if (false) return;'),
            'mute-callback-value':('mute',DEBUG,'UG2FxCallDebugMute(ug2FxDebugRelease8A5CF8,mute)','UG2FxCallDebugMute(ug2FxDebugRelease8A5CF8,!mute)'),
        }
        rejected=[]
        for name,(kind,source,before,after)in mutations.items():
            if args.kind!='all'and args.kind!=kind:continue
            text=source.read_text();assert before in text,(name,before)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(text.replace(before,after,1))
            command=[sys.executable,__file__,'--replace'if source==SOURCE else'--replace-debug',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect deserialize mismatch'in run.stdout,(name,run.stdout);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
