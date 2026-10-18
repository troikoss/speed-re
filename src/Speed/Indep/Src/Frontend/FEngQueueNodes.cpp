#include "FEngPackage.h"

// Original 0x505400: unlink before marking both links, then decrement the count.
void FEngQueueList::Remove(FEngQueueNode *node) {
    if (node) {
        if (node == first8) first8 = node->next4;
        if (node == valueC) valueC = node->previous8;
        if (node->previous8) node->previous8->next4 = node->next4;
        if (node->next4) node->next4->previous8 = node->previous8;
        node->next4 = (FEngQueueNode *)0xabadcafe;
        node->previous8 = (FEngQueueNode *)0xabadcafe;
        --count4;
    }
}

// Original 0x51aa20: callback observes the constructed payload before insertion.
void FEngQueue::Post(unsigned int message, void *sender, FEPackage *package, int target, int mask) {
    FEngQueueNode *node = (FEngQueueNode *)FEngAllocateQueueNode(0x20);
    if (node) {
        node->next4 = (FEngQueueNode *)0xabadcafe;
        node->previous8 = (FEngQueueNode *)0xabadcafe;
        node->vtable0 = (void *)0x79ab60;
    }
    node->value14 = (unsigned int)package;
    node->value18 = message;
    node->valueC = (unsigned int)sender;
    node->value10 = (unsigned int)target;
    node->value1C = (unsigned int)mask;
    if (value5251) value10C->MessagePosted(message, sender, package, target, mask);
    FEngQueueNode *cursor = messages4114.valueC;
    if (cursor) {
        FEngQueueNode *next = cursor->next4;
        node->next4 = next;
        if (next) next->previous8 = node;
        node->previous8 = cursor;
        cursor->next4 = node;
    } else {
        FEngQueueNode *first = messages4114.first8;
        node->next4 = first;
        if (first) first->previous8 = node;
        node->previous8 = 0;
        messages4114.first8 = node;
    }
    if (messages4114.valueC == cursor) messages4114.valueC = node;
    ++messages4114.count4;
}

// Original 0x51af50: byte-valued enable selects opcode 4/8 on a separate list.
void FEngQueue::Call51AF50(FEPackage *package, int value, unsigned int mask) {
    FEngQueueNode *node = (FEngQueueNode *)FEngAllocateQueueNode(0x20);
    if (node) {
        node->next4 = (FEngQueueNode *)0xabadcafe;
        node->previous8 = (FEngQueueNode *)0xabadcafe;
        node->valueC = 0;
        node->value10 = 0;
        node->vtable0 = (void *)0x79ab64;
    }
    node->value1C = (unsigned int)package;
    node->value18 = package->inputMask1C & mask;
    node->value14 = (unsigned char)value ? 8 : 4;
    FEngQueueNode *cursor = operations4124.valueC;
    if (cursor) {
        FEngQueueNode *next = cursor->next4;
        node->next4 = next;
        if (next) next->previous8 = node;
        node->previous8 = cursor;
        cursor->next4 = node;
    } else {
        FEngQueueNode *first = operations4124.first8;
        node->next4 = first;
        if (first) first->previous8 = node;
        node->previous8 = 0;
        operations4124.first8 = node;
    }
    if (operations4124.valueC == cursor) operations4124.valueC = node;
    ++operations4124.count4;
}
