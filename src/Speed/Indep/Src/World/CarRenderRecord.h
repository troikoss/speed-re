#ifndef UG2_CAR_RENDER_RECORD_H
#define UG2_CAR_RENDER_RECORD_H

#include "../Physics/WheelMath.h"
#include "../Physics/Ug2Layout.h"

// PC v1.2 extra render record. Names remain offset-based pending consumer recovery.
struct CarRenderRecord {
    WheelMatrix matrix00;
    WheelMatrix matrix40;
    WheelMatrix matrix80;
    float valueC0[4];
    unsigned int valueD0[4];
    WheelMatrix matrixE0;
    unsigned int value120[40];

    // Original 0x48a3b0 (__thiscall, RET 4), returns this.
    CarRenderRecord *CopyFrom(CarRenderRecord *source);
    // Original 0x489ba0 (__thiscall, RET 4): transforms the first ten four-lane groups.
    void TransformCoefficients(WheelMatrix *matrix);
};
UG2_CHECK_OFFSET(CarRenderRecord, matrix40, 0x40);
UG2_CHECK_OFFSET(CarRenderRecord, matrix80, 0x80);
UG2_CHECK_OFFSET(CarRenderRecord, valueC0, 0xc0);
UG2_CHECK_OFFSET(CarRenderRecord, valueD0, 0xd0);
UG2_CHECK_OFFSET(CarRenderRecord, matrixE0, 0xe0);
UG2_CHECK_OFFSET(CarRenderRecord, value120, 0x120);
UG2_CHECK_SIZE(CarRenderRecord, 0x1c0);

// Original 0x5bb190: D3DX transpose followed by seven zero stores, returns output.
WheelMatrix *CarRenderDirectionMatrix(WheelMatrix *output, WheelMatrix *input);
// Original 0x43d010: length uses XYZW; only output XYZ is written.
void CarRenderNormalize4(float *output, const float *input);

#endif
