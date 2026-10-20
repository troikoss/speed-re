#include "EngineSound.h"
#include "../../Physics/VehicleMover.h"

struct AuxAverageVtable {
    void *unidentified0;
    void (__fastcall *recalculate4)(SampleAverage *);
};

// 0x479700: car-state virtual +0x44 used by binding and activation.
int EngineSoundCar::Value479700() { return value94; }

// 0x45c4b0 / 0x45c670: descriptor and ordered dependency IDs.
unsigned int *EngineSoundAuxiliary::Descriptor45C4B0() { return engineSoundAuxDescriptor7ED2C4; }
int EngineSoundAuxiliary::Dependency45C670(int index) {
    switch (index) {
    case 0: return 3;
    case 1: return 4;
    case 2: return 0;
    case 3: return 8;
    default: return -1;
    }
}

// 0x46f1b0: preserve all unwritten words/padding of the 0x1f0-byte component.
EngineSoundAuxiliary *EngineSoundAuxiliary::Construct46F1B0() {
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
    vtable0 = reinterpret_cast<EngineSoundAuxiliaryVtable *>(0x789f70);
    average90.Construct(3);
    averageB8.Construct(2);
    valueFC = valueE8 = valueF0 = valueF4 = valueEC = valueF8 = 0.0f;
    flag100 = 1;
    value118 = value104 = value10C = value110 = value108 = value114 = 0.0f;
    flag11C = 1;
    sequence154.value74 = sequence154.value0 = 0.0f;
    sequence154.index70 = sequence154.count6C = 0;
    for (int i = 0; i < 6; ++i) {
        sequence154.first1C[i] = sequence154.second34[i] = sequence154.duration4[i] = 0.0f;
        sequence154.flags4C[i] = 0;
        sequence154.kinds54[i] = 0;
    }
    sequence154.flag78 = 1;
    value1E4 = value1D0 = value1D8 = value1DC = value1D4 = value1E0 = 0.0f;
    flag1E8 = 1;
    accel70 = 0;
    shift6C = 0;
    physics74 = 0;
    flag13C = flagE4 = 0;
    value108 = 0.001f;
    value114 = 0.0f;
    flag11C = 0;
    value104 = 0.0f;
    value110 = value10C = value118 = 1.0f;
    valueEC = 0.001f;
    valueF4 = valueF0 = valueF8 = 0.0f;
    flag100 = 0;
    valueFC = valueE8 = 0.0f;
    flag152 = 1;
    sequence154.Construct46AF20(0.0f, 0.0f, 1);
    value1DC = value1D8 = value1E0 = 0.0f;
    flag1E8 = 0;
    value1E4 = value1D0 = 0.0f;
    value1D4 = 0.001f;
    value80 = value84 = value88 = 0.0f;
    return this;
}

// 0x47bfd0: heap headroom gate and constructor tail.
EngineSoundAuxiliary *CreateEngineSoundAuxiliary47BFD0() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x5ef) {
        EngineSoundAuxiliary *value = static_cast<EngineSoundAuxiliary *>(
            EngineSoundAllocate440820(0x1f0, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct46F1B0();
    }
    return 0;
}

// 0x45c550: invalid kinds retain the original subsequent null dereference.
void EngineSoundAuxiliary::Bind45C550(EngineSoundCar *car) {
    if (car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3) car68 = car;
    else car68 = 0;
    state44 = car;
    value64 = car68->vtable0->value44(car68);
}

// 0x46f3e0: save the supplied component across its descriptor query.
void EngineSoundAuxiliary::Attach46F3E0(EngineSoundComponent *component) {
    const unsigned int type = (*component->vtable0->descriptor4(component) >> 4) & 0xfff;
    switch (type) {
    case 3: shift6C = reinterpret_cast<EngineSoundShiftView *>(component); break;
    case 4: accel70 = reinterpret_cast<EngineSoundAccelView *>(component); break;
    case 0: physics74 = reinterpret_cast<EngineSoundPhysicsView *>(component); break;
    case 8: spatial78 = reinterpret_cast<EngineSoundSpatialComponent *>(component); break;
    default: EngineSoundDiagnostic4691C0(1, engineSoundAttachDiagnostic78AA74); break;
    }
}

// 0x45c5a0: reset both averages and retain all three virtual query boundaries.
void EngineSoundAuxiliary::Activate45C5A0() {
    active58 = 1;
    value134 = 150;
    average90.FillSamples(0.0f);
    averageB8.FillSamples(0.0f);
    value12C = 0.0f;
    flag13C = 0;
    value140 = value144 = value148 = 0.0f;
    value14C = value14E = value150 = 0;
    value138 = 24000;
    spatial78->vtable0->value2C(spatial78, 0, &value80);
    void *orientation = reinterpret_cast<char *>(state44->car30) + 0x90;
    spatial78->vtable0->value30(spatial78, 0, orientation);
    const int value = car68->vtable0->value44(car68);
    int *words = reinterpret_cast<int *>(this);
    if (value == 1) words[1] = 0x2aaa;
    else if (value == 2) words[1] = 0x5554;
    else if (value == 3) words[1] = 0x7fff;
    else words[1] = 0;
}

// 0x46f7d0: raw query publication and a separate averaged frequency input.
void EngineSoundAuxiliary::ProduceRpm46F7D0(float duration) {
    (void)duration;
    EngineSoundShiftView *shift = shift6C;
    float target;
    if (shift && shift->value78 != 0) target = shift->valueDC;
    else if (accel70 && accel70->valueC8 != 0) target = accel70->value88;
    else target = car68->value48;
    if (flag13C && shift->value78 == 0) {
        const float carValue = car68->value48;
        const float decrease = engineSoundRpmDecrease7885CC;
        const double observed = vtable0->value34(this);
        volatile float rounded = (float)observed;
        double bound = observed + engineSoundRpmIncrease78AAB0;
        if (!(carValue > bound)) {
            bound = (double)rounded - decrease;
            if (!(carValue < bound)) bound = carValue;
        }
        target = (float)bound;
    }
    float averaged = target;
    *reinterpret_cast<unsigned int *>(&value12C) = *reinterpret_cast<unsigned int *>(&value128);
    value128 = (float)((((double)target + value144) + sequence154.value74) + value144);
    shift = shift6C;
    if (shift->value78 == 2 || shift->value78 == 1) averaged = shift->value154;
    else if (accel70->valueC8 == 1) averaged = car68->value48;
    else if (flagE4) {
        if (flag120) {
            const double value = EngineSoundApproach468E30(value124, 200.0f, 50.0f);
            value124 = (float)value;
            if (value == engineSoundOscillatorLimit78AAAC) flag120 = 0;
        } else {
            const double value = EngineSoundApproach468E30(value124, 0.0f, 50.0f);
            value124 = (float)value;
            if (value == ginsuZero7D7D3C) flag120 = 1;
        }
        averaged = target - value124;
    }
    averageB8.AdvanceSample(averaged);
    static_cast<AuxAverageVtable *>(averageB8.vtable0)->recalculate4(&averageB8);
    EngineSoundPhysicsView *physics = physics74;
    car68->value58 = (float)((((double)averageB8.averageC - engineControlMinimum784550) * engineControlScale78A290) *
                            ((double)physics->value7C - physics->value78) + physics->value78);
}

// 0x45c710: shift overrides accel; normal throttle advances rather than fills.
void EngineSoundAuxiliary::ProduceThrottle45C710(float duration) {
    (void)duration;
    EngineSoundShiftView *shift = shift6C;
    if (shift && shift->value78 != 0) average90.FillSamples(shift->valueC0);
    else if (accel70->valueC8 != 0) average90.FillSamples(accel70->valueC0);
    else average90.AdvanceSample(car68->value44);
    static_cast<AuxAverageVtable *>(average90.vtable0)->recalculate4(&average90);
    value130 = average90.averageC + value148;
}
