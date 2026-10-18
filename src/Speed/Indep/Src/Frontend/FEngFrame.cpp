#include "FEngPackage.h"

// FEng frame manager 0x558c00 and the pad/mouse latches it calls. The caller
// 0x5662c0 passes the frame time in milliseconds and, as the second argument,
// whether the time may be applied in one step.

extern unsigned int fengFlagMask7F7C8C; // 0x7f7c8c

// Original 0x504fa0: latch a pad's buttons and accumulate the hold time of
// buttons held across both frames.
void FEngPadState::Latch(unsigned int buttons, int delta) {
    previous0 = current4;
    current4 = buttons;
    unsigned int *hold = hold8;
    for (int i = 0; i < 32; ++i, ++hold) {
        unsigned int bit = 1 << i;
        if (current4 & bit) {
            if (previous0 & bit) *hold += delta;
            else *hold = 0;
        }
    }
}

// Original 0x50b0c0: latch the mouse position and buttons; +0x1d records movement.
void FEngMouseState::Latch(FEngMousePacket *packet, int delta) {
    moved1D = packet->x0 != x0 || packet->y2 != y4;
    x0 = packet->x0;
    y4 = packet->y2;
    z8 = packet->z4;
    previousButtonsC = buttonsE;
    buttonsE = packet->buttons6;
    int *hold = hold10;
    for (int i = 0; i < 3; ++i, ++hold) {
        int bit = 1 << i;
        if (bit & buttonsE) {
            if (bit & previousButtonsC) *hold += delta;
            else *hold = 0;
        }
    }
}

// Original 0x558c00: read and dispatch input, then advance the package scripts.
// With +0 set the time is applied in steps of at most 20 ms unless unlimited,
// and every step drains messages, consumes operations and drains again.
void FEngQueue::Update(int delta, int unlimited) {
    if (value5251) value10C->BeginFrame();
    if (scripts0) {
        if (value1) {
            consumed34 = 0;
            if (mouse2) {
                FEngMousePacket packet;
                value10C->ReadMouse(&packet);
                mouseC.Latch(&packet, delta);
            }
            for (unsigned char pad = 0; pad < padCountD4; ++pad)
                pads8[pad].Latch(value10C->ReadPad(pad), delta);
            for (FEPackage *package = packagesDC.first8; package; package = package->next4) {
                DispatchPad(package);
                if (mouse2) DispatchMouse(package);
            }
            unsigned int bit = 1;
            unsigned int *consumed = value38;
            for (int count = 19; count; --count) {
                if (consumed34 & bit) {
                    for (unsigned char pad = 0; pad < padCountD4; ++pad) {
                        unsigned int amount = *consumed;
                        unsigned int *hold = pads8[pad].hold8;
                        for (int i = 0; i < 32; ++i, ++hold) {
                            if (bit & (1 << i)) {
                                if (*hold > amount) *hold -= amount;
                                else *hold = 0;
                            }
                        }
                    }
                }
                *consumed = 0;
                bit <<= 1;
                ++consumed;
            }
            value2C = value30;
        }
        if (scripts0) {
            int remaining = delta;
            do {
                int step = remaining;
                if (!unlimited && remaining > 20) step = 20;
                fengFlagMask7F7C8C = value5250 ? 0 : 0xffffffff;
                FEPackage *package = packagesDC.first8;
                while (package) {
                    FEPackage *next = package->next4;
                    package->queue30 = this;
                    package->delta34 = step;
                    for (HudVisibilityObject *object = package->firstObject4C; object; object = object->next4)
                        package->UpdateObject(object, step);
                    package = next;
                }
                DeliverMessages();
                ConsumeOperations();
                if (messages4114.first8) DeliverMessages();
                remaining -= step;
                value5250 = 0;
            } while (remaining);
            goto done;
        }
    }
    // Without +0: one script update of the whole time and no drains.
    for (FEPackage *package = packagesDC.first8; package; package = package->next4) {
        package->queue30 = this;
        package->delta34 = delta;
        for (HudVisibilityObject *object = package->firstObject4C; object; object = object->next4)
            package->UpdateObject(object, delta);
    }
done:
    if (value5251) value10C->EndFrame();
}
