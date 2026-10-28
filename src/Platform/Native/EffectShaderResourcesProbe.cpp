#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectShaderResources.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
void __fastcall ConstructParameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor construct) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));
    if (!cookie) return 0;
    *cookie=count;T *p=reinterpret_cast<T *>(cookie+1);memset(p,0xa5,count*sizeof(T));
    D3DXEffectConstructArray(p,sizeof(T),count,construct);return p;
}
bool Probe(unsigned int mode,FILE *capture) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
    effect.value98=2;effect.valueA0=0;effect.valueA8=effect.valueAC=2;
    D3DXEffectParameter *parameters=Array<D3DXEffectParameter>(2,ConstructParameter);
    D3DXEffectGroup64 *groups=Array<D3DXEffectGroup64>(4,D3DXConstructGroup64);
    D3DXEffectReference18 *references=Array<D3DXEffectReference18>(3,D3DXConstructReference18);
    D3DXEffectRecord64 *records=Array<D3DXEffectRecord64>(1,D3DXConstructRecord64);
    if (!parameters||!groups||!references||!records) return false;
    effect.value9C=Address(parameters);effect.value2C=Address(groups);effect.value18=Address(references);effect.value14=3;effect.value34=4;
    groups[0].array04=records;groups[0].value36=groups[0].value38=1;
    effect.value20=Address(UG2FxMalloc(0x400));effect.value1C=0x400;
    effect.valueB0=Address(UG2FxMalloc(8));if (!effect.value20||!effect.valueB0) return false;
    memset(reinterpret_cast<void *>(effect.value20),0,0x400);
    reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[0]=parameters;
    reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[1]=parameters+1;
    parameters[0].value30=0x100;*reinterpret_cast<unsigned int *>(effect.value20+0x100)=1;
    D3DXEffectReference18 *reference=references+1;
    reference->value00=mode==6 ? 3 : 4;reference->value04=reference->value08=reference->value0C=reference->value14=0;reference->value10=0xa5a50000;
    const unsigned int sizes[8]={0,1,3,4,7,16,32,63};unsigned int size=mode<8 ? sizes[mode] : 0;
    std::vector<unsigned char> payload((mode<8 ? 8 : 4)+size);
    unsigned int *header=reinterpret_cast<unsigned int *>(payload.data());header[0]=mode<8 ? 1 : 0;if (mode<8) header[1]=size;
    for (unsigned int i=0;i<size;++i) payload[8+i]=static_cast<unsigned char>(i*17+3);
    if (size&&mode!=7) payload.back()=0;
    D3DXEffectBuffer *buffer=0;
    if (D3DXEffectBufferCreate(payload.size(),&buffer)<0) return false;
    memcpy(buffer->data0C,payload.data(),payload.size());effect.value28=Address(buffer);
    unsigned int position=0;int result;
    if (mode<8) result=effect.ReadResource(buffer,&position);
    else {
        records[0].value52=mode==9 ? 147 : mode==10 ? 0 : 146;
        records[0].value08.value14=records[0].value08.value1C=0;
        effect.value30=1;effect.value34=mode==11 ? 1 : 4;
        result=effect.ReadShaderRecord(0,0,buffer,&position);
    }
    std::vector<unsigned int> words;
    words.push_back(result);words.push_back(position);words.push_back(effect.value30);words.push_back(reference->value00);
    words.push_back(reference->value04!=0);words.push_back(reference->value08!=0);words.push_back(reference->value10);words.push_back(reference->value14);
    D3DXEffectBuffer *resource=reinterpret_cast<D3DXEffectBuffer *>(reference->value04);
    unsigned int length=resource ? resource->size08 : 0,digest=2166136261;
    for (unsigned int i=0;i<length;++i) digest=(digest^resource->data0C[i])*16777619;
    words.push_back(length);words.push_back(digest);words.push_back(groups[0].value0C);words.push_back(groups[0].value34);
    words.push_back(records[0].value00);words.push_back(records[0].value52);words.push_back(records[0].value54);words.push_back(records[0].value56);words.push_back(records[0].value58);words.push_back(records[0].value48);
    words.push_back(groups[1].value36);words.push_back(groups[1].value38);words.push_back(groups[1].value18);words.push_back(groups[1].value1A);
    D3DXEffectRecord64 *target=groups[1].array04;words.push_back(target!=0);
    if (target) {
        words.push_back(target->value00);words.push_back(target->value52);words.push_back(target->value54);words.push_back(target->value56);words.push_back(target->value58);words.push_back(target->value30&0x1f);
    }
    D3DXDestroyEffect(&effect);bool ok=!NativeFxAllocationCount();
    if (capture) {
        unsigned int row[2]={mode,static_cast<unsigned int>(words.size())};
        ok=ok && fwrite(row,1,sizeof(row),capture)==sizeof(row) && fwrite(words.data(),4,words.size(),capture)==words.size();
    }
    printf("effect shader resources %u: result %08x cursor %u resource bytes %u, 0 live allocations\n",mode,result,position,length);return ok;
}
}
int main(int argc,char **argv) {
    if (argc!=2&&argc!=3) { fprintf(stderr,"usage: ug2effectshaderresources_probe speed2.exe [capture.bin]\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *capture=argc==3 ? fopen(argv[2],"wb") : 0;
    if (argc==3&&!capture) return 3;
    if (capture&&fwrite("UG2FXSR1",1,8,capture)!=8) return 3;
    for (unsigned int mode=0;mode<12;++mode) if (!Probe(mode,capture)) return 4;
    if (capture&&fclose(capture)) return 3;
    printf("effect shader resources: 12 CPU-only fixtures, 0 live allocations\n");return 0;
}
