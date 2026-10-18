#ifndef UG2_FENG_SCRIPT_READER_H
#define UG2_FENG_SCRIPT_READER_H
#include "FEngPackage.h"

// Observed script-reader prefix of the package object reader context (0x5515e0);
// objectC and value18 coincide with FEngResponseReader. Other fields keep offsets.
struct FEngScriptReader {
    char unidentified0[0xa];
    unsigned char value0A;      // nonzero: Sn calls owned name setter 0x518730
    char unidentified0B;
    HudVisibilityObject *objectC;
    char unidentified10[8];
    unsigned char value18;      // zero: append the final script to object +0x44
    char unidentified19[0x23];
    FEngQueue *queue3C;         // Ti only: type registry at +0x4134
    unsigned char Read(const unsigned short *data, unsigned int size); // 0x5515e0, AL only
};
UG2_CHECK_OFFSET(FEngScriptReader, value0A, 0xa);
UG2_CHECK_OFFSET(FEngScriptReader, objectC, 0xc);
UG2_CHECK_OFFSET(FEngScriptReader, value18, 0x18);
UG2_CHECK_OFFSET(FEngScriptReader, queue3C, 0x3c);
#endif
