#include "../../Speed/Indep/Libs/D3DX/EffectPreparation.h"
extern "C" void __cdecl abort(void);
// Shader GetDevice/GetFunction require an actual GPU shader implementation.
// CPU metadata probes never enter these unsupported interface methods.
extern "C" int __stdcall UG2FxShaderDevice(unsigned int,unsigned int,unsigned int *) { abort();return static_cast<int>(0x80004005); }
extern "C" int __stdcall UG2FxShaderFunction(unsigned int,unsigned int,void *,unsigned int *) { abort();return static_cast<int>(0x80004005); }
