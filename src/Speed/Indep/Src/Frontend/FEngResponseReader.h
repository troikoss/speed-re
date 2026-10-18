#ifndef UG2_FENG_RESPONSE_READER_H
#define UG2_FENG_RESPONSE_READER_H
#include "FEngPackage.h"
#include "FEngResponse.h"

struct FEngResponseReader {
    FEPackage *package0;
    char unidentified4[8];
    HudVisibilityObject *objectC;
    char unidentified10[8];
    unsigned char value18;
    unsigned char Read(const void *data, unsigned int size, unsigned char packageMode); // 0x54d1d0, AL only
};
UG2_CHECK_OFFSET(FEngResponseReader, objectC, 0xc);
UG2_CHECK_OFFSET(FEngResponseReader, value18, 0x18);
#endif
