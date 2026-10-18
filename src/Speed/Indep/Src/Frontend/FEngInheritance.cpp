#include "FEngInheritance.h"

FEngInheritanceRecord *FEPackage::FindInheritance(unsigned int key) { // Original 0x504960
    unsigned int count = inheritanceCount84;
    if (count) {
        FEngInheritanceRecord *records = inheritance88;
        unsigned int i = 0;
        do {
            if (records[i].key0 == key) return records + i;
            ++i;
        } while (i < count);
    }
    return 0;
}

FEPackage *FEngQueue::FindByEncodedNameHash(unsigned int hash) { // Original 0x50b240
    FEPackage *package = first104;
    while (package) {
        const unsigned char *name = (const unsigned char *)((unsigned int)package->value38 + 2);
        unsigned int value = 0xffffffff;
        if (name) {
            unsigned char c = *name;
            while (c) {
                value *= 33;
                if ((signed char)c >= 'a' && (signed char)c <= 'z') c += 'A' - 'a';
                value += c;
                ++name;
                c = *name;
            }
        }
        if (value == hash) return package;
        package = package->next4;
    }
    return 0;
}

unsigned char FEngInheritanceContext::Resolve(unsigned int key, HudVisibilityObject **object,
                                             FEPackage **package) { // Original 0x5186c0
    *object = 0;
    *package = 0;
    FEngInheritanceRecord *record = package0->FindInheritance(key);
    if (record) {
        FEPackage *found = queue3C->FindByEncodedNameHash(record->packageHash4);
        *package = found;
        if (found) {
            HudVisibilityObject *value = found->FindObjectByGuid(record->guid8);
            *object = value;
            return value != 0;
        }
    }
    return 0;
}
