// Platform scaffolding for the native audio device behind the snd library seam (see NativeAudio.cpp). Not
// recovered code. The demo opens the device after SDL_Init and closes it after the engine sound is destroyed.
#ifndef UG2_NATIVE_AUDIO_H
#define UG2_NATIVE_AUDIO_H

// Opens the device (44100 Hz, stereo S16, callback mode) when enabled. Returns false with a message in error when
// the device cannot be opened; the snd calls then return error codes (see NativeAudio.cpp).
bool NativeAudioOpen(bool enabled, char *error, int errorSize);
void NativeAudioClose(void);
bool NativeAudioIsOpen(void);
// SDL_GetTicks when the device was opened: the device timeline (sample 0 of the output) starts here.
unsigned int NativeAudioOpenTicks(void);
// Audio callbacks, packets released to the synthesizer and device frames with no packet to play, since open.
void NativeAudioStatistics(unsigned int *callbacks, unsigned int *releases, unsigned int *underrunFrames);

#endif
