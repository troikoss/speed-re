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
    int value4;
    unsigned int value8, valueC;
    int value10; unsigned int value14;
    unsigned int value18;
    char unidentified1C[0xc];
    int count28, count2C, count30;
    unsigned int *inputs34;
    int *fixed38, *spatial3C, *all40;
    void *section44;
    int index48;
    int *values4C;
    unsigned int *map50;
    void Setup477A50(unsigned int id, void *section);
    void SetOutput467400(unsigned int id);
    unsigned int Output467410();
};
UG2_CHECK_SIZE(EngineSoundMixerChannel, 0x54);
unsigned int *__stdcall EngineSoundMixerDecode479030(void *section, EngineSoundMixerChannel *channel, unsigned int *record);
struct EngineSoundMixerSpatial {
    EngineSoundMixerChannelVtable *vtable0;
    char *input4;
    int value8, valueC, value10, value14, value18;
    unsigned int key1C, key20, *data24, *record28;
    void Setup4683E0(unsigned int *record, unsigned int instance, unsigned int index);
    void Attach468430(EngineSoundMixerChannel *, EngineSound *, EngineSoundScheduledNode *input);
    unsigned int Key503B00();
    unsigned int Input6E18A0();
    int *Values40BDE0();
};
UG2_CHECK_SIZE(EngineSoundMixerSpatial, 0x2c);
extern int engineSoundMixerScratch82BC9C, engineSoundMixerScratch82BCA0;
extern char engineSoundMixerDecodeMaster78BA1C[], engineSoundMixerDecodeSub78B9F0[];
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
