#!/usr/bin/env python3
"""Replay original effect pass captures through native recovered ABI/ownership code."""
import argparse,json,re,resource,subprocess
from pathlib import Path
import verify_pc_effect_pass as core

def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def run(binary,fixtures):return subprocess.run([str(binary),str(core.EXE),str(fixtures)],cwd=core.ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,preexec_fn=no_core)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    native=core.ROOT/'build/native';binary=native/'ug2effectpass_probe';baseline=run(binary,args.fixtures);assert baseline.returncode==0,baseline.stdout;print(baseline.stdout,end='')
    rejected=[]
    if args.mutation_checks:
        names=('end-queue-bit','end-snapshot-count','commit-ignore-control','commit-return-on-error','begin-clobber-upper-word','begin-publish-failed-pass','group-narrow-forwarded-index','group-ignore-state-four-errors','deferred-texture-tag','texture-reference-kind','stale-deferred-tail','unmapped-texture-cache','wrong-resolver-create','no-texture-notification')
        for name in names:
            kind,before,after=core.MUTATIONS[name];content=core.SOURCE.read_text();assert before in content;content=content.replace(before,after,1)
            if name=='stale-deferred-tail':content=content.replace('reinterpret_cast<D3DXEffectValuePrefix *>(valueFC)->value18','reinterpret_cast<D3DXEffectValuePrefix *>(oldTail)->value18')
            path=core.BUILD/('mutation-native-'+name+'.cpp');path.write_text(content);obj=path.with_suffix('.o');mutant=path.with_suffix('.bin')
            subprocess.run(['g++','-m32','-O1','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-include',str(core.ROOT/'src/Platform/Native/NativeCompat.h'),'-I'+str(core.LIB),'-c',str(path),'-o',str(obj)],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            objects=[native/'CMakeFiles/ug2effectpass_probe.dir/src/Platform/Native/EffectPassProbe.cpp.o']+[native/('CMakeFiles/ug2pass_'+s+'_probe.dir/src/Speed/Indep/Libs/D3DX/Effect'+s+'.cpp.o')for s in('Setup','Handles','Values')]+[obj]
            subprocess.run(['g++','-m32','-no-pie',str(core.ROOT/'src/Platform/Native/OriginalSymbols.ld'),*map(str,objects),'-o',str(mutant),'-Wl,--wrap=UG2FxMalloc','-Wl,--wrap=UG2FxFree','-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            checked=run(mutant,args.fixtures);path.with_suffix('.log').write_text(checked.stdout);assert checked.returncode and 'native effect pass mismatch'in checked.stdout,(name,checked.returncode,checked.stdout);rejected.append(name)
    matched=re.search(r'native effect pass: (\d+) exact original captures, (\d+) controlled state calls',baseline.stdout);assert matched,baseline.stdout
    report=dict(original_captures=int(matched[1]),controlled_state_calls=int(matched[2]),failures=0,rejected_mutations=rejected,limits='synthetic graphs and allocation outcomes; state/preparation services controlled; defined x87 control bits compare with reserved bit 6 excluded; no pixel rendering or byte identity')
    (core.BUILD/'verification-native.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
