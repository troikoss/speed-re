#include "FEngScript.h"
#include "FEngScriptKeyPool.h"
#include "FEngResponseArray.h"
#include "FEngQueueNodes.h"

typedef void (__thiscall *FEngKeyDeletingDestructor)(void *, unsigned int);

void *__fastcall FEngConstructScriptKey(void *memory) { // Original 0x4fe8b0
    FEngScriptKey *key = (FEngScriptKey *)memory;
    key->next4 = (FEngScriptKey *)0xabadcafe;
    key->previous8 = (FEngScriptKey *)0xabadcafe;
    key->vtable0 = (void *)0x79aa34;
    return memory;
}
void __fastcall FEngDestructScriptKey(void *memory) { // Original 0x51ab10, callback ABI view
    ((FEngScriptKey *)memory)->vtable0 = (void *)0x79a9e8;
}
FEngScriptKey *FEngScriptKey::Destroy(unsigned int flags) { // Original 0x531fe0
    ((FEngQueueNode *)this)->DestructMessage();
    if (flags & 1) {
        ((FEngKeyDeletingDestructor *)vtable0)[0](this, 0);
        FEngScriptKeysPool.Release(this);
    }
    return this;
}

void FEngReference::Remove(FEngScriptKey *key) { // Original 0x5055c0
    if (key) {
        if (key == value8) value8 = key->next4;
        if (key == valueC) valueC = key->previous8;
        if (key->previous8) key->previous8->next4 = key->next4;
        if (key->next4) key->next4->previous8 = key->previous8;
        key->next4 = (FEngScriptKey *)0xabadcafe;
        key->previous8 = (FEngScriptKey *)0xabadcafe;
    }
}
void FEngReference::ClearOwned() { // Original 0x511d00
    FEngScriptKey *key = (FEngScriptKey *)value8;
    if (key) Remove(key);
    if (key) {
        do {
            if (key) ((FEngKeyDeletingDestructor *)key->vtable0)[0](key, 1);
            key = (FEngScriptKey *)value8;
            if (key) {
                value8 = key->next4;
                if (key == valueC) valueC = key->previous8;
                if (key->previous8) key->previous8->next4 = key->next4;
                if (key->next4) key->next4->previous8 = key->previous8;
                key->next4 = (FEngScriptKey *)0xabadcafe;
                key->previous8 = (FEngScriptKey *)0xabadcafe;
            }
        } while (key);
    }
}

void *__fastcall FEngConstructScriptTrack(void *memory) { // Original 0x525e80
    FEScriptTrack *track = (FEScriptTrack *)memory;
    track->value0 = 0; track->value1 = 0; track->value2 = 0; track->flags3 = 0;
    *(unsigned int *)track->unidentified4 &= 0xff000000;
    track->state7 = 0;
    track->key8.next4 = (FEngScriptKey *)0xabadcafe;
    track->key8.previous8 = (FEngScriptKey *)0xabadcafe;
    track->key8.vtable0 = (void *)0x79aa34;
    *(unsigned int *)track->unidentified28 = 0x79aa30;
    track->value30 = 0;
    *(unsigned int *)track->unidentified34 = 0;
    track->value2C = 0;
    return memory;
}
void __fastcall FEngDestructScriptTrack(void *memory) { // Original 0x52b960, normal return
    FEScriptTrack *track = (FEScriptTrack *)memory;
    unsigned char reference = track->value2C;
    *(unsigned int *)track->unidentified28 = 0x79aa30;
    if (!reference) ((FEngReference *)track->unidentified28)->ClearOwned();
    track->key8.vtable0 = (void *)0x79a9e8;
}

void FEScript::SetTracks(unsigned int count) { // Original 0x535210, normal return
    void *old = tracks20;
    if (old) {
        unsigned int *base = (unsigned int *)old - 1;
        FEngDestructArrayWithCleanup(old, 0x38, *base, FEngDestructScriptTrack);
        FEngFreeQueueNode(base);
    }
    tracks20 = 0;
    trackCount1C = (int)count;
    if (count) {
        unsigned int *allocation = (unsigned int *)FEngAllocateQueueNode(count * 0x38 + 4);
        void *buffer = 0;
        if (allocation) {
            buffer = allocation + 1;
            *allocation = count;
            FEngConstructArrayWithCleanup(buffer, 0x38, count, FEngConstructScriptTrack, FEngDestructScriptTrack);
        }
        tracks20 = buffer;
    }
}

void *__fastcall FEngConstructScript(void *memory) { // Original 0x511d70
    FEScript *script = (FEScript *)memory;
    script->next4 = (FEScript *)0xabadcafe;
    script->previous8 = (FEScript *)0xabadcafe;
    script->vtable0 = (void *)0x79ad28;
    script->eventCount24 = 0; script->events28 = 0; script->name2C = 0; script->hash30 = 0;
    script->lengthC = 0; script->flags14 = 0; script->chain18 = 0;
    script->trackCount1C = 0; script->tracks20 = 0;
    return memory; // time10 is untouched.
}
void __fastcall FEngDestructScript(void *memory) { // Original 0x535170, normal return
    FEScript *script = (FEScript *)memory;
    script->vtable0 = (void *)0x79ad28;
    void *old = script->tracks20;
    if (old) {
        unsigned int *base = (unsigned int *)old - 1;
        FEngDestructArrayWithCleanup(old, 0x38, *base, FEngDestructScriptTrack);
        FEngFreeQueueNode(base);
    }
    script->tracks20 = 0;
    if (script->name2C) FEngFreeQueueNode(script->name2C);
    script->name2C = 0;
    FEngFreeQueueNode(script->events28);
    script->events28 = 0;
    script->vtable0 = (void *)0x79a9e8;
}

void FEScriptTrack::Assign(const FEScriptTrack *other) { // Original 0x54d350
    FEngReference *reference = (FEngReference *)unidentified28;
    reference->ClearOwned();
    value0 = other->value0;
    value1 = other->value1;
    value2 = other->value2;
    flags3 = other->flags3;
    unsigned int bits = *(unsigned int *)unidentified4;
    *(unsigned int *)unidentified4 = bits ^ ((*(const unsigned int *)other->unidentified4 ^ bits) & 0xffffff);
    state7 = other->state7;
    key8.timeC = other->key8.timeC;
    ((unsigned int *)base18)[0] = ((const unsigned int *)other->base18)[0];
    ((unsigned int *)base18)[1] = ((const unsigned int *)other->base18)[1];
    ((unsigned int *)base18)[2] = ((const unsigned int *)other->base18)[2];
    ((unsigned int *)base18)[3] = ((const unsigned int *)other->base18)[3];
    if (other->value2C) {
        void *target = other->value30;
        if (!target) {
            reference->value8 = 0;
            reference->valueC = 0;
            reference->value4 = 0;
        } else {
            if (!reference->value4) reference->ClearOwned();
            reference->value8 = target;
            reference->value4 = 1;
        }
        return;
    }
    FEngScriptKey *source = (FEngScriptKey *)other->value30;
    while (source) {
        FEngScriptKey *key = FEngScriptKeysPool.Acquire();
        if (key) {
            key->next4 = (FEngScriptKey *)0xabadcafe;
            key->previous8 = (FEngScriptKey *)0xabadcafe;
            key->vtable0 = (void *)0x79aa34;
        }
        key->timeC = source->timeC;
        ((unsigned int *)key->value10)[0] = ((const unsigned int *)source->value10)[0];
        ((unsigned int *)key->value10)[1] = ((const unsigned int *)source->value10)[1];
        ((unsigned int *)key->value10)[2] = ((const unsigned int *)source->value10)[2];
        ((unsigned int *)key->value10)[3] = ((const unsigned int *)source->value10)[3];
        FEngScriptKey *cursor = (FEngScriptKey *)reference->valueC;
        if (cursor) {
            FEngScriptKey *next = cursor->next4;
            key->next4 = next;
            if (next) next->previous8 = key;
            key->previous8 = cursor;
            cursor->next4 = key;
        } else {
            FEngScriptKey *first = (FEngScriptKey *)reference->value8;
            key->next4 = first;
            if (first) first->previous8 = key;
            key->previous8 = 0;
            reference->value8 = key;
        }
        if (reference->valueC == cursor) reference->valueC = key;
        source = source->next4;
    }
}
