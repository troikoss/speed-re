#!/usr/bin/env python3
"""Compare UG2 floor's SSE/x87 paths, controls, classification and matherr chain."""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError,Uc,UC_ARCH_X86,UC_MODE_32
import unicorn.x86_const as xc
import verify_pc_effect_vm as vm

md=vm.md;ROOT,EXE,LIB=vm.ROOT,vm.EXE,vm.LIB;ARENA,STOP=vm.ARENA,vm.STOP
BUILD=ROOT/'build/analysis/crt-floor';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/crt-floor.json';md.BUILD=BUILD
SOURCES=dict(floor=LIB/'CrtFloor.cpp',error=LIB/'CrtMathError.cpp',callback=LIB/'CrtMathCallback.cpp')
SPANS=dict(floor=(0x75eb50,498),control=(0x7676a4,36),round=(0x76cc73,19),classify=(0x766ea9,91),errno=(0x7661ba,9),set_errno=(0x767485,40),default=(0x727e6b,3),user=(0x7674da,158),qnan=(0x767578,83),sse_error=(0x7694d0,660))
NAMES=dict(floor='D3DXCrtFloor',control='D3DXCrtRawControl',round='D3DXCrtRoundCurrent',classify='D3DXCrtSpecialType',errno='D3DXCrtErrno',set_errno='D3DXCrtSetMathErrno',default='D3DXCrtDefaultMathError',user='D3DXCrtUserMathError',qnan='D3DXCrtHandleQnan',sse_error='D3DXCrtSseMathError')
md.FREE.update(NAMES);md.CDECL.update(NAMES);md.VOID.add('sse_error')
DATA,THREAD,RECORD=ARENA+0x1000,ARENA+0x2000,ARENA+0x3000
CALLBACK=STOP+0xc000;RESULT_STUB=STOP+0xc100;RESULT=STOP+0xc200;RAISE=STOP+0xc300
FLOATING={'floor','round','user','qnan'}
ERROR_CODES=(2,3,8,9,14,15,24,25,26,27,28,29,161,162,166,170,171,1000,1001,1002,1003,1004,1005,1006,1007,0,-1,160,163,999,1008,0x7fffffff)
ERROR_CODES+=tuple(code for code in(20,29,26,27,31,19,33,14,13,15,16,5,30,18,32,12,11,21,28,25,17,24,22,23,34,35,36,37,38)if code not in ERROR_CODES)


def words(bits):return bits&0xffffffff,bits>>32


def cases(n):
    rng=random.Random(0x75eb50)
    fixed=(0,1,0x8000000000000000,0x8000000000000001,0x3fefffffffffffff,0xbfefffffffffffff,0x3ff0000000000000,0xbff0000000000000,0x3ff0000000000001,0xbff0000000000001,0x432fffffffffffff,0xc32fffffffffffff,0x4330000000000000,0xc330000000000000,0x7ff0000000000000,0xfff0000000000000,0x7ff8000000001234,0xfff8000000001234,0x7ff0000000001234,0xfff0000000001234,0x000fffffffffffff,0x800fffffffffffff,0x0010000000000000,0x8010000000000000)
    for i in range(n):
        bits=fixed[i%len(fixed)]if i<3072 else rng.getrandbits(64)
        control=(0x7f,0x27f,0x37f)[(i//24)%3]|((i//72)%4)*0x400
        if i%31==0:control&=~0x20
        if i%43==0:control|=0x8000
        yield dict(bits=bits,other=fixed[(i+7)%len(fixed)],result=fixed[(i+11)%len(fixed)],control=control,target_control=(rng.getrandbits(16)<<16)|(0x7f,0x27f,0x37f)[(i+1)%3]|((i+2)%4)*0x400,mxcsr=0x1f80|((i//13)%4)*0x2000|((i//17)%4 in(1,3))*0x40|((i//17)%4 in(2,3))*0x8000,
            sse=(i//len(fixed))%2,mode=(0,9876)[i%2],operation=ERROR_CODES[i%len(ERROR_CODES)],type=(-3,0,1,2,3,4,5,0x10000001)[(i//3)%8],
            raw_value=rng.getrandbits(32),raw_mask=(0,0xffff,0xffffffff,0x300,0xc00,0x8000,rng.getrandbits(32))[i%7],
            callback=i%3!=0,callback_result=i%2,callback_write=i%5==0,alias=(0,4,8,16,32)[i%5],seed=rng.getrandbits(32))


def native_cases(n):
    for i,c in enumerate(cases(n)):
        c=dict(c);c.update(control=(c['control']&0xf00)|0x7f,callback=False,seed=0x12345678)
        if i%11==0:c['mxcsr']&=~(1<<(7+i%6)) # Must choose x87 fallback.
        yield c


class Machine(vm.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.real_result=False;self.coverage=Counter()
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        self.u.hook_add(UC_HOOK_CODE,self.thread_entry,begin=0x761e3c,end=0x761e3c)
        self.u.hook_add(UC_HOOK_CODE,self.callback_entry,begin=CALLBACK,end=CALLBACK)
        self.u.hook_add(UC_HOOK_CODE,self.default_entry,begin=self.entries['default'],end=self.entries['default'])
        if linked:self.u.hook_add(UC_HOOK_CODE,self.default_entry,begin=0x727e6b,end=0x727e6b)
        self.u.hook_add(UC_HOOK_CODE,self.result_entry,begin=0x7675cb,end=0x7675cb)
        self.put(0x7832a4,RAISE);self.u.hook_add(UC_HOOK_CODE,self.raise_entry,begin=RAISE,end=RAISE)
        if not linked:
            for address,label in((0x75eb90,'sse'),(0x769764,'x87'),(0x75ebe9,'sse_nan'),(0x7697ea,'x87_round'),(0x769795,'classify_nonfinite')):
                self.u.hook_add(UC_HOOK_CODE,self.path_entry,user_data=label,begin=address,end=address)
        self.u.mem_write(RESULT_STUB,b'\xdd\x05'+struct.pack('<I',RESULT)+b'\xc3')
        if linked:
            for name,method in(('UG2FxCrtCallMathError',self.callback_bridge),('UG2FxCrtHandleResult',self.result_entry)):
                at=linked['exports'][name if gcc else'_'+name];self.u.hook_add(UC_HOOK_CODE,method,begin=at,end=at)
    def math_bridge(self,u,address,size,info):pass
    def math_entry(self,u,address,size,info):pass
    def fp_state(self):return self.u.reg_read(xc.UC_X86_REG_FPCW),self.u.reg_read(xc.UC_X86_REG_MXCSR),self.u.reg_read(xc.UC_X86_REG_FPSW)&0x3f
    def path_entry(self,u,address,size,label):self.coverage[label]+=1
    def thread_entry(self,u,address,size,user):self.events.append(('thread',self.fp_state()));self.ret(THREAD)
    def observe_callback(self,pointer,kind):
        self.events.append((kind,bytes(self.u.mem_read(pointer,32)).hex(),self.fp_state(),bytes(self.u.mem_read(DATA,48)).hex()))
        if kind=='custom'and self.case['callback_write']:
            self.u.mem_write(pointer+24,struct.pack('<d',-7.25));self.put(pointer,0x2468)
            self.u.reg_write(xc.UC_X86_REG_FPCW,(self.case['target_control']&0xff00)|0x7f)
    def callback_entry(self,u,address,size,user):self.observe_callback(self.args(1)[0],'custom');self.ret(self.case['callback_result'])
    def callback_bridge(self,u,address,size,user):
        token,pointer=self.args(2)
        if token==CALLBACK:self.observe_callback(pointer,'custom');self.ret(self.case['callback_result'])
        else:assert token==0x727e6b
    def default_entry(self,u,address,size,user):
        if self.kind!='default':self.observe_callback(self.args(1)[0],'default')
    def result_entry(self,u,address,size,user):
        if self.real_result and address!=0x7675cb:u.reg_write(xc.UC_X86_REG_EIP,0x7675cb);return
        self.events.append(('result',self.args(7),self.fp_state()))
        if self.real_result:return
        self.u.reg_write(xc.UC_X86_REG_FPSW,self.u.reg_read(xc.UC_X86_REG_FPSW)&~0x3f)
        self.u.reg_write(xc.UC_X86_REG_FPCW,self.args(7)[6]&0xffff)
        self.u.reg_write(xc.UC_X86_REG_EIP,RESULT_STUB)
    def raise_entry(self,u,address,size,user):
        code,flags,count,args=self.args(4);assert flags==0 and count==1
        record=self.word(args);fields=struct.unpack('<28I',u.mem_read(record,112))
        known=(fields[0]&0x1ffff,*fields[1:6],fields[8]&31,fields[16]&1,*fields[20:22],fields[24]&31)
        self.events.append(('raise',code,known,self.fp_state()))
        if self.case['callback_write']:
            self.put(record,(fields[0]&~31)|2|(1<<2));u.mem_write(record+80,struct.pack('<d',-6.5))
        self.ret(0,16)
    def call_crt(self,kind,args):
        u=self.u;stack=md.core.STACK+0xf000;self.put(stack,STOP,*args);regs=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(regs,saved):u.reg_write(r,v)
        for r,v in((xc.UC_X86_REG_ESP,stack),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,self.case['control']),(xc.UC_X86_REG_MXCSR,self.case['mxcsr']),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)):u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=200000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        assert u.reg_read(xc.UC_X86_REG_ESP)==stack+4+(56 if kind=='vm'else 0),('stack',kind)
        assert tuple(u.reg_read(r)for r in regs)==saved,('registers',kind)
        top=(u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        live=tuple((j,u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if(tag>>(j*2)&3)!=3)
        result=None if kind in FLOATING or kind=='sse_error'else u.reg_read(xc.UC_X86_REG_EAX)
        return result,self.fp_state(),top,tag,live
    def run_crt(self,kind,c):
        self.kind=kind;self.case=c;self.events=[];self.put(0,0x12345678);self.u.mem_write(DATA,b'\xa5'*128);self.u.mem_write(THREAD,b'\x5a'*32);self.put(THREAD+8,c['seed']);self.u.mem_write(RECORD,b'\x77'*32)
        self.put(0x8ba480,c['sse']);self.put(0x81ace0,c['mode']);self.put(0x81aa5c,CALLBACK if c['callback']else 0x727e6b)
        self.u.mem_write(DATA,struct.pack('<Q',c['bits']));self.u.mem_write(DATA+16,struct.pack('<Q',c['other']));self.u.mem_write(DATA+32,struct.pack('<Q',c['result']));self.u.mem_write(RESULT,struct.pack('<d',-3.75))
        value=words(c['bits']);result=words(c['result'])
        arguments=dict(floor=value,round=value,classify=value,control=(c['raw_value'],c['raw_mask']),errno=(),set_errno=(c['type'],),default=(RECORD,),user=(c['type'],c['operation']&0xffffffff,*value,*words(c['other']),*result,c['target_control']),qnan=(c['operation']&0xffffffff,*value,c['target_control']),sse_error=(DATA,DATA+16,DATA+c['alias'],c['operation']&0xffffffff))
        state=self.call_crt(kind,arguments[kind]);return state,bytes(self.u.mem_read(DATA,128)),bytes(self.u.mem_read(THREAD,32)),self.events


def emulator_audit():
    rows=[]
    for mxcsr in(0x1f80,0x1fc0,0x9f80,0x9fc0):
        for bits in(0x800fffffffffffff,0x7ff0000000001234):
            u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(0x1000,0x2000)
            u.mem_write(0x1000,b'\xdd\x05\x00\x20\x00\x00\xdd\x1d\x08\x20\x00\x00');u.mem_write(0x2000,struct.pack('<Q',bits))
            for r,v in((xc.UC_X86_REG_FPCW,0x37f),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff),(xc.UC_X86_REG_MXCSR,mxcsr)):u.reg_write(r,v)
            u.emu_start(0x1000,0x100c);observed=struct.unpack('<Q',u.mem_read(0x2008,8))[0];expected=bits|0x8000000000000 if bits==0x7ff0000000001234 else bits
            rows.append(dict(mxcsr=hex(mxcsr),input=hex(bits),expected=hex(expected),observed=hex(observed),expected_flags=1 if bits==0x7ff0000000001234 else 2,flags=u.reg_read(xc.UC_X86_REG_FPSW)&0x3f))
    return rows


def native_replay(image,path):
    raw=path.read_bytes();assert raw[:8]==b'UG2FXFD1';count,=struct.unpack_from('<I',raw,8);at=12;matched=skipped=0;m=Machine(image)
    for i,c in enumerate(native_cases(count)):
        header=list(struct.unpack_from('<6I',raw,at));value=raw[at+24:at+34];at+=34
        assert header[0]==i
        snan=(c['bits']&0x7ff0000000000000)==0x7ff0000000000000 and(c['bits']&0xfffffffffffff)!=0 and not(c['bits']&0x8000000000000)
        if c['mxcsr']&0x8040 or snan:skipped+=1;continue
        x=m.run_crt('floor',c);state=x[0];control,mxcsr,flags=state[1];top,tag,live=state[2:];assert len(live)==1
        expected=[i,control,mxcsr&~0x3f,(top+1)&7,0xffff,struct.unpack_from('<I',x[2],8)[0]];header[2]&=~0x3f
        assert header==expected and value==struct.pack('<QH',*live[0][1]),('native CRT floor mismatch',i,c,expected,header,value.hex(),live)
        matched+=1
    assert at==len(raw);return dict(matched=matched,excluded_emulator_limitations=skipped,hardware_cases=count)


def mutation_checks(gcc):
    mutations={
        'cpu-gate':('floor','ug2CrtSseEnabled8BA480 &&','true &&'),
        'mxcsr-gate':('floor','&0x1f80)==0x1f80','&0x1f80)==0'),
        'x87-gate':('floor','&0x7f)==0x7f','&0x7f)==0x3f'),
        'positive-unit-boundary':('floor','exponent<0x3ff','exponent<=0x3ff'),
        'negative-unit-boundary':('floor','exponent<0xbff','exponent<=0xbff'),
        'positive-large-boundary':('floor','exponent<=0x432','exponent<=0x431'),
        'negative-large-boundary':('floor','exponent<=0xc32','exponent<=0xc31'),
        'shift-count':('floor','[0]-(exponent&','[0]+1-(exponent&'),
        'negative-comparison':('floor','unsigned int less=UG2FxCrtSseLess(&bits,&truncated);','unsigned int less=0;'),
        'negative-small-comparison':('floor','less=UG2FxCrtSseLess(&bits,&ug2CrtFloorNegativeZero7D3CC0);','less=UG2FxCrtSseLess(&bits,&ug2CrtFloorNegativeOne7D3CB0);'),
        'adjustment-mask':('floor','CrtFloorBits adjustment=(less ?','CrtFloorBits adjustment=(!less ?'),
        'sse-error-operation':('floor','&value,&value,&value,0x3ed','&value,&value,&value,0x3ec'),
        'fallback-rounding':('floor','D3DXCrtRawControl(ug2CrtFloorControl81AA60,0xffff)','D3DXCrtRawControl(0x37f,0xffff)'),
        'restore-precision':('floor','D3DXCrtRawControl(saved,0xffff)','D3DXCrtRawControl(saved,0xfcff)'),
        'precision-mask-gate':('floor','saved&0x20','saved&0x10'),
        'general-error-flags':('floor','UG2FxCrtHandleResult(0x10,','UG2FxCrtHandleResult(8,'),
        'control-sign':('floor','return static_cast<short>(previous);','return previous;'),
        'control-merge':('floor','previous&~mask','previous&mask'),
        'rounding-instruction':('floor','UG2FxCrtRoundStore(&result,&value)','UG2_FX_VM_STORE_LOAD(result,value)'),
        'quiet-classification':('floor','(high>>16)&0x7ff8','(high>>16)&0x7ff0'),
        'infinity-sign':('floor','high==0xfff00000','high==0x7ff00000'),
        'errno-offset':('error','UG2FxCrtThreadData())+8','UG2FxCrtThreadData())+4'),
        'errno-domain':('error','*value=0x21','*value=0x22'),
        'errno-range':('error','*value=0x22','*value=0x21'),
        'errno-return':('error','return reinterpret_cast<int>(value);','return type;'),
        'name-table-bound':('error','i<29','i<28'),
        'word-payload-copy':('error','UG2_CRT_COPY_DOUBLE_BITS(error.input08,first)','UG2_FX_VM_STORE_LOAD(error.input08,*reinterpret_cast<const double *>(&first))'),
        'payload-order':('error','UG2_CRT_COPY_DOUBLE_BITS(error.input10,second)','UG2_CRT_COPY_DOUBLE_BITS(error.input10,first)'),
        'direct-callback':('error','D3DXCrtDefaultMathError(&error)','UG2FxCrtCallMathError(ug2CrtMathErrorCallback81AA5C,&error)'),
        'qnan-mode':('error','if (!ug2CrtMathErrorFlag81ACE0)','if (ug2CrtMathErrorFlag81ACE0)'),
        'qnan-errno':('error','*D3DXCrtErrno()=0x21','*D3DXCrtErrno()=0x22'),
        'sse-162-range':('error','case 162:type=4;name=reinterpret_cast<const char *>(0x7b3b98);errnoValue=0x22;','case 162:type=4;name=reinterpret_cast<const char *>(0x7b3b98);'),
        'sse-alias-copy':('error','if (copyFirst) UG2_FX_VM_STORE_LOAD(*output,*first);','if (false) UG2_FX_VM_STORE_LOAD(*output,*first);'),
        'sse-record-second':('error','UG2_FX_VM_STORE_LOAD(error.input10,*second)','UG2_FX_VM_STORE_LOAD(error.input10,*first)'),
        'sse-callback-gate':('error','if (!UG2FxCrtCallMathError','if (UG2FxCrtCallMathError'),
        'sse-publish':('error','UG2_FX_VM_STORE_LOAD(*output,error.result18);','UG2_FX_VM_STORE_LOAD(*output,error.input08);'),
        'sse-one':('error','case 26:*output=1.0','case 26:*output=0.0'),
        'default-return':('callback','return 0;','return 1;'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=SOURCES[unit].read_text();assert before in content,(name,before)
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        run=subprocess.run([sys.executable,__file__,'--cases','512','--replace',str(path),'--unit',unit]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'CRT floor mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
    return rejected


def native_mutation_checks():
    native=ROOT/'build/native';fixture=BUILD/'native-mutation-fixtures.bin'
    fixture.write_bytes(b'UG2FXFI1'+struct.pack('<I',1024)+b''.join(struct.pack('<7I',i,c['control'],c['mxcsr'],c['sse'],c['mode'],*words(c['bits']))for i,c in enumerate(native_cases(1024))))
    mutations={
        'large-negative-comparison':(SOURCES['floor'],'unsigned int less=UG2FxCrtSseLess(&bits,&truncated);','unsigned int less=exponent>0xc32 ? 0 : UG2FxCrtSseLess(&bits,&truncated);'),
        'daz-small-negative':(SOURCES['floor'],'less=UG2FxCrtSseLess(&bits,&ug2CrtFloorNegativeZero7D3CC0);','less=bits!=*reinterpret_cast<const CrtFloorBits *>(&ug2CrtFloorNegativeZero7D3CC0) ? ~0u : 0;'),
        'rounding':(SOURCES['floor'],'UG2FxCrtRoundStore(&result,&value)','UG2_FX_VM_STORE_LOAD(result,value)'),
        'control-merge':(SOURCES['floor'],'previous&~mask','previous&mask'),
        'word-payload-copy':(SOURCES['error'],'UG2_CRT_COPY_DOUBLE_BITS(error.input08,first)','UG2_FX_VM_STORE_LOAD(error.input08,*reinterpret_cast<const double *>(&first))'),
        'qnan-errno':(SOURCES['error'],'*D3DXCrtErrno()=0x21','*D3DXCrtErrno()=0x22'),
        'vm-argument-copy':(LIB/'EffectVm.cpp','double argument;UG2_FX_VM_STORE_LOAD(argument,inputs[0][lane]);','double argument=inputs[0][lane];'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=unit.read_text();assert before in content
        path=BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB),'-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),'-c',str(path),'-o',str(obj)],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2crtfloor_probe.dir/src/Platform/Native/CrtFloorProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),str(fixture),str(BUILD/'native-mutation-capture.bin')],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware'in run.stdout and 'mismatch'in run.stdout,(name,run.stdout[-2500:]);rejected.append(name)
    return rejected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=2048);p.add_argument('--replace',type=Path);p.add_argument('--unit',choices=SOURCES,default='floor');p.add_argument('--kind',choices=['all',*SPANS],default='all');p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path);p.add_argument('--real-result-runtime',action='store_true');p.add_argument('--mutation-checks',action='store_true');p.add_argument('--native-mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in[*manifest['functions'].values(),*manifest['data'].values()]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    linked=md.build({SOURCES[args.unit].name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    a.real_result=b.real_result=args.real_result_runtime
    for i,c in enumerate(cases(args.cases)):
        for kind in SPANS if args.kind=='all'else(args.kind,):
            try:x=a.run_crt(kind,c);y=b.run_crt(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('CRT floor mismatch',kind,i,str(e)))from e
            assert x==y,('CRT floor mismatch',kind,i,c,x,y)
            counts[kind]+=1
    report=dict(comparisons=dict(counts),floor_paths=dict(a.coverage),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',result_runtime='original code with controlled OS RaiseException'if args.real_result_runtime else'controlled boundary')
    if not args.replace:
        for i,c in enumerate(cases(512)):
            results=[]
            for machine in(a,b):
                machine.run_crt('classify',c);machine.kind='vm';machine.events=[]
                machine.put(ARENA+0x4000,1,0x10400001,1,0,3,0,0,4,0);machine.put(ARENA+0x5000,1,0,0)
                arguments=[ARENA+0x4000,ARENA+0x5000,DATA,0,0,DATA+64,0,0,0,0,0,0,0,0]
                state=machine.call_crt('vm',arguments);results.append((state,bytes(machine.u.mem_read(DATA,128)),bytes(machine.u.mem_read(THREAD,32)),machine.events))
            assert results[0]==results[1],('CRT floor mismatch','VM fraction chain',i,c,results)
        report['vm_fraction_chains']=512
    if args.write_native_fixtures:
        content=b'UG2FXFI1'+struct.pack('<I',args.cases)
        for i,c in enumerate(native_cases(args.cases)):content+=struct.pack('<7I',i,c['control'],c['mxcsr'],c['sse'],c['mode'],*words(c['bits']))
        args.write_native_fixtures.write_bytes(content)
    if args.native_dump:report['native_replay']=native_replay(image,args.native_dump)
    report['unicorn_x87_copy_audit']=emulator_audit()
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+('-real'if args.real_result_runtime else'')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
