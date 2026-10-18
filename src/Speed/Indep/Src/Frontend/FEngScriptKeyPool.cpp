#include "FEngScriptKeyPool.h"
#include "FEngResponseArray.h"

typedef void (__thiscall *FEngPoolDeletingDestructor)(void *, unsigned int);

FEngScriptKeyPoolBlock *FEngScriptKeyPoolBlock::Construct() { // Original 0x5319c0
    next4 = (FEngScriptKeyPoolBlock *)0xabadcafe;
    previous8 = (FEngScriptKeyPoolBlock *)0xabadcafe;
    vtable0 = (void *)0x79b9ec;
    FEngConstructArrayWithCleanup(keysC, 32, 256, FEngConstructScriptKey, FEngDestructScriptKey);
    free200C.value0 = 0x79a9ec;
    free200C.first8 = 0;
    free200C.valueC = 0;
    free200C.count4 = 0;
    used201C = 0;
    // The retail compiler unrolls four insertions. Their ordered pointer stores
    // are retained here; no out-of-line Insert call exists in this constructor.
    for (unsigned int i = 0; i < 256; ++i) {
        FEngQueueNode *node = (FEngQueueNode *)(keysC + i);
        FEngQueueNode *cursor = free200C.valueC;
        if (node) {
            if (cursor) {
                FEngQueueNode *next = cursor->next4;
                node->next4 = next;
                if (next) next->previous8 = node;
                node->previous8 = cursor;
                cursor->next4 = node;
            } else {
                FEngQueueNode *first = free200C.first8;
                node->next4 = first;
                if (first) first->previous8 = node;
                node->previous8 = 0;
                free200C.first8 = node;
            }
            if (free200C.valueC == cursor) free200C.valueC = node;
            ++free200C.count4;
        }
    }
    return this;
}

FEngScriptKey *FEngScriptKeyPool::Acquire() { // Original 0x53e650
    FEngScriptKeyPoolBlock *block = (FEngScriptKeyPoolBlock *)blocks0.first8;
    while (block && !block->free200C.count4) block = block->next4;
    if (!block) {
        block = (FEngScriptKeyPoolBlock *)FEngAllocateQueueNode(0x2020);
        if (block) block = block->Construct();
        if (block) {
            FEngScriptKeyPoolBlock *first = (FEngScriptKeyPoolBlock *)blocks0.first8;
            block->next4 = first;
            if (first) first->previous8 = block;
            block->previous8 = 0;
            blocks0.first8 = (FEngQueueNode *)block;
            if (!blocks0.valueC) blocks0.valueC = (FEngQueueNode *)block;
            ++blocks0.count4;
        }
    }
    FEngScriptKey *header = (FEngScriptKey *)block->free200C.first8;
    if (header) block->free200C.Remove((FEngQueueNode *)header);
    ++block->used201C;
    return header;
}

void FEngScriptKeyPool::Release(void *header) { // Original 0x525bf0
    FEngScriptKeyPoolBlock *block = (FEngScriptKeyPoolBlock *)blocks0.first8;
    unsigned int address = (unsigned int)header;
    while (block) {
        if (address >= (unsigned int)block + 12 && address < (unsigned int)block + 0x200c) break;
        block = block->next4;
    }
    if (block) {
        block->free200C.InsertAtCursor((FEngQueueNode *)header);
        --block->used201C;
        if (!block->used201C) {
            blocks0.Remove((FEngQueueNode *)block);
            ((FEngPoolDeletingDestructor *)block->vtable0)[0](block, 1);
        }
    }
}

void FEngScriptKeyPoolBlock::Destruct() { // Original 0x531be0, normal return
    vtable0 = (void *)0x79b9ec;
    while (free200C.count4) {
        FEngQueueNode *node = free200C.first8;
        if (node) {
            free200C.first8 = node->next4;
            if (node == free200C.valueC) free200C.valueC = node->previous8;
            if (node->previous8) node->previous8->next4 = node->next4;
            if (node->next4) node->next4->previous8 = node->previous8;
            node->next4 = (FEngQueueNode *)0xabadcafe;
            node->previous8 = (FEngQueueNode *)0xabadcafe;
            --free200C.count4;
        }
    }
    free200C.value0 = 0x79a9ec;
    free200C.ClearOwned();
    FEngDestructArrayWithCleanup(keysC, 32, 256, FEngDestructScriptKey);
    vtable0 = (void *)0x79a9e8;
}
FEngScriptKeyPoolBlock *FEngScriptKeyPoolBlock::Destroy(unsigned int flags) { // Original 0x531bc0
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
