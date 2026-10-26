#!/usr/bin/env python3
"""Compare extracted wheel-input caller fragments with original machine code.

--chain includes recovered camera math, source builder, lighting and selection.
Original matrix copy and D3DX transpose remain the library/leaf boundaries.
"""
import argparse
from collections import Counter
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from pc_native_elf import link_native
from probe_pc_race_reward import ROOT, EXE
from verify_pc_car_render_light_build import selector_cases

WORLD = ROOT / 'src/Speed/Indep/Src/World'
SOURCE = WORLD / 'CarRenderInputs.cpp'
BUILD = ROOT / 'build/analysis/car-render-inputs'
MANIFEST = ROOT / 'config/SPEED2_EXE_1_2/car-render-inputs.json'
ARENA, SIZE, STACK, FRAME, STOP = 0x2000000, 0x12000, 0x2100000, 0x2108000, 0x211f000
GLOBAL, EYE, TARGET = 0x871b50, ARENA + 0x180, ARENA + 0x1a0
GLOBALS = {'ug2WheelReflectionBasis871B50': GLOBAL, 'ug2ReflectionTargetOffset803E50': 0x803e50,
           'ug2ReflectionUp800B70': 0x800b70, 'ug2LightNormalize82DAE4': 0x82dae4,
           'ug2LightTarget7F39B8': 0x7f39b8, 'ug2LightNormalize7F39BC': 0x7f39bc,
           'ug2LightTarget7F39C0': 0x7f39c0, 'ug2LightSectors830D14': 0x830d14,
           'ug2AdditionalLights835850': 0x835850}
ENTRIES = {'target': '_Z25CarRenderReflectionTarget', 'basis': '_Z24CarRenderReflectionBasis',
           'capture': '_Z24CarRenderCapturePosition', 'source': '_Z20CarRenderBuildSource'}
PREFIX = {'target': '?CarRenderReflectionTarget@@', 'basis': '?CarRenderReflectionBasis@@',
          'capture': '?CarRenderCapturePosition@@', 'source': '?CarRenderBuildSource@@'}
ORIG = {'look': 0x5ba9a0, 'copy': 0x4012b0, 'clear': 0x48ab10, 'build': 0x48ed60}
HELPERS = {'look': ('_Z12CameraLookAt', '?CameraLookAt@@'), 'clear': ('_Z20CarRenderClearRecord', '?CarRenderClearRecord@@'),
           'build': ('_Z25CarRenderBuildLightRecord', '?CarRenderBuildLightRecord@@')}


def build(source, gcc, chain):
    sources = [source]
    if chain:
        sources += [WORLD / name for name in ('CarRenderLightBuild.cpp', 'CarRenderLighting.cpp',
                    'CarRenderLightSelect.cpp', 'CarRenderLightTree.cpp', 'CarRenderRecord.cpp')]
        sources += [WORLD / '../Camera/CameraMatrix.cpp', WORLD / '../Physics/WheelMath.cpp']
    if gcc:
        bindings = dict(GLOBALS, D3DXMatrixTranspose=0x63d12c, D3DXMatrixMultiply=0x63cedb,
                        fengZero7D7D3C=0x7d7d3c, fengOne784250=0x784250, sqrt=STOP+0x100, sqrtl=STOP+0x100)
        bindings.update({'_Z14BodyMatrixCopyP11WheelMatrixS0_': ORIG['copy'],
                         '_Z15WheelSineCosinePfS_t': 0x43c390, '_Z10EngineSinef': 0x43c350})
        if not chain:
            bindings.update({'_Z12CameraLookAtP11WheelMatrixP11WheelVectorS2_S2_': ORIG['look'],
                             '_Z20CarRenderClearRecordP15CarRenderRecord': ORIG['clear'],
                             '_Z25CarRenderBuildLightRecordP15CarRenderRecordPK21CarRenderLightProfilePKfP11WheelMatrixS7_PK13CarRenderView': ORIG['build']})
        linked = link_native(sources, BUILD, ROOT, bindings, '_Z25CarRenderReflectionTargetPfPKf')
    else:
        BUILD.mkdir(parents=True, exist_ok=True)
        objects = []
        for i, path in enumerate(sources):
            obj = BUILD / ('inputs-' + str(i) + '.obj')
            subprocess.run([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe', '/nologo',
                            '/c', '/Ox', '/Ob0', '/EHsc', '/Isrc/Speed/Indep/Src/World',
                            '/Fo' + str(obj.relative_to(ROOT)), str(path.relative_to(ROOT))], cwd=ROOT, check=True)
            objects.append(obj)
        defined = {s['name'] for p in objects for s in read_object(p)['symbols'].values() if s['section'] > 0}
        bindings = {'_' + k: v for k, v in GLOBALS.items()}
        bindings.update({'__fltused': 0, '__ftol2': 0x75bed4, '_D3DXMatrixTranspose@8': 0x63d12c,
                         '_D3DXMatrixMultiply@12': 0x63cedb})
        prefixes = {'?BodyMatrixCopy@@': ORIG['copy'], '?WheelSineCosine@@': 0x43c390,
                    '?EngineSine@@': 0x43c350, '?fengZero7D7D3C@@': 0x7d7d3c, '?fengOne784250@@': 0x784250}
        if not chain: prefixes.update({v[1]: ORIG[k] for k, v in HELPERS.items()})
        for path in objects:
            for s in read_object(path)['symbols'].values():
                if s['section'] == 0 and s['name'] not in defined:
                    for prefix, address in prefixes.items():
                        if s['name'].startswith(prefix): bindings[s['name']] = address
        linked = link_objects(objects, bindings, start=0x2400000)
    linked['entries'] = {k: next(v for name, v in linked['exports'].items() if name.startswith(prefix))
                         for k, prefix in (ENTRIES if gcc else PREFIX).items()}
    linked['hooks'] = dict(ORIG)
    if chain:
        for kind, prefixes in HELPERS.items():
            linked['hooks'][kind] = next(v for name, v in linked['exports'].items() if name.startswith(prefixes[0 if gcc else 1]))
    return linked


class Machine:
    def __init__(self, image, linked=None):
        u = self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        u.mem_map(0x400000, (len(image) + 4095) & ~4095); u.mem_write(0x400000, image)
        u.mem_write(0x803f00, struct.pack('<I', 0x63d132))
        u.mem_map(ARENA, SIZE); u.mem_map(STACK, 0x20000)
        if linked:
            u.mem_map(linked['address'], linked['allocation'])
            for address, data in linked['blocks']: u.mem_write(address, data)
        self.linked = linked
        for kind, address in (linked['hooks'] if linked else ORIG).items():
            u.hook_add(UC_HOOK_CODE, self.hook, user_data=kind, begin=address, end=address)
        u.hook_add(UC_HOOK_CODE, self.hook, user_data='unexpected', begin=STOP+0x100, end=STOP+0x100)

    def hook(self, u, address, size, kind):
        assert kind != 'unexpected', 'unexpected square-root fallback'
        n = {'look':4, 'copy':2, 'clear':1, 'build':6}[kind]
        sp = u.reg_read(xc.UC_X86_REG_ESP)
        args = struct.unpack('<' + 'I'*n, bytes(u.mem_read(sp+4, n*4)))
        if kind == 'look':
            self.events.append((kind, tuple(bytes(u.mem_read(p, 12)) for p in args[1:])))
        elif kind == 'copy': self.events.append((kind, bytes(u.mem_read(args[1], 64))))
        else: self.events.append((kind, args))

    def run(self, kind, case, cw, index):
        data, args, _, _, globals_, heads = case
        args = list(args)
        if not args[1]:
            # The builder fixture includes null-output early returns. This caller
            # fragment always has a real profile and car-position argument.
            args[1], args[2], args[5] = ARENA+0x500, ARENA+0x300, 0
        u = self.u; self.events = []
        u.mem_write(ARENA, data)
        u.mem_write(FRAME, bytes([0x93]) * 0x1200)
        for a, value in globals_.items(): u.mem_write(a, struct.pack('<I', value))
        for a, value in heads.items(): u.mem_write(a, value)
        u.mem_write(GLOBAL, bytes(u.mem_read(ARENA+0x840, 64)))
        u.mem_write(EYE, struct.pack('<3f', 3.5, -2.5, 1.5))
        u.mem_write(TARGET, struct.pack('<3f', -1.5, 2.5, -0.5))
        frame = ARENA + 0xa00
        if kind == 'capture' and index % 2:
            frame = FRAME + 0x790 - 0x40 + (index % 4)*4
        if kind == 'target':
            destination, size = FRAME + 0x10, 12
            position = destination + (index % 3)*4 if index % 2 else args[2]
            arguments = [destination, position]
        elif kind == 'capture': destination, size, arguments = FRAME+0x790, 16, [FRAME+0x790, frame]
        elif kind == 'basis': destination, size, arguments = GLOBAL, 64, [GLOBAL, EYE, TARGET]
        else: destination, size, arguments = FRAME+0xb20, 0x1c0, [FRAME+0xb20, args[1], args[2], args[5]]
        u.reg_write(xc.UC_X86_REG_FPCW, cw); u.reg_write(xc.UC_X86_REG_FPSW, 0); u.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
        if self.linked:
            sp = FRAME - 0x100
            u.mem_write(sp, struct.pack('<'+'I'*(len(arguments)+1), STOP, *arguments))
            u.reg_write(xc.UC_X86_REG_ESP, sp)
            saved = (xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
            for j, r in enumerate(saved): u.reg_write(r, 0xabcdef00+j)
            u.emu_start(self.linked['entries'][kind], STOP, count=500000)
            assert u.reg_read(xc.UC_X86_REG_EIP) == STOP
            assert u.reg_read(xc.UC_X86_REG_ESP) == sp+4
            assert [u.reg_read(r) for r in saved] == [0xabcdef00+j for j in range(4)]
        else:
            u.reg_write(xc.UC_X86_REG_ESP, FRAME)
            if kind == 'target':
                u.reg_write(xc.UC_X86_REG_ESI, arguments[1]); start, end = 0x61ac46, 0x61ac6c
            elif kind == 'capture':
                u.mem_write(FRAME+0xd8, struct.pack('<I', frame)); start, end = 0x620f04, 0x620f33
            elif kind == 'basis':
                u.mem_write(FRAME, struct.pack('<4I', FRAME+0x30, EYE, TARGET, 0x800b70)); start, end = 0x5c3524, 0x5c3570
            else:
                u.reg_write(xc.UC_X86_REG_EBP, FRAME+0x1100)
                u.mem_write(FRAME+0x1108, struct.pack('<3I', args[5], args[1], args[2])); start,end = 0x620fcb, 0x620ff8
            u.emu_start(start, end, count=500000); assert u.reg_read(xc.UC_X86_REG_EIP) == end
            if kind == 'capture': u.emu_start(0x620f3a, 0x620f45)
        assert u.reg_read(xc.UC_X86_REG_FPCW) == cw
        assert ((u.reg_read(xc.UC_X86_REG_FPSW)>>11)&7) == 0
        assert u.reg_read(xc.UC_X86_REG_FPTAG) == 0xffff
        return bytes(u.mem_read(destination,size)), bytes(u.mem_read(ARENA,SIZE)), self.events


def verify(image, linked, n, kinds):
    a,b = Machine(image),Machine(image,linked); totals=Counter()
    for index,case in enumerate(selector_cases(n)):
        for kind in kinds:
            for cw in (0x027f,0x037f):
                x,y=a.run(kind,case,cw,index),b.run(kind,case,cw,index)
                if x!=y:
                    offsets=[hex(j) for j in range(0,len(x[0]),4) if x[0][j:j+4]!=y[0][j:j+4]][:8]
                    raise AssertionError(('input fragment mismatch',kind,index,hex(cw),offsets,x[2] if x[2]!=y[2] else [],y[2] if x[2]!=y[2] else []))
                totals[kind]+=1
    return dict(totals)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--gcc',action='store_true');p.add_argument('--chain',action='store_true')
    p.add_argument('--cases',type=int,default=2048);p.add_argument('--replace',type=Path);p.add_argument('--kind',choices=['all']+list(ENTRIES),default='all')
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    assert args.cases > 0
    raw=EXE.read_bytes();image=pefile.PE(data=raw).get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text())
    assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for f in manifest['fragments'].values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    linked=build(args.replace or SOURCE,args.gcc,args.chain);kinds=list(ENTRIES) if args.kind=='all'else[args.kind]
    report=dict(comparisons=verify(image,linked,args.cases,kinds),failures=0,chain=args.chain,
                compiler='GCC i386 C++11 -O1' if args.gcc else 'MSVC 7.1 /Ox /Ob0',byte_identity='not established',
                limitations=['extracted fragments; not the full render-manager/car-draw routines',
                             'finite fixtures; original matrix copy and D3DX transpose execute on both sides'])
    if args.mutation_checks:
        mutations={'wrong-target-height':('target','ug2ReflectionTargetOffset803E50[2]','ug2ReflectionTargetOffset803E50[1]'),
                   'omit-basis-translation-clear':('basis','basis.elements[12] = 0.0f;',''),
                   'wrong-basis-fourth':('basis','basis.elements[15] = 1.0f;','basis.elements[15] = 0.0f;'),
                   'wrong-position-w':('capture','destination[3] = 0x3f800000;','destination[3] = 0;'),
                   'wrong-position-lane':('capture','destination[1] = source[1];','destination[1] = source[2];'),
                   'omit-source-clear':('source','CarRenderClearRecord(output);',''),
                   'wrong-source-transform':('source','carPosition, 0, &ug2WheelReflectionBasis871B50','carPosition, &ug2WheelReflectionBasis871B50, &ug2WheelReflectionBasis871B50')}
        rejected=[];body=SOURCE.read_text()
        for label,(kind,before,after)in mutations.items():
            if kind not in kinds:continue
            assert before in body;path=BUILD/('mutation-'+label+'.cpp');path.write_text(body.replace(before,after))
            result=subprocess.run([sys.executable,__file__,'--replace',str(path),'--kind',kind,'--cases','128']+
                                  (['--gcc']if args.gcc else[])+(['--chain']if args.chain else[]),text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            (BUILD/('mutation-'+label+'.log')).write_text(result.stdout)
            assert result.returncode and 'input fragment mismatch'in result.stdout,(label,result.stdout)
            rejected.append(label)
        report['rejected_mutations']=rejected
    label=('gcc'if args.gcc else'msvc')+('-chain'if args.chain else'-isolated')+'-'+args.kind
    (BUILD/('verification-'+label+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
