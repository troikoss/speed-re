#include "EffectInput.h"

// Original 0x670f13 / 0x670f20 / 0x670f2e.
void D3DXEffectResource::Construct() { handle00=0; data04=0; size08=0; }
void D3DXEffectResource::Destroy() { if (handle00) UG2FxDeleteObject(handle00); }
int D3DXEffectResource::Clear() {
    if (handle00) UG2FxDeleteObject(handle00);
    handle00=0; data04=0; size08=0;
    return 0;
}

// Original 0x670f4f. Failed loads retain the original partial/stale fields.
int D3DXEffectResource::Load(unsigned int module,const void *name,int bitmapFirst,int wide) {
    if (handle00) Clear();
    unsigned int (__stdcall *find)(unsigned int,const void *,unsigned int) =
        wide ? UG2FxFindResourceW : UG2FxFindResourceA;
    unsigned int info=0;
    if (bitmapFirst) info=find(module,name,2);
    if (!info) info=find(module,name,10);
    if (info) {
        size08=UG2FxSizeofResource(module,info);
        if (size08) {
            handle00=UG2FxLoadResource(module,info);
            if (handle00) {
                data04=UG2FxLockResource(handle00);
                if (data04) return 0;
            }
        }
    }
    UG2FxGetLastError();
    return static_cast<int>(0x88760b59);
}

// Original 0x670e8e / 0x670da2.
D3DXEffectBuffer *D3DXEffectBuffer::Construct() {
    data0C=0; size08=0; class00=0x7b711c; refs04=1;
    return this;
}
void D3DXEffectBuffer::Destroy() { class00=0x7b711c; UG2FxFree(data0C); }

// Original 0x670db2. A failed allocation does not replace the size word.
int D3DXEffectBuffer::Allocate(unsigned int size) {
    data0C=static_cast<unsigned char *>(UG2FxMalloc(size));
    if (!data0C) return static_cast<int>(0x8007000e);
    size08=size;
    return 0;
}

// Original 0x670e24. Only flags bit zero controls the second free.
D3DXEffectBuffer *D3DXEffectBuffer::Delete(unsigned char flags) {
    Destroy();
    if (flags&1) UG2FxFree(this);
    return this;
}

// Original 0x670e45. Clear output before reading either IID, including aliases.
extern "C" int __stdcall D3DXEffectBufferQuery(D3DXEffectBuffer *buffer,
    const unsigned int *iid,D3DXEffectBuffer **output) {
    *output=0;
    int i=0;
    while (i<4 && iid[i]==ug2FxIUnknown7D9904[i]) ++i;
    if (i!=4) {
        i=0;
        while (i<4 && iid[i]==ug2FxBufferIid7B7430[i]) ++i;
        if (i!=4) return static_cast<int>(0x80004002);
    }
    *output=buffer;
    UG2FxBufferAddRefVirtual(buffer);
    return 0;
}
// Original 0x670dde / 0x670df0. Unsigned reference-count wrap is preserved.
extern "C" unsigned int __stdcall D3DXEffectBufferAddRef(D3DXEffectBuffer *buffer) {
    return ++buffer->refs04;
}
extern "C" unsigned int __stdcall D3DXEffectBufferRelease(D3DXEffectBuffer *buffer) {
    unsigned int old=buffer->refs04;
    unsigned int count=old-1;
    buffer->refs04=count;
    if (old==1) UG2FxBufferDeleteVirtual(buffer,1);
    return count;
}
// Original 0x6954f7 / 0x670e15.
extern "C" unsigned char *__stdcall D3DXEffectBufferPointer(D3DXEffectBuffer *buffer) { return buffer->data0C; }
extern "C" unsigned int __stdcall D3DXEffectBufferLength(D3DXEffectBuffer *buffer) { return buffer->size08; }

// Original 0x670ea6. Output is written only after successful allocation.
extern "C" int __stdcall D3DXEffectBufferCreate(unsigned int size,D3DXEffectBuffer **output) {
    if (!output) return static_cast<int>(0x8876086c);
    D3DXEffectBuffer *buffer=static_cast<D3DXEffectBuffer *>(UG2FxMalloc(0x10));
    if (buffer) buffer=buffer->Construct();
    if (!buffer) return static_cast<int>(0x8007000e);
    int result=UG2FxBufferAllocateVirtual(buffer,size);
    if (result<0) { UG2FxBufferDeleteVirtual(buffer,1); return result; }
    *output=buffer;
    return 0;
}
// Original forwarding entry 0x670f08.
extern "C" int __stdcall D3DXEffectBufferCreateForward(unsigned int size,D3DXEffectBuffer **output) {
    return D3DXEffectBufferCreate(size,output);
}
