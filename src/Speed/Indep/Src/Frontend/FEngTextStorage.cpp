#include "FEngObjectFactory.h"

void FEngCopyText(unsigned short *destination, const unsigned short *source) { // Original 0x511630
    if (destination) {
        if (source) {
            unsigned short value = *source;
            while (value) {
                *destination = value;
                ++source;
                ++destination;
                value = *source;
            }
        }
        *destination = 0;
    }
}

void FEngTextStorage::Grow(unsigned int request) { // Original 0x51de90
    unsigned int length = 0;
    if (text0) {
        while (text0[length]) ++length;
    }
    if (request > length) {
        capacity4 = request;
        unsigned short *buffer = (unsigned short *)FEngAllocateQueueNode(request * 2 + 2);
        FEngCopyText(buffer, text0);
        FEngFreeQueueNode(text0);
        text0 = buffer;
    }
}

unsigned short *FEngTextStorage::GetBuffer(unsigned int request) { // Original 0x51dee0
    if (request > capacity4) Grow(request);
    return text0;
}

FEngTextStorage *FEngTextStorage::Assign(const unsigned short *source) { // Original 0x52d610
    if (source) {
        unsigned int length = 0;
        while (source[length]) ++length;
        if (length + 1 > capacity4) Grow(length + 1);
        FEngCopyText(text0, source);
    }
    return this;
}

void FEngTextStorage::Destruct() { // Original 0x50d390
    if (text0) FEngFreeQueueNode(text0);
}
