#include "EngineSound.h"
#include "../../Physics/VehicleMover.h"
extern float engineSoundIdleLimit7885D0;

// 0x46f9d0: TEST AH,0x41 accepts unordered as well as <=. Keep the virtual
// return extended and evaluate all three gates before the query.
void EngineSoundAuxiliary::ProduceIdle46F9D0() {
    MoverContext *context = state44->car30->context14;
    if (context->value4 == 1 && context->flag900 == 0 && accel70->valueC8 == 0) {
        const double frequency = vtable0->value34(this);
        if (!(frequency > engineSoundIdleLimit7885D0)) { flag13C = 1; return; }
    }
    flag13C = 0;
}
