#ifndef UG2_FENG_OBJECT_FACTORY_H
#define UG2_FENG_OBJECT_FACTORY_H
#include "FEngCoordinates.h"
#include "FEngList.h"
#include "FEngGrid.h"
#include "FEngLifecycle.h"
struct FEngTextStorage {
    unsigned short *text0;
    unsigned int capacity4;
    FEngTextStorage *Construct(); // 0x51de80
};
struct FEngLeafObject {
    char unidentified0[0x5c];
    unsigned int value5C;
    FEngLeafObject *ConstructImage(); // 0x532640, type1
    FEngLeafObject *ConstructType7(); // 0x52b0b0
    FEngLeafObject *ConstructType9(); // 0x534b90
    FEngLeafObject *ConstructType10(); // 0x534db0
};
struct FEngGroupObject {
    char unidentified0[0x5c];
    FEngQueueList children5C;
    FEngGroupObject *Construct(); // 0x52b080
};
struct FEngType11Object {
    char unidentified0[0x5c];
    FEngType11Object *Construct(); // 0x52b480
};
struct FEngObjectStateSpec { int type0; unsigned int stride4; };
// Observed reader/factory prefix only; full reader ownership/iteration is separate.
struct FEngObjectFactoryContext {
    char unidentified0[0x30];
    unsigned int count30;
    FEngObjectStateSpec *specs34;
    FEngQueueObserver *owner38;
    HudVisibilityObject *Create(int type); // 0x534df0
};
UG2_CHECK_SIZE(FEngTextStorage, 8);
UG2_CHECK_SIZE(FEngLeafObject, 0x60);
UG2_CHECK_SIZE(FEngGroupObject, 0x6c);
UG2_CHECK_SIZE(FEngType11Object, 0x5c);
UG2_CHECK_OFFSET(FEngObjectFactoryContext, owner38, 0x38);
#endif
