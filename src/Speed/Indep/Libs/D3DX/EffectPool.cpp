#include "EffectFactory.h"

// Original 0x641837. Free data before unlinking; retain the original reloads.
void D3DXEffectPoolLink::Unlink() {
    if (data08) { UG2FxFree(data08); data08=0; }
    next10->previous0C=previous0C;
    previous0C->next10=next10;
}
// Original 0x641878. Detach each linked node before its recursive deletion.
void D3DXEffectPoolNode::Destroy() {
    while (linked20) {
        D3DXEffectPoolNode *node=linked20;
        linked20=node->linked20;
        node->linked20=0;
        node->Delete(1);
    }
    link0C.Unlink();
}
// Original 0x641816.
D3DXEffectPoolNode *D3DXEffectPoolNode::Delete(unsigned char flags) {
    Destroy();
    if (flags&1) UG2FxFree(this);
    return this;
}
// Original 0x64285f.
D3DXEffectPool *D3DXEffectPool::Construct() {
    class00=0x7ab4e8; refs04=1; data10=0; node1C=0;
    value14=0; value18=0; value08=0; value0C=0;
    return this;
}
// Original 0x642504.
void D3DXEffectPool::Destroy() {
    class00=0x7ab4e8;
    if (data10) { UG2FxFree(data10); data10=0; }
    if (node1C) { node1C->Delete(1); node1C=0; }
}
// Original 0x642535.
D3DXEffectPool *D3DXEffectPool::Delete(unsigned char flags) {
    Destroy();
    if (flags&1) UG2FxFree(this);
    return this;
}
// Original 0x642883.
extern "C" unsigned int __stdcall D3DXEffectPoolRelease(D3DXEffectPool *pool) {
    --pool->refs04;
    unsigned int count=pool->refs04;
    if (!count) { pool->Delete(1); return 0; }
    return count;
}
// Original 0x642556. Failed queries preserve output, including IID aliases.
extern "C" int __stdcall D3DXEffectPoolQuery(D3DXEffectPool *pool,const unsigned int *iid,D3DXEffectPool **output) {
    int i=0;
    while (i<4 && iid[i]==ug2FxPoolIid7B7410[i]) ++i;
    if (i!=4) {
        i=0;
        while (i<4 && iid[i]==ug2FxIUnknown7D9904[i]) ++i;
        if (i!=4) return static_cast<int>(0x80004002);
    }
    *output=pool;
    UG2FxPoolAddRefVirtual(pool);
    return 0;
}
// Original 0x640aac. Allocation failure does not clear the output.
extern "C" int __stdcall D3DXEffectPoolCreate(D3DXEffectPool **output) {
    if (!output) return static_cast<int>(0x8876086c);
    D3DXEffectPool *pool=static_cast<D3DXEffectPool *>(UG2FxMalloc(0x20));
    if (pool) pool=pool->Construct();
    if (!pool) return static_cast<int>(0x8007000e);
    *output=pool;
    return 0;
}
