#!/usr/bin/env python3
"""Verify original effect state-block caches, shader reset and sampler defaults."""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects
from pc_native_elf import link_native
import verify_pc_effect_input as core
ROOT,EXE=core.ROOT,core.EXE;LIB=ROOT/'src/Speed/Indep/Libs/D3DX';SOURCE=LIB/'EffectStateCache.cpp';BUILD=ROOT/'build/analysis/effect-state-cache';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-state-cache.json'
ARENA,HEAP,STOP=core.ARENA,core.HEAP,core.STOP;EFFECT=ARENA+0x1000
TECH,REQ,DATA,MANAGER,OTHER,VTABLE,BVTABLE,OLD,BLOCKS,OLDBUF=[ARENA+x for x in(0x3000,0x5000,0x5800,0x1800,0x1820,0x1a00,0x1d00,0xa000,0xc000,0)];OLDBUF=HEAP+0x4000;MODEL_HEAP=0x5000
SPANS=dict(vertex_cache=(0x66403c,347),pixel_cache=(0x664197,347),capture_vertex_constants=(0x66356e,305),capture_vertex_samplers=(0x66369f,305),capture_pixel_constants=(0x6637d0,305),capture_pixel_samplers=(0x663901,305),reset_vertex_constants=(0x660205,289),reset_pixel_constants=(0x6603b4,293),reset_vertex_samplers=(0x660326,142),reset_pixel_samplers=(0x6604d9,131),vertex_booleans=(0x65aa5d,77))
METHODS=dict(vertex_cache='CacheVertexState',pixel_cache='CachePixelState',capture_vertex_constants='CaptureVertexConstants',capture_vertex_samplers='CaptureVertexSamplers',capture_pixel_constants='CapturePixelConstants',capture_pixel_samplers='CapturePixelSamplers',reset_vertex_constants='ResetVertexConstants',reset_pixel_constants='ResetPixelConstants',reset_vertex_samplers='ResetVertexSamplers',reset_pixel_samplers='ResetPixelSamplers')
# name: (vtable offset, argument count, receiver class, payload lane size)
APIS=dict(Begin=(0xf0,1,'device',0),End=(0xf4,2,'device',0),Capture=(0x10,1,'block',0),Apply=(0x14,1,'block',0),Release=(8,1,'block',0),SetVertexShader=(0x170,2,'device',0),SetPixelShader=(0x1ac,2,'device',0),VertexFloat=(0x178,4,'device',16),VertexInt=(0x180,4,'device',16),VertexBool=(0x188,4,'device',4),PixelFloat=(0x1b4,4,'device',16),PixelInt=(0x1bc,4,'device',16),PixelBool=(0x1c4,4,'device',4),SetTexture=(0x104,3,'device',0),SetSampler=(0x114,4,'device',0))
NAMES={k:'UG2FxStateBlock'+k if k in('Begin','End','Capture','Apply','Release')else'UG2FxCache'+k for k in APIS}
TARGETS={k:STOP+0x9000+j*16 for j,k in enumerate(APIS)};DEBUG=STOP+0x9200

def pack(words):return struct.pack('<'+'I'*len(words),*(v&0xffffffff for v in words))
def build(gcc,source):
    BUILD.mkdir(parents=True,exist_ok=True);bindings=dict(UG2FxMalloc=0x575620,UG2FxFree=0x575630,D3DXEffectDebugMute=DEBUG,ug2FxSamplerDefaults7B6C54=0x7b6c54,**{NAMES[k]:a for k,a in TARGETS.items()})
    if gcc:
        linked=link_native([source],BUILD,ROOT,bindings,'D3DXEffectSetVertexBooleans',text_address=0x5000000,include_dirs=[LIB]);prefixes={k:'_ZN16D3DXEffectObject'+str(len(v))+v for k,v in METHODS.items()};prefixes['vertex_booleans']='D3DXEffectSetVertexBooleans'
    else:
        obj=BUILD/'state-cache.obj';subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc','/I'+str(LIB.relative_to(ROOT)),'/Fo'+str(obj.relative_to(ROOT)),str(source.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()};bindings['_D3DXEffectDebugMute@4']=DEBUG
        linked=link_objects([obj],bindings,start=0x5000000);prefixes={k:'?'+v+'@D3DXEffectObject@@'for k,v in METHODS.items()};prefixes['vertex_booleans']='_D3DXEffectSetVertexBooleans@16'
    linked['entries']={k:next(a for name,a in linked['exports'].items()if name.startswith(p))for k,p in prefixes.items()};return linked

def cases(n):
    rng=random.Random(0x66403c)
    for i,c in enumerate(core.cases(n)):
        c.update(seed=i,flags=(0,8,0x18,0x118,0x1f8,0xffffffff&~0x200)[i%6],selected=(i//6)%2,enabled=(0,1,7,0xffffffff)[(i//2)%4],
            sizes=((0,1,2,4,16,32,256)[(i//3)%7],(0,1,2,4,16)[(i//4)%5],(0,1,2,4,16)[(i//5)%5],(i//7)%5),old_sizes=((0,1,2,4,16,256)[(i//4)%6],(0,1,2,4,16)[(i//5)%5],(0,1,2,4,16)[(i//6)%5],(i//11)%4),
            old_blocks=(i//3)%5,ready=(0,1,0xffffffff)[(i//5)%3],capacity=(0,16,32,64,128,0xffffffff)[(i//13)%6],old_buffer=i%5!=0,
            fail_at=(0,1,2,3,4,5,6,7,8,10,14,20,40)[(i//4)%13],failure=(0,1,0x80004005,0x8876086c,0xffffffff)[(i//52)%5],out_on_failure=i%2!=0,
            callback_at=(0,1,2,3,4,5,8,12,20)[(i//17)%9],mutations=(i//19)%32,fail_allocation=(0,1,2)[(i//23)%3],
            word=(0x7f,0x27f,0x37f)[i%3]|((i//3)%4)*0x400,mxcsr=0x1f80|((i//12)%4)*0x2000|((i//48)%4 in(1,3))*0x40|((i//48)%4 in(2,3))*0x8000,live=i%3,top=i//3%8,status=i%64,
            start=(0,1,0xffffffff,0xfffffffe)[i%4],boolean_count=i%5)
        yield c

class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.rebuilt=bool(linked)
        if not linked:self.entries={k:a for k,(a,n)in SPANS.items()}
        for k,a in TARGETS.items():self.u.hook_add(UC_HOOK_CODE,self.api,user_data=k,begin=a,end=a)
        at=DEBUG if linked else 0x671aed;self.u.hook_add(UC_HOOK_CODE,self.debug,begin=at,end=at)
    def record(self,name,args,payload=b''):
        self.events.append((name,tuple(args),payload,bytes(self.u.mem_read(EFFECT,0x130)),bytes(self.u.mem_read(TECH,0x168)),self.requests,self.allocated,self.calls,self.ends,self.u.reg_read(xc.UC_X86_REG_FPCW)))
    def transition(self):
        if self.calls!=self.case['callback_at']:return
        m=self.case['mutations']
        if m&1:self.put(EFFECT+0xc,OTHER)
        if m&2:self.u.mem_write(EFFECT+0x42,struct.pack('<H',1-self.case['selected']))
        if m&4:
            for j in range(4):self.put(TECH+self.case['selected']*0xb4+0x8c+j*4,OLD+0x400+j*0x20)
        if m&8:self.u.mem_write(REQ,struct.pack('<4H',1,2,1,2))
        if m&16 and self.word(EFFECT+0xcc):self.put(self.word(EFFECT+0xcc),0x7f800001)
    def api(self,u,address,size,k):
        offset,n,receiver,lanes=APIS[k];a=self.args(n);args=a;payload=b''
        if lanes:args=(*a[:2],a[3]);payload=bytes(u.mem_read(a[2],a[3]*lanes))
        if k=='End':args=(a[0],self.word(a[1]))
        self.record(k,args,payload);self.calls+=1;self.transition();result=self.case['failure']if self.calls==self.case['fail_at']else 0
        if k=='End':
            if not(result&0x80000000)or self.case['out_on_failure']:
                token=BLOCKS+self.ends*0x20;self.ends+=1;assert self.ends<=32;self.put(token,BVTABLE,1,0,0);self.put(a[1],token)
        elif receiver=='block':
            assert a[0],('null block receiver',k)
            off=dict(Capture=8,Apply=12,Release=4)[k]
            value=(self.word(a[0]+off)+(-1 if k=='Release'else 1))&0xffffffff;self.put(a[0]+off,value)
            if k=='Release':result=value
        self.ret(result,0 if self.rebuilt else n*4)
    def debug(self,u,address,size,_):self.record('DebugMute',self.args(1));self.ret(0,4)
    def state(self):return bytes(self.u.mem_read(ARENA,core.SIZE))+bytes(self.u.mem_read(HEAP,MODEL_HEAP))
    def runtime(self,u,address,size,k):
        assert k!='abort';arg,=self.args(1);self.record(k,[arg])
        if k=='malloc':
            self.requests+=1;result=0
            if self.requests!=self.case['fail_allocation']:
                result=HEAP+self.allocated;self.allocated+=max(16,(arg+15)&~15);assert self.allocated<0x3000;self.live[result]=arg
            self.ret(result)
        else:
            assert not arg or arg in self.live,('invalid free',hex(arg));
            if arg:self.live.pop(arg)
            self.ret()
    def setup(self,c):
        self.case=c;self.events=[];self.calls=self.ends=self.requests=self.allocated=0;self.live={}
        self.u.mem_write(ARENA,c['arena']);self.u.mem_write(HEAP,c['heap']);self.put(0,0x12345678)
        self.put(EFFECT,0x7b6fe0);self.put(EFFECT+0xc,MANAGER);self.put(EFFECT+0x40,0xa5a5|(c['selected']<<16));self.put(EFFECT+0xa4,TECH);self.put(EFFECT+0x118,c['flags'])
        self.put(EFFECT+0xc8,c['capacity'],OLDBUF if c['old_buffer']else 0)
        if c['old_buffer']:self.live[OLDBUF]=4096
        else:self.put(EFFECT+0xc8,0)
        self.u.mem_write(REQ,struct.pack('<4H',*c['sizes']))
        for j in range(2):
            t=TECH+j*0xb4
            for k in range(4):self.put(t+0x8c+k*4,OLD+(j*4+k)*0x20 if(c['old_blocks']>k)else 0)
            self.put(t+0x9c,c['ready'],c['ready']);self.u.mem_write(t+0xa4,struct.pack('<4H',*c['old_sizes']));self.u.mem_write(t+0xac,struct.pack('<4H',*c['old_sizes']))
        self.put(MANAGER,VTABLE);self.put(OTHER,VTABLE)
        for k,(off,n,receiver,_)in APIS.items():self.put((VTABLE if receiver=='device'else BVTABLE)+off,TARGETS[k])
        for j in range(40):self.put(OLD+j*0x20,BVTABLE,4,0,0)
        self.put(DATA,*[(0,1,0xffffffff,0x80000000,0x7f800001)[(c['seed']+j)%5]for j in range(8)])
        self.u.reg_write(xc.UC_X86_REG_MXCSR,c['mxcsr'])
    def arguments(self,k,c):return(MANAGER,c['start'],DATA,c['boolean_count'])if k=='vertex_booleans'else(c['enabled'],REQ)
    def fp(self):
        u=self.u;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        return(u.reg_read(xc.UC_X86_REG_FPCW),u.reg_read(xc.UC_X86_REG_MXCSR),u.reg_read(xc.UC_X86_REG_FPSW)&0x383f,tag,tuple((j,u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if(tag>>(j*2)&3)!=3))
    def call(self,k,args):
        u=self.u;member=k!='vertex_booleans';params=(EFFECT,*args)if self.rebuilt and self.gcc and member else args;sp=core.STACK+0xf000;self.put(sp,STOP,*params)
        regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        top=self.case['top'];tag=0xffff
        for j in range(self.case['live']):p=(top+j)&7;u.reg_write(xc.UC_X86_REG_FP0+p,(0x8000000000000000,0x3fff|(j&1)*0x8000));tag&=~(3<<(p*2))
        for r,v in((xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_ECX,EFFECT),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,self.case['word']),(xc.UC_X86_REG_FPTAG,tag),(xc.UC_X86_REG_FPSW,(top<<11)|self.case['status'])):u.reg_write(r,v)
        before=self.fp();u.emu_start(self.entries[k],STOP,count=1000000);pop=0 if member and self.rebuilt and self.gcc else len(args)*4
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4+pop
        assert tuple(u.reg_read(r)for r in regs)==saved and self.fp()==before,('FP/ABI',k,before,self.fp())
        return u.reg_read(xc.UC_X86_REG_EAX)
    def run(self,k,c):self.setup(c);result=self.call(k,self.arguments(k,c));return self.snapshot(result)+(self.fp(),)

def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for k in kinds:
            try:x,y=a.run(k,c),b.run(k,c)
            except Exception as e:raise AssertionError(('effect state cache mismatch',k,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:8];event=next(((j,v[:3],w[:3])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect state cache mismatch',k,i,diffs,x[2],y[2],event,len(x[3]),len(y[3])))
            counts[k]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),calls=dict(events))

def source_digest(source):
    content=source.read_bytes()
    for name in('EffectStateCacheGate.inc','EffectStateCacheCapture.inc'):
        path=source.parent/name
        content+=(path if path.exists()else LIB/name).read_bytes()
    return hashlib.sha256(content).hexdigest()

def trace_bytes(events):
    out=bytearray()
    for name,args,payload,effect,technique,requests,allocated,calls,ends,cw in events:
        out.extend(name.encode()+b'\0'+pack([len(args),*args,len(payload)])+payload+effect+technique+pack([requests,allocated,calls,ends,cw]))
    return bytes(out)

def export_native(image,path):
    selected=set(range(64))|{(37*j)%2048 for j in range(64)};m=Machine(image);temporary=path.with_name(path.name+'.partial')
    with temporary.open('wb')as f:
        f.write(b'UG2FXSC1'+pack([len(selected)*len(SPANS)]))
        for i,c in enumerate(cases(2048)):
            if i not in selected:continue
            c=dict(c,top=(-c['live'])&7)
            for k in SPANS:
                m.setup(c);initial=bytes(m.u.mem_read(ARENA,core.SIZE))+bytes(m.u.mem_read(HEAP,MODEL_HEAP));args=m.arguments(k,c)
                result=m.call(k,args);trace=trace_bytes(m.events)
                fields=[list(SPANS).index(k),len(args),*(list(args)+[0]*(4-len(args))),c['selected'],c['word'],c['mxcsr'],c['live'],c['status'],c['fail_allocation'],c['fail_at'],c['failure'],int(c['out_on_failure']),c['callback_at'],c['mutations'],result,len(trace),m.requests,m.allocated,m.calls,m.ends,len(m.live)]
                f.write(pack(fields)+initial+bytes(m.u.mem_read(ARENA,core.SIZE))+bytes(m.u.mem_read(HEAP,MODEL_HEAP))+trace)
    temporary.replace(path);return len(selected)*len(SPANS)

MUTATIONS={
'gate-ignore-enabled-bit':('vertex_cache','gate','if (!(value118&8))','if (false)'),
'gate-ignore-constant-bit':('pixel_cache','gate','if (value118&0x10)','if (true)'),
'gate-truncate-size':('vertex_cache','gate','requested[0]<=old[0]','static_cast<unsigned char>(requested[0])<=old[0]'),
'gate-shrink-capacity':('vertex_cache','gate','old[0]>requested[0] ? old[0] : requested[0]','requested[0]'),
'gate-ignore-retained-shader':('pixel_cache','gate','(*ready || enabled) ? 1 : 0','enabled ? 1 : 0'),
'gate-wrong-ready-store':('vertex_cache','gate','*ready=captureShader','*ready=enabled'),
'gate-write-after-failure':('pixel_cache','gate','if (result<0) return result;','if (false) return result;'),
'gate-force-sampler-recapture':('vertex_cache','gate','*oldSampler<requested[3]','*oldSampler<=requested[3]'),
'gate-shrink-sampler-count':('pixel_cache','gate','requested[3]<*oldSampler ? *oldSampler : requested[3]','requested[3]'),
'gate-write-failed-sampler-count':('vertex_cache','gate','if (result>=0) *oldSampler=count;','*oldSampler=count;'),
'capture-wrong-slot':('capture_vertex_constants','source','#define FX_SLOT 0x94','#define FX_SLOT 0x98'),
'capture-one-block':('capture_pixel_constants','capture','i<2','i<1'),
'capture-no-mute-restore':('capture_vertex_constants','capture','D3DXEffectDebugMute(0);',''),
'capture-no-failed-end':('capture_pixel_constants','capture','UG2FxStateBlockEnd(value0C,blocks+i);goto finish;','goto finish;'),
'capture-use-failed-end-result':('capture_vertex_constants','capture','UG2FxStateBlockEnd(value0C,blocks+i);goto finish;','result=UG2FxStateBlockEnd(value0C,blocks+i);goto finish;'),
'capture-skip-first-capture':('capture_pixel_constants','capture','result=UG2FxStateBlockCapture(blocks[0]);','result=0;'),
'capture-skip-old-apply':('capture_vertex_samplers','capture','result=*slot ? UG2FxStateBlockApply(*slot) : 0;','result=0;'),
'capture-wrong-restore-block':('capture_pixel_samplers','capture','result=UG2FxStateBlockApply(blocks[0]);','result=UG2FxStateBlockApply(blocks[1]);'),
'capture-release-published-block':('capture_vertex_samplers','capture','*slot=blocks[1];blocks[1]=0;','*slot=blocks[1];'),
'capture-leak-first-block':('capture_pixel_constants','capture','if (blocks[0]) { UG2FxStateBlockRelease(blocks[0]);blocks[0]=0; }',''),
'capture-stale-manager':('capture_vertex_constants','capture','unsigned int blocks[2]={0,0};','unsigned int manager=value0C;unsigned int blocks[2]={0,0};'),
'capture-stale-old-block':('capture_pixel_samplers','capture','unsigned int blocks[2]={0,0};','unsigned int previous=*slot;unsigned int blocks[2]={0,0};'),
'zero-buffer-wrong-int-size':('reset_vertex_constants','source','bytes<=sizes[2]*16u','bytes<=sizes[2]*4u'),
'zero-buffer-no-initialize':('reset_pixel_constants','source','grown[i]=0;','grown[i]=0x7f800001;'),
'zero-buffer-no-free':('reset_vertex_constants','source','UG2FxFree(reinterpret_cast<void *>(effect->valueCC));',''),
'zero-buffer-change-failed-state':('reset_pixel_constants','source','if (!grown) return static_cast<int>(0x8007000e);','if (!grown) {effect->valueCC=0;return static_cast<int>(0x8007000e);}'),
'no-vertex-shader-reset':('reset_vertex_constants','source','if (enabled) { result=UG2FxCacheSetVertexShader','if (false) { result=UG2FxCacheSetVertexShader'),
'bulk-vertex-booleans':('reset_vertex_constants','source','D3DXEffectSetVertexBooleans(value0C,0,reinterpret_cast<unsigned int *>(valueCC),sizes[1])','UG2FxCacheVertexBool(value0C,0,reinterpret_cast<unsigned int *>(valueCC),sizes[1])'),
'wrong-pixel-bool-method':('reset_pixel_constants','source','UG2FxCachePixelBool(value0C,0','UG2FxCachePixelInt(value0C,0'),
'ignore-vertex-float-failure':('reset_vertex_constants','source','result=UG2FxCacheVertexFloat(value0C,0,reinterpret_cast<void *>(valueCC),sizes[0]);if (result<0) return result;','result=UG2FxCacheVertexFloat(value0C,0,reinterpret_cast<void *>(valueCC),sizes[0]);'),
'vertex-sampler-base':('reset_vertex_samplers','source','stage=0x101','stage=0x100'),
'vertex-sampler-missing-default':('reset_vertex_samplers','source','offset<0x188','offset<0x16c'),
'pixel-sampler-extra-default':('reset_pixel_samplers','source','offset<0x16c','offset<0x188'),
'sampler-missing-key-mask':('reset_vertex_samplers','source','row[0]&0xffffff','row[0]'),
'sampler-wrong-default-word':('reset_vertex_samplers','source','reinterpret_cast<const unsigned int *>(row[1])[1]','reinterpret_cast<const unsigned int *>(row[1])[0]'),
'boolean-one-instead-of-mask':('vertex_booleans','source','data[i] ? 0xffffffff : 0','data[i] ? 1 : 0'),
'boolean-wrong-start':('vertex_booleans','source','device,start+i,&value,1','device,i,&value,1'),
'boolean-ignore-failure':('vertex_booleans','source','if (result<0) return result;','if (false) return result;'),
}

def mutation_source(name,gcc):
    kind,where,before,after=MUTATIONS[name];folder=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name);folder.mkdir(parents=True,exist_ok=True)
    files={'source':'EffectStateCache.cpp','gate':'EffectStateCacheGate.inc','capture':'EffectStateCacheCapture.inc'}
    for key,filename in files.items():
        content=(LIB/filename).read_text()
        if key==where:
            assert before in content,name;content=content.replace(before,after,1)
            if name=='capture-stale-manager':content=content.replace('UG2FxStateBlockBegin(value0C)','UG2FxStateBlockBegin(manager)').replace('UG2FxStateBlockEnd(value0C','UG2FxStateBlockEnd(manager')
            if name=='capture-stale-old-block':content=content.replace('result=*slot ? UG2FxStateBlockApply(*slot) : 0;','result=previous ? UG2FxStateBlockApply(previous) : 0;')
            if name=='zero-buffer-wrong-int-size':content=content.replace('bytes=sizes[2]*16u','bytes=sizes[2]*4u')
        (folder/filename).write_text(content)
    return kind,folder/files['source']

def mutation_checks(gcc):
    rejected=[]
    for name in MUTATIONS:
        kind,path=mutation_source(name,gcc);result=subprocess.run([sys.executable,__file__,'--cases','1024','--kind',kind,'--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        (path.parent/'result.log').write_text(result.stdout);assert result.returncode and 'effect state cache mismatch'in result.stdout,(name,result.returncode,result.stdout[-3000:]);rejected.append(name)
    return rejected

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);args=p.parse_args();assert args.cases>0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();m=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for row in list(m['functions'].values())+m['data']:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    report=verify(image,build(args.gcc,args.replace or SOURCE),args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind]);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.write_native_fixtures:report['native_captures']=export_native(image,args.write_native_fixtures)
    report['source_sha256']=source_digest(args.replace or SOURCE)
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
