#include "EngineSound.h"
#include "EngineSoundEnvelope.h"
#include "../../Physics/VehicleMover.h"
#include "../../Physics/Engine.h"
#include "../../../Libs/bWare/Random.h"
extern float engineSoundOffsetCycle789FB4;
extern float engineSoundBackfireBase7885E4, engineSoundBackfireScale7885E8;
extern float engineSoundCutoffRange7885DC, engineSoundCutoffSpecial7885E0;
extern float engineSoundCutoffOrigin78AAA8, engineSoundCutoffPosition78AAA4;
extern float engineSoundCutoffLow7885D4, engineGainCutoffMaximum7885D8;
extern float engineGainTwoScale78A6C8;

// 0x45c7a0: all period clamps occur before the three ordered sine calls.
void EngineSoundAuxiliary::ProduceOffsets45C7A0(float duration) {
    int level = 0, throttle = 0, rpm = 0;
    int levelPeriod = 0, throttlePeriod = 0, rpmPeriod = 0;
    EngineSoundShiftView *shift = shift6C;
    if (shift && shift->value78 != 0) {
        level = shift->value80;
        throttle = shift->value88;
        rpm = shift->value90;
        levelPeriod = shift->value84;
        throttlePeriod = shift->value8C;
        rpmPeriod = shift->value94;
    } else {
        value140 = 0.0f;
        value150 = 0x4097;
        value144 = 0.0f;
        value14C = 0x4097;
        value148 = 0.0f;
        value14E = 0x4097;
    }
    if (rpmPeriod < 1) rpmPeriod = 1;
    else if (rpmPeriod > 10000) rpmPeriod = 10000;
    if (throttlePeriod < 1) throttlePeriod = 1;
    else if (throttlePeriod > 10000) throttlePeriod = 10000;
    if (levelPeriod < 1) levelPeriod = 1;
    else if (levelPeriod > 10000) levelPeriod = 10000;
    if (rpm) {
        const int step = (int)(((double)duration / ((double)rpmPeriod * engineSoundSequenceMilliseconds78435C)) * engineSoundOffsetCycle789FB4);
        value14C = (unsigned short)((unsigned short)(value14C - (unsigned short)step) % 65535u);
        value144 = (float)(EngineSineUnits(value14C) * (double)rpm);
    }
    if (throttle) {
        const int step = (int)(((double)duration / ((double)throttlePeriod * engineSoundSequenceMilliseconds78435C)) * engineSoundOffsetCycle789FB4);
        value14E = (unsigned short)((unsigned short)(value14E - (unsigned short)step) % 65535u);
        value148 = (float)(EngineSineUnits(value14E) * (double)throttle);
    }
    if (level) {
        const int step = (int)(((double)duration / ((double)levelPeriod * engineSoundSequenceMilliseconds78435C)) * engineSoundOffsetCycle789FB4);
        value150 = (unsigned short)((unsigned short)(value150 - (unsigned short)step) % 65535u);
        value140 = (float)(EngineSineUnits(value150) * (double)level);
    }
}

// 0x46f580: step first, then replace the sequence with two pulse segments.
void EngineSoundAuxiliary::ProduceBackfire46F580(float duration) {
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 8) = 0;
    sequence154.Step46AF70(duration);
    if (flag152) {
        const int milliseconds = (int)bRandom(100) + 25;
        const float peak = (float)((double)bRandomFloat(engineSoundBackfireScale7885E8) + engineSoundBackfireBase7885E4);
        sequence154.Reset4598C0();
        sequence154.Append4598F0(0.0f, peak, milliseconds, 6);
        sequence154.Append4598F0(peak, 0.0f, milliseconds, 6);
        flag152 = 0;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 8) = 1;
    }
}

// 0x46f460: keep the spatial integer exact through subtraction/division,
// round the fraction, then clamp and apply the original addressed curves.
void EngineSoundAuxiliary::ProduceCutoff46F460() {
    float range = engineSoundCutoffRange7885DC;
    MoverModeOwner *owner = state44->car30;
    MoverContext *context = owner->context14;
    if (context->value4 == 1 && !context->flag900 && owner->raceEntry18) {
        const int mode = *reinterpret_cast<int *>(reinterpret_cast<char *>(owner->raceEntry18) + 0x58);
        if (mode == 6 || mode == 7) range = engineSoundCutoffSpecial7885E0;
    }
    const int spatial = *reinterpret_cast<int *>(reinterpret_cast<char *>(spatial78) + 8);
    volatile float raw = (float)(((double)spatial - engineSoundCutoffOrigin78AAA8) / range);
    float fraction = ginsuZero7D7D3C;
    if (raw > ginsuZero7D7D3C) {
        fraction = raw;
        if (ginsuOne784250 < raw) fraction = ginsuOne784250;
    }
    const int position = (int)((double)fraction * engineSoundCutoffPosition78AAA4);
    int curve = (int)((unsigned int)EngineSoundCurve468160(4, position, 1) << 1);
    if (curve < -9600) curve = -10000;
    const int inverse = EngineSoundCurveInverse467FF0(curve);
    const int cutoff = (int)(((double)inverse * engineGainTwoScale78A6C8) *
                            ((double)engineGainCutoffMaximum7885D8 - engineSoundCutoffLow7885D4) + engineSoundCutoffLow7885D4);
    value138 = cutoff;
    if (cutoff < 1) value138 = 0;
    else if (cutoff > 0x7fff) value138 = 0x7fff;
}
