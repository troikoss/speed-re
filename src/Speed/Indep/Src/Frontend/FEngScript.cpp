#include "FEngScript.h"
#include "../../Libs/bWare/Strings.h"

// Original 0x4fe890: follow references until a direct head pointer is reached.
void *FEngReference::Resolve() {
    FEngReference *reference = this;
    while (reference->value4) reference = (FEngReference *)reference->value8;
    return reference->value8;
}

// Original 0x505580. Cache all inputs before writes; subtraction wraps modulo 2^32.
FEngInteger4 *FEngInteger4::Subtract(FEngInteger4 *out, const FEngInteger4 *other) {
    unsigned int a0 = value0[0], a1 = value0[1], a2 = value0[2], a3 = value0[3];
    unsigned int b0 = other->value0[0], b1 = other->value0[1], b2 = other->value0[2], b3 = other->value0[3];
    out->value0[0] = a0 - b0; out->value0[1] = a1 - b1;
    out->value0[2] = a2 - b2; out->value0[3] = a3 - b3;
    return out;
}

// Original 0x4fe6b0: retain the observed float rounding of the third cross component.
FEngQuaternion *FEngQuaternion::Multiply(FEngQuaternion *out, const FEngQuaternion *other) {
    double a0 = value0[0], a1 = value0[1], a2 = value0[2], a3 = value0[3];
    double b0 = other->value0[0], b1 = other->value0[1], b2 = other->value0[2], b3 = other->value0[3];
    volatile float crossZ = (float)(b1 * a0 - a1 * b0);
    float x = (float)((a3 * b0 + b3 * a0) + (a1 * b2 - b1 * a2));
    float y = (float)((b3 * a1 + b1 * a3) + (b0 * a2 - a0 * b2));
    float z = (float)((a3 * b2 + b3 * a2) + crossZ);
    float w = (float)(b3 * a3 - ((b0 * a0 + b1 * a1) + b2 * a2));
    out->value0[0] = x; out->value0[1] = y; out->value0[2] = z; out->value0[3] = w;
    return out;
}

// Original 0x517720. Snapshot track count/table; live state is reloaded for each track.
void HudVisibilityObject::InitializeScript() {
    FEScriptTrack *track = (FEScriptTrack *)currentScript54->tracks20;
    unsigned int count = currentScript54->trackCount1C;
    while (count) {
        track->flags3 &= 0x7f;
        if (track->value2 == 3 || track->value2 == 4) {
            float *state = data2C + track->state7;
            void *key = track->value2C ? ((FEngReference *)track->value30)->Resolve() : track->value30;
            if (key) {
                unsigned int *out = (unsigned int *)((char *)key + 0x10);
                float *value = (float *)out;
                switch (track->value0) {
                case 1: out[0] = *(unsigned int *)state - *(unsigned int *)track->base18; break;
                case 2: value[0] = state[0] - track->base18[0]; break;
                case 3: {
                    float x = state[0] - track->base18[0], y = state[1] - track->base18[1];
                    value[0] = x; value[1] = y; break;
                }
                case 4: {
                    float x = state[0] - track->base18[0], y = state[1] - track->base18[1];
                    float z = state[2] - track->base18[2];
                    value[0] = x; value[2] = z; value[1] = y; break;
                }
                case 5: {
                    FEngQuaternion inverse, result;
                    inverse.value0[0] = -track->base18[0]; inverse.value0[1] = -track->base18[1];
                    inverse.value0[2] = -track->base18[2]; inverse.value0[3] = track->base18[3];
                    ((FEngQuaternion *)state)->Multiply(&result, &inverse);
                    for (int i = 0; i < 4; ++i) out[i] = ((unsigned int *)result.value0)[i];
                    break;
                }
                case 6: {
                    FEngInteger4 result;
                    ((FEngInteger4 *)state)->Subtract(&result, (FEngInteger4 *)track->base18);
                    out[3] = result.value0[3]; out[2] = result.value0[2];
                    out[1] = result.value0[1]; out[0] = result.value0[0]; break;
                }
                }
            }
        }
        ++track;
        --count;
    }
}

// Original 0x5178b0: assignment is unconditional, initialization only for non-null script.
void HudVisibilityObject::SetScript(FEScript *script) {
    currentScript54 = script;
    if (script) InitializeScript();
}

// Original 0x51cf70: hash the name on every visited script, not once before searching.
void FngSetObjectScript(void *value, const char *name, unsigned char restart) {
    HudVisibilityObject *object = (HudVisibilityObject *)value;
    if (!object) return;
    for (FEScript *script = object->scripts4C; script; script = script->next4) {
        if (script->hash30 == cihash(name)) {
            if (script != object->currentScript54) {
                object->currentScript54 = script;
                object->InitializeScript();
                object->currentScript54->time10 = 0;
            }
            if (restart) script->time10 = 0;
            return;
        }
    }
}

// Original 0x51cfe0: the hash variant keeps the same selection/reset boundaries.
void FngSetObjectScriptHash(void *value, unsigned int hash, unsigned char restart) {
    HudVisibilityObject *object = (HudVisibilityObject *)value;
    if (!object) return;
    for (FEScript *script = object->scripts4C; script; script = script->next4) {
        if (script->hash30 == hash) {
            if (script != object->currentScript54) {
                object->currentScript54 = script;
                object->InitializeScript();
                object->currentScript54->time10 = 0;
            }
            if (restart) script->time10 = 0;
            return;
        }
    }
}
