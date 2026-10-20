#include "EngineSound.h"

// 0x45e1d0: retain the supplied position pointer, not an output copy.
void EngineSoundSpatialComponent::SetPosition45E1D0(float *position) {
    position70 = position;
}
