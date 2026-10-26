#include "CarRenderLightLoad.h"
#include "../../Libs/bWare/bSlotPool.h"

// Original 0x598750. Initialise stays across a translation-unit boundary.
CarRenderLightList *CarRenderLightList::Construct() {
    Initialise();
    return this;
}

// Original 0x5bb590.
void CarRenderLightPack::AllocateState() {
    value00 = bOMalloc(ug2LightStatePool8708B8);
}

// Original 0x5bb5b0. Unrecognized hashes retain both existing cells.
void CarRenderLightPack::ApplyDefaults() {
    int first, second;
    switch (hash0C) {
    case 0x12c9453c: case 0x3ed70c43: case 0x471a1dca: first = 0; second = 4; break;
    case 0x89f23a14: case 0x89946400: first = 1; second = 5; break;
    case 0x8050ca3f: case 0x8a616c4c: case 0xd68ea0c1: case 0xd6ec76d5: first = 2; second = 6; break;
    case 0x2388dd82: case 0x23e6a3f4: case 0x23e6b396: case 0x6cf41096:
    case 0x8062e2c0: case 0x8cb61dcd: case 0x8cb694ed: case 0x8cb95e6b:
    case 0x8cbc02ec: case 0x8cbe3497: case 0xb3645f81: case 0xc9d2b801: first = 3; second = 7; break;
    case 0x33a26cb6: case 0xa6348ee3: first = 8; second = 9; break;
    default: return;
    }
    value50 = ug2LightDefaults7FF060[first];
    value54 = ug2LightDefaults7FF060[second];
}

// Original 0x4022c0, shared empty routine; the tree pointer is in ECX here.
void CarRenderLightTree::LoadNoop() {}
