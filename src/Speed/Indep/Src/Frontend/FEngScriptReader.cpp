#include "FEngScriptReader.h"
#include "FEngScriptPool.h"
#include "FEngScriptKeyPool.h"
#include "FEngResponseArray.h"

// Original 0x5515e0: one Scrp tag stream. Null script/track checks exist only
// where the original has them; other tags dereference the current pointers.
unsigned char FEngScriptReader::Read(const unsigned short *data, unsigned int size) {
    const unsigned short *end = (const unsigned short *)((const char *)data + size);
    FEScript *script = 0;
    FEScriptTrack *track = 0;
    int trackIndex = -1;
    int nextState = 0;
    for (const unsigned short *tag = data; tag < end; tag = (const unsigned short *)((const char *)tag + tag[1] + 4)) {
        const unsigned int *payload = (const unsigned int *)(tag + 2);
        switch (tag[0]) {
        case 0x6653: // Sf
            if (script) script->flags14 = payload[0];
            break;
        case 0x6154: // Ta
            if (script) track->flags3 = *(const unsigned char *)payload;
            break;
        case 0x4946: { // FI
            ++trackIndex;
            track = static_cast<FEScriptTrack *>(script->tracks20) + trackIndex;
            const unsigned char *bytes = (const unsigned char *)payload;
            track->value0 = bytes[0];
            track->value1 = bytes[1];
            track->value2 = bytes[2];
            track->flags3 = bytes[3];
            unsigned int *bits = (unsigned int *)((char *)track + 4);
            *bits ^= (payload[1] ^ *bits) & 0xffffff;
            track->state7 = (signed char)nextState;
            nextState += track->value1 >> 2;
            break;
        }
        case 0x4953: // SI
            script = objectC->FindScriptByHash(payload[0]);
            track = 0;
            break;
        case 0x5645: { // EV
            int count = tag[1] / 12;
            ((FEngScriptEventStorage *)&script->eventCount24)->Resize(count);
            FEScriptEvent *event = (FEScriptEvent *)script->events28;
            const unsigned int *source = payload;
            do {
                event->message = source[0];
                event->target = source[1];
                event->time = (int)source[2];
                ++event;
                source += 3;
            } while (--count);
            break;
        }
        case 0x6254: { // Tb
            unsigned int words = track->value1 >> 2;
            track->key8.timeC = (int)payload[0];
            unsigned int *base = (unsigned int *)track->base18;
            for (unsigned int i = 0; i < words; ++i) base[i] = payload[i + 1];
            break;
        }
        case 0x6353: // Sc
            script->chain18 = (FEScript *)payload[0];
            break;
        case 0x644b: { // Kd
            unsigned int stride = track->value1 + 4;
            unsigned int count = tag[1] / stride;
            unsigned int words = (stride >> 2) - 1;
            const unsigned int *source = payload;
            FEngReference *keys = (FEngReference *)track->unidentified28;
            if (keys->value4) {
                keys->value8 = 0;
                keys->valueC = 0;
                keys->value4 = 0;
            }
            unsigned int index = 0;
            do {
                FEngScriptKey *key;
                if (index) {
                    key = FEngScriptKeysPool.Acquire();
                    if (key) {
                        key->next4 = (FEngScriptKey *)0xabadcafe;
                        key->previous8 = (FEngScriptKey *)0xabadcafe;
                        key->vtable0 = (void *)0x79aa34;
                    } else {
                        key = 0;
                    }
                } else {
                    key = (FEngScriptKey *)&track->key8; // first key is inline at track +8
                }
                key->timeC = (int)source[0];
                for (unsigned int i = 0; i < words; ++i) ((unsigned int *)key->value10)[i] = source[i + 1];
                if (index) {
                    FEngScriptKey *tail = (FEngScriptKey *)keys->valueC;
                    if (tail) {
                        FEngScriptKey *next = tail->next4;
                        key->next4 = next;
                        if (next) next->previous8 = key;
                        key->previous8 = tail;
                        tail->next4 = key;
                    } else {
                        FEngScriptKey *head = (FEngScriptKey *)keys->value8;
                        key->next4 = head;
                        if (head) head->previous8 = key;
                        key->previous8 = 0;
                        keys->value8 = key;
                    }
                    if (keys->valueC == tail) keys->valueC = key;
                }
                source = (const unsigned int *)((const char *)source + stride);
                ++index;
            } while (index < count);
            break;
        }
        case 0x6e53: // Sn: inline construction, unlike Sh
            script = FEngScriptHeaderPool.Acquire();
            script->next4 = (FEScript *)0xabadcafe;
            script->previous8 = (FEScript *)0xabadcafe;
            script->vtable0 = (void *)0x79ad28;
            script->eventCount24 = 0;
            script->events28 = 0;
            script->name2C = 0;
            script->hash30 = 0;
            script->lengthC = 0;
            script->flags14 = 0;
            script->chain18 = 0;
            script->trackCount1C = 0;
            script->tracks20 = 0;
            script->time10 = 0;
            if (value0A) script->SetName((const char *)payload);
            trackIndex = -1;
            nextState = 0;
            break;
        case 0x6853: // Sh
            if (!script) {
                FEScript *header = FEngScriptHeaderPool.Acquire();
                header->name2C = 0;
                header->hash30 = 0;
                header->lengthC = 0;
                header->flags14 = 0;
                header->chain18 = 0;
                header->trackCount1C = 0;
                header->tracks20 = 0;
                script = (FEScript *)FEngConstructScript(header);
                script->time10 = 0;
                trackIndex = -1;
                nextState = 0;
            }
            script->hash30 = payload[0];
            script->lengthC = (int)payload[1];
            script->flags14 = payload[2];
            script->SetTracks(payload[3]);
            break;
        case 0x6954: // Ti
            if (script) {
                unsigned int selector = tag[2];
                track = script->FindTrack((int)selector);
                if (!track) {
                    unsigned int count = (unsigned int)script->trackCount1C + 1;
                    unsigned int *allocation = (unsigned int *)FEngAllocateQueueNode(count * 0x38 + 4);
                    FEScriptTrack *tracks = 0;
                    if (allocation) {
                        tracks = (FEScriptTrack *)(allocation + 1);
                        *allocation = count;
                        FEngConstructArrayWithCleanup(tracks, 0x38, count, FEngConstructScriptTrack, FEngDestructScriptTrack);
                    }
                    FEngTypeProperty *property = queue3C->types4134.Find(objectC->type18)->GetProperty((int)selector);
                    FEScriptTrack *old = static_cast<FEScriptTrack *>(script->tracks20);
                    FEScriptTrack *source = old;
                    FEScriptTrack *destination = tracks;
                    unsigned int copied = 0;
                    unsigned int index = 0;
                    do {
                        if (old && copied < (unsigned int)script->trackCount1C &&
                            (!property || (int)source->state7 < property->offset1C / 4)) {
                            destination->Assign(source);
                            ++copied;
                            ++source;
                        } else {
                            destination->value0 = (unsigned char)property->value14;
                            destination->value1 = (unsigned char)property->value18;
                            destination->value2 = 1;
                            destination->flags3 = 0;
                            unsigned int *bits = (unsigned int *)((char *)destination + 4);
                            *bits ^= ((unsigned int)script->lengthC ^ *bits) & 0xffffff;
                            destination->state7 = (signed char)((unsigned int)property->offset1C >> 2);
                            track = destination;
                            property = 0;
                        }
                        ++index;
                        ++destination;
                    } while (index <= (unsigned int)script->trackCount1C);
                    void *previous = script->tracks20;
                    if (previous) {
                        unsigned int *base = (unsigned int *)previous - 1;
                        FEngDestructArrayWithCleanup(previous, 0x38, *base, FEngDestructScriptTrack);
                        FEngFreeQueueNode(base);
                    }
                    script->tracks20 = tracks;
                    script->trackCount1C = script->trackCount1C + 1;
                }
            }
            break;
        case 0x6c53: // Sl
            script->lengthC = (int)payload[0];
            break;
        case 0x6f54: // To
            track->state7 = *(const signed char *)payload;
            break;
        case 0x7454: // Tt
            if (script) track->value2 = *(const unsigned char *)payload;
            break;
        }
    }
    if (!value18) {
        FEngQueueList *scripts = (FEngQueueList *)((char *)objectC + 0x44);
        FEScript *last = (FEScript *)scripts->valueC;
        if (script) {
            if (last) {
                FEScript *next = last->next4;
                script->next4 = next;
                if (next) next->previous8 = script;
                script->previous8 = last;
                last->next4 = script;
            } else {
                FEScript *first = (FEScript *)scripts->first8;
                script->next4 = first;
                if (first) first->previous8 = script;
                script->previous8 = 0;
                scripts->first8 = (FEngQueueNode *)script;
            }
            if ((FEScript *)scripts->valueC == last) scripts->valueC = (FEngQueueNode *)script;
            ++scripts->count4;
        }
    }
    if (script->hash30 == 0x1744b3) objectC->currentScript54 = script; // "Init"
    return 1;
}
