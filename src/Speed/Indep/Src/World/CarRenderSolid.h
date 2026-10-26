#ifndef UG2_CAR_RENDER_SOLID_H
#define UG2_CAR_RENDER_SOLID_H

#include "CarRenderWheels.h"

// PC v1.2 record prefixes used by 0x5c5930/0x5ceb30. Four-byte layout is checked;
// these are original records, separate from native GL objects and callbacks.
struct CarRenderTechnique {
    unsigned int value00;
    int type04;
    unsigned int value08[2];
    unsigned int value10;
};
UG2_CHECK_SIZE(CarRenderTechnique, 0x14);

struct CarRenderPrimitive {
    float minimum00[3];
    unsigned int indexCount0C;
    float maximum10[3];
    unsigned char texture1C;
    unsigned char value1D[3];
    unsigned char lightPack20;
    unsigned char value21[0x34 - 0x21];
    unsigned int value34;
    unsigned int flags38;
};
UG2_CHECK_OFFSET(CarRenderPrimitive, texture1C, 0x1c);
UG2_CHECK_OFFSET(CarRenderPrimitive, lightPack20, 0x20);
UG2_CHECK_OFFSET(CarRenderPrimitive, flags38, 0x38);
UG2_CHECK_SIZE(CarRenderPrimitive, 0x3c);

struct CarRenderPlatformSolid {
    unsigned char pad00[0x10];
    int primitiveCount10;
    CarRenderPrimitive *primitives14;
    unsigned char pad18[0x38 - 0x18];
    const int *remap38;
    CarRenderTechnique *technique3C;
};
UG2_CHECK_OFFSET(CarRenderPlatformSolid, primitives14, 0x14);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, remap38, 0x38);
UG2_CHECK_SIZE(CarRenderPlatformSolid, 0x40);

struct CarRenderQueueLink {
    CarRenderQueueLink *next00;
    CarRenderQueueLink *previous04;
};
UG2_CHECK_SIZE(CarRenderQueueLink, 8);
struct CarRenderTextureGroup;
struct CarRenderTextureState {
    unsigned char pad00[0x1c];
    CarRenderTextureGroup *group1C;
};
UG2_CHECK_OFFSET(CarRenderTextureState, group1C, 0x1c);
UG2_CHECK_SIZE(CarRenderTextureState, 0x20);

struct CarRenderTexture {
    CarRenderTextureState *state00;
    unsigned char pad04[0x24 - 4];
    unsigned int hash24;
    unsigned char pad28[0x51 - 0x28];
    signed char priority51;
    unsigned char pad52[2];
    unsigned char value54;
    unsigned char value55;
    unsigned char material56;
};
UG2_CHECK_OFFSET(CarRenderTexture, priority51, 0x51);
UG2_CHECK_OFFSET(CarRenderTexture, material56, 0x56);

struct CarRenderTextureSlot {
    unsigned int hash00;
    CarRenderTexture *texture04;
};
UG2_CHECK_SIZE(CarRenderTextureSlot, 8);
struct CarRenderTextureOverride {
    unsigned int key00;
    unsigned int hash04;
    CarRenderTexture *value08;
    CarRenderTexture **location0C;
};
UG2_CHECK_SIZE(CarRenderTextureOverride, 0x10);

struct CarRenderTextureGroup {
    CarRenderQueueLink link00;
    unsigned int count08;
    unsigned int indexCount0C;
    CarRenderTexture *texture10;
    CarRenderTechnique *technique14;
    CarRenderQueueLink commands18;
};
UG2_CHECK_OFFSET(CarRenderTextureGroup, commands18, 0x18);
UG2_CHECK_SIZE(CarRenderTextureGroup, 0x20);
struct CarRenderCommand {
    CarRenderQueueLink link00;
    CarRenderPrimitive *primitive08;
    unsigned int value0C;
    CarRenderSlotTable *table10;
    unsigned int flags14;
    CarRenderTechnique *technique18;
    void *record1C;
    CarRenderLightPack *lightPack20;
    WheelMatrix *world24;
    unsigned int value28;
};
UG2_CHECK_OFFSET(CarRenderCommand, lightPack20, 0x20);
UG2_CHECK_OFFSET(CarRenderCommand, record1C, 0x1c);
UG2_CHECK_OFFSET(CarRenderCommand, world24, 0x24);
UG2_CHECK_SIZE(CarRenderCommand, 0x2c);

extern "C" {
extern CarRenderTechnique *ug2SolidTechniques86EB28[23];
extern const int ug2SolidRemap7FF090[9], ug2SolidRemap7FF0B4[9], ug2SolidRemap7FF0D8[9];
extern int ug2SolidFlag870CC4, ug2SolidFlag870CD4, ug2SolidFlag870D14;
extern unsigned char ug2SolidDebug8707CD;
extern int *ug2SolidViewMode870848;
extern unsigned int ug2SolidSkipTexture870A18;
extern int ug2SolidMode8A1C8C;
extern void *ug2SolidGroupPool870918, *ug2SolidCommandPool87091C;
extern CarRenderQueueLink ug2SolidGroups870D30[], ug2SolidGroups8712F0[];
extern unsigned int ug2SolidGroupCount87094C;
// Original 0x4901d0; texture lookup remains an external boundary.
CarRenderTexture *UG2SolidFindTexture(unsigned int hash, int one, int zero);
}
CarRenderLightPack *CarRenderDefaultLightPack(); // 0x48ac50
bool CarRenderSpecialMode();                   // 0x60fdf0
// Original 0x5ceb30, cdecl. Allocation failures retain original behavior.
void CarRenderQueuePrimitive(CarRenderPrimitive *primitive, CarRenderSlotTable *table,
                             unsigned int flags, CarRenderTechnique *technique,
                             CarRenderTexture *texture, WheelMatrix *world, void *record,
                             CarRenderLightPack *lightPack, unsigned int zero);

#endif
