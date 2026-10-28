#!/usr/bin/env python3
"""Compare compiled-effect record readers and complete retail metadata chains.

All readers, constructors, validators, size walks, buffer getters and cleanup
execute recovered source. Allocation/free and selected getter-entry perturbations
are the only controlled operations. No shader or relocation result is fabricated.
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
import verify_pc_effect_metadata as md

ROOT,EXE,LIB=md.ROOT,md.EXE,md.LIB
SOURCE=LIB/'EffectRecordReaders.cpp';BUILD=ROOT/'build/analysis/effect-readers';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-readers.json'
ARENA,HEAP,STOP=md.ARENA,md.HEAP,md.STOP
EFFECT,OBJECT,POSITION=md.EFFECT,md.OBJECT,ARENA+0x3020
SPANS=dict(annotation=(0x66a574,315),parameter=(0x66b420,493),pass_read=(0x66a6af,397),record=(0x66164d,1011),technique=(0x66b60d,1505),units=(0x65b019,127))
MEMBERS={k:('D3DXEffectObject',v)for k,v in dict(annotation='ReadAnnotation',parameter='ReadParameter',pass_read='ReadPass',record='ReadRecord',technique='ReadTechnique').items()}
md.SOURCES=md.SOURCES+[SOURCE];md.BUILD=BUILD;md.MEMBERS.update(MEMBERS)
md.FREE.update(units='D3DXEffectUnitCount',destroy='D3DXDestroyEffect');md.STDCALL['units']=8;md.VOID.add('destroy')


def retail_inputs(pe):
    recipes={}
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id!=10:continue
        for entry in t.directory.entries:
            d=entry.directory.entries[0].data.struct;recipes[str(entry.name)]=pe.get_data(d.OffsetToData,d.Size)
    result=[]
    for recipe in range(23):
        address=pe.get_dword_at_rva(0x7ff7d0-0x400000+recipe*0x8c+0x84);name=pe.get_string_at_rva(address-0x400000).decode('ascii');payload=recipes[name]
        magic,size=struct.unpack_from('<2I',payload);assert magic==0xfeff0901
        cursor=2+(size+3)//4;counts=struct.unpack_from('<4I',payload,cursor*4)
        result.append(dict(recipe=recipe,name=name,payload=payload,blob=payload[8:8+size],counts=counts,cursor=cursor+4))
    return result


class Machine(md.Machine):
    instruction_limit=1500000
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()});self.entries['destroy']=0x66adec
        for kind,original,name,n in [('length',0x670e15,'D3DXEffectBufferLength',4),('pointer',0x6954f7,'D3DXEffectBufferPointer',4)]:
            address=original if not linked else linked['exports'][name if gcc else'_'+name+'@'+str(n)]
            self.u.hook_add(UC_HOOK_CODE,self.getter,user_data=kind,begin=address,end=address)
    def getter(self,u,address,size,kind):
        obj,=self.args(1);self.events.append((kind,obj,self.state()))
        self.getters[kind]+=1
        if self.case['callback']and self.getters[kind]==1 and self.active_kind in('annotation','parameter','record'):
            # The real getter still runs after this synthetic pre-entry change.
            # This tests the caller's cursor reload across its virtual call.
            self.put(self.position,(self.word(self.position)+1)&0xffffffff)
    def type_description(self,blob,offset,c,annotation=False):
        shape=c['shape'];count=c['elements'];kind=0 if shape==5 else 1
        self.put(blob+offset,kind,shape,0,0x20,count)
        if shape<4:self.put(blob+offset+0x14,c['rows'],c['columns'])
        elif shape==5:
            self.put(blob+offset+0x14,c['members'])
            at=offset+0x18
            for i in range(c['members']):self.put(blob+at,1,i%4,0,0x20,1,1+i,1);at+=0x1c
    def setup(self,c,retail=None):
        self.prepare(c);self.getters=Counter();self.position=POSITION
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
        counts=retail['counts']if retail else(2,1,8,12)
        parameters,techniques,groups,resources=counts
        blob=self.alloc(max(0x1000,len(retail['blob'])if retail else 0x1000));self.u.mem_write(blob,bytes(max(0x1000,len(retail['blob'])if retail else 0x1000)))
        if retail:self.u.mem_write(blob,retail['blob']);span=len(retail['blob'])
        else:
            self.put(blob,2);self.u.mem_write(blob+4,b'a\0');self.put(blob+0x20,0)
            self.type_description(blob,0x100,c);self.type_description(blob,0x200,c,True);self.put(blob+0x300,c['resource'])
            if c['bad_name']:self.u.mem_write(blob+4,b'.\0')
            span=c['span']
        self.put(EFFECT+0x20,blob);self.put(EFFECT+0x1c,span)
        self.put(EFFECT+0x2c,self.array(groups,0x64,'group_construct')if groups else 0)
        self.put(EFFECT+0x9c,self.array(parameters,0x50,'parameter_ctor')if parameters else 0);self.put(EFFECT+0x98,parameters)
        self.put(EFFECT+0xa4,self.array(techniques,0xb4,'technique_construct')if techniques else 0);self.put(EFFECT+0xa0,techniques)
        self.put(EFFECT+0x18,self.array(resources,0x18,'reference_construct')if resources else 0);self.put(EFFECT+0x14,resources)
        self.put(EFFECT+0xa8,1);self.put(EFFECT+0x34,groups if retail else c['group_limit']);self.put_half(EFFECT+0x30,0 if retail else c['group_start'])
        for offset in(0x124,0x128,0x12c):self.put(EFFECT+offset,0 if retail else c['storage'])
        if not retail:
            for i in range(groups):self.put(self.word(EFFECT+0x2c)+i*0x64+0xc,c['storage'])
        # Oversized fixture allocation gives the wraparound read a mapped guard
        # word without changing the buffer object's four-word public layout.
        self.buffer=self.alloc(32);self.stream=self.alloc(max(0x1000,len(retail['payload'])if retail else 0x1000))
        self.u.mem_write(self.stream,bytes(max(0x1000,len(retail['payload'])if retail else 0x1000)))
        self.put(self.buffer,0x7b711c,1,len(retail['payload'])if retail else 0,self.stream);self.put(EFFECT+0x28,self.buffer)
        if retail:self.u.mem_write(self.stream,retail['payload'])
        self.blob=blob;self.counts=counts
    def stream_words(self,kind,c):
        annotations=lambda:[word for _ in range(c['annotations'])for word in(0x200,0x500)]
        records=lambda p:[word for j in range(c['records'])for word in(self.code_word(c,p+j),self.slot_word(c,p+j),0x100,0x300)]
        pass_words=lambda p:[0,c['annotations'],c['records']]+annotations()+records(p)
        if kind=='annotation':words=[0x100,0x300]
        elif kind=='parameter':words=[0x100,0x300,c['flags'],c['annotations']]+annotations()
        elif kind=='record':words=[self.code_word(c,0),self.slot_word(c,0),0x100,0x300]
        elif kind=='pass_read':words=pass_words(0)
        else:words=[0,c['annotations'],c['passes']]+annotations()+[v for p in range(c['passes'])for v in pass_words(p)]
        self.u.mem_write(self.stream,struct.pack('<'+'I'*len(words),*words));self.put(self.stream-4,0 if kind in('pass_read','technique')else 0x100)
        limit=len(words)*4 if c['limit_mode']==0 else(0,1,2,3,4,len(words)-1,len(words))[c['limit_mode']-1]
        self.put(self.buffer+8,limit)
    def code_word(self,c,i):return((c['code']+i)%184)|c['upper_code']
    def slot_word(self,c,i):return(c['slot']if not c['vary_slots']else(c['slot']+i)%16)|c['upper_slot']
    def run(self,kind,c,retail=None):
        self.active_kind=kind;self.setup(c,retail)
        if retail:
            self.put(POSITION,retail['cursor']);result=0
            for i in range(self.counts[0]):
                result=self.call('parameter',[self.word(EFFECT+0x9c)+i*0x50,self.buffer,POSITION],EFFECT)
                if result>=0x80000000:break
            if result<0x80000000:
                for i in range(self.counts[1]):
                    obj=self.word(EFFECT+0xa4)+i*0xb4;self.put_half(obj+0x14,i)
                    result=self.call('technique',[obj,self.buffer,POSITION],EFFECT)
                    if result>=0x80000000:break
            summary=self.summary(result)
            self.call('destroy',receiver=EFFECT);assert not self.live,('retail cleanup leak',self.live)
            return self.snapshot(result),summary
        if kind=='units':
            self.put(POSITION,self.blob+0x100)
            return self.snapshot(self.call(kind,[POSITION,c['alignment']]))
        self.stream_words(kind,c);self.put(POSITION,c['start'])
        buffer=0 if c['null_buffer']else self.buffer
        if kind in('annotation','parameter'):
            obj=self.word(EFFECT+0x9c);self.put(obj+0x2c,0x100,0x300);self.put(obj+0x38,c['annotations'])
            if c['alias']:self.position=obj+0x18
            if self.position!=POSITION:self.put(self.position,c['start'])
            result=self.call(kind,[obj,buffer,self.position],EFFECT)
        elif kind=='record':
            group=self.word(EFFECT+0x2c)+(c['group']&0xffff)*0x64
            self.put(group+4,self.array(2,0x64,'record_construct'));self.put_half(group+0x34,c['use_count'])
            self.put_half(group+0x36,2);self.put_half(group+0x38,2)
            obj=self.word(group+4)+(c['record']&0xffff)*0x64
            if c['alias']:self.position=obj+0x1c;self.put(self.position,c['start'])
            result=self.call(kind,[c['group'],c['record'],buffer,self.position],EFFECT)
        elif kind=='pass_read':
            self.call('pass_construct',receiver=OBJECT);self.put(OBJECT+0x28,0,c['annotations']);self.put(OBJECT+0x34,c['group']&0xffff)
            result=self.call(kind,[OBJECT,buffer,self.position],EFFECT)
        else:
            obj=self.word(EFFECT+0xa4);self.put(obj+0x28,0,c['annotations']);self.put(obj+0x34,c['passes'])
            self.put_half(obj+0x14,c['ordinal'])
            result=self.call('technique',[obj,buffer,self.position],EFFECT)
        return self.snapshot(result)
    def summary(self,result):
        words=[result,self.word(POSITION),self.word(EFFECT+0xa8),self.word(EFFECT+0x124),self.word(EFFECT+0x128),self.word(EFFECT+0x12c),self.half(EFFECT+0x30)]
        def annotations(head):
            seen=set()
            while head:
                assert head not in seen;seen.add(head)
                words.extend(self.word(head+o)for o in(0x2c,0x30,0x28,0x38));head=self.word(head+0x3c)
            words.append(len(seen))
        for i in range(self.counts[0]):
            p=self.word(EFFECT+0x9c)+i*0x50;words.extend(self.word(p+o)for o in(0x2c,0x30,0x28,0x38));annotations(self.word(p+0x3c))
        for i in range(self.counts[1]):
            p=self.word(EFFECT+0xa4)+i*0xb4;words.extend(self.word(p+o)for o in(0x28,0x2c,0x34));words.extend([self.half(p+0x14),self.half(p+0x3c)])
            words.extend(self.half(p+o)for o in range(0xa4,0xb4,2));annotations(self.word(p+0x30))
            for j in range(self.word(p+0x34)):
                q=self.word(p+0x38)+j*0x38;words.extend([self.word(q+0x28),self.word(q+0x2c),self.half(q+0x14),self.half(q+0x18),self.half(q+0x34)]);annotations(self.word(q+0x30))
        for i in range(self.counts[2]):
            p=self.word(EFFECT+0x2c)+i*0x64;words.extend([self.word(p+0xc),self.half(p+0x18),self.half(p+0x1a),self.half(p+0x34),self.half(p+0x36),self.half(p+0x38),self.half(p+0x3e)])
            for j in range(self.half(p+0x36)):
                q=self.word(p+4)+j*0x64;code=self.half(q+0x52)
                words.extend([self.word(q),self.word(q+0x30)&3,self.half(q+0x52),self.half(q+0x54),self.half(q+0x56),self.word(q+0x58),self.word(q+0x48),self.word(q+0x20),self.word(q+0x24)])
                if self.word(0x7b583c+code*28)>>24==8:words.append(self.half(q+0x4c))
        for i in range(self.counts[3]):
            p=self.word(EFFECT+0x18)+i*0x18;words.extend([self.word(p),self.word(p+0x14)&3])
        digest=2166136261
        for byte in struct.pack('<'+'I'*len(words),*words):digest=((digest^byte)*16777619)&0xffffffff
        return words[:7]+[len(words),digest]


def cases(n):
    rng=random.Random(0x66b60d)
    for i,c in enumerate(md.core.cases(n)):
        c.update(effect=rng.randbytes(0x130),shape=(0,1,2,3,4,5,6,0xffffffff)[(i//3)%8],elements=(0,1,2,3)[(i//8)%4],rows=(0,1,3,4,7,65535)[(i//2)%6],columns=(0,1,2,4)[(i//4)%4],members=(i//2)%3,
            span=(0,0x113,0x114,0x2000)[(i//5)%4],storage=rng.getrandbits(32),resource=1+i%10,
            annotations=(0,1,2)[(i//7)%3],passes=(0,1,2,3)[(i//9)%4],records=(0,1,2,3)[(i//11)%4],
            group_limit=(0,1,3,8)[(i//13)%4],group_start=(0,1,7,0xffff)[(i//17)%4],
            code=i%184,slot=(0,1,15,16,0x100,0x101,0x104,0x105)[(i//23)%8],upper_code=rng.randrange(0x10000)<<16,upper_slot=rng.randrange(0x10000)<<16,vary_slots=bool(i&1),
            flags=rng.getrandbits(32),use_count=(0,1,0xfffe,0xffff)[i%4],group=(0,1,0x10000,0x10001)[i%4],record=(0,1,0x10000,0x10001)[(i//2)%4],ordinal=rng.randrange(0x10000),
            start=(0,1,0xffffffff)[(i//19)%3],limit_mode=(0,1,2,3,4,5,6,7)[(i//29)%8],null_buffer=i%31==0,alias=i%7==0,callback=i%11==0,alignment=(1,4,7)[i%3])
        c['bad_name']=i%13==0
        if i%3:c['slot']=0
        if i%179==0:c.update(code=177,slot=0x100,vary_slots=False)
        c['fail_allocation']=0 if(i//6)%3 else 1+(i//18)%12
        # Every code also receives an ordinary full-input fixture.
        if i%4 or(i//184)%4:c.update(start=0,limit_mode=0,null_buffer=False,alias=False,callback=False,span=0x2000,group_start=0,group_limit=8)
        if i%173==0:c.update(code=165,slot=0x101,vary_slots=False,passes=1,records=1,annotations=0,start=0,limit_mode=0,null_buffer=False,alias=False,callback=False,span=0x2000,group_start=0,group_limit=8,fail_allocation=0)
        if i%67==0:c.update(start=0,limit_mode=0,null_buffer=False,alias=False,callback=True,flags=0,annotations=0,records=0,passes=0)
        yield c


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();outcomes=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect readers mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect readers mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[hex(x[2])]+=1;outcomes[str((kind,hex(x[2])))]+=1
    summaries=[]
    for r in retail:
        c=next(cases(1));c.update(callback=False,fail_allocation=0)
        try:x,s=a.run('retail',c,r);y,t=b.run('retail',c,r)
        except(AssertionError,UcError)as e:raise AssertionError(('effect readers mismatch','retail',r['name'],str(e)))from e
        assert x==y and s==t,('effect readers mismatch','retail',r['name'],'summary',s,t)
        assert x[2]==0,('retail reader failure',r['name'],hex(x[2]))
        counts['retail_chain']+=1;events.update(e[0]for e in x[3]);summaries.append(dict(recipe=r['recipe'],name=r['name'],summary=s))
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),returns_by_body=dict(outcomes),retail_summaries=summaries)


def native_replay(path,pe,image,linked,gcc):
    raw=path.read_bytes();assert raw[:8]==b'UG2FXRD1'and(len(raw)-8)%40==0
    rows=list(struct.iter_unpack('<10I',raw[8:]));assert len(rows)==23
    recipes={r['recipe']:r for r in retail_inputs(pe)};seen=set();a,b=Machine(image),Machine(image,linked,gcc)
    c=next(cases(1));c.update(callback=False,fail_allocation=0)
    for row in rows:
        recipe=row[0];assert recipe in recipes and recipe not in seen;seen.add(recipe)
        x,s=a.run('retail',c,recipes[recipe]);y,t=b.run('retail',c,recipes[recipe])
        assert x==y and s==t and tuple(s)==row[1:],('native reader mismatch',recipe,s,t,row[1:])
    return dict(recipes=23,all_cursors_counts_and_normalized_fields_match=True,all_chains_destroyed=True)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS,'retail'],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true')
    p.add_argument('--native-dump',type=Path,help='Replay UG2FXRD1 native normalized metadata capture')
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        offset=int(f['address'],16)-0x400000;assert hashlib.sha256(image[offset:offset+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[]if args.kind=='retail'else[args.kind],retail_inputs(pe)if args.kind in('all','retail')else[])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free controlled; selected getter-entry perturbations test cursor rereads while recovered getter bodies still execute',
        'bounded mapped streams and valid objects; malformed pointers, arbitrary object classes, exceptions and giant loops not established',
        'retail chains parse metadata and destroy all owned allocations; relocation, shared-pool binding, shader creation and rendering remain open'])
    if args.mutation_checks:
        mutations={
            'skip-cursor-increment':('parameter','++*position;','(void)0;'),
            'strict-stream-read-bound':('record','*position+1<=limit','*position+1<limit'),
            'divide-buffer-length':('annotation','limit=UG2FxBufferLengthVirtual(buffer);','limit=UG2FxBufferLengthVirtual(buffer)/4;'),
            'cursor-cache-before-length':('parameter','if (*position+1<=limit)','if (saved+1<=limit)'),
            'cursor-cache-before-pointer':('record','bytes+*position*4','bytes+saved*4'),
            'omit-final-stream-check':('annotation','if (limit<*position) return','if (false) return'),
            'wrong-annotation-flags':('annotation','p->value28=4;','p->value28=0;'),
            'wrong-scalar-storage-count':('annotation','(count>1 ? count : 1)*0x10','count*0x10'),
            'wrong-matrix-storage-size':('parameter','(count>1 ? count : 1)*0x40','(count>1 ? count : 1)*0x10'),
            'skip-struct-validation':('annotation','int result=ValidateType(&type,&data,0);','int result=0;'),
            'omit-annotation-delete':('parameter','annotation->Delete(1);','annotation->Delete(0);'),
            'overwrite-annotation-head':('parameter','tail->value3C=reinterpret_cast<unsigned int>(annotation);tail=annotation;','p->value3C=reinterpret_cast<unsigned int>(annotation);tail=annotation;'),
            'double-count-parameter':('parameter','D3DXEffectParameter *tail=p;','++valueA8; D3DXEffectParameter *tail=p;'),
            'omit-pass-name-validation':('pass_read','int result=ValidateString(p->value28);','int result=0;'),
            'wrong-pass-annotation-pointer':('pass_read','p->parameter30=annotation;','p->parameter30=tail;'),
            'omit-pass-record-grow':('pass_read','result=group->ResizeRecords(static_cast<unsigned short>(count));','result=group->ResizeRecords(0);'),
            'narrow-first-record-store':('record','FX_READ_WORD(*reinterpret_cast<unsigned int *>(&p->value52));','FX_READ_WORD(p->value52);'),
            'narrow-second-record-store':('record','FX_READ_WORD(*reinterpret_cast<unsigned int *>(&p->value54));','FX_READ_WORD(p->value54);'),
            'late-slot-upper-check':('record','limit && limit<=p->value54','limit && limit<p->value54'),
            'allow-reserved-texture-slot':('record','==0x0300000d','==0x0300000e'),
            'omit-resource-kind-write':('record','[index].value00=ug2FxTypeTable7B582C[p->value52][0];','[index].value00=0x7fffffff;'),
            'swap-shader-selectors':('record','group->value18=recordIndex;','group->value1A=recordIndex;'),
            'wrong-register-code-remap':('record','p->value52=0x94;','p->value52=0x9c;'),
            'wrong-bool-unit-divisor':('record','D3DXEffectUnitCount(&type,1)','D3DXEffectUnitCount(&type,4)'),
            'omit-group-word34-increment':('record','++group->value34;','(void)0;'),
            'round-storage-offset':('record','group->value0C=offset; p->value48=offset;','offset=(offset+3)&0xfffffffc; group->value0C=offset; p->value48=offset;'),
            'erase-record-other-flags':('record','p->value30&=0xfffffffd;','p->value30=0;'),
            'wrong-unit-ceiling':('units','(rows%alignment!=0)+rows/alignment','rows/alignment'),
            'omit-unit-element-default':('units','if (!count) count=1;','(void)0;'),
            'omit-unit-struct-scaling':('units','return sum*count;','return sum;'),
            'overwrite-technique-row-high-word':('technique','*reinterpret_cast<unsigned short *>(&p->array38[i].value34)=value30;','p->array38[i].value34=value30;'),
            'wrong-pass-group-limit':('technique','if (value34<=value30)','if (value34<value30)'),
            'skip-pass-ordinal':('technique','*reinterpret_cast<unsigned short *>(&p->array38[i].value00.value18)=i;','*reinterpret_cast<unsigned short *>(&p->array38[i].value00.value18)=0xffff;'),
            'wrong-unique-map-size':('technique','maximum*2+2','maximum*2+4'),
            'count-repeated-sampler':('technique','&& !unique[record->value54]',''),
            'undersize-resource-record':('technique','))*0x68', '))*0x64'),
            'forward-dirty-scan':('technique','offset-=0x64;','offset=static_cast<unsigned int>(group->value36-1-remaining)*0x64;'),
            'mark-all-class-seven':('technique','bool mark=!seen;','bool mark=true;'),
            'omit-forced-sampler-dirty':('technique','if (record->value58==0x0800000d)','if (false)'),
            'wrong-register-bank-threshold':('technique','record->value54>0x100','record->value54>0x101'),
            'clear-technique-packing-gap':('technique','p->unknown9C[2]=0;','p->unknown9C[1]=0; p->unknown9C[2]=0;'),
            'wrong-max-register-offset':('technique','(p)+0xb2','(p)+0xb0'),
            'reverse-technique-temp-frees':('technique','UG2FxFree(unique); if (used) UG2FxFree(used);','if (used) UG2FxFree(used); UG2FxFree(unique);'),
        }
        selected=set(SPANS)if args.kind=='all'else{args.kind};rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in selected:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            if name=='cursor-cache-before-length':body=body.replace('unsigned int limit=UG2FxBufferLengthVirtual(buffer);','unsigned int saved=*position; unsigned int limit=UG2FxBufferLengthVirtual(buffer);')
            if name=='cursor-cache-before-pointer':body=body.replace('unsigned char *bytes=UG2FxBufferPointerVirtual(buffer);','unsigned int saved=*position; unsigned char *bytes=UG2FxBufferPointerVirtual(buffer);')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect readers mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    if args.native_dump:report['native_replay']=native_replay(args.native_dump,pe,image,linked,args.gcc)
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
