#!/usr/bin/env python3
"""Compare complete UG2 vertex/pixel group preparation and its CRT sorting.

Recovered parsers, readback callers, constant/use construction, sorting and
storage execute. Shader interface services are controlled bytecode fixtures;
GPU creation stays disabled by the original flag. Native CPU probes use their
own test-only readback adapters, never a production successful GPU shader.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_effect_preparation as p

md,ds,cp=p.md,p.ds,p.cp;ROOT,EXE,LIB=p.ROOT,p.EXE,p.LIB
SOURCE=LIB/'EffectPrepareGroups.cpp';BODY=LIB/'EffectPrepareGroup.inc';SORT=LIB/'EffectSort.cpp';BUILD=ROOT/'build/analysis/effect-prepare-groups';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-prepare-groups.json'
ARENA,HEAP,EFFECT,STOP=p.ARENA,p.HEAP,p.EFFECT,p.STOP
ARRAY,ENTRIES=ARENA+0xa000,ARENA+0xb000;COMPARE=STOP+0xa000
SPANS=dict(prepare_a=(0x666df7,3930),prepare_b=(0x667d51,3951),compare_constants=(0x65b64b,53),short_sort=(0x75cc00,109),sort=(0x75cc70,643))
md.SOURCES += [SOURCE,SORT];md.BUILD=BUILD;md.core.GLOBALS['ug2FxConstantOrder7B5048']=0x7b5048
md.MEMBERS.update(prepare_a=('D3DXEffectObject','PrepareGroupA'),prepare_b=('D3DXEffectObject','PrepareGroupB'),records_delete=('D3DXEffectRecord64','DeleteArray'))
md.FREE.update(compare_constants='D3DXEffectCompareConstants',short_sort='D3DXEffectShortSort',sort='D3DXEffectSort');md.CDECL.update(('compare_constants','short_sort','sort'));md.VOID.update(('short_sort','sort'))


def shader_program(c):
    code=bytearray(cp.payload(c));words=list(struct.unpack('<'+'I'*(len(code)//4),code));at=1
    while at<len(words):
        token=words[at];at+=1
        if token&0xffff==0xffff:break
        if token&0xffff==0xfffe:
            count=(token>>16)&0x7fff
            if count>1 and words[at]==0x42415443:
                table=(at+1)*4
                for i in range(c['constants']):struct.pack_into('<I',code,table+28+i*20+12,0x60)
                struct.pack_into('<I',code,table+0x60,c['table_class'])
                for i in range(32):struct.pack_into('<I',code,table+0x200+i*4,0x3f000000+i)
                break
            at+=count
    code=bytes(code)
    if not c['pres']:return code
    pc=dict(c,version=0x46580200,constants=c['pres_constants'],omit_table=False,missing_name=False,missing_index=None,default_offset=0,bank=2,same_bank=True,registers=1,start_register=0,register_starts=None,vary_registers=False)
    data=list(struct.unpack('<'+'I'*(len(cp.payload(pc))//4),cp.payload(pc)))
    rsi=[0,0,0,*c['pres_counts']]
    for count in c['pres_counts']:rsi += [1,0,count]if count else[0]
    data[-1:-1]=[((len(rsi)+1)<<16)|0xfffe,0x49535250,*rsi]
    if c['bad_pres']:data[0]=0x11110200
    words=list(struct.unpack('<'+'I'*(len(code)//4),code));words[-1:-1]=[((len(data)+1)<<16)|0xfffe,0x53455250,*data]
    return ds.pack(words)


def cases(n):
    rng=random.Random(0x666df7)
    for i,c in enumerate(p.cases(n)):
        c.update(sort_count=(0,1,2,7,8,9,10,17,31,32,63,96,257)[i%13],sort_width=(0,1,2,3,4,7,8,12)[i%8],sort_mode=(i//3)%5,short_count=1+i%9,
            compare_bank=i%4,compare_other_bank=(i//3)%4,compare_start=(0,1,0x7fff,0xfffe,0xffff)[(i//5)%5],compare_other_start=(0xffff,1,0x8000,0,0xfffe)[(i//7)%5],
            version=0x46580200,constants=i%4,bank=(i//3)%4,same_bank=bool(i%2),registers=1+(i//7)%3,start_register=(i//11)%4,register_starts=None,vary_registers=bool(i%3),
            default_offset=0x200 if i%3 else 0,table_class=(1,2,3)[(i//5)%3],bad_name=False,missing_name=i%13==0,missing_index=1 if i%11==0 else None,table_words=192,short_comment=False,
            instructions=0,operands=0,terms=0,omit_table=i%17==0,omit_instructions=False,omit_literals=False,negative_token=False,legacy_def=False,instruction_skip=False,short_comment_unused=False,tail=0,
            group=0,record=0,already_bound=0,parameter_flags=i%4,shared=False,name_mode=0,record_flags=rng.getrandbits(32),record_tag=0,
            shape=(0,1,2,3,5)[(i//7)%5],elements=(0,1,2)[(i//9)%3],rows=1+(i//11)%4,columns=1+(i//13)%4,
            pending=i%5!=0,has_shader=i%7!=0,has_code=i%9!=0,valid_code=i%11!=0,have_bytecode=i%19!=0,grids=i%4!=0,suppress_grids=i%3==0,pres=i%3==0,pres_counts=(i%3,(i//3)%3,(i//7)%3),pres_constants=(i//5)%3,bad_pres=i%29==0,
            shared_graph=i%3==1,group_flags=rng.getrandbits(32),capacity=(2,8)[i%2],repeat=bool(i%4),change_dispatch=False,query_result=0,device_result=0,read_result=0,write_query=True,write_device=True,device_alias=1,device_resizes=False)
        c['fail_allocation']=(0,0,0,0,1,2,3,4,7,10)[(i//2)%10]
        if not c['valid_code']:c['version']=0x11110200
        if 1<=i<=8:
            c.update(shape={1:0,2:1,3:2,4:2,5:2,6:5,7:5,8:3}[i],elements=1 if i==3 else 2,columns=3 if i in(3,5,7)else 4,
                table_class=3,constants=3,bank=2,same_bank=True,registers=3,start_register=0,vary_registers=True,missing_name=False,missing_index=None,
                pending=True,has_shader=True,valid_code=True,have_bytecode=True,pres=False,bad_pres=False,omit_table=False,repeat=False,fail_allocation=0)
        if 9<=i<=13:
            c.update(constants=3,bank=1 if i<=10 else 0 if i<=12 else 3,same_bank=True,registers=2,start_register=0,vary_registers=True,
                default_offset=0 if i in(9,11,13)else 0x200,missing_name=False,missing_index=None,pending=True,has_shader=True,has_code=True,
                valid_code=True,version=0x46580200,have_bytecode=True,pres=False,bad_pres=False,omit_table=False,repeat=False,fail_allocation=0)
        if i==14:c['have_bytecode']=False
        c['program']=shader_program(c);yield c


class Machine(p.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()},records_delete=0x664f63)
        self.u.hook_add(UC_HOOK_CODE,self.compare,begin=COMPARE,end=COMPARE)
        for key in('capture_vertex','capture_pixel'):
            self.u.hook_add(UC_HOOK_CODE,self.capture_entry,user_data=key,begin=self.entries[key],end=self.entries[key])
        for kind in('short_sort','sort','compare_constants'):
            at=self.entries[kind];self.u.hook_add(UC_HOOK_CODE,self.sort_entry,user_data=kind,begin=at,end=at)
    def capture_entry(self,u,address,size,kind):
        args=self.args(2)if self.gcc else(u.reg_read(xc.UC_X86_REG_ECX),*self.args(1));self.events.append((kind,args,self.state()))
    def sort_entry(self,u,address,size,kind):
        if kind=='short_sort':
            low,high,width=self.args(4)[:3]if self.is_linked else(self.args(3)[0],u.reg_read(xc.UC_X86_REG_EAX),self.args(3)[1])
            args=(low,high,width)
        elif kind=='sort':args=self.args(4)[:3]
        else:args=self.args(2)
        self.events.append((kind,args,self.state()))
    def compare(self,u,address,size,user):
        a,b=self.args(2);left,right=self.u.mem_read(a,1)[0],self.u.mem_read(b,1)[0]
        self.compare_trace.extend((a-ARRAY,b-ARRAY,left,right,digest(bytes(self.u.mem_read(ARRAY,self.sort_bytes)))));self.events.append(('compare',(a,b,left,right),self.state()));self.ret((left>right)-(left<right))
    def short_original(self,low,high,width):
        u=self.u;stack=md.core.STACK+0xf000;self.put(stack,STOP,low,width,COMPARE)
        regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        for r,v in((xc.UC_X86_REG_ESP,stack),(xc.UC_X86_REG_EAX,high),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,0x27f),(xc.UC_X86_REG_FPTAG,0xffff)):u.reg_write(r,v)
        u.emu_start(self.entries['short_sort'],STOP,count=8000000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP and u.reg_read(xc.UC_X86_REG_ESP)==stack+4
        assert tuple(u.reg_read(r)for r in regs)==saved and u.reg_read(xc.UC_X86_REG_FPCW)==0x27f and u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff
    def group_fixture(self,c,native=False):
        self.active_kind='group';self.base(c,native);self.put(EFFECT+0x118,0x200)
        for j in range(3):
            parameter=self.parameters+j*0x50;self.put(parameter,self.blob+0x800+j*0x100);self.put(parameter+0x30,0x800+j*0x100);self.put_half(parameter+0x44,j+1);self.put_half(parameter+0x46,3)
            self.u.mem_write(self.blob+0x800+j*0x100,bytes(0x100));self.put(self.blob+0x200+j*0x40,3)
            if c['shape']==5:self.put(self.blob+0x200+j*0x40,0,5,0x80+j*16,0,1,1,3,2 if c['columns']==3 else 1,0x80+j*16,0,1,1,4)
        self.device=self.buffer(8);self.put(EFFECT+0xc,self.device);self.other_device=self.device
        refs=self.array(1,0x18,'reference_construct');self.reference=refs;self.put(EFFECT+0x14,1,refs)
        self.shader=self.buffer(len(c['program']))if c['has_shader']else 0
        if self.shader:self.u.mem_write(self.word(self.shader+12),c['program'])
        self.old_code=self.buffer(len(c['program']))if c['has_code']else 0
        if self.old_code:self.u.mem_write(self.word(self.old_code+12),c['program'])
        flags=(2 if c['have_bytecode']else 0)|(1 if c['pending']else 0)
        self.put(refs,16,self.shader,self.old_code,0,0x12345678,flags)
        self.put(self.group+8,c['group_flags']);self.put_half(self.group+0x38,c['capacity'])
        if c['capacity']>2:
            old=self.word(self.group+4);self.call('records_delete',[3],old)
            records=self.array(c['capacity'],0x64,'record_construct');self.put(self.group+4,records);self.record=records
        self.put(self.record,1);self.put(self.record+0x14,16);self.put(self.record+0x1c,0,0,0);self.put_half(self.record+0x52,0x92);self.put(self.record+0x58,0x0a000000)
        if c['shared_graph']:
            pool=self.alloc(0x20);self.call('pool_construct',receiver=pool);shared=self.alloc(0x1000);self.u.mem_write(shared,bytes(self.u.mem_read(self.blob,0x1000)));self.put(pool+0x10,shared,0x1000,0x1000);self.put(EFFECT+0x38,pool)
            link=self.alloc(0x14);mapping=self.alloc(16);self.put(mapping,0,0,0,0);self.put(link,EFFECT,self.parameters,mapping,link,link)
            self.put(self.parameters+0x40,link);self.put(self.parameters+0x28,self.word(self.parameters+0x28)|1);self.put(self.parameters,shared+0x800,shared+0x200);self.put(shared+0x800,3)
        primary_use=self.array(1,0x20,'use_construct');self.put(self.record+0x34,primary_use);self.put_half(self.record+0x50,1);self.put_half(self.record+0x4e,0)
        self.grid_nodes=[]
        if c['grids']:
            for group_index in(0,1):
                at=self.word(self.word(EFFECT+0x2c)+group_index*0x64+4)+0x64
                use=self.array(1,0x20,'use_construct');self.put(at+0x34,use);self.put_half(at+0x50,1);self.put_half(at+0x4e,1);self.put_half(at+0x52,0x94);self.put(at+0x58,0x0c000000)
                self.call('link',[group_index,1,0,self.parameters,0],EFFECT);self.grid_nodes.append(use)
            self.put(EFFECT+0xa0,2)
            for off in(0xb4,0xc0):
                grid=self.alloc(24);self.put(EFFECT+off,grid)
                for row in range(6):
                    slots=self.alloc(20 if off==0xb4 else 12);self.put(grid+row*4,slots)
                    self.put(slots,self.grid_nodes[1],0,0,0,0)if off==0xb4 else self.put(slots,*self.grid_nodes,0)
        self.put(EFFECT+0x118,0x200|(0x800 if c['suppress_grids']else 0))
        self.case=dict(self.case,read_bytes=len(c['program']),shader_fill=c['program'],query_result=c['query_result'],device_result=c['device_result'],read_result=c['read_result'],device_alias=1,write_query=True,write_device=True,device_resizes=False,change_dispatch=False)
        self.requests=0;self.events=[];self.capture_active=True
    def rewrite_shader(self,program):
        incoming=self.buffer(len(program));self.u.mem_write(self.word(incoming+12),program)
        index=(self.reference-self.word(EFFECT+0x18))//0x18
        self.call('shader',[incoming,index,0],EFFECT);self.call_buffer_release(incoming);self.shader=self.word(self.reference+4);self.old_code=self.word(self.reference+8)
        self.case=dict(self.case,read_bytes=len(program),shader_fill=program);self.capture_active=True
    def step_group(self,kind):
        self.capture_active=True;result=self.call(kind,[0],EFFECT);self.capture_active=False;return result
    def retail_graph(self,row,capture=False):
        self.capture_active=False;self.shader=0;c=ds.valid_case();c.update(pool=bool(row['recipe']%2),flags=0x200,fail_allocation=0)
        self.setup_deserialize(c,row['payload']);result=self.call('deserialize',[self.device,self.pool,self.serialized,ds.POSITION],EFFECT);assert result==0
        groups=self.word(EFFECT+0x2c);statuses=[];words=[];self.grid_nodes=[]
        for index in range(self.half(EFFECT+0x30)):
            group=groups+index*0x64;record=self.word(group+4)
            if not record:continue
            tag=self.word(record+0x58)
            if tag not in(0x0a000000,0x0b000000):continue
            owner=self.word(self.word(EFFECT+0xb0)+self.word(record+0x1c)*4);base=self.word(self.word(EFFECT+0x38)+0x10)if self.word(owner+0x40)else self.word(EFFECT+0x20);resource=self.word(base+self.word(owner+0x30)+self.word(record+0x24));resource=self.word(self.word(self.word(owner+0x40)+8)+resource*4)if self.word(owner+0x40)else resource;ref=self.word(EFFECT+0x18)+resource*0x18
            code=self.word(ref+8);payload=bytes(self.u.mem_read(self.word(code+12),self.word(code+8)))if code else b''
            if payload and not self.word(ref+4):
                shader=self.buffer(len(payload));self.u.mem_write(self.word(shader+12),payload);self.put(ref+4,shader)
            self.reference=ref;self.shader=self.word(ref+4);self.old_code=code;self.other_device=self.device
            self.case=dict(self.case,read_bytes=len(payload),shader_fill=payload,query_result=0,device_result=0,read_result=0,device_alias=1,write_query=True,write_device=True,device_resizes=False,change_dispatch=False)
            self.capture_active=True;self.active_kind='retail';result=self.call('prepare_a'if tag==0x0a000000 else'prepare_b',[index],EFFECT);self.capture_active=False;statuses.append(result)
            if capture:words.extend(capture_group(self,index,ref,result))
            if payload:
                self.rewrite_shader(payload);result=self.call('prepare_a'if tag==0x0a000000 else'prepare_b',[index],EFFECT);self.capture_active=False;statuses.append(result)
                if capture:words.extend(capture_group(self,index,ref,result))
        self.call('destroy',receiver=EFFECT);self.call_buffer_release(self.device);self.call_buffer_release(self.serialized)
        if self.pool:self.call('pool_release',[self.pool])
        assert not self.live,('retail preparation cleanup',row['recipe'],self.live)
        return (self.snapshot(tuple(statuses)),words)if capture else self.snapshot(tuple(statuses))
    def run(self,kind,c):
        self.active_kind=kind
        if kind in('prepare_a','prepare_b'):
            self.group_fixture(c);self.active_kind=kind;self.put_half(self.record+0x52,0x92 if kind=='prepare_a'else 0x93);self.put(self.record+0x58,0x0a000000 if kind=='prepare_a'else 0x0b000000);self.put(self.reference,16 if kind=='prepare_a'else 15);result=self.step_group(kind)
            if c['repeat']:
                cc=dict(c,constants=(c['constants']+1)%4,pres=not c['pres']);self.rewrite_shader(shader_program(cc));result=(result,self.step_group(kind))
        else:
            self.prepare(c);self.getters=Counter();self.setup_debug(dict(c,cached_exports=0,setting=-1));self.capture_active=False;self.shader=0
            if kind=='compare_constants':
                self.put(ARRAY,ENTRIES,ENTRIES+20);self.put(ENTRIES+4,(c['compare_start']<<16)|c['compare_bank']);self.put(ENTRIES+24,(c['compare_other_start']<<16)|c['compare_other_bank']);result=self.call(kind,[ARRAY,ARRAY+4])
            else:
                count=c['sort_count']if kind=='sort'else c['short_count'];width=c['sort_width']if kind=='sort'else max(1,c['sort_width'])
                self.compare_trace=[];self.sort_bytes=count*width;values=[]
                for i in range(count):
                    value=(i,255-i,0,i%3,(i*37+11)%256)[c['sort_mode']];values.extend([value&255]+[(i*13+j)&255 for j in range(1,width)])
                if width:self.u.mem_write(ARRAY,bytes(values))
                self.events=[]
                if kind=='short_sort'and not self.is_linked:self.short_original(ARRAY,ARRAY+(count-1)*width,width);result=None
                else:result=self.call(kind,[ARRAY,count,width,COMPARE]if kind=='sort'else[ARRAY,ARRAY+(count-1)*width,width,COMPARE])
        return self.snapshot(result)


def native_cases():
    for i,c in enumerate(cases(32)):
        c.update(effect=b'\xa5'*0x130,arena=b'\xa5'*md.core.SIZE,heap=b'\xa5'*md.core.HSIZE,fail_allocation=0,group_flags=0x12345678,record_flags=0x1234567b,parameter_flags=i%2)
        cc=dict(c,constants=(c['constants']+1)%4,pres=not c['pres'])
        yield i,c,shader_program(cc)


def digest(data):
    value=2166136261
    for byte in data:value=((value^byte)*16777619)&0xffffffff
    return value


def cache_offset(m,record):
    w=m.word;owner=w(w(EFFECT+0xb0)+w(record+0x1c)*4);shared=bool(w(owner+0x40));base=w(w(EFFECT+0x38)+0x10)if shared else w(EFFECT+0x20);offset=w(record+0x5c)-base
    if not shared and offset>=w(EFFECT+0x1c):offset=0x80000000+offset-(((base+w(EFFECT+0x1c))&~31)-base+32)
    return offset


def capture_group(m,index,reference,result):
    w=m.word;group=w(EFFECT+0x2c)+index*0x64;records=w(group+4);words=[result,w(group+8)&31]
    for off in(0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c):words.append(w(group+off))
    words.append(w(records-4));words.extend((w(reference+0x14)&3,))
    code=w(reference+8);table=w(reference+0xc);words.append(table-w(code+12)+1 if table and code else 0)
    for off in(4,8):
        buffer=w(reference+off);words.extend((int(bool(buffer)),w(buffer+4)if buffer else 0,w(buffer+8)if buffer else 0,digest(bytes(m.u.mem_read(w(buffer+12),w(buffer+8))))if buffer else 0))
    built=result==0 and table!=0
    words.append(int(built))
    if built:
        words.extend(w(group+off)for off in(0x40,0x44,0x48));words.extend(m.half(group+0x4c+i*2)for i in range(11))
        for r in range(m.half(group+0x36)):
            record=records+r*0x64;tag=w(record+0x58);pres=m.half(record+0x52)in(0xb4,0xb5);primary=r==0
            words.extend((w(record),w(record+4),w(record+0x30)&31,m.half(record+0x4e),m.half(record+0x50),w(w(record+0x34)-4)if w(record+0x34)else 0))
            if not primary:words.extend((m.half(record+0x52),m.half(record+0x54),m.half(record+0x56),tag))
            if not primary and not pres:
                words.append(w(record+0x48));size=4 if tag==0x0f000000 else m.half(record+0x56)*(4 if tag in(0x0c010000,0x0d010000)else 16)
                words.append(digest(bytes(m.u.mem_read(w(group)+w(record+0x48),size))))
                cached=tag in(0x0c000000,0x0d000000)and bool(w(record+0x30)&0x10)
                words.extend((int(cached),cache_offset(m,record)if cached else 0,w(record+0x60)if cached else 0))
            if pres:
                buffer=w(record+0x38);data=w(buffer+12);words.extend((w(buffer+4),w(buffer+8)))
                words.extend(w(data+j*4)if j not in(13,14,15)or w(record+0x3c)else 0 for j in[*range(22),23,24,25])
                words.append(digest(bytes(m.u.mem_read(data+0x68,w(buffer+8)-0x68))))
                words.extend(w(record+off)-data+1 if w(record+off)else 0 for off in(0x3c,0x40,0x44))
        words.append(digest(bytes(m.u.mem_read(w(group)+w(group+0x14),m.half(group+0x34)*2))))
    for i in range(w(EFFECT+0x98)):
        head=w(w(EFFECT+0x9c)+i*0x50+0x34);at=head;links=[]
        if head:
            while True:
                entry=w(at+0x10);links.extend((w(at),w(at+4),m.half(at+0x14),m.half(at+0x16)))
                links.extend(w(entry+j*4)if entry else 0 for j in range(5));at=w(at+0x1c)
                if at==head:break
                assert len(links)<10000
        words.append(len(links)//9);words.extend(links)
    words.extend((int(bool(w(EFFECT+0xb4))),int(bool(w(EFFECT+0xc0)))))
    for off in(0xb4,0xc0):
        grid=w(EFFECT+off)
        if grid:
            for row in range(6):
                slots=w(grid+row*4);cells=[];at=0
                while w(slots+at*4):
                    pointer=w(slots+at*4);cells.append(1 if pointer==m.grid_nodes[0]else 2 if pointer==m.grid_nodes[1]else 0);at+=1;assert at<8
                words.append(len(cells));words.extend(cells)
    return [value&0xffffffff for value in words]


def native_graph(m,mode,c,program2):
    m.group_fixture(c,True);kind='prepare_a'if mode%2==0 else'prepare_b'
    m.put_half(m.record+0x52,0x92 if kind=='prepare_a'else 0x93);m.put(m.record+0x58,0x0a000000 if kind=='prepare_a'else 0x0b000000);m.put(m.reference,16 if kind=='prepare_a'else 15)
    result=m.step_group(kind);words=capture_group(m,0,m.reference,result)
    if c['repeat']:
        m.rewrite_shader(program2);result=m.step_group(kind);words.extend(capture_group(m,0,m.reference,result))
    m.call('destroy',receiver=EFFECT);assert not m.live,('native group cleanup',mode,m.live)
    return words


def verify(image,linked,gcc,n,kinds,retail):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect prepare groups mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:8];event=next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect prepare groups mismatch',kind,i,diffs,x[2],y[2],event,len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[str((kind,x[2]))]+=1
    for row in retail if 'prepare_a'in kinds and 'prepare_b'in kinds else():
        try:x=a.retail_graph(row);y=b.retail_graph(row)
        except(AssertionError,UcError)as e:raise AssertionError(('effect prepare groups mismatch','retail',row['recipe'],str(e)))from e
        if x!=y:
            diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:8];event=next(((j,v[:-1],w[:-1])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
            raise AssertionError(('effect prepare groups mismatch','retail',row['recipe'],diffs,x[2],y[2],event,len(x[3]),len(y[3])))
        counts['retail_chains']+=1;counts['retail_groups']+=len(x[2]);returns.update(str(('retail',v))for v in x[2])
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--gcc',action='store_true');parser.add_argument('--cases',type=int,default=512);parser.add_argument('--kind',choices=['all',*SPANS],default='all');parser.add_argument('--replace',type=Path);parser.add_argument('--replace-sort',type=Path);parser.add_argument('--no-retail',action='store_true');parser.add_argument('--write-native-fixtures',type=Path);parser.add_argument('--native-dump',type=Path);parser.add_argument('--mutation-checks',action='store_true')
    args=parser.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in manifest['functions'].values():
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    for row in manifest.get('data',{}).values():
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    replace={}
    if args.replace:replace[SOURCE.name]=args.replace
    if args.replace_sort:replace[SORT.name]=args.replace_sort
    linked=md.build(replace,args.gcc);report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind],[]if args.no_retail else cp.rd.retail_inputs(pe));report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.write_native_fixtures:
        content=b'UG2FXGF1'+ds.pack([32])
        for mode,c,program2 in native_cases():
            config=[mode,c['shape'],c['elements'],c['rows'],c['columns'],c['parameter_flags'],int(c['pending']),int(c['has_shader']),int(c['has_code']),int(c['have_bytecode']),c['group_flags'],c['capacity'],int(c['grids']),int(c['suppress_grids']),int(c['repeat']),len(c['program']),len(program2),c['storage_offset'],int(c['shared_graph'])]
            content+=ds.pack(config)+c['program']+program2
        content+=ds.pack([32])
        for i,c in enumerate(cases(32)):
            private=i%2;count=c['short_count']if private else c['sort_count'];width=max(1,c['sort_width'])if private else c['sort_width'];values=[]
            for j in range(count):values.extend([(j,255-j,0,j%3,(j*37+11)%256)[c['sort_mode']]&255]+[(j*13+k)&255 for k in range(1,width)])
            raw=bytes(values)if width else b'';content+=ds.pack([i,private,count,width,len(raw)])+raw
        rows=[]if args.no_retail else cp.rd.retail_inputs(pe);content+=ds.pack([len(rows)])
        for row in rows:content+=ds.pack([row['recipe'],len(row['payload'])])+row['payload']
        args.write_native_fixtures.write_bytes(content)
    if args.kind=='all'and not args.replace and not args.replace_sort:
        a,b=Machine(image),Machine(image,linked,args.gcc);native=[]
        for mode,c,program2 in native_cases():
            try:x=native_graph(a,mode,c,program2);y=native_graph(b,mode,c,program2)
            except(AssertionError,UcError)as e:raise AssertionError(('effect prepare groups mismatch','native',mode,str(e)))from e
            assert x==y and a.snapshot(tuple(x))==b.snapshot(tuple(y)),('effect prepare groups mismatch','native',mode,x,y)
            native.append(x)
        report['native_group_chains']=len(native)
        for i,c in enumerate(cases(32)):
            kind='short_sort'if i%2 else'sort';x=a.run(kind,c);y=b.run(kind,c)
            assert x==y,('effect prepare groups mismatch','native sort',i)
            words=[digest(bytes(a.u.mem_read(ARRAY,a.sort_bytes))),len(a.compare_trace)//5,*a.compare_trace]
            assert words==[digest(bytes(b.u.mem_read(ARRAY,b.sort_bytes))),len(b.compare_trace)//5,*b.compare_trace]
            native.append(words)
        report['native_sort_chains']=32
        for row in []if args.no_retail else cp.rd.retail_inputs(pe):
            x,words=a.retail_graph(row,True);y,other=b.retail_graph(row,True)
            assert x==y and words==other,('effect prepare groups mismatch','native retail',row['recipe'])
            native.append(words)
        report['native_retail_chains']=len(native)-64
        report['native_chains']=len(native)
        if args.native_dump:
            raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXGD1';at=8
            for i,expected in enumerate(native):
                row,count=struct.unpack_from('<2I',raw,at);at+=8;values=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
                assert row==i and values==expected,('native group preparation mismatch',i,len(expected),len(values),[(j,v,w)for j,(v,w)in enumerate(zip(expected,values))if v!=w][:16])
            assert at==len(raw);report['native_replay']=len(native)
    if args.mutation_checks:
        mutations={
            'primary-data-cache':('body','prepare_a','primary->value08.value00=base+owner->value30+primary->value08.value1C;','primary->value08.value00=base+owner->value2C+primary->value08.value1C;'),
            'primary-type-cache':('body','prepare_a','primary->value08.value04=base+owner->value2C+primary->value08.value18;','primary->value08.value04=base+owner->value30+primary->value08.value18;'),
            'shared-index-map':('body','prepare_a','if (owner->value40) index=static_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(owner->value40)->data08)[index];','(void)0;'),
            'pending-gate':('body','prepare_a','if (!(reference->value14&1))','if (true)'),
            'grid-group-filter':('body','prepare_a','(*second)->value14==groupIndex','(*second)->value14!=groupIndex'),
            'grid-end-scan':('body','prepare_a','while (*end) ++end;','if (*end) end+=2;'),
            'grid-stage-flag':('body','prepare_a','group->value08&=0xfffffff7;','(void)0;'),
            'empty-shader-valid-flag':('body','prepare_a','reference->value14|=2;','(void)0;'),
            'reference-request-clear':('body','prepare_a','reference->value14&=0xfffffffe;','(void)0;'),
            'old-pres-cleanup':('body','prepare_a','record->value52==0xb4 || record->value52==0xb5','false'),
            'old-use-removal':('body','prepare_a','else if (record->value4E)','else if (false)'),
            'primary-record-count':('body','prepare_a','group->value36=1;','group->value36=2;'),
            'bytecode-valid-gate':('body','prepare_a','if (!(reference->value14&2)) goto finish;','(void)0;'),
            'bulk-register-origin':('body','prepare_a','minimum[j]==0 && maximum[j]==counts[j]','minimum[j]==1 && maximum[j]==counts[j]'),
            'pres-limit-sum':('body','prepare_a','rsi[j]+rsi[j+3]','rsi[j]'),
            'pres-error-cleanup-order':('body','prepare_a','if (result<0) return result;','if (result<0) goto finish;'),
            'pres-record-reservation':('body','prepare_a','unsigned int required=table[3]+prefix;','unsigned int required=table[3]+1;'),
            'advertised-capacity-word':('body','prepare_a','static_cast<unsigned short>(table[3]+1);','static_cast<unsigned short>(required);'),
            'primary-prefix-width':('body','prepare_a','word<10;++word) reinterpret_cast<unsigned int *>(&records->value08)','word<6;++word) reinterpret_cast<unsigned int *>(&records->value08)'),
            'primary-dirty-transfer':('body','prepare_a','records->value00=primary->value00;','records->value00=0;'),
            'primary-use-ownership':('body','prepare_a','primary->array34=0;','(void)0;'),
            'boolean-bank-width':('body','prepare_a','intBase=boolBase+amount*4','intBase=boolBase+amount*16'),
            'pres-mask-construction':('body','prepare_a','mask=count*4|((count&0x3fffffff)<<1);','mask=count*4;'),
            'pres-header-bank-offset':('body','prepare_a','data[11]=boolBase;','data[11]=intBase;'),
            'pres-third-range-offset':('body','prepare_a','data[25]=data[24]+4+','data[25]=data[24]+8+'),
            'pres-section-parse':('body','prepare_a','result=D3DXEffectReadCompiledSections(data+26,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));','result=0;'),
            'pres-cover-flag':('body','prepare_a','if (counts[bank]+sum!=maximum)','if (counts[bank]+sum==maximum)'),
            'group-storage-allocation':('body','prepare_a','result=group->AllocateData();','result=0;'),
            'sort-array-size':('body','prepare_a','UG2FxMalloc(table[3]*4)','UG2FxMalloc(table[3]*4+4)'),
            'constant-sort-call':('body','prepare_a','D3DXEffectSort(sorted,table[3],4,D3DXEffectCompareConstants);','(void)0;'),
            'float-record-partition':('body','prepare_a','else if (bank==2) ++floatCount;','else if (bank==2) ++intCount;'),
            'missing-parameter-state':('body','prepare_a','if (!parameter) record->value00=0;','if (!parameter) record->value00=1;'),
            'parameter-prefix-width':('body','prepare_a','word<10;++word) reinterpret_cast<unsigned int *>(&record->value08)','word<6;++word) reinterpret_cast<unsigned int *>(&record->value08)'),
            'integer-default-word':('body','prepare_a','output[2]=1;','output[2]=0x3f800000;'),
            'matrix-class-gate':('body','prepare_a','+entry[3])==3','+entry[3])==2'),
            'matrix-array-column-gate':('body','prepare_a','else if (columns==4)','else if (columns==3)'),
            'struct-vector-only-gate':('body','prepare_a','if (D3DXEffectVectorOnly(reinterpret_cast<unsigned int **>(&type)))','if (true)'),
            'float-cache-span':('body','prepare_a','(units<count ? units : static_cast<unsigned int>(count))*16','static_cast<unsigned int>(count)*16'),
            'sampler-owner-storage':('body','prepare_a','storage[0]=parameter->value44+1;','storage[0]=parameter->value44;'),
            'sampler-index-list':('body','prepare_a','static_cast<unsigned short>(recordIndex);','static_cast<unsigned short>(recordIndex+1);'),
            'pixel-bank-layout':('wrapper','prepare_b','#define FX_PREPARE_BANK_OFFSET 0x24','#define FX_PREPARE_BANK_OFFSET 0x1c'),
            'pixel-ready-bit':('wrapper','prepare_b','#define FX_PREPARE_STAGE_FLAG 0','#define FX_PREPARE_STAGE_FLAG 2'),
            'pixel-capture-boundary':('wrapper','prepare_b','#define FX_PREPARE_CAPTURE CapturePixelBytecode','#define FX_PREPARE_CAPTURE CaptureVertexBytecode'),
            'vertex-sampler-bias':('wrapper','prepare_a','#define FX_PREPARE_SAMPLER_BIAS 0x101','#define FX_PREPARE_SAMPLER_BIAS 0'),
            'constant-priority-order':('sort','compare_constants','ug2FxConstantOrder7B5048[left[2]]-ug2FxConstantOrder7B5048[right[2]]','left[2]-right[2]'),
            'constant-register-order':('sort','compare_constants','static_cast<int>(left[3])-static_cast<int>(right[3])','static_cast<int>(right[3])-static_cast<int>(left[3])'),
            'shortsort-equal-selection':('sort','short_sort','if (compare(at,maximum)>0)','if (compare(at,maximum)>=0)'),
            'shortsort-swap-width':('sort','short_sort','byte<width;','byte+1<width;'),
            'qsort-singleton-call':('sort','sort','if (count<2 || !width)','if (count<1 || !width)'),
            'qsort-short-threshold':('sort','sort','/width+1>8','/width+1>9'),
            'qsort-median-direction':('sort','sort','if (compare(low,middle)>0)','if (compare(low,middle)<0)'),
            'qsort-left-equals':('sort','sort','while (compare(first,middle)<=0)','while (compare(first,middle)<0)'),
            'qsort-right-equals':('sort','sort','while (compare(last,middle)>0)','while (compare(last,middle)>=0)'),
            'qsort-pivot-move':('sort','sort','if (middle==last) middle=first;','(void)0;'),
            'qsort-equal-trim':('sort','sort','while (!compare(last,middle));','while (compare(last,middle)>0);'),
        }
        rejected=[]
        for name,(unit,kind,before,after)in mutations.items():
            if args.kind!='all'and args.kind!=kind:continue
            source=BODY if unit=='body'else SOURCE if unit=='wrapper'else SORT;text=source.read_text();assert before in text,(name,before)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+('.inc'if unit=='body'else'.cpp'));path.write_text(text.replace(before,after,1))
            if unit=='body':
                wrapper=path.with_suffix('.cpp');wrapper.write_text(SOURCE.read_text().replace('"EffectPrepareGroup.inc"','"'+str(path)+'"'));path=wrapper
            command=[sys.executable,__file__,'--replace-sort'if unit=='sort'else'--replace',str(path),'--kind',kind,'--cases','16'if kind=='all'else'128']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect prepare groups mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
