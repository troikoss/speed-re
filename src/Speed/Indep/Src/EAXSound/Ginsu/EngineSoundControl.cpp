#include "EngineSound.h"

// 0x461490: dependency IDs in virtual slot +0x0c of the engine table.
int EngineSound::Dependency461490(int index) {
    if ((unsigned int)index > 3u) return -1;
    return index + 5;
}

// 0x4614e0: query the supplied descriptor before choosing the destination.
// No guard is introduced for the component, descriptor or unknown IDs.
void EngineSound::Attach4614E0(EngineSoundComponent *component) {
    const unsigned int descriptor = *component->vtable0->descriptor4(component);
    switch ((descriptor >> 4) & 0xfffu) {
    case 5: auxiliaryB8 = reinterpret_cast<EngineSoundAuxiliary *>(component); break;
    case 6: controlC0 = reinterpret_cast<EngineSoundControl *>(component); break;
    case 7: valueC4 = reinterpret_cast<int>(component); break;
    case 8: valueB4 = reinterpret_cast<int>(component); break;
    }
}

// 0x45c530 / 0x45c540: return through ST0 without an extra store.
float EngineSoundAuxiliary::Value45C530() { return value128; }
float EngineSoundAuxiliary::Value45C540() { return value130; }

// 0x45d190: the integer operands are loaded exactly by FILD/FIMUL before
// multiplication, rather than first rounded to binary32. Each conversion
// completes before the following field is stored, as in the original.
void EngineSoundControl::Scale45D190() {
    EngineSoundAuxiliary *engine = engine6C;
    const int scaledB8 = (int)(engine->valueFC * (double)valueB8);
    const int saved7C = value7C;
    valueB8 = scaledB8;
    const int scaled7C = (int)((double)saved7C * engine->value118);
    const int saved80 = value80;
    value7C = scaled7C;
    const int scaled80 = (int)((double)saved80 * engine->value118);
    const int saved84 = value84;
    value80 = scaled80;
    value84 = (int)((double)saved84 * engine->value118);
}

// 0x47c3d0: outer HybridMotor update, retaining calls to recovered prepare,
// mode gain producers and conversion tail.
void EngineSoundControl::Update47C3D0(float duration) {
    if (active58) {
        delta60 = duration;
        time5C = duration + time5C;
    }
    Prepare45D0A0();
    // FST rounds a copy for the upper clamp, while the first comparison
    // still uses the value returned in ST0. Keep those two values distinct.
    const double observed = engine6C->vtable0->value34(engine6C);
    volatile float rounded = (float)observed;
    float value = engineControlMinimum784550;
    if (observed > engineControlMinimum784550) {
        value = rounded;
        if (engineControlMaximum784500 < rounded) value = engineControlMaximum784500;
    }
    EngineSoundCarValue *record = car68->value80;
    frequencyF8 = (value - engineControlMinimum784550) * engineControlScale78A290 *
                  (record->value1C - record->value20) + record->value20;
    if (car68->mode78) ModeTwo470540(duration);
    else ModeOne4701A0(duration);
    Scale45D190();
}
