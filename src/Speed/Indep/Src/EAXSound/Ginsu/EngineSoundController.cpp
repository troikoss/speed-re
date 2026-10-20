#include "EngineSound.h"

static int ClampSound127(int value) {
    if (value < 0) return 0;
    if (value > 127) return 127;
    return value;
}

// IMUL keeps the low 32 bits before either arithmetic shift, including overflow.
static int SoundProduct(int a, int b) {
    return (int)((unsigned int)a * (unsigned int)b);
}

// The return stays in ST0: no float store follows the original FDIVR. Only
// the integer rate and the cycle argument are rounded to float before the call.
static float EngineCurrentFrequency(GinsuSynthesis *synth) {
    if (synth->sampleRate24 == 0) return ginsuZero7D7D3C;
    volatile float rate = (float)synth->sampleRate24;
    volatile float cycle = synth->currentCycle40 + ginsuHalf784260;
    return rate / synth->synthData28->CyclePeriod(cycle);
}

// 0x473e60..0x474028: one-slot engine controller. Slot +0x38 is an integer
// gain source, not the frequency reader; the latter is frequency104.
void EngineSound::Update473E60() {
    if (!activeC8) return;
    const int gain = vtable0->value38(this, 0, 0);
    float target = frequency104;
    volatile float ratio = 1.0f;
    if (frequency104 < slotsCC[0].minFrequency10) {
        target = slotsCC[0].minFrequency10;
        ratio = frequency104 / slotsCC[0].minFrequency10;
    }
    const int value = ClampSound127(engineSoundValue82A998 >> 8);
    volume108 = (SoundProduct(controlC0->value80, gain) >> 15) >> 8;
    volume108 = ClampSound127(volume108);
    const int value2 = ClampSound127(SoundProduct(engineSoundValue82B078, gain) >> 23);
    SNDSYS_entercritical();
    EngineSnd72412A(slotsCC[0].startResult14, volume108);
    EngineSnd724093(slotsCC[0].startResult14, vtable0->value3C(this, 0, 0), 0);
    EngineSnd723D9B(slotsCC[0].startResult14, (int)(ratio * valueBC * engineSoundScale78A2E0));
    EngineSnd72403E(slotsCC[0].startResult14, (int)controlC0->valueBC);
    EngineSnd723C41(slotsCC[0].startResult14, value);
    EngineSnd723BC1(slotsCC[0].startResult14, 0, value2);
    SNDSYS_leavecritical();
    slotsCC[0].synth4->UpdateFrequency(target, 60.0f);
    frequency104 = EngineCurrentFrequency(slotsCC[0].synth4) * ratio * engineSoundScale78AF80;
}

// 0x474030..0x47430a: two slots. The lock encloses both synth updates and
// frequency feedback; the second slot uses the same saved gain/target/ratio.
void EngineSound::Update474030() {
    if (!activeC8) return;
    const int gain = vtable0->value38(this, 0, 0);
    float target = frequency104;
    volatile float ratio = 1.0f;
    if (frequency104 < slotsCC[0].minFrequency10) {
        target = slotsCC[0].minFrequency10;
        ratio = frequency104 / slotsCC[0].minFrequency10;
    }
    int value = ClampSound127(engineSoundValue82A998 >> 8);
    volume108 = (SoundProduct(controlC0->value80, gain) >> 15) >> 8;
    volume108 = ClampSound127(volume108);
    int value2 = ClampSound127(SoundProduct(engineSoundValue82B078, gain) >> 23);
    SNDSYS_entercritical();
    EngineSnd72412A(slotsCC[0].startResult14, volume108);
    EngineSnd724093(slotsCC[0].startResult14, vtable0->value3C(this, 0, 0), 0);
    EngineSnd723D9B(slotsCC[0].startResult14, (int)(ratio * valueBC * engineSoundScale78A2E0));
    EngineSnd72403E(slotsCC[0].startResult14, (int)controlC0->valueBC);
    EngineSnd723C41(slotsCC[0].startResult14, value);
    EngineSnd723BC1(slotsCC[0].startResult14, 0, value2);
    slotsCC[0].synth4->UpdateFrequency(target, 60.0f);
    frequency104 = EngineCurrentFrequency(slotsCC[0].synth4) * ratio * engineSoundScale78AF80;

    value = ClampSound127(engineSoundValue82A998 >> 8);
    volume10C = (SoundProduct(controlC0->value84, gain) >> 15) >> 8;
    volume10C = ClampSound127(volume10C);
    value2 = ClampSound127(SoundProduct(engineSoundValue82B078, gain) >> 23);
    EngineSnd72412A(slotsCC[1].startResult14, volume10C);
    EngineSnd724093(slotsCC[1].startResult14, vtable0->value3C(this, 0, 0), 0);
    EngineSnd723D9B(slotsCC[1].startResult14, (int)(ratio * valueBC * engineSoundScale78A2E0));
    EngineSnd72403E(slotsCC[1].startResult14, (int)controlC0->valueBC);
    EngineSnd723C41(slotsCC[1].startResult14, value);
    EngineSnd723BC1(slotsCC[1].startResult14, 0, value2);
    slotsCC[1].synth4->UpdateFrequency(target, 60.0f);
    SNDSYS_leavecritical();
}
