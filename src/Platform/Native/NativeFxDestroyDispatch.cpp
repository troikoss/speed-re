#include "../../Speed/Indep/Libs/D3DX/EffectDestruction.h"

// Adapt the existing explicit-object factory boundary to the recovered ECX
// destructor. Kept separate so analysis can still bind that boundary alone.
extern "C" void UG2FxDestroyEffect(D3DXEffectObject *effect) { D3DXDestroyEffect(effect); }
