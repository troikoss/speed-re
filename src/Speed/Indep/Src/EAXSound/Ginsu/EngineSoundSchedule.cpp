#include "EngineSoundSchedule.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;

// 0x4022c0 / 0x4031c0: shared RET / RET 4 leaves.
void __fastcall EngineSoundNoop4022C0(void *receiver) { (void)receiver; }
void __fastcall EngineSoundNoop4031C0(void *receiver, void *, float duration) {
    (void)receiver; (void)duration;
}
// 0x464ba0.
void EngineSoundCar::Bind464BA0(MoverModeOwner *owner) { car30 = owner; }

// 0x464a10: 64-entry scratch, initial key ceiling 64, strict tie comparison.
// Counts/list lengths and keys are unchecked in the original.
void EngineSoundCar::Sort464A10() {
    EngineSoundScheduledNode *nodes[64] = {0};
    EngineSoundScheduledNode **destination = nodes;
    for (EngineSoundScheduledNode *node = first1C; node; node = node->next50) *destination++ = node;
    first1C = 0;
    EngineSoundScheduledNode *tail = 0;
    for (;;) {
        bool found = false;
        int selected = 0;
        unsigned int lowest = 64;
        for (int i = 0; i < count2C; ++i) {
            if (nodes[i]) {
                const unsigned int key = (nodes[i]->key4C >> 4) & 127;
                found = true;
                if (key < lowest) { lowest = key; selected = i; }
            }
        }
        if (!found) return;
        EngineSoundScheduledNode *node = nodes[selected];
        if (!first1C) first1C = node;
        else tail->next50 = node;
        tail = node;
        tail->next50 = 0;
        nodes[selected] = 0;
    }
}

// 0x464c10: callback-time table, second head and next-link reloads matter.
// The clock gate does not suppress the duration passed to component callbacks.
void EngineSoundCar::Update464C10(float duration) {
    if (!*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(audio82B884) + 0x13)) {
        *reinterpret_cast<unsigned int *>(&delta38) = *reinterpret_cast<unsigned int *>(&duration);
        time34 = duration + time34;
    } else *reinterpret_cast<unsigned int *>(&delta38) = 0;
    for (EngineSoundScheduledNode *node = first1C; node; node = node->next50) {
        node->vtable0->update14(node, 0, duration);
        node->vtable0->post18(node);
    }
    for (EngineSoundScheduledNode *node = first20; node; node = node->next50) {
        node->vtable0->update24(node, 0, duration);
        node->vtable0->post34(node);
    }
}
// 0x469cf0: owner gate precedes dispatch; clear only the byte at +0xc0.
void EngineSoundCar::Update469CF0(float duration) {
    if (car30) {
        vtable0->pre34(this);
        Update464C10(duration);
        flagC0 = 0;
    }
}
// 0x469cb0: separate playback pass, with the original owner gate.
void EngineSoundCar::Post469CB0() {
    if (car30) {
        for (EngineSoundScheduledNode *node = first20; node; node = node->next50) node->vtable0->post28(node);
    }
}
// 0x460ff0: playback's virtual +0x28 tail-dispatches its +0x48 implementation.
void EngineSound::Forward460FF0() { vtable0->value48(this); }
