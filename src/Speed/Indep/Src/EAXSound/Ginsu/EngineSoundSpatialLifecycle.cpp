#include "EngineSoundSpatial.h"

// 0x45dd60: explicit normal-return construction; keep dirty unwritten fields.
EngineSoundSpatialComponent *EngineSoundSpatialComponent::Construct45DD60() {
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
    words[0] = 0x78a0b4;
    averageA8.ConstructDefault576880();
    position70 = 0;
    orientation74 = 0;
    for (int j = 0; j < 6; ++j) limits88[j] = 5000;
    averageA8.Allocate5768A0(4);
    currentA0 = previousA4 = -1.0f;
    flagD4 = flagD5 = flagD6 = 0;
    listenerD0 = 0;
    words[0x30 / 4] = 0;
    mode6C = 1;
    return this;
}

// 0x471d60 / 0x47c7a0: derived table and observed 0xd8-byte allocation.
EngineSoundSpatialComponent *EngineSoundSpatialComponent::Construct471D60() {
    Construct45DD60();
    vtable0 = reinterpret_cast<EngineSoundSpatialVtable *>(0x78ab68);
    return this;
}
EngineSoundSpatialComponent *CreateEngineSoundSpatial47C7A0() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x4d7) {
        EngineSoundSpatialComponent *value = static_cast<EngineSoundSpatialComponent *>(
            EngineSoundAllocate440820(0xd8, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct471D60();
    }
    return 0;
}

// 0x45dac0: invalid kinds clear the typed car but retain the supplied state.
void EngineSoundSpatialComponent::Bind45DAC0(EngineSoundCar *car) {
    if (car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3) car68 = car;
    else car68 = 0;
    state44 = car;
}
// 0x45bad0.
void EngineSoundSpatialComponent::Activate45BAD0() { active58 = 1; }
// 0x45de30: no introduced index bound.
void EngineSoundSpatialComponent::Limit45DE30(int index, int limit) {
    if (limits88[index] < limit) limits88[index] = limit;
}
// 0x45de50: FILD preserves the integer exactly through ST0; RET 4
// consumes one unused stack argument.
float EngineSoundSpatialComponent::Value45DE50(int value) {
    (void)value;
    return (double)value8;
}
