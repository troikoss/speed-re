#include "FEngGrid.h"

void *__fastcall FEngConstructCell(void *value) { // Original 0x504150
    FEGridCell *cell = (FEGridCell *)value;
    cell->value0[0] = 0;
    cell->value0[1] = 0x3f800000;
    cell->value0[2] = 0x3f800000;
    cell->value0[3] = 0;
    cell->value0[4] = 0;
    cell->value0[5] = 0;
    cell->value0[6] = 0;
    cell->value0[8] = 0;
    cell->value0[9] = 0xffffffff;
    return value; // Words +0x1c/+0x28/+0x2c are untouched.
}

void __stdcall FEngConstructArray(void *memory, unsigned int stride, unsigned int count,
                                 FEngArrayConstructor constructor) { // Original 0x401650
    if ((int)(count - 1) >= 0) {
        do {
            constructor(memory);
            memory = (void *)((unsigned int)memory + stride);
        } while (--count);
    }
}
