#include "EffectSort.h"
// Compile the separately evidenced original bodies with their stage-specific
// constants. Expansion preserves two member entries and all original callees.
#define FX_PREPARE_METHOD PrepareGroupA
#define FX_PREPARE_CAPTURE CaptureVertexBytecode
#define FX_PREPARE_BANK_OFFSET 0x1c
#define FX_PREPARE_BULK_OFFSET 0x2c
#define FX_PREPARE_SELECTOR value18
#define FX_PREPARE_STAGE_FLAG 2
#define FX_PREPARE_PRES_CODE 0xb4
#define FX_PREPARE_PRES_TAG 0x10000000
#define FX_PREPARE_BASE_CODE 0x94
#define FX_PREPARE_BASE_TAG 0x0c000000
#define FX_PREPARE_SAMPLER_BIAS 0x101
// Original 0x666df7..0x667d50, 3,930 bytes.
#include "EffectPrepareGroup.inc"
#undef FX_PREPARE_METHOD
#undef FX_PREPARE_CAPTURE
#undef FX_PREPARE_BANK_OFFSET
#undef FX_PREPARE_BULK_OFFSET
#undef FX_PREPARE_SELECTOR
#undef FX_PREPARE_STAGE_FLAG
#undef FX_PREPARE_PRES_CODE
#undef FX_PREPARE_PRES_TAG
#undef FX_PREPARE_BASE_CODE
#undef FX_PREPARE_BASE_TAG
#undef FX_PREPARE_SAMPLER_BIAS
#define FX_PREPARE_METHOD PrepareGroupB
#define FX_PREPARE_CAPTURE CapturePixelBytecode
#define FX_PREPARE_BANK_OFFSET 0x24
#define FX_PREPARE_BULK_OFFSET 0x30
#define FX_PREPARE_SELECTOR value1A
#define FX_PREPARE_STAGE_FLAG 0
#define FX_PREPARE_PRES_CODE 0xb5
#define FX_PREPARE_PRES_TAG 0x11000000
#define FX_PREPARE_BASE_CODE 0x9c
#define FX_PREPARE_BASE_TAG 0x0d000000
#define FX_PREPARE_SAMPLER_BIAS 0
// Original 0x667d51..0x668cbf, 3,951 bytes.
#include "EffectPrepareGroup.inc"
