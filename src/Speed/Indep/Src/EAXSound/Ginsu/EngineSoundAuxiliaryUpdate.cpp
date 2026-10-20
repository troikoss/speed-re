#include "EngineSound.h"
#include "EngineSoundEnvelope.h"
#include "../../Physics/VehicleMover.h"
#include "../../Physics/WheelMath.h"
extern float engineSoundPositionScale784554;

// 0x47c010: preserve direct/virtual boundaries and reload receivers after
// spatial orientation dispatch. Position/normalization uses rounded products.
void EngineSoundAuxiliary::Update47C010(float duration) {
    if (active58) {
        *reinterpret_cast<unsigned int *>(&delta60) = *reinterpret_cast<unsigned int *>(&duration);
        time5C = duration + time5C;
    }
    spatial78->vtable0->value30(spatial78, 0, reinterpret_cast<char *>(state44->car30) + 0x90);
    const float *position = reinterpret_cast<const float *>(reinterpret_cast<char *>(state44->car30) + 0x60);
    *reinterpret_cast<unsigned int *>(&value80) = *reinterpret_cast<const unsigned int *>(position);
    value84 = position[1];
    value88 = position[2];
    const float *orientation = reinterpret_cast<const float *>(reinterpret_cast<char *>(state44->car30) + 0x90);
    WheelVector input, normalized;
    *reinterpret_cast<unsigned int *>(&input.x) = *reinterpret_cast<const unsigned int *>(orientation);
    input.y = orientation[1];
    input.z = orientation[2];
    WheelNormalize(&normalized, &input);
    volatile float x = normalized.x * engineSoundPositionScale784554;
    volatile float y = normalized.y * engineSoundPositionScale784554;
    volatile float z = normalized.z * engineSoundPositionScale784554;
    value80 = x + value80;
    value84 = value84 + y;
    value88 = value88 + z;
    reinterpret_cast<EngineSoundEnvelope *>(&value1D0)->Step46B100(duration);
    ProduceIdle46F9D0();
    vtable0->value3C(this, 0, duration);
    ProduceBackfire46F580(duration);
    vtable0->value24(this, 0, duration);
    vtable0->value28(this, 0, duration);
    vtable0->value2C(this, 0, duration);
    ProduceCutoff46F460();
    vtable0->value30(this, 0, duration);
}
