#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectCompiledPayload.h"
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
unsigned int Offset(const void *p,const void *base) { return p ? Address(p)-Address(base) : 0xffffffff; }
void Parse(const unsigned int *tokens,std::vector<unsigned int> &words) {
    unsigned int *data=0,*table=0,*instructions=0,*literals=0,bytes=0;
    unsigned short counts[4],minimum[4],maximum[4];
    words.push_back(D3DXEffectFindComment(tokens,0x42415443,&data,&bytes));words.push_back(Offset(data,tokens));words.push_back(bytes);
    words.push_back(D3DXEffectReadConstantTable(tokens,&table,counts,minimum,maximum));words.push_back(Offset(table,tokens));
    for (unsigned int i=0;i<4;++i) words.push_back(counts[i]);
    for (unsigned int i=0;i<4;++i) words.push_back(minimum[i]);
    for (unsigned int i=0;i<4;++i) words.push_back(maximum[i]);
    words.push_back(D3DXEffectReadCompiledSections(tokens,&instructions,&literals));words.push_back(Offset(instructions,tokens));words.push_back(Offset(literals,tokens));
}
bool Write(FILE *capture,unsigned int kind,unsigned int index,const std::vector<unsigned int> &words) {
    unsigned int header[3]={kind,index,static_cast<unsigned int>(words.size())};
    return fwrite(header,1,sizeof(header),capture)==sizeof(header)&&fwrite(words.data(),4,words.size(),capture)==words.size();
}
bool Graph(const unsigned int config[8],const std::vector<unsigned char> &payload,std::vector<unsigned int> &words) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
    effect.value98=3;effect.valueA0=0;effect.valueA8=effect.valueAC=3;effect.value118=0;
    effect.value20=Address(UG2FxMalloc(0x1000));effect.value1C=0x1000;
    D3DXEffectParameter *parameters=Array<D3DXEffectParameter>(3,ConstructParameter);
    D3DXEffectGroup64 *groups=Array<D3DXEffectGroup64>(2,D3DXConstructGroup64);
    effect.valueB0=Address(UG2FxMalloc(12));
    if (!effect.value20||!parameters||!groups||!effect.valueB0) return false;
    memset(reinterpret_cast<void *>(effect.value20),0,0x1000);effect.value9C=Address(parameters);effect.value2C=Address(groups);effect.value34=2;
    for (unsigned int i=0;i<3;++i) {
        D3DXEffectParameter *p=parameters+i;unsigned int offset=0x200+i*0x40;
        reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[i]=p;
        p->value14=i;p->value18=p->value1C=0;p->value28=config[3];p->value2C=offset;p->value40=0;p->value04=effect.value20+offset;
        unsigned int descriptor[7]={1,0,0x80+i*16,0x20,1,config[6],config[7]};memcpy(reinterpret_cast<void *>(p->value04),descriptor,sizeof(descriptor));
        unsigned int *name=reinterpret_cast<unsigned int *>(effect.value20+0x80+i*16);*name=3;
        char text[3]={'p',static_cast<char>('0'+i),0};memcpy(name+1,text,sizeof(text));
    }
    for (unsigned int i=0;i<2;++i) {
        groups[i].array04=Array<D3DXEffectRecord64>(2,D3DXConstructRecord64);if (!groups[i].array04) return false;
        groups[i].value36=groups[i].value38=2;groups[i].value0C=config[5];
    }
    D3DXEffectRecord64 *record=groups[0].array04;record->value08.value18=0x200;record->value58=0;record->value30=0x1234567b;record->value50=config[4];
    D3DXEffectBuffer *buffer=0;
    if (D3DXEffectBufferCreate(payload.size()+4,&buffer)<0) return false;
    *reinterpret_cast<unsigned int *>(buffer->data0C)=config[2];memcpy(buffer->data0C+4,payload.data(),payload.size());effect.value28=Address(buffer);
    unsigned int position=0;int result=effect.ReadCompiledRecord(0,0,buffer,&position);
    D3DXEffectBuffer *dataBuffer=reinterpret_cast<D3DXEffectBuffer *>(record->value38);
    unsigned int *data=dataBuffer ? reinterpret_cast<unsigned int *>(dataBuffer->data0C) : 0;
    words.push_back(result);words.push_back(position);words.push_back(groups[0].value0C);words.push_back(record->value00);
    words.push_back(record->value4E);words.push_back(record->value50);words.push_back(record->value30);
    words.push_back(record->value3C!=0);words.push_back(record->value40!=0);words.push_back(record->value44!=0);words.push_back(record->array34!=0);
    words.push_back(record->array34 ? reinterpret_cast<unsigned int *>(record->array34)[-1] : 0);
    words.push_back(dataBuffer!=0);words.push_back(dataBuffer ? dataBuffer->size08 : 0);words.push_back(data ? data[2] : 0);
    if (result==0&&config[2]) {
        words.push_back(data[0]);words.push_back(data[1]);words.push_back(data[3]);
        for (unsigned int i=4;i<7;++i) words.push_back(record->value3C ? data[i] : 0);
    } else for (unsigned int i=0;i<6;++i) words.push_back(0);
    for (unsigned int i=0;i<3;++i) {
        D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(parameters[i].value34),*at=head;
        std::vector<unsigned int> links;
        if (head) do {
            links.push_back(at->value00);links.push_back(at->value04);links.push_back(at->value14);links.push_back(at->value16);
            unsigned int *entry=reinterpret_cast<unsigned int *>(at->value10);links.push_back(entry[1]);links.push_back(entry[2]);
            at=at->next1C;if (links.size()>100) return false;
        } while (at!=head);
        words.push_back(links.size()/6);words.insert(words.end(),links.begin(),links.end());
    }
    unsigned int handles[2]={Address("p1"),~Address(parameters+1)};
    for (unsigned int i=0;i<2;++i) {
        D3DXEffectParameter *p=effect.FindRootParameter(handles[i],10);words.push_back(p ? static_cast<unsigned int>(p-parameters)+1 : 0);
    }
    D3DXDestroyEffect(&effect);return !NativeFxAllocationCount();
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectcompiledpayload_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;
    char magic[8];if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXCF2",8)||fwrite("UG2FXCP1",1,8,capture)!=8) return 3;
    unsigned int count;
    if (fread(&count,4,1,fixtures)!=1||count!=16) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[8];if (fread(config,4,8,fixtures)!=8||config[0]!=i||config[1]>0x10000||config[2]>config[1]) return 3;
        std::vector<unsigned char> payload(config[1]);if (fread(payload.data(),1,payload.size(),fixtures)!=payload.size()) return 3;
        std::vector<unsigned int> words;if (!Graph(config,payload,words)||!Write(capture,0,i,words)) return 4;
        printf("compiled record %u: result %08x cursor %u group bytes %u, 0 live allocations\n",i,words[0],words[1],words[2]);
    }
    if (fread(&count,4,1,fixtures)!=1||count>10000) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int row[3];if (fread(row,4,3,fixtures)!=3||row[0]>=23) return 3;
        const char *name=*reinterpret_cast<const char *const *>(0x7ff7d0+row[0]*0x8c+0x84);
        D3DXEffectResource resource;resource.Construct();
        if (resource.Load(0,name,0,0)<0||row[1]>resource.size08||row[2]>resource.size08-row[1]) return 4;
        std::vector<unsigned int> words;Parse(reinterpret_cast<const unsigned int *>(resource.data04+row[1]),words);
        resource.Clear();if (NativeFxAllocationCount()||!Write(capture,1,i,words)) return 4;
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    printf("compiled payload: 16 linked record graphs, %u retail parser chains, 0 live allocations\n",count);return 0;
}
