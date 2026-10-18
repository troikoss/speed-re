#include "FEngObjectFactory.h"

FEngTextStorage *FEngTextStorage::Construct() { // Original 0x51de80
    text0 = 0;
    capacity4 = 0;
    return this;
}

FEStringObject *FEStringObject::Construct() { // Original 0x495630, normal return
    ConstructBase();
    *(unsigned int *)unidentified0 = 0x78f53c;
    *(unsigned int *)(unidentified58 + 4) = 0;
    languageHash60 = 0xffffffff;
    ((FEngTextStorage *)&text64)->Construct();
    justification6C = 0;
    leading70 = 0;
    maxWidth74 = 0;
    type18 = 2;
    return this;
}

FEngLeafObject *FEngLeafObject::ConstructImage() { // Original 0x532640
    ((HudVisibilityObject *)this)->ConstructBase();
    *(unsigned int *)unidentified0 = 0x79ba0c;
    *(unsigned int *)(unidentified0 + 0x18) = 1;
    value5C = 0;
    return this;
}
FEngGroupObject *FEngGroupObject::Construct() { // Original 0x52b080
    ((HudVisibilityObject *)this)->ConstructBase();
    *(unsigned int *)unidentified0 = 0x79b8e8;
    children5C.first8 = 0;
    children5C.valueC = 0;
    children5C.count4 = 0;
    children5C.value0 = 0x79a9ec;
    *(unsigned int *)(unidentified0 + 0x18) = 5;
    return this;
}
FEngLeafObject *FEngLeafObject::ConstructType7() { // Original 0x52b0b0
    ((HudVisibilityObject *)this)->ConstructBase();
    *(unsigned int *)unidentified0 = 0x79b8f0;
    value5C = 0;
    *(unsigned int *)(unidentified0 + 0x18) = 7;
    return this;
}
FEngLeafObject *FEngLeafObject::ConstructType9() { // Original 0x534b90
    ((HudVisibilityObject *)this)->ConstructBase();
    value5C = 0;
    *(unsigned int *)unidentified0 = 0x79baec;
    *(unsigned int *)(unidentified0 + 0x18) = 9;
    return this;
}
FEngLeafObject *FEngLeafObject::ConstructType10() { // Original 0x534db0
    ((HudVisibilityObject *)this)->ConstructBase();
    value5C = 0;
    *(unsigned int *)unidentified0 = 0x79baf4;
    *(unsigned int *)(unidentified0 + 0x18) = 10;
    return this;
}
FEngType11Object *FEngType11Object::Construct() { // Original 0x52b480
    ((HudVisibilityObject *)this)->ConstructBase();
    *(unsigned int *)unidentified0 = 0x79b8fc;
    *(unsigned int *)(unidentified0 + 0x18) = 11;
    return this;
}

FEListObject *FEListObject::Construct() { // Original 0x5349c0
    ((HudVisibilityObject *)this)->ConstructBase();
    count60 = 0;
    count64 = 0;
    *(unsigned int *)unidentified0 = 0x79bae4;
    value5C = 4;
    value68 = 0;
    value6C = 0;
    value70 = 0;
    value74 = 0;
    value78 = 0;
    value7C = 0;
    value80 = 1;
    value84 = 1;
    value88 = 0;
    value8C = 0;
    value90 = 0;
    value94 = 0;
    value98 = 0;
    value9C = 0;
    valueA0 = 0;
    valueA4 = 1;
    *(unsigned int *)&valueA8 = 0xbab60b61;
    *(unsigned int *)(unidentified0 + 0x18) = 4;
    return this;
}
