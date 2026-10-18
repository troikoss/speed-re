#ifndef UG2_FENG_SCRIPT_H
#define UG2_FENG_SCRIPT_H
#include "../Hud/HudVisibility.h"

struct FEScriptEvent {
    unsigned int message, target;
    int time;
};
struct FEngScriptEventStorage {
    int count0;
    FEScriptEvent *data4;
    void Resize(int count); // 0x519a10; receiver is script+0x24, not script itself.
};
UG2_CHECK_SIZE(FEScriptEvent, 12);
UG2_CHECK_OFFSET(FEScriptEvent, time, 8);
UG2_CHECK_SIZE(FEngScriptEventStorage, 8);

struct FEngScriptKey {
    void *vtable0;
    FEngScriptKey *next4, *previous8;
    int timeC;
    unsigned char value10[16];
    FEngScriptKey *Destroy(unsigned int flags); // 0x531fe0
};
UG2_CHECK_SIZE(FEngScriptKey, 0x20);
void *__fastcall FEngConstructScriptKey(void *memory); // 0x4fe8b0
// ECX-only callback ABI view of the already recovered 0x51ab10 body.
void __fastcall FEngDestructScriptKey(void *memory);
void *__fastcall FEngConstructScriptTrack(void *memory); // 0x525e80
void __fastcall FEngDestructScriptTrack(void *memory); // 0x52b960
void *__fastcall FEngConstructScript(void *memory); // 0x511d70
void __fastcall FEngDestructScript(void *memory); // 0x535170

struct FEngReference {
    char unidentified0[4];
    unsigned char value4;
    char unidentified5[3];
    void *value8;
    void *valueC;
    void Remove(FEngScriptKey *key); // 0x5055c0, no count field
    void ClearOwned(); // 0x511d00, actual first Remove call; later inline unlinking
    void *Resolve(); // 0x4fe890
};
struct FEngQuaternion {
    float value0[4];
    FEngQuaternion *Multiply(FEngQuaternion *out, const FEngQuaternion *other); // 0x4fe6b0
    FEngQuaternion *Assign(const FEngQuaternion *other); // 0x4955f0, returns this
};
struct FEngInteger4 {
    unsigned int value0[4];
    FEngInteger4 *Subtract(FEngInteger4 *out, const FEngInteger4 *other); // 0x505580
};
UG2_CHECK_OFFSET(FEngReference, value4, 4);
UG2_CHECK_OFFSET(FEngReference, value8, 8);
UG2_CHECK_SIZE(FEngQuaternion, 16);
UG2_CHECK_SIZE(FEngInteger4, 16);
UG2_CHECK_OFFSET(FEScriptTrack, value2C, 0x2c);
UG2_CHECK_OFFSET(FEScriptTrack, value30, 0x30);
void FngSetObjectScript(void *object, const char *name, unsigned char restart); // 0x51cf70
void FngSetObjectScriptHash(void *object, unsigned int hash, unsigned char restart); // 0x51cfe0
#endif
