#include "CarRenderRecord.h"

// Original 0x489ba0. Four independent lanes of ten coefficients occupy +0..+0x9c.
// Matrix inputs are snapshotted before any record store. Keep the asymmetric
// binary32 temporaries and addition order observed in the x87 instructions.
void CarRenderRecord::TransformCoefficients(WheelMatrix *matrix) {
    const float u0 = matrix->elements[0], u1 = matrix->elements[4], u2 = matrix->elements[8];
    const float v0 = matrix->elements[1], v1 = matrix->elements[5], v2 = matrix->elements[9];
    const float w0 = matrix->elements[2], w1 = matrix->elements[6], w2 = matrix->elements[10];
    float *values = reinterpret_cast<float *>(this);
    for (int lane = 0; lane < 4; ++lane) {
        float *p = values + lane;
        const long double b = p[4], c = p[8], d = p[12];
        const float e = p[16], f = p[20], g = p[24];
        const float h = p[28], i = p[32], j = p[36];

        p[4] = (((static_cast<long double>(g) * u1 + d * u2) +
                 static_cast<long double>(f) * u0) * u2 +
                (c * u1 + static_cast<long double>(e) * u0) * u1) + (b * u0) * u0;

        volatile float gv1 = static_cast<long double>(g) * v1;
        volatile float dv2 = d * v2;
        volatile float fv0 = static_cast<long double>(f) * v0;
        volatile float cv1 = c * v1;
        volatile float ev0 = static_cast<long double>(e) * v0;
        volatile float bv0 = b * v0;
        p[8] = (((static_cast<long double>(fv0) + dv2) + gv1) * v2 +
                (static_cast<long double>(ev0) + cv1) * v1) + static_cast<long double>(bv0) * v0;

        volatile float gw1 = static_cast<long double>(g) * w1;
        volatile float dw2 = d * w2;
        volatile float fw0 = static_cast<long double>(f) * w0;
        const long double cw1 = c * w1, ew0 = static_cast<long double>(e) * w0, bw0 = b * w0;
        p[12] = (((static_cast<long double>(fw0) + dw2) + gw1) * w2 + (cw1 + ew0) * w1) + bw0 * w0;

        long double xy = ((static_cast<long double>(g) * v2 * u1 +
                           static_cast<long double>(f) * v2 * u0) + static_cast<long double>(e) * v1 * u0);
        long double term = static_cast<long double>(bv0) * u0;
        xy += term + term;
        xy += static_cast<long double>(ev0) * u1;
        term = static_cast<long double>(cv1) * u1;
        xy += term + term;
        xy += static_cast<long double>(fv0) * u2;
        term = static_cast<long double>(dv2) * u2;
        xy += term + term;
        xy += static_cast<long double>(gv1) * u2;
        p[16] = xy;

        const long double gw2 = static_cast<long double>(g) * w2;
        const long double fw2 = static_cast<long double>(f) * w2;
        const long double ew1 = static_cast<long double>(e) * w1;
        volatile float ew1Stored = ew1; // 0x489db3 stores, but keeps the unrounded x87 value too
        long double xz = (ew1 + fw2) * u0 + gw2 * u1;
        term = bw0 * u0;
        xz += term + term;
        xz += ew0 * u1;
        term = cw1 * u1;
        xz += term + term;
        xz += static_cast<long double>(fw0) * u2;
        term = static_cast<long double>(dw2) * u2;
        xz += term + term;
        xz += static_cast<long double>(gw1) * u2;
        p[20] = xz;

        long double yz = (static_cast<long double>(ew1Stored) + fw2) * v0 + gw2 * v1;
        term = bw0 * v0;
        yz += term + term;
        yz += ew0 * v1;
        term = cw1 * v1;
        yz += term + term;
        yz += static_cast<long double>(fw0) * v2;
        term = static_cast<long double>(dw2) * v2;
        yz += term + term;
        yz += static_cast<long double>(gw1) * v2;
        p[24] = yz;

        p[28] = (static_cast<long double>(j) * u2 + static_cast<long double>(i) * u1) +
                static_cast<long double>(h) * u0;
        p[32] = (static_cast<long double>(j) * v2 + static_cast<long double>(i) * v1) +
                static_cast<long double>(h) * v0;
        p[36] = (static_cast<long double>(j) * w2 + static_cast<long double>(i) * w1) +
                static_cast<long double>(h) * w0;
    }
}
