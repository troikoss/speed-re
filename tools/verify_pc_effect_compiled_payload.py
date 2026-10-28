#!/usr/bin/env python3
"""Compare compiled effect token parsing, constants and linked record storage.

All eight bodies and recovered metadata/buffer/use/cleanup dependencies execute.
Only allocation/free and selected getter-entry cursor changes are controlled.
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
import verify_pc_effect_readers as rd

md=rd.md;ROOT,EXE,LIB=rd.ROOT,rd.EXE,rd.LIB
SOURCE=LIB/'EffectCompiledPayload.cpp';BUILD=ROOT/'build/analysis/effect-compiled-payload'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-compiled-payload.json'
ARENA,HEAP,STOP=rd.ARENA,rd.HEAP,rd.STOP
EFFECT,POSITION=rd.EFFECT,rd.POSITION
TOKENS,OUT,COUNTS,MINIMUM,MAXIMUM,QUERY=ARENA+0x4000,ARENA+0x3100,ARENA+0x3140,ARENA+0x3160,ARENA+0x3180,ARENA+0x3400
SPANS=dict(comment=(0x671db5,231),constants=(0x65b3f7,596),operands=(0x688d2c,96),result=(0x688d8c,61),
    sections=(0x688dc9,176),root=(0x65be0d,267),bind=(0x660d4e,519),read=(0x661a40,485))
MEMBERS=dict(root=('D3DXEffectObject','FindRootParameter'),bind=('D3DXEffectObject','BindCompiledRecord'),read=('D3DXEffectObject','ReadCompiledRecord'))
FREE=dict(comment='D3DXEffectFindComment',constants='D3DXEffectReadConstantTable',operands='D3DXEffectWalkOperands',result='D3DXEffectWalkResult',sections='D3DXEffectReadCompiledSections')
md.SOURCES=md.SOURCES+[SOURCE];md.BUILD=BUILD;md.MEMBERS.update(MEMBERS);md.FREE.update(FREE);md.STDCALL.update(comment=16,constants=20,operands=8,result=4,sections=12)


def pack(words):return struct.pack('<'+'I'*len(words),*(v&0xffffffff for v in words))
def operands(c):
    return [v for _ in range(c['operands'])for v in([c['terms']]+[x for _ in range(c['terms'])for x in(c['term_kind'],0x12345678)]+[c['operand_kind'],0])]
def payload(c):
    instructions=[c['instructions']]
    for i in range(c['instructions']):instructions += [0x1234,c['operands']]+operands(c)+[c['result_prefix'],c['result_kind'],0]
    table=bytearray(0x300)
    struct.pack_into('<7I',table,0,28,0,c['version'],c['constants'],28,0,0)
    for i in range(c['constants']):
        name=(b'p0',b'p1',b'p2',b'missing')[i%4]if not c['missing_name']and c.get('missing_index')!=i else b'missing'
        name_offset=0x80+i*16
        table[name_offset:name_offset+len(name)+1]=name+b'\0'
        if c['bad_name']:name_offset=0x2ff;table[-1]=1
        bank=c['bank']if c['same_bank']else(c['bank']+i)%5
        start=c['register_starts'][i]if c.get('register_starts')else(c['start_register']+i)&0xffff if c.get('vary_registers')else c['start_register']
        struct.pack_into('<5I',table,28+i*20,name_offset,(start<<16)|bank,(0xa5a5<<16)|c['registers'],0,c['default_offset'])
    words=[c['version']]
    if c['negative_token']:words.append(0x80000000)
    if c['legacy_def']:words += [0x51,1,2,3,4,0xffff]
    if c['instruction_skip']:
        words += [0x03000001,0xffff,0x22222222,0x33333333]
    def comment(tag,data):words.extend([((len(data)+1)<<16)|0xfffe,tag,*data])
    if c.get('short_comment'):comment(c['tag'],[])
    if not c['omit_instructions']:comment(0x434c5846,instructions)
    if not c['omit_literals']:comment(0x54494c43,[0,0x12345678])
    if not c['omit_table']:
        data=list(struct.unpack('<192I',table));comment(0x42415443,data[:c['table_words']])
    words.append(0xffff)
    return pack(words)+bytes(c['tail'])


def retail_programs(pe):
    # Diagnostic fixture locator only. Bound each token walk to its retail
    # resource; the recovered scanner itself deliberately has no byte limit.
    versions={0x46580200,0x46580201,0x54580100,0xfffe0200,0xffff0200,0xfffe0300,0xffff0300,0xfffe0101,0xffff0101}
    rows=[]
    for r in rd.retail_inputs(pe):
        data=r['payload'];words=struct.unpack_from('<'+'I'*(len(data)//4),data)
        for start,version in enumerate(words):
            if version not in versions:continue
            at=start+1;tags=[]
            while at<len(words):
                token=words[at];opcode=token&0xffff
                if token&0x80000000:at+=1
                elif opcode==0xffff:
                    if any(tag in(0x42415443,0x434c5846,0x54494c43)for tag in tags):
                        body=data[start*4:(at+1)*4]
                        rows.append(dict(recipe=r['recipe'],name=r['name'],offset=start*4,payload=body,version=version,tags=tags))
                    break
                elif opcode==0xfffe:
                    count=(token>>16)&0x7fff
                    if count>1 and at+1<len(words):tags.append(words[at+1])
                    at+=count+1
                elif version&0xffff>=0x200:at+=1+((token>>24)&15)
                elif opcode==0x51:at+=6
                else:at+=1
    return rows


def native_cases():
    for mode in range(16):
        c=next(cases(1));c.update(version=0x46580200,instructions=1,operands=1,terms=1,term_kind=1,operand_kind=4,result_prefix=0,result_kind=4,
            constants=2,bank=2,same_bank=False,start_register=1,registers=1,default_offset=0,bad_name=False,missing_name=False,table_words=192,
            omit_instructions=False,omit_literals=False,omit_table=False,negative_token=False,legacy_def=False,instruction_skip=False,short_comment=False,vary_registers=False,register_starts=None,tail=0,
            parameter_flags=1,group=0,record=0,shared=False,name_mode=0,shape=0,elements=1,rows=1,columns=1,storage_offset=7,record_flags=0x1234567b,record_tag=0,already_bound=0,
            zero_payload=False,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False,fail_allocation=0,effect=b'\xa5'*0x130)
        if mode==1:c['term_kind']=9
        elif mode==2:c['result_prefix']=1
        elif mode==3:c['omit_literals']=True
        elif mode==4:c['omit_table']=True
        elif mode==5:c['missing_index']=1
        elif mode==6:c.update(default_offset=0x2f0,bank=2,same_bank=True)
        elif mode==7:c['constants']=0
        elif mode==8:c['zero_payload']=True
        elif mode==9:c['version']=0
        elif mode==10:c['parameter_flags']=4
        elif mode==11:c['version']=0x54580100
        elif mode==12:c.update(constants=3,bank=3,same_bank=True,start_register=1,registers=0xffff)
        elif mode==13:c.update(term_kind=0,operand_kind=9,result_kind=7)
        elif mode==14:c.update(instructions=0,rows=7,columns=2)
        elif mode==15:c['omit_instructions']=True
        yield mode,c


class Machine(rd.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for kind in('comment','constants','sections','root'):
            self.u.hook_add(UC_HOOK_CODE,self.observe,user_data=kind,begin=self.entries[kind],end=self.entries[kind])
    def observe(self,u,address,size,kind):self.events.append((kind,self.state()))
    def getter(self,u,address,size,kind):
        super().getter(u,address,size,kind)
        if self.active_kind=='read'and self.case['cursor_callback']and kind==self.case['callback_kind']and self.getters[kind]==self.case['callback_ordinal']:
            self.put(POSITION,(self.word(POSITION)+1)&0xffffffff)
    def setup(self,c,native=False):
        self.prepare(c);self.active_kind=getattr(self,'active_kind','');self.getters=Counter();self.position=POSITION
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
        self.put(EFFECT+0x98,3);self.put(EFFECT+0xa0,0);self.put(EFFECT+0xa8,3,3);self.put(EFFECT+0x118,0)
        self.blob=self.alloc(0x1000);self.shared_blob=self.blob if native else self.alloc(0x1000)
        self.u.mem_write(self.blob,bytes(0x1000));self.u.mem_write(self.shared_blob,bytes(0x1000));self.put(EFFECT+0x20,self.blob);self.put(EFFECT+0x1c,0x1000)
        parameters=self.array(3,0x50,'parameter_ctor');table=self.alloc(12);self.put(table,parameters,parameters+0x50,parameters+0xa0)
        self.put(EFFECT+0x9c,parameters);self.put(EFFECT+0xb0,table)
        if not native:
            pool=self.alloc(0x20);self.put(pool+0x10,self.shared_blob);self.put(EFFECT+0x38,pool)
        self.parameters=parameters
        for i in range(3):
            p=parameters+i*0x50;offset=0x200+i*0x40
            self.put(p+0x14,i,0,0);self.put(p+0x28,c['parameter_flags']);self.put(p+0x2c,offset);self.put(p+0x40,1 if c['shared']and i%2 else 0)
            for base in(self.blob,self.shared_blob):
                self.put(base+offset,1,c['shape'],0x80+i*16,0x20,c['elements'],c['rows'],c['columns'])
                name=(b'p'+str(i).encode())if c['name_mode']!=3 else b'P'+str(i).encode()
                if c['name_mode']==4:name=b'\x80'+str(i).encode()
                if c['shared']and i%2 and base==self.blob:name=b'shadow'+str(i).encode()
                prefix=0 if c['name_mode']==1 else 1 if c['name_mode']==2 else len(name)+1
                self.put(base+0x80+i*16,prefix);self.u.mem_write(base+0x84+i*16,(b''if c['name_mode']==2 else name)+b'\0')
            self.put(p+4,self.blob+offset)
        groups=self.array(2,0x64,'group_construct');self.put(EFFECT+0x2c,groups);self.put(EFFECT+0x34,2)
        for i in range(2):
            records=self.array(2,0x64,'record_construct');self.put(groups+i*0x64+4,records);self.put_half(groups+i*0x64+0x36,2);self.put_half(groups+i*0x64+0x38,2)
            self.put(groups+i*0x64+0xc,c['storage_offset'])
        self.group=groups+c['group']*0x64;self.record=self.word(self.group+4)+c['record']*0x64
        self.put(self.record+0x20,0x200);self.put(self.record+0x58,c['record_tag']);self.put(self.record+0x30,c['record_flags']);self.put_half(self.record+0x50,c['already_bound'])
        data=payload(c);self.payload=data;self.u.mem_write(TOKENS,data);self.put(OUT,0x12345678,0x23456789)
    def run(self,kind,c):
        self.active_kind=kind;self.setup(c)
        if kind=='comment':
            out=0 if c['null_out']else TOKENS if c['alias']==2 else OUT
            span=0 if c['null_span']else TOKENS if c['alias']==3 else OUT if c['alias']==1 else OUT+4
            result=self.call(kind,[0 if c['null_tokens']else TOKENS,c['tag'],out,span])
        elif kind=='constants':
            counts=COUNTS;minimum=counts if c['alias']==1 else MINIMUM;maximum=minimum if c['alias']==2 else MAXIMUM
            result=self.call(kind,[0 if c['null_tokens']else TOKENS,COUNTS if c['alias']==3 else OUT,counts,minimum,maximum])
        elif kind=='sections':result=self.call(kind,[TOKENS,OUT,OUT if c['alias']==1 else OUT+4])
        elif kind=='operands':
            self.u.mem_write(TOKENS,pack(operands(c))+bytes(64));self.put(OUT,TOKENS);result=self.call(kind,[c['operands'],OUT])
        elif kind=='result':
            self.put(TOKENS,c['result_prefix'],c['result_kind'],0);self.put(OUT,TOKENS);result=self.call(kind,[OUT])
        elif kind=='root':
            self.u.mem_write(QUERY,c['query']+b'\0');obj=ARENA+0x1400;self.put(obj,0,0,c['element'],0,c['handle_kind'],c['owner'],c['offset18'],c['offset1c'],0,0)
            handle=(0,1,0xffff,(~obj)&0xffffffff,QUERY)[c['handle_mode']]
            result=self.call(kind,[handle,c['depth']],EFFECT)
        elif kind=='bind':
            header=self.alloc(len(self.payload)+32);self.u.mem_write(header,bytes(32)+self.payload)
            result=self.call(kind,[c['group'],c['record'],header],EFFECT)
        else:
            buffer=self.alloc(32);stream=self.alloc(len(self.payload)+32);size=0 if c['zero_payload']else len(self.payload)
            raw=pack([size])+self.payload;offset=-4 if c['stream_start']==0xffffffff else c['stream_start']*4
            self.u.mem_write(stream+offset,raw)
            length=len(raw)+max(offset,0)if c['limit_mode']==0 else c['limit']
            self.put(buffer,0x7b711c,1,length,stream);self.put(POSITION,c['stream_start'])
            result=self.call(kind,[c['group'],c['record'],0 if c['null_buffer']else buffer,POSITION],EFFECT)
        return self.snapshot(result)
    def retail(self,r):
        c=next(cases(1));self.active_kind='retail';self.setup(c);self.u.mem_write(TOKENS,r['payload']);self.put(OUT,0,0,0,0,0)
        result=self.call('comment',[TOKENS,0x42415443,OUT,OUT+4]);offset=lambda address:(self.word(address)-TOKENS)&0xffffffff if self.word(address)else 0xffffffff
        summary=[result,offset(OUT),self.word(OUT+4)]
        result=self.call('constants',[TOKENS,OUT+8,COUNTS,MINIMUM,MAXIMUM]);summary.extend([result,offset(OUT+8)])
        summary.extend(self.half(base+i*2)for base in(COUNTS,MINIMUM,MAXIMUM)for i in range(4))
        result=self.call('sections',[TOKENS,OUT+12,OUT+16]);summary.extend([result,offset(OUT+12),offset(OUT+16)])
        return self.snapshot(tuple(summary)),summary
    def native(self,c):
        self.active_kind='native';self.setup(c,native=True)
        size=0 if c['zero_payload']else len(self.payload);buffer=self.alloc(16);stream=self.alloc(len(self.payload)+4)
        self.put(buffer,0x7b711c,1,len(self.payload)+4,stream);self.u.mem_write(stream,pack([size])+self.payload);self.put(EFFECT+0x28,buffer);self.put(POSITION,0)
        result=self.call('read',[0,0,buffer,POSITION],EFFECT)
        record=self.record;data_buffer=self.word(record+0x38);data=self.word(data_buffer+12)if data_buffer else 0
        summary=[result,self.word(POSITION),self.word(self.group+0xc),self.word(record),self.half(record+0x4e),self.half(record+0x50),self.word(record+0x30)]
        summary.extend(int(bool(self.word(record+o)))for o in(0x3c,0x40,0x44,0x34));summary.append(self.word(self.word(record+0x34)-4)if self.word(record+0x34)else 0)
        summary.extend([int(bool(data_buffer)),self.word(data_buffer+8)if data_buffer else 0,self.word(data+8)if data else 0])
        if result==0 and size:
            summary.extend(self.word(data+i*4)for i in(0,1,3))
            summary.extend(self.word(data+i*4)if self.word(record+0x3c)else 0 for i in(4,5,6))
        else:summary.extend([0]*6)
        for i in range(3):
            head=self.word(self.parameters+i*0x50+0x34);at=head;links=[]
            if head:
                while True:
                    links.extend([self.word(at),self.word(at+4),self.half(at+0x14),self.half(at+0x16)])
                    entry=self.word(at+0x10);links.extend([self.word(entry+4),self.word(entry+8)])
                    at=self.word(at+0x1c)
                    if at==head:break
                    assert len(links)<100
            summary.append(len(links)//6);summary.extend(links)
        self.u.mem_write(QUERY,b'p1\0')
        for handle in(QUERY,(~(self.parameters+0x50))&0xffffffff):
            value=self.call('root',[handle,10],EFFECT);summary.append((value-self.parameters)//0x50+1 if value else 0)
        self.call('destroy',receiver=EFFECT);assert not self.live,('native compiled cleanup',self.live)
        return summary


def cases(n):
    rng=random.Random(0x660d4e)
    for i,c in enumerate(rd.cases(n)):
        c.update(version=(0x46580200,0x46580201,0x54580100,0x465801ff,0x46580202,0xffff0300,0xfffe0101,0x7ffe0100,0x7fff0200,0x11110200)[i%10],
            instructions=(i//2)%3,operands=(i//3)%3,terms=(i//5)%3,term_kind=(0,1,8,9,10,0xffffffff)[(i//7)%6],operand_kind=(0,1,4,9,10)[(i//11)%5],
            result_prefix=(0,0,0,1,0xffffffff)[(i//13)%5],result_kind=(0,4,5,6,7,8,9)[(i//17)%7],
            constants=(i//2)%4,bank=(i//3)%5,same_bank=bool(i%3),start_register=(0,1,7,0xfffe,0xffff)[(i//19)%5],registers=(0,1,2,0xffff)[(i//23)%4],
            default_offset=(0,0x2f0,0x2ff,0x300,0xfffffff0)[(i//29)%5],bad_name=i%31==0,missing_name=i%17==0,table_words=(4,7,9,192)[(i//37)%4],short_comment=i%61==0,vary_registers=False,
            omit_instructions=i%19==0,omit_literals=i%23==0,omit_table=i%29==0,negative_token=i%7==0,legacy_def=False,instruction_skip=False,tail=i%4,
            null_tokens=i%13==0,null_out=i%11==0,null_span=i%17==0,tag=(0x42415443,0x434c5846,0x54494c43,0x12345678)[(i//7)%4],alias=(i//13)%4,
            group=i%2,record=(i//2)%2,storage_offset=(0,1,7,0x100,0xfffffff9)[(i//9)%5],record_flags=rng.getrandbits(32),record_tag=(0,0x0a000000,0x0b000000)[(i//7)%3],already_bound=int(i%41==0),
            parameter_flags=rng.getrandbits(8),shared=i%3==0,name_mode=(i//17)%5,query=(b'p0',b'p1',b'p2',b'missing',b'P0',b'',b'\x801')[(i//5)%7],
            handle_mode=(i//2)%5,handle_kind=(2,3,4)[(i//7)%3],owner=i%4,element=(0xffffffff,0,1)[(i//11)%3],offset18=i%2,offset1c=(i//3)%2,depth=rng.getrandbits(32),
            zero_payload=i%37==0,stream_start=(0,1,0xffffffff)[(i//23)%3],limit_mode=(i//7)%3,limit=(0,1,2,3,4,16)[(i//13)%6],null_buffer=i%31==0,cursor_callback=i%19==0,
            callback_kind='length'if i%2 else'pointer',callback_ordinal=(2,3,4,5)[(i//17)%4],shape=0,elements=(0,1,2)[(i//5)%3],rows=(0,1,3,4,7)[(i//7)%5],columns=(0,1,2,4)[(i//11)%4],callback=False)
        c['fail_allocation']=0 if i%4 else 1+(i//8)%4
        if i%3:
            c.update(version=0x46580200,instructions=1,operands=1,terms=1,term_kind=1,operand_kind=4,result_prefix=0,result_kind=4,constants=2,bank=2,same_bank=False,
                start_register=i%8,registers=1,default_offset=0,bad_name=False,missing_name=False,table_words=192,omit_instructions=False,omit_literals=False,omit_table=False,name_mode=0,
                parameter_flags=i&1,stream_start=0,limit_mode=0,null_buffer=False,cursor_callback=False,already_bound=0)
        if i%47==0:c.update(version=0xfffe0101,legacy_def=True,instruction_skip=False,negative_token=True,omit_table=False,table_words=192)
        if i%53==0:c.update(version=0xffff0300,instruction_skip=True,legacy_def=False,omit_table=False,table_words=192)
        if i%67==0:c.update(constants=1,bank=2,same_bank=True,registers=1,default_offset=0x2f0,table_words=192,null_tokens=False,alias=0,bad_name=False,missing_name=False,omit_table=False)
        if i%71==0:c.update(constants=2,bank=3,same_bank=True,start_register=0xfffe,registers=1,vary_registers=True,default_offset=0,table_words=192,null_tokens=False,alias=0,bad_name=False,missing_name=False,omit_table=False)
        if i%73==0:c.update(constants=3,bank=3,same_bank=True,registers=0xffff,default_offset=0,table_words=192,null_tokens=False,alias=0,bad_name=False,missing_name=False,omit_table=False)
        if i%79==0:c.update(handle_mode=3,handle_kind=3,owner=1,element=0xffffffff,offset18=0,offset1c=0,parameter_flags=1)
        if i%83==0:c.update(handle_mode=4,query=b'p1',name_mode=0,parameter_flags=4)
        if i%89==0:c.update(handle_mode=3,handle_kind=3,owner=1,element=0xffffffff,offset18=0,offset1c=0,parameter_flags=4)
        if i%97==0:c.update(handle_mode=4,query=b'\x801',name_mode=4)
        if i%101==0:c.update(handle_mode=4,query=b'',name_mode=2)
        if i%103==0:c.update(constants=2,bank=0,same_bank=False,registers=0,register_starts=[1,0x8000],default_offset=0,table_words=192,null_tokens=False,alias=0,bad_name=False,missing_name=False,omit_table=False)
        if i%107==0:c.update(handle_mode=3,handle_kind=3,owner=3,element=0xffffffff,offset18=0,offset1c=0,parameter_flags=1)
        if i%109==0:c.update(handle_mode=3,handle_kind=3,owner=1,element=0,offset18=0,offset1c=0,parameter_flags=1)
        if i%113==0:c.update(handle_mode=3,handle_kind=3,owner=1,element=0xffffffff,offset18=1,offset1c=0,parameter_flags=1)
        if i%127==0:c.update(version=0x46580200,constants=1,bank=3,same_bank=True,start_register=1,registers=0,bad_name=True,table_words=192,null_tokens=False,alias=0,omit_table=False)
        yield c


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();outcomes=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect compiled payload mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:1],w[:1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect compiled payload mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[hex(x[2])]+=1;outcomes[str((kind,hex(x[2])))]+=1
    summaries=[]
    for r in retail:
        try:x,s=a.retail(r);y,t=b.retail(r)
        except(AssertionError,UcError)as e:raise AssertionError(('effect compiled payload mismatch','retail',r['recipe'],r['offset'],str(e)))from e
        assert x==y and s==t,('effect compiled payload mismatch','retail',r['recipe'],r['offset'],s,t)
        counts['retail_parser_chain']+=1;events.update(e[0]for e in x[3])
        summaries.append(dict(recipe=r['recipe'],offset=r['offset'],bytes=len(r['payload']),sha256=hashlib.sha256(r['payload']).hexdigest(),summary=s))
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),returns_by_body=dict(outcomes),retail_summaries=summaries)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS,'retail'],default='all');p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert args.cases>0 and hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    retail=retail_programs(pe)if args.kind in('all','retail')or args.write_native_fixtures or args.native_dump else[]
    if args.write_native_fixtures:
        content=b'UG2FXCF2'+pack([16])
        for mode,c in native_cases():
            body=payload(c);content+=pack([mode,len(body),0 if c['zero_payload']else len(body),c['parameter_flags'],c['already_bound'],c['storage_offset'],c['rows'],c['columns']])+body
        content+=pack([len(retail)])+b''.join(pack([r['recipe'],r['offset'],len(r['payload'])])for r in retail);args.write_native_fixtures.write_bytes(content)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[]if args.kind=='retail'else[args.kind],retail if args.kind in('all','retail')else[])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free and selected getter-entry cursor changes controlled; recovered parser, metadata, buffer, linking and cleanup bodies execute',
        'finite mapped bounded token streams and graphs; arbitrary malformed pointers, unterminated token walks, exceptions and giant counts not established',
        'main deserializer, relocation/shared-pool producer, handle/symbol readers and shader/GPU activation remain open'])
    if args.mutation_checks:
        mutations={
            'omit-comment-data-clear':('comment','if (data) *data=0;','(void)0;'),
            'omit-comment-size-clear':('comment','if (bytes) *bytes=0;','(void)0;'),
            'wrong-null-token-error':('comment','0x8876086c','0x80004005'),
            'reject-vertex-shader-version':('comment','kind!=0xfffe0000','kind!=0xfffd0000'),
            'accept-tag-only-comment':('comment','if (count>1 &&','if (count>0 &&'),
            'wrong-comment-data-offset':('comment','const_cast<unsigned int *>(cursor+1)','const_cast<unsigned int *>(cursor)'),
            'wrong-comment-size':('comment','*bytes=count*4-4;','*bytes=count*4;'),
            'stop-at-negative-token':('comment','if (static_cast<int>(token)<0) continue;','if (static_cast<int>(token)<0) return 1;'),
            'omit-instruction-length-skip':('comment','cursor+=(token>>24)&15;','cursor+=0;'),
            'short-legacy-def-skip':('comment','cursor+=5;','cursor+=4;'),
            'wrong-comment-eof-result':('comment','if (opcode==0xffff) return 1;','if (opcode==0xffff) return 0;'),
            'wrong-counts-initialization':('constants','counts[i]=0;','counts[i]=1;'),
            'wrong-minimum-initialization':('constants','minimum[i]=0xffff;','minimum[i]=0;'),
            'omit-table-output-clear':('constants','*table=0;','(void)0;'),
            'propagate-comment-format-error':('constants','if (result>=0 && header) {','if (result<0) return result; if (header) {'),
            'strict-ctab-header-bound':('constants','span<header[3]*4+0x1c','span<=header[3]*4+0x1c'),
            'allow-unterminated-boundary-name':('constants','if (span<=entry[0]+length)','if (span<entry[0]+length)'),
            'wrong-bool-register-bank':('constants','case 0: bank=1;unit=4;','case 0: bank=0;unit=4;'),
            'use-full-register-count':('constants','unsigned short count=static_cast<unsigned short>(entry[2]),start=static_cast<unsigned short>(entry[1]>>16);','unsigned int count=entry[2],start=static_cast<unsigned short>(entry[1]>>16);'),
            'replace-register-count-sum':('constants','counts[bank]+count','count'),
            'truncate-end-before-maximum-test':('constants','if (maximum[bank]<=end)','if (maximum[bank]<=static_cast<unsigned short>(end))'),
            'reverse-register-minimum':('constants','if (start<=minimum[bank])','if (start>=minimum[bank])'),
            'relax-default-data-bound':('constants','if (bytes && span<=entry[4]+bytes)','if (bytes && span<entry[4]+bytes)'),
            'accept-operand-term9':('operands','if (kind==9) return static_cast<int>(0x80004005);','(void)0;'),
            'accept-zero-operand-kind':('operands','if (!kind || kind>9)','if (kind>9)'),
            'reject-operand-kind9':('operands','if (!kind || kind>9)','if (!kind || kind>8)'),
            'wrong-operand-pair-step':('operands','*cursor+=2;','*cursor+=1;'),
            'walk-extra-operand':('operands','i<count;++i','i<=count;++i'),
            'ignore-result-prefix':('result','if (!first) {','if (true) {'),
            'reject-result-kind7':('result','kind==7 || kind==4','kind==8 || kind==4'),
            'omit-result-failure-advance':('result','*cursor=start+1;','*cursor=start;'),
            'accept-newer-compiled-version':('sections','version>0x46580201','version>0x46580202'),
            'continue-after-missing-instructions':('sections','if (result<0 || result==1) return','if (result<0) return'),
            'wrong-literal-section-tag':('sections','0x54494c43,literals','0x434c5846,literals'),
            'skip-operand-validation':('sections','result=D3DXEffectWalkOperands(operands,&cursor);','result=0;'),
            'skip-result-validation':('sections','result=D3DXEffectWalkResult(&cursor);','result=0;'),
            'omit-small-handle-gate':('root','if ((handle&0xffff)==handle) return 0;','(void)0;'),
            'wrong-encoded-root-kind':('root','record->value10==3','record->value10==2'),
            'inclusive-root-owner-bound':('root','record->value14<valueA8','record->value14<=valueA8'),
            'accept-nonroot-offset':('root','&& !record->value18 && !record->value1C','&& !record->value1C'),
            'accept-indexed-root-handle':('root','&& record->value08==0xffffffff',''),
            'ignore-annotation-root-flag':('root','if (!(parameter->value28&4))','if (true)'),
            'ignore-shared-name-base':('root','parameter->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20','value20'),
            'omit-name-prefix-word':('root','reinterpret_cast<unsigned char *>(prefix+1)','reinterpret_cast<unsigned char *>(prefix)'),
            'wrong-first-root-index':('root','unsigned int i=0;i<value98','unsigned int i=1;i<value98'),
            'omit-bound-record-guard':('bind','if (record->value50 || record->array34) return static_cast<int>(0x80004005);','(void)0;'),
            'wrong-compiled-record-clear-bit':('bind','record->value30&=0xfffffffd;','record->value30&=0xfffffffe;'),
            'undersize-compiled-use-array':('bind','UG2FxMalloc(count*0x20+4)','UG2FxMalloc(count*0x20)'),
            'omit-compiled-use-count':('bind','record->value50=static_cast<unsigned short>(reinterpret_cast<unsigned int *>(record->value3C)[3]);','record->value50=0;'),
            'omit-linked-use-increment':('bind','unsigned int index=record->value4E++;','unsigned int index=record->value4E;'),
            'mask-use-counts-instead-of-maximum':('bind','static_cast<unsigned int>(maximum[1])+(static_cast<unsigned int>(maximum[0])+maximum[2])*4','static_cast<unsigned int>(counts[1])+(static_cast<unsigned int>(counts[0])+counts[2])*4'),
            'omit-mask-high-bit-spread':('bind','mask|=mask>>16;','(void)0;'),
            'align-constant-storage':('bind','offset=(offset&7)+(offset>>3)*8;','offset=(offset+7)&0xfffffff8;'),
            'wrong-bool-storage-width':('bind','static_cast<unsigned int>(maximum[1])*8','static_cast<unsigned int>(maximum[1])*4'),
            'wrong-compiled-buffer-prefix':('read','D3DXEffectBufferCreateForward(size+0x20','D3DXEffectBufferCreateForward(size+0x1c'),
            'omit-compiled-header-clear':('read','data[2]=0;','(void)0;'),
            'parse-zero-payload':('read','if (!size) return result;','(void)0;'),
            'floor-compiled-payload-words':('read','(size>>2)+((size&3)!=0)','size>>2'),
            'omit-second-section-parse':('read','            result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));','            result=0;'),
            'wrong-compiled-unit-alignment':('read','D3DXEffectUnitCount(&type,4)','D3DXEffectUnitCount(&type,1)'),
            'wrong-unit-storage-width':('read','group->value0C+=mask*8+8;','group->value0C+=mask*4+4;'),
        }
        selected=set(SPANS)if args.kind=='all'else{args.kind};rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in selected:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect compiled payload mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXCP1';at=8;seen=set();a,b=Machine(image),Machine(image,linked,args.gcc);synthetic=dict(native_cases())
        while at<len(raw):
            kind,index,count=struct.unpack_from('<3I',raw,at);at+=12;words=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert(kind,index)not in seen;seen.add((kind,index))
            if kind==0:x=a.native(synthetic[index]);y=b.native(synthetic[index])
            else:assert kind==1;_,x=a.retail(retail[index]);_,y=b.retail(retail[index])
            assert x==y==words,('native compiled payload mismatch',kind,index,x,y,words)
        assert at==len(raw)and len(seen)==16+len(retail)
        report['native_replay']=dict(record_graphs=16,retail_programs=len(retail),all_normalized_parser_link_and_storage_results_match=True,all_graphs_destroyed=True)
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
