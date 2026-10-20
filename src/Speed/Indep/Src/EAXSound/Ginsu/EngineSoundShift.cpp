#include "EngineSoundShift.h"
extern float engineSoundShiftTimeScale7844D8;
extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)
// 0x482d50: shared updates execute before the state-specific transition.
void EngineSoundShiftView::Advance482D50(float duration) {
    if (time5C > valueA0 && flag98) { flag70 = 1; flag98 = 0; }
    value7C = 0;
    if (!value78) return;
    Advance47BF20(duration);
    Advance47BEA0(duration);
    reinterpret_cast<EngineSoundEnvelope *>(&valueE4)->Step46B100(duration);
    Advance46F000(duration);
    EngineSoundEnvelope *rpm = reinterpret_cast<EngineSoundEnvelope *>(&valueC8);
    EngineSoundEnvelope *throttle = reinterpret_cast<EngineSoundEnvelope *>(&valueAC);
    switch (value78) {
    case 1:
        if (flagE0) {
            value78 = value7C = 2;
            const int milliseconds = tuning74->value18;
            const float second = auxiliary6C->physics74->value70;
            const float first = EngineSoundClamp401090((float)((double)tuning74->value14 + second), 1000.0f, 10000.0f);
            rpm->Configure459990(first, second, milliseconds, 0);
            int index = (int)((unsigned int)car68->value50 - 2u);
            if (index < 1) index = 0;
            else if (index > 3) index = 3;
            EngineSoundPhysicsView *physics = auxiliary6C->physics74;
            typedef float (__fastcall *Query)(EngineSoundPhysicsView *);
            const int throttleTime = engineSoundShiftDuration7885B0[index];
            const float last = (*reinterpret_cast<Query **>(physics))[0x24 / 4](physics);
            const float begin = (float)((double)(*reinterpret_cast<Query **>(physics))[0x24 / 4](physics) * engineSoundShiftRpmScale7885A0[index]);
            throttle->Configure459990(begin, last, throttleTime, 0);
            reinterpret_cast<EngineSoundEnvelope *>(&valueE4)->Configure459990(tuning74->value20, 0.0f, tuning74->value1C, 0);
            Configure46F080();
            Advance46F000(duration);
            flag72 = 1;
        }
        break;
    case 2: if (flagE0) value78 = value7C = 3; break;
    case 3:
        if ((flag11C && flag138) || !value13C) Reset45C430();
        break;
    case 4:
        if (flagE0) {
            EngineSoundShiftTuning *tuning = tuning74;
            EngineSoundAuxiliary *aux = auxiliary6C;
            value78 = value7C = 5;
            int milliseconds = tuning->value50;
            EngineSoundPhysicsView *physics = aux->physics74;
            if (physics->valueC8 < 3) milliseconds = (int)((double)milliseconds * engineSoundShiftTimeScale7844D8);
            const float last = EngineSoundClamp401090((float)((double)physics->value70 + tuning->value4C), 1000.0f, 10000.0f);
            const float first = aux->vtable0->value34(aux);
            rpm->Configure459990(first, last, milliseconds, 6);
            const float end = engineSoundShiftFirst7ED29C;
            const int throttleTime = engineSoundShiftTime7ED2A0;
            const float begin = auxiliary6C->vtable0->value38(auxiliary6C);
            throttle->Configure459990(begin, end, throttleTime, 0);
        }
        break;
    case 5:
        if (flagE0) {
            value78 = value7C = 6;
            int milliseconds = tuning74->value58;
            if (auxiliary6C->physics74->valueC8 < 3 && value64 < 2)
                milliseconds = (int)((double)milliseconds * engineSoundShiftTimeScale7844D8);
            EngineSoundAuxiliary *aux = auxiliary6C;
            const float last = (float)((double)aux->vtable0->value34(aux) - tuning74->value54);
            const float first = aux->vtable0->value34(aux);
            rpm->Configure459990(first, last, milliseconds, 0);
            throttle->Configure459990(engineSoundShiftFirst7ED29C, 0.0f, engineSoundShiftTime7ED2A0, 0);
        }
        break;
    case 6:
        if (flagE0) {
            value78 = value7C = 7;
            const float previous = auxiliary6C->physics74->value70;
            const double difference = (double)auxiliary6C->vtable0->value34(auxiliary6C) - previous;
            EngineSoundAuxiliary *aux = auxiliary6C;
            const float last = aux->physics74->value70;
            const float scaled = (float)fabs(difference * tuning74->value5C);
            const int milliseconds = (int)EngineSoundClamp401090(scaled, 0.0f, 800.0f);
            const float first = aux->vtable0->value34(aux);
            rpm->Configure459990(first, last, milliseconds, 6);
            EngineSoundPhysicsView *physics = auxiliary6C->physics74;
            typedef float (__fastcall *Query)(EngineSoundPhysicsView *);
            const float end = (*reinterpret_cast<Query **>(physics))[0x24 / 4](physics);
            throttle->Configure459990(0.0f, end, 60, 0);
        }
        break;
    case 7: if (flagE0) Reset45C430(); break;
    }
}
// 0x483130: frame flags clear even while the component clock is inactive.
void EngineSoundShiftView::Update483130(float duration) {
    if (active58) { delta60 = duration; time5C = (float)((double)duration + time5C); }
    flag70 = 0; flag71 = 0; flag72 = 0;
    EngineSoundPhysicsView *physics = auxiliary6C->physics74;
    const int previous = physics->valueCC, current = physics->valueC8;
    if (previous < current) Upshift46EC70();
    else if (current < previous) Downshift46EE60();
    Advance482D50(duration);
}
