// Pixel size, body LOD and wheel flags of the car draw 0x620ae0 (PC v1.2, speed2.exe): CarViewPixelRadius (0x48acb0),
// CarLodPixelSize (0x620c6b..0x620cc0) and CarLodSelect (0x62107e..0x621230). Addresses are original. Arithmetic: the
// original keeps intermediates in x87 registers and stores floats only where it writes memory; intermediates are double
// here and the x87 precision control of the caller decides their rounding, as in CarRenderWheels.cpp.
#include "CarRenderLod.h"

#ifdef _MSC_VER
// The verifier builds with MSVC 7.1 without the CRT include path. The intrinsic keeps the square root inline (FSQRT), as
// the original does at 0x48ad05.
extern "C" double sqrt(double);
#pragma intrinsic(sqrt)
#else
#include <math.h>
#endif

// 0x48acb0 (__thiscall on the view, RET 8). Operation order follows the original:
//   d = position - frame origin (x, y, z); depth = (dz*A58 + dy*A54) + dx*A50; in front when -radius <= depth;
//   distance = sqrt((dz*dz + dy*dy) + dx*dx); near when distance - radius <= 0.5*radius (result 2*focal);
//   otherwise result = (focal / (distance - radius)) * radius, truncated to int (__ftol2).
int CarViewPixelRadius(CarRenderView *view, const float *position, float radius) {
    const CarRenderFrame *frame = view->frame40;
    const double dx = static_cast<double>(position[0]) - static_cast<double>(frame->position40[0]);
    const double dy = static_cast<double>(position[1]) - static_cast<double>(frame->position40[1]);
    const double dz = static_cast<double>(position[2]) - static_cast<double>(frame->position40[2]);
    const double depth = (dz * static_cast<double>(frame->axis50[2]) + dy * static_cast<double>(frame->axis50[1])) +
                         dx * static_cast<double>(frame->axis50[0]);
    // 0x48acdb..0x48ace8: JNZ is taken when -radius <= depth; otherwise the result is 0.
    const double negRadius = -static_cast<double>(radius);
    if (negRadius > depth) {
        return 0;
    }
    const double distance = sqrt((dz * dz + dy * dy) + dx * dx);
    const double remainder = distance - static_cast<double>(radius);          // 0x48ad0d
    const double focal = static_cast<double>(view->focal0C);
    // 0x48ad1b..0x48ad24: near when remainder <= 0.5 * radius (JNZ taken on equal or below).
    if (remainder <= 0.5 * static_cast<double>(radius)) {
        return static_cast<int>(2.0 * focal);                                 // 0x48ad37..0x48ad3c
    }
    return static_cast<int>((focal / remainder) * static_cast<double>(radius)); // 0x48ad26..0x48ad2d
}

// 0x620c6b..0x620cc0 (see the header). The original calls 0x5b98a0 before the comparison, so the call is made here too.
int CarLodPixelSize(CarRenderView *view, const float *position, int *lodArg, int *clampArg) {
    const int raw = CarViewPixelRadius(view, position, 3.0f);                 // 0x620c79
    if (UG2CarViewModeGlobal() == 3 && ug2WheelLodGate != 3) {               // 0x620c84..0x620c95
        const int scaled = static_cast<int>(static_cast<double>(raw) * static_cast<double>(ug2CarLodScale7A6700));
        *lodArg = 1;                                                           // 0x620cb1
        *clampArg = 1;                                                         // 0x620cb4
        return scaled;                                                         // 0x620ca1..0x620cad
    }
    return raw;
}

// 0x62107e..0x621230. Frame cells are named by the header comments. The original order is kept: the owner words first
// (0x62107e..0x6210b0), the car search (0x6210bb..0x62110b), the wheel flags (0x62110b..0x62111c, 0x621123..0x621142), the
// body LOD and clamp (0x62114d..0x62118b), the forced zero (0x62118f..0x621198), the clamps (0x6211a7..0x6211e8), the
// overrides (0x6211ec..0x621214) and the arg8 OR (0x621224..0x621229).
void CarLodSelect(CarLodInputs *in) {
    CarRenderWheelInfo *ri = in->ri;
    const unsigned char *owner = static_cast<const unsigned char *>(ri->owner000);   // frame+0x18c = [ri+0]

    // 0x62107e..0x6210b0: owner+0x958 (frame+0x180) and whether it is non-zero (frame+0x14c).
    unsigned int owner958 = 0;
    unsigned int ownerHas958 = 0;
    if (owner) {
        owner958 = *reinterpret_cast<const unsigned int *>(owner + 0x958);
        ownerHas958 = owner958 != 0 ? 1u : 0u;
    }
    in->owner958 = owner958;
    in->ownerHas958 = ownerHas958;

    // 0x6210bb..0x62110b: the first car record whose key dword (row+0x840, row = ri+8) and value dword (sign-extended
    // byte car+7, car = ri+4) both match. The index is the count when none matches (and 0 when the count is not positive).
    const unsigned int key = *reinterpret_cast<const unsigned int *>(ri->carTypeRow + 0x840);
    const int carByte = static_cast<signed char>(ri->carObject[7]);
    const int count = ug2CarLodRecordCount8A1CD8;
    int searchIndex = 0;
    if (count > 0) {
        const unsigned char *record = static_cast<const unsigned char *>(ug2CarLodRecords8A1CD4);
        for (searchIndex = 0; searchIndex < count; ++searchIndex, record += 0x40) {
            if (*reinterpret_cast<const unsigned int *>(record) == key &&
                *reinterpret_cast<const int *>(record + 4) == carByte) {
                break;
            }
        }
    }
    in->searchIndex = searchIndex;

    // 0x62110b..0x621142: frame+0xe8 and frame+0x1ac are 1 when the flag is zero or pixel < 18.0 (strict), else 0.
    unsigned int wheelFlag = 0;
    if (ug2CarLodFlag7FEE4C == 0 ||
        static_cast<double>(in->pixel) < static_cast<double>(ug2CarLodPixelLimit7A66FC)) {
        wheelFlag = 1;
    }
    in->flagE8 = wheelFlag;
    in->flag1AC = wheelFlag;

    // 0x62114d..0x62118b: body LOD and clamp start at the inputs. While pixel < table[lod] (exact pixel for the first test,
    // the float pixel for the rest), lod and clamp are incremented. The float pixel is stored at 0x621157.
    int lod = in->lodIn;
    int clamp = in->clampIn;
    in->pixelFloat = static_cast<float>(in->pixel);
    if (static_cast<double>(in->pixel) < static_cast<double>(ug2CarLodTable7A66DC[lod])) {
        do {
            ++lod;
            ++clamp;
        } while (in->pixelFloat < ug2CarLodTable7A66DC[lod]);
    }

    // 0x62118f..0x6211e8: a non-zero 0x499db0 result forces lod and clamp to 0. Otherwise lod is limited to 3, and a
    // lod of 3 becomes 2 when the owner has no +0x424 word and ri+0x2eb8 is non-zero.
    if (UG2CarLodZeroFlag() != 0) {
        lod = 0;
        clamp = 0;
    } else {
        if (lod > 3) {
            lod = 3;                                                           // 0x6211b0
        }
        if (lod > 2 && owner && *reinterpret_cast<const unsigned int *>(owner + 0x424) == 0 && ri->flag2EB8 != 0) {
            lod = 2;                                                           // 0x6211e3
        }
    }

    // 0x6211ec..0x62120f: overrides from 0x802728 (lod) and 0x80272c (clamp) when they are not -1.
    if (ug2CarLodOverride802728 != -1) {
        lod = ug2CarLodOverride802728;
    }
    if (ug2CarLodOverride80272C != -1) {
        clamp = ug2CarLodOverride80272C;
    }
    in->lod = lod;
    in->clamp = clamp;

    // 0x62120f..0x62121c: both -1 leaves the draw. Otherwise arg8 gets 0x800 when lod is 1 or more (0x621224..0x621229).
    in->arg8Out = in->arg8;
    if (lod == -1 && clamp == -1) {
        in->exitDraw = 1;
        return;
    }
    in->exitDraw = 0;
    if (lod >= 1) {
        in->arg8Out |= 0x800u;
    }
}
