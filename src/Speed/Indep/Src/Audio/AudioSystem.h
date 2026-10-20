#ifndef UG2_AUDIO_SYSTEM_H
#define UG2_AUDIO_SYSTEM_H
#include "../Physics/Ug2Layout.h"

// The audio system object at *0x82b884. Only the members reached by recovered callers are named.
// 0x580130 deserializes the enable flag; device setup can clear it on failure.
struct CollisionEvent;
class CarReplayState;
struct MoverModeOwner;
struct AudioCarSoundManager;
struct MoverContext;
struct AudioManagedCarSound;
struct EngineSoundCar;

struct AudioManagedCarSoundVtable {
    void *unidentified0[3];
    void (__fastcall *bind0C)(AudioManagedCarSound *, void *, MoverModeOwner *);
    void *unidentified10[7];
    MoverContext *(__fastcall *context2C)(AudioManagedCarSound *);
};

struct AudioManagedCarSound {
    AudioManagedCarSoundVtable *vtable0;
    AudioManagedCarSound *next4;
    char unidentified8[0x28];
    int value30;
};

struct AudioCarSoundManagerVtable {
    void *unidentified0[4];
    // ABI view for MSVC 7.1: this in ECX, one stack argument, EDX unused.
    CarReplayState *(__fastcall *create10)(AudioCarSoundManager *, void *, MoverModeOwner *);
};

struct AudioCarSoundManager {
    void Initialize463AD0(int kind, int limit);
    CarReplayState *FindCarSound463CC0(MoverModeOwner *car);
    AudioCarSoundManagerVtable *vtable0;
    char unidentified4[8];
    int kindC;
    AudioManagedCarSound *first10;
    int limit14;
    char unidentified18[8];
};

struct AudioQueuedCarSoundManager {
    AudioCarSoundManager manager0;
    CarReplayState *QueueCar463F10(MoverModeOwner *car);
    MoverModeOwner *pending20[16];
    int count60;
};

struct CollisionSoundSet {
    void Add481CD0(CollisionEvent *event);    // not recovered
    void Remove481E50(CollisionEvent *event); // not recovered
};

struct Audio82B884 {
    char unidentified0[0x3c];
    CollisionSoundSet *collisions3C;
    char unidentified40[0x6c];
    AudioCarSoundManager *managerAC;
    AudioCarSoundManager *managerB0;
    AudioCarSoundManager *managerB4;
    char unidentifiedB8[0x24];
    void *engineBanksDC[2]; // 0x47d740: slot 0, indexed by the car's +0x10
    void *engineBanksE4[2]; // 0x47d740: slot 1; table fill still unrecovered
    Audio82B884 *Construct4585A0(); // original constructor; partial view retains unwritten bytes
    EngineSoundCar *FindListener4690B0(int index);
    void Apply(float *volumes);                    // 0x469050
    void RegisterCollision(CollisionEvent *event);   // 0x483e20
    void UnregisterCollision(CollisionEvent *event); // 0x483e40
    CarReplayState *CreateCarSound(MoverModeOwner *car); // 0x4690e0; manager factories remain separate
};

UG2_CHECK_SIZE(Audio82B884, 0xec);
UG2_CHECK_OFFSET(Audio82B884, managerAC, 0xac);
UG2_CHECK_OFFSET(Audio82B884, managerB0, 0xb0);
UG2_CHECK_OFFSET(Audio82B884, managerB4, 0xb4);
UG2_CHECK_OFFSET(Audio82B884, engineBanksDC, 0xdc);
UG2_CHECK_OFFSET(Audio82B884, engineBanksE4, 0xe4);
UG2_CHECK_SIZE(AudioCarSoundManager, 0x20);
UG2_CHECK_OFFSET(AudioCarSoundManager, first10, 0x10);
UG2_CHECK_OFFSET(AudioQueuedCarSoundManager, pending20, 0x20);
UG2_CHECK_OFFSET(AudioQueuedCarSoundManager, count60, 0x60);
UG2_CHECK_SIZE(AudioQueuedCarSoundManager, 0x64);

#endif
