#include "FEngStringReader.h"
#include "FEngObjectFactory.h"
#include "FEngText.h"
#include "FEngHash.h"

void FEStringObject::SetLabelHash(unsigned int hash) { // Original 0x4fe940
    if (languageHash60 != hash) {
        flags1C |= 0x400000;
        languageHash60 = hash;
    }
    if (fengStringObserver838478) fengStringObserver838478->StringChanged(this);
}

void FEStringObject::SetLabelBytes(const char *source) { // Original 0x519f40
    char *&label = *(char **)(unidentified58 + 4);
    FEngFreeQueueNode(label);
    label = 0;
    if (source) {
        unsigned int length = 0;
        do { ++length; } while (source[length - 1]);
        char *buffer = (char *)FEngAllocateQueueNode(length);
        label = buffer;
        char value;
        do {
            value = *source++;
            *buffer++ = value;
        } while (value);
    }
    languageHash60 = FngHash(label);
    flags1C |= 0x400000;
    if (fengStringObserver838478) fengStringObserver838478->StringChanged(this);
}

// Original 0x534fc0. This method does not inspect the payload length word.
void FEngStringPropertyReader::ReadProperty(const unsigned short *property) {
    unsigned int tag = *property;
    FEStringObject *object = objectC;
    const unsigned int *payload = (const unsigned int *)(property + 2);
    if (tag <= 0x6a53) {
        if (tag == 0x6a53) { object->justification6C = (int)*payload; return; }
        if (tag == 0x4853) { object->SetLabelHash(*payload); return; }
        if (tag == 0x4c53) {
            if (value9) object->SetLabelBytes((const char *)payload);
        } else if (tag == 0x6253) {
            ((FEngTextStorage *)&object->text64)->Grow(*payload);
        }
    } else if (tag == 0x6c53) {
        object->leading70 = (int)*payload;
    } else {
        if (tag == 0x7453) { ((FEngTextStorage *)&object->text64)->Assign((const unsigned short *)payload); return; }
        if (tag == 0x7753) object->maxWidth74 = *payload;
    }
}
