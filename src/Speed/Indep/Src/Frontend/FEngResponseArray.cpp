#include "FEngResponseArray.h"
#include "FEngQueueNodes.h"

// Original 0x52ae00, normal-return body. Preserve cookie counts, fresh field
// loads and captured allocation bases across iterator/heap calls.
void FEResponse::ResizeCommands(unsigned int count) {
    if (count != commandCount10) {
        if (!count) {
            FEResponseCommand *old = commands14;
            if (old) {
                unsigned int *base = (unsigned int *)old - 1;
                FEngDestructResponseCommands(old, 12, *base, FEngDestructResponseCommand);
                FEngFreeQueueNode(base);
            }
            commands14 = 0;
            commandCount10 = 0;
        } else {
            unsigned int *allocation = (unsigned int *)FEngAllocateQueueNode(count * 12 + 4);
            FEResponseCommand *buffer = 0;
            if (allocation) {
                *allocation = count;
                buffer = (FEResponseCommand *)(allocation + 1);
                FEngConstructResponseCommands(buffer, 12, count,
                    FEngConstructResponseCommand, FEngDestructResponseCommand);
            }
            unsigned int retained = commandCount10;
            if (count < retained) retained = count;
            unsigned int offset = 0;
            while (retained) {
                ((FEResponseCommand *)((unsigned int)buffer + offset))->Assign(
                    (FEResponseCommand *)((unsigned int)commands14 + offset));
                offset += 12;
                --retained;
            }
            FEResponseCommand *old = commands14;
            if (old) {
                unsigned int *base = (unsigned int *)old - 1;
                FEngDestructResponseCommands(old, 12, *base, FEngDestructResponseCommand);
                FEngFreeQueueNode(base);
            }
            commands14 = buffer;
            commandCount10 = count;
        }
    }
}
