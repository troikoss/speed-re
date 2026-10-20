// Platform scaffolding for native engine playback. Recovered construction, bank loading and slot setup
// run over a temporary car record supplied from --engine-gin. The original car-record producer, mode/index
// selection and controller integration remain open; frames still feed RPM directly to Ginsu. Native file I/O,
// device output and cleanup remain backend seams. See docs/pc-ginsu.md.
#ifndef UG2_NATIVE_ENGINE_SOUND_H
#define UG2_NATIVE_ENGINE_SOUND_H

#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

class NativeEngineSound {
public:
    NativeEngineSound();
    ~NativeEngineSound();
    // Reads <gameDirectory>/SOUND/ENGINE/<ginName>, calls recovered construction 0x473d30, bank loading 0x465c80 and setup 0x47d740, and starts it at the
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
