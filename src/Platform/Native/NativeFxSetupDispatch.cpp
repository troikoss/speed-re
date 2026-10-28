#include "../../Speed/Indep/Libs/D3DX/EffectSetup.h"
// Original binary double at 0x7a1e58, used before truncating a selector index.
extern "C" const double ug2FxIndexBias7A1E58=0.001;
extern "C" int UG2FxSetupEffect(D3DXEffectObject *effect) { return effect->Setup(); }

// Group preparation reaches recovered bodies; shader readback still requires
// GPU interfaces. Evaluation reaches the recovered VM/operation; its four CRT
// math dependencies remain explicit unsupported boundaries.
extern "C" int UG2FxPrepareGroupA(D3DXEffectObject *effect,unsigned int group) { return effect->PrepareGroupA(group); }
extern "C" int UG2FxPrepareGroupB(D3DXEffectObject *effect,unsigned int group) { return effect->PrepareGroupB(group); }
extern "C" int UG2FxEvaluateRecord(D3DXEffectObject *effect,unsigned int group,unsigned int record) { return effect->EvaluateRecord(group,record); }
