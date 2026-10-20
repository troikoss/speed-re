#ifndef UG2_ENGINE_SOUND_MASTER_H
#define UG2_ENGINE_SOUND_MASTER_H
#include "EngineSound.h"
struct EngineSoundMaster {
    EngineSoundComponentVtable *vtable0;
    int values4[16];
    EngineSoundCar *state44;
    char unidentified48[0x10];
    unsigned char active58; char unidentified59[3];
    float time5C, delta60; int value64;
    EngineSoundCar *car68;
    EngineSoundMaster *Construct45DC80();
    void Activate45DCC0();
    void Update471970(float duration);
};
UG2_CHECK_SIZE(EngineSoundMaster, 0x6c);
EngineSoundMaster *CreateEngineSoundMaster471930();
struct EngineSoundModeRecord {
    char unidentified0[0x10]; EngineSoundModeRecord *next10;
    char unidentified14[4]; int value18;
    char unidentified1C[0x10]; float value2C;
    char unidentified30[4]; float value34;
};
struct EngineSoundModeList {
    void *unidentified0;
    EngineSoundModeRecord *first4;
    EngineSoundModeRecord *Find432490(int value);
};
extern EngineSoundModeList engineSoundModeList827B88;
unsigned char EngineSoundFrontend4FE4A0();
extern float engineSoundMasterScale78A6C8;
extern unsigned char engineSoundMasterFlag89E7D8, engineSoundMasterFlag89E7D9;
extern int engineSoundMasterMode890108, engineSoundMasterMode890104;
extern int *engineSoundMasterOwner82B9C8, *engineSoundMasterReplay8650B8;
extern int *engineSoundMasterFrontend8383A4;
extern int engineSoundMasterFrontend8383AC;
#endif
