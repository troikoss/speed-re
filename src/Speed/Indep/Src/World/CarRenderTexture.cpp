#include "CarRenderTexture.h"
#include "CarRenderSolidPack.h"
#include "../../Libs/bWare/Strings.h"
#include "../../Libs/bWare/Formatting.h"

namespace {
void Unlink(CarRenderQueueLink *link) {
    link->previous04->next00 = link->next00;
    link->next00->previous04 = link->previous04;
}
CarRenderTextureIndex *Find(CarRenderTexturePack *pack, unsigned int hash) {
    if (pack->header70 && pack->header70->indices74)
        return static_cast<CarRenderTextureIndex *>(CarRenderFindResource(
            hash,pack->header70->indices74,pack->count74,0,8));
    return 0;
}
bool InRange(int value, int offset, int bytes) {
    // Original ADD wraps before the two signed comparisons.
    int end = static_cast<int>(static_cast<unsigned int>(offset)+static_cast<unsigned int>(bytes));
    return value >= offset && value < end;
}
}

// Original 0x50d510 and 0x4022c0. Arguments are deliberately not inspected.
extern "C" unsigned int UG2TextureInventory(...) { return 0; }
extern "C" void UG2TextureInventoryRelease(unsigned int) {}

// Original 0x5b9690, __thiscall RET 4.
void CarRenderTexture::SetState(CarRenderTextureState *state) { state00 = state; }

// Original 0x48fc40. The strings are original unbounded bWare copies.
CarRenderTexturePack *CarRenderTexturePack::Construct(CarRenderTexturePackHeader *header, int count,
                                                       const char *name, const char *path) {
    if (!name && header) name = reinterpret_cast<const char *>(header)+4;
    if (!path && header) path = reinterpret_cast<const char *>(header)+0x20;
    if (!name) name = ug2TextureNull78DA04;
    if (!path) path = ug2TextureNull78DA04;
    nfs_strcpy(name08,name);
    path28[0] = 0;
    if (path) nfs_strcpy(path28,path);
    header70 = header;
    hash68 = cshash(name08);
    count74 = count;
    value78 = 0;
    CarRenderQueueLink *last = ug2TexturePacks830AF0.previous04;
    last->next00 = &link00;
    link00.previous04 = last;
    ug2TexturePacks830AF0.previous04 = &link00;
    link00.next00 = &ug2TexturePacks830AF0;
    char label[1024];
    bSFormat(label,ug2TextureLabel78D9F4,name08);
    value6C = UG2TextureInventory(label,ug2TextureInventory78D9C0);
    return this;
}
// Original 0x48fd10.
void CarRenderTexturePack::Destroy() {
    Unlink(&link00);
    UG2TextureInventoryRelease(value6C);
}

// Original 0x48fd30. The optional entry suppresses the hash search, even when
// its key differs. Parent resolution observes the index at this call's time.
void CarRenderTexturePack::Register(CarRenderTexture *texture, CarRenderTextureState *state,
                                     CarRenderTextureIndex *entry) {
    if (!entry) entry = Find(this,texture->hash24);
    if (entry) {
        entry->texture04 = texture;
        ++ug2TextureCount833780;
        ++ug2TextureBuckets833784[texture->hash24 & 0x1fff];
    }
    if (!texture->parentHash2C) texture->parent6C = texture;
    else {
        CarRenderTextureIndex *parent = Find(this,texture->parentHash2C);
        texture->parent6C = parent ? parent->texture04 : 0;
    }
    texture->pack68 = this;
    texture->pixels70 = 0;
    texture->palette74 = 0;
    texture->SetState(state);
    ug2TextureChanged82DAB4 = 1;
    if (texture->hash24 == 0x66994f4a) ug2TextureDefault82DB08 = texture;
}
// Original 0x4919e0, __thiscall RET 0xc.
void CarRenderTexturePack::RegisterArray(CarRenderTexture *textures, CarRenderTextureState *states, int count) {
    for (int i=0; i<count; ++i) Register(textures+i,states+i,header70->indices74+i);
}
// Original 0x491a30. State argument is not read; record/index pointers remain.
void CarRenderTexturePack::UnregisterArray(CarRenderTexture *textures, CarRenderTextureState *, int count) {
    for (int i=0; i<count; ++i) {
        CarRenderTextureIndex *entry = header70->indices74+i;
        if (!entry) entry = Find(this,textures[i].hash24);
        if (entry) {
            entry->texture04 = 0;
            --ug2TextureCount833780;
            --ug2TextureBuckets833784[textures[i].hash24 & 0x1fff];
        }
        if (textures+i == ug2TextureDefault82DB08) ug2TextureDefault82DB08 = 0;
    }
}

// Original 0x48fe00. A GPU object is not a condition of CPU readiness.
CarRenderTexture *CarRenderTexturePack::FindLoaded(unsigned int hash) {
    if (ug2TextureBuckets833784[hash & 0x1fff]) {
        CarRenderTextureIndex *entry = Find(this,hash);
        if (entry && entry->texture04 && entry->texture04->pixels70) {
            CarRenderTexture *texture = entry->texture04;
            if (!texture->paletteBytes3C || texture->palette74) return texture;
        }
    }
    return 0;
}

// Original 0x4901d0, cdecl. Override and fallback hits do not set used53.
extern "C" CarRenderTexture *UG2SolidFindTexture(unsigned int hash, int fallback, int rawMode) {
    if (hash) {
        CarRenderTexturePack *pack = reinterpret_cast<CarRenderTexturePack *>(ug2TexturePacks830AF0.next00);
        CarRenderQueueLink *end = reinterpret_cast<CarRenderQueueLink *>(
            reinterpret_cast<CarRenderLightList *>(&ug2TexturePacks830AF0)->End()); // 0x502fd0
        if (reinterpret_cast<CarRenderQueueLink *>(pack) != end) {
            unsigned char bucket = ug2TextureBuckets833784[hash & 0x1fff];
            while (reinterpret_cast<CarRenderQueueLink *>(pack) != end && bucket) {
                CarRenderTexture *texture;
                if (!rawMode) texture = pack->FindLoaded(hash);
                else {
                    CarRenderTextureIndex *entry = Find(pack,hash);
                    texture = entry ? entry->texture04 : 0;
                }
                if (texture) {
                    Unlink(&pack->link00);
                    CarRenderQueueLink *first = ug2TexturePacks830AF0.next00;
                    ug2TexturePacks830AF0.next00 = &pack->link00;
                    first->previous04 = &pack->link00;
                    pack->link00.previous04 = &ug2TexturePacks830AF0;
                    pack->link00.next00 = first;
                    texture->used53 = 1;
                    return texture;
                }
                pack = reinterpret_cast<CarRenderTexturePack *>(pack->link00.next00);
            }
        }
        CarRenderTexture *inventory = reinterpret_cast<CarRenderTexture *>(UG2TextureInventory(hash));
        if (inventory) return inventory;
        for (unsigned int i=0; i<ug2TextureOverrideCount82DA80; ++i) if (ug2TextureOverrideKeys82D530[i] == hash) {
            if (ug2TextureOverrideValues82D5C0[i]) return ug2TextureOverrideValues82D5C0[i];
            break;
        }
    }
    if (fallback) {
        if (ug2TextureDefault82DB08) return ug2TextureDefault82DB08;
        UG2TextureMissingDefault();
    }
    return 0;
}

// Original 0x4902d0. A nonnull local result in mode zero keeps 'current'.
CarRenderTexture *CarRenderRefreshTexture(CarRenderTexture *current, unsigned int hash,
                                           CarRenderTexturePack *pack, char mode) {
    if (!mode) {
        if (current && current->pack68 == pack && !pack->FindLoaded(hash)) return UG2SolidFindTexture(hash,0,0);
    } else if (!current) {
        CarRenderTexture *found = pack->FindLoaded(hash);
        if (found) { found->used53 = 1; return found; }
    }
    return current;
}

// Original 0x494a50. A range test concerns the starting offset, not the end
// of the payload. Palette-only changes can trigger upload of retained pixels.
void CarRenderTexturePack::Attach(unsigned char *data, int offset, int bytes) {
    for (int i=0; i<count74; ++i) {
        CarRenderTexture *texture = header70->indices74[i].texture04;
        if (texture) {
            bool changed = false;
            if (texture->pixelBytes38 && InRange(texture->pixelOffset30,offset,bytes)) {
                texture->pixels70 = data+(static_cast<unsigned int>(texture->pixelOffset30)-static_cast<unsigned int>(offset));
                changed = true;
            }
            if (texture->paletteBytes3C && InRange(texture->paletteOffset34,offset,bytes)) {
                texture->palette74 = data+(static_cast<unsigned int>(texture->paletteOffset34)-static_cast<unsigned int>(offset));
            } else if (!changed) continue;
            if (texture->pixels70) UG2TextureUpload(texture);
        }
    }
    UG2TextureNotifySolids(this,1);
    UG2TextureNotify5C0DC0(this,1);
    UG2TextureNotify51BF70(this,1);
    ug2TextureChanged82DAB4 = 1;
}
// Original 0x494b10. Palette detachment alone retains an existing GPU handle
// when pixels are still attached; the six special hashes notify after release.
void CarRenderTexturePack::Detach(int offset, int bytes) {
    for (int i=0; i<count74; ++i) {
        CarRenderTexture *texture = header70->indices74[i].texture04;
        if (texture) {
            bool changed = false;
            if (texture->pixelBytes38 && InRange(texture->pixelOffset30,offset,bytes)) {
                texture->pixels70 = 0; changed = true;
            }
            if (texture->paletteBytes3C && InRange(texture->paletteOffset34,offset,bytes)) {
                texture->palette74 = 0; changed = true;
            }
            if (!texture->pixels70 && changed) {
                UG2TextureRelease(texture);
                unsigned int hash = texture->hash24;
                if (hash >= 0x530b82b0 && hash <= 0x530b82b5) UG2TextureNotify611A10(hash);
            }
        }
    }
    UG2TextureNotifySolids(this,0);
    UG2TextureNotify5C0DC0(this,0);
    UG2TextureNotify51BF70(this,0);
    ug2TextureChanged82DAB4 = 1;
}

// Original 0x494c10. A decoded slot ends in a 0x7c texture and 0x20 state.
// Offsets remain pack-relative; Attach uses the slot's first pixel as its base.
void CarRenderLoadTextureSlot(unsigned char *decoded, CarRenderTextureRequest *request,
                                CarRenderTextureStreamOwner *owner) {
    CarRenderTexturePack *pack = owner->headerBC->pack70;
    unsigned char *end = decoded+request->bytes0C;
    CarRenderTexture *texture = reinterpret_cast<CarRenderTexture *>(end-0x9c);
    pack->Register(texture,reinterpret_cast<CarRenderTextureState *>(end-0x20),0);
    unsigned int bytes = texture->pixelBytes38;
    if (texture->paletteOffset34 != -1)
        bytes = texture->paletteBytes3C+static_cast<unsigned int>(texture->paletteOffset34)-
                static_cast<unsigned int>(texture->pixelOffset30);
    pack->Attach(decoded,texture->pixelOffset30,static_cast<int>(bytes));
}
// Original 0x494c80. Detachment/notifications precede index unregistration;
// freeing the decoded allocation is the stream owner's responsibility.
void CarRenderUnloadTextureSlot(unsigned char *decoded, CarRenderTextureRequest *request,
                                  CarRenderTextureStreamOwner *owner) {
    CarRenderTexturePack *pack = owner->headerBC->pack70;
    CarRenderTexture *texture = reinterpret_cast<CarRenderTexture *>(decoded+request->bytes0C-0x9c);
    unsigned int bytes = texture->pixelBytes38;
    if (texture->paletteOffset34 != -1)
        bytes = texture->paletteBytes3C+static_cast<unsigned int>(texture->paletteOffset34)-
                static_cast<unsigned int>(texture->pixelOffset30);
    pack->Detach(texture->pixelOffset30,static_cast<int>(bytes));
    CarRenderTextureIndex *entry = Find(pack,texture->hash24);
    if (entry) {
        entry->texture04 = 0;
        --ug2TextureCount833780;
        --ug2TextureBuckets833784[texture->hash24 & 0x1fff];
    }
    if (texture == ug2TextureDefault82DB08) ug2TextureDefault82DB08 = 0;
}
