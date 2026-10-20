#ifndef UG2_EAXSOUND_ENGINE_SOUND_H
#define UG2_EAXSOUND_ENGINE_SOUND_H

#include "Ginsu.h"

// UG2-only views of the fields read by engine SFX setup 0x47d740.
struct EngineSoundCar {
    char unidentified0[0x10];
    int bankIndex10;
    char unidentified14[0x64];
    int mode78;
};

struct EngineSoundSlot {
    GinsuSynthData *data0;
    GinsuSynthesis *synth4;
    void *memory8;
    float maxFrequencyC;
    float minFrequency10;
    int startResult14;
    int value18;
};

struct EngineSound {
    void Setup47D740();

    char unidentified0[0x9c];
    EngineSoundCar *car9C;
    char unidentifiedA0[0x2c];
    EngineSoundSlot slotsCC[2];
};

typedef char EngineSoundSlotSizeCheck[sizeof(EngineSoundSlot) == 0x1c ? 1 : -1];
typedef char EngineSoundSetupViewSizeCheck[sizeof(EngineSound) == 0x104 ? 1 : -1];

// bWare allocation wrapper 0x440820; available-byte query 0x43e910 remains a heap seam.
void *EngineSoundAllocate440820(int size, int flags);
int EngineSoundHeapAvailable43E910(int heap);
extern int engineSoundHeap7EC840;
extern float ginsuEngineBlockRate78A5B0;

#endif
