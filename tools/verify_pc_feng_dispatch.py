#!/usr/bin/env python3
"""Verify full response dispatch and queue-state helpers against UG2 v1.2."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from pc_feng_response import link_response, BOUNDARIES
from probe_pc_race_reward import ROOT, EXE

BUILD = ROOT / 'build/analysis/feng-dispatch'
ARENA, STOP = 0x2000000, 0x2100000
QUEUE, RESPONSE, OBJECT, PACKAGE, COMMANDS, SCRIPTS, STACK = [ARENA+x for x in (0x10000,0x1000,0x2000,0x4000,0x5000,0x3000,0x1f000)]
PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
PREFIXES = {'ExecuteResponse':'?ExecuteResponse@FEngQueue@', 'StoreFocus':'?StoreFocus@FEngQueue@',
            'LoadFocus':'?LoadFocus@FEngQueue@', 'ClearPackageStack':'?ClearPackageStack@FEngQueue@',
            'FindScriptByHash':'?FindScriptByHash@HudVisibilityObject@'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc','clang-cl'), default='msvc')
    parser.add_argument('--source', type=Path, default=ROOT/'src/Speed/Indep/Src/Frontend/FEngResponse.cpp')
    parser.add_argument('--state-source', type=Path, default=ROOT/'src/Speed/Indep/Src/Frontend/FEngQueueState.cpp')
    parser.add_argument('--mutation-checks', action='store_true')
    args = parser.parse_args()
    manifest = json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-response.json').read_text())
    raw = EXE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values())+list(manifest['tables'].values()):
        offset = int(f['address'],16)-0x400000
        assert hashlib.sha256(image[offset:offset+f['size']]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)
    linked = link_response(args.compiler,args.source,args.state_source,BUILD)
    entries = {n:next(a for symbol,a in linked['exports'].items() if symbol.startswith(prefix)) for n,prefix in PREFIXES.items()}
    state, machines = {}, []
    def words(uc,address,count=1): return struct.unpack('<'+'I'*count,uc.mem_read(address,4*count))
    boundaries = {a:(n,c) for n,(a,c) in BOUNDARIES.items()}
    for rebuilt in (False,True):
        uc = Uc(UC_ARCH_X86,UC_MODE_32)
        uc.mem_map(0x400000,(len(image)+4095)&~4095);uc.mem_write(0x400000,image)
        uc.mem_map(ARENA,0x20000);uc.mem_map(STOP,0x1000)
        uc.mem_map(linked['address'],linked['allocation'])
        for address,body in linked['blocks']: uc.mem_write(address,body)
        ranges = linked['ranges'] if rebuilt else [(int(f['address'],16),int(f['address'],16)+f['size']) for f in manifest['functions'].values()]
        def hook(uc,address,size,user,ranges=ranges):
            if address not in boundaries:
                assert any(a<=address<b for a,b in ranges),('unexpected execution',hex(address))
                return
            name,nargs = boundaries[address]
            sp = uc.reg_read(UC_X86_REG_ESP);this = uc.reg_read(UC_X86_REG_ECX)
            ret,*arguments = words(uc,sp,nargs+1)
            normalized = list(arguments)
            if name=='SetFocus': normalized[2] &= 255
            state['trace'].append((name,this,normalized))
            result = 0
            if name=='FindObjectByGuid':
                result = OBJECT if arguments[0] in (1,0xffffffff) else OBJECT+0x100 if arguments[0]==2 else 0
            if name=='SetScript':
                uc.mem_write(this+0x54,struct.pack('<I',arguments[0]))
                # The call boundary may change selection after assignment; the dispatcher resets its local script.
                if state['mutation'] & 1: uc.mem_write(this+0x54,struct.pack('<I',SCRIPTS+0x80))
            if name=='SetFocus': uc.mem_write(this+0x8c,struct.pack('<I',arguments[1]))
            if state['mutation'] & 2 and len(state['trace'])==1:
                uc.mem_write(RESPONSE+0x10,bytes(4))  # cached count must survive
                uc.mem_write(RESPONSE+0x14,struct.pack('<I',COMMANDS+0x800))
            if state['mutation'] & 4 and name=='FindObjectByGuid':
                uc.mem_write(state['command']+4,bytes(4))
            uc.reg_write(UC_X86_REG_EAX,result);uc.reg_write(UC_X86_REG_ECX,0xcccccccc);uc.reg_write(UC_X86_REG_EDX,0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP,sp+4+4*nargs);uc.reg_write(UC_X86_REG_EIP,ret)
        def write(uc,access,address,size,value,user):
            if ARENA<=address<ARENA+0x1e000: state['writes'].append((address,size,value))
        uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,write);machines.append(uc)
    rng = random.Random(0x52c7c0)
    counts = dict.fromkeys(PREFIXES,0)
    def compare(name,memory,arguments,this,mutation=0,command=COMMANDS):
        outputs=[]
        for rebuilt,uc in zip((False,True),machines):
            uc.mem_write(ARENA,bytes(memory));uc.mem_write(STACK,struct.pack('<I',STOP)+struct.pack('<'+'I'*len(arguments),*arguments))
            uc.reg_write(UC_X86_REG_ESP,STACK);uc.reg_write(UC_X86_REG_ECX,this)
            for i,r in enumerate(PRESERVED): uc.reg_write(r,0x12345000+i)
            state.clear();state.update(trace=[],writes=[],mutation=mutation,command=command)
            uc.emu_start(entries[name] if rebuilt else int(manifest['functions'][name]['address'],16),STOP,count=20000)
            assert uc.reg_read(UC_X86_REG_EIP)==STOP,(name,'did not terminate')
            assert uc.reg_read(UC_X86_REG_ESP)==STACK+4+4*len(arguments)
            assert all(uc.reg_read(r)==0x12345000+i for i,r in enumerate(PRESERVED))
            result = uc.reg_read(UC_X86_REG_EAX) if name in ('LoadFocus','FindScriptByHash') else None
            outputs.append((result,bytes(uc.mem_read(ARENA,0x1e000)),state['trace'],state['writes']))
        assert outputs[0]==outputs[1],('state or call order mismatch',name,arguments,mutation,outputs[0][0],outputs[1][0],outputs[0][2:],outputs[1][2:])
        counts[name]+=1
    types=(0,1,2,3,0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,0x200,0x201,0x202,0x203,0x204,0x2c0,0x2c1,0x2c2,0x300,0x301,0x500,0x501,0xffffffff)
    for trial in range(2048):
        memory=bytearray(rng.randbytes(0x20000))
        def put(address,*values): struct.pack_into('<'+'I'*len(values),memory,address-ARENA,*[v&0xffffffff for v in values])
        put(OBJECT+0xc,1);put(OBJECT+0x4c,0 if trial%7==0 else SCRIPTS);put(OBJECT+0x54,SCRIPTS)
        for i in range(4): put(SCRIPTS+i*0x40+4,SCRIPTS+(i+1)*0x40 if i+1<trial%5 else 0);put(SCRIPTS+i*0x40+0x30,(1,2,1,0xffffffff)[i])
        put(PACKAGE+0xc,COMMANDS+0x900,trial%5);put(PACKAGE+0x1c,trial*31);put(PACKAGE+0x8c,OBJECT if trial%2 else 0)
        put(QUEUE+0x4148,trial%17);put(QUEUE+0x514c,trial%32)
        for i in range(32): put(QUEUE+0x5150+i*8,rng.randrange(5),(0,1,2,3,0xffffffff)[(trial+i)%5])
        key=(0,1,2,4,5,0xffffffff)[trial%6]
        compare('LoadFocus',memory,[key],QUEUE)
        index=(0,1,30,31,32,33,-1,-2,-32,-33,0x7fffffff,0x80000000,0xffffffff)[trial%13]
        put(QUEUE+0x514c,index)
        compare('StoreFocus',memory,[key,(0,1,31,32,0xffffffff,0x80000000)[trial%6]],QUEUE)
        compare('ClearPackageStack',memory,[],QUEUE)
        compare('FindScriptByHash',memory,[(0,1,2,3,0xffffffff)[trial%5]],OBJECT)
        put(QUEUE+0x514c,trial%32)
        for mode in range(2):
            commands = [(types[(trial+mode)%len(types)],(0,1,2,3,0xffffffff,0x100)[trial%6],(0,1,2,3,0xfffffffc,0xffffffff)[trial%6])]
            commands += [(rng.choice(types),rng.choice((0,1,2,3,0x100)),rng.choice((0,1,2,3,0xfffffffc,0xffffffff))) for _ in range(trial%8)]
            put(RESPONSE+0x10,len(commands),COMMANDS)
            for i,(type_,value,target) in enumerate(commands): put(COMMANDS+i*12,type_,value,target)
            for i in range(len(commands)+2): put(COMMANDS+0x800+i*12,0x101,1,0)
            put(COMMANDS+len(commands)*12,0xffffffff,0,0)
            compare('ExecuteResponse',memory,[RESPONSE,OBJECT,PACKAGE,(0,1,0x80,0xff,0x100,0xdeadbeef)[trial%6]],QUEUE,
                    mutation=mode*2,command=COMMANDS)
        if trial % 16 == 0:
            put(RESPONSE+0x10,1,COMMANDS)
            put(COMMANDS,0,1,0)
            compare('ExecuteResponse',memory,[RESPONSE,OBJECT,PACKAGE,0xff],QUEUE,mutation=1)
            compare('ExecuteResponse',memory,[RESPONSE,0,PACKAGE,0xff],QUEUE)
            put(COMMANDS,0x100,3,0)
            compare('ExecuteResponse',memory,[RESPONSE,OBJECT,PACKAGE,0xff],QUEUE,mutation=4)
            put(COMMANDS,0x103,3,0)
            for i in range(32): put(QUEUE+0x5150+i*8,0xdeadbeef,0)
            put(QUEUE+0x5150,trial%5,3)
            compare('ExecuteResponse',memory,[RESPONSE,OBJECT,PACKAGE,0xff],QUEUE,mutation=4)
    report={'compiler':args.compiler,'comparisons':counts,'total':sum(counts.values()),'fingerprints':manifest,
            'controlled_interfaces':list(BOUNDARIES),'checks':['guarded memory','ordered arena writes','call arguments/order','return/ABI'],
            'limitations':['finite mapped state and acyclic script lists','external dispatcher callees controlled','MSVC and byte matching pending']}
    if args.mutation_checks:
        sources = {'source':args.source, 'state-source':args.state_source}
        mutations = {
            'reload-count':('source','index < count;','index < response->commandCount10;'),
            'cache-command-storage':('source','    for (unsigned int index = 0; index < count; ++index) {\n        const FEResponseCommand *command = response->commands14 + index;',
                                    '    const FEResponseCommand *storage = response->commands14;\n    for (unsigned int index = 0; index < count; ++index) {\n        const FEResponseCommand *command = storage + index;'),
            'skip-selected-script':('source','if (script) { object->SetScript(script);','if (script && script != object->currentScript54) { object->SetScript(script);'),
            'reset-current-instead-of-local':('source','script->time10 = 0;','object->currentScript54->time10 = 0;'),
            'resolve-sentinel-target':('source','target != 0xfffffffc && target != 0xffffffff','target != 0xffffffff'),
            'skip-null-target-post':('source','Post(command->parameter, object, package, (int)target, mask);','if (target) Post(command->parameter, object, package, (int)target, mask);'),
            'truthy-value1':('source','value1 = command->parameter == 1;','value1 = command->parameter != 0;'),
            'truncate-mask-loop':('source','bit < 8','bit < 7'),
            'stale-focus-parameter':('source','            if (command->parameter) focus = package->FindObjectByGuid(command->parameter);\n            if (focus || !command->parameter)',
                                    '            unsigned int saved = command->parameter;\n            if (saved) focus = package->FindObjectByGuid(saved);\n            if (focus || !saved)'),
            'omit-stack-pop':('source','--value4148;','value4148 = value4148;'),
            'unsigned-focus-modulo':('state-source','& 0x8000001f','& 0x1f'),
            'skip-zero-focus-lookup':('state-source','value5150[i].key0 == key) return','key && value5150[i].key0 == key) return'),
            'clear-focus-values':('state-source','value5150[i].key0 = 0;','value5150[i].key0 = value5150[i].value4 = 0;'),
            'wide-stack-clear':('state-source','value414C[i][0] = 0;','*(unsigned int *)&value414C[i][0] = 0;'),
            'skip-stack-count-reset':('state-source','    value4148 = 0;','    value4148 = value4148;'),
            'wrong-script-field':('state-source','script->hash30 != hash','(unsigned int)script->time10 != hash'),
        }
        report['rejected_mutations'] = []
        for name,(which,before,after) in mutations.items():
            source = sources[which].read_text()
            assert before in source,name
            path = BUILD/('mutation-'+name+'.cpp')
            path.write_text(source.replace(before,after))
            command = [os.sys.executable,str(Path(__file__).resolve()),'--compiler',args.compiler,'--source',str(args.source),'--state-source',str(args.state_source),'--'+which,str(path)]
            result = subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode != 0 and 'state or call order mismatch' in result.stderr,name
            report['rejected_mutations'].append(name)
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__': main()
