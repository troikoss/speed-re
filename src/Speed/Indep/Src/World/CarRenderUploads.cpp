#include "CarRenderUploads.h"
#include "../Camera/CameraMatrix.h"
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output,WheelMatrix *left,WheelMatrix *right);

// Original 0x5bdde0. Cache updates and the second shader-byte check are observable.
void CarRenderUploadWorld(WheelMatrix *world,int view) {
    ++ug2RenderWorldCalls872EF4;
    if (world == &ug2RenderIdentity872E90) ++ug2RenderIdentityCalls872F00;
    if (world == ug2RenderLastWorld876850 && ug2RenderTechnique870910 == ug2RenderLastTechnique87684C && view == ug2RenderLastView800534) return;
    WheelMatrix *matrices = CarRenderViewByIndex(view)->matrices00;
    WheelMatrix *projection = matrices+2;
    if (view != ug2RenderLastView800534) {
        if (!ug2RenderTechnique870910->value18) {
            UG2RenderSetTransform(ug2SolidDevice870974,2,matrices);
            UG2RenderSetTransform(ug2SolidDevice870974,3,matrices+1);
        }
        ++ug2RenderViewChanges872EFC;
        ug2RenderLastView800534 = view;
    } else if (ug2RenderTechnique870910 != ug2RenderLastTechnique87684C && !ug2RenderTechnique870910->value18) {
        UG2RenderSetTransform(ug2SolidDevice870974,2,matrices);
        UG2RenderSetTransform(ug2SolidDevice870974,3,matrices+1);
    }
    ++ug2RenderWorldChanges872EF8;
    ug2RenderLastWorld876850 = world;
    ug2RenderLastTechnique87684C = ug2RenderTechnique870910;
    if (ug2RenderTechnique870910->parameters1C->slots00[58].handle24) {
        WheelMatrix product;
        D3DXMatrixMultiply(&product,world,matrices);
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[58].handle24,&product);
    }
    if (!ug2RenderTechnique870910->value18) {
        UG2RenderSetTransform(ug2SolidDevice870974,0x100,world);
        if (!ug2RenderTechnique870910->value18) return;
    }
    WheelMatrix product;
    D3DXMatrixMultiply(&product,world,projection);
    UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[59].handle24,&product);
}

// Original 0x5bdcd0: sixteen byte-selected matrices, copied as raw words.
void CarRenderUploadBones(void *matrices,CarRenderPrimitive *primitive) {
    WheelMatrix palette[16];
    unsigned int *output = reinterpret_cast<unsigned int *>(palette);
    for (int i=0;i<16;++i) {
        const unsigned int *input = static_cast<const unsigned int *>(matrices)+primitive->boneIndices21[i]*16;
        for (int j=0;j<16;++j) *output++ = *input++;
    }
    UG2EffectSetMatrixArray(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[6].handle24,palette,16);
}

// Original 0x60f510. The surrounding object types are only observed prefixes.
unsigned int CarRenderHeadlightColour(CarRenderHeadlightOwner *object) {
    if (object && object->value554) return object->value554->value380;
    return 0;
}

// Original 0x5bdaa0. The fixed-function path builds both projection and clip matrices.
void CarRenderUploadHeadlights(WheelMatrix *world,CarRenderViewRecord *view) {
    WheelMatrix *projection = view->matrices00+2;
    WheelMatrix bias,projected;
    WheelIdentity(&bias);
    bias.elements[12] = ug2RenderHeadlightBias80052C;
    bias.elements[13] = ug2RenderHeadlightBias800530;
    D3DXMatrixMultiply(&projected,world,projection);
    D3DXMatrixMultiply(&projected,&projected,&bias);
    if (ug2RenderTechnique870910->value18) {
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[36].handle24,&projected);
    } else {
        WheelMatrix inverse,cameraProjection;
        CameraInvertRigid(&inverse,CarRenderViewByIndex(ug2SolidViewMode870848[1])->matrices00);
        D3DXMatrixMultiply(&cameraProjection,projection,&bias);
        D3DXMatrixMultiply(&cameraProjection,&inverse,&cameraProjection);
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[36].handle24,&cameraProjection);
        WheelIdentity(&bias);
        bias.elements[0] = cameraProjection.elements[3]*ug2RenderClipScale79BFC0;
        bias.elements[4] = cameraProjection.elements[7]*ug2RenderClipScale79BFC0;
        bias.elements[8] = cameraProjection.elements[11]*ug2RenderClipScale79BFC0;
        bias.elements[12] = static_cast<float>(static_cast<long double>(cameraProjection.elements[15])*ug2RenderClipScale79BFC0+0.5f);
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[32].handle24,&bias);
    }
    unsigned int colour = 0;
    if (ug2RenderHeadlightContext8900B4 && ug2RenderHeadlightContext8900B4->value04)
        colour = CarRenderHeadlightColour(ug2RenderHeadlightContext8900B4->value04);
    float vector[4];
    vector[0] = static_cast<float>(colour&0xff)*0.0078125f;
    vector[1] = static_cast<float>((colour>>8)&0xff)*0.0078125f;
    vector[2] = static_cast<float>((colour>>16)&0xff)*0.0078125f;
    vector[3] = 0.0f;
    UG2EffectSetVector(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[34].handle24,vector);
}
