#ifndef UG2_FENG_LIFECYCLE_H
#define UG2_FENG_LIFECYCLE_H
#include "FEngQueueNodes.h"
struct FEngStatePoolBlock {
    void *vtable0;
    FEngStatePoolBlock *next4;
    FEngStatePoolBlock *previous8;
    unsigned short strideC;
    unsigned short usedE;
    unsigned char bitmap10[4];
    unsigned char *data14;
};
struct FEngStatePoolClass {
    void *vtable0;
    FEngStatePoolClass *next4;
    FEngStatePoolClass *previous8;
    FEngQueueList blocksC;
    unsigned int stride1C;
    unsigned char ReleaseSlot(void *pointer); // 0x5174a0, AL only
};
struct FEngStatePoolManager {
    FEngQueueList classes0;
    void ReleaseState(void *pointer); // 0x517520; nonnull pointer must belong to a class.
};
struct FEngDestructionObserver {
    virtual void ObjectDestroyed(HudVisibilityObject *object) = 0; // observed slot zero
};
extern FEngStatePoolManager FEngObjectStateManager; // original 0x863770
extern FEngDestructionObserver *FEngObjectDestructionObserver; // original 0x838474
UG2_CHECK_SIZE(FEngStatePoolBlock, 0x18);
UG2_CHECK_OFFSET(FEngStatePoolClass, blocksC, 0xc);
UG2_CHECK_SIZE(FEngStatePoolClass, 0x20);
#endif
