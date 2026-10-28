#!/usr/bin/env python3
"""Compare UG2 parameter handles, dynamic annotations and symbol readers.

Recovered resolver/size/metadata/buffer/compiled-record bodies execute. Allocation
and CRT thread/locale services are controlled, with separate native C-locale QA.
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
import verify_pc_effect_compiled_payload as cp

md=cp.md;ROOT,EXE,LIB=cp.ROOT,cp.EXE,cp.LIB
SOURCE=LIB/'EffectHandles.cpp';BRIDGE=ROOT/'src/Platform/Native/NativeFxCrtLocale.cpp'
BUILD=ROOT/'build/analysis/effect-handles';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-handles.json'
ARENA,HEAP,STOP=cp.ARENA,cp.HEAP,cp.STOP;EFFECT,POSITION=cp.EFFECT,cp.POSITION
QUERY,NAME,GENERIC=ARENA+0x3400,ARENA+0x3500,ARENA+0x1400
THREAD,LOCALE_A,LOCALE_B,CTYPE=ARENA+0x6800,ARENA+0x6900,ARENA+0x6980,ARENA+0x7000
SPANS=dict(resolve=(0x66bbee,401),element=(0x65c123,465),member=(0x66a83c,464),annotation=(0x665a15,1467),technique=(0x65c61f,207),
    pass_find=(0x65c5de,65),members=(0x661c3e,474),symbol=(0x66c37e,711),symbol_record=(0x66c645,688),decimal_body=(0x75d5d3,136),decimal=(0x75d65b,5))
MEMBERS={k:('D3DXEffectObject',v)for k,v in dict(resolve='ResolveParameter',element='ResolveElement',member='ResolveMember',annotation='ResolveAnnotation',
    technique='FindTechnique',pass_find='FindPass',members='BuildMembers',symbol='ReadSymbol',symbol_record='ReadSymbolRecord').items()}
md.SOURCES=md.SOURCES+[SOURCE,BRIDGE];md.BUILD=BUILD;md.MEMBERS.update(MEMBERS)
md.FREE.update(decimal_body='D3DXEffectAtolBody',decimal='D3DXEffectAtol');md.CDECL.update(('decimal_body','decimal'))
md.core.GLOBALS['ug2FxCrtLocale819D24']=0x819d24


def native_cases():
    for mode in range(32):
        c=next(cases(1));c.update(query=b'p0',name=b'n.inner',flags=0,create=0,struct_elements=0,scalar_elements=3,member_elements=3,scalar_shape=0,
            resource_type=5,resource_elements=0,rows=1,columns=1,parameter_flags=1,shared=False,scratch_kind=1,scratch_shape=0,scratch_elements=0,scratch_rows=2,scratch_columns=3,
            upper_word=0xa5a5,group=0,record=0,storage_offset=7,record_flags=0x1234567b,record_tag=0,code=164,stale_locale=False,locale_width=1,update_b=False,depth=10,
            fail_allocation=0,cursor_callback=False,effect=b'\xa5'*0x130,terminated=True,stream_start=0,limit_mode=0,null_buffer=False,
            version=0x46580200,instructions=0,omit_instructions=False,omit_literals=False,omit_table=False,negative_token=False,legacy_def=False,instruction_skip=False,short_comment=False,
            constants=2,table_words=192,bank=2,same_bank=False,registers=1,start_register=0,vary_registers=False,register_starts=None,bad_name=False,missing_name=False,default_offset=0,tail=0)
        op=0;owner=0;index=2;count=2
        if mode==0:c['query']=b'p0.n.inner'
        elif mode==1:c['query']=b'p1[4294967297]'
        elif mode==2:c['query']=b'p1[2]'
        elif mode==3:c.update(query=b'p0[1].n.inner',struct_elements=3)
        elif mode==4:c['query']=b'p0@a'
        elif mode==5:c.update(query=b'p0@new',create=1)
        elif mode==6:c.update(query=b'p0@str',create=1,scratch_kind=4,scratch_shape=4)
        elif mode==7:c.update(query=b'p0@bad.name',create=1)
        elif mode==8:op=3;owner=1;c.update(name=b'new',flags=2)
        elif mode==9:op=3;owner=2;c.update(name=b'new',flags=2,scratch_kind=4,scratch_shape=4)
        elif mode==10:op=2;c['name']=b'n.inner'
        elif mode==11:op=1;owner=3
        elif mode==12:op=4;c['query']=b'T1'
        elif mode==13:op=6;c['query']=b'p2'
        elif mode==14:op=6;c.update(query=b'p2',resource_type=16,code=146)
        elif mode==15:op=7;c['query']=b'p1'
        elif mode==16:op=6;c.update(query=b'p2',resource_type=10,code=178)
        elif mode==17:op=6;c.update(query=b'p2',resource_type=15,code=147)
        elif mode==18:op=7;c.update(query=b'p1',compiled_payload=False)
        elif mode==19:op=3;c.update(name=b'new',flags=2,scratch_kind=5)
        elif mode==20:op=3;owner=3;c.update(name=b'new',flags=2,scratch_kind=2,scratch_shape=3,scratch_elements=2,scratch_rows=4,scratch_columns=4)
        elif mode==21:op=3;c.update(name=b'new',flags=2,scratch_kind=4,scratch_shape=4,scratch_elements=1)
        elif mode==22:c['query']=b'p1[-1]'
        elif mode==23:c['query']=b'p1[3]'
        elif mode==24:c.update(query=b'p0.m@new',create=1)
        elif mode==25:op=8;c['query']=b'\t -4294967297tail'
        elif mode==26:op=8;c['query']=b'18446744073709551616'
        elif mode==27:op=8;c['query']=b'\x80'
        elif mode==28:op=5;owner=2
        elif mode==29:op=9;count=1
        elif mode==30:op=2;c['name']=b'm[1]'
        elif mode==31:op=3;c['name']=b'arr[1]'
        if mode!=18:c['compiled_payload']=True
        c.update(operation=op,native_owner=owner,native_index=index,native_count=count)
        yield mode,c


class Machine(cp.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.image=image
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for original,name,kind in((0x761e3c,'UG2FxCrtGetThreadData','thread'),(0x75e3d3,'UG2FxCrtUpdateLocale','locale'),(0x766361,'UG2FxCrtClassify','classify')):
            address=linked['exports'][name if gcc else'_'+name]if linked else original
            self.u.hook_add(UC_HOOK_CODE,self.crt,user_data=kind,begin=address,end=address)
    def state(self):return super().state()+bytes(self.u.mem_read(0x819d24,4))
    def crt(self,u,address,size,kind):
        if kind=='classify':
            args=self.args(3);locale,character,mask=args;result=self.u.mem_read(CTYPE+character*2,1)[0]&mask
        else:args=();result=THREAD if kind=='thread'else LOCALE_B if self.case['update_b']else LOCALE_A
        self.events.append((kind,args,self.state()));self.ret(result)
    def getter(self,u,address,size,kind):
        super().getter(u,address,size,kind)
        if self.active_kind in('symbol','symbol_record')and self.case['cursor_callback']and kind==self.case['callback_kind']and self.getters[kind]==self.case['callback_ordinal']:
            self.put(POSITION,(self.word(POSITION)+1)&0xffffffff)
    def setup(self,c,native=False):
        self.prepare(c);self.getters=Counter();self.position=POSITION
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
        self.put(EFFECT+0x98,3);self.put(EFFECT+0xa0,2);self.put(EFFECT+0xa8,7,7);self.put(EFFECT+0x118,c['flags'])
        self.blob=self.alloc(0x4000);self.shared_blob=self.blob if native else self.alloc(0x4000)
        self.u.mem_write(self.blob,bytes(0x4000));self.u.mem_write(self.shared_blob,bytes(0x4000));self.put(EFFECT+0x20,self.blob);self.put(EFFECT+0x1c,0x4000)
        self.put(0x819d24,LOCALE_A);self.put(THREAD+0x64,LOCALE_B if c['stale_locale']else LOCALE_A)
        for locale in(LOCALE_A,LOCALE_B):self.put(locale+0x28,c['locale_width']);self.put(locale+0x48,CTYPE)
        self.u.mem_write(CTYPE,self.image[0x7d7e4a-0x400000:0x7d7e4a-0x400000+512])
        if native:self.put(0x819d24,0x819cd0)
        names={0x40:b'p0',0x50:b'p1',0x60:b'p2',0x80:b'm',0x90:b'n',0xa0:b'inner',0xb0:b'a',0xc0:b'arr',0xd0:b'ta',0xe0:b'pa',0x100:b'T0',0x110:b'T1'}
        for base in(self.blob,self.shared_blob):
            for offset,name in names.items():self.put(base+offset,len(name)+1);self.u.mem_write(base+offset+4,name+b'\0')
            self.put(base+0x400,0,5,0x40,0,c['struct_elements'],2)
            self.put(base+0x418,1,c['scalar_shape'],0x80,0,c['member_elements'],c['rows'],c['columns'])
            self.put(base+0x434,0,5,0x90,0,0,1);self.put(base+0x44c,3,0,0xa0,0,1,1,1)
            self.put(base+0x500,1,c['scalar_shape'],0x50,0,c['scalar_elements'],c['rows'],c['columns'])
            self.put(base+0x600,c['resource_type'],4,0x60,0,c['resource_elements'])
            self.put(base+0x700,3,0,0xb0,0,0,1,1);self.put(base+0x740,2,0,0xc0,0,3,1,1)
            self.put(base+0x780,1,0,0xd0,0,0,1,1);self.put(base+0x7c0,1,0,0xe0,0,0,1,1)
            self.put(base+0x2800,1)
        if not native:
            pool=self.alloc(0x20);self.put(pool+0x10,self.shared_blob);self.put(EFFECT+0x38,pool)
        self.parameters=self.array(3,0x50,'parameter_ctor');self.put(EFFECT+0x9c,self.parameters)
        self.table=self.alloc(7*4);self.put(EFFECT+0xb0,self.table)
        metadata=[]
        for i,(offset,data)in enumerate(((0x400,0x1800),(0x500,0x2000),(0x600,0x2800))):metadata.append((self.parameters+i*0x50,offset,data))
        for offset,data in((0x700,0x3000),(0x740,0x3100),(0x780,0x3200),(0x7c0,0x3300)):
            p=self.alloc(0x50);self.call('parameter_ctor',receiver=p);metadata.append((p,offset,data))
        for i,(p,offset,data)in enumerate(metadata):
            self.put(self.table+i*4,p);shared=c['shared']and i<3 and i%2
            base=self.shared_blob if shared else self.blob
            self.put(p,base+data,base+offset,0xffffffff,0,3,i,0,0)
            self.put(p+0x28,c['parameter_flags']if i<3 else 4,offset,data);self.put(p+0x38,0,0,1 if shared else 0)
            self.put(cp.OUT,offset,data);size=self.call('value_size',[base,cp.OUT,cp.OUT+4]);self.put(p+0xc,size)
        self.annotations=[p for p,_,_ in metadata[3:]]
        self.put(self.parameters+0x38,2,self.annotations[0]);self.put(self.annotations[0]+0x3c,self.annotations[1])
        self.techniques=self.array(2,0xb4,'technique_construct');self.put(EFFECT+0xa4,self.techniques)
        for t in range(2):
            technique=self.techniques+t*0xb4;passes=self.array(2,0x38,'pass_construct')
            self.put(technique+0x14,t|(c['upper_word']<<16));self.put(technique+0x28,0x100+t*16,1 if t==0 else 0,self.annotations[2]if t==0 else 0,2,passes)
            for p in range(2):
                item=passes+p*0x38;self.put(item+0x14,t|(c['upper_word']<<16),p|(c['upper_word']<<16));self.put(item+0x2c,1 if t==0 and p==0 else 0,self.annotations[3]if t==0 and p==0 else 0)
        self.groups=self.array(2,0x64,'group_construct');self.put(EFFECT+0x2c,self.groups);self.put(EFFECT+0x34,2)
        self.references=self.array(2,0x18,'reference_construct');self.put(EFFECT+0x14,2,self.references)
        ref_buffer=self.alloc(16);ref_data=self.alloc(4);self.put(ref_buffer,0x7b711c,1,4,ref_data);self.put(ref_data,0x78563412);self.put(self.references,4,ref_buffer)
        for g in range(2):
            records=self.array(2,0x64,'record_construct');self.put(self.groups+g*0x64+4,records);self.put_half(self.groups+g*0x64+0x36,2);self.put_half(self.groups+g*0x64+0x38,2);self.put(self.groups+g*0x64+0xc,c['storage_offset'])
        self.group=self.groups+c['group']*0x64;self.record=self.word(self.group+4)+c['record']*0x64
        self.put_half(self.record+0x52,c['code']);self.put(self.record+0x58,c['record_tag']);self.put(self.record+0x30,c['record_flags'])
        self.put_half(self.record+0x54,0x55aa);self.put(self.record+0x48,9)
        self.scratch=self.alloc(28);self.put(EFFECT+0xc4,self.scratch);self.put(self.scratch,c['scratch_kind'],c['scratch_shape'],0xa5a5a5a5,0x5a5a5a5a,c['scratch_elements'],c['scratch_rows'],c['scratch_columns'])
        self.u.mem_write(QUERY,c['query']+b'\0');self.u.mem_write(NAME,c['name']+b'\0')
        self.put(GENERIC,0,self.blob+0x500,c['element_value'],0,c['handle_kind'],c['owner'],c['offset18'],c['offset1c'],0,0)
        self.requests=0;self.events=[]
    def encoded(self,p):return(~p)&0xffffffff
    def run(self,kind,c):
        self.active_kind=kind;self.setup(c)
        handles=[0,1,QUERY,self.encoded(GENERIC),self.encoded(self.parameters),self.encoded(self.parameters+0x50)]
        handle=handles[c['handle_mode']]
        if kind in('decimal','decimal_body'):result=self.call(kind,[QUERY])
        elif kind=='resolve':result=self.call(kind,[handle,c['depth'],c['create']],EFFECT)
        elif kind=='element':
            if handle==self.encoded(GENERIC)and c['owner']>=7:self.put(GENERIC+0x14,c['owner']%7)
            result=self.call(kind,[handle,c['index'],c['depth']],EFFECT)
            if c['repeat']and result:result=self.call(kind,[handle,c['index'],c['depth']],EFFECT)
        elif kind=='member':
            result=self.call(kind,[handle,0 if c['null_name']else NAME,c['depth']],EFFECT)
            if c['repeat']and result:result=self.call(kind,[handle,0 if c['null_name']else NAME,c['depth']],EFFECT)
        elif kind=='annotation':
            owner=(self.parameters,self.techniques,self.word(self.techniques+0x38),self.parameters+0x50)[c['annotation_owner']]
            result=self.call(kind,[self.encoded(owner),0 if c['null_name']else NAME,c['depth']],EFFECT)
            if c['repeat']and result:result=self.call(kind,[self.encoded(owner),NAME,c['depth']],EFFECT)
        elif kind=='technique':result=self.call(kind,[handle,c['depth']],EFFECT)
        elif kind=='pass_find':result=self.call(kind,[self.encoded(GENERIC),c['depth']],EFFECT)
        elif kind=='members':result=self.call(kind,[self.parameters,c['member_count']],EFFECT)
        else:
            name=c['query']+(b'\0'if c['terminated']else b'x');size=len(name)
            body=cp.payload(c)if c['compiled_payload']else b''
            header=[size]if kind=='symbol'else[4+size+len(body),size]
            raw=cp.pack(header)+name+bytes((-size)&3)+body
            buffer=self.alloc(32);stream=self.alloc(len(raw)+32);offset=-4 if c['stream_start']==0xffffffff else c['stream_start']*4
            self.u.mem_write(stream+offset,raw);self.put(buffer,0x7b711c,1,len(raw)+max(offset,0)if c['limit_mode']==0 else c['limit'],stream);self.put(POSITION,c['stream_start'])
            result=self.call(kind,[c['group'],c['record'],0 if c['null_buffer']else buffer,POSITION],EFFECT)
        return self.snapshot(result)
    def native(self,c):
        self.active_kind='native';self.setup(c,native=True);op=c['operation']
        owners=[self.parameters,self.techniques,self.word(self.techniques+0x38),self.parameters+0x50];owner=owners[c['native_owner']]
        if op==0:
            result=self.call('resolve',[QUERY,c['depth'],c['create']],EFFECT)
            if result:result=self.call('resolve',[QUERY,c['depth'],c['create']],EFFECT)
        elif op==1:
            result=self.call('element',[self.encoded(owner),c['native_index'],c['depth']],EFFECT)
            self.call('element',[self.encoded(owner),0,c['depth']],EFFECT);self.call('element',[self.encoded(owner),1,c['depth']],EFFECT);result=self.call('element',[self.encoded(owner),c['native_index'],c['depth']],EFFECT)
        elif op==2:result=self.call('member',[self.encoded(owner),NAME,c['depth']],EFFECT)
        elif op==3:
            result=self.call('annotation',[self.encoded(owner),NAME,c['depth']],EFFECT)
            if result:result=self.call('annotation',[self.encoded(owner),NAME,c['depth']],EFFECT)
        elif op==4:result=self.call('technique',[QUERY,c['depth']],EFFECT)
        elif op==5:result=self.call('pass_find',[self.encoded(owner),c['depth']],EFFECT)
        elif op in(6,7):
            name=c['query']+b'\0';body=cp.payload(c)if c['compiled_payload']and op==7 else b''
            header=[len(name)]if op==6 else[4+len(name)+len(body),len(name)];raw=cp.pack(header)+name+bytes((-len(name))&3)+body
            buffer=self.alloc(16);stream=self.alloc(len(raw));self.put(buffer,0x7b711c,1,len(raw),stream);self.u.mem_write(stream,raw);self.put(EFFECT+0x28,buffer);self.put(POSITION,0)
            result=self.call('symbol'if op==6 else'symbol_record',[0,0,buffer,POSITION],EFFECT)
        elif op==8:result=self.call('decimal',[QUERY])
        else:
            self.call('members',[self.parameters,2],EFFECT);result=self.call('members',[self.parameters,c['native_count']],EFFECT)
        record=(~result)&0xffffffff if op in(1,2,3)and result else result if op in(0,4,5)else 0
        words=[result if op>=6 else 0,int(bool(record))]
        if record:
            kind=self.word(record+0x10);words.extend([kind,self.word(record+0x14)if kind==3 else self.half(record+0x14),self.word(record+8)if kind==3 else self.half(record+0x18)if kind==1 else 0])
            words.extend(self.word(record+o)if kind==3 else 0 for o in(0x18,0x1c,0xc))
            base=self.base(record)if kind==3 else 0;words.extend((self.word(record+o)-base)&0xffffffff if kind==3 else 0 for o in(4,0))
        else:words.extend([0]*8)
        words.extend(self.word(EFFECT+o)for o in(0xa8,0xac,0x120,0x14,0x118));words.extend([self.word(self.groups+0xc),self.half(self.groups+0x34)])
        def walk(p):
            base=self.base(p);words.extend(self.word(p+o)for o in(8,0xc,0x10,0x14,0x18,0x1c));words.extend((self.word(p+o)-base)&0xffffffff for o in(4,0))
            array=self.word(p+0x20);count=self.word(array-4)if array else 0;words.append(count)
            for i in range(count):walk(array+i*0x28)
            next_node=self.word(p+0x24);words.append(int(bool(next_node)))
            if next_node:walk(next_node)
        def parameter(p):
            walk(p);words.extend([self.word(p+0x28),self.word(p+0x38)if self.word(p+0x14)<self.word(EFFECT+0xac)else 0])
            children=[];at=self.word(p+0x3c)
            while at:children.append(at);at=self.word(at+0x3c)
            words.append(len(children))
            for child in children:parameter(child)
            head=self.word(p+0x34);at=head;links=[]
            if head:
                while True:
                    entry=self.word(at+0x10);links.extend([self.word(at),self.word(at+4),self.half(at+0x14),self.half(at+0x16),self.word(entry+4)if entry else 0,self.word(entry+8)if entry else 0]);at=self.word(at+0x1c)
                    if at==head:break
                    assert len(links)<100
            words.append(len(links)//6);words.extend(links)
        for i in range(3):parameter(self.parameters+i*0x50)
        for t in range(2):
            technique=self.techniques+t*0xb4;words.append(self.word(technique+0x2c));at=self.word(technique+0x30)
            while at:parameter(at);at=self.word(at+0x3c)
            for i in range(2):
                item=self.word(technique+0x38)+i*0x38;words.append(self.word(item+0x2c));at=self.word(item+0x30)
                while at:parameter(at);at=self.word(at+0x3c)
        for i in range(self.word(EFFECT+0x120)):
            blob=self.word(self.word(EFFECT+0x11c)+i*4);size=0
            for j in range(self.word(EFFECT+0xac),self.word(EFFECT+0xa8)):
                p=self.word(self.word(EFFECT+0xb0)+j*4)
                if self.word(p+4)-self.word(p+0x2c)==blob:size=self.word(p+0x30)+self.word(p+0xc);break
            words.append(self.digest(blob,size)if blob else 0)
        for i in range(self.word(EFFECT+0x14)):
            p=self.word(EFFECT+0x18)+i*0x18;buffer=self.word(p+4);words.extend([self.word(p),int(bool(buffer)),int(bool(self.word(p+8)))])
            words.extend([self.word(buffer+4),self.word(buffer+8),self.digest(self.word(buffer+12),self.word(buffer+8))]if buffer else[0,0,0])
        r=self.record;words.extend([self.word(r),self.word(r+0x30),self.half(r+0x52),self.half(r+0x54),self.half(r+0x56),self.word(r+0x48),self.word(r+0x58),self.half(r+0x4e),self.half(r+0x50)])
        if op in(6,7):walk(r+8)
        words.extend(int(bool(self.word(r+o)))for o in(0x34,0x3c,0x40,0x44));buffer=self.word(r+0x38);words.append(int(bool(buffer)))
        if buffer:
            data=self.word(buffer+12);size=self.word(buffer+8);words.append(size);words.extend(self.word(data+i*4)for i in range(4))
            words.extend(self.word(data+i*4)if result==0 and c['compiled_payload']and self.word(r+0x3c)else 0 for i in(4,5,6));words.append(self.digest(data+32,size-32))
        if op in(6,7):words.append(self.word(POSITION))
        self.call('destroy',receiver=EFFECT);assert not self.live,('native handle cleanup',self.live)
        return words
    def base(self,record):
        owner=self.word(record+0x14)
        return self.word(self.word(EFFECT+0x11c)+(owner-self.word(EFFECT+0xac))*4)if owner>=self.word(EFFECT+0xac)else self.blob
    def digest(self,address,size):
        result=2166136261
        for byte in self.u.mem_read(address,size):result=((result^byte)*16777619)&0xffffffff
        return result


def cases(n):
    rng=random.Random(0x665a15)
    for i,c in enumerate(cp.cases(n)):
        c.update(query=(b'p0',b'p1',b'p2',b'p0.m',b'p0.n.inner',b'p1[0]',b'p1[2]',b'p1[4294967297]',b'p1[-1]',b'p1[3]',b'p0@a',b'p0@arr[1]',b'missing',b'T0',b'T1',b'2147483648',b' -4294967297x')[i%17],
            name=(b'm',b'n.inner',b'm[1]',b'a',b'arr[1]',b'ta',b'pa',b'new',b'bad.name',b'new[0]',b'\x80new',b'',b'12345678')[i%13],
            flags=rng.getrandbits(16),parameter_flags=i%2,struct_elements=(0,0,1,3)[(i//5)%4],scalar_elements=(0,1,3,4)[(i//7)%4],member_elements=(0,1,3)[(i//11)%3],scalar_shape=(i//13)%4,
            resource_type=(5,6,9,10,11,14,15,16)[(i//3)%8],resource_elements=(i//17)%3,code=(146,147,12,0)[i%4],handle_mode=(i//2)%6,handle_kind=(0,1,3,4)[(i//7)%4],owner=(i//11)%8,
            element_value=(0xffffffff,0,1)[(i//13)%3],offset18=(i//5)%2,offset1c=(i//3)%2,upper_word=rng.randrange(0x10000),annotation_owner=(i//7)%4,
            scratch_kind=(1,2,3,4,0,5)[(i//3)%6],scratch_shape=(0,1,2,3,4,5,0xffffffff)[(i//5)%7],scratch_elements=(0,1,3)[(i//11)%3],scratch_rows=(0,1,2,4)[(i//13)%4],scratch_columns=(0,1,2,4)[(i//17)%4],
            member_count=(0,1,2)[(i//7)%3],index=(0,1,2,3,0xffffffff)[(i//3)%5],null_name=i%43==0,repeat=bool(i%2),create=i%3,depth=rng.getrandbits(32),
            stale_locale=i%3==0,update_b=bool(i%2),locale_width=(1,2,0xffffffff)[(i//7)%3],terminated=i%19!=0,compiled_payload=i%23!=0,
            cursor_callback=i%37==0,callback_kind='length'if i%2 else'pointer',callback_ordinal=(3,4,5)[(i//19)%3],rows=(0,1,2,4)[(i//17)%4],columns=(0,1,3,4)[(i//23)%4],name_mode=0)
        c['fail_allocation']=0 if i%4 else 1+(i//8)%8
        # Root/array/member and creation success cases are independent of the
        # malformed suffix, unsupported descriptor and allocation-failure cases.
        if i%3:c.update(handle_mode=2,struct_elements=0,scalar_elements=3,rows=1,columns=1,scratch_kind=1,scratch_shape=0,scratch_rows=1,scratch_columns=1,scratch_elements=0,flags=c['flags']|2)
        if i%29==0:c.update(handle_mode=4,struct_elements=0,name=b'n.inner',rows=1,columns=1,null_name=False)
        if i%31==0:c.update(name=b'new',scratch_kind=1,scratch_shape=0,scratch_rows=2,scratch_columns=3,scratch_elements=0,flags=c['flags']|2,null_name=False)
        if i%37==0:c.update(name=b'str',scratch_kind=4,scratch_shape=4,scratch_elements=0,flags=c['flags']|2,null_name=False)
        if i%41==0:c.update(code=164,resource_type=5,query=b'p2',terminated=True,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False)
        if i%43==0:c.update(code=178,resource_type=10,query=b'p2',terminated=True,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False)
        if i%47==0:c.update(code=146,resource_type=16,query=b'p2',terminated=True,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False)
        if i%53==0:c.update(code=147,resource_type=15,query=b'p2',terminated=True,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False)
        if i%59==0:c.update(query=b'p1',terminated=True,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False,compiled_payload=True,
            version=0x46580200,instructions=0,omit_instructions=False,omit_literals=False,omit_table=False,negative_token=False,legacy_def=False,instruction_skip=False,short_comment=False,constants=2,
            table_words=192,bank=2,same_bank=False,registers=1,start_register=0,vary_registers=False,register_starts=None,bad_name=False,missing_name=False,default_offset=0)
        if i%5==0:c.update(name=b'new'+str(i).encode(),flags=c['flags']|2,scratch_kind=4 if(i//5)%2 else 1,scratch_shape=4 if(i//5)%2 else 0,scratch_elements=0,scratch_rows=2,scratch_columns=3,null_name=False,fail_allocation=(i//10)%7)
        if i%73==0:c.update(handle_mode=4,struct_elements=3,name=b'm',null_name=False)
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();outcomes=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect handles mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect handles mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[hex(x[2])]+=1;outcomes[str((kind,hex(x[2])))]+=1
    for mode,c in native_cases():
        try:
            s=a.native(c);t=b.native(c);x=a.snapshot(tuple(s));y=b.snapshot(tuple(t))
        except(AssertionError,UcError)as e:raise AssertionError(('effect handles mismatch','chain',mode,str(e)))from e
        assert x==y and s==t,('effect handles mismatch','chain',mode,s,t)
        counts['typed_chain']+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),returns_by_body=dict(outcomes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert args.cases>0 and hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    if args.write_native_fixtures:
        content=b'UG2FXHF1'+cp.pack([32]);fixture=Machine(image)
        for mode,c in native_cases():
            fixture.active_kind='fixture';fixture.setup(c,native=True)
            query=c['query']+b'\0';name=c['name']+b'\0';body=cp.payload(c)if c['operation']==7 and c['compiled_payload']else b''
            config=[mode,c['operation'],c['create'],c['native_owner'],c['native_index'],c['native_count'],c['flags'],c['scratch_kind'],c['scratch_shape'],c['scratch_elements'],c['scratch_rows'],c['scratch_columns'],c['code'],len(query),len(name),len(body)]
            content+=cp.pack(config)+query+name+bytes(fixture.u.mem_read(fixture.blob,0x4000))+body
        args.write_native_fixtures.write_bytes(content)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free, CRT thread/locale services and selected cursor perturbations controlled; recovered conversion arithmetic and handle/reader dependencies execute',
        'finite mapped typed graphs and terminated streams; arbitrary invalid pointers, unsupported locale implementations, exceptions and giant counts not established',
        'main deserializer, relocation/shared-pool producers, shader preparation/evaluation and GPU/API activation remain open'])
    if args.mutation_checks:
        mutations={
            'skip-locale-refresh':('decimal_body','if (reinterpret_cast<unsigned int>(locale)!=ug2FxCrtLocale819D24)','if (false)'),
            'wrong-whitespace-mask':('decimal_body','space=UG2FxCrtClassify(locale,*cursor,8);','space=UG2FxCrtClassify(locale,*cursor,4);'),
            'saturate-decimal-arithmetic':('decimal_body','value=digit+value*10;','value=value>214748364U ? 2147483647U : digit+value*10;'),
            'omit-negative-decimal-sign':('decimal_body',"if (sign=='-') value=0-value;",'(void)0;'),
            'omit-decimal-thunk-call':('decimal','return D3DXEffectAtolBody(text);','return 0;'),
            'omit-technique-ordinal-gate':('technique','if ((handle&0xffff)==handle) return 0;','(void)0;'),
            'wrong-technique-name-offset':('technique','reinterpret_cast<unsigned char *>(prefix+1)','reinterpret_cast<unsigned char *>(prefix)'),
            'wrong-pass-kind':('pass_find','if (record->value10==1 && index<valueA0)','if (record->value10==0 && index<valueA0)'),
            'use-full-pass-technique-index':('pass_find','unsigned int index=static_cast<unsigned short>(record->value14);','unsigned int index=record->value14;'),
            'use-full-pass-index':('pass_find','unsigned int pass=static_cast<unsigned short>(record->value18);','unsigned int pass=record->value18;'),
            'omit-old-member-array-delete':('members','record->array20->DeleteArray(3); record->array20=0;','record->array20=0;'),
            'wrong-member-array-size':('members','UG2FxMalloc(count*0x28+4)','UG2FxMalloc(count*0x28)'),
            'wrong-member-type-offset':('members','member->value18=type-owner->value2C;','member->value18=type-owner->value2C+4;'),
            'use-unpadded-member-data':('members','member->value1C=paddedData-owner->value30;','member->value1C=data-owner->value30;'),
            'accept-indexed-array-parent':('element','if (record->value08!=0xffffffff || !count || count<=index)','if (!count || count<=index)'),
            'allow-array-end-index':('element','!count || count<=index','!count || count<index'),
            'omit-element-cache-reuse':('element','if (child && child->value08<=index) return','if (false) return'),
            'wrong-element-stride':('element','paddedData-owner->value30-record->value1C)/count','paddedData-owner->value30-record->value1C)*count'),
            'omit-element-size-division':('element','child->value0C=size/count;','child->value0C=size;'),
            'wrong-element-index-store':('element','child->value08=index;','child->value08=index+1;'),
            'omit-resolver-create-bit-clear':('resolve','value118&=0xfffffffd;','(void)0;'),
            'return-complemented-base':('resolve','return record;','return reinterpret_cast<D3DXEffectBaseRecord *>(~reinterpret_cast<unsigned int>(record));'),
            'omit-member-array-build':('member','if (!record->array20 && BuildMembers(record,count)<0) return 0;','if (!record->array20) return 0;'),
            'allow-unindexed-struct-array':('member','if (type[1]!=5 || (record->value08==0xffffffff && type[4]))','if (type[1]!=5)'),
            'return-raw-member-pointer':('member','return ~reinterpret_cast<unsigned int>(member);','return reinterpret_cast<unsigned int>(member);'),
            'wrong-dot-suffix-step':('member','name=cursor+1;descend=true;break;','name=cursor+2;descend=true;break;'),
            'ignore-annotation-create-gate':('annotation','if (!(value118&2)) return 0;','(void)0;'),
            'wrong-dynamic-annotation-base-bound':('annotation','if (annotation->value14<valueAC)','if (annotation->value14<=valueAC)'),
            'allow-dotted-new-annotation':('annotation',"*cursor=='.' || *cursor=='@'", "*cursor=='!' || *cursor=='@'"),
            'wrong-annotation-table-size':('annotation','UG2FxMalloc(valueA8*4+4)','UG2FxMalloc(valueA8*4+8)'),
            'omit-new-blob-slot-clear':('annotation','newBlobs[value120]=0;','(void)0;'),
            'omit-blob-count-publication':('annotation','++value120;newBlobs=0;','newBlobs=0;'),
            'initialize-untouched-annotation-count':('annotation','pending->value2C=padded+4;','pending->value38=0;pending->value2C=padded+4;'),
            'wrong-numeric-annotation-width':('annotation','description[6]*description[5]*4','description[6]*description[5]*8'),
            'omit-reference-ownership-transfer-clear':('annotation','for (i=0;i<words;++i) old[i]=0;','(void)0;'),
            'omit-reference-ownership-copy':('annotation','for (i=0;i<words;++i) copied[i]=old[i];','(void)0;'),
            'overwrite-annotation-chain-head':('annotation','if (tail) tail->value3C=reinterpret_cast<unsigned int>(pending);','if (tail) *head=reinterpret_cast<unsigned int>(pending);'),
            'wrong-symbol-kind5-compatibility':('symbol','if (type!=5 && type!=6','if (type!=4 && type!=6'),
            'wrong-symbol-kind10-compatibility':('symbol','if (type!=10 && type!=11','if (type!=9 && type!=11'),
            'wrong-symbol-use-allocation':('symbol','UG2FxMalloc(0x24)','UG2FxMalloc(0x20)'),
            'omit-symbol-record-group-use':('symbol','++group->value34;','(void)0;'),
            'wrong-symbol-record-prefix':('symbol_record','D3DXEffectBufferCreateForward(bytes+0x20','D3DXEffectBufferCreateForward(bytes+0x1c'),
            'omit-second-symbol-section-parse':('symbol_record','                    result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));','                    result=0;'),
            'wrong-symbol-record-unit-mask':('symbol_record','data[2]=0;data[0]=1;data[1]=3;','data[2]=0;data[0]=1;data[1]=7;'),
        }
        rejected=[];selected=set(SPANS)if args.kind=='all'else{args.kind}
        for name,(kind,before,after)in mutations.items():
            if kind not in selected:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            command=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','256']+(['--gcc']if args.gcc else[])
            if args.native_dump:command+=['--native-dump',str(args.native_dump)]
            r=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(r.stdout)
            assert r.returncode and 'effect handles mismatch'in r.stdout,(name,r.stdout);rejected.append(name)
        report['rejected_mutations']=rejected
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXHD1';at=8;seen=set();a,b=Machine(image),Machine(image,linked,args.gcc);fixtures=dict(native_cases())
        while at<len(raw):
            mode,count=struct.unpack_from('<2I',raw,at);at+=8;words=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert mode in fixtures and mode not in seen;seen.add(mode)
            try:x=a.native(fixtures[mode]);y=b.native(fixtures[mode])
            except(AssertionError,UcError)as e:raise AssertionError(('native effect handles mismatch',mode,str(e)))from e
            assert x==y==words,('native effect handles mismatch',mode,x,y,words)
        assert at==len(raw)and len(seen)==32
        report['native_replay']=dict(graphs=32,all_normalized_handles_caches_annotations_readers_and_conversion_results_match=True,all_graphs_destroyed=True)
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
