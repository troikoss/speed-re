#!/usr/bin/env python3
"""Compare UG2 dirty queues, group storage and secondary device ownership.

All five recovered bodies execute alongside their recovered allocation clients
and known buffer COM methods. Allocation/free and synthetic COM callbacks are
controlled; native replay separately checks normalized synthetic graph results.
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
import verify_pc_effect_metadata as md

ROOT,EXE,LIB=md.ROOT,md.EXE,md.LIB
SOURCE=LIB/'EffectRuntimeStorage.cpp';BUILD=ROOT/'build/analysis/effect-storage'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-storage.json'
ARENA,HEAP,STOP=md.ARENA,md.HEAP,md.STOP
EFFECT,OBJECT=md.EFFECT,md.OBJECT
DEVICE,OTHER,REPLACEMENT,VTABLE=[ARENA+x for x in (0x1600,0x1620,0x1640,0x1680)]
ADDREF,RELEASE=STOP+0x7200,STOP+0x7210
SPANS=dict(reset=(0x65b92d,67),grow=(0x65aaaa,106),group=(0x65b733,90),device=(0x65e591,53),dirty=(0x65ab14,465))
MEMBERS=dict(reset=('D3DXEffectObject','ResetDirtyQueue'),grow=('D3DXEffectObject','GrowDirtyQueue'),
    group=('D3DXEffectGroup64','AllocateData'),device=('D3DXEffectSecondaryPrefix','SetDevice'),dirty=('D3DXEffectObject','MarkParameterDirty'))
md.SOURCES=md.SOURCES+[SOURCE];md.BUILD=BUILD;md.MEMBERS.update(MEMBERS)


class Machine(md.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for address,kind in ((ADDREF,'addref'),(RELEASE,'release')):
            self.u.hook_add(UC_HOOK_CODE,self.com,user_data=kind,begin=address,end=address)
        if linked:
            for name,kind in (('UG2FxComAddRef','addref'),('UG2FxComRelease','release')):
                address=linked['exports'][name if gcc else'_'+name+'@4']
                self.u.hook_add(UC_HOOK_CODE,self.bridge,user_data=kind,begin=address,end=address)
    def bridge(self,u,address,size,kind):
        obj,=self.args(1)
        if self.word(obj)==VTABLE:self.com(u,address,size,kind)
    def com(self,u,address,size,kind):
        obj,=self.args(1);self.events.append((kind,obj,self.state()))
        self.put(obj+4,self.word(obj+4)+(1 if kind=='addref'else-1))
        if self.case['callback']:
            # Exercise the original reread after AddRef and its final writes
            # after Release; these are controlled external virtual callbacks.
            self.put(EFFECT+0xc,REPLACEMENT if kind=='addref'else OTHER)
            self.put(EFFECT+8,0x87654321)
        self.ret(self.word(obj+4),4)
    def setup(self,c):
        self.prepare(c);self.u.mem_write(EFFECT,c['effect']);self.call('effect_ctor',receiver=EFFECT)
    def queue(self,c,grow=False):
        old=self.alloc(128)if c['old']else 0
        count=c['count'];capacity=c['capacity']
        if grow:
            count=c['grow_count'];capacity=c['grow_capacity']
            if count&0x3fffffff>32:count=0
            if ((capacity*8)&0xffffffff)>0x2000:self.case=dict(c,fail_allocation=1)
        self.put(EFFECT+0x108,old,count,capacity)
    def graph(self,c):
        self.queue(c)
        if not c['old']:self.put(EFFECT+0x10c,0,0)
        parameters=self.array(3,0x50,'parameter_ctor');table=self.alloc(12)
        self.put(table,parameters,parameters+0x50,parameters+0xa0);self.put(EFFECT+0xb0,table)
        self.put(parameters+0x14,c['parameter']);self.put(EFFECT+0xac,c['active'])
        groups=self.array(3,0x64,'group_construct');self.put(EFFECT+0x2c,groups)
        records=[]
        for g in range(3):
            records.append(self.array(3,0x64,'record_construct'));self.put(groups+g*0x64+4,records[-1])
            for r in range(3):self.put(records[-1]+r*0x64+0x30,c['record_flags']if (g+r)%2 else c['record_flags']^2)
        uses=self.alloc(6*0x20);row=self.alloc(7*4)
        owner=parameters+c['parameter']*0x50
        for j in range(c['uses']):
            g=j%3;r=(j//3+j)%3;record=records[g]+r*0x64
            if c['duplicate']:g=0;r=0;record=records[0]
            self.put(uses+j*0x20+8,records[(g+1)%3]if c['indirect']else record)
            self.put_half(uses+j*0x20+0x14,g);self.put_half(uses+j*0x20+0x16,r)
            self.put(uses+j*0x20+0x18,uses+((j+c['uses']-1)%c['uses'])*0x20,uses+((j+1)%c['uses'])*0x20)
            self.put(row+j*4,uses+j*0x20)
        self.put(row+c['uses']*4,0);self.put(owner+0x34,uses if c['uses']else 0)
        grid=self.alloc(12);self.put(grid,0,0,0);self.put(grid+c['parameter']*4,row if not c['null_row']else 0)
        dirty=self.alloc(16);self.put(EFFECT+0xb4,grid if c['grid']else 0);self.put(EFFECT+0xb8,grid);self.put(EFFECT+0xbc,dirty)
        self.put(EFFECT+0x118,c['flags'])
        self.parameters,self.groups,self.records,self.uses,self.dirty=parameters,groups,records,uses,dirty
    def run(self,kind,c):
        self.setup(c)
        if kind in('reset','grow'):
            self.queue(c,kind=='grow');result=self.call(kind,receiver=EFFECT)
        elif kind=='group':
            self.call('group_construct',receiver=OBJECT);old=self.alloc(128)if c['old']else 0
            self.put(OBJECT,old);self.put(OBJECT+0xc,c['offset'],c['storage_capacity']);self.put_half(OBJECT+0x34,c['indices'])
            size=(c['offset']+c['indices']*2)&0xffffffff
            if size>0x2000 and size>c['storage_capacity']:self.case=dict(c,fail_allocation=1)
            result=self.call(kind,receiver=OBJECT)
        elif kind=='device':
            token=VTABLE if c['synthetic']else 0x7b711c
            self.put(VTABLE,0,ADDREF,RELEASE)
            for a in(DEVICE,OTHER,REPLACEMENT):self.put(a,token,4,0x12345678,0)
            self.put(EFFECT+8,0x24681357,(0,DEVICE,OTHER)[c['previous']])
            self.case=dict(c,callback=c['callback']and c['synthetic'])
            result=self.call(kind,[(0,DEVICE,OTHER)[c['incoming']]],EFFECT+4)
        else:
            self.graph(c);result=self.call('dirty',[self.parameters],EFFECT)
            if c['repeat']and result==0:result=self.call('dirty',[self.parameters],EFFECT)
        return self.snapshot(result)
    def native(self,mode):
        c=next(cases(1));c.update(effect=b'\xa5'*0x130,fail_allocation=0,callback=False)
        self.setup(c)
        parameters,groups,records,uses,table,grid,row,dirty,group=[ARENA+x for x in(0x2000,0x3000,0x4000,0x5000,0x6000,0x6100,0x6200,0x6300,0x6400)]
        for i in range(3):
            self.u.mem_write(parameters+i*0x50,b'\xa5'*0x50);self.call('parameter_ctor',receiver=parameters+i*0x50)
            self.u.mem_write(groups+i*0x64,b'\xa5'*0x64);self.call('group_construct',receiver=groups+i*0x64)
        for i in range(4):
            self.u.mem_write(records+i*0x64,b'\xa5'*0x64);self.call('record_construct',receiver=records+i*0x64)
            self.put(records+i*0x64+0x30,2 if i==0 else 0)
        self.u.mem_write(uses,b'\0'*128);self.u.mem_write(dirty,b'\0'*16)
        self.put(groups+4,records);self.put(groups+0x64+4,records+0xc8)
        for i in range(4):
            k=0 if i==3 else i
            self.put(uses+i*0x20+8,records+((i+1)%4)*0x64)
            self.put_half(uses+i*0x20+0x14,k//2);self.put_half(uses+i*0x20+0x16,k%2)
            self.put(uses+i*0x20+0x18,uses+((i+3)%4)*0x20,uses+((i+1)%4)*0x20)
        self.put(table,parameters,parameters+0x50,parameters+0xa0);self.put(parameters+0x14,1);self.put(parameters+0x50+0x34,uses)
        self.put(row,uses+0x40,uses,uses+0x60,uses+0x20,0);self.put(grid,0,row,0)
        self.put(EFFECT+0x2c,groups);self.put(EFFECT+0xb0,table,grid if mode&1 else 0,grid,dirty)
        self.put(EFFECT+0xac,1 if mode&4 else 2);self.put(EFFECT+0x118,0x100 if mode&2 else 0)
        words=[self.call('reset',receiver=EFFECT),self.word(EFFECT+0x10c),self.word(EFFECT+0x110)]
        self.put(EFFECT+0x10c,1,1);self.put(self.word(EFFECT+0x108),0x12345678)
        words.append(self.call('dirty',[parameters],EFFECT));words.append(self.call('dirty',[parameters],EFFECT))
        count=self.word(EFFECT+0x10c);words.extend([count,self.word(EFFECT+0x110)])
        words.extend(self.word(self.word(EFFECT+0x108)+i*4)for i in range(count))
        words.extend(self.word(uses+i*0x20)for i in range(4))
        words.extend(self.word(records+i*0x64)for i in range(4))
        words.extend(self.word(records+i*0x64+0x30)&3 for i in range(4));words.append(self.u.mem_read(dirty+1,1)[0])
        self.u.mem_write(group,b'\xa5'*0x64);self.call('group_construct',receiver=group);self.put(group,self.alloc(4))
        self.put(group+0xc,7,4);self.put_half(group+0x34,3)
        words.append(self.call('group',receiver=group));words.extend(self.word(group+o)for o in(0xc,0x10,0x14))
        self.put(group+0xc,3);self.put_half(group+0x34,0)
        words.append(self.call('group',receiver=group));words.extend(self.word(group+o)for o in(0xc,0x10,0x14))
        self.put(DEVICE,0x7b711c,4,0,0);self.put(EFFECT+8,0,0)
        for device in(DEVICE,DEVICE,0):
            words.append(self.call('device',[device],EFFECT+4));words.append(self.word(DEVICE+4))
        words.extend([self.word(EFFECT+8),self.word(EFFECT+0xc)])
        return words


def cases(n):
    rng=random.Random(0x65ab14)
    for i,c in enumerate(md.core.cases(n)):
        c.update(effect=rng.randbytes(0x130),old=bool(i&1),count=i%5,capacity=(0,1,2,4,8)[(i//2)%5],
            grow_count=(0,1,2,4,16,0x40000000,0x80000001)[(i//4)%7],
            grow_capacity=(0,1,2,8,0x20000000,0x40000001,0x80000001)[(i//7)%7],
            offset=(0,1,2,7,31,64,0xfffffff1,0xfffffffe,0xffffffff)[i%9],indices=(0,1,2,7,0xffff)[(i//3)%5],
            storage_capacity=(0,1,2,32,128,0xffffffff)[(i//5)%6],previous=(i//3)%3,incoming=i%3,
            synthetic=bool((i//9)%2),callback=bool((i//6)%2),grid=bool(i&1),parameter=(i//2)%3,active=(i//6)%4,
            uses=(0,1,2,3,5,6)[(i//3)%6],null_row=i%13==0,duplicate=i%7==0,indirect=i%11==0,
            flags=rng.getrandbits(32),record_flags=rng.getrandbits(32),repeat=bool(i%3))
        c['fail_allocation']=0 if i%3 else 1+(i//12)%4
        if not c['old']:c['grow_count']=0
        # Keep ordinary queue writes in their allocated fixture; pathological
        # count/capacity wrapping is tested separately by the grow-only body.
        yield c


def verify(image,linked,gcc,n,kinds):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();returns=Counter();outcomes=Counter()
    for i,c in enumerate(cases(n)):
        for kind in kinds:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect storage mismatch',kind,i,str(e)))from e
            if x!=y:
                offsets=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12]
                event=next(((j,v[:2],w[:2])for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect storage mismatch',kind,i,offsets,'returns',x[2],y[2],'event',event,'counts',len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);returns[hex(x[2])]+=1;outcomes[str((kind,hex(x[2])))]+=1
    return dict(comparisons=dict(counts),boundary_calls=dict(events),returns=dict(returns),returns_by_body=dict(outcomes))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=1024)
    p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--native-dump',type=Path)
    args=p.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert args.cases>0 and hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['functions'].values():
        at=int(f['address'],16)-0x400000;assert hashlib.sha256(image[at:at+f['size']]).hexdigest()==f['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc)
    kinds=list(SPANS)if args.kind=='all'else[args.kind]
    report=verify(image,linked,args.gcc,args.cases,kinds)
    report.update(failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
        limitations=['allocation/free and synthetic COM callbacks controlled; known buffer COM calls execute recovered bodies',
        'finite mapped valid graphs and bounded queue writes; arbitrary malformed pointers, exceptions and unbounded capacities not established',
        'main deserializer, relocation, shared-pool binding, shader preparation and GPU device classes remain open'])
    if args.mutation_checks:
        mutations={
            'reset-skip-null-free':('reset','UG2FxFree(reinterpret_cast<void *>(value108));','if (value108) UG2FxFree(reinterpret_cast<void *>(value108));'),
            'reset-counters-before-allocation':('reset','value108=reinterpret_cast<unsigned int>(UG2FxMalloc(0x400));','value10C=0; value110=0x100; value108=reinterpret_cast<unsigned int>(UG2FxMalloc(0x400));'),
            'wrong-reset-capacity':('reset','value110=0x100;','value110=0x400;'),
            'wrong-reset-allocation':('reset','UG2FxMalloc(0x400)','UG2FxMalloc(0x100)'),
            'grow-retain-capacity-on-oom':('grow','value110<<=1;','unsigned int previousCapacity=value110; value110<<=1;'),
            'wrong-grow-byte-size':('grow','UG2FxMalloc(value110*4)','UG2FxMalloc(value110*8)'),
            'omit-grow-copy':('grow','grown[i]=old[i];','grown[i]=0;'),
            'omit-grow-free':('grow','UG2FxFree(reinterpret_cast<void *>(value108));\n    value108=reinterpret_cast<unsigned int>(grown);','value108=reinterpret_cast<unsigned int>(grown);'),
            'ignore-copy-length-wrap':('grow','unsigned int count=(value10C*4)>>2;','unsigned int count=value10C;'),
            'align-group-offset':('group','unsigned int offset=(value0C&1)+(value0C>>1)*2;','unsigned int offset=(value0C+1)&0xfffffffe;'),
            'omit-group-indices':('group','offset+static_cast<unsigned int>(value34)*2','offset'),
            'grow-equal-group-capacity':('group','if (value10<value0C)','if (value10<=value0C)'),
            'omit-group-old-free':('group','if (data00) { UG2FxFree(data00); data00=0; }','if (data00) { data00=0; }'),
            'group-capacity-before-allocation':('group','void *grown=UG2FxMalloc(value0C);','value10=value0C; void *grown=UG2FxMalloc(value0C);'),
            'omit-device-retain':('device','if (device) UG2FxComAddRef(device);','(void)0;'),
            'cache-device-before-addref':('device','if (device) UG2FxComAddRef(device);','unsigned int previous=value08; if (device) UG2FxComAddRef(device);'),
            'clear-device-before-release':('device','UG2FxComRelease(value08); value08=0;','unsigned int old=value08; value08=0; UG2FxComRelease(old);'),
            'omit-device-borrowed-store':('device','value04=device;','(void)0;'),
            'skip-self-device-assignment':('device','int D3DXEffectSecondaryPrefix::SetDevice(unsigned int device) {','int D3DXEffectSecondaryPrefix::SetDevice(unsigned int device) { if (device==value08) return 0;'),
            'wrong-dirty-queue-flag':('dirty','(&value118)[1]&1','(&value118)[0]&1'),
            'start-use-ring-at-head':('dirty','D3DXEffectUseLink *use=head->previous18;','D3DXEffectUseLink *use=head;'),
            'omit-dirty-link-mark':('dirty','use->value00=1;','use->value00=0;'),
            'omit-dirty-record-mark':('dirty','record->value00=1;','record->value00=0;'),
            'allow-duplicate-queue-entry':('dirty','if (!(record->value30&2))','if (true)'),
            'skip-dirty-queue-growth':('dirty','int result=GrowDirtyQueue();','int result=0;'),
            'wrong-queue-growth-bound':('dirty','if (value110<=value10C)','if (value110<value10C)'),
            'swap-queue-index-words':('dirty','entry[0]=use->value14;\n                        entry[1]=use->value16;','entry[0]=use->value16;\n                        entry[1]=use->value14;'),
            'omit-record-queued-bit':('dirty','record->value30|=2;','(void)0;'),
            'omit-queue-length-increment':('dirty','++value10C;','(void)0;'),
            'inclusive-active-parameter-bound':('dirty','parameter->value14<valueAC','parameter->value14<=valueAC'),
            'omit-grid-dirty-byte':('dirty','reinterpret_cast<unsigned char *>(valueBC)[parameter->value14]=1;','(void)0;'),
            'wrong-grid-record-pointer':('dirty','reinterpret_cast<D3DXEffectRecord64 *>(use->value08)','reinterpret_cast<D3DXEffectGroup64 *>(value2C)[use->value14].array04+use->value16'),
        }
        rejected=[]
        for name,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            body=SOURCE.read_text();assert before in body,name;body=body.replace(before,after)
            if name=='grow-retain-capacity-on-oom':
                start=body.index('int D3DXEffectObject::GrowDirtyQueue()');end=body.index('// Original 0x65b733',start)
                body=body[:start]+body[start:end].replace('if (!grown) return static_cast<int>(0x8007000e);','if (!grown) { value110=previousCapacity; return static_cast<int>(0x8007000e); }')+body[end:]
            if name=='cache-device-before-addref':body=body.replace('if (value08) { UG2FxComRelease(value08);','if (previous) { UG2FxComRelease(previous);')
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(body)
            r=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','512']+(['--gcc']if args.gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            path.with_suffix('.log').write_text(r.stdout);assert r.returncode and 'effect storage mismatch'in r.stdout,(name,r.stdout)
            rejected.append(name)
        report['rejected_mutations']=rejected
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXST1';at=8;seen=set();a,b=Machine(image),Machine(image,linked,args.gcc)
        while at<len(raw):
            mode,count=struct.unpack_from('<2I',raw,at);at+=8;words=list(struct.unpack_from('<'+'I'*count,raw,at));at+=count*4
            assert mode in range(8)and mode not in seen;seen.add(mode)
            x=a.native(mode);y=b.native(mode);assert x==y==words,('native effect storage mismatch',mode,x,y,words)
        assert at==len(raw)and len(seen)==8
        report['native_replay']=dict(graphs=8,all_normalized_queue_record_storage_and_reference_results_match=True)
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'-'+args.kind+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
