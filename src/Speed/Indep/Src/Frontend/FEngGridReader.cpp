#include "FEngGridReader.h"

// Original 0x535060, complete property dispatch. The payload length word is not
// validated here; bounds belong to the unrecovered surrounding reader.
void FEngGridPropertyReader::ReadProperty(const unsigned short *property) {
    unsigned int tag = *property;
    FEGridObject *grid = objectC;
    const unsigned int *payload = (const unsigned int *)(property + 2);
    if (tag <= 0x6a4c) {
        if (tag == 0x6a4c) {
            grid->SetCellValue1C(0, 0, payload[0], grid->count60, grid->count64);
            return;
        }
        if (tag == 0x444c) { grid->ResizeCells(payload[0], payload[1]); return; }
        if (tag == 0x4953) { grid->ResizeStringPool(payload[0], payload[1]); return; }
        if (tag == 0x6343) {
            grid->SetCellValue0(0, 0, payload[0], grid->count60, grid->count64);
            return;
        }
    } else if (tag == 0x7343) {
        unsigned int pair[2];
        pair[0] = payload[0]; pair[1] = payload[1];
        grid->SetCellPair4(0, 0, pair, grid->count60, grid->count64);
    } else {
        if (tag == 0x744c) {
            unsigned int owned = grid->flags68 & 1;
            grid->flags68 = owned;
            grid->flags68 = (payload[0] & 0xfffffffe) | owned;
            return;
        }
        if (tag == 0x764c) {
            float second = ((const float *)payload)[1];
            grid->value84 = ((const float *)payload)[0];
            grid->value88 = second;
            return;
        }
    }
}
