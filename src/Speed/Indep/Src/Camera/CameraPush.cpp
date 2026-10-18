#include "CameraPush.h"
#include "CameraMatrix.h"
#include "../World/WCollision.h"
#include "../Physics/VehicleMover.h"

extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

extern float fengZero7D7D3C, fengOne784250, fengHalf784260;
extern float cameraNearRadius787090;     // 0.7
extern float cameraLookBackRadius787094; // 3.0
extern float cameraCarMargin78708C;      // 1.0, added to the car body's bounding radius
extern float cameraFaceDepth78709C;      // 0.7, face push depth for faces without flag 2
extern float cameraRayMargin7870A0;      // 0.7
extern unsigned char surfaceCameraSkip800EED[]; // per-surface byte in 0x98-byte records
double bDistance(WheelVector *a, WheelVector *b); // 0x43ce10

typedef float (__thiscall *CameraMoverRadius)(CameraMoverBase *);

// Original 0x5ec250: two corners of the car body's box (body +0xe0 and +0xf0).
void MoverModeOwner::GetBodyBox(float *first, float *second) {
    char *body = reinterpret_cast<char *>(collisionBody3C);
    float *a = reinterpret_cast<float *>(body + 0xe0);
    float *b = reinterpret_cast<float *>(body + 0xf0);
    first[0] = a[0];
    first[1] = a[1];
    first[2] = a[2];
    second[0] = b[0];
    second[1] = b[1];
    second[2] = b[2];
}

// Original 0x5ec240: the car body's centre (body +0x110).
float *MoverModeOwner::GetBodyCentre() {
    return reinterpret_cast<float *>(reinterpret_cast<char *>(collisionBody3C) + 0x110);
}

// Original 0x445a60 (drive camera vtable slot 0x30): face push depth.
float CameraMoverBase::NearRadius() {
    if (lookBack51 && player20 && player20->Query5EB520()) return cameraLookBackRadius787094;
    return cameraNearRadius787090;
}

// Original 0x444190: cast from `from` to `point` (mode 1, two-sided), then up to three
// passes push the result out of faces it lies within the push depth of (slot 0x30 for
// faces with flag 2), averaging the normals weighted by the remaining depth. `offset`
// receives the total movement (w = 0); it is left untouched without a pack.
bool CameraMoverBase::PushFromFaces(WCollider *collider, float *from, float *point, float *offset) {
    if (!collider->pack8) return false;
    float end[3];
    end[0] = point[0];
    end[1] = point[1];
    end[2] = point[2];
    bool hit = collider->Ray(1, true, from, end, cameraRayMargin7870A0, 1.0f);
    volatile float moved[3];  // stored binary32 before the passes read it
    moved[0] = end[0] - point[0];
    moved[1] = end[1] - point[1];
    moved[2] = end[2] - point[2];
    WheelVector push;
    push.x = moved[0];
    push.y = moved[1];
    push.z = moved[2];
    for (int pass = 0; pass < 3; ++pass) {
        float probe[3];
        probe[0] = push.x + point[0];
        probe[1] = push.y + point[1];
        probe[2] = push.z + point[2];
        int count = 0;
        WheelVector sum;
        sum.x = 0.0f;
        sum.y = 0.0f;
        sum.z = 0.0f;
        for (int i = 0; i < collider->areaCount2E0; ++i) {
            WCollisionFace *face = collider->areaFaces3C[i];
            if (surfaceCameraSkip800EED[face->surfaceB * 0x98]) continue;
            float depth;
            if (!face->ContainsPoint(probe, &depth)) continue;
            float limit = (face->flagsA & 2) ? reinterpret_cast<CameraMoverRadius>(vtable0[0x30 / 4])(this)
                                             : cameraFaceDepth78709C;
            if (!(depth >= fengZero7D7D3C)) continue;
            if (!(depth < limit)) continue;
            WheelScaleAdd(&sum, &sum, &face->normal70, limit - depth);
            ++count;
        }
        if (count == 0) break;
        WheelScaleAdd(&push, &push, &sum, (float)(fengOne784250 / count));
    }
    offset[0] = push.x;
    offset[1] = push.y;
    offset[2] = push.z;
    offset[3] = 0.0f;
    return hit;
}

// Original 0x444530: push the eye of `view` (its inverse's translation) out of the world
// around the car's focus point (the override, else the body centre in car space or the
// car origin), then keep it outside the car body's bounding sphere and re-aim the view at
// the focus, preserving where the focus projects. Returns the face push's ray hit.
bool CameraMoverBase::PushFromWorld(WheelMatrix *view, MoverModeOwner *car, float *focusOverride) {
    WheelMatrix camera;
    CameraInvertRigid(&camera, view);
    float *eye = &camera.elements[12];
    char *owner = reinterpret_cast<char *>(car);
    float *origin = reinterpret_cast<float *>(owner + 0x50);
    float focus[3];
    if (focusOverride) {
        focus[0] = focusOverride[0];
        focus[1] = focusOverride[1];
        focus[2] = focusOverride[2];
    } else if (car->collisionBody3C) {
        WheelVector offset;
        WheelTransform(&offset, reinterpret_cast<WheelMatrix *>(owner + 0x90),
                       reinterpret_cast<WheelVector *>(reinterpret_cast<char *>(car->collisionBody3C) + 0x110));
        focus[0] = origin[0] + offset.x;
        focus[1] = offset.y + origin[1];
        focus[2] = origin[2] + offset.z;
    } else {
        focus[0] = origin[0];
        focus[1] = origin[1];
        focus[2] = origin[2];
    }
    WheelVector middle;
    middle.x = eye[0] * fengHalf784260;
    middle.y = eye[1] * fengHalf784260;
    middle.z = eye[2] * fengHalf784260;
    WheelScaleAdd(&middle, &middle, reinterpret_cast<WheelVector *>(focus), 0.5f);
    float radius = (float)((bDistance(reinterpret_cast<WheelVector *>(focus), reinterpret_cast<WheelVector *>(eye))
                            + fengOne784250) * fengHalf784260);
    WCollider collider;
    collider.Construct();
    if (car && car->Collider()) {
        WCollider *source = car->Collider();
        int count = source->groupCount2F0;
        collider.groupCount2F0 = count;
        for (int i = 0; i < count; ++i) collider.groups2F4[i] = source->groups2F4[i];
    }
    collider.QueryArea(&middle.x, radius, 0);
    float *m = view->elements;
    volatile float x = focus[1] * m[4] + focus[2] * m[8] + focus[0] * m[0] + m[12];
    float y = focus[1] * m[5] + focus[0] * m[1] + focus[2] * m[9] + m[13];
    volatile float depth = focus[1] * m[6] + focus[0] * m[2] + focus[2] * m[10] + m[14];
    float push[4];
    push[0] = 0.0f;
    push[1] = 0.0f;
    push[2] = 0.0f;
    volatile float inverse = fengOne784250 / depth;
    float projected[2];
    projected[1] = y * inverse;
    projected[0] = inverse * x;
    bool hit = PushFromFaces(&collider, focus, eye, push);
    eye[0] = eye[0] + push[0];
    eye[1] = eye[1] + push[1];
    eye[2] = eye[2] + push[2];
    if (car) {
        const volatile float *storedEye = eye;  // the moved eye is reloaded as stored
        float ax = storedEye[0] - focus[0];
        float ay = storedEye[1] - focus[1];
        volatile float az = storedEye[2] - focus[2];
        volatile float ay2 = ay * ay;
        volatile float ax2 = ax * ax;
        float storedZ = az, storedX2 = ax2, storedY2 = ay2;  // reloaded binary32 values
        volatile float distance = (float)sqrt(storedZ * storedZ + storedX2 + storedY2);
        float corner[3], extent[3];
        car->GetBodyBox(corner, extent);
        float *centre = car->GetBodyCentre();
        volatile float ex = extent[0] - centre[0];
        float ey = extent[1] - centre[1];
        float ez = extent[2] - centre[2];
        float storedEx = ex;
        double reach = sqrt(ez * ez + ey * ey + storedEx * storedEx) + cameraCarMargin78708C;
        if (distance < reach) {
            eye[2] = (float)(sqrt(reach * reach - (storedX2 + storedY2)) + focus[2]);
        }
    }
    CameraReaim(view, reinterpret_cast<WheelVector *>(eye), reinterpret_cast<WheelVector *>(focus), projected);
    collider.Destruct();
    return hit;
}
