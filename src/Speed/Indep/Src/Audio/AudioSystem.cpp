#include "AudioSystem.h"
#include "../Physics/VehicleMover.h"

extern int audioEnabled7FAAA8;
extern void *audioObject82BA30;

// 0x4585a0..0x458671. Offset-based construction view: unidentified fields
// remain unnamed rather than assigning subsystem meanings to these stores.
// The original leaves some bytes untouched and conditionally clears managers.
Audio82B884 *Audio82B884::Construct4585A0() {
    unsigned int *word = reinterpret_cast<unsigned int *>(this);
    unsigned char *byte = reinterpret_cast<unsigned char *>(this);
    word[0x90 / 4] = 1;
    byte[0x10] = 1;
    word[0] = 0x789cf0;
    word[0x98 / 4] = 0;
    engineBanksDC[0] = 0;
    engineBanksDC[1] = 0;
    engineBanksE4[0] = 0;
    engineBanksE4[1] = 0;
    word[0xcc / 4] = 0;
    byte[0x5c] = 0;
    word[0x9c / 4] = 0;
    word[0x88 / 4] = 0;
    byte[0x13] = 0;
    byte[0x14] = 0;
    byte[0x15] = 0;
    word[0x38 / 4] = 0;
    word[0xa0 / 4] = 0;
    word[0x30 / 4] = 0;
    word[0x28 / 4] = 0;
    word[0x2c / 4] = 0;
    word[0xd8 / 4] = 0;
    word[0x60 / 4] = 0x789ce8; // retail "SOUND" string, raw image address
    word[0x84 / 4] = 0;
    word[0x64 / 4] = 0;
    for (int i = 0; i < 9; ++i) {
        if (word[0xa8 / 4 + i]) word[0xa8 / 4 + i] = 0;
    }
    word[0x0c / 4] = 0;
    byte[0x12] = 0;
    word[0x20 / 4] = 0;
    byte[0x11] = 0;
    word[0x78 / 4] = 0;
    word[0x74 / 4] = 0;
    collisions3C = 0;
    word[0x70 / 4] = 0;
    word[0xd0 / 4] = 0;
    audioObject82BA30 = 0;
    word[0x7c / 4] = 0;
    word[0xcc / 4] = 0;
    word[0x80 / 4] = 0;
    byte[0x08] = 0;
    word[0xa4 / 4] = 0;
    return this;
}

// 0x463ad0..0x463ae0: base-manager kind and limit; no other field is changed.
void AudioCarSoundManager::Initialize463AD0(int kind, int limit) {
    kindC = kind;
    limit14 = limit;
}

// 0x463cc0..0x463d03. Re-read the car context for each candidate, before
// its virtual query; the query may change state. No sound object is allocated.
CarReplayState *AudioCarSoundManager::FindCarSound463CC0(MoverModeOwner *car) {
    for (AudioManagedCarSound *sound = first10; sound; sound = sound->next4) {
        MoverContext *context = car->context14;
        if (sound->vtable0->context2C(sound) == context && sound->value30 == 0) {
            sound->vtable0->bind0C(sound, 0, car);
            return reinterpret_cast<CarReplayState *>(sound);
        }
    }
    return 0;
}

// 0x463f10..0x463f22. The original does not check the queue limit here.
CarReplayState *AudioQueuedCarSoundManager::QueueCar463F10(MoverModeOwner *car) {
    pending20[count60] = car;
    ++count60;
    return 0;
}

// 0x4690e0..0x469155. The flag gate precedes all car reads. A set flag900
// routes every context kind through managerB0, including kind 1 and unknowns.
CarReplayState *Audio82B884::CreateCarSound(MoverModeOwner *car) {
    if (!audioEnabled7FAAA8) return 0;
    const int kind = car->context14->value4;
    AudioCarSoundManager *manager;
    if (kind == 1 && !car->context14->flag900) manager = managerAC;
    else if (kind == 2 || car->context14->flag900) manager = managerB0;
    else if (kind == 3) manager = managerB4;
    else return 0;
    if (!manager) return 0;
    return manager->vtable0->create10(manager, 0, car);
}
