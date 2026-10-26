#include "CarRenderInputs.h"
#include "../Camera/CameraMatrix.h"

// Original 0x61ac46..0x61ac6c. Preserve operand order and binary32 stores.
void CarRenderReflectionTarget(float *output, const float *carPosition) {
    output[0] = static_cast<long double>(ug2ReflectionTargetOffset803E50[0]) + carPosition[0];
    output[1] = static_cast<long double>(carPosition[1]) + ug2ReflectionTargetOffset803E50[1];
    output[2] = static_cast<long double>(carPosition[2]) + ug2ReflectionTargetOffset803E50[2];
}

// Original 0x5c3524..0x5c3570, after the outgoing LookAt arguments were prepared.
// Global 0x871b50 is the original destination; output is explicit for verification.
void CarRenderReflectionBasis(WheelMatrix *output, WheelVector *eye, WheelVector *target) {
    WheelMatrix basis;
    CameraLookAt(&basis, eye, target, &ug2ReflectionUp800B70);
    basis.elements[3] = 0.0f;
    basis.elements[7] = 0.0f;
    basis.elements[11] = 0.0f;
    basis.elements[12] = 0.0f;
    basis.elements[13] = 0.0f;
    basis.elements[14] = 0.0f;
    basis.elements[15] = 1.0f;
    BodyMatrixCopy(output, &basis);
}

// Original 0x620f04..0x620f33 and 0x620f3a. Sequential integer copies also keep
// the original overlap behavior and float bit patterns; W is always one.
void CarRenderCapturePosition(float *output, const CarRenderFrame *frame) {
    unsigned int *destination = reinterpret_cast<unsigned int *>(output);
    const unsigned int *source = reinterpret_cast<const unsigned int *>(frame->position40);
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = 0x3f800000;
}

// Original 0x620fcb..0x620ff8. The original fifth builder argument is 0x871b50;
// the builder itself replaces it with view->frame40 when that pointer exists.
void CarRenderBuildSource(CarRenderRecord *output, const CarRenderLightProfile *profile,
                          const float *carPosition, const CarRenderView *view) {
    CarRenderClearRecord(output);
    CarRenderBuildLightRecord(output, profile, carPosition, 0, &ug2WheelReflectionBasis871B50, view);
}
