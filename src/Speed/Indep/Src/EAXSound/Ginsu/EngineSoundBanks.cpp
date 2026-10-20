#include "EngineSoundBanks.h"
#include "../../../Libs/bWare/Strings.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;

// UG2 PC v1.2 0x463500: retain every supplied state at +0x44; only kinds
// 1 and 2 become the engine-specific +0x9c view. Other bytes are untouched.
void EngineSound::Bind463500(EngineSoundCar *car) {
    car9C = car->kind14 == 1 || car->kind14 == 2 ? car : 0;
    *reinterpret_cast<EngineSoundCar **>(reinterpret_cast<char *>(this) + 0x44) = car;
    reinterpret_cast<unsigned char *>(this)[0xa0] = 1;
}

// 0x461550 / 0x461560: cdecl completion callbacks, one byte per slot.
void __cdecl EngineSound::BankRead461550(void *object) {
    static_cast<EngineSound *>(object)->slotsCC[0].value18 = 1;
}
void __cdecl EngineSound::BankRead461560(void *object) {
    static_cast<EngineSound *>(object)->slotsCC[1].value18 = 1;
}

// 0x465c80..0x465e97. Re-read the record index and later car/global inputs
// as the original does; file/heap callbacks may change them. A missing file
// retains its previous bank pointer. Positive sizes still enqueue a read
// after allocation failure; no new early return or null guard is introduced.
void EngineSound::LoadBanks465C80(int *count) {
    engineBankRecords82B098[*count].value0 = 1;
    if (engineParameterMode850078 == 4) {
        engineBankRecords82B098[*count].value4 = 1;
        engineBankRecords82B098[*count].value8 = car9C->value80->value4;
    } else {
        engineBankRecords82B098[*count].value4 = 0;
        engineBankRecords82B098[*count].value8 = car9C->value80->value0;
    }
    engineBankRecords82B098[*count].directoryC = engineBankDirectory7ED164;
    engineBankRecords82B098[*count].value10 = 0;
    engineBankRecords82B098[*count].value14 = 0;
    engineBankRecords82B098[*count].value18 = 0;
    engineBankRecords82B098[*count].value1C = 0;
    engineBankRecords82B098[*count].value20 = 0;
    engineBankRecords82B098[*count].value24 = 0;
    engineBankRecords82B098[*count].value28 = -1;
    *count = (int)((unsigned int)*count + 1u);
    char path[64];
    bStringConcat43DDA0(path, engineBankDirectory7ED164, car9C->value80->fileC);
    int size = EngineBankFileSize57D900(path);
    if (size > 0) {
        void *memory = 0;
        const int available = EngineSoundHeapAvailable43E910(engineSoundHeap7EC840);
        if ((int)((unsigned int)size + 0x400u) <= available) {
            memory = EngineSoundAllocate440820(size, (engineSoundHeap7EC840 & 7) | 0x1000);
        }
        audio82B884->engineBanksDC[car9C->bankIndex10] = memory;
        EngineBankRead57B4A0(memory, path, 0, size, BankRead461550, this);
    }
    if (car9C->mode78 == 1) {
        bStringConcat43DDA0(path, engineBankDirectory7ED164, car9C->value84->file0);
        size = EngineBankFileSize57D900(path);
        if (size > 0) {
            void *memory = 0;
            const int available = EngineSoundHeapAvailable43E910(engineSoundHeap7EC840);
            if ((int)((unsigned int)size + 0x400u) <= available) {
                memory = EngineSoundAllocate440820(size, (engineSoundHeap7EC840 & 7) | 0x1000);
            }
            audio82B884->engineBanksE4[car9C->bankIndex10] = memory;
            EngineBankRead57B4A0(memory, path, 0, size, BankRead461560, this);
        }
    }
}
