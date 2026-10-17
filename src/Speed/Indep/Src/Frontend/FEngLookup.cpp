#include "FEngLookup.h"
#include "PauseMenu.h"
#include "../../Libs/bWare/Strings.h"

// Original 0x5048d0: stop traversal at the first hash match.
unsigned char FEngHashQuery::Visit(HudVisibilityObject *object) {
    if (object->hash10 == hash4) {
        found8 = object;
        return false;
    }
    return true;
}

// Original 0x504830: callbacks precede recursion, and siblings are reloaded after callbacks.
bool FEPackage::VisitChildren(HudVisibilityObject *group, FEngObjectVisitor *visitor) {
    HudVisibilityObject *object = group->firstChild64;
    while (object) {
        if (!visitor->Visit(object)) return false;
        if (object->type18 == 5 && !VisitChildren(object, visitor)) return false;
        object = object->next4;
    }
    return true;
}

// Original 0x504880.
bool FEPackage::VisitObjects(FEngObjectVisitor *visitor) {
    HudVisibilityObject *object = firstObject4C;
    while (object) {
        if (!visitor->Visit(object)) return false;
        if (object->type18 == 5 && !VisitChildren(object, visitor)) return false;
        object = object->next4;
    }
    return true;
}

// Original 0x517f80. The retail compiler adds a temporary query's SEH lifetime frame.
HudVisibilityObject *FEPackage::FindObject(unsigned int hash) {
    FEngHashQuery query(hash);
    VisitObjects(&query);
    return query.found8;
}

// Original 0x505310: signed-byte ASCII case folding and signed final subtraction.
int FEngComparePackageNames(const char *first, const char *second) {
    signed char a, b;
    do {
        a = (signed char)*first++;
        if (a >= 'a' && a <= 'z') a += 'A' - 'a';
        b = (signed char)*second++;
        if (b >= 'a' && b <= 'z') b += 'A' - 'a';
    } while (a && a == b);
    return (int)a - (int)b;
}

// Original 0x519e50. Hashing precedes even an empty list; null names compare as pointers.
FEPackage *FEngPackageList::FindByName(const char *name, FEPackage *first) {
    unsigned int hash = cihash(name);
    for (FEPackage *package = first; package; package = package->next4) {
        if (!package->nameC) {
            if (!name) return package;
        } else if (hash == package->nameHash10 && !FEngComparePackageNames(package->nameC, name)) {
            return package;
        }
    }
    return 0;
}

// Original 0x52c570: named lookup uses one package; null name searches the chain in order.
HudVisibilityObject *FEngQueue::FindObject(unsigned int hash, const char *name) {
    if (name) {
        FEPackage *package = packagesDC.FindByName(name, packagesDC.first8);
        return package ? package->FindObject(hash) : 0;
    }
    for (FEPackage *package = packagesDC.first8; package; package = package->next4) {
        HudVisibilityObject *object = package->FindObject(hash);
        if (object) return object;
    }
    return 0;
}

// Original 0x5379c0: package-local lookup precedes the manager's queue fallback.
void *FngFindObject5379C0(const char *name, unsigned int hash) {
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) {
            HudVisibilityObject *object = package->FindObject(hash);
            if (object) return object;
        }
    }
    return fengManager8384C4 ? fengManager8384C4->queue8->FindObject(hash, name) : 0;
}

// Original 0x537a10. A found object of the wrong type does not retry fallback.
void *FngFindStringByHash(const char *name, unsigned int hash) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    return object && object->type18 == 2 ? object : 0;
}

// Original 0x537a60.
HudVisibilityObject *FngFindImageByHash(const char *name, unsigned int hash) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    return object && object->type18 == 1 ? object : 0;
}
