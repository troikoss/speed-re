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
    CarRenderQueueLink link00;
    unsigned int value08;
    int count0C;
    unsigned char pad10[0x70-0x10];
    unsigned short value70;
    unsigned short value72;
    CarRenderSolidResource *resources74;
    void *stream78;
    unsigned short value7C;
    unsigned short value7E;
    CarRenderQueueLink list80;
    CarRenderQueueLink list88;
};
UG2_CHECK_OFFSET(CarRenderSolidPack, resources74, 0x74);
UG2_CHECK_OFFSET(CarRenderSolidPack, value70, 0x70);
UG2_CHECK_OFFSET(CarRenderSolidPack, value72, 0x72);
UG2_CHECK_OFFSET(CarRenderSolidPack, stream78, 0x78);
UG2_CHECK_OFFSET(CarRenderSolidPack, value7C, 0x7c);
UG2_CHECK_OFFSET(CarRenderSolidPack, list80, 0x80);
UG2_CHECK_OFFSET(CarRenderSolidPack, list88, 0x88);
UG2_CHECK_SIZE(CarRenderSolidPack, 0x90);

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
