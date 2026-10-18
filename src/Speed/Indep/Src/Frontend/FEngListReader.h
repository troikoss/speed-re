#ifndef UG2_FENG_LIST_READER_H
#define UG2_FENG_LIST_READER_H
#include "FEngList.h"

// Observed type-4 reader prefix only; lifecycle and outer iteration are separate.
struct FEngListPropertyReader {
    char unidentified0[0xc];
    FEListObject *objectC;
    char unidentified10[0x14];
    unsigned int value24, value28, value2C;
    void ReadProperty(const unsigned short *property); // 0x52b740
};
UG2_CHECK_OFFSET(FEngListPropertyReader, objectC, 0xc);
UG2_CHECK_OFFSET(FEngListPropertyReader, value24, 0x24);
UG2_CHECK_OFFSET(FEngListPropertyReader, value2C, 0x2c);
#endif
