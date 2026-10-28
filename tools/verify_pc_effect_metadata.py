#!/usr/bin/env python3
"""Compare UG2 type validation, metadata construction, size walks and use linking.

The linked chain includes allocation/deletion/temporary cleanup and type-table
reads. Only allocation/free and the generic iterator test callback are controlled.
Retail scalar descriptors can be checked independently of the full deserializer.
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
from pc_coff import link_objects
from pc_native_elf import link_native
import verify_pc_effect_input as core

ROOT,EXE=core.ROOT,core.EXE;LIB=ROOT/'src/Speed/Indep/Libs/D3DX'
SOURCES=[LIB/'EffectMetadata.cpp',LIB/'EffectTypeValidation.cpp']
BUILD=ROOT/'build/analysis/effect-metadata';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-metadata.json'
ARENA,HEAP,STOP=core.ARENA,core.HEAP,core.STOP
EFFECT,OBJECT,TYPE,DATA,WIRE=ARENA+0x1000,ARENA+0x1400,ARENA+0x3000,ARENA+0x3010,ARENA+0x4000
CALLBACK=STOP+0x7100
SPANS=dict(array=(0x401650,48),use_construct=(0x65b680,20),base_construct=(0x65b694,20),
    group_construct=(0x65b6c9,106),reference_construct=(0x65b78d,32),record_construct=(0x663ec5,80),
    pass_construct=(0x663f4e,30),technique_construct=(0x663f6c,99),resize=(0x664fb1,128),
    string=(0x65bc5e,92),name=(0x65bcba,100),padded=(0x65bf18,283),value_size=(0x65c033,240),
    link=(0x65bd1e,239),validate=(0x66548b,1418))
MEMBERS=dict(resize=('D3DXEffectGroup64','ResizeRecords'),string=('D3DXEffectObject','ValidateString'),
    name=('D3DXEffectObject','ValidateName'),link=('D3DXEffectObject','LinkParameter'),validate=('D3DXEffectObject','ValidateType'),
    effect_ctor=('D3DXEffectObject','Construct'),parameter_ctor=('D3DXEffectParameter','Construct'))
FREE=dict(array='D3DXEffectConstructArray',use_construct='D3DXConstructUse',base_construct='D3DXConstructBase',
    group_construct='D3DXConstructGroup64',reference_construct='D3DXConstructReference18',record_construct='D3DXConstructRecord64',
    pass_construct='D3DXConstructRecord38',technique_construct='D3DXConstructRecordB4',padded='D3DXEffectPaddedSize',value_size='D3DXEffectValueSize')
STDCALL=dict(array=16,padded=12,value_size=12)
CDECL=set()
VOID=set(FREE)-{'padded','value_size'};core.MEMBERS.update(MEMBERS)


def build(replace,gcc):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[replace.get(p.name,p)for p in SOURCES]+[LIB/'EffectObject.cpp',LIB/'EffectPool.cpp',LIB/'EffectDestruction.cpp',core.SOURCE,core.BRIDGE,
        ROOT/'src/Platform/Native/NativeFxPoolDispatch.cpp',ROOT/'src/Platform/Native/NativeFxEffectDispatch.cpp',
        ROOT/'src/Platform/Native/NativeFxDestroyDispatch.cpp',ROOT/'src/Platform/Native/NativeFxComDispatch.cpp']
    bindings=dict(core.GLOBALS,**core.APIS,ug2FxPoolIid7B7410=0x7b7410,ug2FxTypeTable7B582C=0x7b582c,
        UG2FxMalloc=0x575620,UG2FxFree=0x575630,abort=STOP+0x900)
    if gcc:
        linked=link_native(sources,BUILD,ROOT,bindings,FREE['array'],text_address=0x5000000,include_dirs=[LIB])
        prefixes=dict(FREE,**{k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()})
    else:
        objects=[]
        for i,p in enumerate(sources):
            obj=BUILD/('metadata-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/I'+str(LIB.relative_to(ROOT)),'/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()}
        for name,n,_ in core.API_SPECS:bindings['_UG2Fx'+name+'@'+str(n*4)]=core.APIS['UG2Fx'+name]
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:('_'+name if k in CDECL else'_'+name+'@'+str(STDCALL[k])if k in STDCALL else'@'+name+'@4')for k,name in FREE.items()}
        prefixes.update({k:'?'+m+'@'+c+'@@'for k,(c,m)in MEMBERS.items()})
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for name,a in linked['exports'].items()if name==p or(k in MEMBERS and name.startswith(p))]
        assert len(hits)==1,(k,p,hits);linked['entries'][k]=hits[0]
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries=dict({k:a for k,(a,n)in SPANS.items()},effect_ctor=0x66509e,parameter_ctor=0x664ecf)
        self.u.hook_add(UC_HOOK_CODE,self.callback,begin=CALLBACK,end=CALLBACK)
    def call(self,kind,args=(),receiver=0):
        u=self.u;member=kind in MEMBERS;params=([receiver]+list(args))if self.gcc and member else list(args)
        sp=core.STACK+0xf000;self.put(sp,STOP,*params)
        regs=[xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP];seeds=[0x11223344,0x22334455,0x33445566,0x44556677]
        for r,v in zip(regs,seeds):u.reg_write(r,v)
        for r,v in [(xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,0x202),
            (xc.UC_X86_REG_FPCW,0x27f),(xc.UC_X86_REG_FPTAG,0xffff)]:u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=getattr(self,'instruction_limit',200000))
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        pop=0 if kind in CDECL or self.gcc and member else len(args)*4
        assert u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop,('stack',kind)
        assert [u.reg_read(r)for r in regs]==seeds,('registers',kind)
        assert u.reg_read(xc.UC_X86_REG_FPCW)==0x27f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
        assert self.word(0)==0x12345678
        return None if kind in VOID or kind=='parameter_ctor'else u.reg_read(xc.UC_X86_REG_EAX)
    def callback(self,u,address,size,user):
        self.events.append(('construct',u.reg_read(xc.UC_X86_REG_ECX),self.state()));self.ret()
    def half(self,a):return struct.unpack('<H',self.u.mem_read(a,2))[0]
    def put_half(self,a,v):self.u.mem_write(a,struct.pack('<H',v&0xffff))
    def state(self):return bytes(self.u.mem_read(ARENA,core.SIZE))+bytes(self.u.mem_read(HEAP,self.allocated))
    def alloc(self,n):
        a=HEAP+self.allocated;self.allocated+=max(16,(n+15)&~15)
        assert self.allocated<core.HSIZE;self.live[a]=n;return a
    def array(self,count,size,constructor):
        a=self.alloc(4+count*size);self.put(a,count)
        for i in range(count):self.call(constructor,receiver=a+4+i*size)
        return a+4
    def graph(self,c):
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
        self.put(EFFECT+0x20,WIRE);self.put(EFFECT+0x1c,c['span'])
        groups=self.array(8,0x64,'group_construct');self.put(EFFECT+0x2c,groups)
        for i in range(8):self.put(groups+i*0x64+0xc,c['group_size'])
        references=self.array(12,0x18,'reference_construct');self.put(EFFECT+0x18,references);self.put(EFFECT+0x14,12)
        if c['used_reference']:self.put(references+0x18,4)
        parameters=self.array(2,0x50,'parameter_ctor');table=self.alloc(8);self.put(table,parameters,parameters+0x50)
        self.put(EFFECT+0xb0,table)
        for i in range(2):
            self.put(parameters+i*0x50+0x14,i);self.put(parameters+i*0x50+0x28,c['parameter_flags'])
            self.put_half(parameters+i*0x50+0x44,0);self.put_half(parameters+i*0x50+0x46,c['parameter_groups'])
        self.put_half(EFFECT+0x30,c['group_count']);self.put(EFFECT+0x34,c['group_limit'])
        return groups,parameters
    def description(self,offset,kind,shape,count,rows=1,columns=1):
        self.put(WIRE+offset,kind,shape,0,0x20,count)
        if shape<4:self.put(WIRE+offset+0x14,rows,columns);return offset+0x1c
        if shape==5:self.put(WIRE+offset+0x14,rows);return offset+0x18
        return offset+0x14
    def wire(self,c):
        self.u.mem_write(WIRE,bytes(0x6000));self.put(WIRE,2);self.u.mem_write(WIRE+4,b'a\0');self.put(WIRE+0x20,0)
        at=0x200;family=c['family'];count=c['elements'];kind=c['kind'];shape=c['shape']
        if family==0:self.description(at,kind,shape,count,c['rows'],c['columns'])
        elif family==1:self.description(at,kind,shape,count,c['rows'],c['columns'])
        elif family==2:
            at=self.description(at,0,5,count,c['members'])
            for j in range(c['members']):at=self.description(at,1+j%3,j%4,c['child_elements'],c['rows'],c['columns'])
        elif family==3:self.description(at,4+c['kind']%6,4,count)
        elif family==4:self.description(at,10+c['kind']%5,4,count)
        else:self.description(at,15+c['kind']%2,4,count)
        if family==4:
            pos=WIRE+0x800
            for i in range(count or 1):
                self.put(pos,c['records']);pos+=4
                for j in range(c['records']):self.put(pos,c['codes'][j%len(c['codes'])],c['index'],0x1000,0x1100);pos+=16
            self.put(WIRE+0x1100,c['index'])
        else:
            for j in range(16):self.put(WIRE+0x800+j*4,(c['index']+j)%13 if c['distinct']else c['index'])
        if c['bad_name']:self.u.mem_write(WIRE+4,b'.\0')
        self.put(TYPE,WIRE+0x200);self.put(DATA,WIRE+0x800)
    def run(self,kind,c,retail=None):
        self.prepare(c)
        if kind in('link','validate'):groups,parameters=self.graph(c)
        else:
            self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
            self.put(EFFECT+0x20,WIRE);self.put(EFFECT+0x1c,c['span'])
        self.wire(c)
        if retail:
            recipe,ordinal,blob,offset,data=retail
            assert len(blob)<=0x6000;self.u.mem_write(WIRE,blob);self.put(EFFECT+0x1c,len(blob));self.put(TYPE,WIRE+offset);self.put(DATA,WIRE+data)
        if kind.endswith('_construct'):result=self.call(kind,receiver=OBJECT)
        elif kind=='array':result=self.call(kind,[c['first'],c['stride'],c['array_count'],CALLBACK])
        elif kind in('string','name'):
            self.put(WIRE,c['string_length']);self.u.mem_write(WIRE+4,c['string'])
            offset=0
            if c['wrap']:
                self.put(EFFECT+0x20,0xfffffff0);self.put(EFFECT+0x1c,(WIRE+c['span']-0xfffffff0)&0xffffffff);offset=(WIRE-0xfffffff0)&0xffffffff
            result=self.call(kind,[offset],EFFECT)
        elif kind in('padded','value_size'):
            self.put(TYPE,0x200);self.put(DATA,c['data_offset'])
            result=self.call(kind,[WIRE,TYPE,TYPE if c['alias']and c['family']!=2 else DATA])
        elif kind=='resize':
            self.call('group_construct',receiver=OBJECT)
            old=self.array(c['old_records'],0x64,'record_construct')if c['old_records']else 0
            self.put(OBJECT+4,old);self.put_half(OBJECT+0x38,c['old_records']);self.put_half(OBJECT+0x36,c['old_active'])
            if c['requested']&0xffff>=0x1000:self.case=dict(c,fail_allocation=1)
            result=self.call(kind,[c['requested']],OBJECT)
        elif kind=='link':
            records=self.array(1,0x64,'record_construct');self.put(groups+4,records)
            uses=self.array(3,0x20,'use_construct');self.put(records+0x34,uses);self.put(records+0x58,c['tag'])
            self.put(parameters+0x14,c['owner']);owner=parameters+c['owner']*0x50
            if c['existing']:
                self.put(uses+0x18,uses+0x20,uses+0x20);self.put(uses+0x20+0x18,uses,uses);self.put(owner+0x34,uses)
            self.put(EFFECT+0x118,c['flags']);self.put(EFFECT+0xa0,2);self.put(EFFECT+0xac,2)
            if c['grids']:
                for off in(0xb4,0xc0):
                    a=self.alloc(16);self.put(EFFECT+off,a)
                    for j in range(4):self.put(a+j*4,self.alloc(1+j)if j&1 else 0)
            result=self.call(kind,[0,0,2,parameters,c['mark']],EFFECT)
        else:result=self.call(kind,[TYPE,DATA,0],EFFECT)
        return self.snapshot(result)


def cases(n):
    rng=random.Random(0x66548b)
    for i,c in enumerate(core.cases(n)):
        family=i%6
        c.update(effect=rng.randbytes(0x130),span=(0,3,4,19,20,27,28,0x220,0x900,0x2000)[(i//6)%10],family=family,
            kind=(1,2,3,0,4,9,10,14,15,16,0xffffffff,0x80000000)[(i//60)%12],shape=(0,1,2,3,4,5,6,0xffffffff)[(i//12)%8],
            bad_name=i%7==0,elements=(0,1,2,3)[(i//3)%4],rows=(0,1,2,4)[(i//2)%4],columns=(0,1,3,4,16)[i%5],members=(0,1,2,3)[(i//5)%4],child_elements=(0,1,2)[(i//4)%3],
            group_size=rng.getrandbits(32),parameter_flags=rng.getrandbits(32),parameter_groups=1 if i%17==0 else 0,
            group_count=i%4,group_limit=(0,1,3,8)[(i//7)%4],used_reference=i%13==0,
            index=(0,1,2,3,11,12)[(i//11)%6],distinct=bool(i&1),records=(0,1,2,3)[(i//9)%4],
            codes=((164,165,172),(0,146,147),(177,182,183),(165,165,164))[(i//15)%4],
            string=(b'abc\0',b'a.b\0',b'a@b\0',b'a[b\0',b'a]b\0',b'a\0.\0',b'\xff\x80\0',b'\0')[(i//8)%8],
            string_length=(0,1,2,3,4,0xfffffff0)[(i//16)%6],wrap=bool((i//32)&1),alias=bool(i&1),data_offset=rng.getrandbits(32),
            old_records=(0,1,2,4)[(i//5)%4],old_active=i%3,requested=(0,1,2,3,7,0x10002,0xffff)[(i//2)%7],
            existing=bool((i//2)&1),owner=i%2,tag=(0x0a000000,0x0b000000,0x03000001)[i%3],grids=bool((i//3)&1),flags=0x800 if i%3==0 else 0,
            mark=rng.getrandbits(32),first=(ARENA+0x1800,0xfffffff0)[i%2],stride=(0,1,4,8,0xfffffff0)[i%5],
            array_count=(0,1,2,3,0x80000001,0xffffffff)[(i//2)%6])
        # Most fixtures reach the parser; truncation profiles remain separate.
        if i%3:c['span']=0x2000;c['string_length']=len(c['string'])
        if i%37==0:
            c.update(family=0,kind=1,shape=0,elements=1,rows=1,columns=1,span=0x220,bad_name=False)
        elif i%41==0:
            c.update(family=5,kind=1,elements=0,index=1,group_count=0,group_limit=8,parameter_groups=0,bad_name=False,span=0x2000,distinct=True)
        # Failure positions vary independently of the six descriptor families.
        c['fail_allocation']=0 if(i//6)%3 else 1+(i//18)%8
        yield c


def retail_descriptors(pe):
    fixtures=[]
    names={}
    for recipe in range(23):
        address=pe.get_dword_at_rva(0x7ff7d0-0x400000+recipe*0x8c+0x84)
        names[pe.get_string_at_rva(address-0x400000).decode('ascii')]=recipe
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id!=10:continue
        for entry in t.directory.entries:
            recipe=names[str(entry.name)]
            d=entry.directory.entries[0].data.struct;payload=pe.get_data(d.OffsetToData,d.Size)
            signature,size=struct.unpack_from('<2I',payload);assert signature==0xfeff0901
            blob=payload[8:8+size];cursor=2+(size+3)//4
            parameters,techniques,groups,resources=struct.unpack_from('<4I',payload,cursor*4);cursor+=4
            for ordinal in range(parameters):
                offset,data,flags,annotations=struct.unpack_from('<4I',payload,cursor*4);cursor+=4+annotations*2
                kind,shape=struct.unpack_from('<2I',blob,offset)
                if shape<4:fixtures.append((recipe,ordinal,blob,offset,data))
    return fixtures


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();profiles=Counter();allocations=Counter()
    work=[(k,c,None)for c in cases(n)for k in kinds]
    for fixture in retail:work.append(('validate',next(cases(1)),fixture))
    for i,(kind,c,r)in enumerate(work):
        try:x=a.run(kind,c,r);y=b.run(kind,c,r)
        except(AssertionError,UcError)as e:raise AssertionError(('effect metadata mismatch',kind,i,str(e)))from e
        if x!=y:
            offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
            event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
            raise AssertionError(('effect metadata mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
        counts['retail_validate'if r else kind]+=1;events.update(e[0]for e in x[3]);returns['void'if x[2]is None else hex(x[2])]+=1
        if kind=='validate'and not r:
            profiles[str((c['family'],hex(x[2])))]+=1;allocations[str(c['family'])]+=sum(e[0]=='malloc'for e in x[3])
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),validation_profiles=dict(profiles),validation_allocations=dict(allocations))


def native_replay(path,pe,image,linked,gcc):
    raw=path.read_bytes();assert raw[:8]==b'UG2FXMD1'and(len(raw)-8)%32==0
    fixtures={(r[0],r[1]):r for r in retail_descriptors(pe)}
    records=list(struct.iter_unpack('<8I',raw[8:]));assert len(records)==len(fixtures)
    a,b=Machine(image),Machine(image,linked,gcc);seen=set();c=next(cases(1))
    for recipe,ordinal,type_before,data_before,result,type_after,data_after,checksum in records:
        key=(recipe,ordinal);assert key in fixtures and key not in seen;seen.add(key)
        r=fixtures[key];assert(type_before,data_before)==r[3:]
        expected=2166136261
        for byte in r[2]:expected=((expected^byte)*16777619)&0xffffffff
        assert checksum==expected,('native input hash',key)
        x=a.run('validate',c,r);y=b.run('validate',c,r);assert x==y,('native replay source mismatch',key)
        actual=(x[2],(a.word(TYPE)-WIRE)&0xffffffff,(a.word(DATA)-WIRE)&0xffffffff)
        assert actual==(result,type_after,data_after),('native cursor mismatch',key,actual,(result,type_after,data_after))
    return dict(descriptors=len(records),recipes=len({r[0]for r in records}),all_cursor_results_match=True,input_blob_hashes_match=True)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--source-name');p.add_argument('--mutation-checks',action='store_true')
    p.add_argument('--native-metadata',type=Path,help='Replay UG2FXMD1 native scalar validation capture against original x86')
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    replace={args.source_name:args.replace}if args.replace else{}
    retail=retail_descriptors(pe)if args.kind=='all'else[]
    linked=build(replace,args.gcc)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind],retail)
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['only allocator/free and standalone generic constructor callback controlled; all validation and metadata/lifetime helpers execute',
        'finite bounded valid type trees and mapped truncated records; arbitrary malformed addresses, cyclic descriptors and giant loops not established',
        'retail scalar descriptors validate strings, dimensions and cursor advancement; full effect deserialization and shaders remain open',
        'constructor register return values are not an interface contract; only stores and normal constructor completion compared'])
    if args.native_metadata:report['native_replay']=native_replay(args.native_metadata,pe,image,linked,args.gcc)
    if args.mutation_checks:
        mutations={
            'reverse-construction':('array','EffectMetadata.cpp','address+=size;','address-=size;'),
            'unsigned-construction-guard':('array','EffectMetadata.cpp','static_cast<int>(remaining)>=0','count!=0'),
            'zero-use-payload':('use_construct','EffectMetadata.cpp','p->value10=0; p->value04=0;','p->value00=0; p->value10=0; p->value04=0;'),
            'wrong-use-self-link':('use_construct','EffectMetadata.cpp','p->previous18=p;','p->previous18=0;'),
            'clear-group-flags':('group_construct','EffectMetadata.cpp','p->value08&=0xffffffe0;','p->value08=0;'),
            'erase-group-unknown':('group_construct','EffectMetadata.cpp','p->value3E=0xffff;','p->value3E=0xffff; p->value40=0;'),
            'wrong-reference-flags':('reference_construct','EffectMetadata.cpp','p->value14&0xfffffffe','p->value14&0xfffffffa'),
            'wrong-record-mask':('record_construct','EffectMetadata.cpp','p->value30&0xffffffe1','p->value30&0xfffffff1'),
            'wide-record-word-store':('record_construct','EffectMetadata.cpp','p->value50=0;','p->value50=0; p->value52=0;'),
            'wrong-pass-type':('pass_construct','EffectMetadata.cpp','p->value00.value10=1;','p->value00.value10=0;'),
            'erase-technique-gap':('technique_construct','EffectMetadata.cpp','p->value88=0;','p->value88=0; p->value28=0;'),
            'resize-on-equality':('resize','EffectMetadata.cpp','if (value38<count)','if (value38<=count)'),
            'publish-active-on-oom':('resize','EffectMetadata.cpp','if (!records) return','if (!records) { value36=count; } if (!records) return'),
            'drop-old-array-free':('resize','EffectMetadata.cpp','array04->DeleteArray(3);','array04->DeleteArray(2);'),
            'retain-active-count':('resize','EffectMetadata.cpp','value36=count;','(void)0;'),
            'reject-empty-string':('string','EffectMetadata.cpp','if (length) {','if (!length) return static_cast<int>(0x80004005); if (length) {'),
            'strict-string-end':('string','EffectMetadata.cpp','value1C+value20<value20+4+offset','value1C+value20<=value20+4+offset'),
            'ignore-string-terminator':('string','EffectMetadata.cpp','if (*reinterpret_cast<char *>(value20+length-1+offset+4))','if (false)'),
            'accept-empty-name':('name','EffectMetadata.cpp','if (*p<2)','if (*p<1)'),
            'allow-name-at':('name','EffectMetadata.cpp'," || *s=='@'",''),
            'wrong-padded-vector-stride':('padded','EffectMetadata.cpp','shape<2 ? 4 : 0x10','shape<2 ? 1 : 0x10'),
            'pad-matrix-dimensions':('value_size','EffectMetadata.cpp','unsigned int rows=p[5],columns=p[6];','unsigned int rows=4,columns=4;'),
            'count-object-as-storage':('value_size','EffectMetadata.cpp','? 4 : 0;\n        *type=offset+0x14','? 4 : 4;\n        *type=offset+0x14'),
            'omit-struct-size-scaling':('padded','EffectMetadata.cpp','return sum*count;','return sum;'),
            'omit-link-index':('link','EffectMetadata.cpp','link->value16=static_cast<unsigned short>(record);','link->value16=0xffff;'),
            'omit-ring-tail-write':('link','EffectMetadata.cpp','->previous18=link;','->previous18=link->previous18;'),
            'dirty-shader-record':('link','EffectMetadata.cpp','item->value58!=0x0a000000 && item->value58!=0x0b000000','true'),
            'ignore-setup-gate':('link','EffectMetadata.cpp','!(reinterpret_cast<unsigned char *>(&value118)[1]&8)','true'),
            'skip-name-validation':('validate','EffectTypeValidation.cpp','int result=ValidateName(description[2]);','int result=0;'),
            'wrong-numeric-shape-end':('validate','EffectTypeValidation.cpp','end<start+bytes','end<*data+bytes'),
            'skip-struct-array-element':('validate','EffectTypeValidation.cpp','i<count;++i','i+1<count;++i'),
            'accept-used-reference':('validate','EffectTypeValidation.cpp','if (reference->value00!=0x7fffffff)','if (false)'),
            'accept-reference-zero':('validate','EffectTypeValidation.cpp','index>=value14 || !index','index>=value14'),
            'skip-reference-used-flag':('validate','EffectTypeValidation.cpp','[index].value14|=1;','[index].value14|=0;'),
            'omit-parameter-group-count':('validate','EffectTypeValidation.cpp','++p->value46;','(void)0;'),
            'wrong-lazy-shader-tag':('validate','EffectTypeValidation.cpp','group->array04->value58=0x0a000000;','group->array04->value58=0x0b000000;'),
            'short-parameter-copy':('validate','EffectTypeValidation.cpp','j<10;++j','j<6;++j'),
            'omit-lazy-use-allocation':('validate','EffectTypeValidation.cpp','UG2FxMalloc(0x24)','UG2FxMalloc(0x20)'),
            'wrong-record-code-limit':('validate','EffectTypeValidation.cpp','code>=0xb6','code>=0xa5'),
            'round-record-storage':('validate','EffectTypeValidation.cpp','group->value0C=offset;','offset=(offset+3)&0xfffffffc; group->value0C=offset;'),
        }
        selected=set(SPANS)if args.kind=='all'else{args.kind};rejected=[]
        for name,(kind,filename,before,after)in mutations.items():
            if kind not in selected:continue
            body=(LIB/filename).read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--source-name',filename,'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect metadata mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
