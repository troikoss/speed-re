#include "EngineSoundAccel.h"
#include "EngineSoundPhysics.h"
#include "EngineSoundSchedule.h"
#include "EngineSoundSequence.h"
#include "../../Physics/VehicleMover.h"

// 0x45cd30: unordered values pass the x87 rejection tests.
unsigned char EngineSoundAccelView::Query45CD30() {
    EngineSoundCar *car = car68;
    if ((double)*reinterpret_cast<float *>(reinterpret_cast<char *>(car->car30) + 0x430) * engineSoundSpeed78426C > engineSoundAccelSpeed787AF4) return 0;
    if ((double)car->value54 - auxiliary6C->physics74->value90 < engineSoundAccelDelta788618) return 0;
    if (car->value40->flagD8 || valueC8) return 0;
    if (shift70 && shift70->IsActive45C090()) return 0;
    return car->value50 == 2 && engineSoundRaceActive89E7B0 != 8 && !(car->value48 > engineSoundAccelLow789FDC);
}
// 0x470030: retain the RPM snapshot across the actual owner query.
unsigned char EngineSoundAccelView::Query470030() {
    EngineSoundCar *car = car68;
    if ((double)car->value54 - auxiliary6C->physics74->value90 < engineSoundAccelDelta788618 || valueC8) return 0;
    if (shift70 && shift70->value78 != 0) return 0;
    const float rpm = car->value48;
    if ((double)state44->car30->AudioShiftPoint5ECD30(car->value50) - engineSoundAccelRpmMargin78429C < rpm) return 0;
    car = car68;
    if (car->value48 < engineSoundAccelRpmLow78759C) return 0;
    if ((double)time5C - valueCC < engineSoundAccelDelay7844C0) return 0;
    return state44->car30->context14->value4 == 2 || car->value50 >= 3;
}
// 0x45cdc0.
unsigned char EngineSoundAccelView::Query45CDC0() {
    if (valueC8 || (shift70 && shift70->value78 != 0)) return 0;
    if (car68->value48 < engineSoundAccelHigh789FE0) return 0;
    return car68->value50 >= 3;
}

// 0x45cb70: explicit envelope stores are not calls to Configure459990.
void EngineSoundAccelView::Configure45CB70() {
    const float first = car68->value48;
    volatile float last = engineSoundAccelRise788614 + car68->value48;
    value78 = 0.5f; value84 = 6; value80 = first; flag8C = 0;
    value7C = last; value74 = 0.0f; value88 = last;
    valueC8 = 1;
    value98 = engineSoundAccelLevel788610; valueA4 = engineSoundAccelLevel788610;
    value94 = 0.2f; value9C = 0.0f; valueA0 = 0; flagA8 = 0; value90 = 0.0f;
    valueB0 = 0.01f; valueB8 = 100.0f; valueB4 = 100.0f; valueBC = 0;
    flagC4 = 0; valueC0 = 100.0f; valueAC = 0.0f;
    *reinterpret_cast<unsigned int *>(&valueCC) = *reinterpret_cast<unsigned int *>(&time5C);
}
// 0x45cc30: compare extended duration products after storing rounded copies.
void EngineSoundAccelView::Configure45CC30() {
    *reinterpret_cast<unsigned int *>(&valueCC) = *reinterpret_cast<unsigned int *>(&time5C);
    valueC8 = 2;
    const float first = car68->value48;
    const double last = (double)tuningD4->value4 + car68->value48;
    const double duration = (double)tuningD4->value8 * engineSoundSequenceMilliseconds78435C;
    value78 = (float)duration;
    if (duration <= ginsuZero7D7D3C) value78 = 0.01f;
    value80 = (float)last; value84 = 6;
    value7C = first; flag8C = 0; value88 = first; value74 = 0.0f;
    const float level = tuningD4->value0;
    const double levelDuration = (double)tuningD4->value8 * engineSoundSequenceMilliseconds78435C;
    value94 = (float)levelDuration;
    if (levelDuration <= ginsuZero7D7D3C) value94 = 0.01f;
    value9C = level; value98 = 0.0f; valueA0 = 0; flagA8 = 0; valueA4 = 0.0f; value90 = 0.0f;
    const float throttle = car68->value44;
    const float throttleAgain = car68->value44;
    valueB0 = 0.01f; valueBC = 0; valueB8 = throttle; flagC4 = 0;
    valueAC = 0.0f; valueB4 = throttleAgain; valueC0 = throttleAgain;
}

// 0x47c1a0.
void EngineSoundAccelView::Advance47C1A0(float duration) {
    EngineSoundEnvelope *rpm = reinterpret_cast<EngineSoundEnvelope *>(&value74);
    switch (valueC8) {
    case 1: case 3: {
        const float current = car68->value48;
        value80 = current;
        rpm->Step46B100(duration);
        if (flag8C) value88 = current;
        break;
    }
    case 2: rpm->Step46B100(duration); break;
    case 4: rpm->StepTarget47AAA0(duration, car68->value48); break;
    }
}
// 0x47c220.
void EngineSoundAccelView::Advance47C220(float duration) {
    if (valueC8 > 0 && valueC8 < 5) {
        const float current = car68->value44;
        valueB8 = current;
        EngineSoundEnvelope *throttle = reinterpret_cast<EngineSoundEnvelope *>(&valueAC);
        throttle->Step46B100(duration);
        if (flagC4) valueC0 = current;
        if (flagC4) {
            const float value = car68->value44;
            throttle->Configure459990(value, value, 0, 0);
        }
    }
}
// 0x47c290: shared envelope stepping precedes state transitions.
void EngineSoundAccelView::Advance47C290(float duration) {
    EngineSoundEnvelope *level = reinterpret_cast<EngineSoundEnvelope *>(&value90);
    EngineSoundEnvelope *rpm = reinterpret_cast<EngineSoundEnvelope *>(&value74);
    level->Step46B100(duration);
    Advance47C1A0(duration);
    Advance47C220(duration);
    if (!valueC8) return;
    if (!car68->flag4C && valueC8 != 4) {
        const float current = car68->value48;
        const int milliseconds = tuningD4->value10;
        const float first = auxiliary6C->vtable0->value34(auxiliary6C);
        rpm->Configure459990(first, current, milliseconds, 6);
        valueC8 = 4;
        level->Configure459990(valueA4, 0.0f, tuningD4->value10, 0);
    }
    switch (valueC8) {
    case 2:
        if (flag8C) {
            valueC8 = 3;
            rpm->Configure459990(value88, car68->value48, tuningD4->valueC, 0);
            level->Configure459990(valueA4, 0.0f, tuningD4->valueC, 0);
        }
        break;
    case 1: case 3: case 4:
        if (flag8C) valueC8 = 0;
        break;
    }
}
// 0x4831a0: flags are raw bytes, and the current flag is re-read after setup.
void EngineSoundAccelView::Update4831A0(float duration) {
    if (active58) {
        *reinterpret_cast<unsigned int *>(&delta60) = *reinterpret_cast<unsigned int *>(&duration);
        time5C = duration + time5C;
    }
    const unsigned char previous = flagD0;
    flagD8 = 0; flagD1 = previous;
    flagD0 = car68->flag4C; flagD8 = 0;
    if (flagD0) {
        if (!previous) {
            if (Query45CD30()) Configure45CC30();
            else if (Query470030()) Configure45CB70();
        }
        if (flagD0) { Advance47C290(duration); return; }
    }
    if (flagD1 && Query45CDC0()) flagD8 = 1;
    Advance47C290(duration);
}
