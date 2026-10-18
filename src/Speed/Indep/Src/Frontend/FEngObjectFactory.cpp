#include "FEngObjectFactory.h"

// Original 0x534df0, all switch branches and actual constructor/state calls.
HudVisibilityObject *FEngObjectFactoryContext::Create(int type) {
    HudVisibilityObject *object;
    switch (type) {
    case 0: return 0;
    case 1:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x60);
        if (object) object = (HudVisibilityObject *)((FEngLeafObject *)object)->ConstructImage();
        break;
    case 2:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x78);
        if (object) object = ((FEStringObject *)object)->Construct();
        break;
    case 4:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0xac);
        if (object) object = (HudVisibilityObject *)((FEListObject *)object)->Construct();
        break;
    case 5:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x6c);
        if (object) object = (HudVisibilityObject *)((FEngGroupObject *)object)->Construct();
        break;
    case 6:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0xc8);
        if (object) object = (HudVisibilityObject *)((FEGridObject *)object)->Construct();
        ((FEGridObject *)object)->owner5C = owner38;
        break;
    case 7:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x60);
        if (object) object = (HudVisibilityObject *)((FEngLeafObject *)object)->ConstructType7();
        break;
    case 9:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x60);
        if (object) object = (HudVisibilityObject *)((FEngLeafObject *)object)->ConstructType9();
        break;
    case 10:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x60);
        if (object) object = (HudVisibilityObject *)((FEngLeafObject *)object)->ConstructType10();
        break;
    case 11:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x5c);
        if (object) object = (HudVisibilityObject *)((FEngType11Object *)object)->Construct();
        break;
    default:
        object = (HudVisibilityObject *)FEngAllocateQueueNode(0x5c);
        if (object) object = object->ConstructBase();
        break;
    }
    object->type18 = type;
    unsigned int stride = 0;
    unsigned int count = count30;
    if (count) {
        FEngObjectStateSpec *specs = specs34;
        for (unsigned int i = 0; i < count; ++i) {
            if (specs[i].type0 == type) { stride = specs[i].stride4; break; }
        }
    }
    FEngObjectStateManager.ReleaseState(object->data2C);
    object->data2C = 0;
    void *state = FEngObjectStateManager.AcquireState(stride);
    *(unsigned int *)object->unidentified30 = stride;
    object->data2C = (float *)state;
    return object;
}
