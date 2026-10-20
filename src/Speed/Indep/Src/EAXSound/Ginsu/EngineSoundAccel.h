#ifndef UG2_ENGINE_SOUND_ACCEL_H
#define UG2_ENGINE_SOUND_ACCEL_H
#include "EngineSound.h"
#include "EngineSoundEnvelope.h"
struct EngineSoundAccelTuning {
    float value0;
    int value4, value8, valueC, value10;
};
UG2_CHECK_SIZE(EngineSoundAccelTuning, 0x14);
extern EngineSoundAccelTuning engineSoundAccelTuning7F3248[], engineSoundAccelTuning7F3284, engineSoundAccelTuning7F32C0;
extern float engineSoundAccelRise788614, engineSoundAccelLevel788610, engineSoundAccelDelta788618;
extern float engineSoundAccelSpeed787AF4, engineSoundAccelLow789FDC, engineSoundAccelHigh789FE0;
extern float engineSoundAccelRpmMargin78429C, engineSoundAccelRpmLow78759C, engineSoundAccelDelay7844C0;
EngineSoundAccelView *CreateEngineSoundAccel46FFA0();
#endif
