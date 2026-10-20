#include "EngineSound.h"
#include "../../Audio/AudioSystem.h"
#if defined(_MSC_VER) && _MSC_VER < 1400
inline void *operator new(unsigned int, void *memory) throw() { return memory; }
inline void operator delete(void *, void *) throw() {}
#else
#include <new>
#endif

extern Audio82B884 *audio82B884;
void *bMalloc(int size, const char *file, int line, int flags);

// 0x440820: cdecl, two arguments. The full heap implementation remains separate.
void *EngineSoundAllocate440820(int size, int flags) {
    return bMalloc(size, 0, 0, flags);
}

// 0x47d740. Re-read the car mode on each iteration as the original does. In
// particular, allocation failure does not add an early return: callers must
// provide the original heap/state invariants. SetSynthData's result is ignored.
void EngineSound::Setup47D740() {
    for (int i = 0; ; ++i) {
        const int mode = car9C->mode78;
        if (!((mode == 0 && i < 1) || (mode == 1 && i < 2))) return;
        EngineSoundSlot *slot = &slotsCC[i];
        if (slot->data0 == 0) {
            void *memory = 0;
            if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) >= 0x4b0) {
                memory = EngineSoundAllocate440820(0xb0, (engineSoundHeap7EC840 & 7) | 0x1000);
            }
            slot->data0 = memory ? new (memory) GinsuSynthData() : 0;
        }
        const int overhead = SNDPKTPLAY_overhead(8);
        volatile float samples = (ginsuRateScaleC788898 + ginsuRateScaleB788894) * ginsuEngineBlockRate78A5B0;
        const int size = overhead + 4 + 4 * GinsuCeilOf(samples);
        if (slot->memory8 == 0) {
            void *memory = 0;
            if (size + 0x400 <= EngineSoundHeapAvailable43E910(engineSoundHeap7EC840)) {
                memory = EngineSoundAllocate440820(size, (engineSoundHeap7EC840 & 7) | 0x1000);
            }
            slot->memory8 = memory;
        }
        if (slot->synth4 == 0) {
            void *memory = 0;
            if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) >= 0x444) {
                memory = EngineSoundAllocate440820(0x44, (engineSoundHeap7EC840 & 7) | 0x1000);
            }
            slot->synth4 = memory ? new (memory) GinsuSynthesis(slot->memory8, size) : 0;
        }
        const int index = car9C->bankIndex10;
        void *bank = i == 0 ? audio82B884->engineBanksDC[index] : audio82B884->engineBanksE4[index];
        if (slot->data0->BindToData(bank)) {
            slot->synth4->SetSynthData(*slot->data0);
            slot->maxFrequencyC = slot->data0->maxFrequency8;
            const float minimum = slot->data0->minFrequency4;
            slot->minFrequency10 = minimum;
            if (slot->startResult14 < 0) slot->startResult14 = slot->synth4->StartSynthesis(minimum);
        }
    }
}
