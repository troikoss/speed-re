#include "EngineSoundMaster.h"
#include "../../Audio/AudioSystem.h"
#include "EngineSoundEnvelope.h"
extern Audio82B884 *audio82B884;
// 0x45dc80: global MasterVol construction preserves all unwritten bytes.
EngineSoundMaster *EngineSoundMaster::Construct45DC80() {
    int *words = reinterpret_cast<int *>(this);
    words[0] = 0x789ed4;
    for (int i = 1; i <= 16; ++i) {
        words[i] = 0;
        *reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0x48) = 0;
    }
    active58 = 0; value64 = 0; words[0x50 / 4] = 0; state44 = 0; car68 = 0;
    words[0] = 0x78a08c;
    return this;
}
// 0x471930.
EngineSoundMaster *CreateEngineSoundMaster471930() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x46b) {
        EngineSoundMaster *value = static_cast<EngineSoundMaster *>(EngineSoundAllocate440820(0x6c, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct45DC80();
    }
    return 0;
}
// 0x45dcc0: requires the original non-null mix record; active58 is untouched.
void EngineSoundMaster::Activate45DCC0() {
    float *mix = audio82B884->mix38;
    values4[0] = (int)(((double)mix[4] * mix[0]) * engineSoundCurveInteger789CF4);
}
