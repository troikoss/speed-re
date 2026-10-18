#include "FEngResponseArray.h"

// Original 0x75c461, normal path only. Count comparison is signed, unlike
// the unsigned requested size and min-count arithmetic in ResizeCommands.
void __stdcall FEngConstructResponseCommands(void *memory, unsigned int stride, unsigned int count,
                                            FEngCommandConstructor constructor,
                                            FEngCommandDestructor destructor) {
    unsigned int i = 0;
    while ((int)i < (int)count) {
        constructor((FEResponseCommand *)memory);
        memory = (void *)((unsigned int)memory + stride);
        ++i;
    }
}

// Original 0x75c401, normal path only. Decrement wraps before the sign test.
void __stdcall FEngDestructResponseCommands(void *memory, unsigned int stride, unsigned int count,
                                           FEngCommandDestructor destructor) {
    memory = (void *)((unsigned int)memory + stride * count);
    while ((int)--count >= 0) {
        memory = (void *)((unsigned int)memory - stride);
        destructor((FEResponseCommand *)memory);
    }
}
