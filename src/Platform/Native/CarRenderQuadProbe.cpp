#include "OriginalImage.h"
#include "../../Speed/Indep/Src/World/CarWheelSecondary.h"
#include "../../Speed/Indep/Src/World/CarRenderMaterial.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <vector>
// Test-only observation wrappers; their callees are unchanged recovered bodies
// compiled under alternate names. The production ABI bridge and body run.
extern "C" { unsigned int ug2SolidDevice870974=0; } // host-owned test device token
void ProbeRecoveredBindTexture(CarRenderTexture *);
void ProbeRecoveredUploadWorld(WheelMatrix *,int);
namespace {
const unsigned int arena=0x2000000,arenaSize=0x40000,tech=arena,param=arena+0x1000,mode=arena+0x1b800,quad=arena+0x23000,device=arena+0x28000;
struct Row { unsigned int composed,mutate,swap,flip,extraAfter,edit,counterAfter,fourth,live,top,status,mxcsr,cw,hresult,traceCrc,arenaCrc,regions,traceSize; };
struct Region { unsigned int address,size,crc;std::vector<unsigned char> bytes,after; };
struct Stored80 { unsigned char bytes[10]; };
struct Fp { unsigned short env[14];unsigned int mxcsr;unsigned char values[20]; };
typedef char RowSize[(sizeof(Row)==72)?1:-1];
Row current;bool worldSeen;unsigned int crc,apiCalls,rows,traceSize;unsigned char traceBytes[65536];
unsigned int Crc(const void *data,unsigned int bytes,unsigned int previous=0) {
    const unsigned char *p=static_cast<const unsigned char *>(data);unsigned int value=~previous;
    for(unsigned int j=0;j<bytes;++j) { value^=p[j];for(int bit=0;bit<8;++bit) value=(value>>1)^((value&1)?0xedb88320:0); }
    return ~value;
}
void Append(const void *p,unsigned int n) { if(traceSize+n>sizeof(traceBytes)) __builtin_trap();if(n) memcpy(traceBytes+traceSize,p,n);traceSize+=n;crc=Crc(p,n,crc); }
void Record(const char *name,const unsigned int *args,unsigned int n,const void *payload=0,unsigned int bytes=0) {
    ++apiCalls;Append(name,strlen(name)+1);Append(&n,4);Append(args,n*4);Append(&bytes,4);Append(payload,bytes);
    unsigned int state[]={reinterpret_cast<unsigned int>(ug2RenderTechnique870910),ug2RenderVertex87092C,ug2RenderQuadCalls872F08,ug2RenderQuadTexture8708B4,reinterpret_cast<unsigned int>(ug2SolidViewMode870848),ug2RenderQuadEnabled7FF05C};Append(state,sizeof(state));
}
void Transition(unsigned int stage) {
    if(current.swap&(1u<<stage)) ug2RenderTechnique870910=reinterpret_cast<CarRenderTechnique *>(tech+(stage+1)*0x28);
    if(current.flip&(1u<<stage)) ug2RenderQuadEnabled7FF05C=ug2RenderQuadEnabled7FF05C ? 0 : 0x80;
    if(stage==0&&current.mutate) { *reinterpret_cast<unsigned int *>(quad)=current.edit;ug2SolidViewMode870848=reinterpret_cast<int *>(mode+0x10); }
    if(stage==1&&current.mutate) ug2RenderQuadTexture8708B4=current.extraAfter;
    if(stage==6&&current.mutate) ug2RenderQuadCalls872F08=current.counterAfter;
}
void ResetFp() {
    unsigned short cw=current.cw,env[14];asm volatile("fninit; fldcw %0; ldmxcsr %1" : : "m"(cw),"m"(current.mxcsr) : "st");
    if(!current.composed) {
        if(current.live==1) asm volatile("fld1" : : : "st");
        else if(current.live==2) asm volatile("fld1; fchs; fld1" : : : "st");
    }
    asm volatile("fnstenv %0" : "=m"(env));env[2]=(env[2]&0x3800)|(current.composed ? 0 : current.status);asm volatile("fldenv %0" : : "m"(env));
}
Fp State() {
    Fp value;memset(&value,0,sizeof(value));asm volatile("fnstenv %0; fldenv %0; stmxcsr %1" : "=m"(value.env),"=m"(value.mxcsr));
    if(!current.composed) {
        if(current.live) asm volatile("fld %%st(0); fstpt %0" : "=m"(*reinterpret_cast<Stored80 *>(value.values)) : : "st");
        if(current.live==2) asm volatile("fld %%st(1); fstpt %0" : "=m"(*reinterpret_cast<Stored80 *>(value.values+10)) : : "st");
    }
    return value;
}
bool Same(const Fp &a,const Fp &b,bool flags) { return a.env[0]==b.env[0]&&(a.env[2]&0x3800)==(b.env[2]&0x3800)&&(!flags||(a.env[2]&0x3f)==(b.env[2]&0x3f))&&a.env[4]==b.env[4]&&a.mxcsr==b.mxcsr&&!memcmp(a.values,b.values,20); }
}
void CarRenderBindTexture(CarRenderTexture *texture) { unsigned int args[]={reinterpret_cast<unsigned int>(texture)};Record("bind",args,1);Transition(0);if(current.composed) ProbeRecoveredBindTexture(texture); }
void CarRenderUploadWorld(WheelMatrix *world,int view) { unsigned int args[]={reinterpret_cast<unsigned int>(world),static_cast<unsigned int>(view)};worldSeen=true;Record("world",args,2);Transition(1);if(current.composed) ProbeRecoveredUploadWorld(world,view); }
extern "C" int UG2EffectSetTexture(unsigned int effect,unsigned int handle,unsigned int texture) { unsigned int args[]={effect,handle,texture};Record("UG2EffectSetTexture",args,3);if(worldSeen) Transition(2);return current.hresult; }
extern "C" int UG2EffectCommit(unsigned int effect) { Record("UG2EffectCommit",&effect,1);Transition(3);return current.hresult; }
extern "C" int UG2EffectBeginPass(unsigned int effect,unsigned int pass) { unsigned int args[]={effect,pass};Record("UG2EffectBeginPass",args,2);Transition(4);return current.hresult; }
extern "C" int UG2EffectEndPass(unsigned int effect) { Record("UG2EffectEndPass",&effect,1);Transition(6);return current.hresult; }
extern "C" int UG2RenderDrawImmediate(unsigned int token,unsigned int kind,unsigned int count,const void *vertices,unsigned int stride) {
    unsigned int args[]={token,kind,count,stride};unsigned char packed[96];for(int j=0;j<4;++j) memcpy(packed+j*24,static_cast<const unsigned char *>(vertices)+j*stride,24);
    Record("UG2RenderDrawImmediate",args,4,packed,96);Transition(5);return current.hresult;
}
extern "C" int UG2EffectSetMatrix(unsigned int effect,unsigned int handle,const WheelMatrix *matrix) { unsigned int args[]={effect,handle};Record("UG2EffectSetMatrix",args,2,matrix,64);return current.hresult; }
extern "C" int UG2RenderSetTransform(unsigned int token,unsigned int state,const WheelMatrix *matrix) { unsigned int args[]={token,state};Record("UG2RenderSetTransform",args,2,matrix,64);return current.hresult; }
extern "C" int UG2EffectSetVector(unsigned int effect,unsigned int handle,const float *values) { unsigned int args[]={effect,handle,4};Record("UG2EffectSetVector",args,3,values,16);return current.hresult; }
extern "C" int UG2EffectSetVectorArray(unsigned int effect,unsigned int handle,const float *values,unsigned int count) { unsigned int args[]={effect,handle,count*4};Record("UG2EffectSetVectorArray",args,3,values,count*16);return current.hresult; }
extern "C" int UG2EffectSetIntArray(unsigned int effect,unsigned int handle,const int *values,unsigned int count) { unsigned int args[]={effect,handle,count};Record("UG2EffectSetIntArray",args,3,values,count*4);return current.hresult; }
extern "C" int UG2RenderSetState(unsigned int token,unsigned int state,unsigned int value) { unsigned int args[]={token,state,value};Record("UG2RenderSetState",args,3);return current.hresult; }
extern "C" int UG2EffectSetFloat(unsigned int effect,unsigned int handle,float value) { unsigned int args[]={effect,handle,0};memcpy(args+2,&value,4);Record("UG2EffectSetFloat",args,3);return current.hresult; }
extern "C" int UG2EffectSetInt(unsigned int effect,unsigned int handle,int value) { unsigned int args[]={effect,handle,static_cast<unsigned int>(value)};Record("UG2EffectSetInt",args,3);return current.hresult; }
int main(int argc,char **argv) {
    char error[256];if((argc!=3&&argc!=4)||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;
    if(argc==4) {
        if(!strcmp(argv[3],"missing-api")) UG2RenderSetTexture(0,0,0);
        return 4;
    }
    if(mmap(reinterpret_cast<void *>(arena),arenaSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=reinterpret_cast<void *>(arena)) return 2;
    FILE *f=fopen(argv[2],"rb");char magic[8];unsigned int count;if(!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2QDS01",8)||fread(&count,4,1,f)!=1) return 2;
    unsigned int composed=0,totalApis=0;
    for(unsigned int i=0;i<count;++i) {
        if(fread(&current,sizeof(current),1,f)!=1||current.regions>512||current.traceSize>65536||fread(reinterpret_cast<void *>(arena),arenaSize,1,f)!=1) return 2;
        std::vector<Region> regions(current.regions);
        for(unsigned int j=0;j<current.regions;++j) {
            Region &r=regions[j];if(fread(&r,12,1,f)!=1||r.size>0x10000) return 2;r.bytes.resize(r.size);
            r.after.resize(r.size);if(fread(r.bytes.data(),r.size,1,f)!=1||fread(r.after.data(),r.size,1,f)!=1) return 2;memcpy(reinterpret_cast<void *>(r.address),r.bytes.data(),r.size);
        }
        std::vector<unsigned char> expectedArena(arenaSize),expectedTrace(current.traceSize);
        if(fread(expectedArena.data(),arenaSize,1,f)!=1||(current.traceSize&&fread(expectedTrace.data(),current.traceSize,1,f)!=1)) return 2;
        // The device token is host-owned in the native upload backend.
        ug2SolidDevice870974=device;crc=apiCalls=traceSize=0;worldSeen=false;ResetFp();Fp before=State();
        CarRenderSubmitQuad(reinterpret_cast<CarRenderQuad *>(quad),reinterpret_cast<CarRenderTexture *>(arena+0x13000),reinterpret_cast<WheelMatrix *>(arena+0x1b000),current.fourth);
        Fp after=State();unsigned int actualArena=Crc(reinterpret_cast<void *>(arena),arenaSize);
        if(crc!=current.traceCrc||actualArena!=current.arenaCrc||traceSize!=expectedTrace.size()||(traceSize&&memcmp(traceBytes,expectedTrace.data(),traceSize))||memcmp(reinterpret_cast<void *>(arena),expectedArena.data(),arenaSize)||!Same(before,after,!current.composed)) { fprintf(stderr,"native quad submission mismatch row %u composed %u trace %08x/%08x arena %08x/%08x FP %d\n",i,current.composed,crc,current.traceCrc,actualArena,current.arenaCrc,Same(before,after,!current.composed));return 1; }
        for(unsigned int j=0;j<current.regions;++j) { const Region &r=regions[j];if(Crc(reinterpret_cast<void *>(r.address),r.size)!=r.crc||memcmp(reinterpret_cast<void *>(r.address),r.after.data(),r.size)) { fprintf(stderr,"native quad submission mismatch row %u global %08x\n",i,r.address);return 1; } }
        unsigned int counter=ug2RenderQuadCalls872F08;before=State();CarRenderFinishQuad();after=State();
        if(ug2RenderQuadCalls872F08!=counter+1||!Same(before,after,true)) { fprintf(stderr,"native quad submission mismatch row %u standalone counter/FP check\n",i);return 3; }
        composed+=current.composed!=0;totalApis+=apiCalls;++rows;
    }
    asm volatile("fninit");if(fgetc(f)!=EOF) return 2;fclose(f);printf("native quad submission: %u exact original captures (%u composed), %u observed engine/API calls, %u counter/FP-state checks; zero mismatches\n",rows,composed,totalApis,rows);return 0;
}
