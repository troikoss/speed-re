#!/usr/bin/env python3
"""Compare UG2's shared FP result, mask fixups and exception-record runtime.

Only CRT thread data, matherr callbacks and the actual OS RaiseException service
are controlled. Both recovered compiler builds execute every recovered helper.
Unicorn's known x87 flag/NaN/FTZ limitations are reported separately; native
instruction oracles cover hardware behavior without executing the retail PE.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_crt_floor as fl

md=fl.md;ROOT,EXE,LIB=fl.ROOT,fl.EXE,fl.LIB;ARENA,STOP=fl.ARENA,fl.STOP
BUILD=ROOT/'build/analysis/crt-exceptions';md.BUILD=BUILD
SOURCE=LIB/'CrtExceptions.cpp';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/crt-exceptions.json'
SPANS=dict(result=(0x7675cb,190),masks=(0x767263,546),raise_record=(0x766fbe,677),decompose=(0x766f04,186),exponent=(0x766e7f,42),error_code=(0x7674ad,45),status=(0x7676c8,89),read_status=(0x767689,13),clear_status=(0x767696,14))
NAMES=dict(result='D3DXCrtHandleResult',masks='D3DXCrtApplyMasks',raise_record='D3DXCrtRaiseException',decompose='D3DXCrtDecompose',exponent='D3DXCrtSetExponent',error_code='D3DXCrtErrorCode',status='D3DXCrtSetStatus',read_status='D3DXCrtReadStatus',clear_status='D3DXCrtClearStatus')
md.FREE.update(NAMES);md.CDECL.update(NAMES)
FLOATING={'result','decompose','exponent'};VOID={'status','raise_record'}
DATA,THREAD,RECORD=fl.DATA,fl.THREAD,fl.RECORD;ALTERNATE=ARENA+0x3800
FIXED=(0,0x8000000000000000,1,0x8000000000000001,0x000fffffffffffff,0x800fffffffffffff,0x0010000000000000,0x8010000000000000,0x3fefffffffffffff,0xbfefffffffffffff,0x3ff0000000000000,0xbff0000000000000,0x7fefffffffffffff,0xffefffffffffffff,0x7ff0000000000000,0xfff0000000000000,0x7ff8000000001234,0xfff8000000001234,0x7ff0000000001234,0xfff0000000001234,0x5fe0000000000000,0xdfe0000000000000,0x5fdfffffffffffff,0x5ca0000000000001,0x5cafffffffffffff,0x5cb0000000000000,0x5cc0000000000000,0x5cd0000000000000,0xdcd0000000000001,0x5fd0000000000001,0x5fefffffffffffff,0x5ff0000000000000)


def cases(n):
    rng=random.Random(0x7675cb)
    for i in range(n):
        c=dict(next(iter(fl.cases(1))))
        # Enumerate every low flag combination and saved mask independently of
        # the current hardware word. Include all four PC encodings in records.
        c.update(bits=FIXED[i%len(FIXED)]if i<3840 else rng.getrandbits(64),other=FIXED[(i*7+3)%len(FIXED)],result=FIXED[(i*11+7)%len(FIXED)],
            flags=(i//len(FIXED))%64 | (rng.getrandbits(26)<<6 if i%3==0 else 0),
            saved=(i%64)|((i//64)%4)*0x100|((i//256)%4)*0x400|(rng.getrandbits(16)<<16),
            control=(0x7f,0x27f,0x37f)[i%3]|((i//3)%4)*0x400,mxcsr=0x1f80|((i//7)%4)*0x2000,
            status=(0,0x3f,1,2,4,8,0x10,0x20,0x4700,0x8000,0xc73f)[i%11],exponent=(-2147483648,-1075,-1074,-1022,-1021,-1,0,1,1024,0x7fffffff)[(i//3)%10],
            mode=(0,9876)[i%2],operation=(11,0xffffffff,0,0x12345678,38)[i%5],callback=i%3!=0,callback_result=i%2,callback_write=i%5!=0,
            redirect=i%7==0,record_bits=rng.getrandbits(32),record_masks=rng.getrandbits(32),seed=rng.getrandbits(32),
            alias=(0,4,8,12,16,20,32,64,80,96,100,124)[(i//9)%12]if i%4==0 else -1,
            input_alias=(4,8,16,20,32,80,96,100)[(i//7)%8]if i%7==0 else -1,output_alias=(4,8,16,20,32,80,96,100)[(i//11)%8]if i%11==0 else -1,
            live=i%3,top=(i//11)%8,target_control=0x37f)
        yield c


def native_cases(n):
    for i,c in enumerate(cases(n)):
        c=dict(c);c.update(record_masks=0,mxcsr=c['mxcsr']|((i//17)%4 in(1,3))*0x40|((i//17)%4 in(2,3))*0x8000)
        # Keep actual hardware masked, including after handler return. The OS
        # service is a controlled native fixture, not Linux SEH implementation.
        yield c


def write_native_fixtures(path,n):
    raw=b'UG2FXEI1'+struct.pack('<I',n)
    for i,c in enumerate(native_cases(n)):
        raw+=struct.pack('<15I3Q',i,c['flags'],c['saved'],c['control'],c['mxcsr'],c['status'],c['exponent']&0xffffffff,c['mode'],c['operation']&0xffffffff,int(c['callback_write']),int(c['redirect']),c['record_bits'],c['record_masks'],c['seed'],c['alias']&0xffffffff,c['bits'],c['other'],c['result'])
    path.write_bytes(raw)


class Machine(fl.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        if linked:
            symbol='UG2FxCrtOsRaiseException'if gcc else'_UG2FxCrtOsRaiseException@16'
            at=linked['exports'][symbol];self.u.hook_add(UC_HOOK_CODE,self.raise_entry,begin=at,end=at)
    def result_entry(self,u,address,size,user):pass # Execute recovered body/bridge.
    def known_record(self,p):
        w=struct.unpack('<32I',self.u.mem_read(p,128))
        return(w[0]&0x1ffff,*w[1:6],w[8]&31,w[16]&1,*w[20:22],w[24]&31)
    def raise_entry(self,u,address,size,user):
        code,flags,count,args=self.args(4);assert flags==0 and count==1
        record=self.word(args)
        payload=bytes(u.mem_read(record,128)).hex()if self.kind=='raise_record'else self.known_record(record)
        self.events.append(('raise',code,payload,self.fp_state()))
        if self.kind!='raise_record':
            # Local padding is uninitialized in the original. Compare all defined
            # fields first, then initialize padding so callback edits are equal.
            w=list(struct.unpack('<28I',u.mem_read(record,112)));clean=[0xa5a5a5a5]*28
            for j in(1,2,3,4,5,20,21):clean[j]=w[j]
            clean[0]=w[0]&0x1ffff
            for j in(8,24):clean[j]=w[j]&31
            clean[16]=w[16]&1;self.put(record,*clean)
        if self.case['callback_write']:
            if self.case['redirect']:self.put(args,ALTERNATE);record=ALTERNATE
            self.put(record,self.case['record_bits']);self.put(record+8,self.case['record_masks'])
            u.mem_write(record+80,struct.pack('<Q',self.case['other']))
        self.ret(0,16)
    def call_exception(self,kind,args):
        u=self.u;stack=md.core.STACK+0xf000;self.put(stack,STOP,*args)
        regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        c=self.case;top=c['top'];tag=0xffff
        for j in range(c['live']):
            p=(top+j)%8;u.reg_write(xc.UC_X86_REG_FP0+p,(0xa000000000000000,0x4000+j));tag&=~(3<<(p*2))
        for r,v in((xc.UC_X86_REG_ESP,stack),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,c['control']),(xc.UC_X86_REG_MXCSR,c['mxcsr']),(xc.UC_X86_REG_FPSW,(top<<11)|c['status']),(xc.UC_X86_REG_FPTAG,tag)):u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=200000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        assert u.reg_read(xc.UC_X86_REG_ESP)==stack+4,('stack',kind)
        assert tuple(u.reg_read(r)for r in regs)==saved,('registers',kind)
        top=(u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        live=tuple((j,u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if(tag>>(j*2)&3)!=3)
        value=None if kind in FLOATING|VOID else u.reg_read(xc.UC_X86_REG_EAX)
        return(value,self.fp_state(),top,tag,live)
    def run_exception(self,kind,c):
        self.kind=kind;self.case=c;self.events=[];u=self.u
        self.put(0,0x12345678);u.mem_write(DATA,b'\xa5'*128);u.mem_write(THREAD,b'\x5a'*32);self.put(THREAD+8,c['seed'])
        self.put(0x81ace0,c['mode']);self.put(0x81aa5c,fl.CALLBACK if c['callback']else 0x727e6b)
        u.mem_write(DATA,struct.pack('<Q',c['bits']));u.mem_write(DATA+16,struct.pack('<Q',c['other']));u.mem_write(DATA+32,struct.pack('<Q',c['result']));self.put(DATA+64,c['saved'])
        rng=random.Random(c['seed']);self.put(RECORD,*(rng.getrandbits(32)for _ in range(32)));self.put(ALTERNATE,*(rng.getrandbits(32)for _ in range(32)))
        control=RECORD+c['alias']if c['alias']>=0 else DATA+64
        # In caller-local records every defined low field must have been set.
        # Reserved PC encoding retains unknown precision bits; do not claim it
        # for that caller. Standalone initialized-record tests do cover it.
        saved=c['saved'];saved=saved&~0x300 if(saved&0x300)==0x100 else saved
        args=dict(result=(c['flags'],c['operation'],*fl.words(c['bits']),*fl.words(c['result']),saved),masks=(c['flags'],DATA,c['saved']),raise_record=(RECORD,control,c['flags'],c['operation'],RECORD+c['input_alias']if c['input_alias']>=0 else DATA,RECORD+c['output_alias']if c['output_alias']>=0 else DATA+32),decompose=(*fl.words(c['bits']),DATA+80),exponent=(*fl.words(c['bits']),c['exponent']),error_code=(c['flags'],),status=(c['flags'],),read_status=(),clear_status=())
        state=self.call_exception(kind,args[kind]);return(state,bytes(u.mem_read(DATA,128)).hex(),bytes(u.mem_read(THREAD,32)).hex(),bytes(u.mem_read(RECORD,128)).hex(),bytes(u.mem_read(ALTERNATE,128)).hex(),self.events)


def composed(image,linked,gcc):
    machines=Machine(image),Machine(image,linked,gcc);counts=Counter()
    for i,(base,c)in enumerate(zip(cases(2048),fl.cases(2048))):
        c=dict(base,**c);observed=[]
        for m in machines:
            m.put(ALTERNATE,*([0x12345678]*32));observed.append(m.run_crt('floor',c))
        assert observed[0]==observed[1],('CRT exception mismatch','composed floor',i,c,observed)
        counts['floor']+=1
        if i>=512:continue
        observed=[]
        for m in machines:
            m.run_crt('classify',c);m.kind='vm';m.events=[];m.put(ALTERNATE,*([0x12345678]*32))
            m.put(ARENA+0x4000,1,0x10400001,1,0,3,0,0,4,0);m.put(ARENA+0x5000,1,0,0)
            state=m.call_crt('vm',[ARENA+0x4000,ARENA+0x5000,DATA,0,0,DATA+64,0,0,0,0,0,0,0,0])
            observed.append((state,bytes(m.u.mem_read(DATA,128)),bytes(m.u.mem_read(THREAD,32)),m.events))
        assert observed[0]==observed[1],('CRT exception mismatch','composed VM fraction',i,c,observed)
        counts['vm_fraction']+=1
    return dict(counts)


def mutation_checks(gcc):
    mutations={
        'invalid-mask':('(control&1)', '(control&2)'),
        'primary-priority':('else if ((flags&4)', 'if ((flags&4)'),
        'precision-mask':('(flags&0x10)&&(control&0x20)', '(flags&0x10)&&(control&0x10)'),
        'remaining-high-flags':('remaining=flags&0x1f','remaining=flags'),
        'overflow-round-down':('rounding==0x400||rounding==0xc00','rounding==0x800||rounding==0xc00'),
        'underflow-scale':('exponent-=0x600','exponent-=0x5ff'),
        'underflow-zero-boundary':('exponent < -0x432','exponent < -0x430'),
        'underflow-carry':('(payload.high<<31)','(payload.high<<30)'),
        'negative-zero':('*output=0;return 0.0;','*output=0;return number;'),
        'decompose-exponent':('exponent=-1021','exponent=-1022'),
        'decompose-nonfinite':('if (!(value.high&0x7ff00000)', 'if ((value.high&0x7ff00000)==0x7ff00000'),
        'set-exponent-bias':('static_cast<unsigned int>(exponent)+0x3fe','static_cast<unsigned int>(exponent)+0x3ff'),
        'set-exponent-sign':('(value.high>>16)&0x800f','(value.high>>16)&0xf'),
        'status-sign':('static_cast<short>(UG2FxCrtReadStatusWait())','static_cast<unsigned short>(UG2FxCrtReadStatusWait())'),
        'error-priority':('if (flags&0x20) return 5;','if (flags&0x20) return 4;'),
        'exception-code':('code=0xc0000090','code=0xc000008f'),
        'record-mask':('~(*control<<4)','(*control<<4)'),
        'record-status':('if (status&4) record[3]|=8','if (status&4) record[3]|=4'),
        'record-rounding':('record[0]=(record[0]&~1u)|2','record[0]=(record[0]&~1u)|1'),
        'record-operation':('operation<<5','operation<<4'),
        'record-input':('record+4),*input','record+4),*result'),
        'record-result':('record+20),*result','record+20),*input'),
        'record-no-clear':('D3DXCrtClearStatus();UG2FxCrtOsRaiseException','D3DXCrtReadStatus();UG2FxCrtOsRaiseException'),
        'decode-original-mask':('(*control&~0xc00u)|0x300','(*control&~0x300u)|0x300'),
        'callback-gate':('if (!ug2CrtMathErrorFlag81ACE0 && error)','if (ug2CrtMathErrorFlag81ACE0 && error)'),
        'caller-zero':('record[16]&=~1u','record[16]|=1u'),
        'caller-result':('return *value;','return *reinterpret_cast<double *>(&first);'),
    }
    rejected=[]
    for name,(before,after)in mutations.items():
        content=SOURCE.read_text();assert before in content,(name,before)
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        target='masks'if name.startswith(('invalid-','primary-','precision-','remaining-','overflow-','underflow-'))else'decompose'if name.startswith(('negative-zero','decompose-'))else'exponent'if name.startswith('set-exponent-')else'error_code'if name=='error-priority'else'read_status'if name=='status-sign'else'result'if name.startswith(('callback-','caller-'))else'raise_record'
        run=subprocess.run([sys.executable,__file__,'--cases','2048','--kind',target,'--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'CRT exception mismatch'in run.stdout,(name,run.returncode,run.stdout[-3500:]);rejected.append(name)
    return rejected


def native_mutation_checks():
    native=ROOT/'build/native';fixture=BUILD/'native-mutation-fixtures.bin';write_native_fixtures(fixture,8192)
    instruction=ROOT/'src/Platform/Native/NativeFxCrtInstructions.cpp'
    mutations={
        'status-invalid':(SOURCE,'if (flags&1) UG2FxCrtRaiseInvalid();','if (false) UG2FxCrtRaiseInvalid();'),
        'status-overflow':(SOURCE,'if (flags&8) UG2FxCrtRaiseOverflow();','if (flags&8) UG2FxCrtRaiseUnderflow();'),
        'underflow-sticky':(SOURCE,'if (payload.low&1) sticky=true;','if (false) sticky=true;'),
        'set-exponent-load':(SOURCE,'UG2_FX_VM_STORE_LOAD(copy,*reinterpret_cast<const double *>(&value));','UG2_CRT_COPY_DOUBLE_BITS(copy,value);'),
        'unused-input-load':(SOURCE,'double *value=reinterpret_cast<double *>(&result);','double discard;UG2_FX_VM_STORE_LOAD(discard,*reinterpret_cast<double *>(&first));double *value=reinterpret_cast<double *>(&result);'),
        'divide-direction':(instruction,'fldz; fld1; fdivp;','fldz; fld1; fdivrp;'),
        'precision-instruction':(instruction,'fldpi; fstpl %0; fwait','fld1; fstpl %0; fwait'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=unit.read_text();assert before in content,(name,before)
        path=BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1).replace('../../Speed/Indep/Libs/D3DX/CrtExceptions.h','CrtExceptions.h'));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB),'-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),'-c',str(path),'-o',str(obj)],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2crtexceptions_probe.dir/src/Platform/Native/CrtExceptionProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),str(fixture)],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware'in run.stdout and 'mismatch'in run.stdout,(name,run.stdout[-2500:]);rejected.append(name)
    (BUILD/'verification-native-mutations.json').write_text(json.dumps(dict(rejected_mutations=rejected),indent=2)+'\n');return rejected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=8192);p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in[*manifest['functions'].values(),*manifest['data'].values()]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    if args.write_native_fixtures:write_native_fixtures(args.write_native_fixtures,args.cases)
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    for i,c in enumerate(cases(args.cases)):
        for kind in SPANS if args.kind=='all'else(args.kind,):
            try:x=a.run_exception(kind,c);y=b.run_exception(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('CRT exception mismatch',kind,i,c,str(e)))from e
            assert x==y,('CRT exception mismatch',kind,i,c,x,y)
            counts[kind]+=1
    report=dict(comparisons=dict(counts),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',platform_boundary='controlled OS RaiseException; thread data; matherr callbacks',emulator_limitations=fl.emulator_audit())
    if not args.replace:report['composed']=composed(image,linked,args.gcc)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
