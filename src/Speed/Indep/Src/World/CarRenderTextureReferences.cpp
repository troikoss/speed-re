#include "CarRenderTextureReferences.h"

WCollisionLink *bNewListNode();
void bDeleteListNode(WCollisionLink *node);
namespace {
CarRenderQueueLink *End(CarRenderQueueLink *head) {
    return reinterpret_cast<CarRenderQueueLink *>(reinterpret_cast<CarRenderLightList *>(head)->End());
}
void Unlink(CarRenderQueueLink *link) {
    link->previous04->next00 = link->next00;
    link->next00->previous04 = link->previous04;
}
void Prepend(CarRenderQueueLink *head,void *value) {
    reinterpret_cast<CarRenderPointerList *>(head)->Prepend(value);
}
}

// Original 0x4cbab0. Allocation failure retains the original invalid writes.
void CarRenderPointerList::Prepend(void *value) {
    CarRenderPointerNode *node = reinterpret_cast<CarRenderPointerNode *>(bNewListNode());
    if (node) node->value08 = value;
    CarRenderQueueLink *first = head00.next00;
    head00.next00 = &node->link00;
    first->previous04 = &node->link00;
    node->link00.previous04 = &head00;
    node->link00.next00 = first;
}

// Original 0x4907e0, __thiscall RET 4. Only default bindings are reconsidered.
unsigned char CarRenderSlotTable::RefreshLoadedTextures(CarRenderTexturePack *pack) {
    unsigned char changed = 0;
    for (int i=0;i<textureCount19;++i) {
        CarRenderTextureSlot *slot = textures2C+i;
        CarRenderTexture *texture = slot->texture04;
        if (texture == ug2TextureDefault82DB08 && texture == ug2TextureDefault82DB08) {
            CarRenderTexture *found = pack->FindLoaded(slot->hash00);
            if (found) { found->used53 = 1; texture = found; }
        }
        if (texture != slot->texture04) { slot->texture04 = texture; changed = 1; }
    }
    return changed;
}
// Original 0x490850. A successful local lookup keeps the current binding; a
// failed local/global lookup uses the real default and traps if it is absent.
unsigned char CarRenderSlotTable::RefreshUnloadedTextures(CarRenderTexturePack *pack) {
    unsigned char changed = 0;
    for (int i=0;i<textureCount19;++i) {
        CarRenderTextureSlot *slot = textures2C+i;
        CarRenderTexture *texture = slot->texture04;
        unsigned int hash = slot->hash00;
        if (texture->pack68 == pack && !pack->FindLoaded(hash)) {
            texture = UG2SolidFindTexture(hash,0,0);
            if (!texture) {
                texture = ug2TextureDefault82DB08;
                if (!texture) UG2TextureMissingDefault();
            }
        }
        if (texture != slot->texture04) { slot->texture04 = texture; changed = 1; }
    }
    return changed;
}

// Original 0x493800. The pending-list comparison is against the default
// texture's hash24, not the requested slot hash. Preserve that observed test.
void CarRenderBuildPackTextureReferences(CarRenderSolidPack *pack) {
    CarRenderClearPackLists(pack);
    for (int i=0;i<pack->count0C;++i) {
        CarRenderSlotTable *solid = pack->resources74[i].solid04;
        if (!solid) continue;
        for (int j=0;j<solid->textureCount19;++j) {
            CarRenderTextureSlot *slot = solid->textures2C+j;
            CarRenderTexture *texture = slot->texture04;
            if (texture == ug2TextureDefault82DB08) {
                CarRenderQueueLink *node = pack->list88.next00;
                while (node != &pack->list88) {
                    if (reinterpret_cast<CarRenderPointerNode *>(node)->slot08->hash00 == texture->hash24) break;
                    node = node->next00;
                }
                if (node == &pack->list88) { Prepend(&pack->list88,slot); ++pack->value7E; }
            } else {
                CarRenderQueueLink *node = pack->list80.next00;
                while (node != &pack->list80) {
                    if (reinterpret_cast<CarRenderPointerNode *>(node)->pack08 == texture->pack68) break;
                    node = node->next00;
                }
                if (node == &pack->list80) { Prepend(&pack->list80,texture->pack68); ++pack->value7C; }
            }
        }
    }
}

// Original 0x4938e0. Offline slots encode the flag as a byte displacement.
void CarRenderDetachOfflineTextures(CarRenderTexturePack *pack) {
    CarRenderSolidPack *solidPack = reinterpret_cast<CarRenderSolidPack *>(ug2SolidPacks830D1C.next00);
    CarRenderQueueLink *end = End(&ug2SolidPacks830D1C);
    for (;reinterpret_cast<CarRenderQueueLink *>(solidPack)!=end;
         solidPack=reinterpret_cast<CarRenderSolidPack *>(solidPack->link00.next00)) {
        if (solidPack->value7C == 0xffff) CarRenderBuildPackTextureReferences(solidPack);
        for (CarRenderQueueLink *node=solidPack->list80.next00;node!=&solidPack->list80;node=node->next00) {
            if (reinterpret_cast<CarRenderPointerNode *>(node)->pack08 != pack) continue;
            for (int i=0;i<solidPack->count0C;++i) {
                CarRenderSlotTable *solid = solidPack->resources74[i].solid04;
                if (!solid) continue;
                for (int j=0;j<solid->textureCount19;++j) {
                    CarRenderTexture *texture = solid->textures2C[j].texture04;
                    if (texture->pack68 == pack) solid->textures2C[j].texture04 = reinterpret_cast<CarRenderTexture *>(
                        reinterpret_cast<unsigned char *>(texture)+ug2SolidOffline864F60);
                }
            }
            break;
        }
    }
}

// Original 0x4939b0. Only the low byte of the caller's mode is tested.
extern "C" void UG2TextureNotifySolids(CarRenderTexturePack *pack,int mode) {
    if (!static_cast<unsigned char>(mode) && ug2SolidOffline864F60) { CarRenderDetachOfflineTextures(pack); return; }
    if (ug2SolidOffline864F60) return;
    CarRenderSolidPack *solidPack = reinterpret_cast<CarRenderSolidPack *>(ug2SolidPacks830D1C.next00);
    CarRenderQueueLink *end = End(&ug2SolidPacks830D1C);
    for (;reinterpret_cast<CarRenderQueueLink *>(solidPack)!=end;
         solidPack=reinterpret_cast<CarRenderSolidPack *>(solidPack->link00.next00)) {
        if (solidPack->value7C == 0xffff) CarRenderBuildPackTextureReferences(solidPack);
        if (static_cast<unsigned char>(mode)) {
            bool changed = false;
            CarRenderQueueLink *node = solidPack->list88.next00;
            while (node != &solidPack->list88) {
                CarRenderQueueLink *next = node->next00;
                CarRenderTextureSlot *slot = reinterpret_cast<CarRenderPointerNode *>(node)->slot08;
                CarRenderTexture *texture = slot->texture04;
                if (texture == ug2TextureDefault82DB08 && texture == ug2TextureDefault82DB08) {
                    CarRenderTexture *found = pack->FindLoaded(slot->hash00);
                    if (found) { found->used53 = 1; texture = found; }
                }
                if (texture != slot->texture04) {
                    Unlink(node);
                    bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));
                    --solidPack->value7E;
                    changed = true;
                }
                node = next;
            }
            if (changed) {
                node = solidPack->list80.next00;
                while (node != &solidPack->list80) {
                    if (reinterpret_cast<CarRenderPointerNode *>(node)->pack08 == pack) break;
                    node = node->next00;
                }
                if (node == &solidPack->list80) { Prepend(&solidPack->list80,pack); ++solidPack->value7C; }
                for (int i=0;i<solidPack->count0C;++i) if (solidPack->resources74[i].solid04)
                    solidPack->resources74[i].solid04->RefreshLoadedTextures(pack);
            }
        } else {
            for (CarRenderQueueLink *node=solidPack->list80.next00;node!=&solidPack->list80;node=node->next00) {
                if (reinterpret_cast<CarRenderPointerNode *>(node)->pack08 != pack) continue;
                bool changed = false;
                for (int i=0;i<solidPack->count0C;++i) if (solidPack->resources74[i].solid04 &&
                    solidPack->resources74[i].solid04->RefreshUnloadedTextures(pack)) changed = true;
                if (changed) CarRenderBuildPackTextureReferences(solidPack);
                break;
            }
        }
    }
}

// Original 0x5c0dc0 and 0x51bf70; list/next fields are re-read after refresh.
extern "C" void UG2TextureNotify5C0DC0(CarRenderTexturePack *pack,int mode) {
    CarRenderQueueLink *node = ug2TextureReferences830D08.next00, *end = End(&ug2TextureReferences830D08);
    for (;node!=end;node=node->next00) {
        CarRenderTextureReference5C *reference = reinterpret_cast<CarRenderTextureReference5C *>(node);
        reference->texture14 = CarRenderRefreshTexture(reference->texture14,reference->hash0C,pack,static_cast<char>(mode));
    }
}
extern "C" void UG2TextureNotify51BF70(CarRenderTexturePack *pack,int mode) {
    CarRenderQueueLink *node = ug2FontTextures863768.next00, *end = End(&ug2FontTextures863768);
    for (;node!=end;node=node->next00) {
        CarRenderFontTextureReference *reference = reinterpret_cast<CarRenderFontTextureReference *>(node);
        CarRenderTexture *texture = CarRenderRefreshTexture(reference->texture08,reference->hash18,pack,static_cast<char>(mode));
        if (texture != reference->texture08) reference->texture08 = texture;
    }
}
// Original 0x611910 and 0x611a10. Other hashes do not identify these six records.
CarRenderTextureCacheRecord *CarRenderFindTextureCache(unsigned int hash) {
    switch (hash) {
        case 0x530b82b0: return &ug2TextureCache89F050[0];
        case 0x530b82b1: return &ug2TextureCache89F050[1];
        case 0x530b82b2: return &ug2TextureCache89F050[2];
        case 0x530b82b3: return &ug2TextureCache89F050[3];
        case 0x530b82b4: return &ug2TextureCache89F050[4];
        case 0x530b82b5: return &ug2TextureCache89F050[5];
    }
    return 0;
}
extern "C" void UG2TextureNotify611A10(unsigned int hash) {
    CarRenderTextureCacheRecord *record = CarRenderFindTextureCache(hash);
    if (record) for (unsigned int i=0;i<0x72;++i) record->value00[i] = 0;
}
