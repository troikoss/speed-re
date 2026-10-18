#include "FEngObject.h"
#include "FEngGrid.h"

// Original 0x52af20. Do not zero untouched type18/resource22/currentScript54
// or derived fields. The retail vtable words are opaque address tokens natively.
HudVisibilityObject *HudVisibilityObject::ConstructBase() {
    *(unsigned int *)unidentified0 = 0x79b8e0;
    next4 = (HudVisibilityObject *)0xabadcafe;
    previous8 = (HudVisibilityObject *)0xabadcafe;
    hash10 = 0;
    *(unsigned int *)unidentified14 = 0;
    flags1C = 0;
    *(unsigned short *)unidentified20 = 0;
    texture24 = 0;
    value28 = 0;
    data2C = 0;
    *(unsigned int *)unidentified30 = 0;
    responses3C = 0;
    *(unsigned int *)unidentified40 = 0;
    *(unsigned int *)(unidentified30 + 8) = 0;
    *(unsigned int *)(unidentified30 + 4) = 0x79a9ec;
    *(unsigned int *)(unidentified40 + 4) = 0x79a9ec;
    scripts4C = 0;
    *(unsigned int *)unidentified50 = 0;
    *(unsigned int *)(unidentified40 + 8) = 0;
    *(unsigned int *)unidentified58 = 0;
    unsigned int guid = FEngNextObjectGuid;
    FEngNextObjectGuid = guid + 1;
    guidC = guid;
    return this;
}

// Original 0x52b9e0: preserve the base call and copy the opaque +0xbc default.
FEGridObject *FEGridObject::Construct() {
    ((HudVisibilityObject *)this)->ConstructBase();
    owner5C = 0;
    count60 = 0;
    count64 = 0;
    value6C = 0;
    value70 = 0;
    value74 = 0;
    value78 = 0;
    value7C = 0;
    value80 = 0;
    *(unsigned int *)unidentified0 = 0x79b904;
    flags68 = 2;
    value84 = 0;
    value88 = 0;
    cells8C = 0;
    value90 = 0;
    value94 = 0;
    value98 = 0;
    value9C = 0;
    valueA0 = 0;
    *(unsigned int *)unidentifiedA4 = 0x3f800000;
    *(unsigned int *)(unidentifiedA4 + 4) = 0xbab60b61;
    *(unsigned int *)(unidentifiedA4 + 0x14) = 0xff;
    *(unsigned int *)(unidentifiedA4 + 0x10) = 0xff;
    *(unsigned int *)(unidentifiedA4 + 0xc) = 0xff;
    *(unsigned int *)(unidentifiedA4 + 8) = 0xff;
    unsigned int valueBC = FEngValue7F7CC4;
    callbackC0 = 0;
    contextC4 = 0;
    *(unsigned int *)(unidentifiedA4 + 0x18) = valueBC;
    *(unsigned int *)(unidentified0 + 0x18) = 6;
    return this;
}
