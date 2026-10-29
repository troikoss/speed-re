#!/usr/bin/env python3
"""Replay original state-cache traces through native recovered code and cache ABI bridges."""
import argparse,hashlib,json,re,resource,subprocess
from pathlib import Path
import verify_pc_effect_state_cache as core

def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def run(binary,fixtures):return subprocess.run([str(binary),str(core.EXE),str(fixtures)],cwd=core.ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,preexec_fn=no_core)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    assert hashlib.sha256(core.EXE.read_bytes()).hexdigest()==json.loads(core.MANIFEST.read_text())['executable_sha256']
    native=core.ROOT/'build/native';binary=native/'ug2effectstatecache_probe';baseline=run(binary,args.fixtures);assert baseline.returncode==0,baseline.stdout;print(baseline.stdout,end='');rejected=[]
    if args.mutation_checks:
        names=('gate-ignore-enabled-bit','gate-shrink-capacity','gate-ignore-retained-shader','gate-write-after-failure','gate-shrink-sampler-count','capture-one-block','capture-no-mute-restore','capture-no-failed-end','capture-use-failed-end-result','capture-skip-old-apply','capture-wrong-restore-block','capture-release-published-block','capture-stale-manager','capture-stale-old-block','zero-buffer-no-initialize','bulk-vertex-booleans','vertex-sampler-base','vertex-sampler-missing-default','pixel-sampler-extra-default','boolean-one-instead-of-mask')
        for name in names:
            kind,path=core.mutation_source(name,True);obj=path.parent/'native.o';mutant=path.parent/'native.bin'
            subprocess.run(['g++','-m32','-O1','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-include',str(core.ROOT/'src/Platform/Native/NativeCompat.h'),'-I'+str(core.LIB),'-c',str(path),'-o',str(obj)],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            subprocess.run(['g++','-m32','-no-pie',str(core.ROOT/'src/Platform/Native/OriginalSymbols.ld'),str(native/'CMakeFiles/ug2effectstatecache_probe.dir/src/Platform/Native/EffectStateCacheProbe.cpp.o'),str(obj),'-o',str(mutant),'-Wl,--wrap=UG2FxMalloc','-Wl,--wrap=UG2FxFree','-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            checked=run(mutant,args.fixtures);(path.parent/'native-result.log').write_text(checked.stdout);assert checked.returncode and 'native state cache mismatch'in checked.stdout,(name,checked.returncode,checked.stdout);rejected.append(name)
    matched=re.search(r'native state cache: (\d+) exact original captures, (\d+) controlled GPU calls',baseline.stdout);assert matched,baseline.stdout
    report=dict(original_captures=int(matched[1]),controlled_gpu_calls=int(matched[2]),failures=0,rejected_mutations=rejected,limits='synthetic devices/state blocks; GPU and debug-mute services controlled; bounded allocation outcomes; incoming x87 values/control/status preserved; no rendered parity or byte identity')
    (core.BUILD/'verification-native.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
