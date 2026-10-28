#!/usr/bin/env python3
"""Compare the complete UG2 effect VM dispatcher and operation-502 wrapper.

Inline x87 operations and compiler integer conversion execute. CRT/operation
calls use explicitly controlled results or bounded original runtime execution;
no host libm algorithm replaces the original. Native runtime seams stay explicit.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_effect_evaluation as ev

md=ev.md;ROOT,EXE,LIB=ev.ROOT,ev.EXE,ev.LIB;ARENA,HEAP,STOP=ev.ARENA,ev.HEAP,ev.STOP
SOURCE=LIB/'EffectVm.cpp';BUILD=ROOT/'build/analysis/effect-vm';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-vm.json'
CODE,LITERALS,DATA=ARENA+0x1000,ARENA+0x3000,ARENA+0x5000
SPANS=dict(vm=(0x688e79,2324),operation502=(0x688ce2,74));md.BUILD=BUILD;md.FREE.update(vm='D3DXEffectVm',operation502='D3DXEffectVmOperation502');md.STDCALL.update(vm=56,operation502=12);md.VOID.add('operation502')
MATH=dict(floor=(0x75eb50,'UG2FxVmFloor',0),power=(0x75c9b0,'UG2FxVmPower',2),asin=(0x75eea0,'UG2FxVmAsin',1),acos=(0x75db80,'UG2FxVmAcos',1),operation=(0x63f2b5,'UG2FxVmOperation63F2B5',0))
RESULT=STOP+0xf800;STUB=STOP+0xb000
OPCODES=(0x10000000,0x10100000,0x10300000,0x10400000,0x10500000,0x10600000,0x10700000,0x10800000,0x10900000,0x10a00000,0x10b00000,0x10c00000,0x20000000,0x20100000,0x20200000,0x20300000,0x20400000,0x20500000,0x20600000,0x30000000,0x50000000,0x50200000,0x70e00000,0x10200000)


def pack(words):return ev.pack(words)


def instruction(opcode,width,operands,output):
    words=[opcode|width,len(operands)]
    for terms,kind,index in operands:words.extend([len(terms),*sum(([a,b]for a,b in terms),[]),kind,index])
    return words+[0,*output]


def cases(n):
    rng=random.Random(0x688e79)
    for i in range(n):
        op=OPCODES[i%len(OPCODES)];width=(0,1,2,3,4,7,8,16)[(i//len(OPCODES))%8];broadcast=bool((i//(len(OPCODES)*8))%2)
        c=dict(opcode=op,width=width,broadcast=broadcast,input_kind=(1,2,3,4,5,6,7,9,8)[(i//7)%9],other_kind=(2,3,4,5,6,7,1,9)[(i//11)%8],output_kind=(4,5,6,7,99)[(i//13)%5],
            input_index=rng.getrandbits(32),output_index=rng.getrandbits(32),mask=(0,1,3,7,15,255)[(i//5)%6],literal_count=(3,5,7,13,17,256)[(i//17)%6],
            precision=(0x7f,0x27f,0x37f)[(i//3)%3],rounding=(0,0x400,0x800,0xc00)[(i//19)%4],terms=(i//23)%4,
            data_kind=(i//29)%4,alias=(0,4,-4,8,0x1000)[(i//31)%5],controlled=True,math_result=(0.5,-1.25,2.0,3.5)[(i//37)%4],math_write=i%41==0,seed=rng.getrandbits(32),empty=i%97==0)
        if 24<=i<48 or 216<=i<240:
            c.update(width=1 if i<48 else 3,data_kind=0,input_kind=3,other_kind=2,output_kind=4,input_index=0,output_index=0,mask=255,terms=0,alias=0,math_write=False,empty=False)
        if i==3:c.update(width=1,data_kind=0,input_kind=3,input_index=0,output_kind=4,output_index=0,mask=255,terms=0,alias=0,math_write=True,empty=False)
        if c['empty']:c['literal_count']=0
        yield c


def program(c):
    # Seed both scratch values and every pointer slot before stale-pointer and
    # scratch tests. The final drain makes initialized scratch state observable.
    rows=[instruction(0x10000000,256,[([],1,j)for j in range(256)],(7,0))]
    opcode=c['opcode'];count=1
    if opcode in(0x20000000,0x20100000,0x20200000,0x20300000,0x20400000,0x20500000,0x20600000,0x50000000):count=2
    elif opcode==0x30000000:count=3
    elif opcode==0x70e00000:count=1+c['width']%9
    elif opcode==0x10600000:count=max(1,c['width'])
    operands=[]
    for j in range(count):
        terms=[((1,2,3,4,5,6,7,9,8)[(c['seed']+k+j)%9],(c['input_index']+k)%19)for k in range(c['terms'])]
        kind=c['input_kind']if j==0 else c['other_kind'];index=c['input_index']+j
        if kind==7 and c['width']>1:
            if terms and c['width']<=4:index=0
            else:terms=[];index=index%(257-c['width'])
        if opcode==0x10600000:operands.append(([],3 if j==0 else 2,0 if j==0 else j))
        else:operands.append((terms,kind,index))
    output_index=c['output_index']%(257-max(1,c['width']))if c['output_kind']==7 else c['output_index']
    rows.append(instruction(opcode|(0x80000000 if c['broadcast']else 0),c['width'],operands,(c['output_kind'],output_index)))
    if c['output_kind']!=6:rows.append(instruction(0x10000000,256,[([],7,0)],(6,0)))
    return [0]if c['empty']else[len(rows),*sum(rows,[])]


class Machine(md.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.linked=bool(linked)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for index,(kind,(address,name,fpu_inputs))in enumerate(MATH.items()):
            self.u.hook_add(UC_HOOK_CODE,self.math_entry,user_data=(kind,fpu_inputs),begin=address,end=address)
            # Pop exactly the private x87 arguments, load the controlled result,
            # and return. Caller-owned stack arguments retain their normal ABI.
            code=b'\xdd\xd8'*fpu_inputs+b'\xdd\x05'+struct.pack('<I',RESULT)+(b'\xc2\x04\x00'if kind=='operation'else b'\xc3');self.u.mem_write(STUB+index*0x40,code)
            if linked:
                at=linked['exports'][name if gcc else'_'+name+('@4'if kind=='operation'else'')]
                self.u.hook_add(UC_HOOK_CODE,self.math_bridge,user_data=(kind,address),begin=at,end=at)
                adapter=b''
                if kind=='power':adapter=b'\xdd\x44\x24\x04\xdd\x44\x24\x0c'
                elif kind in('asin','acos'):adapter=b'\xdd\x44\x24\x04'
                target=STUB+0x400+index*0x40;adapter+=b'\xe9'+struct.pack('<i',address-(target+len(adapter)+5));self.u.mem_write(target,adapter)
    def math_bridge(self,u,address,size,info):
        kind,target=info;index=list(MATH).index(kind);u.reg_write(xc.UC_X86_REG_EIP,STUB+0x400+index*0x40)
    def math_entry(self,u,address,size,info):
        kind,fpu_inputs=info
        if fpu_inputs:
            top=(u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7;args=tuple(u.reg_read(xc.UC_X86_REG_FP0+((top+j)&7))for j in range(fpu_inputs))
        elif kind=='floor':args=self.args(2)
        else:args=bytes(u.mem_read(self.args(1)[0],12)).hex()
        self.events.append((kind,args,hashlib.sha256(self.state()).hexdigest()))
        if self.case['controlled']:
            if self.case['math_write']and kind=='floor':u.mem_write(DATA,struct.pack('<d',9.75))
            index=list(MATH).index(kind);u.reg_write(xc.UC_X86_REG_EIP,STUB+index*0x40)
    def call(self,kind,args):
        u=self.u;stack=md.core.STACK+0xf000;self.put(stack,STOP,*args);registers=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP);saved=(0x11223344,0x22334455,0x33445566,0x44556677)
        for r,v in zip(registers,saved):u.reg_write(r,v)
        for r,v in((xc.UC_X86_REG_ESP,stack),(xc.UC_X86_REG_EFLAGS,0x202),(xc.UC_X86_REG_FPCW,self.control),(xc.UC_X86_REG_FPSW,0),(xc.UC_X86_REG_FPTAG,0xffff)):u.reg_write(r,v)
        u.emu_start(self.entries[kind],STOP,count=4000000)
        assert u.reg_read(xc.UC_X86_REG_EIP)==STOP,('return',kind,hex(u.reg_read(xc.UC_X86_REG_EIP)))
        assert u.reg_read(xc.UC_X86_REG_ESP)==stack+4+len(args)*4,('stack',kind)
        assert tuple(u.reg_read(r)for r in registers)==saved,('registers',kind,tuple(hex(u.reg_read(r))for r in registers),tuple(hex(v)for v in saved))
        assert u.reg_read(xc.UC_X86_REG_FPCW)==self.control,('x87 control',hex(u.reg_read(xc.UC_X86_REG_FPCW)),hex(self.control))
        assert self.word(0)==0x12345678
        return None if kind=='operation502'else u.reg_read(xc.UC_X86_REG_EAX)
    def fixture(self,c):
        self.case=c;self.u.mem_write(ARENA,b'\xa5'*md.core.SIZE);self.u.mem_write(HEAP,b'\x5a'*md.core.HSIZE);self.put(0,0x12345678);self.live={};self.allocated=0;self.requests=0;self.events=[]
        values=(0,0x8000000000000000,0x3ff0000000000000,0xbff0000000000000,0x3fe0000000000000,0x7ff0000000000000,0xfff0000000000000,0x7ff8123456789abc,0xfff8123456789abc,1,0x7ff0123456789abc,0xfff0123456789abc,0x41e0000000200000,0xc1e0000000200000,0x41f0000000300000)
        for bank in range(6):
            for i in range(512):
                bits=values[(i+bank+c['seed'])%len(values)]if c['data_kind']else struct.unpack('<Q',struct.pack('<d',(i%17-8)*0.25))[0]
                self.u.mem_write(DATA+bank*0x1000+i*8,struct.pack('<Q',bits))
        if c['opcode']==0x10600000:
            for i in range(256):self.u.mem_write(DATA+i*8,struct.pack('<d',0.25+(i%17)*0.125))
        self.put(LITERALS,c['literal_count'])
        for i in range(256):self.u.mem_write(LITERALS+4+i*8,struct.pack('<d',(i%13-6)*0.5))
        if c.get('real_runtime'):
            for bank in range(6):
                for i in range(512):self.u.mem_write(DATA+bank*0x1000+i*8,struct.pack('<d',(i%15-7)*0.125))
            for i in range(256):self.u.mem_write(LITERALS+4+i*8,struct.pack('<d',(i%13-6)*0.125))
            if c['opcode']==0x10600000:
                for i in range(256):self.u.mem_write(DATA+i*8,struct.pack('<d',0.25+(i%7)*0.125))
        if c.get('native_exact')and c['opcode']in(0x10600000,0x10700000,0x10800000,0x10900000,0x10c00000,0x20600000):
            value=1.0 if c['opcode']in(0x10600000,0x10700000)else 0.0
            for bank in range(6):
                for i in range(512):self.u.mem_write(DATA+bank*0x1000+i*8,struct.pack('<d',value))
            for i in range(256):self.u.mem_write(LITERALS+4+i*8,struct.pack('<d',value))
        self.put(CODE,*program(c));self.u.mem_write(RESULT,struct.pack('<d',c['math_result']));self.control=c['precision']|c['rounding']
        self.bases=[DATA,DATA+0x1000,DATA+0x2000,DATA+0x3000+c['alias'],DATA+0x4000,DATA+0x5000];self.args14=[CODE,LITERALS,*self.bases,*([c['mask']]*6)]
    def run(self,kind,c):
        self.fixture(c)
        if kind=='vm':result=self.call(kind,self.args14)
        else:
            self.put(CODE,DATA);result=self.call(kind,[c['width'],CODE,DATA+0x3000+c['alias']])
        tag=self.u.reg_read(xc.UC_X86_REG_FPTAG);top=(self.u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7
        live=tuple((j,self.u.reg_read(xc.UC_X86_REG_FP0+j))for j in range(8)if (tag>>(j*2)&3)!=3)
        return self.snapshot(result)+(self.control,top,tag,live)


def verify(image,linked,gcc,n,real_runtime=False):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();events=Counter()
    for i,c in enumerate(cases(n)):
        if real_runtime:c.update(controlled=False,data_kind=0,real_runtime=True,math_write=False)
        for kind in SPANS:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect VM mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:12];event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect VM mismatch',kind,i,c,diffs,event,len(x[3]),len(y[3]),x[-4:],y[-4:]))
            counts[kind]+=1;events.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),runtime_calls=dict(events),failures=0)


class EvalMachine(ev.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if linked:self.u.hook_add(UC_HOOK_CODE,self.vm_entry,begin=self.entries['vm'],end=self.entries['vm'])
    def vm_bridge(self,u,address,size,user):pass # Execute the recovered native bridge/body.


def verify_evaluation(image,linked,gcc,n):
    a,b=EvalMachine(image),EvalMachine(image,linked,gcc);calls=0
    for i,c in enumerate(ev.cases(n)):
        c.update(controlled=False);x=a.run('evaluate',c);y=b.run('evaluate',c)
        assert x==y,('effect VM mismatch','evaluation chain',i)
        calls+=1+int(c['repeat'])
    return dict(graphs=n,vm_calls=calls)


def native_cases():
    row=0
    for c in cases(384):
        if c['opcode']in(0x10400000,0x10500000,0x10a00000,0x10b00000,0x50200000):continue
        c.update(data_kind=0,native_exact=True,math_write=False);yield row,'vm',c;row+=1
    for c in cases(64):
        c.update(data_kind=0,native_exact=False,math_write=False);yield row,'operation502',c;row+=1


def native_capture(m,result):
    calls=[e[1]for e in m.events if e[0]=='operation'];content=pack([result or 0,m.control,(m.u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7,m.u.reg_read(xc.UC_X86_REG_FPTAG),len(calls)])
    return content+b''.join(bytes.fromhex(values)for values in calls)+bytes(m.u.mem_read(DATA,0x6000))


def mutation_checks(gcc):
    mutations={
        'instruction-count':('instruction<instructions','instruction+1<instructions'),
        'term-index-stride':('index=offset+index*4','index=offset+index*2'),
        'term-index-width':('static_cast<VmInteger>','static_cast<int>'),
        'literal-modulo':('index%literalCount','index&(literalCount-1)'),
        'scratch-index-mask':('scratch[index&0xff]','scratch[index&0x7f]'),
        'scratch-input-mask':('scratch+(index&0xff)','scratch+(index&0x7f)'),
        'scratch-output-mask':('scratch+(outputIndex&0xff)','scratch+(outputIndex&0x7f)'),
        'word-bank-stride':('base9+(index&mask9)*4','base9+(index&mask9)*8'),
        'opcode-mask':('operation&0x7ff00000','operation&0x7fe00000'),
        'broadcast-gate':('static_cast<int>(operation)<0','false'),
        'broadcast-scalar':('UG2_FX_VM_STORE_ADD(output[lane],inputs[1][lane],*inputs[0])','UG2_FX_VM_STORE_ADD(output[lane],inputs[1][lane],inputs[0][lane])'),
        'broadcast-product':('UG2_FX_VM_STORE_MUL(output[lane],inputs[1][lane],*inputs[0])','UG2_FX_VM_STORE_MUL(output[lane],inputs[1][lane],inputs[0][lane])'),
        'broadcast-atan':('UG2_FX_VM_STORE_ATAN2(output[lane],*inputs[0],inputs[1][lane])','UG2_FX_VM_STORE_ATAN2(output[lane],inputs[0][lane],inputs[1][lane])'),
        'broadcast-dot-zero':('*output=0.0;','*output=1.0;'),
        'forward-copy':('to[word]=from[word]','to[width*2-1-word]=from[width*2-1-word]'),
        'negation':('UG2_FX_VM_STORE_NEG(output[lane],inputs[0][lane])','UG2_FX_VM_STORE_LOAD(output[lane],inputs[0][lane])'),
        'reciprocal':('UG2_FX_VM_STORE_RECIP(output[lane],inputs[0][lane])','UG2_FX_VM_STORE_LOAD(output[lane],inputs[0][lane])'),
        'floor-reload':('long double value=UG2FxVmFloor(inputs[0][lane]);UG2_FX_VM_STORE_FRACTION(output[lane],inputs[0][lane],value);','double saved=inputs[0][lane];long double value=UG2FxVmFloor(saved);UG2_FX_VM_STORE_FRACTION(output[lane],saved,value);'),
        'power-base':('UG2FxVmPower(ug2VmTwo787AD0,','UG2FxVmPower(ug2VmOne786E68,'),
        'log-pointer-guard':('*inputs[lane],ug2VmZero786E60','inputs[0][lane],ug2VmZero786E60'),
        'log-input-stride':('UG2_FX_VM_STORE_LOG(output[lane],inputs[0][lane])','UG2_FX_VM_STORE_LOG(output[lane],*inputs[0])'),
        'sine-opcode':('UG2_FX_VM_STORE_SIN(output[lane]','UG2_FX_VM_STORE_COS(output[lane]'),
        'inverse-sine':('output[lane]=UG2FxVmAsin(inputs[0][lane])','output[lane]=UG2FxVmAcos(inputs[0][lane])'),
        'normal-min':('UG2_FX_VM_LT(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane]','UG2_FX_VM_GE(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane]'),
        'normal-max':('UG2_FX_VM_GT(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane]','UG2_FX_VM_LT(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane]'),
        'normal-less':('UG2_FX_VM_LT(inputs[0][lane],inputs[1][lane]) ? 1.0','UG2_FX_VM_GE(inputs[0][lane],inputs[1][lane]) ? 1.0'),
        'normal-ge':('UG2_FX_VM_GE(inputs[0][lane],inputs[1][lane]) ? 1.0','UG2_FX_VM_GT(inputs[0][lane],inputs[1][lane]) ? 1.0'),
        'normal-multiply':('UG2_FX_VM_STORE_MUL(output[lane],inputs[0][lane],inputs[1][lane])','UG2_FX_VM_STORE_ADD(output[lane],inputs[0][lane],inputs[1][lane])'),
        'normal-atan-order':('UG2_FX_VM_STORE_ATAN2(output[lane],inputs[0][lane],inputs[1][lane])','UG2_FX_VM_STORE_ATAN2(output[lane],inputs[1][lane],inputs[0][lane])'),
        'select-branches':('? inputs[1][lane] : inputs[2][lane]','? inputs[2][lane] : inputs[1][lane]'),
        'pair-dot-half':('half=inputCount>>1','half=(inputCount+1)>>1'),
        'pair-dot-pairing':('*inputs[half+input]','*inputs[input]'),
        'wrapper-clamp':('i<count && i<3','i<count && i<2'),
        'wrapper-source':('inputs[0][i]','inputs[0][0]'),
        'wrapper-padding':('values[i]=0.0f','values[i]=1.0f'),
        'wrapper-result':('*output=UG2FxVmOperation63F2B5(values)','*output=-UG2FxVmOperation63F2B5(values)'),
    }
    # MSVC 7.1 emits the same _ftol2 call and consumes its low word for the
    # int and __int64 casts here. That source change is not a distinct mutant.
    if not gcc:del mutations['term-index-width']
    for bank in (2,3,4,5,6):
        mutations['term-mask-'+str(bank)]=(f'base{bank}+(index&mask{bank})*8',f'base{bank}+(index&1)*8')
        mutations['input-mask-'+str(bank)]=(f'inputs[input]=reinterpret_cast<double *>(base{bank}+(index&mask{bank})*8)',f'inputs[input]=reinterpret_cast<double *>(base{bank}+(index&1)*8)')
    for bank in (4,5,6):mutations['output-mask-'+str(bank)]=(f'base{bank}+(outputIndex&mask{bank})*8',f'base{bank}+(outputIndex&1)*8')
    rejected=[];content=SOURCE.read_text()
    for name,(before,after)in mutations.items():
        assert before in content,(name,before)
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,-1 if name=='term-index-width'else 1))
        command=[sys.executable,__file__,'--replace',str(path),'--cases','768']+(['--gcc']if gcc else[])
        run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'effect VM mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
    return rejected


def native_mutation_checks():
    # Reuse the independently built hardware oracle and archive dependencies.
    # Put the mutated dispatcher before the archives so its ordinary symbols
    # satisfy dispatch references without extracting the unmodified VM object.
    header=ROOT/'src/Platform/Native/NativeFxVmIntrinsics.h';native=ROOT/'build/native'
    mutations={
        'comparison-preload':('header','unsigned short vmStatus;asm volatile','volatile double vmRight=(b);unsigned short vmStatus;asm volatile'),
        'arithmetic-preload':('header','#define UG2_FX_VM_STORE_BINARY(to,a,b,instruction) do { asm volatile','#define UG2_FX_VM_STORE_BINARY(to,a,b,instruction) do { volatile double vmRight=(b);asm volatile'),
        'atan-double-spill':('header','fldl %1; fldl %2; fpatan; fstpl %0','fldl %1; fldl %2; fpatan; fstpl %0; fldl %0; fstpl %0'),
        'log-scale-order':('header','fldln2; fxch; fyl2x','fld1; fxch; fyl2x'),
        'rsqrt-absolute':('header','fldl %1; fabs; fsqrt','fldl %1; fsqrt'),
        'reciprocal-operation':('header','fld1; fdivl %1','fld1; fmull %1'),
        'broadcast-load-order':('source','UG2_FX_VM_STORE_ADD(output[lane],inputs[1][lane],*inputs[0])','UG2_FX_VM_STORE_ADD(output[lane],*inputs[0],inputs[1][lane])'),
        'index-conversion-width':('source','static_cast<VmInteger>','static_cast<int>'),
        'narrowing-third-lane':('source','i<count && i<3','i<count && i<2'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=(header if unit=='header'else SOURCE).read_text();assert before in content,name
        content=content.replace(before,after)
        if name=='comparison-preload':content=content.replace('"m"(a),"m"(b) : "st");vmStatus','"m"(a),"m"(vmRight) : "st");vmStatus')
        if name=='arithmetic-preload':content=content.replace('"m"(a),"m"(b) : "st"); } while (0)\n#define UG2_FX_VM_STORE_ADD','"m"(a),"m"(vmRight) : "st"); } while (0)\n#define UG2_FX_VM_STORE_ADD')
        path=BUILD/('mutation-native-'+name+('.h'if unit=='header'else'.cpp'))
        if unit=='header':content=content.replace('UG2_NATIVE_FX_VM_INTRINSICS_H','UG2_NATIVE_FX_VM_MUTATED_H')
        path.write_text(content);obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        command=['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB)]
        if unit=='header':command+=['-DUG2_NATIVE_FX_VM_INTRINSICS_H']
        command+=['-include',str(ROOT/'src/Platform/Native/NativeCompat.h')]
        if unit=='header':command+=['-include',str(path)]
        command+=['-c',str(SOURCE if unit=='header'else path),'-o',str(obj)]
        subprocess.run(command,cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2effectvm_probe.dir/src/Platform/Native/EffectVmProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),'/dev/null',str(BUILD/'mutation-native-capture.bin')],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware VM'in run.stdout and 'mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
    return rejected


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--gcc',action='store_true');parser.add_argument('--cases',type=int,default=512);parser.add_argument('--replace',type=Path);parser.add_argument('--real-runtime',action='store_true');parser.add_argument('--write-native-fixtures',type=Path);parser.add_argument('--native-dump',type=Path)
    parser.add_argument('--mutation-checks',action='store_true')
    parser.add_argument('--native-mutation-checks',action='store_true')
    args=parser.parse_args();raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in [*manifest['functions'].values(),*manifest['data'].values()]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));report=verify(image,linked,args.gcc,args.cases,args.real_runtime)
    if not args.replace:report['evaluation_chains']=verify_evaluation(image,linked,args.gcc,128)
    if args.write_native_fixtures:
        rows=list(native_cases());content=b'UG2FXMF1'+pack([len(rows)]);m=Machine(image)
        for row,kind,c in rows:
            m.fixture(c)
            if kind=='operation502':m.put(CODE,DATA)
            arguments=m.args14 if kind=='vm'else[c['width'],CODE,DATA+0x3000+c['alias']]
            content+=pack([row,int(kind=='operation502'),m.control,len(arguments)])+struct.pack('<d',c['math_result'])+pack(arguments)+bytes(m.u.mem_read(ARENA,md.core.SIZE))
        args.write_native_fixtures.write_bytes(content)
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXMD1';at=8;a,b=Machine(image),Machine(image,linked,args.gcc)
        for row,kind,c in native_cases():
            x=a.run(kind,c);y=b.run(kind,c);assert x==y,('effect VM mismatch','native model',row)
            expected=native_capture(a,x[2]);index,size=struct.unpack_from('<2I',raw,at);at+=8;actual=raw[at:at+size];at+=size
            assert index==row and actual==expected,('native effect VM mismatch',row,kind,[(hex(j),v,w)for j,(v,w)in enumerate(zip(expected,actual))if v!=w][:12])
        assert at==len(raw);report['native_replay']=row+1
    report.update(compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    (BUILD/(('fixtures-'if args.write_native_fixtures else'verification-')+('gcc'if args.gcc else'msvc')+('-real'if args.real_runtime else'')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
