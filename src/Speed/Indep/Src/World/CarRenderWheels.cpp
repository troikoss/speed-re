// Car wheel placement recovered from PC v1.2 speed2.exe: CarRenderWheelLayout (0x61fe10) and the wheel
// block CarWheelDraw (0x62700e..0x62df66 of the car draw 0x620ae0). Addresses are original.
//
// Arithmetic: the original keeps intermediate float results in x87 registers at 53-bit precision and
// rounds only when it stores to memory. Those intermediates are double here, and the stores are float
// at the same points. Matrix elements are generated from the original instruction stream (see the
// WheelChainBlock functions): every element sums its four products in the original order.
#include "CarRenderWheels.h"

void RotateMatrixX(WheelMatrix *output, WheelMatrix *input, unsigned int angle);  // 0x5bacf0 (Camera.cpp)
void RotateMatrixZ(WheelMatrix *output, WheelMatrix *input, unsigned int angle);  // 0x5bae60 (VehicleStateReset.cpp)

// Read-only constants of the executable (values from speed2.exe; the verifier checks the bytes).
static const float kZeroFloat = 0.0f;       // 0x7d7d3c
static const float kHalfFloat = 0.5f;       // 0x8026c0 and 0x784260 (0x3f000000)
static const float kBetaFloat = 0.184f;     // 0x8026c4 (0x3e3c6a7f): fallback scale and width step
static const float kOneFloat = 1.0f;        // 0x784250 (0x3f800000)
static const float kSpinS1 = 0.0f;          // 0x7a6750
static const float kSpinS2 = 7.0f;          // 0x7a6754
static const float kSpinS3 = 0.03f;         // 0x7a6758 (0x3cf5c28f)
static const float kSpinScale = 65536.0f;   // 0x784238

// Run-time copies of the two constants of the 0.184 scale. The division is kept at run time so that the
// x87 precision control of the caller decides its rounding, as in the original FDIV.
extern "C" {
extern volatile float ug2WheelOneRuntime;   // 0x784250 (1.0)
extern volatile float ug2WheelBetaRuntime;  // 0x8026c4 (0.184)
}

// Globals of the executable that the block reads (their initial values are in the image).
extern "C" {
extern float ug2WheelWidthOverrideFront;    // 0x7a668c: 0.0 in PC v1.2; when nonzero it replaces ri+0x430
extern float ug2WheelWidthOverrideRear;     // 0x7a6690: 0.0 in PC v1.2; when nonzero it replaces ri+0x434
extern int ug2WheelLodGate;                 // 0x8654a4: compared with 6 (special path) and 3 (tail)
extern float ug2WheelLodThreshold;          // 0x8a1c5c: compared with the helper results
}

// bStringHash (0x43db50): h = -1; h = h * 33 + c for each character.
static unsigned int WheelStringHash(const char *text) {
    unsigned int h = 0xffffffffu;
    for (; *text; ++text) {
        h = h * 33u + (unsigned char)*text;
    }
    return h;
}

// 0x59a420 (__thiscall on car+0x10, RET 0): 0.5 for each of the bytes +0x354 and +0x353 that is set.
static double SpinTerm(const unsigned char *self) {
    double term = kZeroFloat;
    if (self[0x354]) {
        term = kHalfFloat;
    }
    if (self[0x353]) {
        term = term + kHalfFloat;
    }
    return term;
}

// Original 0x61fe10 (__thiscall on ri, no stack arguments). CarTypeInfo rows are at ri+8: wheel rows at
// +0x120, stride 0x30. The rim parts (car+0x604 and +0x608) give the front (w < 2) and rear flags, which
// are bits 5..7 of byte +5 of each part.
void CarRenderWheelLayout(CarRenderWheelInfo *ri) {
    const unsigned char *row = ri->carTypeRow;
    if (row == 0) {
        for (int i = 0; i < 8; ++i) {
            ri->pivot400[i] = kZeroFloat;   // STOSD with eax = 0 over ri+0x400..0x41f
        }
        return;
    }
    const unsigned char *object = ri->carObject;
    const unsigned char *partA = object ? *(unsigned char *const *)(object + 0x604) : 0;
    const unsigned char *partB = object ? *(unsigned char *const *)(object + 0x608) : 0;
    const unsigned int frontFlag = partA ? (partA[5] >> 5) : 0u;
    const unsigned int rearFlag = partB ? (partB[5] >> 5) : 0u;
    for (int w = 0; w < 4; ++w) {
        const unsigned char *wheel = row + 0x120 + 0x30 * w;
        ri->pivot400[2 * w] = -*(const float *)(wheel + 0x1c);
        ri->pivot400[2 * w + 1] = -*(const float *)(wheel + 0x20);
        if (partA) {
            void *record = reinterpret_cast<CarRenderPart *>(const_cast<unsigned char *>(partA))->FindRecord(
                WheelStringHash("SPINNEROFFSET"), 0);
            const float value = record ? *(const float *)((const unsigned char *)record + 4) : kZeroFloat;
            ri->offset438[2 * w] = value;
            ri->offset438[2 * w + 1] = value;
        }
        const unsigned int flag = (w < 2) ? frontFlag : rearFlag;
        ri->scale420[w] = flag ? *(const float *)(wheel + 0x14) : kBetaFloat;
    }
}

// Wheel presence of the block (0x627592..0x6275c8, 0x6275ba..0x627614): a presence byte, then the owner
// part (dword at ri+0): the byte +0x3f4+w of the owner must be set, or the owner must be absent.
static int WheelPresence(const CarRenderWheelInfo *ri, unsigned char presence, int w) {
    if (presence == 0) {
        return 0;
    }
    const unsigned char *owner = (const unsigned char *)ri->owner000;
    if (owner == 0) {
        return 1;
    }
    return owner[0x3f4 + w] != 0 ? 1 : 0;
}

static void IdentityMatrix(WheelMatrix *m) {
    for (int i = 0; i < 16; ++i) {
        m->elements[i] = 0.0f;
    }
    m->elements[0] = 1.0f;
    m->elements[5] = 1.0f;
    m->elements[10] = 1.0f;
    m->elements[15] = 1.0f;
}

// Generated chain stages: one function per block. Each stage is a 4x4 product of the running matrix (m) with
// its right-hand frame. The element sums keep the original order of their four products (generated from
// the original instruction stream; see docs/NATIVE_REPLACEMENT_INVENTORY.md).
// Block 1 (0x6278aa..0x62871d): front tyre, wheel 0. t = set0[0] x R180; m = t x E; m = m x set1[0]; m = m x BODY.
static void WheelChainBlock1(WheelMatrix *world, const float *s0, const float *e, const float *s1, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 0
    t.elements[0] = s0[0] * r180[0] + s0[3] * r180[12] + s0[2] * r180[8] + s0[1] * r180[4];
    t.elements[1] = r180[1] * s0[0] + s0[3] * r180[13] + s0[2] * r180[9] + s0[1] * r180[5];
    t.elements[2] = r180[2] * s0[0] + s0[3] * r180[14] + s0[2] * r180[10] + s0[1] * r180[6];
    t.elements[3] = r180[3] * s0[0] + s0[3] * r180[15] + s0[2] * r180[11] + s0[1] * r180[7];
    t.elements[4] = s0[7] * r180[12] + s0[6] * r180[8] + s0[5] * r180[4] + s0[4] * r180[0];
    t.elements[5] = s0[7] * r180[13] + s0[6] * r180[9] + s0[5] * r180[5] + s0[4] * r180[1];
    t.elements[6] = s0[7] * r180[14] + s0[6] * r180[10] + s0[5] * r180[6] + s0[4] * r180[2];
    t.elements[7] = s0[7] * r180[15] + s0[6] * r180[11] + s0[5] * r180[7] + s0[4] * r180[3];
    t.elements[8] = s0[11] * r180[12] + s0[10] * r180[8] + s0[9] * r180[4] + s0[8] * r180[0];
    t.elements[9] = s0[11] * r180[13] + s0[10] * r180[9] + s0[9] * r180[5] + s0[8] * r180[1];
    t.elements[10] = s0[11] * r180[14] + s0[10] * r180[10] + s0[9] * r180[6] + s0[8] * r180[2];
    t.elements[11] = s0[11] * r180[15] + s0[10] * r180[11] + s0[9] * r180[7] + s0[8] * r180[3];
    t.elements[12] = s0[15] * r180[12] + s0[14] * r180[8] + s0[13] * r180[4] + s0[12] * r180[0];
    t.elements[13] = s0[15] * r180[13] + s0[14] * r180[9] + s0[13] * r180[5] + s0[12] * r180[1];
    t.elements[14] = s0[15] * r180[14] + s0[14] * r180[10] + s0[13] * r180[6] + s0[12] * r180[2];
    t.elements[15] = s0[15] * r180[15] + s0[14] * r180[11] + s0[13] * r180[7] + s0[12] * r180[3];
    BodyMatrixCopy(&m, &t);
    // trace stage 1
    t.elements[0] = m.elements[3] * e[12] + m.elements[0] * e[0] + m.elements[2] * e[8] + m.elements[1] * e[4];
    t.elements[1] = m.elements[0] * e[1] + m.elements[3] * e[13] + m.elements[2] * e[9] + m.elements[1] * e[5];
    t.elements[2] = m.elements[0] * e[2] + m.elements[3] * e[14] + m.elements[2] * e[10] + m.elements[1] * e[6];
    t.elements[3] = m.elements[0] * e[3] + m.elements[3] * e[15] + m.elements[2] * e[11] + m.elements[1] * e[7];
    t.elements[4] = m.elements[4] * e[0] + m.elements[7] * e[12] + m.elements[6] * e[8] + m.elements[5] * e[4];
    t.elements[5] = m.elements[4] * e[1] + m.elements[7] * e[13] + m.elements[6] * e[9] + m.elements[5] * e[5];
    t.elements[6] = m.elements[7] * e[14] + m.elements[6] * e[10] + m.elements[5] * e[6] + m.elements[4] * e[2];
    t.elements[7] = m.elements[4] * e[3] + m.elements[7] * e[15] + m.elements[6] * e[11] + m.elements[5] * e[7];
    t.elements[8] = m.elements[8] * e[0] + m.elements[11] * e[12] + m.elements[10] * e[8] + m.elements[9] * e[4];
    t.elements[9] = m.elements[8] * e[1] + m.elements[11] * e[13] + m.elements[10] * e[9] + m.elements[9] * e[5];
    t.elements[10] = m.elements[11] * e[14] + m.elements[10] * e[10] + m.elements[9] * e[6] + m.elements[8] * e[2];
    t.elements[11] = m.elements[8] * e[3] + m.elements[11] * e[15] + m.elements[10] * e[11] + m.elements[9] * e[7];
    t.elements[12] = m.elements[12] * e[0] + m.elements[15] * e[12] + m.elements[14] * e[8] + m.elements[13] * e[4];
    t.elements[13] = m.elements[12] * e[1] + m.elements[15] * e[13] + m.elements[14] * e[9] + m.elements[13] * e[5];
    t.elements[14] = m.elements[15] * e[14] + m.elements[14] * e[10] + m.elements[13] * e[6] + m.elements[12] * e[2];
    t.elements[15] = m.elements[12] * e[3] + m.elements[15] * e[15] + m.elements[14] * e[11] + m.elements[13] * e[7];
    BodyMatrixCopy(&m, &t);
    // trace stage 2
    t.elements[0] = s1[0] * m.elements[0] + s1[12] * m.elements[3] + s1[8] * m.elements[2] + s1[4] * m.elements[1];
    t.elements[1] = s1[13] * m.elements[3] + s1[9] * m.elements[2] + s1[5] * m.elements[1] + s1[1] * m.elements[0];
    t.elements[2] = s1[14] * m.elements[3] + s1[10] * m.elements[2] + s1[6] * m.elements[1] + s1[2] * m.elements[0];
    t.elements[3] = s1[15] * m.elements[3] + s1[11] * m.elements[2] + s1[7] * m.elements[1] + s1[3] * m.elements[0];
    t.elements[4] = s1[12] * m.elements[7] + s1[8] * m.elements[6] + s1[4] * m.elements[5] + m.elements[4] * s1[0];
    t.elements[5] = s1[13] * m.elements[7] + s1[9] * m.elements[6] + s1[5] * m.elements[5] + s1[1] * m.elements[4];
    t.elements[6] = s1[14] * m.elements[7] + s1[10] * m.elements[6] + s1[6] * m.elements[5] + s1[2] * m.elements[4];
    t.elements[7] = s1[15] * m.elements[7] + s1[11] * m.elements[6] + s1[7] * m.elements[5] + s1[3] * m.elements[4];
    t.elements[8] = s1[12] * m.elements[11] + s1[8] * m.elements[10] + s1[4] * m.elements[9] + m.elements[8] * s1[0];
    t.elements[9] = s1[13] * m.elements[11] + s1[9] * m.elements[10] + s1[5] * m.elements[9] + s1[1] * m.elements[8];
    t.elements[10] = s1[14] * m.elements[11] + s1[10] * m.elements[10] + s1[6] * m.elements[9] + s1[2] * m.elements[8];
    t.elements[11] = s1[15] * m.elements[11] + s1[11] * m.elements[10] + s1[7] * m.elements[9] + s1[3] * m.elements[8];
    t.elements[12] = s1[12] * m.elements[15] + s1[8] * m.elements[14] + s1[4] * m.elements[13] + m.elements[12] * s1[0];
    t.elements[13] = s1[13] * m.elements[15] + s1[9] * m.elements[14] + s1[5] * m.elements[13] + s1[1] * m.elements[12];
    t.elements[14] = s1[14] * m.elements[15] + s1[10] * m.elements[14] + s1[6] * m.elements[13] + s1[2] * m.elements[12];
    t.elements[15] = s1[15] * m.elements[15] + s1[11] * m.elements[14] + s1[7] * m.elements[13] + s1[3] * m.elements[12];
    BodyMatrixCopy(&m, &t);
    // trace stage 3
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[0] * body[0] + m.elements[1] * body[4];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 2 (0x628741..0x62959b): front brake, wheel 0. t = set0[0] x R180; m = t x E(third0); m = m x set3[0]; m = m x BODY.
static void WheelChainBlock2(WheelMatrix *world, const float *s0, const float *e, const float *s3, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 4
    t.elements[0] = s0[0] * r180[0] + s0[3] * r180[12] + s0[2] * r180[8] + s0[1] * r180[4];
    t.elements[1] = r180[1] * s0[0] + s0[3] * r180[13] + s0[2] * r180[9] + s0[1] * r180[5];
    t.elements[2] = r180[2] * s0[0] + s0[3] * r180[14] + s0[2] * r180[10] + s0[1] * r180[6];
    t.elements[3] = r180[3] * s0[0] + s0[3] * r180[15] + s0[2] * r180[11] + s0[1] * r180[7];
    t.elements[4] = s0[7] * r180[12] + s0[6] * r180[8] + s0[5] * r180[4] + s0[4] * r180[0];
    t.elements[5] = s0[7] * r180[13] + s0[6] * r180[9] + s0[5] * r180[5] + s0[4] * r180[1];
    t.elements[6] = s0[7] * r180[14] + s0[6] * r180[10] + s0[5] * r180[6] + s0[4] * r180[2];
    t.elements[7] = s0[7] * r180[15] + s0[6] * r180[11] + s0[5] * r180[7] + s0[4] * r180[3];
    t.elements[8] = s0[11] * r180[12] + s0[10] * r180[8] + s0[9] * r180[4] + s0[8] * r180[0];
    t.elements[9] = s0[11] * r180[13] + s0[10] * r180[9] + s0[9] * r180[5] + s0[8] * r180[1];
    t.elements[10] = s0[11] * r180[14] + s0[10] * r180[10] + s0[9] * r180[6] + s0[8] * r180[2];
    t.elements[11] = s0[11] * r180[15] + s0[10] * r180[11] + s0[9] * r180[7] + s0[8] * r180[3];
    t.elements[12] = s0[15] * r180[12] + s0[14] * r180[8] + s0[13] * r180[4] + s0[12] * r180[0];
    t.elements[13] = s0[15] * r180[13] + s0[14] * r180[9] + s0[13] * r180[5] + s0[12] * r180[1];
    t.elements[14] = s0[15] * r180[14] + s0[14] * r180[10] + s0[13] * r180[6] + s0[12] * r180[2];
    t.elements[15] = s0[15] * r180[15] + s0[14] * r180[11] + s0[13] * r180[7] + s0[12] * r180[3];
    BodyMatrixCopy(&m, &t);
    // trace stage 5
    t.elements[0] = m.elements[2] * e[8] + m.elements[1] * e[4] + m.elements[0] * e[0] + m.elements[3] * e[12];
    t.elements[1] = m.elements[3] * e[13] + m.elements[2] * e[9] + m.elements[1] * e[5] + m.elements[0] * e[1];
    t.elements[2] = m.elements[0] * e[2] + m.elements[3] * e[14] + m.elements[2] * e[10] + m.elements[1] * e[6];
    t.elements[3] = m.elements[3] * e[15] + m.elements[2] * e[11] + m.elements[1] * e[7] + m.elements[0] * e[3];
    t.elements[4] = m.elements[6] * e[8] + m.elements[5] * e[4] + m.elements[7] * e[12] + m.elements[4] * e[0];
    t.elements[5] = m.elements[7] * e[13] + m.elements[6] * e[9] + m.elements[5] * e[5] + m.elements[4] * e[1];
    t.elements[6] = m.elements[4] * e[2] + m.elements[7] * e[14] + m.elements[6] * e[10] + m.elements[5] * e[6];
    t.elements[7] = m.elements[7] * e[15] + m.elements[6] * e[11] + m.elements[5] * e[7] + m.elements[4] * e[3];
    t.elements[8] = m.elements[10] * e[8] + m.elements[9] * e[4] + m.elements[11] * e[12] + m.elements[8] * e[0];
    t.elements[9] = m.elements[11] * e[13] + m.elements[10] * e[9] + m.elements[9] * e[5] + m.elements[8] * e[1];
    t.elements[10] = m.elements[8] * e[2] + m.elements[11] * e[14] + m.elements[10] * e[10] + m.elements[9] * e[6];
    t.elements[11] = m.elements[11] * e[15] + m.elements[10] * e[11] + m.elements[9] * e[7] + m.elements[8] * e[3];
    t.elements[12] = m.elements[14] * e[8] + m.elements[13] * e[4] + m.elements[15] * e[12] + m.elements[12] * e[0];
    t.elements[13] = m.elements[15] * e[13] + m.elements[14] * e[9] + m.elements[13] * e[5] + m.elements[12] * e[1];
    t.elements[14] = m.elements[12] * e[2] + m.elements[15] * e[14] + m.elements[14] * e[10] + m.elements[13] * e[6];
    t.elements[15] = m.elements[15] * e[15] + m.elements[14] * e[11] + m.elements[13] * e[7] + m.elements[12] * e[3];
    BodyMatrixCopy(&m, &t);
    // trace stage 6
    t.elements[0] = s3[0] * m.elements[0] + s3[12] * m.elements[3] + s3[8] * m.elements[2] + s3[4] * m.elements[1];
    t.elements[1] = s3[1] * m.elements[0] + s3[13] * m.elements[3] + s3[9] * m.elements[2] + s3[5] * m.elements[1];
    t.elements[2] = s3[2] * m.elements[0] + s3[14] * m.elements[3] + s3[10] * m.elements[2] + s3[6] * m.elements[1];
    t.elements[3] = s3[3] * m.elements[0] + s3[15] * m.elements[3] + s3[11] * m.elements[2] + s3[7] * m.elements[1];
    t.elements[4] = s3[12] * m.elements[7] + s3[8] * m.elements[6] + s3[4] * m.elements[5] + m.elements[4] * s3[0];
    t.elements[5] = s3[13] * m.elements[7] + s3[9] * m.elements[6] + s3[5] * m.elements[5] + s3[1] * m.elements[4];
    t.elements[6] = s3[14] * m.elements[7] + s3[10] * m.elements[6] + s3[6] * m.elements[5] + s3[2] * m.elements[4];
    t.elements[7] = s3[15] * m.elements[7] + s3[11] * m.elements[6] + s3[7] * m.elements[5] + s3[3] * m.elements[4];
    t.elements[8] = s3[12] * m.elements[11] + s3[8] * m.elements[10] + s3[4] * m.elements[9] + m.elements[8] * s3[0];
    t.elements[9] = s3[13] * m.elements[11] + s3[9] * m.elements[10] + s3[5] * m.elements[9] + s3[1] * m.elements[8];
    t.elements[10] = s3[14] * m.elements[11] + s3[10] * m.elements[10] + s3[6] * m.elements[9] + s3[2] * m.elements[8];
    t.elements[11] = s3[15] * m.elements[11] + s3[11] * m.elements[10] + s3[7] * m.elements[9] + s3[3] * m.elements[8];
    t.elements[12] = s3[12] * m.elements[15] + s3[8] * m.elements[14] + s3[4] * m.elements[13] + m.elements[12] * s3[0];
    t.elements[13] = s3[13] * m.elements[15] + s3[9] * m.elements[14] + s3[5] * m.elements[13] + s3[1] * m.elements[12];
    t.elements[14] = s3[14] * m.elements[15] + s3[10] * m.elements[14] + s3[6] * m.elements[13] + s3[2] * m.elements[12];
    t.elements[15] = s3[15] * m.elements[15] + s3[11] * m.elements[14] + s3[7] * m.elements[13] + s3[3] * m.elements[12];
    BodyMatrixCopy(&m, &t);
    // trace stage 7
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[0] * body[0] + m.elements[1] * body[4];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 3 (0x6295a0..0x62a084): front tyre, wheel 1 (no R180). t = set0[1] x E(tyre1); m = t x set1[1]; m = m x BODY.
static void WheelChainBlock3(WheelMatrix *world, const float *s0, const float *e, const float *s1, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 8
    t.elements[0] = s0[1] * e[4] + s0[0] * e[0] + s0[2] * e[8] + s0[3] * e[12];
    t.elements[1] = s0[1] * e[5] + s0[0] * e[1] + s0[2] * e[9] + s0[3] * e[13];
    t.elements[2] = s0[3] * e[14] + s0[1] * e[6] + s0[0] * e[2] + s0[2] * e[10];
    t.elements[3] = s0[0] * e[3] + s0[2] * e[11] + s0[1] * e[7] + s0[3] * e[15];
    t.elements[4] = s0[5] * e[4] + s0[4] * e[0] + s0[6] * e[8] + s0[7] * e[12];
    t.elements[5] = s0[5] * e[5] + s0[4] * e[1] + s0[6] * e[9] + s0[7] * e[13];
    t.elements[6] = s0[7] * e[14] + s0[5] * e[6] + s0[4] * e[2] + s0[6] * e[10];
    t.elements[7] = s0[4] * e[3] + s0[6] * e[11] + s0[5] * e[7] + s0[7] * e[15];
    t.elements[8] = s0[9] * e[4] + s0[8] * e[0] + s0[10] * e[8] + s0[11] * e[12];
    t.elements[9] = s0[9] * e[5] + s0[8] * e[1] + s0[10] * e[9] + s0[11] * e[13];
    t.elements[10] = s0[11] * e[14] + s0[9] * e[6] + s0[8] * e[2] + s0[10] * e[10];
    t.elements[11] = s0[8] * e[3] + s0[10] * e[11] + s0[9] * e[7] + s0[11] * e[15];
    t.elements[12] = s0[13] * e[4] + s0[15] * e[12] + s0[14] * e[8] + s0[12] * e[0];
    t.elements[13] = s0[13] * e[5] + s0[15] * e[13] + s0[12] * e[1] + s0[14] * e[9];
    t.elements[14] = s0[15] * e[14] + s0[12] * e[2] + s0[14] * e[10] + s0[13] * e[6];
    t.elements[15] = s0[12] * e[3] + s0[14] * e[11] + s0[15] * e[15] + s0[13] * e[7];
    BodyMatrixCopy(&m, &t);
    // trace stage 9
    t.elements[0] = m.elements[0] * s1[0] + m.elements[1] * s1[4] + m.elements[2] * s1[8] + m.elements[3] * s1[12];
    t.elements[1] = m.elements[0] * s1[1] + m.elements[1] * s1[5] + m.elements[2] * s1[9] + m.elements[3] * s1[13];
    t.elements[2] = m.elements[0] * s1[2] + m.elements[1] * s1[6] + m.elements[2] * s1[10] + m.elements[3] * s1[14];
    t.elements[3] = s1[3] * m.elements[0] + m.elements[1] * s1[7] + m.elements[2] * s1[11] + m.elements[3] * s1[15];
    t.elements[4] = m.elements[5] * s1[4] + m.elements[4] * s1[0] + m.elements[6] * s1[8] + m.elements[7] * s1[12];
    t.elements[5] = m.elements[4] * s1[1] + m.elements[5] * s1[5] + m.elements[6] * s1[9] + m.elements[7] * s1[13];
    t.elements[6] = m.elements[4] * s1[2] + m.elements[5] * s1[6] + m.elements[6] * s1[10] + m.elements[7] * s1[14];
    t.elements[7] = m.elements[4] * s1[3] + m.elements[5] * s1[7] + m.elements[6] * s1[11] + m.elements[7] * s1[15];
    t.elements[8] = m.elements[9] * s1[4] + m.elements[8] * s1[0] + m.elements[10] * s1[8] + m.elements[11] * s1[12];
    t.elements[9] = m.elements[8] * s1[1] + m.elements[9] * s1[5] + m.elements[10] * s1[9] + m.elements[11] * s1[13];
    t.elements[10] = m.elements[8] * s1[2] + m.elements[9] * s1[6] + m.elements[10] * s1[10] + m.elements[11] * s1[14];
    t.elements[11] = m.elements[8] * s1[3] + m.elements[9] * s1[7] + m.elements[10] * s1[11] + m.elements[11] * s1[15];
    t.elements[12] = m.elements[13] * s1[4] + m.elements[12] * s1[0] + m.elements[14] * s1[8] + m.elements[15] * s1[12];
    t.elements[13] = m.elements[12] * s1[1] + m.elements[13] * s1[5] + m.elements[14] * s1[9] + m.elements[15] * s1[13];
    t.elements[14] = m.elements[12] * s1[2] + m.elements[13] * s1[6] + m.elements[14] * s1[10] + m.elements[15] * s1[14];
    t.elements[15] = m.elements[12] * s1[3] + m.elements[13] * s1[7] + m.elements[14] * s1[11] + m.elements[15] * s1[15];
    BodyMatrixCopy(&m, &t);
    // trace stage 10
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[0] * body[0] + m.elements[1] * body[4];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[3] * body[13] + m.elements[0] * body[1];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[3] * body[14] + m.elements[0] * body[2];
    t.elements[3] = m.elements[3] * body[15] + m.elements[0] * body[3] + m.elements[2] * body[11] + m.elements[1] * body[7];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 4 (0x62a0a4..0x62ab64): front brake, wheel 1. t = set0[1] x E(third1); m = t x set3[1]; m = m x BODY.
static void WheelChainBlock4(WheelMatrix *world, const float *s0, const float *e, const float *s3, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 11
    t.elements[0] = s0[0] * e[0] + s0[1] * e[4] + s0[2] * e[8] + s0[3] * e[12];
    t.elements[1] = s0[0] * e[1] + s0[2] * e[9] + s0[1] * e[5] + s0[3] * e[13];
    t.elements[2] = s0[3] * e[14] + s0[0] * e[2] + s0[2] * e[10] + s0[1] * e[6];
    t.elements[3] = s0[1] * e[7] + s0[0] * e[3] + s0[2] * e[11] + s0[3] * e[15];
    t.elements[4] = s0[4] * e[0] + s0[5] * e[4] + s0[6] * e[8] + s0[7] * e[12];
    t.elements[5] = s0[4] * e[1] + s0[6] * e[9] + s0[5] * e[5] + s0[7] * e[13];
    t.elements[6] = s0[7] * e[14] + s0[4] * e[2] + s0[6] * e[10] + s0[5] * e[6];
    t.elements[7] = s0[5] * e[7] + s0[4] * e[3] + s0[6] * e[11] + s0[7] * e[15];
    t.elements[8] = s0[8] * e[0] + s0[9] * e[4] + s0[10] * e[8] + s0[11] * e[12];
    t.elements[9] = s0[8] * e[1] + s0[10] * e[9] + s0[9] * e[5] + s0[11] * e[13];
    t.elements[10] = s0[11] * e[14] + s0[8] * e[2] + s0[10] * e[10] + s0[9] * e[6];
    t.elements[11] = s0[9] * e[7] + s0[8] * e[3] + s0[10] * e[11] + s0[11] * e[15];
    t.elements[12] = s0[14] * e[8] + s0[15] * e[12] + s0[12] * e[0] + s0[13] * e[4];
    t.elements[13] = s0[12] * e[1] + s0[14] * e[9] + s0[15] * e[13] + s0[13] * e[5];
    t.elements[14] = s0[13] * e[6] + s0[15] * e[14] + s0[12] * e[2] + s0[14] * e[10];
    t.elements[15] = s0[13] * e[7] + s0[15] * e[15] + s0[12] * e[3] + s0[14] * e[11];
    BodyMatrixCopy(&m, &t);
    // trace stage 12
    t.elements[0] = m.elements[1] * s3[4] + m.elements[2] * s3[8] + m.elements[3] * s3[12] + s3[0] * m.elements[0];
    t.elements[1] = m.elements[1] * s3[5] + m.elements[2] * s3[9] + m.elements[3] * s3[13] + s3[1] * m.elements[0];
    t.elements[2] = m.elements[1] * s3[6] + m.elements[2] * s3[10] + m.elements[3] * s3[14] + s3[2] * m.elements[0];
    t.elements[3] = m.elements[3] * s3[15] + s3[3] * m.elements[0] + m.elements[1] * s3[7] + m.elements[2] * s3[11];
    t.elements[4] = m.elements[5] * s3[4] + m.elements[4] * s3[0] + m.elements[6] * s3[8] + m.elements[7] * s3[12];
    t.elements[5] = m.elements[5] * s3[5] + m.elements[6] * s3[9] + m.elements[7] * s3[13] + m.elements[4] * s3[1];
    t.elements[6] = m.elements[5] * s3[6] + m.elements[6] * s3[10] + m.elements[7] * s3[14] + m.elements[4] * s3[2];
    t.elements[7] = m.elements[7] * s3[15] + m.elements[4] * s3[3] + m.elements[5] * s3[7] + m.elements[6] * s3[11];
    t.elements[8] = m.elements[9] * s3[4] + m.elements[8] * s3[0] + m.elements[10] * s3[8] + m.elements[11] * s3[12];
    t.elements[9] = m.elements[9] * s3[5] + m.elements[10] * s3[9] + m.elements[11] * s3[13] + m.elements[8] * s3[1];
    t.elements[10] = m.elements[9] * s3[6] + m.elements[10] * s3[10] + m.elements[11] * s3[14] + m.elements[8] * s3[2];
    t.elements[11] = m.elements[11] * s3[15] + m.elements[8] * s3[3] + m.elements[9] * s3[7] + m.elements[10] * s3[11];
    t.elements[12] = m.elements[13] * s3[4] + m.elements[12] * s3[0] + m.elements[14] * s3[8] + m.elements[15] * s3[12];
    t.elements[13] = m.elements[13] * s3[5] + m.elements[14] * s3[9] + m.elements[15] * s3[13] + m.elements[12] * s3[1];
    t.elements[14] = m.elements[13] * s3[6] + m.elements[14] * s3[10] + m.elements[15] * s3[14] + m.elements[12] * s3[2];
    t.elements[15] = m.elements[15] * s3[15] + m.elements[12] * s3[3] + m.elements[13] * s3[7] + m.elements[14] * s3[11];
    BodyMatrixCopy(&m, &t);
    // trace stage 13
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[1] * body[4] + m.elements[0] * body[0];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 5 (0x62ab69..0x62b68d): rear tyre, wheel 2 (no R180). t = set0[2] x TYRE2; m = t x set1[2]; m = m x BODY.
static void WheelChainBlock5(WheelMatrix *world, const float *s0, const float *e, const float *s1, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 14
    t.elements[0] = s0[1] * e[4] + s0[0] * e[0] + s0[2] * e[8] + s0[3] * e[12];
    t.elements[1] = s0[0] * e[1] + s0[1] * e[5] + s0[2] * e[9] + s0[3] * e[13];
    t.elements[2] = s0[3] * e[14] + s0[0] * e[2] + s0[2] * e[10] + s0[1] * e[6];
    t.elements[3] = s0[0] * e[3] + s0[1] * e[7] + s0[2] * e[11] + s0[3] * e[15];
    t.elements[4] = s0[5] * e[4] + s0[4] * e[0] + s0[6] * e[8] + s0[7] * e[12];
    t.elements[5] = s0[4] * e[1] + s0[5] * e[5] + s0[6] * e[9] + s0[7] * e[13];
    t.elements[6] = s0[7] * e[14] + s0[4] * e[2] + s0[6] * e[10] + s0[5] * e[6];
    t.elements[7] = s0[4] * e[3] + s0[5] * e[7] + s0[6] * e[11] + s0[7] * e[15];
    t.elements[8] = s0[9] * e[4] + s0[8] * e[0] + s0[10] * e[8] + s0[11] * e[12];
    t.elements[9] = s0[8] * e[1] + s0[9] * e[5] + s0[10] * e[9] + s0[11] * e[13];
    t.elements[10] = s0[11] * e[14] + s0[8] * e[2] + s0[10] * e[10] + s0[9] * e[6];
    t.elements[11] = s0[8] * e[3] + s0[9] * e[7] + s0[10] * e[11] + s0[11] * e[15];
    t.elements[12] = s0[13] * e[4] + s0[12] * e[0] + s0[14] * e[8] + s0[15] * e[12];
    t.elements[13] = s0[12] * e[1] + s0[13] * e[5] + s0[14] * e[9] + s0[15] * e[13];
    t.elements[14] = s0[15] * e[14] + s0[12] * e[2] + s0[13] * e[6] + s0[14] * e[10];
    t.elements[15] = s0[12] * e[3] + s0[13] * e[7] + s0[14] * e[11] + s0[15] * e[15];
    BodyMatrixCopy(&m, &t);
    // trace stage 15
    t.elements[0] = s1[12] * m.elements[3] + s1[8] * m.elements[2] + s1[4] * m.elements[1] + s1[0] * m.elements[0];
    t.elements[1] = s1[13] * m.elements[3] + s1[9] * m.elements[2] + s1[5] * m.elements[1] + s1[1] * m.elements[0];
    t.elements[2] = s1[14] * m.elements[3] + s1[10] * m.elements[2] + s1[6] * m.elements[1] + s1[2] * m.elements[0];
    t.elements[3] = s1[15] * m.elements[3] + s1[11] * m.elements[2] + s1[7] * m.elements[1] + m.elements[0] * s1[3];
    t.elements[4] = s1[12] * m.elements[7] + s1[8] * m.elements[6] + s1[4] * m.elements[5] + s1[0] * m.elements[4];
    t.elements[5] = s1[13] * m.elements[7] + s1[9] * m.elements[6] + s1[5] * m.elements[5] + s1[1] * m.elements[4];
    t.elements[6] = s1[14] * m.elements[7] + s1[10] * m.elements[6] + s1[6] * m.elements[5] + s1[2] * m.elements[4];
    t.elements[7] = s1[15] * m.elements[7] + s1[11] * m.elements[6] + s1[7] * m.elements[5] + m.elements[4] * s1[3];
    t.elements[8] = s1[12] * m.elements[11] + s1[8] * m.elements[10] + s1[4] * m.elements[9] + s1[0] * m.elements[8];
    t.elements[9] = s1[13] * m.elements[11] + s1[9] * m.elements[10] + s1[5] * m.elements[9] + s1[1] * m.elements[8];
    t.elements[10] = s1[14] * m.elements[11] + s1[10] * m.elements[10] + s1[6] * m.elements[9] + s1[2] * m.elements[8];
    t.elements[11] = s1[15] * m.elements[11] + s1[11] * m.elements[10] + s1[7] * m.elements[9] + m.elements[8] * s1[3];
    t.elements[12] = s1[12] * m.elements[15] + s1[8] * m.elements[14] + s1[4] * m.elements[13] + s1[0] * m.elements[12];
    t.elements[13] = s1[13] * m.elements[15] + s1[9] * m.elements[14] + s1[5] * m.elements[13] + s1[1] * m.elements[12];
    t.elements[14] = s1[14] * m.elements[15] + s1[10] * m.elements[14] + s1[6] * m.elements[13] + s1[2] * m.elements[12];
    t.elements[15] = s1[15] * m.elements[15] + s1[11] * m.elements[14] + s1[7] * m.elements[13] + m.elements[12] * s1[3];
    BodyMatrixCopy(&m, &t);
    // trace stage 16
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[0] * body[0] + m.elements[1] * body[4];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 6 (0x62b692..0x62c169): rear brake, wheel 2. t = set0[2] x THIRD2; m = t x set3[2]; m = m x BODY.
static void WheelChainBlock6(WheelMatrix *world, const float *s0, const float *e, const float *s3, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 17
    t.elements[0] = s0[2] * e[8] + s0[3] * e[12] + s0[0] * e[0] + s0[1] * e[4];
    t.elements[1] = s0[0] * e[1] + s0[1] * e[5] + s0[2] * e[9] + s0[3] * e[13];
    t.elements[2] = s0[3] * e[14] + s0[1] * e[6] + s0[0] * e[2] + s0[2] * e[10];
    t.elements[3] = s0[0] * e[3] + s0[1] * e[7] + s0[2] * e[11] + s0[3] * e[15];
    t.elements[4] = s0[6] * e[8] + s0[7] * e[12] + s0[4] * e[0] + s0[5] * e[4];
    t.elements[5] = s0[4] * e[1] + s0[5] * e[5] + s0[6] * e[9] + s0[7] * e[13];
    t.elements[6] = s0[7] * e[14] + s0[5] * e[6] + s0[4] * e[2] + s0[6] * e[10];
    t.elements[7] = s0[4] * e[3] + s0[5] * e[7] + s0[6] * e[11] + s0[7] * e[15];
    t.elements[8] = s0[10] * e[8] + s0[11] * e[12] + s0[8] * e[0] + s0[9] * e[4];
    t.elements[9] = s0[8] * e[1] + s0[9] * e[5] + s0[10] * e[9] + s0[11] * e[13];
    t.elements[10] = s0[11] * e[14] + s0[9] * e[6] + s0[8] * e[2] + s0[10] * e[10];
    t.elements[11] = s0[8] * e[3] + s0[9] * e[7] + s0[10] * e[11] + s0[11] * e[15];
    t.elements[12] = s0[14] * e[8] + s0[15] * e[12] + s0[12] * e[0] + s0[13] * e[4];
    t.elements[13] = s0[12] * e[1] + s0[13] * e[5] + s0[14] * e[9] + s0[15] * e[13];
    t.elements[14] = s0[15] * e[14] + s0[12] * e[2] + s0[13] * e[6] + s0[14] * e[10];
    t.elements[15] = s0[12] * e[3] + s0[13] * e[7] + s0[14] * e[11] + s0[15] * e[15];
    BodyMatrixCopy(&m, &t);
    // trace stage 18
    t.elements[0] = m.elements[0] * s3[0] + m.elements[1] * s3[4] + m.elements[2] * s3[8] + m.elements[3] * s3[12];
    t.elements[1] = m.elements[0] * s3[1] + m.elements[1] * s3[5] + m.elements[2] * s3[9] + m.elements[3] * s3[13];
    t.elements[2] = m.elements[0] * s3[2] + m.elements[1] * s3[6] + m.elements[2] * s3[10] + m.elements[3] * s3[14];
    t.elements[3] = m.elements[1] * s3[7] + m.elements[0] * s3[3] + m.elements[2] * s3[11] + m.elements[3] * s3[15];
    t.elements[4] = m.elements[5] * s3[4] + m.elements[4] * s3[0] + m.elements[6] * s3[8] + m.elements[7] * s3[12];
    t.elements[5] = m.elements[5] * s3[5] + m.elements[6] * s3[9] + m.elements[7] * s3[13] + m.elements[4] * s3[1];
    t.elements[6] = m.elements[5] * s3[6] + m.elements[6] * s3[10] + m.elements[7] * s3[14] + m.elements[4] * s3[2];
    t.elements[7] = m.elements[6] * s3[11] + m.elements[7] * s3[15] + m.elements[4] * s3[3] + m.elements[5] * s3[7];
    t.elements[8] = m.elements[9] * s3[4] + m.elements[8] * s3[0] + m.elements[10] * s3[8] + m.elements[11] * s3[12];
    t.elements[9] = m.elements[9] * s3[5] + m.elements[10] * s3[9] + m.elements[11] * s3[13] + m.elements[8] * s3[1];
    t.elements[10] = m.elements[9] * s3[6] + m.elements[10] * s3[10] + m.elements[11] * s3[14] + m.elements[8] * s3[2];
    t.elements[11] = m.elements[10] * s3[11] + m.elements[11] * s3[15] + m.elements[8] * s3[3] + m.elements[9] * s3[7];
    t.elements[12] = m.elements[13] * s3[4] + m.elements[12] * s3[0] + m.elements[14] * s3[8] + m.elements[15] * s3[12];
    t.elements[13] = m.elements[13] * s3[5] + m.elements[14] * s3[9] + m.elements[15] * s3[13] + m.elements[12] * s3[1];
    t.elements[14] = m.elements[13] * s3[6] + m.elements[14] * s3[10] + m.elements[15] * s3[14] + m.elements[12] * s3[2];
    t.elements[15] = m.elements[14] * s3[11] + m.elements[15] * s3[15] + m.elements[12] * s3[3] + m.elements[13] * s3[7];
    BodyMatrixCopy(&m, &t);
    // trace stage 19
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[1] * body[4] + m.elements[0] * body[0];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[0] * body[3] + m.elements[2] * body[11] + m.elements[1] * body[7];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 7 (0x62c16e..0x62d049): rear tyre, wheel 3. t = set0[3] x R180; m = t x TYRE3; m = m x set1[3]; m = m x BODY.
static void WheelChainBlock7(WheelMatrix *world, const float *s0, const float *e, const float *s1, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 20
    t.elements[0] = r180[12] * s0[3] + r180[8] * s0[2] + r180[4] * s0[1] + s0[0] * r180[0];
    t.elements[1] = r180[13] * s0[3] + r180[9] * s0[2] + r180[5] * s0[1] + r180[1] * s0[0];
    t.elements[2] = r180[14] * s0[3] + r180[10] * s0[2] + r180[6] * s0[1] + r180[2] * s0[0];
    t.elements[3] = r180[15] * s0[3] + r180[11] * s0[2] + r180[7] * s0[1] + r180[3] * s0[0];
    t.elements[4] = r180[12] * s0[7] + r180[8] * s0[6] + r180[4] * s0[5] + s0[4] * r180[0];
    t.elements[5] = r180[13] * s0[7] + r180[9] * s0[6] + r180[5] * s0[5] + r180[1] * s0[4];
    t.elements[6] = r180[14] * s0[7] + r180[10] * s0[6] + r180[6] * s0[5] + r180[2] * s0[4];
    t.elements[7] = r180[15] * s0[7] + r180[11] * s0[6] + r180[7] * s0[5] + r180[3] * s0[4];
    t.elements[8] = r180[12] * s0[11] + r180[8] * s0[10] + r180[4] * s0[9] + s0[8] * r180[0];
    t.elements[9] = r180[13] * s0[11] + r180[9] * s0[10] + r180[5] * s0[9] + r180[1] * s0[8];
    t.elements[10] = r180[14] * s0[11] + r180[10] * s0[10] + r180[6] * s0[9] + r180[2] * s0[8];
    t.elements[11] = r180[15] * s0[11] + r180[11] * s0[10] + r180[7] * s0[9] + r180[3] * s0[8];
    t.elements[12] = r180[12] * s0[15] + r180[8] * s0[14] + r180[4] * s0[13] + s0[12] * r180[0];
    t.elements[13] = r180[13] * s0[15] + r180[9] * s0[14] + r180[5] * s0[13] + r180[1] * s0[12];
    t.elements[14] = r180[14] * s0[15] + r180[10] * s0[14] + r180[6] * s0[13] + r180[2] * s0[12];
    t.elements[15] = r180[15] * s0[15] + r180[11] * s0[14] + r180[7] * s0[13] + r180[3] * s0[12];
    BodyMatrixCopy(&m, &t);
    // trace stage 21
    t.elements[0] = m.elements[3] * e[12] + m.elements[2] * e[8] + m.elements[1] * e[4] + m.elements[0] * e[0];
    t.elements[1] = m.elements[3] * e[13] + m.elements[2] * e[9] + m.elements[1] * e[5] + m.elements[0] * e[1];
    t.elements[2] = m.elements[2] * e[10] + m.elements[1] * e[6] + m.elements[0] * e[2] + m.elements[3] * e[14];
    t.elements[3] = m.elements[3] * e[15] + m.elements[2] * e[11] + m.elements[1] * e[7] + m.elements[0] * e[3];
    t.elements[4] = m.elements[7] * e[12] + m.elements[4] * e[0] + m.elements[6] * e[8] + m.elements[5] * e[4];
    t.elements[5] = m.elements[7] * e[13] + m.elements[6] * e[9] + m.elements[5] * e[5] + m.elements[4] * e[1];
    t.elements[6] = m.elements[6] * e[10] + m.elements[5] * e[6] + m.elements[4] * e[2] + m.elements[7] * e[14];
    t.elements[7] = m.elements[7] * e[15] + m.elements[6] * e[11] + m.elements[5] * e[7] + m.elements[4] * e[3];
    t.elements[8] = m.elements[11] * e[12] + m.elements[8] * e[0] + m.elements[10] * e[8] + m.elements[9] * e[4];
    t.elements[9] = m.elements[11] * e[13] + m.elements[10] * e[9] + m.elements[9] * e[5] + m.elements[8] * e[1];
    t.elements[10] = m.elements[10] * e[10] + m.elements[9] * e[6] + m.elements[8] * e[2] + m.elements[11] * e[14];
    t.elements[11] = m.elements[11] * e[15] + m.elements[10] * e[11] + m.elements[9] * e[7] + m.elements[8] * e[3];
    t.elements[12] = m.elements[15] * e[12] + m.elements[12] * e[0] + m.elements[14] * e[8] + m.elements[13] * e[4];
    t.elements[13] = m.elements[15] * e[13] + m.elements[14] * e[9] + m.elements[13] * e[5] + m.elements[12] * e[1];
    t.elements[14] = m.elements[14] * e[10] + m.elements[13] * e[6] + m.elements[12] * e[2] + m.elements[15] * e[14];
    t.elements[15] = m.elements[15] * e[15] + m.elements[14] * e[11] + m.elements[13] * e[7] + m.elements[12] * e[3];
    BodyMatrixCopy(&m, &t);
    // trace stage 22
    t.elements[0] = m.elements[1] * s1[4] + m.elements[2] * s1[8] + m.elements[3] * s1[12] + s1[0] * m.elements[0];
    t.elements[1] = m.elements[1] * s1[5] + m.elements[2] * s1[9] + m.elements[3] * s1[13] + s1[1] * m.elements[0];
    t.elements[2] = m.elements[1] * s1[6] + m.elements[2] * s1[10] + m.elements[3] * s1[14] + s1[2] * m.elements[0];
    t.elements[3] = m.elements[1] * s1[7] + m.elements[2] * s1[11] + m.elements[3] * s1[15] + s1[3] * m.elements[0];
    t.elements[4] = m.elements[5] * s1[4] + m.elements[4] * s1[0] + m.elements[6] * s1[8] + m.elements[7] * s1[12];
    t.elements[5] = m.elements[4] * s1[1] + m.elements[5] * s1[5] + m.elements[6] * s1[9] + m.elements[7] * s1[13];
    t.elements[6] = m.elements[4] * s1[2] + m.elements[5] * s1[6] + m.elements[6] * s1[10] + m.elements[7] * s1[14];
    t.elements[7] = m.elements[4] * s1[3] + m.elements[5] * s1[7] + m.elements[6] * s1[11] + m.elements[7] * s1[15];
    t.elements[8] = m.elements[9] * s1[4] + m.elements[8] * s1[0] + m.elements[10] * s1[8] + m.elements[11] * s1[12];
    t.elements[9] = m.elements[8] * s1[1] + m.elements[9] * s1[5] + m.elements[10] * s1[9] + m.elements[11] * s1[13];
    t.elements[10] = m.elements[8] * s1[2] + m.elements[9] * s1[6] + m.elements[10] * s1[10] + m.elements[11] * s1[14];
    t.elements[11] = m.elements[8] * s1[3] + m.elements[9] * s1[7] + m.elements[10] * s1[11] + m.elements[11] * s1[15];
    t.elements[12] = m.elements[13] * s1[4] + m.elements[12] * s1[0] + m.elements[14] * s1[8] + m.elements[15] * s1[12];
    t.elements[13] = m.elements[12] * s1[1] + m.elements[13] * s1[5] + m.elements[14] * s1[9] + m.elements[15] * s1[13];
    t.elements[14] = m.elements[12] * s1[2] + m.elements[13] * s1[6] + m.elements[14] * s1[10] + m.elements[15] * s1[14];
    t.elements[15] = m.elements[12] * s1[3] + m.elements[13] * s1[7] + m.elements[14] * s1[11] + m.elements[15] * s1[15];
    BodyMatrixCopy(&m, &t);
    // trace stage 23
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[1] * body[4] + m.elements[0] * body[0];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[7] * body[12] + m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[7] * body[15] + m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3];
    t.elements[8] = m.elements[11] * body[12] + m.elements[10] * body[8] + m.elements[9] * body[4] + m.elements[8] * body[0];
    t.elements[9] = m.elements[10] * body[9] + m.elements[9] * body[5] + m.elements[8] * body[1] + m.elements[11] * body[13];
    t.elements[10] = m.elements[10] * body[10] + m.elements[9] * body[6] + m.elements[8] * body[2] + m.elements[11] * body[14];
    t.elements[11] = m.elements[11] * body[15] + m.elements[10] * body[11] + m.elements[9] * body[7] + m.elements[8] * body[3];
    t.elements[12] = m.elements[15] * body[12] + m.elements[14] * body[8] + m.elements[13] * body[4] + m.elements[12] * body[0];
    t.elements[13] = m.elements[14] * body[9] + m.elements[13] * body[5] + m.elements[12] * body[1] + m.elements[15] * body[13];
    t.elements[14] = m.elements[14] * body[10] + m.elements[13] * body[6] + m.elements[12] * body[2] + m.elements[15] * body[14];
    t.elements[15] = m.elements[15] * body[15] + m.elements[14] * body[11] + m.elements[13] * body[7] + m.elements[12] * body[3];
    BodyMatrixCopy(world, &t);
}

// Block 8 (0x62d04e..0x62dea8): rear brake, wheel 3. t = set0[3] x R180; m = t x THIRD3; m = m x set3[3]; m = m x BODY.
static void WheelChainBlock8(WheelMatrix *world, const float *s0, const float *e, const float *s3, const float *body, const float *r180) {
    WheelMatrix m, t;
    // trace stage 24
    t.elements[0] = r180[12] * s0[3] + r180[8] * s0[2] + r180[4] * s0[1] + s0[0] * r180[0];
    t.elements[1] = r180[13] * s0[3] + r180[9] * s0[2] + r180[5] * s0[1] + r180[1] * s0[0];
    t.elements[2] = r180[14] * s0[3] + r180[10] * s0[2] + r180[6] * s0[1] + r180[2] * s0[0];
    t.elements[3] = r180[15] * s0[3] + r180[11] * s0[2] + r180[7] * s0[1] + r180[3] * s0[0];
    t.elements[4] = r180[12] * s0[7] + r180[8] * s0[6] + r180[4] * s0[5] + s0[4] * r180[0];
    t.elements[5] = r180[13] * s0[7] + r180[9] * s0[6] + r180[5] * s0[5] + r180[1] * s0[4];
    t.elements[6] = r180[14] * s0[7] + r180[10] * s0[6] + r180[6] * s0[5] + r180[2] * s0[4];
    t.elements[7] = r180[15] * s0[7] + r180[11] * s0[6] + r180[7] * s0[5] + r180[3] * s0[4];
    t.elements[8] = r180[12] * s0[11] + r180[8] * s0[10] + r180[4] * s0[9] + s0[8] * r180[0];
    t.elements[9] = r180[13] * s0[11] + r180[9] * s0[10] + r180[5] * s0[9] + r180[1] * s0[8];
    t.elements[10] = r180[14] * s0[11] + r180[10] * s0[10] + r180[6] * s0[9] + r180[2] * s0[8];
    t.elements[11] = r180[15] * s0[11] + r180[11] * s0[10] + r180[7] * s0[9] + r180[3] * s0[8];
    t.elements[12] = r180[12] * s0[15] + r180[8] * s0[14] + r180[4] * s0[13] + s0[12] * r180[0];
    t.elements[13] = r180[13] * s0[15] + r180[9] * s0[14] + r180[5] * s0[13] + r180[1] * s0[12];
    t.elements[14] = r180[14] * s0[15] + r180[10] * s0[14] + r180[6] * s0[13] + r180[2] * s0[12];
    t.elements[15] = r180[15] * s0[15] + r180[11] * s0[14] + r180[7] * s0[13] + r180[3] * s0[12];
    BodyMatrixCopy(&m, &t);
    // trace stage 25
    t.elements[0] = m.elements[2] * e[8] + m.elements[1] * e[4] + m.elements[0] * e[0] + m.elements[3] * e[12];
    t.elements[1] = m.elements[2] * e[9] + m.elements[1] * e[5] + m.elements[0] * e[1] + m.elements[3] * e[13];
    t.elements[2] = m.elements[3] * e[14] + m.elements[2] * e[10] + m.elements[1] * e[6] + m.elements[0] * e[2];
    t.elements[3] = m.elements[2] * e[11] + m.elements[1] * e[7] + m.elements[0] * e[3] + m.elements[3] * e[15];
    t.elements[4] = m.elements[6] * e[8] + m.elements[5] * e[4] + m.elements[4] * e[0] + m.elements[7] * e[12];
    t.elements[5] = m.elements[6] * e[9] + m.elements[5] * e[5] + m.elements[4] * e[1] + m.elements[7] * e[13];
    t.elements[6] = m.elements[6] * e[10] + m.elements[5] * e[6] + m.elements[4] * e[2] + m.elements[7] * e[14];
    t.elements[7] = m.elements[6] * e[11] + m.elements[5] * e[7] + m.elements[4] * e[3] + m.elements[7] * e[15];
    t.elements[8] = m.elements[10] * e[8] + m.elements[8] * e[0] + m.elements[11] * e[12] + m.elements[9] * e[4];
    t.elements[9] = m.elements[11] * e[13] + m.elements[9] * e[5] + m.elements[10] * e[9] + m.elements[8] * e[1];
    t.elements[10] = m.elements[10] * e[10] + m.elements[8] * e[2] + m.elements[11] * e[14] + m.elements[9] * e[6];
    t.elements[11] = m.elements[10] * e[11] + m.elements[8] * e[3] + m.elements[11] * e[15] + m.elements[9] * e[7];
    t.elements[12] = m.elements[14] * e[8] + m.elements[12] * e[0] + m.elements[15] * e[12] + m.elements[13] * e[4];
    t.elements[13] = m.elements[15] * e[13] + m.elements[13] * e[5] + m.elements[14] * e[9] + m.elements[12] * e[1];
    t.elements[14] = m.elements[14] * e[10] + m.elements[12] * e[2] + m.elements[15] * e[14] + m.elements[13] * e[6];
    t.elements[15] = m.elements[14] * e[11] + m.elements[12] * e[3] + m.elements[15] * e[15] + m.elements[13] * e[7];
    BodyMatrixCopy(&m, &t);
    // trace stage 26
    t.elements[0] = m.elements[1] * s3[4] + m.elements[2] * s3[8] + m.elements[3] * s3[12] + m.elements[0] * s3[0];
    t.elements[1] = m.elements[1] * s3[5] + m.elements[2] * s3[9] + m.elements[3] * s3[13] + m.elements[0] * s3[1];
    t.elements[2] = m.elements[1] * s3[6] + m.elements[2] * s3[10] + m.elements[3] * s3[14] + m.elements[0] * s3[2];
    t.elements[3] = m.elements[1] * s3[7] + m.elements[2] * s3[11] + m.elements[3] * s3[15] + m.elements[0] * s3[3];
    t.elements[4] = m.elements[5] * s3[4] + m.elements[4] * s3[0] + m.elements[6] * s3[8] + m.elements[7] * s3[12];
    t.elements[5] = m.elements[4] * s3[1] + m.elements[5] * s3[5] + m.elements[6] * s3[9] + m.elements[7] * s3[13];
    t.elements[6] = m.elements[4] * s3[2] + m.elements[5] * s3[6] + m.elements[6] * s3[10] + m.elements[7] * s3[14];
    t.elements[7] = m.elements[4] * s3[3] + m.elements[5] * s3[7] + m.elements[6] * s3[11] + m.elements[7] * s3[15];
    t.elements[8] = s3[12] * m.elements[11] + s3[8] * m.elements[10] + s3[4] * m.elements[9] + s3[0] * m.elements[8];
    t.elements[9] = s3[13] * m.elements[11] + s3[9] * m.elements[10] + s3[5] * m.elements[9] + s3[1] * m.elements[8];
    t.elements[10] = s3[14] * m.elements[11] + s3[10] * m.elements[10] + s3[6] * m.elements[9] + s3[2] * m.elements[8];
    t.elements[11] = s3[15] * m.elements[11] + s3[11] * m.elements[10] + s3[7] * m.elements[9] + s3[3] * m.elements[8];
    t.elements[12] = s3[12] * m.elements[15] + s3[8] * m.elements[14] + s3[4] * m.elements[13] + s3[0] * m.elements[12];
    t.elements[13] = s3[13] * m.elements[15] + s3[9] * m.elements[14] + s3[5] * m.elements[13] + s3[1] * m.elements[12];
    t.elements[14] = s3[14] * m.elements[15] + s3[10] * m.elements[14] + s3[6] * m.elements[13] + s3[2] * m.elements[12];
    t.elements[15] = s3[15] * m.elements[15] + s3[11] * m.elements[14] + s3[7] * m.elements[13] + s3[3] * m.elements[12];
    BodyMatrixCopy(&m, &t);
    // trace stage 27
    t.elements[0] = m.elements[3] * body[12] + m.elements[2] * body[8] + m.elements[1] * body[4] + m.elements[0] * body[0];
    t.elements[1] = m.elements[2] * body[9] + m.elements[1] * body[5] + m.elements[0] * body[1] + m.elements[3] * body[13];
    t.elements[2] = m.elements[2] * body[10] + m.elements[1] * body[6] + m.elements[0] * body[2] + m.elements[3] * body[14];
    t.elements[3] = m.elements[3] * body[15] + m.elements[2] * body[11] + m.elements[1] * body[7] + m.elements[0] * body[3];
    t.elements[4] = m.elements[6] * body[8] + m.elements[5] * body[4] + m.elements[4] * body[0] + m.elements[7] * body[12];
    t.elements[5] = m.elements[6] * body[9] + m.elements[5] * body[5] + m.elements[4] * body[1] + m.elements[7] * body[13];
    t.elements[6] = m.elements[6] * body[10] + m.elements[5] * body[6] + m.elements[4] * body[2] + m.elements[7] * body[14];
    t.elements[7] = m.elements[6] * body[11] + m.elements[5] * body[7] + m.elements[4] * body[3] + m.elements[7] * body[15];
    t.elements[8] = m.elements[9] * body[4] + m.elements[8] * body[0] + m.elements[11] * body[12] + m.elements[10] * body[8];
    t.elements[9] = m.elements[10] * body[9] + m.elements[8] * body[1] + m.elements[11] * body[13] + m.elements[9] * body[5];
    t.elements[10] = m.elements[11] * body[14] + m.elements[10] * body[10] + m.elements[8] * body[2] + m.elements[9] * body[6];
    t.elements[11] = m.elements[9] * body[7] + m.elements[10] * body[11] + m.elements[8] * body[3] + m.elements[11] * body[15];
    t.elements[12] = m.elements[13] * body[4] + m.elements[12] * body[0] + m.elements[15] * body[12] + m.elements[14] * body[8];
    t.elements[13] = m.elements[14] * body[9] + m.elements[12] * body[1] + m.elements[15] * body[13] + m.elements[13] * body[5];
    t.elements[14] = m.elements[15] * body[14] + m.elements[14] * body[10] + m.elements[12] * body[2] + m.elements[13] * body[6];
    t.elements[15] = m.elements[13] * body[7] + m.elements[14] * body[11] + m.elements[12] * body[3] + m.elements[15] * body[15];
    BodyMatrixCopy(world, &t);
}

// Spun copy of a frame for the wheels with a spin term (0x6278c0..0x627975 for block 1; the same shape for
// blocks 2 to 4): copy the frame, clear its translation (elements 12..14, element 15 = 1), rotate about the
// axle by the signed angle, then put the translation back. The rotation acts on the frame without moving it.
static const float *SpunFrame(WheelMatrix *copy, const WheelMatrix *source, int angle, int spin) {
    if (static_cast<unsigned short>(spin) == 0) {
        return source->elements;
    }
    BodyMatrixCopy(copy, const_cast<WheelMatrix *>(source));
    const float tx = copy->elements[12];
    const float ty = copy->elements[13];
    const float tz = copy->elements[14];
    copy->elements[12] = kZeroFloat;
    copy->elements[13] = kZeroFloat;
    copy->elements[14] = kZeroFloat;
    copy->elements[15] = kOneFloat;
    RotateMatrixX(copy, copy, static_cast<unsigned int>(angle));
    copy->elements[12] = tx;
    copy->elements[13] = ty;
    copy->elements[14] = tz;
    return copy->elements;
}

// Wheel block 0x62700e..0x62df66 of the car draw 0x620ae0 (see the header for inputs and seams).
void CarWheelDraw(CarWheelDrawInputs *in) {
    CarRenderWheelInfo *ri = in->ri;
    const unsigned char *car = ri->carObject;
    // 0x6274ce: the spinner is present when ri+0x1b58 (solid1B48[1][0]) is set.
    const bool spinner = ri->solid1B48[1][0] != 0;
    // Spin term (0x62708a..0x627102): views 0 and 1 only. Angle is 16-bit: 65536 units per turn.
    int spin = 0;
    double frontTerm = 0.0;                                   // A (0x62712c): S3 * S1 for views 0 and 1
    if (in->lod6C <= 1) {
        const double term = (kSpinS1 != kZeroFloat) ? static_cast<double>(kSpinS1) : SpinTerm(car + 0x10);
        const double product = static_cast<double>(kSpinS2) * term * static_cast<double>(kSpinScale);
        spin = static_cast<int>(product) / 360;               // _ftol2 then division by 360
        frontTerm = static_cast<double>(kSpinS3) * static_cast<double>(kSpinS1);
    }

    // Widths (0x62700e..0x627070): the 0x7a668c / 0x7a6690 override when nonzero, else the MARKER_BRAKE values.
    const float wf = (ug2WheelWidthOverrideFront != kZeroFloat) ? ug2WheelWidthOverrideFront : ri->widthFront430;
    const float wr = (ug2WheelWidthOverrideRear != kZeroFloat) ? ug2WheelWidthOverrideRear : ri->widthRear434;
    // B = C250 / 0.184 (kept at 53-bit precision in the original).
    const double beta = static_cast<double>(ug2WheelOneRuntime) / static_cast<double>(ug2WheelBetaRuntime);

    // Per-wheel sets (0x62731..0x627497): set0 (scale and translation), set1, set2 and set3 frames, and
    // element 13 of the tyre, brake and third frames is zeroed (0x627470..0x627481).
    WheelMatrix set0[4], set1[4], set2[4], set3[4];
    const int k = in->slotD8;
    for (int w = 0; w < 4; ++w) {
        const float c = ri->pivot400[2 * w + k];             // C (ri+0x400 + 8w + 4k)
        const float d = ri->offset438[2 * w + k];            // D (ri+0x438 + 8w + 4k)
        const float s = ri->scale420[w];
        const double e = static_cast<double>(s) * static_cast<double>(kHalfFloat);   // E = s * 0.5
        const float sPrime = (s == kZeroFloat) ? kBetaFloat : s;                    // s' = 0.184 when s == 0
        const double f = beta * static_cast<double>(sPrime);                       // F = B * s'

        IdentityMatrix(&set0[w]);
        set0[w].elements[5] = static_cast<float>(f);
        set0[w].elements[13] = static_cast<float>(-e);

        const double g = -((c < kZeroFloat) ? (static_cast<double>(c) + e) : (static_cast<double>(c) - e));   // G
        IdentityMatrix(&set1[w]);
        set1[w].elements[13] = static_cast<float>(g);

        const double y = (c < kZeroFloat) ? (g + static_cast<double>(d)) : (g - static_cast<double>(d));     // Y
        IdentityMatrix(&set3[w]);
        set3[w].elements[13] = static_cast<float>(y);

        // T = (W != 0) ? F*W : E. The product is stored to a float cell and reloaded (0x6273e9, 0x6273ff).
        const float width = (w < 2) ? wf : wr;
        double t = e;
        if (width != kZeroFloat) {
            // The original stores F*W to a float cell and reloads it (0x6273e9, 0x6273ff): a volatile store keeps
            // that rounding, which a plain cast would not.
            volatile float stored = static_cast<float>(f * static_cast<double>(width));
            t = static_cast<double>(stored);
        }
        const double x = -((c < kZeroFloat) ? (static_cast<double>(c) + t) : (static_cast<double>(c) - t));  // X
        IdentityMatrix(&set2[w]);
        set2[w].elements[13] = static_cast<float>(x);

        if (w < 2) {
            // Front wheels: element 14 of set1..set3 becomes -A (0x627432..0x627462).
            set1[w].elements[14] = static_cast<float>(static_cast<double>(set1[w].elements[14]) - frontTerm);
            set2[w].elements[14] = static_cast<float>(static_cast<double>(set2[w].elements[14]) - frontTerm);
            set3[w].elements[14] = static_cast<float>(static_cast<double>(set3[w].elements[14]) - frontTerm);
        }
        in->tyre[w].elements[13] = kZeroFloat;
        in->brake[w].elements[13] = kZeroFloat;
        in->third[w].elements[13] = kZeroFloat;
    }

    // Verification output of the four sets (the frame cells of the original at 0x200, 0x3d0, 0x4d0 and 0x5d0).
    if (in->setDump) {
        for (int w = 0; w < 4; ++w) {
            in->setDump[w] = set0[w];
            in->setDump[4 + w] = set1[w];
            in->setDump[8 + w] = set2[w];
            in->setDump[12 + w] = set3[w];
        }
    }

    // 0x6274c5: the car gate byte (+0x855, sign-extended) must be set.
    if (static_cast<signed char>(car[0x855]) == 0) {
        return;
    }
    // 0x62750a: views above 2 draw no wheels.
    if (in->lod6C > 2) {
        return;
    }
    // 0x627515..0x62758c: four allocations; a NULL result leaves the block.
    WheelMatrix *a1 = static_cast<WheelMatrix *>(UG2WheelAllocMatrices(4));    // frame+0xc4: tyre worlds
    WheelMatrix *a2 = static_cast<WheelMatrix *>(UG2WheelAllocMatrices(4));    // frame+0xb8: brake worlds
    unsigned char *a3 = static_cast<unsigned char *>(UG2WheelAllocRecords(0x700));   // frame+0xe8: tyre extras
    unsigned char *a4 = static_cast<unsigned char *>(UG2WheelAllocRecords(0x700));   // frame+0xbc: brake extras
    if (!a1 || !a3 || !a2 || !a4) {
        return;
    }
    // 0x627592..0x6275f0: wheel presence (frame+0x68, +0x14c, +0x64, +0xfc).
    const int f68 = WheelPresence(ri, ri->wheelPresent2D78, 0);
    const int f14c = WheelPresence(ri, ri->wheelPresent2D98, 1);
    const int f64 = WheelPresence(ri, ri->wheelPresent2DB8, 2);
    const int fAc = WheelPresence(ri, ri->wheelPresent2DD8, 3);
    // 0x627640: the view clamp (frame+0xf4) is limited to 2.
    int L = in->lodF4;
    if (L > 2) {
        L = 2;
    }
    // 0x627723: R180 = RotZ(identity, 0x8000) (applied to wheels 0 and 3 only).
    WheelMatrix identity, r180;
    IdentityMatrix(&identity);
    RotateMatrixZ(&r180, &identity, 0x8000u);

    // Solid links (0x627774..0x627897): solid1[j] = ri+0x1b48+16j+4L, solid2[j] = ri+0x1b68+16j+4L. A link value
    // V (ri+0x390 for j = 0, ri+0x3ac for j = 1) is stored under three keys on solid1[j] when the solid and V are
    // set (first guard on solid1, second on solid2). The third key's operand reads frame+0x194+4j after a push,
    // which is frame+0x190+4j, so every call here uses solid1[j].
    CarRenderSolid *solid1[2];
    CarRenderSolid *solid2[2];
    for (int j = 0; j < 2; ++j) {
        solid1[j] = ri->solid1B48[j][L];
        solid2[j] = ri->solid1B68[j][L];
        const unsigned int v = (j == 0) ? ri->spinStart390 : ri->spinStart3AC;
        if (solid1[j] && v) {
            solid1[j]->SetSlot(0x22719fa9u, v);
            solid1[j]->SetSlot(0xfd102a92u, v);
            solid1[j]->SetSlot(0x12049251u, v);
        }
        if (solid2[j] && v) {
            solid1[j]->SetSlot(0x22719fa9u, v);
            solid1[j]->SetSlot(0xfd102a92u, v);
            solid1[j]->SetSlot(0x12049251u, v);
        }
    }

    // Preparation inputs are produced before this block in 0x620ae0. Their
    // buffers remain live across the intervening body draws; they are not scratch matrices.
    const unsigned int flagsTyre = in->flagsCC | in->flags1AC | in->flagsArg8;   // 0x6286ea..0x628706
    const unsigned int flagsBrake = in->flags1AC | in->flagsArg8;                // 0x629571..0x629584
    const float *body = in->body->elements;
    const float *rot180 = r180.elements;
    WheelMatrix rot;
    const void *visibleGlobal = &ug2WheelReflectionBasis871B50;

    // Wheel 0 (0x6278aa..0x62959b): tyre (block 1) and brake (block 2), when present and the spinner exists.
    if (f68) {
        const float *e = SpunFrame(&rot, &in->tyre[0], spin, spin);
        WheelChainBlock1(a1, set0[0].elements, e, set1[0].elements, body, rot180);
        UG2WheelVisible(a3, a1, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
        if (solid1[0]) {
            in->view->DrawSolid(solid1[0], a1, a3, flagsTyre, 0);
        }
        if (spinner) {
            e = SpunFrame(&rot, &in->third[0], spin, spin);
            WheelChainBlock2(a2, set0[0].elements, e, set3[0].elements, body, rot180);
            UG2WheelVisible(a4, a2, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
            if (solid1[1]) {
                in->view->DrawSolid(solid1[1], a2, a4, flagsBrake, 0);
            }
        }
    }

    // Wheel 1 (0x6295a0..0x62ab69): tyre (block 3) and brake (block 4).
    if (f14c) {
        const float *e = SpunFrame(&rot, &in->tyre[1], -spin, spin);
        WheelChainBlock3(a1 + 1, set0[1].elements, e, set1[1].elements, body, rot180);
        UG2WheelVisible(a3 + 0x1c0, a1 + 1, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
        if (solid1[0]) {
            in->view->DrawSolid(solid1[0], a1 + 1, a3 + 0x1c0, flagsTyre, 0);
        }
        if (spinner) {
            e = SpunFrame(&rot, &in->third[1], -spin, spin);
            WheelChainBlock4(a2 + 1, set0[1].elements, e, set3[1].elements, body, rot180);
            UG2WheelVisible(a4 + 0x1c0, a2 + 1, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
            if (solid1[1]) {
                in->view->DrawSolid(solid1[1], a2 + 1, a4 + 0x1c0, flagsBrake, 0);
            }
        }
    }

    // Wheel 2 (0x62ab69..0x62c16e): rear tyre (block 5) and rear brake (block 6). Block 5 substitutes
    // ri+0x1b6c when the LOD special path applies (L == 0, gate 6, helper result below the threshold).
    if (f64) {
        const float *e = in->tyre[2].elements;
        WheelChainBlock5(a1 + 2, set0[2].elements, e, set1[2].elements, body, rot180);
        UG2WheelVisible(a3 + 0x380, a1 + 2, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
        CarRenderSolid *solid5 = solid2[0];
        if (L == 0 && ug2WheelLodGate == 6 &&
            UG2WheelFn60d700(a1 + 2, in->view) < ug2WheelLodThreshold) {
            solid5 = ri->solid1B68[0][1];
        }
        if (solid5) {
            in->view->DrawSolid(solid5, a1 + 2, a3 + 0x380, flagsTyre, 0);
        }
        if (spinner) {
            e = in->third[2].elements;
            WheelChainBlock6(a2 + 2, set0[2].elements, e, set3[2].elements, body, rot180);
            UG2WheelVisible(a4 + 0x380, a2 + 2, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
            if (solid2[1]) {
                in->view->DrawSolid(solid2[1], a2 + 2, a4 + 0x380, flagsBrake, 0);
            }
        }
    }

    // Wheel 3 (0x62c16e..0x62deaf): rear tyre (block 7) and rear brake (block 8). Block 7 takes the same LOD
    // substitution as block 5 (ri+0x1b6c) when the helper result is below the threshold.
    if (fAc) {
        const float *e = in->tyre[3].elements;
        WheelChainBlock7(a1 + 3, set0[3].elements, e, set1[3].elements, body, rot180);
        UG2WheelVisible(a3 + 0x540, a1 + 3, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
        CarRenderSolid *solid7 = solid2[0];
        if (L == 0 && ug2WheelLodGate == 6 &&
            UG2WheelFn60d700(a1 + 3, in->view) < ug2WheelLodThreshold) {
            solid7 = ri->solid1B68[0][1];
        }
        if (solid7) {
            in->view->DrawSolid(solid7, a1 + 3, a3 + 0x540, flagsTyre, 0);
        }
        if (spinner) {
            e = in->third[3].elements;
            WheelChainBlock8(a2 + 3, set0[3].elements, e, set3[3].elements, body, rot180);
            UG2WheelVisible(a4 + 0x540, a2 + 3, visibleGlobal, const_cast<float *>(in->renderPosition), in->view, in->sourceRecord);
            if (solid2[1]) {
                in->view->DrawSolid(solid2[1], a2 + 3, a4 + 0x540, flagsBrake, 0);
            }
        }
    }

    // Tail (0x62deb2..0x62df61): with LOD gate 3, each tyre world (k = 0..3) is passed to the helper, and the
    // secondary draw runs when the helper result is below the threshold.
    if (ug2WheelLodGate == 3) {
        for (int kk = 0; kk < 4; ++kk) {
            WheelMatrix *world = a1 + kk;
            if (UG2WheelFn60d700(world, in->view) < ug2WheelLodThreshold) {
                ri->Fn620600(in->view, world);
            }
        }
    }
}
