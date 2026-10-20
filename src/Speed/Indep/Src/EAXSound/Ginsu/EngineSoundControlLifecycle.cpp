#include "EngineSound.h"

// 0x45ce20: normal-return construction; Windows SEH scaffolding is not authored.
// Preserve the original unwritten fields, including links +0x6c/+0x70/+0x74.
EngineSoundControl *EngineSoundControl::Construct45CE20() {
    int *words = reinterpret_cast<int *>(this);
    words[0] = 0x789ed4;
    for (int i = 1; i <= 16; ++i) {
        words[i] = 0;
        *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0x48) = 0;
    }
    active58 = 0;
    words[0x64 / 4] = 0;
    words[0x50 / 4] = 0;
    state44 = 0;
    car68 = 0;
    words[0] = 0x789fe4;
    curve88.Construct(pairs90, 5);
    averageC4.Construct(4);
    averageC4.FillSamples(0.0f);
    valueF4 = valueEC = valueF0 = 0.0f;
    value104 = value100 = valueFC = 0.0f;
    words[0x78 / 4] = 0;
    valueBC = 24000.0f;
    for (int j = 0x90 / 4; j <= 0xbc / 4; ++j) words[j] = 0;
    *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0xc2) = 0;
    return this;
}

// 0x45cf90: integer subtraction wraps before FILD; retain its exact value
// through both curve-point calculations, without an intermediate float store.
void EngineSoundControl::Bind45CF90(EngineSoundCar *car) {
    if (car->kind14 == 1 || car->kind14 == 2 || car->kind14 == 3) car68 = car;
    else car68 = 0;
    state44 = car;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x64) = car68->vtable0->value44(car68);
    car = car68;
    const char *record = reinterpret_cast<const char *>(car->value84);
    const int first = *reinterpret_cast<const int *>(record + 0xc);
    const int last = *reinterpret_cast<const int *>(record + 0x10);
    pairs90[1].value = 0.0f;
    pairs90[1].coordinate = (float)first;
    const double distance = (int)((unsigned int)last - (unsigned int)first);
    record = reinterpret_cast<const char *>(car->value84);
    pairs90[2].value = 1.0f;
    pairs90[2].coordinate = (float)(distance * *reinterpret_cast<const float *>(record + 0x18) +
                                  *reinterpret_cast<const int *>(record + 0xc));
    record = reinterpret_cast<const char *>(car->value84);
    pairs90[3].value = 1.0f;
    pairs90[3].coordinate = (float)(*reinterpret_cast<const int *>(record + 0x10) -
                                  distance * *reinterpret_cast<const float *>(record + 0x14));
    record = reinterpret_cast<const char *>(car->value84);
    pairs90[4].value = 0.0f;
    pairs90[4].coordinate = (float)*reinterpret_cast<const int *>(record + 0x10);
}
