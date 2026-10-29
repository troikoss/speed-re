#include "CrtFloor.h"
// Original 0x727e6b..0x727e6d. Keep the direct user-matherr call boundary.
extern "C" int __cdecl D3DXCrtDefaultMathError(D3DXCrtMathException *) { return 0; }
