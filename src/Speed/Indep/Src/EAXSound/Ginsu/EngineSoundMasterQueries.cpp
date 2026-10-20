#include "EngineSoundMaster.h"
#include "EngineSoundPhysics.h"
// 0x432490: intrusive links address the record at +0x10, including the sentinel.
EngineSoundModeRecord *EngineSoundModeList::Find432490(int value) {
    EngineSoundModeRecord *record = first4 ? reinterpret_cast<EngineSoundModeRecord *>(reinterpret_cast<char *>(first4) - 0x10) : 0;
    for (;;) {
        char *head = reinterpret_cast<char *>(this) + 4;
        EngineSoundModeRecord *end = head ? reinterpret_cast<EngineSoundModeRecord *>(head - 0x10) : 0;
        if (record == end) return 0;
        if (record->value18 == value) return record;
        record = record->next10 ? reinterpret_cast<EngineSoundModeRecord *>(reinterpret_cast<char *>(record->next10) - 0x10) : 0;
    }
}
// 0x435950: only AL is consumed; unordered timing compares as inactive.
unsigned char EngineSoundModeQueries::Is435950() {
    if (!value28) return 1;
    EngineSoundModeRecord *record = engineSoundModeList827B88.Find432490(value28);
    return record && record->value34 < record->value2C;
}
// 0x4fe4a0: only AL is defined by the query's interface.
unsigned char EngineSoundFrontend4FE4A0() { return engineSoundMasterFrontend8383AC != 0; }
