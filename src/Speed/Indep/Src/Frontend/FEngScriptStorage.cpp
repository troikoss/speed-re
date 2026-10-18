#include "FEngScript.h"
#include "FEngHash.h"
#include "FEngQueueNodes.h"

void FEScript::SetName(const char *name) { // Original 0x518730
    if (name2C) {
        FEngFreeQueueNode(name2C);
        name2C = 0;
    }
    hash30 = 0xffffffff;
    if (name) {
        unsigned int length = 0;
        do { ++length; } while (name[length - 1]);
        char *buffer = (char *)FEngAllocateQueueNode(length);
        name2C = buffer;
        char c;
        do {
            c = *name++;
            *buffer++ = c;
        } while (c);
        hash30 = FngHash(name2C);
    }
}

// Original 0x519a10. Preserve signed selection, DWORD-forward copying, fresh
// descriptor loads after heap calls/copying, and wrapped byte-count arithmetic.
void FEngScriptEventStorage::Resize(int count) {
    if (count != count0) {
        if (!count) {
            if (data4) FEngFreeQueueNode(data4);
            data4 = 0;
            count0 = 0;
            return;
        }
        unsigned int bytes = (unsigned int)count * 12;
        unsigned int *buffer = (unsigned int *)FEngAllocateQueueNode(bytes);
        unsigned int *destination = buffer;
        unsigned int *source = (unsigned int *)data4;
        int oldCount = count0;
        if (count < oldCount) {
            unsigned int words = bytes >> 2;
            while (words--) *destination++ = *source++;
        } else {
            unsigned int words = ((unsigned int)oldCount * 12) >> 2;
            while (words--) *destination++ = *source++;
            int freshCount = count0;
            destination = (unsigned int *)((unsigned int)buffer + (unsigned int)freshCount * 12);
            words = (((unsigned int)count - (unsigned int)freshCount) * 12) >> 2;
            while (words--) *destination++ = 0;
        }
        FEngFreeQueueNode(data4);
        data4 = (FEScriptEvent *)buffer;
        count0 = count;
    }
}
