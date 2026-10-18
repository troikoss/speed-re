#ifndef UG2_FENG_SCRIPT_KEY_POOL_H
#define UG2_FENG_SCRIPT_KEY_POOL_H
#include "FEngScript.h"
#include "FEngQueueNodes.h"

struct FEngScriptKeyPoolBlock {
    void *vtable0;
    FEngScriptKeyPoolBlock *next4, *previous8;
    FEngScriptKey keysC[256];
    FEngQueueList free200C;
    unsigned int used201C;
    FEngScriptKeyPoolBlock *Construct(); // 0x5319c0
    void Destruct(); // 0x531be0, normal-return path
    FEngScriptKeyPoolBlock *Destroy(unsigned int flags); // 0x531bc0
};
struct FEngScriptKeyPool {
    FEngQueueList blocks0;
    FEngScriptKey *Acquire(); // 0x53e650
    void Release(void *header); // 0x525bf0; membership is unsigned half-open, no alignment check.
};
extern FEngScriptKeyPool FEngScriptKeysPool; // Original 0x839bd0; native storage is relocated.
UG2_CHECK_OFFSET(FEngScriptKeyPoolBlock, keysC, 0xc);
UG2_CHECK_OFFSET(FEngScriptKeyPoolBlock, free200C, 0x200c);
UG2_CHECK_OFFSET(FEngScriptKeyPoolBlock, used201C, 0x201c);
UG2_CHECK_SIZE(FEngScriptKeyPoolBlock, 0x2020);
#endif
