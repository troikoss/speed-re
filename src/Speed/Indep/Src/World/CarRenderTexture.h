#ifndef UG2_CAR_RENDER_TEXTURE_H
#define UG2_CAR_RENDER_TEXTURE_H
#include "CarRenderSolid.h"

struct CarRenderTextureIndex {
    unsigned int hash00;
    CarRenderTexture *texture04;
};
UG2_CHECK_SIZE(CarRenderTextureIndex,8);
struct CarRenderTextureStreamSlot {
    unsigned int hash00;
    unsigned int offset04;
    unsigned int packed08;
    unsigned int bytes0C;
    unsigned int value10;
    unsigned int value14;
};
UG2_CHECK_SIZE(CarRenderTextureStreamSlot, 0x18);
struct CarRenderTexturePackHeader {
    unsigned char value00[0x6c];
    unsigned int loaded6C;
    CarRenderTexturePack *pack70;
    CarRenderTextureIndex *indices74;
    CarRenderTextureStreamSlot *slots78;
};
UG2_CHECK_OFFSET(CarRenderTexturePackHeader, indices74, 0x74);
UG2_CHECK_SIZE(CarRenderTexturePackHeader, 0x7c);
struct CarRenderTexturePack {
    CarRenderQueueLink link00;
    char name08[32];
    char path28[64];
    unsigned int hash68;
    unsigned int value6C;
    CarRenderTexturePackHeader *header70;
    int count74;
    unsigned int value78;
    CarRenderTexturePack *Construct(CarRenderTexturePackHeader *header, int count,
                                    const char *name, const char *path); // 0x48fc40, RET 0x10
    void Destroy(); // 0x48fd10
    void Register(CarRenderTexture *texture, CarRenderTextureState *state,
                  CarRenderTextureIndex *entry); // 0x48fd30, RET 0xc
    void RegisterArray(CarRenderTexture *textures, CarRenderTextureState *states, int count); // 0x4919e0
    void UnregisterArray(CarRenderTexture *textures, CarRenderTextureState *states, int count); // 0x491a30
    CarRenderTexture *FindLoaded(unsigned int hash); // 0x48fe00, RET 4
    void Attach(unsigned char *data, int offset, int bytes); // 0x494a50, RET 0xc
    void Detach(int offset, int bytes); // 0x494b10, RET 8
};
UG2_CHECK_OFFSET(CarRenderTexturePack, hash68, 0x68);
UG2_CHECK_OFFSET(CarRenderTexturePack, header70, 0x70);
UG2_CHECK_OFFSET(CarRenderTexturePack, count74, 0x74);
UG2_CHECK_SIZE(CarRenderTexturePack, 0x7c);

extern "C" {
extern CarRenderQueueLink ug2TexturePacks830AF0;
extern unsigned int ug2TextureCount833780, ug2TextureChanged82DAB4;
extern unsigned char ug2TextureBuckets833784[8192];
extern unsigned int ug2TextureOverrideCount82DA80, ug2TextureOverrideKeys82D530[];
extern CarRenderTexture *ug2TextureOverrideValues82D5C0[];
extern CarRenderTexture *ug2TextureDefault82DB08;
extern const char ug2TextureNull78DA04[], ug2TextureLabel78D9F4[], ug2TextureInventory78D9C0[];
unsigned int UG2TextureInventory(...); // 0x50d510, always zero in PC v1.2
void UG2TextureInventoryRelease(unsigned int token); // 0x4022c0, empty in PC v1.2
void UG2TextureMissingDefault(); // 0x43bd50, INT 3
// Recovered in CarRenderTextureUpload.cpp; GPU methods have typed adapters.
void __fastcall UG2TextureUpload(CarRenderTexture *texture); // 0x5ce170
void __fastcall UG2TextureRelease(CarRenderTexture *texture); // 0x5caaa0
// Recovered in CarRenderTextureReferences.cpp, retaining original call order.
void UG2TextureNotifySolids(CarRenderTexturePack *pack, int mode); // 0x4939b0
void UG2TextureNotify5C0DC0(CarRenderTexturePack *pack, int mode);
void UG2TextureNotify51BF70(CarRenderTexturePack *pack, int mode);
void UG2TextureNotify611A10(unsigned int hash);
}
CarRenderTexture *CarRenderRefreshTexture(CarRenderTexture *current, unsigned int hash,
                                         CarRenderTexturePack *pack, char mode); // 0x4902d0
struct CarRenderTextureRequest {
    unsigned int value00[3];
    unsigned int bytes0C;
};
struct CarRenderTextureStreamOwner {
    unsigned char value00[0xbc];
    CarRenderTexturePackHeader *headerBC;
};
UG2_CHECK_SIZE(CarRenderTextureRequest, 0x10);
UG2_CHECK_OFFSET(CarRenderTextureStreamOwner, headerBC, 0xbc);
void CarRenderLoadTextureSlot(unsigned char *decoded, CarRenderTextureRequest *request,
                              CarRenderTextureStreamOwner *owner); // 0x494c10, cdecl
void CarRenderUnloadTextureSlot(unsigned char *decoded, CarRenderTextureRequest *request,
                                CarRenderTextureStreamOwner *owner); // 0x494c80, cdecl
#endif
