// Platform seams for the recovered car wheel block (src/Speed/Indep/Src/World/CarRenderWheels.cpp).
// The unrecovered callees that the original draw 0x620ae0 calls from the wheel block are implemented for
// the native demo here. Each seam keeps the argument order of the original; the native behaviour is
// listed per seam. Globals of the block are fixed to the demo's values (see below).
#include "NativeScene.h"

#include <string.h>

#include <GL/gl.h>

#include "../../Speed/Indep/Src/World/CarRenderWheels.h"

// Globals of the block at their original addresses (0x8654a4, 0x8a1c5c, 0x7a668c, 0x7a6690, 0x784250,
// 0x8026c4). The demo selects: no LOD substitution and no secondary draws (gate 0), no width override
// (the widths only feed set2, which no draw reads), and the division constants of the 0.184 scale.
extern "C" {
int ug2WheelLodGate = 0;
float ug2WheelLodThreshold = 0.0f;
float ug2WheelWidthOverrideFront = 0.0f;
float ug2WheelWidthOverrideRear = 0.0f;
volatile float ug2WheelOneRuntime = 1.0f;
volatile float ug2WheelBetaRuntime = 0.184f;
}

// Pass of the current draw: the solids are drawn from their opaque list or their translucent list.
static bool gWheelPassTranslucent = false;

// 0x5c5930 (__thiscall on the view, RET 0x14): draws the solid's list with the world matrix. The world
// matrix of the block already includes the body frame, so it replaces the modelview matrix of the wheel.
// The original's flags (0x401 and the tyre/brake bits), the extra record and the zero argument have no
// native equivalent here. Front face and mirroring are not changed (the GL backend seam; see the docs).
void CarRenderView::DrawSolid(CarRenderSolid *solid, WheelMatrix *world, void *extra, unsigned int flags,
                              unsigned int zero) {
    (void)extra;
    (void)flags;
    (void)zero;
    const NativeDrawSolid *draw = reinterpret_cast<const NativeDrawSolid *>(solid);
    if (!draw || !world) {
        return;
    }
    const GLuint list = gWheelPassTranslucent ? draw->translucentList : draw->opaqueList;
    if (!list) {
        return;
    }
    glPushMatrix();
    glMultMatrixf(world->elements);
    glCallList(list);
    glPopMatrix();
}

// 0x48d860 (__thiscall on the owner, RET 8): the solid slot table is not part of the demo's solids. The
// link values of the block (ri+0x390, ri+0x3ac) are zero in the demo, so the block does not call this.
void CarRenderSolid::SetSlot(unsigned int key, unsigned int value) {
    (void)key;
    (void)value;
}

// 0x60fef0 (__thiscall on the rim part, RET 8): the SPINNEROFFSET records of the rim parts are not in
// the demo's parts, so no record is found and the offsets of ri+0x438 stay zero.
void *CarRenderPart::FindRecord(unsigned int hash, void *previous) {
    (void)hash;
    (void)previous;
    return 0;
}

// 0x620600 (__thiscall on ri, RET 8): secondary draw of a tyre matrix. Reached only with the LOD gate 3,
// which the demo does not select.
void CarRenderWheelInfo::Fn620600(CarRenderView *view, WheelMatrix *world) {
    (void)view;
    (void)world;
}

// 0x48f660 (cdecl): visibility or level-of-detail test. The original ignores its result in the block, and
// the demo does not cull wheels, so the result is visible.
extern "C" unsigned int UG2WheelVisible(void *a1, void *a2, const void *a3, void *a4, void *a5, void *a6) {
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    return 1;
}

// 0x60d700 (cdecl, float in ST0): compared with the threshold only under the LOD gates 3 and 6, which the
// demo does not select.
extern "C" float UG2WheelFn60d700(WheelMatrix *world, void *view) {
    (void)world;
    (void)view;
    return 0.0f;
}

// 0x401e80 (cdecl) and 0x401e30 (cdecl): the block's two scratch allocations of each kind. The demo gives
// the two matrix blocks (four world matrices each) and the two record blocks (four records of 0x1c0 bytes
// each) fixed storage; the block calls each allocator once per kind and in this order.
static WheelMatrix gScratchMatrices[2][4];
static unsigned char gScratchRecords[2][0x700];
static unsigned int gScratchMatrixCount = 0;
static unsigned int gScratchRecordCount = 0;

extern "C" void *UG2WheelAllocMatrices(unsigned int count) {
    (void)count;
    return gScratchMatrices[(gScratchMatrixCount++) & 1];
}

extern "C" void *UG2WheelAllocRecords(unsigned int bytes) {
    (void)bytes;
    return gScratchRecords[(gScratchRecordCount++) & 1];
}

// Runs the layout 0x61fe10 and the wheel block on the render info. `tires`, `brakes` and `third` are the
// arg5, arg6 and arg7 frames of 0x620ae0: the block zeroes their element 13 (as the original does).
void NativeRunCarWheelBlock(CarRenderWheelInfo *ri, const unsigned char *carTypeRow, const float *body,
                            WheelMatrix tires[4], WheelMatrix brakes[4], WheelMatrix third[4], bool translucent) {
    static CarRenderView view;   // receiver of DrawSolid; the demo keeps no view state
    ri->carTypeRow = const_cast<unsigned char *>(carTypeRow);
    CarRenderWheelLayout(ri);    // 0x61fe10 (recovered)
    gWheelPassTranslucent = translucent;
    gScratchMatrixCount = 0;
    gScratchRecordCount = 0;
    CarWheelDrawInputs in;
    memset(&in, 0, sizeof(in));
    in.ri = ri;
    in.view = &view;
    in.flagsArg8 = 0x401;        // arg8 of the car draw in the original's captured calls
    in.body = reinterpret_cast<WheelMatrix *>(const_cast<float *>(body));
    in.tyre = tires;
    in.brake = brakes;
    in.third = third;
    in.viewIndex = 0;            // view 0: the demo draws the view it renders
    in.slotIndex = 0;            // the caller's column of the wheel offsets (0x6316d0 / 0x631250), not recovered
    in.viewClamp = 0;
    in.flagsCC = 0;
    in.flagsAC = 0;
    in.setDump = 0;
    CarWheelDraw(&in);           // 0x62700e..0x62df66 (recovered)
}
