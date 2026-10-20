#ifndef UG2_ENGINE_SOUND_SCHEDULE_H
#define UG2_ENGINE_SOUND_SCHEDULE_H
#include "EngineSound.h"

// Shared scheduling prefix observed in input and playback components.
struct EngineSoundScheduledNode;
struct EngineSoundScheduleVtable {
    void *unidentified0[5];
    void (__fastcall *update14)(EngineSoundScheduledNode *, void *, float);
    void (__fastcall *post18)(EngineSoundScheduledNode *);
    void *unidentified1C[2];
    void (__fastcall *update24)(EngineSoundScheduledNode *, void *, float);
    void (__fastcall *post28)(EngineSoundScheduledNode *);
    void *unidentified2C[2];
    void (__fastcall *post34)(EngineSoundScheduledNode *);
};
struct EngineSoundScheduledNode {
    EngineSoundScheduleVtable *vtable0;
    char unidentified4[0x48];
    unsigned int key4C;
    EngineSoundScheduledNode *next50;
};
UG2_CHECK_OFFSET(EngineSoundScheduledNode, next50, 0x50);
UG2_CHECK_OFFSET(EngineSoundScheduleVtable, update14, 0x14);
UG2_CHECK_OFFSET(EngineSoundScheduleVtable, post34, 0x34);
UG2_CHECK_OFFSET(EngineSoundCar, first1C, 0x1c);
UG2_CHECK_OFFSET(EngineSoundCar, first20, 0x20);
UG2_CHECK_OFFSET(EngineSoundCar, count2C, 0x2c);
UG2_CHECK_OFFSET(EngineSoundVtable, value48, 0x48);
void __fastcall EngineSoundNoop4022C0(void *receiver);
void __fastcall EngineSoundNoop4031C0(void *receiver, void *, float duration);
int EngineSoundApproachInt468E70(int current, int target, int step);
double __fastcall EngineSoundFraction5EC1F0(MoverModeOwner *owner);
extern float engineSoundSpeed78426C, engineSoundSpeedScale786460, engineSoundSpeedScale7875BC;
extern float engineSoundSpeedScale78AABC, engineSoundPacked789CF4;
extern float engineSoundValueOrigin784500, engineSoundValueScale78A290;
extern float engineSoundFraction784D28, engineSoundPostDelay787ADC, engineSoundPostScale78AAB8;
extern int engineSoundRaceActive89E7B0, engineSoundRaceCounts89E7C0[4];
#endif
