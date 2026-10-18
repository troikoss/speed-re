#include "FEngScript.h"
#include "FEngPackage.h"

extern unsigned int FngFormatHash(const char *format, ...);

// Original 0x537c00. Keep the local-package and manager fallback calls explicit.
void FngSetScript(const char *name, unsigned int hash, const char *script, int restart) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    FngSetObjectScript(object, script, (unsigned char)restart);
}

// Original 0x537c60.
void FngSetScriptHash(const char *name, unsigned int hash, unsigned int script, int restart) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    FngSetObjectScriptHash(object, script, (unsigned char)restart);
}

// Original 0x537cc0: formatting occurs after lookup, even when no object was found.
bool FngScriptActive(const char *name, unsigned int hash, const char *script) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    unsigned int scriptHash = FngFormatHash(script);
    return object && object->currentScript54 && object->currentScript54->hash30 == scriptHash &&
           object->currentScript54->time10 <= object->currentScript54->lengthC;
}

// Original 0x537d40: AL is the defined return; time/length comparison is signed and inclusive.
bool FngScriptRunning(const char *name, unsigned int hash, unsigned int script) {
    HudVisibilityObject *object = 0;
    if (name) {
        FEPackage *package = FngFindPackage(name);
        if (package) object = package->FindObject(hash);
    }
    if (!object && fengManager8384C4) object = fengManager8384C4->queue8->FindObject(hash, name);
    return object && object->currentScript54 && object->currentScript54->hash30 == script &&
           object->currentScript54->time10 <= object->currentScript54->lengthC;
}
