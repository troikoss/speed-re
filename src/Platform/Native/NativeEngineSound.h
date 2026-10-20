// Platform scaffolding: the engine sound of the native demo. It binds one retail SOUND/ENGINE/*.gin file, builds the
// recovered Ginsu synthesizer over it and feeds the engine speed to it every frame. Not recovered: the choice of
// the file per car (recovered 0x47d740 reads it from the EAXSound tables at +0xdc/+0xe4), integration of the
// recovered controllers 0x473e60/0x474030 (snd setter/backend and control-state seams), and selection of the car's mode
// (32-bit value at car +0x78; recovered setup implements its one/two-slot counts). See docs/pc-ginsu.md.
#ifndef UG2_NATIVE_ENGINE_SOUND_H
#define UG2_NATIVE_ENGINE_SOUND_H

#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

class NativeEngineSound {
public:
    NativeEngineSound();
    ~NativeEngineSound();
    // Reads <gameDirectory>/SOUND/ENGINE/<ginName>, calls recovered construction 0x473d30 and setup 0x47d740, and starts it at the
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
