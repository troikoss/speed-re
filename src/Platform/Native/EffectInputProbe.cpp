#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectFactory.h"
#include "../../Speed/Indep/Libs/D3DX/EffectDestruction.h"
#include "../../Speed/Indep/Libs/D3DX/EffectSetup.h"
#include "../../Speed/Indep/Libs/D3DX/EffectMetadata.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdint.h>

namespace {
unsigned int Checksum(const unsigned char *data,unsigned int size) {
    unsigned int hash=2166136261u;
    for (unsigned int i=0;i<size;++i) hash=(hash^data[i])*16777619u;
    return hash;
}
template<class T> T *CleanupArray(unsigned int count) {
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(4+count*sizeof(T)));
    *cookie=count;
    T *elements=reinterpret_cast<T *>(cookie+1);
    memset(elements,0,count*sizeof(T));
    return elements;
}
bool ProbeEffectDestruction() {
    for (unsigned int i=0;i<24;++i) {
        D3DXEffectObject *effect=static_cast<D3DXEffectObject *>(UG2FxMalloc(sizeof(D3DXEffectObject)));
        memset(effect,0xa5,sizeof(*effect));effect->Construct();
        D3DXEffectBuffer *buffer=0;
        D3DXEffectPool *pool=0;
        if (D3DXEffectBufferCreate(17+i,&buffer) || D3DXEffectPoolCreate(&pool)) return false;
        // Caller and effect each own a reference. These are CPU objects with
        // recovered class identities, not fabricated GPU interfaces.
        D3DXEffectBufferAddRef(buffer);UG2FxPoolAddRefVirtual(pool);
        effect->value28=reinterpret_cast<unsigned int>(buffer);
        effect->value38=reinterpret_cast<unsigned int>(pool);
        D3DXEffectPoolLink sentinel={0};sentinel.previous0C=sentinel.next10=&sentinel;
        if (i&1) {
            D3DXEffectObject *nested=static_cast<D3DXEffectObject *>(UG2FxMalloc(sizeof(*nested)));
            memset(nested,0xa5,sizeof(*nested));nested->Construct();
            effect->value100=reinterpret_cast<unsigned int>(nested);
            D3DXEffectParameter *parameters=CleanupArray<D3DXEffectParameter>(2);
            effect->value9C=reinterpret_cast<unsigned int>(parameters);
            D3DXEffectPoolLink *link=static_cast<D3DXEffectPoolLink *>(UG2FxMalloc(sizeof(*link)));
            memset(link,0,sizeof(*link));link->data08=UG2FxMalloc(13);
            link->previous0C=link->next10=&sentinel;sentinel.previous0C=sentinel.next10=link;
            parameters[1].value40=reinterpret_cast<unsigned int>(link);
            D3DXEffectReference18 *references=CleanupArray<D3DXEffectReference18>(3);
            effect->value18=reinterpret_cast<unsigned int>(references);
            for (unsigned int j=0;j<3;++j) {
                references[j].value00=4;references[j].value04=reinterpret_cast<unsigned int>(buffer);
                D3DXEffectBufferAddRef(buffer);
            }
            effect->valueE0.value00=0;
            effect->valueE0.value0C=reinterpret_cast<unsigned int>(UG2FxMalloc(19));
            D3DXEffectRecordB4 *records=CleanupArray<D3DXEffectRecordB4>(2);
            effect->valueA4=reinterpret_cast<unsigned int>(records);
            records[1].array38=CleanupArray<D3DXEffectRecord38>(2);
            records[1].value8C[2]=reinterpret_cast<unsigned int>(buffer);D3DXEffectBufferAddRef(buffer);
            records[0].data40=UG2FxMalloc(23);
            D3DXEffectGroup64 *groups=CleanupArray<D3DXEffectGroup64>(2);
            effect->value2C=reinterpret_cast<unsigned int>(groups);
            groups[1].data00=UG2FxMalloc(11);groups[0].array04=CleanupArray<D3DXEffectRecord64>(2);
            groups[0].array04[1].array34=CleanupArray<D3DXEffectRecord20>(2);
        }
        if (D3DXEffectRelease(effect) || buffer->refs04!=1 || pool->refs04!=1 ||
            sentinel.previous0C!=&sentinel || sentinel.next10!=&sentinel) return false;
        if (D3DXEffectBufferRelease(buffer) || D3DXEffectPoolRelease(pool) || NativeFxAllocationCount()) return false;
    }
    return true;
}
bool ProbeEffectSetup() {
    // A synthetic metadata graph exercises setup and full cleanup together.
    // Real effect deserialization and shader preparation remain separate work.
    D3DXEffectObject *effect=static_cast<D3DXEffectObject *>(UG2FxMalloc(sizeof(*effect)));
    memset(effect,0xa5,sizeof(*effect));effect->Construct();
    D3DXEffectGroup64 *groups=CleanupArray<D3DXEffectGroup64>(3);
    effect->value2C=reinterpret_cast<unsigned int>(groups);
    for (unsigned int i=0;i<3;++i) {
        groups[i].data00=UG2FxMalloc(4);groups[i].value18=groups[i].value1A=0xffff;
    }
    groups[1].value08=8;
    D3DXEffectRecordB4 *techniques=CleanupArray<D3DXEffectRecordB4>(2);
    effect->valueA4=reinterpret_cast<unsigned int>(techniques);effect->valueA0=2;
    techniques[0].value34=2;techniques[0].array38=CleanupArray<D3DXEffectRecord38>(2);
    techniques[1].value34=1;techniques[1].array38=CleanupArray<D3DXEffectRecord38>(1);
    techniques[0].array38[0].value34=0;techniques[0].array38[1].value34=1;
    techniques[1].array38[0].value34=2;
    D3DXEffectParameter *parameters=CleanupArray<D3DXEffectParameter>(3);
    effect->value9C=reinterpret_cast<unsigned int>(parameters);effect->valueAC=3;
    D3DXEffectParameter **table=static_cast<D3DXEffectParameter **>(UG2FxMalloc(12));
    effect->valueB0=reinterpret_cast<unsigned int>(table);
    for (unsigned int i=0;i<3;++i) table[i]=parameters+i;
    D3DXEffectUseLink links[4]={};
    for (unsigned int i=0;i<3;++i) {
        links[i].value14=i;links[i].previous18=&links[(i+2)%3];links[i].next1C=&links[(i+1)%3];
    }
    links[3].value14=1;links[3].previous18=links[3].next1C=&links[3];
    parameters[0].value34=reinterpret_cast<unsigned int>(&links[0]);
    parameters[1].value34=reinterpret_cast<unsigned int>(&links[3]);
    if (UG2FxSetupEffect(effect)!=0 || effect->value40!=0xa5a5ffff || (effect->value118&0x800)) return false;
    for (unsigned int i=0;i<3;++i) if (groups[i].value3A || groups[i].value3C || reinterpret_cast<unsigned char *>(effect->valueBC)[i]) return false;
    D3DXEffectUseLink ***first=reinterpret_cast<D3DXEffectUseLink ***>(effect->valueB4);
    D3DXEffectUseLink ***second=reinterpret_cast<D3DXEffectUseLink ***>(effect->valueC0);
    if (!first[0] || first[0][0]!=&links[0] || first[0][1] || first[0][2] ||
        !second[0] || second[0][0]!=&links[1] || second[0][1] ||
        !first[1] || first[1][0] || first[1][1] || !second[1] || second[1][0]!=&links[3] || second[1][1] ||
        first[2] || second[2] || !first[3] || first[3][0]!=&links[2] || first[3][1] || second[3] ||
        first[4] || second[4] || first[5] || second[5]) return false;
    return D3DXEffectRelease(effect)==0 && NativeFxAllocationCount()==0;
}
bool ProbeRetailMetadata(unsigned int recipe,const char *name,FILE *capture,unsigned int &total) {
    D3DXEffectResource resource;resource.Construct();
    if (resource.Load(0,name,0,0)!=0 || resource.size08<8) return false;
    const unsigned int *words=reinterpret_cast<const unsigned int *>(resource.data04);
    unsigned int size=words[1];
    if (words[0]!=0xfeff0901 || size>resource.size08-8) return false;
    const unsigned char *blob=resource.data04+8;
    unsigned int cursor=2+(size+3)/4;
    if (cursor+4>resource.size08/4) return false;
    unsigned int parameters=words[cursor];cursor+=4;
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
    effect.value20=reinterpret_cast<unsigned int>(blob);effect.value1C=size;
    // Probe-only outer-header decoding selects scalar descriptors. It is not
    // the production deserializer and cannot construct an effect/shader.
    for (unsigned int i=0;i<parameters;++i) {
        if (cursor+4>resource.size08/4) return false;
        unsigned int typeOffset=words[cursor],dataOffset=words[cursor+1],annotations=words[cursor+3];
        cursor+=4;
        if (annotations>(resource.size08/4-cursor)/2) return false;
        cursor+=annotations*2;
        if (typeOffset>size || size-typeOffset<0x14) return false;
        const unsigned int *description=reinterpret_cast<const unsigned int *>(blob+typeOffset);
        if (description[1]>=4) continue;
        unsigned int type=effect.value20+typeOffset,data=effect.value20+dataOffset;
        int result=effect.ValidateType(&type,&data,0);
        if (result) return false;
        if (capture) {
            unsigned int record[8]={recipe,i,typeOffset,dataOffset,static_cast<unsigned int>(result),
                type-effect.value20,data-effect.value20,Checksum(blob,size)};
            if (fwrite(record,1,sizeof(record),capture)!=sizeof(record)) return false;
        }
        ++total;
    }
    // The module owns these bytes; this stack fixture never adopts them.
    effect.value20=0;D3DXDestroyEffect(&effect);resource.Clear();
    return NativeFxAllocationCount()==0;
}
bool ProbeTypeMetadata() {
    // Two synthetic wire records exercise the allocation/linking branches.
    // These are metadata fixtures and never enter shader preparation.
    for (unsigned int mode=0;mode<2;++mode) {
        D3DXEffectObject *effect=static_cast<D3DXEffectObject *>(UG2FxMalloc(sizeof(*effect)));
        memset(effect,0xa5,sizeof(*effect));effect->Construct();
        unsigned char *blob=static_cast<unsigned char *>(UG2FxMalloc(0x1000));memset(blob,0,0x1000);
        effect->value20=reinterpret_cast<unsigned int>(blob);effect->value1C=0x1000;
        *reinterpret_cast<unsigned int *>(blob)=2;blob[4]='a';
        unsigned int *description=reinterpret_cast<unsigned int *>(blob+0x40);
        description[0]=mode ? 11 : 16;description[1]=4;description[2]=0;description[3]=0x10;description[4]=mode ? 1 : 2;
        unsigned int *wire=reinterpret_cast<unsigned int *>(blob+0x80);
        if (!mode) { wire[0]=1;wire[1]=2; }
        else {
            wire[0]=3;
            const unsigned int codes[3]={165,164,172};
            for (unsigned int i=0;i<3;++i) {
                wire[1+i*4]=codes[i];wire[2+i*4]=0x12345678;wire[3+i*4]=0x100;wire[4+i*4]=0x140;
            }
            *reinterpret_cast<unsigned int *>(blob+0x140)=1;
        }
        D3DXEffectGroup64 *groups=CleanupArray<D3DXEffectGroup64>(2);
        memset(groups,0xa5,2*sizeof(*groups));D3DXEffectConstructArray(groups,sizeof(*groups),2,D3DXConstructGroup64);
        effect->value2C=reinterpret_cast<unsigned int>(groups);effect->value34=2;
        groups[0].value0C=mode ? 3 : 0;
        D3DXEffectReference18 *references=CleanupArray<D3DXEffectReference18>(3);
        memset(references,0xa5,3*sizeof(*references));D3DXEffectConstructArray(references,sizeof(*references),3,D3DXConstructReference18);
        effect->value18=reinterpret_cast<unsigned int>(references);effect->value14=3;
        D3DXEffectParameter *parameter=CleanupArray<D3DXEffectParameter>(1);parameter->Construct();
        parameter->value14=0;parameter->value28=1;parameter->value44=0;
        effect->value9C=reinterpret_cast<unsigned int>(parameter);
        D3DXEffectParameter **table=static_cast<D3DXEffectParameter **>(UG2FxMalloc(4));*table=parameter;
        effect->valueB0=reinterpret_cast<unsigned int>(table);
        unsigned int type=effect->value20+0x40,data=effect->value20+0x80;
        if (effect->ValidateType(&type,&data,0) || type!=effect->value20+0x54) return false;
        if (!mode) {
            if (data!=effect->value20+0x88 || parameter->value46!=2 || effect->value30!=2) return false;
            for (unsigned int i=0;i<2;++i) {
                D3DXEffectRecord64 *record=groups[i].array04;
                if (groups[i].value18 || groups[i].value36!=1 || record->value52!=0x92 || record->value58!=0x0a000000 || record->value04) return false;
                if (references[i+1].value00!=16 || static_cast<unsigned short>(references[i+1].value10)!=i) return false;
            }
            D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(parameter->value34);
            if (!head || head->value14!=0 || head->next1C->value14!=1 || head->next1C->next1C!=head ||
                head->previous18!=head->next1C || head->value04!=1 || head->next1C->value04!=1) return false;
        } else {
            if (data!=effect->value20+0xb4 || groups[0].value36!=3 || groups[0].value0C!=11 || parameter->value46!=1 ||
                groups[0].array04[0].value48!=3 || groups[0].array04[2].value48!=7 || references[1].value00!=5) return false;
        }
        if (D3DXEffectRelease(effect) || NativeFxAllocationCount()) return false;
    }
    return true;
}
}
int main(int argc,char **argv) {
    if (argc<2 || argc>4) { fprintf(stderr,"usage: ug2effectinput_probe speed2.exe [capture.bin [metadata.bin]]\n");return 2; }
    char error[512];
    if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *capture=argc>=3 ? fopen(argv[2],"wb") : 0;
    if (argc>=3 && !capture) return 15;
    if (capture && fwrite("UG2FXIN1",1,8,capture)!=8) return 15;
    FILE *metadata=argc==4 ? fopen(argv[3],"wb") : 0;
    if (argc==4 && !metadata) return 22;
    if (metadata && fwrite("UG2FXMD1",1,8,metadata)!=8) return 22;
    unsigned int total=0,descriptors=0;
    for (unsigned int i=0;i<23;++i) {
        // Read the resource-name pointer from the verified original recipe table.
        const char *name=*reinterpret_cast<const char *const *>(0x7ff7d0+i*0x8c+0x84);
        std::vector<unsigned short> wide;
        for (const char *c=name;*c;++c) wide.push_back(static_cast<unsigned char>(*c));
        wide.push_back(0);
        unsigned int checksum=0,length=0;
        for (unsigned int mode=0;mode<2;++mode) {
            D3DXEffectResource resource;resource.Construct();
            if (resource.Load(0,mode ? static_cast<const void *>(wide.data()) : name,1,mode)!=0) return 3;
            unsigned int retained=resource.handle00;
            unsigned int expected=Checksum(resource.data04,resource.size08);
            D3DXEffectBuffer *buffer=0;
            if (D3DXEffectBufferCreateForward(resource.size08,&buffer)!=0) return 4;
            length=D3DXEffectBufferLength(buffer);
            if (length!=resource.size08 || *reinterpret_cast<const unsigned int *>(resource.data04)!=0xfeff0901) return 5;
            memcpy(D3DXEffectBufferPointer(buffer),resource.data04,length);
            checksum=Checksum(D3DXEffectBufferPointer(buffer),length);
            if (checksum!=expected) return 6;
            if (capture) {
                unsigned int header[4]={i,mode,static_cast<unsigned int>(strlen(name)),length};
                if (fwrite(header,1,sizeof(header),capture)!=sizeof(header) ||
                    fwrite(name,1,header[2],capture)!=header[2] ||
                    fwrite(D3DXEffectBufferPointer(buffer),1,length,capture)!=length) return 15;
            }
            D3DXEffectBuffer *queried=0;
            if (D3DXEffectBufferQuery(buffer,ug2FxBufferIid7B7430,&queried)!=0 || queried!=buffer) return 7;
            if (D3DXEffectBufferRelease(queried)!=1 || D3DXEffectBufferRelease(buffer)!=0) return 8;
            resource.Destroy();
            if (resource.handle00!=retained || Checksum(resource.data04,length)!=expected || NativeFxAllocationCount()) return 9;
            // Reload clears the old handle via the recovered Clear call, while
            // the underlying module resource bytes retain their lifetime.
            if (resource.Load(0,name,0,0)!=0 || Checksum(resource.data04,length)!=expected) return 10;
            resource.Clear();if (resource.handle00 || resource.data04 || resource.size08) return 11;
            total+=length;
        }
        printf("effect input %u %s %u %08x\n",i,name,length,checksum);
        if (!ProbeRetailMetadata(i,name,metadata,descriptors)) return 23;
    }
    D3DXEffectResource missing;missing.Construct();
    if (static_cast<unsigned int>(missing.Load(0,"MISSING_EFFECT",1,0))!=0x88760b59) return 12;
    // The PE's numeric icon and #number lookup exercise the adapter's two
    // integer-identifier forms without treating them as compiled effects.
    bool numericTested=false;
    for (unsigned int id=1;id<256;++id) {
        unsigned int info=UG2FxFindResourceA(0,reinterpret_cast<const void *>(id),3);
        if (!info) continue;
        char numeric[16];snprintf(numeric,sizeof(numeric),"#%u",id);
        if (UG2FxFindResourceA(0,numeric,3)!=info || !UG2FxSizeofResource(0,info)) return 13;
        numericTested=true;break;
    }
    if (!numericTested) return 14;
    for (unsigned int i=0;i<24;++i) {
        D3DXEffectPool *pool=0,*queried=0;
        if (D3DXEffectPoolCreate(&pool)!=0 || pool->class00!=0x7ab4e8 || pool->refs04!=1 ||
            pool->value08 || pool->value0C || pool->data10 || pool->value14 || pool->value18 || pool->node1C) return 16;
        if (D3DXEffectPoolQuery(pool,ug2FxPoolIid7B7410,&queried)!=0 || queried!=pool || pool->refs04!=2) return 17;
        if (D3DXEffectPoolRelease(queried)!=1) return 18;
        D3DXEffectPoolLink sentinel={0};sentinel.previous0C=sentinel.next10=&sentinel;
        if (i==23) {
            // Native probe fixture for the recovered nonempty-node cleanup.
            // Shared parameter population from real effects remains open.
            D3DXEffectPoolNode *nodes[3];
            for (unsigned int j=0;j<3;++j) {
                nodes[j]=static_cast<D3DXEffectPoolNode *>(UG2FxMalloc(sizeof(D3DXEffectPoolNode)));
                memset(nodes[j],0,sizeof(*nodes[j]));nodes[j]->link0C.data08=UG2FxMalloc(17+j);
            }
            for (unsigned int j=0;j<3;++j) {
                nodes[j]->linked20=j<2 ? nodes[j+1] : 0;
                nodes[j]->link0C.previous0C=j ? &nodes[j-1]->link0C : &sentinel;
                nodes[j]->link0C.next10=j<2 ? &nodes[j+1]->link0C : &sentinel;
            }
            sentinel.next10=&nodes[0]->link0C;sentinel.previous0C=&nodes[2]->link0C;
            pool->node1C=nodes[0];pool->data10=UG2FxMalloc(31);
        }
        if (D3DXEffectPoolRelease(pool)!=0 || NativeFxAllocationCount() ||
            sentinel.next10!=&sentinel || sentinel.previous0C!=&sentinel) return 19;
    }
    if (capture && fclose(capture)!=0) return 15;
    if (metadata && fclose(metadata)!=0) return 22;
    if (!ProbeEffectDestruction()) return 20;
    if (!ProbeEffectSetup()) return 21;
    if (!ProbeTypeMetadata()) return 24;
    printf("effect setup: two rows, three parameters, four circular uses, exact grids and 0 live allocations\n");
    printf("effect metadata: %u retail scalar descriptors validated, 0 live allocations\n",descriptors);
    printf("effect type metadata: lazy-record/use rings and numeric/resource records, 0 live allocations\n");
    printf("effect destruction: 24 constructor/release cycles, nested synthetic cleanup graphs, 0 live allocations\n");
    printf("effect pool: 24 query/release cycles, three-node cleanup, 0 live allocations\n");
    printf("effect inputs: 23 recipes, 46 ANSI/UTF16 buffer cycles, %u copied bytes, %u live allocations\n",total,static_cast<unsigned int>(NativeFxAllocationCount()));
    return 0;
}
