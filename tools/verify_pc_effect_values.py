#!/usr/bin/env python3
"""Verify UG2 effect relocation, recursive value writes and deferred ownership.

Allocation is controlled; shared rings are synthetic and COM receivers use
recovered buffer classes. Native captures cover local CPU graphs, not shaders
or the complete deserializer. Original PE code runs only in the emulator.
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
import verify_pc_effect_handles as hc

md=hc.md;ROOT,EXE,LIB=hc.ROOT,hc.EXE,hc.LIB
SOURCE=LIB/'EffectValues.cpp';BUILD=ROOT/'build/analysis/effect-values';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-values.json'
ARENA,HEAP,STOP,EFFECT=hc.ARENA,hc.HEAP,hc.STOP,hc.EFFECT
INPUT,TEXT,CURSORS,REFS=ARENA+0x8000,ARENA+0x9000,ARENA+0xa000,ARENA+0xb000
SPANS=dict(shader=(0x65cc10,184),object=(0x65ccc8,164),string=(0x65cd6c,303),typed=(0x65ddc8,928),copy=(0x661fc6,875),
    notify=(0x66055c,153),reference_ctor=(0x65ee33,13),walk=(0x65eb8f,644),references=(0x65ee13,32),record_value=(0x666c81,374),set_value=(0x66c8f5,108),relocate=(0x6614a1,428))
MEMBERS={k:('D3DXEffectObject',v)for k,v in dict(shader='WriteShaderResource',object='WriteObjectResource',string='WriteStringResource',typed='CopyTypedValue',
    copy='CopyValue',notify='NotifyValue',record_value='RecordValue',relocate='RelocateParameter').items()}
md.SOURCES+= [SOURCE,LIB/'EffectRuntimeStorage.cpp',ROOT/'src/Platform/Native/NativeFxValueDispatch.cpp',ROOT/'src/Speed/Indep/Src/Physics/D3DXMatrixScalar.cpp']
md.BUILD=BUILD;md.MEMBERS.update(MEMBERS)
md.FREE.update(reference_ctor='D3DXConstructReference08',walk='D3DXEffectWalkValueReferences',references='D3DXEffectValueReferences',set_value='D3DXEffectSetValue')
md.STDCALL.update(walk=20,references=20,set_value=16);md.VOID.add('reference_ctor')
md.MEMBERS.update(dirty=('D3DXEffectObject','MarkParameterDirty'),value_ctor=('D3DXEffectValuePrefix','Construct'))
md.VOID.add('value_ctor');md.FREE['reference_destroy']='D3DXDestroyReference08';md.VOID.add('reference_destroy')


def description(family,kind,shape,elements,rows,columns):
    def node(k,s,n=0,r=1,c=1,children=()):
        prefix=[k,s,0x50,0,n]
        return prefix+([r,c]if 0<=s<4 else[ len(children)]+sum(children,[])if s==5 else[])
    if family==2:
        child=node(kind,shape,elements,rows,columns)
        return node(0,5,elements,children=[node(2,0,2,2,1),node(0,5,0,children=[child,node(3,2,0,2,3)])])
    return node(kind,shape,elements,rows,columns)


def cases(n):
    base=next(hc.native_cases())[1];rng=random.Random(0x6614a1)
    for i in range(n):
        c=dict(base);family=i%4
        kind=([1,2,3,0,17,-1][(i//4)%6]if family in(0,3)else[4,5,6,9,10,15,16][(i//4)%7])
        shape=(i//7)%4 if family==0 else 4 if family in(1,2)else[-1,6,0,2,3][(i//4)%5]
        c.update(family=family,value_kind=kind,value_shape=shape,elements=(0,1,2,3)[(i//5)%4],rows=(i//11)%5,columns=(i//13)%5,
            shared_values=i%3==0,indexed=i%7==0,queue=i%2,selected=(i//3)%5,alias=(0,4,-4,16,256)[(i//17)%5],
            empty_input=i%11==0,same_input=i%13==0,reuse_size=(1,4,6,16,64)[(i//19)%5],tail=i%4,deferred=i%5==0,
            fail_allocation=(0,0,0,1,2,3,4,5,6)[(i//2)%9],handle_mode=(i//23)%4,small_bytes=i%17==0,
            count_only=i%3==0,walk_indexed=i%7==0,root_count=i%5,
            effect=rng.randbytes(0x130),arena=rng.randbytes(md.core.SIZE),heap=rng.randbytes(md.core.HSIZE))
        # Keep compact fixture streams bounded, including invalid descriptor paths.
        c['type_words']=description(family,kind,shape,c['elements'],c['rows'],c['columns'])
        c['input_words']=[rng.getrandbits(32)if j%4 else 0 for j in range(256)]
        yield c


class Machine(hc.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()},dirty=0x65ab14,value_ctor=0x65eb78,reference_destroy=0x65ee40)
        for key,address in [('addref',0x670dde),('release',0x670df0),('dirty',0x65ab14),('set_value',0x66c8f5)]:
            a=address
            if linked:
                if key in('addref','release'):
                    name='D3DXEffectBufferAddRef'if key=='addref'else'D3DXEffectBufferRelease'
                    a=linked['exports'][name if gcc else'_'+name+'@4']
                else:a=linked['entries'][key]
            self.u.hook_add(UC_HOOK_CODE,self.observe_value,user_data=key,begin=a,end=a)
        a=next(v for k,v in linked['exports'].items()if k.startswith('_Z25D3DXMatrixTransposeScalar'if gcc else'?D3DXMatrixTransposeScalar@@'))if linked else 0x63d132
        self.u.hook_add(UC_HOOK_CODE,self.observe_value,user_data='transpose',begin=a,end=a)
        self.put(0x803f00,0x63d132)
    def observe_value(self,u,address,size,kind):
        if kind=='dirty':
            import unicorn.x86_const as xc
            args=self.args(2)if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),*self.args(1))
        else:args=self.args(4 if kind=='set_value'else 2 if kind=='transpose'else 1)
        self.events.append((kind,args,self.state()))
    def buffer(self,size):
        p=self.alloc(16);data=self.alloc(size);self.put(p,0x7b711c,1,size,data);self.u.mem_write(data,b'x'*(size-1)+b'\0');return p
    def setup_values(self,c):
        self.active_kind='values';self.setup(c)
        self.u.mem_write(TEXT,b'value\0');self.u.mem_write(INPUT,hc.cp.pack(c['input_words']))
        self.parameter=self.parameters+0x50;self.put(self.parameter+0x40,0)
        self.u.mem_write(self.blob+0x500,hc.cp.pack(c['type_words']))
        self.u.mem_write(self.shared_blob+0x500,hc.cp.pack(c['type_words']))
        self.put(self.parameter,self.blob+0x2000,self.blob+0x500,0 if c['indexed']else 0xffffffff,256,3,1,0,0)
        self.put(self.parameter+0x28,1,0x500,0x2000)
        self.old=self.buffer(c['reuse_size']);self.new=self.buffer(8)
        # Replace the old fixture reference array with a larger, explicit graph.
        self.old_references=self.references;self.references=self.array(8,0x18,'reference_construct');self.put(EFFECT+0x14,8,self.references)
        for i in range(8):self.put(self.references+i*0x18,4,self.old if i==0 else 0);self.put(self.references+i*0x18+0x14,0xa5a5a5a4)
        self.other=ARENA+0x1600;self.u.mem_write(self.other,bytes(0x130));self.call('effect_ctor',receiver=self.other)
        otherrefs=self.array(8,0x18,'reference_construct');self.put(self.other+0x14,8,otherrefs);self.put(self.other+0xb0,self.table)
        self.put(self.other+0x2c,self.groups);self.put(self.other+0x118,0x100 if c['queue']else 0);self.put(self.other+0x110,1)
        otherqueue=self.alloc(4);self.put(self.other+0x108,otherqueue)
        self.link=self.alloc(0x14);node=self.alloc(0x14);empty=self.alloc(0x14)
        for p,e,record,previous,next_node in((self.link,EFFECT,self.parameter,empty,node),(node,self.other,self.parameter,self.link,empty),(empty,0,0,node,self.link)):
            mapping=self.alloc(32);self.put(mapping,*range(8));self.put(p,e,record,mapping,previous,next_node)
        if c['shared_values']:self.put(self.parameter+0x40,self.link)
        self.put(EFFECT+0x118,0x100 if c['queue']else 0);self.put(EFFECT+0x3c,3 if c['deferred']else 0);self.put(EFFECT+0xfc,EFFECT+0xe0)
        pool=self.word(EFFECT+0x38);self.put(pool+8,0 if c['selected']==0 else 1 if c['selected']<4 else 2)
        self.put(pool+0xc,(0,EFFECT,self.other,0x12345678,0)[c['selected']])
        # Dirty traversal uses actual recovered queue growth, record and use ring.
        use=self.alloc(32);self.put(use,0,0,self.record,0,0);self.put_half(use+0x14,c['group']);self.put_half(use+0x16,c['record']);self.put(use+0x18,use,use)
        self.put(self.parameter+0x34,use);self.put(self.record+0x30,0);self.put(EFFECT+0x110,1);queue=self.alloc(4);self.put(EFFECT+0x108,queue)
        self.put(CURSORS,self.blob+0x500,self.blob+0x2000,INPUT,c['root_count'],0x2400,0x2800,0x3000)
        for i in range(16):self.call('reference_ctor',receiver=REFS+i*8)
        # Patch resource-bearing values and destination indices using descriptor shape.
        def fill(at,input_index,output_offset,force_one=False,override=None):
            kind,shape,_,_,elements=[self.word(at+4*j)for j in range(5)];count=override if override is not None else 1 if force_one or not elements else elements
            if shape<4:
                rows,cols=self.word(at+20),self.word(at+24);return at+28,input_index+rows*cols*count,output_offset+(16 if shape<2 else 64)*count
            if shape==4:
                value=0 if c['empty_input']else TEXT if kind==4 else self.old if c['same_input']else self.new
                for j in range(count):self.put(INPUT+(input_index+j)*4,value);self.put(self.blob+output_offset+j*4,j%8)
                return at+20,input_index+count,output_offset+count*4
            if shape==5:
                first=at+24;members=self.word(at+20);end=first
                for _ in range(count):
                    end=first
                    for j in range(members):end,input_index,output_offset=fill(end,input_index,output_offset)
                return end,input_index,output_offset
            return at+20,input_index,output_offset
        _,self.input_count,_=fill(self.blob+0x500,0,0x2000,c['walk_indexed'] if self.test_kind in ('walk','references') else c['indexed'],c['root_count'] if self.test_kind=='typed' else None)
        self.input_count=max(1,self.input_count)
        self.put(self.parameter+0xc,self.input_count*4)
        self.requests=0;self.events=[]
    def run(self,kind,c):
        self.test_kind=kind;self.setup_values(c);p=self.parameter
        if kind in('shader','object','string'):
            arg=0 if c['empty_input']else TEXT if kind=='string'else self.old if c['same_input']else self.new
            result=self.call(kind,[arg,0,self.link if c['shared_values']else 0],EFFECT)
        elif kind=='copy':
            if c['family']==0 and c['alias']!=256:self.u.mem_write(self.blob+0x2000,hc.cp.pack(c['input_words']));source=self.blob+0x2000+c['alias']
            else:source=INPUT
            result=self.call(kind,[p,source],EFFECT)
        elif kind=='typed':result=self.call(kind,[CURSORS,CURSORS+4,CURSORS+8,c['root_count'],p],EFFECT)
        elif kind=='notify':result=self.call(kind,[p],EFFECT)
        elif kind=='reference_ctor':result=self.call(kind,receiver=REFS)
        elif kind in('walk','references'):
            if kind=='walk':self.put(CURSORS+8,0 if c['count_only']else REFS);args=[CURSORS,CURSORS+4,CURSORS+8,CURSORS+12,c['walk_indexed']];self.put(CURSORS+4,INPUT)
            else:args=[self.blob+0x500,INPUT,0 if c['count_only']else REFS,CURSORS+12,c['walk_indexed']]
            result=self.call(kind,args)
        elif kind=='record_value':result=self.call(kind,[self.encoded(p),INPUT,self.input_count*4+c['tail']],EFFECT)
        elif kind=='set_value':
            handle=(self.encoded(p),1,hc.QUERY,0)[c['handle_mode']];self.u.mem_write(hc.QUERY,b'p1\0')
            size=self.input_count*4-(1 if c['small_bytes']else 0)+(0 if c['small_bytes']else c['tail'])
            result=self.call(kind,[EFFECT,handle,INPUT,size])
        else:
            self.u.mem_write(self.blob+0x2000,bytes(self.u.mem_read(INPUT,self.input_count*4)))
            result=self.call(kind,[p,CURSORS+16,CURSORS+20,CURSORS+24],EFFECT)
        return self.snapshot(result)


def native_cases():
    for mode in range(24):
        kind,shape,elements,rows,columns=3,0,0,3,1
        operation,state,handle_mode,size_mode=0,0,0,0
        if mode==1:shape,elements,rows=1,3,4
        elif mode in(2,3,4,10,15,19):
            kind=1 if mode==2 else 2 if mode==3 else 3
            shape=3 if mode==15 else 2;elements=3 if mode==4 else 2;rows=4 if mode in(3,4)else 3;columns=4 if mode==4 else 2
        elif mode in(5,6,7,8,9,11,12,20):
            kind=5 if mode==8 else 15 if mode==9 else 9 if mode==12 else 4
            shape=4;elements=0 if mode==5 else 2
        elif mode in(13,16):shape,elements=5,2
        elif mode==14:elements=3
        elif mode==21:rows=columns=0;size_mode=3
        if mode in(10,11,12):state=3
        if mode in(14,15,16):operation=2
        elif mode==19:operation=3
        elif mode==20:operation=4
        elif mode==21:operation=1
        if mode==17:size_mode=1
        elif mode==22:size_mode=2
        if mode==18:handle_mode=1
        if shape==5:
            words=[0,5,0x50,0,elements,2,3,0,0x80,0,2,2,1,1,2,0x90,0,0,2,3]
        else:words=description(0,kind,shape,elements,rows,columns)
        def compact(at):
            kind,shape,_,_,n=words[at:at+5];n=n or 1
            if shape<4:return at+7,[0x3f000000+j if j%3 else 0 for j in range(n*words[at+5]*words[at+6])]
            if shape==4:return at+5,[0 if mode==7 else 0xfffffff2 if kind==4 else 0xfffffff1]*n
            out=[];end=at+6
            for j in range(n):
                end=at+6
                for _ in range(words[at+5]):end,values=compact(end);out+=values
            return end,out
        _,data=compact(0)
        config=[mode,operation,kind,shape,elements,rows,columns,state,handle_mode,size_mode]
        yield config,words,data


def native_graph(machine,config,description_words,input_words):
    mode,op,kind,shape,elements,rows,columns,state,handle_mode,size_mode=config
    c=next(cases(1));c.update(arena=bytes(md.core.SIZE),heap=b'\xa5'*md.core.HSIZE,fail_allocation=0)
    machine.active_kind='native';machine.prepare(c);machine.getters=Counter();machine.u.mem_write(EFFECT,b'\xa5'*0x130);machine.call('effect_ctor',receiver=EFFECT)
    machine.put(EFFECT+0x98,1,0,0,0,1,1);machine.put(EFFECT+0xfc,EFFECT+0xe0);machine.put(EFFECT+0x118,0);machine.put(EFFECT+0x3c,state)
    blob=machine.alloc(0x4000);machine.u.mem_write(blob,bytes(0x4000));machine.put(EFFECT+0x1c,0x4000,blob)
    machine.put(blob+0x50,3);machine.u.mem_write(blob+0x54,b'p0\0');machine.u.mem_write(blob+0x500,hc.cp.pack(description_words))
    parameters=machine.array(1,0x50,'parameter_ctor');table=machine.alloc(4);machine.put(table,parameters);machine.put(EFFECT+0x9c,parameters);machine.put(EFFECT+0xb0,table)
    machine.put(parameters,blob+0x2000,blob+0x500,0xffffffff,len(input_words)*4,3,0,0,0);machine.put(parameters+0x28,0,0x500,0x2000);machine.put(parameters+0x38,0,0,0)
    references=machine.array(2,0x18,'reference_construct');machine.put(EFFECT+0x14,2,references)
    old=machine.buffer(16);machine.put(references,4,old);machine.put(references+0x18,4)
    text=b'longer than the existing buffer\0'if mode==6 else b'hi\0';machine.u.mem_write(TEXT,text)
    patched=[old if word==0xfffffff1 else TEXT if word==0xfffffff2 else word for word in input_words]
    machine.u.mem_write(INPUT,hc.cp.pack(patched));machine.put(blob+0x2000,0,1)
    machine.put(CURSORS,blob+0x500,blob+0x2000,INPUT,0x2400,0x2800,0x3000)
    for i in range(4):machine.call('reference_ctor',receiver=REFS+i*8)
    size=len(input_words)*4;size=size-1 if size_mode==1 else 0xffffffff if size_mode==2 else 0 if size_mode==3 else size
    handle=hc.QUERY if handle_mode else (~parameters)&0xffffffff;machine.u.mem_write(hc.QUERY,b'p0\0')
    if op==0:
        result=machine.call('set_value',[EFFECT,handle,INPUT,size])
        if mode==23:result=machine.call('set_value',[EFFECT,handle,INPUT,size])
    elif op==1:result=machine.call('record_value',[(~parameters)&0xffffffff,INPUT,size],EFFECT)
    elif op==2:
        machine.u.mem_write(blob+0x2000,hc.cp.pack(patched));result=machine.call('relocate',[parameters,CURSORS+12,CURSORS+16,CURSORS+20],EFFECT)
    elif op==3:result=machine.call('typed',[CURSORS,CURSORS+4,CURSORS+8,2,parameters],EFFECT)
    else:result=machine.call('references',[blob+0x500,INPUT,REFS,CURSORS+24,0])
    words=[result,machine.word(EFFECT+0x118),machine.word(parameters)-blob,machine.word(parameters+4)-blob,machine.word(parameters+0x30),machine.digest(blob,0x4000)]
    words+=[machine.word(CURSORS+i*4)-(blob if i<2 else INPUT if i==2 else 0)for i in range(6)]
    def reference(kind,pointer):
        words.extend([kind,int(bool(pointer))])
        words.extend([machine.word(pointer+4),machine.word(pointer+8),machine.digest(machine.word(pointer+12),machine.word(pointer+8))]if pointer else[0,0,0])
    for i in range(2):
        p=references+i*0x18;reference(machine.word(p),machine.word(p+4));words.append(machine.word(p+0x14))
    value=machine.word(EFFECT+0xf8);words.append(int(bool(value)))
    while value:
        words.extend([machine.word(value),int(machine.word(value+4)==((~parameters)&0xffffffff)),machine.word(value+8),machine.word(value+0x10)])
        data=machine.word(value+0xc);size=machine.word(value+8)
        normalized=bytes(machine.u.mem_read(data,size))if data else b''
        if data:
            normalized=hc.cp.pack([0xfffffff1 if machine.word(data+i*4)==old else 0xfffffff2 if machine.word(data+i*4)==TEXT else machine.word(data+i*4)for i in range(size//4)])
        digest=2166136261
        for byte in normalized:digest=((digest^byte)*16777619)&0xffffffff
        words.append(digest)
        ref=machine.word(value+0x14)
        for i in range(machine.word(value+0x10)):reference(machine.word(ref+i*8),machine.word(ref+i*8+4))
        value=machine.word(value+0x18);words.append(int(bool(value)))
    count=machine.word(CURSORS+24)if op==4 else 0;words.append(count)
    for i in range(count):reference(machine.word(REFS+i*8),machine.word(REFS+i*8+4))
    if count:
        # Native caller owns the direct walk's reference array and releases it.
        for i in range(count):machine.call('reference_destroy',receiver=REFS+i*8)
    machine.call('destroy',receiver=EFFECT);assert not machine.live,('native value cleanup',mode,machine.live)
    return [word&0xffffffff for word in words]


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();returns=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect values mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect values mismatch',kind,i,diffs,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;returns[str((kind,x[2]))]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),returns=dict(returns),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    if args.write_native_fixtures:
        content=b'UG2FXVF1'+hc.cp.pack([24])
        for config,words,data in native_cases():content+=hc.cp.pack(config+[len(words),len(data)]+words+data)
        args.write_native_fixtures.write_bytes(content)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind]);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    a,b=Machine(image),Machine(image,linked,args.gcc);native=[]
    for config,words,data in native_cases():
        try:x=native_graph(a,config,words,data);y=native_graph(b,config,words,data)
        except(AssertionError,UcError)as e:raise AssertionError(('effect values mismatch','native-chain',config[0],str(e)))from e
        assert x==y and a.snapshot(tuple(x))==b.snapshot(tuple(y)),('effect values mismatch','native-chain',config[0],x,y)
        native.append(x)
    report['native_chains']=24
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXVD1';at=8
        for mode,expected in enumerate(native):
            row,count=struct.unpack_from('<2I',raw,at);at+=8;values=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert row==mode and values==expected,('native effect values mismatch',mode,expected,values)
        assert at==len(raw);report['native_replay']=24
    if args.mutation_checks:
        mutations={
            'shader-omit-effect-flag':('shader','value118|=0x40;','(void)0;'),
            'shader-omit-reference-flag':('shader','reference->value14|=1;','(void)0;'),
            'shader-omit-retain-new':('shader','if (object) UG2FxComAddRef(object);','(void)0;'),
            'object-omit-shared-publication':('object','other->value04=reference->value04;','(void)0;'),
            'object-ignore-self-assignment':('object','if (object!=reference->value04)','if (true)'),
            'string-reallocate-exact-capacity':('string','<size)', '<=size)'),
            'string-omit-tail-copy':('string','i<size;++i) output[i]', 'i<size-1;++i) output[i]'),
            'string-omit-local-release':('string','if (buffer) UG2FxComRelease(reinterpret_cast<unsigned int>(buffer));','(void)0;'),
            'typed-wrong-resource-descriptor-step':('typed','*type+=5;','*type+=7;'),
            'typed-wrong-matrix-stride':('typed','*output+=16;','*output+=4;'),
            'typed-omit-boolean-normalization':('typed','kind==1 ? (word!=0) : word','word'),
            'copy-ignore-element-handle':('copy','record->value08!=0xffffffff || !count','!count'),
            'copy-omit-boolean-normalization':('copy','kind==1 ? (word!=0) : word','word'),
            'copy-wrong-scalar-padding':('copy','output+=4-rows','output+=5-rows'),
            'notify-wrong-selected-receiver':('notify','reinterpret_cast<D3DXEffectObject *>(node->value00)->MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(node->value04))','MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(node->value04))'),
            'notify-ignore-pool-empty':('notify','if (!pool->value08) return 0;','(void)0;'),
            'reference-ctor-wrong-default':('reference_ctor','reference->value00=4;','reference->value00=5;'),
            'walk-ignore-indexed-flag':('walk','if (indexed || !elements)','if (!elements)'),
            'walk-wrong-numeric-width':('walk','description[6]*description[5]*elements','description[6]*description[5]'),
            'walk-retain-object-kind':('walk','kind<10 ? 5 : kind','kind'),
            'walk-wrong-reference-count':('walk','if (count) ++*count;','if (count) *count+=2;'),
            'wrapper-ignore-indexed-flag':('references','&references,count,indexed','&references,count,0'),
            'record-omit-tail-copy':('record_value','i<bytes;++i) copy[i]','i<(bytes&0xfffffffc);++i) copy[i]'),
            'record-wrong-reference-allocation':('record_value','UG2FxMalloc(count*8+4)','UG2FxMalloc(count*8+8)'),
            'record-omit-reference-retention':('record_value','result=D3DXEffectValueReferences(type,reinterpret_cast<unsigned int *>(copy),references,0,record->value08!=0xffffffff);','result=0;'),
            'set-ignore-short-input':('set_value','if (bytes!=0xffffffff && bytes<record->value0C)','if (false)'),
            'set-ignore-deferred-state':('set_value','if (effect->value3C==3)','if (false)'),
            'set-omit-notification':('set_value','if (result>=0) result=effect->NotifyValue(record);','(void)0;'),
            'relocate-wrong-scalar-stride':('relocate','*scalar+=count*16;','*scalar+=count*4;'),
            'relocate-omit-transpose':('relocate','D3DXMatrixTransposeScalar(reinterpret_cast<WheelMatrix *>(first),reinterpret_cast<const WheelMatrix *>(first));','(void)0;'),
            'relocate-wrong-struct-prefix':('relocate','parameter->value00=value20+parameter->value30;','parameter->value00=value20+parameter->value30+4;'),
        }
        functions=dict(shader='WriteShaderResource',object='WriteObjectResource',string='WriteStringResource',typed='CopyTypedValue',copy='CopyValue',notify='NotifyValue',
            reference_ctor='D3DXConstructReference08',walk='D3DXEffectWalkValueReferences',references='D3DXEffectValueReferences',record_value='RecordValue',set_value='D3DXEffectSetValue',relocate='RelocateParameter')
        starts={k:SOURCE.read_text().index(('void __fastcall 'if k=='reference_ctor'else'int __stdcall 'if k in('walk','references','set_value')else'int D3DXEffectObject::')+v+'(')for k,v in functions.items()}
        source=SOURCE.read_text();rejected=[]
        for name,(kind,before,after)in mutations.items():
            if args.kind!='all'and args.kind!=kind:continue
            start=starts[kind];end=min([p for p in starts.values()if p>start]+[len(source)]);body=source[start:end]
            assert before in body,(name,before);mutant=source[:start]+body.replace(before,after,1)+source[end:]
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(mutant)
            command=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect values mismatch'in run.stdout,(name,run.stdout);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
