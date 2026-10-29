#!/usr/bin/env python3
"""Compare UG2 0x764749 and its recovered SSE2 power value flow.

The original SSE arithmetic executes; neither side substitutes a power result.
Only the existing thread/matherr/OS fixtures are controlled. Results retain all
80 return bits, FP controls/flags, caller-owned x87 values and callback payloads.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import unicorn.x86_const as xc
import verify_pc_crt_power as pw
fl,cx,md=pw.fl,pw.cx,pw.md;ROOT,EXE,LIB=pw.ROOT,pw.EXE,pw.LIB;ARENA,STOP=pw.ARENA,pw.STOP
BUILD=ROOT/'build/analysis/crt-power-sse';md.BUILD=BUILD
SOURCE=LIB/'CrtPowerSse.cpp';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/crt-power-sse.json'
md.FREE['sse_body']='D3DXCrtPowerSse80';md.CDECL.add('sse_body');pw.VOID.add('sse_body')
KINDS=('sse_body','sse_wrapper','power','bridge')
BODY_ADAPTER=STOP+0xf400

def cases(n):
    rng=random.Random(0x764749)
    for i,c in enumerate(pw.cases(n)):
        c=dict(c);c['sse']=1
        if i%7==1:
            # Keep table-sensitive normal operands in the early mutation corpus,
            # alongside the inherited exceptional/rounding/control cases.
            c['bits']=((1022+i//7%3)<<52)|rng.getrandbits(52)|((i//7%4==3)<<63)
            exponent=(-7.75,-3,-1.5,-1,0.125,0.5,1,1.75,2,3,7.125,8)[i//7%12]
            c['other']=struct.unpack('<Q',struct.pack('<d',exponent))[0]
            c['extended']=pw.iv.extended(c['bits']);c['exp80']=pw.iv.extended(c['other'])
        # Alternate ordinary and explicitly DAZ/FTZ environments. The direct
        # body is also exercised with unmasked flags in emulation; Linux fault
        # delivery is never inferred from those cases.
        c['mxcsr']|=((i//17)%4 in(1,3))*0x40|((i//17)%4 in(2,3))*0x8000
        yield c

class Machine(pw.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc);self.sse_codes=Counter();self.blocks=set()
        if not linked:
            self.entries['sse_body']=BODY_ADAPTER
            code=b'\xff\x74\x24\x10'*4
            code+=b'\xe8'+struct.pack('<i',0x764749-(BODY_ADAPTER+len(code)+5))
            code+=b'\xdb\x3d'+struct.pack('<I',pw.OUTPUT)+b'\x83\xc4\x10\xc3'
            self.u.mem_write(BODY_ADAPTER,code)
            self.u.hook_add(UC_HOOK_CODE,self.sse_error,begin=0x7694d0,end=0x7694d0)
            self.u.hook_add(UC_HOOK_CODE,self.block,begin=0x764749,end=0x765275)
    def sse(self,u,at,size,user):pass # Execute original/recovered arithmetic.
    def block(self,u,at,size,user):self.blocks.add(at)
    def sse_error(self,u,at,size,user):self.sse_codes[self.args(4)[3]]+=1
    def known_record(self,p):
        result=cx.Machine.known_record(self,p)
        # SSE error opcodes 24..29 and 1006 all name power (operation 29).
        w=struct.unpack('<32I',self.u.mem_read(p,128));return(*result,w[12],w[13],w[16]&31)
    def run_power(self,kind,c):
        if kind!='sse_body':return super().run_power(kind,c)
        self.run_exception('error_code',c);self.kind=kind;self.events=[]
        self.u.mem_write(pw.OUTPUT,b'\xa5'*12)
        args=(*fl.words(c['bits']),*fl.words(c['other']))
        state=self.call_exception(kind,((pw.OUTPUT,*args)if self.linked else args));state=(None,*state[1:])
        return(state,bytes(self.u.mem_read(pw.OUTPUT,10)).hex(),bytes(self.u.mem_read(fl.THREAD,32)).hex(),self.events)

def composed(image,linked,gcc):
    machines=Machine(image),Machine(image,linked,gcc);calls=0
    for i,c in enumerate(cases(1024)):
        width=i%4;offset=(-4,0,4,8,32)[i%5];observed=[]
        for m in machines:
            m.run_power('sse_body',c);m.kind='vm';m.events=[];m.put(0x8ba480,1)
            for lane in range(3):m.u.mem_write(fl.DATA+16+lane*8,struct.pack('<Q',c['other']))
            m.put(ARENA+0x4000,1,0x10500000|width,1,0,3,0,0,4,0);m.put(ARENA+0x5000,1,0,0)
            state=m.call_crt('vm',[ARENA+0x4000,ARENA+0x5000,fl.DATA+16,0,0,fl.DATA+16+offset,0,0,0,0,0,0,0,0])
            observed.append((state,bytes(m.u.mem_read(fl.DATA,128)),bytes(m.u.mem_read(fl.THREAD,32)),m.events))
        assert observed[0]==observed[1],('SSE power mismatch','composed VM',i,c,observed)
        calls+=width
    return dict(programs=1024,power_calls=calls)

def entry_audit(image,n):
    machine=Machine(image);rng=random.Random(0x764749)
    for i,c in enumerate(cases(n)):
        observed=[]
        for mode in range(4):
            for j in range(8):machine.u.reg_write(xc.UC_X86_REG_XMM0+j,0 if mode==0 else(1<<128)-1 if mode==1 else rng.getrandbits(128))
            observed.append(machine.run_power('sse_body',c))
        assert all(x==observed[0]for x in observed),('entry XMM dependence',i,c,observed)
    return dict(operands=n,incoming_full_XMM_seeds=4)

def validate_manifest(raw,image):
    m=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==m['executable_sha256']
    for row in[*m['functions'].values(),*m['data'].values()]:
        offset=int(row['address'],16)-0x400000;assert hashlib.sha256(image[offset:offset+row['size']]).hexdigest()==row['sha256']

def mutation_checks(gcc):
    mutations={
        'first-table-index':('index&=0x1fe;','index&=0x1fc;'),
        'log-exponent-bias':('bound=0x3fe7f;','bound=0x3ff7f;'),
        'log-split-mask':('Table(0x007d7bf8,2)','Table(0x007d7bc0,2)'),
        'packed-log-tail':('UG2FxCrtSseAddPacked(&value6,&source);','UG2FxCrtSseAddScalar(&value6,&source);'),
        'negative-integrality':('signScale=0x3f3;','signScale=0x3f4;'),
        'negative-odd-sign':('signScale&=0x40000;','signScale=0;'),
        'underflow-error':('bound=0x19;','bound=0x18;'),
        'domain-error':('bound=0x1c;','bound=0x1b;'),
        'second-error-input':('&Number(first),&Number(second),&output','&Number(first),&Number(first),&output'),
        'tiny-result-scale':('signScale|=0x1ff80;','signScale|=0x1ff00;'),
    }
    rejected=[]
    for name,(before,after)in mutations.items():
        content=SOURCE.read_text();assert before in content
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,1))
        run=subprocess.run([sys.executable,__file__,'--cases','2048','--kind','sse_body','--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'SSE power mismatch'in run.stdout,(name,run.returncode,run.stdout[-3000:]);rejected.append(name)
    return rejected

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=8192);p.add_argument('--kind',choices=['all',*KINDS],default='all');p.add_argument('--replace',type=Path);p.add_argument('--entry-audit',action='store_true');p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();validate_manifest(raw,image)
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc);counts=Counter()
    for i,c in enumerate(cases(args.cases)):
        for kind in KINDS if args.kind=='all'else(args.kind,):
            try:x=a.run_power(kind,c);y=b.run_power(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('SSE power mismatch',kind,i,c,str(e)))from e
            assert x==y,('SSE power mismatch',kind,i,c,x,y)
            counts[kind]+=1
    report=dict(comparisons=dict(counts),original_instruction_addresses=len(a.blocks),sse_error_codes=dict(a.sse_codes),failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established',boundary='thread data, controlled matherr/custom callbacks, OS RaiseException; unmasked fault delivery not established')
    if not args.replace:report['composed']=composed(image,linked,args.gcc)
    if args.entry_audit:report['entry_XMM_state_audit']=entry_audit(image,args.cases)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if not args.replace:(BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__=='__main__':main()
