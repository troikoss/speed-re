#include "NativeVehicle.h"
#include "NativePlayer.h"
#ifdef UG2_ENABLE_MODDING
#include "NativeModBridge.h"
#endif
#include "../../Speed/Indep/Libs/bWare/Strings.h"
#include "../../Speed/Indep/Src/Physics/ForceFeedback.h"
#include "OriginalImage.h"
#include "../../Speed/Indep/Src/Physics/VehicleIntegrator.h"
#include "../../Speed/Indep/Src/Physics/VehicleAllocation.h"
#include "../../Speed/Indep/Src/Physics/PhysicsUpgradeSource.h"
#include "../../Speed/Indep/Src/World/WCollision.h"
#include "../../Speed/Indep/Src/Camera/CameraPush.h"
#include "../../Speed/Indep/Src/Physics/WorldUpdate.h"
#include "../../Speed/Indep/Src/Physics/CarConstruction.h"
#include "../../Speed/Indep/Libs/bWare/bSlotPool.h"
#include "../../Speed/Indep/Src/Hud/HudPlayerMasks.h"
#include "../../Speed/Indep/Src/Frontend/PauseMenu.h"

#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <vector>

void RunPhysicsStaticInitializers();

void NativeTrap(const char *what) {
    fprintf(stderr, "ug2demo: reached unrecovered original code: %s\n", what);
    abort();
}

// ---------------------------------------------------------------------------
// Integration block vtables. The original classes install these tables:
// body 0x7a20c4, wheel 0x7a20e8, arm 0x7a210c, engine 0x7a235c, drivetrain 0x7a2508.
// Each native slot forwards to the recovered method; tiny slot bodies are noted.

extern unsigned char *carTypeInfoTable; // original 0x8a1ccc

namespace {

template <class Self>
class BlockBase : public IntegrationStateBlock {
protected:
    Self *self() { return reinterpret_cast<Self *>(this); }
public:
    void UnknownSlot0() { NativeTrap("integration block destructor"); }
    void UnknownSlot8() { NativeTrap("integration block state copy-out"); }
    void UnknownSlot16() { NativeTrap("integration block state copy-in"); }
    void UnknownSlot20() {}               // 0x4022c0: empty.
    void BeforeIntegrate() {}             // 0x50d510 returns 0; body 0x5a80d0 returns a static pointer.
};

class BodyBlock : public BlockBase<MoverBody> {
public:
    int GetStateCount() { return 32; }                               // 0x5a80b0
    float *GetStateValues() { return &self()->position20.x; }        // 0x5a80c0
    void WriteDerivatives(float *output) { self()->WriteDerivative(output); }
    void UpdateAfterIntegration() { self()->PostUpdate(); }
};

class WheelBlock : public BlockBase<DrivetrainWheel> {
public:
    int GetStateCount() { return 2; }                                // 0x408cd0
    float *GetStateValues() { return &self()->angle24; }             // 0x5a8230
    void WriteDerivatives(float *output) { self()->WriteBlockDerivative(output); }
    void UpdateAfterIntegration() { self()->UpdateAfterIntegration(); }
};

class ArmBlock : public BlockBase<MoverArm> {
public:
    int GetStateCount() { return 2; }                                // 0x408cd0
    float *GetStateValues() { return &self()->travel1C; }            // 0x5a8740
    void WriteDerivatives(float *output) { self()->WriteDerivative(output); }
    void UpdateAfterIntegration() { self()->UpdateAfterIntegration(); }
};

class EngineBlock : public BlockBase<Engine> {
public:
    int GetStateCount() { return 1; }                                // 0x40cb20
    float *GetStateValues() { return &self()->state14; }             // 0x5aa700
    void WriteDerivatives(float *output) { self()->WriteDerivative(output); }
    void UpdateAfterIntegration() { self()->UpdateAfterIntegration(); }
};

class DrivetrainBlock : public BlockBase<Drivetrain> {
public:
    int GetStateCount() { return 1; }                                // 0x40cb20
    float *GetStateValues() { return &self()->state14; }             // 0x5aa700
    void WriteDerivatives(float *output) { self()->WriteDerivative(output); }
    void UpdateAfterIntegration() { self()->UpdateAfterIntegration(); }
};

// PhysicsMover integrator subobject at mover +8, vtable 0x7a25d4.
class MoverIntegrator : public VehicleIntegrator {
    VehicleMover *mover() { return reinterpret_cast<VehicleMover *>(reinterpret_cast<char *>(this) - 8); }
public:
    void UnknownSlot0() { NativeTrap("PhysicsMover integrator slot 0 (0x5b04c0)"); }
    void AfterIntegrate() { mover()->FinishIntegration(); }          // 0x5b04d0
};

template <class Shim>
void *AllocateBlock(size_t size) {
    // Original slot pools (0x440e50) are not zero-filled; zeroing keeps runs repeatable.
    void *memory = calloc(1, size);
    new (memory) Shim;
    return memory;
}

int raceStatusScaffold[0x400];
// Scaffolds for objects created by race setup (0x60a6d4 / 0x5ff537): a context without
// an entry at +0xa0 (variations enabled) and a random source with a fixed seed.
DrivetrainContext contextScaffold;
// Scaffold world object (*0x890080). Recovered code views it as CarWorld (0x609ce0),
// CollisionEventOwner, CameraCarList and DrivetrainRandom (+0xf8).
CarWorld worldScaffold;
DrivetrainRandom &randomScaffold = *reinterpret_cast<DrivetrainRandom *>(&worldScaffold);
GameFlow8900D8 flowScaffold;
}  // namespace
extern GameFlow8900D8 *gameFlow8900D8;
namespace {

// Scaffold driver (car +0x2c) for the demo's human car: the per-frame driver update
// (slot 0xc) and contact notification (slot 0x38) are not recovered and do nothing.
class NativeParticipant : public RaceParticipant {
public:
    void Slot00() { NativeTrap("participant slot 0x00"); }
    void Slot04() { NativeTrap("participant slot 0x04"); }
    void Slot08() { NativeTrap("participant slot 0x08"); }
    void Update0C(float) {}
    void Slot10() { NativeTrap("participant slot 0x10"); }
    void Slot14() { NativeTrap("participant slot 0x14"); }
    void Slot18() { NativeTrap("participant slot 0x18"); }
    int GetResetId1C() { NativeTrap("participant slot 0x1c"); return -1; }
    void Slot20() { NativeTrap("participant slot 0x20"); }
    void Slot24() { NativeTrap("participant slot 0x24"); }
    void Slot28() { NativeTrap("participant slot 0x28"); }
    void Slot2C() { NativeTrap("participant slot 0x2c"); }
    void Slot30() { NativeTrap("participant slot 0x30"); }
    void Slot34() { NativeTrap("participant slot 0x34"); }
    void NotifyContact38(MoverModeOwner *, int) {}
    void Slot3C() { NativeTrap("participant slot 0x3c"); }
    void Slot40() { NativeTrap("participant slot 0x40"); }
    void Slot44() { NativeTrap("participant slot 0x44"); }
    void Slot48() { NativeTrap("participant slot 0x48"); }
    void Slot4C() { NativeTrap("participant slot 0x4c"); }
    void Slot50() { NativeTrap("participant slot 0x50"); }
    void Slot54() { NativeTrap("participant slot 0x54"); }
    void Slot58() { NativeTrap("participant slot 0x58"); }
    void Slot5C() { NativeTrap("participant slot 0x5c"); }
    void Slot60() { NativeTrap("participant slot 0x60"); }
    void Slot64() { NativeTrap("participant slot 0x64"); }
    void Slot68() { NativeTrap("participant slot 0x68"); }
    void Slot6C() { NativeTrap("participant slot 0x6c"); }
    void Slot70() { NativeTrap("participant slot 0x70"); }
    void Slot74() { NativeTrap("participant slot 0x74"); }
    int GetTuningLevel() { NativeTrap("participant tuning level"); return 0; }
};
NativeParticipant participantScaffold;

}  // namespace

MoverBody *NewMoverBody(MoverBodySpec *spec) {
    MoverBody *body = static_cast<MoverBody *>(AllocateBlock<BodyBlock>(0x210));
    body->Construct(spec);
    return body;
}

AckermannSteering *NewAckermannSteering(CarPhysicsRecord *record) {
    AckermannSteering *steering = static_cast<AckermannSteering *>(calloc(1, 0x94));
    steering->Construct(record);
    return steering;
}

DrivetrainWheel *NewDrivetrainWheel(DrivetrainWheelSpec *spec, WheelCorrectionSpec *correction,
                                    WheelPreparationSpec *preparation, DrivetrainAuxiliary *auxiliary) {
    DrivetrainWheel *wheel = static_cast<DrivetrainWheel *>(AllocateBlock<WheelBlock>(0x1a0));
    wheel->Construct(spec, correction, preparation, auxiliary);
    return wheel;
}

MoverArm *NewMoverArm(int index, MoverBody *body, DrivetrainWheel *wheel, AckermannSteering *steering,
                      SuspensionSpec *spec) {
    MoverArm *arm = static_cast<MoverArm *>(AllocateBlock<ArmBlock>(0xb8));
    arm->Construct(index, body, wheel, steering, spec);
    return arm;
}

Engine *NewEngine(EngineSpec *spec, EngineAttributeOwner *owner) {
    Engine *engine = static_cast<Engine *>(AllocateBlock<EngineBlock>(0xb0));
    engine->Construct(spec, owner);
    return engine;
}

Drivetrain *NewDrivetrain(DrivetrainSpec *spec, DrivetrainTuning *tuning, DrivetrainComponent *engine,
                          DrivetrainWheel **wheels, DrivetrainAuxiliary *auxiliary) {
    Drivetrain *drivetrain = static_cast<Drivetrain *>(AllocateBlock<DrivetrainBlock>(0xf8));
    drivetrain->Construct(spec, tuning, engine, wheels, auxiliary);
    return drivetrain;
}

MoverAero *NewMoverAero(MoverAeroSpec *spec, MoverBody *body) {
    MoverAero *aero = static_cast<MoverAero *>(calloc(1, 0x38));
    aero->Construct(spec, body);
    return aero;
}


// D3DX runtime library (statically linked in the original, dispatched via 0x803e74).
// Row-vector product: output = left * right; operands may alias the output.
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output, WheelMatrix *left,
                                                     WheelMatrix *right) {
    WheelMatrix result;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += left->elements[row * 4 + k] * right->elements[k * 4 + column];
            }
            result.elements[row * 4 + column] = sum;
        }
    }
    *output = result;
    return output;
}

// D3DX transpose (statically linked in the original, dispatched via 0x803f00, identified from the
// use in 0x447280 which reads the result as the transposed matrix). Operands may alias the output.
extern "C" WheelMatrix *__stdcall D3DXMatrixTranspose(WheelMatrix *output, WheelMatrix *input) {
    WheelMatrix result;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            result.elements[row * 4 + column] = input->elements[column * 4 + row];
        }
    }
    *output = result;
    return output;
}

unsigned int bGetTicker() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (unsigned int)(now.tv_sec * 1000000u + now.tv_nsec / 1000u);
}

// Unrecovered original routines. The demo state never reaches them; if it does, stop.
void MoverOwnerObject::Reset5FC2A0(MoverModeOwner *) { NativeTrap("car race object reset 0x5fc2a0"); }
// Unrecovered callees of world update 0x609ce0. The per-frame ones are native no-ops, as
// the demo never ran them before the world update was recovered; ones the demo state
// cannot reach (single car, no race entry, no +0x118 subsystem) trap.
void CarWorld::Prepare5FA770() {}
void UpdateCars5FC0E0() {}
void MoverModeOwner::Update5F6CD0() {}
void WorldObject81CBC0::Update4268A0() {}
void Update614820() {}
void WorldSubsystem118::Update42F910(float) { NativeTrap("world subsystem update 0x42f910"); }
float MoverModeOwner::GetDraft5EBC20(MoverModeOwner *) { NativeTrap("car draft 0x5ebc20"); return 0.0f; }
void Update59E350() {}
void MoverModeOwner::Update5F5580() {}
void UpdateRace572E50(int, WheelVector *) { NativeTrap("race entry update 0x572e50"); }
void MoverModeOwner::Update5F6350() {}
void WorldObject8357E0::Update492E10(float) {}
void MoverModeOwner::UpdateEffects5F5730(float) {}
void Update605780() {}
void Update5E0A00() {}
void Update5E5430() {}
// A list of at most one node (the demo's car) is already in order.
void CarWorld::Sort43E650(int (*)(void *, void *)) {
    if (list0.next != &list0 && list0.next->next != &list0) NativeTrap("world list sort 0x43e650");
}
int CompareWorldEntry5EA240(void *, void *) { NativeTrap("world list comparator 0x5ea240"); return 0; }
void PlayerRecord::Update602650() {}
void WorldObject827B88::Update43AF20(float) {}
void Update638070() {}
void WorldObject88EB00::Update5E86E0() {}
void RaceStatus890118::Update6076D0() {}
// Unrecovered callees of car update 0x6012b0. The reset branch runs only once a reset is
// requested (0x5ecba0 or 0x5ebf60); effect and replay calls need car +0x554/+0x968, which
// the demo leaves null.
void MoverRaceEntry::Reset6009A0() { NativeTrap("race entry reset 0x6009a0"); }
void CarResetHandler::Remove409770(MoverModeOwner *) { NativeTrap("reset handler 0x409770"); }
void CarResetHandler::Restore409960(MoverModeOwner *) { NativeTrap("reset handler 0x409960"); }
ResetPlacement *RaceParticipant::GetResetPlacement() { NativeTrap("participant reset placement 0x40f690"); return 0; }
float *ParticipantRoute::GetDirection(float *, int) { NativeTrap("participant route direction 0x401b20"); return 0; }
float ParticipantRoute::QueryDistance(int) { NativeTrap("participant route distance 0x5e2da0"); return 0.0f; }
void ReleaseResetId403DB0(int) { NativeTrap("reset id release 0x403db0"); }
void MoverModeOwner::Reset5FBED0(ResetPlacement *, ParticipantRoute *, float, int, float, int, int, float) {
    NativeTrap("car reset 0x5fbed0");
}
int WCollider::HasGround(WheelVector *) { NativeTrap("collider ground test 0x5e7e80"); return 0; }
void CarEffectEmitters::Spawn(int, float, int, int, WheelVector *, WheelVector *, WheelVector *) {
    NativeTrap("car effect spawn 0x615cc0");
}
void CarReplayState::Slot00() { NativeTrap("replay slot 0x00"); }
void CarReplayState::Slot04() { NativeTrap("replay slot 0x04"); }
void CarReplayState::Slot08() { NativeTrap("replay slot 0x08"); }
void CarReplayState::Slot0C() { NativeTrap("replay slot 0x0c"); }
void CarReplayState::Slot10() { NativeTrap("replay slot 0x10"); }
void CarReplayState::Slot14() { NativeTrap("replay slot 0x14"); }
void CarReplayState::Slot18() { NativeTrap("replay slot 0x18"); }
void CarReplayState::Slot1C() { NativeTrap("replay slot 0x1c"); }
void CarReplayState::Slot20() { NativeTrap("replay slot 0x20"); }
void CarReplayState::Slot24() { NativeTrap("replay slot 0x24"); }
void CarReplayState::Slot28() { NativeTrap("replay slot 0x28"); }
void CarReplayState::Slot2C() { NativeTrap("replay slot 0x2c"); }
void CarReplayState::Slot30() { NativeTrap("replay slot 0x30"); }
void CarReplayState::Slot34() { NativeTrap("replay slot 0x34"); }
void CarReplayState::Slot38() { NativeTrap("replay slot 0x38"); }
void CarReplayState::Slot3C() { NativeTrap("replay slot 0x3c"); }
void CarReplayState::Slot40() { NativeTrap("replay slot 0x40"); }
void CarReplayState::Slot44() { NativeTrap("replay slot 0x44"); }
void CarReplayState::RecordEvent48(int, int, int) { NativeTrap("replay event 0x48"); }
// Native vtable seam for PhysicsMover 0x7a25e0: slot 4 runs the recovered step.
void VehicleMoverSlots::Slot00() { NativeTrap("mover slot 0x00"); }
float VehicleMoverSlots::Step(float duration) { return reinterpret_cast<VehicleMover *>(this)->Update(duration); }
// Force-feedback device manager (*0x870760) methods are unrecovered DirectInput effects.
// Natively the manager pointer is null, so recovered 0x406460 never reaches them.
bool ForceFeedbackManager::HasDevice5BFD70(int) { NativeTrap("force feedback 0x5bfd70"); return false; }
bool ForceFeedbackManager::IsEffectActive5BFD90(int, int) { NativeTrap("force feedback 0x5bfd90"); return false; }
void ForceFeedbackManager::Stop5BFE00(int) { NativeTrap("force feedback 0x5bfe00"); }
void ForceFeedbackManager::Stop5BFE70(int) { NativeTrap("force feedback 0x5bfe70"); }
void ForceFeedbackManager::Play5C9150(int, int, int, int) { NativeTrap("force feedback 0x5c9150"); }
bool ForceFeedbackManager::IsEffectActive5C9040(int, int) { NativeTrap("force feedback 0x5c9040"); return false; }
void ForceFeedbackManager::SetConstant5C9550(int, int, int) { NativeTrap("force feedback 0x5c9550"); }
void ForceFeedbackManager::SetSpring5C9590(int, int) { NativeTrap("force feedback 0x5c9590"); }
void ForceFeedbackManager::PlayJolt5C9760(int, int) { NativeTrap("force feedback 0x5c9760"); }
void ForceFeedbackManager::SetRumble5C9D90(int, int) { NativeTrap("force feedback 0x5c9d90"); }
void ForceFeedbackManager::SetSurface5C9FD0(int, int, int, int) { NativeTrap("force feedback 0x5c9fd0"); }
void ForceFeedbackManager::Stop5CD850(int) { NativeTrap("force feedback 0x5cd850"); }
void ForceFeedbackManager::Stop5CD910(int) { NativeTrap("force feedback 0x5cd910"); }
void ForceFeedbackManager::Stop5CD9D0(int) { NativeTrap("force feedback 0x5cd9d0"); }
void ForceFeedbackManager::Start5CDA10(int) { NativeTrap("force feedback 0x5cda10"); }
void ForceFeedbackManager::Stop5CDBB0(int) { NativeTrap("force feedback 0x5cdbb0"); }
// Skid-mark trail methods belong to the skid-mark renderer (0x5e6e70/0x5d3230).
// Scaffold: record segments for the demo renderer instead of the original effect.
NativeSkidSegment nativeSkidSegments[NATIVE_SKID_CAPACITY];
int nativeSkidCount;

void SkidMarkTrail::AddSegment(MoverModeOwner *, WheelVector *centre, WheelVector *across, int surface,
                               float intensity) {
    NativeSkidSegment *segment = &nativeSkidSegments[nativeSkidCount % NATIVE_SKID_CAPACITY];
    segment->centre = *centre;
    segment->across = *across;
    segment->intensity = intensity;
    segment->surface = surface;
    segment->connected = trail != 0;
    trail = this;
    ++nativeSkidCount;
}

void SkidMarkTrail::End() {
    trail = 0;
}
void DraftTracker::Update(float, float) { NativeTrap("draft tracker 0x5fa540"); }


// ---------------------------------------------------------------------------
// Controls scaffolding.

void NativeControls::UnknownSlot00() { NativeTrap("controls slot 0x00"); }
void NativeControls::UnknownSlot08() { NativeTrap("controls slot 0x08"); }
void NativeControls::UnknownSlot0C() { NativeTrap("controls slot 0x0c"); }
void NativeControls::UnknownSlot10() { NativeTrap("controls slot 0x10"); }
void NativeControls::UnknownSlot14() { NativeTrap("controls slot 0x14"); }
void NativeControls::UnknownSlot18() { NativeTrap("controls slot 0x18"); }
void NativeControls::UnknownSlot1C() { NativeTrap("controls slot 0x1c"); }
int NativeControls::GetGearRequest() { return ReadGearRequest(); }
int NativeControls::GetDirectGear() { return ReadDirectGear(); }
void NativeControls::SetSelectedGear(int gear) { StoreGearRequest(gear); }
void NativeControls::SetCurrentGear(int gear) { StoreDirectGear(gear); }
void NativeControls::UnknownSlot30() { SelectForward(); }
#define NATIVE_TRAP_SLOT(name) void NativeControls::name() { NativeTrap("controls " #name); }
bool NativeControls::SetTransmission(int mode) { return ApplyTransmission(mode); }
NATIVE_TRAP_SLOT(UnknownSlot38)
NATIVE_TRAP_SLOT(UnknownSlot3C) NATIVE_TRAP_SLOT(UnknownSlot40) NATIVE_TRAP_SLOT(UnknownSlot44)
NATIVE_TRAP_SLOT(UnknownSlot48) NATIVE_TRAP_SLOT(UnknownSlot4C) NATIVE_TRAP_SLOT(UnknownSlot50)
NATIVE_TRAP_SLOT(UnknownSlot58) NATIVE_TRAP_SLOT(UnknownSlot5C)
NATIVE_TRAP_SLOT(UnknownSlot60) NATIVE_TRAP_SLOT(UnknownSlot64) NATIVE_TRAP_SLOT(UnknownSlot68)
NATIVE_TRAP_SLOT(UnknownSlot6C) NATIVE_TRAP_SLOT(UnknownSlot70) NATIVE_TRAP_SLOT(UnknownSlot74)
NATIVE_TRAP_SLOT(UnknownSlot78) NATIVE_TRAP_SLOT(UnknownSlot7C) NATIVE_TRAP_SLOT(UnknownSlot80)
// Slot 0x54 as the human input's 0x410470 (the drive camera's cockpit test reads it through car +0x30).
bool NativeControls::GetWheelMode54() { return wheelMode2FB != 0; }
unsigned short NativeControls::GetGuideHeading() { return 0; }

// 0x578e90 event dispatch remains unrecovered. Native intentionally drops the
// event; the recovered caller retains its original receiver and arguments.

// ---------------------------------------------------------------------------
// Runtime and vehicle assembly.

extern int *raceState890118;
extern unsigned int nativeCollisionHullKey;
void AdvanceWorldClock();
extern float frameDuration865154;
extern float resetDurationDefault;
extern DrivetrainContext *drivetrainContext;
extern DrivetrainRandom *drivetrainRandom;

namespace {

bool ReadFile(const std::string &path, std::vector<unsigned char> *data) {
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        return false;
    }
    unsigned char buffer[65536];
    size_t count;
    while ((count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        data->insert(data->end(), buffer, buffer + count);
    }
    fclose(file);
    return true;
}

unsigned int Read32(const std::vector<unsigned char> &data, size_t offset) {
    return data[offset] | data[offset + 1] << 8 | data[offset + 2] << 16 | (unsigned int)data[offset + 3] << 24;
}

// Platform scaffolding for the GLOBALB.BUN CarTypeInfo chunk (0x34600). The original
// chunk loader remains unrecovered; this seam keeps the chunk's 0x890-byte rows resident
// and installs them as the original table pointer 0x8a1ccc, which recovered composer
// 0x5b4dc0 and the tuning helpers index by car (declared at file scope above).

bool LoadCarTypeInfo(const char *gameDirectory, const char *carName,
                     CameraSettingsTable *cameraSettings, int *carIndex, char *modelName, char *error,
                     int errorSize) {
    static std::vector<unsigned char> bundle;
    if (bundle.empty()) {
        std::string path = std::string(gameDirectory) + "/GLOBAL/GLOBALB.BUN";
        if (!ReadFile(path, &bundle)) {
            snprintf(error, errorSize, "cannot read %s", path.c_str());
            return false;
        }
    }
    for (size_t offset = 0; offset + 8 <= bundle.size();) {
        unsigned int id = Read32(bundle, offset);
        unsigned int size = Read32(bundle, offset + 4);
        if (id == 0x34600) {
            size_t table = offset + 8 + 8;
            int count = (int)((size - 8) / 0x890);
            carTypeInfoTable = &bundle[table];
            for (int i = 0; i < count; ++i) {
                const char *name = reinterpret_cast<const char *>(&bundle[table + i * 0x890]);
                if (strncmp(name, carName, 32) == 0) {
                    memcpy(cameraSettings, &bundle[table + i * 0x890 + 0x720], sizeof(CameraSettingsTable));
                    memcpy(modelName, &bundle[table + i * 0x890 + 0x20], 32);
                    modelName[31] = 0;
                    *carIndex = i;
                    return true;
                }
            }
            snprintf(error, errorSize, "car %s not found in CarTypeInfo", carName);
            return false;
        }
        offset += 8 + size;
    }
    snprintf(error, errorSize, "CarTypeInfo chunk 0x34600 not found");
    return false;
}

// Platform scaffolding for the car geometry loader: read the car's collision hull chunk
// (0x80034020) once per car and hand it to the recovered hull loader 0x5a1750. The hull
// key is the CarTypeInfo model name (+0x20) hashed with "_CV" (0x61c320, stock parts).
bool LoadCarHulls(const char *gameDirectory, const char *carName, const char *modelName, char *error,
                  int errorSize) {
    static std::vector<std::string> loaded;
    nativeCollisionHullKey = cshashWithStartValue("_CV", cshash(modelName));
    for (size_t i = 0; i < loaded.size(); ++i) {
        if (loaded[i] == carName) {
            return true;
        }
    }
    std::vector<unsigned char> geometry;
    std::string path = std::string(gameDirectory) + "/CARS/" + carName + "/GEOMETRY.BIN";
    if (!ReadFile(path, &geometry)) {
        snprintf(error, errorSize, "cannot read %s", path.c_str());
        return false;
    }
    for (size_t offset = 0; offset + 8 <= geometry.size();) {
        unsigned int id = Read32(geometry, offset);
        unsigned int size = Read32(geometry, offset + 4);
        if (id == 0x80034020) {
            // Keep the chunk's 16-byte alignment phase; the loader aligns inside it.
            unsigned char *raw = static_cast<unsigned char *>(malloc(size + 8 + 32));
            unsigned char *chunk = reinterpret_cast<unsigned char *>(
                ((reinterpret_cast<size_t>(raw) + 15) & ~static_cast<size_t>(15)) + (offset & 15));
            memcpy(chunk, &geometry[offset], size + 8);
            LoadCollisionHulls(reinterpret_cast<int *>(chunk));
            break;
        }
        offset += 8 + size;
    }
    loaded.push_back(carName);
    return true;
}

}  // namespace

void DestroyNativeVehicle(NativeVehicle *vehicle) {
#ifdef UG2_ENABLE_MODDING
    NativeModsVehicleDestroying(vehicle);
#endif
    if (vehicle->owner && vehicle->owner->collisionBody3C) {
        delete vehicle->owner->collisionBody3C;
        vehicle->owner->collisionBody3C = 0;
    }
    if (vehicle->controls) {
        vehicle->controls->steering220.Destroy();
        vehicle->controls = 0;
        if (vehicle->owner) vehicle->owner->input30 = 0;
    }
    // Scaffold teardown: remove the car from the world list; the car object itself is not
    // freed (no car destructor is recovered).
    if (vehicle->owner) {
        WCollisionLink *link = &vehicle->owner->link0;
        link->previous->next = link->next;
        link->next->previous = link->previous;
        link->next = link->previous = link;
    }
}

extern int audioEnabled7FAAA8;

bool InitialiseNativeRuntime(const char *gameDirectory, char *error, int errorSize) {
    std::string exe = std::string(gameDirectory) + "/speed2.exe";
    if (!MapOriginalImage(exe.c_str(), error, errorSize)) {
        return false;
    }
    // Platform seam: no sound device, so the audio enable flag takes the value 0x580130 stores
    // when the device is absent.
    audioEnabled7FAAA8 = 0;
    RunPhysicsStaticInitializers();
    // Collision-body world: static list heads, then the pools created at race start
    // (0x5a0200, 0x593db0, 0x592b00).
    RunCollisionStaticInitializers();
    InitialiseCollisionPools();
    InitialiseContactPool();
    InitialiseCollisionEventPool();
    // Scaffold race-status object (constructed by 0x5fc730 with phase 0, free drive).
    raceState890118 = raceStatusScaffold;
    drivetrainContext = &contextScaffold;
    randomScaffold.stateF8 = 0x12345678u;
    drivetrainRandom = &randomScaffold;
    if (!gameFlow8900D8) gameFlow8900D8 = &flowScaffold;
    // The same world object (0x890080) holds the collision event lists at +0 and +8.
    CollisionEventOwner *events = reinterpret_cast<CollisionEventOwner *>(&randomScaffold);
    events->unidentified0.next = events->unidentified0.previous = &events->unidentified0;
    events->events8.next = events->events8.previous = &events->events8;
    return true;
}

// ---------------------------------------------------------------------------
// Car construction seams. Participant setup 0x6065b0, car creation 0x606480, the car
// constructor 0x604260 and physics binding 0x601110 are recovered (CarConstruction.cpp);
// these stand in for their unrecovered callees.

extern MoverContext *participantRecords89E810;
extern int participantCount89E814;
extern void *carNullMoverPool86B304;
extern CarWorld *carWorld890080;

// Allocation boundary 0x575620/0x575630. The original heap is not zero-filled; zeroing keeps
// runs repeatable.
void *AllocateCarMemory(unsigned int size) { return calloc(1, size); }
void FreeCarMemory(void *memory) { free(memory); }

void CarNullMover::Slot00() { NativeTrap("null mover slot 0x00"); }
// 0x5eb930 builds the base driver and its input (0x423330, vtable 0x7855d8). Car creation
// replaces it at once through SetParticipantKind, so the native stand-in builds nothing.
CarDriver::CarDriver(MoverModeOwner *) { input8 = 0; }
// 0x63b590 registers the object in the table at 0x89e970 and the list at 0x8a2ca4, and
// 0x60bcf0 stores the four pointers; nothing native reads them.
CarTracker914::CarTracker914() {}
void CarTracker914::Bind(WheelVector *, WheelVector *, WheelVector *, WheelMatrix *) {}
CarRecorder918::CarRecorder918(MoverModeOwner *) { NativeTrap("car recorder 0x5fcb00"); }
// Force-feedback effects object (0x63a360/0x63a440). There is no device natively and the
// collision consumer 0x637c70 is a native no-op.
CarFeedback958::CarFeedback958(MoverModeOwner *) {}
CarFeedback958::~CarFeedback958() {}
CarReplayState *Audio82B884::CreateCarSound(MoverModeOwner *) { NativeTrap("car sound 0x4690e0"); return 0; }
// 0x5e02b0 adds the callback to the update list at 0x88e9c4; the registered routine is empty.
void EmptyRoutine4022C0() {} // 0x4022c0, the shared empty routine
int RegisterUpdate5E02B0(void (*)(), int) { return 1; }
int SelectMover5F4400(MoverModeOwner *) { NativeTrap("mover selection 0x5f4400"); return 0; }
WCollisionLink *WorldSubsystem118::CreateEntry42F7A0(MoverContext *) { NativeTrap("world subsystem entry 0x42f7a0"); return 0; }
void MoverModeOwner::Prepare5F6E20() { NativeTrap("car preparation 0x5f6e20"); }
// Visual part selection 0x6399d0 and the render object 0x63a5a0 (which owns the effects
// emitters at +0x554) belong to the native renderer; +0x554 stays null.
void PhysicsInfo::SetupParts(bool, int) {}
void MoverModeOwner::CreateRender(PhysicsInfo *) {}
void MoverModeOwner::DestroyRender() { NativeTrap("car render release 0x63a610"); }
// 0x5f54a0 loads the car's collider packs; the native world loads every pack at startup.
void MoverModeOwner::LoadColliderPacks() {}
// Road-network placement (0x5ded90, 0x5da3d0, 0x5da410 through 0x5d9cc0/0x5d9e30) is not
// recovered: the navigators keep their zeroed state, so the constructor's distance is 0.
WRoadNav *WRoadNav::Construct(StartingPosition *) { return Construct(); }
void WRoadNav::SetPosition(WheelVector *) {}
void WRoadNav::SetPosition(WheelVector *, float, int) {}

// Scaffold for 0x5f5010 (ground placement through 0x5f49f0): a yaw matrix about +Z from the
// constructor's heading (rows are the body axes in world space). No ground fit is made.
void MoverModeOwner::PlaceOnGround() {
    DrivetrainAuxiliary *state = State();
    float angle = state->heading3CC * (6.28318530718f / 65536.0f);
    float c = cosf(angle), s = sinf(angle);
    WheelIdentity(&state->matrix50);
    state->matrix50.elements[0] = c;
    state->matrix50.elements[1] = s;
    state->matrix50.elements[4] = -s;
    state->matrix50.elements[5] = c;
    WheelIdentity(&state->matrix90);
}

// Seam for 0x5eba30: only the human kind exists natively. Its driver is the participant
// scaffold and its input the native controls with the recovered human constructor 0x425fc0.
void MoverModeOwner::SetParticipantKind(int kind) {
    if (kind != 1) NativeTrap("participant kind 0x5eba30");
    FreeCarMemory(participant2C);
    value24 = kind;
    NativeControls *controls = static_cast<NativeControls *>(calloc(1, sizeof(NativeControls)));
    new (controls) NativeControls;
    // C++ placement construction supplies the native vtable in place of the original
    // identity writes; SEH is not modelled.
    controls->ConstructHuman(this);
    // Native device seam: the original profile/controller setup assigns a device slot;
    // slot 0 is assigned in the retail image (0x864788).
    controls->device2AC = 0;
    // Native free-drive entry seam: the original session entry is unresolved. It releases
    // the pre-race brake hold (+0x2b4) and selects drive through the recovered human slot
    // instead of the constructor's neutral request.
    controls->flag2B4 = 0;
    controls->SelectForward();
    participant2C = &participantScaffold;
    input30 = controls;
    if (mover34) reinterpret_cast<CarNullMover *>(mover34)->input4 = input30;
    carKinds89CE00[index8] = value24;
    carParticipants89CE58[index8] = participant2C;
    carInputs89CEB0[index8] = input30;
}

// Seam for mover factory 0x5f46a0: type 0 keeps the null mover; type 1 replaces it with
// the PhysicsMover, marks the collision body as mover-driven and attaches it (0x5938c0).
void MoverModeOwner::SwitchMover(int type) {
    if (type == 0) {
        if (moverType28 != 0) NativeTrap("mover switch 0x5f46a0 to the null mover");
        return;
    }
    if (type != 1 || moverType28 != 0) NativeTrap("mover switch 0x5f46a0");
    if (mover34) bFree(carNullMoverPool86B304, mover34);
    VehicleMover *mover = static_cast<VehicleMover *>(calloc(1, sizeof(VehicleMover)));
    new (mover->integrator8) MoverIntegrator;
    mover34 = mover;
    moverType28 = type;
    mover->Construct(record20, reinterpret_cast<MoverCarInfo *>(valueC), input30, this);
    // Native vtable seam: install the slot interface car update 0x6012b0 dispatches through.
    static VehicleMoverSlots moverSlots;
    mover->vtable0 = *reinterpret_cast<void **>(&moverSlots);
    collisionBody3C->hasMover5C = 1;
    collisionBody3C->AttachMover();
}

bool CreateNativeVehicle(NativeVehicle *vehicle, const char *gameDirectory, const char *carName,
                         float startX, float startY, float startZ, float heading, char *error,
                         int errorSize) {
    memset(vehicle, 0, sizeof(*vehicle));
    int carIndex = 0;
    char carTypeName[32];
    if (!LoadCarTypeInfo(gameDirectory, carName, &vehicle->cameraSettings, &carIndex, carTypeName, error, errorSize)) {
        return false;
    }
    // The car constructor creates the CarCollisionBody (0x5aec20) from the car's collision
    // hull (CARS/<car>/GEOMETRY.BIN chunk 0x80034020).
    if (!LoadCarHulls(gameDirectory, carName, carTypeName, error, errorSize)) {
        return false;
    }
    // Native session seam: the frontend and race setup that fill the participant records
    // (0x89e810) are not recovered. One human record (kind 1, grid index 1) receives a stock
    // physics setup for the car and an explicit placement, which participant setup 0x6065b0
    // uses instead of the grid; the demo respawns the car at arbitrary positions.
    static MoverContext record;
    static CarPhysicsSetup setup;
    memset(&record, 0, sizeof(record));
    memset(&setup, 0, sizeof(setup));
    setup.parameters10.Construct();
    setup.value0 = carIndex;
    setup.parameters10.value0 = carIndex;
    record.setup10 = setup; // recovered copy 0x413440 binds the tuning to the record
    record.value4 = 1;
    record.gridIndex3 = 1;
    record.hasPlacement91C = 1;
    record.placement920.x = startX;
    record.placement920.y = startY;
    record.placement920.z = startZ;
    float turns = heading / 6.28318530718f;
    record.heading930 = (unsigned short)(int)floorf((turns - floorf(turns)) * 65536.0f + 0.5f);
    participantRecords89E810 = &record;
    participantCount89E814 = 1;
    // Seam for race-start pool creation 0x598f10: NullMoverSlotPool, one slot per participant.
    if (!carNullMoverPool86B304) carNullMoverPool86B304 = bNewSlotPool(0xc, participantCount89E814, "NullMoverSlotPool", 0);
    SetupParticipants();
    MoverModeOwner *owner = carWorld890080->cars1C[0];
#ifdef UG2_ENABLE_MODDING
    // Mod bridge scaffold: swap in the provider's collision body.
    short id = owner->collisionBody3C->id14;
    delete owner->collisionBody3C;
    owner->collisionBody3C = NativeModsCollisionBody(owner, id);
    owner->collisionBody3C->hasMover5C = 1;
    owner->collisionBody3C->AttachMover();
#endif
    vehicle->mover = owner->mover34;
    vehicle->owner = owner;
    vehicle->controls = static_cast<NativeControls *>(owner->input30);
    vehicle->info = owner->physicsInfo10;
    vehicle->carInfo = reinterpret_cast<MoverCarInfo *>(owner->valueC);
    snprintf(vehicle->carName, sizeof(vehicle->carName), "%s", carName);
    BindNativePlayer(vehicle);
#ifdef UG2_ENABLE_MODDING
    NativeModsVehicleCreated(vehicle);
    if (NativeModsFailed()) { snprintf(error, errorSize, "mod provider initialization failed"); return false; }
#endif
    return true;
}

void SetNativeControls(NativeVehicle *vehicle, float throttle, float brake, float handbrake, float steer) {
    NativeControls *controls = vehicle->controls;
    // SDL capture seam: writes the action fields the original input event
    // handlers write (pedals and the raw -1..1 steering axis at +0x2c8).
    // Recovered human update 0x41e2c0 turns them into pedals and the angle.
    controls->throttle2D4 = throttle;
    controls->brake2DC = brake;
    controls->handbrake2EC = handbrake;
    controls->steering2C8 = steer;
}

extern CarWorld *carWorld890080;

float StepNativeVehicle(NativeVehicle *vehicle, float duration) {
    // Game loop 0x5811c0 seam: the recovered world update 0x609ce0 (at 0x581407), then
    // the world clock 0x57a0e0 (at 0x5814ea). The rest of the loop body is not recovered.
#ifdef UG2_ENABLE_MODDING
    NativeModsBeforeStep(vehicle, duration);
    if (NativeModsFailed()) return 0.0f;
#endif
    carWorld890080->Update(duration);
#ifdef UG2_ENABLE_MODDING
    if (NativeModsFailed()) return 0.0f;
#endif
    frameDuration865154 = duration;
    AdvanceWorldClock();
#ifdef UG2_ENABLE_MODDING
    NativeModsAfterStep(vehicle, duration);
#endif
    float result = 0.0f;
    return result;
}

NativePlayerStorage nativePlayer;
NativeChaseCameraMover nativeChaseCamera;
extern HudPlayerSlot *hudPlayerSlots8900AC[];
extern PlayerRecord *players8900B4[2];
extern int playerCount8900BC;

NativeChaseCameraMover::NativeChaseCameraMover() {
    memset(unidentified4, 0, sizeof(unidentified4));
    kindC = 1;
    value10 = 0;
    active14 = 1;
}
void NativeChaseCameraMover::Virtual00() { NativeTrap("camera mover slot +0x00"); }
void NativeChaseCameraMover::Virtual04() { NativeTrap("camera mover slot +0x04"); }
void NativeChaseCameraMover::Virtual08() { NativeTrap("camera mover slot +0x08"); }
void NativeChaseCameraMover::Virtual0C() { NativeTrap("camera mover slot +0x0c"); }
void NativeChaseCameraMover::Virtual10() { NativeTrap("camera mover slot +0x10"); }
void NativeChaseCameraMover::Virtual14() { NativeTrap("camera mover slot +0x14"); }
void NativeChaseCameraMover::Virtual18() { NativeTrap("camera mover slot +0x18"); }
void NativeChaseCameraMover::Virtual1C() { NativeTrap("camera mover slot +0x1c"); }
// The native drive camera rebuilds itself from the profile option 0x83aa74 that 0x5f2fa0 just set.
void NativeChaseCameraMover::SetType20(int) {}
void NativeChaseCameraMover::Virtual24() { NativeTrap("camera mover slot +0x24"); }
// Original 0x404800, the kind-1 chase mover's slot +0x28.
bool NativeChaseCameraMover::Query28() { return false; }

void BindNativePlayer(NativeVehicle *vehicle) {
    nativePlayer.record.car4 = vehicle->owner;
    nativePlayer.record.camera238 = &nativeChaseCamera;
    hudPlayerSlots8900AC[0] = reinterpret_cast<HudPlayerSlot *>(&nativePlayer.record);
    players8900B4[0] = &nativePlayer.record;
    playerCount8900BC = 1;
}
