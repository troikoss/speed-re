#include "EngineSound.h"

// UG2 PC v1.2 0x45e390, virtual +0x38 of vtable 0x78af30. The
// pointer-table base is known; no table length or record producer is inferred.
int EngineSound::Value45E390(int index) {
    void **table = reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x58);
    const unsigned int *value = static_cast<const unsigned int *>(table[index]);
    return value ? (int)(*value & 0x7fffu) : 0;
}

// 0x45e3b0, virtual +0x3c: MOVZX preserves all 16 bits in EAX.
int EngineSound::Value45E3B0(int index) {
    void **table = reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x58);
    const char *value = static_cast<const char *>(table[index]);
    return value ? (int)*reinterpret_cast<const unsigned short *>(value + 2) : 0;
}
