#include "../../Speed/Indep/Libs/D3DX/CrtFloor.h"
// Native thread-storage ownership for the recovered errno accessor. Only the
// original +8 field is provided; this does not recover Win32 CRT thread setup.
namespace { struct NativeCrtThread { unsigned int value00,value04;int value08; };__thread NativeCrtThread threadState; }
extern "C" void *__cdecl UG2FxCrtThreadData() { return &threadState; }
