#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectPreparation.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
void __fastcall Parameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor constructor) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));if (!cookie) return 0;
    *cookie=count;T *data=reinterpret_cast<T *>(cookie+1);memset(data,0xa5,count*sizeof(T));D3DXEffectConstructArray(data,sizeof(T),count,constructor);return data;
}
unsigned int Normalize(D3DXEffectUseLink *p,D3DXEffectUseLink *base) { return p ? static_cast<unsigned int>(p-base)+1 : 0; }
bool Probe(const unsigned int config[13],std::vector<unsigned int> &tree,const std::vector<unsigned char> &payload,std::vector<unsigned int> &words) {
    unsigned int operation=config[1];
    if (!operation) { unsigned int *cursor=tree.data();words.push_back(D3DXEffectVectorOnly(&cursor));words.push_back(Address(cursor)-Address(tree.data()));return !NativeFxAllocationCount(); }
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();effect.value98=3;effect.valueA0=0;effect.valueA8=effect.valueAC=3;effect.value118=0x200;
    unsigned char *blob=static_cast<unsigned char *>(UG2FxMalloc(0x1000));if (!blob) return false;memset(blob,0,0x1000);effect.value20=Address(blob);effect.value1C=0x1000;
    D3DXEffectParameter *parameters=Array<D3DXEffectParameter>(3,Parameter);D3DXEffectGroup64 *groups=Array<D3DXEffectGroup64>(2,D3DXConstructGroup64);
    effect.valueB0=Address(UG2FxMalloc(12));if (!parameters||!groups||!effect.valueB0) return false;effect.value9C=Address(parameters);effect.value2C=Address(groups);effect.value34=2;
    for (unsigned int i=0;i<3;++i) {
        D3DXEffectParameter *p=parameters+i;unsigned int offset=0x200+i*0x40;reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[i]=p;
        p->value14=i;p->value18=p->value1C=0;p->value28=config[4];p->value2C=offset;p->value40=0;p->value04=Address(blob)+offset;
        unsigned int descriptor[7]={1,0,0x80+i*16,0x20,1,2,3};memcpy(blob+offset,descriptor,sizeof(descriptor));
        *reinterpret_cast<unsigned int *>(blob+0x80+i*16)=3;char name[3]={'p',static_cast<char>('0'+i),0};memcpy(blob+0x84+i*16,name,sizeof(name));
    }
    for (unsigned int i=0;i<2;++i) {
        groups[i].array04=Array<D3DXEffectRecord64>(2,D3DXConstructRecord64);if (!groups[i].array04) return false;groups[i].value36=groups[i].value38=2;groups[i].value0C=config[2];
    }
    D3DXEffectRecord64 *record=groups[0].array04;record->value08.value18=0x200;record->value58=0;record->value30=0x1234567b;record->value50=config[3];
    if (operation==1) {
        D3DXEffectBuffer *buffer=0;if (D3DXEffectBufferCreate(0x68+payload.size(),&buffer)<0) return false;record->value38=Address(buffer);
        memset(buffer->data0C,0xa5,0x68);memcpy(buffer->data0C+0x68,payload.data(),payload.size());unsigned int *data=reinterpret_cast<unsigned int *>(buffer->data0C);
        words.push_back(effect.BindPreparationConstants(0,0,data));words.push_back(groups[0].value0C);words.push_back(record->value00);words.push_back(record->value30);
        words.push_back(record->value4E);words.push_back(record->value50);words.push_back(record->value3C!=0);words.insert(words.end(),data,data+26);
        words.push_back(record->array34 ? reinterpret_cast<unsigned int *>(record->array34)[-1] : 0);
        for (unsigned int i=0;i<3;++i) {
            D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(parameters[i].value34),*at=head;std::vector<unsigned int> links;
            if (head) do { unsigned int *entry=reinterpret_cast<unsigned int *>(at->value10);links.push_back(at->value00);links.push_back(at->value04);links.push_back(at->value14);links.push_back(at->value16);links.push_back(entry[1]);links.push_back(entry[2]);at=at->next1C; } while (at!=head);
            words.push_back(links.size()/6);words.insert(words.end(),links.begin(),links.end());
        }
    } else {
        unsigned int count=config[5];D3DXEffectUseLink *uses=Array<D3DXEffectUseLink>(count,D3DXConstructUse);if (!uses) return false;
        record->array34=reinterpret_cast<D3DXEffectRecord20 *>(uses);record->value4E=record->value50=count;parameters[0].value14=config[8] ? 1 : 0;
        D3DXEffectParameter *owner=parameters+(config[8] ? 1 : 0);
        for (unsigned int i=0;i<count;++i) effect.LinkParameter(0,0,i,parameters,0x12345678+i);
        owner->value34=Address(uses+config[7]%count);effect.value118=config[10] ? 0x800 : 0;effect.valueA0=2;
        if (config[9]) for (unsigned int which=0;which<2;++which) {
            unsigned int *grid=static_cast<unsigned int *>(UG2FxMalloc(24));if (!grid) return false;
            if (which) effect.valueC0=Address(grid);else effect.valueB4=Address(grid);
            for (unsigned int i=0;i<6;++i) { unsigned int *slot=static_cast<unsigned int *>(UG2FxMalloc(8));if (!slot) return false;slot[0]=0x11223344;slot[1]=0;grid[i]=Address(slot); }
        }
        words.push_back(effect.UnlinkParameterUse(0,0,config[6]%count));words.push_back(Normalize(reinterpret_cast<D3DXEffectUseLink *>(owner->value34),uses));
        words.push_back(effect.valueB4!=0);words.push_back(effect.valueC0!=0);
        for (unsigned int i=0;i<count;++i) { words.push_back(Normalize(uses[i].previous18,uses));words.push_back(Normalize(uses[i].next1C,uses));words.push_back(uses[i].value00);words.push_back(uses[i].value04); }
    }
    D3DXDestroyEffect(&effect);return !NativeFxAllocationCount();
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectpreparation_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXQF1",8)||fread(&count,4,1,fixtures)!=1||count!=32||fwrite("UG2FXQD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[13];if (fread(config,4,13,fixtures)!=13||config[0]!=i||config[1]>2||config[5]>3||config[11]>256||config[12]>0x10000) return 3;
        std::vector<unsigned int> tree(config[11]),words;std::vector<unsigned char> payload(config[12]);
        if (fread(tree.data(),4,tree.size(),fixtures)!=tree.size()||fread(payload.data(),1,payload.size(),fixtures)!=payload.size()||!Probe(config,tree,payload,words)) return 4;
        unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;
        printf("effect preparation %u: %u fields, 0 live allocations\n",i,row[1]);
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    puts("effect preparation: 32 CPU lifecycles, 0 live allocations");return 0;
}
