#include "EngineSoundPhysics.h"

// 0x470a00: registered SFXCTL_Physics factory. The original heap headroom
// comparison is strict, and constructor execution follows successful allocation.
EngineSoundPhysicsView *CreateEngineSoundPhysics470A00() {
    if (EngineSoundHeapAvailable43E910(engineSoundHeap7EC840) > 0x4f3) {
        EngineSoundPhysicsView *value = static_cast<EngineSoundPhysicsView *>(
            EngineSoundAllocate440820(0xf4, (engineSoundHeap7EC840 & 7) | 0x1000));
        if (value) return value->Construct45D210();
    }
    return 0;
}
