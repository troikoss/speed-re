#ifndef UG2_ENGINE_SOUND_BANKS_H
#define UG2_ENGINE_SOUND_BANKS_H
#include "EngineSound.h"

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
