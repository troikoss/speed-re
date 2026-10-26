#ifndef UG2_CAR_RENDER_CONSUME_H
#define UG2_CAR_RENDER_CONSUME_H
#include "CarRenderTechnique.h"
#include "../../Libs/bWare/bSlotPool.h"

struct CarRenderCommandList {
    CarRenderQueueLink head00;
    void Clear(); // 0x5cdf10
};
UG2_CHECK_SIZE(CarRenderCommandList, 8);
struct CarRenderViewRecord {
    WheelMatrix *matrices00;
    unsigned char value04[0x6c];
};
UG2_CHECK_SIZE(CarRenderViewRecord, 0x70);
extern "C" {
extern CarRenderTechnique *ug2RenderTechnique870910;
extern unsigned int ug2RenderVertex87092C,ug2RenderIndex870930;
extern bSlotPool *ug2RenderImmediatePool870954;
extern unsigned int ug2RenderFrames872EDC,ug2RenderGroups872ED8,ug2RenderIndexed872EE0,ug2RenderImmediate872EE4;
extern unsigned int ug2RenderVertexChanges872EE8,ug2RenderIndexChanges872EEC,ug2RenderTriangles872EF0,ug2RenderTechniques872F04;
extern CarRenderViewRecord ug2RenderViews832DE0[];
extern unsigned int ug2RenderSkyHashes8A07A8[8],ug2RenderSkyIndex8A1C94;
extern float ug2RenderSkyTime8A1C9C,ug2RenderSkyPeriod802954;
// Original virtual calls are typed host adapters, with original class/code tokens
// kept in records. The native renderer does not yet provide these adapters.
void UG2RenderTechniqueBegin(CarRenderTechnique *technique);
void UG2RenderTechniqueEnd(CarRenderTechnique *technique);
int UG2RenderSetDeclaration(unsigned int device,unsigned int declaration);
int UG2RenderSetStream(unsigned int device,unsigned int stream,unsigned int buffer,unsigned int offset,unsigned int stride);
int UG2RenderSetIndices(unsigned int device,unsigned int buffer);
int UG2RenderDrawIndexed(unsigned int device,unsigned int kind,int base,unsigned int minimum,unsigned int vertices,int start,unsigned int primitives);
int UG2RenderDrawImmediate(unsigned int device,unsigned int kind,unsigned int primitives,const void *vertices,unsigned int stride);
int UG2RenderSetState(unsigned int device,unsigned int state,unsigned int value);
int UG2EffectBegin(unsigned int effect,unsigned int *passes,unsigned int flags);
int UG2EffectCommit(unsigned int effect);
int UG2EffectBeginPass(unsigned int effect,unsigned int index);
int UG2EffectEndPass(unsigned int effect);
int UG2EffectEnd(unsigned int effect);
int UG2EffectSetTexture(unsigned int effect,unsigned int parameter,unsigned int texture);
}
// Original engine calls whose bodies remain pending. These are controlled call
// boundaries in the consumer verifier until complete implementations are linked.
void CarRenderBindTexture(CarRenderTexture *texture); // 0x5bcc30
void CarRenderUploadWorld(WheelMatrix *world,int view); // 0x5bdde0
void CarRenderUploadHeadlights(WheelMatrix *world,CarRenderViewRecord *view); // 0x5bdaa0
void CarRenderUploadRecord(void *record,WheelMatrix *world); // 0x5bd140
void CarRenderUploadLightPack(CarRenderLightPack *pack,unsigned int flags); // 0x5bd4a0
void CarRenderUploadBones(void *matrices,CarRenderPrimitive *primitive); // 0x5bdcd0
CarRenderViewRecord *CarRenderViewByIndex(int index); // 0x48b1e0
long double CarRenderSkyFraction(); // 0x60fde0, x87 return
unsigned int CarRenderSkyIndex(); // 0x60fe00
void CarRenderConsumeCommands(); // 0x5cf420
#endif
