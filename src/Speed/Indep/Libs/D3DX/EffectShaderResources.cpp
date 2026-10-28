#include "EffectShaderResources.h"

// Original 0x65b970. The output argument is pushed before the buffer getter,
// but belongs to the subsequent GPU call, not the getter's signature.
void D3DXEffectObject::CreateVertexShader(D3DXEffectReference18 *reference) {
    UG2FxDebugMute(1);
    if (!(reinterpret_cast<unsigned char *>(&value118)[1]&2)) {
        unsigned int dispatch=*reinterpret_cast<unsigned int *>(value0C);
        unsigned char *code=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
        int result=UG2FxCreateVertexShader(dispatch,value0C,code,&reference->value04);
        if (result<0) {
            UG2FxDebugMute(0);
            reference->value04=0;
            reference->value14&=0xfffffffc;
        } else {
            UG2FxDebugMute(0);
            reference->value14|=3;
        }
    }
}
// Original 0x65b9ce. Preserve the same mute/error behavior for vtable+0x1a8.
void D3DXEffectObject::CreatePixelShader(D3DXEffectReference18 *reference) {
    UG2FxDebugMute(1);
    if (!(reinterpret_cast<unsigned char *>(&value118)[1]&2)) {
        unsigned int dispatch=*reinterpret_cast<unsigned int *>(value0C);
        unsigned char *code=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
        int result=UG2FxCreatePixelShader(dispatch,value0C,code,&reference->value04);
        if (result<0) {
            UG2FxDebugMute(0);
            reference->value04=0;
            reference->value14&=0xfffffffc;
        } else {
            UG2FxDebugMute(0);
            reference->value14|=3;
        }
    }
}

#define FX_RESOURCE_WORD(destination) do { \
    if (buffer) { \
        unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
        if (*position+1<=limit) { \
            unsigned char *bytes=UG2FxBufferPointerVirtual(buffer); \
            destination=*reinterpret_cast<unsigned int *>(bytes+*position*4); \
        } \
    } \
    ++*position; \
} while (0)

// Original 0x65ba2c. Stage the stream bytes before inspecting an existing
// resource. Even zero-size entries allocate and free the temporary block.
int D3DXEffectObject::ReadResource(D3DXEffectBuffer *buffer,unsigned int *position) {
    unsigned int index,size;
    FX_RESOURCE_WORD(index);
    FX_RESOURCE_WORD(size);
    if (!buffer) return static_cast<int>(0x80004005);
    unsigned int headerLimit=UG2FxBufferLengthVirtual(buffer);
    if (headerLimit<*position || value14<=index) return static_cast<int>(0x80004005);
    unsigned char *temporary=static_cast<unsigned char *>(UG2FxMalloc(size));
    int result;
    unsigned int finalLimit;
    if (!temporary) { result=static_cast<int>(0x8007000e); goto finish; }
    if (size) {
        unsigned int count=(size>>2)+((size&3)!=0);
        unsigned int limit=UG2FxBufferLengthVirtual(buffer);
        if (*position+count<=limit) {
            unsigned char *source=UG2FxBufferPointerVirtual(buffer);
            source+=*position*4;
            unsigned int i;
            for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(temporary)[i]=reinterpret_cast<unsigned int *>(source)[i];
            for (i=size&0xfffffffc;i<size;++i) temporary[i]=source[i];
        }
        *position+=count;
    }
    finalLimit=UG2FxBufferLengthVirtual(buffer);
    if (*position<=finalLimit) {
        D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+index;
        result=0;
        if (!reference->value04 && !reference->value08) {
            if (!size) goto finish;
            if (reference->value00==4) {
                if (!temporary[size-1]) {
                    result=D3DXEffectBufferCreateForward(size,reinterpret_cast<D3DXEffectBuffer **>(&reference->value04));
                    if (result>=0) {
                        unsigned char *destination=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value04));
                        unsigned int i;
                        for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(destination)[i]=reinterpret_cast<unsigned int *>(temporary)[i];
                        for (i=size&0xfffffffc;i<size;++i) destination[i]=temporary[i];
                    }
                    goto finish;
                }
            } else if (reference->value00>14 && reference->value00<17) {
                result=D3DXEffectBufferCreateForward(size,reinterpret_cast<D3DXEffectBuffer **>(&reference->value08));
                if (result>=0) {
                    unsigned char *destination=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
                    unsigned int i;
                    for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(destination)[i]=reinterpret_cast<unsigned int *>(temporary)[i];
                    for (i=size&0xfffffffc;i<size;++i) destination[i]=temporary[i];
                    if (reference->value00==16) {
                        CreateVertexShader(reference);
                        reinterpret_cast<D3DXEffectGroup64 *>(value2C)[static_cast<unsigned short>(reference->value10)].value18=0;
                    } else if (reference->value00==15) {
                        CreatePixelShader(reference);
                        reinterpret_cast<D3DXEffectGroup64 *>(value2C)[static_cast<unsigned short>(reference->value10)].value1A=0;
                    }
                    reinterpret_cast<D3DXEffectGroup64 *>(value2C)[static_cast<unsigned short>(reference->value10)].array04->value00=1;
                }
                goto finish;
            }
        }
    }
    result=static_cast<int>(0x80004005);
finish:
    if (temporary) UG2FxFree(temporary);
    return result;
}

// Original 0x66520d. Convert a resource-bearing record into an indirect group
// reference after constructing its lazy shader record and reading its bytes.
int D3DXEffectObject::ReadShaderRecord(unsigned int groupIndex,unsigned int recordIndex,D3DXEffectBuffer *buffer,unsigned int *position) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    unsigned int kind=ug2FxTypeTable7B582C[record->value52][0];
    if (kind!=16 && kind!=15) return 0;
    // The original reuses the incoming group argument as the initial size.
    unsigned int size=groupIndex;
    FX_RESOURCE_WORD(size);
    unsigned int headerLimit=0;
    if (buffer) headerLimit=UG2FxBufferLengthVirtual(buffer);
    if (buffer && *position<=headerLimit) {
        D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14];
        unsigned int index;
        if (!parameter->value40) index=*reinterpret_cast<unsigned int *>(parameter->value30+record->value08.value1C+value20);
        else {
            unsigned int shared=*reinterpret_cast<unsigned int *>(parameter->value30+record->value08.value1C+reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10));
            index=reinterpret_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(parameter->value40)->data08)[shared];
        }
        D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+index;
        if (value30<value34) {
            *reinterpret_cast<unsigned short *>(&reference->value10)=value30;
            ++value30;
            D3DXEffectGroup64 *target=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(reference->value10);
            int result=target->ResizeRecords(1);
            if (result<0) return result;
            reference->value00=ug2FxTypeTable7B582C[record->value52][0];
            target=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(reference->value10);
            if (reference->value00==16) {
                target->value18=0;target->array04->value52=0x92;target->array04->value58=0x0a000000;
            } else {
                target->value1A=0;target->array04->value52=0x93;target->array04->value58=0x0b000000;
            }
            target->array04->value54=0xffff;target->array04->value56=1;
            target->array04->value30|=1;target->array04->value00=1;
            unsigned int *source=reinterpret_cast<unsigned int *>(&record->value08);
            unsigned int *destination=reinterpret_cast<unsigned int *>(&target->array04->value08);
            for (unsigned int i=0;i<10;++i) destination[i]=source[i];
            target->array04->value08.array20=0;target->array04->value08.linked24=0;
            if (!size) return result;
            result=D3DXEffectBufferCreateForward(size,reinterpret_cast<D3DXEffectBuffer **>(&reference->value08));
            if (result<0) return result;
            unsigned char *bytes=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
            unsigned int count=(size>>2)+((size&3)!=0);
            unsigned int limit=UG2FxBufferLengthVirtual(buffer);
            if (*position+count<=limit) {
                unsigned char *input=UG2FxBufferPointerVirtual(buffer);
                input+=*position*4;
                unsigned int i;
                for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(bytes)[i]=reinterpret_cast<unsigned int *>(input)[i];
                for (i=size&0xfffffffc;i<size;++i) bytes[i]=input[i];
            }
            *position+=count;
            unsigned int finalLimit=UG2FxBufferLengthVirtual(buffer);
            if (*position<=finalLimit) {
                if (reference->value00==16) CreateVertexShader(reference);
                else CreatePixelShader(reference);
                record->value54=0xffff;
                record->value00=0;record->value52=0xb3;record->value58=0x0e000000;record->value56=1;
                ++group->value34;
                unsigned int offset=(group->value0C&3)+(group->value0C>>2)*4;
                group->value0C=offset;record->value48=offset;group->value0C+=4;
                return 0;
            }
        }
    }
    return static_cast<int>(0x80004005);
}
#undef FX_RESOURCE_WORD
