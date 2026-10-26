#include "CarRenderRecord.h"

void WheelSineCosine(float *sine, float *cosine, unsigned short angle); // 0x43c390
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

namespace {
// Original .rdata loads are binary32. Volatile reads retain that width under
// C++11 x87 excess-precision rules instead of promoting decimal literals early.
const volatile float light78D944 = 0.282095f;
const volatile float light78D940 = 0.488603f;
const volatile float light78D93C = 1.092548f;
const volatile float light78D938 = 0.315392f;
const volatile float light78D934 = 0.546274f;
const volatile float light78D930 = 0.886227f;
const volatile float light78D92C = 0.247708f;
const volatile float light78D928 = 0.429043f;
const volatile float light78D924 = 0.743125f;
const volatile float light78D920 = 0.858086f;
const volatile float light78D91C = 1.023328f;
const volatile float light78D948 = -0.05f;
}

// Original 0x48ab10.
void CarRenderClearRecord(CarRenderRecord *record) {
    unsigned int *words = reinterpret_cast<unsigned int *>(record);
    for (int i = 0; i < 0x70; ++i) {
        words[i] = 0;
    }
}

// Original 0x48a310. Convert before signed division; the trig helper consumes the
// low 16 bits. The second angle is polar (sine in XY, cosine in Z), not latitude.
void CarRenderLightVector(float *output, float azimuthDegrees, float polarDegrees, float radius) {
    const int azimuth = static_cast<int>(static_cast<long double>(azimuthDegrees) * 65536.0f) / 360;
    float sinAzimuth, cosAzimuth;
    WheelSineCosine(&sinAzimuth, &cosAzimuth, static_cast<unsigned short>(azimuth));
    const int polar = static_cast<int>(static_cast<long double>(polarDegrees) * 65536.0f) / 360;
    float sinPolar, cosPolar;
    WheelSineCosine(&sinPolar, &cosPolar, static_cast<unsigned short>(polar));
    output[0] = (static_cast<long double>(sinPolar) * cosAzimuth) * radius;
    output[1] = (static_cast<long double>(sinPolar) * sinAzimuth) * radius;
    output[2] = static_cast<long double>(cosPolar) * radius;
}

// Original 0x4895e0. Constants are the reference image's binary32 values, checked
// by the verifier. Every observed binary32 product spill is explicit.
void CarRenderRecord::AccumulateLight(const float *color, float weight, float x, float y, float z) {
    const float colors[4] = {color[0], color[1], color[2], color[3]};
    volatile float zTerm = (static_cast<long double>(z) * z) * 3.0f - 1.0f; // 0x787adc / 0x784250
    volatile float difference = static_cast<long double>(x) * x - static_cast<long double>(y) * y;
    float *values = reinterpret_cast<float *>(this);
    for (int lane = 0; lane < 4; ++lane) {
        float *p = values + lane;
        const long double scaled = static_cast<long double>(colors[lane]) * weight;
        const long double constant = light78D944 * scaled; // 0x78d944
        const long double linear = scaled * light78D940;  // 0x78d940
        volatile float linearY = static_cast<long double>(y) * linear;
        volatile float linearZ = static_cast<long double>(z) * linear;
        volatile float linearX = linear * x;
        const long double cross = scaled * light78D93C;   // 0x78d93c
        const long double crossX = static_cast<long double>(x) * cross;
        volatile float xy = static_cast<long double>(y) * crossX;
        volatile float yz = (cross * y) * z;
        volatile float xz = crossX * z;
        volatile float quadraticZ = (scaled * light78D938) * zTerm; // 0x78d938
        volatile float quadraticDifference = (scaled * light78D934) * difference; // 0x78d934
        p[0] = (constant * light78D930 - static_cast<long double>(quadraticZ) * light78D92C) + p[0];
        const long double q = static_cast<long double>(quadraticDifference) * light78D928;
        p[4] = q + p[4];
        p[8] = static_cast<long double>(p[8]) - q;
        p[12] = static_cast<long double>(quadraticZ) * light78D924 + p[12];
        p[16] = static_cast<long double>(xy) * light78D920 + p[16];
        p[20] = static_cast<long double>(xz) * light78D920 + p[20];
        p[24] = static_cast<long double>(yz) * light78D928 + p[24];
        p[28] = static_cast<long double>(linearX) * light78D91C + p[28];
        p[32] = static_cast<long double>(linearY) * light78D91C + p[32];
        p[36] = static_cast<long double>(linearZ) * light78D91C + p[36];
    }
}

// Original 0x489af0. Color is read again after the first accumulation; it may
// alias the receiver. Direction is snapshotted before either call.
void CarRenderRecord::AddLightPair(const float *direction, const float *color) {
    const float x = direction[0], y = direction[1], z = direction[2];
    AccumulateLight(color, 1.0f, x, y, z); // 0x78d784
    float opposite[4];
    for (int i = 0; i < 4; ++i) {
        opposite[i] = static_cast<long double>(color[i]) * light78D948; // 0x78d948
    }
    AccumulateLight(opposite, 1.0f, -x, -y, -z);
}

// Original 0x489ed0. RGB squared sums are stored as binary32 before selecting
// their maximum. The fourth lane stores its square root instead.
void CarRenderRecord::NormalizeLightCoefficients(int firstThree, float firstThreeTarget,
                                               int fourth, float fourthTarget) {
    float *values = reinterpret_cast<float *>(this);
    if (firstThree) {
        float maximum = 0.0f;
        for (int lane = 0; lane < 3; ++lane) {
            static const unsigned char order[3][10] = {
                {2, 3, 4, 5, 6, 7, 8, 0, 9, 1},
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
            };
            const float first = values[order[lane][0] * 4 + lane];
            long double sum = static_cast<long double>(first) * first;
            for (int i = 1; i < 10; ++i) {
                const float value = values[order[lane][i] * 4 + lane];
                sum += static_cast<long double>(value) * value;
            }
            volatile float stored = sum;
            if (stored > maximum) {
                maximum = stored;
            }
        }
#ifdef __GNUC__
        const long double length = __builtin_sqrtl(static_cast<long double>(maximum));
#else
        const long double length = sqrt(maximum);
#endif
        if (length > 0.0f) {
            const long double factor = static_cast<long double>(firstThreeTarget) / length;
            for (int lane = 0; lane < 3; ++lane) {
                for (int group = 0; group < 10; ++group) {
                    values[group * 4 + lane] = factor * values[group * 4 + lane];
                }
            }
        }
    }
    if (fourth) {
        long double sum = static_cast<long double>(values[3]) * values[3];
        for (int group = 1; group < 10; ++group) {
            const float value = values[group * 4 + 3];
            sum += static_cast<long double>(value) * value;
        }
#ifdef __GNUC__
        volatile float length = __builtin_sqrtl(sum);
#else
        volatile float length = sqrt(sum);
#endif
        if (length > 0.0f) {
            const long double factor = static_cast<long double>(fourthTarget) / length;
            for (int group = 0; group < 10; ++group) {
                values[group * 4 + 3] = factor * values[group * 4 + 3];
            }
        }
    }
}
