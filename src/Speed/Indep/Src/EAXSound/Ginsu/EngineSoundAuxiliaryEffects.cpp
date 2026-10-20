#include "EngineSoundPhysics.h"
#include "EngineSoundEnvelope.h"
extern float engineSoundShiftLevel789FB0;
extern float engineSoundHighLimit788600, engineSoundHighEnter7885EC, engineSoundHighExit7885F0;
extern float engineSoundHighRelease7885F4, engineSoundHighAttack7885F8, engineSoundHighScale7885FC;
extern EngineSoundRamp engineSoundHighCurve7F0050;

// 0x45c6c0: integer truncation is completed before the field write/addition.
void EngineSoundAuxiliary::ProduceLevel45C6C0(float duration) {
    (void)duration;
    EngineSoundShiftView *shift = shift6C;
    valueE0 = 0x7fff;
    if (shift->value78 != 0) {
        const int reduction = (int)((double)shift->valueF8 * engineSoundShiftLevel789FB0);
        valueE0 = (int)(0x7fffu - (unsigned int)reduction);
    }
    const int offset = (int)value140;
    valueE0 = (int)((unsigned int)valueE0 + (unsigned int)offset);
}

// 0x46f610: kind 2 bypasses even the envelope steps. Preserve the query
// boundaries, shared curve store/evaluation and the two ordered configurations.
void EngineSoundAuxiliary::ProduceEffects46F610(float duration) {
    if (state44->kind14 == 2) return;
    EngineSoundEnvelope *first = reinterpret_cast<EngineSoundEnvelope *>(&value104);
    EngineSoundEnvelope *second = reinterpret_cast<EngineSoundEnvelope *>(&valueE8);
    if (!flagE4) {
        if (shift6C->value78 == 0 && vtable0->value34(this) > engineSoundHighLimit788600) {
            flagE4 = 1;
            engineSoundHighCurve7F0050.value10 = (float)car68->value50;
            engineSoundHighCurve7F0050.Evaluate4599E0();
            const float curve = engineSoundHighCurve7F0050.value14;
            const int enter = (int)(((double)engineSoundHighEnter7885EC * value118) * curve);
            first->Configure459990(value118, ginsuOne784250 - engineSoundHighScale7885FC, enter, 0);
            const int attack = (int)(((ginsuOne784250 - (double)valueFC) * engineSoundHighAttack7885F8) * curve);
            second->Configure459990(valueFC, engineSoundHighScale7885FC, attack, 0);
            flag120 = 1;
            value124 = 0.0f;
        }
    } else {
        const double frequency = vtable0->value34(this);
        if ((frequency < engineSoundHighLimit788600 || shift6C->value78 != 0) && flagE4) {
            flagE4 = 0;
            const int exit = (int)((ginsuOne784250 - (double)value118) * engineSoundHighExit7885F0);
            first->Configure459990(value118, 1.0f, exit, 0);
            const int release = (int)((double)engineSoundHighRelease7885F4 * valueFC);
            second->Configure459990(valueFC, 0.0f, release, 0);
        }
    }
    first->Step46B100(duration);
    second->Step46B100(duration);
}
