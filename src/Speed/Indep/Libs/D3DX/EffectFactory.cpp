#include "EffectFactory.h"

// Original 0x640c02, RET 36. Compiler and deserializer remain explicit
// boundaries. Native setup/destruction dispatch to their recovered bodies.
extern "C" int __stdcall D3DXEffectCreateFromResource(unsigned int device,unsigned int module,const void *name,
    const void *defines,void *includes,unsigned int flags,D3DXEffectPool *pool,
    D3DXEffectObject **output,D3DXEffectBuffer **errors) {
    D3DXEffectResource resource;resource.Construct();
    void *compiler=0;
    D3DXEffectObject *effect=0;
    D3DXEffectBuffer *buffer=0;
    int result;
    if (output) *output=0;
    if (errors) *errors=0;
    if (!device || !name) { result=static_cast<int>(0x8876086c); goto release_compiler; }
    effect=static_cast<D3DXEffectObject *>(UG2FxMalloc(0x130));
    if (effect) effect=effect->Construct();
    if (!effect) { result=static_cast<int>(0x8007000e); goto release_buffer; }
    result=resource.Load(module,name,0,0);
    if (result<0) goto release_buffer;
    if (resource.size08<4) { result=static_cast<int>(0x80004005); goto release_buffer; }
    if (*reinterpret_cast<const unsigned int *>(resource.data04)==0xfeff0901) {
        if (!output) goto release_buffer;
        result=D3DXEffectBufferCreateForward(resource.size08,&buffer);
        if (result<0) goto release_buffer;
        unsigned int length=resource.size08;
        const unsigned char *source=resource.data04;
        unsigned char *destination=UG2FxBufferPointerVirtual(buffer);
        unsigned int i;
        for (i=length>>2;i;--i) {
            *reinterpret_cast<unsigned int *>(destination)=*reinterpret_cast<const unsigned int *>(source);
            destination+=4;source+=4;
        }
        for (i=length&3;i;--i) *destination++=*source++;
    } else {
        resource.Clear();
        result=UG2FxCompileResource(module,name,defines,includes,flags,&compiler,errors);
        if (result<0) goto release_compiler;
        if (errors && *errors) { UG2FxBufferReleaseVirtual(*errors); *errors=0; }
        result=UG2FxCompilerCompile(compiler,flags&0xffffffe7,&buffer,errors);
        if (result<0) goto release_compiler;
    }
    if (output) {
        unsigned int position=0;
        result=UG2FxDeserializeEffect(effect,device,pool,buffer,&position);
        if (result<0) goto release_compiler;
        result=UG2FxSetupEffect(effect);
        if (result<0) goto release_compiler;
        *output=effect;
        UG2FxEffectAddRefVirtual(effect);
    }
release_compiler:
    if (compiler) UG2FxCompilerRelease(compiler);
release_buffer:
    if (buffer) { UG2FxBufferReleaseVirtual(buffer); buffer=0; }
    if (effect) UG2FxEffectReleaseVirtual(effect);
    resource.Destroy();
    return result;
}
