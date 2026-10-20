// Asset-free check that controller volume calls reach actual SDL packet output.
#include "NativeAudio.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSound.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static GinsuSndPacket packet;
static int player;

static void Release(void *, void *) { SNDPKTPLAY_submit(player, &packet); }

static bool WaitForOutput() {
    unsigned int first, current;
    SNDSYS_entercritical();
    NativeAudioStatistics(&first, 0, 0);
    SNDSYS_leavecritical();
    const Uint32 start = SDL_GetTicks();
    do {
        SDL_Delay(5);
        SNDSYS_entercritical();
        NativeAudioStatistics(&current, 0, 0);
        SNDSYS_leavecritical();
        if (current - first >= 6) return true;
    } while (SDL_GetTicks() - start < 2000);
    return false;
}

int main() {
    if (EngineSnd72412A(0, 127) != -10 || EngineSnd723D9B(0, 4096) != -10) return 1;
    char path[] = "/tmp/ug2-audio-playback-XXXXXX";
    const int fd = mkstemp(path);
    if (fd < 0) return 1;
    close(fd);
    SDL_setenv("SDL_AUDIODRIVER", "disk", 1);
    SDL_setenv("SDL_DISKAUDIOFILE", path, 1);
    char error[256];
    if (!NativeAudioOpen(true, error, sizeof(error))) {
        fprintf(stderr, "audio open: %s\n", error);
        unlink(path);
        return 1;
    }
    short samples[4096];
    for (int i = 0; i < 4096; ++i) samples[i] = 10000;
    char memory[512];
    GinsuSndFormat format = {};
    format.samplerate = 44100;
    format.channels = 1;
    GinsuSndAttr attributes;
    GinsuSndPlayOpts options;
    SND_attrsetdef(&attributes);
    SNDplaysetdef(&options);
    player = SNDPKTPLAY_create(Release, 0, 0, memory, sizeof(memory));
    if (player < 0 || SNDPKTPLAY_start(player, &format, &attributes, &options) < 0) {
        NativeAudioClose();
        unlink(path);
        return 1;
    }
    packet.numframes = 4096;
    packet.psamples[0] = samples;
    SNDSYS_entercritical();
    SNDPKTPLAY_submit(player, &packet);
    EngineSnd72412A(player, 127);
    EngineSnd723D9B(player, 4096);
    SNDSYS_leavecritical();
    bool output = WaitForOutput();
    SNDSYS_entercritical();
    EngineSnd72412A(player, 0);
    SNDSYS_leavecritical();
    output = WaitForOutput() && output;
    SNDSYS_entercritical();
    EngineSnd72412A(player, 64);
    SNDSYS_leavecritical();
    output = WaitForOutput() && output;
    SNDPKTPLAY_stop(player);
    SNDPKTPLAY_destroy(player);
    NativeAudioClose();
    FILE *file = fopen(path, "rb");
    int silent = 0, full = 0, partial = 0, bad = 0;
    short pair[2];
    if (!file) { unlink(path); return 1; }
    while (fread(pair, sizeof(pair), 1, file) == 1) {
        if (pair[0] != pair[1]) ++bad;
        if (pair[0] == 0) ++silent;
        else if (pair[0] > 4500 && pair[0] < 5500) ++full;
        else if (pair[0] > 2000 && pair[0] < 3000) ++partial;
        else ++bad;
    }
    fclose(file);
    unlink(path);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    if (!output || bad || silent < 1024 || full < 1024 || partial < 1024) {
        fprintf(stderr, "unexpected output: silent=%d full=%d partial=%d bad=%d\n", silent, full, partial, bad);
        return 1;
    }
    puts("audio playback: controller volume reaches SDL output at full, zero and partial levels");
    return 0;
}
