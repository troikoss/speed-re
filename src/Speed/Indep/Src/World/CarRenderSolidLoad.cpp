#include "CarRenderSolidLoad.h"

#ifdef __GNUC__
typedef __UINTPTR_TYPE__ SolidAddress;
#else
typedef unsigned __int64 SolidAddress;
#endif

// Original 0x5bc6b0. Later matches do not override an earlier matching mask.
unsigned int CarRenderTechniqueIndex(unsigned int flags) {
    for (unsigned int i = 0; i < 4; ++i) {
        const CarRenderTechniqueRule *rule = ug2SolidRules800464 + i;
        if ((flags & rule->mask00) || rule->mask00 == 0) return rule->technique04;
    }
    return 0xffffffff;
}

// Original 0x5c58a0. The primitive's word at +0x34 changes from an index offset
// to a byte address. Unsigned arithmetic keeps the observed 32-bit wrap. Return
// AL is zero; the remaining return bits come from the selected rule index.
unsigned int CarRenderPlatformSolid::FixupPrimitives(unsigned int offset) {
    for (int i = 0; i < primitiveCount10; ++i) {
        primitives14[i].value34 = primitives14[i].value34 * 2 - offset + indexAddress18;
    }
    // This search is inlined in the original; it does not call 0x5bc6b0.
    unsigned int index = 0xffffffff;
    for (unsigned int i = 0; i < 4; ++i) {
        const CarRenderTechniqueRule *rule = ug2SolidRules800464 + i;
        if ((flags0C & rule->mask00) || rule->mask00 == 0) {
            index = rule->technique04;
            break;
        }
    }
    technique3C = ug2SolidTechniques86EB28[static_cast<int>(index)];
    return index & 0xffffff00;
}

namespace {
void *AlignedPayload(unsigned int *chunk, SolidAddress alignment) {
    return reinterpret_cast<void *>((reinterpret_cast<SolidAddress>(chunk) + 8 + alignment - 1) & ~(alignment - 1));
}

// Original REP MOVSD then REP MOVSB, including forward-overlap behavior.
void CopyBuffer(unsigned char *output, const unsigned char *input, unsigned int bytes) {
    unsigned int words = bytes >> 2;
    while (words--) {
        *reinterpret_cast<unsigned int *>(output) = *reinterpret_cast<const unsigned int *>(input);
        output += 4;
        input += 4;
    }
    bytes &= 3;
    while (bytes--) *output++ = *input++;
}
}

// Original 0x5cc280, __thiscall on the table, RET 4. Retail child order supplies
// the header before dependent records. Do not fabricate missing headers or
// check GPU HRESULTs that the original ignores. Unknown child kinds are skipped.
unsigned int CarRenderSlotTable::LoadPlatformChunk(unsigned int *chunk) {
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk) + chunk[1] + 8;
    unsigned int *child = chunk + 2;
    CarRenderPlatformSolid *current = 0;
    while (reinterpret_cast<unsigned char *>(child) < end) {
        if (child[0] == 0x134900) {
            current = static_cast<CarRenderPlatformSolid *>(AlignedPayload(child, 16));
            platform00 = current;
            CarRenderQueueLink *last = ug2PlatformSolids872ED0.previous04;
            last->next00 = &current->link00;
            ug2PlatformSolids872ED0.previous04 = &current->link00;
            current->link00.previous04 = last;
            current->link00.next00 = &ug2PlatformSolids872ED0;
        } else if (child[0] == 0x134b02) {
            current = platform00;
            current->primitives14 = static_cast<CarRenderPrimitive *>(AlignedPayload(child, 16));
        } else if (child[0] == 0x134b03) {
            current = platform00;
            current->indices18 = static_cast<unsigned char *>(AlignedPayload(child, 16));
            current->indexBytes1C = static_cast<unsigned int>(reinterpret_cast<unsigned char *>(child) + child[1] + 8 - current->indices18);
            current->FixupPrimitives(0);
            if (current->indexBytes1C != 0) {
                UG2SolidCreateIndexBuffer(ug2SolidDevice870974, current->indexBytes1C, 0, 0x65, 1,
                                          &current->indexBuffer20, 0);
                unsigned char *output;
                UG2SolidLockBuffer(current->indexBuffer20, 0, 0, reinterpret_cast<void **>(&output), 0);
                CopyBuffer(output, current->indices18, current->indexBytes1C);
                UG2SolidUnlockBuffer(current->indexBuffer20);
            }
        } else if (child[0] == 0x134b01) {
            current->vertexBuffer2C = 0;
            current = platform00;
            current->value40 = 0;
            current->vertices28 = static_cast<unsigned char *>(AlignedPayload(child, 128));
            current->vertexBytes30 = static_cast<unsigned int>(reinterpret_cast<unsigned char *>(child) + child[1] + 8 - current->vertices28);
            if (current->vertexCount34 != 0) {
                if (ug2SolidLoadMode8900D8 && *ug2SolidLoadMode8900D8 == 4 &&
                    CarRenderTechniqueIndex(current->flags0C) != 5) {
                    current->value40 = 0;
                } else {
                    UG2SolidCreateVertexBuffer(ug2SolidDevice870974, current->vertexBytes30, 8, 0, 1,
                                               &current->vertexBuffer2C, 0);
                    unsigned char *output;
                    current->value40 = 0;
                    UG2SolidLockBuffer(current->vertexBuffer2C, 0, 0, reinterpret_cast<void **>(&output), 0);
                    CopyBuffer(output, current->vertices28, current->vertexBytes30);
                    UG2SolidUnlockBuffer(current->vertexBuffer2C);
                }
            }
        }
        child = reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child) + child[1] + 8);
    }
    return 1;
}
