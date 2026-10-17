#include "PauseMenu.h"
#include "../../Libs/bWare/Strings.h"

// Original 0x4953c0: hash even when the package/name is null, then typed lookup.
void *FngFindNamedString(const char *package, const char *name) {
    return FngFindStringByHash(package, cihash(name));
}

// Original 0x50da50: use the bound labels and reload current icon after the call.
void IconScrollerFNGObject::SelectedIconChanged() {
    IconScrollerIcon *icon = reinterpret_cast<IconScroller4C *>(scroller4C)->currentC;
    FngSetLabelString(title134, icon ? icon->value2C : 0);
    icon = reinterpret_cast<IconScroller4C *>(scroller4C)->currentC;
    FngSetLabelString(description138, icon ? icon->value30 : 0);
}
