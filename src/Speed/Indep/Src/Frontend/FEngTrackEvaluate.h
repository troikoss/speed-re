#ifndef UG2_FENG_TRACK_EVALUATE_H
#define UG2_FENG_TRACK_EVALUATE_H
#include "FEngInterpolation.h"

// cdecl track evaluators writing a track's state value to out.
void __cdecl FEngEvaluateTrack(FEScriptTrack *track, int time, void *out); // 0x52bdc0, interpolation 1/3
void __cdecl FEngStepTrack(FEScriptTrack *track, int time, void *out); // 0x519d40, interpolation 0
void __cdecl FEngUpdateTrack(FEScriptTrack *track, int time, void *out); // 0x5354f0, skips/marks bit 7
void __cdecl FEngApplyScriptTrack(FEScript *script, unsigned char index, int time, HudVisibilityObject *object); // 0x541f60
#endif
