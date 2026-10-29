#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/EffectPassRuntime.h"
#include "../../Speed/Indep/Libs/D3DX/EffectValues.h"
#include "../../Speed/Indep/Src/World/CarRenderConsume.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <vector>
// Native GCC i386 member calls pass this as the first stack argument. These
// test-only aliases expose unchanged bodies without altering production files.
extern int RecoveredAdjust(D3DXEffectObject *,unsigned int,int,int) asm("_ZN16D3DXEffectObject30ProbeRecoveredAdjustReferencesEjii");
extern D3DXEffectBaseRecord *RecoveredResolve(D3DXEffectObject *,unsigned int,unsigned int,unsigned int) asm("_ZN16D3DXEffectObject30ProbeRecoveredResolveParameterEjjj");
extern int RecoveredObject(D3DXEffectObject *,unsigned int,unsigned int,D3DXEffectPoolLink *) asm("_ZN16D3DXEffectObject33ProbeRecoveredWriteObjectResourceEjjP18D3DXEffectPoolLink");
extern int RecoveredNotify(D3DXEffectObject *,D3DXEffectBaseRecord *) asm("_ZN16D3DXEffectObject25ProbeRecoveredNotifyValueEP20D3DXEffectBaseRecord");
extern "C" const double ug2FxIndexBias7A1E58=0.001;
namespace {
const unsigned int arena=0x2000000,arenaSize=0x10000,heap=0x3200000,heapSize=0x1000,queue2=arena+0x6100,tail=arena+0xae00;
D3DXEffectObject *const effect=reinterpret_cast<D3DXEffectObject *>(arena+0x1000);
struct Row { unsigned int kind,n,args[3],cw,mxcsr,live,status,failAllocation,failAt,failure,mutations,state,result,finalCw,traceSize,allocated,requests,liveAllocations; } current;
typedef char RowSize[sizeof(Row)==80?1:-1];
unsigned int calls,requests,allocated,liveAllocations;unsigned int live[256];std::vector<unsigned char> trace;
unsigned int Token(const void *p) { return reinterpret_cast<unsigned int>(p); }
void Append(const void *p,unsigned int n) { if(n) trace.insert(trace.end(),static_cast<const unsigned char *>(p),static_cast<const unsigned char *>(p)+n); }
void Record(const char *name,const unsigned int *args,unsigned int n) {
    unsigned short cw;asm volatile("fnstcw %0":"=m"(cw));Append(name,strlen(name)+1);Append(&n,4);Append(args,n*4);Append(effect,0x130);
    unsigned int state[]={static_cast<unsigned int>(cw)&~0x40u,requests,allocated};Append(state,sizeof(state));
}
void Transition() {
    if(calls!=1)return;
    if(current.mutations&1)effect->value3C=current.state==4?1:4;
    if(current.mutations&2) { effect->value108=queue2;effect->value10C=4; }
    if(current.mutations&4)effect->value118^=0x101;
    if(current.mutations&8) { unsigned short cw=0xa7f;asm volatile("fldcw %0"::"m"(cw)); }
}
int Service(const char *name,const unsigned int *args,unsigned int n) { Record(name,args,n);++calls;Transition();return calls==current.failAt?current.failure:0; }
struct Float80 { unsigned char bytes[10]; };
struct Fp { unsigned short env[14];unsigned int mxcsr;Float80 values[2]; };
void ResetFp() {
    unsigned short cw=current.cw,env[14];asm volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(current.mxcsr):"st");
    if(current.live==1)asm volatile("fld1":::"st");else if(current.live==2)asm volatile("fld1; fchs; fld1":::"st");
    asm volatile("fnstenv %0":"=m"(env));env[2]=(env[2]&0x3800)|current.status;asm volatile("fldenv %0"::"m"(env));
}
Fp State() {
    Fp f;memset(&f,0,sizeof(f));asm volatile("fnstenv %0; fldenv %0; stmxcsr %1":"=m"(f.env),"=m"(f.mxcsr));
    if(current.live)asm volatile("fld %%st(0); fstpt %0":"=m"(f.values[0])::"st");
    if(current.live==2)asm volatile("fld %%st(1); fstpt %0":"=m"(f.values[1])::"st");return f;
}
}
extern "C" void *__wrap_UG2FxMalloc(unsigned int bytes) {
    Record("malloc",&bytes,1);++requests;if(requests==current.failAllocation)return 0;
    unsigned int p=heap+allocated;allocated+=(bytes+15)&~15u;if(!bytes)allocated+=16;
    if(allocated>heapSize||liveAllocations>=256)__builtin_trap();live[liveAllocations++]=p;return reinterpret_cast<void *>(p);
}
extern "C" void __wrap_UG2FxFree(void *p) {
    unsigned int token=Token(p);Record("free",&token,1);if(!p)return;
    unsigned int j=0;for(;j<liveAllocations;++j)if(live[j]==token)break;if(j==liveAllocations)__builtin_trap();live[j]=live[--liveAllocations];
}
int D3DXEffectObject::AdjustReferences(unsigned int group,int direct,int all) { unsigned int a[]={Token(this),group,static_cast<unsigned int>(direct),static_cast<unsigned int>(all)};Record("adjust",a,4);return RecoveredAdjust(this,group,direct,all); }
D3DXEffectBaseRecord *D3DXEffectObject::ResolveParameter(unsigned int handle,unsigned int depth,unsigned int create) { unsigned int a[]={Token(this),handle,depth,create};Record("resolve",a,4);return RecoveredResolve(this,handle,depth,create); }
int D3DXEffectObject::WriteObjectResource(unsigned int object,unsigned int index,D3DXEffectPoolLink *shared) { unsigned int a[]={Token(this),object,index,Token(shared)};Record("object",a,4);return RecoveredObject(this,object,index,shared); }
int D3DXEffectObject::NotifyValue(D3DXEffectBaseRecord *record) { unsigned int a[]={Token(this),Token(record)};Record("notify",a,2);return RecoveredNotify(this,record); }
extern "C" unsigned int __stdcall UG2FxComAddRef(unsigned int token) { Record("addref",&token,1);if(current.mutations&2)effect->valueFC=tail;if(*reinterpret_cast<unsigned int *>(token)!=0x7b711c)__builtin_trap();return D3DXEffectBufferAddRef(reinterpret_cast<D3DXEffectBuffer *>(token)); }
extern "C" unsigned int __stdcall UG2FxComRelease(unsigned int token) { if(*reinterpret_cast<unsigned int *>(token)!=0x7b711c)__builtin_trap();return D3DXEffectBufferRelease(reinterpret_cast<D3DXEffectBuffer *>(token)); }
extern "C" int UG2FxPassCache66403C(D3DXEffectObject *e,int enabled,void *data) { unsigned int a[]={Token(e),static_cast<unsigned int>(enabled),Token(data)};return Service("cache_a",a,3); }
extern "C" int UG2FxPassCache664197(D3DXEffectObject *e,int enabled,void *data) { unsigned int a[]={Token(e),static_cast<unsigned int>(enabled),Token(data)};return Service("cache_b",a,3); }
extern "C" int UG2FxPassFastGroup668DFD(D3DXEffectObject *e,D3DXEffectGroup64 *g) { unsigned int a[]={Token(e),Token(g)};return Service("fast",a,2); }
extern "C" int UG2FxPassRecord669286(D3DXEffectObject *e,unsigned int g,unsigned int r,unsigned int index) { unsigned int a[]={Token(e),g,r,index};return Service("record",a,4); }
extern "C" int UG2FxPassCachedRecord66A25F(D3DXEffectObject *e,D3DXEffectGroup64 *g,D3DXEffectRecord64 *r) { unsigned int a[]={Token(e),Token(g),Token(r)};return Service("cached",a,3); }
extern "C" int UG2FxPrepareGroupA(D3DXEffectObject *e,unsigned int group) { unsigned int a[]={Token(e),group};return Service("prepare_a",a,2); }
extern "C" int UG2FxPrepareGroupB(D3DXEffectObject *e,unsigned int group) { unsigned int a[]={Token(e),group};return Service("prepare_b",a,2); }
extern "C" int UG2FxEvaluateRecord(D3DXEffectObject *,unsigned int,unsigned int) { __builtin_trap(); }
int main(int argc,char **argv) {
    char error[512];if(argc!=3||!MapOriginalImage(argv[1],error,sizeof(error)))return 2;
    for(unsigned int j=0;j<2;++j) { unsigned int a=j?heap:arena,n=j?heapSize:arenaSize;if(mmap(reinterpret_cast<void *>(a),n,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=reinterpret_cast<void *>(a))return 2; }
    FILE *f=fopen(argv[2],"rb");char magic[8];unsigned int count;if(!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2FXPS1",8)||fread(&count,4,1,f)!=1)return 2;
    unsigned int totalCalls=0;
    for(unsigned int i=0;i<count;++i) {
        if(fread(&current,sizeof(current),1,f)!=1||current.kind>6||current.traceSize>0x100000||fread(reinterpret_cast<void *>(arena),arenaSize,1,f)!=1||fread(reinterpret_cast<void *>(heap),heapSize,1,f)!=1)return 2;
        std::vector<unsigned char> expected(arenaSize+heapSize),expectedTrace(current.traceSize);
        if(fread(expected.data(),expected.size(),1,f)!=1||(current.traceSize&&fread(expectedTrace.data(),current.traceSize,1,f)!=1))return 2;
        calls=requests=allocated=liveAllocations=0;trace.clear();ResetFp();Fp before=State();unsigned int result=0;
        switch(current.kind) {
        case 0:result=UG2EffectBeginPass(current.args[0],current.args[1]);break;
        case 1:result=UG2EffectCommit(current.args[0]);break;
        case 2:result=UG2EffectEndPass(current.args[0]);break;
        case 3:result=effect->ApplyPassGroup(current.args[0],current.args[1]);break;
        case 4:result=effect->RecordTexture(current.args[0],current.args[1]);break;
        case 5:result=UG2EffectSetTexture(current.args[0],current.args[1],current.args[2]);break;
        case 6:
            result=UG2EffectBeginPass(current.args[0],0);result|=UG2EffectSetTexture(current.args[0],current.args[1],current.args[2]);
            result|=UG2EffectCommit(current.args[0]);result|=UG2EffectEndPass(current.args[0]);break;
        }
        Fp after=State();
        // Hardware forces reserved CW bit 6 on; Unicorn permits the CRT's
        // mapped write to leave it clear. Compare the defined control bits.
        bool fp=(after.env[0]&~0x40)==(current.finalCw&~0x40)&&after.mxcsr==before.mxcsr&&(after.env[2]&0x383f)==(before.env[2]&0x383f)&&after.env[4]==before.env[4]&&!memcmp(after.values,before.values,sizeof(before.values));
        if(result!=current.result||allocated!=current.allocated||requests!=current.requests||liveAllocations!=current.liveAllocations||trace!=expectedTrace||memcmp(reinterpret_cast<void *>(arena),expected.data(),arenaSize)||memcmp(reinterpret_cast<void *>(heap),expected.data()+arenaSize,heapSize)||!fp) {
            fprintf(stderr,"native effect pass mismatch row %u kind %u result %08x/%08x trace %u/%u arena %d heap %d FP %d CW %04x/%04x allocations %u/%u requests %u/%u\n",i,current.kind,result,current.result,static_cast<unsigned int>(trace.size()),current.traceSize,memcmp(reinterpret_cast<void *>(arena),expected.data(),arenaSize),memcmp(reinterpret_cast<void *>(heap),expected.data()+arenaSize,heapSize),fp,after.env[0],current.finalCw,allocated,current.allocated,requests,current.requests);return 1;
        }
        totalCalls+=calls;
    }
    asm volatile("fninit");if(fgetc(f)!=EOF)return 2;fclose(f);printf("native effect pass: %u exact original captures, %u controlled state calls; zero mismatches\n",count,totalCalls);return 0;
}
