#include "FEngGrid.h"
#include "FEngHash.h"

// Original 0x504790. Do not normalize negative exact multiples to zero.
int FEngGridWrapIndex(int value, int total) {
    if (value >= 0) return value % total;
    if (total > 1) {
        int magnitude = (int)(0u - (unsigned int)value);
        return (int)((unsigned int)total - (unsigned int)(magnitude % total));
    }
    return 0;
}

// Original 0x518e70. Rejected movement can issue two distinct owner messages.
unsigned char FEGridObject::CanShift(int delta, int selection, int offset, int total, int visible) {
    unsigned int flags = flags68;
    if ((flags & 4) && total <= visible) {
        owner5C->GameMessage(FngHash((const char *)0x79b14c), this, 0xff, 0);
    } else {
        if (!(flags & 2)) return 1;
        int candidate = (int)((unsigned int)((flags & 4) ? selection : offset) + (unsigned int)delta);
        if (candidate < 0) {
            owner5C->GameMessage(FngHash((const char *)0x79b14c), this, 0xff, 0);
            return 0;
        }
        int limit = (flags & 4) ? (int)((unsigned int)total - (unsigned int)visible) : total;
        if (candidate < limit) return 1;
    }
    owner5C->GameMessage(FngHash((const char *)0x79b140), this, 0xff, 0);
    return 0;
}

// Original 0x504e40: inline wraps remain inline; only the last path calls 0x504790.
unsigned char FEGridObject::MoveSelection(int delta, unsigned int *selection, unsigned int *offset,
                                         int total, unsigned int visible) {
    unsigned int flags = flags68;
    if (flags & 8) {
        int value = (int)(*selection + (unsigned int)delta);
        unsigned int wrapped;
        if (value < 0) {
            if (total < 2) wrapped = 0;
            else wrapped = (unsigned int)total - (unsigned int)(((int)(0u - (unsigned int)value)) % total);
        } else wrapped = value % total;
        *selection = wrapped;
        value = (int)(*offset + (unsigned int)delta);
        if (value >= 0) *offset = value % total;
        else if (total > 1) *offset = (unsigned int)total - (unsigned int)(((int)(0u - (unsigned int)value)) % total);
        else *offset = 0;
        return 1;
    }
    if (!(flags & 4) || !(flags & 2)) {
        unsigned int previous = *offset;
        int value = (int)(previous + (unsigned int)delta);
        unsigned int wrapped;
        if (value < 0) {
            if (total < 2) wrapped = 0;
            else wrapped = (unsigned int)total - (unsigned int)(((int)(0u - (unsigned int)value)) % total);
        } else wrapped = value % total;
        *offset = wrapped;
        if (delta < 0) {
            if (*selection == previous) { *selection = wrapped; return 1; }
        } else {
            unsigned int selected = *selection;
            if (selected != previous) {
                unsigned int distance = selected < wrapped ? wrapped - selected : wrapped - selected + (unsigned int)total;
                if (visible <= distance) {
                    *selection = FEngGridWrapIndex((int)(selected + (unsigned int)delta), total);
                    return 1;
                }
            }
        }
        return 0;
    }
    int value = (int)(*selection + (unsigned int)delta);
    unsigned int wrapped;
    if (value >= 0) wrapped = value % total;
    else if (total > 1) wrapped = (unsigned int)total - (unsigned int)(((int)(0u - (unsigned int)value)) % total);
    else wrapped = 0;
    *selection = wrapped;
    *offset = wrapped;
    return 1;
}

// Original 0x518f20. Preserve the separate callback/owner paths and shallow
// 0x30-byte copies; only payload DWORD +0x20 is saved for the exposed edge.
unsigned char FEGridObject::ShiftAxis(int delta, unsigned int *selection, unsigned int *offset,
                                     int total, unsigned int visible, unsigned char horizontal) {
    if (!delta) return 0;
    if (!CanShift(delta, (int)*selection, (int)*offset, total, (int)visible)) return 0;
    if (!MoveSelection(delta, selection, offset, total, visible)) return 0;
    if (!total) return 1;
    if (horizontal) {
        if (delta > 0) {
            unsigned int copies = count60 - 1;
            int column = FEngGridWrapIndex((int)(value74 + count60 - 1), value6C);
            if (callbackC0) {
                for (unsigned int row = 0; row < count64; ++row) {
                    unsigned int saved = cells8C[row * count60].value0[8];
                    unsigned int i = 0;
                    while (i < copies) {
                        unsigned int index = row * count60 + i;
                        ++i;
                        cells8C[index] = cells8C[index + 1];
                    }
                    cells8C[row * count60 + i].value0[8] = saved;
                    int y = (int)(value78 + row), rows = value70;
                    if (y < 0) y = rows > 1 ? (int)((unsigned int)rows - (unsigned int)(((int)(0u - (unsigned int)y)) % rows)) : 0;
                    else y %= rows;
                    callbackC0(contextC4, this, column, y);
                }
            } else if (owner5C) {
                for (unsigned int row = 0; row < count64; ++row) {
                    unsigned int saved = cells8C[row * count60].value0[8];
                    unsigned int i = 0;
                    while (i < copies) {
                        unsigned int index = row * count60 + i;
                        ++i;
                        cells8C[index] = cells8C[index + 1];
                    }
                    cells8C[row * count60 + i].value0[8] = saved;
                    owner5C->RefillGrid(this, column, FEngGridWrapIndex((int)(value78 + row), value70));
                }
            }
        } else {
            if (callbackC0) {
                unsigned int stride = 0x30;
                for (unsigned int row = 0; row < count64; ++row, stride += 0x30) {
                    unsigned int count = count60;
                    unsigned int saved = ((unsigned int *)((unsigned char *)cells8C + count * stride - 0x10))[0];
                    while (--count) {
                        unsigned int index = row * count60 + count;
                        cells8C[index] = cells8C[index - 1];
                    }
                    cells8C[row * count60].value0[8] = saved;
                    int y = (int)(value78 + row), rows = value70;
                    if (y < 0) y = rows > 1 ? (int)((unsigned int)rows - (unsigned int)(((int)(0u - (unsigned int)y)) % rows)) : 0;
                    else y %= rows;
                    callbackC0(contextC4, this, (int)value74, y);
                }
            } else if (owner5C) {
                unsigned int stride = 0x30;
                for (unsigned int row = 0; row < count64; ++row, stride += 0x30) {
                    unsigned int count = count60;
                    unsigned int saved = ((unsigned int *)((unsigned char *)cells8C + count * stride - 0x10))[0];
                    while (--count) {
                        unsigned int index = row * count60 + count;
                        cells8C[index] = cells8C[index - 1];
                    }
                    cells8C[row * count60].value0[8] = saved;
                    owner5C->RefillGrid(this, (int)value74, FEngGridWrapIndex((int)(value78 + row), value70));
                }
            }
        }
    } else {
        if (delta > 0) {
            unsigned int rows = count64;
            int row = FEngGridWrapIndex((int)(value78 + rows - 1), value70);
            if (callbackC0) {
                unsigned int stride = 0;
                for (unsigned int column = 0; column < count60; ++column, stride += 0x30) {
                    unsigned int saved = ((unsigned int *)((unsigned char *)cells8C + stride))[8];
                    unsigned int i = 0;
                    if (rows != 1) {
                        do {
                            unsigned int index = count60 * i + column;
                            ++i;
                            cells8C[index] = cells8C[index + count60];
                        } while (i < rows - 1);
                    }
                    cells8C[count60 * i + column].value0[8] = saved;
                    int x = (int)(value74 + column), columns = value6C;
                    if (x < 0) x = columns > 1 ? (int)((unsigned int)columns - (unsigned int)(((int)(0u - (unsigned int)x)) % columns)) : 0;
                    else x %= columns;
                    callbackC0(contextC4, this, x, row);
                }
            } else if (owner5C) {
                unsigned int stride = 0;
                for (unsigned int column = 0; column < count60; ++column, stride += 0x30) {
                    unsigned int saved = ((unsigned int *)((unsigned char *)cells8C + stride))[8];
                    unsigned int i = 0;
                    if (rows != 1) {
                        do {
                            unsigned int index = count60 * i + column;
                            ++i;
                            cells8C[index] = cells8C[index + count60];
                        } while (i < rows - 1);
                    }
                    cells8C[count60 * i + column].value0[8] = saved;
                    owner5C->RefillGrid(this, FEngGridWrapIndex((int)(value74 + column), value6C), row);
                }
            }
        } else {
            if (callbackC0) {
                unsigned int stride = 0;
                for (unsigned int column = 0; column < count60; ++column, stride += 0x30) {
                    unsigned int i = count64 - 1;
                    unsigned int saved = cells8C[count60 * i + column].value0[8];
                    while (i) {
                        unsigned int index = count60 * i + column;
                        --i;
                        cells8C[index] = cells8C[index - count60];
                    }
                    ((unsigned int *)((unsigned char *)cells8C + stride))[8] = saved;
                    int y = (int)(value78 + i), rows = value70;
                    if (y < 0) y = rows > 1 ? (int)((unsigned int)rows - (unsigned int)(((int)(0u - (unsigned int)y)) % rows)) : 0;
                    else y %= rows;
                    callbackC0(contextC4, this, (int)value74, y);
                }
            } else if (owner5C) {
                unsigned int stride = 0;
                for (unsigned int column = 0; column < count60; ++column, stride += 0x30) {
                    unsigned int i = count64 - 1;
                    unsigned int saved = cells8C[count60 * i + column].value0[8];
                    while (i) {
                        unsigned int index = count60 * i + column;
                        --i;
                        cells8C[index] = cells8C[index - count60];
                    }
                    ((unsigned int *)((unsigned char *)cells8C + stride))[8] = saved;
                    owner5C->RefillGrid(this, (int)value74, FEngGridWrapIndex((int)(value78 + i), value70));
                }
            }
        }
    }
    return 1;
}

// Original 0x52bd70: second-axis inputs are read after first-axis callbacks.
void FEGridObject::Navigate(int x, int y) {
    ShiftAxis(x, &value74, &value7C, value6C, count60, 1);
    ShiftAxis(y, &value78, &value80, value70, count64, 0);
}
