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
    void Grow(unsigned int request); // 0x51de90; compares request with text length.
    unsigned short *GetBuffer(unsigned int request); // 0x51dee0
    FEngTextStorage *Assign(const unsigned short *source); // 0x52d610
    void Destruct(); // 0x50d390; does not clear the fields.
};
void FEngCopyText(unsigned short *destination, const unsigned short *source); // 0x511630
struct FEngLeafObject {
    char unidentified0[0x5c];
    unsigned int value5C;
    FEngLeafObject *ConstructImage(); // 0x532640, type1
    void DestructImage(); // 0x532660
    FEngLeafObject *DestroyImage(unsigned int flags); // 0x534dd0
    FEngLeafObject *ConstructType7(); // 0x52b0b0
    FEngLeafObject *ConstructType9(); // 0x534b90
    FEngLeafObject *ConstructType10(); // 0x534db0
};
struct FEngGroupObject {
    char unidentified0[0x5c];
    FEngQueueList children5C;
    FEngGroupObject *Construct(); // 0x52b080
    void Destruct(); // 0x534bf0, normal return
    FEngGroupObject *Destroy(unsigned int flags); // 0x534bd0
    FEngGroupObject *CloneConstruct(const FEngGroupObject *source, unsigned char copyChildren, unsigned int shallow); // 0x5548a0, unconstructed receiver, normal return
    void AppendChild(HudVisibilityObject *child); // 0x517e90, tail insert into the list at +0x5c
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
