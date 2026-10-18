#include "FEngInterpolation.h"

extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
float EngineSine(float angle); // 0x43c350
unsigned short HudArcSineUnits(float ratio); // 0x43c490
extern float fengZero7D7D3C, fengSlerpLimit79B158, fengAngleUnit784240, fengOne784250, fengHalf784260, fengEpsilon7845A0;

// Original 0x5052b0: first key at or after time, else the last; null without keys.
FEngScriptKey *FEScriptTrack::FindKey(int time) {
    FEngScriptKey *key = (FEngScriptKey *)(value2C ? ((FEngReference *)value30)->Resolve() : value30);
    if (!key) return 0;
    while (key->next4 && key->timeC < time) key = key->next4;
    return key;
}

// Original 0x505260: as 0x5052b0, but time <= -1 or no keys yields the inline base key.
FEngScriptKey *FEScriptTrack::FindKeyOrBase(int time) {
    if (time <= -1) return (FEngScriptKey *)&key8;
    FEngScriptKey *key = (FEngScriptKey *)(value2C ? ((FEngReference *)value30)->Resolve() : value30);
    if (!key) return (FEngScriptKey *)&key8;
    while (key->next4 && key->timeC < time) key = key->next4;
    return key;
}

// Original 0x4fe8d0.
FEngScriptKey *FEScriptTrack::FirstKey() {
    return (FEngScriptKey *)(value2C ? ((FEngReference *)value30)->Resolve() : value30);
}

// Original 0x4955f0.
FEngQuaternion *FEngQuaternion::Assign(const FEngQuaternion *other) {
    value0[0] = other->value0[0]; value0[1] = other->value0[1];
    value0[2] = other->value0[2]; value0[3] = other->value0[3];
    return this;
}

// Original 0x5050d0: wrapping integer delta loaded exactly (FILD), +0.5, truncating conversion.
void __cdecl FEngLerpInteger(int a, int b, float fraction, const int *base, int *out) {
    int delta = (int)((unsigned int)b - (unsigned int)a);
    // MSVC's __ftol2 truncates to 64 bits and the caller keeps EAX; spell that out for other compilers.
    unsigned int rounded = (unsigned int)(long long)((double)delta * fraction + fengHalf784260);
    *out = (int)(rounded + (unsigned int)*base + (unsigned int)a);
}

// Original 0x505110: same component order as 0x505150.
void __cdecl FEngLerpVector2(const float *a, const float *b, float fraction, const float *base, float *out) {
    out[0] = (b[0] - a[0]) * fraction + a[0] + base[0];
    out[1] = (b[1] - a[1]) * fraction + base[1] + a[1];
}

FEngQuaternion *__cdecl FEngQuaternionSubtract(FEngQuaternion *out, const FEngQuaternion *a, const FEngQuaternion *b) {
    float x = a->value0[0] - b->value0[0], y = a->value0[1] - b->value0[1];
    float z = a->value0[2] - b->value0[2], w = a->value0[3] - b->value0[3];
    out->value0[0] = x; out->value0[1] = y; out->value0[2] = z; out->value0[3] = w;
    return out;
}
FEngQuaternion *__cdecl FEngQuaternionScale(FEngQuaternion *out, const FEngQuaternion *q, float scale) {
    float x = scale * q->value0[0], y = scale * q->value0[1];
    float z = scale * q->value0[2], w = scale * q->value0[3];
    out->value0[0] = x; out->value0[1] = y; out->value0[2] = z; out->value0[3] = w;
    return out;
}
FEngQuaternion *__cdecl FEngQuaternionAdd(FEngQuaternion *out, const FEngQuaternion *a, const FEngQuaternion *b) {
    float x = a->value0[0] + b->value0[0], y = a->value0[1] + b->value0[1];
    float z = a->value0[2] + b->value0[2], w = a->value0[3] + b->value0[3];
    out->value0[0] = x; out->value0[1] = y; out->value0[2] = z; out->value0[3] = w;
    return out;
}

// Original 0x511c80: unrounded length; an inverse not above epsilon (or NaN) yields identity.
void __cdecl FEngQuaternionNormalize(FEngQuaternion *q) {
    double x = q->value0[0], y = q->value0[1], z = q->value0[2], w = q->value0[3];
    double inverse = fengOne784250 / sqrt(x * x + y * y + z * z + w * w);
    if (inverse > fengEpsilon7845A0) {
        q->value0[0] = (float)(inverse * q->value0[0]);
        q->value0[1] = (float)(inverse * q->value0[1]);
        q->value0[2] = (float)(inverse * q->value0[2]);
        q->value0[3] = (float)(inverse * q->value0[3]);
    } else {
        q->value0[2] = 0; q->value0[1] = 0; q->value0[0] = 0; q->value0[3] = 1.0f;
    }
}

// Original 0x519b00. The unrounded dot product selects the path; only the sine path
// uses the sign-flipped copy of b. Result = interpolated.Multiply(base).
void __cdecl FEngSlerpQuaternion(const FEngQuaternion *a, const FEngQuaternion *b, float fraction,
                                 const FEngQuaternion *base, FEngQuaternion *out) {
    FEngQuaternion q = *b;
    double dot = (double)q.value0[3] * a->value0[3] + (double)q.value0[2] * a->value0[2] +
                 (double)q.value0[1] * a->value0[1] + (double)q.value0[0] * a->value0[0];
    float cosine = (float)dot;
    if (dot < fengZero7D7D3C) {
        dot = -dot;
        cosine = (float)dot;
        q.value0[0] = -q.value0[0]; q.value0[1] = -q.value0[1];
        q.value0[2] = -q.value0[2]; q.value0[3] = -q.value0[3];
    }
    FEngQuaternion result;
    if (dot < fengSlerpLimit79B158) {
        int units = (unsigned short)(0x4000 - HudArcSineUnits(cosine));
        float theta = (float)(units * fengAngleUnit784240);
        float sine = EngineSine(theta);
        float sineT = EngineSine(theta * fraction);
        float sineU = EngineSine((fengOne784250 - fraction) * theta);
        // Volatile locals keep the original's float spills; only u0 stays unrounded.
        volatile float r0 = sineT * q.value0[0], r1 = sineT * q.value0[1];
        volatile float r2 = sineT * q.value0[2], r3 = sineT * q.value0[3];
        FEngQuaternion scaled;
        FEngQuaternion *s = FEngQuaternionScale(&scaled, a, sineU);
        double u0 = (double)r0 + s->value0[0];
        volatile float u1 = r1 + s->value0[1], u2 = r2 + s->value0[2], u3 = r3 + s->value0[3];
        volatile float inverse = fengOne784250 / sine;
        result.value0[2] = u2 * inverse;
        result.value0[3] = u3 * inverse;
        result.value0[0] = (float)(u0 * inverse);
        result.value0[1] = u1 * inverse;
    } else {
        FEngQuaternion difference, step, sum;
        FEngQuaternionAdd(&sum, a, FEngQuaternionScale(&step, FEngQuaternionSubtract(&difference, b, a), fraction));
        FEngQuaternionNormalize(&sum);
        result = sum;
    }
    FEngQuaternion product;
    *out = *result.Multiply(&product, base);
}
