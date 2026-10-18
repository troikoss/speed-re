#include "CameraMatrix.h"

extern float fengZero7D7D3C, fengOne784250;  // 0.0f and 1.0f in .rdata
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output, WheelMatrix *left, WheelMatrix *right);

// Original 0x5ba9a0. Forward = normalize(target - eye); right = cross(-up, forward);
// up' = cross(forward, right); both normalized after the second cross. The rotation is
// written to `output`, then translation(-eye) times it is formed with the zero entries
// still multiplied by the .rdata zero (not folded) and copied over `output` by 0x4012b0.
void CameraLookAt(WheelMatrix *output, WheelVector *eye, WheelVector *target, WheelVector *up) {
    WheelVector forward, down, right;
    forward.x = target->x - eye->x;
    forward.y = target->y - eye->y;
    forward.z = target->z - eye->z;
    WheelNormalize(&forward, &forward);
    down.x = -up->x;
    down.y = -up->y;
    down.z = -up->z;
    WheelCross(&right, &down, &forward);
    WheelCross(&down, &forward, &right);
    WheelNormalize(&right, &right);
    WheelNormalize(&down, &down);
    float *m = output->elements;
    m[0] = right.x;
    m[2] = forward.x;
    m[4] = right.y;
    m[5] = down.y;
    m[6] = forward.y;
    m[8] = right.z;
    m[1] = down.x;
    m[9] = down.z;
    m[3] = 0.0f;
    m[7] = 0.0f;
    m[10] = forward.z;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
    // FCHS on the loaded eye: kept as negated values (an fsub would not flip a NaN's sign).
    volatile float ex = -eye->x, ey = -eye->y, ez = -eye->z;
    const float zero = fengZero7D7D3C;
    WheelMatrix product;
    float *p = product.elements;
    p[0] = right.x + right.y * zero + right.z * zero;
    p[1] = down.x + down.y * zero + down.z * zero;
    p[2] = forward.x + forward.y * zero + forward.z * zero;
    p[3] = 0.0f;
    p[4] = right.y + right.x * zero + right.z * zero;
    p[5] = down.y + down.x * zero + down.z * zero;
    p[6] = forward.y + forward.x * zero + forward.z * zero;
    p[7] = 0.0f;
    p[8] = right.z + right.x * zero + right.y * zero;
    p[9] = down.z + down.x * zero + down.y * zero;
    p[10] = forward.z + forward.x * zero + forward.y * zero;
    p[11] = 0.0f;
    p[12] = right.z * ez + ey * right.y + ex * right.x;
    p[13] = down.z * ez + ey * down.y + ex * down.x;
    p[14] = forward.z * ez + ey * forward.y + ex * forward.x;
    p[15] = ez * zero + ey * zero + ex * zero + fengOne784250;
    BodyMatrixCopy(output, &product);
}

// Original 0x5baf90.
WheelMatrix *CameraInvertRigid(WheelMatrix *output, WheelMatrix *input) {
    float *m = input->elements;
    WheelMatrix rotation, translation;
    float *r = rotation.elements;
    float *t = translation.elements;
    r[1] = m[4];
    r[2] = m[8];
    r[4] = m[1];
    r[5] = m[5];
    r[6] = m[9];
    r[8] = m[2];
    r[0] = m[0];
    r[3] = 0.0f;
    r[9] = m[6];
    r[7] = 0.0f;
    r[10] = m[10];
    r[11] = 0.0f;
    t[12] = -m[12];
    r[12] = 0.0f;
    r[13] = 0.0f;
    t[13] = -m[13];
    r[14] = 0.0f;
    t[14] = -m[14];
    r[15] = 1.0f;
    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 0.0f;
    t[4] = 0.0f;
    t[5] = 1.0f;
    t[6] = 0.0f;
    t[7] = 0.0f;
    t[8] = 0.0f;
    t[9] = 0.0f;
    t[10] = 1.0f;
    t[11] = 0.0f;
    t[15] = 1.0f;
    D3DXMatrixMultiply(output, &translation, &rotation);
    return output;
}

// Original 0x444390. The view-space x and depth are stored as binary32 before the offset
// is applied; y stays on the x87 stack.
void __stdcall CameraReaim(WheelMatrix *view, WheelVector *eye, WheelVector *target, float *offset) {
    WheelVector down;
    down.x = -view->elements[1];
    down.y = -view->elements[5];
    down.z = -view->elements[9];
    WheelMatrix look, inverse;
    CameraLookAt(&look, eye, target, &down);
    CameraInvertRigid(&inverse, &look);
    float *l = look.elements;
    float *n = inverse.elements;
    volatile float x = l[8] * target->z + l[4] * target->y + l[0] * target->x + l[12];
    float y = l[5] * target->y + l[9] * target->z + l[1] * target->x + l[13];
    volatile float depth = l[6] * target->y + l[10] * target->z + l[2] * target->x + l[14];
    x = x - depth * offset[0];
    y = y - depth * offset[1];
    WheelVector moved;
    moved.x = n[4] * y + n[8] * depth + n[0] * x + n[12];
    moved.y = n[5] * y + n[1] * x + n[9] * depth + n[13];
    moved.z = n[6] * y + n[2] * x + n[10] * depth + n[14];
    CameraLookAt(view, eye, &moved, &down);
}
