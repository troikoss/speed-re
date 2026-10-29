#!/usr/bin/env python3
"""Compare native SSE power with hardware-assisted original control flow.

Unicorn executes original integer operations, branches and table accesses. Each
SSE FP instruction is evaluated by a separate native instruction oracle; x87
64-bit loads/stores use physical conversions as well. This avoids Unicorn's
NaN, DAZ/FTZ and FP-status inaccuracies without running any retail code natively.
The native recovered body is invoked separately, with its real ISA adapters.
"""
import argparse
from collections import Counter
import json,struct,subprocess
from pathlib import Path
import capstone as cs
import capstone.x86_const as cc
import pefile
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
import verify_pc_crt_power_sse as sse

OPS={'addsd':0,'subsd':1,'mulsd':2,'divsd':3,'addpd':4,'mulpd':5,'cvtdq2pd':6,'cvtsd2si':7}
class Hardware:
    def __init__(self,binary):
        self.process=subprocess.Popen([str(binary),str(sse.EXE)],stdin=subprocess.PIPE,stdout=subprocess.PIPE);self.counts=Counter()
    def request(self,op,control,mxcsr,status,a=bytes(16),b=bytes(16)):
        self.counts[op]+=1;self.process.stdin.write(struct.pack('<4I',op,control,mxcsr,status)+a+b);self.process.stdin.flush()
        raw=self.process.stdout.read(116);assert len(raw)==116,('hardware server',op,self.process.poll(),raw)
        return raw
    def close(self):
        self.process.stdin.close();assert self.process.wait()==0

def instructions(image):
    decoder=cs.Cs(cs.CS_ARCH_X86,cs.CS_MODE_32);decoder.detail=True;result={}
    for address,size in((0x764749,2861),(0x7694d0,660)):
        for ins in decoder.disasm(image[address-0x400000:address-0x400000+size],address):
            if ins.mnemonic in OPS or ins.mnemonic in('fld','fstp')and ins.operands[0].type==cc.X86_OP_MEM and ins.operands[0].size==8:result[ins.address]=ins
    return result

class Original(sse.Machine):
    def __init__(self,image,hardware):
        super().__init__(image);self.hardware=hardware;self.instructions=instructions(image);self.executed=Counter()
        self.u.hook_add(UC_HOOK_CODE,self.physical,begin=0x764749,end=0x769764)
    def reg(self,ins,reg):return getattr(xc,'UC_X86_REG_'+ins.reg_name(reg).upper())
    def operand(self,ins,operand):
        if operand.type==cc.X86_OP_REG:return self.u.reg_read(self.reg(ins,operand.reg)).to_bytes(16,'little')
        assert operand.type==cc.X86_OP_MEM
        mem=operand.mem;address=mem.disp
        if mem.base:address+=self.u.reg_read(self.reg(ins,mem.base))
        if mem.index:address+=self.u.reg_read(self.reg(ins,mem.index))*mem.scale
        return bytes(self.u.mem_read(address&0xffffffff,operand.size)).ljust(16,b'\0')
    def physical(self,u,at,size,user):
        ins=self.instructions.get(at)
        if ins is None:return
        name=ins.mnemonic;self.executed[name]+=1;cw=u.reg_read(xc.UC_X86_REG_FPCW);mx=u.reg_read(xc.UC_X86_REG_MXCSR);sw=u.reg_read(xc.UC_X86_REG_FPSW)
        if name in OPS:
            dst,src=ins.operands;a=self.operand(ins,src if name in('cvtdq2pd','cvtsd2si')else dst);b=self.operand(ins,src)
            raw=self.hardware.request(OPS[name],cw,mx,sw&0x3f,a,b)
            u.reg_write(self.reg(ins,dst.reg),int.from_bytes(raw[:4 if name=='cvtsd2si'else 16],'little'));u.reg_write(xc.UC_X86_REG_MXCSR,struct.unpack_from('<I',raw,32)[0])
        else:
            top=(sw>>11)&7;tag=u.reg_read(xc.UC_X86_REG_FPTAG);operand=ins.operands[0];mem=operand.mem
            address=mem.disp+(u.reg_read(self.reg(ins,mem.base))if mem.base else 0)
            assert not mem.index
            if name=='fld':
                raw=self.hardware.request(9,cw,mx,sw&0x3f,self.operand(ins,operand));bits=raw[16:26];mantissa,exponent=struct.unpack('<QH',bits)
                top=(top-1)&7;assert(tag>>(top*2)&3)==3
                u.reg_write(xc.UC_X86_REG_FP0+top,(mantissa,exponent));kind=1 if not mantissa and not(exponent&0x7fff)else 2 if exponent&0x7fff in(0,0x7fff)else 0;tag=(tag&~(3<<(top*2)))|(kind<<(top*2))
            else:
                mantissa,exponent=u.reg_read(xc.UC_X86_REG_FP0+top);a=struct.pack('<QH',mantissa,exponent)+bytes(6)
                raw=self.hardware.request(10,cw,mx,sw&0x3f,a);u.mem_write(address&0xffffffff,raw[:8]);tag|=3<<(top*2);top=(top+1)&7
            flags=struct.unpack_from('<I',raw,36)[0];u.reg_write(xc.UC_X86_REG_FPSW,(sw&~0x383f)|(top<<11)|flags);u.reg_write(xc.UC_X86_REG_FPTAG,tag)
        u.reg_write(xc.UC_X86_REG_EIP,at+ins.size)

def cases(n):
    for i,c in enumerate(sse.cases(n)):
        c=dict(c);c.update(control=c['control']|0x7f,saved=c['saved']|0x7f,mxcsr=c['mxcsr']|0x1f80,live=2,top=6,callback=False,seed=0x12345678)
        yield c

def mutation_checks():
    adapter=sse.ROOT/'src/Platform/Native/NativeFxCrtSseInstructions.cpp';native=sse.ROOT/'build/native'
    mutations={
        'subtract-direction':(adapter,'subsd %%xmm1,%%xmm0','subsd %%xmm0,%%xmm1'),
        'convert-rounding':(adapter,'cvtsd2si %%xmm0,%0','cvttsd2si %%xmm0,%0'),
        'packed-high-lane':(adapter,'mulpd %%xmm1,%%xmm0','mulsd %%xmm1,%%xmm0'),
        'dead-invalid-flag':(sse.SOURCE,'UG2FxCrtSseMultiplyScalar(&value1,&value1);',''),
        'callback-second-input':(sse.SOURCE,'&Number(first),&Number(second),&output','&Number(first),&Number(first),&output'),
    }
    rejected=[]
    for name,(source,before,after)in mutations.items():
        content=source.read_text();assert before in content
        path=sse.BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1).replace('../../Speed/Indep/Libs/D3DX/CrtPowerSse.h','CrtPowerSse.h'));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-O1','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-include',str(sse.ROOT/'src/Platform/Native/NativeCompat.h'),'-I'+str(sse.LIB),'-c',str(path),'-o',str(obj)],cwd=sse.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(sse.ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2crtpowersse_probe.dir/src/Platform/Native/CrtPowerSseProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=sse.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([__import__('sys').executable,__file__,'--binary',str(binary),'--cases','2048'],cwd=sse.ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'native SSE power mismatch'in run.stdout,(name,run.returncode,run.stdout[-3500:]);rejected.append(name)
    return rejected

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--binary',type=Path,default=sse.ROOT/'build/native/ug2crtpowersse_probe');parser.add_argument('--cases',type=int,default=8192);parser.add_argument('--mutation-checks',action='store_true');args=parser.parse_args()
    raw=sse.EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();sse.validate_manifest(raw,image)
    hardware=Hardware(args.binary);original=Original(image,hardware);callbacks=0
    try:
        for i,c in enumerate(cases(args.cases)):
            # Supply fixed caller-owned -1/+1 values, independently of scratch
            # registers. The emulated oracle's owner pattern is checked below.
            observed=original.run_power('sse_body',c)
            a=struct.pack('<QQ',c['bits'],c['other']);actual=hardware.request(8,c['control'],c['mxcsr'],c['status']&0x3f,a)
            state,expected80,thread,events=observed;cw,mx,flags=state[1];top,tag=state[2:4]
            native_cw,native_mx,native_sw,native_tag=struct.unpack_from('<4I',actual,28)
            assert actual[16:26].hex()==expected80,('native SSE power mismatch','return',i,c,expected80,actual[16:26].hex())
            assert(cw,mx,flags,top,tag)==(native_cw,native_mx,native_sw&0x3f,native_sw>>11,native_tag),('native SSE power mismatch','FP state',i,c,(cw,mx,flags,top,tag),(native_cw,native_mx,native_sw,native_tag))
            owners=struct.pack('<QH',0x8000000000000000,0xbfff)+struct.pack('<QH',0x8000000000000000,0x3fff)
            assert actual[44:64]==owners,('native SSE power mismatch','owners',i,c,actual[44:64].hex())
            errno,called=struct.unpack_from('<2I',actual,64);expected_errno=struct.unpack_from('<I',bytes.fromhex(thread),8)[0]
            expected_calls=[e for e in events if e[0]=='default'];assert errno==expected_errno and called==len(expected_calls),('native SSE power mismatch','errno/callback',i,c,errno,expected_errno,called,expected_calls)
            if called:
                callbacks+=called;e=expected_calls[-1];assert actual[72:104].hex()==e[1]and struct.unpack_from('<3I',actual,104)==e[2],('native SSE power mismatch','callback',i,c,e,actual[72:].hex())
    finally:hardware.close()
    report=dict(hardware_bodies=args.cases,callbacks=callbacks,original_instructions=dict(original.executed),physical_requests=dict(hardware.counts),failures=0,method='original control flow emulated; SSE arithmetic and x87 double conversions evaluated by independent physical instruction oracle; native source invoked separately',limits='masked hardware; default-zero matherr only; OS and unmasked exception delivery excluded; no byte identity')
    sse.BUILD.mkdir(parents=True,exist_ok=True)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks()
    (sse.BUILD/'verification-native.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
