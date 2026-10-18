#include "FEngList.h"
#include "FEngColour.h"
#include "FEngQueueNodes.h"

void FEListObject::SetCellType(unsigned int value) { // Original 0x5041e0
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    unsigned char *cell = (unsigned char *)((unsigned int)value90 + offset);
    unsigned int previous = *(unsigned int *)(cell + 0x18);
    if (previous != value) {
        if (previous == 2 && *(void **)(cell + 0x20)) {
            FEngFreeQueueNode(*(void **)(cell + 0x20));
            *(void **)(cell + 0x20) = 0;
        }
        *(unsigned int *)(cell + 0x18) = value;
    }
}
void FEListObject::SetCellColour(FEngColour *value) { // Original 0x516cd0
    unsigned int colour = value->Pack();
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    *(unsigned int *)((unsigned int)value90 + offset) = colour;
}
void FEListObject::SetCellPair4(const unsigned int *value) { // Original 0x516d00
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    unsigned int *out = (unsigned int *)((unsigned int)value90 + offset + 4);
    out[0] = value[0];
    out[1] = value[1];
}
void FEListObject::SetCellTripleC(unsigned int one, unsigned int two, unsigned int three) { // Original 0x516d40
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    unsigned int *out = (unsigned int *)((unsigned int)value90 + offset + 0xc);
    out[0] = one;
    out[1] = two;
    out[2] = three;
}
void FEListObject::SetCellFour20(const unsigned int *value) { // Original 0x516d80
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    unsigned int *out = (unsigned int *)((unsigned int)value90 + offset + 0x20);
    out[0] = value[0];
    out[1] = value[1];
    out[2] = value[2];
    out[3] = value[3];
}
void FEListObject::SetCellText(const unsigned short *value) { // Original 0x516e10
    unsigned int offset = (value8C * count60 + value88) * 0x30;
    void **out = (void **)((unsigned int)value90 + offset + 0x20);
    if (*out) {
        FEngFreeQueueNode(*out);
        *out = 0;
    }
    if (value) {
        unsigned int length = 0;
        while (value[length]) ++length;
        unsigned int bytes = length * 2 + 2;
        void *buffer = FEngAllocateQueueNode(bytes);
        *out = buffer;
        const unsigned int *from = (const unsigned int *)value;
        unsigned int *to = (unsigned int *)buffer;
        unsigned int remaining = bytes >> 2;
        while (remaining--) *to++ = *from++;
        const unsigned char *fromByte = (const unsigned char *)from;
        unsigned char *toByte = (unsigned char *)to;
        remaining = bytes & 3;
        while (remaining--) *toByte++ = *fromByte++;
    }
}
