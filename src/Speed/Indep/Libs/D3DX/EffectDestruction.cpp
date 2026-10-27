#include "EffectDestruction.h"

// Original 0x64b5ff. The signed (count-1) guard and address arithmetic are x86
// 32-bit operations. Do not turn the guard into a simple count!=0 test.
extern "C" void __stdcall D3DXEffectDestroyArray(void *first,unsigned int size,
    unsigned int count,D3DXEffectDestructor destroy) {
    unsigned int address=reinterpret_cast<unsigned int>(first)+size*count;
    unsigned int remaining=count-1;
    if (static_cast<int>(remaining)>=0) {
        unsigned int iterations=remaining+1;
        do { address-=size; destroy(reinterpret_cast<void *>(address)); } while (--iterations);
    }
}
// Original 0x6996f4: a shared empty destructor.
extern "C" void __fastcall D3DXDestroyEmpty(void *) {}

// Each expansion is a separate original deleting-destructor entry. Array
// cookies precede the first element; mask 0x02 selects arrays, 0x01 frees storage.
#define FX_DELETE_ARRAY(Type,Destroy) \
Type *Type::DeleteArray(unsigned char flags) { \
    if (flags&2) { \
        unsigned int *cookie=reinterpret_cast<unsigned int *>(this)-1; \
        D3DXEffectDestroyArray(this,sizeof(Type),*cookie,Destroy); \
        if (flags&1) UG2FxFree(cookie); \
        return reinterpret_cast<Type *>(cookie); \
    } \
    Destroy(this); \
    if (flags&1) UG2FxFree(this); \
    return this; \
}
FX_DELETE_ARRAY(D3DXEffectBaseRecord,D3DXDestroyBase) // 0x661405
FX_DELETE_ARRAY(D3DXEffectReference08,D3DXDestroyReference08) // 0x66238c
FX_DELETE_ARRAY(D3DXEffectReference18,D3DXDestroyReference18) // 0x661453
FX_DELETE_ARRAY(D3DXEffectParameter,D3DXDestroyParameter) // 0x6651bf
FX_DELETE_ARRAY(D3DXEffectRecord64,D3DXDestroyRecord64) // 0x664f63
FX_DELETE_ARRAY(D3DXEffectGroup64,D3DXDestroyGroup64) // 0x66a4d5
FX_DELETE_ARRAY(D3DXEffectRecord38,D3DXDestroyRecord38) // 0x665050
FX_DELETE_ARRAY(D3DXEffectRecordB4,D3DXDestroyRecordB4) // 0x66a523
#undef FX_DELETE_ARRAY

// Original 0x660f55. The scalar path does not call the empty destructor.
D3DXEffectRecord20 *D3DXEffectRecord20::DeleteArray(unsigned char flags) {
    if (flags&2) {
        unsigned int *cookie=reinterpret_cast<unsigned int *>(this)-1;
        D3DXEffectDestroyArray(this,sizeof(D3DXEffectRecord20),*cookie,D3DXDestroyEmpty);
        if (flags&1) UG2FxFree(cookie);
        return reinterpret_cast<D3DXEffectRecord20 *>(cookie);
    }
    if (flags&1) UG2FxFree(this);
    return this;
}

#define FX_DELETE(Type,Destroy) \
Type *Type::Delete(unsigned char flags) { \
    Destroy(this); \
    if (flags&1) UG2FxFree(this); \
    return this; \
}
FX_DELETE(D3DXEffectBaseRecord,D3DXDestroyBase) // 0x65b6a8
FX_DELETE(D3DXEffectParameter,D3DXDestroyParameter) // 0x664f42
FX_DELETE(D3DXEffectValuePrefix,D3DXDestroyValue) // 0x66519e
#undef FX_DELETE
// Original 0x6424e3.
D3DXEffectPoolLink *D3DXEffectPoolLink::Delete(unsigned char flags) {
    Unlink();
    if (flags&1) UG2FxFree(this);
    return this;
}
// Original 0x663e90.
extern "C" void __fastcall D3DXDestroyBase(void *object) {
    D3DXEffectBaseRecord *p=static_cast<D3DXEffectBaseRecord *>(object);
    if (p->array20) { p->array20->DeleteArray(3); p->array20=0; }
    while (p->linked24) {
        D3DXEffectBaseRecord *child=p->linked24;
        p->linked24=child->linked24; child->linked24=0;
        child->Delete(1);
    }
}
// Original 0x664efa. The common base is at +0, unlike the 0x64-byte record.
extern "C" void __fastcall D3DXDestroyParameter(void *object) {
    D3DXEffectParameter *p=static_cast<D3DXEffectParameter *>(object);
    while (p->value3C) {
        D3DXEffectParameter *child=reinterpret_cast<D3DXEffectParameter *>(p->value3C);
        p->value3C=child->value3C; child->value3C=0;
        D3DXDestroyParameter(child); UG2FxFree(child);
    }
    if (p->value40) { reinterpret_cast<D3DXEffectPoolLink *>(p->value40)->Delete(1); p->value40=0; }
    D3DXDestroyBase(p);
}
// Original 0x663fcf. Signed type comparisons control ownership of value0C.
extern "C" void __fastcall D3DXDestroyValue(void *object) {
    D3DXEffectValuePrefix *p=static_cast<D3DXEffectValuePrefix *>(object);
    if (p->value14) { reinterpret_cast<D3DXEffectReference08 *>(p->value14)->DeleteArray(3); p->value14=0; }
    int kind=static_cast<int>(p->value00);
    if (kind==0 || kind==2 || kind==4 || (kind>5 && kind<=10)) {
        if (p->value0C) { UG2FxFree(reinterpret_cast<void *>(p->value0C)); p->value0C=0; }
    }
    while (p->value18) {
        D3DXEffectValuePrefix *child=reinterpret_cast<D3DXEffectValuePrefix *>(p->value18);
        p->value18=child->value18; child->value18=0;
        D3DXDestroyValue(child); UG2FxFree(child);
    }
}
// Original 0x65ee40.
extern "C" void __fastcall D3DXDestroyReference08(void *object) {
    D3DXEffectReference08 *p=static_cast<D3DXEffectReference08 *>(object);
    int kind=p->value00;
    if (kind==4 || (kind>4 && (kind<=9 || kind==15 || kind==16))) {
        if (p->value04) { UG2FxComRelease(p->value04); p->value04=0; }
    }
}
// Original 0x65b7ad: value08 is released independently of the type code.
extern "C" void __fastcall D3DXDestroyReference18(void *object) {
    D3DXEffectReference18 *p=static_cast<D3DXEffectReference18 *>(object);
    unsigned int ref=p->value04;
    if (ref) {
        int kind=p->value00;
        if (kind==4 || (kind>4 && (kind<=9 || kind==15 || kind==16))) {
            UG2FxComRelease(ref); p->value04=0;
        }
    }
    if (p->value08) { UG2FxComRelease(p->value08); p->value08=0; }
}
// Original 0x663f15: the base receiver is +8, not +0.
extern "C" void __fastcall D3DXDestroyRecord64(void *object) {
    D3DXEffectRecord64 *p=static_cast<D3DXEffectRecord64 *>(object);
    unsigned int ref=p->value38;
    p->value08.array20=0; p->value08.linked24=0;
    if (ref) { UG2FxComRelease(ref); p->value38=0; }
    if (p->array34) { p->array34->DeleteArray(3); p->array34=0; }
    D3DXDestroyBase(&p->value08);
}
// Original 0x66a419.
extern "C" void __fastcall D3DXDestroyGroup64(void *object) {
    D3DXEffectGroup64 *p=static_cast<D3DXEffectGroup64 *>(object);
    if (p->data00) { UG2FxFree(p->data00); p->data00=0; }
    if (p->array04) { p->array04->DeleteArray(3); p->array04=0; }
}
// Original 0x665031.
extern "C" void __fastcall D3DXDestroyRecord38(void *object) {
    D3DXEffectRecord38 *p=static_cast<D3DXEffectRecord38 *>(object);
    if (p->parameter30) { p->parameter30->Delete(1); p->parameter30=0; }
    D3DXDestroyBase(p);
}
// Original 0x66a442. Release order is +88, +94, +8c, +98, +90.
extern "C" void __fastcall D3DXDestroyRecordB4(void *object) {
    D3DXEffectRecordB4 *p=static_cast<D3DXEffectRecordB4 *>(object);
    if (p->parameter30) { p->parameter30->Delete(1); p->parameter30=0; }
    if (p->array38) { p->array38->DeleteArray(3); p->array38=0; }
    if (p->data40) { UG2FxFree(p->data40); p->data40=0; }
    if (p->value88) { UG2FxComRelease(p->value88); p->value88=0; }
    unsigned int *slot=p->value8C;
    for (unsigned int i=0;i<2;++i,++slot) {
        if (slot[2]) { UG2FxComRelease(slot[2]); slot[2]=0; }
        if (slot[0]) { UG2FxComRelease(slot[0]); slot[0]=0; }
    }
    D3DXDestroyBase(p);
}
// Original 0x65e4ff. Receiver points at the effect's secondary interface (+4).
extern "C" void __fastcall D3DXDestroySecondary(void *object) {
    unsigned int *p=static_cast<unsigned int *>(object);
    unsigned int ref=p[3]; p[0]=0x7b6e00;
    if (ref) { UG2FxComRelease(ref); p[3]=0; }
    if (p[2]) { UG2FxComRelease(p[2]); p[2]=0; }
}
// Original 0x65b7f0. Scratch valueB8 and freed pointer slots are deliberately
// retained. Counts, row pointers and stride are reread after external calls.
extern "C" void __fastcall D3DXDestroyTemporaries(void *object) {
    D3DXEffectObject *p=static_cast<D3DXEffectObject *>(object);
    if (p->valueB4) {
        for (unsigned int i=0;i<p->valueA0;++i) {
            p->valueB8=p->valueB4+p->valueAC*i*4;
            for (unsigned int j=0;j<p->valueAC;++j) UG2FxFree(reinterpret_cast<void **>(p->valueB8)[j]);
        }
        UG2FxFree(reinterpret_cast<void *>(p->valueB4)); p->valueB4=0;
    }
    if (p->valueC0) {
        for (unsigned int i=0;i<p->valueA0;++i) {
            p->valueB8=p->valueC0+p->valueAC*i*4;
            for (unsigned int j=0;j<p->valueAC;++j) UG2FxFree(reinterpret_cast<void **>(p->valueB8)[j]);
        }
        UG2FxFree(reinterpret_cast<void *>(p->valueC0)); p->valueC0=0;
    }
}
// Original 0x66adec, including all embedded destructors and ownership helpers.
extern "C" void __fastcall D3DXDestroyEffect(void *object) {
    D3DXEffectObject *p=static_cast<D3DXEffectObject *>(object);
    p->class00=0x7b6fe0;
    if (p->valueDC) { reinterpret_cast<D3DXEffectBaseRecord *>(p->valueDC)->Delete(1); p->valueDC=0; }
    if (p->valueD8) {
        for (unsigned int i=0;i<p->valueD0;++i) {
            if (reinterpret_cast<D3DXEffectValuePrefix **>(p->valueD8)[i]) {
                reinterpret_cast<D3DXEffectValuePrefix **>(p->valueD8)[i]->Delete(1);
                reinterpret_cast<unsigned int *>(p->valueD8)[i]=0;
            }
        }
    }
    D3DXDestroyTemporaries(p);
    UG2FxFree(reinterpret_cast<void *>(p->valueBC));
    for (unsigned int i=0;i<p->value120;++i) {
        if (reinterpret_cast<void **>(p->value11C)[i]) {
            UG2FxFree(reinterpret_cast<void **>(p->value11C)[i]);
            reinterpret_cast<unsigned int *>(p->value11C)[i]=0;
        }
    }
    if (p->value11C) { UG2FxFree(reinterpret_cast<void *>(p->value11C)); p->value11C=0; }
    if (p->valueD8) { UG2FxFree(reinterpret_cast<void *>(p->valueD8)); p->valueD8=0; }
    if (p->value2C) { reinterpret_cast<D3DXEffectGroup64 *>(p->value2C)->DeleteArray(3); p->value2C=0; }
    if (p->valueC4) { UG2FxFree(reinterpret_cast<void *>(p->valueC4)); p->valueC4=0; }
    if (p->value9C) { reinterpret_cast<D3DXEffectParameter *>(p->value9C)->DeleteArray(3); p->value9C=0; }
    if (p->valueA4) { reinterpret_cast<D3DXEffectRecordB4 *>(p->valueA4)->DeleteArray(3); p->valueA4=0; }
    if (p->value18) { reinterpret_cast<D3DXEffectReference18 *>(p->value18)->DeleteArray(3); p->value18=0; }
    if (p->value20) { UG2FxFree(reinterpret_cast<void *>(p->value20)); p->value20=0; }
    if (p->valueB0) { UG2FxFree(reinterpret_cast<void *>(p->valueB0)); p->valueB0=0; }
    if (p->valueCC) { UG2FxFree(reinterpret_cast<void *>(p->valueCC)); p->valueCC=0; }
    if (p->value100) { UG2FxComRelease(p->value100); p->value100=0; }
    if (p->value104) { UG2FxComRelease(p->value104); p->value104=0; }
    if (p->value28) { UG2FxComRelease(p->value28); p->value28=0; }
    if (p->value38) { UG2FxComRelease(p->value38); p->value38=0; }
    UG2FxFree(reinterpret_cast<void *>(p->value108));
    D3DXDestroyValue(&p->valueE0);
    D3DXDestroyParameter(&p->value48);
    D3DXDestroySecondary(&p->interface04);
}
