#include "EngineSoundSpatial.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;
extern float ginsuZero7D7D3C;
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

struct SpatialAverageVtable {
    void *unidentified0;
    void (__fastcall *recalculate4)(SampleAverage *);
};

// 0x45df00: preserve mode-specific partial writes and shared pointer state.
void EngineSoundSpatialComponent::Compute45DF00() {
    const int index = listenerD0;
    if ((!engineSoundSpatialReady82BB40[index] && !engineSoundSpatialReady82BB40[0]) || !position70) {
        value10 = 0;
        value8 = -1;
        valueC = 0;
        value4 = -1;
        return;
    }
    *reinterpret_cast<unsigned int *>(&snapshot78[0]) = *reinterpret_cast<unsigned int *>(&position70[0]);
    *reinterpret_cast<unsigned int *>(&snapshot78[1]) = *reinterpret_cast<unsigned int *>(&position70[1]);
    switch (engineSoundSpatialMode82BC14) {
    case 0:
        engineSoundSpatialPoint82BB7C = engineSoundSpatialBF10 + index * 2;
        engineSoundSpatialDirection82BB78 = engineSoundSpatialBECC + listenerD0 * 2;
        Angle45DE60();
        value10 = engineSoundSpatialAngle82BB80;
        value8 = (int)EngineSoundDistance2D401150(snapshot78, engineSoundSpatialPoint82BB7C);
        engineSoundSpatialPoint82BB7C = engineSoundSpatialBEDC + listenerD0 * 2;
        engineSoundSpatialDirection82BB78 = engineSoundSpatialC6C4 + listenerD0 * 2;
        goto second;
    case 1:
        engineSoundSpatialPoint82BB7C = engineSoundSpatialBF20 + index * 2;
        engineSoundSpatialDirection82BB78 = engineSoundSpatialBE70 + listenerD0 * 2;
        Angle45DE60();
        value10 = engineSoundSpatialAngle82BB80;
        value8 = (int)EngineSoundDistance2D401150(snapshot78, engineSoundSpatialPoint82BB7C);
        engineSoundSpatialPoint82BB7C = engineSoundSpatialBEDC + listenerD0 * 2;
        engineSoundSpatialDirection82BB78 = engineSoundSpatialC6C4 + listenerD0 * 2;
        goto second;
    case 2: case 3:
        engineSoundSpatialPoint82BB7C = engineSoundSpatialC670 + index * 2;
        engineSoundSpatialDirection82BB78 = engineSoundSpatialBE70 + listenerD0 * 2;
second:
        Angle45DE60();
        valueC = engineSoundSpatialAngle82BB80;
        value4 = (int)EngineSoundDistance2D401150(snapshot78, engineSoundSpatialPoint82BB7C);
        break;
    }
    if (!orientation74) {
        int *words = reinterpret_cast<int *>(this);
        words[0x18 / 4] = 0;
        words[0x1c / 4] = 0;
        words[0x2c / 4] = 0;
    }
}

// 0x45e100: one second-listener delta remains extended; equality and
// unordered comparisons select index 1. Null position leaves all state alone.
void EngineSoundSpatialComponent::Select45E100(int listener) {
    if (!position70) return;
    listenerD0 = listener;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(audio82B884) + 0x84) == 0) {
        engineSoundSpatialMode82BC14 = 0;
        listenerD0 = 0;
        Compute45DF00();
        return;
    }
    engineSoundSpatialMode82BC14 = 1;
    volatile float ax = position70[0] - engineSoundSpatialBEDC[0];
    volatile float ay = position70[1] - engineSoundSpatialBEDC[1];
    volatile float bx = position70[0] - engineSoundSpatialBEDC[2];
    const double by = (double)position70[1] - engineSoundSpatialBEDC[3];
    // Both roots remain extended through the comparison in the original.
    listenerD0 = !(sqrt((double)ay * ay + (double)ax * ax) <
                    sqrt(by * by + (double)bx * bx)) ? 1 : 0;
    Compute45DF00();
}

// 0x45e1f0: the two stack words are consumed but the body uses delta60.
// Re-read currentA0 after the virtual recalculation before copying its word.
void EngineSoundSpatialComponent::Doppler45E1F0(int listener, float duration) {
    (void)listener; (void)duration;
    if (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(audio82B884) + 0x13)) return;
    if (mode6C == 0) currentA0 = (float)value8;
    else if (mode6C == 1) currentA0 = (float)value4;
    float factor;
    if (currentA0 < ginsuZero7D7D3C || previousA4 < ginsuZero7D7D3C) factor = 1.0f;
    else factor = (float)((double)engineSoundDoppler7887FC /
                         ((((double)currentA0 - previousA4) / delta60) + engineSoundDoppler7887FC));
    averageA8.AdvanceSample(factor);
    static_cast<SpatialAverageVtable *>(averageA8.vtable0)->recalculate4(&averageA8);
    *reinterpret_cast<unsigned int *>(&previousA4) = *reinterpret_cast<unsigned int *>(&currentA0);
}

// 0x471dc0: virtual selection completes before re-reading the selected index.
void EngineSoundSpatialComponent::Update471DC0(float duration) {
    if (active58) {
        *reinterpret_cast<unsigned int *>(&delta60) = *reinterpret_cast<unsigned int *>(&duration);
        time5C = duration + time5C;
    }
    vtable0->value24(this, 0, 0);
    Doppler45E1F0(listenerD0, duration);
}
