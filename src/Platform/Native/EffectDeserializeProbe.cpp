#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectSharedPool.h"
#include "../../Speed/Indep/Libs/D3DX/EffectDebug.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned int Word(unsigned int at) { return *reinterpret_cast<unsigned int *>(at); }
unsigned int Digest(unsigned int at,unsigned int count) {
    unsigned int value=2166136261;const unsigned char *bytes=reinterpret_cast<const unsigned char *>(at);
    for (unsigned int i=0;i<count;++i) value=(value^bytes[i])*16777619;return value;
}
void Fields(unsigned int at,const unsigned int *offsets,unsigned int count,std::vector<unsigned int> &words) {
    for (unsigned int i=0;i<count;++i) words.push_back(Word(at+offsets[i]));
}
#define FIELDS(at,...) do { const unsigned int offsets[]={__VA_ARGS__};Fields(at,offsets,sizeof(offsets)/4,words); } while (0)
#define ADD(x) words.push_back(x)
// Compare only fields written by the recovered bodies. Allocator addresses,
// preserved heap bits and undefined buffer-header words are not native outputs.
void Capture(D3DXEffectObject &effect,std::vector<unsigned int> &words) {
    unsigned int e=Address(&effect),blob=effect.value20,mat=((blob+effect.value1C)&~31u)-blob+32;
    FIELDS(e,0x14,0x1c,0x34,0x3c,0x40,0x44,0x98,0xa0,0xa8,0xac,0x10c,0x110,0x118,0x124,0x128,0x12c);
    ADD(effect.value30);ADD(Digest(blob,effect.value1C));ADD((blob+mat)%32==0);
    unsigned int at=blob+mat;ADD(Digest(at,effect.value124));at+=effect.value124;ADD(Digest(at,effect.value128));at+=effect.value128;ADD(Digest(at,effect.value12C));
    for (unsigned int i=0;i<effect.value10C;++i) ADD(Word(effect.value108+i*4));
    D3DXEffectParameter **table=reinterpret_cast<D3DXEffectParameter **>(effect.valueB0);
    D3DXEffectPool *pool=reinterpret_cast<D3DXEffectPool *>(effect.value38);
    for (unsigned int i=0;i<effect.valueA8;++i) {
        D3DXEffectParameter *p=table[i],*owner=table[p->value14];bool shared=owner->value40!=0;unsigned int base=shared ? Address(pool->data10) : blob;
        ADD(p->value00==base+owner->value30+p->value1C);ADD(p->value04==base+owner->value2C+p->value18);
        if (!i) continue;
        FIELDS(Address(p),8,0xc,0x10,0x14,0x18,0x1c,0x28,0x2c,0x38);
        unsigned int offset=p->value30;if (!shared&&offset>=effect.value1C) offset=0x80000000+offset-mat;
        ADD(offset);ADD(shared);ADD(Word(Address(p)+0x44));
    }
    for (unsigned int i=0;i<effect.value14;++i) {
        D3DXEffectReference18 &ref=reinterpret_cast<D3DXEffectReference18 *>(effect.value18)[i];
        ADD(ref.value00);ADD(ref.value0C);ADD(ref.value14&3);ADD((ref.value00==15||ref.value00==16)&&ref.value08 ? ref.value10&0xffff : 0);
        const unsigned int pointers[2]={ref.value04,ref.value08};
        for (unsigned int slot=0;slot<2;++slot) {
            D3DXEffectBuffer *b=reinterpret_cast<D3DXEffectBuffer *>(pointers[slot]);ADD(b!=0);ADD(b ? b->refs04 : 0);ADD(b ? b->size08 : 0);ADD(b ? Digest(Address(b->data0C),b->size08) : 0);
        }
    }
    for (unsigned int i=0;i<effect.value30;++i) {
        D3DXEffectGroup64 &g=reinterpret_cast<D3DXEffectGroup64 *>(effect.value2C)[i];ADD(g.value08&31);FIELDS(Address(&g),0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x34,0x38,0x3c);ADD(Digest(Address(g.data00),g.value0C));
        for (unsigned int j=0;j<g.value36;++j) {
            D3DXEffectRecord64 &r=g.array04[j];FIELDS(Address(&r),0,4,0x10,0x18,0x1c,0x20,0x24);
            ADD((r.value58==0x09000000||r.value58==0x0a000000||r.value58==0x0b000000) ? 0 : r.value48);FIELDS(Address(&r),0x50,0x54,0x58,0x5c);
            ADD(r.value30&31);ADD(r.value4E);ADD(r.value3C!=0);ADD(r.value40!=0);ADD(r.value44!=0);ADD(r.value38!=0);
            if (r.value38) {
                D3DXEffectBuffer *b=reinterpret_cast<D3DXEffectBuffer *>(r.value38);ADD(b->size08);FIELDS(Address(b->data0C),0,4,8,12);ADD(Digest(Address(b->data0C)+32,b->size08-32));
            }
        }
    }
    for (unsigned int i=0;i<effect.valueA0;++i) {
        D3DXEffectRecordB4 &t=reinterpret_cast<D3DXEffectRecordB4 *>(effect.valueA4)[i];FIELDS(Address(&t),0x28,0x2c,0x34);ADD(t.value3C&0xffff);FIELDS(Address(&t),0x88,0x8c,0x90,0x94,0x98);
        for (unsigned int j=0;j<t.value34;++j) { D3DXEffectRecord38 &p=t.array38[j];FIELDS(Address(&p),0x28,0x2c);ADD(p.value34&0xffff); }
    }
}
bool Probe(const std::vector<unsigned char> &payload,bool pooled,std::vector<unsigned int> &words) {
    ug2FxDebugRelease8A5CF8=ug2FxDebugRuntime8A5CF4=0;ug2FxDoNotMute80A170=-1;
    D3DXEffectPool *pool=0;if (pooled&&D3DXEffectPoolCreate(&pool)<0) return false;
    D3DXEffectBuffer *device=0;if (D3DXEffectBufferCreate(8,&device)<0) return false;
    D3DXEffectObject effects[2];D3DXEffectBuffer *buffers[2];unsigned int count=pooled ? 2 : 1;
    for (unsigned int i=0;i<count;++i) {
        D3DXEffectObject &e=effects[i];memset(&e,0xa5,sizeof(e));e.Construct();e.value98=e.value9C=e.valueA0=e.valueA4=0;e.valueA8=e.valueAC=7;e.value14=0;e.value1C=0;e.value118=pooled ? 0x300 : 0x200;
        if (D3DXEffectBufferCreate(payload.size(),&buffers[i])<0) return false;memcpy(buffers[i]->data0C,payload.data(),payload.size());
        unsigned int position=0;int result=UG2FxDeserializeEffect(&e,Address(device),pool,buffers[i],&position);if (result<0) { fprintf(stderr,"deserialize failed: %08x\n",result);return false; }
        ADD(result);ADD(position);ADD(device->refs04);ADD(buffers[i]->refs04);
    }
    for (unsigned int i=0;i<count;++i) Capture(effects[i],words);
    ADD(ug2FxDebugRelease8A5CF8);ADD(ug2FxDebugRuntime8A5CF4);ADD(ug2FxDoNotMute80A170);
    if (pool) { ADD(pool->refs04);ADD(pool->value14);ADD(pool->value18);ADD(Digest(Address(pool->data10),pool->value14)); }
    for (unsigned int i=count;i;--i) { D3DXDestroyEffect(&effects[i-1]);D3DXEffectBufferRelease(buffers[i-1]); }
    ADD(device->refs04);D3DXEffectBufferRelease(device);if (pool) { ADD(pool->refs04);D3DXEffectPoolRelease(pool); }
    return NativeFxAllocationCount()==0;
}
#undef ADD
#undef FIELDS
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectdeserialize_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;
    char magic[8];unsigned int count;if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXDF1",8)||fread(&count,4,1,fixtures)!=1||count>128||fwrite("UG2FXDD1",1,8,capture)!=8) return 3;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[2];if (fread(config,4,2,fixtures)!=2||config[0]>1||config[1]>0x100000) return 3;
        std::vector<unsigned char> payload(config[1]);std::vector<unsigned int> words;
        if (fread(payload.data(),1,payload.size(),fixtures)!=payload.size()||!Probe(payload,config[0]!=0,words)) return 4;
        unsigned int row[2]={i,static_cast<unsigned int>(words.size())};if (fwrite(row,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()) return 3;
        printf("effect deserialize %u: %u fields, 0 live allocations\n",i,row[1]);
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    printf("effect deserialize: %u native lifecycles, 0 live allocations\n",count);return 0;
}
