#include "EngineSoundShift.h"
extern float engineSoundValueOrigin784500;
extern float engineSoundShiftLow784550;
// 0x46ec70: reset inline, then swap the auxiliary's two RPM snapshots.
void EngineSoundShiftView::Upshift46EC70() {
    EngineSoundAuxiliary *aux = auxiliary6C;
    value13C = 0; value80 = value84 = value88 = value8C = value90 = value94 = 0;
    value78 = value7C = 0;
    if (aux->physics74->value70 < engineSoundShiftGate7885C0) return;
    value78 = value7C = 1;
    valueA8 = aux->value12C;
    const float previous = aux->value128;
    aux->value128 = aux->value12C; aux->value12C = previous;
    *reinterpret_cast<unsigned int *>(&valueA4) = *reinterpret_cast<unsigned int *>(&time5C);
    EngineSoundPhysicsView *physics = auxiliary6C->physics74;
    typedef float (__fastcall *Query)(EngineSoundPhysicsView *);
    const float throttle = (*reinterpret_cast<Query **>(physics))[0x24 / 4](physics);
    valueB4 = throttle; valueC0 = throttle; valueB0 = 0.1f;
    valueB8 = 0.0f; valueBC = 0; flagC4 = 0; valueAC = 0.0f;
    const double last = (double)valueA8 - tuning74->valueC;
    const float first = valueA8;
    const double duration = (double)tuning74->value10 * engineSoundSequenceMilliseconds78435C;
    valueCC = (float)duration;
    if (duration <= ginsuZero7D7D3C) valueCC = 0.01f;
    valueD8 = 0; valueD4 = (float)last; flagE0 = 0; valueC8 = 0.0f;
    valueD0 = first; valueDC = first;
    valueE8 = 0.001f; valueF0 = valueEC = 0.0f; valueF4 = 0; flagFC = 0; valueF8 = valueE4 = 0.0f;
    const double followLast = (double)valueA8 - engineSoundShiftOffset78AAA0;
    const float followFirst = valueA8;
    const int milliseconds = (int)((unsigned int)tuning74->value18 + (unsigned int)tuning74->value10);
    const double followDuration = (double)milliseconds * engineSoundSequenceMilliseconds78435C;
    value144 = (float)followDuration;
    if (followDuration <= ginsuZero7D7D3C) value144 = 0.01f;
    value150 = 6; value14C = (float)followLast; flag158 = 0; value140 = 0.0f;
    value148 = followFirst; value154 = followFirst;
    const float delay = tuning74->value8;
    flag98 = 1; value9C = 1; flag71 = 1;
    valueA0 = (float)((double)delay + time5C);
}
// 0x46ee60: reverse/neutral destination gear 1 leaves every field untouched.
void EngineSoundShiftView::Downshift46EE60() {
    EngineSoundAuxiliary *aux = auxiliary6C;
    if (aux->physics74->valueC8 == 1) return;
    value13C = 0; value80 = value84 = value88 = value8C = value90 = value94 = 0;
    value78 = value7C = 4;
    valueA8 = aux->value12C;
    const float previous = aux->value128;
    aux->value128 = aux->value12C; aux->value12C = previous;
    *reinterpret_cast<unsigned int *>(&valueA4) = *reinterpret_cast<unsigned int *>(&time5C);
    EngineSoundPhysicsView *physics = auxiliary6C->physics74;
    typedef float (__fastcall *Query)(EngineSoundPhysicsView *);
    const float throttle = (*reinterpret_cast<Query **>(physics))[0x24 / 4](physics);
    valueB4 = throttle; valueC0 = throttle; valueB0 = 0.05f;
    valueB8 = 0.0f; valueBC = 0; flagC4 = 0; valueAC = 0.0f;
    const double candidate = (double)valueA8 - tuning74->value44;
    const volatile float rounded = (float)candidate;
    float last = engineSoundShiftLow784550;
    if (candidate > engineSoundShiftLow784550) {
        last = rounded;
        if (engineSoundValueOrigin784500 < last) last = engineSoundValueOrigin784500;
    }
    const float first = valueA8;
    const double duration = (double)tuning74->value48 * engineSoundSequenceMilliseconds78435C;
    valueCC = (float)duration;
    if (duration <= ginsuZero7D7D3C) valueCC = 0.01f;
    valueD8 = 0; valueD4 = last; flagE0 = 0; valueC8 = 0.0f;
    valueD0 = first; valueDC = first;
    valueF0 = valueEC = 0.0f; valueF4 = 0; flagFC = 0; valueF8 = valueE4 = 0.0f; valueE8 = 0.001f;
    const float delay = tuning74->value40;
    value9C = 0; flag98 = 1; valueA0 = (float)((double)delay + time5C);
}
