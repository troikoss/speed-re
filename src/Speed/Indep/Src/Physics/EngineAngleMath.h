#ifndef UG2_ENGINE_ANGLE_MATH_H
#define UG2_ENGINE_ANGLE_MATH_H
// Original 0x43c2b0 / 0x43c370 return an unspilled x87 value. Keep these entry
// points distinct from the older float-returning engine adapters.
long double EngineSineUnitsExtended(unsigned int units);
long double EngineCosineUnitsExtended(unsigned int units);
#endif
