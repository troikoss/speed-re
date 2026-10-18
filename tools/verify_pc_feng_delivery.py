#!/usr/bin/env python3
"""Verify complete message routing, object delivery and special handlers against UG2."""
import argparse,hashlib,json,os,random,struct,subprocess
from pathlib import Path
import pefile
import unicorn.x86_const as xc
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from pc_coff import link_objects,read_object
from probe_pc_race_reward import ROOT,EXE
BUILD=ROOT/'build/analysis/feng-delivery'
ARENA,STOP=0x2000000,0x2100000
QUEUE,OBSERVER,VTABLE,STACK=ARENA+0x10000,ARENA+0xa000,ARENA+0xa100,ARENA+0x1f000
PACKAGES=[ARENA+0x2000+i*0x200 for i in range(4)]
OBJECTS=[ARENA+0x3000+i*0x100 for i in range(4)]
RESPONSES=[ARENA+0x4000+i*0x100 for i in range(4)]
OBJECT_RESPONSES=[ARENA+0x5000+i*0x100 for i in range(4)]
TABLES=[ARENA+0x6000+i*0x100 for i in range(4)]
VECTORS=[ARENA+0x7000+i*0x100 for i in range(4)]
NODES=[ARENA+0x8000+i*0x40 for i in range(4)]
PRESERVED=(xc.UC_X86_REG_EBX,xc.UC_X86_REG_ESI,xc.UC_X86_REG_EDI,xc.UC_X86_REG_EBP)
PREFIXES={'DeliverMessages':'?DeliverMessages@FEngQueue@','DeliverObject':'?DeliverObject@FEngQueue@',
          'HandleObject4':'?FEngHandleObject4@','HandleObject6':'?FEngHandleObject6@'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',choices=('msvc','clang-cl'),default='msvc')
    p.add_argument('--source',type=Path,default=ROOT/'src/Speed/Indep/Src/Frontend/FEngDelivery.cpp')
    p.add_argument('--mutation-checks',action='store_true');args=p.parse_args()
    manifest=json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-delivery.json').read_text())
    nodes=json.loads((ROOT/'config/SPEED2_EXE_1_2/feng-queue-nodes.json').read_text())
    assert nodes['executable_sha256']==manifest['executable_sha256']
    functions=dict(manifest['functions']);functions.update(nodes['functions'])
    raw=EXE.read_bytes();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    image=pefile.PE(data=raw).get_memory_mapped_image()
    for f in functions.values():
        a=int(f['address'],16)-0x400000;assert hashlib.sha256(image[a:a+f['size']]).hexdigest()==f['sha256']
    BUILD.mkdir(parents=True,exist_ok=True);objects=[]
    for source in (args.source,ROOT/'src/Speed/Indep/Src/Frontend/FEngQueueNodes.cpp'):
        obj=BUILD/(source.stem+'.obj')
        compiler=([str(ROOT/'build/tools/wibo'),'build/compilers/Win32/7.1/cl.exe'] if args.compiler=='msvc' else
                  ['clang-cl','--target=i686-pc-windows-msvc','/GS-','/GR-','/w','/arch:IA32','/clang:-fno-strict-aliasing'])
        subprocess.run(compiler+['/nologo','/c','/O2','/Ob0','/Fo'+str(obj.relative_to(ROOT)),'/Isrc/Speed/Indep/Src/Frontend',os.path.relpath(source,ROOT)],cwd=ROOT,check=True)
        objects.append(obj)
    externals={};imports={'ExecuteResponse':0x52c7c0,'Call516E90':0x516e90,'Call52BD70':0x52bd70,
                          'FEngAllocateQueueNode':0x575620,'FEngFreeQueueNode':0x575630}
    for obj in objects:
        for s in read_object(obj)['symbols'].values():
            if s['section']==0:
                for n,a in imports.items():
                    if s['name'].startswith('?'+n+'@'):externals[s['name']]=a
    linked=link_objects(objects,externals)
    def export(prefix):return next(a for s,a in linked['exports'].items() if s.startswith(prefix))
    entries={n:export(v) for n,v in PREFIXES.items()}
    state,machines={},[]
    markers={0x542084:'all-package-and-objects',0x542133:'live-package-and-objects',0x542297:'package-only',
             0x542251:'all-package-only',0x542234:'game-owner',0x54203d:'input-owner',0x54205d:'internal-state',
             0x54221b:'direct-object',0x5420fb:'global-object-fanout',0x5421d8:'package-object-fanout'}
    coverage={v:0 for v in markers.values()}
    def words(uc,a,n=1):return struct.unpack('<'+'I'*n,uc.mem_read(a,4*n))
    def active(uc):return next((n for n in NODES if words(uc,n)[0]==0x79ab60 and words(uc,n+4)[0]==0xabadcafe),0)
    def inject(uc):
        if state['injected']:return
        state['injected']=True
        cursor=words(uc,QUEUE+0x4120)[0];first=words(uc,QUEUE+0x411c)[0]
        next_=words(uc,cursor+4)[0] if cursor else first
        uc.mem_write(NODES[3],struct.pack('<8I',0x79ab60,next_,cursor,OBJECTS[2],0xffffffff,PACKAGES[0],state['message'],0x81))
        if next_:uc.mem_write(next_+8,struct.pack('<I',NODES[3]))
        if cursor:uc.mem_write(cursor+4,struct.pack('<I',NODES[3]))
        else:uc.mem_write(QUEUE+0x411c,struct.pack('<I',NODES[3]))
        uc.mem_write(QUEUE+0x4120,struct.pack('<I',NODES[3]));uc.mem_write(QUEUE+0x4118,struct.pack('<I',(words(uc,QUEUE+0x4118)[0]+1)&0xffffffff))
    for rebuilt in (False,True):
        uc=Uc(UC_ARCH_X86,UC_MODE_32);uc.mem_map(0x400000,(len(image)+4095)&~4095);uc.mem_write(0x400000,image)
        uc.mem_map(ARENA,0x20000);uc.mem_map(STOP,0x1000);uc.mem_map(linked['address'],linked['allocation'])
        for a,b in linked['blocks']:uc.mem_write(a,b)
        if rebuilt:uc.mem_write(0x79ab60,struct.pack('<I',export('?DestroyMessage@FEngQueueNode@')))
        ranges=linked['ranges'] if rebuilt else [(int(f['address'],16),int(f['address'],16)+f['size']) for f in functions.values()]
        helpers={export('?Remove@FEngQueueList@') if rebuilt else 0x505400:'unlink-call',
                 export('?DestroyMessage@FEngQueueNode@') if rebuilt else 0x51aaf0:'destroy',
                 entries['DeliverObject'] if rebuilt else 0x5367e0:'object-delivery'}
        def hook(uc,a,size,user,ranges=ranges,helpers=helpers,original=not rebuilt):
            if original and a in markers:coverage[markers[a]]+=1
            sp=uc.reg_read(xc.UC_X86_REG_ESP);this=uc.reg_read(xc.UC_X86_REG_ECX)
            if a in helpers:
                n=4 if helpers[a]=='object-delivery' else 1
                state['trace'].append((helpers[a],this,words(uc,sp+4,n)))
            if a not in (0x52c7c0,0x516e90,0x52bd70,0x575620,0x575630,STOP+0x180,STOP+0x1c0,STOP+0x580):
                assert any(x<=a<y for x,y in ranges),('unexpected execution',hex(a));return
            result,cleanup=0,0;current=active(uc)
            if a==0x52c7c0:
                ret,response,obj,package,mask=words(uc,sp,5);cleanup=16;assert this==QUEUE
                state['trace'].append(('execute',response,obj,package,mask))
                if current and not obj:
                    if state['mode']&2:uc.mem_write(current+0x18,struct.pack('<I',(state['message']+1)&0xffffffff))
                    if state['mode']&4:
                        index=PACKAGES.index(package);uc.mem_write(package+0x6c,struct.pack('<II',1,TABLES[index]+0x80))
                        uc.mem_write(current+0x1c,struct.pack('<I',0x80))
                    if state['mode']&32:uc.mem_write(package+4,bytes(4))
                if current and obj and state['mode']&8 and not state['object_mutated']:
                    state['object_mutated']=True
                    message=words(uc,response+0xc)[0];count,table=words(uc,package+0x6c,2)
                    association=next((table+i*16 for i in range(count) if words(uc,table+i*16)[0]==message),0)
                    if association:
                        uc.mem_write(association+8,struct.pack('<II',0,VECTORS[PACKAGES.index(package)]+0x40))
                    uc.mem_write(current+0x18,struct.pack('<I',(state['message']+2)&0xffffffff));uc.mem_write(current+0x1c,struct.pack('<I',0x80000000))
            elif a in (0x516e90,0x52bd70):
                ret,x,y=words(uc,sp,3);cleanup=8
                state['trace'].append(('navigation',hex(a),this,x,y));uc.mem_write(this+0x18,struct.pack('<I',1));result=0xa5a50000
            elif a==0x575630:
                ret,pointer=words(uc,sp,2);state['trace'].append(('free',pointer))
                if state['mode']&16 and pointer in NODES[:3]:inject(uc)
            elif a==0x575620:
                raise AssertionError('unexpected allocation with response execution controlled')
            elif a==STOP+0x580:
                ret,*arguments=words(uc,sp,6);cleanup=20;assert this==OBSERVER
                state['trace'].append(('delivery-observer',arguments))
                if state['mode']&1:
                    uc.mem_write(current+0x10,struct.pack('<II',0xfffffffd,PACKAGES[2]))
            else:
                ret,*arguments=words(uc,sp,5);cleanup=16;assert this==OBSERVER
                state['trace'].append(('game' if a==STOP+0x180 else 'input',arguments))
                if state['mode']&64:inject(uc)
            uc.reg_write(xc.UC_X86_REG_EAX,result);uc.reg_write(xc.UC_X86_REG_ECX,0xcccccccc);uc.reg_write(xc.UC_X86_REG_EDX,0xdddddddd)
            uc.reg_write(xc.UC_X86_REG_ESP,sp+4+cleanup);uc.reg_write(xc.UC_X86_REG_EIP,ret)
        def write(uc,access,a,size,value,user):
            if ARENA<=a<ARENA+0x1e000:state['writes'].append((a,size,value))
        uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,write);machines.append(uc)
    rng=random.Random(0x541fc0);counts=dict.fromkeys(PREFIXES,0)
    def compare(name,memory,arguments,this,mode,message,byte=False):
        outputs=[]
        for rebuilt,uc in zip((False,True),machines):
            uc.mem_write(ARENA,bytes(memory));uc.mem_write(STACK,struct.pack('<I',STOP)+struct.pack('<'+'I'*len(arguments),*arguments))
            uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
            for i,r in enumerate(PRESERVED):uc.reg_write(r,0x12345000+i)
            state.clear();state.update(trace=[],writes=[],mode=mode,message=message,injected=False,object_mutated=False)
            uc.emu_start(entries[name] if rebuilt else int(manifest['functions'][name]['address'],16),STOP,count=50000)
            assert uc.reg_read(xc.UC_X86_REG_EIP)==STOP,(name,'did not terminate')
            assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+4*len(arguments)
            assert all(uc.reg_read(r)==0x12345000+i for i,r in enumerate(PRESERVED))
            outputs.append((uc.reg_read(xc.UC_X86_REG_EAX)&255 if byte else None,bytes(uc.mem_read(ARENA,0x1e000)),state['trace'],state['writes']))
        assert outputs[0]==outputs[1],('state or call order mismatch',name,mode,hex(message),outputs[0][0],outputs[1][0],outputs[0][2:],outputs[1][2:])
        counts[name]+=1
    targets=(0,0xffffffff,0xfffffffe,0xfffffffd,0xfffffffc,0xfffffffb,0xfffffffa,OBJECTS[0])
    messages=(1,2,3,0x59bed120,0x5d4ce32d,0x030471ac,0xe10814a6,0xe10c4af9,0xfb814f13)
    for trial in range(8192):
        memory=bytearray(rng.randbytes(0x20000))
        def put(a,*v):struct.pack_into('<'+'I'*len(v),memory,a-ARENA,*[x&0xffffffff for x in v])
        mode=(trial//64)%128;message=messages[(trial//32)%len(messages)];length=(trial//512)%4
        put(QUEUE+0xe4,PACKAGES[0] if length else 0);put(QUEUE+0x10c,OBSERVER);put(OBSERVER,VTABLE)
        for offset,callback in ((0x18,STOP+0x180),(0x1c,STOP+0x1c0),(0x58,STOP+0x580)):put(VTABLE+offset,callback)
        memory[QUEUE+0x5251-ARENA]=(0,1,2,128,255)[(trial//64)%5];memory[QUEUE+1-ARENA]=trial%256
        for i,package in enumerate(PACKAGES):
            put(package+4,PACKAGES[i+1] if i+1<length else 0);put(package+0x5c,0 if trial%11==i else RESPONSES[i])
            put(package+0x6c,(trial//96+i)%4,TABLES[i])
            for j in range(3):
                put(RESPONSES[i]+j*32+4,RESPONSES[i]+(j+1)*32 if j<2 else 0)
                put(RESPONSES[i]+j*32+0xc,message if j<2 else message+1,0,0)
                put(TABLES[i]+j*16,message if (j==1 or (j==0 and trial%3==0)) else message+1,3,2+(trial+j)%2,VECTORS[i])
            put(TABLES[i]+0x80,message+1 if mode&2 else message,3,2,VECTORS[i]+0x40)
            put(VECTORS[i],OBJECTS[0],OBJECTS[1],OBJECTS[2]);put(VECTORS[i]+0x40,OBJECTS[3],OBJECTS[2],OBJECTS[1])
        for i,obj in enumerate(OBJECTS):
            put(obj+0x18,(1,2,4,6)[(trial//256+i)%4]);put(obj+0x3c,0 if trial%13==i else OBJECT_RESPONSES[i])
            for j in range(3):
                put(OBJECT_RESPONSES[i]+j*32+4,OBJECT_RESPONSES[i]+(j+1)*32 if j<2 else 0)
                put(OBJECT_RESPONSES[i]+j*32+0xc,message if j<2 else message+1,0,0)
        count=(trial//8)%4;put(QUEUE+0x4118,count,NODES[0] if count else 0,NODES[count-1] if count else 0)
        for i in range(3):put(NODES[i],0x79ab60,NODES[i+1] if i+1<count else 0,NODES[i-1] if i else 0,
                              (0,OBJECTS[2],0xdeadbeef)[(trial+i)%3],targets[(trial+i)%8],PACKAGES[(trial+i)%4],message,rng.getrandbits(32))
        compare('DeliverMessages',memory,[],QUEUE,mode,message)
        compare('DeliverObject',memory,[OBJECTS[trial%4],PACKAGES[trial%4],message,0x800000ff],QUEUE,0,message)
        compare('HandleObject4',memory,[OBJECTS[0],PACKAGES[0],message],0,0,message,True)
        compare('HandleObject6',memory,[OBJECTS[0],PACKAGES[0],message],0,0,message,True)
    assert all(coverage.values()),coverage
    report={'compiler':args.compiler,'comparisons':counts,'total':sum(counts.values()),'original_block_hits':coverage,'fingerprints':functions,
            'actual_composition':['object delivery','special object wrappers','message unlink and deleting helpers'],
            'controlled_interfaces':['ExecuteResponse 0x52c7c0','object mutators 0x516e90/0x52bd70','deallocation 0x575630','owner and delivery observer slots'],
            'limitations':['finite acyclic mapped queues and lists','response execution controlled','native parser/lifecycle not proved','MSVC and byte matching pending']}
    if args.mutation_checks:
        source=args.source.read_text();report['rejected_mutations']=[]
        mutations={
            'skip-delivery-observer':('if (value5251)', 'if (false)'),
            'wrong-observer-target':('(int)node->value10', '(int)node->valueC'),
            'skip-live-package-associations':('if (size)', 'if (size && target != 0xfffffffc)'),
            'skip-live-membership':('while (package && package != (FEPackage *)node->value14) package = package->next4;', 'package = (FEPackage *)node->value14;'),
            'fanout-all-packages-for-live-target':('if (target == 0xfffffffc) break;', 'if (false) break;'),
            'omit-package-response':('if (response) ExecuteResponse(response, 0, package, mask);', 'if (false) ExecuteResponse(response, 0, package, mask);'),
            'fresh-association-count':('j < count;', 'j < association->count8;'),
            'fresh-message-during-fanout':('package, associatedMessage, node->value1C', 'package, node->value18, node->value1C'),
            'cached-mask-during-fanout':('package, associatedMessage, node->value1C', 'package, associatedMessage, mask'),
            'skip-game-owner':('value10C->GameMessage(', 'if (false) value10C->GameMessage('),
            'skip-input-owner':('value10C->InputMessage(', 'if (false) value10C->InputMessage('),
            'wrong-internal-enable':('value1 = 1;', 'value1 = 0;'),
            'wrong-internal-disable':('value1 = 0;', 'value1 = 1;'),
            'skip-virtual-deletion':('((DeletingDestructor *)node->vtable0)[0](node, 1);', '((DeletingDestructor *)node->vtable0)[0](node, 0);'),
            'repeat-unlink-call':('        messages4114.first8 = node->next4;', '        messages4114.Remove(node);\n        continue;\n        messages4114.first8 = node->next4;'),
            'wrong-inline-sentinel':('(FEngQueueNode *)0xabadcafe;', '(FEngQueueNode *)0xabadcaff;'),
            'omit-inline-count':('--messages4114.count4;', 'messages4114.count4 = messages4114.count4;'),
            'skip-object-responses':('if (response) ExecuteResponse(response, object, package, mask);', 'if (false) ExecuteResponse(response, object, package, mask);'),
            'ignore-type4-handler':('object->type18 == 4 &&', 'object->type18 == 5 &&'),
            'ignore-type6-handler':('object->type18 == 6 &&', 'object->type18 == 7 &&'),
            'wrong-type4-navigation':('Call516E90(1, 0)', 'Call516E90(0, 1)'),
            'wrong-type6-navigation':('Call52BD70(1, 0)', 'Call52BD70(0, 1)'),
        }
        for name,(before,after) in mutations.items():
            assert before in source,name
            path=BUILD/('mutation-'+name+'.cpp');path.write_text(source.replace(before,after))
            result=subprocess.run([os.sys.executable,str(Path(__file__).resolve()),'--compiler',args.compiler,'--source',str(path)],cwd=ROOT,capture_output=True,text=True)
            (BUILD/('mutation-'+name+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode!=0 and 'state or call order mismatch' in result.stderr,name
            report['rejected_mutations'].append(name)
    (BUILD/'verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
