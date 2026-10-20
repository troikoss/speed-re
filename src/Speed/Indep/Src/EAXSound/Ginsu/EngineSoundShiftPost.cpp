#include "EngineSoundShift.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;
// 0x45c210: reload the current mix record for each packed result.
void EngineSoundShiftView::Post45C210() {
    int *words = reinterpret_cast<int *>(this);
    float *mix = *reinterpret_cast<float **>(reinterpret_cast<char *>(audio82B884) + 0x38);
    words[1] = (int)(mix ? ((double)mix[1] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
    mix = *reinterpret_cast<float **>(reinterpret_cast<char *>(audio82B884) + 0x38);
    words[2] = (int)(mix ? ((double)mix[2] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
    words[3] = engineSoundShiftPacked7ED344;
    int inverted = (int)(32767u - (unsigned int)engineSoundShiftPacked7ED344);
    if (inverted < 1) inverted = 0;
    else if (inverted > 32767) inverted = 32767;
    words[7] = inverted;
    mix = *reinterpret_cast<float **>(reinterpret_cast<char *>(audio82B884) + 0x38);
    words[4] = (int)(mix ? ((double)mix[4] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
    mix = *reinterpret_cast<float **>(reinterpret_cast<char *>(audio82B884) + 0x38);
    words[5] = (int)(mix ? ((double)mix[5] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
    mix = *reinterpret_cast<float **>(reinterpret_cast<char *>(audio82B884) + 0x38);
    words[6] = (int)(mix ? ((double)mix[6] * mix[0]) * engineSoundCurveInteger789CF4 : engineSoundCurveInteger789CF4);
}
