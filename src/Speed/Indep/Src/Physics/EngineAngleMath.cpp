#include "EngineAngleMath.h"
extern float engineAngleUnit,engineThreeHalfPi,engineTwoPi,engineHalfPi,enginePi;
extern float engineSineC3,engineSineC5,engineSineC7,engineSineC9;

// Original 0x43c2b0: mask before conversion; retain the original powers and
// successive subtraction/addition order without intermediate binary32 stores.
long double EngineSineUnitsExtended(unsigned int units) {
    long double x=static_cast<long double>(units&0xffffu)*engineAngleUnit;
    long double sign=1.0f;
    if (x>=engineThreeHalfPi) x-=engineTwoPi;
    else if (x>=engineHalfPi) { x-=enginePi; sign=-1.0f; }
    long double x2=x*x;
    long double x3=x2*x;
    long double x5=x3*x2;
    long double x7=x5*x2;
    return (((x-x3*engineSineC3)+x5*engineSineC5)-x7*engineSineC7+x7*x2*engineSineC9)*sign;
}
// Original 0x43c370 tail-calls the sine after wrapping the quarter-turn offset.
long double EngineCosineUnitsExtended(unsigned int units) {
    return EngineSineUnitsExtended(units+0x4000u);
}
