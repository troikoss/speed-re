#include "FEngResponseArray.h"

// Original 0x75c461, normal path only. Count comparison is signed, unlike
// the unsigned requested size and min-count arithmetic in ResizeCommands.
void __stdcall FEngConstructArrayWithCleanup(void *memory, unsigned int stride, unsigned int count,
                                            FEngLifetimeConstructor constructor,
                                            FEngLifetimeDestructor destructor) {
    unsigned int i = 0;
    while ((int)i < (int)count) {
        constructor(memory);
        memory = (void *)((unsigned int)memory + stride);
        ++i;
    }
}

// Original 0x75c401, normal path only. Decrement wraps before the sign test.
void __stdcall FEngDestructArrayWithCleanup(void *memory, unsigned int stride, unsigned int count,
                                           FEngLifetimeDestructor destructor) {
    memory = (void *)((unsigned int)memory + stride * count);
    while ((int)--count >= 0) {
        memory = (void *)((unsigned int)memory - stride);
        destructor(memory);
    }
}
