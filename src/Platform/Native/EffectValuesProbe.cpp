#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectValues.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned int Digest(const void *p,unsigned int bytes) {
    unsigned int result=2166136261;const unsigned char *data=static_cast<const unsigned char *>(p);
    for (unsigned int i=0;i<bytes;++i) result=(result^data[i])*16777619;return result;
}
void __fastcall Parameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor constructor) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));if (!cookie) return 0;
    *cookie=count;T *p=reinterpret_cast<T *>(cookie+1);memset(p,0xa5,count*sizeof(T));D3DXEffectConstructArray(p,sizeof(T),count,constructor);return p;
}
void Reference(unsigned int kind,unsigned int pointer,std::vector<unsigned int> &words) {
    D3DXEffectBuffer *buffer=reinterpret_cast<D3DXEffectBuffer *>(pointer);
    words.push_back(kind);words.push_back(buffer!=0);words.push_back(buffer ? buffer->refs04 : 0);words.push_back(buffer ? buffer->size08 : 0);
    words.push_back(buffer ? Digest(buffer->data0C,buffer->size08) : 0);
}
bool Probe(const unsigned int config[10],const std::vector<unsigned int> &description,std::vector<unsigned int> input,std::vector<unsigned int> &words) {
    const unsigned int mode=config[0],op=config[1];D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
    effect.value98=1;effect.valueA0=0;effect.valueA8=effect.valueAC=1;effect.valueFC=Address(&effect.valueE0);effect.value118=0;effect.value3C=config[7];
    unsigned char *blob=static_cast<unsigned char *>(UG2FxMalloc(0x4000));if (!blob) return false;memset(blob,0,0x4000);effect.value20=Address(blob);effect.value1C=0x4000;
    *reinterpret_cast<unsigned int *>(blob+0x50)=3;memcpy(blob+0x54,"p0",3);memcpy(blob+0x500,description.data(),description.size()*4);
    D3DXEffectParameter *parameter=Array<D3DXEffectParameter>(1,Parameter);if (!parameter) return false;effect.value9C=Address(parameter);
    D3DXEffectParameter **table=static_cast<D3DXEffectParameter **>(UG2FxMalloc(4));if (!table) return false;*table=parameter;effect.valueB0=Address(table);
    parameter->value00=Address(blob+0x2000);parameter->value04=Address(blob+0x500);parameter->value08=0xffffffff;parameter->value0C=input.size()*4;parameter->value10=3;parameter->value14=parameter->value18=parameter->value1C=0;
    parameter->value28=0;parameter->value2C=0x500;parameter->value30=0x2000;parameter->value38=parameter->value3C=parameter->value40=0;
    D3DXEffectReference18 *references=Array<D3DXEffectReference18>(2,D3DXConstructReference18);if (!references) return false;effect.value14=2;effect.value18=Address(references);
    D3DXEffectBuffer *old=0;if (D3DXEffectBufferCreate(16,&old)<0) return false;memset(old->data0C,'x',15);old->data0C[15]=0;
    references[0].value00=4;references[0].value04=Address(old);references[1].value00=4;
    const char *text=mode==6 ? "longer than the existing buffer" : "hi";
    for (unsigned int i=0;i<input.size();++i) if (input[i]==0xfffffff1) input[i]=Address(old);else if (input[i]==0xfffffff2) input[i]=Address(text);
    unsigned int *output=reinterpret_cast<unsigned int *>(blob+0x2000);output[0]=0;output[1]=1;
    unsigned int *type=reinterpret_cast<unsigned int *>(blob+0x500),*destination=output;const unsigned int *source=input.data();
    unsigned int scalar=0x2400,matrix=0x2800,structure=0x3000,count=0;
    D3DXEffectReference08 retained[4];for (unsigned int i=0;i<4;++i) D3DXConstructReference08(retained+i);
    unsigned int size=input.size()*4;if (config[9]==1) --size;else if (config[9]==2) size=0xffffffff;else if (config[9]==3) size=0;
    unsigned int handle=config[8] ? Address("p0") : ~Address(parameter),result;
    if (op==0) { result=D3DXEffectSetValue(&effect,handle,input.data(),size);if (mode==23) result=D3DXEffectSetValue(&effect,handle,input.data(),size); }
    else if (op==1) result=effect.RecordValue(~Address(parameter),input.data(),size);
    else if (op==2) { memcpy(blob+0x2000,input.data(),input.size()*4);result=effect.RelocateParameter(parameter,&scalar,&matrix,&structure); }
    else if (op==3) result=effect.CopyTypedValue(&type,&destination,&source,2,reinterpret_cast<D3DXEffectBaseRecord *>(parameter));
    else result=D3DXEffectValueReferences(type,const_cast<unsigned int *>(source),retained,&count,0);
    words.push_back(result);words.push_back(effect.value118);words.push_back(parameter->value00-Address(blob));words.push_back(parameter->value04-Address(blob));words.push_back(parameter->value30);words.push_back(Digest(blob,0x4000));
    words.push_back(Address(type)-Address(blob));words.push_back(Address(destination)-Address(blob));words.push_back(Address(source)-Address(input.data()));words.push_back(scalar);words.push_back(matrix);words.push_back(structure);
    for (unsigned int i=0;i<2;++i) { Reference(references[i].value00,references[i].value04,words);words.push_back(references[i].value14); }
    D3DXEffectValuePrefix *value=reinterpret_cast<D3DXEffectValuePrefix *>(effect.valueE0.value18);words.push_back(value!=0);
    while (value) {
        words.push_back(value->value00);words.push_back(value->value04==~Address(parameter));words.push_back(value->value08);words.push_back(value->value10);
        std::vector<unsigned int> normalized(value->value08/4);if (value->value0C) memcpy(normalized.data(),reinterpret_cast<void *>(value->value0C),normalized.size()*4);
        for (unsigned int i=0;i<normalized.size();++i) if (normalized[i]==Address(old)) normalized[i]=0xfffffff1;else if (normalized[i]==Address(text)) normalized[i]=0xfffffff2;
        words.push_back(Digest(normalized.data(),normalized.size()*4));D3DXEffectReference08 *at=reinterpret_cast<D3DXEffectReference08 *>(value->value14);
        for (unsigned int i=0;i<value->value10;++i) Reference(at[i].value00,at[i].value04,words);
        value=reinterpret_cast<D3DXEffectValuePrefix *>(value->value18);words.push_back(value!=0);
    }
    words.push_back(count);for (unsigned int i=0;i<count;++i) Reference(retained[i].value00,retained[i].value04,words);
    for (unsigned int i=0;i<count;++i) D3DXDestroyReference08(retained+i);
    D3DXDestroyEffect(&effect);return NativeFxAllocationCount()==0;
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectvalues_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;
    char magic[8];unsigned int count;if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXVF1",8)||fread(&count,4,1,fixtures)!=1||count!=24||fwrite("UG2FXVD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[12];if (fread(config,4,12,fixtures)!=12||config[0]!=i||config[10]>256||config[11]>256) return 3;
        std::vector<unsigned int> description(config[10]),input(config[11]),words;if (fread(description.data(),4,description.size(),fixtures)!=description.size()||fread(input.data(),4,input.size(),fixtures)!=input.size()) return 3;
        if (!Probe(config,description,input,words)) return 4;
        unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;
        printf("effect values %u: status %08x, %u fields, 0 live allocations\n",i,words[0],row[1]);
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    puts("effect values: 24 native typed graphs, 0 live allocations");return 0;
}
