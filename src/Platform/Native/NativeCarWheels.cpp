// Platform seams for the recovered car wheel block (src/Speed/Indep/Src/World/CarRenderWheels.cpp) and the recovered
// pixel size and LOD selection (src/Speed/Indep/Src/World/CarRenderLod.cpp). The unrecovered callees that the original
// draw 0x620ae0 calls are implemented for the native demo here. Each seam keeps the argument order of the original; the
// native behaviour is listed per seam. Globals of the block are fixed to the demo's values (see below).
#include "NativeScene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "../../Speed/Indep/Src/World/CarRenderLod.h"

// Globals of the block and of the LOD selection at their original addresses. The demo selects: no LOD gate (gate 0, so
// no special path and no tail draw), no width override (the widths only feed set2, which no draw reads), the division
// constants of the 0.184 scale, and the image values of the LOD globals (0x7fee4c = 1, overrides -1, the pixel
// thresholds of 0x7a66dc and 0x7a66fc, the scale of 0x7a6700). The car record table (0x8a1cd4, 0x8a1cd8) is empty.
extern "C" {
int ug2WheelLodGate = 0;
float ug2WheelLodThreshold = 0.0f;
float ug2WheelWidthOverrideFront = 0.0f;
float ug2WheelWidthOverrideRear = 0.0f;
volatile float ug2WheelOneRuntime = 1.0f;
volatile float ug2WheelBetaRuntime = 0.184f;
int ug2CarLodFlag7FEE4C = 1;
int ug2CarLodOverride802728 = -1;
int ug2CarLodOverride80272C = -1;
float ug2CarLodTable7A66DC[8] = {40.0f, 30.0f, 20.0f, 0.0f, 40.0f, 20.0f, 10.0f, 0.0f};
float ug2CarLodPixelLimit7A66FC = 18.0f;
float ug2CarLodScale7A6700 = 0.7f;
void *ug2CarLodRecords8A1CD4 = 0;
int ug2CarLodRecordCount8A1CD8 = 0;
CarRenderPartTable *ug2CarPartTable8A1CC8 = 0; // part-table loader remains a native seam
}

// 0x5b98a0 (RET 0): the view mode global. The demo's view is mode 0, so the pixel size is not scaled.
extern "C" int UG2CarViewModeGlobal(void) {
    return 0;
}

// 0x499db0 (RET 0): the zero flag of the LOD selection. The demo leaves it clear.
extern "C" unsigned char UG2CarLodZeroFlag(void) {
    return 0;
}

// The car draw's view (arg1 of 0x620ae0) for the pixel size and the DrawSolid receiver. The frame holds the camera origin
// (+0x40) and the depth axis (+0x50) that 0x48acb0 reads; the focal length (+0xc) is set by NativeSetCarView.
static CarRenderFrame gCarViewFrame;
static CarRenderView gCarView;
static bool gCarViewSet = false;

// Sets the view of the car draw from the camera: `eye` is the camera position, `forward` the unit view direction, and
// `focalPixels` the focal length in pixels. The original focal length is set by the camera code, which is not recovered:
// the native value is half the viewport height over the vertical tangent of the projection (docs/NATIVE_DEMO.md).
void NativeSetCarView(const float eye[3], const float forward[3], float focalPixels) {
    for (int i = 0; i < 3; ++i) {
        gCarViewFrame.position40[i] = eye[i];
        gCarViewFrame.axis50[i] = forward[i];
    }
    gCarView.frame40 = &gCarViewFrame;
    gCarView.focal0C = focalPixels;
    gCarViewSet = true;
}

// Without a view (no NativeSetCarView call) the pixel size is taken as large: LOD 0, no skip of the car draw.
static const int kNativeNoViewPixel = 1000;

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

// Runs the layout 0x61fe10, the pixel size and LOD selection, and the wheel block on the render info. `tires`, `brakes`
// and `third` are the arg5, arg6 and arg7 frames of 0x620ae0: the block zeroes their element 13 (as the original does).
// `body` is the body frame of the car draw; its translation (elements 12..14) is the car origin that 0x48acb0 measures.
void NativeRunCarWheelBlock(CarRenderWheelInfo *ri, const unsigned char *carTypeRow, const float *body,
                            WheelMatrix tires[4], WheelMatrix brakes[4], WheelMatrix third[4], bool translucent) {
    ri->carTypeRow = const_cast<unsigned char *>(carTypeRow);
    CarRenderWheelLayout(ri);    // 0x61fe10 (recovered)
    gWheelPassTranslucent = translucent;
    gScratchMatrixCount = 0;
    gScratchRecordCount = 0;

    // Pixel size (0x620c6b..0x620cc0): the caller's LOD inputs [EBP+0x34] and [EBP+0x30] start at 0 in the demo.
    int lodArg = 0;
    int clampArg = 0;
    const int pixel = gCarViewSet ? CarLodPixelSize(&gCarView, body + 12, &lodArg, &clampArg) : kNativeNoViewPixel;
    if (pixel < 4) {
        return;                  // 0x620cba: the car draw leaves before the wheels
    }

    // LOD selection (0x62107e..0x621230). arg8 is 0x401 as in the original's captured calls; the selection ORs 0x800.
    CarLodInputs lod;
    memset(&lod, 0, sizeof(lod));
    lod.ri = ri;
    lod.pixel = pixel;
    lod.lodIn = lodArg;
    lod.clampIn = clampArg;
    lod.arg8 = 0x401;
    CarLodSelect(&lod);
    if (getenv("UG2_DEBUG_CAR_LOD")) {
        fprintf(stderr, "car lod: pixel %d body lod %d clamp %d flags1AC %u arg8 %x\n", pixel, lod.lod, lod.clamp,
                lod.flag1AC, lod.arg8Out);
    }
    if (lod.exitDraw) {
        return;                  // 0x62121c: both LOD values are -1
    }

    CarWheelDrawInputs in;
    memset(&in, 0, sizeof(in));
    in.ri = ri;
    in.view = &gCarView;
    in.flagsArg8 = lod.arg8Out;  // 0x401, plus 0x800 for LOD 1 or more
    in.body = reinterpret_cast<WheelMatrix *>(const_cast<float *>(body));
    in.tyre = tires;
    in.brake = brakes;
    in.third = third;
    in.lod6C = lod.lod;          // frame+0x6c: body LOD
    in.slotD8 = 0;               // frame+0xd8 (k) = [obj+0x2c10] in the original; not modelled, see docs/NATIVE_DEMO.md
    in.lodF4 = lod.clamp;        // frame+0xf4: view clamp
    in.flagsCC = 0;              // frame+0xcc: producer 0x60d1b0 not recovered, see docs/NATIVE_DEMO.md
    in.flags1AC = lod.flag1AC;   // frame+0x1ac: CarLodSelect
    in.setDump = 0;
    CarWheelDraw(&in);           // 0x62700e..0x62df66 (recovered)
}
