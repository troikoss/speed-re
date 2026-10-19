#include "DriveCamera.h"
#include "CameraPush.h"
#include "CameraMatrix.h"
#include "../World/WCollision.h"
#include "../Physics/VehicleMover.h"

extern float cameraCarHalf787088;           // 0.5, also the margin of the cockpit car test
extern float cameraCockpitReach7843E8;      // 5.0, added to the margin before the squared distance test
extern float cameraBlockedRayLength78715C;  // 2.0, length of the ray along the car's +0xb0 row
extern char *cameraPlayerWorld890118;       // object whose +0xbd0 holds the player's car holder
extern CameraCarList *carList890080;

// Original 0x5eac70 (thiscall on the player object at mover +0x20): the car at +4, unless the
// byte +0x24a is set and the holder at *0x890118 +0xbd0 is non-null, then that holder's first
// dword.
MoverModeOwner *CameraPlayer::GetCar() {
    char *player = reinterpret_cast<char *>(this);
    unsigned char overridden = player[0x24a];
    MoverModeOwner *car = *reinterpret_cast<MoverModeOwner **>(player + 4);
    if (overridden) {
        MoverModeOwner **holder = *reinterpret_cast<MoverModeOwner ***>(cameraPlayerWorld890118 + 0xbd0);
        if (holder) {
            car = *holder;
        }
    }
    return car;
}

// Original 0x5f14f0 (thiscall, `ret 0xc`; `this` is unused, the list is read from *0x890080 on
// every iteration): for every car of the world list other than `skip` whose byte +0x550 is set and
// whose origin (+0x50) has a squared distance to `point` below margin + 5.0 (0x7843e8), put the
// point into the car's frame (0x401340 from matrix +0x90 and origin, inverted in place with
// 0x5baf90, then 0x43c9c0) and test it against the car body's box (+0x3c, corners +0xe0 and +0xf0)
// widened by `margin` with 0x43d7a0. True on the first car that contains it. The squared distance
// adds the x and z products first, then y, unrounded, and is compared unrounded.
bool CameraCarList::AnyCarContains(float *point, float margin, MoverModeOwner *skip) {
    for (int i = 0; i < carList890080->count10; ++i) {
        MoverModeOwner *car = carList890080->cars1C[i];
        if (car == skip) continue;
        char *bytes = reinterpret_cast<char *>(car);
        if (!bytes[0x550]) continue;
        float *origin = reinterpret_cast<float *>(bytes + 0x50);
        float dx = point[0] - origin[0];
        float dy = point[1] - origin[1];
        float dz = point[2] - origin[2];
        if (!(dx * dx + dz * dz + dy * dy < margin + cameraCockpitReach7843E8)) continue;
        float *body = reinterpret_cast<float *>(reinterpret_cast<char *>(car->collisionBody3C));
        float minimum[3], maximum[3];
        minimum[0] = body[0xe0 / 4];
        minimum[1] = body[0xe4 / 4];
        minimum[2] = body[0xe8 / 4];
        maximum[0] = body[0xf0 / 4];
        maximum[1] = body[0xf4 / 4];
        maximum[2] = body[0xf8 / 4];
        WheelMatrix frame;
        WheelFrameAt(&frame, reinterpret_cast<WheelMatrix *>(bytes + 0x90), reinterpret_cast<WheelVector *>(origin));
        CameraInvertRigid(&frame, &frame);
        WheelVector local;
        WheelTransform(&local, &frame, reinterpret_cast<WheelVector *>(point));
        if (Bounds3Contains(minimum, maximum, &local.x, margin)) {
            return true;
        }
    }
    return false;
}

// Original 0x445a90 (drive camera vtable slot 0x24): the setting for previousType4C (else the
// one for type 0) is a chase setting, type 1, 0 or 5.
bool DriveCameraMover::UsesChaseSetting() {
    const CameraSetting *setting = settings->FindOrDefault(previousType4C, viewState);
    short type = setting->type0;
    return type == 1 || type == 0 || type == 5;
}

// Original 0x445fc0 (thiscall, no arguments): the camera position the setting for type48 gives
// (0x445eb0: forward, lateral, height), moved through the car frame, is inside another car's
// box (0x5f14f0, margin 0.5 from 0x787088, skipping the mover's own car from slot 0x44).
bool DriveCameraMover::CarsNearCockpit() {
    const CameraSetting *setting = settings->FindOrDefault(type48, viewState);
    float offset[3];
    unsigned short angle;
    ReadSetting(offset, &angle, setting);
    char *car = reinterpret_cast<char *>(car2C);
    WheelMatrix frame;
    WheelFrameAt(&frame, reinterpret_cast<WheelMatrix *>(car + 0x90), reinterpret_cast<WheelVector *>(car + 0x50));
    WheelTransform(reinterpret_cast<WheelVector *>(offset), &frame, reinterpret_cast<WheelVector *>(offset));
    MoverModeOwner *own = reinterpret_cast<MoverModeOwner *(__thiscall *)(DriveCameraMover *)>(vtable0[0x44 / 4])(this);
    return carList890080->AnyCarContains(offset, cameraCarHalf787088, own);
}

// HumanBlocked (0x445ac0, the expression Update inlines at 0x4535a9 and 0x4537c7) is in CameraChase.cpp.

// Original 0x453570..0x4536f4. The hood (4) and bumper (2) types fall back to the far camera (0)
// when the input object's slot 0x54 reports true, when HumanBlocked or 0x445fc0 hold, or when the
// collider's two-sided mode-1 ray (margin 0.5, scale 1) from the camera point to the point 2.0
// (0x78715c) along the car's +0xb0 row hits. The point is car +0x60..+0x68, replaced when the body
// (+0x3c) exists by the body centre (+0x110) through the car matrix (+0x90) plus the origin
// (+0x50), added in x, y, z order. 0x445fc0 is evaluated even when HumanBlocked holds.
int DriveCameraMover::SelectUpdateType() {
    active54 = 0;
    if (player20) {
        car2C = player20->GetCar();
    }
    int type = type48;
    if (type == DRIVE_CAMERA_HOOD || type == DRIVE_CAMERA_BUMPER) {
        bool humanBlocked = HumanBlocked();
        bool carsNear = CarsNearCockpit();
        MoverModeOwner *car = car2C;
        MoverInput *input = car->input30;
        bool blocked;
        if (input && input->GetWheelMode54()) {
            blocked = true;
        } else if (humanBlocked || carsNear) {
            blocked = true;
        } else {
            char *bytes = reinterpret_cast<char *>(car);
            float *centre = reinterpret_cast<float *>(bytes + 0x60);
            float point[3];
            point[0] = centre[0];
            point[1] = centre[1];
            point[2] = centre[2];
            if (car->collisionBody3C) {
                WheelTransform(reinterpret_cast<WheelVector *>(point), reinterpret_cast<WheelMatrix *>(bytes + 0x90),
                               reinterpret_cast<WheelVector *>(reinterpret_cast<char *>(car->collisionBody3C) + 0x110));
                float *origin = reinterpret_cast<float *>(bytes + 0x50);
                point[0] = point[0] + origin[0];
                point[1] = point[1] + origin[1];
                point[2] = point[2] + origin[2];
            }
            WheelVector end;
            WheelScaleAdd(&end, reinterpret_cast<WheelVector *>(point), reinterpret_cast<WheelVector *>(bytes + 0xb0),
                          cameraBlockedRayLength78715C);
            blocked = car->Collider()->Ray(1, true, point, &end.x, 0.5f, 1.0f);
        }
        if (blocked) {
            type = 0;
            snap52 = snap52 | (previousType4C != 0);
        } else {
            snap52 = snap52 | (previousType4C == 0);
        }
    }
    return type;
}
