#ifndef UG2_VEHICLE_MOVER_H
#define UG2_VEHICLE_MOVER_H

#include "Collision/Collision.h"
#include "Drivetrain.h"
#include "Engine.h"
#include "PhysicsUpgrades.h"
#include "PhysicsTuningSelection.h"
#include "Ug2Layout.h"

struct Engine;
struct EngineSpec;
class VehicleIntegrator;

// Body record at car record +0x110: mass, inertia matrix and inertia Z scale.
struct MoverBodySpec {
    float mass0;
    float value4;
    float value8;
    float valueC;
    WheelMatrix inertia10;
    float inertiaScale50;
    char unidentified54[0xc];
};

// Suspension record at car record +0x170 + axle * 0x20.
struct SuspensionSpec {
    float progression0;
    float stiffness4;
    float damping8[2];
    float antiRoll10;
    float upper14;
    float lower18;
    float rest1C;
};

struct MoverAeroSpec {
    float coefficient0;
    float coefficient4;
    float coefficient8;
};

// Copied from CarTypeInfo +0x110 by 0x5ac4f0 (0x2b0 bytes, 0xac dwords).
struct CarPhysicsRecord {
    WheelVector centre0;
    char unidentifiedC[4];
    DrivetrainWheelSpec wheels10[4];
    struct CorrectionBlock {
        WheelCorrectionSpec spec;
        char unidentified14[0xc];
    } correctionD0[2];
    MoverBodySpec body110;
    SuspensionSpec suspension170[2];
    DrivetrainSpec drivetrain1B0;
    char engine1F0[0x70];
    MoverAeroSpec aero260;
    float value26C;
    WheelPreparationSpec preparation270;
    float brakeBias27C;
    float value280;
    float value284;
    float value288;
    float value28C;
    float value290;
    char unidentified294[0xc];
    float nitrousCapacity2A0;
    int nitrousBottles2A4;
    char unidentified2A8[8];
};

// Partial PC x86 layouts: fields observed in recovered mover/body routines.
struct MoverBody {
    void *vtable0;
    char unidentified4[8];
    char name0C[8];
    MoverBodySpec *spec14;
    char unidentified18[8];
    WheelVector position20;
    char unidentified2C[4];
    WheelMatrix matrix30;
    WheelVector vector70;
    char unidentified7C[4];
    WheelVector vector80;
    char unidentified8C[4];
    float value90;
    char unidentified94[0xc];
    WheelVector vectorA0;
    char unidentifiedAC[4];
    WheelVector vectorB0;
    char unidentifiedBC[4];
    WheelVector vectorC0;
    char unidentifiedCC[4];
    WheelMatrix matrixD0;
    WheelVector vector110;
    char unidentified11C[4];
    WheelVector force120;
    char unidentified12C[4];
    WheelVector saved130;
    char unidentified13C[4];
    WheelVector saved140;
    char unidentified14C[4];
    int flag150;
    int flag154;
    int flag158;
    int flag15C;
    float value160;
    float divisor164;
    float value168;
    char unidentified16C[4];
    WheelMatrix matrix170;
    WheelMatrix matrix1B0;
    WheelVector centre1F0;
    char unidentified1FC[4];
    float limit200;
    float value204;
    float value208;
    void Construct(MoverBodySpec *spec);
    void RefreshParameters();
    void SetPose(WheelVector *position, WheelMatrix *matrix, WheelVector *velocity,
                 WheelVector *angular, WheelMatrix *inertiaOutput);
    void MoveCentre(WheelVector *centre);
    void AddLimitedImpulse(WheelVector *impulse, WheelVector *point);
    void AddScaledImpulse(WheelVector *impulse, WheelVector *point, float angularScale);
    void AddForce(WheelVector *input);
    void ApplyForceAtPoint(WheelVector *force, WheelVector *point);
    void PostUpdate();
    float *WriteDerivative(float *output);
};
UG2_CHECK_OFFSET(CarPhysicsRecord, correctionD0, 0xd0);
UG2_CHECK_OFFSET(CarPhysicsRecord, body110, 0x110);
UG2_CHECK_OFFSET(CarPhysicsRecord, suspension170, 0x170);
UG2_CHECK_OFFSET(CarPhysicsRecord, drivetrain1B0, 0x1b0);
UG2_CHECK_OFFSET(CarPhysicsRecord, engine1F0, 0x1f0);
UG2_CHECK_OFFSET(CarPhysicsRecord, aero260, 0x260);
UG2_CHECK_OFFSET(CarPhysicsRecord, preparation270, 0x270);
UG2_CHECK_OFFSET(CarPhysicsRecord, brakeBias27C, 0x27c);
UG2_CHECK_OFFSET(CarPhysicsRecord, value290, 0x290);
UG2_CHECK_SIZE(CarPhysicsRecord, 0x2b0);
UG2_CHECK_OFFSET(MoverBody, spec14, 0x14);
UG2_CHECK_OFFSET(MoverBody, matrix170, 0x170);
UG2_CHECK_OFFSET(MoverBody, centre1F0, 0x1f0);
UG2_CHECK_OFFSET(MoverBody, value208, 0x208);

struct AckermannSteering;

// Suspension block, vtable 0x7a210c ("Suspnsn").
struct MoverArm {
    void *vtable0;
    char unidentified4[8];
    char name0C[8];
    SuspensionSpec *spec14;
    MoverArm *pair18;
    float travel1C;
    float rate20;
    float force24;
    char unidentified28[8];
    WheelVector position30;
    char unidentified3C[4];
    WheelMatrix matrix40;
    int contact80;
    char unidentified84[4];
    float extra88;
    char unidentified8C[4];
    float value90;
    float value94;
    float value98;
    char unidentified9C[4];
    DrivetrainWheel *wheelA0;
    MoverBody *bodyA4;
    AckermannSteering *steeringA8;
    int indexAC;
    float valueB0;
    void Construct(int index, MoverBody *body, DrivetrainWheel *wheel,
                   AckermannSteering *steering, SuspensionSpec *spec);
    void SetMount(float extra, float offset);
    void WriteDerivative(float *output);
    void UpdateAfterIntegration();
    float ApplyBumpStop();
};
UG2_CHECK_OFFSET(MoverArm, contact80, 0x80);
UG2_CHECK_OFFSET(MoverArm, value90, 0x90);
UG2_CHECK_OFFSET(MoverArm, indexAC, 0xac);

// Front steering geometry, vtable 0x7a2084 ("VATwoWheelAckermanSteering").
struct AckermannSteering {
    void *vtable0;
    CarPhysicsRecord *record4;
    WheelMatrix *matrices8;
    char unidentifiedC[4];
    WheelMatrix wheelMatrices10[2];
    unsigned short angle90;
    void Construct(CarPhysicsRecord *record);
    void SetAngle(unsigned short angle);
    unsigned short GetAngle();
};
UG2_CHECK_OFFSET(AckermannSteering, wheelMatrices10, 0x10);
UG2_CHECK_OFFSET(AckermannSteering, angle90, 0x90);

// Aerodynamic/drag helper owned by the mover at +0x50.
struct MoverAero {
    // Created inline by the mover constructor from pool 0x86b2f4.
    MoverAeroSpec *spec0;
    MoverBody *body4;
    float scale8;
    float valueC;
    float control10;
    float grounded14;
    char unidentified18[8];
    WheelVector force20;
    float value2C;
    float value30;
    float value34;
    void Construct(MoverAeroSpec *spec, MoverBody *body);
    void Apply(float *output);
};

// Driver controls object at mover +4; only observed slots are named.
class MoverInput {
public:
    virtual void UnknownSlot00() = 0;
    virtual void Update(float duration) = 0; // slot 4, called by world update 0x609ce0 (human 0x41e2c0)
    virtual void UnknownSlot08() = 0;
    virtual void UnknownSlot0C() = 0;
    virtual void UnknownSlot10() = 0;
    virtual void UnknownSlot14() = 0;
    virtual void UnknownSlot18() = 0;
    virtual void UnknownSlot1C() = 0;
    virtual int GetGearRequest() = 0;
    virtual int GetDirectGear() = 0;
    virtual void SetSelectedGear(int gear) = 0;
    virtual void SetCurrentGear(int gear) = 0;
    virtual void UnknownSlot30() = 0;
    virtual bool SetTransmission(int mode) = 0; // slot 0x34 (human input 0x407280)
    virtual void UnknownSlot38() = 0;
    virtual void UnknownSlot3C() = 0; virtual void UnknownSlot40() = 0; virtual void UnknownSlot44() = 0;
    virtual void UnknownSlot48() = 0; virtual void UnknownSlot4C() = 0; virtual void UnknownSlot50() = 0;
    virtual bool GetWheelMode54() = 0; // slot 0x54 (human input 0x410470: byte +0x2fb is set)
    virtual void UnknownSlot58() = 0; virtual void UnknownSlot5C() = 0;
    virtual void UnknownSlot60() = 0; virtual void UnknownSlot64() = 0; virtual void UnknownSlot68() = 0;
    virtual void UnknownSlot6C() = 0; virtual void UnknownSlot70() = 0; virtual void UnknownSlot74() = 0;
    virtual void UnknownSlot78() = 0; virtual void UnknownSlot7C() = 0; virtual void UnknownSlot80() = 0;
    virtual unsigned short GetGuideHeading() = 0;
    char unidentified4[0xc];
    struct InputRoute {
        signed char count0;
        unsigned char flag1;
        char unidentified2[0xe];
        float points10[20][4];
        short values150[20];
        short values178[20];
        unsigned char values1A0[20];
        signed char index1B4;
        char unidentified1B5[3];
        int tick1B8;
        int value1BC;
        int tick1C0;
        float length1C4;
        unsigned char flag1C8;
        unsigned char flag1C9;
        char unidentified1CA[6];
        void UpdateLength(); // 0x405270
        int FindIndex(float speed, WheelVector *position, WheelVector *velocity); // 0x404ef0
        void RemoveFront(int count, bool recompute); // 0x40f7a0
        int FindPointAhead(struct MoverModeOwner *car, float distance); // 0x415590
        void Advance(struct MoverModeOwner *car, float distance); // 0x41d160
    } route10;
    unsigned short steering1E0;
    char unidentified1E2[2];
    int gearRequest1E4;
    int directGear1E8;
    float wheelInputs1EC[4];
    float wheelTargets1FC[4];
    float throttle20C;
    float handbrake210;
    float brake214;
    void SetBrake(float value);
    void SetHandbrake(float value);
    void SetThrottle(float value);
    MoverInput *ConstructBase(); // 0x423330 without its vtable identity write
};

UG2_CHECK_OFFSET(MoverAero, force20, 0x20);
UG2_CHECK_OFFSET(MoverAero, value34, 0x34);
UG2_CHECK_OFFSET(MoverInput, route10, 0x10);
UG2_CHECK_OFFSET(MoverInput, steering1E0, 0x1e0);
UG2_CHECK_OFFSET(MoverInput, gearRequest1E4, 0x1e4);
UG2_CHECK_OFFSET(MoverInput, directGear1E8, 0x1e8);
UG2_CHECK_OFFSET(MoverInput, wheelInputs1EC, 0x1ec);
UG2_CHECK_OFFSET(MoverInput, throttle20C, 0x20c);

struct MoverOwnerLink {
    int value0;
    int value4;
};

struct VehicleMover;

// Car object +0x18; +0x10 selects a 0x28-byte row of the table at 0x83aa68.
struct MoverRaceEntry {
    char unidentified0[0x10];
    int index10;
    int QueryFlag5EB420(); // 0x5eb420
    void Reset6009A0(); // 0x6009a0, not recovered
};

// World object (0x890080) +0x118, used by the mode-3 reset in 0x6012b0.
struct CarResetHandler {
    void Remove409770(struct MoverModeOwner *car); // 0x409770, not recovered
    void Restore409960(struct MoverModeOwner *car); // 0x409960, not recovered
};

// Car object +0x1c. Only fields used by mover routines are named.
struct MoverOwnerObject {
    char unidentified0[0x10];
    short value10;
    char unidentified12[2];
    float value14;        // compared between the two players by 0x5f6740
    char unidentified18[0x1c8];
    MoverOwnerLink *link1E0;
    bool IsRunning();
    void Reset5FC2A0(struct MoverModeOwner *car); // 0x5fc2a0, not recovered
};

// Original 0x5a7970; tuning fields remain offset-based until identified.
struct PhysicsTuning {
    int value0;
    int value4;
    PhysicsUpgradeSelection *selection8;
    CarPhysicsRecord *recordC;
    int value10;
    PhysicsTuningValues values14;
    char unidentified3D[3];
    float values40[23];
    float values9C[9];
    float valuesC0[9];
    int valueE4;
    PhysicsTuning *Construct();
    void Bind(int carType, CarPhysicsRecord *record, PhysicsUpgradeSelection *selection); // 0x59b060
    int CountActive(); // 0x59b0e0
    bool IsActive(int index); // 0x59b080
    void ApplyEntry(int index, float *first, float *second); // 0x59b160
    float *GetField(void *record, int index, int component); // 0x59b200
    double ConvertUnits(int index, float value, bool forward); // 0x59b4a0
    double GetGearValue(float value, bool inverse); // 0x59b5c0
    float GetSign(int index); // 0x59b6c0
    float GetLinkedExtreme(int index, int component); // 0x5a79f0
    void GetRange(int index, float *minimum, float *maximum, int component, bool average); // 0x5aca50
    void LimitEngineRange(int index, float *minimum, float *maximum, int sample); // 0x5ace80
    void GetDisplayRange(int index, float *minimum, float *maximum, int component, bool display); // 0x5b09a0
    double MapValue(int index, float value, bool toDisplay); // 0x5b2800
    void GetEntryLimits(int index, float *lower, float *upper, int component, bool display); // 0x5b2980
    float SetValue(float value, int index, int component); // 0x5b3ee0
    float SetGearRatio(float value, int index, float *ratios); // 0x5b2ed0
    void SetGearRatios(float value, int index); // 0x5b3060
    void SetLinkedValue(float value, int index); // 0x5b3100
    double GetSetting(int index, int component, bool display); // 0x5b3e40
    void SetEngineSample(float value, int index, int component, bool spread); // 0x5b2c00
    void ApplyValue(float value, int index, int component); // 0x5b3fa0
    void SetTuning(float value, int index, int component, bool fromDisplay); // 0x5b4670
    void ApplyAll(); // 0x5b4780
    void GetBalanceRange(int index, int component, bool average, float *minimum, float *maximum); // 0x5a7b80
    void GetPairRange(int index, int component, bool average, float *minimum, float *maximum); // 0x5a7cb0
    void NormalizeEngine(float total); // 0x59b3c0
    double GetValue(int index, int component); // 0x5a7da0
};
UG2_CHECK_SIZE(PhysicsTuning, 0xe8);
void ExpandPhysicsTuningRange(float *range, float value); // 0x591320
float FindPhysicsCurvePeak(float *samples, int count, float start, float end, float *peakAt); // 0x5ac980

// Original 0x5ac6e0/0x5b4dc0/0x5b5930 operate on PhysicsInfo +0x10.
// Keep this subobject explicit: its record +0x30 is outer object +0x40.
struct PhysicsParameters {
    int value0;
    signed char index4;
    char unidentified5[3];
    float value8;
    float valueC;
    float value10;
    float value14;
    float value18;
    float value1C;
    float value20;
    char unidentified24[0xc];
    CarPhysicsRecord record30;
    PhysicsUpgradeSelection selection2E0;
    PhysicsTuning tuning374;
    float curveSamples45C[64];
    EngineCurve driveCurve55C;
    PhysicsParameters *Construct(); // 0x5ac6e0
    void Compose(int mode, int refresh, int flag); // 0x5b4dc0
    void Refresh(int refresh, int flag, char select);
    CarPhysicsRecord *GetRecord(bool compose); // 0x5b5350
    signed char GetClassIndex(); // 0x5b5330
    float GetPhysicsSpeed(); // 0x5b5930
    float GetDriveArea(); // 0x59a350
    void SimulatePerformance(); // 0x5b06a0
};
UG2_CHECK_OFFSET(PhysicsParameters, record30, 0x30);
UG2_CHECK_OFFSET(PhysicsParameters, selection2E0, 0x2e0);
UG2_CHECK_OFFSET(PhysicsParameters, tuning374, 0x374);
UG2_CHECK_OFFSET(PhysicsParameters, driveCurve55C, 0x55c);
UG2_CHECK_SIZE(PhysicsParameters, 0x570);

struct PhysicsInfo {
    int value0;           // car type index
    char unidentified4[0xc];
    PhysicsParameters parameters10;
    void Refresh(int secondary, int refresh, int flag, char select); // 0x639ce0
    void SetupParts(bool secondary, int unused); // 0x6399d0, not recovered
};
UG2_CHECK_OFFSET(PhysicsInfo, parameters10, 0x10);
UG2_CHECK_SIZE(PhysicsInfo, 0x580);

void ApplyPhysicsVariant(CarPhysicsRecord *, PhysicsUpgradeSelection *, float *); // 0x5a7530
void __stdcall ApplyPhysicsVariant3(CarPhysicsRecord *, PhysicsUpgradeSelection *); // 0x5a7750
void __stdcall ApplyPhysicsVariant10(CarPhysicsRecord *, PhysicsUpgradeSelection *); // 0x5a77c0
void __stdcall ApplyPhysicsVariant2(CarPhysicsRecord *, PhysicsUpgradeSelection *); // 0x5a7830
void BuildPhysicsDriveCurve(EngineCurve *output, PhysicsParameters *parameters); // 0x5b5ab0

// Car object +0xc passed to the mover constructor; only +0xd8 is observed here.
struct MoverCarInfo {
    char unidentified0[0xd8];
    signed char valueD8;
};

// Car object +0x2c; only virtual slot 0x78 is observed by the mover constructor.
class RaceParticipant {
public:
    virtual void Slot00() = 0; virtual void Slot04() = 0; virtual void Slot08() = 0;
    virtual void Update0C(float duration) = 0; // per-frame driver update from 0x609ce0
    virtual void Slot10() = 0; virtual void Slot14() = 0;
    virtual void Slot18() = 0; virtual int GetResetId1C() = 0; virtual void Slot20() = 0;
    virtual void Slot24() = 0; virtual void Slot28() = 0; virtual void Slot2C() = 0;
    virtual void Slot30() = 0; virtual void Slot34() = 0;
    virtual void NotifyContact38(struct MoverModeOwner *other, int value) = 0; // from 0x5f4960
    virtual void Slot3C() = 0; virtual void Slot40() = 0; virtual void Slot44() = 0;
    virtual void Slot48() = 0; virtual void Slot4C() = 0; virtual void Slot50() = 0;
    virtual void Slot54() = 0; virtual void Slot58() = 0; virtual void Slot5C() = 0;
    virtual void Slot60() = 0; virtual void Slot64() = 0; virtual void Slot68() = 0;
    virtual void Slot6C() = 0; virtual void Slot70() = 0; virtual void Slot74() = 0;
    virtual int GetTuningLevel() = 0;
    char unidentified4[8];
    struct ParticipantRoute *routeC;
    unsigned char flag10;
    struct ResetPlacement *GetResetPlacement(); // 0x40f690, not recovered
};

// Returned by 0x40f690; +0x70 bit 1 reverses the reset heading in one mode.
struct ResetPlacement {
    char unidentified0[0x70];
    unsigned int flags70;
};

// Physics setup held by a participant record at +0x10. Its prefix is PhysicsInfo.
struct CarPhysicsSetup {
    int value0;           // car type index
    unsigned char value4;
    unsigned char value5;
    unsigned char value6;
    unsigned char value7;
    char unidentified8[8];
    PhysicsParameters parameters10;
    unsigned char value580;
    unsigned char value581;
    char unidentified582[2];
    int value584;
    int value588;
    int value58C;
    int values590[0xaa];
    unsigned char values838[0xaa];
    char unidentified8E2[2];
    int value8E4;
    int value8E8;
    CarPhysicsSetup &operator=(const CarPhysicsSetup &source); // 0x413440
};
UG2_CHECK_OFFSET(CarPhysicsSetup, parameters10, 0x10);
UG2_CHECK_OFFSET(CarPhysicsSetup, values590, 0x590);
UG2_CHECK_OFFSET(CarPhysicsSetup, values838, 0x838);
UG2_CHECK_SIZE(CarPhysicsSetup, 0x8ec);

// Participant record, 0x940 bytes, in the array at *0x89e810 (count *0x89e814); car +0x14.
struct MoverContext {
    signed char state0;   // records with a positive value are skipped by 0x6065b0
    signed char value1;   // copied to car +0xa
    char unidentified2;
    signed char gridIndex3; // 1..6
    int value4;           // participant kind: 1 human, 2 and 3 others
    char unidentified8[8];
    CarPhysicsSetup setup10;
    char unidentified8FC[4];
    unsigned char flag900;
    char unidentified901[7];
    int carType908;       // setup +0 copied by 0x601110
    char unidentified90C[0x10];
    unsigned char hasPlacement91C;
    char unidentified91D[3];
    WheelVector placement920;
    char unidentified92C[4];
    unsigned short heading930;
    char unidentified932[0xe];
};
UG2_CHECK_OFFSET(MoverContext, setup10, 0x10);
UG2_CHECK_OFFSET(MoverContext, flag900, 0x900);
UG2_CHECK_OFFSET(MoverContext, carType908, 0x908);
UG2_CHECK_OFFSET(MoverContext, placement920, 0x920);
UG2_CHECK_OFFSET(MoverContext, heading930, 0x930);
UG2_CHECK_SIZE(MoverContext, 0x940);

// Object at car +0x968 read by the HUD gauge 0x4c68d0.
class CarReplayState {
public:
    virtual void Slot00(); virtual void Slot04(); virtual void Slot08(); virtual void Slot0C();
    virtual void Slot10(); virtual void Slot14(); virtual void Slot18(); virtual void Slot1C();
    virtual void Slot20(); virtual void Slot24(); virtual void Slot28(); virtual void Slot2C();
    virtual void Slot30(); virtual void Slot34(); virtual void Slot38(); virtual void Slot3C();
    virtual void Slot40(); virtual void Slot44();
    virtual void RecordEvent48(int kind, int value, int extra); // kinds 1 (gear) and 9 (shift effect) from 0x5ebe00
    char unidentified4[0x54];
    float rpm58;
};

// Car +0x554, used by effect emission 0x61fb00.
struct CarEffectEmitters {
    void Spawn(int kind, float intensity, int value, int high, WheelVector *first, WheelVector *second,
               WheelVector *target); // 0x615cc0, not recovered
};

// Car +0x2c +0xc.
struct ParticipantRoute {
    float *GetDirection(float *output, int forward); // 0x401b20, not recovered
    float QueryDistance(int id); // 0x5e2da0, not recovered
};

// The car object; its +0x40 region is DrivetrainAuxiliary.
struct MoverModeOwner {
    WCollisionLink link0; // world list at *0x890080 +0
    short index8;         // slot in the car tables at 0x89ccf8 (0x5f0d20)
    short valueA;         // participant record +1
    char *valueC;         // +0x7f0 is passed to the needle angle 0x4aa0e0 (unread there)

    PhysicsInfo *physicsInfo10;
    MoverContext *context14;
    struct MoverRaceEntry *raceEntry18;
    MoverOwnerObject *object1C;
    CarPhysicsRecord *record20;
    int value24;
    int moverType28;
    RaceParticipant *participant2C;
    MoverInput *input30;
    struct VehicleMover *mover34;
    int state38;
    CarCollisionBody *collisionBody3C;
    union {
        char unidentified40[0x3e4];
        struct {
            char state40[0x20];
            float draftPosition60;
            float draftPosition64;
        } view40;
    };
    int mode424;
    int value428;
    float speed42C;
    float speed430;
    int gear434;
    char unidentified438[8];
    float engineValue440;
    char unidentified444[4];
    int value448;
    char unidentified44C[0x18];
    int stability464;     // stability control option (replay event type 2)
    float value468;
    char unidentified46C[0x64];
    int gear4D0;          // last gear seen by 0x5ebe00
    int value4D4;
    float value4D8;
    float value4DC;
    float value4E0;
    float value4E4;
    float value4E8[4];    // integrated by 0x5ecac0
    float value4F8[4];    // smoothed towards car +0x330 by 0x5ecac0
    char unidentified508[8];
    WheelVector previousPosition510;
    char unidentified51C[4];
    WheelVector previousVelocity520;
    char unidentified52C[4];
    WheelVector previousAngular530;
    char unidentified53C[4];
    int value540;
    int value544;
    int value548;
    float contactTime54C; // 0x5fc050 state-1 timer start
    unsigned char effectsEnabled550;
    unsigned char value551;
    char unidentified552[2];
    CarEffectEmitters *effects554; // created by 0x63a5a0 with the car's render object
    char unidentified558[0x58];
    int value5B0;
    int value5B4;
    int value5B8;
    char unidentified5BC[4];
    unsigned char value5C0;
    char unidentified5C1[3];
    char unidentified5C4[8]; // the collider (see Collider()) starts here
    int colliderPack5CC;
    char unidentified5D0[0x32c]; // the road navigator (see Navigator()) starts at +0x8d8
    float value8FC;       // published by world update 0x609ce0 at world +0xe0 (navigator +0x24)
    float direction900;   // reset direction used when 0x88f2be is set (navigator +0x28)
    float direction904;
    char unidentified908[8];
    float value910;       // 16.0 for kind-3 participants
    struct CarTracker914 *tracker914;
    struct CarRecorder918 *recorder918;
    int value91C;
    int value920;
    unsigned char value924;
    unsigned char value925;
    unsigned char value926;
    unsigned char value927;
    unsigned char value928;
    char unidentified929[3];
    int tick92C;
    float values930[3];
    unsigned char value93C;
    unsigned char value93D;
    char unidentified93E[2];
    int value940;
    unsigned char value944;
    char unidentified945[3];
    int value948;
    int value94C;
    int value950;
    int value954;
    struct CarFeedback958 *forceFeedback958;
    unsigned char mask95C;
    unsigned char value95D;
    char unidentified95E[2];
    int value960;
    unsigned char value964;
    char unidentified965[3];
    CarReplayState *replay968; // created by audio 0x4690e0; +0x58 is the replayed engine RPM

    int resetRequested96C;
    int countdown970;     // decremented each world update 0x609ce0
    int resetTick974;
    float resetDuration978;
    unsigned char value97C;
    char unidentified97D[3];
    int values980[4];
    float value990;       // start position x
    float value994;       // start position y
    unsigned char effectsSuppressed998;
    unsigned char value999;
    char unidentified99A[2];
    int values99C[4];
    int value9AC;
    unsigned char value9B0;
    char unidentified9B1[7];
    int value9B8;
    char unidentified9BC[4];
    int value9C0;
    int value9C4;
    int value9C8;
    int value9CC;
    int value9D0;
    unsigned char value9D4;
    char unidentified9D5[3];
    int shiftTick9D8;     // clock at the last gear change, 0x5ebe00
    char unidentified9DC[4];
    MoverModeOwner *Construct(MoverContext *record, WheelVector *position, unsigned short heading,
                              float distance); // 0x604260
    void SetPhysics(struct CarPhysicsSetup *setup); // 0x601110
    void SetParticipantKind(int kind); // 0x5eba30, not recovered
    void LoadColliderPacks(); // 0x5f54a0, not recovered
    void PlaceOnGround(); // 0x5f5010, not recovered
    void CreateRender(PhysicsInfo *info); // 0x63a5a0, not recovered
    void DestroyRender(); // 0x63a610, not recovered
    void Prepare5F6E20(); // 0x5f6e20, not recovered
    struct WRoadNav *Navigator() { return reinterpret_cast<struct WRoadNav *>(reinterpret_cast<char *>(this) + 0x8d8); }
    float GetDraftSum(); // 0x5ebd80
    void NotifyContact(MoverModeOwner *other); // 0x5f4960
    void SwitchMover(int type); // 0x5f46a0, not recovered
    float GetDraft5EBC20(MoverModeOwner *other); // 0x5ebc20, not recovered
    void Update5F6CD0(); // 0x5f6cd0, not recovered
    void Update5F5580(); // 0x5f5580, not recovered
    void Update5F6350(); // 0x5f6350, not recovered
    void UpdateEffects5F5730(float distance); // 0x5f5730, not recovered
    bool HasPendingShift(); // 0x5ecd70
    WheelVector *GetPosition(); // 0x401890
    WheelVector *GetVelocity(); // 0x4018a0
    int GetWheelState(int wheel); // 0x5ed470
    void GetBodyBox(float *first, float *second); // 0x5ec250
    float *GetBodyCentre(); // 0x5ec240
    float *GetOrigin(); // 0x441890
    struct WCollider *Collider() { return reinterpret_cast<struct WCollider *>(reinterpret_cast<char *>(this) + 0x5c4); }
    DrivetrainAuxiliary *State() {
        return reinterpret_cast<DrivetrainAuxiliary *>(unidentified40);
    }
    bool HasWheelInputs();  // 0x5ed290 (Camera/CameraPush.cpp)
    bool HasWheelTargets(); // 0x5ed300 (Camera/CameraPush.cpp)
    int GetMoverWheelState(int wheel);
    bool IsResetPending();
    int HasContactBodies();
    int AreWheelsSettled();
    void RequestReset();
    void ApplyGuideForce();
    float GetEventCurveValue();
    float GetDraftValue();
    bool HasWheelEffects();
    float Update(float duration); // 0x6012b0
    void UpdateContactState(); // 0x5fc050
    void UpdateOpponentGap(); // 0x5f6740
    void UpdateGear(int gear); // 0x5ebe00
    void UpdateSmoothing(float duration); // 0x5ecac0
    void CheckBelowGround(); // 0x5ecba0
    void EmitEffect(int kind, float intensity, int value, WheelVector *target); // 0x61fb00
    void Reset5FBED0(ResetPlacement *placement, ParticipantRoute *route, float speed, int flag, float direction,
                     int unused0, int unused1, float distance); // 0x5fbed0, not recovered
    // Recovered prefixes only: remaining bodies of 0x5fbbe0/0x604740 are unresolved.
    void SnapshotFrameKinematics(float duration);
    void UpdateFrameAcceleration(float duration);
};
UG2_CHECK_OFFSET(MoverModeOwner, previousPosition510, 0x510);
UG2_CHECK_OFFSET(MoverModeOwner, previousVelocity520, 0x520);
UG2_CHECK_OFFSET(MoverModeOwner, previousAngular530, 0x530);
UG2_CHECK_OFFSET(MoverModeOwner, participant2C, 0x2c);
UG2_CHECK_OFFSET(MoverModeOwner, mode424, 0x424);
UG2_CHECK_OFFSET(MoverModeOwner, engineValue440, 0x440);
UG2_CHECK_OFFSET(MoverModeOwner, value4E4, 0x4e4);
UG2_CHECK_OFFSET(MoverModeOwner, mask95C, 0x95c);
UG2_CHECK_OFFSET(MoverModeOwner, resetTick974, 0x974);
UG2_CHECK_OFFSET(MoverModeOwner, replay968, 0x968);
UG2_CHECK_OFFSET(MoverModeOwner, valueA, 0xa);
UG2_CHECK_OFFSET(MoverModeOwner, value548, 0x548);
UG2_CHECK_OFFSET(MoverModeOwner, value5C0, 0x5c0);
UG2_CHECK_OFFSET(MoverModeOwner, colliderPack5CC, 0x5cc);
UG2_CHECK_OFFSET(MoverModeOwner, value8FC, 0x8fc);
UG2_CHECK_OFFSET(MoverModeOwner, tracker914, 0x914);
UG2_CHECK_OFFSET(MoverModeOwner, tick92C, 0x92c);
UG2_CHECK_OFFSET(MoverModeOwner, forceFeedback958, 0x958);
UG2_CHECK_OFFSET(MoverModeOwner, value990, 0x990);
UG2_CHECK_OFFSET(MoverModeOwner, values99C, 0x99c);
UG2_CHECK_OFFSET(MoverModeOwner, value9B8, 0x9b8);
UG2_CHECK_OFFSET(MoverModeOwner, shiftTick9D8, 0x9d8);
UG2_CHECK_SIZE(MoverModeOwner, 0x9e0);
UG2_CHECK_OFFSET(MoverModeOwner, stability464, 0x464);
UG2_CHECK_OFFSET(MoverModeOwner, physicsInfo10, 0x10);
UG2_CHECK_OFFSET(MoverModeOwner, effectsEnabled550, 0x550);
UG2_CHECK_OFFSET(MoverModeOwner, colliderPack5CC, 0x5cc);
UG2_CHECK_OFFSET(MoverModeOwner, effectsSuppressed998, 0x998);
UG2_CHECK_OFFSET(MoverModeOwner, gear4D0, 0x4d0);
UG2_CHECK_OFFSET(MoverModeOwner, value4E8, 0x4e8);
UG2_CHECK_OFFSET(MoverModeOwner, value4F8, 0x4f8);
UG2_CHECK_OFFSET(MoverModeOwner, contactTime54C, 0x54c);
UG2_CHECK_OFFSET(MoverModeOwner, effects554, 0x554);
UG2_CHECK_OFFSET(MoverModeOwner, value5B0, 0x5b0);
UG2_CHECK_OFFSET(MoverModeOwner, direction900, 0x900);
UG2_CHECK_OFFSET(MoverModeOwner, shiftTick9D8, 0x9d8);
UG2_CHECK_OFFSET(MoverModeOwner, value8FC, 0x8fc);
UG2_CHECK_OFFSET(MoverModeOwner, countdown970, 0x970);
UG2_CHECK_OFFSET(MoverModeOwner, input30, 0x30);
UG2_CHECK_OFFSET(MoverOwnerObject, link1E0, 0x1e0);
UG2_CHECK_OFFSET(MoverOwnerObject, value14, 0x14);

// PhysicsMover vtable 0x7a25e0 as dispatched by car update 0x6012b0: slot 4 is
// the step (recovered VehicleMover::Update). Other slots are unidentified.
class VehicleMoverSlots {
public:
    virtual void Slot00();
    virtual float Step(float duration);
};

// PhysicsMover, 0x130 bytes, vtable 0x7a25e0; integrator subobject at +8.
struct VehicleMover {
    void *vtable0;
    MoverInput *input4;
    char integrator8[0x14];
    int stateCount1C;
    MoverBody *body20;
    AckermannSteering *steering24;
    MoverArm *arms28[4];
    DrivetrainWheel *wheels38[4];
    Engine *engine48;
    Drivetrain *drivetrain4C;
    MoverAero *aero50;
    CarPhysicsRecord *record54;
    DrivetrainAuxiliary *state58;
    MoverModeOwner *owner5C;
    int value60;
    unsigned char flag64;
    char unidentified65[3];
    int value68;
    float value6C;
    float value70;
    int value74[4];
    unsigned char flag84;
    char unidentified85[0xb];
    WheelVector vector90;
    char unidentified9C[0x44];
    WheelVector vectorE0;
    char unidentifiedEC[4];
    WheelVector vectorF0;
    char unidentifiedFC[4];
    WheelVector sum100;
    char unidentified10C[4];
    WheelVector sum110;
    char unidentified11C[8];
    int value124;
    char unidentified128[8];

    VehicleIntegrator *Integrator() {
        return reinterpret_cast<VehicleIntegrator *>(integrator8);
    }
    void Construct(CarPhysicsRecord *record, MoverCarInfo *carInfo, MoverInput *input,
                   MoverModeOwner *owner);
    float Update(float duration);
    void InitialiseFromState();
    void RefreshLoads();
    void RefreshModes();
    void TransferControls();
    void DistributeBrakes(float duration);
    void ApplyLaunchAssist();
    void ExportState(float duration);
    void UpdateAfterIntegration();
    void FinishIntegration();
    void ApplyControllerForces();
    void ApplyYawAssist();
    void ApplyTractionAssist();
    void ApplyAirborneAssist();
    void ApplyNetworkCorrection();
    void SetResponseScale(float scale);
    void ResolveRestAndBumpStops();
    void ApplyWheelLift();
    // Original PhysicsMover vtable slot 0x0c (0x503b00).
    MoverBody *GetBody() {
        return body20;
    }
    void CorrectWheelCoupling();
    void ApplyOpposingBodyForce();
};
UG2_CHECK_OFFSET(VehicleMover, body20, 0x20);
UG2_CHECK_OFFSET(VehicleMover, record54, 0x54);
UG2_CHECK_OFFSET(VehicleMover, flag84, 0x84);
UG2_CHECK_OFFSET(VehicleMover, vectorE0, 0xe0);
UG2_CHECK_OFFSET(VehicleMover, sum100, 0x100);
UG2_CHECK_OFFSET(VehicleMover, value124, 0x124);
UG2_CHECK_SIZE(VehicleMover, 0x130);

struct DraftTable {
    char unidentified0[0x10];
    int *offsets10;
};

// Draft tracker object at 0x8a1f28; 0x5fa540 (Update) is not yet recovered.
class DraftTracker {
public:
    char unidentified0[0xc];
    DraftTable *table0C;
    char unidentified10[8];
    int offset18;
    void Update(float first, float second);
    float GetValue(int index);
};

// Replay/ghost recorder object at 0x89cf48; only two queried methods are observed.
class ReplayRecorder {
public:
    int state0;
    int *state4;
    char unidentified8[0x19];
    unsigned char flag21;
    char unidentified22[0x36];
    int tick58;
    char unidentified5C[0xbd];
    unsigned char flag119;
    bool QueryState5EFE70(); // 0x5efe70
    bool QueryState5F00F0();
    int QueryState5F0880(); // 0x5f0880
};
UG2_CHECK_OFFSET(ReplayRecorder, flag21, 0x21);
UG2_CHECK_OFFSET(ReplayRecorder, tick58, 0x58);
UG2_CHECK_OFFSET(ReplayRecorder, flag119, 0x119);
extern int *raceState890118;
unsigned short WheelAngle(float x, float y);
float ComputeTractionFactor(float slip, float handbrake, float speed, float base, float extra,
                            int grounded, float *tilt, float event);
int UpdateMoverCentre(MoverModeOwner *owner, MoverBody *body, unsigned char *pending);

extern float moverOpposingForceThreshold;

extern float bodyThresholdBase;
extern float bodyFlagThresholdScale;
extern float bodyPositiveTiltLimit;
extern float bodyPositiveTiltFactor;
extern float bodyNegativeTiltLimit;
extern float bodyNegativeTiltFactor;
extern float bodyHorizontalScale;
extern float bodyTiltResponseScale;
extern float bodyTiltResponseMaximum;
extern float bodyAScale;
extern float bodyCScale;
extern float bodyScalarRise;
extern float bodyScalarFall;
extern float bodyAngularDamping;
extern float bodyLinearDamping;
extern float bodyPostSpeedLimit;
extern double bodyPostMassThreshold;
extern float bodyPostResponseScale;
extern float bodyPostAngularLimit;
extern float bodyPointForceLimit;
extern float bodyPointTorqueLimit;
void BodyMatrixDerivative(WheelMatrix *output, WheelVector *input, WheelMatrix *matrix);

#endif
