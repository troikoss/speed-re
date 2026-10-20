#ifndef UG2_EAXSOUND_ENGINE_SOUND_H
#define UG2_EAXSOUND_ENGINE_SOUND_H

#include "Ginsu.h"
#include "../../Physics/Ug2Layout.h"

// UG2-only views of the fields read by engine setup and controllers.
struct EngineSoundCarValue {
    int value0;
    int value4;
    int value8;
    const char *fileC;
    int value10;
    int value14;
    int value18;
    float value1C;
    float value20;
    char unidentified24[0x28];
};

struct EngineSoundCarValue84 {
    const char *file0;
    char unidentified4[0x30];
};

struct MoverContext;
struct EngineSoundChoice;

struct EngineSoundCar {
    char unidentified0[0x10];
    int bankIndex10;
    int kind14;
    char unidentified18[0x5c];
    MoverContext *context74;
    int mode78;
    int value7C;
    EngineSoundCarValue *value80;
    EngineSoundCarValue84 *value84;
    EngineSoundChoice *value88;
    int value8C;
    int value90;
    int value94;
    int value98;
    int value9C;
    int valueA0;
    int valueA4;
    char unidentifiedA8[0x24];
    EngineSoundCar *Construct479660();
    void Select4593B0(MoverContext *context);
};

struct EngineSoundSlot {
    GinsuSynthData *data0;
    GinsuSynthesis *synth4;
    void *memory8;
    float maxFrequencyC;
    float minFrequency10;
    int startResult14;
    unsigned char value18;
    char unidentified19[3];
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

struct EngineSoundAuxiliary;
struct EngineSoundComponent;

struct EngineSoundControl {
    char unidentified0[0x58];
    unsigned char active58;
    char unidentified59[3];
    float time5C;
    float delta60;
    char unidentified64[4];
    EngineSoundCar *car68;
    EngineSoundAuxiliary *engine6C;
    void *value70;
    void *value74;
    char unidentified78[4];
    int value7C;
    int value80;
    int value84;
    char unidentified88[0x30];
    int valueB8;
    float valueBC;
    unsigned char flagC0;
    char unidentifiedC1[0x37];
    float frequencyF8;
    float valueFC;
    float value100;
    float value104;
    float value108;
    void Update47C3D0(float duration);
    void Scale45D190();
    void Prepare45D0A0(); // original dependency, not yet recovered
    void ModeOne4701A0(float duration); // original one-bank gain producer, not yet recovered
    void ModeTwo470540(float duration); // original two-bank gain producer, not yet recovered
};

struct EngineParameterCallback {
    EngineParameterCallback *next0;
    int value4;
    void (__cdecl *callback8)(void *values, void *data);
    void *dataC;
};

struct EngineParameterDispatcher {
    int Dispatch735580(void *values);
    int Value735070(int *value);
    char unidentified0[4];
    int value4;
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
    char unidentified1C[0x18];
    int value34;
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
    void *unidentified0[13];
    float (__fastcall *value34)(EngineSoundAuxiliary *);
    float (__fastcall *value38)(EngineSoundAuxiliary *);
};

struct EngineSoundAuxiliary {
    EngineSoundAuxiliaryVtable *vtable0;
    char unidentified4[0xf8];
    float valueFC;
    char unidentified100[0x18];
    float value118;
    char unidentified11C[0xc];
    float value128;
    char unidentified12C[4];
    float value130;
    int value134;
    int value138;
    float Value45C530();
    float Value45C540();
};

struct EngineSoundComponentVtable {
    void *unidentified0;
    unsigned int *(__fastcall *descriptor4)(EngineSoundComponent *);
};
struct EngineSoundComponent {
    EngineSoundComponentVtable *vtable0;
};

struct EngineSound {
    EngineSound *Construct45E350();
    EngineSound *Construct473BF0();
    EngineSound *Construct473D30();
    void Bind463500(EngineSoundCar *car);
    int Dependency461490(int index);
    void Attach4614E0(EngineSoundComponent *component);
    void LoadBanks465C80(int *count);
    static void __cdecl BankRead461550(void *object);
    static void __cdecl BankRead461560(void *object);
    void Setup47D740();
    void Initialize47D920();
    int Value45E390(int index);
    int Value45E3B0(int index);
    void Update473E60();
    void Update474030();
    void Update474310();

    EngineSoundVtable *vtable0;
    char unidentified4[0x44];
    unsigned short value48;
    char unidentified4A[0x52];
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
UG2_CHECK_OFFSET(EngineSound, value48, 0x48);
UG2_CHECK_OFFSET(EngineSoundCar, value80, 0x80);
UG2_CHECK_OFFSET(EngineSoundCarValue, value8, 8);
UG2_CHECK_OFFSET(EngineSoundParameters, value34, 0x34);
UG2_CHECK_OFFSET(EngineSound, valueBC, 0xbc);
UG2_CHECK_OFFSET(EngineSound, parametersB0, 0xb0);
UG2_CHECK_OFFSET(EngineSound, auxiliaryB8, 0xb8);
UG2_CHECK_OFFSET(EngineSound, controlC0, 0xc0);
UG2_CHECK_OFFSET(EngineSound, activeC8, 0xc8);
UG2_CHECK_OFFSET(EngineSound, slotsCC, 0xcc);
UG2_CHECK_OFFSET(EngineSound, frequency104, 0x104);
UG2_CHECK_OFFSET(EngineSound, volume108, 0x108);
UG2_CHECK_OFFSET(EngineSound, volume10C, 0x10c);
UG2_CHECK_OFFSET(EngineSoundCarValue, value1C, 0x1c);
UG2_CHECK_OFFSET(EngineSoundCarValue, value20, 0x20);
UG2_CHECK_OFFSET(EngineSoundControl, active58, 0x58);
UG2_CHECK_OFFSET(EngineSoundControl, car68, 0x68);
UG2_CHECK_OFFSET(EngineSoundControl, engine6C, 0x6c);
UG2_CHECK_SIZE(EngineSoundControl, 0x10c);
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
UG2_CHECK_OFFSET(EngineSoundAuxiliary, valueFC, 0xfc);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, value118, 0x118);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, value128, 0x128);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, value130, 0x130);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, value134, 0x134);
UG2_CHECK_OFFSET(EngineSoundAuxiliaryVtable, value34, 0x34);
extern float engineControlMinimum784550;
extern float engineControlMaximum784500;
extern float engineControlScale78A290;

// bWare allocation wrapper 0x440820; available-byte query 0x43e910 remains a heap seam.
void *EngineSoundAllocate440820(int size, int flags);
int EngineSoundHeapAvailable43E910(int heap);
extern int engineSoundHeap7EC840;
extern int engineSoundConstructionValues82AF90[6];
extern int engineSoundConstructionValue7ED440;
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
