#include "EffectPreparation.h"

// Original 0x65b098..0x65b0f1, RET 4. Kinds, dimensions and array counts are
// deliberately ignored; scalar/vector shapes and recursively matching structs
// advance the descriptor cursor, while a failing child retains its position.
extern "C" int __stdcall D3DXEffectVectorOnly(unsigned int **type) {
    unsigned int *description=*type;
    int shape=static_cast<int>(description[1]);
    if (shape<0) return 0;
    if (shape<2) { *type=description+7;return 1; }
    if (shape!=5) return 0;
    unsigned int members=description[5];*type=description+6;
    for (unsigned int i=0;i<members;++i) if (!D3DXEffectVectorOnly(type)) return 0;
    return 1;
}
// Original 0x660f9c..0x6611a1, thiscall / RET 12. Unlike BindCompiledRecord,
// missing CTAB names are skipped; +50 is twice the table count, narrowed to a
// word, while the cookie array is allocated for the actual count only.
int D3DXEffectObject::BindPreparationConstants(unsigned int groupIndex,unsigned int recordIndex,unsigned int *data) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    if (record->value50 || record->array34) return static_cast<int>(0x80004005);
    record->value30&=0xfffffffd;record->value4E=0;record->value50=0;record->value00=1;
    unsigned short counts[4],minimum[4],maximum[4];
    int result=D3DXEffectReadConstantTable(data+26,reinterpret_cast<unsigned int **>(&record->value3C),counts,minimum,maximum);
    if (result>=0 && record->value3C) {
        unsigned int count=reinterpret_cast<unsigned int *>(record->value3C)[3];
        unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x20+4));
        D3DXEffectUseLink *uses=0;
        if (cookie) { *cookie=count;uses=reinterpret_cast<D3DXEffectUseLink *>(cookie+1);D3DXEffectConstructArray(uses,0x20,count,D3DXConstructUse); }
        record->array34=reinterpret_cast<D3DXEffectRecord20 *>(uses);
        if (!uses) return static_cast<int>(0x8007000e);
        record->value50=static_cast<unsigned short>(reinterpret_cast<unsigned int *>(record->value3C)[3]<<1);
        unsigned int *entry=reinterpret_cast<unsigned int *>(record->value3C+reinterpret_cast<unsigned int *>(record->value3C)[4]);
        for (unsigned int i=0;i<reinterpret_cast<unsigned int *>(record->value3C)[3];++i,entry+=5) {
            D3DXEffectParameter *parameter=FindRootParameter(record->value3C+entry[0],10);
            if (parameter) {
                unsigned int index=record->value4E++;
                result=LinkParameter(groupIndex,recordIndex,index,parameter,reinterpret_cast<unsigned int>(entry));
                if (result<0) return result;
            }
        }
        unsigned int mask=static_cast<unsigned int>(maximum[1])+(static_cast<unsigned int>(maximum[0])+maximum[2])*4;
        data[6]=mask;if (mask) --mask;
        mask|=mask>>1;mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;data[6]=mask;
        unsigned int offset=group->value0C;
        unsigned int limit=(offset&7)+8+((offset>>3)+mask)*8;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[13]=offset;
        offset=group->value0C+static_cast<unsigned int>(maximum[0])*0x20;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[14]=offset;
        offset=group->value0C+static_cast<unsigned int>(maximum[1])*8;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[15]=offset;
        group->value0C+=static_cast<unsigned int>(maximum[2])*0x20;
        if (group->value0C<=limit) group->value0C=limit;
    }
    return result;
}
// Original 0x6611a2..0x661220, thiscall / RET 12. The parameter pointer in
// the use node can name a child; update the owner selected through +14.
int D3DXEffectObject::UnlinkParameterUse(unsigned int groupIndex,unsigned int recordIndex,unsigned int useIndex) {
    D3DXEffectRecord64 *record=reinterpret_cast<D3DXEffectGroup64 *>(value2C)[groupIndex].array04+recordIndex;
    D3DXEffectUseLink *use=reinterpret_cast<D3DXEffectUseLink *>(record->array34)+useIndex;
    use->previous18->next1C=use->next1C;use->next1C->previous18=use->previous18;
    D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(use->value0C);
    D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];
    if (owner->value34==reinterpret_cast<unsigned int>(use)) owner->value34=use->next1C==use ? 0 : reinterpret_cast<unsigned int>(use->next1C);
    use->previous18=use;use->next1C=use;
    if (!(reinterpret_cast<unsigned char *>(&value118)[1]&8) && valueB4) D3DXDestroyTemporaries(this);
    return 0;
}
// The two original 242-byte bodies differ only in their final shader creation
// call. Expand the common transcription so no new recovered call is introduced.
#define FX_CAPTURE_BYTECODE(create) \
    reference->value14&=0xfffffffe; \
    unsigned int size=reinterpret_cast<unsigned int>(reference),device=0; \
    D3DXEffectBuffer *replacement=0; \
    unsigned int dispatch=*reinterpret_cast<unsigned int *>(reference->value04); \
    int result=UG2FxShaderFunction(dispatch,reference->value04,0,&size); \
    if (result<0) goto finish; \
    dispatch=*reinterpret_cast<unsigned int *>(reference->value04); \
    result=UG2FxShaderDevice(dispatch,reference->value04,&device); \
    if (result<0) goto finish; \
    if (!reference->value08 || UG2FxBufferLengthVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08))<size) { \
        result=D3DXEffectBufferCreateForward(size,&replacement);if (result<0) goto finish; \
        if (reference->value08) { UG2FxComRelease(reference->value08);reference->value08=0; } \
        reference->value08=reinterpret_cast<unsigned int>(replacement);replacement=0; \
    } \
    dispatch=*reinterpret_cast<unsigned int *>(reference->value04); \
    { \
        unsigned char *data=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08)); \
        result=UG2FxShaderFunction(dispatch,reference->value04,data,&size); \
    } \
    if (result>=0 && device!=value0C) { \
        if (reference->value04) { UG2FxComRelease(reference->value04);reference->value04=0; } \
        create(reference); \
    } \
finish: \
    if (replacement) { UG2FxComRelease(reinterpret_cast<unsigned int>(replacement));replacement=0; } \
    if (device) UG2FxComRelease(device); \
    return result
// Original 0x661221..0x661312, thiscall / RET 4.
int D3DXEffectObject::CaptureVertexBytecode(D3DXEffectReference18 *reference) { FX_CAPTURE_BYTECODE(CreateVertexShader); }
// Original 0x661313..0x661404, thiscall / RET 4.
int D3DXEffectObject::CapturePixelBytecode(D3DXEffectReference18 *reference) { FX_CAPTURE_BYTECODE(CreatePixelShader); }
#undef FX_CAPTURE_BYTECODE
