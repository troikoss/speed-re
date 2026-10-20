#ifndef UG2_ENGINE_SOUND_MIXER_H
#define UG2_ENGINE_SOUND_MIXER_H
#include "EngineSound.h"
struct EngineSoundMixerChannel;
// Virtual-slot call view used by the binder. The master base uses slot +0xc
// as a setter in its separate initialization path; the 3D view queries it here.
struct EngineSoundMixerChannelVtable {
    void *unidentified0[2];
    unsigned int (__fastcall *value8)(EngineSoundMixerChannel *);
    unsigned int (__fastcall *valueC)(EngineSoundMixerChannel *);
    unsigned int (__fastcall *value10)(EngineSoundMixerChannel *);
    void (__fastcall *attach14)(EngineSoundMixerChannel *, void *, EngineSoundMixerChannel *, EngineSound *, EngineSoundScheduledNode *);
    int (__fastcall *value18)(EngineSoundMixerChannel *);
};
struct EngineSoundMixerChannel {
    EngineSoundMixerChannelVtable *vtable0;
    char unidentified4[4];
    unsigned int value8, valueC;
    char unidentified10[8];
    unsigned int value18;
    char unidentified1C[0xc];
    int count28, count2C, count30;
    unsigned int *inputs34;
    int *fixed38, *spatial3C, *all40;
    char unidentified44[4];
    int index48;
    int *values4C;
    unsigned int *map50;
};
UG2_CHECK_SIZE(EngineSoundMixerChannel, 0x54);
struct EngineSoundMixerOwner {
    char unidentified0[0x78];
    int *masterInputs78, *subInputs7C;
    char unidentified80[0x24];
    int masterCountA4, subCountA8;
};
struct EngineSoundMixer {
    char unidentified0[4];
    EngineSoundMixerOwner *owner4;
    char *asset8;
    char unidentifiedC[0x18];
    EngineSoundMixerChannel **spatial24;
    char unidentified28[8];
    EngineSoundMixerChannel **sub30, **master34;
    char unidentified38[8];
    int *values40;
    unsigned int kind44;
    char unidentified48[8];
    unsigned int index50;
    char unidentified54[0xc];
    int count60;
    char unidentified64[4];
    int subCount68, masterCount6C;
    void Allocate478120();
    void Bind480420();
};
UG2_CHECK_OFFSET(EngineSoundMixer, master34, 0x34);
UG2_CHECK_OFFSET(EngineSoundMixer, masterCount6C, 0x6c);
struct EngineSoundMixerGroup {
    char unidentified0[0x48]; int count48;
    char unidentified4C[0x2c]; EngineSoundMixer **mixers78;
};
struct EngineSoundMixerTable { char unidentified0[4]; int count4; };
struct EngineSoundMixerSetup {
    char unidentified0[0x28]; EngineSoundMixerTable *table28;
    char unidentified2C[0xc]; EngineSoundMixerGroup **groups38;
    void Bind483900();
};
extern char engineSoundMixerSubCount78B7C8[], engineSoundMixerSubSpatial78B7A0[];
extern char engineSoundMixerMasterCount78BDC4[], engineSoundMixerMasterSpatial78BD98[];
extern char engineSoundMixerLookup78BD74[], engineSoundMixerMap78BD4C[], engineSoundMixerMissing78BD24[];
#endif
