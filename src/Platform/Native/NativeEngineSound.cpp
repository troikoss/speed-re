// Native adapter around recovered engine construction, bank loading and Ginsu setup.
// Component scheduling/lifecycle, shift/effect inputs, file I/O and heap remain seams.
#include "NativeEngineSound.h"
#include "NativeAudio.h"
#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include "../../Speed/Indep/Src/Physics/VehicleMover.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundPhysics.h"

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

extern Audio82B884 *audio82B884;

struct NativeEnginePlayback {
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
// Spatial rendering remains centered. Activation's outputs are retained locally;
// the missing spatial component does not supply attenuation/pan to playback.
void __fastcall SpatialPosition(EngineSoundSpatialComponent *, void *, float *position) {
    position[0] = position[1] = position[2] = 0.0f;
}
void __fastcall SpatialDirection(EngineSoundSpatialComponent *, void *, void *) {}

void InitialiseControlSession(NativeEnginePlayback *session) {
    EngineSoundCar &car = session->car;
    EngineSoundControl &control = session->control;
    control.Construct45CE20();
    control.Bind45CF90(&car);
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
    car.car30 = owner;
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
    Audio82B884 banks = {};
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
    playback_->auxiliaryVtable.value34 = AuxiliaryFrequency;
    playback_->auxiliaryVtable.value38 = AuxiliaryThrottle;
    auxiliary.vtable0 = &playback_->auxiliaryVtable;
    auxiliary.average90.vtable0 = auxiliary.averageB8.vtable0 = &averageVtable;
    auxiliary.Bind45C550(&car);
    // Neutral shift/accel components are explicit fixtures until their producers
    // are recovered. No guessed shift or accel transition is synthesized.
    auxiliary.shift6C = &playback_->shift;
    auxiliary.accel70 = &playback_->accel;
    auxiliary.physics74 = &physics;
    playback_->spatialVtable.value2C = SpatialPosition;
    playback_->spatialVtable.value30 = SpatialDirection;
    playback_->spatial.vtable0 = &playback_->spatialVtable;
    auxiliary.spatial78 = &playback_->spatial;
    auxiliary.Activate45C5A0();
    // Full auxiliary frame 0x47c010 remains open: retain neutral offset/ramp
    // state and a full base level in place of its random/effect producers.
    auxiliary.valueE0 = 32767;
    InitialiseControlSession(playback_);
    engine.auxiliaryB8 = &auxiliary;
    engine.controlC0 = &playback_->control;
    engine.activeC8 = slots == 1 ? engine.slotsCC[0].startResult14 >= 0 :
        engine.slotsCC[0].startResult14 >= 0 && engine.slotsCC[1].startResult14 >= 0;
    started_ = engine.activeC8 != 0;

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
    playback_->car.time34 += duration;
    playback_->physics.Update484C50(duration);
    playback_->auxiliary.ProduceRpm46F7D0(duration);
    playback_->auxiliary.ProduceThrottle45C710(duration);
    playback_->control.Update47C3D0(duration);
    playback_->engine.Update474310();
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
    }
    delete playback_;
    playback_ = 0;
    if (log_ != 0) {
        fclose(log_);
        log_ = 0;
    }
}
