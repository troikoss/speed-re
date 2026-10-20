#ifndef UG2_ENGINE_SOUND_BANKS_H
#define UG2_ENGINE_SOUND_BANKS_H
#include "EngineSound.h"

struct EngineSoundChoice {
    int value0;
    int value4;
    int value8;
};
UG2_CHECK_SIZE(EngineSoundChoice, 0xc);
UG2_CHECK_SIZE(EngineSoundCarValue, 0x4c);
UG2_CHECK_SIZE(EngineSoundCarValue84, 0x34);
// Partial state view ends after the last observed constructor store; this
// does not establish the allocation size of the original derived state.
UG2_CHECK_SIZE(EngineSoundCar, 0xcc);
UG2_CHECK_OFFSET(EngineSoundCar, context74, 0x74);
UG2_CHECK_OFFSET(EngineSoundCar, value94, 0x94);
extern EngineSoundCarValue engineSoundPrimary7F0DE0[];
extern EngineSoundCarValue84 engineSoundSecondary7F1AA8[];
extern EngineSoundChoice engineSoundChoices7F2230[];
int EngineSoundCarIndex459240(unsigned int carType);

struct EngineBankRecord {
    int value0;
    int value4;
    int value8;
    const char *directoryC;
    int value10;
    int value14;
    int value18;
    int value1C;
    int value20;
    int value24;
    int value28;
};
UG2_CHECK_SIZE(EngineBankRecord, 0x2c);
UG2_CHECK_OFFSET(EngineSoundCarValue, fileC, 0xc);
UG2_CHECK_OFFSET(EngineSoundCar, value84, 0x84);
extern EngineBankRecord engineBankRecords82B098[];
extern const char *engineBankDirectory7ED164;
extern int engineParameterMode850078;
int EngineBankFileSize57D900(const char *filename);
int EngineBankRead57B4A0(void *destination, const char *filename, int offset, int size,
                       void (__cdecl *callback)(void *), void *clientdata);
#endif
