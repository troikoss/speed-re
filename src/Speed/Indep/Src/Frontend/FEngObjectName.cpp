#include "FEngObject.h"
#include "FEngHash.h"
#include "FEngQueueNodes.h"

void HudVisibilityObject::SetName(const char *name) { // Original 0x5176b0
    if (name14) {
        FEngFreeQueueNode(name14);
        name14 = 0;
    }
    hash10 = 0xffffffff;
    if (name) {
        unsigned int length = 0;
        do { ++length; } while (name[length - 1]);
        char *buffer = (char *)FEngAllocateQueueNode(length);
        name14 = buffer;
        char value;
        do {
            value = *name++;
            *buffer++ = value;
        } while (value);
        hash10 = FngHash(name14);
    }
}
