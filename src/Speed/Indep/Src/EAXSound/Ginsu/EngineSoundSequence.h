#ifndef UG2_ENGINE_SOUND_SEQUENCE_H
#define UG2_ENGINE_SOUND_SEQUENCE_H
#include "../../Physics/Ug2Layout.h"

// Explicit layout at SFXCTL_Engine +0x154; capacity check in Append is == 5.
struct EngineSoundSequence {
    float value0;
    float duration4[6];
    float first1C[6];
    float second34[6];
    unsigned char flags4C[6];
    char unidentified52[2];
    int kinds54[6];
    int count6C;
    int index70;
    float value74;
    unsigned char flag78;
    char unidentified79[3];
    void Construct46AF20(float first, float second, int duration);
    int Append4598F0(float first, float second, int duration, int kind);
};
UG2_CHECK_SIZE(EngineSoundSequence, 0x7c);
UG2_CHECK_OFFSET(EngineSoundSequence, count6C, 0x6c);
float EngineSoundApproach468E30(float current, float target, float step);
extern float engineSoundSequenceMilliseconds78435C;
#endif
