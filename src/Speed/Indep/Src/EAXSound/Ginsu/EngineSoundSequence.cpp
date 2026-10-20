#include "EngineSoundSequence.h"
#include "Ginsu.h"

// 0x468e30: leave the chosen extended result in ST0; no float store is added.
float EngineSoundApproach468E30(float current, float target, float step) {
    float result = current + step;
    if (!(target > result)) {
        result = current - step;
        if (!(target < result)) result = target;
    }
    return result;
}

// 0x4598f0: retain the equality-only capacity check and FILD integer product.
int EngineSoundSequence::Append4598F0(float first, float second, int duration, int kind) {
    if (count6C == 5) return -1;
    duration4[count6C] = (float)((double)duration * engineSoundSequenceMilliseconds78435C);
    if (duration4[count6C] <= ginsuZero7D7D3C) duration4[count6C] = 0.01f;
    second34[count6C] = second;
    first1C[count6C] = first;
    kinds54[count6C] = kind;
    flags4C[count6C] = 0;
    flag78 = 0;
    if (count6C == 0) value74 = first1C[0];
    return ++count6C;
}

// 0x46af20: six backing entries are cleared before appending the initial one.
void EngineSoundSequence::Construct46AF20(float first, float second, int duration) {
    value74 = 0.0f;
    value0 = 0.0f;
    index70 = 0;
    count6C = 0;
    for (int i = 0; i < 6; ++i) {
        first1C[i] = 0.0f;
        second34[i] = 0.0f;
        duration4[i] = 0.0f;
        flags4C[i] = 0;
        kinds54[i] = 0;
    }
    flag78 = 0;
    Append4598F0(first, second, duration, 0);
    value74 = first;
}
