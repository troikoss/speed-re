#!/usr/bin/env python3
"""Compare original shader-preparation dependencies and CPU metadata lifecycles.

Original/rebuilt CTAB, use-ring, buffer, COM and cleanup bodies execute. Shader
GetDevice/GetFunction and GPU creation are controlled only in the emulator.
Native probes exercise the CPU helpers; no GPU shader is fabricated.
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
import verify_pc_effect_deserialize as ds

md=ds.md;cp=ds.sp.v.hc.cp;ROOT,EXE,LIB=ds.ROOT,ds.EXE,ds.LIB
SOURCE=LIB/'EffectPreparation.cpp';BUILD=ROOT/'build/analysis/effect-preparation';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-preparation.json'
ARENA,HEAP,EFFECT,STOP=ds.ARENA,ds.HEAP,ds.EFFECT,ds.STOP
CURSOR,TYPES=ARENA+0xa000,ARENA+0xa100
SPANS=dict(vector_only=(0x65b098,90),bind_prepare=(0x660f9c,518),unlink_use=(0x6611a2,127),capture_vertex=(0x661221,242),capture_pixel=(0x661313,242))
md.SOURCES += [SOURCE,ROOT/'src/Platform/Native/NativeFxShaderReadback.cpp'];md.BUILD=BUILD
md.FREE['vector_only']='D3DXEffectVectorOnly';md.STDCALL['vector_only']=4
md.MEMBERS.update({k:('D3DXEffectObject',v)for k,v in dict(bind_prepare='BindPreparationConstants',unlink_use='UnlinkParameterUse',capture_vertex='CaptureVertexBytecode',capture_pixel='CapturePixelBytecode',create_vertex='CreateVertexShader',create_pixel='CreatePixelShader').items()})


def type_words(c):
    def node(shape,children=()):return [c['type_kind'],shape,0x87654321,0,c['type_elements']]+([c['rows'],c['columns']]if shape in(0,1,2,3)else[len(children)]+sum(children,[])if shape==5 else[])
    shape=c['type_shape']
    if c['type_depth']:return node(5,[node(0),node(5,[node(shape),node(1)]),node(1)])
    return node(shape,[node(0),node(1)]if shape==5 else[])


def cases(n):
    rng=random.Random(0x660f9c)
    for i,c in enumerate(cp.cases(n)):
        c.update(type_kind=rng.getrandbits(32),type_elements=rng.getrandbits(32),type_shape=(0,1,2,3,4,5,6,0xffffffff)[i%8],type_depth=i%3,
            use_count=1+(i//3)%3,use_index=(i//7)%3,head_index=(i//11)%3,owner_alias=bool(i%2),temporary_grids=i%3!=0,suppress_cleanup=bool(i%5),
            read_bytes=(0,1,3,4,8,16,31,64)[i%8],code_present=i%3!=0,code_size=(0,1,4,8,32,64)[(i//5)%6],
            query_result=(0,0,0,0x80004005)[i%4],device_result=(0,0,0x80004005)[(i//3)%3],read_result=(0,0,0x80004005)[(i//7)%3],
            device_alias=(i//11)%5,write_query=i%13!=0,write_device=i%17!=0,device_resizes=i%19==0,change_dispatch=i%23==0,
            reference_flags=rng.getrandbits(32),shader_fill=bytes(rng.randrange(256)for _ in range(80)))
        c['fail_allocation']=(0,0,0,1,2,3)[(i//2)%6];yield c


class Machine(ds.Machine):
    def __init__(self,image,linked=None,gcc=False):
        self.shader=0;self.capture_active=False
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        if linked:
            for kind,name,n in(('shader-device','UG2FxShaderDevice',3),('shader-function','UG2FxShaderFunction',4)):
                at=linked['exports'][name if gcc else'_'+name+'@'+str(n*4)]
                self.u.hook_add(UC_HOOK_CODE,self.shader_api,user_data=(kind,True),begin=at,end=at)
        for key,at,n in(('link',0x65bd1e,5),('root',0x65be0d,2),('create_vertex',0x65b970,1),('create_pixel',0x65b9ce,1)):
            address=linked['entries'][key]if linked else at
            self.u.hook_add(UC_HOOK_CODE,self.observe_prepare,user_data=(key,n),begin=address,end=address)
    def observe_prepare(self,u,address,size,data):
        key,n=data;args=self.args(n+1)if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),*self.args(n))
        self.events.append((key,args,self.state()))
    def getter(self,u,address,size,kind):
        obj=self.args(1)[0]
        if self.capture_active and not self.is_linked and obj==self.shader:
            self.shader_api(u,address,size,('shader-device'if kind=='pointer'else'shader-function',False));return
        super().getter(u,address,size,kind)
        if self.capture_active and kind=='pointer'and self.case['change_dispatch']and obj!=self.shader:
            self.put(self.shader,0x12345678)
    def shader_api(self,u,address,size,data):
        kind,bridge=data;args=self.args((3 if kind=='shader-device'else 4)if bridge else(2 if kind=='shader-device'else 3))
        dispatch,obj,*args=args if bridge else(0x7b711c,*args);c=self.case
        if kind=='shader-device':
            output,=args;self.events.append((kind,(dispatch,obj,self.word(output)),self.state()))
            if c['write_device']:
                device=(0,self.device,self.other_device,self.shader,self.old_code)[c['device_alias']];self.put(output,device)
                if device:self.put(device+4,self.word(device+4)+1)
            if c['device_resizes']:self.put(self.size_pointer,c['read_bytes']+1)
            result=c['device_result'];pop=12 if bridge else 8
        else:
            output,bytes_pointer=args;self.events.append((kind,(dispatch,obj,output,self.word(bytes_pointer)),self.state()))
            self.put(self.shader,0x7b711c)
            if not output:
                self.size_pointer=bytes_pointer
                if c['write_query']:self.put(bytes_pointer,c['read_bytes'])
                result=c['query_result']
            else:
                count=min(self.word(bytes_pointer),c['read_bytes']);self.u.mem_write(output,c['shader_fill'][:count]);result=c['read_result']
            pop=16 if bridge else 12
        self.ret(result,pop)
    def base(self,c,native=False):
        self.capture_active=False;self.shader=0;cp.Machine.setup(self,c,native)
        self.setup_debug(dict(c,modules=0,open_result=2,cached_exports=0,setting=-1));self.put(EFFECT+0x118,0x200);self.case=dict(c,modules=0,open_result=2,switch_module=False,switch_proc=False)
    def bind_fixture(self,c,native=False):
        self.base(c,native);buffer=self.buffer(0x68+len(self.payload));self.data=self.word(buffer+12)
        self.u.mem_write(self.data,b'\xa5'*0x68+self.payload);self.put(self.record+0x38,buffer)
        self.put_half(self.record+0x50,c['already_bound'])
    def unlink_fixture(self,c,native=False):
        self.base(c,native);self.put_half(self.record+0x50,c['use_count']);self.put_half(self.record+0x4e,c['use_count']);self.uses=self.array(c['use_count'],0x20,'use_construct');self.put(self.record+0x34,self.uses)
        parameter=self.parameters;self.owner=self.parameters+(0x50 if c['owner_alias']else 0);self.put(parameter+0x14,1 if c['owner_alias']else 0)
        for i in range(c['use_count']):self.call('link',[c['group'],c['record'],i,parameter,0x12345678+i],EFFECT)
        self.put(self.owner+0x34,self.uses+(c['head_index']%c['use_count'])*0x20)
        self.put(EFFECT+0x118,0x800 if c['suppress_cleanup']else 0)
        self.put(EFFECT+0xa0,2)
        if c['temporary_grids']:
            for offset in(0xb4,0xc0):
                grid=self.alloc(24);self.put(EFFECT+offset,grid)
                for i in range(6):slot=self.alloc(8);self.put(slot,0x11223344,0);self.put(grid+i*4,slot)
    def capture_fixture(self,c,native=False):
        self.base(c,native);self.device=self.buffer(8);self.other_device=self.buffer(8);self.put(EFFECT+0xc,self.device)
        self.shader=self.buffer(16);self.reference=self.array(1,0x18,'reference_construct');self.old_code=self.buffer(c['code_size'])if c['code_present']else 0
        self.put(self.reference,16 if self.active_kind=='capture_vertex'else 15,self.shader,self.old_code,0x87654321,0xabcdef01,c['reference_flags'])
        self.put(EFFECT+0x14,1,self.reference);self.requests=0;self.events=[];self.capture_active=True
        # A missing size write must take the first error path, avoiding an
        # unbounded pointer-sized allocation outside these bounded fixtures.
        if not c['write_query']:self.case=dict(self.case,query_result=0x80004005)
    def run(self,kind,c):
        self.active_kind=kind
        if kind=='vector_only':
            self.prepare(c);self.getters=Counter();self.setup_debug(dict(c,modules=0,open_result=2,cached_exports=0,setting=-1));self.u.mem_write(TYPES,ds.pack(type_words(c)));self.put(CURSOR,TYPES);self.requests=0;self.events=[];result=self.call(kind,[CURSOR])
        elif kind=='bind_prepare':
            self.bind_fixture(c);self.requests=0;self.events=[];result=self.call(kind,[c['group'],c['record'],self.data],EFFECT)
        elif kind=='unlink_use':
            self.unlink_fixture(c);self.requests=0;self.events=[];result=self.call(kind,[c['group'],c['record'],c['use_index']%c['use_count']],EFFECT)
        else:
            self.capture_fixture(c)
            result=self.call(kind,[self.reference],EFFECT);self.capture_active=False
        return self.snapshot(result)


def native_cases():
    for mode in range(32):
        c=next(cases(1));c.update(group=0,record=0,storage_offset=mode%8,already_bound=1 if mode==17 else 0,
            shape=0,elements=1,rows=2,columns=3,record_flags=0x1234567b,record_tag=0,parameter_flags=mode%3,shared=False,name_mode=0,fail_allocation=0,
            version=0x11110200 if mode==19 else 0x46580200,constants=(mode-8)%4,bank=mode%4,same_bank=mode%2,
            registers=2,start_register=1,register_starts=None,vary_registers=False,default_offset=0,bad_name=False,missing_name=mode==15,missing_index=1 if mode==14 else None,
            table_words=192,short_comment=False,omit_table=mode==18,omit_instructions=False,omit_literals=False,
            type_kind=mode,type_elements=7,type_shape=(0,1,2,3,0,1,2,5)[mode%8],type_depth=mode//4 if mode<8 else 0,
            use_count=1+(mode%3),use_index=(mode//3)%3,head_index=(mode//5)%3,owner_alias=bool(mode%2),temporary_grids=mode%4!=0,suppress_cleanup=bool(mode%3),
            effect=b'\xa5'*0x130,arena=b'\xa5'*md.core.SIZE,heap=b'\xa5'*md.core.HSIZE)
        yield mode,0 if mode<8 else 1 if mode<20 else 2,c


def capture_graph(m,kind,c):
    c=dict(c,shared=False,name_mode=0,parameter_flags=0)
    m.active_kind=kind;m.capture_fixture(c,True);result=m.call(kind,[m.reference],EFFECT);m.capture_active=False
    m.call('destroy',receiver=EFFECT);m.call_buffer_release(m.other_device)
    assert not m.live,('effect preparation mismatch','capture cleanup',kind,m.live)
    return m.snapshot(result)


def native_graph(m,mode,operation,c):
    m.active_kind='native';words=[]
    if not operation:
        m.prepare(c);m.getters=Counter();m.setup_debug(dict(c,cached_exports=0,setting=-1));m.u.mem_write(TYPES,ds.pack(type_words(c)));m.put(CURSOR,TYPES)
        result=m.call('vector_only',[CURSOR]);return [result,m.word(CURSOR)-TYPES]
    if operation==1:
        m.bind_fixture(c,True);result=m.call('bind_prepare',[0,0,m.data],EFFECT)
        words.extend((result,m.word(m.group+0xc),m.word(m.record),m.word(m.record+0x30),m.half(m.record+0x4e),m.half(m.record+0x50),int(bool(m.word(m.record+0x3c)))))
        words.extend(m.word(m.data+i*4)for i in range(26))
        uses=m.word(m.record+0x34);words.append(m.word(uses-4)if uses else 0)
        for i in range(3):
            head=m.word(m.parameters+i*0x50+0x34);at=head;links=[]
            if head:
                while True:
                    entry=m.word(at+0x10);links.extend((m.word(at),m.word(at+4),m.half(at+0x14),m.half(at+0x16),m.word(entry+4),m.word(entry+8)));at=m.word(at+0x1c)
                    if at==head:break
                    assert len(links)<100
            words.append(len(links)//6);words.extend(links)
    else:
        m.unlink_fixture(c,True);result=m.call('unlink_use',[0,0,c['use_index']%c['use_count']],EFFECT)
        normalize=lambda p:(p-m.uses)//0x20+1 if p else 0
        words.extend((result,normalize(m.word(m.owner+0x34)),int(bool(m.word(EFFECT+0xb4))),int(bool(m.word(EFFECT+0xc0)))))
        for i in range(c['use_count']):
            at=m.uses+i*0x20;words.extend((normalize(m.word(at+0x18)),normalize(m.word(at+0x1c)),m.word(at),m.word(at+4)))
    m.call('destroy',receiver=EFFECT);assert not m.live,('effect preparation mismatch','native cleanup',mode,m.live)
    return [x&0xffffffff for x in words]


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();returns=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect preparation mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:8]
                event=next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect preparation mismatch',kind,i,diffs,x[2],y[2],event,len(x[3]),len(y[3])))
            counts[kind]+=1;returns[str((kind,x[2]))]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),returns=dict(returns),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind]);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.write_native_fixtures:
        content=b'UG2FXQF1'+ds.pack([32])
        for mode,operation,c in native_cases():
            tree=type_words(c);payload=cp.payload(c)
            config=[mode,operation,c['storage_offset'],c['already_bound'],c['parameter_flags'],c['use_count'],c['use_index'],c['head_index'],int(c['owner_alias']),int(c['temporary_grids']),int(c['suppress_cleanup']),len(tree),len(payload)]
            content+=ds.pack(config+tree)+payload
        args.write_native_fixtures.write_bytes(content)
    if args.kind=='all'and not args.replace:
        a,b=Machine(image),Machine(image,linked,args.gcc);native=[]
        for mode,operation,c in native_cases():
            x=native_graph(a,mode,operation,c);y=native_graph(b,mode,operation,c)
            assert x==y and a.snapshot(tuple(x))==b.snapshot(tuple(y)),('effect preparation mismatch','native',mode,x,y)
            native.append(x)
        report['native_chains']=len(native)
        for i,c in enumerate(cases(64)):
            for kind in('capture_vertex','capture_pixel'):
                x=capture_graph(a,kind,c);y=capture_graph(b,kind,c)
                assert x==y,('effect preparation mismatch','capture lifecycle',i,kind)
        report['capture_lifecycles']=128
        if args.native_dump:
            raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXQD1';at=8
            for i,expected in enumerate(native):
                row,count=struct.unpack_from('<2I',raw,at);at+=8;values=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
                assert row==i and expected==values,('native effect preparation mismatch',i,expected,values)
            assert at==len(raw);report['native_replay']=len(native)
    if args.mutation_checks:
        mutations={
            'vector-accept-matrix':('vector_only','if (shape<2)','if (shape<4)'),
            'vector-short-cursor':('vector_only','*type=description+7;','*type=description+6;'),
            'vector-ignore-child-failure':('vector_only','if (!D3DXEffectVectorOnly(type)) return 0;','D3DXEffectVectorOnly(type);'),
            'vector-wrong-struct-start':('vector_only','*type=description+6;','*type=description+5;'),
            'binding-repeat-guard':('bind_prepare','if (record->value50 || record->array34)','if (false)'),
            'binding-ready-bit':('bind_prepare','record->value30&=0xfffffffd;','record->value30&=0xfffffffe;'),
            'binding-token-start':('bind_prepare','data+26,','data+25,'),
            'binding-cookie-count':('bind_prepare','UG2FxMalloc(count*0x20+4)','UG2FxMalloc(count*0x40+4)'),
            'binding-doubled-capacity':('bind_prepare','reinterpret_cast<unsigned int *>(record->value3C)[3]<<1','reinterpret_cast<unsigned int *>(record->value3C)[3]'),
            'binding-missing-name-fatal':('bind_prepare','if (parameter) {','if (!parameter) return static_cast<int>(0x80004005);if (parameter) {'),
            'binding-slot-counter':('bind_prepare','unsigned int index=record->value4E++;','unsigned int index=++record->value4E;'),
            'binding-mask-width':('bind_prepare','maximum[0])+maximum[2])*4','maximum[0])+maximum[2])*2'),
            'binding-wrong-storage-field':('bind_prepare','data[13]=offset;','data[12]=offset;'),
            'binding-capacity-floor':('bind_prepare','if (group->value0C<=limit) group->value0C=limit;','(void)0;'),
            'unlink-previous-neighbor':('unlink_use','use->previous18->next1C=use->next1C;','(void)0;'),
            'unlink-direct-owner':('unlink_use','D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];','D3DXEffectParameter *owner=parameter;'),
            'unlink-head-predecessor':('unlink_use','reinterpret_cast<unsigned int>(use->next1C);','reinterpret_cast<unsigned int>(use->previous18);'),
            'unlink-singleton-head':('unlink_use','use->next1C==use ? 0 :','false ? 0 :'),
            'unlink-self-reset':('unlink_use','use->previous18=use;use->next1C=use;','(void)0;'),
            'unlink-temp-gate':('unlink_use','!(reinterpret_cast<unsigned char *>(&value118)[1]&8) && valueB4','(reinterpret_cast<unsigned char *>(&value118)[1]&8) && valueB4'),
            'capture-clear-request-bit':('capture_vertex','reference->value14&=0xfffffffe;','(void)0;'),
            'capture-size-initial-value':('capture_vertex','size=reinterpret_cast<unsigned int>(reference),device=0','size=0,device=0'),
            'capture-first-error':('capture_vertex','if (result<0) goto finish;','if (false) goto finish;'),
            'capture-grow-equal':('capture_vertex',')<size) {',')<=size) {'),
            'capture-old-buffer-release':('capture_vertex','UG2FxComRelease(reference->value08);reference->value08=0;','reference->value08=0;'),
            'capture-dispatch-reread':('capture_vertex','result=UG2FxShaderFunction(dispatch,reference->value04,data,&size);','result=UG2FxShaderFunction(*reinterpret_cast<unsigned int *>(reference->value04),reference->value04,data,&size);'),
            'capture-device-comparison':('capture_vertex','device!=value0C','device==value0C'),
            'capture-read-failure-gate':('capture_vertex','result>=0 && device!=value0C','device!=value0C'),
            'capture-shader-release':('capture_vertex','UG2FxComRelease(reference->value04);reference->value04=0;','reference->value04=0;'),
            'capture-device-cleanup':('capture_vertex','if (device) UG2FxComRelease(device);','if (result>=0 && device) UG2FxComRelease(device);'),
            'capture-pixel-caller':('capture_pixel','FX_CAPTURE_BYTECODE(CreatePixelShader)','FX_CAPTURE_BYTECODE(CreateVertexShader)'),
        }
        source=SOURCE.read_text();rejected=[]
        for name,(kind,before,after)in mutations.items():
            if args.kind!='all'and args.kind!=kind:continue
            assert before in source,(name,before)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(source.replace(before,after,1))
            command=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect preparation mismatch'in run.stdout,(name,run.stdout[-3000:]);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
