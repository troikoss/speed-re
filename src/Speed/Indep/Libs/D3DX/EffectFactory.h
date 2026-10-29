#ifndef UG2_D3DX_EFFECT_FACTORY_H
#define UG2_D3DX_EFFECT_FACTORY_H
#include "EffectInput.h"
struct D3DXEffectRecord38;
struct D3DXEffectRecordB4;
struct D3DXEffectReference18;
struct D3DXEffectBaseRecord;
struct D3DXEffectPoolLink;
struct D3DXEffectPool;

// Secondary interface at effect+4; offsets here are relative to that receiver.
struct D3DXEffectSecondaryPrefix {
    unsigned int class00,value04,value08,value0C;
    int SetDevice(unsigned int device); // 0x65e591
};

struct D3DXEffectParameter {
    unsigned int value00,value04,value08,value0C,value10,value14,value18,value1C;
    unsigned int value20,value24,value28,value2C,value30,value34,value38,value3C,value40;
    unsigned short value44,value46;
    unsigned int value48,value4C;
    void Construct(); // 0x664ecf, normal return leaves the other fields untouched
    D3DXEffectParameter *Delete(unsigned char flags); // 0x664f42
    D3DXEffectParameter *DeleteArray(unsigned char flags); // 0x6651bf
};
struct D3DXEffectValuePrefix {
    unsigned int value00,value04,value08,value0C,value10,value14,value18;
    void Construct(); // 0x65eb78; 0x1c-byte written/read prefix
    D3DXEffectValuePrefix *Delete(unsigned char flags); // 0x66519e
};
struct D3DXEffectObject {
    unsigned int class00,interface04,value08,value0C,value10,value14,value18,value1C;
    unsigned int value20,refs24,value28,value2C;
    unsigned short value30,value32;
    unsigned int value34,value38,value3C,value40,value44;
    D3DXEffectParameter value48;
    unsigned int value98,value9C,valueA0,valueA4,valueA8,valueAC,valueB0,valueB4;
    unsigned int valueB8,valueBC,valueC0,valueC4,valueC8,valueCC,valueD0,valueD4,valueD8,valueDC;
    D3DXEffectValuePrefix valueE0;
    unsigned int valueFC,value100,value104,value108,value10C,value110,value114;
    unsigned int value118,value11C,value120,value124,value128,value12C;
    D3DXEffectObject *Construct(); // 0x66509e
    D3DXEffectObject *Delete(unsigned char flags); // 0x66afc5
    int Setup(); // 0x66afe6
    int AdjustReferences(unsigned int group,int directDelta,int allDelta); // 0x65ae25
    int ResolveReferences(unsigned int group,int direct); // 0x66aa2f
    int ValidateString(unsigned int offset); // 0x65bc5e
    int ValidateName(unsigned int offset); // 0x65bcba
    int ValidateType(unsigned int *type,unsigned int *data,unsigned int parameter); // 0x66548b
    int LinkParameter(unsigned int group,unsigned int record,unsigned int use,
        D3DXEffectParameter *parameter,unsigned int value); // 0x65bd1e
    int ReadAnnotation(D3DXEffectParameter *,D3DXEffectBuffer *,unsigned int *position); // 0x66a574
    int ReadParameter(D3DXEffectParameter *,D3DXEffectBuffer *,unsigned int *position); // 0x66b420
    int ReadPass(D3DXEffectRecord38 *,D3DXEffectBuffer *,unsigned int *position); // 0x66a6af
    int ReadRecord(unsigned short group,unsigned short record,D3DXEffectBuffer *,unsigned int *position); // 0x66164d
    int ReadTechnique(D3DXEffectRecordB4 *,D3DXEffectBuffer *,unsigned int *position); // 0x66b60d
    int ResetDirtyQueue(); // 0x65b92d
    int GrowDirtyQueue(); // 0x65aaaa
    int MarkParameterDirty(D3DXEffectParameter *); // 0x65ab14
    void CreateVertexShader(D3DXEffectReference18 *); // 0x65b970
    void CreatePixelShader(D3DXEffectReference18 *); // 0x65b9ce
    int ReadResource(D3DXEffectBuffer *,unsigned int *position); // 0x65ba2c
    int ReadShaderRecord(unsigned int group,unsigned int record,D3DXEffectBuffer *,unsigned int *position); // 0x66520d
    D3DXEffectParameter *FindRootParameter(unsigned int handle,unsigned int depth); // 0x65be0d, RET 8
    int BindCompiledRecord(unsigned int group,unsigned int record,unsigned int *data); // 0x660d4e
    int ReadCompiledRecord(unsigned int group,unsigned int record,D3DXEffectBuffer *,unsigned int *position); // 0x661a40
    D3DXEffectBaseRecord *ResolveParameter(unsigned int handle,unsigned int depth,unsigned int create); // 0x66bbee
    unsigned int ResolveElement(unsigned int handle,unsigned int index,unsigned int depth); // 0x65c123
    unsigned int ResolveMember(unsigned int handle,const char *name,unsigned int depth); // 0x66a83c
    unsigned int ResolveAnnotation(unsigned int owner,const char *name,unsigned int depth); // 0x665a15
    D3DXEffectRecordB4 *FindTechnique(unsigned int handle,unsigned int depth); // 0x65c61f
    D3DXEffectRecord38 *FindPass(unsigned int handle,unsigned int depth); // 0x65c5de
    int BuildMembers(D3DXEffectBaseRecord *,unsigned int count); // 0x661c3e
    int ReadSymbol(unsigned int group,unsigned int record,D3DXEffectBuffer *,unsigned int *position); // 0x66c37e
    int ReadSymbolRecord(unsigned int group,unsigned int record,D3DXEffectBuffer *,unsigned int *position); // 0x66c645
    int PrepareGroupA(unsigned int group); // 0x666df7
    int PrepareGroupB(unsigned int group); // 0x667d51
    int EvaluateRecord(unsigned int group,unsigned int record); // 0x663a32
    int BindPreparationConstants(unsigned int group,unsigned int record,unsigned int *data); // 0x660f9c
    int UnlinkParameterUse(unsigned int group,unsigned int record,unsigned int use); // 0x6611a2
    int CaptureVertexBytecode(D3DXEffectReference18 *); // 0x661221
    int CapturePixelBytecode(D3DXEffectReference18 *); // 0x661313
    int Deserialize(unsigned int device,D3DXEffectPool *,D3DXEffectBuffer *,unsigned int *position); // 0x66f69c
    int WriteShaderResource(unsigned int object,unsigned int index,D3DXEffectPoolLink *); // 0x65cc10
    int WriteObjectResource(unsigned int object,unsigned int index,D3DXEffectPoolLink *); // 0x65ccc8
    int WriteStringResource(const char *text,unsigned int index,D3DXEffectPoolLink *); // 0x65cd6c
    int CopyTypedValue(unsigned int **type,unsigned int **output,const unsigned int **input,
        unsigned int count,D3DXEffectBaseRecord *); // 0x65ddc8
    int CopyValue(D3DXEffectBaseRecord *,const unsigned int *input); // 0x661fc6
    int NotifyValue(D3DXEffectBaseRecord *); // 0x66055c
    int RecordValue(unsigned int handle,const void *data,unsigned int bytes); // 0x666c81
    int RelocateParameter(D3DXEffectParameter *,unsigned int *scalar,unsigned int *matrix,unsigned int *structure); // 0x6614a1
    int ApplyPassGroup(unsigned int group,unsigned int index); // 0x6691a1
    int RecordTexture(unsigned int handle,unsigned int texture); // 0x666a68
};
struct D3DXEffectPoolLink {
    unsigned int value00,value04;
    void *data08;
    D3DXEffectPoolLink *previous0C,*next10;
    void Unlink(); // 0x641837
    D3DXEffectPoolLink *Delete(unsigned char flags); // 0x6424e3
};
struct D3DXEffectPoolNode {
    unsigned int value00,value04,value08;
    D3DXEffectPoolLink link0C;
    D3DXEffectPoolNode *linked20;
    D3DXEffectPoolNode *Construct(); // 0x641862
    void Destroy(); // 0x641878
    D3DXEffectPoolNode *Delete(unsigned char flags); // 0x641816
};
struct D3DXEffectPool {
    unsigned int class00,refs04,value08,value0C;
    void *data10;
    unsigned int value14,value18;
    D3DXEffectPoolNode *node1C;
    D3DXEffectPool *Construct(); // 0x64285f
    int RefreshPointers(); // 0x6424b2
    int Bind(D3DXEffectObject *,D3DXEffectParameter *); // 0x64259c
    void Destroy(); // 0x642504
    D3DXEffectPool *Delete(unsigned char flags); // 0x642535
};
typedef char D3DXEffectParameterSize[(sizeof(D3DXEffectParameter)==0x50)?1:-1];
typedef char D3DXEffectSecondaryPrefixSize[(sizeof(D3DXEffectSecondaryPrefix)==0x10)?1:-1];
typedef char D3DXEffectObjectSize[(sizeof(D3DXEffectObject)==0x130)?1:-1];
typedef char D3DXEffectPoolNodeSize[(sizeof(D3DXEffectPoolNode)==0x24)?1:-1];
typedef char D3DXEffectPoolLinkSize[(sizeof(D3DXEffectPoolLink)==0x14)?1:-1];
typedef char D3DXEffectPoolSize[(sizeof(D3DXEffectPool)==0x20)?1:-1];
extern "C" {
unsigned int __stdcall D3DXEffectAddRef(D3DXEffectObject *effect); // 0x65b8c6
unsigned int __stdcall D3DXEffectRelease(D3DXEffectObject *effect); // 0x66c361
int __stdcall D3DXEffectPoolCreate(D3DXEffectPool **output); // 0x640aac
int __stdcall D3DXEffectPoolQuery(D3DXEffectPool *pool,const unsigned int *iid,D3DXEffectPool **output); // 0x642556
unsigned int __stdcall D3DXEffectPoolRelease(D3DXEffectPool *pool); // 0x642883
int __stdcall D3DXEffectCreateFromResource(unsigned int device,unsigned int module,const void *name,
    const void *defines,void *includes,unsigned int flags,D3DXEffectPool *pool,
    D3DXEffectObject **output,D3DXEffectBuffer **errors); // 0x640c02, RET 36
extern const unsigned int ug2FxPoolIid7B7410[4];
unsigned int UG2FxPoolAddRefVirtual(D3DXEffectPool *pool);
unsigned int UG2FxEffectAddRefVirtual(D3DXEffectObject *effect);
unsigned int UG2FxEffectReleaseVirtual(D3DXEffectObject *effect);
unsigned char *UG2FxBufferPointerVirtual(D3DXEffectBuffer *buffer);
unsigned int UG2FxBufferLengthVirtual(D3DXEffectBuffer *buffer);
unsigned int UG2FxBufferReleaseVirtual(D3DXEffectBuffer *buffer);
unsigned int __stdcall UG2FxComAddRef(unsigned int object); // vtable +4 boundary
void UG2FxDestroyEffect(D3DXEffectObject *effect); // native ABI bridge to recovered 0x66adec
int UG2FxSetupEffect(D3DXEffectObject *effect); // native ABI bridge to recovered 0x66afe6
// ABI bridge to the recovered deserializer. Runtime preparation, compilation
// and the GPU backend remain required for native technique activation.
int UG2FxDeserializeEffect(D3DXEffectObject *effect,unsigned int device,D3DXEffectPool *pool,
    D3DXEffectBuffer *buffer,unsigned int *position); // 0x66f69c
int __stdcall UG2FxCompileResource(unsigned int module,const void *name,const void *defines,
    void *includes,unsigned int flags,void **compiler,D3DXEffectBuffer **errors); // 0x640aef
int __stdcall UG2FxCompilerCompile(void *compiler,unsigned int flags,D3DXEffectBuffer **buffer,D3DXEffectBuffer **errors); // vtable +0xf4
unsigned int __stdcall UG2FxCompilerRelease(void *compiler); // vtable +8
}
#endif
