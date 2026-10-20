#include "EngineSoundEnvelope.h"
#include "EngineSoundSequence.h"
#include "Ginsu.h"
extern float engineGainTwoScale78A6C8;

// 0x47aaa0: retain the target word before and after the actual envelope step.
void EngineSoundEnvelope::StepTarget47AAA0(float duration, float target) {
    *reinterpret_cast<unsigned int *>(&secondC) = *reinterpret_cast<unsigned int *>(&target);
    Step46B100(duration);
    if (flag18) *reinterpret_cast<unsigned int *>(&value14) = *reinterpret_cast<unsigned int *>(&target);
}

// 0x459990: compare the unrounded duration product after storing a copy.
void EngineSoundEnvelope::Configure459990(float first, float second, int milliseconds, int kind) {
    const double raw = (double)milliseconds * engineSoundSequenceMilliseconds78435C;
    duration4 = (float)raw;
    if (raw <= ginsuZero7D7D3C) duration4 = 0.01f;
    secondC = second;
    first8 = first;
    kind10 = kind;
    flag18 = 0;
    value14 = first;
    time0 = 0.0f;
}

// 0x46b100: time is rounded before completion testing; kind 1 holds value14.
void EngineSoundEnvelope::Step46B100(float duration) {
    if (flag18) return;
    volatile float elapsed = duration + time0;
    time0 = elapsed;
    if (elapsed > duration4) {
        *reinterpret_cast<unsigned int *>(&value14) = *reinterpret_cast<unsigned int *>(&secondC);
        flag18 = 1;
        return;
    }
    switch (kind10) {
    case 1: break;
    case 2: {
        const double fraction = (double)elapsed / duration4;
        value14 = (float)((((double)secondC - first8) * fraction) * fraction + first8);
        break;
    }
    case 3: {
        const double fraction = ((double)ginsuOne784250 / duration4) * elapsed - ginsuOne784250;
        value14 = (float)(((double)secondC - first8) * (ginsuOne784250 - fraction * fraction) + first8);
        break;
    }
    case 6: {
        const double difference = (double)secondC - first8;
        int position = (int)(((double)elapsed / duration4) * engineSoundCurveInteger789CF4);
        if (position < 1) position = 0;
        else if (position > 0x7fff) position = 0x7fff;
        const int curve = EngineSoundCurve468160(2, 0x7fff - position, 0);
        value14 = (float)(((double)curve * engineGainTwoScale78A6C8) * difference + first8);
        break;
    }
    default:
        value14 = (float)(((double)elapsed / duration4) * ((double)secondC - first8) + first8);
        break;
    }
}
