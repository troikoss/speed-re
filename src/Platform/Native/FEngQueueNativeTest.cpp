// Asset-free native ABI/integration probe; synthetic package prefixes are test scaffolding.
#include "../../Speed/Indep/Src/Frontend/FEngPackage.h"
#include "../../Speed/Indep/Src/Frontend/FEngResponse.h"
#include "../../Speed/Indep/Src/Frontend/FEngList.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

static FEngManager manager;
FEngManager *fengManager8384C4 = &manager;
static unsigned int allocations, deallocations, responseCalls;
static unsigned int deliveredMask;
static HudVisibilityObject *deliveredObject;
static FEPackage *deliveredPackage;
void *FEngAllocateQueueNode(unsigned int size) { ++allocations; return malloc(size); }
void FEngFreeQueueNode(void *pointer) { ++deallocations; free(pointer); }
void FEngQueue::Call5362E0(FEPackage *) { abort(); }
FEPackage *FEngQueue::Call555860(const char *, unsigned char, unsigned int) { abort(); }
void HudVisibilityObject::Call52BD70(int, int) { abort(); }
void FEngQueue::ExecuteResponse(FEResponse *, HudVisibilityObject *object, FEPackage *package, unsigned int mask) {
    ++responseCalls; deliveredMask = mask; deliveredObject = object; deliveredPackage = package;
}
static FEngQueueNode *__thiscall DeleteMessage(FEngQueueNode *node, unsigned int flags) {
    return node->DestroyMessage(flags);
}
static FEngQueueNode *__thiscall DeleteOperation(FEngQueueNode *node, unsigned int flags) {
    return node->DestroyOperation(flags);
}
static void CheckMasks(FEPackage &package, unsigned int expected) {
    assert(package.inputMask1C == expected && package.inputMask20 == expected &&
           package.inputMask24 == expected && package.inputMask28 == expected);
}
int main() {
    void *page = mmap((void *)0x79a000, 4096, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    assert(page == (void *)0x79a000);
    *(void **)0x79ab60 = (void *)&DeleteMessage;
    *(void **)0x79ab64 = (void *)&DeleteOperation;
    FEngQueue queue = {};
    FEPackage parent = {}, child = {}, laterChild = {};
    parent.next4 = &child; child.next4 = &laterChild;
    child.parent3C = laterChild.parent3C = &parent;
    queue.packagesDC.first8 = &parent;
    FEResponse packageResponse = {}, objectResponse = {};
    packageResponse.messageC = objectResponse.messageC = 0x334c5493;
    HudVisibilityObject object = {};
    object.type18 = 1; object.responses3C = &objectResponse;
    HudVisibilityObject *targets[] = {&object};
    FEngMessageAssociation association = {0x334c5493, 1, 1, targets};
    parent.responses5C = child.responses5C = &packageResponse;
    parent.associationCount6C = child.associationCount6C = 1;
    parent.associations70 = child.associations70 = &association;
    manager.queue8 = &queue;
    manager.names10.next0 = manager.names10.previous4 = &manager.names10;
    parent.inputMask1C = 0; child.inputMask1C = 0xff;
    queue.Call51AF50(&child, 0x100, 0x55); // low-byte zero -> opcode4
    queue.ConsumeOperations();
    CheckMasks(child, 0xaa); CheckMasks(parent, 0x55);
    assert(queue.operations4124.count4 == 0 && !queue.operations4124.first8 && !queue.operations4124.valueC);
    FEngQueueNode *notification = queue.messages4114.first8;
    assert(notification && notification->value18 == 0x334c5493 && notification->value14 == (unsigned int)&parent);
    assert(notification->value10 == 0xfffffffc && notification->value1C == 0x55);
    queue.DeliverMessages();
    assert(deliveredObject == &object);
    assert(responseCalls == 2 && deliveredPackage == &parent && deliveredMask == 0x55);
    queue.Call51AF50(&parent, 0x101, 0x11); // opcode8 -> first matching child only
    queue.ConsumeOperations();
    CheckMasks(parent, 0x44); CheckMasks(child, 0xbb);
    CheckMasks(laterChild, 0);
    notification = queue.messages4114.first8;
    assert(notification && notification->value14 == (unsigned int)&child && notification->value1C == 0x11);
    queue.DeliverMessages();
    assert(deliveredObject == &object);
    assert(responseCalls == 4 && deliveredPackage == &child && deliveredMask == 0x11);
    queue.Call51AF50(&child, 0, 0);
    queue.ConsumeOperations();
    CheckMasks(parent, 0x44); CheckMasks(child, 0xbb);
    notification = queue.messages4114.first8;
    assert(notification && notification->value1C == 0 && notification->value14 == (unsigned int)&parent);
    queue.DeliverMessages();
    assert(deliveredObject == &object);
    assert(responseCalls == 6 && deliveredPackage == &parent && deliveredMask == 0);
    assert(allocations == 6 && deallocations == 6 && queue.messages4114.count4 == 0);
    // Compose original special-handler delivery with the complete native leaf.
    FEListObject list = {};
    reinterpret_cast<HudVisibilityObject *>(&list)->type18 = 4;
    FEListAxisEntry xrows[] = {{.25f, 0, 0}, {.5f, 2, 0}, {.75f, 4, 0}};
    FEListAxisEntry yrows[] = {{1, 0, 0}, {1, 3, 0}};
    list.value5C = 4; list.count60 = 3; list.count64 = 2;
    list.value78 = xrows; list.value7C = yrows;
    queue.DeliverObject(reinterpret_cast<HudVisibilityObject *>(&list), &parent, 0x030471ac, 0x55);
    assert(list.value88 == 1 && list.value94 == 2 && list.value9C == 1 && list.valueA0 == 0);
    queue.DeliverObject(reinterpret_cast<HudVisibilityObject *>(&list), &parent, 0x030471ac, 0x55);
    assert(list.value88 == 1); // Busy horizontal axis ignores another right message.
    queue.DeliverObject(reinterpret_cast<HudVisibilityObject *>(&list), &parent, 0xe10814a6, 0x55);
    assert(list.value8C == 1 && list.value98 == 3 && (list.value5C & 0x60) == 0x60);
    assert(list.value9C > .5546f && list.value9C < .5548f);
    assert(list.valueA0 > .8319f && list.valueA0 < .8322f);
    assert(responseCalls == 6); // Recognized type-4 messages stop before response lookup.
    munmap(page, 4096);
    puts("PASS recovered queue delivery/associations/deletion and type-4 navigation (native ABI, no assets)");
}
