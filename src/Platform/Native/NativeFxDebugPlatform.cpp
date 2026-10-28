#include "../../Speed/Indep/Libs/D3DX/EffectDebug.h"
extern "C" void __cdecl abort(void);

// Linux supplies neither of the two Windows D3D runtime modules nor the
// Windows Direct3D registry key. The recovered caller takes its absence path.
// Other loader/registry/callback operations remain explicit unsupported seams.
extern "C" unsigned int __stdcall UG2FxGetModuleHandleA(const char *name) {
    if (name!=ug2FxD3D9Name7B72A4 && name!=ug2FxD3D9DebugName7B7288) abort();
    return 0;
}
extern "C" unsigned int UG2FxLoadLibraryA(unsigned int,const char *) { abort();return 0; }
extern "C" unsigned int UG2FxGetProcAddress(unsigned int,unsigned int,const char *) { abort();return 0; }
extern "C" int __stdcall UG2FxRegOpenKeyA(unsigned int key,const char *path,unsigned int *) {
    if (key!=0x80000002 || path!=ug2FxRegistryPath7AB494) abort();
    return 2; // ERROR_FILE_NOT_FOUND: no Windows registry key on this backend.
}
extern "C" int __stdcall UG2FxRegQueryValueExA(unsigned int,const char *,unsigned int *,unsigned int *,void *,unsigned int *) { abort();return 2; }
extern "C" int __stdcall UG2FxRegCloseKey(unsigned int) { abort();return 2; }
extern "C" void UG2FxCallDebugMute(unsigned int,unsigned int) { abort(); }
