#include "Engine.h"
#include "VehicleMover.h"

extern float engineSpecMaximumZero7D7D3C;

// 0x591090: retain the extended maximum across the nine binary32 samples.
float EngineSpec::Maximum591090() {
    float maximum = engineSpecMaximumZero7D7D3C;
    if (maximum < samples10[0]) maximum = samples10[0];
    for (int i = 1; i < 9; ++i) if (maximum < samples10[i]) maximum = samples10[i];
    return maximum;
}

// 0x5928d0: the embedded record's engine-spec subobject, tail-called originally.
float DrivetrainAuxiliary::EngineMaximum5928D0() {
    return reinterpret_cast<EngineSpec *>(record0->engine1F0)->Maximum591090();
}
