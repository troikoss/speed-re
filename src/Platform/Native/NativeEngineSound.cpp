// Native adapter around recovered engine construction, bank loading and Ginsu setup.
// Car-record production, controller wiring, file I/O and heap backend remain seams.
#include "NativeEngineSound.h"
#include "NativeAudio.h"
#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSound.h"

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

extern Audio82B884 *audio82B884;

namespace {

// 0x473fc7 pushes 0x42700000 (60.0f) as the latency of UpdateFrequency in the engine controller 0x473e60.
const float kEngineLatency = 60.0f;

}  // namespace

NativeEngineSound::NativeEngineSound()
    : gin_(0), data_(0), synth_(0), memblock_(0), started_(false), log_(0) {}

NativeEngineSound::~NativeEngineSound() {
    Destroy();
}

bool NativeEngineSound::Load(const char *gameDirectory, const char *ginName, char *error, int errorSize) {
    Destroy();
    // CLI filename and mode/index 0 still stand in for the original car-state
    // record producer. Path construction, bank allocation and queued reads now
    // run through 0x465c80, with native file I/O drained before slot setup.
    if (strlen(engineBankDirectory7ED164) + strlen(ginName) >= 64) {
        snprintf(error, errorSize, "engine sound filename is too long");
        return false;
    }
    Audio82B884 banks = {};
    EngineSoundCarValue primary = {};
    primary.fileC = ginName;
    EngineSoundCar car = {};
    car.value80 = &primary;
    EngineSound engine;
    engine.Construct473D30();
    engine.car9C = &car;
    NativeEngineBankIOBegin(gameDirectory);
    Audio82B884 *previous = audio82B884;
    audio82B884 = &banks;
    int count = 0;
    engine.LoadBanks465C80(&count);
    int length = 0;
    const bool loaded = NativeEngineBankIODrain(error, errorSize, &length);
    gin_ = static_cast<unsigned char *>(banks.engineBanksDC[0]);
    if (!loaded || !gin_ || length <= 0x20 || memcmp(gin_, "Gnsu2", 5) != 0) {
        audio82B884 = previous;
        if (loaded) snprintf(error, errorSize, "engine sound %s is missing, too short or not Gnsu2", ginName);
        Destroy();
        return false;
    }
    engine.Setup47D740();
    audio82B884 = previous;
    data_ = engine.slotsCC[0].data0;
    synth_ = engine.slotsCC[0].synth4;
    memblock_ = engine.slotsCC[0].memory8;
    started_ = engine.slotsCC[0].startResult14 >= 0;

    if (const char *logPath = getenv("UG2_ENGINE_LOG")) {
        log_ = fopen(logPath, "w");
        if (log_ != 0) {
            fprintf(log_, "# audio_start_ticks %u\n", NativeAudioOpenTicks());
        }
    }
    return true;
}

void NativeEngineSound::Update(float rpm, int frame) {
    if (synth_ == 0) {
        return;
    }
    synth_->UpdateFrequency(rpm, kEngineLatency);
    if (log_ != 0) {
        fprintf(log_, "%u %d %.2f\n", (unsigned int)SDL_GetTicks(), frame, rpm);
        fflush(log_);
    }
}

void NativeEngineSound::Destroy() {
    // The synthesizer destructor stops and destroys its player, so the audio device must still be open.
    if (synth_) synth_->~GinsuSynthesis();
    free(synth_);
    synth_ = 0;
    started_ = false;
    free(memblock_);
    memblock_ = 0;
    if (data_) data_->~GinsuSynthData();
    free(data_);
    data_ = 0;
    free(gin_);
    gin_ = 0;
    if (log_ != 0) {
        fclose(log_);
        log_ = 0;
    }
}
