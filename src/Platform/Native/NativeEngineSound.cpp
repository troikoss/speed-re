// Platform scaffolding for the engine sound (see NativeEngineSound.h). The recovered routines are the Ginsu ones;
// setup is recovered; file choice, object/table construction, controller and heap backend remain scaffold.
#include "NativeEngineSound.h"
#include "NativeAudio.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSound.h"

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <string>

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
    const std::string path = std::string(gameDirectory) + "/SOUND/ENGINE/" + ginName;
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        snprintf(error, errorSize, "engine sound %s not found", path.c_str());
        return false;
    }
    fseek(file, 0, SEEK_END);
    const long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length <= 0x20) {
        fclose(file);
        snprintf(error, errorSize, "engine sound %s is too short", path.c_str());
        return false;
    }
    gin_ = static_cast<unsigned char *>(malloc((size_t)length));
    const size_t got = fread(gin_, 1, (size_t)length, file);
    fclose(file);
    if (got != (size_t)length) {
        snprintf(error, errorSize, "engine sound %s could not be read", path.c_str());
        Destroy();
        return false;
    }

    if (memcmp(gin_, "Gnsu2", 5) != 0) {
        snprintf(error, errorSize, "engine sound %s is not a Gnsu2 data block", path.c_str());
        Destroy();
        return false;
    }

    // Scaffold until EAXSound's table loader and engine/car constructors are
    // recovered: one selected gin, bank index 0, mode 0, and a fresh slot.
    // Only setup reads these temporary views; synthesis retains the allocated
    // data and packet memory, not either view. Restore the session audio object.
    Audio82B884 banks = {};
    banks.engineBanksDC[0] = gin_;
    EngineSoundCar car = {};
    EngineSound engine = {};
    engine.car9C = &car;
    engine.slotsCC[0].startResult14 = -1;
    Audio82B884 *previous = audio82B884;
    audio82B884 = &banks;
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
