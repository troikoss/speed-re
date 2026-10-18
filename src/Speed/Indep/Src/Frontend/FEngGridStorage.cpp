#include "FEngGrid.h"

// Original 0x518bb0. The clamped selection is local; +0x7c is not changed.
void FEGridObject::SetLogicalColumns(unsigned int total) {
    unsigned int selected = value7C;
    value6C = (int)total;
    if (selected >= total) selected = total ? total - 1 : 0;
    if (flags68 & 8) {
        int origin = (int)(selected - (count60 >> 1));
        if (origin >= 0) { value74 = origin % (int)total; return; }
        if ((int)total > 1) {
            value74 = total - (unsigned int)(((int)(0u - (unsigned int)origin)) % (int)total);
            return;
        }
        selected = 0;
    }
    value74 = selected;
}

// Original 0x518c00; separate axis body and call boundary.
void FEGridObject::SetLogicalRows(unsigned int total) {
    unsigned int selected = value80;
    value70 = (int)total;
    if (selected >= total) selected = total ? total - 1 : 0;
    if (flags68 & 8) {
        int origin = (int)(selected - (count64 >> 1));
        if (origin >= 0) { value78 = origin % (int)total; return; }
        if ((int)total > 1) {
            value78 = total - (unsigned int)(((int)(0u - (unsigned int)origin)) % (int)total);
            return;
        }
        selected = 0;
    }
    value78 = selected;
}

// Original 0x504290: initialize every DWORD, including the final three words.
void FEngGridResetCells(FEGridCell *cells, unsigned int count) {
    while (count) {
        cells->value0[0] = 0xffffffff;
        cells->value0[1] = 0x3f800000;
        cells->value0[2] = 0x3f800000;
        cells->value0[3] = 0;
        cells->value0[4] = 0;
        cells->value0[5] = 0;
        cells->value0[6] = 0;
        cells->value0[8] = 0;
        cells->value0[9] = 0;
        cells->value0[10] = 0x3f800000;
        cells->value0[11] = 0x3f800000;
        cells->value0[7] = 0;
        ++cells;
        --count;
    }
}

// Original 0x52bb30, normal allocation/resize path. Preserve the new-width
// row copy even when it exceeds the retained columns; do not repair that quirk.
void FEGridObject::ResizeCells(unsigned int columns, unsigned int rows) {
    FEGridCell *previousCells = cells8C;
    unsigned int previousColumns = count60;
    count60 = columns;
    unsigned int allocationCount = columns * rows;
    unsigned int previousRows = count64;
    count64 = rows;
    FEGridCell *allocated = (FEGridCell *)FEngGridAllocateCells(allocationCount * 0x30);
    if (allocated && (int)(allocationCount - 1) >= 0) {
        FEGridCell *cell = allocated;
        unsigned int remaining = allocationCount;
        do {
            cell->value0[0] = 0;
            cell->value0[1] = 0x3f800000;
            cell->value0[2] = 0x3f800000;
            cell->value0[3] = 0;
            cell->value0[4] = 0;
            cell->value0[5] = 0;
            cell->value0[6] = 0;
            cell->value0[8] = 0;
            cell->value0[9] = 0xffffffff;
            ++cell;
        } while (--remaining);
    }
    cells8C = allocated;
    FEngGridResetCells(allocated, count60 * count64);
    SetLogicalColumns(count60);
    SetLogicalRows(count64);
    if (flags68 & 1) {
        unsigned int retainedColumns = previousColumns;
        if (count60 < retainedColumns) retainedColumns = count60;
        unsigned int retainedRows = previousRows;
        if (count64 < retainedRows) retainedRows = count64;
        if (previousCells) {
            unsigned int row = 0;
            if (retainedRows) {
                FEGridCell *source = previousCells;
                FEGridCell *discarded = previousCells + retainedColumns;
                do {
                    // Original inline REP MOVSD; byte count is a multiple of 16.
                    unsigned int words = (count60 * 0x30) >> 2;
                    unsigned int *destination = (unsigned int *)(cells8C + count60 * row);
                    const unsigned int *input = (const unsigned int *)source;
                    while (words) { *destination++ = *input++; --words; }
                    if (retainedColumns < previousColumns) {
                        FEGridCell *cell = discarded;
                        unsigned int remaining = previousColumns - retainedColumns;
                        do {
                            if (cell->value0[6] == 2) {
                                unsigned int payload = cell->value0[8];
                                --value98;
                                value9C[value98] = payload;
                            }
                            ++cell;
                        } while (--remaining);
                    }
                    source += previousColumns;
                    discarded += previousColumns;
                    ++row;
                } while (row < retainedRows);
            }
            if (retainedRows < previousRows) {
                FEGridCell *cell = previousCells + retainedRows * previousColumns;
                unsigned int remainingRows = previousRows - retainedRows;
                do {
                    unsigned int remainingColumns = previousColumns;
                    FEGridCell *next = cell + previousColumns;
                    while (remainingColumns) {
                        if (cell->value0[6] == 2) {
                            unsigned int payload = cell->value0[8];
                            --value98;
                            value9C[value98] = payload;
                        }
                        ++cell;
                        --remainingColumns;
                    }
                    cell = next;
                } while (--remainingRows);
            }
            FEngGridFreeCells(previousCells);
        }
    }
    flags68 |= 1;
}
