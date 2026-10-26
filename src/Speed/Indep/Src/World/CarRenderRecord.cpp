#include "CarRenderRecord.h"

extern "C" WheelMatrix *__stdcall D3DXMatrixTranspose(WheelMatrix *, WheelMatrix *);
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

// Original 0x5bb190. D3DX is a platform library boundary, shared with camera finalisation.
WheelMatrix *CarRenderDirectionMatrix(WheelMatrix *output, WheelMatrix *input) {
    D3DXMatrixTranspose(output, input);
    output->elements[3] = 0.0f;
    output->elements[7] = 0.0f;
    output->elements[11] = 0.0f;
    output->elements[15] = 0.0f;
    output->elements[12] = 0.0f;
    output->elements[13] = 0.0f;
    output->elements[14] = 0.0f;
    return output;
}

// Original 0x43d010. The length and reciprocal stay in x87 precision, without a
// binary32 spill. Snapshot Y/Z before writing X to retain the observed alias behaviour.
void CarRenderNormalize4(float *output, const float *input) {
    const long double x = input[0], y = input[1], z = input[2], w = input[3];
    const long double sum = ((x * x + y * y) + z * z) + w * w;
#ifdef __GNUC__
    const long double length = __builtin_sqrtl(sum);
#else
    const long double length = sqrt(sum);
#endif
    if (length != 0.0) {
        const long double inverse = 1.0 / length;
        const float savedZ = input[2];
        const float savedY = input[1];
        output[0] = inverse * input[0];
        output[1] = savedY * inverse;
        output[2] = savedZ * inverse;
    } else {
        output[0] = 1.0f;
        output[1] = 0.0f;
        output[2] = 0.0f;
    }
}

// Original 0x48a3b0. The four matrix copies run separately and in order; this is
// not a whole-record snapshot or memmove. Each remaining 16-byte group snapshots
// its inputs before stores. Original lanes 1/2 pass through x87 loads/stores.
CarRenderRecord *CarRenderRecord::CopyFrom(CarRenderRecord *source) {
    BodyMatrixCopy(&matrix00, &source->matrix00);
    BodyMatrixCopy(&matrix40, &source->matrix40);
    BodyMatrixCopy(&matrix80, &source->matrix80);
    unsigned int *dst = reinterpret_cast<unsigned int *>(valueC0);
    unsigned int *src = reinterpret_cast<unsigned int *>(source->valueC0);
    unsigned int a = src[0], d = src[3];
    volatile long double b = source->valueC0[1], c = source->valueC0[2];
    valueC0[1] = b;
    dst[0] = a;
    dst[3] = d;
    valueC0[2] = c;
    for (int i = 0; i < 4; ++i) {
        valueD0[i] = source->valueD0[i];
    }
    BodyMatrixCopy(&matrixE0, &source->matrixE0);
    for (int group = 0; group < 10; ++group) {
        dst = value120 + group * 4;
        src = source->value120 + group * 4;
        a = src[0];
        d = src[3];
        b = reinterpret_cast<float *>(src)[1];
        c = reinterpret_cast<float *>(src)[2];
        reinterpret_cast<float *>(dst)[1] = b;
        dst[0] = a;
        reinterpret_cast<float *>(dst)[2] = c;
        dst[3] = d;
    }
    return this;
}
