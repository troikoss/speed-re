#ifndef UG2_ENGINE_SOUND_ENVELOPE_H
#define UG2_ENGINE_SOUND_ENVELOPE_H
#include "../../Physics/Ug2Layout.h"

// UG2 0x459990/0x46b100 time envelope. This differs from the Physics
// interpolation helper at 0x46b240 despite the identical object size.
struct EngineSoundEnvelope {
    float time0, duration4, first8, secondC;
    int kind10;
    float value14;
    unsigned char flag18;
    char unidentified19[3];
    void Configure459990(float first, float second, int milliseconds, int kind);
    void Step46B100(float duration);
};
UG2_CHECK_SIZE(EngineSoundEnvelope, 0x1c);
int EngineSoundCurve468160(unsigned int kind, unsigned int value, unsigned char mode);
int EngineSoundCurveInverse467FF0(int value);
int EngineSoundCurveConvert468050(unsigned int value);
extern int engineSoundCurveTable7EF7F8[];
extern int engineSoundCurveTable7EE7F0[];
extern int engineSoundCurveScratch82BC88;
extern float engineSoundCurveInteger789CF4;
#endif
