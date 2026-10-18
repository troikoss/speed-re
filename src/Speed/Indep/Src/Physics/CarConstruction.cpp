#include "CarConstruction.h"
#if defined(_MSC_VER) && _MSC_VER < 1400
// The verification compile has no system include path.
inline void *operator new(unsigned int, void *memory) throw() { return memory; }
inline void operator delete(void *, void *) throw() {}
#else
#include <new>
#endif

// Participant setup 0x6065b0, car creation 0x606480, car constructor 0x604260, physics
// binding 0x601110, car list rebuild 0x5f0d20, physics setup copy 0x413440 and the small
// leaves they reach. Unrecovered callees are declared in CarConstruction.h.

extern CarWorld *carWorld890080;          // 0x890080
extern MoverContext *participantRecords89E810; // 0x89e810
extern int participantCount89E814;        // 0x89e814
extern float raceValue89E7B8;             // 0x89e7b8; human participants start with kind 3 when positive
extern int raceType89E7B0;                // 0x89e7b0
extern unsigned char *carTypeInfoTable;   // 0x8a1ccc, rows of 0x890 bytes
extern int humanCarType8021B0;            // 0x8021b0
extern PhysicsUpgradeSelection *humanSelection8900E8; // 0x8900e8
extern void *carCollisionBodyPool86AD60;  // 0x86ad60
extern void *carNullMoverPool86B304;      // 0x86b304
extern int drivetrainClock;               // 0x86518c
extern float resetDurationDefault;        // 0x7a55c4
extern unsigned char replayActive89CF69;  // 0x89cf69
extern int replayState8669F4;             // 0x8669f4
extern int replayState866A14;             // 0x866a14
extern int audioEnabled7FAAA8;            // 0x7faaa8
extern Audio82B884 *audio82B884;          // 0x82b884
extern float physicsCurveSpacing7850B4;   // 0x7850b4

void *bOMalloc(void *pool);               // 0x440e50
void *AllocateCarMemory(unsigned int size); // allocation boundary 0x575620
void FreeCarMemory(void *memory);         // allocation boundary 0x575630

// The 64 drive-curve samples at PhysicsParameters +0x45c and the curve after them.
struct PhysicsCurveTable {
    float samples[64];
    EngineCurve curve;
    void Copy(const PhysicsCurveTable *source); // 0x4133d0
};

// Original 0x4133d0: copy 63 of the samples and rebuild the curve over this table.
void PhysicsCurveTable::Copy(const PhysicsCurveTable *source) {
    for (int i = 0; i < 0x3f; ++i) samples[i] = source->samples[i];
    EngineCurve rebuilt;
    rebuilt.count = 0x40;
    rebuilt.start = source->curve.start;
    rebuilt.end = source->curve.end;
    rebuilt.inverseSpacing = physicsCurveSpacing7850B4 / (rebuilt.end - rebuilt.start);
    rebuilt.samples = samples;
    curve = rebuilt;
}

// Original 0x59b060.
void PhysicsTuning::Bind(int carType, CarPhysicsRecord *record, PhysicsUpgradeSelection *selection) {
    value0 = carType;
    recordC = record;
    selection8 = selection;
}

// Original 0x413440: copy a physics setup, rebinding its curve and tuning to this copy.
CarPhysicsSetup &CarPhysicsSetup::operator=(const CarPhysicsSetup &source) {
    value0 = source.value0;
    value4 = source.value4;
    value5 = source.value5;
    value6 = source.value6;
    value7 = source.value7;
    parameters10 = source.parameters10;
    reinterpret_cast<PhysicsCurveTable *>(parameters10.curveSamples45C)
        ->Copy(reinterpret_cast<const PhysicsCurveTable *>(source.parameters10.curveSamples45C));
    parameters10.tuning374.Bind(parameters10.value0, &parameters10.record30, &parameters10.selection2E0);
    value580 = source.value580;
    value581 = source.value581;
    value584 = source.value584;
    value588 = source.value588;
    value58C = source.value58C;
    int i;
    for (i = 0; i < 0xaa; ++i) values590[i] = source.values590[i];
    for (i = 0; i < 0xaa; ++i) values838[i] = source.values838[i];
    value8E4 = source.value8E4;
    value8E8 = source.value8E8;
    return *this;
}

// Original 0x5930a0: the id the next collision body takes.
unsigned short CurrentCollisionId5930A0() {
    return (unsigned short)collisionIds86B2BC->next8;
}

// Original 0x5b5350: the record, composing first when the class index is unset.
CarPhysicsRecord *PhysicsParameters::GetRecord(bool compose) {
    if (index4 == -1 && compose) Refresh(1, 1, 1);
    return &record30;
}

// Original 0x639ce0.
void PhysicsInfo::Refresh(int secondary, int refresh, int flag, char select) {
    if (value0 == -1) return;
    SetupParts(secondary != 0, 0);
    parameters10.Refresh(refresh, flag, select);
}

// Original 0x5a93a0.
DrivetrainAuxiliary *DrivetrainAuxiliary::Construct(WCollider *collider, CarPhysicsRecord *record) {
    timer414 = 0;
    timer418 = 0;
    timer41C = 0;
    timer420 = 0;
    collider4 = collider;
    record0 = record;
    mode424 = 0;
    value3D0 = 0;
    value3D4 = 0;
    for (int i = 0; i < 4; ++i) {
        renderFlags3B4[i] = 1;
        rideOffsets3B8[i] = 0.0f;
        assisted3C8[i] = 0;
    }
    position20.x = 0.0f;
    position20.y = 0.0f;
    position20.z = 0.0f;
    origin10.x = 0.0f;
    origin10.y = 0.0f;
    origin10.z = 0.0f;
    timer414 = 0;
    Reset();
    return this;
}

// Original 0x5f0d20: list the cars by participant kind (1, then 2, then 3) and publish
// them in the per-index tables.
void CarWorld::RebuildCarList() {
    WCollisionLink *node = list0.next;
    count10 = 0;
    count14 = 0;
    count18 = 0;
    WCollisionLink *end = &list0; // 0x502fd0
    for (; node != end; node = node->next) {
        MoverModeOwner *car = reinterpret_cast<MoverModeOwner *>(node);
        if (car->context14->value4 == 1) cars1C[count10++] = car;
    }
    count14 = count10;
    for (node = list0.next; node != end; node = node->next) {
        MoverModeOwner *car = reinterpret_cast<MoverModeOwner *>(node);
        if (car->context14->value4 == 2) cars1C[count10++] = car;
    }
    count18 = count10;
    for (node = list0.next; node != end; node = node->next) {
        MoverModeOwner *car = reinterpret_cast<MoverModeOwner *>(node);
        if (car->context14->value4 == 3) cars1C[count10++] = car;
    }
    for (int i = 0; i < count10; ++i) {
        MoverModeOwner *car = cars1C[i];
        car->index8 = (short)i;
        carTable89CCF8[(short)i] = car;
        carMoverTypes89CD50[car->index8] = car->moverType28;
        carMovers89CDA8[car->index8] = car->mover34;
        carKinds89CE00[car->index8] = car->value24;
        carParticipants89CE58[car->index8] = car->participant2C;
        carInputs89CEB0[car->index8] = car->input30;
    }
}

// Original 0x601110: take a copy of the physics setup into the participant record and
// rebuild everything that depends on it.
void MoverModeOwner::SetPhysics(CarPhysicsSetup *setup) {
    MoverContext *record = context14;
    record->setup10 = *setup;
    record->carType908 = record->setup10.value0;
    physicsInfo10 = reinterpret_cast<PhysicsInfo *>(&context14->setup10);
    record20 = context14->setup10.parameters10.GetRecord(true);
    valueC = reinterpret_cast<char *>(carTypeInfoTable + physicsInfo10->value0 * 0x890);
    physicsInfo10->Refresh(context14->value4 == 2, 1, 1, 1);
    if (context14->value4 == 1 && !context14->flag900) {
        humanCarType8021B0 = physicsInfo10->value0;
        humanSelection8900E8 = &physicsInfo10->parameters10.selection2E0;
    }
    if (collisionBody3C) {
        delete collisionBody3C;
        collisionBody3C = 0;
    }
    void *memory = bOMalloc(carCollisionBodyPool86AD60);
    collisionBody3C = memory ? ::new (memory) CarCollisionBody(this, CurrentCollisionId5930A0()) : 0;
    if (forceFeedback958) {
        forceFeedback958->~CarFeedback958();
        FreeCarMemory(forceFeedback958);
        forceFeedback958 = 0;
    }
    if (*reinterpret_cast<int *>(valueC + 0x844) == 0) {
        void *feedback = AllocateCarMemory(sizeof(CarFeedback958));
        forceFeedback958 = feedback ? new (feedback) CarFeedback958(this) : 0;
    }
    if (effects554) DestroyRender();
    CreateRender(physicsInfo10);
    PlaceOnGround();
    int type = moverType28;
    SwitchMover(0);
    SwitchMover(type);
}

// Original 0x604260: the car constructor. A negative distance places the navigator by
// position only.
MoverModeOwner *MoverModeOwner::Construct(MoverContext *record, WheelVector *position, unsigned short heading,
                                          float distance) {
    DrivetrainAuxiliary *state = State();
    state->Construct(Collider(), record->setup10.parameters10.GetRecord(true));
    value548 = 0;
    value5B0 = 0;
    value5B4 = -1;
    value5B8 = 0;
    Collider()->Construct();
    Navigator()->Construct();
    tick92C = 0;
    value948 = 0;
    value960 = 0;
    resetTick974 = 0;
    value9B8 = 0;
    value9C0 = 0;
    shiftTick9D8 = 0;
    context14 = record;
    previousPosition510.x = 0.0f;
    previousPosition510.y = 0.0f;
    previousPosition510.z = 0.0f;
    previousVelocity520.x = 0.0f;
    previousVelocity520.y = 0.0f;
    previousVelocity520.z = 0.0f;
    WheelVector start;
    start.x = 0.0f;
    start.y = 0.0f;
    start.z = 0.0f;
    previousAngular530.x = 0.0f;
    previousAngular530.y = 0.0f;
    previousAngular530.z = 0.0f;
    if (position) {
        start = *position;
        previousPosition510 = *position;
    }
    if (distance < 0.0f) Navigator()->SetPosition(&start);
    else Navigator()->SetPosition(&start, distance, 0);
    if (context14->value4 == 3) value910 = 16.0f;
    state->position20 = start;
    state->heading3CC = heading;
    mask95C = 0;
    value95D = 0;
    value4D8 = 1.0f;
    value4DC = 0.0f;
    value4E0 = 0.0f;
    value4E4 = 0.0f;
    value964 = 0;
    valueA = record->value1;
    index8 = 0;
    raceEntry18 = 0;
    object1C = 0;
    physicsInfo10 = 0;
    record20 = 0;
    valueC = 0;
    void *memory = AllocateCarMemory(sizeof(CarDriver));
    CarDriver *driver = memory ? new (memory) CarDriver(this) : 0;
    participant2C = reinterpret_cast<RaceParticipant *>(driver);
    input30 = driver->input8;
    void *slot = bOMalloc(carNullMoverPool86B304);
    mover34 = reinterpret_cast<VehicleMover *>(slot ? new (slot) CarNullMover(state) : 0);
    memory = AllocateCarMemory(sizeof(CarTracker914));
    tracker914 = memory ? new (memory) CarTracker914() : 0;
    tracker914->Bind(&state->position20, &state->velocity30, &state->value40, &state->matrix50);
    tracker914->car1C = this;
    value24 = 0;
    moverType28 = 0;
    state38 = 0;
    collisionBody3C = 0;
    value9C4 = 0;
    value9D0 = 0;
    value9C8 = 0;
    value9CC = 0;
    gear434 = 1;
    gear4D0 = 1;
    value4D4 = 0;
    value540 = 0;
    effects554 = 0;
    forceFeedback958 = 0;
    resetRequested96C = 0;
    countdown970 = 0;
    value990 = state->position20.x;
    value994 = state->position20.y;
    effectsEnabled550 = 1;
    value551 = 1;
    RegisterUpdate5E02B0(EmptyRoutine4022C0, 0x10000);
    value940 = 0;
    value91C = 0;
    value94C = 0;
    value950 = 0;
    value954 = 0;
    tick92C = drivetrainClock;
    value93D = 1;
    value93C = 0;
    value926 = 0;
    value925 = 0;
    value927 = 0;
    value928 = 0;
    value944 = 0;
    value948 = 0;
    value920 = 0;
    resetTick974 = 0;
    value544 = 0;
    values930[0] = -1.0f;
    values930[1] = -1.0f;
    values930[2] = -1.0f;
    value548 = 0;
    contactTime54C = 0.0f;
    effectsSuppressed998 = 0;
    resetDuration978 = resetDurationDefault;
    value999 = 1;
    values99C[0] = 0;
    values99C[1] = 0;
    values99C[2] = 0;
    values99C[3] = 0;
    value9AC = 0;
    value9B0 = 0;
    value924 = 1;
    value97C = 0;
    values980[0] = 0;
    values980[1] = 0;
    values980[2] = 0;
    values980[3] = 0;
    value9D4 = 0;
    shiftTick9D8 = 0;
    value5C0 = 0;
    LoadColliderPacks();
    for (int i = 0; i < 4; ++i) {
        value4E8[i] = 0.0f;
        value4F8[i] = 0.0f;
    }
    value9C0 = 0;
    SetPhysics(&context14->setup10);
    recorder918 = 0;
    if (replayActive89CF69 || replayState8669F4 > 0 || replayState866A14 > 0) {
        memory = AllocateCarMemory(sizeof(CarRecorder918));
        recorder918 = memory ? new (memory) CarRecorder918(this) : 0;
    }
    replay968 = 0;
    if (audioEnabled7FAAA8) replay968 = audio82B884->CreateCarSound(this);
    PlaceOnGround();
    return this;
}

// Original 0x606480: create a participant's car at a grid entry (or at another car's state),
// give it its driver and mover, and append it to the world list.
void CreateParticipantCar(StartingPosition *start, MoverContext *record, int kind, DrivetrainAuxiliary *source) {
    unsigned short heading = start->angle10;
    reinterpret_cast<PhysicsInfo *>(&record->setup10)->Refresh(record->value4 == 2, 1, 1, 1);
    WRoadNav navigator;
    navigator.Construct(start);
    void *memory = AllocateCarMemory(sizeof(MoverModeOwner));
    MoverModeOwner *car;
    if (source) {
        car = memory ? static_cast<MoverModeOwner *>(memory)->Construct(record, &source->position20, source->heading3CC,
                                                                        navigator.value24)
                     : 0;
    } else {
        car = memory ? static_cast<MoverModeOwner *>(memory)->Construct(
                           record, reinterpret_cast<WheelVector *>(start->position0), heading, navigator.value24)
                     : 0;
    }
    car->SetParticipantKind(kind);
    if (kind == 3) {
        car->SwitchMover(SelectMover5F4400(car));
        if (!raceType89E7B0) car->Prepare5F6E20();
    } else if (kind == 1) {
        car->SwitchMover(kind);
    } else {
        car->SwitchMover(0);
    }
    CarWorld *world = carWorld890080;
    WCollisionLink *last = world->list0.previous;
    last->next = &car->link0;
    world->list0.previous = &car->link0;
    car->link0.next = &world->list0;
    car->link0.previous = last;
    navigator.Destroy();
}

// Original 0x6065b0: create the cars of the participant records in grid order 1..6, then
// the kind-3 entries of world +0x118, rebuilding the world's car list after each pass.
void SetupParticipants() {
    StartingPosition grid[6];
    GetStartingPositions(6, grid);
    StartingPosition *start = grid;
    for (int index = 1; index <= 6; ++index, ++start) {
        MoverContext *record = participantRecords89E810;
        for (int i = 0; i < participantCount89E814; ++i, ++record) {
            if (record->state0 > 0 || record->gridIndex3 != index) continue;
            if (record->flag900) {
                CreateParticipantCar(start, record, 0, 0);
            } else if (record->value4 == 1) {
                int kind = raceValue89E7B8 > 0.0f ? 3 : 1;
                if (record->hasPlacement91C) {
                    StartingPosition placement;
                    placement.position0[0] = record->placement920.x;
                    placement.position0[1] = record->placement920.y;
                    placement.position0[2] = record->placement920.z;
                    placement.angle10 = record->heading930;
                    CreateParticipantCar(&placement, record, kind, 0);
                } else {
                    CreateParticipantCar(start, record, kind, 0);
                }
            } else if (record->value4 == 2) {
                CreateParticipantCar(start, record, 3, 0);
            }
            break;
        }
    }
    carWorld890080->RebuildCarList();
    for (int i = 0; i < participantCount89E814; ++i) {
        MoverContext *record = &participantRecords89E810[i];
        if (record->value4 != 3) continue;
        CarWorld *world = carWorld890080;
        WCollisionLink *entry = world->value118->CreateEntry42F7A0(record);
        if (entry) {
            WCollisionLink *last = world->list0.previous;
            last->next = entry;
            world->list0.previous = entry;
            entry->previous = last;
            entry->next = &world->list0;
        }
    }
    carWorld890080->RebuildCarList();
}
