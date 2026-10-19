#include "Camera.h"
#include "DriveCamera.h"
#include "../Frontend/PauseOptions.h"
#include "../Hud/HudPlayerMasks.h"

// Original globals (bound to the mapped retail image by OriginalSymbols.ld).
extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
extern float fengZero7D7D3C;                              // 0.0 in .rdata
extern float frameDuration865154;                         // the frame time of the game loop
extern float cameraNegativeOne7842A0;                     // -1.0
extern int cameraViewShakeRequest8650C4;                  // set: slot 0 starts on the next tick
extern float cameraViewShakeStart7FB730[3];               // (0.0, 0.0, 0.3): vector of a request
extern CameraViewShakeCurve cameraViewShakeCurve7FB7A0;   // the five-key shake curve
extern float cameraViewShakeDuration79DC64;               // 0.6: duration of a request
extern float cameraViewShakeFrequency79DC68;              // 17.0: frequency of the shakes
extern float cameraCollisionMagnitudeLow79DCD4;           // 250.0: no shake below this speed
extern float cameraCollisionMagnitudeHigh79DCD8;          // 600.0: magnitude clamp
extern float cameraCollisionAmplitudeLow79DCDC;           // 0.08
extern float cameraCollisionAmplitudeHigh79DCE0;          // 0.16
extern float cameraCollisionDurationLow79DCE4;            // 0.3
extern float cameraCollisionDurationHigh79DCE8;           // 0.7
extern float cameraCollisionScaleX7FB750;                 // 0.7
extern float cameraCollisionScaleY7FB754;                 // 0.5
extern float cameraCollisionScaleZ7FB758;                 // 2.0
extern float cameraCollisionAmplitudeScale7850EC;         // 0.8 when the player is in state 7
extern float cameraCollisionDurationScale78A0EC;          // 1.2 when the player is in state 7
extern CameraViewShakeState cameraViewShake865360[2];
extern PlayerRecord *players8900AC[2];
extern PlayerSettings83AA64 playerSettings83AA64[2];

// Original 0x43cf90 (cdecl): rescales `input` to `length` into `output`; recovered in
// PhysicsMoverController.cpp.
void WheelSetLength(WheelVector *output, WheelVector *input, float length);

// Original 0x572810 (thiscall, `ret 0xc`). The vector is negated into the state; the curve
// 0x7fb7a0 starts at time 0 with the given duration and frequency.
void CameraViewShakeState::Start(float *vector, float duration, float frequency) {
    float x = vector[0] * cameraNegativeOne7842A0;
    float y = vector[1] * cameraNegativeOne7842A0;
    float z = vector[2] * cameraNegativeOne7842A0;
    vector0[0] = x;
    curve10 = &cameraViewShakeCurve7FB7A0;
    time14 = 0.0f;
    duration18 = duration;
    value1C = frequency;
    vector0[1] = y;
    vector0[2] = z;
}

// The per-slot advance in 0x5728c0 (0x572940 for slot 0, 0x57297f for slot 1). The time moves
// by the frame time over the duration. A time below the first key, or at or above the last key,
// drops the curve; an unordered (NaN) time keeps it.
static void AdvanceViewShake(CameraViewShakeState *state) {
    CameraViewShakeCurve *curve = state->curve10;
    if (!curve) {
        return;
    }
    float time = frameDuration865154 / state->duration18 + state->time14;
    state->time14 = time;
    float *points = curve->points0;
    if (time < points[0] || time >= points[curve->count4 * 2 - 2]) {
        state->curve10 = 0;
    }
}

// Original 0x5728c0 (cdecl). A pending request (0x8650c4) starts slot 0 inline with the vector
// (0x7fb730..0x7fb738) negated, the curve 0x7fb7a0 and the constants 0x79dc64 and 0x79dc68; the
// original does not call Start for this. Both slots then advance.
void CameraViewShakeTick() {
    if (cameraViewShakeRequest8650C4) {
        CameraViewShakeState *state = &cameraViewShake865360[0];
        cameraViewShakeRequest8650C4 = 0;
        state->vector0[0] = cameraViewShakeStart7FB730[0] * cameraNegativeOne7842A0;
        state->curve10 = &cameraViewShakeCurve7FB7A0;
        state->time14 = 0.0f;
        state->duration18 = cameraViewShakeDuration79DC64;
        state->value1C = cameraViewShakeFrequency79DC68;
        state->vector0[1] = cameraViewShakeStart7FB730[1] * cameraNegativeOne7842A0;
        state->vector0[2] = cameraViewShakeStart7FB730[2] * cameraNegativeOne7842A0;
    }
    AdvanceViewShake(&cameraViewShake865360[0]);
    AdvanceViewShake(&cameraViewShake865360[1]);
}

// Original 0x5729e0 (cdecl). Both slots lose their vector, curve and time, with duration 1.0 and
// the frequency 0x79dc68. The padding bytes at +0xc are not written. The world constructor
// 0x5ff490 calls this.
void CameraViewShakeReset() {
    for (int slot = 0; slot < 2; ++slot) {
        CameraViewShakeState *state = &cameraViewShake865360[slot];
        state->vector0[0] = 0.0f;
        state->vector0[1] = 0.0f;
        state->vector0[2] = 0.0f;
        state->curve10 = 0;
        state->time14 = 0.0f;
        state->duration18 = 1.0f;
        state->value1C = cameraViewShakeFrequency79DC68;
    }
}

// Original 0x5ea7c0 (thiscall on the player record). True when the chase camera (+0x238) is kind
// 1 and active, and the profile's drive camera option (0x83aa64 row, +0x10) maps to the driver
// camera (type 3) through 0x502500. No option maps to type 3, so this is false for every input.
bool PlayerRecord::QueryDriverCamera() {
    CameraMover *camera = camera238;
    if (!camera || camera->kindC != 1 || camera->active14 == 0) {
        return false;
    }
    return DriveCameraTypeForOption(playerSettings83AA64[index10].value10) == DRIVE_CAMERA_DRIVER;
}

// Original 0x572e50 (cdecl). Unless the player's camera is the driver camera, the acceleration
// (value40 of the car) is scaled per axis to (0.7 x, 0.5 y, 2.0 z). Its magnitude m gives the
// shake: nothing below 250 (NaN falls through to the amplitude test), m clamped to 600, and
// t = (m - 250) / 350. The amplitude is 0.08 + 0.08 t and the duration 0.3 + 0.4 t; a player in
// state 7 scales them by 0.8 and 1.2. The shake starts on the view's state only when the
// amplitude is above the value of the curve still playing there (0 without a curve).
void CameraCollisionShake(int view, WheelVector *acceleration) {
    PlayerRecord *player = players8900AC[view];
    if (player->QueryDriverCamera()) {
        return;
    }
    // The scaled components are rounded to binary32 when they are stored (0x572e7d..0x572e97)
    // and the squares reload them, except the z product, which keeps its unrounded factor.
    volatile float scaled[3];
    scaled[0] = cameraCollisionScaleX7FB750 * acceleration->x;
    scaled[1] = cameraCollisionScaleY7FB754 * acceleration->y;
    scaled[2] = cameraCollisionScaleZ7FB758 * acceleration->z;
    double magnitude = sqrt((cameraCollisionScaleZ7FB758 * acceleration->z) * scaled[2] + scaled[0] * scaled[0] +
                            scaled[1] * scaled[1]);
    if (magnitude < cameraCollisionMagnitudeLow79DCD4) {
        return;
    }
    if (magnitude > cameraCollisionMagnitudeHigh79DCD8) {
        magnitude = cameraCollisionMagnitudeHigh79DCD8;
    }
    volatile float t = (magnitude - cameraCollisionMagnitudeLow79DCD4) /
                       (cameraCollisionMagnitudeHigh79DCD8 - cameraCollisionMagnitudeLow79DCD4);
    volatile float amplitude = (cameraCollisionAmplitudeHigh79DCE0 - cameraCollisionAmplitudeLow79DCDC) * t +
                               cameraCollisionAmplitudeLow79DCDC;
    CameraViewShakeState *state = &cameraViewShake865360[view];
    float current = fengZero7D7D3C;
    if (state->curve10) {
        current = state->curve10->Lookup(state->time14);
    }
    if (!(amplitude > current)) {
        return;
    }
    volatile float duration = (cameraCollisionDurationHigh79DCE8 - cameraCollisionDurationLow79DCE4) * t +
                              cameraCollisionDurationLow79DCE4;
    if (player && player->state58 == 7) {
        amplitude = amplitude * cameraCollisionAmplitudeScale7850EC;
        duration = duration * cameraCollisionDurationScale78A0EC;
    }
    WheelVector shaken;
    WheelSetLength(&shaken, (WheelVector *)scaled, amplitude);
    state->Start(&shaken.x, duration, cameraViewShakeFrequency79DC68);
}
