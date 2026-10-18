#include "FEngList.h"
extern "C" double sqrt(double);

// Original 0x516e90. Axis branches remain separate, including wrap arithmetic
// and float stores inside the wrap loops; no shared navigation helper existed.
// Wide temporaries preserve arithmetic between the observed float field stores.
void FEListObject::Navigate(int x, int y) {
    unsigned int flags = value5C;
    if ((flags & 0x60) == 0x60) return;
    if (flags & 0x20) x = 0;
    if (flags & 0x40) y = 0;
    if (!x && !y) return;
    if (x && !(flags & 0x20)) {
        unsigned int count = count60;
        unsigned int selected = value88 + x;
        FEListAxisEntry *entries;
        if (!(flags & 4) && ((int)selected < 0 || (int)selected >= (int)count)) {
            entries = value78;
            if ((int)selected < 0) {
                unsigned int i = count + selected;
                value94 = entries[i].value4;
                value70 = entries[i].value4;
                do {
                    unsigned int previous = i;
                    i = (i + 1) % count;
                    value70 = entries[previous].value0 + value70;
                } while (i != value88);
            } else {
                unsigned int i = value88;
                value94 = value70;
                do {
                    unsigned int previous = i % count;
                    ++i;
                    value94 = entries[previous].value0 + value94;
                } while (i != selected);
            }
            value5C = flags | 8;
            value88 = selected % count;
        } else {
            if (flags & 4) {
                if ((int)selected >= (int)count) selected = count - 1;
                if ((int)selected < 0) selected = 0;
            }
            entries = value78;
            value88 = selected;
            value94 = entries[selected].value4;
        }
        unsigned int i = value88;
        double remaining = 0.0f;
        if (i < count) {
            FEListAxisEntry *entry = entries + i;
            unsigned int size = count60 - i;
            do { remaining = remaining + entry->value0; ++entry; } while (--size);
        }
        if (!(value5C & 4)) {
            if (remaining < value68) value5C |= 8;
        } else if (remaining < value68) {
            value94 = value94 - (value68 - remaining);
        }
        value5C |= 0x20;
    }
    if (y && !(value5C & 0x40)) {
        flags = value5C;
        unsigned int selected = value8C + y;
        unsigned int count = count64;
        FEListAxisEntry *entries;
        if (!(flags & 4) && ((int)selected < 0 || (int)selected >= (int)count)) {
            entries = value7C;
            if ((int)selected < 0) {
                unsigned int i = count + selected;
                value98 = entries[i].value4;
                value74 = entries[i].value4;
                do {
                    unsigned int previous = i;
                    i = (i + 1) % count;
                    value74 = entries[previous].value0 + value74;
                } while (i != value8C);
            } else {
                unsigned int i = value8C;
                value98 = value74;
                do {
                    unsigned int previous = i % count;
                    ++i;
                    value98 = entries[previous].value0 + value98;
                } while (i != selected);
            }
            value5C = flags | 8;
            value8C = selected % count;
        } else {
            if (flags & 4) {
                if ((int)selected >= (int)count) selected = count - 1;
                if ((int)selected < 0) selected = 0;
            }
            entries = value7C;
            value8C = selected;
            value98 = entries[selected].value4;
        }
        unsigned int i = value8C;
        double remaining = 0.0f;
        if (i < count) {
            FEListAxisEntry *entry = entries + i;
            unsigned int size = count64 - i;
            do { remaining = remaining + entry->value0; ++entry; } while (--size);
        }
        if (!(value5C & 4)) {
            if (remaining < value6C) value5C |= 0x10;
        } else if (remaining < value6C) {
            value98 = value98 - (value6C - remaining);
        }
        value5C |= 0x40;
    }
    value5C |= 2;
    double dx = value94 - value70;
    double dy = value98 - value74;
    value9C = dx;
    valueA0 = dy;
    double length = sqrt(dy * dy + dx * dx);
    // Original x87 parity test also takes this branch for unordered length.
    if (!(length < 0.1f)) {
        double scale = 1.0f / length;
        value9C = scale * dx;
        valueA0 = dy * scale;
    } else {
        value70 = value94;
        value74 = value98;
        flags = value5C;
        value5C = flags & 0xffffff9d;
        if (!value88) { value5C = flags & 0xffffff95; value70 = 0; }
        if (!value8C) { value5C &= 0xffffffef; value74 = 0; }
    }
}
