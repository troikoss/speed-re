#!/usr/bin/env python3
"""Verify UG2 asin/acos and their shared private-x87/control/error helpers.

Private-ABI adapters supply the original incoming x87 value, saved stack word,
logical operation and EAX/ZF classification. Recovered source carries those
values explicitly; no original math code is substituted on the rebuilt side.
Only thread data, observed default matherr and OS exception services are hooked.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_crt_exceptions as cx
fl=cx.fl;md=cx.md;ROOT,EXE,LIB=cx.ROOT,cx.EXE,cx.LIB;ARENA,STOP=cx.ARENA,cx.STOP
BUILD=ROOT/'build/analysis/crt-inverse';md.BUILD=BUILD
SOURCE=LIB/'CrtInverse.cpp';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/crt-inverse.json'
SPANS=dict(asin_entry=(0x75eea0,20),acos_entry=(0x75db80,20),asin_core=(0x75eebd,174),acos_core=(0x75db9d,174),inspect=(0x765748,22),prepare=(0x7656d5,23),nan=(0x7656ec,25),start=(0x765677,60),quick=(0x76575e,13),exit=(0x76576b,42),dispatch=(0x76cb75,254))
NAMES=dict(asin='D3DXCrtAsin80',acos='D3DXCrtAcos80',inspect='D3DXCrtInspectOperand',prepare='D3DXCrtPrepareControl',nan='D3DXCrtPrepareNan',start='D3DXCrtOneArgError',quick='D3DXCrtQuickExit',exit='D3DXCrtMathExit',dispatch='D3DXCrtOneArgDispatch',asin_bridge='UG2FxVmAsin',acos_bridge='UG2FxVmAcos')
md.FREE.update(NAMES);md.CDECL.update(NAMES)
INPUT,OUTPUT,SAVED,ERROR=ARENA+0x6000,ARENA+0x6100,ARENA+0x6200,ARENA+0x6300
ADAPTER=STOP+0xd000
VOID=set(NAMES)-{'inspect','nan'}


def extended(bits):
    sign=bits>>63;exponent=bits>>52&0x7ff;fraction=bits&0xfffffffffffff
    if exponent==0x7ff:return struct.pack('<QH',0x8000000000000000|(fraction<<11),(sign<<15)|0x7fff)
    if not exponent:
        if not fraction:return struct.pack('<QH',0,sign<<15)
        leading=fraction.bit_length()-1;return struct.pack('<QH',fraction<<(63-leading),(sign<<15)|(16383+leading-1074))
    return struct.pack('<QH',((1<<52)|fraction)<<11,(sign<<15)|(exponent-1023+16383))


def cases(n):
    fixed=cx.FIXED+(0x3ff0000000000001,0xbff0000000000001,0x3fe0000000000000,0xbfe0000000000000,0x3feffffffffffffe,0xbfeffffffffffffe)
    for i,c in enumerate(cx.cases(n)):
        c=dict(c);bits=fixed[i%len(fixed)]if i<4096 else c['bits'];value=extended(bits)
        if i%13==0 and bits>>52&0x7ff not in(0,0x7ff):
            significand,exponent=struct.unpack('<QH',value);value=struct.pack('<QH',significand|((i*29)&0x7ff),exponent)
        control=c['control']
        if i%7==0:control&=~1
        if i%11==0:control&=~4
        if i%17==0:control&=~32
        c.update(control=control,status=0 if control&0x3f!=0x3f else c['status'])
        if i%19==0:
            # Exercise original ST0 values outside double's range and with
            # precision beyond its mantissa, including valid extended denormals.
            extra=((0x8000000000000000,1),(1,0),(0x7fffffffffffffff,0),(0x8000000000000000,0x3bcc),(0xffffffffffffffff,0x3ffe),(0x8000000000000001,0x3fff),(0x8000000000000000,0x43ff),(0xffffffffffffffff,0x7ffe))
            significand,exponent=extra[(i//19)%len(extra)];value=struct.pack('<QH',significand,exponent|((i//152)%2)*0x8000)
        saved=c['saved'];saved=saved&~0x300 if saved&0x300==0x100 else saved
        c.update(bits=bits,extended=value,saved=saved,bypass=i%3==0,operation=(13,14,16,22,29,0xffffffff)[i%6],type=(0,1,2,3,4,5,6,7,8,-1,0x7fffffff)[(i//7)%11],name=(0x819cc0,0x819f00)[i%2],alias=(0,2,8,16,24,30)[(i//9)%6]if i%5==0 else -1)
        yield c


def native_cases(n):
    for i,c in enumerate(cases(n)):
        c=dict(c);c['saved']|=0x7f;c['control']|=0x7f;c['mxcsr']|=((i//17)%4 in(1,3))*0x40|((i//17)%4 in(2,3))*0x8000
        yield c


def write_native_fixtures(path,n):
    raw=b'UG2FXAI1'+struct.pack('<I',n)
    for i,c in enumerate(native_cases(n)):
        raw+=struct.pack('<15I3Q',i,c['control'],c['mxcsr'],c['status'],c['saved'],int(c['bypass']),c['mode'],c['operation']&0xffffffff,c['type']&0xffffffff,c['name'],int(c['callback_write']),int(c['redirect']),c['record_bits'],c['seed'],c['alias']&0xffffffff,c['bits'],c['other'],c['result'])+c['extended']+bytes(2)
    path.write_bytes(raw)


class Machine(cx.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:
            self.entries.update(inspect=0x765748,prepare=0x7656d5,dispatch=0x76cb75)
            for j,kind in enumerate(('asin','acos','nan','start','quick','exit','asin_bridge','acos_bridge')):self.entries[kind]=ADAPTER+j*0x100
    def default_entry(self,u,address,size,user):
        pointer=self.args(1)[0];record=bytearray(u.mem_read(pointer,32))
        if self.kind!='dispatch':record[16:24]=bytes(8) # Uninitialized second input in the one-arg caller.
        self.events.append(('default',record.hex(),self.fp_state()))
    def known_record(self,p):
        result=super().known_record(p)
        if self.kind=='dispatch'and self.case['operation']in(16,22,29):
            w=struct.unpack('<32I',self.u.mem_read(p,128));result=(*result,w[12],w[13],w[16]&31)
        return result
    def adapters(self,c):
        # Hand-written ABI shims only: source algorithms are never patched.
        def imm(x):return struct.pack('<I',x&0xffffffff)
        load=b'\xdb\x2d'+imm(INPUT);store=b'\xdb\x3d'+imm(OUTPUT)
        mov=lambda op,x:bytes([op])+imm(x)
        for j,kind in enumerate(('asin','acos','nan','start','quick','exit','asin_bridge','acos_bridge')):
            at=ADAPTER+j*0x100;code=b''
            def call(target):return b'\xe8'+struct.pack('<i',target-(at+len(code)+5))
            if kind in('asin_bridge','acos_bridge'):
                target=0x75eea0 if kind=='asin_bridge'else 0x75db80
                code=b'\xdd\x44\x24\x04';code+=b'\xe9'+struct.pack('<i',target-(at+len(code)+5))
            elif kind in('asin','acos'):
                code=load;code+=call(SPANS[kind+'_entry'][0]);code+=store+b'\xc3'
            elif kind=='nan':
                code=load+mov(0xb8,c['bits']>>32);code+=call(0x7656ec);code+=b'\xdb\x3d'+imm(INPUT)+b'\xc3'
            elif kind=='start':
                low,high=fl.words(c['bits']);code=b'\x68'+imm(high)+b'\x68'+imm(low)+b'\x6a\x00\x68'+imm(c['saved'])
                code+=load+mov(0xb8,c['type'])+mov(0xb9,c['name'])+mov(0xba,13);code+=call(0x765677);code+=store+b'\x83\xc4\x10\xc3'
            else:
                target=0x76575e if kind=='quick'else 0x76576b
                prefix=b'' if kind=='quick'else b'\x68'+imm(c['bits']>>32)+b'\x68'+imm(c['bits'])
                # Private exit pops the saved word, then RETs to this shim.
                before=prefix+load+mov(0xba,13)+mov(0xb9,c['name'])
                after=at+len(before)+5+5+5
                code=before+b'\x68'+imm(after)+b'\x68'+imm(c['saved']);code+=b'\xe9'+struct.pack('<i',target-(at+len(code)+5))
                code+=store+(b''if kind=='quick'else b'\x83\xc4\x08')+b'\xc3'
            self.u.mem_write(at,code)
        self.u.ctl_remove_cache(ADAPTER,ADAPTER+0x800)
    def run_inverse(self,kind,c):
        # Reuse fully initialized record/data/thread fixtures and controlled OS
        # callback state; this preliminary classifier has no floating effects.
        self.run_exception('error_code',c);self.kind=kind;self.events=[];u=self.u
        u.mem_write(INPUT,c['extended']+b'\xa5\xa5');u.mem_write(OUTPUT,b'\xa5'*12);self.put(SAVED,c['saved']);self.put(0x8b8424,int(c['bypass']))
        self.put(ERROR,c['type'],c['name'],*fl.words(c['bits']),*fl.words(c['other']),*fl.words(c['result']))
        if not self.linked and getattr(self,'adapter_case',None)is not c:self.adapters(c);self.adapter_case=c
        control=ERROR+c['alias']if c['alias']>=0 else SAVED
        # The dispatcher allocates an uninitialized exception record. Reserved
        # PC=1 leaves its precision field undefined and changes return decoding.
        # Initialized-record PC=1 behavior is covered by the prior CRT suite.
        word=self.half(control)
        if word&0x300==0x100:self.put_half(control,word&~0x300)
        args=dict(asin=(OUTPUT,INPUT),acos=(OUTPUT,INPUT),asin_bridge=fl.words(c['bits']),acos_bridge=fl.words(c['bits']),inspect=fl.words(c['bits']),prepare=(c['saved'],),nan=(INPUT,c['bits']>>32),start=(13,c['type'],c['name'],*fl.words(c['bits']),c['saved'],INPUT),quick=(c['saved'],),exit=(13,c['name'],*fl.words(c['bits']),c['saved'],INPUT),dispatch=(c['operation'],ERROR,control))
        # Source start/exit update INPUT in place; original shims store there too.
        if not self.linked and kind in('start','exit','quick'):
            self.u.mem_write(OUTPUT,c['extended']+b'\xa5\xa5')
        state=self.call_exception(kind,args[kind]);state=(None if kind in VOID else state[0],*state[1:])
        data=bytes(u.mem_read(INPUT,10))if self.linked and kind in('start','exit')else bytes(u.mem_read(OUTPUT,10))if kind in('start','exit')else b''
        return(state,bytes(u.mem_read(INPUT,10)).hex()if kind not in('start','exit','quick')else'',bytes(u.mem_read(OUTPUT,10)).hex()if kind in('asin','acos')else'',data.hex(),bytes(u.mem_read(ERROR,32)).hex(),bytes(u.mem_read(SAVED,4)).hex(),bytes(u.mem_read(fl.THREAD,32)).hex(),self.events)


def composed(image,linked,gcc):
    machines=Machine(image),Machine(image,linked,gcc);calls=0
    for i,c in enumerate(cases(1024)):
        for cosine in(False,True):
            width=i%4;offset=(0,4,8,16,32)[i%5];observed=[]
            for m in machines:
                m.run_crt('classify',c);m.kind='vm';m.events=[];m.put(cx.ALTERNATE,*([0x12345678]*32));m.put(0x8b8424,int(c['bypass']))
                m.put(ARENA+0x4000,1,(0x10b00000 if cosine else 0x10a00000)|width,1,0,3,0,0,4,0);m.put(ARENA+0x5000,1,0,0)
                state=m.call_crt('vm',[ARENA+0x4000,ARENA+0x5000,fl.DATA,0,0,fl.DATA+offset,0,0,0,0,0,0,0,0])
                observed.append((state,bytes(m.u.mem_read(fl.DATA,128)),bytes(m.u.mem_read(fl.THREAD,32)),m.events))
            assert observed[0]==observed[1],('CRT inverse mismatch','composed VM',i,cosine,c,observed)
            calls+=width
    return dict(programs=2048,inverse_calls=calls)


def mutation_checks(gcc):
    mutations={
        'inspect-mask':('inspect','exponent=value.high&0x7ff00000','exponent=value.high&0x7fe00000'),
        'prepare-precision':('prepare','(saved&0x300)|0x7f','(saved&0)|0x7f'),
        'prepare-rounding':('prepare','(saved&0x300)|0x7f','(saved&0xf00)|0x7f'),
        'nan-kind':('nan','if (high&0x80000) return 7','if (high&0x40000) return 7'),
        'nan-arithmetic':('nan','UG2FxCrtExtendedAddDouble(value,&ug2CrtNanAdd7D7C40);',''),
        'dispatch-overflow':('dispatch','case 3:flags=0x11','case 3:flags=0x1'),
        'dispatch-underflow':('dispatch','case 4:flags=0x12','case 4:flags=0x2'),
        'dispatch-seven':('dispatch','case 7:error->type00=1','case 7:error->type00=2'),
        'dispatch-eight':('dispatch','error->type00!=8','error->type00==8'),
        'dispatch-mode':('dispatch','!ug2CrtMathErrorFlag81ACE0','ug2CrtMathErrorFlag81ACE0'),
        'dispatch-second-operation':('dispatch','operation==0x16','operation==0x15'),
        'dispatch-second-payload':('dispatch','record)+12),error->input10','record)+12),error->input08'),
        'dispatch-restore':('dispatch','D3DXCrtRawControl(control,0xffff)','D3DXCrtRawControl(control,0xff) '),
        'start-result-load':('start','UG2FxCrtExtendedFromDouble(value,&error.result18)','UG2FxCrtExtendedFromDouble(value,&error.input08)'),
        'start-second-restore':('start','if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);','if (false) UG2FxWriteControlWord(saved);'),
        'quick-restore':('quick','if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);','if (false) UG2FxWriteControlWord(saved);'),
        'exit-precision-gate':('exit','(saved&0x20)&&(D3DXCrtReadStatus()&0x20)','!(saved&0x20)&&(D3DXCrtReadStatus()&0x20)'),
        'exit-error-type':('exit','D3DXCrtOneArgError(operation,8,name','D3DXCrtOneArgError(operation,1,name'),
        'entry-endpoint':('asin','classification<0x3ff00000','classification<=0x3ff00000'),
        'entry-domain-fraction':('acos','(bits.high&0xfffff)||bits.low) {','(bits.high&0xfffff)) {'),
        'entry-nonfinite':('asin','bool special=(bits.high&0x7ff00000)==0x7ff00000','bool special=false'),
        'entry-bypass':('asin','if (ug2CrtBypassMathExit8B8424)','if (!ug2CrtBypassMathExit8B8424)'),
        'entry-sign':('asin','value.exponent^=0x8000','value.exponent^=0'),
        'entry-acos-pi':('acos','UG2FxCrtExtendedPi(&value)','UG2FxCrtExtendedSqrt(&value)'),
        'entry-atan-order':('asin','UG2FxCrtExtendedAtan2(&result,&value,&root)','UG2FxCrtExtendedAtan2(&result,&root,&value)'),
        'entry-operation':('asin','cosine ? 0xd : 0xe','cosine ? 0xd : 0xc'),
        'entry-double-narrow':('asin','UG2FxCrtExtendedToDouble(&argument,&value);D3DXCrtDoubleBits','UG2FxCrtExtendedToDouble(&argument,&value);UG2FxCrtExtendedFromDouble(&value,&argument);D3DXCrtDoubleBits'),
    }
    rejected=[]
    for name,(kind,before,after)in mutations.items():
        content=SOURCE.read_text();assert before in content,(name,before)
        # The first matching restore belongs to Start; explicitly target Quick.
        if name=='quick-restore':
            before='D3DXCrtQuickExit(unsigned int saved) { '+before;after='D3DXCrtQuickExit(unsigned int saved) { '+after
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        run=subprocess.run([sys.executable,__file__,'--cases','2048','--kind',kind,'--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'CRT inverse mismatch'in run.stdout,(name,run.returncode,run.stdout[-3000:]);rejected.append(name)
    return rejected


def native_mutation_checks():
    native=ROOT/'build/native';fixture=BUILD/'native-mutation-fixtures.bin';write_native_fixtures(fixture,8192)
    adapter=ROOT/'src/Platform/Native/NativeFxCrtExtended.cpp'
    mutations={
        'extended-result':(SOURCE,'CopyExtended(*out,value);','double narrowed;UG2FxCrtExtendedToDouble(&narrowed,&value);UG2FxCrtExtendedFromDouble(out,&narrowed);'),
        'incoming-value':(SOURCE,'UG2FxCrtExtendedToDouble(&argument,&value);D3DXCrtDoubleBits','UG2FxCrtExtendedToDouble(&argument,&value);UG2FxCrtExtendedFromDouble(&value,&argument);D3DXCrtDoubleBits'),
        'one-minus-direction':(adapter,'fldt %1; fld1; fsubp; fstpt %0','fldt %1; fld1; fsubrp; fstpt %0'),
        'sqrt-instruction':(adapter,'fldt %0; fsqrt; fstpt %0','fldt %0; fstpt %0'),
        'atan-order':(SOURCE,'UG2FxCrtExtendedAtan2(&result,&value,&root)','UG2FxCrtExtendedAtan2(&result,&root,&value)'),
        'floating-input-copy':(SOURCE,'UG2_CRT_COPY_DOUBLE_BITS(error.input08,input);','UG2_FX_VM_STORE_LOAD(error.input08,*reinterpret_cast<double *>(&input));'),
        'quick-restore':(SOURCE,'D3DXCrtQuickExit(unsigned int saved) { if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);','D3DXCrtQuickExit(unsigned int saved) { if (false) UG2FxWriteControlWord(saved);'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=unit.read_text();assert before in content,(name,before)
        path=BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1).replace('../../Speed/Indep/Libs/D3DX/CrtInverse.h','CrtInverse.h'));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB),'-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),'-c',str(path),'-o',str(obj)],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2crtinverse_probe.dir/src/Platform/Native/CrtInverseProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),str(fixture)],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware CRT inverse'in run.stdout and 'mismatch'in run.stdout,(name,run.returncode,run.stdout[-2500:]);rejected.append(name)
    (BUILD/'verification-native-mutations.json').write_text(json.dumps(dict(rejected_mutations=rejected),indent=2)+'\n');return rejected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=8192);p.add_argument('--kind',choices=['all',*NAMES],default='all');p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');p.add_argument('--native-mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();m=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for row in[*m['functions'].values(),*m['data'].values()]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    if args.write_native_fixtures:write_native_fixtures(args.write_native_fixtures,args.cases)
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    for i,c in enumerate(cases(args.cases)):
        for kind in NAMES if args.kind=='all'else(args.kind,):
            try:x=a.run_inverse(kind,c);y=b.run_inverse(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('CRT inverse mismatch',kind,i,c,str(e)))from e
            assert x==y,('CRT inverse mismatch',kind,i,c,x,y)
            counts[kind]+=1
    report=dict(comparisons=dict(counts),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',platform_boundary='OS RaiseException; thread data; observed default matherr')
    if not args.replace:report['composed']=composed(image,linked,args.gcc)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
