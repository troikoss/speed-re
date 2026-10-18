#include "WCollision.h"

extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)

extern float fengZero7D7D3C, fengOne784250;  // 0.0f and 1.0f in .rdata
extern float rayMinusOne7842A0;              // -1.0f
void WheelSetLength(WheelVector *output, WheelVector *input, float length);  // 0x43cf90

// Original 0x43d540: 3D bounds of one point.
void Bounds3Set(float *minimum, float *maximum, float *point) {
    float x = point[0], y = point[1], z = point[2];
    minimum[0] = x;
    minimum[1] = y;
    minimum[2] = z;
    maximum[0] = x;
    maximum[1] = y;
    maximum[2] = z;
}

// Original 0x43d570: grow 3D bounds by a point, writing each component in place.
void Bounds3Add(float *minimum, float *maximum, float *point) {
    float x = point[0], y = point[1], z = point[2];
    if (x < minimum[0]) minimum[0] = x;
    if (y < minimum[1]) minimum[1] = y;
    if (z < minimum[2]) minimum[2] = z;
    if (x > maximum[0]) maximum[0] = x;
    if (y > maximum[1]) maximum[1] = y;
    if (z > maximum[2]) maximum[2] = z;
}

// Original 0x43d610: grow 3D bounds by a sphere; all six extents are read first.
void Bounds3AddSphere(float *minimum, float *maximum, float *center, float radius) {
    float lowX = center[0] - radius;
    float lowY = center[1] - radius;
    float lowZ = center[2] - radius;
    float highX = radius + center[0];
    float highY = radius + center[1];
    float highZ = radius + center[2];
    if (lowX < minimum[0]) minimum[0] = lowX;
    if (lowY < minimum[1]) minimum[1] = lowY;
    if (lowZ < minimum[2]) minimum[2] = lowZ;
    if (highX > maximum[0]) maximum[0] = highX;
    if (highY > maximum[1]) maximum[1] = highY;
    if (highZ > maximum[2]) maximum[2] = highZ;
}

// Original 0x43d820: strict 3D overlap; touching or unordered bounds do not overlap.
int Bounds3Overlap(float *minimumA, float *maximumA, float *minimumB, float *maximumB) {
    if (!(minimumB[0] < maximumA[0])) return 0;
    if (!(maximumB[0] > minimumA[0])) return 0;
    if (!(minimumB[1] < maximumA[1])) return 0;
    if (!(maximumB[1] > minimumA[1])) return 0;
    if (!(minimumB[2] < maximumA[2])) return 0;
    if (!(maximumB[2] > minimumA[2])) return 0;
    return 1;
}

// Original 0x5d35b0: the point lies inside the prism over the face when no edge cross
// product points against the normal (a NaN dot passes); `distance` receives the signed
// distance of the point below the plane.
bool WCollisionFace::ContainsPoint(float *point, float *distance) {
    float relative[4][4];
    int count = (signed char)vertexCount9;
    for (int i = 0; i < count; ++i) {
        relative[i][0] = vertices30[i][0] - point[0];
        relative[i][1] = vertices30[i][1] - point[1];
        relative[i][2] = vertices30[i][2] - point[2];
    }
    WheelVector edges[4];
    WheelCross(&edges[0], (WheelVector *)relative[0], (WheelVector *)relative[1]);
    WheelCross(&edges[1], (WheelVector *)relative[1], (WheelVector *)relative[2]);
    if ((signed char)vertexCount9 == 4) {
        WheelCross(&edges[2], (WheelVector *)relative[2], (WheelVector *)relative[3]);
        WheelCross(&edges[3], (WheelVector *)relative[3], (WheelVector *)relative[0]);
    } else {
        WheelCross(&edges[2], (WheelVector *)relative[2], (WheelVector *)relative[0]);
    }
    count = (signed char)vertexCount9;
    for (int i = 0; i < count; ++i) {
        if (edges[i].z * normal70.z + edges[i].x * normal70.x + normal70.y * edges[i].y < fengZero7D7D3C) return false;
    }
    if (distance) {
        *distance = -(relative[0][1] * normal70.y + relative[0][2] * normal70.z + relative[0][0] * normal70.x);
    }
    return true;
}

// Original 0x5d36f0: where the segment crosses the face plane (strictly opposite sides),
// if that point lies inside the face. The side test uses the unrounded end distance; the
// interpolation fraction uses it rounded to binary32.
bool WCollisionFace::SegmentHit(float *from, float *to, float *hit) {
    float *vertex = vertices30[0];
    float ax = from[0] - vertex[0];
    float ay = from[1] - vertex[1];
    float az = from[2] - vertex[2];
    volatile float bx = to[0] - vertex[0];
    volatile float by = to[1] - vertex[1];
    volatile float bz = to[2] - vertex[2];
    float fromDistance = az * normal70.z + ay * normal70.y + ax * normal70.x;
    float toDistance = by * normal70.y + bz * normal70.z + bx * normal70.x;
    volatile float storedTo = toDistance;
    if (!(toDistance * fromDistance < fengZero7D7D3C)) return false;
    WheelVector direction;
    direction.x = to[0] - from[0];
    direction.y = to[1] - from[1];
    direction.z = to[2] - from[2];
    double fromAbsolute = fabs(fromDistance);
    double fraction = fromAbsolute / (fabs(storedTo) + fromAbsolute);
    WheelVector point;
    WheelScaleAdd(&point, (WheelVector *)from, &direction, (float)fraction);
    if (!ContainsPoint(&point.x, 0)) return false;
    if (hit) {
        hit[0] = point.x;
        hit[1] = point.y;
        hit[2] = point.z;
    }
    return true;
}

// Original 0x5d37f0: segment hit nearer than *best, after a face-bounds overlap test
// against the ray bounds; one-sided faces only count when the ray runs against the normal.
bool WCollisionFace::RayHit(float *best, float *from, float *to, float *rayMinimum, float *rayMaximum,
                            bool twoSided, float *hit) {
    float faceMinimum[3], faceMaximum[3];
    Bounds3Set(faceMinimum, faceMaximum, vertices30[0]);
    for (int i = 1; i < (signed char)vertexCount9; ++i) Bounds3Add(faceMinimum, faceMaximum, vertices30[i]);
    if (!Bounds3Overlap(faceMinimum, faceMaximum, rayMinimum, rayMaximum)) return false;
    float point[3];
    if (!SegmentHit(from, to, point)) return false;
    float dx = point[0] - from[0];
    float dy = point[1] - from[1];
    float dz = point[2] - from[2];
    if (!twoSided && !(dy * normal70.y + dz * normal70.z + dx * normal70.x < fengZero7D7D3C)) return false;
    double length = sqrt(dy * dy + dx * dx + dz * dz);
    if (!(length < *best)) return false;
    *best = (float)length;
    if (hit) {
        hit[0] = point[0];
        hit[1] = point[1];
        hit[2] = point[2];
    }
    return true;
}

// Original 0x5d39f0: cast from `from` to `to` against the collider's face area. On a hit
// nearer than the segment length, mode 1 pulls `to` back along the ray to `margin` before
// the face; mode 0 does the same with a height lift of `scale` at the face for faces with
// flag 2, else sets `to`'s height on the face plane offset by `margin` (down with flag 8).
bool WCollider::Ray(int mode, bool twoSided, float *from, float *to, float margin, float scale) {
    float rayMinimum[3], rayMaximum[3];
    Bounds3Set(rayMinimum, rayMaximum, from);
    Bounds3AddSphere(rayMinimum, rayMaximum, to, margin);
    float best = 999999.0f;
    WCollisionFace *hitFace = 0;
    for (int i = 0; i < areaCount2E0; ++i) {
        WCollisionFace *face = areaFaces3C[i];
        if (face->RayHit(&best, from, to, rayMinimum, rayMaximum, twoSided, 0)) hitFace = face;
    }
    WheelVector direction;
    direction.x = to[0] - from[0];
    direction.y = to[1] - from[1];
    direction.z = to[2] - from[2];
    const volatile WheelVector *stored = &direction;  // the length squares the stored binary32 components
    float sx = stored->x, sy = stored->y, sz = stored->z;
    volatile float length = (float)sqrt(sy * sy + sx * sx + sz * sz);
    if (!(best < length)) return false;
    switch (mode) {
    case 1: {
        WheelSetLength(&direction, &direction, best - margin);
        float x = direction.x + from[0];
        float y = direction.y + from[1];
        float z = direction.z + from[2];
        to[0] = x;
        to[2] = z;
        to[1] = y;
        return true;
    }
    case 0:
        if (hitFace->flagsA & 2) {
            volatile float travel = best - margin;
            WheelSetLength(&direction, &direction, travel);
            float lift = (fengOne784250 - travel / length) * scale + direction.z;
            float x = direction.x + from[0];
            float y = direction.y + from[1];
            float z = lift + from[2];
            to[0] = x;
            to[2] = z;
            to[1] = y;
            return true;
        } else {
            WheelVector *normal = &hitFace->normal70;
            float height = (normal->y * to[1] + normal->x * to[0] - hitFace->distance7C) * (rayMinusOne7842A0 / normal->z);
            if (hitFace->flagsA & 8) to[2] = height - margin;
            else to[2] = height + margin;
            return true;
        }
    }
    return true;
}
