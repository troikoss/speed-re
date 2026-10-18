#include "FEngList.h"
#include "FEngGrid.h"

// Original 0x52a910, normal-return body; compiler SEH unwind remains separate.
void FEListObject::ResizeColumns(unsigned int count) {
    if (!count) {
        if (count60) {
            FEngFreeQueueNode(value78);
            value78 = 0;
            count60 = 0;
        }
        ReleaseCells();
        return;
    }
    FEListAxisEntry *axes = (FEListAxisEntry *)FEngAllocateQueueNode(count * 12);
    unsigned int retained = 0;
    if (count60) {
        retained = count60;
        if (count <= retained) retained = count;
        unsigned int *from = (unsigned int *)value78, *to = (unsigned int *)axes;
        unsigned int words = (retained * 12) >> 2;
        while (words--) *to++ = *from++;
        FEngFreeQueueNode(value78);
    }
    unsigned int added = count - retained;
    unsigned int *zero = (unsigned int *)((unsigned int)axes + retained * 12);
    unsigned int words = (added * 12) >> 2;
    while (words--) *zero++ = 0;
    unsigned int allocationCount = count64 * count;
    FEGridCell *cells = 0;
    if (allocationCount) {
        cells = (FEGridCell *)FEngAllocateQueueNode(allocationCount * 0x30);
        if (cells) FEngConstructArray(cells, 0x30, allocationCount, FEngConstructCell);
        if (value90) {
            unsigned int row = 0;
            if (count64) {
                unsigned char *destination = (unsigned char *)cells;
                FEGridCell *tail = (FEGridCell *)((unsigned int)cells + retained * 0x30);
                unsigned int stride = count * 0x30;
                do {
                    unsigned char *source = (unsigned char *)value90 + count60 * row * 0x30;
                    unsigned int *from = (unsigned int *)source, *to = (unsigned int *)destination;
                    // Original copies retained BYTES per row, not retained*0x30.
                    unsigned int remaining = retained >> 2;
                    while (remaining--) *to++ = *from++;
                    unsigned char *fromByte = (unsigned char *)from, *toByte = (unsigned char *)to;
                    remaining = retained & 3;
                    while (remaining--) *toByte++ = *fromByte++;
                    FEngGridResetCells(tail, added);
                    destination += stride;
                    tail = (FEGridCell *)((unsigned int)tail + stride);
                    ++row;
                } while (row < count64);
            }
            FEngFreeQueueNode(value90);
        } else {
            FEngGridResetCells(cells, allocationCount);
        }
    }
    count60 = count;
    value90 = cells;
    value78 = axes;
}

// Original 0x52ab30, normal return. Cell migration uses a byte count here too.
void FEListObject::ResizeRows(unsigned int count) {
    if (!count) {
        if (count64) {
            FEngFreeQueueNode(value7C);
            value7C = 0;
            count64 = 0;
        }
        ReleaseCells();
        return;
    }
    FEListAxisEntry *axes = (FEListAxisEntry *)FEngAllocateQueueNode(count * 12);
    unsigned int retained = 0;
    if (count64) {
        retained = count64;
        if (count <= retained) retained = count;
        unsigned int *from = (unsigned int *)value7C, *to = (unsigned int *)axes;
        unsigned int words = (retained * 12) >> 2;
        while (words--) *to++ = *from++;
        FEngFreeQueueNode(value7C);
    }
    unsigned int added = count - retained;
    unsigned int *zero = (unsigned int *)((unsigned int)axes + retained * 12);
    unsigned int words = (added * 12) >> 2;
    while (words--) *zero++ = 0;
    unsigned int allocationCount = count60 * count;
    FEGridCell *cells = 0;
    if (allocationCount) {
        cells = (FEGridCell *)FEngAllocateQueueNode(allocationCount * 0x30);
        if (cells) FEngConstructArray(cells, 0x30, allocationCount, FEngConstructCell);
        if (value90) {
            unsigned int copiedBytes = count60 * retained;
            unsigned int *from = (unsigned int *)value90, *to = (unsigned int *)cells;
            unsigned int remaining = copiedBytes >> 2;
            while (remaining--) *to++ = *from++;
            unsigned char *fromByte = (unsigned char *)from, *toByte = (unsigned char *)to;
            remaining = copiedBytes & 3;
            while (remaining--) *toByte++ = *fromByte++;
            unsigned int columns = count60;
            FEngGridResetCells((FEGridCell *)((unsigned int)cells + columns * retained * 0x30), columns * added);
            FEngFreeQueueNode(value90);
        } else {
            FEngGridResetCells(cells, allocationCount);
        }
    }
    count64 = count;
    value7C = axes;
    value90 = cells;
}
