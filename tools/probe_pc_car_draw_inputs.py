#!/usr/bin/env python3
"""Trace ESP through the original car-draw prefix up to the wheel block.

This proves stack depth and direct frame references, not arbitrary callee memory
effects. Adjacent by-reference ranges and producer semantics are audited separately.
"""
from collections import defaultdict, deque
import hashlib
import json
import struct
import capstone
import pefile
from probe_pc_race_reward import ROOT, EXE

BEGIN, WHEELS = 0x620af9, 0x62700e
SWITCHES = {0x6230d4:(0x630d24,0x630d2c,44), 0x62313a:(0x630d58,0x630d98,26),
            0x6253fa:(0x630db4,0x630dbc,26)}
FLOAT_STORES = {'fst','fstp','fist','fistp','fisttp','fnstcw','fstcw'}


def trace(image, code, pops, virtual_cleanup):
    queue = deque([(BEGIN,0)]); states=defaultdict(set); references=[]; indexed=[]
    while queue:
        address,depth=queue.popleft()
        if depth in states[address]:continue
        assert abs(depth)<10000
        states[address].add(depth)
        if address>=WHEELS:continue
        i=code[address]; operands=i.operands
        for p in operands:
            if p.type==capstone.x86.X86_OP_MEM and p.mem.base==capstone.x86.X86_REG_ESP:
                offset=depth+p.mem.disp
                if p.mem.index:
                    indexed.append(dict(address=hex(address),base_offset=hex(offset),instruction=i.mnemonic))
                elif 0x790<=offset<0x7a0 or 0xb20<=offset<0xce0:
                    references.append(dict(address=hex(address),offset=hex(offset),depth=depth,
                        kind='address' if i.mnemonic=='lea' else 'write' if
                        p.access&capstone.CS_AC_WRITE or i.mnemonic in FLOAT_STORES else 'read'))
        if i.mnemonic=='push':depth-=4
        elif i.mnemonic=='pop':depth+=4
        elif i.mnemonic in ('add','sub') and operands[0].type==capstone.x86.X86_OP_REG and operands[0].reg==capstone.x86.X86_REG_ESP:
            assert operands[1].type==capstone.x86.X86_OP_IMM
            depth+=(1 if i.mnemonic=='add'else-1)*operands[1].imm
        elif i.mnemonic=='call':
            depth+=pops[operands[0].imm] if operands[0].type==capstone.x86.X86_OP_IMM else virtual_cleanup
        elif operands and operands[0].type==capstone.x86.X86_OP_REG and operands[0].reg==capstone.x86.X86_REG_ESP:
            assert i.mnemonic=='lea' and operands[1].mem.base==capstone.x86.X86_REG_ESP and not operands[1].mem.index
            depth+=operands[1].mem.disp
        if i.group(capstone.CS_GRP_JUMP):
            if operands[0].type==capstone.x86.X86_OP_IMM:queue.append((operands[0].imm,depth))
            else:
                table,mapping,count=SWITCHES[address]
                indices=set(image[mapping-0x400000:mapping-0x400000+count])
                for index in indices:queue.append((struct.unpack_from('<I',image,table-0x400000+index*4)[0],depth))
            if i.mnemonic=='jmp':continue
        if i.mnemonic=='ret':continue
        queue.append((address+i.size,depth))
    return states,references,indexed


def main():
    manifest=json.loads((ROOT/'config/SPEED2_EXE_1_2/car-render-inputs.json').read_text())
    raw=EXE.read_bytes();assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    image=pefile.PE(data=raw).get_memory_mapped_image();md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);md.detail=True
    for span in manifest['trace_spans']:
        a=int(span['address'],16)-0x400000
        assert hashlib.sha256(image[a:a+span['size']]).hexdigest()==span['sha256']
    pops={}
    for name,span in manifest['trace_callees'].items():
        a=int(name,16);body=image[a-0x400000:a-0x400000+span['size']]
        assert hashlib.sha256(body).hexdigest()==span['sha256']
        instructions=list(md.disasm(body,a))
        if 'tail'in span:
            assert instructions[-1].mnemonic=='jmp' and instructions[-1].operands[0].imm==int(span['tail'],16)
            assert manifest['trace_callees'][span['tail']]['cleanup']==span['cleanup']
        else:
            amounts={i.operands[0].imm if i.operands else 0 for i in instructions if i.mnemonic=='ret'}
            assert amounts=={span['cleanup']},(name,amounts)
        pops[a]=span['cleanup']
    code={i.address:i for i in md.disasm(image[0x220ae0:0x230d24],0x620ae0)}
    states,refs,indexed=trace(image,code,pops,4)
    assert not any(len(v)!=1 for v in states.values())
    assert states[WHEELS]=={0}
    for wrong in (0,8):
        alternatives,_,_=trace(image,code,pops,wrong)
        assert any(len(v)>1 for v in alternatives.values())
    writes=[r for r in refs if r['kind']=='write']
    assert {r['address']for r in writes}=={'0x620f11','0x620f22','0x620f2c','0x620f3a'}
    escapes=[r for r in refs if r['kind']=='address']
    assert {r['address']for r in escapes}=={'0x620fcb','0x620feb','0x620ffb','0x621012'}
    report=dict(reachable_instruction_states=len(states),conflicting_depths=0,wheel_entry_depth=0,
        indirect_call_cleanup=dict(address='0x621983',bytes=4,inference='unique among 0/4/8 consistent with CFG merges'),
        direct_buffer_references=sorted(refs,key=lambda r:r['address']),indexed_frame_references=indexed,
        apparent_position_overwrites={hex(a):next(iter(states[a]))for a in (0x6242c3,0x6242e8,0x62430d,0x624332,0x625140)},
        limitations=['stack-depth/direct-reference analysis; not a general pointer-alias or callee side-effect proof',
                     'indexed arrays and adjacent vector/matrix destinations require the documented instruction audit'])
    path=ROOT/'build/analysis/car-render-inputs/stack-trace.json';path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
