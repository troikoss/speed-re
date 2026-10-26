#ifndef UG2_CAR_RENDER_UPLOADS_H
#define UG2_CAR_RENDER_UPLOADS_H
#include "CarRenderConsume.h"

// Observed prefixes used by 0x60f510; surrounding ownership remains unidentified.
struct CarRenderHeadlightValue {
    unsigned char value00[0x380];
    unsigned int value380;
};
struct CarRenderHeadlightOwner {
    unsigned char value00[0x554];
    CarRenderHeadlightValue *value554;
};
struct CarRenderHeadlightContext {
    unsigned int value00;
    CarRenderHeadlightOwner *value04;
};
UG2_CHECK_OFFSET(CarRenderHeadlightOwner, value554, 0x554);
UG2_CHECK_OFFSET(CarRenderHeadlightValue, value380, 0x380);
UG2_CHECK_SIZE(CarRenderHeadlightOwner, 0x558);
UG2_CHECK_SIZE(CarRenderHeadlightValue, 0x384);
UG2_CHECK_SIZE(CarRenderHeadlightContext, 8);
extern "C" {
extern WheelMatrix *ug2RenderLastWorld876850;
extern CarRenderTechnique *ug2RenderLastTechnique87684C;
extern int ug2RenderLastView800534;
extern unsigned int ug2RenderWorldCalls872EF4,ug2RenderWorldChanges872EF8,ug2RenderViewChanges872EFC,ug2RenderIdentityCalls872F00;
extern WheelMatrix ug2RenderIdentity872E90;
extern float ug2RenderHeadlightBias80052C,ug2RenderHeadlightBias800530,ug2RenderClipScale79BFC0;
extern CarRenderHeadlightContext *ug2RenderHeadlightContext8900B4;
extern unsigned int ug2RenderTexture870894,ug2RenderTexture870858,ug2RenderTexture87088C;
extern int ug2RenderMode8654A4;
extern float ug2RenderColour800520[3],ug2RenderReflectionMinimum7A27E4;
extern const char ug2RenderHeadlights7A3658[];
int UG2RenderSetTransform(unsigned int device,unsigned int state,const WheelMatrix *matrix);
int UG2RenderSetTextureStageState(unsigned int device,unsigned int stage,unsigned int state,unsigned int value);
int UG2EffectSetMatrix(unsigned int effect,unsigned int parameter,const WheelMatrix *matrix);
int UG2EffectSetMatrixArray(unsigned int effect,unsigned int parameter,const WheelMatrix *matrices,unsigned int count);
int UG2EffectSetVector(unsigned int effect,unsigned int parameter,const float *value);
}
unsigned int CarRenderHeadlightColour(CarRenderHeadlightOwner *object); // 0x60f510
#endif
