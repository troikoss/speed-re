#include "Camera.h"
#include "../Physics/Engine.h"

// Original globals (bound to the mapped retail image by OriginalSymbols.ld).
extern int cameraFinaliseDisabled828640;
extern int cameraDebugEnabled82A480;
extern int cameraDebugSuppressed865094;
extern int cameraDebugActive82A4F8;
extern WheelMatrix cameraDebugMatrix82A490;
extern float cameraDebugScale78425C;           // 0.01, scales the debug matrix translation
extern float fengZero7D7D3C;                    // 0.0 in .rdata
extern float cameraNoiseScale787580;            // 2^-30, scales the 31-bit hash
extern float cameraNoiseFalloff786F6C;          // 0.5, amplitude of the next octave
extern float cameraTurnUnits784238;             // 65536.0
extern unsigned short cameraReferenceFov7EC1F0; // reference full angle for 0x4412e0

// Recovered in the physics and HUD sources.
float EngineCosineUnits(unsigned int units);                                      // 0x43c370
void RotateMatrixY(WheelMatrix *output, WheelMatrix *input, unsigned int angle);  // 0x5bada0
unsigned short HudArcSineUnits(float ratio);                                      // 0x43c490

// Statically linked D3DX multiply dispatched through the pointer at 0x803e74.
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output, WheelMatrix *left, WheelMatrix *right);

// Original 0x4411b0. The lattice index is the float truncated toward zero, lowered by one when
// that is above the value; the two hashes are folded into [0, 1) values and blended linearly.
float CameraNoiseValue(float x) {
    int truncated = (int)x;
    float lower = (float)truncated;
    if (!(lower <= x)) {
        lower = lower - 1.0f;
    }
    int index = (int)lower;
    unsigned int first = ((unsigned int)index << 13) ^ (unsigned int)index;
    unsigned int next = (unsigned int)index + 1;
    unsigned int second = (next << 13) ^ next;
    float fraction = x - (float)index;
    unsigned int hash0 = (((first * first) * 0x3d73u + 0xc0ae5u) * first - 0x2df722f3u) & 0x7fffffffu;
    unsigned int hash1 = (((second * second) * 0x3d73u + 0xc0ae5u) * second - 0x2df722f3u) & 0x7fffffffu;
    return (1.0f - (float)(int)hash0 * cameraNoiseScale787580) * (1.0f - fraction)
         + (1.0f - (float)(int)hash1 * cameraNoiseScale787580) * fraction;
}

// Original 0x441270.
float CameraNoiseSum(float x) {
    float frequency = 1.0f;
    float amplitude = 1.0f;
    volatile float sum = 0.0f;   // stored every octave in the original
    int octave = 6;
    do {
        sum = CameraNoiseValue(frequency * x) * amplitude + sum;
        --octave;
        frequency = frequency + frequency;
        amplitude = cameraNoiseFalloff786F6C * amplitude;
    } while (octave != 0);
    return sum;
}

// Original 0x5bacf0: the rotation about X is built next to the product 0x63cedb as 0x5bada0 does
// for Y; the cosine is a call of 0x43c370 here.
void RotateMatrixX(WheelMatrix *output, WheelMatrix *input, unsigned int angle) {
    volatile float cosine = EngineCosineUnits(angle);
    float sine = EngineSineUnits(angle);
    WheelMatrix rotation;
    float *m = rotation.elements;
    m[5] = cosine;
    m[6] = sine;
    m[9] = -sine;
    m[10] = cosine;
    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
    D3DXMatrixMultiply(output, input, &rotation);
}

// Original 0x5baf80 (cdecl): hands both arguments to the D3DX transpose and returns its result.
WheelMatrix *CameraTranspose(WheelMatrix *output, WheelMatrix *input) {
    return D3DXMatrixTranspose(output, input);
}

// Original 0x4412e0.
unsigned short Camera::GetFieldAngle(int angle) {
    float fieldSine = EngineSineUnits(current0.fovC4 >> 1);
    float product = EngineSineUnits(angle) * fieldSine;
    float referenceSine = EngineSineUnits(cameraReferenceFov7EC1F0 >> 1);
    float ratio = product / referenceSine;
    return HudArcSineUnits(ratio);
}

// Original 0x441340. The two noise sets (time scales +0x70/+0x90, amplitudes +0x80/+0xa0) are
// summed per component and scaled; components 0 and 1 translate, 2 and 3 become the angles of
// a rotation about X and then Y, in 1/65536 turn units divided by 360 (the shake amplitudes are
// degrees). The shaken matrix is the input times that transform, written back over the input.
void Camera::ApplyNoise(WheelMatrix *matrix, float time, float scale) {
    const float *scale70 = current0.noise70;
    const float *amplitude80 = current0.noise80;
    const float *scale90 = current0.noise90;
    const float *amplitudeA0 = current0.noiseA0;
    // The volatile results keep the eight calls in the original's order.
    float p0 = scale70[0] * time;
    float p1 = scale70[1] * time;
    float p2 = scale70[2] * time;
    float p3 = scale70[3] * time;
    volatile float n0 = CameraNoiseSum(p0);
    volatile float n1 = CameraNoiseSum(p1);
    volatile float n2 = CameraNoiseSum(p2);
    volatile float n3 = CameraNoiseSum(p3);
    volatile float v0 = n0 * amplitude80[0];
    volatile float v1 = n1 * amplitude80[1];
    volatile float v2 = n2 * amplitude80[2];
    volatile float v3 = n3 * amplitude80[3];
    float q0 = scale90[0] * time;
    float q1 = scale90[1] * time;
    float q2 = scale90[2] * time;
    float q3 = scale90[3] * time;
    volatile float m0 = CameraNoiseSum(q0);
    volatile float m1 = CameraNoiseSum(q1);
    volatile float m2 = CameraNoiseSum(q2);
    volatile float m3 = CameraNoiseSum(q3);
    volatile float t1 = m1 * amplitudeA0[1];
    volatile float t2 = m2 * amplitudeA0[2];
    volatile float t3 = m3 * amplitudeA0[3];
    float w0 = (m0 * amplitudeA0[0] + v0) * scale;
    float w1 = (t1 + v1) * scale;
    volatile float sum2 = t2 + v2;
    float w2 = sum2 * scale;
    volatile float sum3 = t3 + v3;
    float w3 = sum3 * scale;

    WheelMatrix rotation;
    WheelIdentity(&rotation);
    rotation.elements[12] = w0;
    rotation.elements[13] = w1;
    int angleX = (int)(w2 * cameraTurnUnits784238) / 360;
    RotateMatrixX(&rotation, &rotation, GetFieldAngle(angleX));
    int angleY = (int)(w3 * cameraTurnUnits784238) / 360;
    RotateMatrixY(&rotation, &rotation, GetFieldAngle(angleY));

    const float *r = rotation.elements;
    const float *i = matrix->elements;
    WheelMatrix product;
    float *o = product.elements;
    o[0] = r[0] * i[0] + r[12] * i[3] + r[8] * i[2] + r[4] * i[1];
    o[1] = r[13] * i[3] + r[9] * i[2] + r[5] * i[1] + r[1] * i[0];
    o[2] = r[10] * i[2] + r[6] * i[1] + r[14] * i[3] + r[2] * i[0];
    o[3] = r[11] * i[2] + r[7] * i[1] + r[15] * i[3] + r[3] * i[0];
    o[4] = r[12] * i[7] + r[0] * i[4] + r[4] * i[5] + r[8] * i[6];
    o[5] = r[1] * i[4] + r[13] * i[7] + r[5] * i[5] + r[9] * i[6];
    o[6] = r[2] * i[4] + r[14] * i[7] + r[6] * i[5] + r[10] * i[6];
    o[7] = r[3] * i[4] + r[15] * i[7] + r[7] * i[5] + r[11] * i[6];
    o[8] = r[12] * i[11] + r[0] * i[8] + r[4] * i[9] + r[8] * i[10];
    o[9] = r[1] * i[8] + r[13] * i[11] + r[5] * i[9] + r[9] * i[10];
    o[10] = r[2] * i[8] + r[14] * i[11] + r[6] * i[9] + r[10] * i[10];
    o[11] = r[3] * i[8] + r[15] * i[11] + r[7] * i[9] + r[11] * i[10];
    o[12] = r[12] * i[15] + r[0] * i[12] + r[4] * i[13] + r[8] * i[14];
    o[13] = r[1] * i[12] + r[13] * i[15] + r[5] * i[13] + r[9] * i[14];
    o[14] = r[2] * i[12] + r[14] * i[15] + r[6] * i[13] + r[10] * i[14];
    o[15] = r[3] * i[12] + r[15] * i[15] + r[7] * i[13] + r[11] * i[14];
    BodyMatrixCopy(matrix, &product);
}

// Original 0x447280.
void Camera::Finalise(WheelMatrix *matrix, float step) {
    if (cameraFinaliseDisabled828640) {
        return;
    }
    previousD0 = current0;
    step274 = step;
    WheelMatrix debug;
    WheelMatrix *source;
    if (cameraDebugEnabled82A480 && !cameraDebugSuppressed865094) {
        if (!cameraDebugActive82A4F8) {
            cameraDebugActive82A4F8 = 1;
        }
        debug = cameraDebugMatrix82A490;
        debug.elements[12] = debug.elements[12] * cameraDebugScale78425C;
        debug.elements[13] = debug.elements[13] * cameraDebugScale78425C;
        debug.elements[14] = debug.elements[14] * cameraDebugScale78425C;
        debug.elements[15] = 1.0f;
        source = &debug;
    } else {
        if (cameraDebugActive82A4F8) {
            cameraDebugActive82A4F8 = 0;
        }
        source = matrix;
    }
    BodyMatrixCopy(&current0.matrix0, source);

    WheelMatrix transposed;
    CameraTranspose(&transposed, &current0.matrix0);
    float *t = transposed.elements;
    const float *c = current0.matrix0.elements;
    t[3] = 0.0f;
    t[7] = 0.0f;
    t[11] = 0.0f;
    float x = t[0] * c[12] + t[4] * c[13] + t[8] * c[14] + t[12];
    float y = t[1] * c[12] + t[5] * c[13] + t[9] * c[14] + t[13];
    float z = t[2] * c[12] + t[6] * c[13] + t[10] * c[14] + t[14];
    current0.position40.x = -x;
    current0.position40.y = -y;
    current0.position40.z = -z;
    WheelNormalize(&current0.direction50, reinterpret_cast<WheelVector *>(&t[8]));
    float focal = current0.focalB0;
    float directionZ = current0.direction50.z;
    volatile float targetX = current0.direction50.x * focal;
    volatile float targetY = current0.direction50.y * focal;
    volatile float targetZ = directionZ * focal;
    current0.target60.x = targetX + current0.position40.x;
    current0.target60.y = targetY + current0.position40.y;
    current0.target60.z = targetZ + current0.position40.z;

    if (reset270) {
        previousD0 = current0;
        reset270 = 0;
        step274 = 1.0f;
    }
    if (!(step274 > fengZero7D7D3C)) {
        return;
    }
    float inverse = 1.0f / step274;
    volatile float *v;
    v = &velocity1E0.x;
    v[0] = current0.position40.x - previousD0.position40.x;
    v[1] = current0.position40.y - previousD0.position40.y;
    v[2] = current0.position40.z - previousD0.position40.z;
    zero1EC = 0.0f;
    v[0] = inverse * v[0];
    v[1] = inverse * v[1];
    v[2] = inverse * v[2];
    v = &velocity1F0.x;
    v[0] = current0.direction50.x - previousD0.direction50.x;
    v[1] = current0.direction50.y - previousD0.direction50.y;
    v[2] = current0.direction50.z - previousD0.direction50.z;
    zero1FC = 0.0f;
    v[0] = inverse * v[0];
    v[1] = inverse * v[1];
    v[2] = inverse * v[2];
    v = &velocity200.x;
    v[0] = current0.target60.x - previousD0.target60.x;
    v[1] = current0.target60.y - previousD0.target60.y;
    v[2] = current0.target60.z - previousD0.target60.z;
    zero20C = 0.0f;
    v[0] = inverse * v[0];
    v[1] = inverse * v[1];
    v[2] = inverse * v[2];

    unsigned short fovDelta = (unsigned short)(current0.fovC4 - previousD0.fovC4);
    focalVelocity250 = (current0.focalB0 - previousD0.focalB0) * inverse;
    velocityB4_254 = (current0.valueB4 - previousD0.valueB4) * inverse;
    velocityB8_258 = (current0.valueB8 - previousD0.valueB8) * inverse;
    nearVelocity25C = (current0.nearBC - previousD0.nearBC) * inverse;
    farVelocity260 = (current0.farC0 - previousD0.farC0) * inverse;
    fovVelocity264 = (short)(int)((float)(int)fovDelta * inverse);

    volatile float *w;
    w = noiseVelocity210;
    w[0] = current0.noise70[0] - previousD0.noise70[0];
    w[1] = current0.noise70[1] - previousD0.noise70[1];
    w[2] = current0.noise70[2] - previousD0.noise70[2];
    w[3] = current0.noise70[3] - previousD0.noise70[3];
    w[0] = inverse * w[0];
    w[1] = inverse * w[1];
    w[2] = inverse * w[2];
    w[3] = inverse * w[3];
    w = noiseVelocity230;
    w[0] = current0.noise90[0] - previousD0.noise90[0];
    w[1] = current0.noise90[1] - previousD0.noise90[1];
    w[2] = current0.noise90[2] - previousD0.noise90[2];
    w[3] = current0.noise90[3] - previousD0.noise90[3];
    w[0] = inverse * w[0];
    w[1] = inverse * w[1];
    w[2] = inverse * w[2];
    w[3] = inverse * w[3];
    w = noiseVelocity220;
    w[0] = current0.noise80[0] - previousD0.noise80[0];
    w[1] = current0.noise80[1] - previousD0.noise80[1];
    w[2] = current0.noise80[2] - previousD0.noise80[2];
    w[3] = current0.noise80[3] - previousD0.noise80[3];
    w[0] = inverse * w[0];
    w[1] = inverse * w[1];
    w[2] = inverse * w[2];
    w[3] = inverse * w[3];
    w = noiseVelocity240;
    w[0] = current0.noiseA0[0] - previousD0.noiseA0[0];
    w[1] = current0.noiseA0[1] - previousD0.noiseA0[1];
    w[2] = current0.noiseA0[2] - previousD0.noiseA0[2];
    w[3] = current0.noiseA0[3] - previousD0.noiseA0[3];
    w[0] = inverse * w[0];
    w[1] = inverse * w[1];
    w[2] = inverse * w[2];
    w[3] = inverse * w[3];
}
