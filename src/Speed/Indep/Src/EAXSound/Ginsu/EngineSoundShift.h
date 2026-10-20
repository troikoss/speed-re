#ifndef UG2_ENGINE_SOUND_SHIFT_H
#define UG2_ENGINE_SOUND_SHIFT_H
#include "EngineSound.h"
#include "EngineSoundEnvelope.h"
struct EngineSoundShiftTuning {
    const char *file0; int value4; float value8;
    int valueC, value10, value14, value18, value1C;
    float value20;
    int value24, value28, value2C, value30, value34, value38, value3C;
    float value40;
    int value44, value48, value4C, value50, value54, value58;
    float value5C;
};
UG2_CHECK_SIZE(EngineSoundShiftTuning, 0x60);
extern EngineSoundShiftTuning engineSoundShiftTuning7F2860[], engineSoundShiftTuning7F2EC0;
extern int engineSoundShiftSelection7F2234[], engineSoundShiftPacked7ED344;
extern float engineSoundShiftGate7885C0, engineSoundShiftScale7885C4, engineSoundShiftScale7885C8;
extern float engineSoundShiftOffset78AAA0, engineSoundShiftRpmScale7885A0[];
extern int engineSoundShiftDuration7885B0[];
extern float engineSoundShiftFirst7ED29C;
extern int engineSoundShiftTime7ED2A0;
EngineSoundShiftView *CreateEngineSoundShift46EBD0();
float EngineSoundClamp401090(float value, float low, float high);
#endif
