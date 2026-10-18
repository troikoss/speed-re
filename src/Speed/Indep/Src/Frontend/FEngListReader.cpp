#include "FEngListReader.h"
#include "FEngColour.h"

// Original 0x52b740: length word ignored. Cache the current object across callees.
void FEngListPropertyReader::ReadProperty(const unsigned short *property) {
    unsigned int tag = *property;
    FEListObject *list = objectC;
    const unsigned int *payload = (const unsigned int *)(property + 2);
    if (tag <= 0x724c) {
        if (tag == 0x724c) {
            ++value28;
            FEListAxisEntry *entry = (FEListAxisEntry *)((unsigned int)list->value7C + value28 * 12);
            *(unsigned int *)&entry->value0 = payload[0];
            *(unsigned int *)&entry->value8 = payload[1];
            return;
        }
        if (tag <= 0x644c) {
            if (tag == 0x644c) {
                list->ResizeColumns(payload[0]);
                list->ResizeRows(payload[1]);
                value2C = 0xffffffff;
                value28 = 0xffffffff;
                value24 = 0xffffffff;
                list->SetColumn(0);
                list->SetRow(0);
                return;
            }
            if (tag == 0x5443) { list->SetCellType(payload[0]); return; }
            if (tag == 0x6343) {
                ++value2C;
                if (value2C) list->AdvanceCell();
                FEngColour colour;
                colour.Unpack(payload[0]);
                list->SetCellColour(&colour);
                return;
            }
            if (tag == 0x634c) {
                ++value24;
                FEListAxisEntry *entry = (FEListAxisEntry *)((unsigned int)list->value78 + value24 * 12);
                *(unsigned int *)&entry->value0 = payload[0];
                *(unsigned int *)&entry->value8 = payload[1];
                return;
            }
        } else {
            if (tag == 0x6943) {
                unsigned int values[4];
                values[1] = payload[1]; values[0] = payload[0];
                values[2] = payload[2]; values[3] = payload[3];
                list->SetCellFour20(values);
                return;
            }
            if (tag == 0x7243) { list->SetCellTripleC(0, 0, payload[0]); return; }
        }
    } else if (tag <= 0x7443) {
        if (tag == 0x7443) { list->SetCellText((const unsigned short *)payload); return; }
        if (tag == 0x7343) {
            unsigned int values[2];
            values[1] = payload[1]; values[0] = payload[0];
            list->SetCellPair4(values);
            return;
        }
        if (tag == 0x734c) {
            float second = ((const float *)payload)[1];
            list->value80 = ((const float *)payload)[0];
            list->value84 = second;
            return;
        }
    } else if (tag == 0x764c) {
        float second = ((const float *)payload)[1];
        list->value68 = ((const float *)payload)[0];
        list->value6C = second;
    } else if (tag == 0x774c) {
        list->SetWrap(payload[0] != 0);
    }
}
