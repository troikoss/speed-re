#ifndef UG2_CAR_RENDER_SOLID_PACK_H
#define UG2_CAR_RENDER_SOLID_PACK_H
#include "CarRenderSolidLoad.h"
#include "CarRenderLightLoad.h"

struct CarRenderSolidAux {
    void *value00;
    void *value04;
    unsigned short count08;
    unsigned short count0A;
};
UG2_CHECK_SIZE(CarRenderSolidAux, 12);
struct CarRenderSolidResource {
    unsigned int hash00;
    CarRenderSlotTable *solid04;
};
UG2_CHECK_SIZE(CarRenderSolidResource, 8);
struct CarRenderSolidPack {
    unsigned char pad00[0xc];
    int count0C;
    unsigned char pad10[0x74-0x10];
    CarRenderSolidResource *resources74;
    unsigned int value78;
    unsigned short value7C;
};
UG2_CHECK_OFFSET(CarRenderSolidPack, resources74, 0x74);
UG2_CHECK_OFFSET(CarRenderSolidPack, value7C, 0x7c);

extern "C" {
extern int ug2SolidOffline864F60, ug2SolidFlag82DAA8;
extern unsigned int ug2SolidAuxBytes82D38C, ug2SolidBytes82D390;
extern unsigned int ug2SolidLoads82D384, ug2SolidChunkBytes82D388, ug2SolidChanged82DAB0;
extern unsigned int ug2SolidLoadedCount830DA0;
extern unsigned char ug2SolidBuckets830DA4[8192];
extern CarRenderQueueLink ug2LoadedSolids835794, ug2FailedSolids835858;
// Original cross-pack lookup 0x48df90 and owner search 0x48df40 remain boundaries.
CarRenderSlotTable *UG2SolidFindLoaded(unsigned int hash, CarRenderSolidPack *pack);
CarRenderSolidPack *UG2SolidFindOwner(unsigned int hash);
}
CarRenderLightPack *__fastcall CarRenderProfileFromLink(void *link); // 0x431780
unsigned int CarRenderSolidLoadAllowed();                          // 0x40cb20
void *CarRenderFindResource(unsigned int hash, void *base, int count,
                            unsigned int offset, unsigned int stride); // 0x488bc0
CarRenderSlotTable *CarRenderLoadSolidChunk(unsigned int *chunk, CarRenderSolidPack *pack); // 0x493bb0
#endif
