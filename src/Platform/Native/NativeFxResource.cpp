#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectInput.h"
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <set>

namespace {
thread_local unsigned int lastError;
std::set<void *> allocations;
const unsigned int infoBase=0x60000000,handleBase=0x61000000;
unsigned int Find(unsigned int module,const void *name,unsigned int type,bool wide) {
    if (module || !IsOriginalImageMapped()) { lastError=6; return 0; }
    uintptr_t raw=reinterpret_cast<uintptr_t>(name);unsigned int identifier=0,index=0;
    std::string text;
    if (raw<0x10000) identifier=static_cast<unsigned int>(raw);
    else {
        for (size_t i=0;;++i) {
            unsigned int ch=wide ? static_cast<const unsigned short *>(name)[i] : static_cast<const unsigned char *>(name)[i];
            if (!ch) break;
            if (ch>127) { lastError=1814; return 0; }
            text.push_back(static_cast<char>(ch));
        }
        if (text.size()>1 && text[0]=='#') {
            identifier=0;
            for (size_t i=1;i<text.size();++i) {
                if (text[i]<'0' || text[i]>'9' || identifier>6553) { lastError=1814; return 0; }
                identifier=identifier*10+text[i]-'0';
            }
            if (identifier>65535) { lastError=1814; return 0; }
            text.clear();
        }
    }
    index=FindOriginalResource(type,raw>=0x10000 && !text.empty() ? text.c_str() : 0,identifier);
    if (!index) { lastError=1814; return 0; }
    return infoBase+index;
}
}
// Current-module ASCII names (ANSI or UTF-16) and numeric identifiers from the
// verified PE image only. This is not a general Win32 resource/GDI implementation.
extern "C" void *UG2FxMalloc(unsigned int size) {
    void *memory=malloc(size);if (memory) allocations.insert(memory);return memory;
}
extern "C" void UG2FxFree(void *memory) { allocations.erase(memory);free(memory); }
size_t NativeFxAllocationCount() { return allocations.size(); }
extern "C" unsigned int __stdcall UG2FxFindResourceA(unsigned int module,const void *name,unsigned int type) { return Find(module,name,type,false); }
extern "C" unsigned int __stdcall UG2FxFindResourceW(unsigned int module,const void *name,unsigned int type) { return Find(module,name,type,true); }
extern "C" unsigned int __stdcall UG2FxSizeofResource(unsigned int module,unsigned int info) {
    unsigned int size=0;
    if (module || info<=infoBase || !OriginalResourceData(info-infoBase,&size)) { lastError=6; return 0; }
    return size;
}
extern "C" unsigned int __stdcall UG2FxLoadResource(unsigned int module,unsigned int info) {
    unsigned int size=0;
    if (module || info<=infoBase || !OriginalResourceData(info-infoBase,&size)) { lastError=6; return 0; }
    return handleBase+(info-infoBase);
}
extern "C" const unsigned char *__stdcall UG2FxLockResource(unsigned int handle) {
    unsigned int size=0;const unsigned char *data=0;
    if (handle>handleBase) data=OriginalResourceData(handle-handleBase,&size);
    if (!data) lastError=6;
    return data;
}
extern "C" int __stdcall UG2FxDeleteObject(unsigned int) {
    // Loaded PE resource handles are not GDI objects. Retain their immutable
    // bytes for the mapped module lifetime; the recovered caller ignores BOOL.
    lastError=6;return 0;
}
extern "C" unsigned int __stdcall UG2FxGetLastError() { return lastError; }
