#ifndef UG2_CAR_RENDER_LIGHT_LOAD_H
#define UG2_CAR_RENDER_LIGHT_LOAD_H

#include "CarRenderLightSelect.h"
#include "WCollision.h"

// Chunk 0x135200 prefix; only offsets observed in 0x48e070/0x5bb5b0 are named.
struct CarRenderLightPack {
    void *value00;
    CarRenderLightLink link04;
    unsigned int hash0C;
    unsigned int kind10;
    unsigned char pad14[0x50 - 0x14];
    unsigned int value50;
    unsigned int value54;
    unsigned char pad58[0x80 - 0x58];
    float value80;
    unsigned char pad84[0x90 - 0x84];
    float value90;
    void AllocateState();         // 0x5bb590, bOMalloc through pool 0x8708b8
    void ApplyDefaults();         // 0x5bb5b0, hash-based pair of image words
};
UG2_CHECK_OFFSET(CarRenderLightPack, hash0C, 0xc);
UG2_CHECK_OFFSET(CarRenderLightPack, value50, 0x50);
UG2_CHECK_OFFSET(CarRenderLightPack, value80, 0x80);
UG2_CHECK_OFFSET(CarRenderLightPack, value90, 0x90);

struct CarRenderLightAnimation {
    CarRenderLightLink link00;
    unsigned int kind08;
    unsigned char pad0C[0x50 - 0xc];
    unsigned short count50;
    unsigned char initialized52;
    unsigned char value53;
    unsigned int section54;
    CarRenderLightLink list58;
};
UG2_CHECK_OFFSET(CarRenderLightAnimation, count50, 0x50);
UG2_CHECK_OFFSET(CarRenderLightAnimation, section54, 0x54);
UG2_CHECK_OFFSET(CarRenderLightAnimation, list58, 0x58);
extern "C" {
extern void *ug2LightStatePool8708B8;
extern unsigned int ug2LightDefaults7FF060[10];
extern CarRenderLightPack *ug2LightDefaultPack7F399C;
extern CarRenderLightPack ug2LightFallbackPack830BD8;
}

// Original 0x48e070/0x48e270, cdecl: three retail lighting chunk kinds.
unsigned int CarRenderLoadLightChunk(unsigned int *chunk);
unsigned int CarRenderUnloadLightChunk(unsigned int *chunk);
// Original 0x48e380/0x48e3b0, cdecl: append/remove additional light sectors.
unsigned int CarRenderRegisterAdditionalLights(CarRenderLightSector *sector);
unsigned int CarRenderRemoveLightSector(CarRenderLightSector *sector);

#endif
