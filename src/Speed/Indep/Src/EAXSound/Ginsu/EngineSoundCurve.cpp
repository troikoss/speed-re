#include "EngineSoundEnvelope.h"

// 0x468050: preserve the scratch write even on zero/early returns. Inputs
// outside the original table's domain retain its unchecked indexing.
int EngineSoundCurveConvert468050(unsigned int value) {
    engineSoundCurveScratch82BC88 = -1;
    if (value == 0) return -10000;
    int shift = 0;
    while ((value & (0x4000u >> shift)) == 0) {
        if (shift == 16) return -10000;
        ++shift;
    }
    const unsigned int masks[12] = {0,1,3,7,15,31,63,127,255,511,1023,2047};
    const int rest = (int)(value - (0x4000u >> shift));
    const int index = shift < 5 ? rest >> (5 - shift) :
        (int)(((unsigned int)rest << (shift - 5)) | masks[shift - 5]);
    const int result = (int)((unsigned int)((-1 - shift) * 602) + engineSoundCurveTable7EE7F0[index]);
    return result < -10000 ? -10000 : result;
}

static int CurveProduct(int first, int second) {
    return (int)((unsigned int)first * (unsigned int)second);
}
static int CurveFloor(int value) { return value < -9600 ? -10000 : value; }

// 0x468160: table interpolation, reflected curves and recursive conversion.
// Preserve the low-byte mode, low 32-bit products and -9600 -> -10000 floor.
int EngineSoundCurve468160(unsigned int kind, unsigned int value, unsigned char mode) {
restart:
    if (kind > 9) return 0;
    int result;
    switch (kind) {
    case 0: {
        const int index = (int)value >> 6;
        result = engineSoundCurveTable7EF7F8[index];
        if (result) {
            const int difference = (int)((unsigned int)engineSoundCurveTable7EF7F8[index + 1] - result);
            result = (int)((unsigned int)result + (CurveProduct(difference, ((value & 0x1e) << 9) | 0x3ff) >> 15));
        }
        break;
    }
    case 1: case 3: case 5: case 7: case 9:
        if (mode) return (int)(0u - EngineSoundCurve468160(kind - 1, value, 1));
        value = 0x7fffu - value;
        --kind;
        goto restart;
    case 2: case 6:
        if (mode) return CurveFloor((int)((unsigned int)EngineSoundCurve468160(kind - 2, value, 1) << 1));
        result = EngineSoundCurve468160(kind - 2, value, 0);
        return CurveProduct(result, result) >> 15;
    case 4: {
        const int index = 0x1ff - ((int)value >> 6);
        const int first = (int)(0x7fffu - engineSoundCurveTable7EF7F8[index]);
        const int second = (int)(0x7fffu - engineSoundCurveTable7EF7F8[index + 1]);
        const int difference = (int)((unsigned int)second - first);
        result = (int)((unsigned int)first + (CurveProduct(difference, ((value & 0x1e) << 9) | 0x3ff) >> 15));
        break;
    }
    case 8:
        result = (int)(0x7fffu - value);
        return mode ? EngineSoundCurveConvert468050(result) : result;
    default: return 0;
    }
    return mode ? CurveFloor(EngineSoundCurveConvert468050(result)) : result;
}
