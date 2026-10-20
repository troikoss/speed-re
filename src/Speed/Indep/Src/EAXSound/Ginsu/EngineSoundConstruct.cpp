#include "EngineSound.h"

// UG2 PC v1.2 0x45e350..0x45e38f: shared sound base construction.
// Unknown fields stay offset-based. In particular, the unmentioned bytes
// between the cleared regions retain their prior contents.
EngineSound *EngineSound::Construct45E350() {
    int *words = reinterpret_cast<int *>(this);
    vtable0 = reinterpret_cast<EngineSoundVtable *>(0x789ed4);
    int i;
    for (i = 1; i <= 16; ++i) {
        words[i] = 0;
        value48 = 0;
    }
    vtable0 = reinterpret_cast<EngineSoundVtable *>(0x78a0f0);
    words[0x50 / 4] = 0;
    for (i = 0; i < 16; ++i) words[0x58 / 4 + i] = 0;
    return this;
}

// 0x473bf0..0x473c50: intermediate engine-sound base. The six globals
// are reset on each construction, not only during audio system startup.
EngineSound *EngineSound::Construct473BF0() {
    Construct45E350();
    char *bytes = reinterpret_cast<char *>(this);
    int *words = reinterpret_cast<int *>(this);
    bytes[0xa0] = 0;
    words[0xac / 4] = 0;
    words[0xa8 / 4] = 0;
    car9C = 0;
    vtable0 = reinterpret_cast<EngineSoundVtable *>(0x78ae90);
    parametersB0 = 0;
    auxiliaryB8 = 0;
    valueB4 = 0;
    bytes[0x54] = 0;
    for (int i = 0; i < 6; ++i) engineSoundConstructionValues82AF90[i] = 0;
    engineSoundConstructionValue7ED440 = -1;
    words[0xbc / 4] = 0;
    return this;
}

// 0x473d30..0x473d83: full address span is 84 bytes, including a
// three-byte alignment gap excluded from Ghidra's 81-byte body measure.
EngineSound *EngineSound::Construct473D30() {
    Construct473BF0();
    vtable0 = reinterpret_cast<EngineSoundVtable *>(0x78af30);
    for (int i = 0; i < 2; ++i) {
        EngineSoundSlot *slot = &slotsCC[i];
        slot->data0 = 0;
        slot->synth4 = 0;
        slot->memory8 = 0;
        slot->maxFrequencyC = 0.0f;
        slot->minFrequency10 = 0.0f;
        slot->startResult14 = -1;
        slot->value18 = 0;
    }
    controlC0 = 0;
    auxiliaryB8 = 0;
    activeC8 = 0;
    return this;
}
