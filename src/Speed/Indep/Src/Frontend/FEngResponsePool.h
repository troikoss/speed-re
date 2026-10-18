#ifndef UG2_FENG_RESPONSE_POOL_H
#define UG2_FENG_RESPONSE_POOL_H
#include "FEngResponse.h"
#include "FEngQueueNodes.h"

struct FEngResponsePoolBlock {
    void *vtable0;
    FEngResponsePoolBlock *next4, *previous8;
    FEResponse headersC[64];
    FEngQueueList free60C;
    unsigned int used61C;
    FEngResponsePoolBlock *Construct(); // 0x53e710
    void Destruct(); // 0x53eb40, normal-return path
    FEngResponsePoolBlock *Destroy(unsigned int flags); // 0x53eb20
};
struct FEngResponsePool {
    FEngQueueList blocks0;
    FEResponse *Acquire(); // 0x54bdf0
    void Release(void *header); // 0x525b30; membership is unsigned half-open, no alignment check.
};
extern FEngResponsePool FEngResponseHeaderPool; // Original 0x8637a0; native storage is relocated.
UG2_CHECK_OFFSET(FEngResponsePoolBlock, headersC, 0xc);
UG2_CHECK_OFFSET(FEngResponsePoolBlock, free60C, 0x60c);
UG2_CHECK_OFFSET(FEngResponsePoolBlock, used61C, 0x61c);
UG2_CHECK_SIZE(FEngResponsePoolBlock, 0x620);
#endif
