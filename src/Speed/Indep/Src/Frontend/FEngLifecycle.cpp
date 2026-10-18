#include "FEngLifecycle.h"
#include "FEngGrid.h"

typedef void (__thiscall *FEngDeletingDestructor)(void *, unsigned int);

// Original 0x511670. Preserve the first Remove call and later inline unlinking,
// and re-read first8 after virtual callbacks instead of caching next links.
void FEngQueueList::ClearOwned() {
    FEngQueueNode *node = first8;
    if (node) {
        Remove(node);
        do {
            if (node) ((FEngDeletingDestructor *)node->vtable0)[0](node, 1);
            node = first8;
            if (node) {
                first8 = node->next4;
                if (node == valueC) valueC = node->previous8;
                if (node->previous8) node->previous8->next4 = node->next4;
                if (node->next4) node->next4->previous8 = node->previous8;
                node->next4 = (FEngQueueNode *)0xabadcafe;
                node->previous8 = (FEngQueueNode *)0xabadcafe;
                --count4;
            }
        } while (node);
    }
}

// Original 0x5174a0. Membership is a half-open unsigned 32-slot byte range;
// stride and used count are 16-bit and the slot need not be aligned.
unsigned char FEngStatePoolClass::ReleaseSlot(void *pointer) {
    FEngStatePoolBlock *block = (FEngStatePoolBlock *)blocksC.first8;
    unsigned int address = (unsigned int)pointer;
    while (block) {
        unsigned int base = (unsigned int)block->data14;
        if (base <= address && address < base + (unsigned int)block->strideC * 32) break;
        block = block->next4;
    }
    if (!block) return 0;
    unsigned int slot = (address - (unsigned int)block->data14) / block->strideC;
    --block->usedE;
    block->bitmap10[slot >> 3] &= (unsigned char)~(1u << (slot & 7));
    if (!block->usedE) {
        blocksC.Remove((FEngQueueNode *)block);
        ((FEngDeletingDestructor *)block->vtable0)[0](block, 1);
    }
    return 1;
}

// Original 0x517520. A nonmember nonnull pointer faults at the original null
// class dereference; do not turn that invalid-input path into a silent success.
void FEngStatePoolManager::ReleaseState(void *pointer) {
    if (pointer) {
        FEngStatePoolClass *node = (FEngStatePoolClass *)classes0.first8;
        while (node && !node->ReleaseSlot(pointer)) node = node->next4;
        if (!node->blocksC.count4) {
            classes0.Remove((FEngQueueNode *)node);
            ((FEngDeletingDestructor *)node->vtable0)[0](node, 1);
        }
    }
}

// Original 0x52af90, normal-return body only. The compiler's SEH registration
// and unwind funclets are separate evidence, not reconstructed by this method.
void HudVisibilityObject::DestructBase() {
    *(unsigned int *)unidentified0 = 0x79b8e0;
    if (FEngObjectDestructionObserver) FEngObjectDestructionObserver->ObjectDestroyed(this);
    FEngObjectStateManager.ReleaseState(data2C);
    if (*(unsigned int *)unidentified14) FEngFreeQueueNode((void *)*(unsigned int *)unidentified14);
    *(unsigned int *)(unidentified40 + 4) = 0x79a9ec;
    ((FEngQueueList *)((unsigned char *)this + 0x44))->ClearOwned();
    *(unsigned int *)(unidentified30 + 4) = 0x79a9ec;
    ((FEngQueueList *)((unsigned char *)this + 0x34))->ClearOwned();
    *(unsigned int *)unidentified0 = 0x79a9e8;
}

// Original 0x52baa0, normal-return body. Each pointer reload follows the prior
// free callback; fields retain their pointer bits after release.
void FEGridObject::Destruct() {
    *(unsigned int *)unidentified0 = 0x79b904;
    if (cells8C) FEngGridFreeCells(cells8C);
    if (valueA0) FEngGridFreeCells((void *)valueA0);
    if (value9C) FEngGridFreeCells(value9C);
    ((HudVisibilityObject *)this)->DestructBase();
}
