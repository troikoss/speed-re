#ifndef UG2_FENG_LOOKUP_H
#define UG2_FENG_LOOKUP_H
#include "FEngPackage.h"

// Observed visitor ABI: destructor slot +0, Visit slot +4.
struct FEngObjectVisitor {
    virtual ~FEngObjectVisitor() {}
    virtual unsigned char Visit(HudVisibilityObject *object) = 0;
};
struct FEngHashQuery : FEngObjectVisitor {
    FEngHashQuery(unsigned int value) : hash4(value), found8(0) {}
    unsigned char Visit(HudVisibilityObject *object); // 0x5048d0
    unsigned int hash4;
    HudVisibilityObject *found8;
};
UG2_CHECK_OFFSET(FEngHashQuery, hash4, 4);
UG2_CHECK_OFFSET(FEngHashQuery, found8, 8);
UG2_CHECK_SIZE(FEngHashQuery, 12);
int FEngComparePackageNames(const char *first, const char *second); // 0x505310
void *FngFindObject5379C0(const char *package, unsigned int hash); // 0x5379c0

#endif
