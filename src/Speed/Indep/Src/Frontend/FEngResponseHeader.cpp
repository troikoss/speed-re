#include "FEngResponsePool.h"
#include "FEngResponseArray.h"

typedef void (__thiscall *FEngResponseDeletingDestructor)(void *, unsigned int);

void *__fastcall FEngConstructResponseHeader(void *memory) { // Original 0x4fe920
    FEResponse *header = (FEResponse *)memory;
    header->next4 = (FEResponse *)0xabadcafe;
    header->previous8 = (FEResponse *)0xabadcafe;
    *(unsigned int *)header->unidentified0 = 0x79aa38;
    header->messageC = 0;
    header->commandCount10 = 0;
    header->commands14 = 0;
    return memory;
}

// Original 0x534b10, ECX-only normal-return destructor body.
void __fastcall FEngDestructResponseHeader(void *memory) {
    FEResponse *header = (FEResponse *)memory;
    *(unsigned int *)header->unidentified0 = 0x79aa38;
    FEResponseCommand *commands = header->commands14;
    if (commands) {
        unsigned int *base = (unsigned int *)commands - 1;
        FEngDestructArrayWithCleanup(commands, 12, *base, (FEngLifetimeDestructor)FEngDestructResponseCommand);
        FEngFreeQueueNode(base);
    }
    header->commands14 = 0;
    header->commandCount10 = 0;
    *(unsigned int *)header->unidentified0 = 0x79a9e8;
}

FEResponse *FEResponse::Destroy(unsigned int flags) { // Original 0x53ee60
    FEngDestructResponseHeader(this);
    if (flags & 1) {
        ((FEngResponseDeletingDestructor *)*(void **)unidentified0)[0](this, 0);
        FEngResponseHeaderPool.Release(this);
    }
    return this;
}
