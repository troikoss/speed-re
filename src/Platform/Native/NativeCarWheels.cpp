// Platform seams for the recovered car wheel block (src/Speed/Indep/Src/World/CarRenderWheels.cpp) and the recovered
// pixel size and LOD selection (src/Speed/Indep/Src/World/CarRenderLod.cpp). The unrecovered callees that the original
// draw 0x620ae0 calls are implemented for the native demo here; record preparation
// now executes recovered bodies. Each seam keeps the argument order of the original; the
// native behaviour is listed per seam. Globals of the block are fixed to the demo's values (see below).
#include "NativeScene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "../../Speed/Indep/Src/World/CarRenderLod.h"
#include "../../Speed/Indep/Src/World/CarRenderInputs.h"
#include "../../Speed/Indep/Src/World/CarRenderView.h"
#include "../../Speed/Indep/Src/World/CarRenderConsume.h"
#include "../../Speed/Indep/Src/World/CarRenderContext.h"
#include "../../Speed/Indep/Src/Camera/Camera.h"
#include "../../Speed/Indep/Src/Camera/CameraMatrix.h"

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
int ug2WheelFacingEnabled802914 = 1; // original image default; normal producer remains open
float ug2WheelFacingX8A1C54 = 0.0f;
float ug2WheelFacingY802910 = 1.0f;
float ug2WheelFacingZ8A1C58 = 0.0f;
WheelMatrix ug2WheelReflectionBasis871B50;
}

// Use original view-table entry 1 and its recovered matrix/plane storage. The
// native session supplies its one finalized camera and viewport; 0x5c7ec0 now
// produces the focal input consumed by the original pixel-size/LOD routines.
static CameraState gCarViewCamera;
static CarRenderFrame &gCarViewFrame=*reinterpret_cast<CarRenderFrame *>(&gCarViewCamera);
static CarRenderView &gCarView=*reinterpret_cast<CarRenderView *>(ug2RenderViews832DE0+1);
static CarRenderViewport gCarViewport;
static CarRenderDrawContext gCarContext;
static bool gCarViewSet = false;

const CarRenderDrawContext &NativeCarContext() { return gCarContext; }

// Original view+0x40 is the Camera (0x44ab90). Use the complete finalized
// prefix, rather than rebuilding a direction from target-eye. A null finalState
// is the native orbit-inspection camera and builds its frame with recovered LookAt.
void NativeSetCarView(const CameraState *finalState, const float eye[3], const float target[3],
                      const float carPosition[3], unsigned short fov,float nearPlane,int width,int height) {
    static bool constructed=false;
    if (!constructed) {
        // The original CRT initializer 0x77ccd0 constructs exactly 22 views.
        for (int i=0;i<22;++i) reinterpret_cast<CarRenderView *>(ug2RenderViews832DE0+i)->Construct();
        constructed=true;
    }
    if (finalState) {
        memcpy(&gCarViewCamera, finalState, sizeof(gCarViewCamera));
    } else {
        memset(&gCarViewCamera,0,sizeof(gCarViewCamera));
        WheelVector cameraEye = {eye[0], eye[1], eye[2]};
        WheelVector cameraTarget = {target[0], target[1], target[2]};
        CameraLookAt(&gCarViewFrame.matrix00, &cameraEye, &cameraTarget, &ug2ReflectionUp800B70);
        for (int i = 0; i < 3; ++i) {
            gCarViewFrame.position40[i] = eye[i];
            gCarViewFrame.axis50[i] = gCarViewFrame.matrix00.elements[2 + i * 4];
        }
        gCarViewCamera.fovC4=fov;
        gCarViewCamera.nearBC=nearPlane;
        gCarViewCamera.farC0=15000.0f;
    }
    gCarView.frame40 = &gCarViewFrame;
    gCarViewport.width14=width;
    gCarViewport.height18=height;
    gCarView.viewport58=&gCarViewport;
    gCarView.active08=1;
    gCarView.clip00->Update(&gCarView,0,0,0);
    CarRenderSetupContext(&gCarView,&gCarContext,0);
    gCarViewSet = true;
    static bool reportedContext=false;
    if (!reportedContext && getenv("UG2_DEBUG_VIEW_CONTEXT")) {
        fprintf(stderr,"native draw context: view %d mode %d reverse %d flags %08x lod-zero %u\n",
            gCarView.type04,UG2CarViewModeGlobal(),ug2RenderReverse89E7A4,gCarContext.flags44,UG2CarLodZeroFlag());
        reportedContext=true;
    }
    static bool dumpedContext=false;
    const char *contextDump=getenv("UG2_CAR_CONTEXT_DUMP");
    if (!dumpedContext && contextDump) {
        FILE *output=fopen(contextDump,"wb");
        if (!output) { perror("UG2_CAR_CONTEXT_DUMP"); abort(); }
        const int mode=UG2CarViewModeGlobal(),reverse=ug2RenderReverse89E7A4;
        const unsigned char zero=UG2CarLodZeroFlag();
        bool ok=fwrite("UG2CTX01",1,8,output)==8 &&
            fwrite(&gCarView,1,sizeof(gCarView),output)==sizeof(gCarView) &&
            fwrite(gCarView.clip00,1,sizeof(*gCarView.clip00),output)==sizeof(*gCarView.clip00) &&
            fwrite(&gCarContext,1,sizeof(gCarContext),output)==sizeof(gCarContext) &&
            fwrite(&mode,1,4,output)==4 && fwrite(&reverse,1,4,output)==4 && fwrite(&zero,1,1,output)==1;
        if (fclose(output)!=0 || !ok) { fprintf(stderr,"incomplete native car-context dump\n"); abort(); }
        dumpedContext=true;
    }
    static bool dumped=false;
    const char *dump=getenv("UG2_CAR_VIEW_DUMP");
    if (!dumped && dump) {
        FILE *output=fopen(dump,"wb");
        if (!output) { perror("UG2_CAR_VIEW_DUMP"); abort(); }
        bool ok=fwrite("UG2VIEW1",1,8,output)==8 &&
            fwrite(&gCarViewCamera,1,sizeof(gCarViewCamera),output)==sizeof(gCarViewCamera) &&
            fwrite(&gCarViewport,1,sizeof(gCarViewport),output)==sizeof(gCarViewport) &&
            fwrite(&gCarView,1,sizeof(gCarView),output)==sizeof(gCarView) &&
            fwrite(gCarView.clip00,1,sizeof(*gCarView.clip00),output)==sizeof(*gCarView.clip00);
        if (fclose(output)!=0 || !ok) { fprintf(stderr,"incomplete native car-view dump\n"); abort(); }
        dumped=true;
    }
    // 0x61ac46 and 0x5c3524: a separate basis aimed at the selected car + offset.
    // The single native player supplies the original caller's selected-car origin.
    WheelVector reflectionTarget;
    CarRenderReflectionTarget(&reflectionTarget.x, carPosition);
    WheelVector reflectionEye = {gCarViewFrame.position40[0], gCarViewFrame.position40[1], gCarViewFrame.position40[2]};
    CarRenderReflectionBasis(&ug2WheelReflectionBasis871B50, &reflectionEye, &reflectionTarget);
}

// Without a view (no NativeSetCarView call) the pixel size is taken as large: LOD 0, no skip of the car draw.
static const int kNativeNoViewPixel = 1000;

// Pass of the current draw: the solids are drawn from their opaque list or their translucent list.
static bool gWheelPassTranslucent = false;

// Native boundary for 0x5c5930: draws the solid's list with the world matrix. The world
// matrix of the block already includes the body frame, so it replaces the modelview matrix of the wheel.
// The original's flags (0x401 and the tyre/brake bits), the extra record and the zero argument have no
// native equivalent here. CarRenderSolid.cpp recovers the dispatcher/queue, but
// native effect support, frame/cache scheduling and consumer API adapters are still
// needed to activate the recovered CarRenderConsume.cpp path.
// Front face and mirroring are not changed (the GL backend seam; see the docs).
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

// 0x48f660 (cdecl): original render-record preparation calls 0x48e6c0 then 0x48a6a0.
// Execute the recovered preparation chain. GL consumption remains a separate seam.
extern "C" unsigned int UG2WheelVisible(void *a1, void *a2, const void *a3, void *a4, void *a5, void *a6) {
    return CarRenderPrepareRecord(static_cast<CarRenderRecord *>(a1), static_cast<WheelMatrix *>(a2),
                                   static_cast<const WheelMatrix *>(a3), static_cast<const float *>(a4),
                                   a5, static_cast<CarRenderRecord *>(a6));
}

// 0x401e80 (cdecl) and 0x401e30 (cdecl): the block's two scratch allocations of each kind. The demo gives
// the two matrix blocks (four world matrices each) and the two record blocks (four records of 0x1c0 bytes
// each) fixed storage; the block calls each allocator once per kind and in this order.
static WheelMatrix gScratchMatrices[2][4];
static CarRenderRecord gScratchRecords[2][4];
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

    // 0x620f04/0x620fcb: caller buffers remain live until the wheel block. The
    // original position has four words; its source record is 0x1c0 bytes.
    float renderPosition[4];
    CarRenderRecord sourceRecord;
    CarRenderCapturePosition(renderPosition, &gCarViewFrame);
    CarRenderBuildSource(&sourceRecord, &ug2CarLightProfile7F3F00, body + 12, &gCarView);

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
    in.renderPosition = renderPosition;
    in.sourceRecord = &sourceRecord;
    CarWheelDraw(&in);           // 0x62700e..0x62df66 (recovered)
    static bool reportedRecords = false;
    if (!reportedRecords && getenv("UG2_DEBUG_CAR_RECORDS")) {
        reportedRecords = true;
        unsigned int sourceHash = 2166136261u, recordHash = 2166136261u;
        const unsigned char *source = reinterpret_cast<const unsigned char *>(&sourceRecord);
        const unsigned char *record = reinterpret_cast<const unsigned char *>(&gScratchRecords[0][0]);
        for (unsigned int i = 0; i < sizeof(CarRenderRecord); ++i) {
            sourceHash = (sourceHash ^ source[i]) * 16777619u;
            recordHash = (recordHash ^ record[i]) * 16777619u;
        }
        fprintf(stderr, "native wheel records: source %08x record0 %08x camera %.9g %.9g %.9g outputC0 %.9g %.9g %.9g %.9g\n",
                sourceHash, recordHash, renderPosition[0], renderPosition[1], renderPosition[2],
                gScratchRecords[0][0].valueC0[0], gScratchRecords[0][0].valueC0[1],
                gScratchRecords[0][0].valueC0[2], gScratchRecords[0][0].valueC0[3]);
    }
}
