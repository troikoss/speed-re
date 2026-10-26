#include "CarRenderTexturePack.h"
#include "CarRenderSolidPack.h"
#include "../../Libs/bWare/bSlotPool.h"

#ifdef __GNUC__
typedef __UINTPTR_TYPE__ TextureAddress;
#else
typedef unsigned __int64 TextureAddress;
#endif
namespace {
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
// Original 0x48b1f0, including the original pool size/count/name/heap flags.
void CarRenderInitTexturePackPool() {
    ug2TexturePackPool82DB0C = bNewSlotPool(0x7c,0x80,ug2TexturePoolName78D950,0);
}
// Original 0x5b96f0: state is read before allocation, token after allocation.
unsigned int *__fastcall CarRenderTextureSnapshot(CarRenderTexture *texture) {
    CarRenderTextureState *state = texture->state00;
    unsigned int *snapshot = static_cast<unsigned int *>(bOMalloc(ug2TextureSnapshotPool870790));
    *snapshot = state->texture18;
    return snapshot;
}
// Original 0x5b9710.
void __stdcall CarRenderFreeTextureSnapshot(unsigned int *snapshot) { bFree(ug2TextureSnapshotPool870790,snapshot); }

// Original 0x494380. Valid child order puts the header before its indices.
// The original SEH construction guard is not a new malformed-data/OOM policy.
CarRenderTexturePackHeader *CarRenderLoadTextureHeader(unsigned int *chunk) {
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderTexturePackHeader *header = 0;
    CarRenderTexture *textures = 0;
    CarRenderTextureState *states = 0;
    unsigned int count = 0, stateCount = 0, indexCount = 0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)!=end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0] == 0x33310001) header = reinterpret_cast<CarRenderTexturePackHeader *>(child+2);
        else if (child[0] == 0x33310002) {
            header->indices74 = reinterpret_cast<CarRenderTextureIndex *>(child+2);
            indexCount = child[1] >> 3;
        } else if (child[0] == 0x33310003) header->slots78 = reinterpret_cast<CarRenderTextureStreamSlot *>(child+2);
        else if (child[0] == 0x33310004) { textures = reinterpret_cast<CarRenderTexture *>(child+2); count = child[1]/0x7c; }
        else if (child[0] == 0x33310005) { states = reinterpret_cast<CarRenderTextureState *>(child+2); stateCount = child[1] >> 5; }
    }
    header->loaded6C = 1;
    if (count != stateCount) return 0;
    CarRenderTexturePack *pack = static_cast<CarRenderTexturePack *>(bSlotPoolAllocate(ug2TexturePackPool82DB0C));
    if (pack) pack = pack->Construct(header,static_cast<int>(indexCount),0,0);
    header->pack70 = pack;
    if (textures) pack->RegisterArray(textures,states,static_cast<int>(count));
    return header;
}
// Original 0x494dd0. Range detachment runs before any index unregistration.
void CarRenderUnloadTextureHeader(unsigned int *chunk) {
    unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
    CarRenderTexturePackHeader *header = 0;
    CarRenderTexture *textures = 0;
    CarRenderTextureState *states = 0;
    unsigned int count = 0;
    for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)!=end;
         child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
        if (child[0] == 0x33310001) header = reinterpret_cast<CarRenderTexturePackHeader *>(child+2);
        else if (child[0] == 0x33310004) { textures = reinterpret_cast<CarRenderTexture *>(child+2); count = child[1]/0x7c; }
        else if (child[0] == 0x33310005) states = reinterpret_cast<CarRenderTextureState *>(child+2);
    }
    header->pack70->Detach(0,static_cast<int>(header->pack70->value78));
    if (textures) header->pack70->UnregisterArray(textures,states,static_cast<int>(count));
    CarRenderTexturePack *pack = header->pack70;
    if (pack) { pack->Destroy(); bFree(ug2TexturePackPool82DB0C,pack); }
    header->pack70 = 0;
}
// Original 0x48fc00. The queue link is inside the chunk at +0x10.
void CarRenderQueueTexturePixels(unsigned int *chunk) {
    unsigned char *base = reinterpret_cast<unsigned char *>(chunk);
    unsigned int bytes = *reinterpret_cast<unsigned int *>(base+chunk[3]+0x14);
    if (!chunk[8]) chunk[8] = 1;
    *reinterpret_cast<unsigned char **>(chunk+9) = base+bytes+chunk[3]+0x18;
    Append(&ug2TexturePixelQueue8358C8,reinterpret_cast<CarRenderQueueLink *>(chunk+4));
}
// Original 0x48fbc0. The key is three words after the embedded link.
CarRenderQueueLink *CarRenderFindTexturePixels(unsigned int key) {
    CarRenderQueueLink *node = ug2TexturePixelQueue8358C8.next00;
    CarRenderQueueLink *end = reinterpret_cast<CarRenderQueueLink *>(
        reinterpret_cast<CarRenderLightList *>(&ug2TexturePixelQueue8358C8)->End());
    while (node != end) {
        if (reinterpret_cast<unsigned int *>(node)[3] == key) return node;
        node = node->next00;
    }
    return 0;
}

// Original 0x48fe60, __thiscall RET 0x14.
CarRenderTextureAnimation *CarRenderTextureAnimation::Construct(CarRenderTexturePack *pack,
    CarRenderTextureAnimationEntry *entries, CarRenderTextureAnimationFrame *frames, int count, unsigned int value) {
    pack08 = pack; entries0C = entries; frames10 = frames; count14 = count; value18 = value;
    for (int i=0;i<count;++i) {
        entries0C[i].frames28 = frames;
        frames = reinterpret_cast<CarRenderTextureAnimationFrame *>(reinterpret_cast<unsigned char *>(frames)+
            (static_cast<unsigned int>(entries0C[i].count1C) << 4));
    }
    Append(&ug2TextureAnimations82DC00,&link00);
    return this;
}
// Original 0x48ff40. Readiness is all-or-nothing per entry, including empty entries.
void CarRenderTextureAnimation::Update() {
    CarRenderTextureAnimationEntry *entry = entries0C;
    int remaining = count14;
    for (;remaining>0;--remaining,++entry) {
        int frames = entry->count1C;
        CarRenderTextureAnimationFrame *frame = entry->frames28;
        unsigned int ready = 1;
        for (;frames>0;--frames,++frame) {
            CarRenderTexture *texture = 0;
            if (ug2TextureBuckets833784[frame->hash00 & 0x1fff] && pack08->header70 && pack08->header70->indices74) {
                CarRenderTextureIndex *index = static_cast<CarRenderTextureIndex *>(CarRenderFindResource(
                    frame->hash00,pack08->header70->indices74,pack08->count74,0,8));
                if (index && index->texture04 && index->texture04->pixels70 &&
                    (!index->texture04->paletteBytes3C || index->texture04->palette74)) texture = index->texture04;
            }
            if (texture) { frame->texture04 = texture; frame->snapshot08 = CarRenderTextureSnapshot(texture); }
            else { frame->texture04 = 0; frame->snapshot08 = 0; ready = 0; }
        }
        entry->ready2C = ready;
        entry->value30 = 0;
    }
}
// Original 0x48fed0. Only nonnull texture pointers cause snapshot release.
void CarRenderTextureAnimation::Destroy() {
    for (int i=0;i<count14;++i) {
        CarRenderTextureAnimationEntry *entry = entries0C+i;
        int count = entry->count1C;
        CarRenderTextureAnimationFrame *frame = entry->frames28;
        for (;count>0;--count,++frame) if (frame->texture04) CarRenderFreeTextureSnapshot(frame->snapshot08);
    }
    Unlink(&link00);
}

// Original 0x494ea0, cdecl. The B332 queue branch is inlined in the original
// B330 walk; retain that behavior and the final header's pixel-key selection.
unsigned int CarRenderLoadTexturePack(unsigned int *chunk) {
    unsigned char *base = reinterpret_cast<unsigned char *>(chunk);
    if (chunk[0] == 0xb3300000) {
        unsigned char *end = base+chunk[1]+8;
        CarRenderTexturePackHeader *header = 0;
        for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)!=end;
             child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
            if (child[0] == 0xb3310000) {
                header = CarRenderLoadTextureHeader(child);
                ug2TextureCurrentPack82DB10 = header->pack70;
            } else if (child[0] == 0xb3320000) {
                unsigned char *p = reinterpret_cast<unsigned char *>(child);
                unsigned int bytes = *reinterpret_cast<unsigned int *>(p+child[3]+0x14);
                if (!child[8]) child[8] = 1;
                *reinterpret_cast<unsigned char **>(child+9) = p+bytes+child[3]+0x18;
                Append(&ug2TexturePixelQueue8358C8,reinterpret_cast<CarRenderQueueLink *>(child+4));
            }
        }
        CarRenderQueueLink *queued = CarRenderFindTexturePixels(*reinterpret_cast<unsigned int *>(
            reinterpret_cast<unsigned char *>(header)+0x60));
        if (queued) {
            unsigned int *pixels = *reinterpret_cast<unsigned int **>(reinterpret_cast<unsigned char *>(queued)+0x14);
            unsigned char *data = reinterpret_cast<unsigned char *>((reinterpret_cast<TextureAddress>(pixels)+0x87)&~TextureAddress(127));
            unsigned int bytes = static_cast<unsigned int>(reinterpret_cast<unsigned char *>(pixels)+pixels[1]+8-data);
            CarRenderTexturePack *pack = header->pack70;
            pack->Attach(data,0,static_cast<int>(bytes));
            pack->value78 = bytes;
            Unlink(queued);
        }
        return 1;
    }
    if (chunk[0] == 0xb3320000) { CarRenderQueueTexturePixels(chunk); return 1; }
    if (chunk[0] != 0xb0300100) return 0;
    unsigned int bytes = *reinterpret_cast<unsigned int *>(base+chunk[3]+0x14);
    if (chunk[4] != 1) { chunk[5] = 0; chunk[6] = 1; return 1; }
    unsigned int extra = *reinterpret_cast<unsigned int *>(base+bytes+chunk[3]+0x1c);
    CarRenderTextureAnimation *animation = static_cast<CarRenderTextureAnimation *>(bSlotPoolAllocate(ug2TexturePackPool82DB0C));
    if (animation) animation = animation->Construct(ug2TextureCurrentPack82DB10,
        reinterpret_cast<CarRenderTextureAnimationEntry *>(base+chunk[3]+0x18),
        reinterpret_cast<CarRenderTextureAnimationFrame *>(base+bytes+chunk[3]+0x20),static_cast<int>(bytes/0x34),extra >> 4);
    *reinterpret_cast<CarRenderTextureAnimation **>(chunk+5) = animation;
    animation->Update(); // original also calls through null on failed allocation
    chunk[6] = 1;
    return 1;
}
// Original 0x495040. A direct pixel unload is accepted without queue removal.
unsigned int CarRenderUnloadTexturePack(unsigned int *chunk) {
    if (chunk[0] == 0xb3300000) {
        unsigned char *end = reinterpret_cast<unsigned char *>(chunk)+chunk[1]+8;
        for (unsigned int *child=chunk+2; reinterpret_cast<unsigned char *>(child)!=end;
             child=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child)+child[1]+8)) {
            if (child[0] == 0xb3310000) { CarRenderUnloadTextureHeader(child); ug2TextureCurrentPack82DB10 = 0; }
        }
        return 1;
    }
    if (chunk[0] == 0xb3320000) return 1;
    if (chunk[0] != 0xb0300100) return 0;
    if (chunk[4] == 1) {
        CarRenderTextureAnimation *animation = *reinterpret_cast<CarRenderTextureAnimation **>(chunk+5);
        if (animation) { animation->Destroy(); bFree(ug2TexturePackPool82DB0C,animation); }
    }
    return 1;
}
