#include "NativeTextureUpload.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureUpload.h"
#include <map>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
int ug2TextureSkip870794;
unsigned char ug2TextureDisabled87079C;
unsigned int ug2TextureUploads864F50;
CarRenderQueueLink ug2TextureStates872B4C;
CarRenderDeferredBufferList ug2TextureReleases8763B0;
}
namespace {
struct Level {
    int pitch;
    std::vector<unsigned char> bytes;
};
struct Upload {
    int format;
    std::vector<Level> levels;
};
std::map<unsigned int,Upload> buffers;
unsigned int nextToken=0x10000000;
void CheckGuards(const Level &level) {
    for (size_t i=0;i<16;++i)
        if (level.bytes[i]!=0xa5 || level.bytes[level.bytes.size()-16+i]!=0xa5) abort();
}
}
void NativeInitialiseTextureUploads() {
    static bool ready=false;
    if (ready) return;
    ug2TextureStates872B4C.next00=ug2TextureStates872B4C.previous04=&ug2TextureStates872B4C;
    ug2TextureReleases8763B0.head00.next00=ug2TextureReleases8763B0.head00.previous04=&ug2TextureReleases8763B0.head00;
    ready=true;
}
extern "C" void *UG2TextureMalloc(unsigned int bytes) { return malloc(bytes); }
extern "C" void UG2TextureFree(void *memory) { free(memory); }
extern "C" int UG2TextureCreate(unsigned int device,int width,int height,int levels,unsigned int usage,
    int format,unsigned int pool,unsigned int *output,void *shared) {
    if (device!=1 || usage || pool!=1 || shared) abort();
    *output=0;
    // Bounded CPU upload storage, not a D3D implementation. The verified car
    // packs request ARGB8 or DXT1/3; DXT5 uses the same staging capacity bound.
    if (width<=0 || height<=0 || width>4096 || height>4096 || levels<1 || levels>32 ||
        (format!=21 && format!=0x31545844 && format!=0x33545844 && format!=0x35545844)) return -1;
    Upload upload;upload.format=format;upload.levels.resize(levels);
    for (int i=0;i<levels;++i) {
        unsigned int w=static_cast<unsigned int>(width)>>i,h=static_cast<unsigned int>(height)>>i;
        if (!w) w=1;
        if (!h) h=1;
        // Reserve the uncompressed four-byte upper bound, including small block
        // levels. The recovered uploader decides the actual flat copy length.
        size_t capacity=static_cast<size_t>(w)*h*4;
        if (capacity<16) capacity=16;
        upload.levels[i].pitch=static_cast<int>(w*4);
        upload.levels[i].bytes.resize(capacity+32);
        for (size_t j=0;j<16;++j) {
            upload.levels[i].bytes[j]=0xa5;
            upload.levels[i].bytes[capacity+16+j]=0xa5;
        }
    }
    unsigned int token=nextToken++;if (!token) abort();
    buffers[token]=std::move(upload);*output=token;
    return 0;
}
extern "C" int UG2TextureLock(unsigned int token,unsigned int level,CarRenderLockedTexture *output,
    void *rectangle,unsigned int flags) {
    std::map<unsigned int,Upload>::iterator found=buffers.find(token);
    if (found==buffers.end() || level>=found->second.levels.size() || rectangle || flags) abort();
    Level &data=found->second.levels[level];
    output->pitch00=data.pitch;output->pixels04=&data.bytes[16];return 0;
}
extern "C" int UG2TextureUnlock(unsigned int token,unsigned int level) {
    std::map<unsigned int,Upload>::iterator found=buffers.find(token);
    if (found==buffers.end() || level>=found->second.levels.size()) abort();
    CheckGuards(found->second.levels[level]);
    return 0;
}
extern "C" unsigned int UG2TextureReleaseGpu(unsigned int token) {
    std::map<unsigned int,Upload>::iterator found=buffers.find(token);
    if (found==buffers.end()) abort();
    for (size_t i=0;i<found->second.levels.size();++i) CheckGuards(found->second.levels[i]);
    buffers.erase(found);
    return 0; // Staging storage has a single owner and no in-flight driver work.
}
void NativeDrainTextureUploads() {
    // Run the original bounded retire operation until this CPU lifetime boundary
    // is drained. This does not change the recovered per-pass limit of three.
    while (!ug2TextureDisabled87079C && ug2TextureReleases8763B0.head00.next00!=&ug2TextureReleases8763B0.head00)
        CarRenderRetireTextures();
}
size_t NativeTextureBufferCount() { return buffers.size(); }
bool NativeTextureBufferExists(unsigned int token) { return buffers.count(token)!=0; }
