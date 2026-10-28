#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectMetadata.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
void __fastcall ConstructParameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor construct) {
    if (!count) return 0;
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));
    if (!cookie) return 0;
    *cookie=count;T *p=reinterpret_cast<T *>(cookie+1);memset(p,0xa5,count*sizeof(T));
    D3DXEffectConstructArray(p,sizeof(T),count,construct);return p;
}
unsigned short Half(const void *p,unsigned int offset) { return *reinterpret_cast<const unsigned short *>(static_cast<const unsigned char *>(p)+offset); }
void AppendAnnotations(std::vector<unsigned int> &words,const D3DXEffectParameter *p) {
    unsigned int count=0;
    for (;p;p=reinterpret_cast<const D3DXEffectParameter *>(p->value3C)) {
        words.push_back(p->value2C);words.push_back(p->value30);words.push_back(p->value28);words.push_back(p->value38);++count;
    }
    words.push_back(count);
}
void Summary(D3DXEffectObject *effect,int result,unsigned int position,unsigned int output[9]) {
    // Only parsed/constructor-defined fields are normalized. Unspecified heap
    // bytes and native addresses are deliberately outside this capture format.
    std::vector<unsigned int> words;
    words.push_back(result);words.push_back(position);words.push_back(effect->valueA8);
    words.push_back(effect->value124);words.push_back(effect->value128);words.push_back(effect->value12C);words.push_back(effect->value30);
    D3DXEffectParameter *parameters=reinterpret_cast<D3DXEffectParameter *>(effect->value9C);
    for (unsigned int i=0;i<effect->value98;++i) {
        D3DXEffectParameter *p=parameters+i;
        words.push_back(p->value2C);words.push_back(p->value30);words.push_back(p->value28);words.push_back(p->value38);
        AppendAnnotations(words,reinterpret_cast<const D3DXEffectParameter *>(p->value3C));
    }
    D3DXEffectRecordB4 *techniques=reinterpret_cast<D3DXEffectRecordB4 *>(effect->valueA4);
    for (unsigned int i=0;i<effect->valueA0;++i) {
        D3DXEffectRecordB4 *p=techniques+i;
        words.push_back(p->value28);words.push_back(p->value2C);words.push_back(p->value34);words.push_back(Half(p,0x14));words.push_back(Half(p,0x3c));
        for (unsigned int o=0xa4;o<0xb4;o+=2) words.push_back(Half(p,o));
        AppendAnnotations(words,p->parameter30);
        for (unsigned int j=0;j<p->value34;++j) {
            D3DXEffectRecord38 *q=p->array38+j;
            words.push_back(q->value28);words.push_back(q->value2C);words.push_back(Half(q,0x14));words.push_back(Half(q,0x18));words.push_back(Half(q,0x34));
            AppendAnnotations(words,q->parameter30);
        }
    }
    D3DXEffectGroup64 *groups=reinterpret_cast<D3DXEffectGroup64 *>(effect->value2C);
    for (unsigned int i=0;i<effect->value34;++i) {
        D3DXEffectGroup64 *p=groups+i;
        words.push_back(p->value0C);words.push_back(p->value18);words.push_back(p->value1A);words.push_back(p->value34);
        words.push_back(p->value36);words.push_back(p->value38);words.push_back(p->value3E);
        for (unsigned int j=0;j<p->value36;++j) {
            D3DXEffectRecord64 *q=p->array04+j;
            words.push_back(q->value00);words.push_back(q->value30&3);words.push_back(q->value52);words.push_back(q->value54);words.push_back(q->value56);
            words.push_back(q->value58);words.push_back(q->value48);words.push_back(q->value08.value18);words.push_back(q->value08.value1C);
            if ((ug2FxTypeTable7B582C[q->value52][4]>>24)==8) words.push_back(q->value4C);
        }
    }
    D3DXEffectReference18 *references=reinterpret_cast<D3DXEffectReference18 *>(effect->value18);
    for (unsigned int i=0;i<effect->value14;++i) { words.push_back(references[i].value00);words.push_back(references[i].value14&3); }
    unsigned int hash=2166136261;
    for (unsigned int i=0;i<words.size();++i) {
        unsigned int value=words[i];for (unsigned int j=0;j<4;++j) { hash=(hash^(value&255))*16777619;value>>=8; }
    }
    for (unsigned int i=0;i<7;++i) output[i]=words[i];
    output[7]=static_cast<unsigned int>(words.size());output[8]=hash;
}
bool Probe(unsigned int recipe,const char *name,FILE *capture) {
    D3DXEffectResource resource;resource.Construct();
    if (resource.Load(0,name,0,0)!=0 || resource.size08<8) return false;
    const unsigned int *raw=reinterpret_cast<const unsigned int *>(resource.data04);
    unsigned int size=raw[1],position=2+(size+3)/4;
    if (raw[0]!=0xfeff0901 || size>resource.size08-8 || position+4>resource.size08/4) return false;
    const unsigned int *counts=raw+position;position+=4;
    D3DXEffectObject *effect=static_cast<D3DXEffectObject *>(UG2FxMalloc(sizeof(*effect)));
    if (!effect) return false;
    memset(effect,0xa5,sizeof(*effect));effect->Construct();
    D3DXEffectBuffer *buffer=0;
    if (D3DXEffectBufferCreate(resource.size08,&buffer)!=0) { D3DXEffectRelease(effect);return false; }
    memcpy(D3DXEffectBufferPointer(buffer),resource.data04,resource.size08);effect->value28=reinterpret_cast<unsigned int>(buffer);
    effect->value20=reinterpret_cast<unsigned int>(UG2FxMalloc(size));effect->value1C=size;
    if (!effect->value20) { D3DXEffectRelease(effect);return false; }
    memcpy(reinterpret_cast<void *>(effect->value20),resource.data04+8,size);
    effect->value98=counts[0];effect->valueA0=counts[1];effect->value34=counts[2];effect->value14=counts[3];effect->valueA8=1;
    effect->value2C=reinterpret_cast<unsigned int>(Array<D3DXEffectGroup64>(counts[2],D3DXConstructGroup64));
    effect->value9C=reinterpret_cast<unsigned int>(Array<D3DXEffectParameter>(counts[0],ConstructParameter));
    effect->valueA4=reinterpret_cast<unsigned int>(Array<D3DXEffectRecordB4>(counts[1],D3DXConstructRecordB4));
    effect->value18=reinterpret_cast<unsigned int>(Array<D3DXEffectReference18>(counts[3],D3DXConstructReference18));
    if ((counts[0]&&!effect->value9C)||(counts[1]&&!effect->valueA4)||(counts[2]&&!effect->value2C)||(counts[3]&&!effect->value18)) { D3DXEffectRelease(effect);return false; }
    // This probe supplies the already-known outer header and allocations. The
    // main deserializer, relocation and compiled shader payload readers remain open.
    int result=0;
    for (unsigned int i=0;i<counts[0];++i) {
        result=effect->ReadParameter(reinterpret_cast<D3DXEffectParameter *>(effect->value9C)+i,buffer,&position);
        if (result<0) break;
    }
    if (result>=0) for (unsigned int i=0;i<counts[1];++i) {
        D3DXEffectRecordB4 *p=reinterpret_cast<D3DXEffectRecordB4 *>(effect->valueA4)+i;
        *reinterpret_cast<unsigned short *>(&p->value00.value14)=static_cast<unsigned short>(i);
        result=effect->ReadTechnique(p,buffer,&position);if (result<0) break;
    }
    unsigned int summary[9]={0};
    if (result>=0) Summary(effect,result,position,summary);
    bool ok=result>=0;
    if (ok && capture) { unsigned int record[10];record[0]=recipe;memcpy(record+1,summary,sizeof(summary));ok=fwrite(record,1,sizeof(record),capture)==sizeof(record); }
    printf("effect readers %u %s: result %08x cursor %u parameters %u techniques %u groups %u words %u hash %08x\n",recipe,name,result,position,counts[0],counts[1],effect->value30,summary[7],summary[8]);
    D3DXEffectRelease(effect);resource.Clear();
    return ok && !NativeFxAllocationCount();
}
}
int main(int argc,char **argv) {
    if (argc!=2 && argc!=3) { fprintf(stderr,"usage: ug2effectreaders_probe speed2.exe [capture.bin]\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *capture=argc==3 ? fopen(argv[2],"wb") : 0;
    if (argc==3&&!capture) return 3;
    if (capture&&fwrite("UG2FXRD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<23;++i) {
        const char *name=*reinterpret_cast<const char *const *>(0x7ff7d0+i*0x8c+0x84);
        if (!Probe(i,name,capture)) return 4;
    }
    if (capture&&fclose(capture)) return 3;
    printf("effect readers: 23 retail metadata chains, 0 live allocations\n");return 0;
}
