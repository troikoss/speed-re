#include "EngineSoundSpatial.h"
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
extern float ginsuZero7D7D3C;

// 0x401150: extended planar deltas and unspilled square root result.
float EngineSoundDistance2D401150(float *first, float *second) {
    const double x = (double)first[0] - second[0];
    const double y = (double)first[1] - second[1];
    return sqrt(x * x + y * y);
}

// 0x45de60: rounded delta/normal, then rounded dot argument. The cross
// comparison remains extended; only the low angle word is overwritten.
void EngineSoundSpatialComponent::Angle45DE60() {
    float delta[2], normal[2];
    delta[0] = snapshot78[0] - engineSoundSpatialPoint82BB7C[0];
    delta[1] = snapshot78[1] - engineSoundSpatialPoint82BB7C[1];
    Normalize2(normal, delta);
    volatile float dot = (float)((double)normal[1] * engineSoundSpatialDirection82BB78[1] +
                                (double)normal[0] * engineSoundSpatialDirection82BB78[0]);
    engineSoundSpatialAngle82BB80 = (unsigned short)(0x4000 - HudArcSineUnits(dot));
    const double cross = (double)normal[0] * engineSoundSpatialDirection82BB78[1] -
                         (double)normal[1] * engineSoundSpatialDirection82BB78[0];
    if (cross < ginsuZero7D7D3C) engineSoundSpatialAngle82BB80 = (unsigned short)(0xffff - engineSoundSpatialAngle82BB80);
}
