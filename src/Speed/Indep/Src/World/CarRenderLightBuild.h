#ifndef UG2_CAR_RENDER_LIGHT_BUILD_H
#define UG2_CAR_RENDER_LIGHT_BUILD_H

#include "CarRenderRecord.h"
#include "CarRenderWheels.h"

// Prefixes read by 0x48ed60; names describe only the observed use in that routine.
struct CarRenderProfileLight {
    unsigned int flag00;
    float angle04;
    float angle08;
    float color0C[3];
    float intensity18;
};
UG2_CHECK_SIZE(CarRenderProfileLight, 0x1c);
struct CarRenderLightProfile {
    unsigned char pad00[0x20];
    CarRenderProfileLight lights20[3];
};
UG2_CHECK_OFFSET(CarRenderLightProfile, lights20, 0x20);

struct CarRenderLight {
    unsigned int value00;
    unsigned char kind04;
    unsigned char pad05[7];
    unsigned int color0C;
    float position10[3];
    unsigned int value1C;
    float direction20[3];
    unsigned char pad2C[8];
    float angle34;
};
UG2_CHECK_OFFSET(CarRenderLight, color0C, 0xc);
UG2_CHECK_OFFSET(CarRenderLight, position10, 0x10);
UG2_CHECK_OFFSET(CarRenderLight, direction20, 0x20);
UG2_CHECK_OFFSET(CarRenderLight, angle34, 0x34);

struct CarRenderLightSelection {
    CarRenderLight *lights00[24];
    float weights60[24];
    float valueC0[4];
};
UG2_CHECK_OFFSET(CarRenderLightSelection, weights60, 0x60);
UG2_CHECK_OFFSET(CarRenderLightSelection, valueC0, 0xc0);
UG2_CHECK_SIZE(CarRenderLightSelection, 0xd0);

// Original 0x48e3d0, still unrecovered. Verification executes a controlled callee
// fixture at this boundary; no native implementation or activation is supplied.
int CarRenderSelectLights(CarRenderLightSelection *output, const float *position);
extern "C" {
extern int ug2LightNormalize82DAE4;
extern float ug2LightTarget7F39B8;
extern int ug2LightNormalize7F39BC;
extern float ug2LightTarget7F39C0;
}

// Original 0x48ed60 (cdecl): builds the source record used by car/wheel drawing.
// Only the observed matrix00/matrix40 cells and value120 are written.
unsigned int CarRenderBuildLightRecord(CarRenderRecord *output, const CarRenderLightProfile *profile,
                                      const float *position, WheelMatrix *matrixArg4,
                                      WheelMatrix *matrixArg5, const CarRenderView *view);

#endif
