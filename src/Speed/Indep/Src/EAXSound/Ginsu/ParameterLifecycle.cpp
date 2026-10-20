#include "ParameterLifecycle.h"

// 0x734df0: the second read follows the output write, which can alias the global.
int EngineParameterGetAllocator734DF0(EngineParameterAllocator **allocator) {
    *allocator = engineParameterAllocator8B4F3C;
    return engineParameterAllocator8B4F3C ? 0 : -6;
}

// 0x734e40 / 0x734e60. OS status is ignored by both wrappers.
int EngineParameterLock734E40() {
    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);
    return 0;
}
int EngineParameterUnlock734E60() {
    EngineDispatchRelease(engineSoundMutex8B4F38);
    return 0;
}

// 0x458160: allocator release wrapper used by the setup caller's unwind cleanup.
void EngineParameterFree458160(void *memory) {
    EngineParameterAllocator *allocator;
    EngineParameterGetAllocator734DF0(&allocator);
    EngineParameterLock734E40();
    allocator->vtable0->free4(allocator, 0, memory, 0);
    EngineParameterUnlock734E60();
}

// 0x735070: writes the whole word; the engine setup caller stores only AX.
int EngineParameterDispatcher::Value735070(int *value) {
    *value = value4;
    return 0;
}
