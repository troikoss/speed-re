#include "WorldUpdate.h"
#include "../Hud/HudPlayerMasks.h"
#include "../Frontend/PauseMenu.h"

// World update 0x609ce0 and the two small car/world queries it reaches every
// frame. Calls to the shared empty routine 0x4022c0 (five free calls and one on
// the object at 0x8357e0) are not reproduced.

extern CarWorld *carWorld890080;      // 0x890080
extern float engineSubstepDuration;   // 0x7fc800
extern int gameMode89008C;            // 0x89008c
extern int playerCount8900BC;         // 0x8900bc
extern PlayerRecord *players8900B4[2];
extern GameFlow8900D8 *gameFlow8900D8;
extern unsigned char replayActive89CF69; // 0x89cf69
extern int replayState8669F4;         // 0x8669f4
extern int replayState866A14;         // 0x866a14
extern float worldNearDistance7A548C; // 0x7a548c (100.0)
extern float worldEffectsDistance7A5490; // 0x7a5490 (150.0)
extern float carDraftLimit7A55BC;     // 0x7a55bc (0.9)

double bDistance(WheelVector *a, WheelVector *b); // 0x43ce10
extern WorldObject81CBC0 worldObject81CBC0;
extern WorldObject8357E0 worldObject8357E0;
extern WorldObject827B88 worldObject827B88;
extern WorldObject88EB00 worldObject88EB00;
extern RaceStatus890118 *raceStatus890118;

// Node of the world list at +0; +0xa is an id where -1 ends the search below.
struct WorldListNode {
    WorldListNode *next;
    char unidentified4[6];
    short idA;
};
struct WorldListHead {
    WorldListNode *first;
    WorldListNode *End(); // 0x502fd0 (shared list helper): the head is its own sentinel
};

WorldListNode *WorldListHead::End() {
    return reinterpret_cast<WorldListNode *>(this);
}

// Original 0x5ebd80: the summed draft from the first count18 other cars,
// saturating at 0.9.
float MoverModeOwner::GetDraftSum() {
    float sum = 0.0f;
    CarWorld *world = carWorld890080;
    for (int i = 0; i < world->count18; ++i) {
        MoverModeOwner *other = world->cars1C[i];
        if (other != this) {
            sum = GetDraft5EBC20(other) + sum;
            world = carWorld890080;
        }
        if (sum >= carDraftLimit7A55BC) return carDraftLimit7A55BC;
    }
    if (sum < carDraftLimit7A55BC) return sum;
    return carDraftLimit7A55BC;
}

// Original 0x5f1160: the smallest distance from a point to the published
// positions; positions 2..5 count only in replays or with more than two cars.
float CarWorld::NearestPlayerDistance(WheelVector *point) {
    float first = (float)bDistance(point, &positions80[0].position);
    float second = (float)bDistance(point, &positions80[1].position);
    double nearest;
    if ((replayActive89CF69 || replayState8669F4 > 0 || replayState866A14 > 0) && count14 > 2) {
        float third = (float)bDistance(point, &positions80[2].position);
        float fourth = (float)bDistance(point, &positions80[3].position);
        // The last distance stays on the x87 stack unrounded.
        if (count14 < 5) {
            nearest = third < fourth ? third : fourth;
        } else {
            float fifth = (float)bDistance(point, &positions80[4].position);
            nearest = bDistance(point, &positions80[5].position);
            if (fifth < nearest) nearest = fifth;
            if (fourth < nearest) nearest = fourth;
            if (third < nearest) nearest = third;
        }
        if (second < nearest) nearest = second;
    } else {
        nearest = second;
    }
    if (first < nearest) nearest = first;
    return (float)nearest;
}

// Original 0x609ce0, called from game loop 0x5811c0 on *0x890080.
void CarWorld::Update(float duration) {
    Prepare5FA770();
    engineSubstepDuration = 0.0f;
    UpdateCars5FC0E0();
    for (int car = 0; car < count10; ++car) cars1C[car]->Update5F6CD0();
    worldObject81CBC0.Update4268A0();
    Update614820();
    // Publish the leading cars' positions and +0x8fc; the rest are far away.
    int leading = count14;
    int i = 0;
    for (; i < leading; ++i) {
        MoverModeOwner *car = cars1C[i];
        positions80[i].position = car->State()->position20;
        valuesE0[i] = car->value8FC;
    }
    for (; i < 6; ++i) {
        positions80[i].position.x = 9999999.0f;
        positions80[i].position.y = 9999999.0f;
        positions80[i].position.z = 9999999.0f;
        valuesE0[i] = 9999999.0f;
    }
    int mode = gameMode89008C;
    if (mode && playerCount8900BC >= 1) {
        PlayerRecord *player = players8900B4[0];
        if (mode != player->car4->value24) {
            if (mode == 1) player->value230 = 2;
            else if (mode == 3) player->value230 = 4;
        }
    }
    if (value118) value118->Update42F910(duration);
    // Driver updates: cars [count14, count18), then [count18, count10), then the leading cars.
    for (i = 0; i < count18 - count14; ++i) {
        MoverModeOwner *car = cars1C[count14 + i];
        car->value4DC = car->GetDraftSum();
        car->participant2C->Update0C(duration);
    }
    for (i = 0; i < count10 - count18; ++i) {
        MoverModeOwner *car = cars1C[count18 + i];
        car->value4DC = car->GetDraftSum();
        car->participant2C->Update0C(duration);
    }
    for (i = 0; i < count14; ++i) {
        MoverModeOwner *car = cars1C[i];
        car->value4DC = car->GetDraftSum();
        car->participant2C->Update0C(duration);
    }
    for (i = 0; i < count10; ++i) {
        MoverModeOwner *car = cars1C[i];
        car->input30->Update(duration);
        if (car->countdown970) --car->countdown970;
    }
    for (i = 0; i < count10; ++i) cars1C[i]->Update5F6CD0();
    // 0x5fbbe0 and 0x604740 are recovered only as their kinematics prefixes.
    for (i = 0; i < count10; ++i) cars1C[i]->SnapshotFrameKinematics(duration);
    Update59E350();
    AdvanceCollisionBodies(duration);
    for (i = 0; i < count10; ++i) cars1C[i]->Update5F6CD0();
    UpdateCollisionWorld(reinterpret_cast<CollisionEventOwner *>(this), duration);
    for (i = 0; i < count10; ++i) cars1C[i]->UpdateFrameAcceleration(duration);
    for (i = 0; i < count10; ++i) cars1C[i]->Update5F6CD0();
    engineSubstepDuration = 0.0f;
    // The original searches the list at +0 for an id of -1 and discards the result.
    WorldListHead *list = reinterpret_cast<WorldListHead *>(carWorld890080);
    WorldListNode *node = list->first;
    WorldListNode *end = list->End();
    while (node != end && node->idA != -1) node = node->next;
    for (i = 0; i < count10; ++i) {
        MoverModeOwner *car = cars1C[i];
        if (NearestPlayerDistance(&car->State()->position20) < worldNearDistance7A548C) car->Update5F5580();
    }
    if (gameFlow8900D8->state0 != 5) {
        for (i = 0; i < count14; ++i) {
            MoverModeOwner *car = cars1C[i];
            if (car->raceEntry18) UpdateRace572E50(car->raceEntry18->index10, &car->State()->value40);
            if (gameFlow8900D8->state0 != 5) car->Update5F6350();
        }
    }
    worldObject8357E0.Update492E10(duration);
    for (i = 0; i < count10; ++i) {
        MoverModeOwner *car = cars1C[i];
        float distance = NearestPlayerDistance(&car->State()->position20);
        if (NearestPlayerDistance(&car->State()->position20) < worldEffectsDistance7A5490 &&
            car->context14->value4 != 3 && !car->effectsSuppressed998 && car->effectsEnabled550 && car->colliderPack5CC) {
            car->UpdateEffects5F5730(distance);
        }
    }
    Update605780();
    Update5E0A00();
    Update5E5430();
    Sort43E650(CompareWorldEntry5EA240);
    for (i = 0; i < playerCount8900BC; ++i) players8900B4[i]->Update602650();
    worldObject827B88.Update43AF20(duration);
    Update638070();
    worldObject88EB00.Update5E86E0();
    if (raceStatus890118) raceStatus890118->Update6076D0();
}
