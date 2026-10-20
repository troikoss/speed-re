#include "EngineSoundShift.h"
// 0x45c0d0: preserve padding and fields not written by the original constructor.
EngineSoundShiftView *EngineSoundShiftView::Construct45C0D0() {
    int *words = reinterpret_cast<int *>(this);
    words[0] = 0x789ed4;
    for (int i = 1; i <= 16; ++i) {
        words[i] = 0;
        *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0x48) = 0;
    }
    active58 = 0; value64 = 0; words[0x50 / 4] = 0; state44 = 0; car68 = 0;
    words[0] = 0x789f28;
    valueC0 = valueAC = valueB4 = valueB8 = valueB0 = 0.0f; valueBC = 0; flagC4 = 1;
    valueDC = valueC8 = valueD0 = valueD4 = valueCC = 0.0f; valueD8 = 0; flagE0 = 1;
    valueF8 = valueE4 = valueEC = valueF0 = valueE8 = 0.0f; valueF4 = 0; flagFC = 1;
    value118 = value104 = value10C = value110 = value108 = 0.0f; value114 = 0; flag11C = 1;
    value134 = value120 = value128 = value12C = value124 = 0.0f; value130 = 0; flag138 = 1;
    value154 = value140 = value148 = value14C = value144 = 0.0f; value150 = 0; flag158 = 1;
    tuning74 = 0;
    return this;
}
// 0x46ebd0.
EngineSoundShiftView *CreateEngineSoundShift46EBD0() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x55b) {
        EngineSoundShiftView *value = static_cast<EngineSoundShiftView *>(EngineSoundAllocate440820(0x15c, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct45C0D0();
    }
    return 0;
}
// 0x45c330: the non-player query is retained, including the final unconditional tuning choice.
void EngineSoundShiftView::Bind45C330(EngineSoundCar *car) {
    car68 = car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3 ? car : 0;
    state44 = car;
    value64 = car68->valueA4;
    if (car->kind14 == 1) {
        const int selector = (int)((unsigned int)value64 + (unsigned int)car68->value8C * 4u);
        tuning74 = engineSoundShiftTuning7F2860 + engineSoundShiftSelection7F2234[selector * 3];
    } else {
        car68->vtable0->value44(car68);
        tuning74 = &engineSoundShiftTuning7F2EC0;
    }
    value100 = value64 % 2;
}
// 0x45c3d0.
void EngineSoundShiftView::Activate45C3D0() {
    active58 = 1;
    value80 = value84 = value88 = value8C = value90 = value94 = 0;
    value78 = value7C = value13C = 0; flag98 = 0;
    value9C = 1; valueA0 = valueA4 = 0.0f; value100 = 0;
}
// 0x45c310 / 0x46ec30.
int EngineSoundShiftView::Dependency45C310(int index) { return index == 0 ? 5 : -1; }
void EngineSoundShiftView::Attach46EC30(EngineSoundComponent *component) {
    if ((*component->vtable0->descriptor4(component) & 0xfff0) != 0x50)
        EngineSoundDiagnostic4691C0(1, engineSoundAttachDiagnostic78AA74);
    else auxiliary6C = reinterpret_cast<EngineSoundAuxiliary *>(component);
}
// 0x45c430. No envelope contents or pending-notification flag are reset here.
void EngineSoundShiftView::Reset45C430() {
    value13C = 0; value80 = value84 = value88 = value8C = value90 = value94 = 0;
    value78 = value7C = 0;
}
