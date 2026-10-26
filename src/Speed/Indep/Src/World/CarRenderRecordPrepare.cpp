#include "CarRenderRecord.h"

extern "C" int ug2WheelLodGate; // 0x8654a4
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

// Original 0x48e6c0. The source may alias the destination: copy first, transpose
// the world frame second, then snapshot the source basis before writing it.
void CarRenderPrepareWorldRecord(CarRenderRecord *output, CarRenderRecord *source, WheelMatrix *world) {
    CarRenderRecord coefficients;
    unsigned int *words = reinterpret_cast<unsigned int *>(&coefficients);
    for (int i = 0; i < 40; ++i) {
        words[i] = 0;
    }
    output->CopyFrom(source);
    WheelMatrix direction;
    CarRenderDirectionMatrix(&direction, world);

    const float *m = direction.elements;
    volatile float tx = static_cast<long double>(m[12]) * 0.0f;
    volatile float ty = static_cast<long double>(m[13]) * 0.0f;
    volatile float tz = static_cast<long double>(m[14]) * 0.0f;
    const float translations[3] = {tx, ty, tz};
    const float *input = source->matrix40.elements;
    const float s0 = input[0], s4 = input[4], s8 = input[8];
    const float s1 = input[1], s5 = input[5], s9 = input[9];
    const float s2 = input[2], s6 = input[6], s10 = input[10];
    const float first[3] = {s0, s1, s2};
    const float second[3] = {s4, s5, s6};
    const float third[3] = {s8, s9, s10};
    // 0x48e710..0x48e970: Z product, Y product, X product, translation.
    // Destination groups contain one transformed component from each source column.
    for (int component = 0; component < 3; ++component) {
        for (int column = 0; column < 3; ++column) {
            output->matrix40.elements[component * 4 + column] =
                ((static_cast<long double>(m[component + 8]) * third[column] +
                  static_cast<long double>(m[component + 4]) * second[column]) +
                 static_cast<long double>(m[component]) * first[column]) + translations[component];
        }
    }
    // Copy coefficients after the basis stores (important for partially overlapping records).
    for (int i = 0; i < 40; ++i) {
        words[i] = source->value120[i];
    }
    coefficients.TransformCoefficients(&direction);
    for (int i = 0; i < 40; ++i) {
        output->value120[i] = words[i];
    }
}

// Original 0x48a6a0. Relative X stays extended; relative Y/Z are binary32.
// Clamp selection compares the unrounded length with the lower bound, then
// compares its stored float with the upper bound. No other record fields change.
unsigned int CarRenderPrepareViewRecord(CarRenderRecord *output, WheelMatrix *world,
                                      const WheelMatrix *matrixArg3, const float *position) {
    WheelMatrix direction;
    CarRenderDirectionMatrix(&direction, world);
    const float *m = direction.elements;
    if (position) {
        const long double dx = static_cast<long double>(position[0]) - world->elements[12];
        volatile float dy = static_cast<long double>(position[1]) - world->elements[13];
        volatile float dz = static_cast<long double>(position[2]) - world->elements[14];
        float point[4];
        point[0] = ((static_cast<long double>(m[8]) * dz + static_cast<long double>(m[4]) * dy) + m[0] * dx) + m[12];
        point[1] = ((static_cast<long double>(m[9]) * dz + static_cast<long double>(m[5]) * dy) + m[1] * dx) + m[13];
        point[2] = ((static_cast<long double>(m[10]) * dz + static_cast<long double>(m[6]) * dy) + m[2] * dx) + m[14];
        // 0x48a775: the Z cell is overwritten before it is read for the W component.
        point[3] = ((static_cast<long double>(m[11]) * point[2] + static_cast<long double>(m[7]) * dy) + m[3] * dx) + m[15];
        const float minimum = 2.78f; // 0x78d788 / 0x78d790
        const float maximum = ug2WheelLodGate == 3 ? 8.0f : 2.78f; // 0x78d794 / 0x78d78c
        const long double sum = ((static_cast<long double>(point[3]) * point[3] +
                                  static_cast<long double>(point[2]) * point[2]) +
                                 static_cast<long double>(point[1]) * point[1]) +
                                static_cast<long double>(point[0]) * point[0];
#ifdef __GNUC__
        const long double length = __builtin_sqrtl(sum);
#else
        const long double length = sqrt(sum);
#endif
        volatile float magnitude = length;
        if (length < minimum) {
            magnitude = minimum;
        } else if (magnitude > maximum) {
            magnitude = maximum;
        }
        point[3] = 0.0f;
        CarRenderNormalize4(point, point);
        volatile float z = static_cast<long double>(magnitude) * point[2];
        output->valueC0[3] = 1.0f;
        output->valueC0[2] = z;
        output->valueC0[0] = static_cast<long double>(magnitude) * point[0];
        output->valueC0[1] = static_cast<long double>(magnitude) * point[1];
    }
    if (matrixArg3 && position) {
        // Component-specific product order at 0x48a8a7..0x48aac7 (PC v1.2).
        WheelMatrix product;
        product.elements[0] =
            ((static_cast<long double>(world->elements[0]) * matrixArg3->elements[0] +
              static_cast<long double>(world->elements[3]) * matrixArg3->elements[12]) +
             static_cast<long double>(world->elements[2]) * matrixArg3->elements[8]) +
            static_cast<long double>(world->elements[1]) * matrixArg3->elements[4];
        product.elements[1] =
            ((static_cast<long double>(world->elements[0]) * matrixArg3->elements[1] +
              static_cast<long double>(world->elements[1]) * matrixArg3->elements[5]) +
             static_cast<long double>(world->elements[2]) * matrixArg3->elements[9]) +
            static_cast<long double>(world->elements[3]) * matrixArg3->elements[13];
        product.elements[2] =
            ((static_cast<long double>(world->elements[3]) * matrixArg3->elements[14] +
              static_cast<long double>(world->elements[1]) * matrixArg3->elements[6]) +
             static_cast<long double>(world->elements[0]) * matrixArg3->elements[2]) +
            static_cast<long double>(world->elements[2]) * matrixArg3->elements[10];
        product.elements[3] =
            ((static_cast<long double>(world->elements[2]) * matrixArg3->elements[11] +
              static_cast<long double>(world->elements[3]) * matrixArg3->elements[15]) +
             static_cast<long double>(world->elements[0]) * matrixArg3->elements[3]) +
            static_cast<long double>(world->elements[1]) * matrixArg3->elements[7];
        product.elements[4] =
            ((static_cast<long double>(world->elements[6]) * matrixArg3->elements[8] +
              static_cast<long double>(world->elements[7]) * matrixArg3->elements[12]) +
             static_cast<long double>(world->elements[5]) * matrixArg3->elements[4]) +
            static_cast<long double>(world->elements[4]) * matrixArg3->elements[0];
        product.elements[5] =
            ((static_cast<long double>(world->elements[5]) * matrixArg3->elements[5] +
              static_cast<long double>(world->elements[7]) * matrixArg3->elements[13]) +
             static_cast<long double>(world->elements[6]) * matrixArg3->elements[9]) +
            static_cast<long double>(world->elements[4]) * matrixArg3->elements[1];
        product.elements[6] =
            ((static_cast<long double>(world->elements[6]) * matrixArg3->elements[10] +
              static_cast<long double>(world->elements[5]) * matrixArg3->elements[6]) +
             static_cast<long double>(world->elements[4]) * matrixArg3->elements[2]) +
            static_cast<long double>(world->elements[7]) * matrixArg3->elements[14];
        product.elements[7] =
            ((static_cast<long double>(world->elements[6]) * matrixArg3->elements[11] +
              static_cast<long double>(world->elements[4]) * matrixArg3->elements[3]) +
             static_cast<long double>(world->elements[5]) * matrixArg3->elements[7]) +
            static_cast<long double>(world->elements[7]) * matrixArg3->elements[15];
        product.elements[8] =
            ((static_cast<long double>(world->elements[10]) * matrixArg3->elements[8] +
              static_cast<long double>(world->elements[11]) * matrixArg3->elements[12]) +
             static_cast<long double>(world->elements[9]) * matrixArg3->elements[4]) +
            static_cast<long double>(world->elements[8]) * matrixArg3->elements[0];
        product.elements[9] =
            ((static_cast<long double>(world->elements[9]) * matrixArg3->elements[5] +
              static_cast<long double>(world->elements[11]) * matrixArg3->elements[13]) +
             static_cast<long double>(world->elements[10]) * matrixArg3->elements[9]) +
            static_cast<long double>(world->elements[8]) * matrixArg3->elements[1];
        product.elements[10] =
            ((static_cast<long double>(world->elements[10]) * matrixArg3->elements[10] +
              static_cast<long double>(world->elements[9]) * matrixArg3->elements[6]) +
             static_cast<long double>(world->elements[8]) * matrixArg3->elements[2]) +
            static_cast<long double>(world->elements[11]) * matrixArg3->elements[14];
        product.elements[11] =
            ((static_cast<long double>(world->elements[10]) * matrixArg3->elements[11] +
              static_cast<long double>(world->elements[8]) * matrixArg3->elements[3]) +
             static_cast<long double>(world->elements[9]) * matrixArg3->elements[7]) +
            static_cast<long double>(world->elements[11]) * matrixArg3->elements[15];
        product.elements[12] =
            ((static_cast<long double>(world->elements[12]) * matrixArg3->elements[0] +
              static_cast<long double>(world->elements[15]) * matrixArg3->elements[12]) +
             static_cast<long double>(world->elements[14]) * matrixArg3->elements[8]) +
            static_cast<long double>(world->elements[13]) * matrixArg3->elements[4];
        product.elements[13] =
            ((static_cast<long double>(world->elements[13]) * matrixArg3->elements[5] +
              static_cast<long double>(world->elements[12]) * matrixArg3->elements[1]) +
             static_cast<long double>(world->elements[15]) * matrixArg3->elements[13]) +
            static_cast<long double>(world->elements[14]) * matrixArg3->elements[9];
        product.elements[14] =
            ((static_cast<long double>(world->elements[14]) * matrixArg3->elements[10] +
              static_cast<long double>(world->elements[13]) * matrixArg3->elements[6]) +
             static_cast<long double>(world->elements[15]) * matrixArg3->elements[14]) +
            static_cast<long double>(world->elements[12]) * matrixArg3->elements[2];
        product.elements[15] =
            ((static_cast<long double>(world->elements[14]) * matrixArg3->elements[11] +
              static_cast<long double>(world->elements[13]) * matrixArg3->elements[7]) +
             static_cast<long double>(world->elements[12]) * matrixArg3->elements[3]) +
            static_cast<long double>(world->elements[15]) * matrixArg3->elements[15];
        BodyMatrixCopy(&output->matrixE0, &product);
        output->matrixE0.elements[3] = 0.0f;
        output->matrixE0.elements[7] = 0.0f;
        output->matrixE0.elements[11] = 0.0f;
        // These are bit copies in the original, after the matrix copy (alias-sensitive).
        unsigned int *dst = reinterpret_cast<unsigned int *>(output->matrixE0.elements);
        const unsigned int *src = reinterpret_cast<const unsigned int *>(output->valueC0);
        dst[12] = src[0];
        dst[13] = src[1];
        dst[14] = src[2];
        dst[15] = 0;
    }
    return 1;
}

// Original 0x48f660. Argument 5 is unused; both producer calls run unconditionally.
unsigned int CarRenderPrepareRecord(CarRenderRecord *output, WheelMatrix *world,
                                  const WheelMatrix *matrixArg3, const float *position,
                                  void *view, CarRenderRecord *source) {
    (void)view;
    CarRenderPrepareWorldRecord(output, source, world);
    CarRenderPrepareViewRecord(output, world, matrixArg3, position);
    return 1;
}
