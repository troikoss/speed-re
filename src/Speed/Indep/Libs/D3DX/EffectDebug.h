#ifndef UG2_D3DX_EFFECT_DEBUG_H
#define UG2_D3DX_EFFECT_DEBUG_H
extern "C" {
int __stdcall D3DXEffectRegistryValue(unsigned int type,const char *name,void *data,unsigned int bytes); // 0x641539, RET 16
void __stdcall D3DXEffectDebugMute(unsigned int mute); // 0x671aed, RET 4
extern unsigned int ug2FxDebugRelease8A5CF8,ug2FxDebugRuntime8A5CF4;
extern int ug2FxDoNotMute80A170;
extern unsigned int ug2FxGetProcAddressImport783194,ug2FxLoadLibraryImport783198;
extern const char ug2FxD3D9Name7B72A4[],ug2FxD3D9DebugName7B7288[],ug2FxDebugMuteName7B7294[],ug2FxDoNotMuteName7B7278[],ug2FxRegistryPath7AB494[];
unsigned int __stdcall UG2FxGetModuleHandleA(const char *name);
unsigned int UG2FxLoadLibraryA(unsigned int dispatch,const char *name);
unsigned int UG2FxGetProcAddress(unsigned int dispatch,unsigned int module,const char *name);
int __stdcall UG2FxRegOpenKeyA(unsigned int key,const char *path,unsigned int *output);
int __stdcall UG2FxRegQueryValueExA(unsigned int key,const char *name,unsigned int *reserved,unsigned int *type,void *data,unsigned int *bytes);
int __stdcall UG2FxRegCloseKey(unsigned int key);
void UG2FxCallDebugMute(unsigned int callback,unsigned int mute);
}
#endif
