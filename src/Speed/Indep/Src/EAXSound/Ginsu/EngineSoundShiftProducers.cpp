#include "EngineSoundShift.h"
// 0x45d300: original Physics virtual +0x24.
float EngineSoundPhysicsView::Value45D300() { return value84; }
// 0x401090: unordered comparisons keep the lower endpoint.
float EngineSoundClamp401090(float value, float low, float high) {
    if (low < value) low = value;
    if (high < low) return high;
    return low;
}
// 0x47bf20: the follow envelope runs before the state-specific RPM envelope.
void EngineSoundShiftView::Advance47BF20(float duration) {
    if (!flag158) {
        const float current = auxiliary6C->physics74->value70;
        value14C = current;
        reinterpret_cast<EngineSoundEnvelope *>(&value140)->Step46B100(duration);
        if (flag158) value154 = current;
    }
    switch (value78) {
    case 1: case 4: case 5: case 6:
        reinterpret_cast<EngineSoundEnvelope *>(&valueC8)->Step46B100(duration); break;
    case 2: case 3: case 7: {
        const float current = auxiliary6C->physics74->value70;
        valueD4 = current;
        reinterpret_cast<EngineSoundEnvelope *>(&valueC8)->Step46B100(duration);
        if (flagE0) valueDC = current;
        break;
    }
    }
}
// 0x47bea0: the virtual result is rounded before the actual envelope step.
void EngineSoundShiftView::Advance47BEA0(float duration) {
    switch (value78) {
    case 1: case 4: case 5: case 6:
        reinterpret_cast<EngineSoundEnvelope *>(&valueAC)->Step46B100(duration); break;
    case 2: case 3: case 7: {
        EngineSoundPhysicsView *physics = auxiliary6C->physics74;
        typedef float (__fastcall *Query)(EngineSoundPhysicsView *);
        Query *table = *reinterpret_cast<Query **>(physics);
        const float current = table[0x24 / 4](physics);
        valueB8 = current;
        reinterpret_cast<EngineSoundEnvelope *>(&valueAC)->Step46B100(duration);
        if (flagC4) valueC0 = current;
        break;
    }
    }
}
// 0x46f000: completion of the first effect resets the entire shifting state.
void EngineSoundShiftView::Advance46F000(float duration) {
    if (!value13C) return;
    reinterpret_cast<EngineSoundEnvelope *>(&value104)->Step46B100(duration);
    reinterpret_cast<EngineSoundEnvelope *>(&value120)->Step46B100(duration);
    if (flag11C) { Reset45C430(); return; }
    value90 = (int)value118;
    EngineSoundShiftTuning *tuning = tuning74;
    value94 = tuning->value34;
    value80 = (int)value134;
    value84 = tuning->value28;
}
// 0x46f080: exact integer loads and extended duration comparisons.
void EngineSoundShiftView::Configure46F080() {
    const int gear = auxiliary6C->physics74->valueC8;
    if (gear >= 5) return;
    float scale = ginsuOne784250;
    if (gear == 3) scale = engineSoundShiftScale7885C4;
    if (gear == 4) scale = engineSoundShiftScale7885C8;
    value13C = 1;
    const double first = (double)scale * tuning74->value30;
    const double duration = (double)tuning74->value38 * engineSoundSequenceMilliseconds78435C;
    value108 = (float)duration;
    if (duration <= ginsuZero7D7D3C) value108 = 0.01f;
    value10C = (float)first; value118 = (float)first;
    value110 = 0.0f; value114 = 0; flag11C = 0; value104 = 0.0f;
    const double second = tuning74->value24;
    const double otherDuration = (double)tuning74->value2C * engineSoundSequenceMilliseconds78435C;
    value124 = (float)otherDuration;
    if (otherDuration <= ginsuZero7D7D3C) value124 = 0.01f;
    value128 = (float)second; value12C = 0.0f; value134 = (float)second;
    value130 = 0; flag138 = 0; value120 = 0.0f;
}
