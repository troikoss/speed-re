// Asset-free native ABI/integration probe; synthetic package prefixes are test scaffolding.
#include "../../Speed/Indep/Src/Frontend/FEngPackage.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

static FEngManager manager;
FEngManager *fengManager8384C4 = &manager;
static unsigned int allocations, deallocations;
void *FEngAllocateQueueNode(unsigned int size) { ++allocations; return malloc(size); }
void FEngFreeQueueNode(void *pointer) { ++deallocations; free(pointer); }
void FEngQueue::Call5362E0(FEPackage *) { abort(); }
FEPackage *FEngQueue::Call555860(const char *, unsigned char, unsigned int) { abort(); }
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
    *(void **)0x79ab64 = (void *)&DeleteOperation;
    FEngQueue queue = {};
    FEPackage parent = {}, child = {}, laterChild = {};
    parent.next4 = &child; child.next4 = &laterChild;
    child.parent3C = laterChild.parent3C = &parent;
    queue.packagesDC.first8 = &parent;
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
    queue.messages4114.Remove(notification); notification->DestroyMessage(1);
    queue.Call51AF50(&parent, 0x101, 0x11); // opcode8 -> first matching child only
    queue.ConsumeOperations();
    CheckMasks(parent, 0x44); CheckMasks(child, 0xbb);
    CheckMasks(laterChild, 0);
    notification = queue.messages4114.first8;
    assert(notification && notification->value14 == (unsigned int)&child && notification->value1C == 0x11);
    queue.messages4114.Remove(notification); notification->DestroyMessage(1);
    queue.Call51AF50(&child, 0, 0);
    queue.ConsumeOperations();
    CheckMasks(parent, 0x44); CheckMasks(child, 0xbb);
    notification = queue.messages4114.first8;
    assert(notification && notification->value1C == 0 && notification->value14 == (unsigned int)&parent);
    queue.messages4114.Remove(notification); notification->DestroyMessage(1);
    assert(allocations == 6 && deallocations == 6 && queue.messages4114.count4 == 0);
    munmap(page, 4096);
    puts("PASS recovered operation enqueue/consume/mask transfer/notification/deletion (native ABI, no assets)");
}
