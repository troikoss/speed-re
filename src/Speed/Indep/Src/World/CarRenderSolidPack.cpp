#include "CarRenderSolidPack.h"

#ifdef __GNUC__
typedef __UINTPTR_TYPE__ PackAddress;
#else
typedef unsigned __int64 PackAddress;
#endif

// Original 0x431780, fastcall: list links are four bytes into profile records.
CarRenderLightPack *__fastcall CarRenderProfileFromLink(void *link) {
    return link ? reinterpret_cast<CarRenderLightPack *>(static_cast<unsigned char *>(link)-4) : 0;
}
// Original 0x40cb20. The PC reference unconditionally allows this load.
unsigned int CarRenderSolidLoadAllowed() { return 1; }

// Original 0x488bc0: unsigned-key binary search, then rewind to the first duplicate.
void *CarRenderFindResource(unsigned int hash, void *base, int count,
                            unsigned int offset, unsigned int stride) {
    if (base && count > 0 && offset+4 <= stride) {
        int last = count-1, first = 0;
        while (first <= last) {
            int middle = (last+first)/2;
            unsigned char *record = static_cast<unsigned char *>(base)+middle*stride;
            unsigned int key = *reinterpret_cast<unsigned int *>(record+offset);
            if (hash < key) last = middle-1;
            else if (hash > key) first = middle+1;
            else {
                while (middle > 0 && *reinterpret_cast<unsigned int *>(record-stride+offset) == hash) {
                    record -= stride;
                    --middle;
                }
                return record;
            }
        }
    }
    return 0;
}

// Original 0x4907a0, __thiscall-compatible fastcall receiver. Count is re-read
// after each lookup, as is the slot base on the next iteration.
void CarRenderSlotTable::ResolveTextures() {
    if (ug2SolidOffline864F60 == 0) {
        for (int i = 0; i < textureCount19; ++i) {
            CarRenderTextureSlot *slots = textures2C;
            CarRenderTexture *texture = UG2SolidFindTexture(slots[i].hash00,1,0);
            slots[i].texture04 = texture;
        }
    }
}

// Original 0x493bb0. Valid retail order puts the header and auxiliary record
// before their dependent children. No new validation or malformed-data repair.
CarRenderSlotTable *CarRenderLoadSolidChunk(unsigned int *chunk, CarRenderSolidPack *pack) {
    if (chunk[0] != 0x80134010) return 0;
    unsigned int *child = chunk+2;
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderSlotTable *solid = 0;
    bool allowed = true;
    while (reinterpret_cast<unsigned char *>(child) < end) {
        if (child[0] == 0x134011) {
            solid = reinterpret_cast<CarRenderSlotTable *>((reinterpret_cast<PackAddress>(child)+0x17)&~PackAddress(15));
            solid->list88.next00 = &solid->list88;
            solid->list88.previous04 = &solid->list88;
        } else if (child[0] == 0x134012) {
            solid->textures2C = reinterpret_cast<CarRenderTextureSlot *>(child+2);
        } else if (child[0] == 0x134013) {
            solid->slots3C = reinterpret_cast<CarRenderSlot *>(child+2);
        } else if (child[0] == 0x134017) {
            solid->aux84 = reinterpret_cast<CarRenderSolidAux *>(child+2);
            ug2SolidAuxBytes82D38C += child[1];
        } else if (child[0] == 0x134018) {
            solid->aux84->value00 = child+2;
            solid->aux84->count08 = static_cast<unsigned short>(child[1]>>3);
            ug2SolidAuxBytes82D38C += child[1];
        } else if (child[0] == 0x134019) {
            solid->aux84->value04 = child+2;
            solid->aux84->count0A = static_cast<unsigned short>(child[1]/12);
            ug2SolidAuxBytes82D38C += child[1];
        } else if (child[0] == 0x13401b) {
            solid->value90 = child+2;
            ug2SolidBytes82D390 += child[1];
        } else if (child[0] == 0x13401a) {
            solid->markers80 = reinterpret_cast<void *>((reinterpret_cast<PackAddress>(child)+0x17)&~PackAddress(15));
            unsigned int bytes = static_cast<unsigned int>(reinterpret_cast<unsigned char *>(child)+child[1]+8 -
                                                           static_cast<unsigned char *>(solid->markers80));
            solid->markerCount1B = static_cast<unsigned char>(bytes/0x50);
        } else if (child[0] == 0x80134100) {
            if (!solid->LoadPlatformChunk(child)) allowed = false;
        }
        child = reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8);
    }
    if (solid) solid->loaded0D = 1;
    pack->value7C = 0xffff;
    solid->ResolveTextures();
    int count = solid->count1A;
    CarRenderSlot *slot = solid->slots3C;
    if (count > 0) {
        CarRenderLightPack *endProfile = CarRenderProfileFromLink(&ug2LightProfiles8358C0);
        do {
            unsigned char *first = reinterpret_cast<unsigned char *>(ug2LightProfiles8358C0.next00);
            CarRenderLightPack *profile = first ? reinterpret_cast<CarRenderLightPack *>(first-4) : 0;
            CarRenderLightPack *selected = ug2LightDefaultPack7F399C;
            while (profile != endProfile) {
                if (profile->hash0C == slot->key) { selected = profile; break; }
                CarRenderLightLink *next = profile->link04.next00;
                profile = next ? reinterpret_cast<CarRenderLightPack *>(reinterpret_cast<unsigned char *>(next)-4) : 0;
            }
            slot->lightPack = selected;
            ++slot;
        } while (--count != 0);
    }
    if (ug2SolidOffline864F60 == 0 && !(solid->flags0E & 0x10)) {
        CarRenderSlotTable *other = UG2SolidFindLoaded(solid->hash10,0);
        if (other) {
            solid->flags0E |= 0x100;
            other->flags0E |= 0x100;
            if (ug2SolidFlag82DAA8 == 0) {
                solid->flags0E |= 0x200;
                other->flags0E |= 0x200;
                UG2SolidFindOwner(solid->hash10);
            }
        }
    }
    if (!CarRenderSolidLoadAllowed()) allowed = false;
    CarRenderSolidResource *resource = static_cast<CarRenderSolidResource *>(
        CarRenderFindResource(solid->hash10,pack->resources74,pack->count0C,0,8));
    CarRenderQueueLink *link = &solid->link04;
    if (allowed) {
        if (resource) resource->solid04 = solid;
        CarRenderQueueLink *last = ug2LoadedSolids835794.previous04;
        last->next00 = link;
        link->previous04 = last;
        ug2LoadedSolids835794.previous04 = link;
        link->next00 = &ug2LoadedSolids835794;
        ++ug2SolidLoadedCount830DA0;
        ++ug2SolidBuckets830DA4[solid->hash10 & 0x1fff];
    } else {
        if (resource) resource->solid04 = 0;
        CarRenderQueueLink *last = ug2FailedSolids835858.previous04;
        last->next00 = link;
        link->previous04 = last;
        ug2FailedSolids835858.previous04 = link;
        link->next00 = &ug2FailedSolids835858;
    }
    ++ug2SolidLoads82D384;
    ug2SolidChanged82DAB0 = 1;
    ug2SolidChunkBytes82D388 += chunk[1];
    return solid;
}
