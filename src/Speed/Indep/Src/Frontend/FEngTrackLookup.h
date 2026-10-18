#ifndef UG2_FENG_TRACK_LOOKUP_H
#define UG2_FENG_TRACK_LOOKUP_H
#include "../Hud/HudVisibility.h"

// Observed only from the script reader's Ti branch (0x55194a..0x5519fc): a property
// node supplies the inserted track's type +0x14, interpolation +0x18 and state byte
// offset +0x1c. Other fields and the registry's producer remain unidentified.
struct FEngTypeProperty {
    char unidentified0[4];
    FEngTypeProperty *next4;
    char unidentified8[0xc];
    unsigned int value14;
    unsigned int value18;
    int offset1C;
};
struct FEngTypeDescriptor {
    char unidentified0[4];
    FEngTypeDescriptor *next4;
    char unidentified8[0x14];
    FEngTypeProperty *first1C;
    char unidentified20[4];
    int type24;
    FEngTypeProperty *GetProperty(int index); // 0x5147f0
};
// FEngQueue +0x4134; the reader reaches it through its context +0x3c.
struct FEngTypeRegistry {
    char unidentified0[8];
    FEngTypeDescriptor *first8;
    char unidentifiedC[8];
    FEngTypeDescriptor *Find(int type); // 0x505480
};
UG2_CHECK_OFFSET(FEngTypeProperty, value14, 0x14);
UG2_CHECK_OFFSET(FEngTypeProperty, offset1C, 0x1c);
UG2_CHECK_OFFSET(FEngTypeDescriptor, first1C, 0x1c);
UG2_CHECK_OFFSET(FEngTypeDescriptor, type24, 0x24);
UG2_CHECK_OFFSET(FEngTypeRegistry, first8, 8);
UG2_CHECK_SIZE(FEngTypeRegistry, 0x14);
#endif
