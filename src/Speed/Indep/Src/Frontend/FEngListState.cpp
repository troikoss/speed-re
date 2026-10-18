#include "FEngList.h"
extern "C" double __cdecl fabs(double);
// Compiler intrinsic spelling only: original Tick uses x87 FABS on its wide sum.
#if defined(__clang__) || defined(__GNUC__)
#define FENG_LIST_FABS __builtin_fabsl
#else
#define FENG_LIST_FABS fabs
#endif

// Original 0x504180 / 0x5041b0: unsigned selection clamp, including empty axes.
void FEListObject::SetColumn(unsigned int value) {
    if (count60) value88 = value < count60 ? value : count60 - 1;
    else value88 = 0;
}
void FEListObject::SetRow(unsigned int value) {
    if (count64) value8C = value < count64 ? value : count64 - 1;
    else value8C = 0;
}
// Original 0x504230: advance columns first and wrap both coordinates.
void FEListObject::AdvanceCell() {
    ++value88;
    if (value88 >= count60) {
        value88 = 0;
        ++value8C;
        if (value8C >= count64) value8C = 0;
    }
}
void FEListObject::SetWrap(unsigned char value) { // 0x504270
    if (value) value5C &= ~4u;
    else value5C |= 4;
}

// Original 0x504360: the accumulator is not rounded by the entry+4 store.
void FEListObject::CalculateOffsets() {
    FEListAxisEntry *entry = value78;
    unsigned int i = 0;
    long double offset = 0.0;
    if (count60) {
        do {
            entry->value4 = (float)offset;
            offset = offset + entry->value0;
            ++entry;
            ++i;
        } while (i < count60);
    }
    entry = value7C;
    i = 0;
    offset = 0.0;
    if (count64) {
        do {
            entry->value4 = (float)offset;
            offset = offset + entry->value0;
            ++entry;
            ++i;
        } while (i < count64);
    }
}

// Original 0x5043b0, a separate call from Tick's overshoot path.
void FEListObject::FinishMotion() {
    value70 = value94;
    value74 = value98;
    unsigned int flags = value5C;
    value5C = flags & 0xffffff9d;
    if (!value88) { value5C = flags & 0xffffff95; value70 = 0; }
    if (!value8C) { value5C &= 0xffffffef; value74 = 0; }
}

// Original 0x517230, including the single-precision vertical step spill.
void FEListObject::Tick(float milliseconds) {
    long double phase = (long double)milliseconds * valueA8 + valueA4;
    valueA4 = (float)phase;
    if (phase < 0.0) {
        valueA4 = 0;
        valueA8 = -valueA8;
    } else if (phase > 1.0) {
        valueA4 = 1;
        valueA8 = -valueA8;
    }
    if (value5C & 2) {
        long double dx = value9C, dy = valueA0;
        long double step = FENG_LIST_FABS(dy * value84 + dx * value80) * milliseconds;
        float verticalStep = (float)(dy * step);
        long double x = dx * step + value70;
        value70 = (float)x;
        long double y = (long double)verticalStep + value74;
        value74 = (float)y;
        if ((dy * value98 + dx * value94) - (y * dy + x * dx) < 0.0)
            FinishMotion();
    }
}

#undef FENG_LIST_FABS
