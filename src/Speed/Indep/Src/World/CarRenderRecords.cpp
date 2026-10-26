#include "CarRenderWheels.h"

// Original 0x48d860 (PC v1.2). The count is signed; a zero value does not clear slots.
void CarRenderSolid::SetSlot(unsigned int key, unsigned int value) {
    CarRenderSlotTable *table = table0C;
    if (table && value) {
        CarRenderSlot *slot = table->slots3C;
        int count = table->count1A;
        while (count > 0) {
            if (slot->key == key) {
                slot->value = value;
            }
            ++slot;
            --count;
        }
    }
}

// Original 0x60fef0 (PC v1.2). index0A is an unsigned word offset into a signed
// count/index table. Record indices are signed, and records have an eight-byte stride.
void *CarRenderPart::FindRecord(unsigned int hash, void *previous) {
    if (index0A == 0xffff) {
        return 0;
    }
    const short *indices = ug2CarPartTable8A1CC8->indices14 + index0A;
    int count = indices[0];
    int i = 0;
    if (previous) {
        while (i < count) {
            unsigned char *record = ug2CarPartTable8A1CC8->records1C + indices[i + 1] * 8;
            ++i;
            if (record == previous) {
                break;
            }
        }
        if (i == count) {
            return 0;
        }
    }
    while (i < count) {
        unsigned char *record = ug2CarPartTable8A1CC8->records1C + indices[i + 1] * 8;
        if (hash == 0 || *reinterpret_cast<unsigned int *>(record) == hash) {
            return record;
        }
        ++i;
    }
    return 0;
}
