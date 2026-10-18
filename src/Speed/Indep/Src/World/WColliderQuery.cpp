#include "WCollision.h"

extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

extern float collisionAreaHalf;            // 0x784260 (0.5)
extern float colliderQuerySpan7A5338;      // largest swept query extent
extern float colliderQueryDecay785510;     // look-ahead scale factor per retry

// Original 0x43d220: empty 2D bounds (minimum +FLT_MAX, maximum -FLT_MAX).
void Bounds2Reset(float *minimum, float *maximum) {
    minimum[0] = 3.40282347e+38f;
    minimum[1] = 3.40282347e+38f;
    maximum[0] = -3.40282347e+38f;
    maximum[1] = -3.40282347e+38f;
}

// Original 0x43d2f0: grow the bounds by a point and radius. Every input is read before
// the four stores; an unordered comparison takes the point's value.
void Bounds2Expand(float *minimum, float *maximum, float *point, float radius) {
    float lowX = point[0] - radius;
    float minX = minimum[0] < lowX ? minimum[0] : lowX;
    float lowY = point[1] - radius;
    float minY = minimum[1] < lowY ? minimum[1] : lowY;
    float highX = radius + point[0];
    float maxX = maximum[0] > highX ? maximum[0] : highX;
    float highY = radius + point[1];
    float maxY = maximum[1] > highY ? maximum[1] : highY;
    minimum[0] = minX;
    minimum[1] = minY;
    maximum[0] = maxX;
    maximum[1] = maxY;
}

// Original 0x43d470: 1 when the inner bounds lie within the outer bounds. Each x87
// comparison fails on an unordered result.
int Bounds2Contains(float *outerMinimum, float *outerMaximum, float *innerMinimum, float *innerMaximum) {
    if (!(innerMinimum[0] >= outerMinimum[0])) return 0;
    if (!(innerMaximum[0] <= outerMaximum[0])) return 0;
    if (!(innerMinimum[1] >= outerMinimum[1])) return 0;
    if (!(innerMaximum[1] <= outerMaximum[1])) return 0;
    return 1;
}

// Original 0x43d090: output = base + direction * scale in 2D (y computed first).
void ScaleAdd2(float *output, float *base, float *direction, float scale) {
    float y = scale * direction[1] + base[1];
    float x = scale * direction[0] + base[0];
    output[0] = x;
    output[1] = y;
}

// Original 0x5e6fa0: the sphere's bounds (+0x1c/+0x24) must lie in the face area
// (+0x2c/+0x34); otherwise the area is rebuilt around the sphere grown by 0.5 and, with a
// velocity, the point moved by 0.15 s of it, retried with a shorter look-ahead (zero on
// the last retry) while the area's diagonal is too large, and its faces are refreshed.
void WCollider::QueryArea(float *point, float radius, float *velocity) {
    UpdatePack(point, 0.0f);
    Bounds2Reset(boundsMinimum1C, boundsMaximum24);
    Bounds2Expand(boundsMinimum1C, boundsMaximum24, point, radius);
    if (Bounds2Contains(areaMinimum2C, areaMaximum34, boundsMinimum1C, boundsMaximum24)) return;
    if (!velocity) {
        Bounds2Reset(areaMinimum2C, areaMaximum34);
        Bounds2Expand(areaMinimum2C, areaMaximum34, point, radius + collisionAreaHalf);
        RefreshArea();
        return;
    }
    float scale = 0.15f;
    radius = radius + collisionAreaHalf;
    for (int attempt = 0;;) {
        Bounds2Reset(areaMinimum2C, areaMaximum34);
        Bounds2Expand(areaMinimum2C, areaMaximum34, point, radius);
        float ahead[2];
        ScaleAdd2(ahead, point, velocity, scale);
        Bounds2Expand(areaMinimum2C, areaMaximum34, ahead, radius);
        float dx = areaMinimum2C[0] - areaMaximum34[0];
        float dy = areaMinimum2C[1] - areaMaximum34[1];
        if (sqrt(dy * dy + dx * dx) < colliderQuerySpan7A5338) break;
        scale = scale * colliderQueryDecay785510;
        if (attempt == 3) scale = 0.0f;
        if (++attempt >= 5) break;
    }
    RefreshArea();
}
