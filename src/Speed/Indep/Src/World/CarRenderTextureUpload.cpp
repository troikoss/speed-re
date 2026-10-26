#include "CarRenderTextureUpload.h"

void bDeleteListNode(WCollisionLink *node);
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

// Original 0x5caaa0, fastcall receiver. Retirement precedes clearing/unlinking.
extern "C" void __fastcall UG2TextureRelease(CarRenderTexture *texture) {
    CarRenderTextureState *state = texture->state00;
    if (state->texture18) {
        ug2TextureReleases8763B0.Add(state->texture18);
        state->texture18 = 0;
        Unlink(&state->link00);
    }
}

// Original 0x5ce170, fastcall receiver. Preserve palette conversion, the early
// return/free placement, signed mip arithmetic and forward REP-style copies.
extern "C" void __fastcall UG2TextureUpload(CarRenderTexture *texture) {
    CarRenderTextureState *state = texture->state00;
    if (state->texture18) {
        if (state->texture18) {
            ug2TextureReleases8763B0.Add(state->texture18);
            state->texture18 = 0;
            Unlink(&state->link00);
        }
    }
    if (!texture->pixels70) return;
    int format = state->format14;
    int levelBytes = static_cast<int>(texture->value40);
    unsigned char *start = texture->pixels70;
    bool expanded = false;
    if (format == 0x29) {
        unsigned char *source = texture->pixels70;
        unsigned int *palette = reinterpret_cast<unsigned int *>(texture->palette74);
        start = static_cast<unsigned char *>(UG2TextureMalloc(texture->pixelBytes38 << 2));
        unsigned int *output = reinterpret_cast<unsigned int *>(start);
        expanded = true;
        for (int i=0;i<texture->mipCount4E;++i) {
            int width = static_cast<int>(texture->width44) >> (i & 31);
            int height = static_cast<int>(texture->height46) >> (i & 31);
            int count = static_cast<int>(static_cast<unsigned int>(width)*static_cast<unsigned int>(height));
            for (;count>0;--count) *output++ = palette[*source++];
        }
        levelBytes = static_cast<int>(static_cast<unsigned int>(levelBytes) << 2);
        format = 0x15;
        texture->compression4A = 0x20;
    }
    if (ug2TextureDisabled87079C) { state->texture18 = 0; return; }
    if (ug2TextureSkip870794 && texture->mipCount4E > ug2TextureSkip870794) {
        int divisor = static_cast<int>(static_cast<unsigned int>(ug2TextureSkip870794)*2);
        UG2TextureCreate(ug2SolidDevice870974,texture->width44/divisor,texture->height46/divisor,
            texture->mipCount4E-ug2TextureSkip870794,0,format,1,&state->texture18,0);
    } else {
        UG2TextureCreate(ug2SolidDevice870974,texture->width44,texture->height46,texture->mipCount4E,
            0,format,1,&state->texture18,0);
    }
    if (state->texture18) {
        ++ug2TextureUploads864F50;
        Append(&ug2TextureStates872B4C,&state->link00);
        int levels = texture->mipCount4E;
        unsigned char *position = start;
        if (ug2TextureSkip870794 && levels > ug2TextureSkip870794 && ug2TextureSkip870794 > 0) {
            int skip = ug2TextureSkip870794;
            levels -= skip;
            for (;skip>0;--skip) { position += levelBytes; levelBytes /= 4; }
        }
        unsigned int shift = 0;
        for (int level=0;level<levels;++level,shift+=2) {
            CarRenderLockedTexture locked;
            UG2TextureLock(state->texture18,level,&locked,0,0);
            int bytes = levelBytes >> (shift & 31);
            unsigned int *destination = reinterpret_cast<unsigned int *>(locked.pixels04);
            unsigned int *source = reinterpret_cast<unsigned int *>(position);
            unsigned int words = static_cast<unsigned int>(bytes) >> 2;
            for (unsigned int i=0;i<words;++i) destination[i] = source[i];
            unsigned char *tailOut = reinterpret_cast<unsigned char *>(destination+words);
            unsigned char *tailIn = reinterpret_cast<unsigned char *>(source+words);
            for (unsigned int i=0;i<(static_cast<unsigned int>(bytes)&3);++i) tailOut[i] = tailIn[i];
            UG2TextureUnlock(state->texture18,level);
            position += bytes;
        }
        if (expanded) UG2TextureFree(start);
    }
}

// Original 0x5ce4c0. Return counts are ignored; only three nodes are retired.
void CarRenderRetireTextures() {
    if (!ug2TextureDisabled87079C) {
        for (int i=0;i<3 && ug2TextureReleases8763B0.head00.next00!=&ug2TextureReleases8763B0.head00;++i) {
            CarRenderDeferredBuffer *node = reinterpret_cast<CarRenderDeferredBuffer *>(ug2TextureReleases8763B0.head00.next00);
            UG2TextureReleaseGpu(node->buffer08);
            node = reinterpret_cast<CarRenderDeferredBuffer *>(ug2TextureReleases8763B0.head00.next00);
            Unlink(&node->link00);
            bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));
        }
    }
}
// Original 0x5ce400. Live handles are re-read between Release calls; deferred
// handles are captured once. The queue nodes are freed only after refcount zero.
void CarRenderReleaseTextures() {
    while (ug2TextureStates872B4C.next00 != &ug2TextureStates872B4C) {
        CarRenderTextureState *state = reinterpret_cast<CarRenderTextureState *>(ug2TextureStates872B4C.next00);
        Unlink(&state->link00);
        if (state->texture18) {
            while (UG2TextureReleaseGpu(state->texture18)) {}
            state->texture18 = 0;
        }
    }
    ug2TextureStates872B4C.next00 = ug2TextureStates872B4C.previous04 = &ug2TextureStates872B4C;
    while (ug2TextureReleases8763B0.head00.next00 != &ug2TextureReleases8763B0.head00) {
        CarRenderDeferredBuffer *node = reinterpret_cast<CarRenderDeferredBuffer *>(ug2TextureReleases8763B0.head00.next00);
        unsigned int token = node->buffer08;
        while (UG2TextureReleaseGpu(token)) {}
        node = reinterpret_cast<CarRenderDeferredBuffer *>(ug2TextureReleases8763B0.head00.next00);
        Unlink(&node->link00);
        bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));
    }
}
