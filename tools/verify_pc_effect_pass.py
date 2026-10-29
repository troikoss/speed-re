#!/usr/bin/env python3
"""Compare original effect pass control and texture ownership; device states controlled."""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
import verify_pc_effect_values as values
md=values.md;core=md.core;ROOT,EXE,LIB=values.ROOT,values.EXE,values.LIB
ARENA,HEAP,STOP,EFFECT=core.ARENA,core.HEAP,core.STOP,md.EFFECT
BUILD=ROOT/'build/analysis/effect-pass';SOURCE=LIB/'EffectPassRuntime.cpp';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-pass.json'
GROUPS,RECORDS,TECHNIQUES,PASSES,QUEUE,QUEUE2,PARAMETERS,TYPE,SCRATCH,REFS,INDEX,NAMES,SHARED,POOL,TABLE,TEXTURES,TAIL=[ARENA+x for x in(0x2000,0x3000,0x5000,0x5400,0x6000,0x6100,0x8000,0x9000,0x9400,0x9800,0x9a00,0x9c00,0xa000,0xa300,0xa400,0xac00,0xae00)]
SPANS=dict(begin_pass=(0x66acd2,282),commit=(0x66ac17,187),end_pass=(0x65adcb,90),group_apply=(0x6691a1,229),record_texture=(0x666a68,179),set_texture=(0x66d7d9,212))
KINDS=list(SPANS)+['lifecycle']
FREE=dict(begin_pass='D3DXEffectBeginPass',commit='D3DXEffectCommit',end_pass='D3DXEffectEndPass',set_texture='D3DXEffectSetTexture')
MEMBERS=dict(group_apply=('D3DXEffectObject','ApplyPassGroup'),record_texture=('D3DXEffectObject','RecordTexture'),adjust=('D3DXEffectObject','AdjustReferences'))
# Ordinary C ABI shims expose the still-unrecovered thiscall boundaries.
SERVICES=dict(cache_a=(0x66403c,2,'UG2FxPassCache66403C'),cache_b=(0x664197,2,'UG2FxPassCache664197'),fast=(0x668dfd,1,'UG2FxPassFastGroup668DFD'),record=(0x669286,3,'UG2FxPassRecord669286'),cached=(0x66a25f,2,'UG2FxPassCachedRecord66A25F'),prepare_a=(0x666df7,1,'UG2FxPrepareGroupA'),prepare_b=(0x667d51,1,'UG2FxPrepareGroupB'))
SHIMS={k:STOP+0x8000+i*16 for i,k in enumerate(SERVICES)}
md.SOURCES += [SOURCE,LIB/'EffectSetup.cpp',LIB/'EffectFpControl.cpp',ROOT/'src/Platform/Native/NativeFxFpControl.cpp']
md.BUILD=BUILD;md.MEMBERS.update(MEMBERS);md.FREE.update(FREE);md.STDCALL.update(begin_pass=8,commit=4,end_pass=4,set_texture=12)
core.GLOBALS.update({row[2]:SHIMS[k]for k,row in SERVICES.items()});core.GLOBALS.update(UG2FxEvaluateRecord=STOP+0x8900,ug2FxIndexBias7A1E58=0x7a1e58,_fltused=0,_ftol2=0x75dd28)

def pack(words):return struct.pack('<'+'I'*len(words),*(v&0xffffffff for v in words))

def cases(n):
    rng=random.Random(0x66acd2)
    for i,c in enumerate(core.cases(n)):
        c.update(seed=i,word=(0x7f,0x27f,0x37f)[i%3]|((i//3)%4)*0x400,mxcsr=0x1f80|((i//12)%4)*0x2000,live=i%3,top=i//3%8,status=i%64,
            flags=rng.getrandbits(32),saved=(0,0x10000,0x20000,0x30000,0xffffffff)[i%5],state=(0,1,2,3,4,5)[i%6],count=(0,1,2,3,5,8)[i%6],
            selected=(i//6)%2,prior=(0xffff,0,1,2,0x100)[(i//12)%5],pass_index=(0,1,2,3,0xffff,0x10000,0xffffffff)[(i//5)%7],
            group=(0,1,2,0x10000,0x80000002)[i%5],index=rng.getrandbits(32),record_flags=rng.getrandbits(32),
            fail_at=(0,1,2,3,5)[(i//3)%5],failure=(0,1,0x80004005,0x8876086c,0xffffffff)[(i//15)%5],mutations=(i//7)%16,
            fail_allocation=(0,1,2)[(i//11)%3],texture=(0,TEXTURES,TEXTURES+0x20,TEXTURES+0x40)[i%4],handle_mode=(i//4)%4,
            type_kind=(4,5,6,7,8,9,10,0xffffffff)[(i//16)%8],elements=(i//8)%3,element=(0xffffffff,0,1)[(i//24)%3],shared=i%3==0,
            controlled=i%7==0,repeat=i%9==0)
        yield c

class Machine(md.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.rebuilt=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()},adjust=0x65ae25,resolve=0x66bbee,object=0x65ccc8,notify=0x66055c)
        for k,(original,n,name)in SERVICES.items():
            at=SHIMS[k]if linked else original;self.u.hook_add(UC_HOOK_CODE,self.service,user_data=k,begin=at,end=at)
        for k in('adjust','resolve','object','notify'):
            at=self.entries[k];self.u.hook_add(UC_HOOK_CODE,self.observe,user_data=k,begin=at,end=at)
        at=linked['exports']['D3DXEffectBufferAddRef'if gcc else'_D3DXEffectBufferAddRef@4']if linked else 0x670dde
        self.u.hook_add(UC_HOOK_CODE,self.addref,begin=at,end=at)
    def record(self,name,args):
        self.events.append((name,tuple(args),bytes(self.u.mem_read(EFFECT,0x130)),self.u.reg_read(xc.UC_X86_REG_FPCW)&~0x40,self.requests,self.allocated))
    def member_args(self,n):
        return self.args(n+1)if self.rebuilt and self.gcc else(self.u.reg_read(xc.UC_X86_REG_ECX),*self.args(n))
    def transition(self):
        if self.calls!=1:return
        m=self.case['mutations']
        if m&1:self.put(EFFECT+0x3c,1 if self.case['state']==4 else 4)
        if m&2:self.put(EFFECT+0x108,QUEUE2,4)
        if m&4:self.put(EFFECT+0x118,self.word(EFFECT+0x118)^0x101)
        if m&8:self.u.reg_write(xc.UC_X86_REG_FPCW,0xa7f)
    def service(self,u,address,size,k):
        n=SERVICES[k][1];a=self.args(n+1)if self.rebuilt else self.member_args(n)
        self.record(k,a);self.calls+=1;self.transition();result=self.case['failure']if self.calls==self.case['fail_at']else 0
        self.ret(result,0 if self.rebuilt else n*4)
    def observe(self,u,address,size,k):
        n=dict(adjust=3,resolve=3,object=3,notify=1)[k];a=self.member_args(n);self.record(k,a)
        # Error-propagation cases isolate only these existing recovered bodies.
        if self.case['controlled']:
            self.calls+=1
            if k=='resolve':result=0 if self.case['fail_at']==1 else PARAMETERS
            else:result=self.case['failure']if self.calls==self.case['fail_at']else 0
            self.ret(result,0 if self.rebuilt and self.gcc else n*4)
    def addref(self,u,address,size,_):
        self.record('addref',self.args(1))
        if self.case['mutations']&2:self.put(EFFECT+0xfc,TAIL)
    def state(self):return bytes(self.u.mem_read(ARENA,core.SIZE))+bytes(self.u.mem_read(HEAP,self.allocated))
    def runtime(self,u,address,size,kind):
        if kind=='abort':raise AssertionError('unexpected native class/boundary')
        arg,=self.args(1);self.record(kind,[arg])
        if kind=='malloc':
            self.requests+=1;result=0
            if self.requests!=self.case['fail_allocation']:
                result=HEAP+self.allocated;self.allocated+=max(16,(arg+15)&~15);assert self.allocated<0x1000;self.live[result]=arg
            self.ret(result)
        else:
            assert not arg or arg in self.live
            if arg:self.live.pop(arg)
            self.ret()
    def fixture(self,c):
        self.case=c;self.events=[];self.calls=self.requests=self.allocated=0;self.live={}
        self.u.mem_write(ARENA,c['arena']);self.u.mem_write(HEAP,c['heap']);self.put(0,0x12345678)
        # Initialize only the fields consumed by this valid synthetic graph.
        self.put(EFFECT,0x7b6fe0);self.put(EFFECT+0x2c,GROUPS);self.put(EFFECT+0x3c,c['state'],((1-c['selected'])if c['seed']&1 else c['selected'])|(c['selected']<<16),0xa5a50000|c['prior'])
        self.put(EFFECT+0xa4,TECHNIQUES,3,3,TABLE,0,0,0);self.put(EFFECT+0x108,QUEUE,c['count'],16,c['saved'],c['flags'])
        for g in range(3):
            p=GROUPS+g*0x64;self.put(p,ARENA+0x7000,RECORDS+g*0x400,(c['seed']+g)%16)
            self.put_half(p+0x18,0xffff if(c['seed']+g)%2 else 0);self.put_half(p+0x1a,0xffff if(c['seed']+g)%3 else 1)
            self.put_half(p+0x34,0);self.put_half(p+0x36,(c['seed']+g)%5);self.put_half(p+0x3a,(c['seed']+g)%4);self.put_half(p+0x3c,(0,1,0xffff)[(c['seed']+g)%3])
            for r in range(8):
                at=RECORDS+g*0x400+r*0x64;self.put(at,0 if(c['seed']+r)%3==0 else 1);self.put(at+0x30,c['record_flags']^(r*10))
        for j in range(16):self.put(QUEUE+j*4,((j%3)<<0)|(((j//3)%8)<<16));self.put(QUEUE2+j*4,((2-j%3)<<0)|(((j+1)%8)<<16))
        for j in range(2):
            self.put(TECHNIQUES+j*0xb4+0x34,3,PASSES+j*0x200)
            for r in (0,1,2,0x100):self.put(PASSES+j*0x200+r*0x38+0x34,(j+r)%3)
        self.put(EFFECT+0x18,REFS);self.put(EFFECT+0x20,NAMES);self.put(EFFECT+0x98,3,PARAMETERS);self.put(EFFECT+0xc4,SCRATCH);self.put(EFFECT+0xfc,EFFECT+0xe0);self.put(EFFECT+0xe0,3,0,0,0,0,0,0);self.put(TAIL,3,0,0,0,0,0,0)
        self.put(EFFECT+0x38,POOL);self.put(POOL+8,2,0,NAMES);self.put(SHARED,0,0,INDEX+0x100,SHARED,SHARED)
        for j in range(3):
            p=PARAMETERS+j*0x50;self.put(TABLE+j*4,p);self.put(p,INDEX+j*4,TYPE+j*0x40,c['element'],0,3,j);self.put(p+0x34,0);self.put(p+0x40,SHARED if c['shared']else 0)
            self.put(TYPE+j*0x40,c['type_kind'],4,0x100+j*0x20,0,c['elements']);self.put(INDEX+j*4,j);self.put(INDEX+0x100+j*4,2-j)
            self.put(NAMES+0x100+j*0x20,3);self.u.mem_write(NAMES+0x104+j*0x20,('p'+str(j)).encode()+b'\0')
            self.put(TEXTURES+j*0x20,0x7b711c,4,0,0);self.put(REFS+j*0x18,0,(0,TEXTURES,TEXTURES+0x20)[(c['seed']+j)%3],0,0,0,0)
        self.u.mem_write(NAMES+0x200,b'p0\0'if c['handle_mode']!=3 else b'missing\0')
        self.u.reg_write(xc.UC_X86_REG_MXCSR,c['mxcsr'])
    def arguments(self,kind,c):
        c=self.case
        handle=(~PARAMETERS)&0xffffffff if c['handle_mode']==0 else 0 if c['handle_mode']==1 else NAMES+0x200
        return dict(begin_pass=(EFFECT,c['pass_index']),commit=(EFFECT,),end_pass=(EFFECT,),group_apply=(c['group'],c['index']),record_texture=(c['index'],c['texture']),set_texture=(EFFECT,handle,c['texture']),lifecycle=(EFFECT,handle,c['texture']))[kind]
    def call(self,kind,args=(),receiver=EFFECT,seed_fp=True):
        u=self.u;member=kind in MEMBERS;params=(receiver,*args)if self.rebuilt and self.gcc and member else args
        sp=core.STACK+0xf000;self.put(sp,STOP,*params)
        regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        for r,v in((xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,0x202)):u.reg_write(r,v)
        if seed_fp:
            top=self.case['top'];tag=0xffff
            for j in range(self.case['live']):
                p=(top+j)&7;u.reg_write(xc.UC_X86_REG_FP0+p,(0x8000000000000000,0x3fff|(j&1)*0x8000));tag&=~(3<<(p*2))
            for r,v in((xc.UC_X86_REG_FPCW,self.case['word']),(xc.UC_X86_REG_FPTAG,tag),(xc.UC_X86_REG_FPSW,(top<<11)|self.case['status'])):u.reg_write(r,v)
        before=self.fp();u.emu_start(self.entries[kind],STOP,count=1000000)
        pop=0 if member and self.rebuilt and self.gcc else len(args)*4
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop,('ABI',kind)
        assert tuple(u.reg_read(r)for r in regs)==saved
        after=self.fp();assert after[1:]==before[1:],('FP owner/status changed',kind,before,after)
        return u.reg_read(xc.UC_X86_REG_EAX)
    def fp(self):
        u=self.u;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        return(u.reg_read(xc.UC_X86_REG_FPCW),u.reg_read(xc.UC_X86_REG_MXCSR),u.reg_read(xc.UC_X86_REG_FPSW)&0x383f,tag,tuple((j,u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if(tag>>(j*2)&3)!=3))
    def setup(self,kind,c):
        if kind=='lifecycle':c=dict(c,controlled=False,state=4,flags=c['flags']&~0x188,prior=0xffff,selected=0,pass_index=0,type_kind=5,elements=0,element=0xffffffff,texture=TEXTURES+0x40,handle_mode=0 if c['seed']%2 else 2)
        self.fixture(c)
        if kind in ('commit','end_pass') and c['seed']%53==0:
            self.put(GROUPS+4,EFFECT+0xdc);self.put_half(GROUPS+0x3c,0);self.put(EFFECT+0x10c,3)
        if kind=='lifecycle':
            use=ARENA+0xb000;self.put(PARAMETERS+0x34,use);self.put(use+0x14,0,use,use)
            self.put(GROUPS+8,0);self.put_half(GROUPS+0x3c,0);self.put_half(GROUPS+0x36,0);self.put(RECORDS,0);self.put(RECORDS+0x30,8 if c['seed']%2 else 0);self.put(EFFECT+0x10c,0)
    def run(self,kind,c):
        self.setup(kind,c);result=self.execute(kind,c);return self.snapshot(result)+(self.fp(),)
    def execute(self,kind,c):
        if kind!='lifecycle':return self.call(kind,self.arguments(kind,c))
        results=[self.call('begin_pass',(EFFECT,0))]
        results.append(self.call('set_texture',self.arguments(kind,c),seed_fp=False))
        results.append(self.call('commit',(EFFECT,),seed_fp=False));results.append(self.call('end_pass',(EFFECT,),seed_fp=False))
        assert results==[0,0,0,0],('lifecycle returns',results)
        return 0

def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x,y=a.run(kind,c),b.run(kind,c)
            except Exception as e:raise AssertionError(('effect pass mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:8]
                event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect pass mismatch',kind,i,diffs,x[2],y[2],x[-1],y[-1],event and(event[0],event[1][:2],event[2][:2])))
            counts[kind]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),calls=dict(events))

def trace_bytes(events):
    out=bytearray()
    for name,args,effect,cw,requests,allocated in events:
        out.extend(name.encode()+b'\0'+pack([len(args),*args])+effect+pack([cw,requests,allocated]))
    return bytes(out)

def export_native(image,path):
    m=Machine(image);temporary=path.with_name(path.name+'.partial');count=128*len(KINDS)
    with temporary.open('wb')as f:
        f.write(b'UG2FXPS1'+pack([count]))
        for c in cases(128):
            c=dict(c,controlled=False,top=(-c['live'])&7)
            for kind in KINDS:
                m.setup(kind,c);arena=bytes(m.u.mem_read(ARENA,core.SIZE));heap=bytes(m.u.mem_read(HEAP,0x1000));args=m.arguments(kind,c)
                result=m.execute(kind,c);trace=trace_bytes(m.events)
                config=[KINDS.index(kind),len(args),*(list(args)+[0]*(3-len(args))),c['word'],c['mxcsr'],c['live'],c['status'],c['fail_allocation'],c['fail_at'],c['failure'],c['mutations'],m.case['state'],result,m.fp()[0],len(trace),m.allocated,m.requests,len(m.live)]
                f.write(pack(config)+arena+heap+bytes(m.u.mem_read(ARENA,core.SIZE))+bytes(m.u.mem_read(HEAP,0x1000))+trace)
    temporary.replace(path)
    return count

MUTATIONS={
'end-queue-bit':('end_pass','[1]&=0xfe','[1]&=0xfd'),
'end-record-bit':('end_pass','group->array04[entry[1]].value30&=~2u','group->array04[entry[1]].value30&=~1u'),
'end-snapshot-count':('end_pass','for (unsigned int i=0;i<effect->value10C;++i)','for (unsigned int i=0,n=effect->value10C;i<n;++i)'),
'commit-processing-bit':('commit','effect->value118|=4;','effect->value118|=8;'),
'commit-skip-queued-clear':('commit','record->value30&=~2u;',''),
'commit-ignore-group-ref':('commit','if (group->value3C && record->value00)','if (record->value00)'),
'commit-ignore-record-dirty':('commit','if (group->value3C && record->value00)','if (group->value3C)'),
'commit-wrong-record-branch':('commit','if (record->value30&8)','if (!(record->value30&8))'),
'commit-wrong-sentinel':('commit','entry[0],entry[1],0xffff','entry[0],entry[1],0'),
'commit-return-on-error':('commit','UG2FxPassCachedRecord66A25F(effect,group,record);','{ int result=UG2FxPassCachedRecord66A25F(effect,group,record);if(result<0)return result; }'),
'commit-ignore-control':('commit','D3DXEffectControlFp(effect->value114,0x30000);',''),
'commit-wrong-control-mask':('commit','D3DXEffectControlFp(effect->value114,0x30000);','D3DXEffectControlFp(effect->value114,0x300);'),
'commit-retain-processing-bit':('commit','effect->value118&=~4u;',''),
'begin-only-state-one':('begin_pass','if (effect->value3C) {','if (effect->value3C==1) {'),
'begin-signed-index':('begin_pass','if (pass<technique->value34)','if (static_cast<int>(pass)<static_cast<int>(technique->value34))'),
'begin-skip-old-reference':('begin_pass','if (previous!=0xffff)','if (false)'),
'begin-old-delta':('begin_pass','selected[0]==selected[1] ? -1 : 0','selected[0]==selected[1] ? 1 : 0'),
'begin-clobber-upper-word':('begin_pass','reinterpret_cast<unsigned short *>(&effect->value44)[0]=static_cast<unsigned short>(pass);','effect->value44=static_cast<unsigned short>(pass);'),
'begin-new-delta':('begin_pass','static_cast<unsigned short>(pass)].value34),1,selected[0]==selected[1]','static_cast<unsigned short>(pass)].value34),-1,selected[0]==selected[1]'),
'begin-retain-flag80':('begin_pass','reinterpret_cast<unsigned char *>(&effect->value118)[0]&=0x7f;',''),
'begin-ignore-existing-pass':('begin_pass','if (!(effect->value118&0x100))','if (true)'),
'begin-publish-failed-pass':('begin_pass','if (result>=0) effect->value118|=0x100;','effect->value118|=0x100;'),
'group-ignore-cache-gate':('group_apply','if (value118&8)','if (true)'),
'group-invert-cache-selector':('group_apply','p->value18!=0xffff','p->value18==0xffff'),
'group-wrong-preparation':('group_apply','p->value18==0xffff ? UG2FxPrepareGroupB','p->value18!=0xffff ? UG2FxPrepareGroupB'),
'group-narrow-forwarded-index':('group_apply','UG2FxPassRecord669286(this,group,i,index)','UG2FxPassRecord669286(this,static_cast<unsigned short>(group),i,index)'),
'group-stop-state-one-errors':('group_apply','if (result<0 && value3C!=1)','if (result<0)'),
'group-ignore-state-four-errors':('group_apply','if (result<0) return result;\n        }\n        return 0;','if (result<0 && value3C!=1) return result;\n        }\n        return 0;'),
'texture-record-size':('record_texture','UG2FxMalloc(12)','UG2FxMalloc(8)'),
'no-texture-record-cleanup':('record_texture','value->Delete(1);return static_cast<int>(0x8007000e);','return static_cast<int>(0x8007000e);'),
'texture-reference-kind':('record_texture','references->value00=5','references->value00=4'),
'deferred-texture-tag':('record_texture','value->value00=12','value->value00=0'),
'no-texture-retain':('record_texture','if (texture) UG2FxComAddRef(texture);',''),
'stale-deferred-tail':('record_texture','if (texture) UG2FxComAddRef(texture);','unsigned int oldTail=valueFC;if (texture) UG2FxComAddRef(texture);'),
'no-deferred-publication':('record_texture','reinterpret_cast<D3DXEffectValuePrefix *>(valueFC)->value18=reinterpret_cast<unsigned int>(value);',''),
'scratch-texture-kind':('set_texture','reinterpret_cast<unsigned int *>(effect->valueC4)[0]=5;','reinterpret_cast<unsigned int *>(effect->valueC4)[0]=6;'),
'wrong-resolver-create':('set_texture','ResolveParameter(handle,1,1)','ResolveParameter(handle,1,0)'),
'reject-indexed-textures':('set_texture','type[4] && record->value08==0xffffffff','type[4]'),
'accept-wrong-texture-kind':('set_texture','static_cast<int>(type[0])<5','static_cast<int>(type[0])<4'),
'wrong-deferred-state':('set_texture','effect->value3C==3','effect->value3C==4'),
'unmapped-texture-cache':('set_texture','[mapped].value04','[index].value04'),
'notify-unchanged-texture':('set_texture','[mapped].value04) return 0;','[mapped].value04) return effect->NotifyValue(record);'),
'no-texture-notification':('set_texture','if (result>=0) result=effect->NotifyValue(record);',''),
'ignore-texture-write-error':('set_texture','if (result>=0) result=effect->NotifyValue(record);','result=effect->NotifyValue(record);'),
}

def mutation_checks(gcc):
    rejected=[]
    for name,(kind,before,after)in MUTATIONS.items():
        content=SOURCE.read_text();assert before in content,name;content=content.replace(before,after,1)
        if name=='stale-deferred-tail':content=content.replace('reinterpret_cast<D3DXEffectValuePrefix *>(valueFC)->value18','reinterpret_cast<D3DXEffectValuePrefix *>(oldTail)->value18')
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content)
        command=[sys.executable,__file__,'--cases','1024','--kind',kind,'--replace',str(path)]+(['--gcc']if gcc else[])
        result=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(result.stdout)
        assert result.returncode and 'effect pass mismatch'in result.stdout,(name,result.returncode,result.stdout[-3000:]);rejected.append(name)
    return rejected

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=4096);p.add_argument('--kind',choices=['all',*KINDS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in list(manifest['functions'].values())+manifest['data']:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));report=verify(image,linked,args.gcc,args.cases,KINDS if args.kind=='all'else[args.kind]);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.write_native_fixtures:report['native_captures']=export_native(image,args.write_native_fixtures)
    report['source_sha256']=hashlib.sha256((args.replace or SOURCE).read_bytes()).hexdigest()
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
