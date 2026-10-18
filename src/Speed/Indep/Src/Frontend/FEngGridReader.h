#ifndef UG2_FENG_GRID_READER_H
#define UG2_FENG_GRID_READER_H
#include "FEngGrid.h"
// Observed prefix of the original reader. This does not recover its ownership,
// lifecycle, chunk iteration, factory, or the other object-type dispatch paths.
struct FEngGridPropertyReader {
    char unidentified0[0xc];
    FEGridObject *objectC;
    void ReadProperty(const unsigned short *property); // 0x535060
};
UG2_CHECK_OFFSET(FEngGridPropertyReader, objectC, 0xc);
#endif
