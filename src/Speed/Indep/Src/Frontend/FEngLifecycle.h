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
    FEngStatePoolBlock *Construct(unsigned short stride); // 0x517310, normal return
    void *AcquireSlot(); // 0x504400
    void Destruct(); // 0x5173b0, normal return
    FEngStatePoolBlock *Destroy(unsigned int flags); // 0x517390
};
struct FEngStatePoolClass {
    void *vtable0;
    FEngStatePoolClass *next4;
    FEngStatePoolClass *previous8;
    FEngQueueList blocksC;
    unsigned int stride1C;
    void *Acquire(); // 0x517400, normal return
    void Destruct(); // 0x534ac0, normal return
    FEngStatePoolClass *Destroy(unsigned int flags); // 0x534aa0
    unsigned char ReleaseSlot(void *pointer); // 0x5174a0, AL only
};
struct FEngStatePoolManager {
    FEngQueueList classes0;
    void *AcquireState(unsigned int stride); // 0x52ace0
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
