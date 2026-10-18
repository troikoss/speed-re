#ifndef UG2_FENG_SCRIPT_POOL_H
#define UG2_FENG_SCRIPT_POOL_H
#include "FEngScript.h"
#include "FEngQueueNodes.h"

struct FEngScriptPoolBlock {
    void *vtable0;
    FEngScriptPoolBlock *next4, *previous8;
    FEScript headersC[32];
    FEngQueueList free68C;
    unsigned int used69C;
    FEngScriptPoolBlock *Construct(); // 0x53e910
    void Destruct(); // 0x53ec40, normal-return path
    FEngScriptPoolBlock *Destroy(unsigned int flags); // 0x53ec20
};
struct FEngScriptPool {
    FEngQueueList blocks0;
    FEScript *Acquire(); // 0x54beb0
    void Release(void *header); // 0x525b90; membership is unsigned half-open, no alignment check.
};
extern FEngScriptPool FEngScriptHeaderPool; // Original 0x839c30; native storage is relocated.
UG2_CHECK_OFFSET(FEngScriptPoolBlock, headersC, 0xc);
UG2_CHECK_OFFSET(FEngScriptPoolBlock, free68C, 0x68c);
UG2_CHECK_OFFSET(FEngScriptPoolBlock, used69C, 0x69c);
UG2_CHECK_SIZE(FEngScriptPoolBlock, 0x6a0);
#endif
