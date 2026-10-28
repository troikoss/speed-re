#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectSort.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

namespace {
unsigned int testDevice;
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned int Word(unsigned int at) { return *reinterpret_cast<const unsigned int *>(at); }
unsigned int Digest(unsigned int at,unsigned int count) { unsigned int value=2166136261;const unsigned char *p=reinterpret_cast<const unsigned char *>(at);for (unsigned int i=0;i<count;++i) value=(value^p[i])*16777619;return value; }
void __fastcall Parameter(void *p) { static_cast<D3DXEffectParameter *>(p)->Construct(); }
template<class T> T *Array(unsigned int count,D3DXEffectConstructor ctor) { unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*sizeof(T)+4));if (!cookie) return 0;*cookie=count;T *p=reinterpret_cast<T *>(cookie+1);memset(p,0xa5,count*sizeof(T));D3DXEffectConstructArray(p,sizeof(T),count,ctor);return p; }
D3DXEffectBuffer *Buffer(const unsigned char *data,unsigned int bytes) { D3DXEffectBuffer *p=0;if (D3DXEffectBufferCreate(bytes,&p)<0) return 0;if (bytes) { if (data) memcpy(p->data0C,data,bytes);else memset(p->data0C,0,bytes); }return p; }
#define ADD(value) words.push_back(value)
void Capture(D3DXEffectObject &effect,unsigned int index,D3DXEffectReference18 *reference,unsigned int result,D3DXEffectUseLink *nodes[2],std::vector<unsigned int> &words) {
    D3DXEffectGroup64 &g=reinterpret_cast<D3DXEffectGroup64 *>(effect.value2C)[index];D3DXEffectRecord64 *records=g.array04;ADD(result);ADD(g.value08&31);
    const unsigned int fields[]={0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c};for (unsigned int j=0;j<13;++j) ADD(Word(Address(&g)+fields[j]));
    ADD(reinterpret_cast<unsigned int *>(records)[-1]);ADD(reference->value14&3);
    D3DXEffectBuffer *code=reinterpret_cast<D3DXEffectBuffer *>(reference->value08);ADD(reference->value0C&&code ? reference->value0C-Address(code->data0C)+1 : 0);
    const unsigned int pointers[2]={reference->value04,reference->value08};for (unsigned int j=0;j<2;++j) { D3DXEffectBuffer *b=reinterpret_cast<D3DXEffectBuffer *>(pointers[j]);ADD(b!=0);ADD(b ? b->refs04 : 0);ADD(b ? b->size08 : 0);ADD(b ? Digest(Address(b->data0C),b->size08) : 0); }
    bool built=result==0&&reference->value0C;ADD(built);
    if (built) {
        ADD(g.value40);ADD(g.value44);ADD(g.value48);for (unsigned int j=0;j<11;++j) ADD(*reinterpret_cast<unsigned short *>(Address(&g)+0x4c+j*2));
        for (unsigned int j=0;j<g.value36;++j) {
            D3DXEffectRecord64 &r=records[j];bool pres=r.value52==0xb4||r.value52==0xb5;ADD(r.value00);ADD(r.value04);ADD(r.value30&31);ADD(r.value4E);ADD(r.value50);ADD(r.array34 ? reinterpret_cast<unsigned int *>(r.array34)[-1] : 0);
            if (j) { ADD(r.value52);ADD(r.value54);ADD(r.value56);ADD(r.value58); }
            if (j&&!pres) {
                ADD(r.value48);unsigned int bytes=r.value58==0x0f000000 ? 4 : r.value56*(r.value58==0x0c010000||r.value58==0x0d010000 ? 4 : 16);ADD(Digest(Address(g.data00)+r.value48,bytes));
                bool cached=(r.value58==0x0c000000||r.value58==0x0d000000)&&(r.value30&0x10);ADD(cached);D3DXEffectParameter *cacheOwner=cached ? reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[r.value08.value14] : 0;unsigned int cacheBase=cached&&cacheOwner->value40 ? Address(reinterpret_cast<D3DXEffectPool *>(effect.value38)->data10) : effect.value20;unsigned int cacheOffset=cached ? r.value5C-cacheBase : 0;if (cached&&!cacheOwner->value40&&cacheOffset>=effect.value1C) cacheOffset=0x80000000+cacheOffset-(((cacheBase+effect.value1C)&~31u)-cacheBase+32);ADD(cacheOffset);ADD(cached ? r.value60 : 0);
            }
            if (pres) {
                D3DXEffectBuffer *b=reinterpret_cast<D3DXEffectBuffer *>(r.value38);unsigned int *data=reinterpret_cast<unsigned int *>(b->data0C);ADD(b->refs04);ADD(b->size08);
                for (unsigned int k=0;k<22;++k) ADD(k<13||k>15||r.value3C ? data[k] : 0);for (unsigned int k=23;k<26;++k) ADD(data[k]);ADD(Digest(Address(data)+0x68,b->size08-0x68));
                const unsigned int ptrs[3]={r.value3C,r.value40,r.value44};for (unsigned int k=0;k<3;++k) ADD(ptrs[k] ? ptrs[k]-Address(data)+1 : 0);
            }
        }
        ADD(Digest(Address(g.data00)+g.value14,g.value34*2));
    }
    for (unsigned int j=0;j<effect.value98;++j) {
        D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(effect.value9C)+j;D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(parameter->value34),*at=head;std::vector<unsigned int> links;
        if (head) do { links.push_back(at->value00);links.push_back(at->value04);links.push_back(at->value14);links.push_back(at->value16);const unsigned int *entry=reinterpret_cast<const unsigned int *>(at->value10);for (unsigned int k=0;k<5;++k) links.push_back(entry ? entry[k] : 0);at=at->next1C; } while (at!=head);
        ADD(links.size()/9);words.insert(words.end(),links.begin(),links.end());
    }
    ADD(effect.valueB4!=0);ADD(effect.valueC0!=0);const unsigned int grids[2]={effect.valueB4,effect.valueC0};
    for (unsigned int j=0;j<2;++j) if (grids[j]) for (unsigned int row=0;row<6;++row) {
        unsigned int *slots=reinterpret_cast<unsigned int **>(grids[j])[row];std::vector<unsigned int> cells;for (;*slots;++slots) cells.push_back(*slots==Address(nodes[0]) ? 1 : *slots==Address(nodes[1]) ? 2 : 0);ADD(cells.size());words.insert(words.end(),cells.begin(),cells.end());
    }
}
bool Probe(const unsigned int c[19],const std::vector<unsigned char> &program,const std::vector<unsigned char> &program2,std::vector<unsigned int> &words) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();effect.value98=3;effect.valueA0=0;effect.valueA8=effect.valueAC=3;effect.value118=0x200;
    unsigned char *blob=static_cast<unsigned char *>(UG2FxMalloc(0x1000));if (!blob) return false;memset(blob,0,0x1000);effect.value20=Address(blob);effect.value1C=0x1000;
    D3DXEffectParameter *parameters=Array<D3DXEffectParameter>(3,Parameter);D3DXEffectGroup64 *groups=Array<D3DXEffectGroup64>(2,D3DXConstructGroup64);effect.valueB0=Address(UG2FxMalloc(12));if (!parameters||!groups||!effect.valueB0) return false;effect.value9C=Address(parameters);effect.value2C=Address(groups);effect.value34=2;
    for (unsigned int j=0;j<3;++j) {
        D3DXEffectParameter *p=parameters+j;reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[j]=p;p->value00=Address(blob)+0x800+j*0x100;p->value04=Address(blob)+0x200+j*0x40;p->value08=0xffffffff;p->value0C=64;p->value10=3;p->value14=j;p->value18=p->value1C=0;p->value28=c[5];p->value2C=0x200+j*0x40;p->value30=0x800+j*0x100;p->value40=0;p->value44=j+1;p->value46=3;
        unsigned int descriptor[7]={3,c[1],0x80+j*16,0x20,c[2],c[3],c[4]};memcpy(reinterpret_cast<void *>(p->value04),descriptor,sizeof(descriptor));
        if (c[1]==5) { unsigned int tree[13]={0,5,0x80+j*16,0,1,1,3,c[4]==3 ? 2u : 1u,0x80+j*16,0,1,1,4};memcpy(reinterpret_cast<void *>(p->value04),tree,sizeof(tree)); }
        *reinterpret_cast<unsigned int *>(blob+0x80+j*16)=3;char name[3]={'p',static_cast<char>('0'+j),0};memcpy(blob+0x84+j*16,name,sizeof(name));
    }
    for (unsigned int j=0;j<2;++j) { unsigned int count=j==0 ? c[11] : 2;groups[j].array04=Array<D3DXEffectRecord64>(count,D3DXConstructRecord64);if (!groups[j].array04) return false;groups[j].value36=2;groups[j].value38=count;groups[j].value0C=c[17]; }
    D3DXEffectRecord64 *record=groups[0].array04;if (c[11]==2) record->value30=0x1234567b;record->value00=1;record->value08.value0C=16;record->value08.value14=record->value08.value18=record->value08.value1C=0;record->value52=c[0]%2 ? 0x93 : 0x92;record->value58=c[0]%2 ? 0x0b000000 : 0x0a000000;groups[0].value08=c[10];
    D3DXEffectBuffer *device=Buffer(0,8);if (!device) return false;effect.value0C=Address(device);testDevice=Address(device);
    D3DXEffectReference18 *reference=Array<D3DXEffectReference18>(1,D3DXConstructReference18);if (!reference) return false;effect.value14=1;effect.value18=Address(reference);reference->value00=c[0]%2 ? 15 : 16;reference->value04=c[7] ? Address(Buffer(program.data(),program.size())) : 0;reference->value08=c[8] ? Address(Buffer(program.data(),program.size())) : 0;reference->value0C=0;reference->value10=0x12345678;reference->value14=(c[9] ? 2 : 0)|(c[6] ? 1 : 0);
    if (c[18]) {
        D3DXEffectPool *pool=static_cast<D3DXEffectPool *>(UG2FxMalloc(0x20));if (!pool) return false;pool->Construct();unsigned char *shared=static_cast<unsigned char *>(UG2FxMalloc(0x1000));if (!shared) return false;memcpy(shared,blob,0x1000);pool->data10=shared;pool->value14=pool->value18=0x1000;effect.value38=Address(pool);
        D3DXEffectPoolLink *link=static_cast<D3DXEffectPoolLink *>(UG2FxMalloc(0x14));unsigned int *mapping=static_cast<unsigned int *>(UG2FxMalloc(16));if (!link||!mapping) return false;memset(mapping,0,16);link->value00=Address(&effect);link->value04=Address(parameters);link->data08=mapping;link->previous0C=link->next10=link;
        parameters[0].value40=Address(link);parameters[0].value28|=1;parameters[0].value00=Address(shared)+0x800;parameters[0].value04=Address(shared)+0x200;*reinterpret_cast<unsigned int *>(shared+0x800)=3;
    }
    D3DXEffectUseLink *primaryUse=Array<D3DXEffectUseLink>(1,D3DXConstructUse);if (!primaryUse) return false;record->array34=reinterpret_cast<D3DXEffectRecord20 *>(primaryUse);record->value50=1;record->value4E=0;
    D3DXEffectUseLink *nodes[2]={0,0};
    if (c[12]) {
        for (unsigned int j=0;j<2;++j) { D3DXEffectRecord64 &r=groups[j].array04[1];nodes[j]=Array<D3DXEffectUseLink>(1,D3DXConstructUse);if (!nodes[j]) return false;r.array34=reinterpret_cast<D3DXEffectRecord20 *>(nodes[j]);r.value50=r.value4E=1;r.value52=0x94;r.value58=0x0c000000;effect.LinkParameter(j,1,0,parameters,0); }
        effect.valueA0=2;
        for (unsigned int which=0;which<2;++which) { unsigned int *grid=static_cast<unsigned int *>(UG2FxMalloc(24));if (!grid) return false;if (which) effect.valueC0=Address(grid);else effect.valueB4=Address(grid);
            for (unsigned int row=0;row<6;++row) { unsigned int *slots=static_cast<unsigned int *>(UG2FxMalloc(which ? 12 : 20));if (!slots) return false;grid[row]=Address(slots);if (which) { slots[0]=Address(nodes[0]);slots[1]=Address(nodes[1]);slots[2]=0; }else { slots[0]=Address(nodes[1]);for (unsigned int k=1;k<5;++k) slots[k]=0; } }
        }
    }
    effect.value118=0x200|(c[13] ? 0x800 : 0);unsigned int result=c[0]%2 ? effect.PrepareGroupB(0) : effect.PrepareGroupA(0);Capture(effect,0,reference,result,nodes,words);
    if (c[14]) { D3DXEffectBuffer *incoming=Buffer(program2.data(),program2.size());if (!incoming) return false;effect.WriteShaderResource(Address(incoming),0,0);D3DXEffectBufferRelease(incoming);result=c[0]%2 ? effect.PrepareGroupB(0) : effect.PrepareGroupA(0);Capture(effect,0,reference,result,nodes,words); }
    D3DXDestroyEffect(&effect);return !NativeFxAllocationCount();
}
bool Retail(unsigned int recipe,const std::vector<unsigned char> &payload,std::vector<unsigned int> &words) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();effect.value98=effect.value9C=effect.valueA0=effect.valueA4=0;effect.valueA8=effect.valueAC=7;effect.value14=effect.value1C=0;effect.value118=0x200;
    D3DXEffectBuffer *device=Buffer(0,8),*input=Buffer(payload.data(),payload.size());if (!device||!input) return false;testDevice=Address(device);unsigned int position=0;
    D3DXEffectPool *pool=0;if ((recipe%2)&&D3DXEffectPoolCreate(&pool)<0) return false;
    if (effect.Deserialize(Address(device),pool,input,&position)<0) return false;
    D3DXEffectUseLink *nodes[2]={0,0};D3DXEffectGroup64 *groups=reinterpret_cast<D3DXEffectGroup64 *>(effect.value2C);
    for (unsigned int index=0;index<effect.value30;++index) {
        D3DXEffectRecord64 *r=groups[index].array04;if (!r||(r->value58!=0x0a000000&&r->value58!=0x0b000000)) continue;
        bool pixel=r->value58==0x0b000000;D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(effect.valueB0)[r->value08.value14];
        unsigned int base=owner->value40 ? Address(pool->data10) : effect.value20;unsigned int resource=Word(base+owner->value30+r->value08.value1C);if (owner->value40) resource=static_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(owner->value40)->data08)[resource];D3DXEffectReference18 *ref=reinterpret_cast<D3DXEffectReference18 *>(effect.value18)+resource;
        D3DXEffectBuffer *code=reinterpret_cast<D3DXEffectBuffer *>(ref->value08);std::vector<unsigned char> program;
        if (code) program.assign(code->data0C,code->data0C+code->size08);
        if (!program.empty()&&!ref->value04) ref->value04=Address(Buffer(program.data(),program.size()));
        unsigned int result=pixel ? effect.PrepareGroupB(index) : effect.PrepareGroupA(index);Capture(effect,index,ref,result,nodes,words);
        if (!program.empty()) { D3DXEffectBuffer *incoming=Buffer(program.data(),program.size());if (!incoming) return false;effect.WriteShaderResource(Address(incoming),resource,0);D3DXEffectBufferRelease(incoming);result=pixel ? effect.PrepareGroupB(index) : effect.PrepareGroupA(index);Capture(effect,index,ref,result,nodes,words); }
    }
    D3DXDestroyEffect(&effect);D3DXEffectBufferRelease(device);D3DXEffectBufferRelease(input);if (pool) D3DXEffectPoolRelease(pool);return !NativeFxAllocationCount();
}
unsigned char *sortBase;unsigned int sortBytes;std::vector<unsigned int> sortTrace;
int __cdecl Compare(const void *a,const void *b) {
    unsigned int left=*static_cast<const unsigned char *>(a),right=*static_cast<const unsigned char *>(b);
    sortTrace.push_back(Address(a)-Address(sortBase));sortTrace.push_back(Address(b)-Address(sortBase));sortTrace.push_back(left);sortTrace.push_back(right);sortTrace.push_back(Digest(Address(sortBase),sortBytes));return left>right ? 1 : left<right ? -1 : 0;
}
bool Write(FILE *capture,unsigned int row,const std::vector<unsigned int> &words) { unsigned int header[2]={row,static_cast<unsigned int>(words.size())};return fwrite(header,4,2,capture)==2&&fwrite(words.data(),4,words.size(),capture)==words.size(); }
#undef ADD
}
// Test-only CPU adapters. These return staged bytecode and an owned CPU buffer
// device. No GPU shader is created, and production readback remains unsupported.
extern "C" int __stdcall UG2FxShaderDevice(unsigned int dispatch,unsigned int object,unsigned int *device) {
    if (dispatch!=0x7b711c||Word(object)!=0x7b711c) abort();D3DXEffectBufferAddRef(reinterpret_cast<D3DXEffectBuffer *>(testDevice));*device=testDevice;return 0;
}
extern "C" int __stdcall UG2FxShaderFunction(unsigned int dispatch,unsigned int object,void *output,unsigned int *bytes) {
    if (dispatch!=0x7b711c||Word(object)!=0x7b711c) abort();D3DXEffectBuffer *buffer=reinterpret_cast<D3DXEffectBuffer *>(object);if (output) { if (*bytes<buffer->size08) return static_cast<int>(0x80004005);memcpy(output,buffer->data0C,buffer->size08); }*bytes=buffer->size08;return 0;
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectpreparegroups_probe speed2.exe fixtures.bin capture.bin\n");return 2; }char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXGF1",8)||fread(&count,4,1,fixtures)!=1||count!=32||fwrite("UG2FXGD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) { unsigned int c[19];if (fread(c,4,19,fixtures)!=19||c[0]!=i||c[15]>0x10000||c[16]>0x10000) return 3;std::vector<unsigned char> program(c[15]),program2(c[16]);std::vector<unsigned int> words;
        if (fread(program.data(),1,program.size(),fixtures)!=program.size()||fread(program2.data(),1,program2.size(),fixtures)!=program2.size()||!Probe(c,program,program2,words)) return 4;unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;printf("effect prepare groups %u: %u fields, 0 live allocations\n",i,row[1]);
    }
    if (fread(&count,4,1,fixtures)!=1||count!=32) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int c[5];if (fread(c,4,5,fixtures)!=5||c[0]!=i||c[1]>1||c[4]>0x10000||c[4]!=c[2]*c[3]) return 3;std::vector<unsigned char> data(c[4]);if (fread(data.data(),1,data.size(),fixtures)!=data.size()) return 3;
        sortBase=data.data();sortBytes=data.size();sortTrace.clear();if (c[1]) D3DXEffectShortSort(sortBase,sortBase+(c[2]-1)*c[3],c[3],Compare);else D3DXEffectSort(sortBase,c[2],c[3],Compare);
        std::vector<unsigned int> words;words.push_back(Digest(Address(sortBase),sortBytes));words.push_back(sortTrace.size()/5);words.insert(words.end(),sortTrace.begin(),sortTrace.end());if (NativeFxAllocationCount()||!Write(capture,32+i,words)) return 4;
    }
    if (fread(&count,4,1,fixtures)!=1||count>23) return 3;
    for (unsigned int i=0;i<count;++i) { unsigned int c[2];if (fread(c,4,2,fixtures)!=2||c[0]!=i||c[1]>0x100000) return 3;std::vector<unsigned char> payload(c[1]);std::vector<unsigned int> words;if (fread(payload.data(),1,payload.size(),fixtures)!=payload.size()||!Retail(c[0],payload,words)||!Write(capture,64+i,words)) return 4;printf("effect retail preparation %u: %u fields, 0 live allocations\n",i,static_cast<unsigned int>(words.size())); }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;printf("effect prepare groups: 32 synthetic, 32 sort, %u retail lifecycles, 0 live allocations\n",count);return 0;
}
