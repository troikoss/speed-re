#include "WCollision.h"

// Original 0x5e6360. The record payloads are not cleared during construction;
// only links, the pointer table, counters and final eight words are initialized.
TrackTopology *TrackTopology::Construct() {
    WCollisionLink *heads = &sections0;
    for (int i = 0; i < 9; ++i) {
        heads[i].next = &heads[i];
        heads[i].previous = &heads[i];
    }
    free6414.next = &free6414;
    free6414.previous = &free6414;
    *reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(this) + 0x48) = 0;
    header4C = 0;
    for (int i = 0; i < 2800; ++i) resources50[i] = 0;
    resourceCount2C10 = 0;
    for (int i = 0; i < 512; ++i) {
        WCollisionLink *last = free6414.previous;
        WCollisionLink *record = &storage2C14[i].free0;
        last->next = record;
        free6414.previous = record;
        record->next = &free6414;
        record->previous = last;
    }
    for (int i = 0; i < 8; ++i) value641C[i] = 0;
    return this;
}

// Original 0x5db680. The original assumes a valid index and an available free
// record; it does not add bounds checks or an exhausted-pool fallback.
TrackSectionResource *TrackTopology::AcquireResource(int section) {
    TrackSectionResource *record = resources50[section];
    if (!record) {
        record = reinterpret_cast<TrackSectionResource *>(free6414.next);
        ++resourceCount2C10;
        WCollisionLink *next = record->free0.next;
        WCollisionLink *previous = record->free0.previous;
        previous->next = next;
        next->previous = previous;
        unsigned int *words = reinterpret_cast<unsigned int *>(record);
        for (int i = 0; i < 7; ++i) words[i] = 0;
        resources50[section] = record;
    }
    ++record->used.references0;
    return record;
}

// Original 0x5db6e0. A final release appends the record without clearing payloads.
void TrackTopology::ReleaseResource(int section) {
    TrackSectionResource *record = resources50[section];
    if (record && --record->used.references0 == 0) {
        --resourceCount2C10;
        WCollisionLink *last = free6414.previous;
        last->next = &record->free0;
        free6414.previous = &record->free0;
        record->free0.previous = last;
        record->free0.next = &free6414;
        resources50[section] = 0;
    }
}
