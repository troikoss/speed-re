#include "FEngScript.h"

// Original 0x551be0, copy construction, normal return. The receiver must be
// unconstructed storage. This is not assignment and does not free prior fields.
FEScript *FEScript::CopyConstruct(const FEScript *other, unsigned char shallow) {
    next4 = (FEScript *)0xabadcafe;
    previous8 = (FEScript *)0xabadcafe;
    vtable0 = (void *)0x79ad28;
    eventCount24 = 0; events28 = 0; name2C = 0; hash30 = 0;
    lengthC = 0; flags14 = 0; chain18 = 0; trackCount1C = 0; tracks20 = 0;
    SetName(other->name2C);
    hash30 = other->hash30;
    lengthC = other->lengthC;
    time10 = other->time10;
    flags14 = other->flags14;
    SetTracks((unsigned int)other->trackCount1C);
    unsigned int offset = 0;
    if (shallow) {
        unsigned int index = 0;
        while (index < (unsigned int)trackCount1C) {
            ((FEScriptTrack *)((char *)tracks20 + offset))->value0 = ((const FEScriptTrack *)((const char *)other->tracks20 + offset))->value0;
            ((FEScriptTrack *)((char *)tracks20 + offset))->value1 = ((const FEScriptTrack *)((const char *)other->tracks20 + offset))->value1;
            ((FEScriptTrack *)((char *)tracks20 + offset))->value2 = ((const FEScriptTrack *)((const char *)other->tracks20 + offset))->value2;
            ((FEScriptTrack *)((char *)tracks20 + offset))->flags3 = ((const FEScriptTrack *)((const char *)other->tracks20 + offset))->flags3;
            unsigned int sourceBits = *(const unsigned int *)((const char *)other->tracks20 + offset + 4);
            unsigned int *bits = (unsigned int *)((char *)tracks20 + offset + 4);
            *bits ^= (sourceBits ^ *bits) & 0xffffff;
            ((FEScriptTrack *)((char *)tracks20 + offset))->state7 = ((const FEScriptTrack *)((const char *)other->tracks20 + offset))->state7;
            const unsigned int *source = (const unsigned int *)((const char *)other->tracks20 + offset + 8);
            unsigned int *destination = (unsigned int *)((char *)tracks20 + offset + 8);
            destination[1] = source[1];
            destination[2] = source[2];
            destination[3] = source[3];
            destination[4] = source[4];
            destination[5] = source[5];
            destination[6] = source[6];
            destination[7] = source[7];
            void *target = (char *)other->tracks20 + offset + 0x28;
            FEngReference *reference = (FEngReference *)((char *)tracks20 + offset + 0x28);
            if (!target) {
                reference->value8 = 0; reference->valueC = 0; reference->value4 = 0;
            } else {
                if (!reference->value4) reference->ClearOwned();
                reference->value8 = target;
                reference->value4 = 1;
            }
            ++index;
            offset += 0x38;
        }
    } else {
        unsigned int index = 0;
        while (index < (unsigned int)trackCount1C) {
            ((FEScriptTrack *)((char *)tracks20 + offset))->Assign((const FEScriptTrack *)((const char *)other->tracks20 + offset));
            ++index;
            offset += 0x38;
        }
    }
    ((FEngScriptEventStorage *)&eventCount24)->Resize(other->eventCount24);
    unsigned int bytes = (unsigned int)eventCount24 * 12;
    unsigned int *destination = (unsigned int *)events28;
    const unsigned int *source = (const unsigned int *)other->events28;
    unsigned int words = bytes >> 2;
    while (words--) *destination++ = *source++;
    // The retail residual-byte count is always zero because bytes is a multiple of four.
    return this;
}
