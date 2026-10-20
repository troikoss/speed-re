// Native adapter around recovered engine construction, bank loading and Ginsu setup.
// Registry/list ownership, file I/O and heap remain seams.
// Recovered sort, component scheduling and post passes execute through ABI bridges.
#include "NativeEngineSound.h"
#include "NativeAudio.h"
#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include "../../Speed/Indep/Src/Physics/VehicleMover.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundPhysics.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundSpatial.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundSchedule.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundAccel.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundShift.h"

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

extern Audio82B884 *audio82B884;

struct NativeEnginePlayback {
    Audio82B884 audio;
    AudioCarSoundManager listenerManager;
    EngineSoundCar car;
    EngineSoundCarValue overrideRecord;
    EngineSound engine;
    EngineSoundControl control;
    EngineSoundVtable vtable;
    unsigned int level;
    EngineSoundPhysicsView physics;
    EngineSoundAuxiliary auxiliary;
    EngineSoundShiftView shift;
    EngineSoundAccelView accel;
    EngineSoundCarVtable carVtable;
    EngineSoundAuxiliaryVtable auxiliaryVtable;
    EngineSoundSpatialComponent spatial;
    EngineSoundSpatialVtable spatialVtable;
    EngineSoundScheduleVtable physicsVtable, controlVtable, accelVtable, shiftVtable;
};

// Notification ownership is still absent. Normal native playback explicitly
// bypasses this branch with physics.flag6C; unexpected entry must stay visible.
void NativeTrap(const char *what);
unsigned char __fastcall EngineSoundBlown5ECE00(MoverModeOwner *) { NativeTrap("audio blown-engine query"); return 0; }
int EngineSoundNotificationView::Query46C9A0() { NativeTrap("audio notification query"); return 0; }
unsigned char EngineSoundNotificationView::Ready46C790() { NativeTrap("audio notification ready"); return 0; }
void EngineSoundNotificationView::Play484510(int, unsigned int, int, void (__cdecl *)(), int) { NativeTrap("audio notification play"); }
void EngineSoundNotificationView::Clear45AD40(int) { NativeTrap("audio notification clear"); }
void EngineSoundDiagnostic4691C0(int, const char *) { NativeTrap("audio component attachment"); }

namespace {

struct NativeAverageVtable {
    void *unidentified0;
    void (__fastcall *recalculate4)(SampleAverage *);
};
void __fastcall RecalculateAverage(SampleAverage *average) { average->RecalculateSamples(); }
NativeAverageVtable averageVtable = {0, RecalculateAverage};
int __fastcall CarValue(EngineSoundCar *car) { return car->Value479700(); }
float __fastcall AuxiliaryFrequency(EngineSoundAuxiliary *auxiliary) { return auxiliary->Value45C530(); }
float __fastcall AuxiliaryThrottle(EngineSoundAuxiliary *auxiliary) { return auxiliary->Value45C540(); }
void __fastcall AuxiliaryRpm(EngineSoundAuxiliary *value, void *, float duration) { value->ProduceRpm46F7D0(duration); }
void __fastcall AuxiliaryThrottleUpdate(EngineSoundAuxiliary *value, void *, float duration) { value->ProduceThrottle45C710(duration); }
void __fastcall AuxiliaryLevel(EngineSoundAuxiliary *value, void *, float duration) { value->ProduceLevel45C6C0(duration); }
void __fastcall AuxiliaryEffects(EngineSoundAuxiliary *value, void *, float duration) { value->ProduceEffects46F610(duration); }
void __fastcall AuxiliaryOffsets(EngineSoundAuxiliary *value, void *, float duration) { value->ProduceOffsets45C7A0(duration); }
void __fastcall SpatialSelect(EngineSoundSpatialComponent *value, void *, int listener) {
    value->Select45E100(listener);
}
void __fastcall PhysicsUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundPhysicsView *>(value)->Update484C50(duration);
}
float __fastcall PhysicsThrottle(EngineSoundPhysicsView *value) { return value->Value45D300(); }
void __fastcall ShiftUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundShiftView *>(value)->Update483130(duration);
}
void __fastcall ShiftPost(EngineSoundScheduledNode *value) { reinterpret_cast<EngineSoundShiftView *>(value)->Post45C210(); }
void __fastcall PhysicsPost(EngineSoundScheduledNode *value) { reinterpret_cast<EngineSoundPhysicsView *>(value)->Post470A60(); }
void __fastcall AuxiliaryUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundAuxiliary *>(value)->Update47C010(duration);
}
void __fastcall ControlUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundControl *>(value)->Update47C3D0(duration);
}
void __fastcall ControlPost(EngineSoundScheduledNode *value) { reinterpret_cast<EngineSoundControl *>(value)->Post470940(); }
void __fastcall AccelUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundAccelView *>(value)->Update4831A0(duration);
}
void __fastcall SpatialUpdate(EngineSoundScheduledNode *value, void *, float duration) {
    reinterpret_cast<EngineSoundSpatialComponent *>(value)->Update471DC0(duration);
}
void __fastcall EmptyPost(EngineSoundScheduledNode *value) { EngineSoundNoop4022C0(value); }
void __fastcall CarPre(EngineSoundCar *value) { EngineSoundNoop4022C0(value); }
void __fastcall EngineTick(EngineSound *value, void *, float duration) { EngineSoundNoop4031C0(value, 0, duration); }
void __fastcall EngineNoop(EngineSound *value) { EngineSoundNoop4022C0(value); }
void __fastcall EngineForward(EngineSound *value) { value->Forward460FF0(); }
void __fastcall EngineFrame(EngineSound *value) { value->Update474310(); }

void InitialiseSchedule(NativeEnginePlayback *session) {
    EngineSoundScheduledNode *inputs[] = {
        reinterpret_cast<EngineSoundScheduledNode *>(&session->physics),
        reinterpret_cast<EngineSoundScheduledNode *>(&session->spatial),
        reinterpret_cast<EngineSoundScheduledNode *>(&session->auxiliary),
        reinterpret_cast<EngineSoundScheduledNode *>(&session->control),
        reinterpret_cast<EngineSoundScheduledNode *>(&session->accel),
        reinterpret_cast<EngineSoundScheduledNode *>(&session->shift)};
    const unsigned int ids[] = {0, 8, 5, 6, 4, 3};
    // Registry/factory ownership remains native. 0x476890 supplies this key
    // encoding; 0x464a10 sorts the original low-seven-bit component IDs.
    for (int i = 0; i < 6; ++i) {
        inputs[i]->key4C = ((((unsigned int)session->car.kind14 << 5) |
                             (unsigned int)session->car.bankIndex10) << 7 | ids[i]) << 4;
        inputs[i]->next50 = i == 5 ? 0 : inputs[i+1];
    }
    session->car.first1C = inputs[0];
    session->car.count2C = 6;
    session->car.first20 = reinterpret_cast<EngineSoundScheduledNode *>(&session->engine);
    session->car.Sort464A10();
    session->physicsVtable.update14 = PhysicsUpdate;
    session->physicsVtable.post18 = PhysicsPost;
    session->physicsVtable.update24 = reinterpret_cast<void (__fastcall *)(EngineSoundScheduledNode *, void *, float)>(PhysicsThrottle);
    inputs[0]->vtable0 = &session->physicsVtable;
    session->controlVtable.update14 = ControlUpdate;
    session->controlVtable.post18 = ControlPost;
    inputs[3]->vtable0 = &session->controlVtable;
    session->accelVtable.update14 = AccelUpdate;
    session->accelVtable.post18 = EmptyPost;
    inputs[4]->vtable0 = &session->accelVtable;
    session->shiftVtable.update14 = ShiftUpdate;
    session->shiftVtable.post18 = ShiftPost;
    inputs[5]->vtable0 = &session->shiftVtable;
    inputs[1]->vtable0->update14 = SpatialUpdate;
    inputs[1]->vtable0->post18 = EmptyPost;
    inputs[2]->vtable0->update14 = AuxiliaryUpdate;
    inputs[2]->vtable0->post18 = EmptyPost;
    session->carVtable.pre34 = CarPre;
    session->vtable.value24 = EngineTick;
    session->vtable.value28 = EngineForward;
    session->vtable.value34 = EngineNoop;
    session->vtable.value48 = EngineFrame;
}
// The original callbacks retain pointers into auxiliary/vehicle storage.
void __fastcall SpatialPosition(EngineSoundSpatialComponent *value, void *, float *position) {
    value->SetPosition45E1D0(position);
}
void __fastcall SpatialDirection(EngineSoundSpatialComponent *value, void *, void *orientation) {
    value->SetOrientation45E1E0(orientation);
}

void InitialiseControlSession(NativeEnginePlayback *session) {
    EngineSoundCar &car = session->car;
    EngineSoundControl &control = session->control;
    control.Construct45CE20();
    control.Bind45CF90(&car);
    control.Activate45D050();
    // Registry attachment/scheduling remain a native session seam.
    control.engine6C = &session->auxiliary;
    control.value70 = &session->shift;
    control.value74 = &session->accel;
    control.averageC4.vtable0 = &averageVtable;
}

int __fastcall PlaybackGain(EngineSound *engine, void *, int index) {
    return engine->Value45E390(index);
}

int __fastcall PlaybackPan(EngineSound *engine, void *, int index) {
    return engine->Value45E3B0(index);
}

}  // namespace

NativeEngineSound::NativeEngineSound()
    : playback_(0), started_(false), log_(0) {
    for (int i = 0; i < 2; ++i) { gin_[i] = 0; data_[i] = 0; synth_[i] = 0; memblock_[i] = 0; }
}

NativeEngineSound::~NativeEngineSound() {
    Destroy();
}

bool NativeEngineSound::Load(const char *gameDirectory, MoverModeOwner *owner, const char *ginOverride,
                             char *error, int errorSize) {
    Destroy();
    playback_ = new NativeEnginePlayback();
    EngineSoundCar &car = playback_->car;
    car.Construct479660();
    // Manager ownership/index allocation is still a native session seam.
    car.kind14 = owner->context14->value4;
    car.Bind464BA0(owner);
    car.Select4593B0(owner->context14);
    if (ginOverride) {
        playback_->overrideRecord = *car.value80;
        playback_->overrideRecord.fileC = ginOverride;
        car.value80 = &playback_->overrideRecord;
        car.mode78 = 0;
    }
    const int slots = car.mode78 == 1 ? 2 : 1;
    const char *filenames[2] = {car.value80->fileC, car.value84->file0};
    for (int i = 0; i < slots; ++i) {
        // Native input guard; the recovered concatenation has no capacity check.
        if (strlen(engineBankDirectory7ED164) + strlen(filenames[i]) >= 64) {
            snprintf(error, errorSize, "engine sound filename is too long");
            Destroy();
            return false;
        }
    }
    fprintf(stderr, "ug2demo: engine banks %s%s%s (mode %d%s)\n", filenames[0],
            slots == 2 ? ", " : "", slots == 2 ? filenames[1] : "", car.mode78,
            ginOverride ? ", CLI override" : ", recovered selection");
    Audio82B884 &banks = playback_->audio;
    banks.Construct4585A0();
    EngineSound &engine = playback_->engine;
    engine.Construct473D30();
    engine.Bind463500(&car);
    NativeEngineBankIOBegin(gameDirectory);
    Audio82B884 *previous = audio82B884;
    audio82B884 = &banks;
    int count = 0;
    engine.LoadBanks465C80(&count);
    int lengths[2] = {};
    const bool loaded = NativeEngineBankIODrain(error, errorSize, &lengths[0], &lengths[1]);
    gin_[0] = static_cast<unsigned char *>(banks.engineBanksDC[0]);
    gin_[1] = static_cast<unsigned char *>(banks.engineBanksE4[0]);
    bool valid = loaded;
    for (int i = 0; i < slots; ++i) {
        if (!gin_[i] || lengths[i] <= 0x20 || memcmp(gin_[i], "Gnsu2", 5) != 0) valid = false;
    }
    if (!valid) {
        audio82B884 = previous;
        if (loaded) snprintf(error, errorSize, "engine sound bank is missing, too short or not Gnsu2");
        Destroy();
        return false;
    }
    engine.Setup47D740();
    audio82B884 = previous;
    for (int i = 0; i < slots; ++i) {
        data_[i] = engine.slotsCC[i].data0;
        synth_[i] = engine.slotsCC[i].synth4;
        memblock_[i] = engine.slotsCC[i].memory8;
        if (engine.slotsCC[i].startResult14 >= 0) started_ = true;
    }
    // Native session ownership and vtable ABI bridges into recovered routines.
    playback_->level = 32767;
    *reinterpret_cast<unsigned int **>(reinterpret_cast<char *>(&engine) + 0x58) = &playback_->level;
    playback_->vtable.value38 = PlaybackGain;
    playback_->vtable.value3C = PlaybackPan;
    engine.vtable0 = &playback_->vtable;
    playback_->carVtable.value44 = CarValue;
    car.vtable0 = &playback_->carVtable;
    EngineSoundPhysicsView &physics = playback_->physics;
    physics.Construct45D210();
    physics.Bind45D4F0(&car);
    physics.Activate45D370();
    physics.average94.vtable0 = &averageVtable;
    // Notification subsystem remains a seam; do not reach its unavailable owner.
    physics.flag6C = 1;
    EngineSoundAuxiliary &auxiliary = playback_->auxiliary;
    auxiliary.Construct46F1B0();
    playback_->auxiliaryVtable.value24 = AuxiliaryRpm;
    playback_->auxiliaryVtable.value28 = AuxiliaryThrottleUpdate;
    playback_->auxiliaryVtable.value2C = AuxiliaryLevel;
    playback_->auxiliaryVtable.value30 = AuxiliaryEffects;
    playback_->auxiliaryVtable.value3C = AuxiliaryOffsets;
    playback_->auxiliaryVtable.value34 = AuxiliaryFrequency;
    playback_->auxiliaryVtable.value38 = AuxiliaryThrottle;
    auxiliary.vtable0 = &playback_->auxiliaryVtable;
    auxiliary.average90.vtable0 = auxiliary.averageB8.vtable0 = &averageVtable;
    auxiliary.Bind45C550(&car);
    playback_->shift.Construct45C0D0();
    playback_->shift.Bind45C330(&car);
    playback_->shift.auxiliary6C = &auxiliary;
    playback_->shift.Activate45C3D0();
    auxiliary.shift6C = &playback_->shift;
    auxiliary.accel70 = &playback_->accel;
    auxiliary.physics74 = &physics;
    playback_->accel.Construct45C9C0();
    playback_->accel.Bind45CA80(&car);
    playback_->accel.auxiliary6C = &auxiliary;
    playback_->accel.shift70 = &playback_->shift;
    playback_->accel.Activate45CB10();
    playback_->spatial.Construct471D60();
    playback_->spatial.Bind45DAC0(&car);
    playback_->spatial.Activate45BAD0();
    playback_->spatial.averageA8.vtable0 = &averageVtable;
    playback_->spatialVtable.value24 = SpatialSelect;
    playback_->spatialVtable.value2C = SpatialPosition;
    playback_->spatialVtable.value30 = SpatialDirection;
    playback_->spatial.vtable0 = &playback_->spatialVtable;
    auxiliary.spatial78 = &playback_->spatial;
    auxiliary.Activate45C5A0();
    // Manager allocation/registry remain a native ownership seam. Publish the
    // constructed car as the single list entry read by original listener code.
    playback_->listenerManager.first10 = reinterpret_cast<AudioManagedCarSound *>(&car);
    playback_->audio.managerAC = &playback_->listenerManager;
    InitialiseControlSession(playback_);
    engine.auxiliaryB8 = &auxiliary;
    engine.controlC0 = &playback_->control;
    engine.activeC8 = slots == 1 ? engine.slotsCC[0].startResult14 >= 0 :
        engine.slotsCC[0].startResult14 >= 0 && engine.slotsCC[1].startResult14 >= 0;
    started_ = engine.activeC8 != 0;
    InitialiseSchedule(playback_);

    if (const char *logPath = getenv("UG2_ENGINE_LOG")) {
        log_ = fopen(logPath, "w");
        if (log_ != 0) {
            fprintf(log_, "# audio_start_ticks %u\n", NativeAudioOpenTicks());
        }
    }
    return true;
}

void NativeEngineSound::Update(float rpm, float duration, int frame) {
    if (synth_[0] == 0) {
        return;
    }
    // ExportState 0x5a5540 already publishes the filtered engine value at
    // state +0x400 / owner +0x440. Consume that original field directly;
    // the raw RPM argument is used only by the diagnostic log.
    Audio82B884 *previous = audio82B884;
    audio82B884 = &playback_->audio;
    // Original audio frame 0x483b40 produces listeners before manager updates.
    // The native player points at the same finalized Camera used for rendering.
    EngineSoundUpdateListeners46D7E0();
    playback_->car.Update469CF0(duration);
    playback_->car.Post469CB0();
    audio82B884 = previous;
    if (log_ != 0) {
        fprintf(log_, "%u %d %.2f\n", (unsigned int)SDL_GetTicks(), frame, rpm);
        fprintf(log_, "# playback %d %d %.2f %d %d %d %d\n", frame,
                playback_->car.mode78, playback_->engine.frequency104,
                playback_->engine.volume108, playback_->engine.volume10C,
                synth_[0]->targetPos34, synth_[1] ? synth_[1]->targetPos34 : 0);
        fprintf(log_, "# control %d %.2f %.2f %.2f %.2f %d %d %d %.2f\n", frame,
                playback_->car.value48, playback_->auxiliary.value128,
                playback_->auxiliary.value130, playback_->control.frequencyF8,
                playback_->control.value7C, playback_->control.value80,
                playback_->control.value84, playback_->control.valueBC);
        fprintf(log_, "# auxiliary %d %d %d %d %.4f %.4f %.2f %d %d\n", frame,
                playback_->auxiliary.flag13C, playback_->auxiliary.flagE4,
                playback_->auxiliary.valueE0, playback_->auxiliary.value118,
                playback_->auxiliary.valueFC, playback_->auxiliary.value124,
                playback_->auxiliary.sequence154.index70, playback_->auxiliary.sequence154.flag78);
        fprintf(log_, "# spatial %d %d %d %d %.6f\n", frame,
                playback_->spatial.listenerD0, playback_->spatial.value4,
                playback_->spatial.value8, playback_->spatial.averageA8.averageC);
        fprintf(log_, "# acceleration %d %d %.2f %.2f %.3f %u\n", frame,
                playback_->accel.valueC8, playback_->accel.value88,
                playback_->accel.valueC0, playback_->accel.valueA4,
                (unsigned int)playback_->accel.flagD8);
        fprintf(log_, "# shifting %d %d %d %.2f %.2f %u %u %u\n", frame,
                playback_->shift.value78, playback_->shift.value7C, playback_->shift.valueDC,
                playback_->shift.valueC0, (unsigned int)playback_->shift.flag70,
                (unsigned int)playback_->shift.flag71, (unsigned int)playback_->shift.flag72);
        fflush(log_);
    }
}

void NativeEngineSound::Destroy() {
    // Each synthesizer stops and destroys its player while the device is still open.
    for (int i = 0; i < 2; ++i) {
        if (synth_[i]) synth_[i]->~GinsuSynthesis();
        free(synth_[i]); synth_[i] = 0;
        free(memblock_[i]); memblock_[i] = 0;
        if (data_[i]) data_[i]->~GinsuSynthData();
        free(data_[i]); data_[i] = 0;
        free(gin_[i]); gin_[i] = 0;
    }
    started_ = false;
    if (playback_) {
        if (playback_->physics.average94.samples10) playback_->physics.average94.Destroy();
        if (playback_->auxiliary.average90.samples10) playback_->auxiliary.average90.Destroy();
        if (playback_->auxiliary.averageB8.samples10) playback_->auxiliary.averageB8.Destroy();
        if (playback_->control.averageC4.samples10) playback_->control.averageC4.Destroy();
        if (playback_->spatial.averageA8.samples10) playback_->spatial.averageA8.Destroy();
    }
    delete playback_;
    playback_ = 0;
    if (log_ != 0) {
        fclose(log_);
        log_ = 0;
    }
}
