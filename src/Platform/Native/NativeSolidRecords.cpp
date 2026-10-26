// Platform scaffolding for the recovered solid manager; no draw/lighting algorithm.
#include "NativeSolidRecords.h"
#include "../../Speed/Indep/Src/World/CarRenderSolidManager.h"
#include "../../Speed/Indep/Src/World/CarRenderTexture.h"
#include <time.h>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
CarRenderTechnique *ug2SolidTechniques86EB28[23]; // effect setup is not active yet
CarRenderTechniqueRule ug2SolidRules800464[4] = {{1,4,0},{0x40,2,0},{0x80,5,0},{0,0,0}};
CarRenderQueueLink ug2PlatformSolids872ED0, ug2SolidPacks830D1C;
CarRenderQueueLink ug2LoadedSolids835794, ug2FailedSolids835858;
CarRenderQueueLink ug2UnresolvedSolids82DBC0, ug2OfflineSolids832DA4, ug2QueuedSolids872B44;
CarRenderDeferredBufferList ug2VertexReleases870D28;
int *ug2SolidLoadMode8900D8;
unsigned int ug2SolidDevice870974 = 1; // backend token; never a host COM pointer
int ug2SolidOffline864F60, ug2SolidFlag82DAA8;
unsigned int ug2SolidAuxBytes82D38C, ug2SolidBytes82D390, ug2SolidLoads82D384, ug2SolidChunkBytes82D388;
unsigned int ug2SolidChanged82DAB0, ug2SolidLoadedCount830DA0, ug2SolidPackCount82D380;
unsigned char ug2SolidBuckets830DA4[8192];
unsigned int ug2SolidTimerShift8284E4 = 8;
float ug2SolidTimerScale828028 = 0.000001f; // platform monotonic nanoseconds to milliseconds
float ug2SolidLookupTime82DAC0, ug2SolidResolveMax82DA8C, ug2SolidResolveSum82DA94;
float ug2SolidDetachMax82DA90, ug2SolidDetachSum82DA98, ug2SolidRefTime82DA9C;
unsigned int ug2SolidResolveCount82DAA0, ug2SolidDetachCount82DAA4;
}
void bDeleteListNode(WCollisionLink *node);

namespace {
struct Upload {
    bool vertex;
    std::vector<unsigned char> bytes;
};
std::map<unsigned int, Upload> uploads;
unsigned int nextToken = 1;

void Initialise() {
    static bool ready = false;
    if (ready) return;
    CarRenderQueueLink *heads[] = {&ug2PlatformSolids872ED0,&ug2SolidPacks830D1C,&ug2LoadedSolids835794,
        &ug2FailedSolids835858,&ug2UnresolvedSolids82DBC0,&ug2OfflineSolids832DA4,&ug2QueuedSolids872B44,
        &ug2VertexReleases870D28.head00};
    for (size_t i=0;i<sizeof(heads)/sizeof(heads[0]);++i) heads[i]->next00=heads[i]->previous04=heads[i];
    ready = true;
}
int Create(unsigned int device,unsigned int bytes,unsigned int usage,unsigned int format,unsigned int pool,
           unsigned int *output,void *shared,bool vertex) {
    if (device!=1 || pool!=1 || shared || (vertex ? (usage!=8 || format!=0) : (usage!=0 || format!=0x65))) abort();
    unsigned int token=nextToken++;
    if (!token) abort();
    Upload upload;upload.vertex=vertex;upload.bytes.resize(bytes);
    uploads[token]=std::move(upload);*output=token;
    return 0;
}
// CPU backing storage has no driver work in flight. Flush original deferred
// nodes at this platform lifetime boundary, retaining the recovered enqueue order.
void FlushRetiredVertices() {
    CarRenderQueueLink *head=&ug2VertexReleases870D28.head00;
    while (head->next00!=head) {
        CarRenderDeferredBuffer *node=reinterpret_cast<CarRenderDeferredBuffer *>(head->next00);
        std::map<unsigned int,Upload>::iterator found=uploads.find(node->buffer08);
        if (found==uploads.end() || !found->second.vertex) abort();
        uploads.erase(found);
        node->link00.previous04->next00=node->link00.next00;
        node->link00.next00->previous04=node->link00.previous04;
        bDeleteListNode(reinterpret_cast<WCollisionLink *>(node));
    }
}
}

void NativeInitialiseSolidRecords() { Initialise(); }

extern "C" unsigned int UG2SolidReadTicker() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC,&now);
    const uint64_t ticks=static_cast<uint64_t>(now.tv_sec)*1000000000+now.tv_nsec;
    return static_cast<unsigned int>(ticks >> (ug2SolidTimerShift8284E4 & 31));
}
extern "C" int UG2SolidCreateVertexBuffer(unsigned int d,unsigned int n,unsigned int u,unsigned int f,
                                         unsigned int p,unsigned int *out,void *shared) {
    return Create(d,n,u,f,p,out,shared,true);
}
extern "C" int UG2SolidCreateIndexBuffer(unsigned int d,unsigned int n,unsigned int u,unsigned int f,
                                        unsigned int p,unsigned int *out,void *shared) {
    return Create(d,n,u,f,p,out,shared,false);
}
extern "C" int UG2SolidLockBuffer(unsigned int token,unsigned int offset,unsigned int bytes,void **output,unsigned int flags) {
    std::map<unsigned int,Upload>::iterator found=uploads.find(token);
    if (found==uploads.end() || offset || bytes || flags) abort();
    // A zero-byte vertex upload is legal in the original path; it performs no stores.
    *output=found->second.bytes.empty() ? 0 : &found->second.bytes[0];
    return 0;
}
extern "C" int UG2SolidUnlockBuffer(unsigned int token) {
    if (!uploads.count(token)) abort();
    return 0;
}
extern "C" unsigned int UG2SolidReleaseIndexBuffer(unsigned int token) {
    std::map<unsigned int,Upload>::iterator found=uploads.find(token);
    if (found==uploads.end() || found->second.vertex) abort();
    uploads.erase(found);return 0;
}
bool NativeSolidRecords::Load(const unsigned char *data,size_t bytes) {
    Initialise();
    Clear();
    storage_.resize(bytes+127);
    unsigned char *base=reinterpret_cast<unsigned char *>((reinterpret_cast<uintptr_t>(&storage_[0])+127)&~uintptr_t(127));
    memcpy(base,data,bytes);
    // This platform entry point accepts complete retail files. The recovered
    // callbacks retain their original valid-child-order/data preconditions.
    for (size_t offset=0;offset+8<=bytes;) {
        unsigned int *chunk=reinterpret_cast<unsigned int *>(base+offset);
        size_t length=static_cast<size_t>(chunk[1])+8;
        if (length>bytes-offset) { Clear(); return false; }
        if (chunk[0]==0x80134000) {
            CarRenderLoadSolidPack(chunk);
            roots_.push_back(chunk);
        }
        offset+=length;
    }
    if (roots_.empty()) return false;
    if (getenv("UG2_DEBUG_SOLIDS")) {
        unsigned int bound=0,defaults=0,missing=0;
        for (CarRenderQueueLink *link=ug2SolidPacks830D1C.next00;link!=&ug2SolidPacks830D1C;link=link->next00) {
            CarRenderSolidPack *pack=reinterpret_cast<CarRenderSolidPack *>(link);
            for (int i=0;i<pack->count0C;++i) if (pack->resources74[i].solid04) {
                CarRenderSlotTable *solid=pack->resources74[i].solid04;
                for (int j=0;j<solid->textureCount19;++j) {
                    CarRenderTexture *texture=solid->textures2C[j].texture04;
                    if (!texture) ++missing; else if (texture==ug2TextureDefault82DB08) ++defaults; else ++bound;
                }
            }
        }
        fprintf(stderr,"native solid records: %u packs, %u solids, %zu upload buffers; textures %u matched, %u default, %u missing\n",
            ug2SolidPackCount82D380,ug2SolidLoadedCount830DA0,uploads.size(),bound,defaults,missing);
    }
    return true;
}
void NativeSolidRecords::Clear() {
    if (roots_.empty()) return;
    for (size_t i=0;i<roots_.size();++i) CarRenderUnloadSolidPack(roots_[i]);
    FlushRetiredVertices();
    if (!roots_.empty() && getenv("UG2_DEBUG_SOLIDS")) fprintf(stderr,"native solid records released: %u packs, %u solids, %zu upload buffers\n",
        ug2SolidPackCount82D380,ug2SolidLoadedCount830DA0,uploads.size());
    roots_.clear();
}
NativeSolidRecords::~NativeSolidRecords() { Clear(); }
CarRenderSlotTable *NativeSolidRecords::Find(unsigned int hash) {
    if (storage_.empty()) return 0;
    const uintptr_t begin=reinterpret_cast<uintptr_t>(&storage_[0]), end=begin+storage_.size();
    for (CarRenderQueueLink *link=ug2SolidPacks830D1C.next00;link!=&ug2SolidPacks830D1C;link=link->next00) {
        const uintptr_t address=reinterpret_cast<uintptr_t>(link);
        if (address>=begin && address<end) {
            CarRenderSlotTable *solid=UG2SolidFindLoaded(hash,reinterpret_cast<CarRenderSolidPack *>(link));
            if (solid) return solid;
        }
    }
    return 0;
}
