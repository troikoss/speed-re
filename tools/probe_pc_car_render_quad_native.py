#!/usr/bin/env python3
"""Replay original quad traces through native recovered code and API fixtures."""
import argparse,json,re,resource,subprocess
from pathlib import Path
import verify_pc_car_render_quad as core

def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def run(binary,fixtures,mode=None):
    return subprocess.run([str(binary),str(core.EXE),str(fixtures)]+([mode]if mode else[]),cwd=core.ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,preexec_fn=no_core)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('fixtures',type=Path);p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    native=core.ROOT/'build/native';binary=native/'ug2quadsubmit_probe';baseline=run(binary,args.fixtures);assert baseline.returncode==0,baseline.stdout;print(baseline.stdout,end='')
    boundaries=[]
    for mode,message in(('missing-api','unimplemented native renderer API: UG2RenderSetTexture'),):
        checked=run(binary,args.fixtures,mode);assert checked.returncode==-6 and message in checked.stdout,(mode,checked.returncode,checked.stdout);boundaries.append(mode)
    rejected=[]
    if args.mutation_checks:
        variants={
            'zero-overlay-handle':('if (extra) {','if (extra&&ug2RenderTechnique870910->parameters1C->slots00[10].handle24) {'),
            'colour-order':('(colour&0xff000000)|((colour&0xff)<<16)|(colour&0xff00)|((colour>>16)&0xff)','colour'),
            'snapshot-after-bind':('    CarRenderBindTexture(texture);',''),
            'stale-technique':('UG2EffectCommit(ug2RenderTechnique870910->effect20);\n        CarRenderTechnique *afterCommit=ug2RenderTechnique870910;','CarRenderTechnique *afterCommit=ug2RenderTechnique870910;\n        UG2EffectCommit(afterCommit->effect20);'),
            'cache-clear':('ug2RenderVertex87092C=0;',''),
            'early-mode-check':('    unsigned int vertices[4][8];','    if(!ug2SolidViewMode870848) return;\n    unsigned int vertices[4][8];'),
            'counter-wrap':('++ug2RenderQuadCalls872F08;','if(ug2RenderQuadCalls872F08!=0xffffffff) ++ug2RenderQuadCalls872F08;'),
            'ignore-error-return':('UG2EffectCommit(ug2RenderTechnique870910->effect20);','if(UG2EffectCommit(ug2RenderTechnique870910->effect20)<0) return;'),
            'floating-copy':('vertices[j][0]=positions[j*4];','volatile double value=quad->position00[j][0];volatile float narrowed=value;*reinterpret_cast<float *>(&vertices[j][0])=narrowed;'),
        }
        for name,(before,after)in variants.items():
            content=core.SOURCE.read_text();assert before in content;content=content.replace(before,after,1)
            if name=='snapshot-after-bind':content=content.replace('    unsigned int vertices[4][8];','    CarRenderBindTexture(texture);\n    unsigned int vertices[4][8];')
            path=core.BUILD/('mutation-native-'+name+'.cpp');path.write_text(content);obj=path.with_suffix('.o');mutant=path.with_suffix('.bin')
            subprocess.run(['g++','-m32','-O1','-fno-strict-aliasing','-fno-fast-math','-ffp-contract=off','-include',str(core.ROOT/'src/Platform/Native/NativeCompat.h'),'-I'+str(core.WORLD),'-c',str(path),'-o',str(obj)],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            objects=[native/'CMakeFiles/ug2quadsubmit_probe.dir/src/Platform/Native/CarRenderQuadProbe.cpp.o',native/'CMakeFiles/ug2quad_material_probe.dir/src/Speed/Indep/Src/World/CarRenderMaterial.cpp.o',native/'CMakeFiles/ug2quad_world_probe.dir/src/Speed/Indep/Src/World/CarRenderUploads.cpp.o',obj]
            subprocess.run(['g++','-m32','-no-pie',str(core.ROOT/'src/Platform/Native/OriginalSymbols.ld'),*map(str,objects),'-o',str(mutant),'-Wl,--start-group',str(native/'libug2native.a'),str(native/'libug2recovered.a'),'-Wl,--end-group','-lm'],cwd=core.ROOT,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            tested=run(mutant,args.fixtures);path.with_suffix('.log').write_text(tested.stdout);assert tested.returncode and 'native quad submission mismatch'in tested.stdout,(name,tested.returncode,tested.stdout);rejected.append(name)
    summary=re.search(r'native quad submission: (\d+) exact original captures \((\d+) composed\), (\d+) observed engine/API calls, (\d+) counter/FP-state checks',baseline.stdout);assert summary,baseline.stdout
    captured,composed,calls,counters=map(int,summary.groups())
    report=dict(original_captures=captured,composed_captures=composed,counter_fp_checks=counters,observed_engine_api_calls=calls,failures=0,rejected_mutations=rejected,visible_boundaries=boundaries,limits='synthetic frame/texture contexts; graphics APIs controlled; indeterminate vertex words excluded; composed x87 flags not established; no pixel rendering or byte identity')
    (core.BUILD/'verification-native.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
