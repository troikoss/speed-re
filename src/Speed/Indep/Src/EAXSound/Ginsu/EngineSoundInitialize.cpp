#include "ParameterLifecycle.h"

static int ClampInitialParameter(int value, int maximum) {
    if (value < 0) return 0;
    if (value > maximum) return maximum;
    return value;
}

// The original MSVC unwind entry 0x772610 calls 0x458160 for this owner.
// Ownership ends before the final dispatcher query. Exception execution is
// outside the normal-return verifier; no Windows SEH frame is authored here.
struct EngineParameterOwner {
    void *memory;
    bool armed;
    explicit EngineParameterOwner(void *value) : memory(value), armed(true) {}
    ~EngineParameterOwner() { if (armed) EngineParameterFree458160(memory); }
};

// UG2 PC v1.2 0x47d920..0x47dade: slot setup, activation and parameters.
void EngineSound::Initialize47D920() {
    Setup47D740();
    const int mode = car9C->mode78;
    if ((mode == 0 && slotsCC[0].startResult14 >= 0) ||
        (mode == 1 && slotsCC[0].startResult14 >= 0 && slotsCC[1].startResult14 >= 0)) {
        activeC8 = 1;
    }
    const int initial34 = engineParameterMode850078 != 4;
    EngineParameterAllocator *allocator;
    EngineParameterGetAllocator734DF0(&allocator);
    EngineParameterLock734E40();
    EngineParameterContext context;
    context.value0 = 0;
    context.value4 = 0;
    context.value8 = 0;
    EngineSoundParameters *parameters = static_cast<EngineSoundParameters *>(
        allocator->vtable0->allocate0(allocator, 0, 0x6c, &context));
    EngineParameterUnlock734E60();
    EngineParameterOwner owner(parameters);
    if (parameters) {
        parameters->value4 = ClampInitialParameter(car9C->value80->value8, 60);
        int *words = reinterpret_cast<int *>(parameters);
        int i;
        for (i = 2; i <= 12; ++i) words[i] = 0;
        parameters->value34 = ClampInitialParameter(initial34, 2);
        for (i = 14; i <= 26; ++i) words[i] = 0;
        if (EngineParameterRegister7358C0(&engineParameterRegistry82C538, &parameters->value4, parameters) < 0) {
            engineParameterRegistry82C538.Bind735840(engineParameterDescriptor7EDE18);
            EngineParameterRegister7358C0(&engineParameterRegistry82C538, &parameters->value4, parameters);
        }
    }
    parametersB0 = parameters;
    EngineParameterDispatcher *dispatcher = parameters->dispatcher0;
    owner.armed = false;
    owner.memory = 0;
    if (dispatcher) {
        int value;
        dispatcher->Value735070(&value);
        value48 = (unsigned short)value;
    } else {
        value48 = 0;
    }
}
