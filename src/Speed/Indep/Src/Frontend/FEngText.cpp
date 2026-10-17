#include "FEngText.h"

// Original 0x50c900. The pre-callback flag bit controls the final dirty write.
void FngSetLabelString(void *value, unsigned int hash) {
    HudVisibilityObject *object = static_cast<HudVisibilityObject *>(value);
    if (object) {
        unsigned int flags = object->flags1C;
        if (object->languageHash60 != hash) {
            object->flags1C = flags | 0x400000;
            object->languageHash60 = hash;
        }
        if (fengStringObserver838478) fengStringObserver838478->StringChanged(object);
        unsigned int updated = object->flags1C & ~2u;
        object->flags1C = updated;
        if (flags & 2) object->flags1C = updated | 0x400000;
    }
}
