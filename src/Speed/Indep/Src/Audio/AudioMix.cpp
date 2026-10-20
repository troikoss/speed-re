#include "AudioSystem.h"
struct AudioMixCallbackVtable {
    void *unidentified0[5];
    void (__fastcall *update14)(struct AudioMixCallback *);
};
struct AudioMixCallback { AudioMixCallbackVtable *vtable0; };
extern int audioEnabled7FAAA8;
extern AudioMixCallback *audioMixFirst82B974, *audioMixSecond82B978;
extern float audioMixValue7EC848, audioMixScale789CF4;
extern int audioMixPacked7EC84C;

// 0x469050: retain the caller's pointer, callback order and final reloads.
void Audio82B884::Apply(float *volumes) {
    mix38 = volumes;
    if (audioEnabled7FAAA8 && *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa0)) {
        if (audioMixFirst82B974) audioMixFirst82B974->vtable0->update14(audioMixFirst82B974);
        if (audioMixSecond82B978) audioMixSecond82B978->vtable0->update14(audioMixSecond82B978);
    }
    *reinterpret_cast<unsigned int *>(&audioMixValue7EC848) = *reinterpret_cast<unsigned int *>(&volumes[7]);
    audioMixPacked7EC84C = (int)((double)audioMixValue7EC848 * audioMixScale789CF4);
}
