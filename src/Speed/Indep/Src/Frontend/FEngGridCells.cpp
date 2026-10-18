#include "FEngGrid.h"

// Original 0x518c60. Old pointers survive allocation/free callback changes;
// current fields reload for zeroing, table creation and type-2 cell migration.
void FEGridObject::ResizeStringPool(unsigned int count, unsigned int capacity) {
    unsigned int previousData = valueA0;
    unsigned int *previousTable = value9C;
    value90 = 0;
    value98 = 0;
    value94 = 0;
    valueA0 = 0;
    value9C = 0;
    if (count && capacity) {
        unsigned int bytes = count * capacity * 2;
        valueA0 = (unsigned int)FEngGridAllocateCells(bytes);
        value9C = (unsigned int *)FEngGridAllocateCells(count * 4);
        // Original inline REP STOSD/STOSB over the wrapped byte count.
        unsigned int *destination = (unsigned int *)valueA0;
        unsigned int words = bytes >> 2;
        while (words) { *destination++ = 0; --words; }
        unsigned char *tail = (unsigned char *)destination;
        unsigned int remainder = bytes & 3;
        while (remainder) { *tail++ = 0; --remainder; }
        unsigned int offset = 0;
        for (unsigned int i = 0; i < count; ++i) {
            value9C[i] = valueA0 + offset;
            offset += capacity * 2;
        }
        value90 = count;
        value94 = capacity;
        if (previousData && previousTable) {
            for (unsigned int row = 0; row < count64; ++row) {
                for (unsigned int column = 0; column < count60; ++column) {
                    FEGridCell *cell = cells8C + count60 * row + column;
                    if (cell->value0[6] == 2) {
                        unsigned short *output = (unsigned short *)value9C[value98];
                        const unsigned short *input = (const unsigned short *)cell->value0[8];
                        ++value98;
                        *output = 0;
                        cell->value0[8] = (unsigned int)output;
                        if (input) {
                            unsigned int copied = 0;
                            while (*input) {
                                if (capacity - 1 == copied) break;
                                *output++ = *input++;
                                ++copied;
                            }
                        }
                        *output = 0;
                    }
                }
            }
        }
    }
    if (previousData) FEngGridFreeCells((void *)previousData);
    if (previousTable) FEngGridFreeCells(previousTable);
}

// Original 0x5196b0. The coordinate projections are inline at each call site.
void FEGridObject::SetCellValue0(unsigned int x, unsigned int y, unsigned int value,
                             unsigned int columns, unsigned int rows) {
    unsigned int endRow = y + rows;
    unsigned int endColumn = x + columns;
    for (unsigned int row = y; row < endRow; ++row) {
        for (unsigned int column = x; column < endColumn; ++column) {
            int logical = value6C;
            int physicalColumns = (int)count60;
            int projectedColumn;
            if (!logical) projectedColumn = -1;
            else {
                int coordinate = (int)column;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value74);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalColumns < 2) projectedColumn = 0;
                    else projectedColumn = (int)((unsigned int)physicalColumns - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalColumns));
                } else projectedColumn = difference % physicalColumns;
            }
            logical = value70;
            int physicalRows = (int)count64;
            int projectedRow;
            if (!logical) projectedRow = -1;
            else {
                int coordinate = (int)row;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value78);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalRows < 2) projectedRow = 0;
                    else projectedRow = (int)((unsigned int)physicalRows - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalRows));
                } else projectedRow = difference % physicalRows;
            }
            unsigned int index = (unsigned int)physicalColumns * (unsigned int)projectedRow + (unsigned int)projectedColumn;
            FEGridCell *cell = (FEGridCell *)((unsigned char *)cells8C + index * 0x30);
            cell->value0[0] = value;
        }
    }
}

// Original 0x5197d0. The coordinate projections are inline at each call site.
void FEGridObject::SetCellPair4(unsigned int x, unsigned int y, const unsigned int *pair,
                             unsigned int columns, unsigned int rows) {
    unsigned int endRow = y + rows;
    unsigned int endColumn = x + columns;
    for (unsigned int row = y; row < endRow; ++row) {
        for (unsigned int column = x; column < endColumn; ++column) {
            int logical = value6C;
            int physicalColumns = (int)count60;
            int projectedColumn;
            if (!logical) projectedColumn = -1;
            else {
                int coordinate = (int)column;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value74);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalColumns < 2) projectedColumn = 0;
                    else projectedColumn = (int)((unsigned int)physicalColumns - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalColumns));
                } else projectedColumn = difference % physicalColumns;
            }
            logical = value70;
            int physicalRows = (int)count64;
            int projectedRow;
            if (!logical) projectedRow = -1;
            else {
                int coordinate = (int)row;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value78);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalRows < 2) projectedRow = 0;
                    else projectedRow = (int)((unsigned int)physicalRows - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalRows));
                } else projectedRow = difference % physicalRows;
            }
            unsigned int index = (unsigned int)physicalColumns * (unsigned int)projectedRow + (unsigned int)projectedColumn;
            FEGridCell *cell = (FEGridCell *)((unsigned char *)cells8C + index * 0x30);
            cell->value0[1] = pair[0];
            cell->value0[2] = pair[1];
        }
    }
}

// Original 0x5198f0. The coordinate projections are inline at each call site.
void FEGridObject::SetCellValue1C(unsigned int x, unsigned int y, unsigned int value,
                             unsigned int columns, unsigned int rows) {
    unsigned int endRow = y + rows;
    unsigned int endColumn = x + columns;
    for (unsigned int row = y; row < endRow; ++row) {
        for (unsigned int column = x; column < endColumn; ++column) {
            int logical = value6C;
            int physicalColumns = (int)count60;
            int projectedColumn;
            if (!logical) projectedColumn = -1;
            else {
                int coordinate = (int)column;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value74);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalColumns < 2) projectedColumn = 0;
                    else projectedColumn = (int)((unsigned int)physicalColumns - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalColumns));
                } else projectedColumn = difference % physicalColumns;
            }
            logical = value70;
            int physicalRows = (int)count64;
            int projectedRow;
            if (!logical) projectedRow = -1;
            else {
                int coordinate = (int)row;
                if (coordinate >= logical) coordinate %= logical;
                int difference = (int)((unsigned int)coordinate - value78);
                if (difference < 0) difference = (int)((unsigned int)difference + (unsigned int)logical);
                if (difference < 0) {
                    if (physicalRows < 2) projectedRow = 0;
                    else projectedRow = (int)((unsigned int)physicalRows - (unsigned int)(((int)(0u - (unsigned int)difference)) % physicalRows));
                } else projectedRow = difference % physicalRows;
            }
            unsigned int index = (unsigned int)physicalColumns * (unsigned int)projectedRow + (unsigned int)projectedColumn;
            FEGridCell *cell = (FEGridCell *)((unsigned char *)cells8C + index * 0x30);
            cell->value0[7] = value;
        }
    }
}
