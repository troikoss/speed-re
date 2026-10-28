#!/usr/bin/env python3
"""Verify UG2 shared-pool layouts, binding, resource maps and pointer rebasing.

Allocation and non-C-locale lowercase services are controlled. Native CPU chains
exercise recovered pool ownership; no shader or PE function is executed natively.
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
import verify_pc_effect_values as v

md=v.md;ROOT,EXE,LIB=v.ROOT,v.EXE,v.LIB
SOURCE=LIB/'EffectSharedPool.cpp';BUILD=ROOT/'build/analysis/effect-shared-pool';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-shared-pool.json'
ARENA,HEAP,STOP,EFFECT=v.ARENA,v.HEAP,v.STOP,v.EFFECT
POOL,CURSORS,OUTPUT,SEMANTIC,OTHERNAME=ARENA+0x1900,ARENA+0xa000,ARENA+0xc000,0x180,0x200
SPANS=dict(ascii=(0x767730,78),case_compare=(0x75dd6f,105),compare=(0x6418a2,411),match=(0x641a3d,35),size_shared=(0x641a60,438),
    copy_shared=(0x641c16,690),create_shared=(0x641ec8,69),indices_walk=(0x641f0d,240),indices=(0x641ffd,26),numeric_walk=(0x642017,249),numeric=(0x642127,26),
    resources_walk=(0x642141,838),resources=(0x642487,43),node_construct=(0x641862,22),rebase=(0x6424b2,49),bind=(0x64259c,707))
FREE=dict(ascii='D3DXEffectAsciiCompare',case_compare='D3DXEffectCaseCompare',compare='D3DXEffectCompareSharedType',match='D3DXEffectMatchSharedType',size_shared='D3DXEffectSharedSize',
    copy_shared='D3DXEffectCopySharedType',create_shared='D3DXEffectCreateSharedType',indices_walk='D3DXEffectWalkSharedIndices',indices='D3DXEffectSharedIndices',
    numeric_walk='D3DXEffectCopySharedNumeric',numeric='D3DXEffectRefreshSharedNumeric',resources_walk='D3DXEffectCopySharedResources',resources='D3DXEffectRefreshSharedResources')
md.SOURCES+=[SOURCE];md.BUILD=BUILD;md.FREE.update(FREE);md.CDECL.update(('ascii','case_compare'))
md.STDCALL.update(compare=20,match=20,size_shared=20,copy_shared=32,create_shared=32,indices_walk=12,indices=12,numeric_walk=12,numeric=12,resources_walk=16,resources=16)
md.MEMBERS.update(node_construct=('D3DXEffectPoolNode','Construct'),rebase=('D3DXEffectPool','RefreshPointers'),bind=('D3DXEffectPool','Bind'),pool_construct=('D3DXEffectPool','Construct'),pool_delete=('D3DXEffectPool','Delete'))
md.FREE['free_fn']='UG2FxFree';md.CDECL.add('free_fn');md.VOID.add('free_fn')
md.FREE['pool_release']='D3DXEffectPoolRelease';md.STDCALL['pool_release']=4;md.MEMBERS['link_delete']=('D3DXEffectPoolLink','Delete')


def cases(n):
    rng=random.Random(0x64259c)
    for i,c in enumerate(v.cases(n)):
        c.update(prefill=i%3!=0,capacity=(0,0x100,0x1000)[(i//3)%3],semantic=(b'',b'WORLD',b'world',b'MiXeD',b'\x80')[i%5],
            other_semantic=(b'',b'world',b'World',b'MIXED',b'\x80')[i%5],name=(b'p1',b'p1',b'p1',b'p1',b'P1')[i%5],
            difference=(i//7)%7,force_growth=i%11==0,device_mismatch=i%13==0,alias_links=i%17==0,null_link=i%19==0,
            null_names=i%4,non_c_locale=i%7==0,lower_bias=(i//9)%3,stale_locale=i%5==0,update_b=i%2==0,latin=bytes([i%256,0]),
            comparison=(b'AbC',b'abc',b'abd',b'AB',b'\x80a',b'\xffA',b'')[i%7],compare_other=(b'aBc',b'ABC',b'ABc',b'abc',b'\x80A',b'\x80a',b'')[i%7])
        # Include every case-folding byte, and names/semantics of odd lengths.
        if i<256 or i%9==0:c['comparison']=c['latin'];c['compare_other']=bytes([rng.randrange(256),0])
        yield c


class Machine(v.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()},pool_construct=0x64285f,pool_delete=0x642535,free_fn=0x575630,pool_release=0x642883,link_delete=0x6424e3)
        address=linked['exports']['UG2FxCrtLowerLocale'if gcc else'_UG2FxCrtLowerLocale']if linked else 0x75de10
        self.u.hook_add(UC_HOOK_CODE,self.lower,begin=address,end=address)
        self.entries['buffer_release']=linked['exports']['D3DXEffectBufferRelease'if gcc else'_D3DXEffectBufferRelease@4']if linked else 0x670df0
    def crt(self,u,address,size,kind):
        if getattr(self,'crt_native',False):
            assert kind=='thread';self.events.append((kind,(),self.state()));self.ret(v.hc.THREAD)
        else:super().crt(u,address,size,kind)
    def call_buffer_release(self,buffer):
        self.entries['buffer_release']=self.entries.get('buffer_release',0x670df0);return self.call('buffer_release',[buffer])
    def lower(self,u,address,size,user):
        locale,character=self.args(2);c=self.case
        result=character+32 if 65<=character<=90 else character
        if character>=128:result=(character+c['lower_bias'])&0xff
        self.events.append(('locale-lower',(locale,character),self.state()));self.ret(result)
    def setup_shared(self,c,kind):
        self.test_kind='copy';self.setup_values(c);self.active_kind='pool'
        self.call('pool_construct',receiver=POOL);self.put(EFFECT+0x38,POOL);self.put(self.other+0x38,POOL)
        self.put(self.other+0x1c,0x4000,self.shared_blob)
        self.put(self.parameter+0x40,0)
        self.put(v.hc.LOCALE_A+0x14,1 if c['non_c_locale']else 0);self.put(v.hc.LOCALE_B+0x14,1 if c['non_c_locale']else 0)
        self.other_parameter=self.alloc(0x50);self.call('parameter_ctor',receiver=self.other_parameter)
        self.put(self.other_parameter,self.shared_blob+0x2000,self.shared_blob+0x500,0xffffffff,256,3,1,0,0)
        self.put(self.other_parameter+0x28,1,0x500,0x2000);self.put(self.other_parameter+0x38,0,0,0)
        other_refs=self.word(self.other+0x18);other_buffer=self.buffer(16);other_code=self.buffer(8)
        self.put(other_refs+4,other_buffer,other_code,0x12345678);self.put(other_refs+0x14,0x56789abc)
        self.put(self.references+8,self.new,0x87654321);self.put(self.references+0x14,0xabcdef02)
        def names(base,name,semantic):
            self.put(base+0x50,len(name)+1);self.u.mem_write(base+0x54,name+b'\0')
            self.put(base+SEMANTIC,len(semantic)+1 if semantic else 0);self.u.mem_write(base+SEMANTIC+4,semantic+b'\0')
            def annotate(at):
                shape=self.word(at+4);self.put(at+12,SEMANTIC)
                if shape==5:
                    count=self.word(at+20);end=at+24
                    for _ in range(count):end=annotate(end)
                    return end
                return at+(28 if shape<4 else 20)
            annotate(base+0x500)
        names(self.blob,b'p1',c['semantic']);names(self.shared_blob,c['name'],c['other_semantic'])
        # The pool walkers use resource indices, never the value-setter inputs.
        for base,shift in((self.blob,0),(self.shared_blob,3)):
            # Compact refresh can encounter padded numeric words as IDs in a
            # mixed struct. Keep the first pooled member's words in map range.
            data=[0]*512 if kind=='bind' and c['family']==2 and base==self.blob else [(i+shift)%8 for i in range(512)]
            self.put(base+0x2000,*data)
        self.put(self.other+0xc,1 if c['device_mismatch']else 0)
        if c['capacity']:
            data=self.alloc(c['capacity']);self.u.mem_write(data,bytes(c['capacity']));self.put(POOL+0x10,data);self.put(POOL+0x18,c['capacity'])
        self.case=dict(c,fail_allocation=0)
        # Struct children may be unsupported; only retain a successful prefill.
        if c['prefill']or kind=='rebase':
            result=self.call('bind',[EFFECT,self.parameter],POOL)
            if result==0 and c['force_growth']:
                self.put(POOL+0x14,self.word(POOL+0x18)-4)
        self.case=c;self.requests=0
        if c['difference']:
            offset=(0,0,4,16,20,24,12)[c['difference']]
            if c['difference']==6:self.put(self.shared_blob+0x500+offset,0)
            else:self.put(self.shared_blob+0x500+offset,(self.word(self.shared_blob+0x500+offset)+1)&0xffffffff)
        if kind in('compare','match'):
            if c['null_names']in(1,3):self.put(self.blob+0x50,0)
            if c['null_names']in(2,3):self.put(self.shared_blob+0x50,0)
        self.u.mem_write(v.TEXT,c['comparison']+b'\0');self.u.mem_write(v.TEXT+0x100,c['compare_other']+b'\0')
        self.put(CURSORS,self.blob+0x500,self.shared_blob+0x500,self.blob+0x2000,self.shared_blob+0x2000,OUTPUT+0x1000,OUTPUT+0x2000,0x80,0xa5a5a5a5)
        self.destination=self.link if c['alias_links']else self.word(self.link+0x10)
        if c['null_link']:self.put(self.destination,0)
        # Null links are supported only by the wrapper; the walker dereferences them.
        if kind=='resources_walk'and c['null_link']:self.put(self.destination,self.other)
        self.events=[]
    def run(self,kind,c):
        self.setup_shared(c,kind);a=self.blob+0x500;b=self.shared_blob+0x500
        if kind in('ascii','case_compare'):result=self.call(kind,[v.TEXT,v.TEXT+0x100])
        elif kind=='compare':result=self.call(kind,[self.blob,CURSORS,self.shared_blob,CURSORS+4,CURSORS+28])
        elif kind=='match':result=self.call(kind,[self.blob,a,self.shared_blob,b,CURSORS+28])
        elif kind=='size_shared':result=self.call(kind,[self.blob,CURSORS,CURSORS+16,CURSORS+20,CURSORS+24])
        elif kind=='copy_shared':result=self.call(kind,[self.blob,CURSORS,CURSORS+8,OUTPUT,CURSORS+24,CURSORS+16,CURSORS+20,CURSORS+28])
        elif kind=='create_shared':result=self.call(kind,[self.blob,a,self.blob+0x2000,OUTPUT,0x80,0x400,0x400,CURSORS+28])
        elif kind in('indices_walk','numeric_walk','resources_walk'):
            if kind=='indices_walk':args=[CURSORS,CURSORS+8,CURSORS+20]
            elif kind=='numeric_walk':args=[CURSORS,CURSORS+12,CURSORS+8]
            else:args=[CURSORS,CURSORS+8,self.link,self.destination]
            result=self.call(kind,args)
        elif kind in('indices','numeric','resources'):
            if kind=='indices':args=[a,self.blob+0x2000,OUTPUT+0x2000]
            elif kind=='numeric':args=[a,self.shared_blob+0x2000,self.blob+0x2000]
            else:args=[a,self.blob+0x2000,self.destination,self.link]
            result=self.call(kind,args)
        elif kind=='node_construct':result=self.call(kind,receiver=OUTPUT)
        elif kind=='rebase':
            old=self.word(POOL+0x10)
            if old:
                new=self.alloc(self.word(POOL+0x18));self.u.mem_write(new,bytes(self.u.mem_read(old,self.word(POOL+0x18))));self.call('free_fn',[old]);self.put(POOL+0x10,new)
            self.events=[];result=self.call(kind,receiver=POOL)
        else:result=self.call(kind,[self.other,self.other_parameter],POOL)
        return self.snapshot(result)


def native_cases():
    for mode in range(21):
        kind,shape,count,rows,columns=3,0,0,3,1
        if mode==1:kind,shape,count,rows=2,1,3,2
        elif mode in(2,3):kind,shape,count,rows,columns=(1,2,2,2,3)if mode==2 else(3,3,3,4,4)
        elif mode in(4,5,6,7,9,10,11,16,17,18,20):
            kind=15 if mode in(6,16,18)else 16 if mode==7 else 9 if mode==9 else 5 if mode in(5,10,17,18,20)else 4
            shape,count=4,2
        elif mode==8:kind,shape,count=0,5,2
        elif mode==12:count,rows=3,2
        if shape==5:desc=[0,5,0x50,0x180,count,2,3,0,0x80,0,2,2,1,4,4,0x80,0,2]
        else:desc=[kind,shape,0x50,0x180,count]+([rows,columns]if shape<4 else[])
        yield [mode,kind,shape,count,rows,columns],desc


def native_graph(m,config,description):
    mode,kind,shape,count,rows,columns=config;c=next(cases(1));c.update(arena=bytes(md.core.SIZE),heap=b'\xa5'*md.core.HSIZE,fail_allocation=0)
    m.active_kind='native';m.prepare(c);m.getters=Counter();m.put(0x819d24,0x819cd0)
    # The native runtime supplies this same original immutable C-locale context.
    m.put(v.hc.THREAD+0x64,0x819cd0)
    m.crt_native=True
    pool=m.alloc(0x20);m.call('pool_construct',receiver=pool);graphs=[];words=[]
    for g in range(3 if mode==17 else 2):
        effect=EFFECT+g*0x200;m.u.mem_write(effect,b'\xa5'*0x130);m.call('effect_ctor',receiver=effect)
        m.put(effect+0x98,1,0,1,0,1,1);m.put(effect+0xfc,effect+0xe0);m.put(effect+0x118,0x100 if mode==19 else 0)
        blob=m.alloc(0x4000);m.u.mem_write(blob,bytes(0x4000));m.put(effect+0x1c,0x4000,blob)
        name=b'other'if g and mode==13 else b'P1'if g and mode==15 else b'p1'
        semantic=b'OTHER'if g and mode==14 else b'world'if g else b'WORLD'
        m.put(blob+0x50,len(name)+1);m.u.mem_write(blob+0x54,name+b'\0');m.put(blob+0x80,2);m.u.mem_write(blob+0x84,b'm\0')
        m.put(blob+0x180,len(semantic)+1);m.u.mem_write(blob+0x184,semantic+b'\0');m.u.mem_write(blob+0x500,v.hc.cp.pack(description))
        m.put(blob+0x2000,*[(i+g)%4 for i in range(512)])
        parameter=m.array(1,0x50,'parameter_ctor');table=m.alloc(4);m.put(table,parameter);m.put(effect+0x9c,parameter);m.put(effect+0xb0,table)
        m.put(parameter,blob+0x2000,blob+0x500,0xffffffff,8 if mode==20 else 64,3,0,0,0);m.put(parameter+0x28,1,0x500,0x2000);m.put(parameter+0x38,0,0,0)
        refs=m.array(4,0x18,'reference_construct');m.put(effect+0x14,4,refs)
        for i in range(4):
            buf=m.buffer(16);data=m.word(buf+12);m.u.mem_write(data,bytes([65+g*4+i])*15+b'\0');m.put(refs+i*0x18,4,buf)
            code=m.buffer(8);data=m.word(code+12);m.u.mem_write(data,bytes([97+g*4+i])*7+b'\0');m.put(refs+i*0x18+8,code,0x100+g*4+i);m.put(refs+i*0x18+0x14,0xa5a5a5a0+(g&1)*2)
        grid=m.alloc(4);m.put(grid,0);dirty=m.alloc(1);m.u.mem_write(dirty,b'\0');m.put(effect+0xb4,grid,grid,dirty)
        m.put(effect+0x38,pool);m.put(pool+4,m.word(pool+4)+1)
        if mode in(10,11):m.put(effect+0xc,m.buffer(4))
        if g and mode==13:m.put(pool+0x14,m.word(pool+0x18)-4)
        result=m.call('bind',[effect,parameter],pool);words.append(result);graphs.append((effect,parameter,refs,blob,dirty))
        if g==0 and mode==12:
            m.call('link_delete',[1],m.word(parameter+0x40));m.put(parameter+0x40,0)
        if g==1 and mode==17:
            slot=refs+m.word(m.word(m.word(parameter+0x40)+8))*0x18
            m.call_buffer_release(m.word(slot+4))
            replacement=m.buffer(16);m.u.mem_write(m.word(replacement+12),b'Z'*15+b'\0');m.put(slot+4,replacement)
    if mode in(18,20):
        incoming=m.buffer(16);m.u.mem_write(m.word(incoming+12),b'Q'*15+b'\0');effect,parameter,_,_,_=graphs[0]
        if mode==18:result=m.call('shader',[incoming,0,m.word(parameter+0x40)],effect)
        else:m.put(v.INPUT,incoming,incoming);result=m.call('set_value',[effect,(~parameter)&0xffffffff,v.INPUT,8])
        words.append(result);m.call_buffer_release(incoming)
    if mode==19:
        m.put(pool+8,1,graphs[1][0]);words.append(m.call('notify',[graphs[0][1]],graphs[0][0]))
    data=m.word(pool+0x10);used=m.word(pool+0x14);words.extend([used,m.word(pool+0x18),m.digest(data,used)])
    nodes=[];node=m.word(pool+0x1c)
    while node:nodes.append(node);node=m.word(node+0x20)
    words.append(len(nodes))
    for node in nodes:
        words.extend(m.word(node+o)for o in(0,4,8));head=node+0xc;link=m.word(head+0x10);members=[]
        while link!=head:
            effect=m.word(link);members.append(next(i for i,(e,*_)in enumerate(graphs)if e==effect));mapping=m.word(link+8);words.append(int(bool(mapping)))
            if mapping:words.extend(m.word(mapping+i*4)for i in range(m.word(node+8)))
            link=m.word(link+0x10)
        words.append(len(members));words.extend(members)
    for effect,parameter,refs,blob,dirty in graphs:
        words.extend([m.word(effect+0x118),m.u.mem_read(dirty,1)[0],int(bool(m.word(parameter+0x40))),m.word(parameter+0x2c),m.word(parameter+0x30),0x80002000 if m.word(parameter)==blob+0x2000 else m.word(parameter)-data])
        # Growth intentionally leaves type pointers pointing at the former block.
        words.append(int(m.word(parameter+4)==data+m.word(parameter+0x2c)))
        for i in range(4):
            ref=refs+i*0x18;words.extend([m.word(ref),m.word(ref+0xc),m.word(ref+0x14)])
            for off in(4,8):
                buf=m.word(ref+off);words.extend([int(bool(buf)),m.word(buf+4)if buf else 0,m.word(buf+8)if buf else 0,m.digest(m.word(buf+12),m.word(buf+8))if buf else 0])
    for effect,*_ in reversed(graphs):m.call('destroy',receiver=effect)
    words.append(m.word(pool+4));m.call('pool_release',[pool]);assert not m.live,('native pool cleanup',mode,m.live)
    m.crt_native=False;return [x&0xffffffff for x in words]


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();returns=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect shared pool mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect shared pool mismatch',kind,i,diffs,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;returns[str((kind,x[2]))]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),returns=dict(returns),boundary_calls=dict(events))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=512);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    if args.write_native_fixtures:
        content=b'UG2FXPF1'+v.hc.cp.pack([21])
        for config,desc in native_cases():content+=v.hc.cp.pack(config+[len(desc)]+desc)
        args.write_native_fixtures.write_bytes(content)
    report=verify(image,linked,args.gcc,args.cases,list(SPANS)if args.kind=='all'else[args.kind]);report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    a,b=Machine(image),Machine(image,linked,args.gcc);native=[]
    for config,desc in native_cases():
        try:x=native_graph(a,config,desc);y=native_graph(b,config,desc)
        except(AssertionError,UcError)as e:raise AssertionError(('effect shared pool mismatch','native-chain',config[0],str(e)))from e
        assert x==y and a.snapshot(tuple(x))==b.snapshot(tuple(y)),('effect shared pool mismatch','native-chain',config[0],x,y)
        native.append(x)
    report['native_chains']=len(native)
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXPD1';at=8
        for mode,expected in enumerate(native):
            row,count=struct.unpack_from('<2I',raw,at);at+=8;values=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert row==mode and values==expected,('native effect shared pool mismatch',mode,expected,values)
        assert at==len(raw);report['native_replay']=len(native)
    if args.mutation_checks:
        mutations={
            'ascii-omit-uppercase-fold':('ascii','offset<0x1a ? 0x20 : 0','false ? 0x20 : 0'),
            'ascii-signed-byte-order':('ascii','return a<b ? -1 : 1;','return static_cast<signed char>(a)<static_cast<signed char>(b) ? -1 : 1;'),
            'case-omit-locale-refresh':('case_compare','if (reinterpret_cast<unsigned int>(locale)!=ug2FxCrtLocale819D24)','if (false)'),
            'case-force-ascii':('case_compare','if (!*reinterpret_cast<const unsigned int *>(locale+0x14))','if (true)'),
            'compare-ignore-kind':('compare',' || description[0]!=other[0]',''),
            'compare-ignore-array-count':('compare',' || description[4]!=other[4]',''),
            'compare-name-case-fold':('compare','if (*a!=*b) return','if (D3DXEffectCaseCompare(reinterpret_cast<const char *>(a),reinterpret_cast<const char *>(b))) return'),
            'compare-omit-semantic-compare':('compare','!sb || D3DXEffectCaseCompare(sa,sb)','!sb'),
            'compare-ignore-column-count':('compare',' || description[6]!=(*otherType)[6]',''),
            'compare-mark-strings-device-bound':('compare','if ((kind>4 && kind<10)','if ((kind>=4 && kind<10)'),
            'match-omit-resource-flag-clear':('match','*resources=0;','(void)0;'),
            'size-wrong-name-prefix':('size_shared','extra)*4+8','extra)*4+4'),
            'size-compact-matrix':('size_shared','count*4 : count*16','count*4 : count*4'),
            'size-repeat-struct-descriptors':('size_shared','*types+=childType;','*types+=childType*count;'),
            'copy-compact-numeric':('copy_shared','shape<2 ? count*16 : count*64','description[5]*description[6]*count*4'),
            'copy-omit-resource-counter':('copy_shared','++*resources;','(void)0;'),
            'copy-omit-struct-name-reset':('copy_shared','*names=firstNames;','(void)0;'),
            'create-wrong-data-start':('create_shared','outputBase+names+namesSize+typeSize','outputBase+names+namesSize'),
            'indices-use-padded-skip':('indices_walk','description[6]*description[5]*count','(shape<2 ? 4 : 16)*count'),
            'indices-omit-output-step':('indices_walk','++*output;','(void)0;'),
            'indices-wrapper-skip-first-input':('indices','&type,&input,&output','&type,&(++input),&output'),
            'numeric-wrong-copy-width':('numeric_walk','description[6]*description[5]*count*4','description[6]*description[5]*count*8'),
            'numeric-advance-unsupported-resources':('numeric_walk','else if (!((kind>=10 && kind<=14) || kind==17 || kind==18))','else if (((kind>=10 && kind<=14) || kind==17 || kind==18)) { *output+=count;*input+=count; } else if (true)'),
            'numeric-wrapper-swap-data':('numeric','&type,&output,&input','&type,&input,&output'),
            'resources-omit-code-retain':('resources_walk','if ((kind==15 || kind==16) && from->value08) UG2FxComAddRef(from->value08);','(void)0;'),
            'resources-overwrite-all-flags':('resources_walk','(to->value14&0xfffffffd)|(from->value14&2)','from->value14'),
            'resources-omit-value0c-copy':('resources_walk','to->value0C=from->value0C;','(void)0;'),
            'resources-ignore-self-assignment-gate':('resources','destination==source || !destination->value00','!destination->value00'),
            'node-initialize-unknown-prefix':('node_construct','link0C.data08=0;','value04=0;link0C.data08=0;'),
            'rebase-also-write-type-pointer':('rebase','parameter->value00=reinterpret_cast<unsigned int>(data10)+parameter->value30;','parameter->value00=reinterpret_cast<unsigned int>(data10)+parameter->value30;parameter->value04=reinterpret_cast<unsigned int>(data10)+parameter->value2C;'),
            'bind-wrong-minimum-growth':('bind','doubled<0x10000 ? 0x10000','doubled<0x8000 ? 0x8000'),
            'bind-wrong-capacity-doubling':('bind','doubled=value18*2','doubled=value18*3'),
            'bind-omit-growth-rebase':('bind','data10=grown;RefreshPointers();','data10=grown;'),
            'bind-ignore-device-mismatch':('bind','if (resources && node->link0C.next10->value00','if (false && node->link0C.next10->value00'),
            'bind-wrong-map-allocation':('bind','UG2FxMalloc(node->value08*4)','UG2FxMalloc(node->value08*8)'),
            'bind-omit-used-publication':('bind','node1C=node;value14=required;','node1C=node;'),
            'bind-copy-newest-member':('bind','node->link0C.previous0C);','node->link0C.next10->next10);'),
            'bind-omit-failure-link-cleanup':('bind','if (link) link->Delete(1);','(void)0;'),
        }
        functions=dict(FREE,node_construct='D3DXEffectPoolNode::Construct',rebase='D3DXEffectPool::RefreshPointers',bind='D3DXEffectPool::Bind')
        source=SOURCE.read_text();starts={k:source.index(name+'(')for k,name in functions.items()};rejected=[]
        for name,(kind,before,after)in mutations.items():
            if args.kind!='all'and kind!=args.kind:continue
            start=starts[kind];end=min([pos for pos in starts.values()if pos>start]+[len(source)]);body=source[start:end]
            assert before in body,(name,before);path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(source[:start]+body.replace(before,after,1)+source[end:])
            command=[sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect shared pool mismatch'in run.stdout,(name,run.stdout);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
