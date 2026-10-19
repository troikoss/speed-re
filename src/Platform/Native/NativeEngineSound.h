// Platform scaffolding: the engine sound of the native demo. It binds one retail SOUND/ENGINE/*.gin file, builds the
// recovered Ginsu synthesizer over it and feeds the engine speed to it every frame. Not recovered: the choice of
// the file per car (0x47d740 reads it from the EAXSound tables at +0xdc/+0xe4), the engine controller 0x473e60
// (volume, pan and pitch through the snd calls at 0x72412a and others) and the number of synthesizers per car
// (mode byte at car +0x78). See docs/pc-ginsu.md.
#ifndef UG2_NATIVE_ENGINE_SOUND_H
#define UG2_NATIVE_ENGINE_SOUND_H

#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

class NativeEngineSound {
public:
    NativeEngineSound();
    ~NativeEngineSound();
    // Reads <gameDirectory>/SOUND/ENGINE/<ginName>, sets up the synthesizer as 0x47d740 does and starts it at the
    // gin's minimum frequency. Returns false with a message in error when the file is missing or invalid.
    bool Load(const char *gameDirectory, const char *ginName, char *error, int errorSize);
    // One frame of the engine controller: UpdateFrequency(rpm, 60.0f). 60.0 is the latency pushed by 0x473e60.
    void Update(float rpm, int frame);
    void Destroy();
    bool Started() const { return started_; }

private:
    unsigned char *gin_;
    GinsuSynthData *data_;
    GinsuSynthesis *synth_;
    void *memblock_;
    bool started_;
    FILE *log_;  // UG2_ENGINE_LOG: one line per frame, "ticks frame rpm"
};

#endif
