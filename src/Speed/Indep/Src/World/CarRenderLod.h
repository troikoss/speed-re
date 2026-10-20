#ifndef UG2_CAR_RENDER_LOD_H
#define UG2_CAR_RENDER_LOD_H

#include "CarRenderWheels.h"

// Pixel size, body LOD and wheel flags of the car draw 0x620ae0 (PC v1.2, speed2.exe). Addresses in comments are
// original. The verifier (tools/verify_pc_car_draw_lod.py) runs the original byte ranges under emulation and compares
// them with these routines. The frame cells of the original are named by their offsets (frame+0x6c = [ESP+0x6c]).

// Frame of the view (view+0x40 is a pointer to it): the position at +0x40 and the depth axis at +0x50.
struct CarRenderFrame {
    unsigned char pad00[0x40];
    float position40[3];     // +0x40..+0x48: origin subtracted from the point
    unsigned char pad4C[0x50 - 0x4C];
    float axis50[3];         // +0x50..+0x58: depth axis (dot product of the offset)
};
UG2_CHECK_OFFSET(CarRenderFrame, position40, 0x40);
UG2_CHECK_OFFSET(CarRenderFrame, axis50, 0x50);

// Platform seams (cdecl getters of the original, linked to them by the verifier; the native demo defines them).
extern "C" {
// 0x5b98a0 (RET 0): returns the dword at 0x8707dc. The pixel scale applies when it is 3.
int UG2CarViewModeGlobal(void);
// 0x499db0 (RET 0): returns the byte at 0x836352. A non-zero value forces the body LOD and the clamp to 0.
unsigned char UG2CarLodZeroFlag(void);
}

// Globals of the car draw. Their image addresses are in the comments; the native platform defines them.
extern "C" {
extern int ug2WheelLodGate;                  // 0x8654a4: the LOD gate (3 and 6 select the special paths)
extern int ug2CarLodFlag7FEE4C;              // 0x7fee4c: when zero, both wheel flags are 1
extern int ug2CarLodOverride802728;          // 0x802728: body LOD override (-1 = none)
extern int ug2CarLodOverride80272C;          // 0x80272c: view clamp override (-1 = none)
extern float ug2CarLodTable7A66DC[8];        // 0x7a66dc: pixel thresholds indexed by the body LOD
extern float ug2CarLodPixelLimit7A66FC;      // 0x7a66fc: pixel threshold of the wheel flags (strict <)
extern float ug2CarLodScale7A6700;           // 0x7a6700: scale of the view mode 3 pixel path
extern void *ug2CarLodRecords8A1CD4;         // 0x8a1cd4: car records of 0x40 bytes (key dword, value dword at +4)
extern int ug2CarLodRecordCount8A1CD8;       // 0x8a1cd8: number of car records
}

// Inputs and outputs of the LOD selection 0x62107e..0x621230. Inputs come from the frame and the arguments of the
// car draw; outputs are the frame cells the original writes on the same path.
struct CarLodInputs {
    CarRenderWheelInfo *ri;      // frame+0x58 (ri): owner at +0, car object at +4, car type row at +8, flag at +0x2eb8
    int pixel;                   // frame+0x60: the pixel size after CarLodPixelSize
    int lodIn;                   // [EBP+0x34]: body LOD input (CarLodPixelSize sets it to 1 on the scale path)
    int clampIn;                 // [EBP+0x30]: view clamp input (CarLodPixelSize sets it to 1 on the scale path)
    unsigned int arg8;           // [EBP+0x24]: draw flags, OR-ed with 0x800 when the body LOD is 1 or more
    // Outputs
    int searchIndex;             // frame+0x68: index of the first matching car record (the count when none matches)
    float pixelFloat;            // frame+0x64: the pixel as float (stored at 0x621157)
    unsigned int owner958;       // frame+0x180: owner+0x958 (0 without an owner)
    unsigned int ownerHas958;    // frame+0x14c: 1 when there is an owner and owner+0x958 is non-zero
    int lod;                     // frame+0x6c: body LOD (the wheel block's lod6C)
    int clamp;                   // frame+0xf4: view clamp (the wheel block's lodF4)
    unsigned int flagE8;         // frame+0xe8: 0 or 1 (the wheel block overwrites it with an allocation)
    unsigned int flag1AC;        // frame+0x1ac: 0 or 1 (the wheel block's flags1AC)
    unsigned int arg8Out;        // [EBP+0x24] after the 0x800 OR
    int exitDraw;                // 1 when the original leaves through 0x630d1b (lod == -1 and clamp == -1)
};

// 0x48acb0 (__thiscall on the view, RET 8): projected radius in pixels of a sphere of `radius` at `position`. Returns 0
// when the sphere lies behind the view plane (depth < -radius), 2 * focal when it is close (distance - radius <= 0.5
// radius), and otherwise (int)((focal / (distance - radius)) * radius). The focal length is view+0xc.
int CarViewPixelRadius(CarRenderView *view, const float *position, float radius);

// 0x620c6b..0x620cc0: pixel size of the car. CarViewPixelRadius with radius 3.0; when UG2CarViewModeGlobal() is 3 and the
// gate is not 3, the pixel is scaled by ug2CarLodScale7A6700 (truncated) and both LOD arguments are set to 1. The caller
// leaves the car draw when the result is below 4 (0x620cba).
int CarLodPixelSize(CarRenderView *view, const float *position, int *lodArg, int *clampArg);

// 0x62107e..0x621230: owner words, car record search (0x6210bb..0x62110b), wheel flags (0x621115..0x621142), body LOD
// and view clamp by the pixel thresholds (0x62114d..0x62118b), the forced zero (0x62118f), the clamps (0x6211a7..0x6211e8),
// the overrides (0x6211ec..0x62120f), the exit when both are -1 (0x62121c), and arg8 |= 0x800 for LOD 1 or more (0x621229).
void CarLodSelect(CarLodInputs *in);

#endif
