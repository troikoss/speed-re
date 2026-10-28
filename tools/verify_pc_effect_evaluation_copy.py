#!/usr/bin/env python3
"""Compare original effect-to-double copy bodies and recursive cursor walkers.

All four recovered routines execute, including nested reflection/member walks.
This verifies VM input conversion, not the shader VM or GPU effect activation.
"""
import argparse
from collections import Counter
import hashlib,json,random,struct,subprocess,sys
from pathlib import Path
import pefile
from unicorn import UC_HOOK_CODE,UcError
import verify_pc_effect_metadata as md

ROOT,EXE,LIB=md.ROOT,md.EXE,md.LIB;SOURCE=LIB/'EffectEvaluationCopy.cpp';BODY=LIB/'EffectEvaluationCopy.inc'
BUILD=ROOT/'build/analysis/effect-evaluation-copy';MANIFEST=ROOT/'config/SPEED2_EXE_1_2/effect-evaluation-copy.json'
ARENA,HEAP,STOP=md.ARENA,md.HEAP,md.STOP
TYPE,TABLE,INPUT,OUTPUT,CURSORS,PARAMETER=[ARENA+x for x in(0x2000,0x3000,0x6000,0x9000,0xe000,0xe100)]
SPANS=dict(walk_vectors=(0x65fab5,1034),walk_scalars=(0x65febf,838),copy_vectors=(0x662ecc,921),copy_scalars=(0x663265,777))
NAMES=dict(walk_vectors='D3DXEffectWalkEvaluationVectors',walk_scalars='D3DXEffectWalkEvaluationScalars',copy_vectors='D3DXEffectCopyEvaluationVectors',copy_scalars='D3DXEffectCopyEvaluationScalars')
md.SOURCES.append(SOURCE);md.BUILD=BUILD;md.FREE.update(NAMES);md.STDCALL.update({k:28 for k in SPANS});md.VOID.update(SPANS)


def pack(words):return struct.pack('<'+'I'*len(words),*(x&0xffffffff for x in words))


def tree(c):
    def leaf(kind,shape,elements,rows,columns):return(kind,shape,elements,rows,columns,())
    leaf0=leaf(c['kind'],c['shape'],c['elements'],c['rows'],c['columns'])
    if c['depth']==0:return leaf0
    children=[leaf(2,1,2,3,2),leaf0,leaf(3,3,1,1,3)]
    if c['depth']==2:children[1]=(0,5,2,0,0,(leaf(1,2,2,2,3),leaf0,leaf(3,0,0,2,1)))
    return(0,5,c['elements'],0,0,tuple(children))


def serialize(c):
    types=[];reflection=bytearray(0x2000);next_offset=0x40
    def node(item,root=False):
        nonlocal next_offset
        kind,shape,elements,rows,columns,children=item;offset=next_offset;next_offset+=16
        reflected=shape if shape==5 or not c['reflect_override'] else c['reflect_shape']
        struct.pack_into('<H',reflection,offset,reflected&0xffff)
        types.extend([kind,shape,0x12345678,0x89abcdef,elements])
        if shape==5:
            types.append(len(children));list_offset=next_offset;next_offset+=len(children)*8;struct.pack_into('<I',reflection,offset+12,list_offset)
            for j,child in enumerate(children):child_offset=node(child);struct.pack_into('<2I',reflection,list_offset+j*8,0x55550000+j,child_offset)
        else:types.extend([rows,columns])
        return offset
    root=node(tree(c),True);return pack(types),bytes(reflection),root


def cases(n):
    rng=random.Random(0x662ecc)
    for i in range(n):
        c=dict(kind=(1,2,3,0,17,0xffffffff)[i%6],shape=(0,1,2,3,4,6,0xffffffff)[(i//6)%7],elements=(0,1,2,3)[(i//7)%4],rows=(i//11)%5,columns=(i//13)%5,
            depth=(i//17)%3,reflection=bool(i%2),reflect_override=i%7==0,reflect_shape=(0,1,2,3,4,0xffff)[(i//19)%6],override=(0xffffffff,0,1,2,3)[(i//5)%5],
            count=(0,1,2,3,5,8,16,32,64,0xffff,0xffffffff)[(i//3)%11],alias=(0,0,0,-32,16,64)[(i//23)%6],seed=rng.getrandbits(32),snan=i%31==0)
        # First cases enter every numeric leaf/array/cache-independent copy path.
        if i<96:
            c.update(kind=1+i%3,shape=(i//3)%4,depth=(i//24)%3,rows=1+(i//12)%4,columns=1+(i//9)%4,elements=1+i%3,
                reflection=bool((i//2)%2),reflect_override=False,override=0xffffffff,count=(1,2,5,16,64)[(i//4)%5])
        c['types'],c['table'],c['root']=serialize(c);yield c


def memory(c,native=False):
    rng=random.Random(c['seed']);data=bytearray(b'\xa5'*md.core.SIZE)
    data[0x2000:0x2000+len(c['types'])]=c['types'];data[0x3000:0x5000]=c['table']
    # Kind-dependent FILD/FLD reads see the same raw words, including signs,
    # denormals, infinities and quiet NaNs. SNaNs are kept explicit for native QA.
    words=[0,0x80000000,1,0xffffffff,0x7fffffff,0x3f800000,0xbf800000,0x00800000,0x007fffff,0x7f800000,0xff800000,0x7fc12345,0xffc12345,0x41234567]
    if c['snan']:words += [0x7f812345,0xff812345]
    if native:words=[0,1,0xffffffff,0x80000000,0x7fffffff,0x3f800000,0xbf800000,0x40000000,0xc0000000]
    for j in range(0x800):struct.pack_into('<I',data,0x6000+j*4,words[(j+c['seed'])%len(words)]if native or j%3 else rng.getrandbits(32))
    return bytes(data)


class Machine(md.Machine):
    def __init__(self,image,linked=None,gcc=False):
        super().__init__(image,linked,gcc)
        if not linked:self.entries.update({k:a for k,(a,n)in SPANS.items()})
        for key in('walk_vectors','walk_scalars'):
            at=self.entries[key];self.u.hook_add(UC_HOOK_CODE,self.walk_entry,user_data=key,begin=at,end=at)
        self.instruction_limit=1000000
    def walk_entry(self,u,address,size,kind):
        t,s,d,base,offset,left,elements=self.args(7)
        self.events.append((kind,(self.word(t),self.word(s),self.word(d),base,offset,self.word(left),elements),hashlib.sha256(self.state()).hexdigest()))
    def run(self,kind,c,native=False):
        self.u.mem_write(ARENA,memory(c,native));self.u.mem_write(HEAP,b'\x5a'*md.core.HSIZE);self.put(0,0x12345678);self.events=[];self.live={};self.allocated=0;self.requests=0
        destination=INPUT+c['alias']if c['alias']else OUTPUT;self.put(CURSORS,TYPE,INPUT,destination,c['count'],0x12345678);self.put(PARAMETER,INPUT,TYPE)
        base=TABLE if c['reflection']else 0;entry=TABLE+0x10;self.put(entry+12,c['root'])
        if kind.startswith('walk'):
            result=self.call(kind,[CURSORS,CURSORS+4,CURSORS+8,base,c['root'],CURSORS+12,c['override']])
        else:result=self.call(kind,[PARAMETER,destination,c['count'],base,entry,CURSORS+16,c['override']])
        return self.snapshot(result)


def verify(image,linked,gcc,n):
    a,b=Machine(image),Machine(image,linked,gcc);counts=Counter();calls=Counter()
    for i,c in enumerate(cases(n)):
        for kind in SPANS:
            try:x=a.run(kind,c);y=b.run(kind,c)
            except(AssertionError,UcError)as e:raise AssertionError(('effect evaluation copy mismatch',kind,i,str(e)))from e
            if x!=y:
                diffs=[(hex(j),v,w)for j,(v,w)in enumerate(zip(x[0],y[0]))if v!=w][:12];event=next(((j,v,w)for j,(v,w)in enumerate(zip(x[3],y[3]))if v!=w),None)
                raise AssertionError(('effect evaluation copy mismatch',kind,i,c,diffs,event,len(x[3]),len(y[3])))
            counts[kind]+=1;calls.update(e[0]for e in x[3])
    return dict(comparisons=dict(counts),recursive_calls=dict(calls),failures=0)


def native_capture(m):
    return pack([m.word(CURSORS)-ARENA,m.word(CURSORS+4)-ARENA,m.word(CURSORS+8)-ARENA,m.word(CURSORS+12),m.word(CURSORS+16)])+bytes(m.u.mem_read(ARENA+0x5000,0x8000))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--gcc',action='store_true');parser.add_argument('--cases',type=int,default=1024);parser.add_argument('--replace',type=Path);parser.add_argument('--write-native-fixtures',type=Path);parser.add_argument('--native-dump',type=Path);parser.add_argument('--mutation-checks',action='store_true')
    args=parser.parse_args();raw=EXE.read_bytes();pe=pefile.PE(data=raw);image=pe.get_memory_mapped_image();manifest=json.loads(MANIFEST.read_text());assert hashlib.sha256(raw).hexdigest()==manifest['executable_sha256']
    for row in manifest['functions'].values():
        at=int(row['address'],16)-0x400000;assert hashlib.sha256(image[at:at+row['size']]).hexdigest()==row['sha256']
    linked=md.build({SOURCE.name:args.replace}if args.replace else{},args.gcc);report=verify(image,linked,args.gcc,args.cases);report.update(compiler='GCC i386 -O1'if args.gcc else'MSVC 7.1 /Ox /Ob0',byte_identity='not established')
    if args.write_native_fixtures:
        content=b'UG2FXEF1'+pack([128])
        for i,c in enumerate(cases(128)):
            content+=pack([i,c['root'],int(c['reflection']),c['count'],c['override'],c['alias'],md.core.SIZE])+memory(c,True)
        args.write_native_fixtures.write_bytes(content)
    if args.native_dump:
        raw=args.native_dump.read_bytes();assert raw[:8]==b'UG2FXED1';at=8;a,b=Machine(image),Machine(image,linked,args.gcc);row=0
        for i,c in enumerate(cases(128)):
            for kind in SPANS:
                x=a.run(kind,c,True);y=b.run(kind,c,True);assert x==y,('effect evaluation copy mismatch','native model',kind,i)
                expected=native_capture(a);index,size=struct.unpack_from('<2I',raw,at);at+=8;actual=raw[at:at+size];at+=size
                assert index==row and actual==expected,('native evaluation copy mismatch',kind,i,[(hex(j),v,w)for j,(v,w)in enumerate(zip(expected,actual))if v!=w][:12]);row+=1
        assert at==len(raw);report['native_replay']=row
    if args.mutation_checks:
        mutations={
            'reflection-shape':('body','int shape=tableBase ? static_cast<int>(*reflection) : static_cast<int>(type[1]);','int shape=static_cast<int>(type[1]);'),
            'default-element-count':('body','if (!arrays) arrays=1;','if (!arrays) arrays=0;'),
            'explicit-zero-elements':('body','unsigned int arrays=elements;','unsigned int arrays=elements ? elements : 1;'),
            'shape-zero-gate':('body','if (shape<0)','if (shape<=0)'),
            'shape-three-gate':('body','if (shape>3)','if (shape>=3)'),
            'struct-child-origin':('body','*children=type+6','*children=type+7'),
            'struct-array-reset':('body','if (!*remaining) return;\n            *typeCursor=children;','if (!*remaining) return;'),
            'reflected-member-offset':('body','list[member*2+1]','list[member*2]'),
            'recursive-element-count':('body','memberOffset,remaining,0xffffffff','memberOffset,remaining,1'),
            'top-array-source-reset':('body','unsigned int *child=children;','unsigned int *child=children;source=reinterpret_cast<unsigned int *>(parameter->value00);'),
            'top-exhausted-member-gate':('body','for (unsigned int member=0;member<members;++member) {\n                if (!count) break;','for (unsigned int member=0;member<members;++member) {'),
            'matrix-row-count':('body','unsigned int rows=type[5];','unsigned int rows=type[5]+1;'),
            'matrix-column-count':('body','unsigned int columns=type[6];','unsigned int columns=type[6]+1;'),
            'scalar-integer-matrix-source':('body','reinterpret_cast<int *>(source)[row]','reinterpret_cast<int *>(matrix)[row]'),
            'vector-integer-matrix-source':('body','reinterpret_cast<int *>(source)[row+column*4]','reinterpret_cast<int *>(matrix)[row+column*4]'),
            'scalar-float-matrix-source':('body','reinterpret_cast<float *>(matrix)[row]','reinterpret_cast<float *>(source)[row]'),
            'vector-float-matrix-source':('body','reinterpret_cast<float *>(matrix)[row+column*4]','reinterpret_cast<float *>(source)[row+column*4]'),
            'matrix-source-stride':('body','matrix+=16;','matrix+=4;'),
            'matrix-vector-output-stride':('body','destination+=4;','destination+=1;'),
            'scalar-overlap-output-stride':('body','destination+=FX_COPY_SCALAR ? 1 : 4;','destination+=4;'),
            'float-four-lane-tail':('body','destination[3]=static_cast<double>(reinterpret_cast<float *>(source)[3]);','(void)0;'),
            'matrix-signed-integer':('body','static_cast<double>(reinterpret_cast<int *>(source)[row]);','static_cast<double>(source[row]);'),
            'matrix-remaining-count':('body','--count;','count-=2;'),
            'ordinary-column-count':('body','unsigned int columns=type[6],padding','unsigned int columns=type[5],padding'),
            'matrix-three-padding':('body','(4-columns)*4 : 0','(4-columns) : 0'),
            'scalar-integer-first-lane':('body','static_cast<double>(*reinterpret_cast<int *>(source));','static_cast<double>(reinterpret_cast<int *>(source)[1]);'),
            'vector-integer-component-count':('body','row<rows;++row) destination[row]','row<4;++row) destination[row]'),
            'ordinary-source-stride':('body','source+=4;','source+=1;'),
            'published-source-cursor':('body','*sourceCursor=source;','(void)0;'),
            'published-remaining-count':('body','*remaining=count;','(void)0;'),
            'leaf-type-cursor':('body','*typeCursor+=7;','*typeCursor+=6;'),
            'reported-vector-count':('body','>>(FX_COPY_SCALAR ? 3 : 5)','>>(FX_COPY_SCALAR ? 3 : 4)'),
            'top-reflection-offset':('body','reinterpret_cast<unsigned int *>(entry)[3]','reinterpret_cast<unsigned int *>(entry)[4]'),
            'recursive-reflection-offset':('body','tableBase+typeOffset','tableBase+typeOffset+4'),
            'recursive-vector-mode':('wrapper','#define FX_COPY_SCALAR 0','#define FX_COPY_SCALAR 1'),
            'recursive-vector-callee':('wrapper','#define FX_COPY_WALK D3DXEffectWalkEvaluationVectors','#define FX_COPY_WALK D3DXEffectWalkEvaluationScalars'),
        }
        rejected=[]
        for name,(unit,before,after)in mutations.items():
            source=BODY if unit=='body'else SOURCE;content=source.read_text();assert before in content,(name,before)
            path=BUILD/('mutation-'+('gcc-'if args.gcc else'msvc-')+name+('.inc'if unit=='body'else'.cpp'));path.write_text(content.replace(before,after,1))
            if unit=='body':
                wrapper=path.with_suffix('.cpp');wrapper.write_text(SOURCE.read_text().replace('"EffectEvaluationCopy.inc"','"'+str(path)+'"'));path=wrapper
            else:path.write_text(path.read_text().replace('"EffectEvaluationCopy.inc"','"'+str(BODY)+'"'))
            command=[sys.executable,__file__,'--replace',str(path),'--cases','256']+(['--gcc']if args.gcc else[])
            run=subprocess.run(command,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);path.with_suffix('.log').write_text(run.stdout)
            assert run.returncode and 'effect evaluation copy mismatch'in run.stdout,(name,run.stdout[-3500:]);rejected.append(name)
        report['rejected_mutations']=rejected
    (BUILD/('verification-'+('gcc'if args.gcc else'msvc')+'.json')).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
