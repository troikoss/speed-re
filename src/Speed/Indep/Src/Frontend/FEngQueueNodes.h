#ifndef UG2_FENG_QUEUE_NODES_H
#define UG2_FENG_QUEUE_NODES_H
#include "../Hud/HudVisibility.h"
struct FEPackage;
struct FEngQueueNode {
    void *vtable0;
    FEngQueueNode *next4;
    FEngQueueNode *previous8;
    unsigned int valueC, value10, value14, value18, value1C;
};
struct FEngQueueList {
    unsigned int value0;
    unsigned int count4;
    FEngQueueNode *first8;
    FEngQueueNode *valueC;
    void Remove(FEngQueueNode *node); // 0x505400
};
// Only virtual slot +0x54 is established by recovered message posting.
struct FEngQueueObserver {
    virtual void Unidentified00() = 0;
    virtual void Unidentified04() = 0;
    virtual void Unidentified08() = 0;
    virtual void Unidentified0C() = 0;
    virtual void Unidentified10() = 0;
    virtual void Unidentified14() = 0;
    virtual void Unidentified18() = 0;
    virtual void Unidentified1C() = 0;
    virtual void Unidentified20() = 0;
    virtual void Unidentified24() = 0;
    virtual void Unidentified28() = 0;
    virtual void Unidentified2C() = 0;
    virtual void Unidentified30() = 0;
    virtual void Unidentified34() = 0;
    virtual void Unidentified38() = 0;
    virtual void Unidentified3C() = 0;
    virtual void Unidentified40() = 0;
    virtual void Unidentified44() = 0;
    virtual void Unidentified48() = 0;
    virtual void Unidentified4C() = 0;
    virtual void Unidentified50() = 0;
    virtual void MessagePosted(unsigned int message, void *sender, FEPackage *package, int target, int mask) = 0;
};
void *FEngAllocateQueueNode(unsigned int size); // original allocation boundary 0x575620
UG2_CHECK_SIZE(FEngQueueNode, 0x20);
UG2_CHECK_SIZE(FEngQueueList, 0x10);
UG2_CHECK_OFFSET(FEngQueueNode, value1C, 0x1c);
UG2_CHECK_OFFSET(FEngQueueList, first8, 8);
#endif
