#!/usr/bin/env python3
"""Original wheel block versus native GCC block with real record preparation.

The actual native UG2WheelVisible definition is compiled with recovered record
producers. Renderer, slot and secondary-draw callbacks retain the existing I/O
fixtures; analysis-only adaptation accounts for GCC's member-call ABI.
"""
import argparse
import hashlib
import json
import random
import struct
import pefile
from unicorn import UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE
import verify_pc_wheel_draw as wheel

BUILD=ROOT/'build/analysis/native-wheel-records'


class Machine(wheel.Machine):
    def __init__(self,image,linked=None):
        super().__init__(image,linked)
        self.gcc=linked is not None
        self.uc.mem_write(0x803f00,struct.pack('<I',0x63d132))
        self.uc.reg_write(xc.UC_X86_REG_FPTAG,0xffff)
        self.uc.reg_write(xc.UC_X86_REG_FPSW,0)
        if linked:
            address=linked['exports']['UG2WheelVisible']
            self.uc.hook_add(UC_HOOK_CODE,self._stub,user_data=0x48f660,begin=address,end=address)

    def _stub(self,u,address,size,name):
        if name==0x48f660:
            args=self._args(u,6)
            assert args[3] in (wheel.S_FRAME+0x790,wheel.RENDER_POSITION)
            assert args[5] in (wheel.S_FRAME+0xb20,wheel.SOURCE_RECORD)
            self.events.append(('Visible',args,bytes(u.mem_read(args[3],16)),bytes(u.mem_read(args[5],0x1c0))))
            return  # Original/recovered preparation executes, including every record write.
        if self.gcc and name in (0x5c5930,0x48d860,0x620600):
            # GNU cdecl(this,args) -> the existing I/O fixture's thiscall view.
            # Only observation/cleanup changes; no game input or arithmetic changes.
            sp=u.reg_read(xc.UC_X86_REG_ESP)
            ret,receiver=struct.unpack('<2I',bytes(u.mem_read(sp,8)))
            u.mem_write(sp+4,struct.pack('<I',ret));u.reg_write(xc.UC_X86_REG_ESP,sp+4)
            u.reg_write(xc.UC_X86_REG_ECX,receiver)
            super()._stub(u,address,size,name)
            u.mem_write(sp+4,struct.pack('<I',receiver));u.reg_write(xc.UC_X86_REG_ESP,sp+4)
        else:super()._stub(u,address,size,name)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cases',type=int,default=1024);args=p.parse_args()
    assert args.cases > 0
    BUILD.mkdir(parents=True,exist_ok=True)
    raw=EXE.read_bytes();manifest=wheel.load_manifest();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    image=pefile.PE(data=raw).get_memory_mapped_image()
    for f in manifest['functions'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    native=(ROOT/'src/Platform/Native/NativeCarWheels.cpp').read_text()
    begin=native.index('extern "C" unsigned int UG2WheelVisible(');end=native.index('\n}',begin)+2
    adapter=BUILD/'visible.cpp';adapter.write_text('#include "CarRenderRecord.h"\n'+native[begin:end]+'\n')
    world=ROOT/'src/Speed/Indep/Src/World'
    sources=[world/name for name in ('CarRenderWheels.cpp','CarRenderRecordPrepare.cpp',
                                    'CarRenderRecord.cpp','CarRenderRecordTransform.cpp')]+[adapter]
    bindings={k:v for k,v in wheel.EXTERNALS.items()if not k.startswith('?')and not k.startswith('__')and k!='UG2WheelVisible'}
    bindings.update({'_Z14BodyMatrixCopyP11WheelMatrixS0_':0x4012b0,'_Z13RotateMatrixXP11WheelMatrixS0_j':0x5bacf0,
                     '_Z13RotateMatrixZP11WheelMatrixS0_j':0x5bae60,'_ZN13CarRenderPart10FindRecordEjPv':0x60fef0,
                     '_ZN13CarRenderView9DrawSolidEP14CarRenderSolidP11WheelMatrixPvjj':0x5c5930,
                     '_ZN14CarRenderSolid7SetSlotEjj':0x48d860,
                     '_ZN18CarRenderWheelInfo8Fn620600EP13CarRenderViewP11WheelMatrix':0x620600,
                     'D3DXMatrixTranspose':0x63d12c,'sqrtl':wheel.STOP+0x300})
    linked=link_native(sources,BUILD,ROOT,bindings,'_Z12CarWheelDrawP18CarWheelDrawInputs')
    entry=linked['exports']['_Z12CarWheelDrawP18CarWheelDrawInputs']
    rng=random.Random(0x62700e);counts={'cases':0,'events':0,'prepared_records':0};precision={}
    for index in range(args.cases):
        case=wheel.random_case(rng);blobs=wheel.build_fixture(case,random.Random(index))
        original,rebuilt=Machine(image),Machine(image,linked)
        wheel.write_case_memory(original,blobs,case,None);wheel.write_case_memory(rebuilt,blobs,case,None)
        wheel.run_block(original,case);wheel.run_rebuilt_block(rebuilt,case,entry)
        counts['events']+=wheel.compare_block(original,rebuilt,case)
        counts['prepared_records']+=sum(e[0]=='Visible'for e in original.events)
        counts['cases']+=1;precision[hex(case['fpcw'])]=precision.get(hex(case['fpcw']),0)+1
    report=dict(comparisons=counts,precision_cases=precision,failures=0,compiler='native GCC i386 C++11 -O1',
                native_visibility_definition_sha256=hashlib.sha256(native[begin:end].encode()).hexdigest(),
                byte_identity='not established',limitations=['finite fixtures, not exhaustive floating-point coverage',
                    'renderer/slot/facing/secondary-draw and allocation fixtures remain; D3DX multiply fixture unchanged',
                    'original preparation versus recovered native preparation; known separate legacy transform precision gap unchanged'])
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
