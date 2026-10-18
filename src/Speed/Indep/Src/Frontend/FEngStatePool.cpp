#include "FEngLifecycle.h"

// Original 0x517310, normal-return construction; bitmap clears after allocation.
FEngStatePoolBlock *FEngStatePoolBlock::Construct(unsigned short stride) {
    next4 = (FEngStatePoolBlock *)0xabadcafe;
    previous8 = (FEngStatePoolBlock *)0xabadcafe;
    strideC = stride;
    vtable0 = (void *)0x79b13c;
    usedE = 0;
    data14 = 0;
    data14 = (unsigned char *)FEngAllocateQueueNode((unsigned int)stride * 32);
    *(unsigned int *)bitmap10 = 0;
    return this;
}

// Original 0x504400. Fullness is equality to32, not a bitmap-popcount check.
void *FEngStatePoolBlock::AcquireSlot() {
    unsigned short used = usedE;
    if (used == 32) return 0;
    unsigned int byte = 0;
    while (bitmap10[byte] == 0xff) ++byte;
    unsigned int slot = byte << 3;
    while (bitmap10[slot >> 3] & (1u << (slot & 7))) ++slot;
    usedE = (unsigned short)(used + 1);
    bitmap10[slot >> 3] |= (unsigned char)(1u << (slot & 7));
    return (void *)((unsigned int)data14 + (unsigned int)strideC * slot);
}

// Original 0x517400. Keep first suitable block, allocation/constructor call
// boundaries, post-allocation list reloads and the final AcquireSlot call.
void *FEngStatePoolClass::Acquire() {
    FEngStatePoolBlock *block = (FEngStatePoolBlock *)blocksC.first8;
    while (block && block->usedE == 32) block = block->next4;
    if (!block) {
        block = (FEngStatePoolBlock *)FEngAllocateQueueNode(0x18);
        if (block) block = block->Construct((unsigned short)stride1C);
        if (block) {
            FEngStatePoolBlock *first = (FEngStatePoolBlock *)blocksC.first8;
            block->next4 = first;
            if (first) first->previous8 = block;
            block->previous8 = 0;
            blocksC.first8 = (FEngQueueNode *)block;
            if (!blocksC.valueC) blocksC.valueC = (FEngQueueNode *)block;
            ++blocksC.count4;
        }
    }
    return block->AcquireSlot();
}

// Original 0x52ace0: full DWORD class keys, low16 stride at block construction.
void *FEngStatePoolManager::AcquireState(unsigned int stride) {
    if (!stride) return 0;
    FEngStatePoolClass *node = (FEngStatePoolClass *)classes0.first8;
    while (node && node->stride1C != stride) node = node->next4;
    if (!node) {
        node = (FEngStatePoolClass *)FEngAllocateQueueNode(0x20);
        if (node) {
            node->next4 = (FEngStatePoolClass *)0xabadcafe;
            node->previous8 = (FEngStatePoolClass *)0xabadcafe;
            node->vtable0 = (void *)0x79b8dc;
            node->blocksC.value0 = 0x79a9ec;
            node->blocksC.first8 = 0;
            node->blocksC.valueC = 0;
            node->blocksC.count4 = 0;
            node->stride1C = stride;
        }
        if (node) {
            FEngStatePoolClass *first = (FEngStatePoolClass *)classes0.first8;
            node->next4 = first;
            if (first) first->previous8 = node;
            node->previous8 = 0;
            classes0.first8 = (FEngQueueNode *)node;
            if (!classes0.valueC) classes0.valueC = (FEngQueueNode *)node;
            ++classes0.count4;
        }
    }
    return node->Acquire();
}

// Original 0x5173b0, normal return: free data14 unconditionally, including null.
void FEngStatePoolBlock::Destruct() {
    vtable0 = (void *)0x79b13c;
    FEngFreeQueueNode(data14);
    vtable0 = (void *)0x79a9e8;
}
FEngStatePoolBlock *FEngStatePoolBlock::Destroy(unsigned int flags) { // 0x517390
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}

// Original 0x534ac0; no initial rewrite of the class's own vtable word.
void FEngStatePoolClass::Destruct() {
    blocksC.value0 = 0x79a9ec;
    blocksC.ClearOwned();
    vtable0 = (void *)0x79a9e8;
}
FEngStatePoolClass *FEngStatePoolClass::Destroy(unsigned int flags) { // 0x534aa0
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
