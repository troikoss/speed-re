#ifndef UG2_CAR_RENDER_SOLID_LOAD_H
#define UG2_CAR_RENDER_SOLID_LOAD_H

#include "CarRenderSolid.h"

struct CarRenderTechniqueRule {
    unsigned int mask00;
    unsigned int technique04;
    unsigned int value08;
};
UG2_CHECK_SIZE(CarRenderTechniqueRule, 12);
extern "C" {
extern CarRenderTechniqueRule ug2SolidRules800464[4];
extern CarRenderQueueLink ug2PlatformSolids872ED0;
extern int *ug2SolidLoadMode8900D8;
extern unsigned int ug2SolidDevice870974;

// Platform adapters for the original D3D device methods +0x68/+0x6c and
// buffer methods +0x2c/+0x30. Tokens retain four-byte original record fields;
// a native backend must keep its host objects/callbacks outside those records.
int UG2SolidCreateVertexBuffer(unsigned int device, unsigned int bytes, unsigned int usage,
                               unsigned int format, unsigned int pool, unsigned int *output,
                               void *shared);
int UG2SolidCreateIndexBuffer(unsigned int device, unsigned int bytes, unsigned int usage,
                              unsigned int format, unsigned int pool, unsigned int *output,
                              void *shared);
int UG2SolidLockBuffer(unsigned int buffer, unsigned int offset, unsigned int bytes,
                       void **output, unsigned int flags);
int UG2SolidUnlockBuffer(unsigned int buffer);
}
// Original 0x5bc6b0, cdecl: first matching rule, including the zero-mask default.
unsigned int CarRenderTechniqueIndex(unsigned int flags);

#endif
