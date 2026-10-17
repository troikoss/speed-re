#include "PauseMenu.h"
#include "FEngPackage.h"

int FngScrollerDiagnostic(const char *, ...); // 0x50d510

// Original 0x50da20: save the message and its three deferred arguments.
void IconScrollerFNGObject::Value50DA20(unsigned int message, int value8, int valueC, int value10) {
    value13C = message;
    value140 = value8;
    value144 = valueC;
    value148 = value10;
}

// Original 0x52fbf0: preserve the saved mask; stop the two live input routes.
// The original requires a valid package and does not check the lookup result.
void IconScrollerFNGObject::StopInput() {
    reinterpret_cast<IconScroller4C *>(scroller4C)->value2F = 0;
    FEPackage *package = FngFindPackage(package4);
    FngScrollerDiagnostic("IconScrollerMenu : StopInput (%s, %x).  Mask (%d) to (%d).\n",
                          package4, package, package->inputMask1C, 0);
    package->inputMask1C = 0;
    package->inputMask24 = 0;
    inputStopped14C = 1;
}

// Original 0x52fba0: any nonzero stopped byte restores the saved input mask.
void IconScrollerFNGObject::StartInput() {
    if (inputStopped14C) {
        reinterpret_cast<IconScroller4C *>(scroller4C)->value2F = 1;
        FEPackage *package = FngFindPackage(package4);
        unsigned int mask = package->inputMask20;
        unsigned int previous = package->inputMask1C;
        package->inputMask1C = mask;
        package->inputMask28 = mask;
        FngScrollerDiagnostic("IconScrollerMenu : StartInput (%s, %x).  Mask (%d) to (%d).\n",
                              package4, package, previous, mask);
        inputStopped14C = 0;
    }
}
