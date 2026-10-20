// Platform scaffolding: the snd library that the recovered Ginsu synthesizer calls (SNDSYS_*, SNDPKTPLAY_*,
// SND_attrsetdef, SNDplaysetdef; retail 0x721f99..0x72423e). Its packet player remains native; parameter
// dispatch and small allocator/mutex wrappers are recovered separately. This SDL2 backend models:
//   - create() returns a player handle (a free slot) and keeps the release callback and its client data. It returns
//     -9 when every slot is taken, -6 when the memory is too small for two packets, and -10 without an audio device
//     (the retail library reports -10 when it is not initialised).
//   - start() resets the queue, takes the sample rate of a mono format and returns the snd handle (the slot index).
//   - submit() queues one packet: frames in the low 31 bits of the word at +4 (bit 31 is the continuation flag, not
//     used here) and the sample pointer psamples[0] at +0xC. It returns -13 when the queue is full.
//   - the player consumes packets at their own rate with linear resampling to the device rate. When a packet has
//     been consumed, the release callback is called with its samples and the client data. The callback runs on the
//     audio thread with the device lock held (the Ginsu release submits the same buffer again).
//   - stop() stops the player and drops the queued packets without release callbacks (purge is not implemented).
//   - destroy() frees the slot.
// Engine controller setters use native adapters below: volume and pitch reach
// the mixer; other values are retained without a claimed DSP implementation.
// Not modelled: retail voices/render modes, pan vectors, filters, effect sends,
// channels other than mono, and the frame-count callback. SND_attrsetdef and SNDplaysetdef write the retail bytes (0x72423e and
// 0x721a6b); the padding words and tsdatasize, which the retail does not write, are left alone.
//
// SNDSYS_entercritical and SNDSYS_leavecritical are the device lock. SDL's device lock is a recursive mutex, so the
// release callback can take it again on the audio thread (tools/smoke_native_engine_audio.py exercises this path).
#include "NativeAudio.h"
#include "../../Speed/Indep/Src/EAXSound/Ginsu/Ginsu.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

namespace {

enum {
    kChannels = 8,             // player slots (not recovered)
    kQueueCapacity = 32,       // packet entries per player (the retail queue is (memsize - 0x9c) / 0x20 long)
    kDeviceRate = 44100,
    kDeviceChannels = 2,
    kDeviceFrames = 512,       // requested callback size
    kMixBatch = 16384,         // frames mixed per pass; larger callbacks are split
    kReleasesPerPass = 256,    // bound on packets consumed per channel in one pass (zero-length packets cannot spin)
};

// Scaffold gain applied to each player before the sum. The retail mix is not modelled.
const float kChannelGain = 0.5f;

struct Packet {
    short *samples;
    unsigned int frames;
    unsigned int continuation;
    int handle;
};

struct Channel {
    bool allocated;
    bool playing;
    int ring;                     // queue length (the retail maxpackets)
    int sampleRate;
    float volume;
    unsigned short pitch;
    short pan;
    int elevation;
    int cutoff;
    unsigned char value5F;
    int outputLevel;
    void (*release)(void *, void *);
    void (*frames)(int, int, void *);
    void *clientdata;
    Packet queue[kQueueCapacity];
    int head;                     // queue index of the packet being played
    int count;                    // packets queued, including the one being played
    int nextHandle;
    double position;              // read position in the packet being played, in source frames
};

Channel g_channels[kChannels];
SDL_AudioDeviceID g_device = 0;   // 0 when no device is open: every snd call then degrades to an error or no-op
unsigned int g_callbacks = 0;
unsigned int g_releases = 0;
unsigned int g_underrunFrames = 0;
float g_mix[kMixBatch];
Uint32 g_openTicks = 0;           // SDL_GetTicks when the device was opened (the start of the device timeline)

bool ValidChannel(int handle) {
    return handle >= 0 && handle < kChannels && g_channels[handle].allocated;
}

// Consumes the packet being played and releases it. The callback normally submits a packet to this channel, so
// the queue is read again by the caller.
void ReleaseHead(Channel &channel) {
    const Packet packet = channel.queue[channel.head];
    channel.head = (channel.head + 1) % channel.ring;
    channel.count -= 1;
    ++g_releases;
    if (channel.release) {
        channel.release(packet.samples, channel.clientdata);
    }
}

// Adds one channel to mix[0..frames). Each device frame reads the packet at the read position with linear
// interpolation; across a packet boundary the interpolation reads the first sample of the next queued packet.
void MixChannel(Channel &channel, float *mix, int frames) {
    const double step = ((double)channel.sampleRate / (double)kDeviceRate) * channel.pitch / 4096.0;
    int released = 0;
    for (int i = 0; i < frames; ++i) {
        bool ready = false;
        while (channel.count > 0 && released < kReleasesPerPass) {
            const Packet &packet = channel.queue[channel.head];
            if (channel.position < (double)packet.frames) {
                ready = true;
                break;
            }
            channel.position -= (double)packet.frames;
            ReleaseHead(channel);
            ++released;
        }
        if (!ready) {
            ++g_underrunFrames;
            continue;
        }
        const Packet &packet = channel.queue[channel.head];
        const int index = (int)channel.position;
        const double fraction = channel.position - (double)index;
        const float s0 = packet.samples[index];
        float s1 = s0;
        if (index + 1 < (int)packet.frames) {
            s1 = packet.samples[index + 1];
        } else if (channel.count > 1) {
            const Packet &next = channel.queue[(channel.head + 1) % channel.ring];
            if (next.frames > 0) {
                s1 = next.samples[0];
            }
        }
        mix[i] += kChannelGain * channel.volume * (s0 + (s1 - s0) * (float)fraction);
        channel.position += step;
    }
}

// Device callback: S16 stereo at kDeviceRate. Every playing channel is mixed mono into both outputs.
void AudioCallback(void *userdata, Uint8 *stream, int length) {
    (void)userdata;
    ++g_callbacks;
    short *out = reinterpret_cast<short *>(stream);
    const int frames = length / (int)(sizeof(short) * kDeviceChannels);
    int done = 0;
    while (done < frames) {
        const int batch = frames - done < kMixBatch ? frames - done : kMixBatch;
        for (int i = 0; i < batch; ++i) {
            g_mix[i] = 0.0f;
        }
        for (int c = 0; c < kChannels; ++c) {
            if (g_channels[c].allocated && g_channels[c].playing) {
                MixChannel(g_channels[c], g_mix, batch);
            }
        }
        for (int i = 0; i < batch; ++i) {
            float value = g_mix[i];
            if (value > 32767.0f) value = 32767.0f;
            if (value < -32768.0f) value = -32768.0f;
            const short sample = (short)value;
            out[2 * (done + i)] = sample;
            out[2 * (done + i) + 1] = sample;
        }
        done += batch;
    }
}

}  // namespace

// Backend storage for the original callback-dispatch mutex at 0x8b4f38.
// This is separate from the packet player's SDL device critical section.
void *engineSoundMutex8B4F38 = 0;

unsigned int __attribute__((stdcall)) EngineDispatchWait(void *mutex, unsigned int timeout) {
    if (timeout != 0xffffffffu || !mutex) return 0xffffffffu;
    return SDL_LockMutex(static_cast<SDL_mutex *>(mutex)) == 0 ? 0u : 0xffffffffu;
}

int __attribute__((stdcall)) EngineDispatchRelease(void *mutex) {
    return mutex && SDL_UnlockMutex(static_cast<SDL_mutex *>(mutex)) == 0 ? 1 : 0;
}

bool NativeAudioOpen(bool enabled, char *error, int errorSize) {
    if (!enabled || g_device != 0) {
        return true;
    }
    if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        snprintf(error, errorSize, "%s", SDL_GetError());
        return false;
    }
    SDL_AudioSpec want;
    SDL_AudioSpec have;
    memset(&want, 0, sizeof(want));
    memset(&have, 0, sizeof(have));
    want.freq = kDeviceRate;
    want.format = AUDIO_S16SYS;
    want.channels = kDeviceChannels;
    want.samples = kDeviceFrames;
    want.callback = AudioCallback;
    const SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device == 0) {
        snprintf(error, errorSize, "%s", SDL_GetError());
        return false;
    }
    if (have.freq != kDeviceRate || have.channels != kDeviceChannels || have.format != AUDIO_S16SYS) {
        SDL_CloseAudioDevice(device);
        snprintf(error, errorSize, "audio device did not accept 44100 Hz stereo S16");
        return false;
    }
    engineSoundMutex8B4F38 = SDL_CreateMutex();
    if (!engineSoundMutex8B4F38) {
        SDL_CloseAudioDevice(device);
        snprintf(error, errorSize, "%s", SDL_GetError());
        return false;
    }
    g_device = device;
    g_openTicks = SDL_GetTicks();
    SDL_PauseAudioDevice(device, 0);
    return true;
}

unsigned int NativeAudioOpenTicks(void) {
    return (unsigned int)g_openTicks;
}

void NativeAudioClose(void) {
    if (g_device != 0) {
        SDL_CloseAudioDevice(g_device);
        g_device = 0;
    }
    if (engineSoundMutex8B4F38) {
        SDL_DestroyMutex(static_cast<SDL_mutex *>(engineSoundMutex8B4F38));
        engineSoundMutex8B4F38 = 0;
    }
    for (int i = 0; i < kChannels; ++i) {
        g_channels[i].allocated = false;
        g_channels[i].playing = false;
    }
}

bool NativeAudioIsOpen(void) {
    return g_device != 0;
}

void NativeAudioStatistics(unsigned int *callbacks, unsigned int *releases, unsigned int *underrunFrames) {
    if (callbacks) *callbacks = g_callbacks;
    if (releases) *releases = g_releases;
    if (underrunFrames) *underrunFrames = g_underrunFrames;
}

// The critical section is the device lock. The release callback reaches these from the audio thread, which
// already holds the lock; SDL's device lock is recursive, so the nested lock is taken without a deadlock.
void SNDSYS_entercritical() {
    if (g_device != 0) {
        SDL_LockAudioDevice(g_device);
    }
}

void SNDSYS_leavecritical() {
    if (g_device != 0) {
        SDL_UnlockAudioDevice(g_device);
    }
}

// 0x723124: SNDPKTPLAYI_overhead, 32 bytes per packet plus 0x9c.
int SNDPKTPLAY_overhead(int maxpackets) {
    return maxpackets * 32 + 0x9c;
}

// 0x723138. The player keeps its state in the slot table, so the memory block is only sized, not used.
int SNDPKTPLAY_create(void (*preleasefunc)(void *, void *), void (*pframesfunc)(int, int, void *),
                      void *pclientdata, void *pmem, int memsize) {
    (void)pmem;
    if (g_device == 0) {
        return -10;
    }
    int result = -9;
    SNDSYS_entercritical();
    for (int slot = 0; slot < kChannels; ++slot) {
        if (g_channels[slot].allocated) {
            continue;
        }
        const int ring = (memsize - 0x9c) / 32;
        if (ring < 2) {
            result = -6;
            break;
        }
        Channel &channel = g_channels[slot];
        memset(&channel, 0, sizeof(channel));
        channel.allocated = true;
        channel.ring = ring < kQueueCapacity ? ring : kQueueCapacity;
        channel.sampleRate = 0;
        channel.release = preleasefunc;
        channel.frames = pframesfunc;
        channel.clientdata = pclientdata;
        result = slot;
        break;
    }
    SNDSYS_leavecritical();
    return result;
}

// 0x7231d4. The snd handle is the slot index. Only mono formats are played; the attributes and options are the
// retail defaults and are not modelled (see the header comment).
int SNDPKTPLAY_start(int packetinstancehandle, GinsuSndFormat *pssf, GinsuSndAttr *pssa, GinsuSndPlayOpts *pspo) {
    (void)pssa;
    (void)pspo;
    if (g_device == 0) {
        return -10;
    }
    if (!ValidChannel(packetinstancehandle) || pssf == 0) {
        return -1;
    }
    if (pssf->channels != 1 || pssf->samplerate == 0) {
        return -2;
    }
    SNDSYS_entercritical();
    Channel &channel = g_channels[packetinstancehandle];
    channel.sampleRate = pssf->samplerate;
    channel.volume = 1.0f;
    channel.pitch = 4096;
    channel.head = 0;
    channel.count = 0;
    channel.nextHandle = 0;
    channel.position = 0.0;
    channel.playing = true;
    SNDSYS_leavecritical();
    return packetinstancehandle;
}

// 0x723515. Callers hold the critical section (Ginsu wraps each submit); the release callback runs under it too.
int SNDPKTPLAY_submit(int packetinstancehandle, GinsuSndPacket *psp) {
    if (!ValidChannel(packetinstancehandle) || psp == 0) {
        return -1;
    }
    Channel &channel = g_channels[packetinstancehandle];
    if (channel.count >= channel.ring - 1) {
        return -13;
    }
    short *samples = static_cast<short *>(psp->psamples[0]);
    Packet &packet = channel.queue[(channel.head + channel.count) % channel.ring];
    packet.samples = samples;
    packet.frames = samples != 0 ? (psp->numframes & 0x7fffffffu) : 0u;
    packet.continuation = psp->numframes >> 31;
    packet.handle = channel.nextHandle;
    channel.count += 1;
    return channel.nextHandle++;
}

// 0x72387b: stops the player and drops its queue (no release callbacks).
int SNDPKTPLAY_stop(int packetinstancehandle) {
    if (!ValidChannel(packetinstancehandle)) {
        return -1;
    }
    SNDSYS_entercritical();
    Channel &channel = g_channels[packetinstancehandle];
    channel.playing = false;
    channel.count = 0;
    channel.head = 0;
    channel.position = 0.0;
    SNDSYS_leavecritical();
    return 0;
}

// 0x72366a: frees the slot.
int SNDPKTPLAY_destroy(int packetinstancehandle) {
    if (!ValidChannel(packetinstancehandle)) {
        return -1;
    }
    SNDSYS_entercritical();
    Channel &channel = g_channels[packetinstancehandle];
    channel.playing = false;
    channel.allocated = false;
    channel.count = 0;
    SNDSYS_leavecritical();
    return 0;
}

// Native snd adapters, not recovered middleware. The recovered controllers
// already bound their inputs. Slot validation replaces the retail generation
// registry; no voice graph, optional output callbacks or DSP is fabricated.
int EngineSnd72412A(int handle, int value) {
    if (!g_device) return -10;
    if (!ValidChannel(handle)) return -8;
    g_channels[handle].volume = value / 127.0f;
    return handle;
}

int EngineSnd724093(int handle, int value, int value2) {
    if (!g_device) return -10;
    if (!ValidChannel(handle)) return -8;
    g_channels[handle].pan = (short)value;
    g_channels[handle].elevation = value2;
    return handle;
}

int EngineSnd723D9B(int handle, int value) {
    if (!g_device) return -10;
    if (!ValidChannel(handle)) return -8;
    g_channels[handle].pitch = (unsigned short)value;
    return handle;
}

int EngineSnd72403E(int handle, int value) {
    if (!g_device) return -10;
    if (!ValidChannel(handle)) return -8;
    g_channels[handle].cutoff = value;
    return handle;
}

int EngineSnd723C41(int handle, int value) {
    if (!g_device) return -10;
    if (!ValidChannel(handle)) return -8;
    g_channels[handle].value5F = (unsigned char)value;
    return handle;
}

int EngineSnd723BC1(int handle, int index, int value) {
    if (!g_device) return -10;
    if (!ValidChannel(handle) || index != 0) return -8;
    g_channels[handle].outputLevel = value;
    return handle;
}

// 0x72423e: the retail defaults. Fields the retail does not write (padchar, tsdatasize) are not written here.
int SND_attrsetdef(GinsuSndAttr *pssa) {
    pssa->detune = 0;
    pssa->priority = 0;
    pssa->vol = 0x7f;
    pssa->pan = 0x40;
    pssa->fxlevel0 = 0;
    pssa->bendrange = 0;
    pssa->platformver = 0;
    pssa->rendermode = 0;
    for (int i = 0; i < 6; ++i) {
        pssa->azimuth[i] = 0;
        pssa->ptsdata[i] = 0;
    }
    for (int i = 0; i < 4; ++i) {
        pssa->puserdata[i] = 0;
        pssa->userdatasize[i] = 0;
    }
    return 0;
}

// 0x721a6b: the retail defaults. The pad words are not written by the retail and are not written here.
int SNDplaysetdef(GinsuSndPlayOpts *pspo) {
    pspo->vol = 0x7f;
    pspo->bend = 0x40;
    pspo->keynum = 0x3c;
    pspo->velocity = 0x7f;
    pspo->drylevel = 0x7f;
    pspo->fxlevel0 = 0;
    pspo->azimuth = 0;
    pspo->elevation = 0;
    pspo->pitchmult = 0x1000;
    pspo->timemult = 0x1000;
    pspo->tempomult = 0x1000;
    pspo->lowpasscutoff = 0xffff;
    pspo->highpasscutoff = 0;
    return 0;
}
