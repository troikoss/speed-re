#include "FEngTrackLookup.h"

extern int fengStateKeys7992F4[];

// Original 0x504a00: first track whose signed state byte matches the selector's key.
FEScriptTrack *FEScript::FindTrack(int selector) {
    int key = fengStateKeys7992F4[selector];
    unsigned int index = 0;
    while (index < (unsigned int)trackCount1C) {
        FEScriptTrack *track = static_cast<FEScriptTrack *>(tracks20) + index;
        if ((int)track->state7 == key) return track;
        ++index;
    }
    return 0;
}

// Original 0x505480: first descriptor with a matching type, or null.
FEngTypeDescriptor *FEngTypeRegistry::Find(int type) {
    FEngTypeDescriptor *descriptor = first8;
    while (descriptor && descriptor->type24 != type) descriptor = descriptor->next4;
    return descriptor;
}

// Original 0x5147f0: zero-based property node, or null past the end.
FEngTypeProperty *FEngTypeDescriptor::GetProperty(int index) {
    FEngTypeProperty *property = first1C;
    int position = 0;
    while (property && position != index) {
        property = property->next4;
        ++position;
    }
    return property;
}
