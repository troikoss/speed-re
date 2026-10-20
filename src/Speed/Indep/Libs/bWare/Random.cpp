#include "Random.h"

// 0x43c1f0: use the previous unsigned seed modulo INT_MAX, then advance
// the shared seed. The unreachable unsigned-FILD correction is not authored.
float bRandomFloat(float scale) {
    const unsigned int value = bRandomSeed7EA624 % 0x7fffffffu;
    const unsigned int mixed = bRandomSeed7EA624 ^ 0x1d872b41;
    const unsigned int shifted = (mixed >> 5) ^ mixed;
    bRandomSeed7EA624 = (shifted << 27) ^ shifted ^ mixed;
    return ((double)(int)value * bRandomFloatScale786CE8) * scale;
}
