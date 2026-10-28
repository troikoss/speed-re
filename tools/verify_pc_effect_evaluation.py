#!/usr/bin/env python3
"""Verify the record evaluator, CRT x87 controls and recovered input-copy chain.

Bounded programs execute the original VM on both sides; explicit controlled VM
cases check propagation/call contracts. No native shader VM is fabricated.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_effect_evaluation_copy as cp

md=cp.md;ROOT,EXE,LIB=cp.ROOT,cp.EXE,cp.LIB;ARENA,HEAP,STOP,EFFECT=md.ARENA,md.HEAP,md.STOP,md.EFFECT
SOURCE=LIB/'EffectEvaluation.cpp';CONTROL=LIB/'EffectFpControl.cpp';BUILD=ROOT/'build/analysis/effect-evaluation';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-evaluation.json'
SPANS=dict(evaluate=(0x663a32,383),abstract=(0x75da18,146),hardware=(0x75daaa,142),control87=(0x75db38,50),controlfp=(0x75db6a,22))
GROUPS,RECORDS,DATA,TABLE,TYPE,PARAMETERS,INPUT,HEADER,BUFFER,CODE,LITERALS,ENTRIES=[ARENA+x for x in(0x2000,0x2200,0x3000,0x8000,0xa000,0xa800,0xb000,0xc000,0xc100,0xc200,0xc400,0xd000)]
md.core.HSIZE=0x220000;md.SOURCES += [SOURCE,CONTROL,LIB/'EffectVm.cpp',ROOT/'src/Platform/Native/NativeFxFpControl.cpp',ROOT/'src/Platform/Native/NativeFxVmDispatch.cpp',ROOT/'src/Platform/Native/NativeFxVmCrtPlatform.cpp'];md.BUILD=BUILD
md.core.GLOBALS.update(ug2VmZero786E60=0x786e60,ug2VmOne786E68=0x786e68,ug2VmTwo787AD0=0x787ad0,ug2VmLogScale7BAA88=0x7baa88,_fltused=0,_ftol2=0x75dd28)
md.MEMBERS['evaluate']=('D3DXEffectObject','EvaluateRecord');md.FREE.update(abstract='D3DXEffectAbstractControl',hardware='D3DXEffectHardwareControl',control87='D3DXEffectControl87',controlfp='D3DXEffectControlFp');md.CDECL.update(('abstract','hardware','control87','controlfp'))


def pack(words):return cp.pack(words)


def digest(data):
    value=2166136261
    for byte in data:value=((value^byte)*16777619)&0xffffffff
    return value


def cases(n):
    rng=random.Random(0x663a32)
    for i,c in enumerate(cp.cases(n)):
        c.update(word=(0x7f,0x27f,0x37f,0x67f,0xa7f,0xe7f)[i%6],second_word=(0x7f,0x27f,0x37f)[(i//3)%3],flags=rng.getrandbits(32),saved=rng.getrandbits(32),
            group=i%2,record=(i//3)%4,uses=(0,1,2,3,4,7,16)[(i//2)%7],table_present=i%7!=0,mask=(0,1,3,7,15)[(i//3)%5],input_mask=(0,1,3,7,15)[(i//5)%5],
            controlled=i%5==0,vm_result=(0,1,0x80004005,0x88760859)[(i//5)%4],vm_write=i%3==0,repeat=i%4!=0,
            source_kind=1+i%3,value00=(0,1,2,0x80000000,0xffffffff)[i%5],value04=(0,1,2,0x12345678,0xffffffff)[(i//7)%5],banks=(i//11)%4)
        if i%2:c['flags']|=1
        else:c['flags']&=~1
        c['alias']=bool(i%9==0);c['depth']=(i//17)%3;c['shape']=(i//7)%4;c['kind']=c['source_kind'];c['rows']=1+(i//11)%4;c['columns']=1+(i//13)%4;c['elements']=(i//9)%3;c['reflect_override']=False
        c['types'],c['table'],c['root']=cp.serialize(c)
        if i==n-1 and n>=1024:c.update(uses=0xffff,controlled=False,repeat=False,table_present=True,value00=0)
        c['mapping']=rng.getrandbits(32);c['new_value']=rng.getrandbits(32);c['mask_bits']=rng.getrandbits(32)
        yield c


class Machine(cp.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.is_linked=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        self.instruction_limit=4000000
        for key in('abstract','hardware','control87','controlfp'):
            at=self.entries[key];self.u.hook_add(UC_HOOK_CODE,self.control_entry,user_data=key,begin=at,end=at)
        for key in('copy_vectors','copy_scalars'):
            at=self.entries[key];self.u.hook_add(UC_HOOK_CODE,self.copy_entry,user_data=key,begin=at,end=at)
        self.u.hook_add(UC_HOOK_CODE,self.vm_entry,begin=0x688e79,end=0x688e79)
        if linked:
            at=linked['exports']['UG2FxExecuteVm'if gcc else'_UG2FxExecuteVm@56'];self.u.hook_add(UC_HOOK_CODE,self.vm_bridge,begin=at,end=at)
    def control_entry(self,u,address,size,kind):
        if getattr(self,'mapping_quiet',False):return
        args=self.args(1)if self.is_linked else(u.reg_read(xc.UC_X86_REG_EBX),)if kind in('abstract','hardware')else self.args(2)
        if kind not in('abstract','hardware'):args=self.args(2)
        if kind=='abstract':args=(args[0]&0xffff,)
        self.events.append((kind,args,u.reg_read(xc.UC_X86_REG_FPCW),hashlib.sha256(self.state()).hexdigest()))
    def copy_entry(self,u,address,size,kind):
        args=list(self.args(7));args[5]=self.word(args[5]);self.events.append((kind,tuple(args),u.reg_read(xc.UC_X86_REG_FPCW),hashlib.sha256(self.state()).hexdigest()))
    def vm_bridge(self,u,address,size,user):u.reg_write(xc.UC_X86_REG_EIP,0x688e79)
    def vm_entry(self,u,address,size,user):
        args=self.args(14);self.vm_args=args;self.events.append(('vm',args,u.reg_read(xc.UC_X86_REG_FPCW),hashlib.sha256(self.state()).hexdigest()))
        normalized=[value-ARENA if index<8 and value else value for index,value in enumerate(args)]
        self.vm_calls.extend(normalized+[u.reg_read(xc.UC_X86_REG_FPCW)&~0x40,self.word(EFFECT+0x114),self.word(EFFECT+0x118),digest(bytes(u.mem_read(DATA,0x4000))),digest(self.use_bytes())])
        if self.case['controlled']:
            if self.case['vm_write']:self.put(args[5],0x89abcdef,0x01234567)
            self.ret(self.case['vm_result'],56)
    def call(self,kind,args=(),receiver=0):
        u=self.u;private=kind in('abstract','hardware')and not self.is_linked;member=kind=='evaluate';params=([receiver]+list(args))if self.gcc and member else []if private else list(args)
        stack=md.core.STACK+0xf000;self.put(stack,STOP,*params);registers=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(args[0]if private else 0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(registers,saved):u.reg_write(r,v)
        for r,v in((xc.UC_X86_REG_ESP,stack),(xc.UC_X86_REG_ECX,receiver),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,self.control_word),(xc.UC_X86_REG_FPTAG,0xffff)):u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=self.instruction_limit)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        assert u.reg_read(xc.UC_X86_REG_ESP)==stack+4+(8 if member and not self.gcc else 0),('stack',kind)
        assert tuple(u.reg_read(r)for r in registers)==saved,('registers',kind)
        assert u.reg_read(xc.UC_X86_REG_FPTAG)==0xffff and self.word(0)==0x12345678
        self.control_word=u.reg_read(xc.UC_X86_REG_FPCW);return u.reg_read(xc.UC_X86_REG_EAX)
    def fixture(self,c,native=False):
        self.case=c;self.u.mem_write(ARENA,b'\xa5'*md.core.SIZE);self.u.mem_write(HEAP,b'\x5a'*md.core.HSIZE);self.put(0,0x12345678);self.live={};self.allocated=c['uses']*32;self.requests=0
        self.put(EFFECT+0x2c,GROUPS);self.put(EFFECT+0x114,c['saved'],c['flags']);self.arena_relocations=[EFFECT+0x2c-ARENA];self.heap_relocations=[]
        for group in range(2):
            at=GROUPS+group*0x64;self.put(at,DATA+group*0x2000,RECORDS+group*0x200);self.arena_relocations += [at-ARENA,at+4-ARENA]
            for j in range(0,0x1800,8):self.u.mem_write(DATA+group*0x2000+j,struct.pack('<d',((j//8)%17-8)*0.25))
        self.group=GROUPS+c['group']*0x64;self.record=RECORDS+c['group']*0x200+c['record']*0x64;data=self.word(self.group)
        self.put(self.record+0x34,HEAP,BUFFER,TABLE if c['table_present']else 0,CODE,LITERALS);self.put_half(self.record+0x4e,c['uses']);self.arena_relocations += [self.record+off-ARENA for off in(0x34,0x38,0x3c,0x40,0x44)]
        self.put(BUFFER,0x7b711c,1,28,HEADER);self.arena_relocations.append(BUFFER+12-ARENA)
        self.put(HEADER,0x12345678,c['mask'],c['input_mask'],0x500 if c['alias']else 0x100,0x500,0x900,0xd00)
        self.u.mem_write(TABLE,c['table']);self.u.mem_write(TYPE,c['types'])
        for j in range(4):
            parameter=PARAMETERS+j*0x50;self.put(parameter,INPUT+j*0x200,TYPE);self.arena_relocations += [parameter-ARENA,parameter+4-ARENA]
            for k in range(128):self.put(INPUT+j*0x200+k*4,((j+1)*0x3f000000+k*0x1000)&0xffffffff)
            self.put(ENTRIES+j*20,0,((j%3)<<16)|((j+c['banks'])%4),1+j%3,c['root'],0)
        for j in range(c['uses']):
            active=c['value00']if j%3 else 0;self.put(HEAP+j*32,active,c['value04'],0x55555555,PARAMETERS+(j%4)*0x50,ENTRIES+(j%4)*20 if active else 0,0xa5a5a5a5,0xa5a5a5a5,0xa5a5a5a5);self.heap_relocations += [j*32+12,j*32+16]
        operands=[0,2 if c['table_present']else 4,0,0,1,1]
        opcode=(0x10000001,0x20400001,0x20500001)[c['seed']%3];number=1 if opcode==0x10000001 else 2
        program=[1,opcode,number,*operands[:number*3],0,4,1];self.put(CODE,*program);self.put(LITERALS,4)
        for j,value in enumerate((0.5,-1.25,2.0,3.5)):self.u.mem_write(LITERALS+4+j*8,struct.pack('<d',value))
        self.control_word=c['word'];self.events=[];self.vm_args=();self.vm_calls=[]
    def use_bytes(self):return pack([self.word(HEAP+j*32+off)for j in range(self.case['uses'])for off in(0,4)])
    def run(self,kind,c,native=False):
        self.fixture(c,native)
        if kind=='evaluate':
            result=self.call(kind,[c['group'],c['record']],EFFECT)
            if c['repeat']:
                self.control_word=c['second_word'];result=(result,self.call(kind,[c['group'],c['record']],EFFECT))
        else:result=self.call(kind,[c['mapping']]if kind in('abstract','hardware')else[c['new_value'],c['mask_bits']])
        return self.snapshot(result)+(self.control_word,)


def verify(image,linked,gcc,n):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter();results=Counter()
    for i,c in enumerate(cases(n)):
        for kind in SPANS:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect evaluation mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0]+x[1],y[0]+y[1]))if v!=w][:12];event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect evaluation mismatch',kind,i,diffs,x[2],y[2],x[-1],y[-1],event,len(x[3]),len(y[3])))
            counts[kind]+=1;events.update(e[0]for e in x[3]);results[str((kind,x[2]))]+=1
    return dict(comparisons=dict(counts),calls=dict(events),returns=dict(results),failures=0)


def native_cases():
    for i,c in enumerate(cases(64)):
        c.update(controlled=True,repeat=i%4!=0,vm_result=(0,1,0x80004005,0x88760859)[i%4]);yield i,c


def native_capture(m,result):
    values=result if isinstance(result,tuple)else(result,0xffffffff)
    return pack([*values,m.control_word&~0x40,m.word(EFFECT+0x114),m.word(EFFECT+0x118),len(m.vm_calls)//19,*m.vm_calls])+bytes(m.u.mem_read(DATA,0x4000))+m.use_bytes()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--gcc',action='store_true');parser.add_argument('--cases',type=int,default=1024);parser.add_argument('--replace',type=Path);parser.add_argument('--replace-control',type=Path);parser.add_argument('--write-native-fixtures',type=Path);parser.add_argument('--native-dump',type=Path);parser.add_argument('--mutation-checks',action='store_true')
    args=parser.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in manifest['functions'].values():
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    replacements={}
    if args.replace:replacements[SOURCE.name]=args.replace
    if args.replace_control:replacements[CONTROL.name]=args.replace_control
    linked=md.build(replacements,args.gcc);report=verify(image,linked,args.gcc,args.cases);report.update(compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.write_native_fixtures:
        content=b'UG2FXVF1'+pack([64]);m=Machine(image)
        for i,c in native_cases():
            m.fixture(c,True);arena=bytes(m.u.mem_read(ARENA,md.core.SIZE));heap=bytes(m.u.mem_read(HEAP,m.allocated))
            config=[i,c['group'],c['record'],c['uses'],c['word'],c['second_word'],int(c['repeat']),c['vm_result'],int(c['vm_write']),len(arena),len(heap),len(m.arena_relocations),len(m.heap_relocations)]
            content+=pack(config)+arena+heap+pack(m.arena_relocations)+pack(m.heap_relocations)
        m.mapping_quiet=True;content+=pack([65536])
        for word in range(65536):
            hardware=word|((word^0xa5a5)<<16);abstract=(word&0x3ff)|((word&0xfc00)<<6)
            content+=pack([hardware,m.call('abstract',[hardware]),abstract,m.call('hardware',[abstract])])
        content+=pack([2048]);rng=random.Random(0x75db38)
        for i in range(2048):
            initial=(((i//2)&31)<<8)|0x7f;new=rng.getrandbits(32)if i>=64 else 0;mask=rng.getrandbits(32)if i>=64 else 0;kind=i%2;m.control_word=initial;result=m.call('controlfp'if kind else'control87',[new,mask]);content+=pack([initial,new,mask,kind,result,m.control_word&~0x40])
        args.write_native_fixtures.write_bytes(content)
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXVD1';at=8;a,b=Machine(image),Machine(image,linked,args.gcc)
        for i,c in native_cases():
            x=a.run('evaluate',c,True);y=b.run('evaluate',c,True);assert x==y,('effect evaluation mismatch','native model',i)
            expected=native_capture(a,x[2]);index,size=struct.unpack_from('<2I',raw,at);at+=8;actual=raw[at:at+size];at+=size
            assert index==i and actual==expected,('native effect evaluation mismatch',i,[(hex(j),v,w)for j,(v,w)in enumerate(zip(expected,actual))if v!=w][:12])
        mapping,controls=struct.unpack_from('<2I',raw,at);at+=8;assert mapping==131072 and controls==2048
        assert at==len(raw);report.update(native_replay=64,native_mapping_checks=mapping,native_control_checks=controls)
    if args.mutation_checks:
        mutations={
            'group-selection':('caller','+groupIndex;','+(groupIndex^1);'),
            'record-selection':('caller','group->array04+recordIndex;','group->array04+(recordIndex^1);'),
            'output-offset':('caller','group->data00)+header[3]','group->data00)+header[4]'),
            'output-mask':('caller','outputMask=header[1]','outputMask=header[2]'),
            'first-control-gate':('caller','if (!(value118&1))','if (!(value118&2))'),
            'saved-abstract-control':('caller','value114=D3DXEffectControlFp(0,0);','D3DXEffectControlFp(0,0);'),
            'precision-mask':('caller','D3DXEffectControlFp(0,0x30000);','D3DXEffectControlFp(0,0x10000);'),
            'control-ready-flag':('caller','value118|=1;','value118|=2;'),
            'constant-table-gate':('caller','if (record->value3C)','if (false)'),
            'dirty-use-gate':('caller','if (node->value00)','if (!node->value00)'),
            'dirty-use-publication':('caller','node->value00=node->value04;','node->value00=0;'),
            'active-use-count':('caller','use<record->value4E','use<record->value50'),
            'constant-register-count':('caller','count=static_cast<unsigned short>(entry[2]);','count=static_cast<unsigned short>(entry[2]>>16);'),
            'scalar-copy-boundary':('caller','if (!bank) D3DXEffectCopyEvaluationScalars','if (!bank) D3DXEffectCopyEvaluationVectors'),
            'integer-bank-base':('caller','integers+start*32','vectors+start*32'),
            'float-bank-base':('caller','vectors+start*32','integers+start*32'),
            'scalar-register-stride':('caller','scalars+start*8','scalars+start*32'),
            'integer-register-stride':('caller','integers+start*32','integers+start*16'),
            'vm-input-base':('caller','input=reinterpret_cast<unsigned int>(group->data00)+header[4];','input=reinterpret_cast<unsigned int>(group->data00)+header[5];'),
            'vm-input-mask':('caller','inputMask=header[2];','inputMask=header[1];'),
            'vm-input-argument':('caller','record->value44),0,input,0,','record->value44),0,0,0,'),
            'vm-input-mask-argument':('caller','output,output,output,0,inputMask,0,','output,output,output,0,0,0,'),
            'vm-third-output-bank':('caller','output,output,output,0,inputMask,0,','output,output,0,0,inputMask,0,'),
            'vm-third-output-mask':('caller','outputMask,outputMask,outputMask);','outputMask,outputMask,0);'),
            'vm-negative-result':('caller','outputMask,outputMask,outputMask);','outputMask,outputMask,outputMask)&0x7fffffff;'),
            'abstract-invalid-mask':('control','if (word&1) value=0x10;','if (word&1) value=8;'),
            'abstract-denormal-mask':('control','if (word&2) value|=0x80000;','if (word&2) value|=0;'),
            'abstract-round-down':('control','rounding==0x400) value|=0x100;','rounding==0x400) value|=0x200;'),
            'abstract-single-precision':('control','if (!(word&0x300)) value|=0x20000;','if (!(word&0x300)) value|=0x10000;'),
            'abstract-infinity-control':('control','if (word&0x1000)','if (word&0x2000)'),
            'hardware-invalid-mask':('control','unsigned int word=(value&0x10)!=0;','unsigned int word=(value&0x10)!=0 ? 2 : 0;'),
            'hardware-denormal-mask':('control','if (value&0x80000) word|=2;','if (value&0x80000) word|=0;'),
            'hardware-round-down':('control','rounding==0x100) word|=0x400;','rounding==0x100) word|=0x800;'),
            'hardware-extended-precision':('control','if (!(value&0x30000)) word|=0x300;','if (!(value&0x30000)) word|=0x200;'),
            'hardware-invalid-precision':('control','else if ((value&0x30000)==0x10000)','else if ((value&0x30000)>=0x10000)'),
            'hardware-infinity-control':('control','if (value&0x40000)','if (value&0x80000)'),
            'control-merge':('control','(old&~mask)|(value&mask)','(old&mask)|(value&~mask)'),
            'zero-mask-rewrite':('control','unsigned int result=(old&~mask)|(value&mask);','unsigned int result=(old&~mask)|(value&mask);if (!mask) return result;'),
            'abstract-return-bits':('control','return result;','return D3DXEffectAbstractControl(D3DXEffectHardwareControl(result));'),
            'hardware-word-publication':('control','UG2FxWriteControlWord(D3DXEffectHardwareControl(result));','UG2FxWriteControlWord(result);'),
            'controlfp-denormal-protection':('control','mask&0xfff7ffff','mask'),
        }
        rejected=[]
        for name,(unit,before,after)in mutations.items():
            source=SOURCE if unit=='caller'else CONTROL;content=source.read_text();assert before in content,(name,before)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
            command=[sys.executable,__file__,'--replace'if unit=='caller'else'--replace-control',str(path),'--cases','64']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect evaluation mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/(('fixtures-'if args.write_native_fixtures else'verification-')+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
