#ifndef UG2_WORLD_UPDATE_H
#define UG2_WORLD_UPDATE_H
#include "VehicleMover.h"
#include "Collision/Collision.h"

// The world object at *0x890080. Other recovered code views the same object as
// CollisionEventOwner (+0/+8), CameraCarList (+0x10/+0x1c) and DrivetrainRandom (+0xf8).
struct WorldCarPosition {
    WheelVector position;
    float unused;
};

struct WorldSubsystem118 {
    void Update42F910(float duration); // 0x42f910, not recovered
    WCollisionLink *CreateEntry42F7A0(struct MoverContext *record); // 0x42f7a0, not recovered (kind-3 records)
};

struct CarWorld {
    WCollisionLink list0;   // ordered by 0x43e650 with comparator 0x5ea240
    WCollisionLink events8;
    int count10;            // cars at +0x1c
    int count14;            // leading cars whose positions are published at +0x80
    int count18;
    MoverModeOwner *cars1C[0x19];
    WorldCarPosition positions80[6]; // unused entries hold 9999999
    float valuesE0[6];      // car +0x8fc of the leading cars
    char unidentifiedF8[0x20];
    WorldSubsystem118 *value118;
    void Update(float duration); // 0x609ce0
    float NearestPlayerDistance(WheelVector *point); // 0x5f1160
    void Prepare5FA770(); // 0x5fa770, not recovered
    void Sort43E650(int (*compare)(void *, void *)); // 0x43e650, not recovered
    void RebuildCarList(); // 0x5f0d20
};
UG2_CHECK_OFFSET(CarWorld, count10, 0x10);
UG2_CHECK_OFFSET(CarWorld, cars1C, 0x1c);
UG2_CHECK_OFFSET(CarWorld, positions80, 0x80);
UG2_CHECK_OFFSET(CarWorld, valuesE0, 0xe0);
UG2_CHECK_OFFSET(CarWorld, value118, 0x118);

// Objects with unrecovered per-frame updates.
struct WorldObject81CBC0 { void Update4268A0(); }; // 0x4268a0
// The world object at 0x8357e0: +8 is the sentinel of a list of registration nodes (see
// CollisionWorldObject.cpp), +0x50 their count.
struct WorldRegistrationNode;
struct WorldRegistrationLink {
    WorldRegistrationNode *next;
    WorldRegistrationNode *previous;
};
struct WorldObject8357E0 {
    char unidentified0[8];
    WorldRegistrationLink list8;
    char unidentified10[0x40];
    int count50;
    void Update492E10(float duration); // 0x492e10
    void Register48BC50(WorldRegistrationNode *node); // 0x48bc50
};
UG2_CHECK_OFFSET(WorldObject8357E0, list8, 0x8);
UG2_CHECK_OFFSET(WorldObject8357E0, count50, 0x50);
struct WorldObject827B88 { void Update43AF20(float duration); }; // 0x43af20
struct WorldObject88EB00 { void Update5E86E0(); }; // 0x5e86e0

void UpdateCars5FC0E0(); // 0x5fc0e0, not recovered
void Update614820(); // 0x614820, not recovered
void Update59E350(); // 0x59e350, not recovered
void UpdateRace572E50(int index, WheelVector *acceleration); // 0x572e50, not recovered
void Update605780(); // 0x605780, not recovered
void Update5E0A00(); // 0x5e0a00, not recovered
void Update5E5430(); // 0x5e5430, not recovered
void Update638070(); // 0x638070, not recovered
int CompareWorldEntry5EA240(void *first, void *second); // 0x5ea240, not recovered

// Per-index car tables (22 entries each) filled by 0x5f0d20 and 0x5eba30.
extern MoverModeOwner *carTable89CCF8[22];
extern int carMoverTypes89CD50[22];
extern struct VehicleMover *carMovers89CDA8[22];
extern int carKinds89CE00[22];
extern RaceParticipant *carParticipants89CE58[22];
extern MoverInput *carInputs89CEB0[22];

#endif
