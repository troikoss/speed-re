// Native ownership and explicit backend boundaries; algorithms live in World/.
#include "NativeTextureRecords.h"
#include "NativeSolidRecords.h"
#include "NativeTextureUpload.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureReferences.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureUpload.h"
#include "../../Speed/Indep/Src/World/CarRenderTexturePack.h"
#include "../../Speed/Indep/Libs/bWare/Compression.h"
#include "../../Speed/Indep/Libs/bWare/bSlotPool.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

extern bSlotPoolRegistry slotPoolRegistry828530;
extern "C" {
CarRenderQueueLink ug2TexturePacks830AF0, ug2TexturePixelQueue8358C8, ug2TextureAnimations82DC00;
unsigned int ug2TextureCount833780, ug2TextureChanged82DAB4;
unsigned char ug2TextureBuckets833784[8192];
unsigned int ug2TextureOverrideCount82DA80, ug2TextureOverrideKeys82D530[36];
CarRenderTexture *ug2TextureOverrideValues82D5C0[36], *ug2TextureDefault82DB08;
void *ug2TexturePackPool82DB0C, *ug2TextureSnapshotPool870790;
CarRenderTexturePack *ug2TextureCurrentPack82DB10;
CarRenderQueueLink ug2TextureReferences830D08, ug2FontTextures863768;
CarRenderTextureCacheRecord ug2TextureCache89F050[6];
const char ug2TextureNull78DA04[] = "<null>";
const char ug2TextureLabel78D9F4[] = "TexturePack,%s";
const char ug2TextureInventory78D9C0[] = "[Enable 'CodeineInventoryTextures' for this to work";
const char ug2TexturePoolName78D950[] = "TexturePackSlotPool";
}
namespace {
void Initialise() {
    NativeInitialiseSolidRecords();
    static bool ready = false;
    if (!ready) {
        CarRenderQueueLink *heads[] = {&ug2TexturePacks830AF0,&ug2TexturePixelQueue8358C8,&ug2TextureAnimations82DC00,
            &ug2TextureReferences830D08,&ug2FontTextures863768};
        for (size_t i=0;i<sizeof(heads)/sizeof(heads[0]);++i) heads[i]->next00=heads[i]->previous04=heads[i];
        ready = true;
    }
    NativeInitialiseTextureUploads();
    if (!ug2TexturePackPool82DB0C) CarRenderInitTexturePackPool();
    if (!ug2TexturePackPool82DB0C) abort();
}
unsigned char *Aligned(std::vector<unsigned char> &storage) {
    return reinterpret_cast<unsigned char *>((reinterpret_cast<uintptr_t>(&storage[0])+127)&~uintptr_t(127));
}
}
extern "C" void UG2TextureMissingDefault() {
    fprintf(stderr,"native texture records: requested DEFAULTALPHA is absent\n");
    raise(SIGTRAP);
}
struct NativeTextureRecords::File {
    struct Slot {
        std::vector<unsigned char> storage;
        CarRenderTextureRequest request;
        CarRenderTextureStreamOwner owner;
    };
    std::string key;
    std::vector<unsigned char> storage;
    std::vector<unsigned int *> roots;
    std::vector<CarRenderTexturePackHeader *> headers;
    std::vector<Slot *> slots;
    ~File() {
        for (size_t i=slots.size();i>0;--i) {
            Slot *slot=slots[i-1];
            CarRenderUnloadTextureSlot(Aligned(slot->storage),&slot->request,&slot->owner);
            delete slot;
        }
        for (size_t i=roots.size();i>0;--i) CarRenderUnloadTexturePack(roots[i-1]);
    }
};
bool NativeTextureRecords::Add(const unsigned char *data,size_t bytes,const std::string &key) {
    for (size_t i=0;i<files_.size();++i) if (files_[i]->key==key) return true;
    if (!bytes) return true;
    Initialise();
    File *file=new File; file->key=key; file->storage.resize(bytes+127);
    unsigned char *base=Aligned(file->storage); memcpy(base,data,bytes);
    for (size_t offset=0;offset+8<=bytes;) {
        unsigned int *chunk=reinterpret_cast<unsigned int *>(base+offset);
        size_t length=static_cast<size_t>(chunk[1])+8;
        if (length>bytes-offset) { delete file; return false; }
        if (chunk[0]==0xb3300000) {
            // Check file extents before handing valid retail child order to
            // the recovered callbacks. Assets and decoded storage stay owned.
            for (size_t p=offset+8;p<offset+length;) {
                if (p+8>offset+length) { delete file; return false; }
                unsigned int *child=reinterpret_cast<unsigned int *>(base+p);
                size_t childBytes=static_cast<size_t>(child[1])+8;
                if (childBytes>offset+length-p) { delete file; return false; }
                p+=childBytes;
            }
            CarRenderLoadTexturePack(chunk); file->roots.push_back(chunk);
            for (size_t p=offset+8;p<offset+length;) {
                unsigned int *child=reinterpret_cast<unsigned int *>(base+p);
                if (child[0]==0xb3310000) {
                    for (size_t q=p+8;q<p+child[1]+8;) {
                        unsigned int *part=reinterpret_cast<unsigned int *>(base+q);
                        if (part[0]==0x33310001) file->headers.push_back(reinterpret_cast<CarRenderTexturePackHeader *>(part+2));
                        q+=static_cast<size_t>(part[1])+8;
                    }
                }
                p+=static_cast<size_t>(child[1])+8;
            }
        }
        offset+=length;
    }
    // Synchronous stream backend. Use the original decompressor, then original
    // slot callbacks and real per-pack sorted indices, including duplicate keys.
    for (size_t h=0;h<file->headers.size();++h) {
        CarRenderTexturePackHeader *header=file->headers[h];
        if (!header->slots78) continue;
        for (int i=0;i<header->pack70->count74;++i) {
            const CarRenderTextureStreamSlot &entry=header->slots78[i];
            if (entry.offset04>bytes || entry.packed08>bytes-entry.offset04 || entry.bytes0C<0x9c) { delete file; return false; }
            std::vector<unsigned char> source(entry.packed08+64);
            memcpy(&source[0],base+entry.offset04,entry.packed08);
            File::Slot *slot=new File::Slot;
            memset(&slot->request,0,sizeof(slot->request)); memset(&slot->owner,0,sizeof(slot->owner));
            slot->storage.resize(static_cast<size_t>(entry.bytes0C)+127+64);
            unsigned char *decoded=Aligned(slot->storage);
            if (bDecompress(reinterpret_cast<unsigned int *>(&source[0]),decoded)!=static_cast<int>(entry.bytes0C)) {
                delete slot; delete file; return false;
            }
            CarRenderTexture *texture=reinterpret_cast<CarRenderTexture *>(decoded+entry.bytes0C-0x9c);
            if (texture->hash24!=entry.hash00) { delete slot; delete file; return false; }
            slot->request.bytes0C=entry.bytes0C; slot->owner.headerBC=header;
            CarRenderLoadTextureSlot(decoded,&slot->request,&slot->owner);
            file->slots.push_back(slot);
        }
    }
    if (file->roots.empty()) { delete file; return true; }
    files_.push_back(file);
    if (getenv("UG2_DEBUG_TEXTURES")) fprintf(stderr,"native texture records: %zu packs, %zu textures, %zu staged upload handles\n",
        PackCount(),TextureCount(),NativeTextureBufferCount());
    return true;
}
void NativeTextureRecords::Clear() {
    for (size_t i=files_.size();i>0;--i) delete files_[i-1];
    files_.clear();
    if (ug2TexturePackPool82DB0C) NativeDrainTextureUploads();
    if (ug2TexturePackPool82DB0C && ug2TexturePacks830AF0.next00==&ug2TexturePacks830AF0) {
        bSlotPool *pool=static_cast<bSlotPool *>(ug2TexturePackPool82DB0C);
        if (pool->used18) abort();
        if (NativeTextureBufferCount() || ug2TextureStates872B4C.next00!=&ug2TextureStates872B4C) abort();
        slotPoolRegistry828530.Delete(pool); ug2TexturePackPool82DB0C=0;
    }
    if (getenv("UG2_DEBUG_TEXTURES")) fprintf(stderr,"native texture records released: %u textures, %zu staged handles\n",
        ug2TextureCount833780,NativeTextureBufferCount());
}
NativeTextureRecords::~NativeTextureRecords() { Clear(); }
size_t NativeTextureRecords::PackCount() const {
    size_t count=0; for (size_t i=0;i<files_.size();++i) count+=files_[i]->headers.size(); return count;
}
size_t NativeTextureRecords::TextureCount() const {
    size_t count=0;
    for (size_t i=0;i<files_.size();++i) for (size_t j=0;j<files_[i]->headers.size();++j)
        count+=files_[i]->headers[j]->pack70->count74;
    return count;
}
