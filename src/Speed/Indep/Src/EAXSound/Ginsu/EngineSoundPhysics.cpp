#include "EngineSoundPhysics.h"
#include "../../../Libs/bWare/Strings.h"
#include "../../Physics/VehicleMover.h"

extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)

struct EnginePhysicsAverageVtable {
    void *unidentified0;
    void (__fastcall *recalculate4)(SampleAverage *);
};

// Three independent mode queries: short-circuit order belongs to the caller.
bool EngineSoundModeQueries::Is430560() { return value40 != 0; }
bool EngineSoundModeQueries::Is430420() { return value28 != 0; }
bool EngineSoundModeQueries::Is430350() { return value10 != 0; }

// 0x46b240: only the original six words are initialized; byte +0x18 is not.
EngineSoundRamp *EngineSoundRamp::Construct46B240(float first, float second, float start, float end) {
    const bool close = fabs((double)end - start) < pairCurveEpsilon;
    value0 = first;
    value8 = start;
    value4 = second;
    valueC = end;
    if (close) valueC = end + pairCurveEpsilon;
    value14 = first;
    value10 = 0.0f;
    return this;
}

// 0x4599e0: compare the extended quotient, but clamp/interpolate its rounded copy.
void EngineSoundRamp::Evaluate4599E0() {
    const double raw = ((double)value10 - value8) / ((double)valueC - value8);
    volatile float rounded = (float)raw;
    float fraction = ginsuZero7D7D3C;
    if (raw > ginsuZero7D7D3C) {
        fraction = rounded;
        if (ginsuOne784250 < rounded) fraction = ginsuOne784250;
    }
    flag18 = 0;
    value14 = (float)(((double)value4 - value0) * fraction + value0);
}

// 0x45d210: explicit component prefix and embedded two-sample average.
EngineSoundPhysicsView *EngineSoundPhysicsView::Construct45D210() {
    int *words = reinterpret_cast<int *>(this);
    words[0] = 0x789ed4;
    for (int i = 1; i <= 16; ++i) {
        words[i] = 0;
        *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0x48) = 0;
    }
    active58 = 0;
    value64 = 0;
    words[0x50 / 4] = 0;
    state44 = 0;
    car68 = 0;
    words[0] = 0x78a008;
    average94.Construct(2);
    average94.FillSamples(0.0f);
    valueC8 = 1;
    valueCC = 1;
    flagD8 = 1;
    valueDC = 1;
    value90 = 0.0f;
    value8C = 0.0f;
    flagBC = 0;
    value70 = 1000.0f;
    value84 = 0.0f;
    value88 = 0.0f;
    flagD0 = 0;
    valueD4 = 0;
    valueEC = 1000.0f;
    valueF0 = 0.0f;
    value7C = 10000.0f;
    return this;
}

// 0x45d4f0: invalid kinds still lead to the original null write, not a guard.
void EngineSoundPhysicsView::Bind45D4F0(EngineSoundCar *car) {
    if (car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3) car68 = car;
    else car68 = 0;
    state44 = car;
    car68->value40 = this;
}

// 0x45d370: each receiver is selected after the preceding call returns.
void EngineSoundPhysicsView::Activate45D370() {
    active58 = 1;
    value88 = reinterpret_cast<DrivetrainAuxiliary *>(state44->car30->unidentified40)->EngineMaximum5928D0();
    CarPhysicsRecord *record = state44->car30->physicsInfo10->parameters10.GetRecord(true);
    const float *engine = reinterpret_cast<const float *>(record->engine1F0);
    value7C = engine[2];
    value78 = engine[0];
    value80 = engine[1];
    value74 = (float)(((double)value80 / value7C) * engineControlMaximum784500);
}

static EngineSoundNotificationView *Notification8() {
    return *reinterpret_cast<EngineSoundNotificationView **>(static_cast<char *>(audioObject82BA30) + 8);
}

// 0x45d360: cdecl completion callback, reaching the current audio owner.
void __cdecl EngineSoundPhysicsView::Notify45D360() { Notification8()->Clear45AD40(8); }

// 0x484c50: normal input production and the complete override/notification gates.
void EngineSoundPhysicsView::Update484C50(float duration) {
    if (active58) { delta60 = duration; time5C = duration + time5C; }
    bool mode = false;
    if (flagD0) {
        const int state = *engineSoundMode890118;
        if (state == 3 || state == 2 || state == 4) mode = true;
    }
    flagD8 = engineSoundModes827E88.Is430560() || engineSoundModes827E88.Is430420() ||
        (engineSoundModes827E88.Is430350() && !mode) || engineSoundOverride82A3A4 > 0;
    if (flagD8) { Override46FA30(duration); return; }
    valueCC = valueC8;
    const int gear = state44->car30->gear434;
    value90 = value8C;
    valueC8 = gear;
    // FST to +0x8c does not round the value used in the subtraction.
    const double throttle = (double)state44->car30->input30->throttle20C * engineSoundPercent784258;
    value8C = (float)throttle;
    average94.AdvanceSample((float)(throttle - value90));
    static_cast<EnginePhysicsAverageVtable *>(average94.vtable0)->recalculate4(&average94);
    if (value8C > engineSoundStep784478) {
        if (!flagBC) { flagBC = 1; valueC4 = car68->time34; }
    } else if (flagBC) { flagBC = 0; valueC0 = car68->time34; }
    double filtered = (double)value84 + engineSoundStep784478;
    if (!(value8C > filtered)) {
        filtered = (double)value84 - engineSoundStep784478;
        if (!(value8C < filtered)) filtered = value8C;
    }
    value84 = (float)filtered;
    const float engine = engineSpeedOverride == engineSoundPercent784258 ?
        state44->car30->engineValue440 : engineSpeedOverride;
    value70 = (float)((((double)engine - value78) / ((double)value7C - value78)) *
                     engineSoundRange78C194 + engineControlMinimum784550);
    car68->value44 = (float)filtered;
    car68->value48 = value70;
    car68->flag4C = (float)flagBC != ginsuZero7D7D3C;
    car68->value50 = valueC8;
    car68->value54 = value8C;
    if (flag6C != 1 && *engineSoundMode8900D8 != 2 && *engineSoundMode8900D8 != 3 &&
        *engineSoundMode8900D8 != 7 && state44->kind14 == 1 && EngineSoundBlown5ECE00(state44->car30) &&
        Notification8()->Query46C9A0() == 0) {
        if (flag6C == 1) flag6C = 0;
        const unsigned int name = cshash(engineSoundBlownName78C154);
        if (audioObject82BA30) {
            EngineSoundNotificationView *value = *reinterpret_cast<EngineSoundNotificationView **>(
                reinterpret_cast<char *>(Notification8()) + 0x34);
            value = reinterpret_cast<EngineSoundNotificationView *>(reinterpret_cast<char *>(value) + 4);
            if (value->Ready46C790() == 1) {
                (*reinterpret_cast<EngineSoundNotificationView **>(static_cast<char *>(audioObject82BA30) + 0x28))->
                    Play484510(8, name, 0, Notify45D360, 0);
                flag6C = 1;
            }
        }
    }
}

// 0x46fa30: scripted normalized RPM/throttle sequence, including both state machines.
void EngineSoundPhysicsView::Override46FA30(float duration) {
    (void)duration;
    if (valueDC == 1 && car68->flagB4) {
        valueDC = 2;
        car68->flagB4 = 0;
        valueD4 = (int)((unsigned int)(int)car68->valueB8 + 1u);
    }
    if (car68->flagC0) {
        const int kind = car68->valueC4;
        car68->flagC0 = 0;
        valueE8 = 0;
        valueDC = 1;
        switch (kind) {
        case 2: valueE8 = engineSoundSequence7F0198; valueE4 = 0x27; break;
        case 3: valueE8 = engineSoundSequence7F0238; valueE4 = 0x1a; break;
        case 4: valueE8 = engineSoundSequence7F02A8; valueE4 = 0x1e; break;
        case 5: valueE8 = engineSoundSequence7F0328; valueE4 = 0x1b; break;
        case 6: valueE8 = engineSoundSequence7F0398; valueE4 = 0x16; break;
        case 7: valueE8 = engineSoundSequence7F03F8; valueE4 = 0x13; break;
        case 8: valueE8 = engineSoundSequence7F0448; valueE4 = 0x43; break;
        case 9: valueE8 = engineSoundSequence7F0558; valueE4 = 0x38; break;
        case 10: valueE8 = engineSoundSequence7F0640; valueE4 = 0x22; break;
        case 11: valueE8 = engineSoundSequence7F06D0; valueE4 = 0x1e; break;
        case 12: valueE8 = engineSoundSequence7F0750; valueE4 = 0x1d; break;
        case 13: valueE8 = engineSoundSequence7F07C8; valueE4 = 0x2f; break;
        case 14: valueE8 = engineSoundSequence7F0888; valueE4 = 0x1b; break;
        case 15: valueE8 = engineSoundSequence7F08F8; valueE4 = 0x18; break;
        default: valueE8 = engineSoundSequence7F0070; valueE4 = 0x48; break;
        }
        if (valueE8) {
            valueDC = 3;
            valueE0 = 0;
            const unsigned short *pair = valueE8;
            EngineSoundRamp rpm, throttle;
            rpm.Construct46B240((float)pair[0], (float)pair[2], 0.0f, 4.0f);
            throttle.Construct46B240((float)(unsigned char)pair[1], (float)(unsigned char)pair[3], 0.0f, 4.0f);
            rpm.value10 = 0.0f;
            rpm.Evaluate4599E0();
            valueEC = rpm.value14;
            throttle.value10 = 0.0f;
            throttle.Evaluate4599E0();
            valueF0 = throttle.value14;
        }
    }
    if (valueDC == 2) {
        valueD4 = (int)((unsigned int)valueD4 - 1u);
        if (valueD4 < 0) valueD4 = 0;
        if (valueD4 == 0) valueDC = 1;
        else {
            valueEC = (float)((double)engineSoundOverrideStep788608 * car68->valueBC * engineGainPercent78425C + valueEC);
            valueF0 = valueF0 + engineSoundThrottleStep787AF4;
        }
    } else if (valueDC == 3) {
        valueE0 = (int)((unsigned int)valueE0 + 1u);
        if (valueE0 == 4) {
            valueE4 = (int)((unsigned int)valueE4 - 1u);
            valueE0 = 0;
            if (valueE4 == 0) valueDC = 1;
            else valueE8 += 2;
        }
        if (valueDC != 1) {
            const unsigned short *pair = valueE8;
            EngineSoundRamp rpm, throttle;
            rpm.Construct46B240((float)pair[0], (float)pair[2], 0.0f, 4.0f);
            throttle.Construct46B240((float)(unsigned char)pair[1], (float)(unsigned char)pair[3], 0.0f, 4.0f);
            volatile float position = (float)valueE0;
            rpm.value10 = position;
            rpm.Evaluate4599E0();
            valueEC = rpm.value14;
            throttle.value10 = position;
            throttle.Evaluate4599E0();
            valueF0 = throttle.value14;
        }
    } else {
        valueEC = valueEC - engineSoundOverrideDecay78860C;
        valueF0 = valueF0 - engineSoundThrottleStep787AF4;
    }
    float rpm = engineControlMinimum784550;
    if (valueEC > engineControlMinimum784550) {
        rpm = valueEC;
        if (engineControlMaximum784500 < rpm) rpm = engineControlMaximum784500;
    }
    valueEC = rpm;
    float throttle = ginsuZero7D7D3C;
    if (valueF0 > ginsuZero7D7D3C) {
        throttle = valueF0;
        if (engineSoundPercent784258 < throttle) throttle = engineSoundPercent784258;
    }
    value70 = rpm;
    valueF0 = throttle;
    value84 = throttle;
    flagBC = throttle > engineSoundStep784478;
    value8C = throttle;
    car68->value44 = throttle;
    car68->value48 = value70;
    car68->flag4C = (float)flagBC != ginsuZero7D7D3C;
    car68->value50 = valueC8;
    car68->value54 = value8C;
}
