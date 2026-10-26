#ifndef UG2_CAR_RENDER_MATERIAL_H
#define UG2_CAR_RENDER_MATERIAL_H
#include "CarRenderUploads.h"

// Render-consumer view, distinct from the shorter chunk-loader prefix. This
// records reads through +0x9c without changing the chunk's ownership or extent.
struct CarRenderMaterialPackView {
    unsigned char value00[0x30];
    float value30,value34[3],value40,value44[3];
    float value50,value54,value58,value5C,value60[3],value6C,value70[3];
    float value7C,value80,value84[3],value90,value94[3];
};
UG2_CHECK_OFFSET(CarRenderMaterialPackView, value50, 0x50);
UG2_CHECK_OFFSET(CarRenderMaterialPackView, value80, 0x80);
UG2_CHECK_OFFSET(CarRenderMaterialPackView, value94, 0x94);
UG2_CHECK_SIZE(CarRenderMaterialPackView, 0xa0);
struct CarRenderDeviceMaterial {
    float diffuse00[4],ambient10[4],specular20[4],emissive30[4];
    float power40;
};
UG2_CHECK_SIZE(CarRenderDeviceMaterial, 0x44);
// Original D3DLIGHT9 layout. UploadRecord leaves several fields indeterminate;
// do not invent zero stores for those fields or claim matching their bytes.
struct CarRenderDeviceLight {
    unsigned int type00;
    float diffuse04[4],specular14[4],ambient24[4],position34[3],direction40[3];
    float value4C[7];
};
UG2_CHECK_OFFSET(CarRenderDeviceLight, direction40, 0x40);
UG2_CHECK_SIZE(CarRenderDeviceLight, 0x68);
extern "C" {
extern CarRenderTexture *ug2RenderLastTexture870940;
extern CarRenderTechnique *ug2RenderLastTextureTechnique870944;
extern unsigned int ug2RenderTextureChanges872F10;
extern int ug2RenderUvDirty870948,ug2RenderUvEnable870CE8,ug2RenderClock86518C;
extern float ug2RenderUvOverride870924,ug2RenderUvOverride870928;
extern int ug2RenderUvMode800504,ug2RenderFlag870CB8;
extern float ug2RenderUvPeriod800508;
extern float ug2RenderRecordScale80050C,ug2RenderDiffuseScale800510,ug2RenderSpecularScale800514;
extern float ug2RenderPower80051C,ug2RenderShaderPower800B94;
int UG2RenderSetTexture(unsigned int device,unsigned int stage,unsigned int texture);
int UG2RenderSetLight(unsigned int device,unsigned int index,const CarRenderDeviceLight *light);
int UG2RenderLightEnable(unsigned int device,unsigned int index,unsigned int enabled);
int UG2RenderSetMaterial(unsigned int device,const CarRenderDeviceMaterial *material);
int UG2EffectSetVectorArray(unsigned int effect,unsigned int parameter,const float *values,unsigned int count);
}
void CarRenderSetAlphaTest(unsigned int enabled,unsigned int reference); // 0x5bcb70
void CarRenderSetBlend(unsigned int enabled,unsigned int source,unsigned int destination); // 0x5bcbc0
long double __stdcall CarRenderTextureWave(float time,float frequency,int mode,float period); // 0x48b450, RET 0x10
void CarRenderClampColour(float *value); // 0x5bd060
void CarRenderTransform4(float *output,const WheelMatrix *matrix,const float *input); // 0x486f50, thunk 0x43c9b0
#endif
