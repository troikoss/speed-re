#ifndef UG2_CAR_RENDER_SOLID_H
#define UG2_CAR_RENDER_SOLID_H

#include "CarRenderWheels.h"

// PC v1.2 record prefixes used by 0x5c5930/0x5ceb30. Four-byte layout is checked;
// these are original records, separate from native GL objects and callbacks.
struct CarRenderTechniqueParameters;
struct CarRenderTechnique {
    unsigned int value00;
    int type04;
    unsigned int stride08;
    int index0C;
    unsigned int value10;
    int passCount14;
    unsigned char value18;
    unsigned char pad19[3];
    CarRenderTechniqueParameters *parameters1C;
    unsigned int effect20;
    unsigned int declaration24;
    CarRenderTechnique *Construct(); // 0x5d0f80
    void CacheParameters();          // 0x5d1bd0
    void Setup(int index);           // 0x5d1e20, RET 4
};
UG2_CHECK_OFFSET(CarRenderTechnique, stride08, 8);
UG2_CHECK_OFFSET(CarRenderTechnique, index0C, 0xc);
UG2_CHECK_OFFSET(CarRenderTechnique, value10, 0x10);
UG2_CHECK_OFFSET(CarRenderTechnique, passCount14, 0x14);
UG2_CHECK_OFFSET(CarRenderTechnique, value18, 0x18);
UG2_CHECK_OFFSET(CarRenderTechnique, parameters1C, 0x1c);
UG2_CHECK_OFFSET(CarRenderTechnique, effect20, 0x20);
UG2_CHECK_OFFSET(CarRenderTechnique, declaration24, 0x24);
UG2_CHECK_SIZE(CarRenderTechnique, 0x28);

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
    CarRenderQueueLink link00;
    unsigned int value08;
    unsigned int flags0C;
    int primitiveCount10;
    CarRenderPrimitive *primitives14;
    union {
        unsigned char *indices18;
        unsigned int indexAddress18; // original four-byte address word, used by relocation
    };
    unsigned int indexBytes1C;
    unsigned int indexBuffer20; // original GPU object token, not a host pointer
    unsigned int value24;
    unsigned char *vertices28;
    unsigned int vertexBuffer2C;
    unsigned int vertexBytes30;
    unsigned int vertexCount34;
    const int *remap38;
    CarRenderTechnique *technique3C;
    unsigned int value40;
    unsigned int FixupPrimitives(unsigned int offset); // 0x5c58a0, RET 4
};
UG2_CHECK_OFFSET(CarRenderPlatformSolid, primitives14, 0x14);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, remap38, 0x38);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, technique3C, 0x3c);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, indices18, 0x18);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, indexBuffer20, 0x20);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, vertices28, 0x28);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, vertexCount34, 0x34);
UG2_CHECK_OFFSET(CarRenderPlatformSolid, value40, 0x40);
UG2_CHECK_SIZE(CarRenderPlatformSolid, 0x44);
struct CarRenderTextureGroup;
struct CarRenderTexturePack;
struct CarRenderTextureState {
    CarRenderQueueLink link00;
    unsigned int value08[3];
    int format14;
    unsigned int texture18;
    CarRenderTextureGroup *group1C;
};
UG2_CHECK_OFFSET(CarRenderTextureState, group1C, 0x1c);
UG2_CHECK_SIZE(CarRenderTextureState, 0x20);

struct CarRenderTexture {
    CarRenderTextureState *state00;
    unsigned int value04[2];
    char name0C[24];
    unsigned int hash24;
    unsigned int value28;
    unsigned int parentHash2C;
    int pixelOffset30;
    int paletteOffset34;
    unsigned int pixelBytes38;
    unsigned int paletteBytes3C;
    unsigned int value40;
    short width44;
    short height46;
    unsigned char value48[2];
    unsigned char compression4A;
    unsigned char value4B[3];
    signed char mipCount4E;
    unsigned char value4F[2];
    signed char priority51;
    unsigned char value52;
    unsigned char used53;
    unsigned char value54;
    unsigned char value55;
    unsigned char material56;
    unsigned char pad57[0x68-0x57];
    CarRenderTexturePack *pack68;
    CarRenderTexture *parent6C;
    unsigned char *pixels70;
    unsigned char *palette74;
    unsigned int value78;
    void SetState(CarRenderTextureState *state); // 0x5b9690, RET 4
};
UG2_CHECK_OFFSET(CarRenderTexture, priority51, 0x51);
UG2_CHECK_OFFSET(CarRenderTexture, material56, 0x56);
UG2_CHECK_OFFSET(CarRenderTexture, parentHash2C, 0x2c);
UG2_CHECK_OFFSET(CarRenderTexture, pixels70, 0x70);
UG2_CHECK_OFFSET(CarRenderTexture, pack68, 0x68);
UG2_CHECK_SIZE(CarRenderTexture, 0x7c);

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
// Original 0x4901d0, recovered in CarRenderTexture.cpp. Native file ownership
// is separate; individual solid verifiers can bind this lookup as a boundary.
CarRenderTexture *UG2SolidFindTexture(unsigned int hash, int fallback, int rawMode);
}
CarRenderLightPack *CarRenderDefaultLightPack(); // 0x48ac50
bool CarRenderSpecialMode();                   // 0x60fdf0
// Original 0x5ceb30, cdecl. Allocation failures retain original behavior.
void CarRenderQueuePrimitive(CarRenderPrimitive *primitive, CarRenderSlotTable *table,
                             unsigned int flags, CarRenderTechnique *technique,
                             CarRenderTexture *texture, WheelMatrix *world, void *record,
                             CarRenderLightPack *lightPack, unsigned int zero);

#endif
