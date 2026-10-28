#include "EffectEvaluationCopy.h"
// Expand into four separately evidenced entries. No extra recovered helper
// replaces their calls, and scalar/vector cursor quirks remain distinct.
#define FX_COPY_SCALAR 0
#define FX_COPY_RECURSIVE 1
#define FX_COPY_NAME D3DXEffectWalkEvaluationVectors
#define FX_COPY_WALK D3DXEffectWalkEvaluationVectors
// Original 0x65fab5..0x65febe, 1,034 bytes.
#include "EffectEvaluationCopy.inc"
#undef FX_COPY_NAME
#undef FX_COPY_WALK
#undef FX_COPY_SCALAR
#define FX_COPY_SCALAR 1
#define FX_COPY_NAME D3DXEffectWalkEvaluationScalars
#define FX_COPY_WALK D3DXEffectWalkEvaluationScalars
// Original 0x65febf..0x660204, 838 bytes.
#include "EffectEvaluationCopy.inc"
#undef FX_COPY_NAME
#undef FX_COPY_WALK
#undef FX_COPY_RECURSIVE
#undef FX_COPY_SCALAR
#define FX_COPY_SCALAR 0
#define FX_COPY_RECURSIVE 0
#define FX_COPY_NAME D3DXEffectCopyEvaluationVectors
#define FX_COPY_WALK D3DXEffectWalkEvaluationVectors
// Original 0x662ecc..0x663264, 921 bytes.
#include "EffectEvaluationCopy.inc"
#undef FX_COPY_NAME
#undef FX_COPY_WALK
#undef FX_COPY_SCALAR
#define FX_COPY_SCALAR 1
#define FX_COPY_NAME D3DXEffectCopyEvaluationScalars
#define FX_COPY_WALK D3DXEffectWalkEvaluationScalars
// Original 0x663265..0x66356d, 777 bytes.
#include "EffectEvaluationCopy.inc"
