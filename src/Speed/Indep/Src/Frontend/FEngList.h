#ifndef UG2_FENG_LIST_H
#define UG2_FENG_LIST_H
#include "../Hud/HudVisibility.h"

struct FEListAxisEntry {
    float value0, value4, value8;
};
// Observed type-4 prefix, allocated as 0xac bytes by 0x534df0.
struct FEListObject {
    char unidentified0[0x5c];
    unsigned int value5C, count60, count64;
    float value68, value6C;
    volatile float value70, value74; // Original wrap loops store/reload at single precision.
    FEListAxisEntry *value78, *value7C;
    float value80, value84;
    unsigned int value88, value8C;
    void *value90;
    volatile float value94, value98;
    float value9C, valueA0, valueA4, valueA8;
    FEListObject *Construct(); // 0x5349c0
    void ReleaseCells(); // 0x5042e0
    void ReleaseStorage(); // 0x516dc0
    void Destruct(); // 0x534a50, normal return
    FEListObject *Destroy(unsigned int flags); // 0x541d90
    void SetColumn(unsigned int value); // 0x504180
    void SetRow(unsigned int value); // 0x5041b0
    void AdvanceCell(); // 0x504230
    void SetWrap(unsigned char value); // 0x504270
    void CalculateOffsets(); // 0x504360
    void FinishMotion(); // 0x5043b0
    void Tick(float milliseconds); // 0x517230
    void Navigate(int x, int y); // 0x516e90, thiscall
};
UG2_CHECK_SIZE(FEListAxisEntry, 0xc);
UG2_CHECK_OFFSET(FEListObject, value5C, 0x5c);
UG2_CHECK_OFFSET(FEListObject, value78, 0x78);
UG2_CHECK_OFFSET(FEListObject, value88, 0x88);
UG2_CHECK_OFFSET(FEListObject, value94, 0x94);
UG2_CHECK_OFFSET(FEListObject, valueA0, 0xa0);
UG2_CHECK_SIZE(FEListObject, 0xac);
#endif
