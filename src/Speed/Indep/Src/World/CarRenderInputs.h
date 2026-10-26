#ifndef UG2_CAR_RENDER_INPUTS_H
#define UG2_CAR_RENDER_INPUTS_H

#include "CarRenderLightBuild.h"
#include "CarRenderLod.h"

extern "C" {
extern float ug2ReflectionTargetOffset803E50[3];
extern WheelVector ug2ReflectionUp800B70;
extern CarRenderLightProfile ug2CarLightProfile7F3F00;
}

// Fragments of 0x61ab50 and 0x5c3450. These are extracted caller fragments,
// not newly identified original functions. Target is the selected car, not camera.target60.
void CarRenderReflectionTarget(float *output, const float *carPosition); // 0x61ac46..0x61ac6c
void CarRenderReflectionBasis(WheelMatrix *output, WheelVector *eye, WheelVector *target); // 0x5c3524..0x5c3570
void CarRenderCapturePosition(float *output, const CarRenderFrame *frame); // 0x620f04..0x620f33, W store 0x620f3a
void CarRenderBuildSource(CarRenderRecord *output, const CarRenderLightProfile *profile,
                          const float *carPosition, const CarRenderView *view); // 0x620fcb..0x620ff8

#endif
