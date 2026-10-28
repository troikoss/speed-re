#include "EffectMetadata.h"

// Original 0x65b019, RET 8. alignment is a nonzero caller-supplied divisor.
extern "C" unsigned int __stdcall D3DXEffectUnitCount(unsigned int *type,unsigned int alignment) {
    unsigned int start=*type,*description=reinterpret_cast<unsigned int *>(start);
    unsigned int count=description[4]; if (!count) count=1;
    int shape=static_cast<int>(description[1]);
    if (shape>=0 && shape<4) {
        unsigned int rows=description[5],columns=description[6];
        *type=start+0x1c;
        return ((rows%alignment!=0)+rows/alignment)*columns*count;
    }
    if (shape==5) {
        unsigned int members=description[5],sum=0; *type=start+0x18;
        while (members--) sum+=D3DXEffectUnitCount(type,alignment);
        return sum*count;
    }
    return 0;
}

// These reads are duplicated in the original routines. The explicit calls and
// rereads retain getter/cursor ordering; a cursor advances even on a skipped read.
#define FX_READ_WORD(target) do { \
    if (buffer) { \
        unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
        if (*position+1<=limit) { \
            unsigned char *bytes=UG2FxBufferPointerVirtual(buffer); \
            (target)=*reinterpret_cast<unsigned int *>(bytes+*position*4); \
        } \
    } \
    ++*position; \
} while (0)
#define FX_CHECK_STREAM() do { \
    if (!buffer) return static_cast<int>(0x80004005); \
    unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
    if (limit<*position) return static_cast<int>(0x80004005); \
} while (0)

// Original 0x66a574.
int D3DXEffectObject::ReadAnnotation(D3DXEffectParameter *p,D3DXEffectBuffer *buffer,unsigned int *position) {
    p->value28=4; p->value38=0; p->value18=0; p->value1C=0;
    FX_READ_WORD(p->value2C); FX_READ_WORD(p->value30); FX_CHECK_STREAM();
    ++valueA8;
    unsigned int base=value20,offset=p->value2C;
    if (value1C+base<base+0x14+offset) return 0;
    unsigned int shape=reinterpret_cast<unsigned int *>(base+offset)[1];
    unsigned int count=reinterpret_cast<unsigned int *>(base+offset)[4];
    if (shape<=1) { value128+=(count>1 ? count : 1)*0x10; return 0; }
    if (shape<4) { value124+=(count>1 ? count : 1)*0x40; return 0; }
    if (shape!=5) return 0;
    unsigned int dataOffset=p->value30,type=offset+base,data=dataOffset+base;
    int result=ValidateType(&type,&data,0); if (result<0) return result;
    value12C+=D3DXEffectPaddedSize(value20,&offset,&dataOffset);
    return result;
}
// Original 0x66b420, ECX effect, RET 12.
int D3DXEffectObject::ReadParameter(D3DXEffectParameter *p,D3DXEffectBuffer *buffer,unsigned int *position) {
    p->value1C=0; p->value18=0;
    FX_READ_WORD(p->value2C); FX_READ_WORD(p->value30); FX_READ_WORD(p->value28); FX_READ_WORD(p->value38); FX_CHECK_STREAM();
    int result=0;
    D3DXEffectParameter *tail=p;
    for (unsigned int i=0;i<p->value38;++i) {
        D3DXEffectParameter *annotation=static_cast<D3DXEffectParameter *>(UG2FxMalloc(0x50));
        if (!annotation) return static_cast<int>(0x8007000e);
        annotation->Construct();
        result=ReadAnnotation(annotation,buffer,position);
        if (result<0) { annotation->Delete(1); return result; }
        tail->value3C=reinterpret_cast<unsigned int>(annotation);tail=annotation;
    }
    ++valueA8;
    unsigned int base=value20,offset=p->value2C;
    if (value1C+base<base+0x14+offset) return result;
    unsigned int shape=reinterpret_cast<unsigned int *>(base+offset)[1];
    unsigned int count=reinterpret_cast<unsigned int *>(base+offset)[4];
    if (shape<=1) { value128+=(count>1 ? count : 1)*0x10; return result; }
    if (shape<4) { value124+=(count>1 ? count : 1)*0x40; return result; }
    if (shape!=5) return result;
    unsigned int dataOffset=p->value30,type=offset+base,data=dataOffset+base;
    result=ValidateType(&type,&data,0); if (result<0) return result;
    value12C+=D3DXEffectPaddedSize(value20,&offset,&dataOffset);
    return result;
}
// Original 0x66a6af, ECX effect, RET 12.
int D3DXEffectObject::ReadPass(D3DXEffectRecord38 *p,D3DXEffectBuffer *buffer,unsigned int *position) {
    unsigned int count=reinterpret_cast<unsigned int>(p);
    FX_READ_WORD(p->value28); FX_READ_WORD(p->value2C); FX_READ_WORD(count); FX_CHECK_STREAM();
    int result=ValidateString(p->value28); if (result<0) return result;
    D3DXEffectParameter *tail=0;
    for (unsigned int i=0;i<p->value2C;++i) {
        D3DXEffectParameter *annotation=static_cast<D3DXEffectParameter *>(UG2FxMalloc(0x50));
        if (!annotation) return static_cast<int>(0x8007000e);
        annotation->Construct();
        result=ReadAnnotation(annotation,buffer,position);
        if (result<0) { annotation->Delete(1); return result; }
        if (!tail) p->parameter30=annotation; else tail->value3C=reinterpret_cast<unsigned int>(annotation);
        tail=annotation;
    }
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(p->value34);
    result=group->ResizeRecords(static_cast<unsigned short>(count)); if (result<0) return result;
    for (unsigned int i=0;(i&0xffff)<count;++i) {
        result=ReadRecord(static_cast<unsigned short>(p->value34),static_cast<unsigned short>(i),buffer,position);
        if (result<0) return result;
    }
    return result;
}
// Original 0x66164d, RET 16. The first two stream writes are DWORD stores
// across adjacent halfword fields. Keep them wide even on truncated input.
int D3DXEffectObject::ReadRecord(unsigned short groupIndex,unsigned short recordIndex,D3DXEffectBuffer *buffer,unsigned int *position) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *p=group->array04+recordIndex;
    p->value00=0; p->value08.value14=0;
    FX_READ_WORD(*reinterpret_cast<unsigned int *>(&p->value52));
    FX_READ_WORD(*reinterpret_cast<unsigned int *>(&p->value54));
    FX_READ_WORD(p->value08.value18); FX_READ_WORD(p->value08.value1C); FX_CHECK_STREAM();
    unsigned int code=p->value52;
    if (code>=0xb6) return static_cast<int>(0x80004005);
    unsigned int limit=ug2FxTypeTable7B582C[code][3];
    if (limit && limit<=p->value54) return static_cast<int>(0x80004005);
    if (limit==0x105) {
        unsigned short slot=p->value54;
        if (slot<0x10 || slot==0x101 || slot==0x102 || slot==0x103 || slot==0x104) {
            if (ug2FxTypeTable7B582C[code][4]==0x0300000d) return static_cast<int>(0x80004005);
        } else if (slot!=0x100) return static_cast<int>(0x80004005);
    }
    p->value56=0;
    unsigned int tag=ug2FxTypeTable7B582C[code][4]; p->value58=tag;
    unsigned int bytes=0;
    if (tag==0x09000000 || tag==0x0a000000 || tag==0x0b000000) {
        if (tag==0x0a000000) group->value18=recordIndex;
        else if (tag==0x0b000000) group->value1A=recordIndex;
        unsigned int index=*reinterpret_cast<unsigned int *>(p->value08.value1C+value20);
        reinterpret_cast<D3DXEffectReference18 *>(value18)[index].value00=ug2FxTypeTable7B582C[p->value52][0];
        p->value00=0; p->value30&=0xfffffffd; p->value56=1; p->value48=0;
        return 0;
    }
    if ((tag>=0x0c000001 && tag<=0x0c000004) || (tag>=0x0d000001 && tag<=0x0d000004)) {
        if ((tag&0xffff)==4) p->value56=1;
        if ((tag&0xffff)>=3) ++p->value56;
        if ((tag&0xffff)>=2) ++p->value56;
        ++p->value56;
        if (tag>=0x0d000001) { p->value52=0x9c; p->value58=0x0d000000; }
        else { p->value52=0x94; p->value58=0x0c000000; }
        unsigned int currentTag=ug2FxTypeTable7B582C[p->value52][4];
        if (currentTag==0x0d000000 || currentTag==0x0c000000) {
            unsigned int type=p->value08.value18+value20;
            p->value56=static_cast<unsigned short>(D3DXEffectUnitCount(&type,4));
        }
        bytes=static_cast<unsigned int>(p->value56)<<4; p->value00=1;
    } else if (tag==0x0c000000 || tag==0x0d000000 || tag==0x0c020000 || tag==0x0d020000) {
        unsigned int type=p->value08.value18+value20;
        p->value56=static_cast<unsigned short>(D3DXEffectUnitCount(&type,4));
        bytes=static_cast<unsigned int>(p->value56)<<4; p->value00=1;
    } else if (tag==0x0c010000 || tag==0x0d010000) {
        unsigned int type=p->value08.value18+value20;
        p->value56=static_cast<unsigned short>(D3DXEffectUnitCount(&type,1));
        bytes=static_cast<unsigned int>(p->value56)<<2; p->value00=1;
    } else if (tag==0x0e000000 || tag==0x0f000000) {
        p->value56=1; ++group->value34;
        p->value00=0; p->value30&=0xfffffffd; bytes=4;
        goto assign_storage;
    } else {
        unsigned int t=ug2FxTypeTable7B582C[code][0];
        unsigned int width=(t==1 || t==2 || t==3 || t==0xf20000) ? 4 : 0;
        bytes=ug2FxTypeTable7B582C[code][2]*ug2FxTypeTable7B582C[code][1]*width;
        p->value56=1; p->value00=1;
    }
    p->value30&=0xfffffffd;
    if (!bytes) { p->value48=0; return 0; }
assign_storage:
    unsigned int offset=group->value0C;
    offset=(offset&3)+(offset>>2)*4;
    offset=(offset&1)+(offset>>1)*2;
    offset=(offset&3)+(offset>>2)*4;
    offset=(offset&3)+(offset>>2)*4;
    offset=(offset&3)+(offset>>2)*4;
    group->value0C=offset; p->value48=offset; group->value0C+=bytes;
    return 0;
}
// Original 0x66b60d. Parse pass groups and then aggregate unique-resource and
// register maxima by scanning each group's records in the original directions.
int D3DXEffectObject::ReadTechnique(D3DXEffectRecordB4 *p,D3DXEffectBuffer *buffer,unsigned int *position) {
    FX_READ_WORD(p->value28); FX_READ_WORD(p->value2C); FX_READ_WORD(p->value34); FX_CHECK_STREAM();
    int result=ValidateString(p->value28); if (result<0) return result;
    D3DXEffectParameter *tail=0;
    for (unsigned int i=0;i<p->value2C;++i) {
        D3DXEffectParameter *annotation=static_cast<D3DXEffectParameter *>(UG2FxMalloc(0x50));
        if (!annotation) return static_cast<int>(0x8007000e);
        annotation->Construct(); result=ReadAnnotation(annotation,buffer,position);
        if (result<0) { annotation->Delete(1); return result; }
        if (!tail) p->parameter30=annotation; else tail->value3C=reinterpret_cast<unsigned int>(annotation);
        tail=annotation;
    }
    if (p->value34) {
        unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(p->value34*0x38+4));
        D3DXEffectRecord38 *passes=0;
        if (cookie) {
            *cookie=p->value34;passes=reinterpret_cast<D3DXEffectRecord38 *>(cookie+1);
            D3DXEffectConstructArray(passes,0x38,p->value34,D3DXConstructRecord38);
        }
        p->array38=passes;
        if (!passes) return static_cast<int>(0x8007000e);
    }
    for (unsigned short i=0;i<p->value34;++i) {
        if (value34<=value30) return static_cast<int>(0x80004005);
        *reinterpret_cast<unsigned short *>(&p->array38[i].value34)=value30; ++value30;
        *reinterpret_cast<unsigned short *>(&p->array38[i].value00.value14)=static_cast<unsigned short>(p->value00.value14);
        *reinterpret_cast<unsigned short *>(&p->array38[i].value00.value18)=i;
        result=ReadPass(p->array38+i,buffer,position); if (result<0) return result;
    }
    unsigned int maximum=0;
    for (unsigned int i=0;(i&0xffff)<p->value34;++i) {
        D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(p->array38[i&0xffff].value34);
        unsigned int remaining=group->value36;
        D3DXEffectRecord64 *record=group->array04;
        while (remaining--) {
            if ((ug2FxTypeTable7B582C[record->value52][4]&0xff000000)==0x08000000 && maximum<record->value54) maximum=record->value54;
            ++record;
        }
    }
    unsigned int mapBytes=maximum*2+2;
    unsigned short *unique=static_cast<unsigned short *>(UG2FxMalloc(mapBytes));
    unsigned int *used=0;
    if (!unique) return static_cast<int>(0x8007000e);
    {
        unsigned char *bytes=reinterpret_cast<unsigned char *>(unique);
        for (unsigned int i=0;i<mapBytes;++i) bytes[i]=0;
    }
    *reinterpret_cast<unsigned short *>(&p->value3C)=0;
    for (unsigned int i=0;(i&0xffff)<p->value34;++i) {
        D3DXEffectRecord38 *pass=p->array38+(i&0xffff);
        D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(pass->value34);
        for (unsigned int j=0;j<group->value36;++j) {
            D3DXEffectRecord64 *record=group->array04+j;
            if ((ug2FxTypeTable7B582C[record->value52][4]&0xff000000)==0x08000000 && !unique[record->value54]) {
                ++*reinterpret_cast<unsigned short *>(&p->value3C);
                unique[record->value54]=*reinterpret_cast<unsigned short *>(&p->value3C);
            }
        }
    }
    if (*reinterpret_cast<unsigned short *>(&p->value3C)) {
        p->data40=UG2FxMalloc(static_cast<unsigned int>(*reinterpret_cast<unsigned short *>(&p->value3C))*0x68);
        used=static_cast<unsigned int *>(UG2FxMalloc(static_cast<unsigned int>(*reinterpret_cast<unsigned short *>(&p->value3C))*4));
        if (!p->data40 || !used) { result=static_cast<int>(0x8007000e); goto finish_technique; }
    }
    p->unknown9C[2]=0; p->unknown9C[3]=0; p->unknown9C[4]=0; p->unknown9C[5]=0;
    for (unsigned int i=0;(i&0xffff)<p->value34;++i) {
        D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(p->array38[i&0xffff].value34);
        bool seen=false;
        for (unsigned int j=0;j<*reinterpret_cast<unsigned short *>(&p->value3C);++j) used[j]=0;
        unsigned int remaining=group->value36,offset=remaining*0x64;
        while (remaining--) {
            offset-=0x64;
            D3DXEffectRecord64 *record=reinterpret_cast<D3DXEffectRecord64 *>(reinterpret_cast<unsigned int>(group->array04)+offset);
            unsigned int tag=ug2FxTypeTable7B582C[record->value52][4],category=tag>>24;
            if (category==7) {
                bool mark=!seen;seen=true;
                record->value30^=(static_cast<unsigned int>(mark)^record->value30)&1;
            } else if (category==8) {
                record->value4C=static_cast<unsigned short>(unique[record->value54]-1);
                if (record->value58==0x0800000d) record->value30|=1;
                else {
                    record->value30^=(static_cast<unsigned int>(!used[record->value4C])^record->value30)&1;
                    used[record->value4C]=1;
                }
            } else {
                unsigned short *target=0;
                unsigned int value=0;
                if (category==3 || category==9 || category==15) {
                    if (record->value54<0x100) { value=record->value54+record->value56; target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xb2); }
                    else if (record->value54>0x100) { value=(record->value56-0x101)+static_cast<unsigned int>(record->value54); target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xaa); }
                } else if (category==12) {
                    if (tag==0x0c000000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xa4);
                    else if (tag==0x0c010000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xa6);
                    else if (tag==0x0c020000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xa8);
                    value=record->value54+record->value56;
                } else if (category==13) {
                    if (tag==0x0d000000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xac);
                    else if (tag==0x0d010000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xae);
                    else if (tag==0x0d020000) target=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(p)+0xb0);
                    value=record->value54+record->value56;
                }
                if (target) {
                    if (static_cast<int>(value)<static_cast<int>(*target)) value=*target;
                    *target=static_cast<unsigned short>(value);
                }
            }
        }
    }
finish_technique:
    UG2FxFree(unique); if (used) UG2FxFree(used);
    return result;
}
#undef FX_READ_WORD
#undef FX_CHECK_STREAM
