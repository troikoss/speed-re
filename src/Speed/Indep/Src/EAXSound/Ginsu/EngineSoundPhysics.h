#ifndef UG2_ENGINE_SOUND_PHYSICS_H
#define UG2_ENGINE_SOUND_PHYSICS_H
#include "EngineSound.h"

struct EngineSoundModeQueries {
    char unidentified0[0x10];
    int value10;
    char unidentified14[0x14];
    int value28;
    char unidentified2C[0x14];
    int value40;
    unsigned char Is435950();
    bool Is430560();
    bool Is430420();
    bool Is430350();
};

struct EngineSoundRamp {
    float value0, value4, value8, valueC, value10, value14;
    unsigned char flag18;
    char unidentified19[3];
    EngineSoundRamp *Construct46B240(float first, float second, float start, float end);
    void Evaluate4599E0();
};
UG2_CHECK_SIZE(EngineSoundRamp, 0x1c);

// Original external subsystem boundaries; no native implementations are supplied.
struct EngineSoundNotificationView {
    int Query46C9A0();
    unsigned char Ready46C790();
    void Play484510(int kind, unsigned int name, int value, void (__cdecl *callback)(), int data);
    void Clear45AD40(int kind);
};
unsigned char __fastcall EngineSoundBlown5ECE00(struct MoverModeOwner *car);
extern EngineSoundModeQueries engineSoundModes827E88;
extern int *engineSoundMode890118, *engineSoundMode8900D8;
extern int engineSoundOverride82A3A4;
extern void *audioObject82BA30;
extern float engineSoundPercent784258, engineSoundStep784478, engineSoundRange78C194;
extern float engineSpeedOverride;
extern float engineSoundOverrideStep788608, engineSoundOverrideDecay78860C, engineSoundThrottleStep787AF4;
extern unsigned short engineSoundSequence7F0070[], engineSoundSequence7F0198[], engineSoundSequence7F0238[];
extern unsigned short engineSoundSequence7F02A8[], engineSoundSequence7F0328[], engineSoundSequence7F0398[];
extern unsigned short engineSoundSequence7F03F8[], engineSoundSequence7F0448[], engineSoundSequence7F0558[];
extern unsigned short engineSoundSequence7F0640[], engineSoundSequence7F06D0[], engineSoundSequence7F0750[];
extern unsigned short engineSoundSequence7F07C8[], engineSoundSequence7F0888[], engineSoundSequence7F08F8[];
extern char engineSoundBlownName78C154[];
EngineSoundPhysicsView *CreateEngineSoundPhysics470A00();
#endif
