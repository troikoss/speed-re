#ifndef UG2_CAR_CONSTRUCTION_H
#define UG2_CAR_CONSTRUCTION_H
#include "WorldUpdate.h"
#include "../World/TrackPositionMarkers.h"
#include "../Audio/AudioSystem.h"

// Car construction: participant setup 0x6065b0, car creation 0x606480, the car
// constructor 0x604260 and its physics binding 0x601110.

// Placeholder at car +0x34 until the first mover switch: slot pool *0x86b304, vtable 0x7a5bb0.
class CarNullMover {
public:
    CarNullMover(DrivetrainAuxiliary *state) : input4(0), state8(state) {}
    virtual void Slot00(); // the vtable's slots are not recovered
    MoverInput *input4;
    DrivetrainAuxiliary *state8;
};

// Base driver, 0x54 bytes, vtable 0x7a5ca8 (constructor 0x5eb930); +8 is its input object.
struct CarDriver {
    CarDriver(MoverModeOwner *car); // 0x5eb930, not recovered
    char unidentified0[8];
    MoverInput *input8;
    char unidentifiedC[0x48];
};

// Car +0x914, 0x2c bytes (constructor 0x63b590): registered by id in the table at 0x89e970
// and in the list at 0x8a2ca4; +0xc..+0x18 point at the car's position, velocity,
// acceleration and matrix.
struct CarTracker914 {
    CarTracker914(); // 0x63b590, not recovered
    void Bind(WheelVector *position, WheelVector *velocity, WheelVector *acceleration,
              WheelMatrix *matrix); // 0x60bcf0, not recovered
    char unidentified0[0x1c];
    MoverModeOwner *car1C;
    char unidentified20[0xc];
};

// Car +0x918, 0x240 bytes, created when a replay is active or pending.
struct CarRecorder918 {
    CarRecorder918(MoverModeOwner *car); // 0x5fcb00, not recovered
    char unidentified0[0x240];
};

// Car +0x958, 0x170 bytes, created when the car type row +0x844 is zero.
struct CarFeedback958 {
    CarFeedback958(MoverModeOwner *car); // 0x63a360, not recovered
    ~CarFeedback958(); // 0x63a440, not recovered
    char unidentified0[0x170];
};

void EmptyRoutine4022C0(); // 0x4022c0
int RegisterUpdate5E02B0(void (*update)(), int flags); // 0x5e02b0, not recovered
unsigned short CurrentCollisionId5930A0(); // 0x5930a0
int SelectMover5F4400(MoverModeOwner *car); // 0x5f4400, not recovered

// 0x606480 takes start in ecx and record in edx (a compiler-chosen convention).
void CreateParticipantCar(StartingPosition *start, MoverContext *record, int kind,
                          DrivetrainAuxiliary *source); // 0x606480
void SetupParticipants(); // 0x6065b0

#endif
