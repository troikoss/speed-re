#include "FEngTrackEvaluate.h"
#include "../Hud/HudAnimation.h"

extern float fengZero7D7D3C, fengOne784250, fengEpsilon7845A0;

static int TrackDuration(const FEScriptTrack *track) { // dword +4, sign-extended 24 bits
    return (int)(*(const unsigned int *)track->unidentified4 << 8) >> 8;
}

// Original 0x52bdc0. Key selection by cycle mode, exact 0/1 fractions select a key
// value plus the base, other fractions use the type's interpolator.
void __cdecl FEngEvaluateTrack(FEScriptTrack *track, int time, void *out) {
    FEngScriptKey *lower = 0, *upper = 0;
    float fraction = 0;
    const unsigned int *base = (const unsigned int *)track->base18;
    FEngScriptKey *key = (FEngScriptKey *)(track->value2C ? ((FEngReference *)track->value30)->Resolve() : track->value30);
    if (key) {
        int count = 0;
        do { key = key->next4; ++count; } while (key);
        if (count) {
            switch (track->flags3 & 0x7f) {
            case 0: // clamp
                upper = track->FindKey(time);
                if (!upper) break;
                lower = upper->previous8;
                if (!lower || time >= upper->timeC) {
                    fraction = 1.0f;
                } else {
                    double span = (int)((unsigned int)upper->timeC - (unsigned int)lower->timeC);
                    if (span <= fengEpsilon7845A0) fraction = 0;
                    else fraction = (float)((int)((unsigned int)time - (unsigned int)lower->timeC) / span);
                }
                break;
            case 1: { // loop over the 24-bit duration
                upper = track->FindKey(time);
                if (!upper) break;
                int upperTime = upper->timeC;
                if (upperTime < time) {
                    lower = upper;
                    upper = track->FirstKey();
                    double span = (int)((unsigned int)TrackDuration(track) + (unsigned int)upper->timeC - (unsigned int)lower->timeC);
                    if (span <= fengEpsilon7845A0) fraction = 0;
                    else fraction = (float)((int)((unsigned int)time - (unsigned int)lower->timeC) / span);
                } else if (upperTime == time) {
                    lower = upper;
                    fraction = 0;
                } else if ((lower = upper->previous8) != 0) {
                    double span = (int)((unsigned int)upperTime - (unsigned int)lower->timeC);
                    if (span <= fengEpsilon7845A0) fraction = 0;
                    else fraction = (float)((int)((unsigned int)time - (unsigned int)lower->timeC) / span);
                } else {
                    int duration = TrackDuration(track);
                    lower = track->FindKeyOrBase(duration);
                    unsigned int offset = (unsigned int)duration - (unsigned int)lower->timeC;
                    double span = (int)(offset + (unsigned int)upperTime);
                    if (span <= fengEpsilon7845A0) fraction = 0;
                    else fraction = (float)((int)(offset + (unsigned int)time) / span);
                }
                break;
            }
            case 2: { // ping-pong; no lower key, so the zero fraction reads null +0x10 as the original does
                int duration = TrackDuration(track);
                int mirrored = time;
                if (time > duration) mirrored = (int)((unsigned int)duration * 2 - (unsigned int)time);
                upper = track->FindKey(mirrored);
                break;
            }
            }
        }
    }
    if (!upper) {
        unsigned int *destination = (unsigned int *)out;
        switch (track->value0) {
        case 1: case 2: destination[0] = base[0]; break;
        case 3: destination[0] = base[0]; destination[1] = base[1]; break;
        case 4: destination[0] = base[0]; destination[1] = base[1]; destination[2] = base[2]; break;
        case 5: case 6:
            destination[0] = base[0]; destination[3] = base[3];
            destination[1] = base[1]; destination[2] = base[2];
            break;
        }
        return;
    }
    if (fraction != fengZero7D7D3C && fraction != fengOne784250) {
        void *a = lower->value10, *b = upper->value10;
        switch (track->value0) {
        case 1: FEngLerpInteger(*(int *)a, *(int *)b, fraction, (const int *)base, (int *)out); break;
        case 2: {
            float l = *(float *)a;
            *(float *)out = (*(float *)b - l) * fraction + l + *(const float *)base;
            break;
        }
        case 3: FEngLerpVector2((const float *)a, (const float *)b, fraction, (const float *)base, (float *)out); break;
        case 4: HudLerpVector3((const float *)a, (const float *)b, fraction, (const float *)base, (float *)out); break;
        case 5: FEngSlerpQuaternion((const FEngQuaternion *)a, (const FEngQuaternion *)b, fraction,
                                    (const FEngQuaternion *)base, (FEngQuaternion *)out); break;
        case 6: HudLerpColour((const int *)a, (const int *)b, fraction, (const int *)base, (int *)out); break;
        }
        return;
    }
    const unsigned int *value = fraction == fengZero7D7D3C ? (const unsigned int *)((char *)lower + 0x10)
                                                            : (const unsigned int *)upper->value10;
    switch (track->value0) {
    case 1: *(unsigned int *)out = base[0] + value[0]; break;
    case 2: *(float *)out = *(const float *)base + *(const float *)value; break;
    case 3:
        ((float *)out)[0] = ((const float *)base)[0] + ((const float *)value)[0];
        ((float *)out)[1] = ((const float *)value)[1] + ((const float *)base)[1];
        break;
    case 4:
        ((float *)out)[0] = ((const float *)base)[0] + ((const float *)value)[0];
        ((float *)out)[1] = ((const float *)value)[1] + ((const float *)base)[1];
        ((float *)out)[2] = ((const float *)value)[2] + ((const float *)base)[2];
        break;
    case 5: {
        FEngQuaternion product;
        ((FEngQuaternion *)out)->Assign(((FEngQuaternion *)value)->Multiply(&product, (const FEngQuaternion *)base));
        break;
    }
    case 6: {
        unsigned int *destination = (unsigned int *)out;
        destination[2] = value[2] + base[2];
        destination[1] = value[1] + base[1];
        destination[0] = base[0] + value[0];
        destination[3] = value[3] + base[3];
        break;
    }
    }
}

// rep movsd/movsb copy of a key's value size.
static void CopyValue(void *out, const void *value, unsigned int size) {
    unsigned int *d = (unsigned int *)out;
    const unsigned int *s = (const unsigned int *)value;
    for (unsigned int i = size >> 2; i; --i) *d++ = *s++;
    unsigned char *db = (unsigned char *)d;
    const unsigned char *sb = (const unsigned char *)s;
    for (unsigned int i = size & 3; i; --i) *db++ = *sb++;
}

// Original 0x519d40: interpolation 0 holds key values. Past the duration it looks up
// duration - time (the base key) and takes its successor while that precedes time.
void __cdecl FEngStepTrack(FEScriptTrack *track, int time, void *out) {
    int duration = TrackDuration(track);
    unsigned int size = track->value1;
    FEngScriptKey *key;
    if (time <= duration) {
        key = track->FindKeyOrBase(time);
        if (key->previous8 && key->timeC > time) { CopyValue(out, key->previous8->value10, size); return; }
    } else {
        key = track->FindKeyOrBase((int)((unsigned int)duration - (unsigned int)time));
        if (key->next4 && key->timeC < time) { CopyValue(out, key->next4->value10, size); return; }
    }
    CopyValue(out, key->value10, size);
}

// Original 0x5354f0: bit 7 of +3 skips evaluation; a keyless track sets it afterwards.
void __cdecl FEngUpdateTrack(FEScriptTrack *track, int time, void *out) {
    if ((signed char)track->flags3 < 0) return;
    switch (track->value2) {
    case 0: FEngStepTrack(track, time, out); break;
    case 1: case 3: FEngEvaluateTrack(track, time, out); break;
    }
    int count = 0;
    FEngScriptKey *key = (FEngScriptKey *)(track->value2C ? ((FEngReference *)track->value30)->Resolve() : track->value30);
    if (key) do { key = key->next4; ++count; } while (key);
    if (!count) track->flags3 |= 0x80;
}

// Original 0x541f60: one script track into the object's state block.
void __cdecl FEngApplyScriptTrack(FEScript *script, unsigned char index, int time, HudVisibilityObject *object) {
    FEScriptTrack *track = static_cast<FEScriptTrack *>(script->tracks20) + index;
    switch (track->value2) {
    case 0: FEngStepTrack(track, time, object->data2C + track->state7); break;
    case 1: case 3: FEngEvaluateTrack(track, time, object->data2C + track->state7); break;
    }
}
