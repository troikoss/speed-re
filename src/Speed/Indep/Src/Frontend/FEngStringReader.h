#ifndef UG2_FENG_STRING_READER_H
#define UG2_FENG_STRING_READER_H
#include "FEngCoordinates.h"

// Observed reader prefix only; surrounding chunk iteration and ownership are separate.
struct FEngStringPropertyReader {
    char unidentified0[9];
    unsigned char value9; // Nonzero permits the SL byte-label property.
    char unidentifiedA[2];
    FEStringObject *objectC;
    void ReadProperty(const unsigned short *property); // 0x534fc0
};
UG2_CHECK_OFFSET(FEngStringPropertyReader, value9, 9);
UG2_CHECK_OFFSET(FEngStringPropertyReader, objectC, 0xc);
#endif
