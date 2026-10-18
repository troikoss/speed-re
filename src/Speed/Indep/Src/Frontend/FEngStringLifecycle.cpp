#include "FEngObjectFactory.h"

// Original 0x4956a0, normal-return body. Preserve label free before text teardown
// and the final base call. Compiler SEH registration/unwind is separate evidence.
void FEStringObject::DestructString() {
    *(unsigned int *)unidentified0 = 0x78f53c;
    FEngFreeQueueNode(*(void **)(unidentified58 + 4));
    ((FEngTextStorage *)&text64)->Destruct();
    DestructBase();
}

FEStringObject *FEStringObject::DestroyString(unsigned int flags) { // Original 0x495780
    DestructString();
    if (flags & 1) FEngFreeQueueNode(this);
    return this;
}
