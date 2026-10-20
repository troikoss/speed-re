#include "EngineSoundSchedule.h"
#include "EngineSoundPhysics.h"
#include "../../Physics/VehicleMover.h"
#include "../../Hud/HudPlayerMasks.h"
extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)
unsigned int bRandom(unsigned int maximum);

// 0x5ec1f0: each nonzero byte contributes the addressed fraction, in order.
double __fastcall EngineSoundFraction5EC1F0(MoverModeOwner *owner) {
    unsigned char *bytes = reinterpret_cast<unsigned char *>(owner);
    double value = bytes[0x390] ? engineSoundFraction784D28 : ginsuZero7D7D3C;
    if (bytes[0x391]) value += engineSoundFraction784D28;
    if (bytes[0x392]) value += engineSoundFraction784D28;
    if (bytes[0x393]) value += engineSoundFraction784D28;
    return value;
}
// 0x468e70: x86 addition/subtraction wrap before the signed comparisons.
int EngineSoundApproachInt468E70(int current, int target, int step) {
    const int high = (int)((unsigned int)current + (unsigned int)step);
    if (target > high) return high;
    const int low = (int)((unsigned int)current - (unsigned int)step);
    if (target < low) return low;
    return target;
}
static int Packed(int value) {
    if (value < 1) return 0;
    if (value > 32767) return 32767;
    return value;
}

// 0x470a60: the owner's speed is reloaded for each packed output. Arithmetic
// remains extended up to each original integer conversion boundary.
void EngineSoundPhysicsView::Post470A60() {
    int *words = reinterpret_cast<int *>(this);
    EngineSoundCar *state = state44;
    words[1] = Packed((int)((fabs((double)*reinterpret_cast<float *>(reinterpret_cast<char *>(state->car30) + 0x430) * engineSoundSpeed78426C) * engineSoundSpeedScale786460) * engineSoundPacked789CF4));
    words[2] = Packed((int)((fabs((double)*reinterpret_cast<float *>(reinterpret_cast<char *>(state->car30) + 0x430) * engineSoundSpeed78426C) * engineSoundSpeedScale7875BC) * engineSoundPacked789CF4));
    words[3] = Packed((int)((fabs((double)*reinterpret_cast<float *>(reinterpret_cast<char *>(state->car30) + 0x430) * engineSoundSpeed78426C) * engineGainPercent78425C) * engineSoundPacked789CF4));
    words[4] = Packed((int)((fabs((double)*reinterpret_cast<float *>(reinterpret_cast<char *>(state->car30) + 0x430) * engineSoundSpeed78426C) * engineSoundSpeedScale78AABC) * engineSoundPacked789CF4));
    words[5] = Packed((int)((((double)engineSoundValueOrigin784500 - car68->value48) * engineSoundValueScale78A290) * engineSoundPacked789CF4));
    words[6] = EngineSoundFraction5EC1F0(state->car30) < ginsuOne784250 ? 1 : 0;
    const int mode = *reinterpret_cast<int *>(reinterpret_cast<char *>(car68) + 0xa8);
    int target = 0;
    if (mode == 0 || mode == 1 || mode == 5) target = 32000;
    else if (mode == 4) target = 4000;
    const int high = (int)((unsigned int)words[7] + 16383u);
    if (target > high) target = high;
    else {
        const int low = (int)((unsigned int)words[7] - 16383u);
        if (target < low) target = low;
    }
    words[7] = target;
    words[9] = words[8] = 0;
    state = state44;
    PlayerRecord *player = reinterpret_cast<PlayerRecord *>(state->car30->raceEntry18);
    if (player && (player->state58 == 6 || player->state58 == 7)) {
        if (player->camera238 && player->camera238->kindC == 1) words[9] = 1;
        else words[8] = 1;
    }
    if (!engineSoundRaceActive89E7B0) { words[10] = 0; return; }
    const int c8 = engineSoundRaceCounts89E7C0[2], cc = engineSoundRaceCounts89E7C0[3];
    const int c4 = engineSoundRaceCounts89E7C0[1];
    const int total = (int)((unsigned int)c8 + (unsigned int)cc + (unsigned int)c4 + (unsigned int)engineSoundRaceCounts89E7C0[0]);
    if (total == 1) { words[10] = 0; return; }
    if (engineSoundMode890118) {
        signed char *participant = *reinterpret_cast<signed char **>(reinterpret_cast<char *>(state->car30) + 0x1c);
        const int last = (int)((unsigned int)*reinterpret_cast<int *>(reinterpret_cast<char *>(engineSoundMode890118) + 0xbe8) - 1u);
        if ((int)participant[9] == last) {
            const int count = (int)((unsigned int)c8 - (unsigned int)(int)participant[10] + (unsigned int)cc + (unsigned int)c4 + (unsigned int)engineSoundRaceCounts89E7C0[0]);
            const int value = (int)(((double)count / total) * engineSoundPostScale78AAB8);
            words[10] = EngineSoundApproachInt468E70(words[10], value, 491);
        }
    }
}

// 0x470940: deferred pulse request and packed ramp, with unsigned 16-bit
// countdown semantics. The two unordered comparisons follow the x87 tests.
void EngineSoundControl::Post470940() {
    bool active = false;
    const double speed = (double)*reinterpret_cast<float *>(reinterpret_cast<char *>(car68->car30) + 0x430) * engineSoundSpeed78426C;
    float *last = reinterpret_cast<float *>(reinterpret_cast<char *>(this) + 0x78);
    unsigned short *remaining = reinterpret_cast<unsigned short *>(reinterpret_cast<char *>(this) + 0xc2);
    if (!(speed > engineSoundStep784478) || !(fabs((double)averageC4.averageC) < engineSoundStep784478))
        *reinterpret_cast<unsigned int *>(last) = *reinterpret_cast<unsigned int *>(&time5C);
    else if ((double)*last + engineSoundPostDelay787ADC < time5C) {
        active = true;
        if (*remaining == 0) {
            *remaining = (unsigned short)(bRandom(150) + 60);
            engine6C->flag152 = 1;
        }
        *remaining = (unsigned short)(*remaining - 1);
    }
    int *word = reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 4);
    const int high = (int)((unsigned int)*word + 983u);
    const int target = active ? 32767 : 0;
    if (target > high) *word = high;
    else {
        const int low = (int)((unsigned int)*word - 196u);
        *word = target < low ? low : target;
    }
}
