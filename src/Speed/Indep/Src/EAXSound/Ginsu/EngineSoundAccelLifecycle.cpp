#include "EngineSoundAccel.h"
unsigned int bRandom(unsigned int range);

// 0x45c9c0: normal construction, including all unwritten/padding bytes.
EngineSoundAccelView *EngineSoundAccelView::Construct45C9C0() {
    int *words = reinterpret_cast<int *>(this);
    words[0] = 0x789ed4;
    for (int i = 1; i <= 16; ++i) {
        words[i] = 0;
        *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0x48) = 0;
    }
    active58 = 0; value64 = 0; words[0x50 / 4] = 0;
    state44 = 0; car68 = 0;
    words[0] = 0x789fb8;
    value88 = value74 = value7C = value80 = value78 = 0.0f;
    value84 = 0; flag8C = 1;
    valueA4 = value90 = value98 = value9C = value94 = 0.0f;
    valueA0 = 0; flagA8 = 1;
    valueC0 = valueAC = valueB4 = valueB8 = valueB0 = 0.0f;
    valueBC = 0; flagC4 = 1;
    auxiliary6C = 0; shift70 = 0; tuningD4 = 0;
    return this;
}
// 0x46ffa0.
EngineSoundAccelView *CreateEngineSoundAccel46FFA0() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x4db) {
        EngineSoundAccelView *value = static_cast<EngineSoundAccelView *>(EngineSoundAllocate440820(0xdc, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct45C9C0();
    }
    return 0;
}
// 0x45ca80: query precedes the reloaded kind and tuning-record selection.
void EngineSoundAccelView::Bind45CA80(EngineSoundCar *car) {
    car68 = car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3 ? car : 0;
    state44 = car;
    value64 = car68->vtable0->value44(car68);
    if (state44->kind14 == 1) tuningD4 = engineSoundAccelTuning7F3248 + car68->value80->value18;
    else tuningD4 = bRandom(2) ? &engineSoundAccelTuning7F3284 : &engineSoundAccelTuning7F32C0;
}
// 0x45cb10.
void EngineSoundAccelView::Activate45CB10() {
    active58 = 1; valueCC = 0.0f; valueC8 = 0;
    flagD0 = 0; flagD1 = 0; flagD8 = 0;
}
// 0x45cb40 / 0x46ffe0.
int EngineSoundAccelView::Dependency45CB40(int index) {
    if (index == 0) return 5;
    if (index == 1) return 3;
    return -1;
}
void EngineSoundAccelView::Attach46FFE0(EngineSoundComponent *component) {
    const unsigned int id = (*component->vtable0->descriptor4(component) >> 4) & 0xfff;
    if (id == 3) shift70 = reinterpret_cast<EngineSoundShiftView *>(component);
    else if (id == 5) auxiliary6C = reinterpret_cast<EngineSoundAuxiliary *>(component);
    else EngineSoundDiagnostic4691C0(1, engineSoundAttachDiagnostic78AA74);
}
// 0x45c090: complete EAX result, although the acceleration caller tests AL.
int EngineSoundShiftView::IsActive45C090() {
    if (value78 != 3 && value78 == 0) return 0;
    return 1;
}
