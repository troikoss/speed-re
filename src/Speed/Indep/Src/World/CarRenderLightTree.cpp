#include "CarRenderLightSelect.h"

extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)
#ifdef __GNUC__
#define LIGHT_ABS(value) __builtin_fabsl(value)
#else
#define LIGHT_ABS(value) fabs(value)
#endif

// Original 0x502fd0. Kept across the selector's translation-unit boundary.
CarRenderLightSector *CarRenderLightList::End() {
    return reinterpret_cast<CarRenderLightSector *>(this);
}

// Original 0x5746e0. Equality rejects an axis. Unordered comparisons do not
// reject it, matching TEST AH,0x41 / JNP rather than an inclusive AABB test.
unsigned int CarRenderLightTreeNode::Contains(float x, float y, float z) {
    if (extent10[0] <= LIGHT_ABS(static_cast<long double>(x) - center00[0])) return 0;
    if (extent10[1] <= LIGHT_ABS(static_cast<long double>(y) - center00[1])) return 0;
    if (extent10[2] <= LIGHT_ABS(static_cast<long double>(z) - center00[2])) return 0;
    return 1;
}

// Original 0x574730. The original inlines the same bounds tests for each child.
// Indices are signed 16-bit; a nonpositive count identifies a leaf.
CarRenderLightTreeNode *CarRenderLightTree::Descend(CarRenderLightTreeNode *node, float x, float y, float z) {
    const int count = node->count0E;
    for (int i = 0; i < count; ++i) {
        CarRenderLightTreeNode *child = nodes00 + node->indices1C[i];
        if (child->extent10[0] <= LIGHT_ABS(static_cast<long double>(x) - child->center00[0])) continue;
        if (child->extent10[1] <= LIGHT_ABS(static_cast<long double>(y) - child->center00[1])) continue;
        if (child->extent10[2] <= LIGHT_ABS(static_cast<long double>(z) - child->center00[2])) continue;
        if (child->count0E <= 0) return child;
        CarRenderLightTreeNode *result = Descend(child, x, y, z);
        if (result) return result;
    }
    return 0;
}

// Original 0x5747e0. No root means no query; a root leaf returns directly.
CarRenderLightTreeNode *CarRenderLightTree::Find(float x, float y, float z) {
    CarRenderLightTreeNode *root = nodes00;
    if (!root || !root->Contains(x, y, z)) return 0;
    if (root->count0E <= 0) return root;
    return Descend(root, x, y, z);
}
