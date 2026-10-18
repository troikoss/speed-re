#include "FEngObjectFactory.h"

void FEngLeafObject::DestructImage() { // Original 0x532660
    *(unsigned int *)unidentified0 = 0x79ba0c;
    ((HudVisibilityObject *)this)->DestructBase();
}
FEngLeafObject *FEngLeafObject::DestroyImage(unsigned int flags) { // Original 0x534dd0
    DestructImage();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}

void FEngGroupObject::Destruct() { // Original 0x534bf0, normal return
    // The original does not rewrite this object's vtable before clearing children.
    children5C.value0 = 0x79a9ec;
    children5C.ClearOwned();
    ((HudVisibilityObject *)this)->DestructBase();
}
FEngGroupObject *FEngGroupObject::Destroy(unsigned int flags) { // Original 0x534bd0
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}

void FEListObject::ReleaseCells() { // Original 0x5042e0
    unsigned int count = count64 * count60;
    if (count) {
        unsigned int offset = 0;
        do {
            unsigned char *cell = (unsigned char *)value90 + offset;
            if (*(unsigned int *)(cell + 0x18) == 2 && *(void **)(cell + 0x20)) {
                FEngFreeQueueNode(*(void **)(cell + 0x20));
                *(void **)((unsigned char *)value90 + offset + 0x20) = 0;
            }
            offset += 0x30;
        } while (--count);
        FEngFreeQueueNode(value90);
        value90 = 0;
    }
}

void FEListObject::ReleaseStorage() { // Original 0x516dc0
    if (value5C & 1) {
        value5C &= ~1u;
        if (count60) {
            FEngFreeQueueNode(value78);
            value78 = 0;
            count60 = 0;
        }
        if (count64) {
            FEngFreeQueueNode(value7C);
            value7C = 0;
            count64 = 0;
        }
        ReleaseCells();
    }
}
void FEListObject::Destruct() { // Original 0x534a50, normal return
    *(unsigned int *)unidentified0 = 0x79bae4;
    ReleaseStorage();
    ((HudVisibilityObject *)this)->DestructBase();
}
FEListObject *FEListObject::Destroy(unsigned int flags) { // Original 0x541d90
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
