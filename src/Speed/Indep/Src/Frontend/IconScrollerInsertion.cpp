#include "PauseMenu.h"
#include "FEngCoordinates.h"
#include "FEngPackage.h"

// Original 0x51f670. Links, colour, dimensions and final padding are untouched.
IconScrollerIcon::IconScrollerIcon(unsigned int value0C, unsigned int value2C_, unsigned int value30_) {
    value2C = value2C_;
    valueC = value0C;
    value30 = value30_;
    object10 = 0;
    offset14 = value18 = 0.0f;
    value20 = 0;
    unidentified21[0] = 1;
    unidentified21[1] = 0;
    unidentified21[2] = 0;
    value34 = value38 = value3C = value40 = 1.0f;
    value44 = 1;
    value45 = 0;
}

// Original 0x543c10. Repeated size reads and colour writes preserve call order.
HudVisibilityObject *IconScroller4C::InsertIcon(IconScrollerIcon *icon) {
    char name[32];
    FngFormatBuffer(name, 32, "%s%d", prefix20, option28);
    HudVisibilityObject *object = FngFindNamedImage(package1C, name);
    if (!object) {
        delete icon;
        return 0;
    }
    option28 = (int)((unsigned int)option28 + 1u);
    if (!icon) return 0;
    icon->object10 = object;
    FEngGetSize(object, &icon->width24, &icon->height28);
    icon->colour1C = FEngGetColour(object);
    icon->offset14 = runningE0;
    icon->colour1C = 0xffffffff;
    FEngGetSize(static_cast<HudVisibilityObject *>(icon->object10), &icon->width24, &icon->height28);
    float width, height;
    FEngGetSize(static_cast<HudVisibilityObject *>(icon->object10), &width, &height);
    runningE0 = (float)(((double)spacing24 + runningE0) + width);
    void *node = &icon->next4;
    void *tail = previous8;
    *static_cast<void **>(tail) = node;
    previous8 = node;
    icon->previous8 = tail;
    icon->next4 = &next4;
    if (!currentC) {
        currentC = next4 ? reinterpret_cast<IconScrollerIcon *>(static_cast<char *>(next4) - 4) : 0;
        HudVisibilityObject *first = static_cast<HudVisibilityObject *>(currentC->object10);
        if (fengManager8384C4 && first) fengManager8384C4->SetFocusByHash(package1C, first->hash10);
    }
    return object;
}

// Original 0x50dab0: insertion owns deletion on failure; texture changes mark dirty.
void IconScrollerFNGObject::AddIcon(IconScrollerIcon *icon) {
    HudVisibilityObject *object = reinterpret_cast<IconScroller4C *>(scroller4C)->UnknownSlot08(icon);
    unsigned int texture = icon->valueC;
    if (object && object->texture24 != texture) {
        object->texture24 = texture;
        object->flags1C |= 0x400000;
    }
}
