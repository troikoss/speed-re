#include "CarRenderContext.h"
#include "../Physics/D3DXMatrixScalar.h"

unsigned short HudAngleUnits(float x,float y); // 0x43e6d0, existing recovered helper
void MakeRotationZ(WheelMatrix *output,unsigned int angle); // 0x5bac90, existing recovered helper

extern "C" int UG2CarViewModeGlobal() { return ug2RenderViewMode8707DC; }
extern "C" unsigned char UG2CarLodZeroFlag() { return ug2RenderLodZero836352; }

// Original 0x5c26e0. Retain the heading/rotation/product calls even though the
// resulting local matrix is unused. Only the selected-view pointer is written
// to the context; this does not assign the eView's camera at its own +0x40.
void CarRenderBindView(CarRenderView *view,CarRenderDrawContext *context) {
    const float *matrix=view->clip00->matrices00[0].elements;
    unsigned int angle=HudAngleUnits(matrix[2],matrix[0]);
    WheelMatrix identity,rotation;
    WheelIdentity(&identity);
    MakeRotationZ(&rotation,angle);
    D3DXMatrixMultiplyScalar(&identity,&identity,&rotation);
    context->view40=view;
}

// Original 0x4888a0. Flags are calculated before binding and written afterwards.
void CarRenderSetupContext(CarRenderView *view,CarRenderDrawContext *context,unsigned int flags) {
    int mode=UG2CarViewModeGlobal();
    int type=view->type04;
    unsigned int result=flags|0x10;
    if (type==1 || type==2) result|=2;
    if (mode==4 || mode==3) result|=1;
    if (ug2RenderReverse89E7A4==0) result|=8;
    else if (ug2RenderReverse89E7A4==1) result|=4;
    if (result&0x1800) result|=0x40;
    if (type==3) result|=0x20;
    CarRenderBindView(view,context);
    context->flags44=result;
}

// Original conditional at 0x5d65bc..0x5d65cf, within 0x5d6570. This is the
// byte-flag admission gate only, not the complete instance/LOD/queue routine.
bool CarRenderDrawContext::AdmitsInstance(unsigned short flags) const {
    return (static_cast<unsigned char>(flags44)&static_cast<unsigned char>(flags^0x60))==0;
}
