#!/usr/bin/env python3
"""Verify UG2's complete x87 power family and dispatch/SSE argument wrapper.

The original SSE body 0x764749 remains an explicit controlled boundary here.
Its arithmetic is NOT claimed recovered. All x87 helpers, environment save/
restore, exceptional-value handling and CRT error dispatch execute on both sides.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_crt_inverse as iv
cx,fl,md=iv.cx,iv.fl,iv.md;ROOT,EXE,LIB=iv.ROOT,iv.EXE,iv.LIB;ARENA,STOP=iv.ARENA,iv.STOP
BUILD=ROOT/'build/analysis/crt-power';md.BUILD=BUILD
SOURCE=LIB/'CrtPower.cpp';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/crt-power-x87.json'
SPANS=dict(power=(0x75c9b0,84),sse_wrapper=(0x764730,25),x87=(0x75ca0d,453),load=(0x765705,67),exp2=(0x7656c0,21),parity80=(0x75cbd2,40),parity=(0x76584c,106),classify=(0x75dc8e,154),special=(0x7658b6,354),two_arg=(0x765660,23),exit=(0x7657a9,163))
NAMES=dict(power='D3DXCrtPower80',sse_wrapper='D3DXCrtPowerSseWrapper80',x87='D3DXCrtPowerX8780',load='D3DXCrtPowerLoadOperand',exp2='D3DXCrtReconstructExp2',parity80='D3DXCrtPowerParity80',parity='D3DXCrtPowerParity',classify='D3DXCrtFpClass',special='D3DXCrtPowerSpecial',two_arg='D3DXCrtTwoArgError',exit='D3DXCrtPowerMathExit',bridge='UG2FxVmPower')
md.FREE.update(NAMES);md.CDECL.update(NAMES)
BASE,EXPONENT,VALUE,OUTPUT,CARRY,SAVED,RESULT= [ARENA+x for x in(0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600)]
ADAPTER=STOP+0xe000;SSE_STUB=STOP+0xf200
VOID=set(NAMES)-{'load','parity80','parity','classify','special'}
EXPONENTS=(0,0x8000000000000000,0x3ff0000000000000,0xbff0000000000000,0x3fe0000000000000,0xbfe0000000000000,0x4000000000000000,0xc000000000000000,0x4008000000000000,0xc008000000000000,0x3ff8000000000000,0xbff8000000000000,0x7ff0000000000000,0xfff0000000000000,0x7ff8000000001234,0x7ff0000000001234,0x4090c80000000000,0xc090c80000000000,0x4090000000000000,0xc090000000000000,0x4340000000000001,0x4330000000000001,1,0x8000000000000001)


def cases(n):
    rng=random.Random(0x75c9b0)
    for i,c in enumerate(iv.cases(n)):
        c=dict(c);exponent=EXPONENTS[(i//len(cx.FIXED))%len(EXPONENTS)]if i<6144 else rng.getrandbits(64)
        # A separate domain includes moderate real exponents and bases around 1.
        if i%5==0:
            c['bits']=struct.unpack('<Q',struct.pack('<d',(0.125,0.5,0.9999999999999999,1,1.0000000000000002,1.5,2,4,-0.5,-2)[i//5%10]))[0]
            c['extended']=iv.extended(c['bits']);exponent=struct.unpack('<Q',struct.pack('<d',(-2048,-1536,-1075,-1074,-1024,-128,-3,-2,-1,-0.5,0,0.5,1,2,3,128,1024,1074,1536,2048)[i//50%20]))[0]
        exp80=iv.extended(exponent)
        if i%23==0:
            significand,word=struct.unpack('<QH',exp80)
            if word&0x7fff not in(0,0x7fff):exp80=struct.pack('<QH',significand|0x155,word)
        c.update(other=exponent,exp80=exp80,sse=i//17%2,name=0x819c88,sse_result=(0x3ff8000000000000,0xc00e000000000000,0,0x800fffffffffffff)[i%4],operation=(29,13,16,22)[i%4])
        if i%41==0:c['mxcsr']&=~(1<<(7+i%6))
        c['exit_operation']=29 if c['operation']==29 else 13
        yield c


def native_cases(n):
    for c in cases(n):
        c=dict(c);c['control']|=0x7f;c['saved']|=0x7f;c['mxcsr']|=((c['seed']>>4)%4 in(1,3))*0x40|((c['seed']>>4)%4 in(2,3))*0x8000
        yield c


def write_native_fixtures(path,n):
    raw=b'UG2FXPI1'+struct.pack('<I',n)
    for i,c in enumerate(native_cases(n)):
        raw+=struct.pack('<15I3Q',i,c['control'],c['mxcsr'],c['status'],c['saved'],c['sse'],int(c['bypass']),c['mode'],c['operation'],c['exit_operation'],c['type']&0xffffffff,c['seed'],int(c['callback_write']),int(c['redirect']),c['record_bits'],c['bits'],c['other'],c['sse_result'])+c['extended']+bytes(2)+c['exp80']+bytes(2)
    path.write_bytes(raw)


class Machine(iv.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.paths=Counter()
        if not linked:
            self.entries.update(parity=0x76584c,classify=0x75dc8e,special=0x7658b6)
            for j,kind in enumerate(('power','sse_wrapper','x87','load','exp2','parity80','two_arg','exit','bridge')):self.entries[kind]=ADAPTER+j*0x100
            self.u.hook_add(UC_HOOK_CODE,self.sse,begin=0x764749,end=0x764749)
            for at,label in((0x75c9eb,'x87'),(0x764730,'sse_wrapper'),(0x7658b6,'exceptional_C_helper'),(0x765660,'two_arg_error')):self.u.hook_add(UC_HOOK_CODE,self.path,user_data=label,begin=at,end=at)
        else:
            name='UG2FxCrtPowerSseBody'if gcc else'_UG2FxCrtPowerSseBody';at=linked['exports'][name];self.u.hook_add(UC_HOOK_CODE,self.sse,begin=at,end=at)
        self.u.mem_write(SSE_STUB,b'\xdd\x05'+struct.pack('<I',RESULT)+b'\xc3')
    def path(self,u,at,size,label):self.paths[label]+=1
    def sse(self,u,at,size,user):
        if self.linked:
            out,*args=self.args(5);u.mem_write(out,struct.pack('<Q',self.case['sse_result']));self.events.append(('sse_body',tuple(args),self.fp_state()));self.ret()
        else:self.events.append(('sse_body',self.args(4),self.fp_state()));u.reg_write(xc.UC_X86_REG_EIP,SSE_STUB)
    def default_entry(self,u,at,size,user):
        data=bytearray(u.mem_read(self.args(1)[0],32))
        if self.kind=='exit'and self.case['exit_operation']!=29:data[16:24]=bytes(8)
        self.events.append(('default',data.hex(),self.fp_state()))
    def known_record(self,p):
        result=cx.Machine.known_record(self,p)
        operation=29 if self.kind in('power','x87','bridge','vm')else self.case['exit_operation']if self.kind=='exit'else self.case['operation']
        if operation in(16,22,29):
            w=struct.unpack('<32I',self.u.mem_read(p,128));result=(*result,w[12],w[13],w[16]&31)
        return result
    def adapters(self,c):
        imm=lambda x:struct.pack('<I',x&0xffffffff)
        mov=lambda op,x:bytes([op])+imm(x)
        fld=lambda p:b'\xdb\x2d'+imm(p)
        store=lambda p:b'\xdb\x3d'+imm(p)
        for j,kind in enumerate(('power','sse_wrapper','x87','load','exp2','parity80','two_arg','exit','bridge')):
            at=ADAPTER+j*0x100;code=b''
            def call(target):return b'\xe8'+struct.pack('<i',target-(at+len(code)+5))
            if kind in('power','sse_wrapper'):
                code=fld(BASE)+fld(EXPONENT);code+=call(SPANS[kind][0]);code+=store(OUTPUT)+b'\xc3'
            elif kind=='x87':
                args=[*fl.words(c['bits']),*fl.words(c['other'])];code=b''.join(b'\x68'+imm(x)for x in reversed(args))+fld(EXPONENT)+mov(0xb8,c['other']>>32)
                code+=call(0x75ca0d);code+=store(OUTPUT)+b'\x83\xc4\x10\xc3'
            elif kind=='load':
                code=mov(0xba,fl.DATA)+mov(0xb9,c['seed']);code+=call(0x765705);code+=store(OUTPUT)+b'\x89\x0d'+imm(CARRY)+b'\xc3'
            elif kind in('exp2','parity80'):
                code=fld(VALUE);code+=call(SPANS[kind][0]);code+=(store(VALUE)if kind=='exp2'else b'\x0f\xb6\xc1')+b'\xc3'
            elif kind=='two_arg':
                args=[c['saved'],0,*fl.words(c['bits']),*fl.words(c['other'])];code=b''.join(b'\x68'+imm(x)for x in reversed(args))+fld(VALUE)+mov(0xb8,c['type'])+mov(0xb9,c['name'])+mov(0xba,c['operation'])
                code+=call(0x765660);code+=store(VALUE)+b'\x83\xc4\x18\xc3'
            elif kind=='exit':
                code=b''.join(b'\x68'+imm(x)for x in reversed([*fl.words(c['bits']),*fl.words(c['other'])]))+fld(VALUE)+mov(0xba,c['exit_operation'])+mov(0xb9,c['name'])
                after=at+len(code)+15;code+=b'\x68'+imm(after)+b'\x68'+imm(c['saved']);code+=b'\xe9'+struct.pack('<i',0x7657a9-(at+len(code)+5));code+=store(VALUE)+b'\x83\xc4\x10\xc3'
            else:code=b'\xdd\x44\x24\x04\xdd\x44\x24\x0c';code+=b'\xe9'+struct.pack('<i',0x75c9b0-(at+len(code)+5))
            self.u.mem_write(at,code)
        self.u.ctl_remove_cache(ADAPTER,ADAPTER+0x900)
    def run_power(self,kind,c):
        self.run_exception('error_code',c);self.kind=kind;self.events=[];u=self.u
        for at,data in((BASE,c['extended']),(EXPONENT,c['exp80']),(VALUE,c['extended'])):u.mem_write(at,data+b'\xa5\xa5')
        u.mem_write(OUTPUT,b'\xa5'*12);self.put(CARRY,c['seed']);self.put(SAVED,c['saved']);self.put(0x8ba480,c['sse']);self.put(0x8b8424,int(c['bypass']));u.mem_write(RESULT,struct.pack('<Q',c['sse_result']))
        if not self.linked and getattr(self,'adapter_case',None)is not c:self.adapters(c);self.adapter_case=c
        a=fl.words(c['bits']);b=fl.words(c['other'])
        args=dict(power=(OUTPUT,BASE,EXPONENT),sse_wrapper=(OUTPUT,BASE,EXPONENT),x87=(OUTPUT,*a,*b,EXPONENT),load=(OUTPUT,*a,CARRY),exp2=(VALUE,),parity80=(VALUE,),parity=a,classify=a,special=(*a,*b,fl.DATA+64),two_arg=(c['operation'],c['type'],c['name'],*a,*b,c['saved'],VALUE),exit=(c['exit_operation'],c['name'],*a,*b,c['saved'],VALUE),bridge=(*a,*b))
        state=self.call_exception(kind,args[kind]);state=(None if kind in VOID else state[0],*state[1:])
        return(state,bytes(u.mem_read(VALUE,10)).hex(),bytes(u.mem_read(OUTPUT,10)).hex(),bytes(u.mem_read(CARRY,4)).hex(),bytes(u.mem_read(fl.DATA,128)).hex(),bytes(u.mem_read(fl.THREAD,32)).hex(),self.events)


def composed(image,linked,gcc):
    machines=Machine(image),Machine(image,linked,gcc);calls=0
    for i,c in enumerate(cases(1024)):
        width=i%4;offset=(-4,0,4,8,32)[i%5];observed=[]
        for m in machines:
            m.run_power('classify',c);m.kind='vm';m.events=[]
            for lane in range(3):m.u.mem_write(fl.DATA+16+lane*8,struct.pack('<Q',c['other']))
            m.put(ARENA+0x4000,1,0x10500000|width,1,0,3,0,0,4,0);m.put(ARENA+0x5000,1,0,0)
            state=m.call_crt('vm',[ARENA+0x4000,ARENA+0x5000,fl.DATA+16,0,0,fl.DATA+16+offset,0,0,0,0,0,0,0,0])
            observed.append((state,bytes(m.u.mem_read(fl.DATA,128)),bytes(m.u.mem_read(fl.THREAD,32)),m.events))
        assert observed[0]==observed[1],('CRT power mismatch','composed VM',i,c,observed)
        calls+=width
    return dict(programs=1024,power_calls=calls,SSE_body='controlled original/copy boundary; arithmetic not recovered')


def mutation_checks(gcc):
    mutations={
        'gate-cpu':('power','if (ug2CrtSseEnabled8BA480&&','if (true&&'),
        'gate-mxcsr':('power','(UG2FxCrtReadMxcsr()&0x1f80)==0x1f80','(UG2FxCrtReadMxcsr()&0x1f80)==0'),
        'gate-x87':('power','(UG2FxCrtReadControlNoWait()&0x7f)==0x7f','(UG2FxCrtReadControlNoWait()&0x7f)==0x3f'),
        'classify-negative-normal':('classify','return negative ? 8 : 0x100','return negative ? 4 : 0x100'),
        'classify-negative-denormal':('classify','return negative ? 0x10 : 0x80','return negative ? 0x20 : 0x80'),
        'parity-second-test':('parity','UG2_FX_VM_EQ(rounded,half) ? 2 : 1','UG2_FX_VM_EQ(rounded,half) ? 1 : 2'),
        'parity80-unordered':('parity80','UG2FxCrtExtendedCompare(&rounded,value)&0x4000','UG2FxCrtExtendedCompare(&rounded,value)&0x100'),
        'parity80-odd':('parity80','&half)&0x4000) ? 2 : 1','&half)&0x4000) ? 1 : 2'),
        'load-payload':('load','prepared.low=value.low<<11','prepared.low=value.low<<10'),
        'load-carry':('load','*carry=value.low<<11','*carry=value.low<<10'),
        'load-normal-sign':('load','value.high&0x7ff00000','value.high&0xfff00000'),
        'exp2-negation':('exp2','UG2FxCrtExtendedNegate(&difference);',''),
        'exp2-scale':('exp2','UG2FxCrtExtendedScale(&result,&rounded);',''),
        'special-unit-infinite':('special','ug2CrtPowerIndefinite81A818);error=1','ug2CrtPowerIndefinite81A818);error=0'),
        'special-odd-infinite':('special','if (parity==1) UG2_FX_VM_STORE_NEG','if (parity==2) UG2_FX_VM_STORE_NEG'),
        'special-negative-zero':('special','UG2_FX_VM_STORE_LOAD(result,ug2CrtPowerNegativeZero81A830)','UG2_FX_VM_STORE_LOAD(result,ug2VmZero786E60)'),
        'two-arg-second':('two_arg','UG2_CRT_COPY_DOUBLE_BITS(error.input10,second)','UG2_CRT_COPY_DOUBLE_BITS(error.input10,first)'),
        'two-arg-reload':('two_arg','UG2FxCrtExtendedFromDouble(value,&error.result18)','UG2FxCrtExtendedFromDouble(value,&error.input10)'),
        'two-arg-second-restore':('two_arg','if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);','if (false) UG2FxWriteControlWord(saved);'),
        'exit-precision-mask':('exit','!(saved&0x20)&&(D3DXCrtReadStatus()&0x20)','(saved&0x20)&&(D3DXCrtReadStatus()&0x20)'),
        'exit-scale':('exit','!exponent ? &ug2CrtPowerScaleUp7D7C6C : &ug2CrtPowerScaleDown7D7C64','!exponent ? &ug2CrtPowerScaleDown7D7C64 : &ug2CrtPowerScaleUp7D7C6C'),
        'exit-underflow-compare':('exit','if (status&0x100)','if (!(status&0x100))'),
        'exit-overflow-compare':('exit','if (!(status&0x4100))','if (status&0x4100)'),
        'exit-one-arg':('exit','if (operation==0x1d)','if (operation==0x1c)'),
        'core-zero-zero':('x87','value.high=0x80000000;value.exponent=0x3fff','value.high=0;value.exponent=0'),
        'core-zero-sign':('x87','(parity&1)','(parity&2)'),
        'core-negative-parity':('x87','if (parity==1) UG2FxCrtExtendedNegate(&value)','if (parity==2) UG2FxCrtExtendedNegate(&value)'),
        'core-negative-domain':('x87','(base.high&0x80000000)&&!parity','(base.high&0x80000000)&&parity'),
        'core-bypass':('x87','quick||ug2CrtBypassMathExit8B8424','quick||!ug2CrtBypassMathExit8B8424'),
        'core-restore-environment':('x87','UG2FxCrtRestoreEnvironment(&environment);',''),
    }
    rejected=[]
    for name,(kind,before,after)in mutations.items():
        content=SOURCE.read_text();assert before in content,(name,before)
        # Target the loader's mask rather than the classifier's identical text.
        if name=='load-normal-sign':
            before='unsigned int exponent=value.high&0x7ff00000;\n    if (exponent!=0x7ff00000)';after='unsigned int exponent=value.high&0xfff00000;\n    if (exponent!=0x7ff00000)'
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        run=subprocess.run([sys.executable,__file__,'--cases','2048','--kind',kind,'--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'CRT power mismatch'in run.stdout,(name,run.returncode,run.stdout[-3000:]);rejected.append(name)
    return rejected


def native_mutation_checks():
    native=ROOT/'build/native';fixture=BUILD/'native-mutation-fixtures.bin';write_native_fixtures(fixture,8192)
    adapter=ROOT/'src/Platform/Native/NativeFxCrtPowerInstructions.cpp'
    mutations={
        'classifier-copy':(SOURCE,'double prepared;UG2_FX_VM_STORE_LOAD(prepared,Number(value));int type=D3DXCrtSpecialType(prepared);','int type=((value.high&0xfffff)||value.low) ? ((value.high&0x80000) ? 3 : 4) : ((value.high&0x80000000) ? 2 : 1);'),
        'parity-denormal-round':(SOURCE,'D3DXCrtFpClass(Bits(prepared))&0x90','D3DXCrtFpClass(Bits(prepared))&0'),
        'exp2-instruction':(adapter,'fldt %0; f2xm1; fstpt %0','fldt %0; fsin; fstpt %0'),
        'fraction-narrow':(SOURCE,'UG2FxCrtExtendedF2xm1(&difference);','double narrowed;UG2FxCrtExtendedToDouble(&narrowed,&difference);UG2FxCrtExtendedFromDouble(&difference,&narrowed);UG2FxCrtExtendedF2xm1(&difference);'),
        'environment-restore':(SOURCE,'UG2FxCrtRestoreEnvironment(&environment);',''),
        'second-input-copy':(SOURCE,'UG2_CRT_COPY_DOUBLE_BITS(error.input10,second);','UG2_FX_VM_STORE_LOAD(error.input10,Number(second));'),
        'exit-mask':(SOURCE,'!(saved&0x20)&&(D3DXCrtReadStatus()&0x20)','(saved&0x20)&&(D3DXCrtReadStatus()&0x20)'),
        'sse-arguments':(SOURCE,'UG2FxCrtPowerSseBody(&result,Bits(a),Bits(b))','UG2FxCrtPowerSseBody(&result,Bits(b),Bits(a))'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=unit.read_text();assert before in content,(name,before)
        path=BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1).replace('../../Speed/Indep/Libs/D3DX/CrtPower.h','CrtPower.h'));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB),'-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),'-c',str(path),'-o',str(obj)],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2crtpower_probe.dir/src/Platform/Native/CrtPowerProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),str(fixture)],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware CRT power'in run.stdout and 'mismatch'in run.stdout,(name,run.returncode,run.stdout[-2500:]);rejected.append(name)
    (BUILD/'verification-native-mutations.json').write_text(json.dumps(dict(rejected_mutations=rejected),indent=2)+'\n');return rejected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=8192);p.add_argument('--kind',choices=['all',*NAMES],default='all');p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--native-mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();m=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for row in[*m['functions'].values(),*m['data'].values(),m['unrecovered_sse_body']]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    if args.write_native_fixtures:write_native_fixtures(args.write_native_fixtures,args.cases)
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    for i,c in enumerate(cases(args.cases)):
        for kind in NAMES if args.kind=='all'else(args.kind,):
            try:x=a.run_power(kind,c);y=b.run_power(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('CRT power mismatch',kind,i,c,str(e)))from e
            assert x==y,('CRT power mismatch',kind,i,c,x,y)
            counts[kind]+=1
    report=dict(comparisons=dict(counts),paths=dict(a.paths),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',platform_boundary='SSE body 0x764749 controlled; OS RaiseException; thread data; default matherr observed')
    if not args.replace:report['composed']=composed(image,linked,args.gcc)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
