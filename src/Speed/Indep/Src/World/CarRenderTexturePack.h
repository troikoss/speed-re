#ifndef UG2_CAR_RENDER_TEXTURE_PACK_H
#define UG2_CAR_RENDER_TEXTURE_PACK_H
#include "CarRenderTexture.h"

struct CarRenderTextureAnimationFrame {
    unsigned int hash00;
    CarRenderTexture *texture04;
    unsigned int *snapshot08;
    unsigned int value0C;
};
UG2_CHECK_SIZE(CarRenderTextureAnimationFrame, 0x10);
struct CarRenderTextureAnimationEntry {
    unsigned char value00[0x1c];
    int count1C;
    unsigned int value20[2];
    CarRenderTextureAnimationFrame *frames28;
    unsigned int ready2C;
    unsigned int value30;
};
UG2_CHECK_OFFSET(CarRenderTextureAnimationEntry, frames28, 0x28);
UG2_CHECK_SIZE(CarRenderTextureAnimationEntry, 0x34);
struct CarRenderTextureAnimation {
    CarRenderQueueLink link00;
    CarRenderTexturePack *pack08;
    CarRenderTextureAnimationEntry *entries0C;
    CarRenderTextureAnimationFrame *frames10;
    int count14;
    unsigned int value18;
    CarRenderTextureAnimation *Construct(CarRenderTexturePack *pack, CarRenderTextureAnimationEntry *entries,
                                        CarRenderTextureAnimationFrame *frames, int count, unsigned int value); // 0x48fe60
    void Update(); // 0x48ff40
    void Destroy(); // 0x48fed0
};
UG2_CHECK_SIZE(CarRenderTextureAnimation, 0x1c);
extern "C" {
extern void *ug2TexturePackPool82DB0C, *ug2TextureSnapshotPool870790;
extern CarRenderTexturePack *ug2TextureCurrentPack82DB10;
extern CarRenderQueueLink ug2TexturePixelQueue8358C8, ug2TextureAnimations82DC00;
extern const char ug2TexturePoolName78D950[];
}
void CarRenderInitTexturePackPool(); // 0x48b1f0
unsigned int *__fastcall CarRenderTextureSnapshot(CarRenderTexture *texture); // 0x5b96f0
void __stdcall CarRenderFreeTextureSnapshot(unsigned int *snapshot); // 0x5b9710, RET 4
CarRenderTexturePackHeader *CarRenderLoadTextureHeader(unsigned int *chunk); // 0x494380
void CarRenderUnloadTextureHeader(unsigned int *chunk); // 0x494dd0
void CarRenderQueueTexturePixels(unsigned int *chunk); // 0x48fc00
CarRenderQueueLink *CarRenderFindTexturePixels(unsigned int key); // 0x48fbc0
unsigned int CarRenderLoadTexturePack(unsigned int *chunk); // 0x494ea0
unsigned int CarRenderUnloadTexturePack(unsigned int *chunk); // 0x495040
#endif
