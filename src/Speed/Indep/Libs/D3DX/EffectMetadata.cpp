#include "EffectMetadata.h"

// Original 0x401650: forward construction, with the signed count-1 guard.
extern "C" void __stdcall D3DXEffectConstructArray(void *first,unsigned int size,unsigned int count,D3DXEffectConstructor construct) {
    unsigned int remaining=count-1;
    if (static_cast<int>(remaining)>=0) {
        unsigned int address=reinterpret_cast<unsigned int>(first),iterations=remaining+1;
        do { construct(reinterpret_cast<void *>(address)); address+=size; } while (--iterations);
    }
}
// Original 0x65b680. Unspecified link payload words remain untouched.
extern "C" void __fastcall D3DXConstructUse(void *object) {
    D3DXEffectUseLink *p=static_cast<D3DXEffectUseLink *>(object);
    p->previous18=p; p->next1C=p; p->value10=0; p->value04=0; p->value08=0;
}
// Original 0x65b694.
extern "C" void __fastcall D3DXConstructBase(void *object) {
    D3DXEffectBaseRecord *p=static_cast<D3DXEffectBaseRecord *>(object);
    p->value08=0xffffffff; p->linked24=0; p->array20=0; p->value00=0; p->value04=0;
}
// Original 0x65b6c9.
extern "C" void __fastcall D3DXConstructGroup64(void *object) {
    D3DXEffectGroup64 *p=static_cast<D3DXEffectGroup64 *>(object);
    p->value08&=0xffffffe0; p->value3E=0xffff;
    p->data00=0; p->value0C=0; p->value10=0; p->array04=0;
    p->value34=0; p->value36=0; p->value38=0; p->value3C=0; p->value3A=0;
    unsigned short *selectors=&p->value18;
    unsigned char *bytes=reinterpret_cast<unsigned char *>(&p->value2C);
    unsigned short *words=reinterpret_cast<unsigned short *>(&p->value1C);
    for (unsigned int i=0;i<2;++i) {
        selectors[i]=0xffff;
        for (unsigned int j=0;j<4;++j) { bytes[i*4+j]=0; *words++=0; }
    }
}
// Original 0x65b78d.
extern "C" void __fastcall D3DXConstructReference18(void *object) {
    D3DXEffectReference18 *p=static_cast<D3DXEffectReference18 *>(object);
    p->value04=0; p->value08=0; p->value0C=0;
    unsigned int flags=(p->value14&0xfffffffe)|2;
    p->value00=0x7fffffff; p->value14=flags;
}
// Original 0x663ec5; its common base is embedded at +8.
extern "C" void __fastcall D3DXConstructRecord64(void *object) {
    D3DXEffectRecord64 *p=static_cast<D3DXEffectRecord64 *>(object);
    p->value08.value08=0xffffffff; p->value08.linked24=0; p->value08.array20=0;
    p->value08.value00=0; p->value08.value04=0;
    unsigned int flags=(p->value30&0xffffffe1)|1;
    p->value08.value10=3; p->value08.value14=0; p->value38=0; p->array34=0;
    p->value00=0; p->value04=0; p->value5C=0; p->value30=flags;
    p->value56=1; p->value4E=0; p->value50=0; p->value3C=0; p->value40=0; p->value44=0;
}
// Original 0x663f4e.
extern "C" void __fastcall D3DXConstructRecord38(void *object) {
    D3DXEffectRecord38 *p=static_cast<D3DXEffectRecord38 *>(object);
    p->value00.value08=0xffffffff; p->value00.linked24=0; p->value00.array20=0;
    p->value00.value00=0; p->value00.value04=0; p->value00.value10=1; p->parameter30=0;
}
// Original 0x663f6c.
extern "C" void __fastcall D3DXConstructRecordB4(void *object) {
    D3DXEffectRecordB4 *p=static_cast<D3DXEffectRecordB4 *>(object);
    p->value00.value08=0xffffffff; p->value00.linked24=0; p->value00.array20=0;
    p->value00.value00=0; p->value00.value04=0; p->value00.value10=0;
    p->parameter30=0; p->array38=0; p->data40=0; p->value88=0;
    for (unsigned int i=0;i<2;++i) {
        p->unknown9C[i]=0xffff; p->unknown9C[2+i*2]=0; p->unknown9C[3+i*2]=0;
        p->value8C[2+i]=0; p->value8C[i]=0;
    }
}
// Original 0x664fb1. Grow with freshly constructed records, destroy the old
// array only after successful allocation. Shrinking retains the capacity/data.
int D3DXEffectGroup64::ResizeRecords(unsigned short count) {
    if (value38<count) {
        unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(static_cast<unsigned int>(count)*0x64+4));
        D3DXEffectRecord64 *records=0;
        if (cookie) {
            *cookie=count; records=reinterpret_cast<D3DXEffectRecord64 *>(cookie+1);
            D3DXEffectConstructArray(records,0x64,count,D3DXConstructRecord64);
        }
        if (!records) return static_cast<int>(0x8007000e);
        if (array04) { array04->DeleteArray(3); array04=0; }
        array04=records; value38=count;
    }
    value36=count;
    return 0;
}
// Original 0x65bc5e. Bounds checks deliberately use wrapped absolute addresses.
int D3DXEffectObject::ValidateString(unsigned int offset) {
    if (value1C+value20<value20+4+offset) return static_cast<int>(0x80004005);
    unsigned int length=*reinterpret_cast<unsigned int *>(value20+offset);
    if (length) {
        if (value20+value1C<value20+length+offset+4) return static_cast<int>(0x80004005);
        if (*reinterpret_cast<char *>(value20+length-1+offset+4)) return static_cast<int>(0x80004005);
    }
    return 0;
}
// Original 0x65bcba. Names require at least one character and exclude four
// delimiter bytes. The length-prefixed string validator checks termination.
int D3DXEffectObject::ValidateName(unsigned int offset) {
    if (value1C+value20<value20+4+offset) return static_cast<int>(0x80004005);
    unsigned int *p=reinterpret_cast<unsigned int *>(offset+value20);
    if (*p<2) return static_cast<int>(0x80004005);
    int result=ValidateString(offset);
    if (result<0) return result;
    const char *s=reinterpret_cast<const char *>(p+1);
    while (*s) {
        if (*s=='.' || *s=='@' || *s=='[' || *s==']') return static_cast<int>(0x80004005);
        ++s;
    }
    return result;
}
// Original 0x65bf18. This walk counts padded storage; it is not the value-size
// walk below. Arithmetic and cursor writes retain unsigned 32-bit wrapping.
extern "C" unsigned int __stdcall D3DXEffectPaddedSize(unsigned int base,unsigned int *type,unsigned int *data) {
    unsigned int offset=*type,*p=reinterpret_cast<unsigned int *>(base+offset);
    unsigned int count=p[4]; if (!count) count=1;
    int shape=static_cast<int>(p[1]);
    if (shape>=0 && shape<4) {
        unsigned int width=(p[0]==1 || p[0]==2 || p[0]==3) ? 4 : 0;
        *type=offset+0x1c;
        unsigned int stride=shape<2 ? 4 : 0x10;
        *data=*data+width*count*stride;
        return width*count*stride;
    }
    if (shape==4) {
        unsigned int width=(static_cast<int>(p[0])<10 || static_cast<int>(p[0])>14) ? 4 : 0;
        *type=offset+0x14; *data=*data+count*4;
        return width*count;
    }
    if (shape==5) {
        unsigned int members=p[5],sum=0;
        *type=offset+0x18; unsigned int start=*data;
        while (members--) sum+=D3DXEffectPaddedSize(base,type,data);
        *data=(*data-start)*(count-1)+*data;
        return sum*count;
    }
    return 0;
}
// Original 0x65c033. Scalar/vector/matrix values use their actual dimensions.
extern "C" unsigned int __stdcall D3DXEffectValueSize(unsigned int base,unsigned int *type,unsigned int *data) {
    unsigned int offset=*type,*p=reinterpret_cast<unsigned int *>(base+offset);
    unsigned int count=p[4]; if (!count) count=1;
    int shape=static_cast<int>(p[1]);
    if (shape>=0 && shape<4) {
        unsigned int width=(p[0]==1 || p[0]==2 || p[0]==3) ? 4 : 0;
        unsigned int rows=p[5],columns=p[6];
        *type=offset+0x1c; *data=*data+width*columns*rows*count;
        return columns*rows*width*count;
    }
    if (shape==4) {
        unsigned int width=(static_cast<int>(p[0])<10 || static_cast<int>(p[0])>14) ? 4 : 0;
        *type=offset+0x14; *data=*data+count*4;
        return width*count;
    }
    if (shape==5) {
        unsigned int members=p[5],sum=0;
        *type=offset+0x18; unsigned int start=*data;
        while (members--) sum+=D3DXEffectValueSize(base,type,data);
        *data=(*data-start)*(count-1)+*data;
        return sum*count;
    }
    return 0;
}
// Original 0x65bd1e: splice a constructed use node into its parameter's ring.
int D3DXEffectObject::LinkParameter(unsigned int group,unsigned int record,unsigned int use,D3DXEffectParameter *parameter,unsigned int value) {
    D3DXEffectRecord64 *item=reinterpret_cast<D3DXEffectGroup64 *>(value2C)[group].array04+record;
    D3DXEffectUseLink *link=reinterpret_cast<D3DXEffectUseLink *>(item->array34)+use;
    link->value10=value; link->value08=reinterpret_cast<unsigned int>(item); link->value00=1;
    link->value0C=reinterpret_cast<unsigned int>(parameter); link->value14=static_cast<unsigned short>(group); link->value16=static_cast<unsigned short>(record);
    D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];
    if (!owner->value34) owner->value34=reinterpret_cast<unsigned int>(link);
    else {
        link->previous18=reinterpret_cast<D3DXEffectUseLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14]->value34)->previous18;
        link->next1C=reinterpret_cast<D3DXEffectUseLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14]->value34);
        reinterpret_cast<D3DXEffectUseLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14]->value34)->previous18->next1C=link;
        reinterpret_cast<D3DXEffectUseLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14]->value34)->previous18=link;
    }
    if (reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14]->value28&1) {
        link->value04=1;
        if (item->value58!=0x0a000000 && item->value58!=0x0b000000) item->value04=1;
    }
    if (!(reinterpret_cast<unsigned char *>(&value118)[1]&8) && valueB4) D3DXDestroyTemporaries(this);
    return 0;
}
