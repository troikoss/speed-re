#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectHandles.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
void __fastcall ConstructParameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor construct) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));if (!cookie) return 0;
    *cookie=count;T *p=reinterpret_cast<T *>(cookie+1);memset(p,0xa5,count*sizeof(T));D3DXEffectConstructArray(p,sizeof(T),count,construct);return p;
}
unsigned int Digest(const void *pointer,unsigned int size) {
    unsigned int result=2166136261;const unsigned char *bytes=static_cast<const unsigned char *>(pointer);
    for (unsigned int i=0;i<size;++i) result=(result^bytes[i])*16777619;return result;
}
unsigned int Base(const D3DXEffectObject &effect,const D3DXEffectBaseRecord *record) {
    return record->value14>=effect.valueAC ? reinterpret_cast<unsigned int *>(effect.value11C)[record->value14-effect.valueAC] : effect.value20;
}
void AppendBase(const D3DXEffectObject &effect,const D3DXEffectBaseRecord *record,std::vector<unsigned int> &words) {
    unsigned int base=Base(effect,record);
    words.push_back(record->value08);words.push_back(record->value0C);words.push_back(record->value10);words.push_back(record->value14);words.push_back(record->value18);words.push_back(record->value1C);
    words.push_back(record->value04-base);words.push_back(record->value00-base);
    unsigned int count=record->array20 ? reinterpret_cast<const unsigned int *>(record->array20)[-1] : 0;words.push_back(count);
    for (unsigned int i=0;i<count;++i) AppendBase(effect,record->array20+i,words);
    words.push_back(record->linked24!=0);if (record->linked24) AppendBase(effect,record->linked24,words);
}
void AppendParameter(const D3DXEffectObject &effect,const D3DXEffectParameter *p,std::vector<unsigned int> &words) {
    AppendBase(effect,reinterpret_cast<const D3DXEffectBaseRecord *>(p),words);words.push_back(p->value28);words.push_back(p->value14<effect.valueAC ? p->value38 : 0);
    unsigned int count=0;
    for (const D3DXEffectParameter *child=reinterpret_cast<const D3DXEffectParameter *>(p->value3C);child;child=reinterpret_cast<const D3DXEffectParameter *>(child->value3C)) ++count;
    words.push_back(count);
    for (const D3DXEffectParameter *child=reinterpret_cast<const D3DXEffectParameter *>(p->value3C);child;child=reinterpret_cast<const D3DXEffectParameter *>(child->value3C)) AppendParameter(effect,child,words);
    D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(p->value34),*at=head;std::vector<unsigned int> links;
    if (head) do {
        unsigned int *entry=reinterpret_cast<unsigned int *>(at->value10);
        links.push_back(at->value00);links.push_back(at->value04);links.push_back(at->value14);links.push_back(at->value16);links.push_back(entry ? entry[1] : 0);links.push_back(entry ? entry[2] : 0);at=at->next1C;
    } while (at!=head);
    words.push_back(links.size()/6);words.insert(words.end(),links.begin(),links.end());
}
bool Probe(const unsigned int config[16],const char *query,const char *name,const unsigned char *blob,const std::vector<unsigned char> &payload,std::vector<unsigned int> &words) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();effect.value98=3;effect.valueA0=2;effect.valueA8=effect.valueAC=7;effect.value118=config[6];
    effect.value20=Address(UG2FxMalloc(0x4000));effect.value1C=0x4000;if (!effect.value20) return false;memcpy(reinterpret_cast<void *>(effect.value20),blob,0x4000);
    D3DXEffectParameter *parameters=Array<D3DXEffectParameter>(3,ConstructParameter);effect.value9C=Address(parameters);effect.valueB0=Address(UG2FxMalloc(28));
    if (!parameters||!effect.valueB0) return false;
    D3DXEffectParameter *metadata[7]={parameters,parameters+1,parameters+2,0,0,0,0};
    for (unsigned int i=3;i<7;++i) { metadata[i]=static_cast<D3DXEffectParameter *>(UG2FxMalloc(0x50));if (!metadata[i]) return false;memset(metadata[i],0xa5,0x50);metadata[i]->Construct(); }
    const unsigned int types[7]={0x400,0x500,0x600,0x700,0x740,0x780,0x7c0},values[7]={0x1800,0x2000,0x2800,0x3000,0x3100,0x3200,0x3300};
    for (unsigned int i=0;i<7;++i) {
        D3DXEffectParameter *p=metadata[i];reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[i]=p;
        p->value00=effect.value20+values[i];p->value04=effect.value20+types[i];p->value08=0xffffffff;p->value10=3;p->value14=i;p->value18=p->value1C=0;
        p->value28=i<3 ? 1 : 4;p->value2C=types[i];p->value30=values[i];p->value38=p->value3C=p->value40=0;
        unsigned int type=types[i],data=values[i];p->value0C=D3DXEffectValueSize(effect.value20,&type,&data);
    }
    parameters[0].value38=2;parameters[0].value3C=Address(metadata[3]);metadata[3]->value3C=Address(metadata[4]);
    D3DXEffectRecordB4 *techniques=Array<D3DXEffectRecordB4>(2,D3DXConstructRecordB4);effect.valueA4=Address(techniques);if (!techniques) return false;
    for (unsigned int t=0;t<2;++t) {
        D3DXEffectRecordB4 *technique=techniques+t;technique->array38=Array<D3DXEffectRecord38>(2,D3DXConstructRecord38);if (!technique->array38) return false;
        technique->value00.value14=t|0xa5a50000;technique->value28=0x100+t*16;technique->value2C=t==0 ? 1 : 0;technique->parameter30=t==0 ? metadata[5] : 0;technique->value34=2;
        for (unsigned int i=0;i<2;++i) {
            D3DXEffectRecord38 *pass=technique->array38+i;pass->value00.value14=t|0xa5a50000;pass->value00.value18=i|0xa5a50000;pass->value2C=t==0&&i==0 ? 1 : 0;pass->parameter30=t==0&&i==0 ? metadata[6] : 0;
        }
    }
    D3DXEffectGroup64 *groups=Array<D3DXEffectGroup64>(2,D3DXConstructGroup64);effect.value2C=Address(groups);effect.value34=2;if (!groups) return false;
    D3DXEffectReference18 *references=Array<D3DXEffectReference18>(2,D3DXConstructReference18);effect.value14=2;effect.value18=Address(references);if (!references) return false;
    D3DXEffectBuffer *referenceBuffer=0;if (D3DXEffectBufferCreate(4,&referenceBuffer)<0) return false;*reinterpret_cast<unsigned int *>(referenceBuffer->data0C)=0x78563412;
    references[0].value00=4;references[0].value04=Address(referenceBuffer);
    for (unsigned int i=0;i<2;++i) { groups[i].array04=Array<D3DXEffectRecord64>(2,D3DXConstructRecord64);if (!groups[i].array04) return false;groups[i].value36=groups[i].value38=2;groups[i].value0C=7; }
    D3DXEffectRecord64 *record=groups[0].array04;record->value52=config[12];record->value58=0;record->value30=0x1234567b;record->value54=0x55aa;record->value48=9;
    unsigned int *scratch=static_cast<unsigned int *>(UG2FxMalloc(28));if (!scratch) return false;effect.valueC4=Address(scratch);
    unsigned int descriptor[7]={config[7],config[8],0xa5a5a5a5,0x5a5a5a5a,config[9],config[10],config[11]};memcpy(scratch,descriptor,sizeof(descriptor));
    unsigned int op=config[1],result=0;void *owners[4]={parameters,techniques,techniques[0].array38,parameters+1};unsigned int owner=Address(owners[config[3]]),position=0;
    if (op==0) { result=Address(effect.ResolveParameter(Address(query),10,config[2]));if (result) result=Address(effect.ResolveParameter(Address(query),10,config[2])); }
    else if (op==1) { result=effect.ResolveElement(~owner,config[4],10);effect.ResolveElement(~owner,0,10);effect.ResolveElement(~owner,1,10);result=effect.ResolveElement(~owner,config[4],10); }
    else if (op==2) result=effect.ResolveMember(~owner,name,10);
    else if (op==3) { result=effect.ResolveAnnotation(~owner,name,10);if (result) result=effect.ResolveAnnotation(~owner,name,10); }
    else if (op==4) result=Address(effect.FindTechnique(Address(query),10));
    else if (op==5) result=Address(effect.FindPass(~owner,10));
    else if (op==6||op==7) {
        unsigned int size=strlen(query)+1,header=op==6 ? 4 : 8,total=header+((size+3)&0xfffffffc)+payload.size();D3DXEffectBuffer *buffer=0;
        if (D3DXEffectBufferCreate(total,&buffer)<0) return false;effect.value28=Address(buffer);
        unsigned int *out=reinterpret_cast<unsigned int *>(buffer->data0C);out[0]=op==6 ? size : 4+size+payload.size();if (op==7) out[1]=size;
        memcpy(buffer->data0C+header,query,size);memset(buffer->data0C+header+size,0,(-size)&3);if (!payload.empty()) memcpy(buffer->data0C+header+((size+3)&0xfffffffc),payload.data(),payload.size());
        result=op==6 ? effect.ReadSymbol(0,0,buffer,&position) : effect.ReadSymbolRecord(0,0,buffer,&position);
    } else if (op==8) result=D3DXEffectAtol(query);
    else { effect.BuildMembers(reinterpret_cast<D3DXEffectBaseRecord *>(parameters),2);result=effect.BuildMembers(reinterpret_cast<D3DXEffectBaseRecord *>(parameters),config[5]); }
    D3DXEffectBaseRecord *returned=reinterpret_cast<D3DXEffectBaseRecord *>((op==1||op==2||op==3)&&result ? ~result : op==0||op==4||op==5 ? result : 0);
    words.push_back(op>=6 ? result : 0);words.push_back(returned!=0);
    if (returned) {
        unsigned int kind=returned->value10;words.push_back(kind);words.push_back(kind==3 ? returned->value14 : static_cast<unsigned short>(returned->value14));words.push_back(kind==3 ? returned->value08 : kind==1 ? static_cast<unsigned short>(returned->value18) : 0);
        words.push_back(kind==3 ? returned->value18 : 0);words.push_back(kind==3 ? returned->value1C : 0);words.push_back(kind==3 ? returned->value0C : 0);
        unsigned int base=kind==3 ? Base(effect,returned) : 0;words.push_back(kind==3 ? returned->value04-base : 0);words.push_back(kind==3 ? returned->value00-base : 0);
    } else for (unsigned int i=0;i<8;++i) words.push_back(0);
    words.push_back(effect.valueA8);words.push_back(effect.valueAC);words.push_back(effect.value120);words.push_back(effect.value14);words.push_back(effect.value118);words.push_back(groups[0].value0C);words.push_back(groups[0].value34);
    for (unsigned int i=0;i<3;++i) AppendParameter(effect,parameters+i,words);
    for (unsigned int t=0;t<2;++t) {
        words.push_back(techniques[t].value2C);for (D3DXEffectParameter *p=techniques[t].parameter30;p;p=reinterpret_cast<D3DXEffectParameter *>(p->value3C)) AppendParameter(effect,p,words);
        for (unsigned int i=0;i<2;++i) { words.push_back(techniques[t].array38[i].value2C);for (D3DXEffectParameter *p=techniques[t].array38[i].parameter30;p;p=reinterpret_cast<D3DXEffectParameter *>(p->value3C)) AppendParameter(effect,p,words); }
    }
    for (unsigned int i=0;i<effect.value120;++i) {
        unsigned int block=reinterpret_cast<unsigned int *>(effect.value11C)[i],size=0;
        for (unsigned int j=effect.valueAC;j<effect.valueA8;++j) { D3DXEffectParameter *p=reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[j];if (p->value04-p->value2C==block) { size=p->value30+p->value0C;break; } }
        words.push_back(block ? Digest(reinterpret_cast<void *>(block),size) : 0);
    }
    for (unsigned int i=0;i<effect.value14;++i) {
        D3DXEffectReference18 *p=reinterpret_cast<D3DXEffectReference18 *>(effect.value18)+i;D3DXEffectBuffer *buffer=reinterpret_cast<D3DXEffectBuffer *>(p->value04);
        words.push_back(p->value00);words.push_back(buffer!=0);words.push_back(p->value08!=0);words.push_back(buffer ? buffer->refs04 : 0);words.push_back(buffer ? buffer->size08 : 0);words.push_back(buffer ? Digest(buffer->data0C,buffer->size08) : 0);
    }
    words.push_back(record->value00);words.push_back(record->value30);words.push_back(record->value52);words.push_back(record->value54);words.push_back(record->value56);words.push_back(record->value48);words.push_back(record->value58);words.push_back(record->value4E);words.push_back(record->value50);
    if (op==6||op==7) AppendBase(effect,&record->value08,words);
    words.push_back(record->array34!=0);words.push_back(record->value3C!=0);words.push_back(record->value40!=0);words.push_back(record->value44!=0);
    D3DXEffectBuffer *buffer=reinterpret_cast<D3DXEffectBuffer *>(record->value38);words.push_back(buffer!=0);
    if (buffer) {
        unsigned int *data=reinterpret_cast<unsigned int *>(buffer->data0C);words.push_back(buffer->size08);for (unsigned int i=0;i<4;++i) words.push_back(data[i]);
        for (unsigned int i=4;i<7;++i) words.push_back(result==0&&!payload.empty()&&record->value3C ? data[i] : 0);words.push_back(Digest(data+8,buffer->size08-32));
    }
    if (op==6||op==7) words.push_back(position);
    D3DXDestroyEffect(&effect);return !NativeFxAllocationCount();
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effecthandles_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;
    char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXHF1",8)||fread(&count,4,1,fixtures)!=1||count!=32||fwrite("UG2FXHD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[16];if (fread(config,4,16,fixtures)!=16||config[0]!=i||config[1]>9||config[3]>3||config[13]>1024||config[14]>1024||config[15]>0x10000) return 3;
        std::vector<char> query(config[13]),name(config[14]);std::vector<unsigned char> blob(0x4000),payload(config[15]);
        if (query.empty()||name.empty()||fread(query.data(),1,query.size(),fixtures)!=query.size()||fread(name.data(),1,name.size(),fixtures)!=name.size()||fread(blob.data(),1,blob.size(),fixtures)!=blob.size()||fread(payload.data(),1,payload.size(),fixtures)!=payload.size()||query.back()||name.back()) return 3;
        std::vector<unsigned int> words;if (!Probe(config,query.data(),name.data(),blob.data(),payload,words)) return 4;
        unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;
        printf("effect handles %u: operation %u status %08x fields %u, 0 live allocations\n",i,config[1],words[0],row[1]);
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    printf("effect handles: 32 native typed graphs, 0 live allocations\n");return 0;
}
