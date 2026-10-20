#include "EngineSoundEnvelope.h"
extern int engineSoundCurveTable7EE7EC[];
extern int engineSoundCurveShift82BC8C, engineSoundCurveRemainder82BC90;

// 0x467ff0: the signed multiply/shift implements truncating division by
// -602. The table is addressed relative to 0x7ee7ec, including negative
// indices; the original SAR uses only the low five quotient bits.
int EngineSoundCurveInverse467FF0(int value) {
    if (value == 0) return 0x7fff;
    const int quotient = value / -602;
    const int remainder = value % 602;
    engineSoundCurveShift82BC8C = quotient;
    engineSoundCurveRemainder82BC90 = -remainder;
    int result = engineSoundCurveTable7EE7EC[remainder] >> (quotient & 31);
    engineSoundCurveScratch82BC88 = result;
    if (result < 50) { result = 0; engineSoundCurveScratch82BC88 = 0; }
    return result;
}
