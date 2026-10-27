#ifndef UG2_CAR_RENDER_CONTEXT_H
#define UG2_CAR_RENDER_CONTEXT_H
#include "CarRenderView.h"

// Binding/admission read prefix, distinct from the 0x70-byte eView. The rest
// of the draw context (queues, visibility index and ownership) remains separate.
struct CarRenderDrawContext {
    unsigned char value00[0x40];
    CarRenderView *view40;
    unsigned int flags44;
    bool AdmitsInstance(unsigned short flags) const; // 0x5d65bc..0x5d65cf fragment
};
UG2_CHECK_OFFSET(CarRenderDrawContext, view40, 0x40);
UG2_CHECK_OFFSET(CarRenderDrawContext, flags44, 0x44);
UG2_CHECK_SIZE(CarRenderDrawContext, 0x48);
extern "C" {
extern int ug2RenderViewMode8707DC,ug2RenderReverse89E7A4;
extern unsigned char ug2RenderLodZero836352;
int UG2CarViewModeGlobal(); // 0x5b98a0
unsigned char UG2CarLodZeroFlag(); // 0x499db0
}
void CarRenderBindView(CarRenderView *view,CarRenderDrawContext *context); // 0x5c26e0
void CarRenderSetupContext(CarRenderView *view,CarRenderDrawContext *context,unsigned int flags); // 0x4888a0
#endif
