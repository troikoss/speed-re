#!/usr/bin/env python3
"""Replay original secondary-wheel quad captures through native recovered code."""
import argparse,json,struct,subprocess
from pathlib import Path
import verify_pc_wheel_secondary as core

def run(binary,fixtures):
    return subprocess.run([str(binary),str(core.EXE),str(fixtures)],cwd=core.ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    native=core.ROOT/'build/native';raw=args.fixtures.read_bytes();assert raw[:8]==b'UG2WHQS1';count,=struct.unpack_from('<I',raw,8);assert len(raw)==12+count*128
    core.BUILD.mkdir(parents=True,exist_ok=True);baseline=run(native/'ug2wheelsecondary_probe',args.fixtures);assert baseline.returncode==0,baseline.stdout;print(baseline.stdout,end='')
    rejected=[]
    if args.mutation_checks:
        rotation=core.WORLD_DIR/'../Physics/WheelRotateY.cpp'
        variants={
            'rim-signedness':(core.SOURCE,'static_cast<signed char>(part->RimValue())','static_cast<unsigned char>(part->RimValue())'),
            'scale-order':(core.SOURCE,'rim)*ug2WheelSecondaryScale7A889C;\n    volatile long double size=scaledRim*ug2WheelSecondaryScale7A6728','rim)*ug2WheelSecondaryScale7A6728;\n    volatile long double size=scaledRim*ug2WheelSecondaryScale7A889C'),
            'sine-negative-rounding':(rotation,'m[2]=negativeSine;m[8]=sine','m[2]=-static_cast<float>(sine);m[8]=sine'),
            'quad-corner':(core.SOURCE,'quad.position00[2][0]=x2','quad.position00[2][0]=x3'),
            'caller-flag-gate':(core.SOURCE,'gate==6 ? 0x8000','gate==3 ? 0x8000'),
        }
        for name,(source,before,after)in variants.items():
            content=source.read_text();assert before in content
            path=core.BUILD/('mutation-native-'+name+'.cpp');path.write_text(content.replace(before,after,1));obj=path.with_suffix('.o');binary=path.with_suffix('.bin')
            subprocess.run(['g++','-m32','-O1','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-include',str(core.ROOT/'src/Platform/Native/NativeCompat.h'),'-I'+str(core.WORLD_DIR),'-I'+str(core.scalar.PHYSICS),'-c',str(path),'-o',str(obj)],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            subprocess.run(['g++','-m32','-no-pie',str(core.ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2wheelsecondary_probe.dir/src/Platform/Native/WheelSecondaryProbe.cpp.o'),str(obj),'-o',str(binary),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            tested=run(binary,args.fixtures);path.with_suffix('.log').write_text(tested.stdout);assert tested.returncode and 'native wheel secondary mismatch'in tested.stdout,(name,tested.returncode,tested.stdout);rejected.append(name)
    report=dict(exact_native_quads=count,failures=0,rejected_mutations=rejected,limits='original packing/GPU submission and texture lookup controlled; FP flags and byte identity not established; empty incoming x87 stack')
    (core.BUILD/'verification-native.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
