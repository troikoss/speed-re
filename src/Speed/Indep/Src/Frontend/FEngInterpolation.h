#ifndef UG2_FENG_INTERPOLATION_H
#define UG2_FENG_INTERPOLATION_H
#include "FEngScript.h"

// Track interpolation primitives reached from track evaluator 0x52bdc0. All are
// cdecl; quaternion helpers return their hidden output pointer in EAX.
void __cdecl FEngLerpInteger(int a, int b, float fraction, const int *base, int *out); // 0x5050d0
void __cdecl FEngLerpVector2(const float *a, const float *b, float fraction, const float *base, float *out); // 0x505110
FEngQuaternion *__cdecl FEngQuaternionSubtract(FEngQuaternion *out, const FEngQuaternion *a, const FEngQuaternion *b); // 0x4fe830
FEngQuaternion *__cdecl FEngQuaternionScale(FEngQuaternion *out, const FEngQuaternion *q, float scale); // 0x4fe770
FEngQuaternion *__cdecl FEngQuaternionAdd(FEngQuaternion *out, const FEngQuaternion *a, const FEngQuaternion *b); // 0x4fe7d0
void __cdecl FEngQuaternionNormalize(FEngQuaternion *q); // 0x511c80
void __cdecl FEngSlerpQuaternion(const FEngQuaternion *a, const FEngQuaternion *b, float fraction,
                                 const FEngQuaternion *base, FEngQuaternion *out); // 0x519b00
#endif
