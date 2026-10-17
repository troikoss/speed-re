#ifndef UG2_FENG_TEXT_H
#define UG2_FENG_TEXT_H

#include "../Hud/HudVisibility.h"

// Only the observer's first virtual slot is established by 0x50c900.
struct FEngStringObserver {
    virtual void StringChanged(HudVisibilityObject *object) = 0;
};
extern FEngStringObserver *fengStringObserver838478;
void FngSetLabelString(void *object, unsigned int hash); // 0x50c900, cdecl

#endif
