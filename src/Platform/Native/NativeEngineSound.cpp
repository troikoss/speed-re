// Platform scaffolding for the engine sound (see NativeEngineSound.h). The recovered routines are the Ginsu ones;
// the file choice, the controller and the memory are scaffold, with the retail evidence cited below.
#include "NativeEngineSound.h"
#include "NativeAudio.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <string>

// Retail constant of the engine setup 0x47d740: the block rate 48000.0 at 0x78a5b0, mapped in OriginalSymbols.ld.
extern float ginsuEngineBlockRate78A5B0;

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

    data_ = new GinsuSynthData();
    if (!data_->BindToData(gin_)) {
        snprintf(error, errorSize, "engine sound %s is not a Gnsu2 data block", path.c_str());
        Destroy();
        return false;
    }

    // 0x47d740: size = overhead(8) + 4 + 4 * ceil((0x788898 + 0x788894) * 0x78a5b0). For the 48 kHz block rate this is
    // two 552-sample packet buffers plus the player (2624 bytes); the synthesizer reads n = (size - overhead) >> 3.
    const int overhead = SNDPKTPLAY_overhead(8);
    volatile float blockSamples = (ginsuRateScaleC788898 + ginsuRateScaleB788894) * ginsuEngineBlockRate78A5B0;
    const int size = overhead + 4 + 4 * (int)ceilf(blockSamples);
    memblock_ = calloc(1, (size_t)size);
    synth_ = new GinsuSynthesis(memblock_, size);
    if (!synth_->SetSynthData(*data_)) {
        snprintf(error, errorSize, "engine sound %s needs a larger packet block", path.c_str());
        Destroy();
        return false;
    }
    // 0x47d8f9: StartSynthesis is called with the minimum frequency of the data block.
    started_ = synth_->StartSynthesis(data_->minFrequency4) >= 0;

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
    delete synth_;
    synth_ = 0;
    started_ = false;
    free(memblock_);
    memblock_ = 0;
    delete data_;
    data_ = 0;
    free(gin_);
    gin_ = 0;
    if (log_ != 0) {
        fclose(log_);
        log_ = 0;
    }
}
