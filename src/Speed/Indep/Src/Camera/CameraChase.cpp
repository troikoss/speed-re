#include "DriveCamera.h"
#include "CameraPush.h"
#include "../Physics/VehicleMover.h"

extern float fengZero7D7D3C, fengOne784250;
extern float cameraReflectScale7855D0;   // -2.0, multiplies the negative dot product
extern float cameraHeadingDecay787158;   // 2.0, divides the elapsed ground time
extern float cameraClockScale784268;     // 0.00025, seconds per tick of the clock 0x86518c
extern int cameraClock86518C;            // the tick global; the native demo passes the same one to Update
extern CameraPairCurve cameraHeadingCurve82A4E0;

// MoverModeOwner::State() (car +0x40) written out so the /Ob0 verifier build needs no extra symbol.
#define CarState(car) reinterpret_cast<DrivetrainAuxiliary *>((car)->unidentified40)

typedef bool (__thiscall *DriveCameraChaseQuery)(DriveCameraMover *);

// Original 0x574cd0 (thiscall, `ret 4`): the three floats, pushed z, y, x, to 0x570ba0.
void CameraCubicVector::SetTarget(const float *target) {
    SetTarget(target[0], target[1], target[2]);
}

// Original 0x445ac0 (thiscall, no arguments): slot 0x24, then the car's wheel inputs (0x5ed290),
// wheel targets (0x5ed300), or an unfinished state (car +0x38 set and mode +0x424 at most 2,
// signed). Update inlines the same expression at 0x4535a9 and 0x4537c3.
bool DriveCameraMover::HumanBlocked() {
    if (!reinterpret_cast<DriveCameraChaseQuery>(vtable0[0x24 / 4])(this)) {
        return false;
    }
    MoverModeOwner *car = car2C;
    if (car->HasWheelInputs()) return true;
    if (car->HasWheelTargets()) return true;
    return car->state38 != 0 && car->mode424 <= 2;
}

// Original 0x445c20 (thiscall, `ret 8`): moves the heading follower heading40. For a setting that
// is not a chase type (1, 0 or 5) the target is row 0 of the car matrix (car +0x90) and the
// follower is reset. For a chase type nothing happens when !snap and 0x445ac0 holds; otherwise
// the target is the car velocity (+0x70), reflected through the matrix row 0 when it points
// backwards (dot product accumulated z, y, x; scale-added with dot * -2.0), blended towards that
// row by the curve 0x82a4e0 at the car's +0x430: normalised, scaled by the curve value s, then
// scale-added with row 0 and 1.0 - s. Its z is scaled by 1.0 - elapsed/2.0, the elapsed time
// being (clock 0x86518c - groundTime58) * 0.00025, clamped to 0..1 (not above 0 and NaN give 0).
// With snap set, or for a non-chase setting, each channel gets value = target and
// velocity = target velocity and its follower state is cleared (the stores at 0x445dbb).
void DriveCameraMover::SteerHeading(const CameraSetting *setting, bool snap) {
    short type = setting->type0;
    if (type == 1 || type == 0 || type == 5) {
        if (!snap && HumanBlocked()) {
            return;
        }
        DrivetrainAuxiliary *state = CarState(car2C);
        WheelVector velocity = state->velocity30;
        WheelVector *forward = reinterpret_cast<WheelVector *>(&state->matrix50);
        float *row = state->matrix50.elements;
        float dot = velocity.z * row[2] + velocity.y * row[1] + velocity.x * row[0];
        if (dot < fengZero7D7D3C) {
            WheelScaleAdd(&velocity, &velocity, forward, dot * cameraReflectScale7855D0);
        }
        float s = cameraHeadingCurve82A4E0.Lookup(car2C->speed430);
        WheelNormalize(&velocity, &velocity);
        velocity.x = velocity.x * s;
        velocity.y = velocity.y * s;
        velocity.z = velocity.z * s;
        WheelScaleAdd(&velocity, &velocity, forward, fengOne784250 - s);
        // The original stores the clamp value as binary32 (FST at 0x445d58) and reloads it at 0x445d69
        // and 0x445d80..; MSVC without /Op keeps it unrounded in ST0, so the spill is written as a
        // volatile local. The first compare at 0x445d5c is made on the unrounded ST0, here on the
        // rounded spill; rounding 1 - x to binary32 cannot change its sign or make it zero.
        // This spelling is driven by verification, not by evidence of the original source.
        volatile float t = fengOne784250 - ((float)(cameraClock86518C - groundTime58) * cameraClockScale784268) / cameraHeadingDecay787158;
        if (!(t > fengZero7D7D3C)) {
            t = fengZero7D7D3C;
        } else if (fengOne784250 < t) {
            t = fengOne784250;
        }
        velocity.z = t * velocity.z;
        heading40->SetTarget(&velocity.x);
        if (!snap) {
            return;
        }
    } else {
        heading40->SetTarget(reinterpret_cast<float *>(&CarState(car2C)->matrix50));
    }
    CameraCubicChannel *channels = heading40->channels0;
    for (int i = 0; i < 3; ++i) {
        channels[i].state2C = 0;
        channels[i].velocity4 = channels[i].targetVelocityC;
        channels[i].value0 = channels[i].target8;
    }
}

// Original 0x445e00 (thiscall, `ret 4`). For a chase setting (slot 0x24, 0x445a90) the frame is
// built from the heading follower: identity, row 0 the normalised heading values (channel values
// at +0x00, +0x30, +0x60 of heading40), row 1 = row 2 x row 0, row 2 = row 0 x row 1 (0x43d1e0
// output, a, b), translation the car origin (+0x50), w = 1.0. Otherwise 0x401340 on the car
// matrix (+0x90) and origin.
void DriveCameraMover::GetCarFrame(WheelMatrix *output) {
    if (reinterpret_cast<DriveCameraChaseQuery>(vtable0[0x24 / 4])(this)) {
        WheelIdentity(output);
        WheelVector heading;
        heading.x = heading40->channels0[0].value0;
        heading.y = heading40->channels0[1].value0;
        heading.z = heading40->channels0[2].value0;
        WheelVector *row0 = reinterpret_cast<WheelVector *>(&output->elements[0]);
        WheelVector *row1 = reinterpret_cast<WheelVector *>(&output->elements[4]);
        WheelVector *row2 = reinterpret_cast<WheelVector *>(&output->elements[8]);
        WheelNormalize(row0, &heading);
        WheelCross(row1, row2, row0);
        WheelCross(row2, row0, row1);
        DrivetrainAuxiliary *state = CarState(car2C);
        output->elements[12] = state->origin10.x;
        output->elements[13] = state->origin10.y;
        output->elements[14] = state->origin10.z;
        output->elements[15] = 1.0f;
    } else {
        DrivetrainAuxiliary *state = CarState(car2C);
        WheelFrameAt(output, &state->matrix50, &state->origin10);
    }
}
