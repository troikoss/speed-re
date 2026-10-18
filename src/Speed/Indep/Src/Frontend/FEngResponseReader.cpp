#include "FEngResponseReader.h"
#include "FEngResponsePool.h"
#include "FEngResponseArray.h"

FEResponse *HudVisibilityObject::FindResponseByMessage(unsigned int message) { // Original 0x5046d0
    FEResponse *response = responses3C;
    while (response && response->messageC != message) response = response->next4;
    return response;
}

void FEPackage::AppendResponse(FEResponse *response) { // Original 0x512120
    FEResponse *cursor = responseCursor60;
    if (response) {
        if (cursor) {
            FEResponse *next = cursor->next4;
            response->next4 = next;
            if (next) next->previous8 = response;
            response->previous8 = cursor;
            cursor->next4 = response;
        } else {
            FEResponse *first = responses5C;
            response->next4 = first;
            if (first) first->previous8 = response;
            response->previous8 = 0;
            responses5C = response;
        }
        if (responseCursor60 == cursor) responseCursor60 = response;
        ++responseCount58;
    }
}

// Original 0x54d1d0. This method trusts tags, payload widths, command indices
// and byte-string termination. Bounds/validation belong to the native adapter.
unsigned char FEngResponseReader::Read(const void *data, unsigned int size, unsigned char packageMode) {
    const unsigned short *property = (const unsigned short *)data;
    unsigned int end = (unsigned int)data + size;
    FEResponse *response = 0;
    FEResponseCommand *command = 0;
    unsigned int index = 0xffffffff;
    while ((unsigned int)property < end) {
        unsigned int tag = property[0];
        const unsigned int *payload = (const unsigned int *)(property + 2);
        if (tag == 0x694d) {
            unsigned int message = *payload;
            if (!packageMode && value18 && (response = objectC->FindResponseByMessage(message)) != 0) {
                FEResponseCommand *old = response->commands14;
                if (old) {
                    unsigned int *base = (unsigned int *)old - 1;
                    FEngDestructArrayWithCleanup(old, 12, *base, (FEngLifetimeDestructor)FEngDestructResponseCommand);
                    FEngFreeQueueNode(base);
                }
                response->commands14 = 0;
                response->commandCount10 = 0;
            } else {
                response = FEngResponseHeaderPool.Acquire();
                response->next4 = (FEResponse *)0xabadcafe;
                response->previous8 = (FEResponse *)0xabadcafe;
                *(unsigned int *)response->unidentified0 = 0x79aa38;
                response->messageC = 0;
                response->commandCount10 = 0;
                response->commands14 = 0;
                response->messageC = message;
                if (packageMode) package0->AppendResponse(response);
                else ((FEngQueueList *)((unsigned char *)objectC + 0x34))->InsertAtCursor((FEngQueueNode *)response);
            }
            index = 0xffffffff;
        } else if (tag == 0x434d) {
            response->ResizeCommands(*payload);
        } else if (tag == 0x6952) {
            ++index;
            command = (FEResponseCommand *)((unsigned int)response->commands14 + index * 12);
            command->type = *payload;
        } else if (tag == 0x7352) {
            command->SetString((const char *)payload);
        } else if (tag == 0x7452) {
            command->target = *payload;
        } else if (tag == 0x7552) {
            command->parameter = *payload;
        }
        property = (const unsigned short *)((unsigned int)property + property[1] + 4);
    }
    return 1;
}
