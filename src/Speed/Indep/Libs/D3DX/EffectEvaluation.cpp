#include "EffectEvaluation.h"

// Original 0x663a32..0x663bb0. Preserve dirty-use publication, bank strides,
// first-call control state and the complete VM argument list. No restore here.
int D3DXEffectObject::EvaluateRecord(unsigned int groupIndex,unsigned int recordIndex) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    unsigned int *header=reinterpret_cast<unsigned int *>(UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(record->value38)));
    unsigned int output=reinterpret_cast<unsigned int>(group->data00)+header[3],outputMask=header[1];
    unsigned int input=0,inputMask=0;
    if (!(value118&1)) {
        value114=D3DXEffectControlFp(0,0);
        D3DXEffectControlFp(0,0x30000);
        value118|=1;
    }
    if (record->value3C) {
        unsigned int base=reinterpret_cast<unsigned int>(group->data00);
        unsigned int vectors=base+header[4],scalars=base+header[5],integers=base+header[6];
        for (unsigned short use=0;use<record->value4E;++use) {
            D3DXEffectUseLink *node=reinterpret_cast<D3DXEffectUseLink *>(record->array34)+use;
            if (node->value00) {
                node->value00=node->value04;
                unsigned int *entry=reinterpret_cast<unsigned int *>(node->value10);
                unsigned short bank=static_cast<unsigned short>(entry[1]),start=static_cast<unsigned short>(entry[1]>>16),count=static_cast<unsigned short>(entry[2]);
                D3DXEffectBaseRecord *parameter=reinterpret_cast<D3DXEffectBaseRecord *>(node->value0C);
                if (!bank) D3DXEffectCopyEvaluationScalars(parameter,reinterpret_cast<double *>(scalars+start*8),count,record->value3C,reinterpret_cast<unsigned int>(entry),&input,0xffffffff);
                else if (bank==1) D3DXEffectCopyEvaluationVectors(parameter,reinterpret_cast<double *>(integers+start*32),count,record->value3C,reinterpret_cast<unsigned int>(entry),&input,0xffffffff);
                else if (bank==2) D3DXEffectCopyEvaluationVectors(parameter,reinterpret_cast<double *>(vectors+start*32),count,record->value3C,reinterpret_cast<unsigned int>(entry),&input,0xffffffff);
            }
        }
        input=reinterpret_cast<unsigned int>(group->data00)+header[4];inputMask=header[2];
    }
    return UG2FxExecuteVm(reinterpret_cast<unsigned int *>(record->value40),reinterpret_cast<unsigned int *>(record->value44),0,input,0,
        output,output,output,0,inputMask,0,outputMask,outputMask,outputMask);
}
