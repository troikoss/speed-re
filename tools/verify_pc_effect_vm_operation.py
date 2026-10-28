#!/usr/bin/env python3
"""Compare original table-driven effect VM operation 0x63f2b5 and its callers."""
import argparse
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
import unicorn.x86_const as xc
import verify_pc_effect_vm as vm

md=vm.md;ROOT,EXE,LIB=vm.ROOT,vm.EXE,vm.LIB
SOURCE=LIB/'EffectVmOperation.cpp';BUILD=ROOT/'build/analysis/effect-vm-operation'
MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-vm-operation.json'
md.BUILD=BUILD;md.FREE['operation']='D3DXEffectVmOperation63F2B5';md.STDCALL['operation']=4


def cases(n):
    rng=random.Random(0x63f2b5)
    fixed=(0,0x80000000,0x3f800000,0xbf800000,0x3f000000,0xbf000000,0x461c4000,0xc61c4000,0x7f800000,0xff800000,0x7fc12345,0x7f812345,1,0x80000001,0x7f7fffff,0xff7fffff)
    for i in range(n):
        if i<256:values=tuple(struct.unpack('<I',struct.pack('<f',(rng.randrange(-200000,200000))/16.0))[0]for j in range(3))
        elif i<512:values=tuple(struct.unpack('<I',struct.pack('<f',rng.uniform(-1.0,1.0)))[0]for j in range(3))
        elif i<768:
            values=tuple((rng.randrange(94,115)<<23)|rng.getrandbits(23)if j==i%3 else struct.unpack('<I',struct.pack('<f',rng.uniform(-1.0,1.0)))[0]for j in range(3))
        elif i<4864:values=tuple(fixed[((i-768)//(16**j))%16]for j in range(3))
        elif i<5376:
            edge=(0xc61c3fff,0xc61c4000,0xc61c4001,0x461c3fff,0x461c4000,0x461c4001,0x4f000000,0xcf000000,0x4f800000,0xcf800000,0x5f000000,0xdf000000)
            values=tuple(edge[(i+j*5)%len(edge)]if j==i%3 else struct.unpack('<I',struct.pack('<f',rng.uniform(-1.0,1.0)))[0]for j in range(3))
        else:values=tuple(rng.getrandbits(32)for j in range(3))
        yield dict(bits=values,precision=(0x7f,0x27f,0x37f)[i%3],rounding=(i//3%4)*0x400)


class Machine(vm.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries['operation']=0x63f2b5
        self.put(0,0x12345678)
    def math_bridge(self,u,address,size,info):
        if info[0]!='operation':super().math_bridge(u,address,size,info)
    def math_entry(self,u,address,size,info):
        if info[0]!='operation':super().math_entry(u,address,size,info)
    def run_operation(self,c):
        self.control=c['precision']|c['rounding'];self.u.mem_write(vm.DATA,struct.pack('<3I',*c['bits']))
        self.call('operation',[vm.DATA]);u=self.u
        top=(u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7;tag=u.reg_read(xc.UC_X86_REG_FPTAG)
        live=tuple(u.reg_read(xc.UC_X86_REG_FP0+j)for j in range(8)if(tag>>(j*2)&3)!=3)
        return bytes(u.mem_read(vm.DATA,12)),top,tag,live


def mutation_checks(gcc):
    mutations={
        'bias':('+ug2VmOperationBias784500','+ug2VmOperationOne784250'),
        'signed-fraction':('x=sumX-integerX','x=sumX-static_cast<unsigned int>(integerX)'),
        'integer-width':('static_cast<OperationInteger>','static_cast<int>'),
        'x-mask':('static_cast<unsigned int>(integerX)&255','static_cast<unsigned int>(integerX)&127'),
        'y-mask':('static_cast<unsigned int>(integerY)&255','static_cast<unsigned int>(integerY)&127'),
        'z-mask':('static_cast<unsigned int>(integerZ)&255','static_cast<unsigned int>(integerZ)&127'),
        'next-x':('nextX=(ix+1)&255','nextX=(ix+2)&255'),
        'next-y':('nextY=(iy+1)&255','nextY=(iy+2)&255'),
        'next-z':('nextZ=(iz+1)&255','nextZ=(iz+2)&255'),
        'permutation-x':('Permutation7A9460[ix]','Permutation7A9460[iy]'),
        'permutation-nesting':('Permutation7A9460[p0+iy]','Permutation7A9460[iy]'),
        'permutation-next-row':('Permutation7A9460[p1+nextY]','Permutation7A9460[p1+iy]'),
        'gradient-corner':('Gradients7A9C68[p10+iz]','Gradients7A9C68[p00+iz]'),
        'gradient-next-plane':('Gradients7A9C68[p00+nextZ]','Gradients7A9C68[p00+iz]'),
        'x-extended-lifetime':('xMinus*g[0]','static_cast<long double>(xMinusStored)*g[0]'),
        'x-final-reload':('static_cast<long double>(xMinusStored)*g[0];\n    const long double difference','xMinus*g[0];\n    const long double difference'),
        'y-narrowing':('volatile float yMinus','volatile long double yMinus'),
        'z-narrowing':('volatile float zStored=z,zMinus=','volatile long double zStored=z;volatile float zMinus='),
        'z-minus-narrowing':('volatile float zStored=z,zMinus=','volatile float zStored=z;volatile long double zMinus='),
        'x-weight-narrowing':('volatile float weightX','volatile long double weightX'),
        'y-weight-narrowing':('volatile float weightY','volatile long double weightY'),
        'first-plane-narrowing':('volatile float plane0','volatile long double plane0'),
        'upper-row-narrowing':('volatile float row01','volatile long double row01'),
        'x-weight-polynomial':('(x+x)','(x+y)'),
        'y-weight-polynomial':('(y+y)','(x+y)'),
        'z-weight-polynomial':('(static_cast<long double>(zStored)+zStored)','(static_cast<long double>(zStored)+y)'),
        'gradient-x-component':('x*g[0]','x*g[1]'),
        'gradient-y-component':('y*g[1]','y*g[2]'),
        'gradient-z-component':('z*g[2]','z*g[0]'),
        'first-blend-direction':('(dot100-dot000)*weightX+dot000','(dot000-dot100)*weightX+dot100'),
        'plane-blend-weight':('row00)*weightY+row00','row00)*weightX+row00'),
        'final-plane':('+row01-plane0;','+row01-row01;'),
        'final-scale':('*ug2VmOperationScale785110;','*ug2VmOperationOne784250;'),
    }
    if not gcc:del mutations['integer-width'] # Same _ftol2 low word in MSVC.
    rejected=[];content=SOURCE.read_text()
    for name,(before,after)in mutations.items():
        assert before in content,(name,before)
        path=BUILD/('mutation-'+('gcc-'if gcc else'msvc-')+name+'.cpp');path.write_text(content.replace(before,after,-1 if name in('integer-width','x-extended-lifetime')else 1))
        run=subprocess.run([sys.executable,__file__,'--cases','8192','--replace',str(path)]+(['--gcc']if gcc else[]),cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'effect VM operation mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
    return rejected


def native_replay(image,path):
    raw=path.read_bytes();assert raw[:8]==b'UG2FXOD1';count,=struct.unpack_from('<I',raw,8);at=12;a=Machine(image)
    for i,c in enumerate(cases(count)):
        x=a.run_operation(c);top,tag,live=x[1:];assert len(live)==1
        expected=struct.pack('<4I',i,c['precision']|c['rounding'],(top+1)&7,0xffff)+struct.pack('<QH',*live[0])+struct.pack('<3I',*c['bits'])
        actual=raw[at:at+len(expected)];at+=len(expected);assert actual==expected,('native effect VM operation mismatch',i,c,expected.hex(),actual.hex())
    assert at==len(raw);return count


def native_mutation_checks():
    native=ROOT/'build/native';bridge=ROOT/'src/Platform/Native/NativeFxVmCrtPlatform.cpp'
    fixture=BUILD/'native-mutation-fixtures.bin';fixture.write_bytes(b'UG2FXOI1'+struct.pack('<I',8192)+b''.join(struct.pack('<5I',i,c['precision']|c['rounding'],*c['bits'])for i,c in enumerate(cases(8192))))
    mutations={
        'first-plane-narrowing':(SOURCE,'volatile float plane0','volatile long double plane0'),
        'x-final-reload':(SOURCE,'static_cast<long double>(xMinusStored)*g[0];\n    const long double difference','xMinus*g[0];\n    const long double difference'),
        'z-weight':(SOURCE,'(static_cast<long double>(zStored)+zStored)','(static_cast<long double>(zStored)+y)'),
        'integer-width':(SOURCE,'static_cast<OperationInteger>','static_cast<int>'),
        'bridge-return-narrowing':(bridge,'return D3DXEffectVmOperation63F2B5(values);','volatile double narrowed=D3DXEffectVmOperation63F2B5(values);return narrowed;'),
    }
    rejected=[]
    for name,(unit,before,after)in mutations.items():
        content=unit.read_text();assert before in content
        if unit==bridge:content=content.replace('"../../Speed/Indep/Libs/D3DX/EffectVm.h"','"EffectVm.h"')
        path=BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
        subprocess.run(['g++','-m32','-std=c++11','-O1','-fno-pie','-mfpmath=387','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-fno-stack-protector','-I'+str(LIB),'-include',str(ROOT/'src/Platform/Native/NativeCompat.h'),'-c',str(path),'-o',str(obj)],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        subprocess.run(['g++','-m32','-no-pie',str(ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2effectvmoperation_probe.dir/src/Platform/Native/EffectVmOperationProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        run=subprocess.run([str(binary),str(EXE),str(fixture),str(BUILD/'native-mutation-capture.bin')],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
        assert run.returncode and 'hardware VM operation mismatch'in run.stdout,(name,run.stdout[-2500:]);rejected.append(name)
    return rejected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--cases',type=int,default=16384);p.add_argument('--replace',type=Path);p.add_argument('--write-native-fixtures',type=Path);p.add_argument('--native-dump',type=Path);p.add_argument('--mutation-checks',action='store_true')
    p.add_argument('--native-mutation-checks',action='store_true');args=p.parse_args()
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in[*manifest['functions'].values(),*manifest['data'].values()]:
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc,reuse_dependencies=bool(args.replace));a,b=Machine(image),Machine(image,linked,args.gcc)
    for i,c in enumerate(cases(args.cases)):
        x=a.run_operation(c);y=b.run_operation(c)
        assert x==y,('effect VM operation mismatch',i,c,x,y)
    report=dict(comparisons=args.cases,failures=0,compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if not args.replace:
        counts={}
        for kind in('operation502','vm'):
            for i,c in enumerate(vm.cases(256)):
                c.update(controlled=False,opcode=0x50200000,broadcast=False,input_kind=3,input_index=0,output_kind=4,output_index=0,mask=255,terms=0,alias=(-0x3000,-0x2ffc,-0x3004,-0x2ff8,0x1000)[i%5],literal_count=256,empty=False)
                x=a.run(kind,c);y=b.run(kind,c);assert x==y,('effect VM operation mismatch',kind,i)
            counts[kind]=i+1
        report['composed_callers']=counts
    if args.write_native_fixtures:
        content=b'UG2FXOI1'+struct.pack('<I',args.cases)
        for i,c in enumerate(cases(args.cases)):content+=struct.pack('<5I',i,c['precision']|c['rounding'],*c['bits'])
        args.write_native_fixtures.write_bytes(content)
    if args.native_dump:
        report['native_replay']=native_replay(image,args.native_dump)
    if args.mutation_checks:report['rejected_mutations']=mutation_checks(args.gcc)
    if args.native_mutation_checks:report['rejected_native_mutations']=native_mutation_checks()
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
