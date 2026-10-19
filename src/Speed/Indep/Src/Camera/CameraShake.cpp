#include "DriveCamera.h"
#include "Camera.h"
#include "CameraPush.h"
#include "../Physics/VehicleMover.h"

// Original globals (bound to the mapped retail image by OriginalSymbols.ld).
extern float fengZero7D7D3C, fengOne784250;     // 0.0 and 1.0 in .rdata
extern float cameraShakeSpacing784258;          // 100.0, spacing when the car has no physics info
extern float cameraShakeKeyCount7844F8;         // 4.0, keys spanned by one spacing
extern float cameraShakeKeys7EC430[5][4];       // the five keys 0x446d00 blends
extern float cameraBumpQuarter784D28;           // 0.25
extern float cameraBumpGain7870BC;              // 10.0
extern float cameraNitrousAdd7870B0;            // 0.2
extern float cameraNitrousScale7870B4;          // 1.4
extern float cameraShakeNoise7EC480[4];
extern float cameraShakeNoise7EC490[4];
extern float cameraShakeNoise7EC4A0[4];
extern float cameraShakeNoise7EC4B0[4];
extern float cameraSpeedScale78426C;            // 2.237
extern float cameraThrottleLimit78435C;         // 0.001
extern float cameraSpeedLimit7A5660;            // 5.0
extern float cameraClockScale784268;
extern int cameraClock86518C;
extern float cameraViewShakeAngle7FB740;        // -1200.0
extern float cameraViewShakeAngle7FB744;        // 2000.0
extern float cameraViewShakeAngle7FB748;        // 0.0
extern CameraViewShakeState cameraViewShake865360[2];
extern CameraCarList *carList890080;

// Original 0x401020 / 0x5bada0 / 0x5bae60 are recovered in the physics sources.
void RotateMatrixY(WheelMatrix *output, WheelMatrix *input, unsigned int angle);
void RotateMatrixZ(WheelMatrix *output, WheelMatrix *input, unsigned int angle);

// MoverModeOwner::State() (car +0x40) written out so the /Ob0 verifier build needs no extra symbol.
#define CarState(car) reinterpret_cast<DrivetrainAuxiliary *>((car)->unidentified40)

// Original 0x43d100 (cdecl). All four loads precede the stores, so `output` may alias `base`.
void CameraScaleAdd4(float *output, float *base, float *direction, float scale) {
    float y = scale * direction[1] + base[1];
    float z = scale * direction[2] + base[2];
    float w = scale * direction[3] + base[3];
    float x = scale * direction[0] + base[0];
    output[3] = w;
    output[0] = x;
    output[1] = y;
    output[2] = z;
}

// Original 0x5766e0 (`ret 0x10`).
void __stdcall CameraBlend4(float *output, float *first, float *second, float t) {
    float a1 = first[1];
    float a2 = first[2];
    float a3 = first[3];
    output[0] = t * first[0];
    output[1] = a1 * t;
    output[2] = a2 * t;
    output[3] = a3 * t;
    CameraScaleAdd4(output, output, second, 1.0f - t);
}

// Original 0x446d00.
void CameraShakeCurve::Sample(float *output, float x) {
    int count = count0;
    // The original truncates the product before it is rounded to a binary32 (FST at 0x446d11 keeps
    // the register, 0x75bed4 converts it) and blends with the rounded copy.
    int index = (int)((x - start4) * inverseSpacingC);
    volatile float position = (x - start4) * inverseSpacingC;
    if (index < 0) {
        float *key = keys10;
        output[0] = key[0];
        output[1] = key[1];
        output[2] = key[2];
        output[3] = key[3];
        return;
    }
    if (count - 1 <= index) {
        float *key = keys10 + (count - 1) * 4;
        output[0] = key[0];
        output[1] = key[1];
        output[2] = key[2];
        output[3] = key[3];
        return;
    }
    float floorValue = (float)index;
    if (position < floorValue) {
        floorValue = floorValue - 1.0f;
    }
    float *low = keys10 + index * 4;
    CameraBlend4(output, low + 4, low, position - floorValue);
}

// Original 0x5ecdb0: the nitrous stage of the car when the throttle is inside the dead zone or
// the scaled speed is below the limit, else 0.
int MoverModeOwner::GetCameraNitrousIndex() {
    float scaledSpeed = speed430 * cameraSpeedScale78426C;
    float throttle = input30->throttle20C;
    float magnitude = throttle < 0.0f ? -throttle : throttle;
    if (magnitude <= cameraThrottleLimit78435C || scaledSpeed < cameraSpeedLimit7A5660) {
        return CarState(this)->GetNitrousIndex();
    }
    return 0;
}

// Original 0x452ed0. The key curve is spaced by the car's PhysicsParameters +0x10; its sample at
// the car speed (x, y, z, w) is combined with the bump sums of the four wheel surfaces (road
// surface +0x18 frequency, +0x1c amplitude) into the Camera noise fields (+0x70, +0x80, +0x90,
// +0xa0 by the four tables 0x7ec480..0x7ec4b0), then 0x441340 applies them with the clock and
// a scale of one.
void DriveCameraMover::Shake(WheelMatrix *matrix, MoverModeOwner *car, float scale, float secondScale) {
    float spacing = cameraShakeSpacing784258;
    if (car->physicsInfo10) {
        spacing = car->physicsInfo10->parameters10.value10;
    }
    CameraShakeCurve curve;
    curve.count0 = 5;
    curve.start4 = 0.0f;
    curve.spacing8 = spacing;
    curve.inverseSpacingC = cameraShakeKeyCount7844F8 / spacing;
    curve.keys10 = &cameraShakeKeys7EC430[0][0];
    float sample[4];
    curve.Sample(sample, car->speed430);
    volatile float lifted = sample[2];

    DrivetrainAuxiliary *state = CarState(car);
    RoadSurface *surface0 = &roadSurfaces[state->surfaces354[0]];
    RoadSurface *surface1 = &roadSurfaces[state->surfaces354[1]];
    RoadSurface *surface2 = &roadSurfaces[state->surfaces354[2]];
    RoadSurface *surface3 = &roadSurfaces[state->surfaces354[3]];
    volatile float amplitude01 = surface0->bumpAmplitude1C + surface1->bumpAmplitude1C;
    volatile float frequency012 = (surface0->bumpFrequency18 + surface1->bumpFrequency18) + surface2->bumpFrequency18;
    volatile float frequency = (frequency012 + surface3->bumpFrequency18) * cameraBumpQuarter784D28;
    volatile float amplitude = (cameraBumpGain7870BC * ((amplitude01 + surface2->bumpAmplitude1C) + surface3->bumpAmplitude1C))
                      * cameraBumpQuarter784D28;
    volatile float speedShake = sample[0] * scale;
    if (state && state->GetNitrousIndex() && !car->GetCameraNitrousIndex()) {
        speedShake = cameraNitrousAdd7870B0 + speedShake;
        lifted = cameraNitrousScale7870B4 * lifted;
    }
    Camera *target = camera1C;
    float *a = target->current0.noise70;
    float *b = target->current0.noise80;
    float *c = target->current0.noise90;
    float *d = target->current0.noiseA0;
    float held = sample[1] * amplitude * secondScale;
    volatile float frequencyShake = sample[3] * frequency;
    a[0] = cameraShakeNoise7EC480[0] * lifted;
    a[1] = cameraShakeNoise7EC480[1] * lifted;
    a[2] = cameraShakeNoise7EC480[2] * lifted;
    a[3] = cameraShakeNoise7EC480[3] * lifted;
    b[0] = cameraShakeNoise7EC490[0] * speedShake;
    b[1] = cameraShakeNoise7EC490[1] * speedShake;
    b[2] = cameraShakeNoise7EC490[2] * speedShake;
    b[3] = cameraShakeNoise7EC490[3] * speedShake;
    c[0] = cameraShakeNoise7EC4A0[0] * frequencyShake;
    c[1] = cameraShakeNoise7EC4A0[1] * frequencyShake;
    c[2] = cameraShakeNoise7EC4A0[2] * frequencyShake;
    c[3] = cameraShakeNoise7EC4A0[3] * frequencyShake;
    d[0] = cameraShakeNoise7EC4B0[0] * held;
    d[1] = cameraShakeNoise7EC4B0[1] * held;
    d[2] = cameraShakeNoise7EC4B0[2] * held;
    d[3] = cameraShakeNoise7EC4B0[3] * held;
    target->ApplyNoise(matrix, (float)cameraClock86518C * cameraClockScale784268, 1.0f);
}

// Original 0x5722a0.
float CameraViewShakeCurve::Lookup(float input) {
    float *p = points0;
    int count = count4;
    if (input < p[0]) {
        return fengZero7D7D3C;
    }
    if (p[count * 2 - 2] <= input) {
        return fengZero7D7D3C;
    }
    int i = 0;
    if (count > 0) {
        do {
            if (input < p[i * 2]) {
                break;
            }
            ++i;
        } while (i < count);
        if (i > 0) {
            --i;
        }
    }
    if (i > count - 2) {
        return (input - p[i * 2]) * fengZero7D7D3C + p[i * 2 + 1];
    }
    return (input - p[i * 2]) * ((p[i * 2 + 3] - p[i * 2 + 1]) / (p[i * 2 + 2] - p[i * 2])) + p[i * 2 + 1];
}

// Original 0x572860.
void CameraViewShakeState::Evaluate(float *output) {
    float value;
    if (!((int)(value1C * time14) & 1) && curve10) {
        value = curve10->Lookup(time14);
    } else {
        value = fengZero7D7D3C;
    }
    float z = vector0[2];
    output[0] = value * vector0[0];
    output[1] = vector0[1] * value;
    output[2] = z * value;
}

// Original 0x572a50.
void CameraViewShake(int view, WheelMatrix *matrix) {
    if (*reinterpret_cast<float *>(reinterpret_cast<char *>(carList890080) + 0x108) < fengOne784250) {
        return;
    }
    int slot;
    if (view == 1 || view == 3) {
        slot = 0;
    } else if (view == 2) {
        slot = 1;
    } else {
        return;
    }
    float v[3];
    cameraViewShake865360[slot].Evaluate(v);
    WheelMatrix translation;
    WheelIdentity(&translation);
    const float *m = matrix->elements;
    translation.elements[15] = 1.0f;
    float x = v[2] * m[8] + v[1] * m[4] + v[0] * m[0] + m[12] * fengZero7D7D3C;
    float y = v[1] * m[5] + v[2] * m[9] + v[0] * m[1] + m[13] * fengZero7D7D3C;
    float z = v[1] * m[6] + v[2] * m[10] + v[0] * m[2] + m[14] * fengZero7D7D3C;
    // The translation is stored to the matrix and the angles read it back (binary32); the
    // volatile access keeps a compiler from forwarding the unrounded x87 values.
    volatile float *t = translation.elements;
    t[14] = z;
    t[12] = x;
    t[13] = y;
    RotateMatrixX(&translation, &translation, (unsigned int)(int)((t[13] + t[14]) * cameraViewShakeAngle7FB740));
    RotateMatrixY(&translation, &translation, (unsigned int)(int)(cameraViewShakeAngle7FB744 * t[12]));
    RotateMatrixZ(&translation, &translation, (unsigned int)(int)((t[12] + t[13]) * cameraViewShakeAngle7FB748));

    WheelMatrix product;
    float *o = product.elements;
    const float *r = translation.elements;
    o[0] = r[4] * m[1] + r[0] * m[0] + r[8] * m[2] + r[12] * m[3];
    o[1] = r[5] * m[1] + r[1] * m[0] + r[9] * m[2] + r[13] * m[3];
    o[2] = r[6] * m[1] + r[2] * m[0] + r[10] * m[2] + r[14] * m[3];
    o[3] = r[7] * m[1] + r[3] * m[0] + r[11] * m[2] + r[15] * m[3];
    o[4] = r[4] * m[5] + r[12] * m[7] + r[0] * m[4] + r[8] * m[6];
    o[5] = r[5] * m[5] + r[13] * m[7] + r[1] * m[4] + r[9] * m[6];
    o[6] = r[6] * m[5] + r[14] * m[7] + r[2] * m[4] + r[10] * m[6];
    o[7] = r[7] * m[5] + r[15] * m[7] + r[3] * m[4] + r[11] * m[6];
    o[8] = r[8] * m[10] + r[12] * m[11] + r[0] * m[8] + r[4] * m[9];
    o[9] = r[9] * m[10] + r[13] * m[11] + r[1] * m[8] + r[5] * m[9];
    o[10] = r[10] * m[10] + r[14] * m[11] + r[2] * m[8] + r[6] * m[9];
    o[11] = r[11] * m[10] + r[15] * m[11] + r[3] * m[8] + r[7] * m[9];
    o[12] = r[4] * m[13] + r[0] * m[12] + r[12] * m[15] + r[8] * m[14];
    o[13] = r[5] * m[13] + r[1] * m[12] + r[13] * m[15] + r[9] * m[14];
    o[14] = r[6] * m[13] + r[2] * m[12] + r[14] * m[15] + r[10] * m[14];
    o[15] = r[7] * m[13] + r[3] * m[12] + r[15] * m[15] + r[11] * m[14];
    BodyMatrixCopy(matrix, &product);
}
