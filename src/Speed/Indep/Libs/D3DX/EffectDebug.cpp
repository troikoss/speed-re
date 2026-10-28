#include "EffectDebug.h"

// Original 0x641539. The third argument's stack slot is reused for returned
// type; preserve its pointer-valued initialization and the queried byte count.
extern "C" int __stdcall D3DXEffectRegistryValue(unsigned int type,const char *name,void *data,unsigned int bytes) {
    unsigned int key=0;
    int result=UG2FxRegOpenKeyA(0x80000002,ug2FxRegistryPath7AB494,&key);
    if (!result) {
        unsigned int actualType=reinterpret_cast<unsigned int>(data);
        result=UG2FxRegQueryValueExA(key,name,0,&actualType,data,&bytes);
        UG2FxRegCloseKey(key);
        if (!result && actualType==type) return 1;
    }
    return 0;
}
// Original 0x671aed. Probe modules before checking the cached registry setting.
// Missing exports are retried while both callback slots remain zero.
extern "C" void __stdcall D3DXEffectDebugMute(unsigned int mute) {
    if (!ug2FxDebugRelease8A5CF8 && !ug2FxDebugRuntime8A5CF4) {
        unsigned int module=UG2FxGetModuleHandleA(ug2FxD3D9Name7B72A4);
        unsigned int procedureDispatch=ug2FxGetProcAddressImport783194,loadDispatch=ug2FxLoadLibraryImport783198;
        if (module) {
            module=UG2FxLoadLibraryA(loadDispatch,ug2FxD3D9Name7B72A4);
            if (module) ug2FxDebugRelease8A5CF8=UG2FxGetProcAddress(procedureDispatch,module,ug2FxDebugMuteName7B7294);
        }
        module=UG2FxGetModuleHandleA(ug2FxD3D9DebugName7B7288);
        if (module) {
            module=UG2FxLoadLibraryA(loadDispatch,ug2FxD3D9DebugName7B7288);
            if (module) ug2FxDebugRuntime8A5CF4=UG2FxGetProcAddress(procedureDispatch,module,ug2FxDebugMuteName7B7294);
        }
    }
    if (ug2FxDoNotMute80A170==-1) {
        if (!D3DXEffectRegistryValue(4,ug2FxDoNotMuteName7B7278,&ug2FxDoNotMute80A170,4)) ug2FxDoNotMute80A170=0;
        if (ug2FxDoNotMute80A170) ug2FxDoNotMute80A170=1;
    }
    if (ug2FxDoNotMute80A170) return;
    if (ug2FxDebugRelease8A5CF8) UG2FxCallDebugMute(ug2FxDebugRelease8A5CF8,mute);
    if (ug2FxDebugRuntime8A5CF4) UG2FxCallDebugMute(ug2FxDebugRuntime8A5CF4,mute);
}
