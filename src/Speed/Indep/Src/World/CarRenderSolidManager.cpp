#include "CarRenderSolidManager.h"

WCollisionLink *bNewListNode();
void bDeleteListNode(WCollisionLink *node);

namespace {
CarRenderQueueLink *End(CarRenderQueueLink *head) {
    return reinterpret_cast<CarRenderQueueLink *>(reinterpret_cast<CarRenderLightList *>(head)->End()); // 0x502fd0
}
void Unlink(CarRenderQueueLink *link) {
    link->previous04->next00 = link->next00;
    link->next00->previous04 = link->previous04;
}
void Append(CarRenderQueueLink *head, CarRenderQueueLink *link) {
    CarRenderQueueLink *last = head->previous04;
    last->next00 = link;
    head->previous04 = link;
    link->previous04 = last;
    link->next00 = head;
}
}

// Original 0x43be20. SUB/JNS examines the wrapped sign bit, not signed overflow.
// The shifted multiplier is loaded as a signed integer before both multiplies.
long double CarRenderSolidElapsed(unsigned int start, unsigned int end) {
    int delta = static_cast<int>(end-start);
    if (delta < 0) delta = 0;
    int multiplier = static_cast<int>(1u << (ug2SolidTimerShift8284E4 & 31));
    return static_cast<long double>(multiplier) * delta * ug2SolidTimerScale828028;
}

// Original 0x5bb720: visits child headers without changing their contents.
unsigned int CarRenderWalkPackPlatform(unsigned int *chunk) {
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {}
    return 1;
}

// Original 0x48d910. The parent callback supplies a valid 0x80134001 container.
CarRenderSolidPack *CarRenderLoadPackHeader(unsigned int *chunk) {
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSolidPack *pack = 0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0] == 0x134002) {
            pack = reinterpret_cast<CarRenderSolidPack *>(child+2);
            Append(&ug2SolidPacks830D1C,&pack->link00);
            ++ug2SolidPackCount82D380;
        } else if (child[0] == 0x134003) pack->resources74 = reinterpret_cast<CarRenderSolidResource *>(child+2);
        else if (child[0] == 0x134004) pack->stream78 = child+2;
        else if (child[0] == 0x80134008) CarRenderWalkPackPlatform(child);
    }
    if (pack) pack->value72 = 1;
    pack->list80.next00 = &pack->list80; pack->list80.previous04 = &pack->list80;
    pack->list88.next00 = &pack->list88; pack->list88.previous04 = &pack->list88;
    return pack;
}

// Original 0x493790; these are original bWare node-pool frees.
void CarRenderClearPackLists(CarRenderSolidPack *pack) {
    CarRenderQueueLink *heads[2] = {&pack->list80,&pack->list88};
    for (int i=0;i<2;++i) while (heads[i]->next00 != heads[i]) {
        CarRenderQueueLink *node = heads[i]->next00;
        Unlink(node);
        bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));
    }
    pack->value7C = 0; pack->value7E = 0;
}

// Original 0x493ea0. Header unregistering precedes the parent's solid unloads.
CarRenderSolidPack *CarRenderUnloadPackHeader(unsigned int *chunk) {
    if (chunk[0] != 0x80134001) return 0;
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSolidPack *pack = 0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0] == 0x134002) {
            pack = reinterpret_cast<CarRenderSolidPack *>(child+2);
            Unlink(&pack->link00);
            CarRenderClearPackLists(pack);
            --ug2SolidPackCount82D380;
        } else if (child[0] == 0x80134008) CarRenderWalkPackPlatform(child);
    }
    return pack;
}

// Original 0x48df40; first pack whose sorted index contains the key.
extern "C" CarRenderSolidPack *UG2SolidFindOwner(unsigned int hash) {
    CarRenderSolidPack *pack = reinterpret_cast<CarRenderSolidPack *>(ug2SolidPacks830D1C.next00);
    CarRenderQueueLink *end = End(&ug2SolidPacks830D1C);
    while (reinterpret_cast<CarRenderQueueLink *>(pack) != end) {
        if (pack && CarRenderFindResource(hash,pack->resources74,pack->count0C,0,8)) return pack;
        pack = reinterpret_cast<CarRenderSolidPack *>(pack->link00.next00);
    }
    return 0;
}

// Original 0x48df90. Bucket misses return before reading the clock or pack list.
extern "C" CarRenderSlotTable *UG2SolidFindLoaded(unsigned int hash, CarRenderSolidPack *requested) {
    if (!ug2SolidBuckets830DA4[hash & 0x1fff]) return 0;
    unsigned int start = UG2SolidReadTicker();
    CarRenderSlotTable *result = 0;
    if (!requested) {
        CarRenderSolidPack *pack = reinterpret_cast<CarRenderSolidPack *>(ug2SolidPacks830D1C.next00);
        CarRenderQueueLink *end = End(&ug2SolidPacks830D1C);
        while (reinterpret_cast<CarRenderQueueLink *>(pack) != end) {
            CarRenderSolidResource *entry = pack ? static_cast<CarRenderSolidResource *>(
                CarRenderFindResource(hash,pack->resources74,pack->count0C,0,8)) : 0;
            if (entry && entry->solid04) {
                result = entry->solid04;
                Unlink(&pack->link00);
                CarRenderQueueLink *first = ug2SolidPacks830D1C.next00;
                first->previous04 = &pack->link00;
                ug2SolidPacks830D1C.next00 = &pack->link00;
                pack->link00.previous04 = &ug2SolidPacks830D1C;
                pack->link00.next00 = first;
                break;
            }
            pack = reinterpret_cast<CarRenderSolidPack *>(pack->link00.next00);
        }
    } else {
        CarRenderSolidResource *entry = static_cast<CarRenderSolidResource *>(
            CarRenderFindResource(hash,requested->resources74,requested->count0C,0,8));
        if (entry) result = entry->solid04;
    }
    unsigned int finish = UG2SolidReadTicker();
    long double elapsed = CarRenderSolidElapsed(start,finish);
    ug2SolidLookupTime82DAC0 = static_cast<long double>(ug2SolidLookupTime82DAC0)+elapsed;
    return result;
}

// Original 0x490650. Snapshot the next unresolved reference before lookup/relink.
void CarRenderResolveSolidReferences(CarRenderSolidPack *pack) {
    if (!ug2SolidOffline864F60) {
        unsigned int start = UG2SolidReadTicker();
        CarRenderQueueLink *node = ug2UnresolvedSolids82DBC0.next00;
        CarRenderQueueLink *end = End(&ug2UnresolvedSolids82DBC0);
        while (node != end) {
            CarRenderSolidReference *reference = reinterpret_cast<CarRenderSolidReference *>(node);
            node = node->next00;
            if (reference->hash08) {
                CarRenderSlotTable *solid = UG2SolidFindLoaded(reference->hash08,pack);
                if (reference->hash08 && solid != reference->solid0C) {
                    Unlink(&reference->link00);
                    Append(solid ? &solid->list88 : &ug2UnresolvedSolids82DBC0,&reference->link00);
                    reference->solid0C = solid;
                }
            }
        }
        unsigned int finish = UG2SolidReadTicker();
        long double elapsed = CarRenderSolidElapsed(start,finish), maximum = elapsed;
        if (elapsed < ug2SolidResolveMax82DA8C) maximum = ug2SolidResolveMax82DA8C;
        ug2SolidResolveMax82DA8C = maximum;
        ++ug2SolidResolveCount82DAA0;
        ug2SolidResolveSum82DA94 = static_cast<long double>(ug2SolidResolveSum82DA94)+elapsed;
        ug2SolidRefTime82DA9C = static_cast<long double>(ug2SolidDetachSum82DA98)+ug2SolidResolveSum82DA94;
    } else while (ug2OfflineSolids832DA4.next00 != &ug2OfflineSolids832DA4) {
        CarRenderSolidReference *reference = reinterpret_cast<CarRenderSolidReference *>(ug2OfflineSolids832DA4.next00);
        Unlink(&reference->link00);
        Append(&reference->solid0C->list88,&reference->link00);
    }
}

// Original 0x493650. Valid attached references move off this head on every pass.
void CarRenderDetachSolidReferences(CarRenderSlotTable *solid) {
    if (!ug2SolidOffline864F60) {
        unsigned int start = UG2SolidReadTicker();
        CarRenderQueueLink *head = &solid->list88;
        if (head->next00 != head) {
            CarRenderSlotTable *other = (solid->flags0E & 0x100) ? UG2SolidFindLoaded(solid->hash10,0) : 0;
            while (head->next00 != head) {
                CarRenderSolidReference *reference = reinterpret_cast<CarRenderSolidReference *>(head->next00);
                if (reference->hash08 && other != reference->solid0C) {
                    Unlink(&reference->link00);
                    Append(other ? &other->list88 : &ug2UnresolvedSolids82DBC0,&reference->link00);
                    reference->solid0C = other;
                }
            }
        }
        unsigned int finish = UG2SolidReadTicker();
        long double elapsed = CarRenderSolidElapsed(start,finish), maximum = elapsed;
        if (elapsed < ug2SolidDetachMax82DA90) maximum = ug2SolidDetachMax82DA90;
        ug2SolidDetachMax82DA90 = maximum;
        ++ug2SolidDetachCount82DAA4;
        ug2SolidDetachSum82DA98 = static_cast<long double>(ug2SolidDetachSum82DA98)+elapsed;
        ug2SolidRefTime82DA9C = static_cast<long double>(ug2SolidResolveSum82DA94)+ug2SolidDetachSum82DA98;
    } else while (solid->list88.next00 != &solid->list88) {
        CarRenderSolidReference *reference = reinterpret_cast<CarRenderSolidReference *>(solid->list88.next00);
        Unlink(&reference->link00);
        Append(&ug2OfflineSolids832DA4,&reference->link00);
        // Original encoded pointer: the offline flag is added in byte units.
        reference->solid0C = reinterpret_cast<CarRenderSlotTable *>(
            reinterpret_cast<unsigned char *>(solid)+ug2SolidOffline864F60);
    }
}

// Original 0x479130. Node allocation failure retains the original invalid-write behavior.
void CarRenderDeferredBufferList::Add(unsigned int buffer) {
    CarRenderDeferredBuffer *node = reinterpret_cast<CarRenderDeferredBuffer *>(bNewListNode());
    if (node) node->buffer08 = buffer;
    Append(&head00,&node->link00);
}

// Original 0x5ce7d0, __thiscall RET 4; the chunk argument is not read.
unsigned int CarRenderSlotTable::UnloadPlatformChunk(unsigned int *) {
    CarRenderPlatformSolid *platform = platform00;
    CarRenderQueueLink *node = ug2QueuedSolids872B44.next00, *end = End(&ug2QueuedSolids872B44);
    while (node != end) {
        CarRenderDeferredBuffer *record = reinterpret_cast<CarRenderDeferredBuffer *>(node);
        // +8 contains the original platform pointer word rather than a GPU token.
        CarRenderPlatformSolid *candidate = *reinterpret_cast<CarRenderPlatformSolid **>(&record->buffer08);
        if (candidate == platform) { Unlink(node); bDeleteListNode(reinterpret_cast<WCollisionLink *>(node)); break; }
        node = node->next00;
    }
    if (platform->vertexBuffer2C) { ug2VertexReleases870D28.Add(platform->vertexBuffer2C); platform->vertexBuffer2C=0; }
    if (platform->indexBuffer20) { UG2SolidReleaseIndexBuffer(platform->indexBuffer20); platform->indexBuffer20=0; }
    Unlink(&platform->link00);
    platform00 = 0;
    return 1;
}

// Original 0x493f20. A recognized, valid solid has a nonnull pack context.
void CarRenderUnloadSolidChunk(unsigned int *chunk, CarRenderSolidPack *pack) {
    if (chunk[0] != 0x80134010) return;
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSlotTable *solid = 0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0] == 0x134011) {
#ifdef __GNUC__
            typedef __UINTPTR_TYPE__ Address;
#else
            typedef unsigned __int64 Address;
#endif
            solid = reinterpret_cast<CarRenderSlotTable *>((reinterpret_cast<Address>(child)+23)&~Address(15));
        } else if (child[0]==0x134017 || child[0]==0x134018 || child[0]==0x134019) ug2SolidAuxBytes82D38C-=child[1];
        else if (child[0]==0x13401b) ug2SolidBytes82D390-=child[1];
        else if (child[0]==0x80134100) solid->UnloadPlatformChunk(child);
    }
    if (pack) {
        CarRenderSolidResource *entry = static_cast<CarRenderSolidResource *>(CarRenderFindResource(solid->hash10,pack->resources74,pack->count0C,0,8));
        if (entry) { entry->solid04=0; --ug2SolidLoadedCount830DA0; --ug2SolidBuckets830DA4[solid->hash10 & 0x1fff]; }
    }
    pack->value7C=0xffff;
    CarRenderDetachSolidReferences(solid);
    Unlink(&solid->link04);
    --ug2SolidLoads82D384; ug2SolidChanged82DAB0=1; ug2SolidChunkBytes82D388-=chunk[1];
}

// Original 0x494060..0x4940d6. This callback has no Ghidra function definition;
// recovery follows the original instructions and containing retail chunk format.
unsigned int CarRenderLoadSolidPack(unsigned int *chunk) {
    if (chunk[0] != 0x80134000) return 0;
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSolidPack *pack=0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0]==0x80134001) {
            if (pack) CarRenderResolveSolidReferences(pack);
            pack=CarRenderLoadPackHeader(child);
        } else if (child[0]==0x80134010) CarRenderLoadSolidChunk(child,pack);
    }
    if (pack) CarRenderResolveSolidReferences(pack);
    return 1;
}
// Original 0x4940e0..0x49413c.
unsigned int CarRenderUnloadSolidPack(unsigned int *chunk) {
    if (chunk[0] != 0x80134000) return 0;
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSolidPack *pack=0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)<end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0]==0x80134001) pack=CarRenderUnloadPackHeader(child);
        else if (child[0]==0x80134010) CarRenderUnloadSolidChunk(child,pack);
    }
    return 1;
}
