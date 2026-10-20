// Native adapter around recovered engine construction, bank loading and Ginsu setup.
// Participant production, controller wiring, file I/O and heap backend remain seams.
#include "NativeEngineSound.h"
#include "NativeAudio.h"
#include "NativeEngineBankIO.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundBanks.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include "../../Speed/Indep/Src/Physics/VehicleMover.h"
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
    : started_(false), log_(0) {
    for (int i = 0; i < 2; ++i) { gin_[i] = 0; data_[i] = 0; synth_[i] = 0; memblock_[i] = 0; }
}

NativeEngineSound::~NativeEngineSound() {
    Destroy();
}

bool NativeEngineSound::Load(const char *gameDirectory, MoverContext *context, const char *ginOverride,
                             char *error, int errorSize) {
    Destroy();
    EngineSoundCar car;
    car.Construct479660();
    // Manager ownership/index allocation is still a native session seam.
    car.kind14 = context->value4;
    car.Select4593B0(context);
    EngineSoundCarValue primary;
    if (ginOverride) {
        primary = *car.value80;
        primary.fileC = ginOverride;
        car.value80 = &primary;
        car.mode78 = 0;
    }
    const int slots = car.mode78 == 1 ? 2 : 1;
    const char *filenames[2] = {car.value80->fileC, car.value84->file0};
    for (int i = 0; i < slots; ++i) {
        // Native input guard; the recovered concatenation has no capacity check.
        if (strlen(engineBankDirectory7ED164) + strlen(filenames[i]) >= 64) {
            snprintf(error, errorSize, "engine sound filename is too long");
            return false;
        }
    }
    fprintf(stderr, "ug2demo: engine banks %s%s%s (mode %d%s)\n", filenames[0],
            slots == 2 ? ", " : "", slots == 2 ? filenames[1] : "", car.mode78,
            ginOverride ? ", CLI override" : ", recovered selection");
    Audio82B884 banks = {};
    EngineSound engine;
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

    if (const char *logPath = getenv("UG2_ENGINE_LOG")) {
        log_ = fopen(logPath, "w");
        if (log_ != 0) {
            fprintf(log_, "# audio_start_ticks %u\n", NativeAudioOpenTicks());
        }
    }
    return true;
}

void NativeEngineSound::Update(float rpm, int frame) {
    if (synth_[0] == 0) {
        return;
    }
    synth_[0]->UpdateFrequency(rpm, kEngineLatency);
    if (log_ != 0) {
        fprintf(log_, "%u %d %.2f\n", (unsigned int)SDL_GetTicks(), frame, rpm);
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
    if (log_ != 0) {
        fclose(log_);
        log_ = 0;
    }
}
