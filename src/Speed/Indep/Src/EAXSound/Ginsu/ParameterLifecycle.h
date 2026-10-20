#ifndef UG2_PARAMETER_LIFECYCLE_H
#define UG2_PARAMETER_LIFECYCLE_H
#include "EngineSound.h"

struct EngineParameterContext {
    int value0;
    void *value4;
    int value8;
};
struct EngineParameterAllocator;
struct EngineParameterAllocatorVtable {
    // Free-function ABI view of thiscall: dummy EDX, two stack arguments,
    // and callee RET 8. The original virtual callees remain separate.
    void *(__fastcall *allocate0)(EngineParameterAllocator *, void *, int, EngineParameterContext *);
    void (__fastcall *free4)(EngineParameterAllocator *, void *, void *, int);
};
struct EngineParameterAllocator {
    EngineParameterAllocatorVtable *vtable0;
};
UG2_CHECK_SIZE(EngineParameterContext, 12);
UG2_CHECK_OFFSET(EngineParameterContext, value4, 4);
UG2_CHECK_OFFSET(EngineParameterContext, value8, 8);
UG2_CHECK_OFFSET(EngineParameterAllocatorVtable, free4, 4);
struct EngineParameterRegistry {
    int Bind735840(void *descriptor); // middleware lookup remains separate
    char unidentified0[8];
};
extern EngineParameterAllocator *engineParameterAllocator8B4F3C;
extern EngineParameterRegistry engineParameterRegistry82C538;
extern char engineParameterDescriptor7EDE18[];
extern int engineParameterMode850078;
int EngineParameterGetAllocator734DF0(EngineParameterAllocator **allocator);
int EngineParameterLock734E40();
int EngineParameterUnlock734E60();
void EngineParameterFree458160(void *memory);
int EngineParameterRegister7358C0(EngineParameterRegistry *registry, void *values, void *result);
#endif
