#include "EffectSetup.h"

// Original 0x65b92d. The old allocation is freed even when null. On failure
// only the pointer changes; the count and capacity retain their old values.
int D3DXEffectObject::ResetDirtyQueue() {
    UG2FxFree(reinterpret_cast<void *>(value108));
    value108=reinterpret_cast<unsigned int>(UG2FxMalloc(0x400));
    if (!value108) return static_cast<int>(0x8007000e);
    value10C=0;
    value110=0x100;
    return 0;
}

// Original 0x65aaaa. Capacity doubles before allocation, including on OOM.
int D3DXEffectObject::GrowDirtyQueue() {
    value110<<=1;
    unsigned int *grown=static_cast<unsigned int *>(UG2FxMalloc(value110*4));
    if (!grown) return static_cast<int>(0x8007000e);
    unsigned int *old=reinterpret_cast<unsigned int *>(value108);
    unsigned int count=(value10C*4)>>2;
    for (unsigned int i=0;i<count;++i) grown[i]=old[i];
    UG2FxFree(reinterpret_cast<void *>(value108));
    value108=reinterpret_cast<unsigned int>(grown);
    return 0;
}

// Original 0x65b733. The shift/mask expression retains an odd input offset;
// it does not align it. Allocate before freeing, without copying old data.
int D3DXEffectGroup64::AllocateData() {
    unsigned int offset=(value0C&1)+(value0C>>1)*2;
    value14=offset;
    value0C=offset+static_cast<unsigned int>(value34)*2;
    if (value10<value0C) {
        void *grown=UG2FxMalloc(value0C);
        if (!grown) return static_cast<int>(0x8007000e);
        if (data00) { UG2FxFree(data00); data00=0; }
        data00=grown;
        value10=value0C;
    }
    return 0;
}

// Original 0x65e591, secondary-interface receiver. Retain the new device
// before rereading/releasing the old one, including a self-assignment.
int D3DXEffectSecondaryPrefix::SetDevice(unsigned int device) {
    if (device) UG2FxComAddRef(device);
    if (value08) { UG2FxComRelease(value08); value08=0; }
    value08=device;
    value04=device;
    return 0;
}

// Original 0x65ab14. Before setup, walk the parameter's use ring and locate
// records by group/record words. After setup, walk its null-terminated grid
// row and use each link's direct record pointer. Bit 0 of byte 0x119 enables
// the queue; bit 1 at record+0x30 suppresses duplicate queue entries.
int D3DXEffectObject::MarkParameterDirty(D3DXEffectParameter *parameter) {
    if (!valueB4) {
        D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];
        D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(owner->value34);
        if (head) {
            D3DXEffectUseLink *use=head->previous18;
            if (!(reinterpret_cast<unsigned char *>(&value118)[1]&1)) {
                do {
                    use=use->next1C;
                    use->value00=1;
                    reinterpret_cast<D3DXEffectGroup64 *>(value2C)[use->value14].array04[use->value16].value00=1;
                } while (use->next1C!=reinterpret_cast<D3DXEffectUseLink *>(owner->value34));
            } else {
                do {
                    use=use->next1C;
                    use->value00=1;
                    D3DXEffectRecord64 *record=reinterpret_cast<D3DXEffectGroup64 *>(value2C)[use->value14].array04+use->value16;
                    record->value00=1;
                    if (!(record->value30&2)) {
                        if (value110<=value10C) { int result=GrowDirtyQueue(); if (result<0) return result; }
                        unsigned short *entry=reinterpret_cast<unsigned short *>(value108+value10C*4);
                        entry[0]=use->value14;
                        entry[1]=use->value16;
                        record->value30|=2;
                        ++value10C;
                    }
                } while (use->next1C!=reinterpret_cast<D3DXEffectUseLink *>(owner->value34));
            }
        }
    } else if (parameter->value14<valueAC) {
        reinterpret_cast<unsigned char *>(valueBC)[parameter->value14]=1;
        D3DXEffectUseLink **row=reinterpret_cast<D3DXEffectUseLink ***>(valueB8)[parameter->value14];
        if (row) {
            if (!(reinterpret_cast<unsigned char *>(&value118)[1]&1)) {
                for (;*row;++row) {
                    (*row)->value00=1;
                    *reinterpret_cast<unsigned int *>((*row)->value08)=1;
                }
            } else {
                for (;*row;++row) {
                    D3DXEffectUseLink *use=*row;
                    D3DXEffectRecord64 *record=reinterpret_cast<D3DXEffectRecord64 *>(use->value08);
                    use->value00=1;
                    record->value00=1;
                    if (!(record->value30&2)) {
                        if (value110<=value10C) { int result=GrowDirtyQueue(); if (result<0) return result; }
                        unsigned short *entry=reinterpret_cast<unsigned short *>(value108+value10C*4);
                        entry[0]=use->value14;
                        entry[1]=use->value16;
                        record->value30|=2;
                        ++value10C;
                    }
                }
            }
        }
    }
    return 0;
}
