#!/usr/bin/env python3
"""Verify complete queued FEng operations with actual nodes, strings and destructors."""
import argparse,hashlib,json,os,random,struct,subprocess
from pathlib import Path
import pefile
import unicorn.x86_const as xc
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from pc_coff import link_objects,read_object
from probe_pc_race_reward import ROOT,EXE
BUILD=ROOT/'build/analysis/feng-operations'
ARENA,STOP,TIB,GDT=0x2000000,0x2100000,0x2120000,0x2130000
QUEUE=ARENA+0x10000
PACKAGES=[ARENA+0x2000+i*0x200 for i in range(4)]
NODES=[ARENA+0x5000+i*0x40 for i in range(4)]
NAMES=[ARENA+0x7000+i*0x40 for i in range(6)]
RECORDS=[ARENA+0x8000+i*0x40 for i in range(3)]
MANAGER,STACK=ARENA+0x9000,ARENA+0x1f000
PRESERVED=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',choices=('msvc','clang-cl'),default='msvc')
    p.add_argument('--source',type=Path,default=ROOT/'src/Speed/Indep/Src/Frontend/FEngOperations.cpp')
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    manifest=json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-operations.json').read_text())
    nodes=json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-queue-nodes.json').read_text())
    assert nodes['executable_sha256']==manifest['executable_sha256']
    functions=dict(manifest['functions']);functions.update(nodes['functions'])
    raw=EXE.read_bytes();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    image=pefile.PE(data=raw).get_memory_mapped_image()
    for f in functions.values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);objects=[]
    for source in (args.source,ROOT/'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp',ROOT/'src/Speed/Indep/Src/Frontend/FEngDiagnostic.cpp',ROOT/'src/Speed/Indep/Libs/bWare/Strings.cpp'):
        obj=BUILD/(source.stem+'.obj')
        compiler=([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe'] if args.compiler=='msvc' else
                  ['clang-cl','--target=i686-pc-windows-msvc','/GS-','/GR-','/w','/arch:IA32','/clang:-fno-strict-aliasing','/clang:-fno-builtin'])
        subprocess.run(compiler+['/nologo','/c','/O2','/Ob0','/Fo'+str(obj.relative_to(ROOT)),'/Isrc/Speed/Indep/Src/Frontend',os.path.relpath(source,ROOT)],cwd=ROOT,check=True)
        objects.append(obj)
    external={'_strlen':STOP+0x500}
    names={'Call5362E0':0x5362e0,'Call555860':0x555860,'FEngAllocateQueueNode':0x575620,'FEngFreeQueueNode':0x575630,'fengManager8384C4':0x8384c4}
    for obj in objects:
        for s in read_object(obj)['symbols'].values():
            if s['section']==0:
                for n,a in names.items():
                    if s['name'].startswith('?'+n+'@'):external[s['name']]=a
    linked=link_objects(objects,external)
    def export(prefix):return next(a for s,a in linked['exports'].items() if s.startswith(prefix))
    entry=export('?ConsumeOperations@FEngQueue@')
    state,machines={},[]
    markers={0x5559ee:'unlink',0x555a61:'remove-path',0x555a8b:'stale-parent',0x555abd:'reparent-detach',
             0x555ac9:'save-replacement',0x555ae1:'restore-parent-mask',0x555b03:'restore-all-masks',
             0x555b61:'registry-miss',0x555b85:'load-call',0x555b9a:'attach-loaded-parent',
             0x555bba:'replacement-parent',0x555bd9:'replacement-child',0x555c05:'exclusive-mask',
             0x555c23:'transfer-to-parent',0x555c62:'notify-parent',0x555c9d:'transfer-to-child',
             0x555cdc:'notify-child',0x555c92:'delete-without-child',0x555ce7:'delete-common'}
    coverage={n:0 for n in markers.values()}
    coverage.update({'load-success':0,'load-failure':0})
    def words(uc,a,n=1):return struct.unpack('<'+'I'*n,uc.mem_read(a,4*n))
    def text_at(uc,a):
        b=bytearray()
        for i in range(128):
            c=uc.mem_read(a+i,1)[0]
            if not c:return bytes(b)
            b.append(c)
        raise AssertionError('unbounded text')
    for rebuilt in (False,True):
        uc=Uc(UC_ARCH_X86,UC_MODE_32);uc.mem_map(0x400000,(len(image)+4095)&~4095);uc.mem_write(0x400000,image)
        uc.mem_map(ARENA,0x20000);uc.mem_map(STOP,0x1000);uc.mem_map(TIB,0x1000);uc.mem_map(GDT,0x1000)
        def descriptor(base,limit,access,flags=0xc):return ((limit&0xffff)|(base&0xffffff)<<16|access<<40|(limit>>16&15)<<48|flags<<52|(base>>24)<<56)
        uc.mem_write(GDT+8,struct.pack('<3Q',descriptor(0,0xfffff,0x9b),descriptor(0,0xfffff,0x93),descriptor(TIB,0xfff,0x93,4)))
        uc.reg_write(xc.UC_X86_REG_GDTR,(0,GDT,0x1f,0))
        for r,s in ((xc.UC_X86_REG_CS,8),(xc.UC_X86_REG_SS,16),(xc.UC_X86_REG_DS,16),(xc.UC_X86_REG_ES,16),(xc.UC_X86_REG_FS,24)):uc.reg_write(r,s)
        uc.mem_map(linked['address'],linked['allocation'])
        for a,b in linked['blocks']:uc.mem_write(a,b)
        if rebuilt:uc.mem_write(0x79ab64,struct.pack('<I',export('?DestroyOperation@FEngQueueNode@')))
        ranges=linked['ranges'] if rebuilt else [(int(f['address'],16),int(f['address'],16)+f['size']) for f in functions.values()]
        calls={export('?Post@FEngQueue@') if rebuilt else 0x51aa20:'post',
               export('?DestroyOperation@FEngQueueNode@') if rebuilt else 0x51abe0:'destroy',
               export('?FngScrollerDiagnostic@') if rebuilt else 0x50d510:'diagnostic',
               export('_nfs_stricmp') if rebuilt else 0x43dcc0:'compare',
               export('?Remove@FEngQueueList@') if rebuilt else 0x505400:'remove-node'}
        def hook(uc,a,size,user,ranges=ranges,calls=calls,original=not rebuilt):
            if original and a in markers:coverage[markers[a]]+=1
            sp=uc.reg_read(xc.UC_X86_REG_ESP)
            if a in calls:
                name=calls[a]
                if name=='compare':state['trace'].append((name,*words(uc,sp+4,2)))
                elif name=='diagnostic':state['trace'].append((name,words(uc,sp+4)[0]))
                elif name=='post':state['trace'].append((name,uc.reg_read(xc.UC_X86_REG_ECX),*words(uc,sp+4,5)))
                else:state['trace'].append((name,uc.reg_read(xc.UC_X86_REG_ECX),words(uc,sp+4)[0]))
            if a not in (0x5362e0,0x555860,0x575620,0x575630,STOP+0x600):
                assert any(x<=a<y for x,y in ranges),('unexpected execution',hex(a));return
            result,cleanup=0,0
            if a==0x575620:
                ret,size=words(uc,sp,2);assert size==32
                result=ARENA+0x16000+state['allocated']*32;state['allocated']+=1
                state['trace'].append(('allocate',size));assert result<ARENA+0x1e000
            elif a==0x575630:
                ret,pointer=words(uc,sp,2);state['trace'].append(('free',pointer))
                if state['mode']&1 and pointer in NODES[:3] and not state['injected']:
                    state['injected']=True
                    head=words(uc,QUEUE+0x412c)[0]
                    uc.mem_write(NODES[3],struct.pack('<8I',0x79ab64,head,0,0,0,0,0,PACKAGES[0]))
                    if head:uc.mem_write(head+8,struct.pack('<I',NODES[3]))
                    else:uc.mem_write(QUEUE+0x4130,struct.pack('<I',NODES[3]))
                    uc.mem_write(QUEUE+0x412c,struct.pack('<I',NODES[3]));uc.mem_write(QUEUE+0x4128,struct.pack('<I',(words(uc,QUEUE+0x4128)[0]+1)&0xffffffff))
            elif a==STOP+0x600:
                ret,*arguments=words(uc,sp,6);cleanup=20
                assert uc.reg_read(xc.UC_X86_REG_ECX)==ARENA+0xa000
                state['trace'].append(('observer',arguments))
                active=next(n for n in NODES if words(uc,n)[0]==0x79ab64 and words(uc,n+4)[0]==0xabadcafe)
                if state['mode']&8:uc.mem_write(active+0x14,struct.pack('<I',words(uc,active+0x14)[0]&~8))
                if state['mode']&16:uc.mem_write(active+0x18,bytes(4))
            elif a==0x5362e0:
                ret,package=words(uc,sp,2);cleanup=4
                assert uc.reg_read(xc.UC_X86_REG_ECX)==QUEUE
                state['trace'].append(('remove',package))
                # No real free: later flag paths intentionally retain mapped package state.
                previous=0;current=words(uc,QUEUE+0xe4)[0]
                while current:
                    next_=words(uc,current+4)[0]
                    if current==package:
                        uc.mem_write(previous+4 if previous else QUEUE+0xe4,struct.pack('<I',next_));break
                    previous,current=current,next_
            else:
                ret,name,level,mask=words(uc,sp,4);cleanup=12
                assert uc.reg_read(xc.UC_X86_REG_ECX)==QUEUE
                state['trace'].append(('load',name,level&255,mask,text_at(uc,name)))
                result=0 if state['mode']&2 else PACKAGES[3]
                if original:coverage['load-success' if result else 'load-failure']+=1
                if result:
                    current=words(uc,QUEUE+0xe4)[0];present=False
                    while current:
                        if current==result:present=True;break
                        current=words(uc,current+4)[0]
                    if not present:
                        uc.mem_write(result+4,struct.pack('<I',words(uc,QUEUE+0xe4)[0]));uc.mem_write(QUEUE+0xe4,struct.pack('<I',result))
                if state['mode']&4:
                    active=next(n for n in NODES[:3] if words(uc,n+0xc)[0]==name)
                    uc.mem_write(active+0x14,struct.pack('<I',0x10))
            uc.reg_write(xc.UC_X86_REG_EAX,result);uc.reg_write(xc.UC_X86_REG_ECX,0xcccccccc);uc.reg_write(xc.UC_X86_REG_EDX,0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP,sp+4+cleanup);uc.reg_write(xc.UC_X86_REG_EIP,ret)
        def write(uc,access,a,size,value,user):
            if ARENA<=a<ARENA+0x1e000:state['writes'].append((a,size,value))
        uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,write);machines.append(uc)
    rng=random.Random(0x5559c0);comparisons=0
    for trial in range(4097):
        memory=bytearray(rng.randbytes(0x20000))
        def put(a,*v):struct.pack_into('<'+'I'*len(v),memory,a-ARENA,*[x&0xffffffff for x in v])
        put(MANAGER+0x10,RECORDS[0] if trial%5 else MANAGER+0x10,RECORDS[2],0)
        for i,record in enumerate(RECORDS):put(record,RECORDS[i+1] if i<2 else MANAGER+0x10,RECORDS[i-1] if i else MANAGER+0x10,NAMES[i])
        for i,name in enumerate(NAMES):
            text=(b'UI_A',b'ui_B',b'\xffAZ',b'ui_a',b'missing',b'')[i];memory[name-ARENA:name-ARENA+len(text)+1]=text+b'\0'
        put(QUEUE+0xe4,PACKAGES[0]);put(QUEUE+0x4118,0,0,0);memory[QUEUE+0x5251-ARENA]=0
        for i,package in enumerate(PACKAGES):
            put(package+4,PACKAGES[i+1] if i<2 else 0)
            put(package+0x18,(0,1,255,256,0x7fffffff,0x80000000,0xffffffff)[(trial+i)%7])
            put(package+0x1c,*[rng.getrandbits(32) for _ in range(4)])
            put(package+0x3c,((0,PACKAGES[0],PACKAGES[1],PACKAGES[3])[((trial//128)+i)%4]))
        mode=(trial//128)%32
        if mode&24:
            memory[QUEUE+0x5251-ARENA]=255
            put(QUEUE+0x10c,ARENA+0xa000);put(ARENA+0xa000,ARENA+0xa100);put(ARENA+0xa154,STOP+0x600)
        count=(trial//32)%4
        put(QUEUE+0x4128,count,NODES[0] if count else 0,NODES[count-1] if count else 0)
        for i in range(3):
            flags=((trial+i*7)%32)|((trial%3)*0x100)
            package=PACKAGES[(trial+i)%3]
            if not flags&0xd and trial%11==0:package=0
            put(NODES[i],0x79ab64,NODES[i+1] if i+1<count else 0,NODES[i-1] if i else 0,NAMES[(trial+i)%6],0,flags,rng.getrandbits(32),package)
        if trial==4096:
            put(QUEUE+0x4128,3,NODES[0],NODES[2])
            for i in range(3):
                put(NODES[i],0x79ab64,NODES[i+1] if i<2 else 0,NODES[i-1] if i else 0,NAMES[(0,1,3)[i]],0,3,0x55,PACKAGES[i])
                put(PACKAGES[i]+0x18,(2,0xffffffff,255)[i])
        outputs=[]
        for rebuilt,uc in zip((False,True),machines):
            uc.mem_write(ARENA,bytes(memory));uc.mem_write(0x8384c4,struct.pack('<I',MANAGER));uc.mem_write(TIB,struct.pack('<I',0x12345678))
            uc.mem_write(STACK,struct.pack('<I',STOP));uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,QUEUE)
            for i,r in enumerate(PRESERVED):uc.reg_write(r,0x12345000+i)
            state.clear();state.update(trace=[],writes=[],mode=mode,allocated=0,injected=False)
            uc.emu_start(entry if rebuilt else 0x5559c0,STOP,count=50000)
            assert uc.reg_read(xc.UC_X86_REG_EIP)==STOP,(trial,'did not terminate')
            assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4
            assert all(uc.reg_read(r)==0x12345000+i for i,r in enumerate(PRESERVED))
            assert words(uc,TIB)[0]==0x12345678
            outputs.append((bytes(uc.mem_read(ARENA,0x1e000)),state['trace'],state['writes']))
        assert outputs[0]==outputs[1],('state or call order mismatch',trial,outputs[0][1:],outputs[1][1:])
        comparisons+=1
    assert all(coverage.values()),coverage
    report={'original_block_hits':coverage,'compiler':args.compiler,'comparisons':comparisons,'fingerprints':functions,
            'controlled_interfaces':['package removal 0x5362e0','package loading 0x555860','allocation 0x575620','deallocation 0x575630','post observer virtual +0x54'],
            'actual_composition':['Post','operation deleting destructor/helpers','case-insensitive string compare','retail diagnostic sink'],
            'limitations':['finite mapped live-package/registry chains, including selected parent aliases','lifecycle and allocator boundaries controlled','normal-return SEH only','MSVC and byte matching pending']}
    if args.mutation_checks:
        source=args.source.read_text();report['rejected_mutations']=[]
        mutations={
            'skip-remove-flag':('(node->value14 & 1) && level >= 0','(node->value14 & 0) && level >= 0'),
            'keep-stale-parent':('if (!found) {','if (false) {'),
            'skip-child-reparent':('if (other->parent3C == package) other->parent3C = parent;', 'if (false) other->parent3C = parent;'),
            'reset-saved-parent-per-node':('    for (;;) {','    for (;;) {\n        savedPackage = 0; savedParent = 0;'),
            'inverted-registry-match':('if (!nfs_stricmp(record->name8, name))','if (nfs_stricmp(record->name8, name))'),
            'wrong-next-level':('(unsigned char)((unsigned char)level + 1)','(unsigned char)((unsigned char)level + 2)'),
            'skip-exclusive-on-load-failure':('if (node->value14 & 0x10)', 'if (loaded && (node->value14 & 0x10))'),
            'update-extra-load-masks':('previous->inputMask24 = mask;', 'previous->inputMask24 = mask; previous->inputMask20 = mask; previous->inputMask28 = mask;'),
            'skip-parent-transfer':('if (node->value14 & 4)', 'if (false)'),
            'skip-child-transfer':('if (node->value14 & 8)', 'if (false)'),
            'transfer-request-instead-of-owned':('unsigned int moved = package->inputMask1C & node->value18;', 'unsigned int moved = node->value18;'),
            'skip-zero-mask-notification':('Post(0x334c5493,', 'if (moved) Post(0x334c5493,'),
            'omit-count-decrement':('--operations4124.count4;', 'operations4124.count4 = operations4124.count4;'),
            'skip-virtual-delete':('((DeletingDestructor *)node->vtable0)[0](node, 1);','node->value0 = 0;'),
        }
        # A valid no-delete mutation must compile and simply omit the call.
        mutations['skip-virtual-delete']=(mutations['skip-virtual-delete'][0], '(void)node;')
        mutated={name:source.replace(before,after) for name,(before,after) in mutations.items()}
        for name,(before,_) in mutations.items():assert before in source,name
        unlink='        operations4124.first8 = node->next4;\n        if (node == operations4124.valueC) operations4124.valueC = node->previous8;\n        if (node->previous8) node->previous8->next4 = node->next4;\n        if (node->next4) node->next4->previous8 = node->previous8;\n        node->next4 = (FEngQueueNode *)0xabadcafe;\n        node->previous8 = (FEngQueueNode *)0xabadcafe;\n        --operations4124.count4;'
        assert unlink in source
        mutated['extra-remove-call-boundary']=source.replace(unlink,'        operations4124.Remove(node);')
        cached=source.replace('    for (;;) {\n        FEngQueueNode *node = operations4124.first8;', '    FEngQueueNode *node = operations4124.first8;\n    for (;;) {')
        cached=cached.replace('        if (!node) return;', '        if (!node) return;\n        FEngQueueNode *next = node->next4;')
        cached=cached.replace('((DeletingDestructor *)node->vtable0)[0](node, 1);','((DeletingDestructor *)node->vtable0)[0](node, 1);\n        node = next;')
        mutated['cache-next-across-deleting-callback']=cached
        flags=source.replace('        if (!node) return;', '        if (!node) return;\n        bool savedFlag8 = (node->value14 & 8) != 0;')
        mutated['cache-input-flag-before-callbacks']=flags.replace('if (node->value14 & 8)', 'if (savedFlag8)')
        mask=source.replace('        if (!node) return;', '        if (!node) return;\n        unsigned int savedMask = node->value18;')
        mutated['cache-input-mask-before-callbacks']=mask.replace('unsigned int moved = package->inputMask1C & node->value18;', 'unsigned int moved = package->inputMask1C & savedMask;')
        for name,text in mutated.items():
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(text)
            result=subprocess.run([os.sys.executable,str(Path(__file__).resolve()),'--compiler',args.compiler,'--source',str(path)],cwd=ROOT,capture_output=True,text=True)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode!=0 and 'state or call order mismatch' in result.stderr,name
            report['rejected_mutations'].append(name)
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
