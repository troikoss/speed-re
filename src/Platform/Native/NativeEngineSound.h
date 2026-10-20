// Native adapter around recovered car selection, binding, bank loading and engine setup.
// Default records come from the participant car type and upgrade state; --engine-gin is an
// explicit one-bank override. Per-frame controller integration and cleanup remain seams.
// Both original setup slots are retained, but only slot 0 receives the direct-RPM scaffold.
#ifndef UG2_NATIVE_ENGINE_SOUND_H
#define UG2_NATIVE_ENGINE_SOUND_H

#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

struct MoverContext;

class NativeEngineSound {
public:
    NativeEngineSound();
    ~NativeEngineSound();
    // Uses participant state through selection 0x4593b0, or the explicit filename override,
    // then construction, binding, bank loading and setup. Missing/invalid banks report an error.
    bool Load(const char *gameDirectory, MoverContext *context, const char *ginOverride, char *error, int errorSize);
    // One frame of the engine controller: UpdateFrequency(rpm, 60.0f). 60.0 is the latency pushed by 0x473e60.
    void Update(float rpm, int frame);
    void Destroy();
    bool Started() const { return started_; }

private:
    unsigned char *gin_[2];
    GinsuSynthData *data_[2];
    GinsuSynthesis *synth_[2];
    void *memblock_[2];
    bool started_;
    FILE *log_;  // UG2_ENGINE_LOG: one line per frame, "ticks frame rpm"
};

#endif
