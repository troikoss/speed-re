#include "EngineSoundMaster.h"
#include "EngineSoundPhysics.h"
#include "EngineSoundEnvelope.h"
#include "EngineSoundShift.h"
#include "EngineSoundSchedule.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;
// 0x471970: global MasterVol input; duration is popped but never read.
void EngineSoundMaster::Update471970(float duration) {
    (void)duration;
    Audio82B884 *audio = audio82B884;
    if (audio->mix38) {
        const int current = (int)((double)audio->mix38[0] * engineSoundCurveInteger789CF4);
        const int complement = (int)(32767u - (unsigned int)current);
        const double scale = (double)EngineSoundCurve468160(0, complement, 0) * engineSoundMasterScale78A6C8;
        float *mix = audio->mix38;
        values4[0] = (int)((ginsuOne784250 - ((double)mix[2] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[1] = (int)((ginsuOne784250 - ((double)mix[3] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[2] = (int)((ginsuOne784250 - ((double)mix[1] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[3] = (int)((ginsuOne784250 - ((double)mix[4] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[4] = (int)((ginsuOne784250 - ((double)mix[6] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[5] = (int)((ginsuOne784250 - ((double)mix[5] * mix[0]) * scale) * engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[8] = (int)(mix ? ((double)mix[2] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[9] = (int)(mix ? ((double)mix[3] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[10] = (int)(mix ? ((double)mix[1] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[11] = (int)(mix ? ((double)mix[4] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[12] = (int)(mix ? ((double)mix[6] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        mix = audio82B884->mix38;
        values4[13] = (int)(mix ? ((double)mix[5] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
        if (!engineSoundMasterFlag89E7D8 && !engineSoundMasterMode890108 &&
            !engineSoundMasterFlag89E7D9 && !engineSoundMasterMode890104 &&
            (!engineSoundMasterOwner82B9C8 || engineSoundMasterOwner82B9C8[0x98 / 4])) values4[14] = values4[1];
        else values4[14] = 32767;
        audio = audio82B884;
        engineSoundShiftPacked7ED344 = values4[14];
    }
    bool paused = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(audio) + 0x13) != 0;
    if (!paused && engineSoundMode8900D8) {
        if (*engineSoundMode8900D8 == 1) paused = true;
        else {
            if (*engineSoundMode8900D8 == 7 && engineSoundModes827E88.Is435950()) paused = true;
            if (!paused && engineSoundMode8900D8 && *engineSoundMode8900D8 == 5 &&
                engineSoundMasterReplay8650B8 && engineSoundMasterReplay8650B8[0x14 / 4]) paused = true;
        }
    }
    values4[6] = paused ? 1 : 0;
    const unsigned char frontend = EngineSoundFrontend4FE4A0();
    values4[15] = frontend && engineSoundMasterFrontend8383A4 &&
        *engineSoundMasterFrontend8383A4 >= 30 && *engineSoundMasterFrontend8383A4 <= 34 ? 1 : 0;
    values4[7] = engineSoundRaceActive89E7B0 == 8 && engineSoundMode890118 &&
        *engineSoundMode890118 == 4 && !engineSoundModes827E88.value40 &&
        !engineSoundModes827E88.value28 && !engineSoundModes827E88.value10 ? 1 : 0;
}
