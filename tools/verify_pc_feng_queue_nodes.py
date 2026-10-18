#!/usr/bin/env python3
"""Compare original/rebuilt FEng queue insertion and removal, with observer mutations."""
import argparse,hashlib,json,os,random,struct,subprocess
from pathlib import Path
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESP,
                              UC_X86_REG_EIP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
from pc_coff import link_objects,read_object
from probe_pc_race_reward import ROOT,EXE
BUILD=ROOT/'build/analysis/feng-queue-nodes'
ARENA,STOP=0x2000000,0x2100000
QUEUE,PACKAGE,OBSERVER,VTABLE,NODES,ALLOC,STACK=[ARENA+x for x in (0x1000,0x7000,0x7100,0x7200,0x8000,0x9000,0x1f000)]
PRESERVED=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
PREFIXES={'Post':'?Post@FEngQueue@','Call51AF50':'?Call51AF50@FEngQueue@','Remove':'?Remove@FEngQueueList@'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',choices=('msvc','clang-cl'),default='msvc')
    p.add_argument('--source',type=Path,default=ROOT/'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp')
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    manifest=json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-queue-nodes.json').read_text())
    raw=EXE.read_bytes();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    image=pefile.PE(data=raw).get_memory_mapped_image()
    for f in list(manifest['functions'].values())+list(manifest['tables'].values()):
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);obj=BUILD/(args.source.stem+'.obj')
    command=([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe'] if args.compiler=='msvc' else
             ['clang-cl','--target=i686-pc-windows-msvc','/GS-','/GR-','/w','/arch:IA32','/clang:-fno-strict-aliasing'])
    subprocess.run(command+['/nologo','/c','/O2','/Ob0','/Fo'+str(obj.relative_to(ROOT)),'/Isrc/Speed/Indep/Src/Frontend',os.path.relpath(args.source,ROOT)],cwd=ROOT,check=True)
    externals={s['name']:0x575620 for s in read_object(obj)['symbols'].values() if s['section']==0 and s['name'].startswith('?FEngAllocateQueueNode@')}
    linked=link_objects([obj],externals)
    entries={n:next(a for s,a in linked['exports'].items() if s.startswith(prefix)) for n,prefix in PREFIXES.items()}
    state,machines={},[]
    def words(uc,a,n=1):return struct.unpack('<'+'I'*n,uc.mem_read(a,n*4))
    for rebuilt in (False,True):
        uc=Uc(UC_ARCH_X86,UC_MODE_32);uc.mem_map(0x400000,(len(image)+4095)&~4095);uc.mem_write(0x400000,image)
        uc.mem_map(ARENA,0x20000);uc.mem_map(STOP,0x1000);uc.mem_map(linked['address'],linked['allocation'])
        for a,b in linked['blocks']:uc.mem_write(a,b)
        ranges=linked['ranges'] if rebuilt else [(int(f['address'],16),int(f['address'],16)+f['size']) for f in manifest['functions'].values()]
        def hook(uc,a,size,user,ranges=ranges):
            if a not in (0x575620,STOP+0x100):
                assert any(x<=a<y for x,y in ranges),('unexpected execution',hex(a));return
            sp=uc.reg_read(UC_X86_REG_ESP)
            if a==0x575620:
                ret,size=words(uc,sp,2);assert size==32
                result=ALLOC;cleanup=0;state['trace'].append(('allocate',size))
                if state['mode']&1:uc.mem_write(QUEUE+0x5251,b'\xff')
            else:
                ret,*arguments=words(uc,sp,6);assert uc.reg_read(UC_X86_REG_ECX)==OBSERVER
                state['trace'].append(('observer',arguments,bytes(uc.mem_read(ALLOC,32)),words(uc,QUEUE+0x4118,3)))
                result=0;cleanup=20
                if state['mode']&2:uc.mem_write(QUEUE+0x4120,struct.pack('<I',NODES))
                if state['mode']&4:uc.mem_write(QUEUE+0x411c,struct.pack('<I',NODES+0x40))
                if state['mode']&8:uc.mem_write(QUEUE+0x4118,struct.pack('<I',0xffffffff))
            uc.reg_write(UC_X86_REG_EAX,result);uc.reg_write(UC_X86_REG_ECX,0xcccccccc);uc.reg_write(UC_X86_REG_EDX,0xdddddddd)
            uc.reg_write(UC_X86_REG_ESP,sp+4+cleanup);uc.reg_write(UC_X86_REG_EIP,ret)
        def write(uc,access,a,size,value,user):
            if ARENA<=a<ARENA+0x1e000:state['writes'].append((a,size,value))
        uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,write);machines.append(uc)
    rng=random.Random(0x51aa20);counts=dict.fromkeys(PREFIXES,0)
    def compare(name,memory,arguments,this,mode):
        outputs=[]
        for rebuilt,uc in zip((False,True),machines):
            uc.mem_write(ARENA,bytes(memory));uc.mem_write(STACK,struct.pack('<I',STOP)+struct.pack('<'+'I'*len(arguments),*arguments))
            uc.reg_write(UC_X86_REG_ESP,STACK);uc.reg_write(UC_X86_REG_ECX,this)
            for i,r in enumerate(PRESERVED):uc.reg_write(r,0x12345000+i)
            state.clear();state.update(trace=[],writes=[],mode=mode)
            uc.emu_start(entries[name] if rebuilt else int(manifest['functions'][name]['address'],16),STOP,count=10000)
            assert uc.reg_read(UC_X86_REG_EIP)==STOP
            assert uc.reg_read(UC_X86_REG_ESP)==STACK+4+4*len(arguments)
            assert all(uc.reg_read(r)==0x12345000+i for i,r in enumerate(PRESERVED))
            outputs.append((bytes(uc.mem_read(ARENA,0x1e000)),state['trace'],state['writes']))
        assert outputs[0]==outputs[1],('state or call order mismatch',name,mode,outputs[0][1:],outputs[1][1:])
        counts[name]+=1
    for trial in range(4096):
        memory=bytearray(rng.randbytes(0x20000))
        def put(a,*v):struct.pack_into('<'+'I'*len(v),memory,a-ARENA,*v)
        for base in (QUEUE+0x4114,QUEUE+0x4124):
            put(base+4,(0,1,2,3,0xffffffff)[trial%5],0 if trial%4==0 else NODES,
                (0,NODES,NODES+0x40,NODES+0x80)[trial%4])
        for i in range(3):put(NODES+i*0x40+4,NODES+(i+1)*0x40 if i<2 else 0,NODES+(i-1)*0x40 if i else 0)
        memory[QUEUE+0x5251-ARENA]=(0,1,2,128,255)[trial%5]
        put(QUEUE+0x10c,OBSERVER);put(OBSERVER,VTABLE);put(VTABLE+0x54,STOP+0x100)
        mask=rng.getrandbits(32);put(PACKAGE+0x1c,rng.getrandbits(32))
        compare('Post',memory,[rng.getrandbits(32),PACKAGE+0x100,PACKAGE,(0,1,0xfffffffc,0xffffffff)[trial%4],mask],QUEUE,trial%16)
        compare('Call51AF50',memory,[PACKAGE,(0,1,2,128,255,0x100,0x10001,0xffffffff)[trial%8],mask],QUEUE,0)
        compare('Remove',memory,[(0,NODES,NODES+0x40,NODES+0x80)[trial%4]],QUEUE+0x4114,0)
    report={'compiler':args.compiler,'comparisons':counts,'total':sum(counts.values()),'fingerprints':manifest,
            'controlled_interfaces':['allocation 0x575620','observer virtual +0x54'],
            'limitations':['finite mapped lists/cursor states','allocation failure and arbitrary aliases excluded','node destruction/delivery unverified','MSVC and byte matching pending']}
    if args.mutation_checks:
        source=args.source.read_text();report['rejected_mutations']=[]
        mutations={
            'cache-cursor-before-observer':('    if (value5251) value10C->MessagePosted(message, sender, package, target, mask);\n    FEngQueueNode *cursor = messages4114.valueC;',
                                           '    FEngQueueNode *cursor = messages4114.valueC;\n    if (value5251) value10C->MessagePosted(message, sender, package, target, mask);'),
            'observer-before-payload':('    node->value14 = (unsigned int)package;\n    node->value18 = message;\n    node->valueC = (unsigned int)sender;\n    node->value10 = (unsigned int)target;\n    node->value1C = (unsigned int)mask;\n    if (value5251) value10C->MessagePosted(message, sender, package, target, mask);',
                                        '    if (value5251) value10C->MessagePosted(message, sender, package, target, mask);\n    node->value14 = (unsigned int)package;\n    node->value18 = message;\n    node->valueC = (unsigned int)sender;\n    node->value10 = (unsigned int)target;\n    node->value1C = (unsigned int)mask;'),
            'skip-observer':('if (value5251) value10C->MessagePosted','if (false) value10C->MessagePosted'),
            'word-enable':('(unsigned char)value ? 8 : 4','value ? 8 : 4'),
            'ignore-package-mask':('package->inputMask1C & mask','mask'),
            'wrong-node-type':('(void *)0x79ab60','(void *)0x79ab64'),
            'omit-message-count':('++messages4114.count4;','messages4114.count4 = messages4114.count4;'),
            'omit-input-count':('++operations4124.count4;','operations4124.count4 = operations4124.count4;'),
            'omit-previous-link':('if (next) next->previous8 = node;','if (false) next->previous8 = node;'),
            'wrong-unlink-sentinel':('(FEngQueueNode *)0xabadcafe;','(FEngQueueNode *)0xabadcaff;'),
            'omit-remove-count':('--count4;','count4 = count4;'),
        }
        for name,(before,after) in mutations.items():
            assert before in source
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(source.replace(before,after))
            result=subprocess.run([os.sys.executable,str(Path(__file__).resolve()),'--compiler',args.compiler,'--source',str(path)],cwd=ROOT,capture_output=True,text=True)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode!=0 and 'state or call order mismatch' in result.stderr,name
            report['rejected_mutations'].append(name)
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
