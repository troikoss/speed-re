// Native adapter around recovered car selection, binding, bank loading and engine setup.
// Default records come from the participant car type and upgrade state; --engine-gin is an
// explicit one-bank override. Recovered playback controllers update both slots.
// Registry/list ownership and manager lifecycle remain seams.
#ifndef UG2_NATIVE_ENGINE_SOUND_H
#define UG2_NATIVE_ENGINE_SOUND_H

#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

struct MoverModeOwner;
struct NativeEnginePlayback;

class NativeEngineSound {
public:
    NativeEngineSound();
    ~NativeEngineSound();
    // Uses participant state through selection 0x4593b0, or the explicit filename override,
    // then construction, binding, bank loading and setup. Missing/invalid banks report an error.
    bool Load(const char *gameDirectory, MoverModeOwner *owner, const char *ginOverride, char *error, int errorSize);
    // Recovered physics, shifting, acceleration, spatial and auxiliary/control updates feed both banks.
    // Paused frames still update original producers, with their audio flag and zero step.
    void Update(float rpm, float duration, int frame, bool paused = false);
    void Destroy();
    bool Started() const { return started_; }

private:
    NativeEnginePlayback *playback_;
    unsigned char *gin_[2];
    GinsuSynthData *data_[2];
    GinsuSynthesis *synth_[2];
    void *memblock_[2];
    bool started_;
    FILE *log_;  // UG2_ENGINE_LOG: one line per frame, "ticks frame rpm"
};

#endif
