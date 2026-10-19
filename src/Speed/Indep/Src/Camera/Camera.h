#ifndef UG2_CAMERA_H
#define UG2_CAMERA_H

// Recovered UG2 PC v1.2 Camera object (partial, offset-based). The drive camera mover reaches it
// through mover +0x1c; the base constructor 0x44ab90 takes it from the view object returned by
// 0x48b1e0 (view +0x40) and stores 10000.0 at +0xc0. Its own constructor and the code that fills
// +0x70..+0xac outside the drive camera are not recovered; the native demo supplies a zeroed
// object (DemoMain.cpp, documented there).
//
// Recovered here: finalisation 0x447280, the noise apply 0x441340 with 0x441270, 0x4411b0 and
// 0x4412e0, the matrix rotation 0x5bacf0 and the D3DX transpose wrapper 0x5baf80.

#include "../Physics/Ug2Layout.h"
#include "../Physics/WheelMath.h"

struct MoverModeOwner;

// The first 0xd0 bytes of the Camera. 0x447280 copies it to +0xd0 before it rewrites it, so the
// following frame can difference the two.
struct CameraState {
    WheelMatrix matrix0;                 // view matrix (D3D row vectors, translation in row 3)
    WheelVector position40;              // negated rotated translation of matrix0
    char unidentified4C[4];
    WheelVector direction50;             // normalised third column of the matrix
    char unidentified5C[4];
    WheelVector target60;                // position + direction * focalB0
    char unidentified6C[4];
    float noise70[4];                    // noise time scales, written by DriveCameraMover::Shake
    float noise80[4];                    // noise amplitudes
    float noise90[4];
    float noiseA0[4];
    float focalB0;                       // distance from the camera to the car point (car +0x60)
    float valueB4;
    float valueB8;
    float nearBC;
    float farC0;                         // 10000.0 from the base constructor
    unsigned short fovC4;                // full horizontal angle in 1/65536 turn
    char unidentifiedC6[10];
};
UG2_CHECK_SIZE(CameraState, 0xd0);
UG2_CHECK_OFFSET(CameraState, position40, 0x40);
UG2_CHECK_OFFSET(CameraState, direction50, 0x50);
UG2_CHECK_OFFSET(CameraState, target60, 0x60);
UG2_CHECK_OFFSET(CameraState, noise70, 0x70);
UG2_CHECK_OFFSET(CameraState, noiseA0, 0xa0);
UG2_CHECK_OFFSET(CameraState, focalB0, 0xb0);
UG2_CHECK_OFFSET(CameraState, nearBC, 0xbc);
UG2_CHECK_OFFSET(CameraState, fovC4, 0xc4);

struct Camera {
    CameraState current0;
    CameraState previousD0;              // copy of current0 made by 0x447280
    char unidentified1A0[0x40];
    WheelVector velocity1E0;             // per-field change over the step (0x447280), the vector
    float zero1EC;                       // groups with a stored zero in the fourth float
    WheelVector velocity1F0;
    float zero1FC;
    WheelVector velocity200;
    float zero20C;
    float noiseVelocity210[4];           // from noise70
    float noiseVelocity220[4];           // from noise80
    float noiseVelocity230[4];           // from noise90
    float noiseVelocity240[4];           // from noiseA0
    float focalVelocity250;
    float velocityB4_254;
    float velocityB8_258;
    float nearVelocity25C;
    float farVelocity260;
    short fovVelocity264;
    char unidentified266[10];
    unsigned char reset270;              // set: the next finalise copies the state and uses a step of 1
    char unidentified271[3];
    float step274;
    char unidentified278[8];
    MoverModeOwner *attachedCar280;      // the car for the bumper and driver cameras, else null
    int value284;                        // read by the base constructor 0x44ab90

    // 0x447280 (thiscall, `ret 8`). `step` is the frame time 0x865154. Does nothing while the
    // global 0x828640 is set.
    void Finalise(WheelMatrix *matrix, float step);
    // 0x441340 (thiscall, `ret 0xc`): shakes `matrix` in place with the noise fields +0x70..+0xac.
    void ApplyNoise(WheelMatrix *matrix, float time, float scale);
    // 0x4412e0 (thiscall, `ret 4`): arcsine(sin(angle) * sin(fov / 2) / sin(reference fov / 2)) in
    // 1/65536 turn units; the reference field of view is the word at 0x7ec1f0.
    unsigned short GetFieldAngle(int angle);
};
UG2_CHECK_OFFSET(Camera, previousD0, 0xd0);
UG2_CHECK_OFFSET(Camera, velocity1E0, 0x1e0);
UG2_CHECK_OFFSET(Camera, noiseVelocity210, 0x210);
UG2_CHECK_OFFSET(Camera, focalVelocity250, 0x250);
UG2_CHECK_OFFSET(Camera, fovVelocity264, 0x264);
UG2_CHECK_OFFSET(Camera, reset270, 0x270);
UG2_CHECK_OFFSET(Camera, step274, 0x274);
UG2_CHECK_OFFSET(Camera, attachedCar280, 0x280);
UG2_CHECK_SIZE(Camera, 0x288);

// 0x441270 (cdecl): six octaves of 0x4411b0, each at twice the frequency and 0x786f6c times the
// amplitude of the last.
float CameraNoiseSum(float x);
// 0x4411b0 (cdecl): integer-hash value noise, interpolated linearly between the lattice points.
float CameraNoiseValue(float x);
// 0x5bacf0 (cdecl): right-multiply by a rotation about X by a 16-bit angle through D3DX.
void RotateMatrixX(WheelMatrix *output, WheelMatrix *input, unsigned int angle);
// 0x5baf80 (cdecl): the statically linked D3DX transpose, reached through the pointer at 0x803f00.
WheelMatrix *CameraTranspose(WheelMatrix *output, WheelMatrix *input);

// Platform seam: the D3DX matrix transpose (see Platform/Native/NativeVehicle.cpp).
extern "C" WheelMatrix *__stdcall D3DXMatrixTranspose(WheelMatrix *output, WheelMatrix *input);

// Per-view shake state at 0x865360 (stride 0x20). The collision shake (0x572e50) and the tick
// and reset below write it; the camera reads it (0x572a50).
struct CameraViewShakeCurve {
    float *points0;                      // (x, y) pairs
    int count4;
    // 0x5722a0 (thiscall, `ret 4`): piecewise linear value, 0 outside [first x, last x).
    float Lookup(float input);
};

struct CameraViewShakeState {
    float vector0[3];
    char unidentifiedC[4];
    CameraViewShakeCurve *curve10;
    float time14;
    float duration18;                    // 0.6 for a tick request, 1.0 after the reset
    float value1C;                       // frequency: 17.0 (0x79dc68)
    // 0x572810 (thiscall, `ret 0xc`): the vector negated, the curve 0x7fb7a0 at time 0, the
    // duration and the frequency. CameraCollisionShake.cpp.
    void Start(float *vector, float duration, float frequency);
    // 0x572860 (thiscall, `ret 4`): the vector times the curve value at time14, zero when
    // value1C * time14 truncates to an odd number.
    void Evaluate(float *output);
};
UG2_CHECK_SIZE(CameraViewShakeState, 0x20);
UG2_CHECK_OFFSET(CameraViewShakeState, curve10, 0x10);
UG2_CHECK_OFFSET(CameraViewShakeState, duration18, 0x18);

// 0x572a50 (cdecl): view 1 and 3 use slot 0 of the states, view 2 slot 1; right-multiplies
// `matrix` by a translation and three rotations built from the state's vector. Gated on the car
// list field *(float *)(*0x890080 + 0x108).
void CameraViewShake(int view, WheelMatrix *matrix);
// 0x5728c0 (cdecl): starts slot 0 when the request flag 0x8650c4 is set, then advances both
// slots' curves by the frame time. CameraCollisionShake.cpp.
void CameraViewShakeTick();
// 0x5729e0 (cdecl): clears both slots (duration 1.0, frequency 17.0). CameraCollisionShake.cpp.
void CameraViewShakeReset();
// 0x572e50 (cdecl): the collision shake for `view` from the car acceleration. CameraCollisionShake.cpp.
void CameraCollisionShake(int view, WheelVector *acceleration);

// Sample curve of 16-byte keys used by the drive camera shake (0x446d00); the same layout as
// CameraDynamicsCurve with keys of four floats.
struct CameraShakeCurve {
    int count0;
    float start4;
    float spacing8;
    float inverseSpacingC;
    float *keys10;
    // 0x446d00 (thiscall, `ret 8`): linear blend of the two keys around (x - start) / spacing,
    // clamped to the first and last key.
    void Sample(float *output, float x);
};
UG2_CHECK_SIZE(CameraShakeCurve, 0x14);

// 0x5766e0 (`ret 0x10`, ECX unused, so stdcall): output = first * t + (1 - t) * second over four
// floats.
void __stdcall CameraBlend4(float *output, float *first, float *second, float t);
// 0x43d100 (cdecl): output = base + direction * scale over four floats.
void CameraScaleAdd4(float *output, float *base, float *direction, float scale);

#endif
