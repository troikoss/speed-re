#include "FEngScriptPool.h"
#include "FEngResponseArray.h"

typedef void (__thiscall *FEngPoolDeletingDestructor)(void *, unsigned int);

FEngScriptPoolBlock *FEngScriptPoolBlock::Construct() { // Original 0x53e910
    next4 = (FEngScriptPoolBlock *)0xabadcafe;
    previous8 = (FEngScriptPoolBlock *)0xabadcafe;
    vtable0 = (void *)0x79bf54;
    FEngConstructArrayWithCleanup(headersC, 0x34, 32, FEngConstructScript, FEngDestructScript);
    free68C.value0 = 0x79a9ec;
    free68C.first8 = 0;
    free68C.valueC = 0;
    free68C.count4 = 0;
    used69C = 0;
    // The retail compiler unrolls four insertions. Their ordered pointer stores
    // are retained here; no out-of-line Insert call exists in this constructor.
    for (unsigned int i = 0; i < 32; ++i) {
        FEngQueueNode *node = (FEngQueueNode *)(headersC + i);
        FEngQueueNode *cursor = free68C.valueC;
        if (node) {
            if (cursor) {
                FEngQueueNode *next = cursor->next4;
                node->next4 = next;
                if (next) next->previous8 = node;
                node->previous8 = cursor;
                cursor->next4 = node;
            } else {
                FEngQueueNode *first = free68C.first8;
                node->next4 = first;
                if (first) first->previous8 = node;
                node->previous8 = 0;
                free68C.first8 = node;
            }
            if (free68C.valueC == cursor) free68C.valueC = node;
            ++free68C.count4;
        }
    }
    return this;
}

FEScript *FEngScriptPool::Acquire() { // Original 0x54beb0
    FEngScriptPoolBlock *block = (FEngScriptPoolBlock *)blocks0.first8;
    while (block && !block->free68C.count4) block = block->next4;
    if (!block) {
        block = (FEngScriptPoolBlock *)FEngAllocateQueueNode(0x6a0);
        if (block) block = block->Construct();
        if (block) {
            FEngScriptPoolBlock *first = (FEngScriptPoolBlock *)blocks0.first8;
            block->next4 = first;
            if (first) first->previous8 = block;
            block->previous8 = 0;
            blocks0.first8 = (FEngQueueNode *)block;
            if (!blocks0.valueC) blocks0.valueC = (FEngQueueNode *)block;
            ++blocks0.count4;
        }
    }
    FEScript *header = (FEScript *)block->free68C.first8;
    if (header) block->free68C.Remove((FEngQueueNode *)header);
    ++block->used69C;
    return header;
}

void FEngScriptPool::Release(void *header) { // Original 0x525b90
    FEngScriptPoolBlock *block = (FEngScriptPoolBlock *)blocks0.first8;
    unsigned int address = (unsigned int)header;
    while (block) {
        if (address >= (unsigned int)block + 12 && address < (unsigned int)block + 0x68c) break;
        block = block->next4;
    }
    if (block) {
        block->free68C.InsertAtCursor((FEngQueueNode *)header);
        --block->used69C;
        if (!block->used69C) {
            blocks0.Remove((FEngQueueNode *)block);
            ((FEngPoolDeletingDestructor *)block->vtable0)[0](block, 1);
        }
    }
}

void FEngScriptPoolBlock::Destruct() { // Original 0x53ec40, normal return
    vtable0 = (void *)0x79bf54;
    while (free68C.count4) {
        FEngQueueNode *node = free68C.first8;
        if (node) {
            free68C.first8 = node->next4;
            if (node == free68C.valueC) free68C.valueC = node->previous8;
            if (node->previous8) node->previous8->next4 = node->next4;
            if (node->next4) node->next4->previous8 = node->previous8;
            node->next4 = (FEngQueueNode *)0xabadcafe;
            node->previous8 = (FEngQueueNode *)0xabadcafe;
            --free68C.count4;
        }
    }
    free68C.value0 = 0x79a9ec;
    free68C.ClearOwned();
    FEngDestructArrayWithCleanup(headersC, 0x34, 32, FEngDestructScript);
    vtable0 = (void *)0x79a9e8;
}
FEngScriptPoolBlock *FEngScriptPoolBlock::Destroy(unsigned int flags) { // Original 0x53ec20
    Destruct();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}

FEScript *FEScript::Destroy(unsigned int flags) { // Original 0x53ee30
    FEngDestructScript(this);
    if (flags & 1) {
        ((FEngPoolDeletingDestructor *)vtable0)[0](this, 0);
        FEngScriptHeaderPool.Release(this);
    }
    return this;
}
