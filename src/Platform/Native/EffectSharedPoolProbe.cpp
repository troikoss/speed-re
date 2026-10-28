#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectSharedPool.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned int Digest(const void *pointer,unsigned int count) {
    unsigned int value=2166136261;const unsigned char *bytes=static_cast<const unsigned char *>(pointer);
    for (unsigned int i=0;i<count;++i) value=(value^bytes[i])*16777619;return value;
}
void __fastcall Parameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor constructor) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));if (!cookie) return 0;
    *cookie=count;T *data=reinterpret_cast<T *>(cookie+1);memset(data,0xa5,count*sizeof(T));D3DXEffectConstructArray(data,sizeof(T),count,constructor);return data;
}
D3DXEffectBuffer *Buffer(unsigned int bytes,unsigned char fill) {
    D3DXEffectBuffer *buffer=0;if (D3DXEffectBufferCreate(bytes,&buffer)<0) return 0;
    memset(buffer->data0C,fill,bytes-1);buffer->data0C[bytes-1]=0;return buffer;
}
struct Graph { D3DXEffectObject effect;D3DXEffectParameter *parameter;D3DXEffectReference18 *references;unsigned char *blob,*dirty; };
bool Probe(const unsigned int config[6],const std::vector<unsigned int> &description,std::vector<unsigned int> &words) {
    unsigned int mode=config[0],count=mode==17 ? 3 : 2;
    D3DXEffectPool *pool=static_cast<D3DXEffectPool *>(UG2FxMalloc(0x20));if (!pool) return false;pool->Construct();Graph graphs[3];
    for (unsigned int g=0;g<count;++g) {
        Graph &graph=graphs[g];D3DXEffectObject &effect=graph.effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
        effect.value98=1;effect.valueA0=1;effect.valueA8=effect.valueAC=1;effect.valueFC=Address(&effect.valueE0);effect.value118=mode==19 ? 0x100 : 0;
        graph.blob=static_cast<unsigned char *>(UG2FxMalloc(0x4000));if (!graph.blob) return false;memset(graph.blob,0,0x4000);effect.value20=Address(graph.blob);effect.value1C=0x4000;
        const char *name=g&&mode==13 ? "other" : g&&mode==15 ? "P1" : "p1",*semantic=g&&mode==14 ? "OTHER" : g ? "world" : "WORLD";
        *reinterpret_cast<unsigned int *>(graph.blob+0x50)=strlen(name)+1;memcpy(graph.blob+0x54,name,strlen(name)+1);
        *reinterpret_cast<unsigned int *>(graph.blob+0x80)=2;memcpy(graph.blob+0x84,"m",2);
        *reinterpret_cast<unsigned int *>(graph.blob+0x180)=strlen(semantic)+1;memcpy(graph.blob+0x184,semantic,strlen(semantic)+1);
        memcpy(graph.blob+0x500,description.data(),description.size()*4);for (unsigned int i=0;i<512;++i) reinterpret_cast<unsigned int *>(graph.blob+0x2000)[i]=(i+g)%4;
        graph.parameter=Array<D3DXEffectParameter>(1,Parameter);if (!graph.parameter) return false;effect.value9C=Address(graph.parameter);
        D3DXEffectParameter **table=static_cast<D3DXEffectParameter **>(UG2FxMalloc(4));if (!table) return false;*table=graph.parameter;effect.valueB0=Address(table);
        D3DXEffectParameter *p=graph.parameter;p->value00=Address(graph.blob+0x2000);p->value04=Address(graph.blob+0x500);p->value08=0xffffffff;p->value0C=mode==20 ? 8 : 64;p->value10=3;p->value14=p->value18=p->value1C=0;
        p->value28=1;p->value2C=0x500;p->value30=0x2000;p->value38=p->value3C=p->value40=0;
        graph.references=Array<D3DXEffectReference18>(4,D3DXConstructReference18);if (!graph.references) return false;effect.value14=4;effect.value18=Address(graph.references);
        for (unsigned int i=0;i<4;++i) {
            D3DXEffectBuffer *buffer=Buffer(16,65+g*4+i),*code=Buffer(8,97+g*4+i);if (!buffer||!code) return false;
            D3DXEffectReference18 &ref=graph.references[i];ref.value00=4;ref.value04=Address(buffer);ref.value08=Address(code);ref.value0C=0x100+g*4+i;ref.value14=0xa5a5a5a0+(g&1)*2;
        }
        unsigned int *grid=static_cast<unsigned int *>(UG2FxMalloc(4));graph.dirty=static_cast<unsigned char *>(UG2FxMalloc(1));if (!grid||!graph.dirty) return false;
        *grid=0;*graph.dirty=0;effect.valueB4=effect.valueB8=Address(grid);effect.valueBC=Address(graph.dirty);
        effect.value38=Address(pool);UG2FxComAddRef(Address(pool));
        if (mode==10||mode==11) { D3DXEffectBuffer *device=Buffer(4,'x');if (!device) return false;effect.value0C=Address(device); }
        if (g&&mode==13) pool->value14=pool->value18-4;
        unsigned int result=pool->Bind(&effect,p);words.push_back(result);
        if (!g&&mode==12) { reinterpret_cast<D3DXEffectPoolLink *>(p->value40)->Delete(1);p->value40=0; }
        if (g==1&&mode==17) {
            D3DXEffectPoolLink *link=reinterpret_cast<D3DXEffectPoolLink *>(p->value40);unsigned int index=static_cast<unsigned int *>(link->data08)[0];
            D3DXEffectBufferRelease(reinterpret_cast<D3DXEffectBuffer *>(graph.references[index].value04));D3DXEffectBuffer *replacement=Buffer(16,'Z');if (!replacement) return false;graph.references[index].value04=Address(replacement);
        }
    }
    if (mode==18||mode==20) {
        D3DXEffectBuffer *incoming=Buffer(16,'Q');if (!incoming) return false;Graph &graph=graphs[0];unsigned int result;
        if (mode==18) result=graph.effect.WriteShaderResource(Address(incoming),0,reinterpret_cast<D3DXEffectPoolLink *>(graph.parameter->value40));
        else { unsigned int input[2]={Address(incoming),Address(incoming)};result=D3DXEffectSetValue(&graph.effect,~Address(graph.parameter),input,8); }
        words.push_back(result);D3DXEffectBufferRelease(incoming);
    }
    if (mode==19) { pool->value08=1;pool->value0C=Address(&graphs[1].effect);words.push_back(graphs[0].effect.NotifyValue(reinterpret_cast<D3DXEffectBaseRecord *>(graphs[0].parameter))); }
    words.push_back(pool->value14);words.push_back(pool->value18);words.push_back(Digest(pool->data10,pool->value14));
    unsigned int nodes=0;for (D3DXEffectPoolNode *node=pool->node1C;node;node=node->linked20) ++nodes;words.push_back(nodes);
    for (D3DXEffectPoolNode *node=pool->node1C;node;node=node->linked20) {
        words.push_back(node->value00);words.push_back(node->value04);words.push_back(node->value08);std::vector<unsigned int> members;
        for (D3DXEffectPoolLink *link=node->link0C.next10;link!=&node->link0C;link=link->next10) {
            unsigned int index=0;while (index<count&&Address(&graphs[index].effect)!=link->value00) ++index;if (index==count) return false;members.push_back(index);
            words.push_back(link->data08!=0);if (link->data08) for (unsigned int i=0;i<node->value08;++i) words.push_back(static_cast<unsigned int *>(link->data08)[i]);
        }
        words.push_back(members.size());words.insert(words.end(),members.begin(),members.end());
    }
    for (unsigned int g=0;g<count;++g) {
        Graph &graph=graphs[g];D3DXEffectParameter *p=graph.parameter;words.push_back(graph.effect.value118);words.push_back(*graph.dirty);words.push_back(p->value40!=0);words.push_back(p->value2C);words.push_back(p->value30);
        words.push_back(p->value00==Address(graph.blob+0x2000) ? 0x80002000 : p->value00-Address(pool->data10));words.push_back(p->value04==Address(pool->data10)+p->value2C);
        for (unsigned int i=0;i<4;++i) {
            D3DXEffectReference18 &ref=graph.references[i];words.push_back(ref.value00);words.push_back(ref.value0C);words.push_back(ref.value14);
            const unsigned int pointers[2]={ref.value04,ref.value08};for (unsigned int slot=0;slot<2;++slot) {
                D3DXEffectBuffer *buffer=reinterpret_cast<D3DXEffectBuffer *>(pointers[slot]);words.push_back(buffer!=0);words.push_back(buffer ? buffer->refs04 : 0);words.push_back(buffer ? buffer->size08 : 0);words.push_back(buffer ? Digest(buffer->data0C,buffer->size08) : 0);
            }
        }
    }
    for (unsigned int g=count;g;--g) D3DXDestroyEffect(&graphs[g-1].effect);
    words.push_back(pool->refs04);D3DXEffectPoolRelease(pool);return NativeFxAllocationCount()==0;
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectsharedpool_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;
    char magic[8];unsigned int count;if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXPF1",8)||fread(&count,4,1,fixtures)!=1||count!=21||fwrite("UG2FXPD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[7];if (fread(config,4,7,fixtures)!=7||config[0]!=i||config[6]>256) return 3;
        std::vector<unsigned int> description(config[6]),words;if (fread(description.data(),4,description.size(),fixtures)!=description.size()||!Probe(config,description,words)) return 4;
        unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;
        printf("effect shared pool %u: %u fields, 0 live allocations\n",i,row[1]);
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    puts("effect shared pool: 21 native lifecycles, 0 live allocations");return 0;
}
