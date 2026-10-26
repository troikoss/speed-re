#ifndef UG2_CAR_RENDER_SOLID_MANAGER_H
#define UG2_CAR_RENDER_SOLID_MANAGER_H
#include "CarRenderSolidPack.h"

struct CarRenderSolidReference {
    CarRenderQueueLink link00;
    unsigned int hash08;
    CarRenderSlotTable *solid0C;
};
UG2_CHECK_SIZE(CarRenderSolidReference, 16);
struct CarRenderDeferredBuffer {
    CarRenderQueueLink link00;
    unsigned int buffer08;
};
UG2_CHECK_SIZE(CarRenderDeferredBuffer, 12);
struct CarRenderDeferredBufferList {
    CarRenderQueueLink head00;
    void Add(unsigned int buffer); // 0x479130, __thiscall
};
extern "C" {
extern CarRenderQueueLink ug2SolidPacks830D1C, ug2UnresolvedSolids82DBC0, ug2OfflineSolids832DA4;
extern CarRenderQueueLink ug2QueuedSolids872B44;
extern CarRenderDeferredBufferList ug2VertexReleases870D28;
extern unsigned int ug2SolidPackCount82D380;
extern unsigned int ug2SolidTimerShift8284E4;
extern float ug2SolidTimerScale828028;
extern float ug2SolidLookupTime82DAC0;
extern float ug2SolidResolveMax82DA8C, ug2SolidResolveSum82DA94;
extern float ug2SolidDetachMax82DA90, ug2SolidDetachSum82DA98, ug2SolidRefTime82DA9C;
extern unsigned int ug2SolidResolveCount82DAA0, ug2SolidDetachCount82DAA4;
// Original clock acquisition 0x43bdf0 and COM Release(+8) are platform boundaries.
unsigned int UG2SolidReadTicker();
unsigned int UG2SolidReleaseIndexBuffer(unsigned int token);
}
long double CarRenderSolidElapsed(unsigned int start, unsigned int end); // 0x43be20
CarRenderSolidPack *CarRenderLoadPackHeader(unsigned int *chunk);       // 0x48d910
void CarRenderClearPackLists(CarRenderSolidPack *pack);                  // 0x493790
unsigned int CarRenderWalkPackPlatform(unsigned int *chunk);            // 0x5bb720
CarRenderSolidPack *CarRenderUnloadPackHeader(unsigned int *chunk);     // 0x493ea0
void CarRenderResolveSolidReferences(CarRenderSolidPack *pack);         // 0x490650
void CarRenderDetachSolidReferences(CarRenderSlotTable *solid);         // 0x493650
void CarRenderUnloadSolidChunk(unsigned int *chunk, CarRenderSolidPack *pack); // 0x493f20
unsigned int CarRenderLoadSolidPack(unsigned int *chunk);               // 0x494060
unsigned int CarRenderUnloadSolidPack(unsigned int *chunk);             // 0x4940e0
#endif
