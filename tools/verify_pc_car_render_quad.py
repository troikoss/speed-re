#!/usr/bin/env python3
"""Verify original quad packing/submission and its actual bind/world composition.

Isolated cases control the two engine calls and graphics APIs to expose raw-bit
packing, callback re-reads and FP preservation. Composed cases execute recovered
texture/world uploads; only effect/device services remain controlled.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys,zlib
from pathlib import Path
import pefile
import unicorn.x86_const as xc
import verify_pc_car_render_material as material
uploads,core=material.uploads,material.core
ROOT,EXE,WORLD=material.ROOT,material.EXE,uploads.WORLD
SOURCE=WORLD/'CarRenderQuadSubmit.cpp';BUILD=ROOT/'build/analysis/car-render-quad';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/car-render-quad.json'
QUAD=core.ARENA+0x23000;STAGES=('bind','world','overlay','commit','begin','draw','end')
GLOBALS=dict(ug2RenderQuadTexture8708B4=0x8708b4,ug2RenderQuadCalls872F08=0x872f08,ug2RenderQuadEnabled7FF05C=0x7ff05c)
PREFIX=dict(quad='?CarRenderSubmitQuadRecovered@@',finish='?CarRenderFinishQuad@@')
GCC={k:'_Z'+str(len(n))+n for k,n in dict(quad='CarRenderSubmitQuadRecovered',finish='CarRenderFinishQuad').items()}

def metadata():
    result=json.loads(core.MANIFEST.read_text())
    for p in(uploads.MANIFEST,material.MANIFEST,MANIFEST):result['functions'].update(json.loads(p.read_text())['functions'])
    return result

def build(gcc,source):
    uploads.BUILD=BUILD;BUILD.mkdir(parents=True,exist_ok=True)
    linked=uploads.build(uploads.UPLOADS,uploads.RUNTIME,uploads.BRIDGE,gcc,more_sources=[material.SOURCE,source],more_globals=dict(material.GLOBALS,**GLOBALS),more_supplied=[material.FUNCTIONS[k]for k in material.COVERED])
    for table in((material.GCC if gcc else material.PREFIX),(GCC if gcc else PREFIX)):
        for k,p in table.items():linked['entries'][k]=next(a for n,a in linked['exports'].items()if n.startswith(p))
    return linked

class Machine(material.Machine):
    def state_words(self):return tuple(self.word(a)for a in(0x870910,0x87092c,0x872f08,0x8708b4,0x870848))+(self.u.mem_read(0x7ff05c,1)[0],)
    def record(self,name,args=(),payload=b''):self.events.append((name,tuple(args),payload,self.state_words()));self.trace.append(self.events[-1])
    def transition(self,stage):
        n=STAGES.index(stage);c=self.case
        if c['swap']&(1<<n):self.put(0x870910,core.TECH+(n+1)*0x28)
        if c['flip']&(1<<n):self.u.mem_write(0x7ff05c,bytes([0 if self.u.mem_read(0x7ff05c,1)[0]else 0x80]))
        if stage=='bind'and c['mutate']:
            self.put(QUAD,c['edit']);self.put(0x870848,core.MODE+0x10)
        if stage=='world'and c['mutate']:self.put(0x8708b4,c['extra_after'])
        if stage=='end'and c['mutate']:self.put(0x872f08,c['counter_after'])
    def material_observe(self,u,address,size,k):
        if k=='bind'and self.kind in('quad','finish'):
            self.record(k,self.args(1));self.transition(k)
            if not self.case['composed']:self.ret()
        else:super().material_observe(u,address,size,k)
    def observe(self,u,address,size,k):
        if k=='world'and self.kind in('quad','finish'):
            self.world_seen=True;self.record(k,self.args(2));self.transition(k)
            if not self.case['composed']:self.ret()
        else:super().observe(u,address,size,k)
    def api(self,u,address,size,info):
        if self.kind not in('quad','finish'):return super().api(u,address,size,info)
        name,n=info;a=self.args(n);args=a;payload=b'';stage=None
        if name=='UG2RenderDrawImmediate':
            assert a[1:3]==(6,2)and a[4]==32
            args=(a[0],a[1],a[2],a[4]);payload=b''.join(bytes(u.mem_read(a[3]+j*32,24))for j in range(4));stage='draw'
        elif name in('UG2EffectSetMatrix','UG2RenderSetTransform'):
            args=a[:2];payload=bytes(u.mem_read(a[2],64))
        elif name in('UG2EffectSetVector','UG2EffectSetVectorArray','UG2EffectSetIntArray'):
            count=4 if name=='UG2EffectSetVector'else a[3]*(4 if name=='UG2EffectSetVectorArray'else 1)
            args=(*a[:2],count);payload=bytes(u.mem_read(a[2],count*4))
        elif name=='UG2EffectSetTexture'and self.world_seen:stage='overlay'
        elif name in('UG2EffectCommit','UG2EffectBeginPass','UG2EffectEndPass'):stage={'UG2EffectCommit':'commit','UG2EffectBeginPass':'begin','UG2EffectEndPass':'end'}[name]
        elif name not in('UG2RenderSetState','UG2EffectSetTexture','UG2EffectSetFloat','UG2EffectSetInt'):raise AssertionError(('unexpected quad API',name,a))
        self.record(name,args,payload)
        if stage:self.transition(stage)
        self.ret(self.case['hresult'],0 if self.rebuilt else n*4)
    def prepare(self,kind,c):
        super().prepare(kind,c)
        if kind in('quad','finish'):
            self.kind=kind;self.world_seen=False;self.put(0x870910,0 if c['null_technique']else core.TECH)
    def fp(self):
        u=self.u;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        return(u.reg_read(xc.UC_X86_REG_FPCW),u.reg_read(xc.UC_X86_REG_MXCSR),u.reg_read(xc.UC_X86_REG_FPSW)&0x383f,tag,tuple((j,u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if(tag>>(j*2)&3)!=3))
    def call(self,kind,args=(),receiver=0):
        if self.case['composed']:return super().call(kind,args,receiver)
        u=self.u;sp=core.STACK+0xf000;self.put(sp,core.STOP,*args)
        regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x12345678,0x23456789,0x3456789a,0x456789ab)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        top=self.case['top'];tag=0xffff
        for j in range(self.case['live']):
            p=(top+j)&7;u.reg_write(xc.UC_X86_REG_FP0+p,(0x8000000000000000,0x3fff|(j&1)*0x8000));tag&=~(3<<(p*2))
        for r,v in((xc.UC_X86_REG_ESP,sp),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,self.case['cw']),(xc.UC_X86_REG_MXCSR,self.case['mxcsr']),(xc.UC_X86_REG_FPSW,(top<<11)|self.case['status']),(xc.UC_X86_REG_FPTAG,tag)):u.reg_write(r,v)
        before=self.fp();u.emu_start(self.entries[kind],core.STOP,count=100000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==core.STOP and u.reg_read(xc.UC_X86_REG_ESP)==sp+4
        assert tuple(u.reg_read(r)for r in regs)==saved and self.fp()==before,('quad FP/ABI preservation',kind,before,self.fp())
    def run(self,kind,c):
        self.kind=kind;self.trace=[];self.reset(kind,c);
        self.u.reg_write(xc.UC_X86_REG_MXCSR,c['mxcsr']);args=(QUAD,core.TEXTURE,core.WORLD,c['fourth'])if kind=='quad'else()
        self.call(kind,args)
        return(self.snapshot(None),self.state_words(),bytes(self.u.mem_read(0x7ff05c,1)))


def cases(n,image,composed=False):
    rng=random.Random(0x5c5580)
    for i,c in enumerate(material.cases(n,image)):
        c=dict(c);data=bytearray(c['data']);g=dict(c['globals']);data[QUAD-core.ARENA:QUAD-core.ARENA+116]=rng.randbytes(116)
        # Include signaling payloads in every coordinate/UV lane over the corpus.
        pattern=(0,0x80000000,0x7f800001,0xff800001,0x7fc12345,0x7f800000,0xff800000,1,0x807fffff,rng.getrandbits(32))
        for j in range(24):struct.pack_into('<I',data,QUAD-core.ARENA+j*4,pattern[(i+j)%len(pattern)])
        for j in range(23):struct.pack_into('<I',data,core.PARAM+j*0xa00+0x1b4-core.ARENA,(0,0x8028,0xdeadbeef,rng.getrandbits(32))[(i+j)%4])
        struct.pack_into('<2I',data,core.MODE+0x10-core.ARENA,0,(0,1,9,10)[(i+1)%4])
        g[0x8708b4]=struct.pack('<I',0 if i%3==0 else 0xabcd0000+i);g[0x7ff05c]=bytes([(0,1,128,255)[i%4]]);g[0x872f08]=struct.pack('<I',(0,0xffffffff,0xfffffffe,i)[i%4])
        c.update(data=bytes(data),globals=g,composed=composed,mutate=i%3==0,swap=(i//4)%128,flip=(i//64)%128,extra_after=0 if i%2 else 0xdcba0000+i,edit=rng.getrandbits(32),counter_after=rng.getrandbits(32),fourth=rng.getrandbits(32),live=i%3,top=i//3%8,status=i%64,mxcsr=0x1f80|((i//64)%4)*0x2000|((i//256)%4 in(1,3))*0x40|((i//256)%4 in(2,3))*0x8000,cw=(0x7f,0x27f,0x37f)[i%3]|((i//3)%4)*0x400,null_technique=False)
        if i%127==0:c.update(null_technique=True,swap=0,flip=0,mutate=False);g[0x8708b4]=bytes(4);g[0x7ff05c]=bytes(1)
        if composed and c['null_technique']:
            struct.pack_into('<I',data,core.STATE+0x18-core.ARENA,0)
            g[0x87684c]=struct.pack('<2I',0,core.WORLD);g[0x800534]=bytes(data[core.MODE+4-core.ARENA:core.MODE+8-core.ARENA]);c['data']=bytes(data)
        if not c['null_technique']and i%131==1:
            c['mutate']=True;g[0x870848]=bytes(4)
        yield c


def verify(image,manifest,linked,gcc,n,composed):
    a,b=Machine(image,manifest),Machine(image,manifest,linked,gcc);counts=Counter();events=Counter()
    for i,c in enumerate(cases(n,image,composed)):
        for kind in('quad','finish')if not composed else('quad',):
            try:x,y=a.run(kind,c),b.run(kind,c)
            except Exception as e:raise AssertionError(('quad submission mismatch',composed,kind,i,str(e)))from e
            assert x==y,('quad submission mismatch',composed,kind,i,next(((p,q)for p,q in zip(x[0][2],y[0][2])if p!=q),None),x[1:],y[1:])
            counts[kind]+=1;events.update(e[0]for e in a.events)
    return dict(comparisons=dict(counts),observed_calls=dict(events))


def trace_bytes(events):
    result=bytearray()
    for name,args,payload,state in events:
        raw=name.encode()+b'\0'+struct.pack('<I',len(args))+struct.pack('<'+'I'*len(args),*(v&0xffffffff for v in args))+struct.pack('<I',len(payload))+payload+struct.pack('<6I',*state)
        result.extend(raw)
    return bytes(result)

def export_native(image,manifest,path):
    n_iso,n_composed=1024,128;machine=Machine(image,manifest)
    temporary=path.with_name(path.name+'.partial')
    with temporary.open('wb')as f:
        f.write(b'UG2QDS01'+struct.pack('<I',n_iso+n_composed))
        for composed,n in((False,n_iso),(True,n_composed)):
            for i,c in enumerate(cases(n,image,composed)):
                machine.kind='quad';machine.trace=[];machine.reset('quad',c);machine.u.reg_write(xc.UC_X86_REG_MXCSR,c['mxcsr'])
                before=bytes(machine.u.mem_read(core.ARENA,core.SIZE))
                regions=sorted((a,len(b))for a,b in c['globals'].items())
                globals_before=[bytes(machine.u.mem_read(a,size))for a,size in regions]
                machine.call('quad',(QUAD,core.TEXTURE,core.WORLD,c['fourth']))
                trace=trace_bytes(machine.trace)
                fields=[int(composed),int(c['mutate']),c['swap'],c['flip'],c['extra_after'],c['edit'],c['counter_after'],c['fourth'],c['live'],c['top'],c['status'],c['mxcsr'],c['cw'],c['hresult']&0xffffffff,zlib.crc32(trace),zlib.crc32(machine.u.mem_read(core.ARENA,core.SIZE)),len(regions),len(trace)]
                f.write(struct.pack('<18I',*fields));f.write(before)
                for (a,size),body in zip(regions,globals_before):f.write(struct.pack('<3I',a,size,zlib.crc32(machine.u.mem_read(a,size)))+body+bytes(machine.u.mem_read(a,size)))
                f.write(bytes(machine.u.mem_read(core.ARENA,core.SIZE)));f.write(trace)
    temporary.replace(path)
    return dict(isolated=n_iso,composed=n_composed)

def mutation_checks(gcc):
    variants={
        'position-stride':('positions[j*4+1]','positions[j*3+1]'),
        'colour-order':('(colour&0xff000000)|((colour&0xff)<<16)|(colour&0xff00)|((colour>>16)&0xff)','colour'),
        'uv-lane':('vertices[j][5]=uv[j*2+1]','vertices[j][5]=uv[j*2]'),
        'overlay-handle':('slots00[10].handle24','slots00[7].handle24'),
        'zero-overlay-handle':('if (extra) {','if (extra&&ug2RenderTechnique870910->parameters1C->slots00[10].handle24) {'),
        'early-mode-check':('    unsigned int vertices[4][8];','    if(!ug2SolidViewMode870848) return;\n    unsigned int vertices[4][8];'),
        'skip-world':('CarRenderUploadWorld(world,ug2SolidViewMode870848[1]);',''),
        'stale-view':('CarRenderBindTexture(texture);\n    CarRenderUploadWorld(world,ug2SolidViewMode870848[1]);','int view=ug2SolidViewMode870848[1];CarRenderBindTexture(texture);\n    CarRenderUploadWorld(world,view);'),
        'stale-extra':('CarRenderUploadWorld(world,ug2SolidViewMode870848[1]);\n    unsigned int extra=ug2RenderQuadTexture8708B4;','unsigned int extra=ug2RenderQuadTexture8708B4;\n    CarRenderUploadWorld(world,ug2SolidViewMode870848[1]);'),
        'stale-gate':('if (extra) {','unsigned int enabled=ug2RenderQuadEnabled7FF05C;\n    if (extra) {'),
        'stale-technique':('UG2EffectCommit(ug2RenderTechnique870910->effect20);\n        CarRenderTechnique *afterCommit=ug2RenderTechnique870910;','CarRenderTechnique *afterCommit=ug2RenderTechnique870910;\n        UG2EffectCommit(afterCommit->effect20);'),
        'skip-cache-clear':('ug2RenderVertex87092C=0;',''),
        'primitive-kind':('ug2SolidDevice870974,6,2,vertices','ug2SolidDevice870974,5,2,vertices'),
        'vertex-stride':('vertices,0x20','vertices,0x18'),
        'skip-end-pass':('UG2EffectEndPass(ug2RenderTechnique870910->effect20);',''),
        'skip-count':('    CarRenderFinishQuad();',''),
        'saturating-counter':('++ug2RenderQuadCalls872F08;','if(ug2RenderQuadCalls872F08!=0xffffffff) ++ug2RenderQuadCalls872F08;'),
        'stop-on-hresult':('UG2EffectCommit(ug2RenderTechnique870910->effect20);','if(UG2EffectCommit(ug2RenderTechnique870910->effect20)<0) return;'),
        'floating-copy':('vertices[j][0]=positions[j*4];','volatile double value=quad->position00[j][0];volatile float narrowed=value;*reinterpret_cast<float *>(&vertices[j][0])=narrowed;'),
    }
    rejected=[]
    for name,(before,after)in variants.items():
        text=SOURCE.read_text();assert before in text
        body=text.replace(before,after,1)
        if name=='stale-gate':body=body.replace('if (ug2RenderQuadEnabled7FF05C)','if (enabled)')
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(body)
        command=[sys.executable,__file__,'--cases','1024','--composed-cases','0','--replace',str(path)]+(['--gcc']if gcc else[])
        result=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(result.stdout)
        assert result.returncode and 'quad submission mismatch'in result.stdout,(name,result.returncode,result.stdout[-2500:]);rejected.append(name)
    return rejected

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=8192);p.add_argument('--composed-cases',type=int,default=512);p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    assert args.cases>0 and args.composed_cases>=0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=metadata();own=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==own['executable_sha256']
    for row in manifest['functions'].values():
        a=int(row['address'],16)-0x400000;assert hashlib.sha256(image[a:a+row['size']]).hexdigest()==row['sha256']
    linked=build(args.gcc,args.replace or SOURCE);report=verify(image,manifest,linked,args.gcc,args.cases,False)
    if args.composed_cases:report['composed']=verify(image,manifest,linked,args.gcc,args.composed_cases,True)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.write_native_fixtures:report['native_fixtures']=export_native(image,manifest,args.write_native_fixtures)
    report.update(source_sha256=hashlib.sha256((args.replace or SOURCE).read_bytes()).hexdigest(),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',boundary='isolated engine helpers controlled; composed texture/world uploads recovered; effect/device services controlled; indeterminate vertex words excluded')
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
