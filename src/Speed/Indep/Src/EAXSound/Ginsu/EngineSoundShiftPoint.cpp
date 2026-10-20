#include "../../Physics/VehicleMover.h"
#include "../../Physics/Drivetrain.h"
extern float ginsuZero7D7D3C;
// 0x5947b0: no introduced gear bound.
float Drivetrain::AudioUpshift5947B0(int gear) { return upshift84[gear]; }
// 0x5ecd30: preserve the separate kind-1 call/tail boundary.
float MoverModeOwner::AudioShiftPoint5ECD30(int gear) {
    if (moverType28 == 1) return mover34->drivetrain4C->AudioUpshift5947B0(gear);
    if (moverType28 == 3) {
        char *value = *reinterpret_cast<char **>(reinterpret_cast<char *>(mover34) + 0x24);
        return *reinterpret_cast<float *>(value + 0x19c + gear * 4);
    }
    return ginsuZero7D7D3C;
}
