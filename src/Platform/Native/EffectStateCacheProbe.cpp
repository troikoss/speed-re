#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/EffectStateCache.h"
#include "../../Speed/Indep/Libs/D3DX/EffectPassRuntime.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <vector>
namespace {
const unsigned int arena=0x2000000,arenaSize=0x10000,heap=0x3200000,heapSize=0x5000,tech=arena+0x3000,req=arena+0x5000,other=arena+0x1820,old=arena+0xa000,blocks=arena+0xc000,blockTable=arena+0x1d00;
D3DXEffectObject *const effect=reinterpret_cast<D3DXEffectObject *>(arena+0x1000);
struct Row { unsigned int kind,n,args[4],selected,cw,mxcsr,live,status,failAllocation,failAt,failure,outOnFailure,callbackAt,mutations,result,traceSize,requests,allocated,calls,ends,liveCount; } current;
typedef char RowSize[sizeof(Row)==96?1:-1];
unsigned int requests,allocated,calls,ends,liveCount,live[64];std::vector<unsigned char> trace;
void Put(unsigned int p,unsigned int value) { *reinterpret_cast<unsigned int *>(p)=value; }
void Append(const void *p,unsigned int n) { if(n)trace.insert(trace.end(),static_cast<const unsigned char *>(p),static_cast<const unsigned char *>(p)+n); }
void Record(const char *name,const unsigned int *args,unsigned int n,const void *payload=0,unsigned int bytes=0) {
    unsigned short cw;asm volatile("fnstcw %0":"=m"(cw));Append(name,strlen(name)+1);Append(&n,4);Append(args,n*4);Append(&bytes,4);Append(payload,bytes);Append(effect,0x130);Append(reinterpret_cast<void *>(tech),0x168);
    unsigned int state[]={requests,allocated,calls,ends,cw};Append(state,sizeof(state));
}
void Transition() {
    if(calls!=current.callbackAt)return;unsigned int m=current.mutations;
    if(m&1)effect->value0C=other;
    if(m&2)reinterpret_cast<unsigned short *>(&effect->value40)[1]=1-current.selected;
    if(m&4)for(unsigned int j=0;j<4;++j)Put(tech+current.selected*0xb4+0x8c+j*4,old+0x400+j*0x20);
    if(m&8) { unsigned short changed[]={1,2,1,2};memcpy(reinterpret_cast<void *>(req),changed,sizeof(changed)); }
    if((m&16)&&effect->valueCC)Put(effect->valueCC,0x7f800001);
}
int Call(const char *name,const unsigned int *args,unsigned int n,const void *payload=0,unsigned int bytes=0) { Record(name,args,n,payload,bytes);++calls;Transition();return calls==current.failAt?current.failure:0; }
int Block(const char *name,unsigned int token,unsigned int offset,int delta) { int result=Call(name,&token,1);if(!token) { fprintf(stderr,"native state cache mismatch: null %s receiver\n",name);__builtin_trap(); }unsigned int value=*reinterpret_cast<unsigned int *>(token+offset)+delta;Put(token+offset,value);return delta<0?value:result; }
struct Float80 { unsigned char bytes[10]; };
struct Fp { unsigned short env[14];unsigned int mxcsr;Float80 values[2]; };
void ResetFp() {
    unsigned short cw=current.cw,env[14];asm volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(current.mxcsr):"st");
    if(current.live==1)asm volatile("fld1":::"st");else if(current.live==2)asm volatile("fld1; fchs; fld1":::"st");
    asm volatile("fnstenv %0":"=m"(env));env[2]=(env[2]&0x3800)|current.status;asm volatile("fldenv %0"::"m"(env));
}
Fp State() {
    Fp f;memset(&f,0,sizeof(f));asm volatile("fnstenv %0; fldenv %0; stmxcsr %1":"=m"(f.env),"=m"(f.mxcsr));
    if(current.live)asm volatile("fld %%st(0); fstpt %0":"=m"(f.values[0])::"st");if(current.live==2)asm volatile("fld %%st(1); fstpt %0":"=m"(f.values[1])::"st");return f;
}
}
extern "C" void *__wrap_UG2FxMalloc(unsigned int bytes) { Record("malloc",&bytes,1);++requests;if(requests==current.failAllocation)return 0;unsigned int token=heap+allocated;allocated+=bytes?((bytes+15)&~15u):16;if(allocated>=0x3000||liveCount>=64)__builtin_trap();live[liveCount++]=token;return reinterpret_cast<void *>(token); }
extern "C" void __wrap_UG2FxFree(void *p) { unsigned int token=reinterpret_cast<unsigned int>(p);Record("free",&token,1);if(!token)return;unsigned int j=0;for(;j<liveCount;++j)if(live[j]==token)break;if(j==liveCount)__builtin_trap();live[j]=live[--liveCount]; }
extern "C" void __stdcall D3DXEffectDebugMute(unsigned int mute) { Record("DebugMute",&mute,1); }
extern "C" int UG2FxStateBlockBegin(unsigned int device) { return Call("Begin",&device,1); }
extern "C" int UG2FxStateBlockEnd(unsigned int device,unsigned int *out) { unsigned int a[]={device,*out};int result=Call("End",a,2);if(result>=0||current.outOnFailure) { unsigned int token=blocks+ends*0x20;++ends;if(ends>32)__builtin_trap();Put(token,blockTable);Put(token+4,1);Put(token+8,0);Put(token+12,0);*out=token; }return result; }
extern "C" int UG2FxStateBlockCapture(unsigned int block) { return Block("Capture",block,8,1); }
extern "C" int UG2FxStateBlockApply(unsigned int block) { return Block("Apply",block,12,1); }
extern "C" unsigned int UG2FxStateBlockRelease(unsigned int block) { return Block("Release",block,4,-1); }
extern "C" int UG2FxCacheSetVertexShader(unsigned int device,unsigned int shader) { unsigned int a[]={device,shader};return Call("SetVertexShader",a,2); }
extern "C" int UG2FxCacheSetPixelShader(unsigned int device,unsigned int shader) { unsigned int a[]={device,shader};return Call("SetPixelShader",a,2); }
#define CACHE_UPLOAD(Name,Bytes) extern "C" int UG2FxCache##Name(unsigned int device,unsigned int start,const void *data,unsigned int count) { unsigned int a[]={device,start,count};return Call(#Name,a,3,data,count*Bytes); }
CACHE_UPLOAD(VertexFloat,16)
CACHE_UPLOAD(VertexInt,16)
CACHE_UPLOAD(VertexBool,4)
CACHE_UPLOAD(PixelFloat,16)
CACHE_UPLOAD(PixelInt,16)
CACHE_UPLOAD(PixelBool,4)
#undef CACHE_UPLOAD
extern "C" int UG2FxCacheSetTexture(unsigned int device,unsigned int stage,unsigned int texture) { unsigned int a[]={device,stage,texture};return Call("SetTexture",a,3); }
extern "C" int UG2FxCacheSetSampler(unsigned int device,unsigned int stage,unsigned int state,unsigned int value) { unsigned int a[]={device,stage,state,value};return Call("SetSampler",a,4); }
int main(int argc,char **argv) {
    char error[512];if(argc!=3||!MapOriginalImage(argv[1],error,sizeof(error)))return 2;
    for(unsigned int j=0;j<2;++j) { unsigned int a=j?heap:arena,n=j?heapSize:arenaSize;if(mmap(reinterpret_cast<void *>(a),n,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=reinterpret_cast<void *>(a))return 2; }
    FILE *f=fopen(argv[2],"rb");char magic[8];unsigned int count;if(!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2FXSC1",8)||fread(&count,4,1,f)!=1)return 2;unsigned int totalCalls=0;
    for(unsigned int i=0;i<count;++i) {
        if(fread(&current,sizeof(current),1,f)!=1||current.kind>10||current.traceSize>0x400000||fread(reinterpret_cast<void *>(arena),arenaSize,1,f)!=1||fread(reinterpret_cast<void *>(heap),heapSize,1,f)!=1)return 2;
        std::vector<unsigned char> expected(arenaSize+heapSize),expectedTrace(current.traceSize);if(fread(expected.data(),expected.size(),1,f)!=1||(current.traceSize&&fread(expectedTrace.data(),current.traceSize,1,f)!=1))return 2;
        requests=allocated=calls=ends=liveCount=0;if(effect->valueCC)live[liveCount++]=effect->valueCC;trace.clear();ResetFp();Fp before=State();unsigned short *sizes=reinterpret_cast<unsigned short *>(current.args[1]);unsigned int result=0;
        switch(current.kind) {
        case 0:result=UG2FxPassCache66403C(effect,current.args[0],sizes);break;
        case 1:result=UG2FxPassCache664197(effect,current.args[0],sizes);break;
        case 2:result=effect->CaptureVertexConstants(current.args[0],sizes);break;
        case 3:result=effect->CaptureVertexSamplers(current.args[0],sizes);break;
        case 4:result=effect->CapturePixelConstants(current.args[0],sizes);break;
        case 5:result=effect->CapturePixelSamplers(current.args[0],sizes);break;
        case 6:result=effect->ResetVertexConstants(current.args[0],sizes);break;
        case 7:result=effect->ResetPixelConstants(current.args[0],sizes);break;
        case 8:result=effect->ResetVertexSamplers(current.args[0],sizes);break;
        case 9:result=effect->ResetPixelSamplers(current.args[0],sizes);break;
        case 10:result=D3DXEffectSetVertexBooleans(current.args[0],current.args[1],reinterpret_cast<const unsigned int *>(current.args[2]),current.args[3]);break;
        }
        Fp after=State();bool fp=after.env[0]==before.env[0]&&after.mxcsr==before.mxcsr&&(after.env[2]&0x383f)==(before.env[2]&0x383f)&&after.env[4]==before.env[4]&&!memcmp(after.values,before.values,sizeof(before.values));
        if(result!=current.result||requests!=current.requests||allocated!=current.allocated||calls!=current.calls||ends!=current.ends||liveCount!=current.liveCount||trace!=expectedTrace||memcmp(reinterpret_cast<void *>(arena),expected.data(),arenaSize)||memcmp(reinterpret_cast<void *>(heap),expected.data()+arenaSize,heapSize)||!fp) {
            fprintf(stderr,"native state cache mismatch row %u kind %u result %08x/%08x trace %u/%u arena %d heap %d FP %d calls %u/%u allocations %u/%u\n",i,current.kind,result,current.result,static_cast<unsigned int>(trace.size()),current.traceSize,memcmp(reinterpret_cast<void *>(arena),expected.data(),arenaSize),memcmp(reinterpret_cast<void *>(heap),expected.data()+arenaSize,heapSize),fp,calls,current.calls,allocated,current.allocated);return 1;
        }
        totalCalls+=calls;
    }
    asm volatile("fninit");if(fgetc(f)!=EOF)return 2;fclose(f);printf("native state cache: %u exact original captures, %u controlled GPU calls; zero mismatches\n",count,totalCalls);return 0;
}
