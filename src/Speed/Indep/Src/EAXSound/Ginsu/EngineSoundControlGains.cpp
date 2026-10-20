#include "EngineSound.h"
#include "../../Physics/VehicleMover.h"

extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)

struct EngineAverageVtable {
    void *unidentified0;
    void (__fastcall *recalculate4)(SampleAverage *);
};

// 0x45d0a0: choose the ring update/reset, then dispatch its recalculation.
void EngineSoundControl::Prepare45D0A0() {
    EngineSoundCar *car = car68;
    valueEC = car->value48 - valueF4;
    valueF0 = engine6C->value128 - engine6C->value12C;
    valueF4 = car->value48;
    if (car->value40->flagD8) {
        averageC4.AdvanceSample(valueF0);
    } else if (value74->valueC8) {
        if (value74->valueC8 == 1) averageC4.FillSamples(valueEC);
        else averageC4.AdvanceSample(valueF0);
    } else if (value70->value78 == 0) {
        averageC4.AdvanceSample(valueEC);
    } else if (value70->value78 == 5) {
        averageC4.FillSamples(valueF0);
    } else if (value70->value7C == 2) {
        averageC4.FillSamples(engineGainPrepareScale788620 * valueEC);
    } else {
        averageC4.AdvanceSample(valueEC);
    }
    static_cast<EngineAverageVtable *>(averageC4.vtable0)->recalculate4(&averageC4);
}

// 0x4701a0: one-bank gains and cutoff. Keep the record selected before the
// virtual query and re-read the later car/object inputs as the original does.
void EngineSoundControl::ModeOne4701A0(float duration) {
    (void)duration;
    EngineSoundCar *car = car68;
    EngineSoundCarValue **recordSlot = &car->value80;
    bool smooth = true;
    const double raw = ginsuOne784250 -
        ((double)car->value80->value2C - averageC4.averageC) / car->value80->value2C;
    volatile float rounded = (float)raw;
    value104 = rounded;
    float fraction = ginsuZero7D7D3C;
    if (raw > ginsuZero7D7D3C) {
        fraction = rounded;
        if (ginsuOne784250 < rounded) fraction = ginsuOne784250;
    }
    value104 = fraction;
    volatile float targetFirst, targetSecond;
    EngineSoundCarValue *record;
    if (car->value40->flagD8) goto primary;
    if (!state44->car30->HasPendingShift() && value70->value78 == 0) {
        if (value74->valueC8) {
            if (value74->valueC8 == 1) smooth = false;
            goto primary;
        }
        recordSlot = &car68->value80;
        record = *recordSlot;
        if (car68->value54 > engineGainCarLimit7843E8) goto interpolate;
        flagC0 = 1;
        goto fixed;
    }
    switch (value70->value78) {
    case 2: smooth = false;
    case 3:
    case 5: goto primary;
    case 7:
        recordSlot = &car68->value80;
        record = *recordSlot;
        if (car68->flag4C) { smooth = false; goto interpolate; }
        goto fixed;
    default:
        recordSlot = &car68->value80;
        record = *recordSlot;
        goto fixed;
    }
primary:
    recordSlot = &car68->value80;
    record = *recordSlot;
interpolate:
    targetFirst = (record->value30 - (double)record->value38) * value104 + record->value38;
    targetSecond = (record->value34 - (double)record->value3C) * value104 + record->value3C;
    goto cutoff;
fixed:
    targetSecond = record->value44;
    targetFirst = record->value40;
cutoff:
    record = *recordSlot;
    volatile float minimum = (float)record->value48;
    const float maximum = engineGainCutoffMaximum7885D8;
    const double linked = engine6C->vtable0->value38(engine6C);
    double cutoffValue = (maximum - (double)record->value48) * linked * engineGainPercent78425C + record->value48;
    if (!(cutoffValue > minimum)) cutoffValue = minimum;
    if (maximum < cutoffValue) cutoffValue = maximum;
    EngineSoundAuxiliary *engine = engine6C;
    if ((double)engine->value138 < cutoffValue) cutoffValue = (float)engine->value138;
    flagC0 = 0;
    if (smooth) {
        double bound = (double)value100 + engineGainStep78861C;
        if (!(targetFirst > bound)) {
            bound = (double)value100 - engineGainStep78861C;
            if (!(targetFirst < bound)) bound = targetFirst;
        }
        value100 = (float)bound;
        bound = (double)valueFC + engineGainStep78861C;
        if (!(targetSecond > bound)) {
            bound = (double)valueFC - engineGainStep78861C;
            if (!(targetSecond < bound)) bound = targetSecond;
        }
        valueFC = (float)bound;
        volatile float upper = valueBC + engineGainCutoffStep788624;
        if (cutoffValue > upper) cutoffValue = upper;
        else {
            volatile float lower = valueBC - engineGainCutoffStep788624;
            if (cutoffValue < lower) cutoffValue = lower;
        }
    } else {
        value100 = targetFirst;
        valueFC = targetSecond;
    }
    car = car68;
    valueBC = (float)cutoffValue;
    const double factor = (double)engine->valueE0 * engineGainOneScale78AAB4;
    value7C = (int)(((double)car->value80->value24 * value100) * factor);
    value80 = (int)(((double)car->value80->value28 * valueFC) * factor);
    value84 = 0;
    valueB8 = (int)((double)car->value80->value24 * factor);
}

// 0x470540: two-bank tuning, smoothing, curve blend and five ordered gain
// queries. The first two queries use the pointer saved before curve lookup.
void EngineSoundControl::ModeTwo470540(float duration) {
    (void)duration;
    const float step = engineGainStep78861C;
    EngineSoundCar *car = car68;
    bool smooth = true;
    const double delta = fabs((double)averageC4.averageC);
    const double raw = ginsuOne784250 - ((double)car->value80->value2C - delta) / car->value80->value2C;
    volatile float rounded = (float)raw;
    value104 = rounded;
    float fraction = ginsuZero7D7D3C;
    if (raw > ginsuZero7D7D3C) {
        fraction = rounded;
        if (ginsuOne784250 < rounded) fraction = ginsuOne784250;
    }
    value104 = fraction;
    const double rawSecondary = ginsuOne784250 - ((double)car->value84->value1C - delta) / car->value84->value1C;
    rounded = (float)rawSecondary;
    value108 = rounded;
    fraction = ginsuZero7D7D3C;
    if (rawSecondary > ginsuZero7D7D3C) {
        fraction = rounded;
        if (ginsuOne784250 < rounded) fraction = ginsuOne784250;
    }
    value108 = fraction;
    double targetFirst;
    volatile float targetSecond;
    EngineSoundCarValue *primaryRecord;
    EngineSoundCarValue84 *secondaryRecord;
    if (car->value40->flagD8) goto primary;
    if (!state44->car30->HasPendingShift() && value70->value78 == 0) {
        if (value74->valueC8) {
            if (value74->valueC8 == 1) smooth = false;
            goto primary;
        }
        if (car68->flag4C) goto primary;
        goto secondary;
    }
    switch (value70->value78) {
    case 2: smooth = false;
    case 3:
    case 5: goto primary;
    case 7:
        if (car68->flag4C) smooth = false;
        goto primary;
    default: goto secondary;
    }
primary:
    primaryRecord = car68->value80;
    targetFirst = (primaryRecord->value30 - (double)primaryRecord->value38) * value104 + primaryRecord->value38;
    targetSecond = (primaryRecord->value34 - (double)primaryRecord->value3C) * value104 + primaryRecord->value3C;
    goto targets;
secondary:
    secondaryRecord = car68->value84;
    targetFirst = (secondaryRecord->value24 - (double)secondaryRecord->value2C) * value108 + secondaryRecord->value2C;
    targetSecond = (secondaryRecord->value28 - (double)secondaryRecord->value30) * value108 + secondaryRecord->value30;
targets:
    flagC0 = 0;
    if (smooth) {
        volatile float upper = value100 + step;
        if (targetFirst > upper) targetFirst = upper;
        else {
            volatile float lower = value100 - step;
            if (targetFirst < lower) targetFirst = lower;
        }
        value100 = (float)targetFirst;
        double bound = (double)valueFC + step;
        if (!(targetSecond > bound)) {
            bound = (double)valueFC - step;
            if (!(targetSecond < bound)) bound = targetSecond;
        }
        valueFC = (float)bound;
    } else {
        value100 = (float)targetFirst;
        valueFC = targetSecond;
    }
    EngineSoundAuxiliary *savedEngine = engine6C;
    volatile float curve = curve88.Lookup(frequencyF8);
    const double firstGain = savedEngine->vtable0->value38(savedEngine);
    volatile float firstPart = (float)((ginsuOne784250 - firstGain * engineGainPercent78425C) * curve);
    const double secondGain = savedEngine->vtable0->value38(savedEngine);
    const double blend = secondGain * engineGainPercent78425C + firstPart;
    EngineSoundAuxiliary *engine = engine6C;
    car = car68;
    value100 = (float)((ginsuOne784250 - blend) + blend * value100);
    valueFC = (float)(blend * valueFC);
    valueBC = (float)engine->value138;
    primaryRecord = car->value80;
    secondaryRecord = car->value84;
    volatile float factor = (float)((double)engine->valueE0 * engineGainTwoScale78A6C8);
    double gain = engine->vtable0->value38(engine);
    const int secondaryBase = secondaryRecord->value4;
    const int difference = (int)((unsigned int)primaryRecord->value24 - (unsigned int)secondaryBase);
    const int adjustment = (int)((gain * difference) * engineGainNegativePercent784364);
    const int base = (int)((unsigned int)secondaryBase - (unsigned int)adjustment);
    value7C = (int)(((double)base * value100) * factor);
    engine = engine6C;
    primaryRecord = car68->value80;
    gain = engine->vtable0->value38(engine);
    value80 = (int)((((gain * primaryRecord->value28) * factor) * valueFC) * engineGainPercent78425C);
    secondaryRecord = car68->value84;
    engine = engine6C;
    gain = engine->vtable0->value38(engine);
    value84 = (int)((((ginsuOne784250 - gain * engineGainPercent78425C) * secondaryRecord->value8) * factor) * valueFC);
    valueB8 = (int)((double)car68->value80->value24 * factor);
}
