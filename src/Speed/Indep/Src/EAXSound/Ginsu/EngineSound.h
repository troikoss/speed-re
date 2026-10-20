#ifndef UG2_EAXSOUND_ENGINE_SOUND_H
#define UG2_EAXSOUND_ENGINE_SOUND_H

#include "Ginsu.h"
#include "../../Physics/Ug2Layout.h"

// UG2-only views of the fields read by engine setup and controllers.
struct EngineSoundCar {
    char unidentified0[0x10];
    int bankIndex10;
    char unidentified14[0x64];
    int mode78;
};

struct EngineSoundSlot {
    GinsuSynthData *data0;
    GinsuSynthesis *synth4;
    void *memory8;
    float maxFrequencyC;
    float minFrequency10;
    int startResult14;
    int value18;
};

struct EngineSound;
struct EngineSoundVtable {
    void *unidentified0[14];
    // MSVC 7.1 cannot spell a free __thiscall pointer. This ABI view keeps
    // this in ECX and the one stack argument; the dummy EDX is not an argument
    // of the retail callee. Both calls and RET 4 are checked by the verifier.
    int (__fastcall *value38)(EngineSound *, void *, int);
    int (__fastcall *value3C)(EngineSound *, void *, int);
};

struct EngineSoundControl {
    char unidentified0[0x7c];
    int value7C;
    int value80;
    int value84;
    char unidentified88[0x30];
    int valueB8;
    float valueBC;
    unsigned char flagC0;
    char unidentifiedC1[0x37];
    float frequencyF8;
};

struct EngineParameterCallback {
    EngineParameterCallback *next0;
    int value4;
    void (__cdecl *callback8)(void *values, void *data);
    void *dataC;
};

struct EngineParameterDispatcher {
    int Dispatch735580(void *values);
    char unidentified0[8];
    EngineParameterCallback *first8;
};

UG2_CHECK_SIZE(EngineParameterCallback, 0x10);
UG2_CHECK_OFFSET(EngineParameterDispatcher, first8, 8);
extern void *engineSoundMutex8B4F38;
unsigned int __stdcall EngineDispatchWait(void *mutex, unsigned int timeout);
int __stdcall EngineDispatchRelease(void *mutex);

struct EngineSoundParameters {
    EngineParameterDispatcher *dispatcher0;
    int value4;
    int value8;
    int valueC;
    int value10;
    int value14;
    int value18;
    char unidentified1C[0x1c];
    int value38;
    int value3C;
    int value40;
    int value44;
    int value48;
    char unidentified4C[0xc];
    int value58;
    int value5C;
    int value60;
    int value64;
    int value68;
};

struct EngineSoundAuxiliary;
struct EngineSoundAuxiliaryVtable {
    void *unidentified0[14];
    float (__fastcall *value38)(EngineSoundAuxiliary *);
};

struct EngineSoundAuxiliary {
    EngineSoundAuxiliaryVtable *vtable0;
    char unidentified4[0x130];
    int value134;
    int value138;
};

struct EngineSound {
    void Setup47D740();
    void Update473E60();
    void Update474030();
    void Update474310();

    EngineSoundVtable *vtable0;
    char unidentified4[0x98];
    EngineSoundCar *car9C;
    char unidentifiedA0[0x10];
    EngineSoundParameters *parametersB0;
    int valueB4;
    EngineSoundAuxiliary *auxiliaryB8;
    float valueBC;
    EngineSoundControl *controlC0;
    int valueC4;
    unsigned char activeC8;
    char unidentifiedC9[3];
    EngineSoundSlot slotsCC[2];
    float frequency104;
    int volume108;
    int volume10C;
};

typedef char EngineSoundSlotSizeCheck[sizeof(EngineSoundSlot) == 0x1c ? 1 : -1];
typedef char EngineSoundViewSizeCheck[sizeof(EngineSound) == 0x110 ? 1 : -1];

UG2_CHECK_OFFSET(EngineSound, car9C, 0x9c);
UG2_CHECK_OFFSET(EngineSound, valueBC, 0xbc);
UG2_CHECK_OFFSET(EngineSound, parametersB0, 0xb0);
UG2_CHECK_OFFSET(EngineSound, auxiliaryB8, 0xb8);
UG2_CHECK_OFFSET(EngineSound, controlC0, 0xc0);
UG2_CHECK_OFFSET(EngineSound, activeC8, 0xc8);
UG2_CHECK_OFFSET(EngineSound, slotsCC, 0xcc);
UG2_CHECK_OFFSET(EngineSound, frequency104, 0x104);
UG2_CHECK_OFFSET(EngineSound, volume108, 0x108);
UG2_CHECK_OFFSET(EngineSound, volume10C, 0x10c);
UG2_CHECK_OFFSET(EngineSoundControl, value80, 0x80);
UG2_CHECK_OFFSET(EngineSoundControl, value84, 0x84);
UG2_CHECK_OFFSET(EngineSoundControl, valueBC, 0xbc);
UG2_CHECK_OFFSET(EngineSoundControl, value7C, 0x7c);
UG2_CHECK_OFFSET(EngineSoundControl, valueB8, 0xb8);
UG2_CHECK_OFFSET(EngineSoundControl, flagC0, 0xc0);
UG2_CHECK_OFFSET(EngineSoundControl, frequencyF8, 0xf8);
UG2_CHECK_SIZE(EngineSoundParameters, 0x6c);
UG2_CHECK_OFFSET(EngineSoundParameters, value38, 0x38);
UG2_CHECK_OFFSET(EngineSoundParameters, value68, 0x68);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, value134, 0x134);

// bWare allocation wrapper 0x440820; available-byte query 0x43e910 remains a heap seam.
void *EngineSoundAllocate440820(int size, int flags);
int EngineSoundHeapAvailable43E910(int heap);
extern int engineSoundHeap7EC840;
extern float ginsuEngineBlockRate78A5B0;
extern int engineSoundValue82A998;
extern int engineSoundValue82B078;
extern float engineSoundScale78A2E0; // 4096.0f
extern float engineSoundScale78AF80; // 120.0f
extern int engineSoundValue82B070;
extern int engineSoundValue82B880;
extern float engineSoundScale78AF84; // -16383.0f
extern float engineSoundScale78A1C8; // binary32 approximation of 10.24

// Stateful snd boundaries reached by the controllers. Names stay address-based
// until the middleware parameter semantics are established from its consumers.
int EngineSnd72412A(int handle, int value);
int EngineSnd724093(int handle, int value, int value2);
int EngineSnd723D9B(int handle, int value);
int EngineSnd72403E(int handle, int value);
int EngineSnd723C41(int handle, int value);
int EngineSnd723BC1(int handle, int index, int value);

#endif
