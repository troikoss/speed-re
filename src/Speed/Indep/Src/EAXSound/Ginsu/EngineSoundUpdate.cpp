#include "EngineSound.h"

static int ClampEngineParameter(int value, int minimum, int maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static int EngineUpdateProduct(int a, int b) {
    return (int)((unsigned int)a * (unsigned int)b);
}

static int EngineUpdateSubtract(int a, int b) {
    return (int)((unsigned int)a - (unsigned int)b);
}

// 0x474310..0x4745a1: frequency source, controller selection and the complete
// parameter update tail. Unknown parameter meanings stay offset-based.
void EngineSound::Update474310() {
    // Original MOV/MOV pair: preserve the word, including NaN payload bits.
    *reinterpret_cast<unsigned int *>(&frequency104) =
        *reinterpret_cast<unsigned int *>(&controlC0->frequencyF8);
    valueBC = 1.0f;
    if (car9C->mode78 == 0) Update473E60();
    else Update474030();
    if (!parametersB0) return;

    const int gain = vtable0->value38(this, 0, 0);
    const int level = (EngineUpdateProduct(controlC0->value7C, gain) >> 15) - 32767;
    const int globalLevel = EngineUpdateProduct(engineSoundValue82B070, gain) >> 15;
    const int savedValue = engineSoundValue82B880;
    parametersB0->value14 = ClampEngineParameter(level, -32767, 0);
    parametersB0->value18 = ClampEngineParameter(level, -32767, 0);
    parametersB0->value68 = ClampEngineParameter(EngineUpdateProduct(controlC0->valueB8, gain) >> 15, 0, 32767);
    parametersB0->value8 = ClampEngineParameter((int)frequency104, 0, 10000);
    parametersB0->value44 = ClampEngineParameter(
        EngineUpdateSubtract(-16383, (int)(valueBC * engineSoundScale78AF84)), -16383, 16383);
    parametersB0->value10 = ClampEngineParameter(
        (int)(auxiliaryB8->vtable0->value38(auxiliaryB8) * engineSoundScale78A1C8), 0, 1024);
    parametersB0->value48 = ClampEngineParameter(auxiliaryB8->value134, 0, 1024);
    parametersB0->value40 = ClampEngineParameter(vtable0->value3C(this, 0, 0), 0, 65536);
    parametersB0->value38 = ClampEngineParameter(EngineUpdateSubtract(savedValue, 32767), -32767, 0);
    parametersB0->value3C = ClampEngineParameter(globalLevel, 0, 32767);
    parametersB0->value58 = ClampEngineParameter(auxiliaryB8->value138, 0, 25000);
    parametersB0->value5C = controlC0->flagC0 ? 11000 : 0;
    parametersB0->value64 = 0;
    if (parametersB0->dispatcher0) parametersB0->dispatcher0->Dispatch735580(&parametersB0->value4);
}
