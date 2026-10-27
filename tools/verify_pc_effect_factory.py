#!/usr/bin/env python3
"""Verify UG2 effect construction, resource factory and shared-pool lifetime.

The deserializer, effect runtime setup and source compiler are controlled
boundaries. The default suite also controls destruction; --real-destroy links
the recovered full destructor and COM dispatch into factory ownership chains.
Recovered resources, buffers, constructors, references, pool/node cleanup and
the complete factory execute in both modes.
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

ROOT,EXE=core.ROOT,core.EXE
LIB=ROOT/'src/Speed/Indep/Libs/D3DX'
SOURCES=[LIB/'EffectObject.cpp',LIB/'EffectPool.cpp',LIB/'EffectFactory.cpp']
BUILD=ROOT/'build/analysis/effect-factory'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-factory.json'
ARENA,SIZE,HEAP,STOP=core.ARENA,core.SIZE,core.HEAP,core.STOP
EFFECT,POOL,NODE,SENTINEL,COMPILER,ERROR_BUFFER,ERRORS,DEFINES,INCLUDES=[ARENA+x for x in (0x1000,0x1400,0x1500,0x1800,0x2000,0x2200,0x2400,0x2500,0x2600)]
VTABLE=ARENA+0x3000
SPANS=dict(parameter=(0x664ecf,43),value=(0x65eb78,23),effect_construct=(0x66509e,256),effect_delete=(0x66afc5,33),
    effect_addref=(0x65b8c6,18),effect_release=(0x66c361,29),factory=(0x640c02,453),pool_construct=(0x64285f,36),
    pool_destroy=(0x642504,49),pool_delete=(0x642535,33),pool_release=(0x642883,29),pool_query=(0x642556,70),
    pool_create=(0x640aac,67),unlink=(0x641837,43),node_destroy=(0x641878,42),node_delete=(0x641816,33))
MEMBERS=dict(parameter=('D3DXEffectParameter','Construct'),value=('D3DXEffectValuePrefix','Construct'),
    effect_construct=('D3DXEffectObject','Construct'),effect_delete=('D3DXEffectObject','Delete'),
    pool_construct=('D3DXEffectPool','Construct'),pool_destroy=('D3DXEffectPool','Destroy'),pool_delete=('D3DXEffectPool','Delete'),
    unlink=('D3DXEffectPoolLink','Unlink'),node_destroy=('D3DXEffectPoolNode','Destroy'),node_delete=('D3DXEffectPoolNode','Delete'))
FREE=dict(effect_addref='D3DXEffectAddRef',effect_release='D3DXEffectRelease',factory='D3DXEffectCreateFromResource',
    pool_release='D3DXEffectPoolRelease',pool_query='D3DXEffectPoolQuery',pool_create='D3DXEffectPoolCreate')
core.MEMBERS.update(MEMBERS);core.VOID.update(('parameter','value','pool_destroy','unlink','node_destroy'))
GLOBALS=dict(core.GLOBALS,ug2FxPoolIid7B7410=0x7b7410)
RUNTIME=dict(destroy=(0x66adec,1),deserialize=(0x66f69c,5),setup=(0x66afe6,1),
    compiler_create=(0x640aef,7),compiler_compile=(STOP+0x4000,4),compiler_release=(STOP+0x4010,1))
NAMES=dict(destroy='UG2FxDestroyEffect',deserialize='UG2FxDeserializeEffect',setup='UG2FxSetupEffect',
    compiler_create='UG2FxCompileResource',compiler_compile='UG2FxCompilerCompile',compiler_release='UG2FxCompilerRelease')
SHIMS={k:STOP+0x5000+i*16 for i,k in enumerate(RUNTIME)}


def build(replace,gcc,real_destroy=False):
    BUILD.mkdir(parents=True,exist_ok=True)
    sources=[replace.get(p.name,p)for p in SOURCES]+[core.SOURCE,core.BRIDGE,
        ROOT/'src/Platform/Native/NativeFxPoolDispatch.cpp',ROOT/'src/Platform/Native/NativeFxEffectDispatch.cpp']
    if real_destroy:
        sources += [replace.get("EffectDestruction.cpp",LIB/"EffectDestruction.cpp"),
            ROOT/"src/Platform/Native/NativeFxDestroyDispatch.cpp",ROOT/"src/Platform/Native/NativeFxComDispatch.cpp"]
    bindings=dict(GLOBALS,**core.APIS,UG2FxMalloc=0x575620,UG2FxFree=0x575630,abort=STOP+0x900)
    bindings.update({NAMES[k]:a for k,a in SHIMS.items()if not(real_destroy and k=="destroy")})
    if gcc:
        linked=link_native(sources,BUILD,ROOT,bindings,FREE['factory'],text_address=0x5000000,include_dirs=[LIB])
        prefixes={k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in MEMBERS.items()};prefixes.update(FREE)
        prefixes.update({k:'_ZN'+str(len(c))+c+str(len(m))+m for k,(c,m)in core.MEMBERS.items()if k not in MEMBERS})
        prefixes.update(core.FREE)
    else:
        objects=[]
        for i,p in enumerate(sources):
            obj=BUILD/('factory-'+str(i)+'.obj');objects.append(obj)
            subprocess.run([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe','/nologo','/c','/Ox','/Ob0','/EHsc',
                '/I'+str(LIB.relative_to(ROOT)),'/Fo'+str(obj.relative_to(ROOT)),str(p.relative_to(ROOT))],cwd=ROOT,check=True)
        bindings={'_'+k:v for k,v in bindings.items()}
        for name,n,_ in core.API_SPECS:bindings['_UG2Fx'+name+'@'+str(n*4)]=core.APIS['UG2Fx'+name]
        for k in ('compiler_create','compiler_compile','compiler_release'):bindings['_'+NAMES[k]+'@'+str(RUNTIME[k][1]*4)]=SHIMS[k]
        linked=link_objects(objects,bindings,start=0x5000000)
        prefixes={k:'?'+m+'@'+c+'@@'for k,(c,m)in core.MEMBERS.items()};prefixes.update({k:'_'+n+'@'for k,n in dict(core.FREE,**FREE).items()})
    linked['entries']={}
    for k,p in prefixes.items():
        hits=[a for n,a in linked['exports'].items()if n==p or n.startswith(p if k in core.MEMBERS or not gcc else p+'@')]
        assert len(hits)==1,(k,p,hits);linked['entries'][k]=hits[0]
    return linked


class Machine(core.Machine):
    def __init__(self,image,linked=None,gcc=False,real_destroy=False):
        super().__init__(image,linked,gcc);self.rebuilt=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for k,(original,n)in RUNTIME.items():
            if real_destroy and k=="destroy":continue
            a=SHIMS[k]if linked else original;self.u.hook_add(UC_HOOK_CODE,self.dependency,user_data=k,begin=a,end=a)
    def args_member(self,n):
        if self.rebuilt:
            a=self.args(n+1);return a[0],a[1:]
        return self.u.reg_read(xc.UC_X86_REG_ECX),self.args(n)
    def state(self):return super().state()+bytes(self.u.mem_read(EFFECT,0x1700))
    def own(self,a,n):self.live[a]=n
    def dependency_free(self,a):
        if not a:return
        assert a in self.live,('dependency free',hex(a))
        self.events.append(('dependency-free',a,bytes(self.u.mem_read(a,self.live.pop(a))),self.state()))
    def drop_buffer(self,a):
        if not a:return
        refs=(self.word(a+4)-1)&0xffffffff;self.put(a+4,refs)
        if not refs:self.dependency_free(self.word(a+12));self.dependency_free(a)
    def error_buffer(self,output):
        if output:
            self.own(ERROR_BUFFER,16);self.put(ERROR_BUFFER,0x7b711c,1,0,0);self.put(output,ERROR_BUFFER)
    def dependency(self,u,address,size,kind):
        c=self.case;pop=0;result=0
        if kind in ('destroy','deserialize','setup'):
            obj,a=self.args_member(4 if kind=='deserialize'else 0)
            if not self.rebuilt:pop=len(a)*4
            if kind=='deserialize':
                device,pool,buffer,position=a
                self.events.append((kind,obj,device,pool,self.word(position),bytes(u.mem_read(buffer,16)),
                    bytes(u.mem_read(self.word(buffer+12),self.word(buffer+8))),bytes(u.mem_read(obj,0x130))))
                assert self.word(position)==0
                self.put(position,c['position']);self.put(obj+0x14,c['mark']);self.put(obj+0x1c,c['position'])
                if c['retain']:
                    self.put(obj+0x28,buffer);self.put(buffer+4,(self.word(buffer+4)+1)&0xffffffff)
                    if pool:self.put(obj+0x38,pool);self.put(pool+4,(self.word(pool+4)+1)&0xffffffff)
                result=c['deserialize_result']
            elif kind=='setup':
                self.events.append((kind,obj,bytes(u.mem_read(obj,0x130))))
                self.put(obj+0x3c,c['mark']^0x11223344);result=c['setup_result']
            else:
                self.events.append((kind,obj,bytes(u.mem_read(obj,0x130))))
                self.drop_buffer(self.word(obj+0x28));self.put(obj+0x28,0)
                pool=self.word(obj+0x38)
                if pool:self.put(pool+4,(self.word(pool+4)-1)&0xffffffff);self.put(obj+0x38,0)
        elif kind=='compiler_create':
            module,name,defines,includes,flags,out,errors=self.args(7);pop=28
            self.events.append((kind,module,name,defines,includes,flags,bool(errors),self.word(errors)if errors else None))
            if c['compiler_created']:
                self.own(COMPILER,0x20);self.put(COMPILER,VTABLE,1);self.put(out,COMPILER)
            if c['first_error']:self.error_buffer(errors)
            result=c['compiler_result']
        elif kind=='compiler_compile':
            compiler,flags,out,errors=self.args(4);pop=16
            self.events.append((kind,compiler,flags,bool(errors),self.word(errors)if errors else None))
            if c['compiled_buffer']:
                self.own(core.BUFFER,16);self.own(core.BLOB, len(c['compiled_payload']))
                self.put(core.BUFFER,0x7b711c,1,len(c['compiled_payload']),core.BLOB)
                u.mem_write(core.BLOB,c['compiled_payload']);self.put(out,core.BUFFER)
            if c['second_error']:self.error_buffer(errors)
            result=c['compile_result']
        else:
            compiler,=self.args(1);pop=4
            self.events.append((kind,compiler,bytes(u.mem_read(compiler,0x20))))
            self.put(compiler+4,(self.word(compiler+4)-1)&0xffffffff)
            if not self.word(compiler+4):self.dependency_free(compiler)
        self.ret(result,pop)
    def prepare(self,c):
        super().prepare(c)
        self.put(core.BUFFER,0x7b711c,1,0,0)
        self.put(VTABLE+8,RUNTIME['compiler_release'][0]);self.put(VTABLE+0xf4,RUNTIME['compiler_compile'][0])
        self.put(POOL,0x7ab4e8,1,0,0,0,0,0,0);self.put(POOL+4,c['refs'])
        self.u.mem_write(EFFECT,c['effect']);self.put(EFFECT,0x7b6fe0);self.put(EFFECT+0x24,c['refs']);self.put(EFFECT+0x28,0);self.put(EFFECT+0x38,0)
        guid=bytes(self.u.mem_read(c['pool_guid'],16))if c['pool_guid']else c['bad_guid'];self.u.mem_write(core.IID,guid)
    def nodes(self,c,owned=True):
        self.put(SENTINEL+12,SENTINEL,SENTINEL)
        nodes=[NODE+j*0x40 for j in range(c['nodes']+1)]
        ring=[SENTINEL]+[node+12 for node in nodes]
        for i,node in enumerate(nodes):
            self.own(node,0x24)
            self.put(node+12+8,0,ring[i],ring[i+2]if i+2<len(ring)else SENTINEL)
            self.put(node+0x20,nodes[i+1]if i+1<len(nodes)else 0)
            if (c['nodes']+i)&1:
                data=ARENA+0x5000+i*0x40;self.own(data,17);self.put(node+12+8,data)
        self.put(SENTINEL+12,nodes[-1]+12,nodes[0]+12);self.put(POOL+0x1c,nodes[0])
        return nodes[0]
    def run(self,kind,c,chain=False):
        self.prepare(c)
        if kind=='parameter':result=self.call(kind,receiver=EFFECT+0x48)
        elif kind=='value':result=self.call(kind,receiver=EFFECT+0xe0)
        elif kind=='effect_construct':result=self.call(kind,receiver=EFFECT)
        elif kind in ('effect_addref','effect_release','effect_delete'):
            self.own(EFFECT,0x130);result=self.call(kind,[c['flags']],EFFECT)if kind=='effect_delete'else self.call(kind,[EFFECT])
        elif kind.startswith('pool'):
            if kind=='pool_construct':result=self.call(kind,receiver=POOL)
            elif kind=='pool_create':result=self.call(kind,[0 if c['null_pool_output']else core.OUT])
            elif kind=='pool_query':result=self.call(kind,[POOL,core.IID,(core.OUT,core.IID,POOL+4)[c['alias']]])
            else:
                self.own(POOL,0x20)
                if c['has_nodes']:self.nodes(c)
                if c['pool_data']:self.own(ARENA+0x4f00,31);self.put(POOL+16,ARENA+0x4f00)
                result=self.call(kind,[POOL])if kind=='pool_release'else self.call(kind,[c['flags']]if kind=='pool_delete'else [],POOL)
        elif kind in ('unlink','node_destroy','node_delete'):
            node=self.nodes(c)
            if kind=='unlink':self.put(node+0x20,0)
            result=self.call(kind,[c['flags']]if kind=='node_delete'else [],node+12 if kind=='unlink'else node)
        else:
            self.own(POOL,0x20);self.put(POOL+4,1)
            output=0 if c['null_effect_output']else core.OUT
            errors=0 if c['null_errors']else core.OUT if c['alias_outputs']else ERRORS
            result=self.call('factory',[c['device'],c['module'],0 if c['null_name']else core.NAME,DEFINES,INCLUDES,c['compile_flags'],
                0 if c['null_pool']else POOL,output,errors])
            if chain:
                if output and self.word(output) and result<0x80000000:self.call('effect_release',[self.word(output)])
                if errors and self.word(errors) and (not output or errors!=output):self.call('release',[self.word(errors)])
                self.call('pool_release',[POOL])
                assert not self.live,('factory chain leaked',self.live)
        return self.snapshot(result)


def cases(n):
    rng=random.Random(0x640c02)
    for i,base in enumerate(core.cases(n)):
        c=dict(base)
        length=(0,1,3,4,5,6,7,8,31,64,255,1024)[i%12]
        payload=bytearray(rng.randbytes(length))
        if length>=4:struct.pack_into('<I',payload,0,0xfeff0901 if (i//12)%3 else 0x12345678)
        c.update(payload=bytes(payload),effect=rng.randbytes(0x130),pool_guid=(0x7b7410,0x7d9904,0)[i%3],
            nodes=i%5,has_nodes=bool(i&1),pool_data=bool(i&2),null_pool_output=i%11==0,
            device=0 if i%13==0 else 0x31400000,null_name=i%17==0,null_effect_output=i%5==0,null_errors=i%7==0,
            null_pool=i%3==0,alias_outputs=i%19==0,compiler_created=(i//3)%3!=2 or not(i&1),compiler_result=(0,1,0x80004005)[(i//3)%3],
            compiled_buffer=True,compile_result=(0,1,0x8007000e)[(i//9)%3],first_error=bool(i&1),second_error=bool(i&2),
            compiled_payload=rng.randbytes(4+(i%17)),deserialize_result=(0,1,0x80004005)[(i//5)%3],setup_result=(0,2,0x80004005)[(i//7)%3],
            compile_flags=rng.getrandbits(32),position=rng.getrandbits(32),mark=rng.getrandbits(32),retain=bool(i&1))
        c['profile']='random'
        if i%4:
            step=i//4
            c.update(device=0x31400000,module=0,null_name=False,raw_found=True,fail_stage='none',callback=False,
                retain=bool(step&1),null_effect_output=False)
            if i%4 in (1,2):
                c['payload']=struct.pack('<I',0xfeff0901)+rng.randbytes(step%20)
                if i%4==1:
                    c.update(profile='compiled-allocation',fail_allocation=step%4,null_effect_output=step%6==0)
                else:
                    c.update(profile='compiled-runtime',fail_allocation=0,
                        deserialize_result=(0,1,0x80004005)[step%3],setup_result=(0,2,0x80004005)[(step//3)%3])
            else:
                c.update(profile='source-compiler',payload=struct.pack('<I',0x12345678)+rng.randbytes(step%13),fail_allocation=0,
                    null_effect_output=step%5==0,compiler_result=(0,1,0x80004005)[step%3],compiler_created=step%3!=2 or not(step&1),
                    compile_result=(0,1,0x8007000e)[(step//3)%3],first_error=bool(step&1),second_error=bool((step//2)&1))
        yield c


def verify(image,linked,gcc,n,kinds,retail,real_destroy=False):
    a,b=Machine(image,real_destroy=real_destroy),Machine(image,linked,gcc,real_destroy);counts=Counter();boundaries=Counter();returns=Counter();profiles=Counter()
    fixtures=[(k,c)for c in cases(n)for k in kinds]
    for name,payload in retail:
        c=next(cases(1));c.update(payload=payload,device=0x31400000,module=0,null_name=False,null_effect_output=False,null_errors=False,
            alias_outputs=False,raw_found=True,fail_stage='none',fail_allocation=0,callback=False,deserialize_result=0,setup_result=0,retain=True,profile='retail')
        fixtures.append(('factory_chain',c))
    for i,(kind,c)in enumerate(fixtures):
        if kind=='factory_chain':c=dict(c,alias_outputs=False)
        try:x=a.run(kind,c,kind=='factory_chain');y=b.run(kind,c,kind=='factory_chain')
        except (AssertionError,UcError)as e:raise AssertionError(('effect factory mismatch',kind,i,str(e)))from e
        if x!=y:
            offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
            event=next(((j,v[:3],w[:3])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
            raise AssertionError(('effect factory mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event))
        counts[kind]+=1;boundaries.update(e[0]for e in x[3]);returns[hex(x[2])if x[2]is not None else'void']+=1
        if kind in ('factory','factory_chain'):profiles[c['profile']]+=1
    return dict(comparisons=dict(counts),boundary_calls=dict(boundaries),returns=dict(returns),factory_profiles=dict(profiles))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512)
    p.add_argument('--kind',choices=['all',*SPANS,'factory_chain'],default='all');p.add_argument('--replace',type=Path);p.add_argument('--source-name');p.add_argument('--mutation-checks',action='store_true')
    p.add_argument('--real-destroy',action='store_true',help='Execute the full destructor; use factory or factory_chain fixtures')
    args=p.parse_args()
    if args.real_destroy and args.kind=='all':args.kind='factory_chain'
    if args.real_destroy and args.kind not in ('factory','factory_chain'):p.error('--real-destroy requires factory or factory_chain')
    raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in list(manifest['functions'].values())+manifest['data']:
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    retail=[]
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id!=10:continue
        for entry in t.directory.entries:
            d=entry.directory.entries[0].data.struct;retail.append((str(entry.name),pe.get_data(d.OffsetToData,d.Size)))
    replace={args.source_name:args.replace}if args.replace else{}
    report=verify(image,build(replace,args.gcc,args.real_destroy),args.gcc,args.cases,[*SPANS,'factory_chain']if args.kind=='all'else[args.kind],
        retail if args.kind=='all' or args.real_destroy else[],args.real_destroy)
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['compiler creation/compilation/release, deserialization, runtime setup and full effect destruction are controlled boundaries',
        'resources/buffers and pool/node lifetimes execute recovered code; allocation and Win32 APIs remain controlled',
        'retail fixtures validate factory byte copies and ownership under controlled deserialization, not parsing or shaders',
        'finite bounded records and valid dependency objects; malformed pointers, arbitrary class identities and exceptions not established'])
    if args.real_destroy:
        report['full_destruction']='recovered source executes; COM class-token adapter dispatches recovered buffer/pool/effect releases'
        report['limitations'][0]='compiler creation/compilation/release, deserialization and runtime setup are controlled boundaries'
    if args.mutation_checks:
        mutations={
            'erase-parameter-unknown':('parameter','EffectObject.cpp','value08=0xffffffff;','value08=0xffffffff; value0C=0;'),
            'wrong-parameter-type':('parameter','EffectObject.cpp','value10=3;','value10=0;'),
            'wide-parameter-word-clear':('parameter','EffectObject.cpp','value46=0;','value44=0; value46=0;'),
            'erase-value-unknown':('value','EffectObject.cpp','value00=3;','value00=3; value04=0;'),
            'zero-whole-effect':('effect_construct','EffectObject.cpp','class00=0x7b6fe0;','for (unsigned int i=0;i<sizeof(*this);++i) reinterpret_cast<unsigned char *>(this)[i]=0; class00=0x7b6fe0;'),
            'wrong-effect-flag-mask':('effect_construct','EffectObject.cpp','&=0xe478;','&=0xe470;'),
            'erase-effect-high-flags':('effect_construct','EffectObject.cpp','*reinterpret_cast<unsigned short *>(&value118)&=0xe478;','value118&=0xe478;'),
            'wrong-effect-ref-offset':('effect_addref','EffectObject.cpp','return ++effect->refs24;','return ++effect->interface04;'),
            'wrong-effect-release-count':('effect_release','EffectObject.cpp','--effect->refs24;','++effect->refs24;'),
            'free-effect-on-any-flag':('effect_delete','EffectObject.cpp','if (flags&1)','if (flags)'),
            'wrong-pool-class':('pool_construct','EffectPool.cpp','class00=0x7ab4e8;','class00=0x7ab4e0;'),
            'preclear-pool-output':('pool_create','EffectPool.cpp','if (!output) return','if (output) *output=0; if (!output) return'),
            'clear-failed-pool-query':('pool_query','EffectPool.cpp','int i=0;','*output=0; int i=0;'),
            'ignore-pool-iid':('pool_query','EffectPool.cpp','iid[i]==ug2FxPoolIid7B7410[i]','false'),
            'omit-pool-query-ref':('pool_query','EffectPool.cpp','UG2FxPoolAddRefVirtual(pool);','(void)0;'),
            'wrong-pool-release-test':('pool_release','EffectPool.cpp','if (!count)','if (count==1)'),
            'reverse-pool-free-order':('pool_destroy','EffectPool.cpp','if (data10) { UG2FxFree(data10); data10=0; }\n    if (node1C) { node1C->Delete(1); node1C=0; }','if (node1C) { node1C->Delete(1); node1C=0; }\n    if (data10) { UG2FxFree(data10); data10=0; }'),
            'free-pool-on-any-flag':('pool_delete','EffectPool.cpp','if (flags&1)','if (flags)'),
            'retain-node-data':('unlink','EffectPool.cpp','UG2FxFree(data08); data08=0;','UG2FxFree(data08);'),
            'wrong-next-backlink':('unlink','EffectPool.cpp','next10->previous0C=previous0C;','next10->next10=previous0C;'),
            'omit-child-detach':('node_destroy','EffectPool.cpp','node->linked20=0;','(void)0;'),
            'wrong-embedded-link':('node_destroy','EffectPool.cpp','link0C.Unlink();','reinterpret_cast<D3DXEffectPoolLink *>(this)->Unlink();'),
            'free-node-on-any-flag':('node_delete','EffectPool.cpp','if (flags&1)','if (flags)'),
            'omit-output-preclear':('factory','EffectFactory.cpp','if (output) *output=0;','(void)0;'),
            'omit-error-preclear':('factory','EffectFactory.cpp','if (errors) *errors=0;','(void)0;'),
            'wrong-invalid-hresult':('factory','EffectFactory.cpp','0x8876086c','0x80004005'),
            'reject-four-byte-resource':('factory','EffectFactory.cpp','resource.size08<4','resource.size08<=4'),
            'wrong-effect-signature':('factory','EffectFactory.cpp','==0xfeff0901','==0xfeff0902'),
            'omit-source-clear':('factory','EffectFactory.cpp','resource.Clear();','(void)0;'),
            'wrong-compiler-flags':('factory','EffectFactory.cpp','flags&0xffffffe7','flags&0xfffffff7'),
            'omit-first-diagnostic-release':('factory','EffectFactory.cpp','if (errors && *errors) { UG2FxBufferReleaseVirtual(*errors); *errors=0; }','(void)0;'),
            'truncate-byte-copy-tail':('factory','EffectFactory.cpp','for (i=length&3;i;--i) *destination++=*source++;','(void)0;'),
            'wrong-deserializer-position':('factory','EffectFactory.cpp','unsigned int position=0;','unsigned int position=1;'),
            'publish-before-setup':('factory','EffectFactory.cpp','result=UG2FxSetupEffect(effect);','*output=effect; result=UG2FxSetupEffect(effect);'),
            'omit-published-reference':('factory','EffectFactory.cpp','UG2FxEffectAddRefVirtual(effect);','(void)0;'),
            'leak-compiler':('factory','EffectFactory.cpp','if (compiler) UG2FxCompilerRelease(compiler);','(void)0;'),
            'reverse-temporary-release':('factory','EffectFactory.cpp','if (buffer) { UG2FxBufferReleaseVirtual(buffer); buffer=0; }\n    if (effect) UG2FxEffectReleaseVirtual(effect);','if (effect) UG2FxEffectReleaseVirtual(effect);\n    if (buffer) { UG2FxBufferReleaseVirtual(buffer); buffer=0; }'),
            'release-caller-diagnostics':('factory','EffectFactory.cpp','release_compiler:\n','release_compiler:\n    if (errors && *errors) { UG2FxBufferReleaseVirtual(*errors); *errors=0; }\n'),
        }
        kinds=set(SPANS)|{'factory_chain'}if args.kind=='all'else{args.kind};rejected=[]
        for name,(kind,filename,before,after)in mutations.items():
            if kind not in kinds:continue
            body=(LIB/filename).read_text();assert before in body,name
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body.replace(before,after))
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--source-name',filename,'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[]),
                cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect factory mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+('-real-destroy'if args.real_destroy else'')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
