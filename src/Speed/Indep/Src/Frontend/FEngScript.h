#ifndef UG2_FENG_SCRIPT_H
#define UG2_FENG_SCRIPT_H
#include "../Hud/HudVisibility.h"

struct FEngReference {
    char unidentified0[4];
    unsigned char value4;
    char unidentified5[3];
    void *value8;
    void *Resolve(); // 0x4fe890
};
struct FEngQuaternion {
    float value0[4];
    FEngQuaternion *Multiply(FEngQuaternion *out, const FEngQuaternion *other); // 0x4fe6b0
};
struct FEngInteger4 {
    unsigned int value0[4];
    FEngInteger4 *Subtract(FEngInteger4 *out, const FEngInteger4 *other); // 0x505580
};
UG2_CHECK_OFFSET(FEngReference, value4, 4);
UG2_CHECK_OFFSET(FEngReference, value8, 8);
UG2_CHECK_SIZE(FEngQuaternion, 16);
UG2_CHECK_SIZE(FEngInteger4, 16);
UG2_CHECK_OFFSET(FEScriptTrack, value2C, 0x2c);
UG2_CHECK_OFFSET(FEScriptTrack, value30, 0x30);
#endif
