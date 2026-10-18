#include "FEngPackage.h"
#include "FEngTrackEvaluate.h"
#include "FEngList.h"

extern float fengZero7D7D3C, fengOne784250;
extern unsigned int fengFlagMask7F7C8C; // retail 0xffffffff: the masked clears below keep every bit

void __fastcall FEngTickType6(HudVisibilityObject *object, int unused, float milliseconds); // 0x504dc0

// Original 0x541db0. The state-offset-0 (colour) track is applied first; the full
// track list (count byte +0x1c) only while state dword 3 is nonzero. Mode +0x14
// routes through 0x5354f0 and maintains flags 0x800000 (tracks running) and
// 0x1000000 (all tracks below state 0x10 finished).
void FEPackage::ApplyScript(HudVisibilityObject *object, FEScript *script) {
    float *state = object->data2C;
    int time = script->time10;
    FEScriptTrack *track = static_cast<FEScriptTrack *>(script->tracks20);
    if (value14) {
        if (track && !(*(unsigned int *)track->unidentified4 & 0xff000000)) {
            if ((signed char)track->flags3 < 0) object->flags1C &= fengFlagMask7F7C8C | 0xff7fffff;
            else object->flags1C |= 0x800000;
            FEngUpdateTrack(track, time, state);
        } else {
            object->flags1C &= fengFlagMask7F7C8C | 0xff7fffff;
        }
        unsigned int finished = 0x80;
        unsigned char count;
        if (*(int *)((char *)object->data2C + 0xc) && (count = *(unsigned char *)&script->trackCount1C) != 0) {
            unsigned int remaining = count;
            do {
                if (track->state7 < 0x10) finished &= track->flags3;
                FEngUpdateTrack(track, time, state + track->state7);
                ++track;
            } while (--remaining);
            if (!finished) {
                object->flags1C |= 0x1000000;
                goto done;
            }
        }
        object->flags1C &= fengFlagMask7F7C8C | 0xfeffffff;
    } else {
        if (track && !(*(unsigned int *)track->unidentified4 & 0xff000000)) {
            switch (track->value2) {
            case 0: FEngStepTrack(track, time, state); break;
            case 1: case 3: FEngEvaluateTrack(track, time, state); break;
            }
        }
        unsigned char count;
        if (*(int *)((char *)object->data2C + 0xc) && (count = *(unsigned char *)&script->trackCount1C) != 0) {
            unsigned int remaining = count;
            do {
                float *out = state + track->state7;
                switch (track->value2) {
                case 0: FEngStepTrack(track, time, out); break;
                case 1: case 3: FEngEvaluateTrack(track, time, out); break;
                }
                ++track;
            } while (--remaining);
        }
    }
done:
    if (object->flags1C & 0x1c00000) object->flags1C |= 0x2000000;
}

// Original 0x534c70: events with from <= time < to (unsigned), to extended by one at
// the script length. Targets: -1 observer game message, -4/-5/-6/0 posts, others a
// GUID object; message 0x1b3909aa focuses its target instead of posting.
void FEPackage::FireScriptEvents(FEngQueue *queue, HudVisibilityObject *object, FEScript *script, int from, int to) {
    int count = script->eventCount24;
    FEScriptEvent *event = (FEScriptEvent *)script->events28;
    if (to < from) return;
    if (to == script->lengthC) ++to;
    int index = 0;
    if (count <= 0) return;
    while ((unsigned int)event[index].time < (unsigned int)from) {
        if (++index >= count) return;
    }
    for (event += index; index < count; ++index, ++event) {
        if ((unsigned int)event->time >= (unsigned int)to) return;
        unsigned int target = event->target;
        if (target == 0xfffffffb) {
            queue->Post(event->message, object, this, -5, 0);
        } else if (target == 0) {
            if (event->message == 0x1b3909aa) SetFocus(queue, FindObjectByGuid(0), 1);
            else queue->Post(event->message, object, this, 0, 0);
        } else if (target == 0xfffffffa) {
            queue->Post(event->message, object, this, -6, 0);
        } else if (target == 0xfffffffc) {
            queue->Post(event->message, object, this, -4, 0);
        } else if (target == 0xffffffff) {
            queue->value10C->GameMessage(event->message, object, 0, this);
        } else {
            HudVisibilityObject *found = FindObjectByGuid(target);
            if (event->message == 0x1b3909aa) SetFocus(queue, FindObjectByGuid(event->target), 1);
            else if (object) queue->Post(event->message, object, this, (int)found, 0);
        }
    }
}

// Original 0x504910: update each child, then OR in the group's flags 0x3c00000.
void FEPackage::UpdateChildren(HudVisibilityObject *group, int delta) {
    for (HudVisibilityObject *child = group->firstChild64; child; child = child->next4) {
        UpdateObject(child, delta);
        child->flags1C |= group->flags1C & 0x3c00000;
    }
}

// Original 0x504dc0: optional callback +0xbc, then phase +0xa4 advances by rate
// +0xa8 and reflects at 0 and 1.
void __fastcall FEngTickType6(HudVisibilityObject *object, int, float milliseconds) {
    char *bytes = (char *)object;
    void (__cdecl *callback)(HudVisibilityObject *) = *(void (__cdecl **)(HudVisibilityObject *))(bytes + 0xbc);
    if (callback) callback(object);
    float *phase = (float *)(bytes + 0xa4), *rate = (float *)(bytes + 0xa8);
    double value = milliseconds * *rate + *phase;
    *phase = (float)value;
    if (value < fengZero7D7D3C) {
        *phase = 0;
        *rate = -*rate;
    } else if (value > fengOne784250) {
        *phase = 1.0f;
        *rate = -*rate;
    }
}

// Original 0x54ce60: advance the current script by +0x34, handle its end (chain,
// once, loop, ping-pong), fire events per elapsed window, apply tracks unless a
// finished script is unchanged, then run type-specific ticks with delta.
void FEPackage::UpdateObject(HudVisibilityObject *object, int delta) {
    unsigned int flags = object->flags1C;
    if (flags & 0x1c00000) object->flags1C = flags | 0x2000000;
    else object->flags1C = (fengFlagMask7F7C8C | 0xfdffffff) & flags;
    FEScript *script = object->currentScript54;
    int old = script->time10, length = script->lengthC;
    int now = delta34 + old;
    script->time10 = now;
    if (now < 0) script->time10 = 0;
    now = script->time10;
    if (now >= length) {
        if (!value14) goto tail;
        if (script->chain18) {
            ApplyScript(object, script);
            int carry = script->time10 - length;
            script->time10 = 0;
            if (script->eventCount24) FireScriptEvents(queue30, object, script, old, length);
            script = script->chain18;
            object->currentScript54 = script;
            if (script) object->InitializeScript();
            script->time10 = carry;
            if (script->eventCount24) FireScriptEvents(queue30, object, script, 0, carry);
            goto tail;
        }
        switch (script->flags14 & 3) {
        case 0:
            if (script->eventCount24) FireScriptEvents(queue30, object, script, old, length);
            script->time10 = script->lengthC + 1;
            break;
        case 1:
            if (length <= 0) { script->time10 = 0; break; }
            if (script->eventCount24) FireScriptEvents(queue30, object, script, old, length);
            {
                int wrapped = script->time10 % script->lengthC;
                script->time10 = wrapped;
                if (script->eventCount24) FireScriptEvents(queue30, object, script, 0, wrapped);
            }
            object->InitializeScript();
            break;
        case 2:
            if (length <= 0) { script->time10 = 0; break; }
            script->time10 = now % (length * 2);
            break;
        }
    } else if (value14 && script->eventCount24) {
        switch (script->flags14 & 3) {
        case 0:
            FireScriptEvents(queue30, object, script, old, now);
            break;
        case 1:
            if (now < old) {
                FireScriptEvents(queue30, object, script, old, length);
                FireScriptEvents(queue30, object, script, 0, script->time10);
            } else {
                FireScriptEvents(queue30, object, script, old, now);
            }
            break;
        case 2:
            if (old < length) {
                if (now < length) {
                    FireScriptEvents(queue30, object, script, old, now);
                } else {
                    FireScriptEvents(queue30, object, script, old, length);
                    FireScriptEvents(queue30, object, script, length, script->time10 - length);
                }
            } else if (now < length) {
                FireScriptEvents(queue30, object, script, old - length, 0);
                FireScriptEvents(queue30, object, script, 0, script->time10);
            } else {
                FireScriptEvents(queue30, object, script, old - length, now - length);
            }
            break;
        }
    }
tail:
    if (!value14 || old != script->time10 || old != script->lengthC + 1 || (object->flags1C & 0x400000))
        ApplyScript(object, script);
    switch (object->type18) {
    case 4: ((FEListObject *)object)->Tick((float)delta); break;
    case 5: UpdateChildren(object, delta); break;
    case 6: FEngTickType6(object, 0, (float)delta); break;
    case 7: if (value14) *(int *)((char *)object + 0x5c) += delta; break;
    }
    if (value14 == 1 && old == script->time10 && old == script->lengthC + 1 && !(object->flags1C & 0x400000))
        object->flags1C &= fengFlagMask7F7C8C | 0xfe7fffff;
    flags = object->flags1C & (fengFlagMask7F7C8C | 0xffbfffff);
    object->flags1C = flags;
    if (flags & 0x1c00000) object->flags1C = flags | 0x2000000;
}
