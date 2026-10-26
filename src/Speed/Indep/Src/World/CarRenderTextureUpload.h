#ifndef UG2_CAR_RENDER_TEXTURE_UPLOAD_H
#define UG2_CAR_RENDER_TEXTURE_UPLOAD_H
#include "CarRenderTexture.h"
#include "CarRenderSolidManager.h"

struct CarRenderLockedTexture {
    int pitch00;
    unsigned char *pixels04;
};
UG2_CHECK_SIZE(CarRenderLockedTexture, 8);
extern "C" {
extern int ug2TextureSkip870794;
extern unsigned char ug2TextureDisabled87079C;
extern unsigned int ug2TextureUploads864F50;
extern CarRenderQueueLink ug2TextureStates872B4C;
extern CarRenderDeferredBufferList ug2TextureReleases8763B0;
// Original D3D methods: device +0x5c, texture +0x4c/+0x50/+8.
// Host objects stay behind four-byte tokens and typed adapters.
int UG2TextureCreate(unsigned int device, int width, int height, int levels, unsigned int usage,
                     int format, unsigned int pool, unsigned int *output, void *shared);
int UG2TextureLock(unsigned int token, unsigned int level, CarRenderLockedTexture *output,
                   void *rectangle, unsigned int flags);
int UG2TextureUnlock(unsigned int token, unsigned int level);
unsigned int UG2TextureReleaseGpu(unsigned int token);
void *UG2TextureMalloc(unsigned int bytes); // original CRT 0x575620
void UG2TextureFree(void *memory); // original CRT 0x575630
}
void CarRenderReleaseTextures(); // 0x5ce400: drain live and deferred handles to refcount zero
void CarRenderRetireTextures(); // 0x5ce4c0: at most three deferred Release calls per enabled pass
#endif
