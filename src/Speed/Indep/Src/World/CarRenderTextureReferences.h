#ifndef UG2_CAR_RENDER_TEXTURE_REFERENCES_H
#define UG2_CAR_RENDER_TEXTURE_REFERENCES_H
#include "CarRenderTexture.h"
#include "CarRenderSolidManager.h"

struct CarRenderPointerNode {
    CarRenderQueueLink link00;
    union {
        void *value08;
        CarRenderTexturePack *pack08;
        CarRenderTextureSlot *slot08;
    };
};
UG2_CHECK_SIZE(CarRenderPointerNode, 12);
struct CarRenderPointerList {
    CarRenderQueueLink head00;
    void Prepend(void *value); // 0x4cbab0, RET 4
};
UG2_CHECK_SIZE(CarRenderPointerList, 8);
// Observed prefixes used by the two standalone refresh loops.
struct CarRenderTextureReference5C {
    CarRenderQueueLink link00;
    unsigned int value08;
    unsigned int hash0C;
    unsigned int value10;
    CarRenderTexture *texture14;
};
UG2_CHECK_OFFSET(CarRenderTextureReference5C, texture14, 0x14);
UG2_CHECK_SIZE(CarRenderTextureReference5C, 0x18);
struct CarRenderFontTextureReference {
    CarRenderQueueLink link00;
    CarRenderTexture *texture08;
    unsigned int value0C[3];
    unsigned int hash18;
};
UG2_CHECK_OFFSET(CarRenderFontTextureReference, hash18, 0x18);
UG2_CHECK_SIZE(CarRenderFontTextureReference, 0x1c);
struct CarRenderTextureCacheRecord {
    unsigned int value00[0x72];
};
UG2_CHECK_SIZE(CarRenderTextureCacheRecord, 0x1c8);
extern "C" {
extern CarRenderQueueLink ug2TextureReferences830D08, ug2FontTextures863768;
extern CarRenderTextureCacheRecord ug2TextureCache89F050[6];
}
void CarRenderBuildPackTextureReferences(CarRenderSolidPack *pack); // 0x493800
void CarRenderDetachOfflineTextures(CarRenderTexturePack *pack); // 0x4938e0
CarRenderTextureCacheRecord *CarRenderFindTextureCache(unsigned int hash); // 0x611910
#endif
