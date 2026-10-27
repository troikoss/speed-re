#ifndef UG2_D3DX_EFFECT_INPUT_H
#define UG2_D3DX_EFFECT_INPUT_H

// UG2's statically linked D3DX resource/buffer implementation. Four-byte
// original class tokens are data, never native function-pointer tables.
struct D3DXEffectResource {
    unsigned int handle00;
    const unsigned char *data04;
    unsigned int size08;
    void Construct(); // 0x670f13
    void Destroy(); // 0x670f20; does not clear fields
    int Clear(); // 0x670f2e
    int Load(unsigned int module,const void *name,int bitmapFirst,int wide); // 0x670f4f, RET 16
};
struct D3DXEffectBuffer {
    unsigned int class00,refs04,size08;
    unsigned char *data0C;
    D3DXEffectBuffer *Construct(); // 0x670e8e
    void Destroy(); // 0x670da2; preserves the dangling data pointer
    D3DXEffectBuffer *Delete(unsigned char flags); // 0x670e24, RET 4
    int Allocate(unsigned int size); // 0x670db2, RET 4
};
typedef char D3DXEffectResourceSize[(sizeof(D3DXEffectResource)==12)?1:-1];
typedef char D3DXEffectBufferSize[(sizeof(D3DXEffectBuffer)==16)?1:-1];
extern "C" {
int __stdcall D3DXEffectBufferQuery(D3DXEffectBuffer *buffer,const unsigned int *iid,D3DXEffectBuffer **output); // 0x670e45
unsigned int __stdcall D3DXEffectBufferAddRef(D3DXEffectBuffer *buffer); // 0x670dde
unsigned int __stdcall D3DXEffectBufferRelease(D3DXEffectBuffer *buffer); // 0x670df0
unsigned char *__stdcall D3DXEffectBufferPointer(D3DXEffectBuffer *buffer); // 0x6954f7
unsigned int __stdcall D3DXEffectBufferLength(D3DXEffectBuffer *buffer); // 0x670e15
int __stdcall D3DXEffectBufferCreate(unsigned int size,D3DXEffectBuffer **output); // 0x670ea6
int __stdcall D3DXEffectBufferCreateForward(unsigned int size,D3DXEffectBuffer **output); // 0x670f08
extern const unsigned int ug2FxIUnknown7D9904[4],ug2FxBufferIid7B7430[4];
void *UG2FxMalloc(unsigned int size); // 0x575620
void UG2FxFree(void *memory); // 0x575630
// Typed native/runtime boundaries preserve the original virtual call sites.
unsigned int UG2FxBufferAddRefVirtual(D3DXEffectBuffer *buffer);
D3DXEffectBuffer *UG2FxBufferDeleteVirtual(D3DXEffectBuffer *buffer,unsigned char flags);
int UG2FxBufferAllocateVirtual(D3DXEffectBuffer *buffer,unsigned int size);
unsigned int __stdcall UG2FxFindResourceA(unsigned int module,const void *name,unsigned int type);
unsigned int __stdcall UG2FxFindResourceW(unsigned int module,const void *name,unsigned int type);
unsigned int __stdcall UG2FxSizeofResource(unsigned int module,unsigned int info);
unsigned int __stdcall UG2FxLoadResource(unsigned int module,unsigned int info);
const unsigned char *__stdcall UG2FxLockResource(unsigned int handle);
int __stdcall UG2FxDeleteObject(unsigned int handle);
unsigned int __stdcall UG2FxGetLastError();
}
#endif
