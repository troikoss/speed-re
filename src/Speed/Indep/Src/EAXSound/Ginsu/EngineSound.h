#ifndef UG2_EAXSOUND_ENGINE_SOUND_H
#define UG2_EAXSOUND_ENGINE_SOUND_H

#include "Ginsu.h"
#include "EngineSoundSequence.h"
#include "../../../Libs/bWare/TimedAverage.h"
#include "../../Physics/DriveFeedbackCurve.h"
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
    int value24;
    int value28;
    float value2C;
    float value30;
    float value34;
    float value38;
    float value3C;
    float value40;
    float value44;
    int value48;
};

struct EngineSoundCarValue84 {
    const char *file0;
    int value4;
    int value8;
    char unidentifiedC[0x10];
    float value1C;
    char unidentified20[4];
    float value24;
    float value28;
    float value2C;
    float value30;
};

struct MoverContext;
struct EngineSoundChoice;

struct MoverModeOwner;
struct EngineSoundCar;
struct EngineSoundPhysicsView {
    char unidentified0[0x44];
    EngineSoundCar *state44;
    char unidentified48[0x10];
    unsigned char active58;
    char unidentified59[3];
    float time5C;
    float delta60;
    int value64;
    EngineSoundCar *car68;
    unsigned char flag6C;
    char unidentified6D[3];
    float value70, value74, value78, value7C, value80, value84, value88, value8C, value90;
    SampleAverage average94;
    unsigned char flagBC;
    char unidentifiedBD[3];
    float valueC0, valueC4;
    int valueC8, valueCC;
    unsigned char flagD0;
    char unidentifiedD1[3];
    int valueD4;
    unsigned char flagD8;
    char unidentifiedD9[3];
    int valueDC, valueE0, valueE4;
    const unsigned short *valueE8;
    float valueEC, valueF0;
    EngineSoundPhysicsView *Construct45D210();
    void Bind45D4F0(EngineSoundCar *car);
    void Activate45D370();
    void Update484C50(float duration);
    void Post470A60();
    void Override46FA30(float duration);
    static void __cdecl Notify45D360();
};
UG2_CHECK_OFFSET(EngineSoundPhysicsView, flagD8, 0xd8);
UG2_CHECK_OFFSET(EngineSoundPhysicsView, average94, 0x94);
UG2_CHECK_SIZE(EngineSoundPhysicsView, 0xf4);
struct EngineSoundShiftView {
    char unidentified0[0x78]; int value78, value7C;
    int value80, value84, value88, value8C, value90, value94;
    char unidentified98[0x28]; float valueC0;
    char unidentifiedC4[0x18]; float valueDC;
    char unidentifiedE0[0x18]; float valueF8;
    char unidentifiedFC[0x58]; float value154;
};
struct EngineSoundAccelView {
    char unidentified0[0x88]; float value88;
    char unidentified8C[0x34]; float valueC0;
    char unidentifiedC4[4]; int valueC8;
};

struct EngineSoundScheduledNode;
struct EngineSoundCarVtable {
    void *unidentified0[13];
    void (__fastcall *pre34)(EngineSoundCar *);
    void *unidentified38[3];
    int (__fastcall *value44)(EngineSoundCar *);
};
struct EngineSoundCar {
    EngineSoundCarVtable *vtable0;
    char unidentified4[0xc];
    int bankIndex10;
    int kind14;
    char unidentified18[4];
    EngineSoundScheduledNode *first1C, *first20;
    int value24, value28, count2C;
    MoverModeOwner *car30;
    float time34;
    float delta38;
    char unidentified3C[4];
    EngineSoundPhysicsView *value40;
    float value44;
    float value48;
    unsigned char flag4C;
    char unidentified4D[3];
    int value50;
    float value54;
    float value58;
    char unidentified5C[0x18];
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
    char unidentifiedA8[0xc];
    unsigned char flagB4;
    char unidentifiedB5[3];
    float valueB8, valueBC;
    unsigned char flagC0;
    char unidentifiedC1[3];
    int valueC4;
    char unidentifiedC8[4];
    int Value479700();
    EngineSoundCar *Construct479660();
    void Bind464BA0(MoverModeOwner *owner);
    void Sort464A10();
    void Update464C10(float duration);
    void Update469CF0(float duration);
    void Post469CB0();
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
    void *unidentified0[9];
    void (__fastcall *value24)(EngineSound *, void *, float);
    void (__fastcall *value28)(EngineSound *);
    void *unidentified2C[2];
    void (__fastcall *value34)(EngineSound *);
    // MSVC 7.1 cannot spell a free __thiscall pointer. This ABI view keeps
    // this in ECX and the one stack argument; the dummy EDX is not an argument
    // of the retail callee. Both calls and RET 4 are checked by the verifier.
    int (__fastcall *value38)(EngineSound *, void *, int);
    int (__fastcall *value3C)(EngineSound *, void *, int);
    void *unidentified40[2];
    void (__fastcall *value48)(EngineSound *);
};

struct EngineSoundAuxiliary;
struct EngineSoundComponent;

struct EngineSoundControl {
    char unidentified0[0x44];
    EngineSoundCar *state44;
    char unidentified48[0x10];
    unsigned char active58;
    char unidentified59[3];
    float time5C;
    float delta60;
    char unidentified64[4];
    EngineSoundCar *car68;
    EngineSoundAuxiliary *engine6C;
    EngineSoundShiftView *value70;
    EngineSoundAccelView *value74;
    char unidentified78[4];
    int value7C;
    int value80;
    int value84;
    DriveFeedbackCurve curve88;
    DriveFeedbackPair pairs90[5];
    int valueB8;
    float valueBC;
    unsigned char flagC0;
    char unidentifiedC1[3];
    SampleAverage averageC4;
    float valueEC;
    float valueF0;
    float valueF4;
    float frequencyF8;
    float valueFC;
    float value100;
    float value104;
    float value108;
    EngineSoundControl *Construct45CE20();
    void Bind45CF90(EngineSoundCar *car);
    void Activate45D050();
    void Update47C3D0(float duration);
    void Post470940();
    void Scale45D190();
    void Prepare45D0A0();
    void ModeOne4701A0(float duration);
    void ModeTwo470540(float duration);
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
    void *unidentified0[9];
    void (__fastcall *value24)(EngineSoundAuxiliary *, void *, float);
    void (__fastcall *value28)(EngineSoundAuxiliary *, void *, float);
    void (__fastcall *value2C)(EngineSoundAuxiliary *, void *, float);
    void (__fastcall *value30)(EngineSoundAuxiliary *, void *, float);
    float (__fastcall *value34)(EngineSoundAuxiliary *);
    float (__fastcall *value38)(EngineSoundAuxiliary *);
    void (__fastcall *value3C)(EngineSoundAuxiliary *, void *, float);
};

struct EngineSoundSpatialComponent;
struct EngineSoundSpatialVtable {
    void *unidentified0[9];
    void (__fastcall *value24)(EngineSoundSpatialComponent *, void *, int);
    void *unidentified28;
    void (__fastcall *value2C)(EngineSoundSpatialComponent *, void *, float *);
    void (__fastcall *value30)(EngineSoundSpatialComponent *, void *, void *);
};
// UG2 3DCarPos/base-position component, observed allocation 0xd8.
struct EngineSoundSpatialComponent {
    EngineSoundSpatialVtable *vtable0;
    int value4, value8, valueC, value10;
    char unidentified14[0x30];
    EngineSoundCar *state44;
    char unidentified48[0x10];
    unsigned char active58;
    char unidentified59[3];
    float time5C, delta60;
    int value64;
    EngineSoundCar *car68;
    int mode6C;
    float *position70;
    void *orientation74;
    float snapshot78[2];
    char unidentified80[8];
    int limits88[6];
    float currentA0, previousA4;
    SampleAverage averageA8;
    int listenerD0;
    unsigned char flagD4, flagD5, flagD6;
    char unidentifiedD7;
    EngineSoundSpatialComponent *Construct45DD60();
    EngineSoundSpatialComponent *Construct471D60();
    void Bind45DAC0(EngineSoundCar *car);
    void Activate45BAD0();
    void Angle45DE60();
    void Compute45DF00();
    void Select45E100(int listener);
    void Doppler45E1F0(int listener, float duration);
    void Update471DC0(float duration);
    void Limit45DE30(int index, int limit);
    float Value45DE50(int value);
    void SetPosition45E1D0(float *position);
    void SetOrientation45E1E0(void *orientation);
};
UG2_CHECK_SIZE(EngineSoundSpatialComponent, 0xd8);
UG2_CHECK_OFFSET(EngineSoundSpatialComponent, position70, 0x70);
UG2_CHECK_OFFSET(EngineSoundSpatialComponent, orientation74, 0x74);

struct EngineSoundComponent;
struct EngineSoundAuxiliary {
    EngineSoundAuxiliaryVtable *vtable0;
    char unidentified4[0x40];
    EngineSoundCar *state44;
    char unidentified48[0x10];
    unsigned char active58;
    char unidentified59[3];
    float time5C, delta60;
    int value64;
    EngineSoundCar *car68;
    EngineSoundShiftView *shift6C;
    EngineSoundAccelView *accel70;
    EngineSoundPhysicsView *physics74;
    EngineSoundSpatialComponent *spatial78;
    char unidentified7C[4];
    float value80, value84, value88;
    char unidentified8C[4];
    SampleAverage average90;
    SampleAverage averageB8;
    int valueE0;
    unsigned char flagE4;
    char unidentifiedE5[3];
    float valueE8, valueEC, valueF0, valueF4, valueF8, valueFC;
    unsigned char flag100;
    char unidentified101[3];
    float value104, value108, value10C, value110, value114, value118;
    unsigned char flag11C;
    char unidentified11D[3];
    unsigned char flag120;
    char unidentified121[3];
    float value124;
    float value128, value12C, value130;
    int value134, value138;
    unsigned char flag13C;
    char unidentified13D[3];
    float value140, value144, value148;
    unsigned short value14C, value14E, value150;
    unsigned char flag152;
    char unidentified153;
    EngineSoundSequence sequence154;
    float value1D0, value1D4, value1D8, value1DC, value1E0, value1E4;
    unsigned char flag1E8;
    char unidentified1E9[7];
    float Value45C530();
    float Value45C540();
    unsigned int *Descriptor45C4B0();
    int Dependency45C670(int index);
    EngineSoundAuxiliary *Construct46F1B0();
    void Bind45C550(EngineSoundCar *car);
    void Attach46F3E0(EngineSoundComponent *component);
    void Activate45C5A0();
    void Update47C010(float duration);
    void ProduceOffsets45C7A0(float duration);
    void ProduceBackfire46F580(float duration);
    void ProduceCutoff46F460();
    void ProduceIdle46F9D0();
    void ProduceLevel45C6C0(float duration);
    void ProduceEffects46F610(float duration);
    void ProduceRpm46F7D0(float duration);
    void ProduceThrottle45C710(float duration);
};
UG2_CHECK_SIZE(EngineSoundAuxiliary, 0x1f0);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, average90, 0x90);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, averageB8, 0xb8);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, sequence154, 0x154);
UG2_CHECK_OFFSET(EngineSoundShiftView, valueF8, 0xf8);
UG2_CHECK_OFFSET(EngineSoundShiftView, value154, 0x154);
UG2_CHECK_OFFSET(EngineSoundAccelView, valueC0, 0xc0);
EngineSoundAuxiliary *CreateEngineSoundAuxiliary47BFD0();
extern unsigned int engineSoundAuxDescriptor7ED2C4[];
extern float engineSoundRpmDecrease7885CC, engineSoundRpmIncrease78AAB0, engineSoundOscillatorLimit78AAAC;
extern char engineSoundAttachDiagnostic78AA74[];
void EngineSoundDiagnostic4691C0(int level, const char *text);

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
    void Forward460FF0();

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
UG2_CHECK_OFFSET(EngineSoundControl, averageC4, 0xc4);
UG2_CHECK_OFFSET(EngineSoundControl, valueEC, 0xec);
UG2_CHECK_OFFSET(EngineSoundControl, curve88, 0x88);
UG2_CHECK_OFFSET(EngineSoundCar, value48, 0x48);
UG2_CHECK_OFFSET(EngineSoundCar, value54, 0x54);
UG2_CHECK_OFFSET(EngineSoundAuxiliary, valueE0, 0xe0);
extern float engineGainPrepareScale788620;
extern float engineGainStep78861C;
extern float engineGainCutoffStep788624;
extern float engineGainCutoffMaximum7885D8;
extern float engineGainCarLimit7843E8;
extern float engineGainOneScale78AAB4;
extern float engineGainTwoScale78A6C8;
extern float engineGainNegativePercent784364;
extern float engineGainPercent78425C;
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
