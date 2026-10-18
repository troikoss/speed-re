#include "FEngResponsePool.h"
#include "FEngResponseArray.h"

typedef void (__thiscall *FEngPoolDeletingDestructor)(void *, unsigned int);

FEngQueueNode *FEngQueueNode::DestroyBase(unsigned int flags) { // Original 0x4fdb00
    vtable0 = (void *)0x79a9e8;
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
void FEngQueueList::Insert(FEngQueueNode *cursor, FEngQueueNode *node) { // Original 0x5053b0
    if (node) {
        if (cursor) {
            FEngQueueNode *next = cursor->next4;
            node->next4 = next;
            if (next) next->previous8 = node;
            node->previous8 = cursor;
            cursor->next4 = node;
        } else {
            FEngQueueNode *first = first8;
            node->next4 = first;
            if (first) first->previous8 = node;
            node->previous8 = 0;
            first8 = node;
        }
        if (valueC == cursor) valueC = node;
        ++count4;
    }
}
void FEngQueueList::InsertAtCursor(FEngQueueNode *node) { // Original 0x4951b0, thunk
    Insert(valueC, node);
}

FEngResponsePoolBlock *FEngResponsePoolBlock::Construct() { // Original 0x53e710
    next4 = (FEngResponsePoolBlock *)0xabadcafe;
    previous8 = (FEngResponsePoolBlock *)0xabadcafe;
    vtable0 = (void *)0x79bf50;
    FEngConstructArrayWithCleanup(headersC, 24, 64, FEngConstructResponseHeader, FEngDestructResponseHeader);
    free60C.value0 = 0x79a9ec;
    free60C.first8 = 0;
    free60C.valueC = 0;
    free60C.count4 = 0;
    used61C = 0;
    // The retail compiler unrolls four insertions. Their ordered pointer stores
    // are retained here; no out-of-line Insert call exists in this constructor.
    for (unsigned int i = 0; i < 64; ++i) {
        FEngQueueNode *node = (FEngQueueNode *)(headersC + i);
        FEngQueueNode *cursor = free60C.valueC;
        if (node) {
            if (cursor) {
                FEngQueueNode *next = cursor->next4;
                node->next4 = next;
                if (next) next->previous8 = node;
                node->previous8 = cursor;
                cursor->next4 = node;
            } else {
                FEngQueueNode *first = free60C.first8;
                node->next4 = first;
                if (first) first->previous8 = node;
                node->previous8 = 0;
                free60C.first8 = node;
            }
            if (free60C.valueC == cursor) free60C.valueC = node;
            ++free60C.count4;
        }
    }
    return this;
}

FEResponse *FEngResponsePool::Acquire() { // Original 0x54bdf0
    FEngResponsePoolBlock *block = (FEngResponsePoolBlock *)blocks0.first8;
    while (block && !block->free60C.count4) block = block->next4;
    if (!block) {
        block = (FEngResponsePoolBlock *)FEngAllocateQueueNode(0x620);
        if (block) block = block->Construct();
        if (block) {
            FEngResponsePoolBlock *first = (FEngResponsePoolBlock *)blocks0.first8;
            block->next4 = first;
            if (first) first->previous8 = block;
            block->previous8 = 0;
            blocks0.first8 = (FEngQueueNode *)block;
            if (!blocks0.valueC) blocks0.valueC = (FEngQueueNode *)block;
            ++blocks0.count4;
        }
    }
    FEResponse *header = (FEResponse *)block->free60C.first8;
    if (header) block->free60C.Remove((FEngQueueNode *)header);
    ++block->used61C;
    return header;
}

void FEngResponsePool::Release(void *header) { // Original 0x525b30
    FEngResponsePoolBlock *block = (FEngResponsePoolBlock *)blocks0.first8;
    unsigned int address = (unsigned int)header;
    while (block) {
        if (address >= (unsigned int)block + 12 && address < (unsigned int)block + 0x60c) break;
        block = block->next4;
    }
    if (block) {
        block->free60C.InsertAtCursor((FEngQueueNode *)header);
        --block->used61C;
        if (!block->used61C) {
            blocks0.Remove((FEngQueueNode *)block);
            ((FEngPoolDeletingDestructor *)block->vtable0)[0](block, 1);
        }
    }
}

void FEngResponsePoolBlock::Destruct() { // Original 0x53eb40, normal return
    vtable0 = (void *)0x79bf50;
    while (free60C.count4) {
        FEngQueueNode *node = free60C.first8;
        if (node) {
            free60C.first8 = node->next4;
            if (node == free60C.valueC) free60C.valueC = node->previous8;
            if (node->previous8) node->previous8->next4 = node->next4;
            if (node->next4) node->next4->previous8 = node->previous8;
            node->next4 = (FEngQueueNode *)0xabadcafe;
            node->previous8 = (FEngQueueNode *)0xabadcafe;
            --free60C.count4;
        }
    }
    free60C.value0 = 0x79a9ec;
    free60C.ClearOwned();
    FEngDestructArrayWithCleanup(headersC, 24, 64, FEngDestructResponseHeader);
    vtable0 = (void *)0x79a9e8;
}
FEngResponsePoolBlock *FEngResponsePoolBlock::Destroy(unsigned int flags) { // Original 0x53eb20
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
