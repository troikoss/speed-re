#include "FEngPackage.h"

// Original 0x5046b0: first matching script, with no implicit selection/reset.
FEScript *HudVisibilityObject::FindScriptByHash(unsigned int hash) {
    FEScript *script = scripts4C;
    while (script && script->hash30 != hash) script = script->next4;
    return script;
}

// Original 0x50b300: clear every duplicate key, preserving the stale values.
void FEngQueue::StoreFocus(unsigned int key, unsigned int value) {
    for (int i = 0; i < 32; ++i) {
        if (value5150[i].key0 == key) value5150[i].key0 = 0;
    }
    // Reload the insertion index after each store: negative indices can alias it.
    *(unsigned int *)((char *)this + 0x5150 + (unsigned int)value514C * 8) = key;
    *(unsigned int *)((char *)this + 0x5154 + (unsigned int)value514C * 8) = value;
    unsigned int next = ((unsigned int)value514C + 1) & 0x8000001f;
    if ((int)next < 0) next = ((next - 1) | 0xffffffe0) + 1;
    value514C = (int)next;
}

// Original 0x50b360: ascending slot order, including zero-key entries.
unsigned int FEngQueue::LoadFocus(unsigned int key) {
    for (int i = 0; i < 32; ++i) {
        if (value5150[i].key0 == key) return value5150[i].value4;
    }
    return 0;
}

// Original 0x50b390: clear only the first byte of each 256-byte slot, then depth.
void FEngQueue::ClearPackageStack() {
    for (int i = 0; i < 16; ++i) value414C[i][0] = 0;
    value4148 = 0;
}
