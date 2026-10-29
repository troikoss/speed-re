#include "../../Speed/Indep/Libs/D3DX/CrtPowerSse.h"
// Numeric service retained for the private x87-to-SSE wrapper. The caller
// performs the original final FLD; the recovered body writes raw double bits.
extern "C" void __cdecl UG2FxCrtPowerSseBody(double *out,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second) {
    D3DXCrtPowerSseResult(out,first,second);
}
