#ifndef UG2_BWARE_RANDOM_H
#define UG2_BWARE_RANDOM_H
// UG2 PC v1.2 shared seed; bounded integer body is in CollisionShape.cpp.
extern unsigned int bRandomSeed7EA624;
unsigned int bRandom(unsigned int range); // 0x43c1c0
float bRandomFloat(float scale); // 0x43c1f0, return remains in ST0
extern float bRandomFloatScale786CE8;
#endif
