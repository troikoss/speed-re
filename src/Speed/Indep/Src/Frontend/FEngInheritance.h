#ifndef UG2_FENG_INHERITANCE_H
#define UG2_FENG_INHERITANCE_H
#include "FEngPackage.h"

// Observed reader prefix only; clone/outer-reader lifecycle remain separate.
struct FEngInheritanceContext {
    FEPackage *package0;
    char unidentified4[0x38];
    FEngQueue *queue3C;
    unsigned char Resolve(unsigned int key, HudVisibilityObject **object, FEPackage **package); // 0x5186c0, AL only
};
UG2_CHECK_OFFSET(FEngInheritanceContext, queue3C, 0x3c);
#endif
