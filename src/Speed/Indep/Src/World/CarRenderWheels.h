#ifndef UG2_CAR_RENDER_WHEELS_H
#define UG2_CAR_RENDER_WHEELS_H

#include "../Physics/WheelMath.h"
#include "../Physics/Ug2Layout.h"

// Car wheel placement for the render info (CarRenderInfo, ri) and the wheel block of the car draw
// 0x620ae0 (PC v1.2, speed2.exe). Addresses in comments are original. Unrecovered callees are
// platform seams; SetSlot and FindRecord are recovered in CarRenderRecords.cpp. Thiscall callees
// are members so their calling convention matches the original,
// cdecl callees are extern "C". The verifier (tools/verify_pc_wheel_draw.py) links these names to
// the original addresses and compares their argument sequences.

struct CarRenderSolid;     // drawable solid object (a platform type; receiver of SetSlot)
struct CarRenderView;      // view object (arg1 of the car draw; receiver of DrawSolid)
struct CarRenderPart;      // rim part object (ri+4 +0x604/+0x608; receiver of FindRecord)
struct CarRenderFrame;     // view frame (view+0x40): origin and depth axis of the projection (CarRenderLod.h)
struct CarRenderWheelInfo;

struct CarRenderSlot {
    unsigned int key;
    unsigned int value;
};
UG2_CHECK_SIZE(CarRenderSlot, 8);

struct CarRenderSlotTable {
    unsigned char pad00[0x1a];
    signed char count1A;
    unsigned char pad1B[0x3c - 0x1b];
    CarRenderSlot *slots3C;
};
UG2_CHECK_OFFSET(CarRenderSlotTable, count1A, 0x1a);
UG2_CHECK_OFFSET(CarRenderSlotTable, slots3C, 0x3c);

// 0x48d860 (__thiscall, RET 8): updates every matching slot if value is nonzero.
struct CarRenderSolid {
    unsigned char pad00[0x0c];
    CarRenderSlotTable *table0C;
    void SetSlot(unsigned int key, unsigned int value);
};
UG2_CHECK_OFFSET(CarRenderSolid, table0C, 0x0c);

// 0x5c5930 (__thiscall on the view, RET 0x14): solid draw. Arguments: solid, world matrix, extra
// record, flags, zero.
struct CarRenderView {
    unsigned char pad00[0x0C];
    float focal0C;                  // +0x0c: focal length in pixels (0x48acb0 divides by it)
    unsigned char pad10[0x40 - 0x10];
    const CarRenderFrame *frame40;  // +0x40: view frame (origin +0x40 and depth axis +0x50)
    void DrawSolid(CarRenderSolid *solid, WheelMatrix *world, void *extra, unsigned int flags, unsigned int zero);
};
UG2_CHECK_OFFSET(CarRenderView, focal0C, 0x0c);
UG2_CHECK_OFFSET(CarRenderView, frame40, 0x40);

// 0x60fef0 (__thiscall on the part, RET 8): record with the given hash after `previous` (0 = first).
// A record is 8 bytes: u32 hash at +0, float value at +4.
struct CarRenderPart {
    unsigned char pad00[0x0a];
    unsigned short index0A;
    void *FindRecord(unsigned int hash, void *previous);
};
UG2_CHECK_OFFSET(CarRenderPart, index0A, 0x0a);

struct CarRenderPartTable {
    unsigned char pad00[0x14];
    const short *indices14;
    unsigned char pad18[4];
    unsigned char *records1C;
};
UG2_CHECK_OFFSET(CarRenderPartTable, indices14, 0x14);
UG2_CHECK_OFFSET(CarRenderPartTable, records1C, 0x1c);
extern "C" CarRenderPartTable *ug2CarPartTable8A1CC8;

// ri: CarRenderInfo layout fields of the wheel code. Offsets are original (PC v1.2).
// ri+4 is the car object: byte +0x855 gates the block; +0x604/+0x608 are the rim parts (CarRenderPart);
// the spin term 0x59a420 reads bytes +0x353/+0x354 of the sub-object at car+0x10.
struct CarRenderWheelInfo {
    void *owner000;                         // +0x000: first dword (checked against wheel presence rules)
    unsigned char *carObject;               // +0x004: car object (see above)
    unsigned char *carTypeRow;              // +0x008: CarTypeInfo row (wheel rows at +0x120, stride 0x30)
    unsigned char pad00C[0x390 - 0x00C];
    unsigned int spinStart390;              // +0x390: V0 of the solid-link loop (0x48f6a0 of a constructor argument)
    unsigned char pad394[0x3AC - 0x394];
    unsigned int spinStart3AC;              // +0x3ac: V1 (zeroed by the constructor)
    unsigned char pad3B0[0x400 - 0x3B0];
    float pivot400[8];                      // +0x400: [2w] = -(wheel+0x1c), [2w+1] = -(wheel+0x20)
    float scale420[4];                      // +0x420: per wheel: wheel+0x14, or 0.184 when the rim flag is 0
    float widthFront430;                    // +0x430: front width (MARKER_BRAKE value); feeds set2 only, no draw reads it
    float widthRear434;                     // +0x434: rear width, same use
    float offset438[8];                     // +0x438: [2w] = SPINNEROFFSET float, [2w+1] = same
    unsigned char pad458[0x1B48 - 0x458];
    CarRenderSolid *solid1B48[2][4];        // +0x1b48: [j][L]: j = 0 front tyre, 1 front brake; L = view clamp 0..2
    CarRenderSolid *solid1B68[2][4];        // +0x1b68: [j][L]: j = 0 rear tyre, 1 rear brake
    unsigned char pad1B88[0x2D78 - 0x1B88];
    unsigned char wheelPresent2D78;         // +0x2d78: wheel 0 presence (stride 0x20 per wheel)
    unsigned char pad2D79[0x2D98 - 0x2D79];
    unsigned char wheelPresent2D98;         // +0x2d98: wheel 1
    unsigned char pad2D99[0x2DB8 - 0x2D99];
    unsigned char wheelPresent2DB8;         // +0x2db8: wheel 2
    unsigned char pad2DB9[0x2DD8 - 0x2DB9];
    unsigned char wheelPresent2DD8;         // +0x2dd8: wheel 3
    unsigned char pad2DD9[0x2EB8 - 0x2DD9];
    unsigned char flag2EB8;                 // +0x2eb8: non-zero allows the LOD clamp to 2 (0x6211d9; CarRenderLod.cpp)

    // 0x620600 (__thiscall on ri, RET 8): secondary draw of a tyre matrix for the view (tail, LOD gate 3).
    void Fn620600(CarRenderView *view, WheelMatrix *world);
};

UG2_CHECK_OFFSET(CarRenderWheelInfo, carObject, 0x004);
UG2_CHECK_OFFSET(CarRenderWheelInfo, carTypeRow, 0x008);
UG2_CHECK_OFFSET(CarRenderWheelInfo, spinStart390, 0x390);
UG2_CHECK_OFFSET(CarRenderWheelInfo, spinStart3AC, 0x3ac);
UG2_CHECK_OFFSET(CarRenderWheelInfo, pivot400, 0x400);
UG2_CHECK_OFFSET(CarRenderWheelInfo, scale420, 0x420);
UG2_CHECK_OFFSET(CarRenderWheelInfo, widthFront430, 0x430);
UG2_CHECK_OFFSET(CarRenderWheelInfo, widthRear434, 0x434);
UG2_CHECK_OFFSET(CarRenderWheelInfo, offset438, 0x438);
UG2_CHECK_OFFSET(CarRenderWheelInfo, solid1B48, 0x1b48);
UG2_CHECK_OFFSET(CarRenderWheelInfo, solid1B68, 0x1b68);
UG2_CHECK_OFFSET(CarRenderWheelInfo, wheelPresent2D78, 0x2d78);
UG2_CHECK_OFFSET(CarRenderWheelInfo, wheelPresent2D98, 0x2d98);
UG2_CHECK_OFFSET(CarRenderWheelInfo, wheelPresent2DB8, 0x2db8);
UG2_CHECK_OFFSET(CarRenderWheelInfo, wheelPresent2DD8, 0x2dd8);
UG2_CHECK_OFFSET(CarRenderWheelInfo, flag2EB8, 0x2eb8);

// Original 0x61fe10 (__thiscall on ri, no stack arguments): wheel layout from the CarTypeInfo row
// (ri+8) and the rim parts of the car object (ri+4). Fills ri+0x400..0x43c and ri+0x420..0x42c.
void CarRenderWheelLayout(CarRenderWheelInfo *ri);

// Inputs of the wheel block that the original reads from its caller and from the frame before
// 0x62700e. Each field is explicit; the verifier fills them the same way for both sides.
struct CarWheelDrawInputs {
    CarRenderWheelInfo *ri;          // ecx on entry (saved at frame+0x58)
    CarRenderView *view;             // arg1 [ebp+8]: view object (receiver of DrawSolid; first argument of Visible)
    unsigned int flagsArg8;          // arg8 [ebp+0x24]: OR-ed into the DrawSolid flags
    WheelMatrix *body;               // ebx in the block: the body matrix (a prologue pool block in the original)
    WheelMatrix *tyre;               // arg5 [ebp+0x18]: four car-local tyre frames (GetWheelRenderMatrices)
    WheelMatrix *brake;              // arg6 [ebp+0x1c]: four car-local brake frames
    WheelMatrix *third;              // arg7 [ebp+0x20]: four third-set frames (GetWheelSpinMatrices)
    int lod6C;                       // frame+0x6c: body LOD index (CarLodSelect). Above 2 draws no wheels; <= 1 enables the spin
    int slotD8;                      // frame+0xd8 (k, 0 or 1): selects the ri+0x400 / ri+0x438 column (k source: see docs)
    int lodF4;                       // frame+0xf4: view clamp, raw value, limited to 2 by the block
    unsigned int flagsCC;            // frame+0xcc: DrawSolid flags, first part (tyre blocks; producer 0x60d1b0 not recovered)
    unsigned int flags1AC;           // frame+0x1ac: DrawSolid flags, second part (all blocks; CarLodSelect)
    WheelMatrix *setDump;            // verification output: set0[0..3], set1[0..3], set2[0..3], set3[0..3] (0 in the game)
};

// Wheel block 0x62700e..0x62df66 of 0x620ae0. Zeroes element 13 of tyre[w], brake[w] and third[w]
// (as the original does), builds the per-wheel sets, and draws through the seams below.
void CarWheelDraw(CarWheelDrawInputs *in);

// Platform seams (cdecl, original addresses in comments; the verifier links them by name).
extern "C" {
// 0x48f660 (cdecl): prepares the extra render record, then returns 1 (ignored here).
unsigned int UG2WheelVisible(void *a1, void *a2, const void *a3, void *a4, void *a5, void *a6);
// 0x60d700 (cdecl): unrounded x87 ST0 for the matrix and view; compared with 0x8a1c5c.
long double UG2WheelFn60d700(WheelMatrix *world, void *view);
extern int ug2WheelFacingEnabled802914; // 0x802914
extern float ug2WheelFacingX8A1C54;     // 0x8a1c54
extern float ug2WheelFacingY802910;     // 0x802910
extern float ug2WheelFacingZ8A1C58;     // 0x8a1c58
// 0x401e80 (cdecl): allocation of four wheel matrices (the original passes 4).
void *UG2WheelAllocMatrices(unsigned int count);
// 0x401e30 (cdecl): allocation of 0x700 bytes (four per-wheel records of 0x1c0 bytes).
void *UG2WheelAllocRecords(unsigned int bytes);
}

#endif
