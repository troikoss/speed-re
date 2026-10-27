#include "../../Speed/Indep/Libs/D3DX/EffectSetup.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif

// Original binary double at 0x7a1e58, used before truncating a selector index.
extern "C" const double ug2FxIndexBias7A1E58=0.001;
extern "C" int UG2FxSetupEffect(D3DXEffectObject *effect) { return effect->Setup(); }

// These shader preparation/evaluation bodies remain unrecovered. Metadata-only
// setup can run; reaching one of these boundaries must not fabricate success.
extern "C" int UG2FxPrepareGroupA(D3DXEffectObject *,unsigned int) { abort(); }
extern "C" int UG2FxPrepareGroupB(D3DXEffectObject *,unsigned int) { abort(); }
extern "C" int UG2FxEvaluateRecord(D3DXEffectObject *,unsigned int,unsigned int) { abort(); }
