#include "EngineSoundSequence.h"
#include "EngineSoundEnvelope.h"
#include "Ginsu.h"
extern float engineGainTwoScale78A6C8;

// 0x4598c0: reset without appending an initial segment.
void EngineSoundSequence::Reset4598C0() {
    value74 = value0 = 0.0f;
    index70 = count6C = 0;
    for (int i = 0; i < 6; ++i) {
        first1C[i] = second34[i] = duration4[i] = 0.0f;
        flags4C[i] = 0;
        kinds54[i] = 0;
    }
    flag78 = 0;
}

// 0x46af70: consume at most one segment, retaining excess elapsed time.
void EngineSoundSequence::Step46AF70(float duration) {
    if (flag78 || count6C == 0) return;
    const int index = index70;
    volatile float elapsed = duration + value0;
    value0 = elapsed;
    if (elapsed > duration4[index]) {
        *reinterpret_cast<unsigned int *>(&value74) = *reinterpret_cast<unsigned int *>(&second34[index]);
        if (index >= (int)((unsigned int)count6C - 1u)) { flag78 = 1; return; }
        index70 = (int)((unsigned int)index + 1u);
        value0 = elapsed - duration4[index];
        if (flags4C[index70]) {
            *reinterpret_cast<unsigned int *>(&first1C[index70]) = *reinterpret_cast<unsigned int *>(&second34[index]);
        }
        return;
    }
    switch (kinds54[index]) {
    case 1: break;
    case 2: {
        const double fraction = (double)elapsed / duration4[index];
        value74 = (float)((((double)second34[index] - first1C[index]) * fraction) * fraction + first1C[index]);
        break;
    }
    case 3: {
        const double fraction = ((double)ginsuOne784250 / duration4[index]) * elapsed - ginsuOne784250;
        value74 = (float)(((double)second34[index] - first1C[index]) *
                          (ginsuOne784250 - fraction * fraction) + first1C[index]);
        break;
    }
    case 6: {
        const double difference = (double)second34[index] - first1C[index];
        int position = (int)(((double)elapsed / duration4[index]) * engineSoundCurveInteger789CF4);
        if (position < 1) position = 0;
        else if (position > 0x7fff) position = 0x7fff;
        const int curve = EngineSoundCurve468160(2, 0x7fff - position, 0);
        value74 = (float)(((double)curve * engineGainTwoScale78A6C8) * difference + first1C[index70]);
        break;
    }
    default:
        value74 = (float)(((double)elapsed / duration4[index]) *
                          ((double)second34[index] - first1C[index]) + first1C[index]);
        break;
    }
}
