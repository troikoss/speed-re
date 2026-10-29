#ifndef UG2_CAR_WHEEL_SECONDARY_H
#define UG2_CAR_WHEEL_SECONDARY_H
#include "CarRenderWheels.h"

// Original 0x5bb1c0 initialization. Position fourth words and trailing bytes
// are not written; preserve that distinction from the XYZ/UV/colour payload.
// This is the observed prefix; the total retail object size is unverified.
struct CarRenderQuad {
    float position00[4][4];
    float uv40[4][2];
    unsigned int colour60[4];
    unsigned char value70;
    unsigned char value71[3];
    void Initialize();
};
UG2_CHECK_OFFSET(CarRenderQuad,uv40,0x40);
UG2_CHECK_OFFSET(CarRenderQuad,colour60,0x60);
UG2_CHECK_OFFSET(CarRenderQuad,value70,0x70);
UG2_CHECK_SIZE(CarRenderQuad,0x74);

// Logical flag fragment 0x622eee..0x622f06 of the original caller. It is
// separate from the earlier geometry count occupying the same stack slot.
unsigned int CarWheelCallerFlags(int gate);
// Original 0x5c5580: quad packing/render submission. This remains an explicit
// native backend boundary until the corresponding renderer is integrated.
void CarRenderSubmitQuad(CarRenderQuad *,CarRenderTexture *,WheelMatrix *,unsigned int);
extern "C" {
extern const volatile float ug2WheelSecondaryScale7A889C,ug2WheelSecondaryScale7A6728,ug2WheelSecondaryY7A672C;
extern const char ug2WheelSecondaryTexture7A88A0[];
}
#endif
