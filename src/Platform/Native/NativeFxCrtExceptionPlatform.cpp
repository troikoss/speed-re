#include "../../Speed/Indep/Libs/D3DX/CrtExceptions.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// Actual Win32 RaiseException boundary (IAT 0x7832a4). The recovered CRT
// constructs and decodes its record, but Linux SEH delivery is not recovered.
// Keep this in a separate object so the analysis probe can supply a test handler.
extern "C" void __stdcall UG2FxCrtOsRaiseException(unsigned int,unsigned int,unsigned int,unsigned int *) { abort(); }
