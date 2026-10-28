#!/usr/bin/env python3
"""Compare serialized effect resources and lazy shader-record construction.

Recovered readers, metadata, byte buffers and shader-creation callers execute
source. Allocator, debug-mute and GPU creation calls are controlled boundaries.
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
SOURCE=LIB/'EffectShaderResources.cpp';BUILD=ROOT/'build/analysis/effect-shader-resources'
BRIDGE=ROOT/'src/Platform/Native/NativeFxShaderDispatch.cpp'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-shader-resources.json'
ARENA,HEAP,STOP=rd.ARENA,rd.HEAP,rd.STOP
EFFECT,POSITION=rd.EFFECT,rd.POSITION
DEVICE_A,DEVICE_B,VTABLE_A,VTABLE_B=[ARENA+x for x in(0x6500,0x6520,0x6800,0x6c00)]
GPU_A_V,GPU_A_P,GPU_B_V,GPU_B_P,MUTE=[STOP+x for x in(0x7300,0x7310,0x7320,0x7330,0x7340)]
SPANS=dict(vertex=(0x65b970,94),pixel=(0x65b9ce,94),resource=(0x65ba2c,562),shader_record=(0x66520d,638))
MEMBERS={k:('D3DXEffectObject',v)for k,v in dict(vertex='CreateVertexShader',pixel='CreatePixelShader',resource='ReadResource',shader_record='ReadShaderRecord').items()}
md.SOURCES=md.SOURCES+[SOURCE,BRIDGE];md.BUILD=BUILD;md.MEMBERS.update(MEMBERS);md.VOID.update(('vertex','pixel'))


class Machine(rd.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.linked=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for a,dispatch,slot in((GPU_A_V,VTABLE_A,0x16c),(GPU_A_P,VTABLE_A,0x1a8),(GPU_B_V,VTABLE_B,0x16c),(GPU_B_P,VTABLE_B,0x1a8)):
            self.u.hook_add(UC_HOOK_CODE,self.gpu,user_data=(dispatch,slot,False),begin=a,end=a)
        if linked:
            for name,slot in(('UG2FxCreateVertexShader',0x16c),('UG2FxCreatePixelShader',0x1a8)):
                address=linked['exports'][name if gcc else'_'+name+'@16'];self.u.hook_add(UC_HOOK_CODE,self.gpu,user_data=(0,slot,True),begin=address,end=address)
            address=linked['exports']['UG2FxDebugMute'if gcc else'_UG2FxDebugMute@4']
        else:address=0x671aed
        self.u.hook_add(UC_HOOK_CODE,self.mute,begin=address,end=address)
    def getter(self,u,address,size,kind):
        super().getter(u,address,size,kind)
        if kind=='pointer'and self.active_kind in('vertex','pixel')and self.case['getter_device']and self.getters[kind]==1:
            self.put(EFFECT+0xc,DEVICE_B)
        if self.active_kind in('resource','shader_record'):
            selected='length'if self.case['stream_callback']in(1,3)else'pointer'
            ordinal=5 if self.case['stream_callback']==3 else(3 if self.active_kind=='resource'else 2)if selected=='length'else 3
            if self.case['stream_callback']and kind==selected and self.getters[kind]==ordinal:self.put(POSITION,(self.word(POSITION)+1)&0xffffffff)
    def gpu(self,u,address,size,info):
        dispatch,slot,bridge=info
        if bridge:dispatch,device,code,output=self.args(4)
        else:device,code,output=self.args(3)
        self.events.append(('gpu',dispatch,slot,device,code,output,self.state()))
        if self.case['write_shader']:self.put(output,self.case['shader_token'])
        self.ret(self.case['gpu_result'],16 if bridge else 12)
    def mute(self,u,address,size,user):
        mute,=self.args(1);self.events.append(('mute',mute,self.state()));self.mutes+=1
        if self.case['mute_flag']and self.mutes==1:self.put(EFFECT+0x118,self.word(EFFECT+0x118)^0x200)
        if self.case['mute_output']and mute==0:self.put(self.reference+4,0x1234abcd);self.put(self.reference+0x14,self.case['reference_flags']^0x30)
        self.ret(0,4)
    def setup(self,c,retail=None):
        super().setup(c);self.mutes=0
        self.put(DEVICE_A,VTABLE_A);self.put(DEVICE_B,VTABLE_B)
        self.put(VTABLE_A+0x16c,GPU_A_V);self.put(VTABLE_A+0x1a8,GPU_A_P);self.put(VTABLE_B+0x16c,GPU_B_V);self.put(VTABLE_B+0x1a8,GPU_B_P)
        self.put(EFFECT+0xc,DEVICE_A);self.put(EFFECT+0x118,c['effect_flags'])
        self.references=self.word(EFFECT+0x18);self.reference=self.references+c['index']*0x18
        code=self.alloc(16);data=self.alloc(64);self.put(code,0x7b711c,1,64,data)
        self.u.mem_write(data,c['code_bytes'])
        for i in range(12):
            reference=self.references+i*0x18
            self.put(reference,c['resource_kind'],c['existing04'],c['existing08'],0,c['target_group']|0xa5a50000,c['reference_flags'])
        if self.active_kind in('vertex','pixel'):self.put(self.reference+8,code)
        self.code=code
    def write_stream(self,words,c):
        payload=struct.pack('<'+'I'*len(words),*words)+c['code_bytes']
        offset=-4 if c['start']==0xffffffff else c['start']*4
        self.u.mem_write(self.stream+offset,payload)
        self.put(self.buffer+8,len(payload)+offset if c['limit_mode']==0 else c['limit']);self.put(POSITION,c['start'])
    def run(self,kind,c):
        self.active_kind=kind;self.setup(c)
        if kind in('vertex','pixel'):result=self.call(kind,[self.reference],EFFECT)
        elif kind=='resource':
            self.write_stream([c['wire_index'],c['size']],c);self.put(EFFECT+0x14,c['resource_count'])
            for i in range(8):
                group=self.word(EFFECT+0x2c)+i*0x64;self.put(group+4,self.array(1,0x64,'record_construct'))
            result=self.call(kind,[0 if c['null_buffer']else self.buffer,POSITION],EFFECT)
        else:
            self.write_stream([c['size']],c)
            groups=self.word(EFFECT+0x2c);group=groups+c['group']*0x64;record_array=self.array(2,0x64,'record_construct');self.put(group+4,record_array)
            record=record_array+c['record']*0x64;self.put_half(record+0x52,c['code']);self.put(record+0x1c,c['parameter'])
            self.put(record+0x24,c['data_offset']);self.put(EFFECT+0x30,c['group_start']|(c['group_high']<<16));self.put(EFFECT+0x34,c['group_limit'])
            if c['preexisting']and 0<c['group_start']<8:
                target=groups+c['group_start']*0x64;old=self.array(2,0x64,'record_construct');self.put(target+4,old)
                self.put_half(target+0x36,2);self.put_half(target+0x38,2);self.put(old+0x30,c['target_flags']&0xfffffffe)
            self.put(group+0xc,c['storage_offset']);self.put_half(group+0x34,c['group_uses'])
            parameters=self.word(EFFECT+0x9c);table=self.alloc(8);self.put(table,parameters,parameters+0x50);self.put(EFFECT+0xb0,table)
            parameter=parameters+c['parameter']*0x50;self.put(parameter+0x30,0x300)
            self.put(self.blob+0x300+c['data_offset'],c['index'])
            if c['shared']:
                pool=self.alloc(0x20);link=self.alloc(0x14);indices=self.alloc(12*4)
                self.put(pool+0x10,self.blob);self.put(EFFECT+0x38,pool);self.put(parameter+0x40,link);self.put(link+8,indices)
                self.put(self.blob+0x300+c['data_offset'],c['index']%12)
                for i in range(12):self.put(indices+i*4,c['index'])
            else:self.put(parameter+0x40,0)
            self.put(POSITION,c['start'])
            result=self.call(kind,[c['group'],c['record'],0 if c['null_buffer']else self.buffer,POSITION],EFFECT)
        return self.snapshot(result)
    def native(self,mode):
        c=next(cases(1));c.update(effect=b'\xa5'*0x130,fail_allocation=0,callback=False)
        self.active_kind='native';self.prepare(c);self.getters=Counter();self.position=POSITION;self.mutes=0
        self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
        self.put(EFFECT+0x98,2);self.put(EFFECT+0xa0,0);self.put(EFFECT+0xa8,2,2)
        def array(count,size,constructor):
            cookie=self.alloc(count*size+4);self.put(cookie,count);start=cookie+4;self.u.mem_write(start,b'\xa5'*(count*size))
            for i in range(count):self.call(constructor,receiver=start+i*size)
            return start
        parameters=array(2,0x50,'parameter_ctor');groups=array(4,0x64,'group_construct');references=array(3,0x18,'reference_construct')
        self.put(EFFECT+0x9c,parameters);self.put(EFFECT+0x2c,groups);self.put(EFFECT+0x18,references);self.put(EFFECT+0x14,3);self.put(EFFECT+0x34,4)
        records=array(1,0x64,'record_construct');self.put(groups+4,records);self.put_half(groups+0x36,1);self.put_half(groups+0x38,1)
        blob=self.alloc(0x400);self.u.mem_write(blob,bytes(0x400));self.put(EFFECT+0x20,blob);self.put(EFFECT+0x1c,0x400)
        table=self.alloc(8);self.put(table,parameters,parameters+0x50);self.put(EFFECT+0xb0,table);self.put(parameters+0x30,0x100);self.put(blob+0x100,1)
        reference=references+0x18;self.reference=reference;self.put(reference,3 if mode==6 else 4,0,0,0,0xa5a50000,0)
        size=(0,1,3,4,7,16,32,63)[mode]if mode<8 else 0
        data=bytearray((i*17+3)&255 for i in range(size))
        if size and mode!=7:data[-1]=0
        words=[1,size]if mode<8 else[0];payload=struct.pack('<'+'I'*len(words),*words)+data
        buffer=self.alloc(16);stream=self.alloc(len(payload));self.put(buffer,0x7b711c,1,len(payload),stream);self.u.mem_write(stream,payload);self.put(EFFECT+0x28,buffer);self.put(POSITION,0)
        if mode<8:result=self.call('resource',[buffer,POSITION],EFFECT)
        else:
            self.put_half(records+0x52,147 if mode==9 else 0 if mode==10 else 146);self.put(records+0x1c,0);self.put(records+0x24,0)
            self.put_half(EFFECT+0x30,1);self.put(EFFECT+0x34,1 if mode==11 else 4)
            result=self.call('shader_record',[0,0,buffer,POSITION],EFFECT)
        output=[result,self.word(POSITION),self.half(EFFECT+0x30),self.word(reference),int(bool(self.word(reference+4))),int(bool(self.word(reference+8))),self.word(reference+0x10),self.word(reference+0x14)]
        resource=self.word(reference+4);length=self.word(resource+8)if resource else 0;digest=2166136261
        if resource:
            for byte in self.u.mem_read(self.word(resource+12),length):digest=((digest^byte)*16777619)&0xffffffff
        output.extend([length,digest,self.word(groups+0xc),self.half(groups+0x34)])
        output.extend([self.word(records),self.half(records+0x52),self.half(records+0x54),self.half(records+0x56),self.word(records+0x58),self.word(records+0x48)])
        target=groups+0x64;output.extend(self.half(target+o)for o in(0x36,0x38,0x18,0x1a));record=self.word(target+4)
        output.extend([int(bool(record))])
        if record:output.extend([self.word(record),self.half(record+0x52),self.half(record+0x54),self.half(record+0x56),self.word(record+0x58),self.word(record+0x30)&0x1f])
        self.call('destroy',receiver=EFFECT);assert not self.live,('native fixture cleanup',self.live)
        return output


def cases(n):
    rng=random.Random(0x66520d)
    for i,c in enumerate(rd.cases(n)):
        code=bytearray(rng.randbytes(64));code[31]=0 if i%3 else 1;code[0]=0 if i%5 else 1
        c.update(code_bytes=bytes(code),effect_flags=rng.getrandbits(32),resource_kind=(4,15,16,0,3,5,14,17)[(i//2)%8],
            reference_flags=rng.getrandbits(32),gpu_result=(0,1,0x80004005,0x8876086c,0xffffffff)[i%5],shader_token=0x12345678+i,
            write_shader=bool(i%3),getter_device=i%4==0,mute_flag=i%11==0,mute_output=i%7==0,
            existing04=0 if i%7 else 0x12345678,existing08=0 if i%13 else 0x23456789,index=(i//3)%10,wire_index=(i//3)%14,
            size=(0,1,3,4,7,16,32,63)[(i//4)%8],target_group=(i//6)%8,resource_count=(0,1,5,12)[(i//9)%4],
            start=(0,1,0xffffffff)[(i//23)%3],limit=(0,1,2,3,4,8,16)[(i//13)%7],limit_mode=(i//7)%3,
            null_buffer=i%19==0,callback=False,stream_callback=(0,0,1,2,3)[(i//11)%5],span=0x2000,code=(146,147,0,12,178,179)[i%6],group=0,record=(i//2)%2,
            parameter=i%2,data_offset=(i//3)%3*4,group_start=(1,2,7,8,0xffff)[(i//5)%5],group_limit=(0,1,4,8)[(i//11)%4],
            group_high=rng.randrange(0x10000),storage_offset=(0,1,3,7,0xfffffffe)[(i//17)%5],group_uses=(0,1,0xffff)[(i//29)%3],shared=(i//5)%2==0,preexisting=(i//7)%2==0,target_flags=rng.getrandbits(32))
        c['fail_allocation']=0 if i%4 else 1+(i//8)%5
        # Ordinary successful streams complement all truncation/index fixtures.
        if i%3:c.update(start=0,limit_mode=0,null_buffer=False,wire_index=c['index'],resource_count=12,group_start=1,group_limit=8)
        if i%29==0:
            size=(1,3,7,32)[(i//29)%4];code=bytearray(c['code_bytes']);code[size-1]=0
            c.update(start=0,limit_mode=0,null_buffer=False,wire_index=c['index'],resource_count=12,size=size,resource_kind=4,existing04=0,existing08=0,stream_callback=0,fail_allocation=0,code_bytes=bytes(code))
        if i%31==0:
            code=bytearray(c['code_bytes']);code[3]=0
            c.update(start=0,limit_mode=1,limit=3,null_buffer=False,wire_index=c['index'],resource_count=12,size=4,resource_kind=4,existing04=0,existing08=0,stream_callback=3,fail_allocation=0,code_bytes=bytes(code))
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();outcomes=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect shader resource mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect shader resource mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[str(x[2])]+=1;outcomes[str((kind,x[2]))]+=1
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),returns_by_body=dict(outcomes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--native-dump',type=Path);p.add_argument('--mutation-checks',action='store_true')
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert args.cases>0 and hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind])
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocator/free, debug mute and GPU shader creation controlled; no native shader success fabricated',
        'finite mapped streams/graphs; arbitrary malformed pointers, exceptions and giant loops not established',
        'main deserializer, relocation, shared-pool producer, remaining shader payload readers and GPU activation remain open'])
    if args.mutation_checks:
        mutations={
            'omit-mute-enable':('vertex','UG2FxDebugMute(1);','(void)0;'),
            'omit-mute-disable':('pixel','UG2FxDebugMute(0);','(void)0;'),
            'ignore-shader-disable-flag':('vertex','if (!(reinterpret_cast<unsigned char *>(&value118)[1]&2))','if (true)'),
            'fail-success-zero-hresult':('pixel','if (result<0) {','if (result<=0) {'),
            'erase-reference-other-flags':('vertex','reference->value14|=3;','reference->value14=3;'),
            'omit-failed-shader-output-clear':('pixel','reference->value04=0;','(void)0;'),
            'wrong-failed-reference-mask':('vertex','reference->value14&=0xfffffffc;','reference->value14&=0xfffffffe;'),
            'reread-class-after-getter':('vertex','UG2FxCreateVertexShader(dispatch,value0C,code','UG2FxCreateVertexShader(*reinterpret_cast<unsigned int *>(value0C),value0C,code'),
            'wrong-shader-api-slot':('pixel','UG2FxCreatePixelShader(dispatch,value0C,code','UG2FxCreateVertexShader(dispatch,value0C,code'),
            'omit-cursor-advance':('resource','++*position;','(void)0;'),
            'divide-buffer-length':('resource','unsigned int limit=UG2FxBufferLengthVirtual(buffer);','unsigned int limit=UG2FxBufferLengthVirtual(buffer)/4;'),
            'strict-header-word-bound':('resource','*position+1<=limit','*position+1<limit'),
            'allow-resource-end-index':('resource','value14<=index','value14<index'),
            'wrong-temporary-byte-size':('resource','UG2FxMalloc(size)','UG2FxMalloc(size+1)'),
            'floor-payload-word-count':('resource','(size>>2)+((size&3)!=0)','size>>2'),
            'omit-partial-word-copy':('resource','for (i=size&0xfffffffc;i<size;++i) temporary[i]=source[i];','(void)0;'),
            'omit-string-terminator-check':('resource','if (!temporary[size-1])','if (true)'),
            'allow-one-existing-resource':('resource','if (!reference->value04 && !reference->value08)','if (!reference->value04 || !reference->value08)'),
            'allow-resource-kind14':('resource','reference->value00>14','reference->value00>13'),
            'wrong-string-buffer-slot':('resource','reinterpret_cast<D3DXEffectBuffer **>(&reference->value04)','reinterpret_cast<D3DXEffectBuffer **>(&reference->value08)'),
            'skip-zero-resource-return':('resource','if (!size) goto finish;','(void)0;'),
            'omit-resource-temporary-free':('resource','if (temporary) UG2FxFree(temporary);','(void)0;'),
            'cache-final-cursor-before-getter':('resource','finalLimit=UG2FxBufferLengthVirtual(buffer);','unsigned int saved=*position; finalLimit=UG2FxBufferLengthVirtual(buffer);'),
            'omit-lazy-kind-filter':('shader_record','if (kind!=16 && kind!=15) return 0;','(void)0;'),
            'inclusive-lazy-group-bound':('shader_record','if (value30<value34)','if (value30<=value34)'),
            'overwrite-group-index-high-word':('shader_record','*reinterpret_cast<unsigned short *>(&reference->value10)=value30;','reference->value10=value30;'),
            'omit-group-count-increment':('shader_record','++value30;','(void)0;'),
            'omit-lazy-record-grow':('shader_record','int result=target->ResizeRecords(1);','int result=target->ResizeRecords(0);'),
            'swap-lazy-shader-code':('shader_record','target->array04->value52=0x92;','target->array04->value52=0x93;'),
            'wrong-lazy-shader-tag':('shader_record','target->array04->value58=0x0b000000;','target->array04->value58=0x0a000000;'),
            'omit-lazy-owned-flag':('shader_record','target->array04->value30|=1;','(void)0;'),
            'short-lazy-metadata-copy':('shader_record','for (unsigned int i=0;i<10;++i)','for (unsigned int i=0;i<7;++i)'),
            'skip-zero-shader-return':('shader_record','if (!size) return result;','(void)0;'),
            'ignore-shared-resource-map':('shader_record','index=reinterpret_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(parameter->value40)->data08)[shared];','index=shared+1;'),
            'omit-source-record-disable':('shader_record','record->value00=0;record->value52=0xb3;','record->value00=1;record->value52=0xb3;'),
            'wrong-indirect-record-tag':('shader_record','record->value58=0x0e000000;','record->value58=0x0f000000;'),
            'omit-group-use-increment':('shader_record','++group->value34;','(void)0;'),
            'align-indirect-storage':('shader_record','unsigned int offset=(group->value0C&3)+(group->value0C>>2)*4;','unsigned int offset=(group->value0C+3)&0xfffffffc;'),
            'wrong-indirect-storage-size':('shader_record','group->value0C+=4;','group->value0C+=8;'),
        }
        rejected=[];selected=set(SPANS)if args.kind=='all'else{args.kind}
        for name,(kind,before,after)in mutations.items():
            if kind not in selected:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after,1 if name=='cache-final-cursor-before-getter'else-1)
            if name=='cache-final-cursor-before-getter':body=body.replace('unsigned int saved=*position;','saved=*position;').replace('unsigned int finalLimit;','unsigned int finalLimit; unsigned int saved;').replace('if (*position<=finalLimit)','if (saved<=finalLimit)',1)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect shader resource mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXSR1';at=8;seen=set();a,b=Machine(image),Machine(image,linked,args.gcc)
        while at<len(raw):
            mode,count=struct.unpack_from('<2I',raw,at);at+=8;words=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert mode in range(12)and mode not in seen;seen.add(mode)
            x=a.native(mode);y=b.native(mode);assert x==y==words,('native shader resource mismatch',mode,x,y,words)
        assert at==len(raw)and len(seen)==12
        report['native_replay']=dict(fixtures=12,all_normalized_resource_and_lazy_record_results_match=True,all_fixtures_destroyed=True)
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
