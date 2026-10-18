#include "VehicleMover.h"
#include "Collision/Collision.h"
#include "../World/WCollision.h"
#include "DriveFeedbackCurve.h"

// Car update 0x6012b0 and its per-frame leaves. Fields beyond the observed
// reads remain offset-based; the reset branch reaches unrecovered callees.

struct HudPlayerSlot {
    char unidentified0[4];
    MoverModeOwner *car4;
};

extern int drivetrainClock;           // 0x86518c
extern float drivetrainClockScale;    // 0x784268
extern float carResetSpeedLimit8021CC; // 0x8021cc (8.0)
extern float carResetExtension8021D0; // 0x8021d0 (0.5)
extern float hudOne784250;           // 0x784250 (1.0)
extern float carSmoothingStep7A5654;  // 0x7a5654 (0.0225)
extern float carSmoothingRate7A5658;  // 0x7a5658 (0.0035)
extern float hudZero7D7D3C;          // 0x7d7d3c (0.0)
extern float carContactHold7A565C;    // 0x7a565c (0.1)
extern float carGapScale88F2B0;       // 0x88f2b0
extern unsigned char carResetDirectionFlag88F2BE; // 0x88f2be
extern unsigned char gameFlag89E7D8;  // 0x89e7d8
extern int gameFlag890108;            // 0x890108
extern int carGapEnabled89E7F8;       // 0x89e7f8
extern int playerCount8900BC;         // 0x8900bc
extern HudPlayerSlot *hudPlayerSlots8900AC[];
extern char *raceTuningData81CB60;

void __cdecl ReleaseResetId403DB0(int id); // 0x403db0, not recovered

// 0x4ae7b0: a tick count from seconds, rounded through _ftol.
struct CarTicks {
    int value0;
    CarTicks *Construct(float seconds);
};

CarTicks *CarTicks::Construct(float seconds) {
    value0 = (int)(seconds * 4000.0f + 0.5f);
    return this;
}

// Original 0x5995e0.
int PhysicsUpgradeSelection::GetLevel(int index) {
    return (int)(values0[index] + 0.5f);
}

// Original 0x5fc050: contact state at car +0x38 from the collision body's event
// count. State 1 returns to 0 once no events have been seen for 0.1 s.
void MoverModeOwner::UpdateContactState() {
    switch (state38) {
    case 0:
        if (collisionBody3C->events18 > 0) state38 = 2;
        break;
    case 1:
        if (collisionBody3C->events18 == 0 && mode424 == 4) {
            float now = drivetrainClock * drivetrainClockScale;
            if (hudZero7D7D3C == contactTime54C) {
                contactTime54C = now;
            } else if (now - contactTime54C >= carContactHold7A565C) {
                state38 = 0;
            }
        } else {
            contactTime54C = 0.0f;
        }
        break;
    case 2:
        if (collisionBody3C->events18 == 0 && mode424 == 4) state38 = 0;
        break;
    }
}

// Original 0x5f6740: in two-player mode-1 races, maps the gap between the two
// players' race objects (+0x14) through the curve at race tuning +0xa6c.
void MoverModeOwner::UpdateOpponentGap() {
    if (context14->value4 == 1 && !context14->flag900 && carGapEnabled89E7F8 && playerCount8900BC == 2) {
        HudPlayerSlot *other = raceEntry18->index10 ? hudPlayerSlots8900AC[0] : hudPlayerSlots8900AC[1];
        float scale = carGapScale88F2B0;
        float own = object1C->value14;
        float opponent = other->car4->object1C->value14;
        DriveFeedbackCurve curve;
        curve.Construct(reinterpret_cast<DriveFeedbackPair *>(raceTuningData81CB60 + 0xa6c), 4);
        value468 = curve.Lookup(own * scale - opponent * scale);
    }
}

// Original 0x5ebe00: on a gear change, notify the replay and, after a shift
// interval, emit the shift effect scaled by upgrade level 4.
void MoverModeOwner::UpdateGear(int gear) {
    bool shifted = false;
    if (gear != gear4D0) {
        if (replay968) replay968->RecordEvent48(1, 0, 0);
        gear4D0 = gear;
        int now = drivetrainClock;
        if (moverType28 == 1 || (float)(now - shiftTick9D8) * drivetrainClockScale > 0.5f) {
            shifted = true;
        } else {
            shifted = false;
        }
        shiftTick9D8 = now;
    }
    if (moverType28 == 1 && gear4D0 > 2) {
        shifted = value4D4 == 0 && value448 != 0;
    }
    PhysicsUpgradeSelection *selection = &physicsInfo10->parameters10.selection2E0;
    float level = (float)selection->GetLevel(4);
    if (selection->values28[4] != 0) level += 0.5f;
    if (shifted && level >= 2.0f) {
        EmitEffect(10, level * 0.2857143f, gear434, 0);
        if (replay968) replay968->RecordEvent48(9, 0, 0);
    }
    value4D4 = value448;
}

// Original 0x5ecac0: moves car +0x4f8 towards +0x330 and integrates it into +0x4e8.
void MoverModeOwner::UpdateSmoothing(float duration) {
    float *target = reinterpret_cast<float *>(reinterpret_cast<char *>(this) + 0x330);
    float rate = hudOne784250 / carSmoothingStep7A5654;
    for (int i = 0; i < 4; ++i) {
        value4F8[i] += (target[i] - value4F8[i]) * carSmoothingRate7A5658 * rate * duration;
        value4E8[i] += value4F8[i] * duration;
    }
}

// Original 0x5ecba0: request a reset when ground is found more than 0.25
// above a touching car in mode 4.
void MoverModeOwner::CheckBelowGround() {
    if (collisionBody3C->events18 && mode424 == 4) {
        char found;
        float height = Collider()->QueryGround(&State()->position20, 0, 0, &found);
        if (found && height > State()->position20.z + 0.25f) {
            resetRequested96C = 1;
        }
    }
}

// Original 0x61fb00.
void MoverModeOwner::EmitEffect(int kind, float intensity, int value, WheelVector *target) {
    CarEffectEmitters *effects = effects554;
    if (!effects || intensity == hudZero7D7D3C) return;
    if (!kind || value5B0) return;
    char *base = reinterpret_cast<char *>(this);
    int high = 0;
    WheelVector *first, *second;
    if ((kind == 3 || kind == 4 || kind == 8 || kind == 9) && (high = value >> 16) > 0 && high <= 8) {
        first = reinterpret_cast<WheelVector *>(base + 0x60);
        second = reinterpret_cast<WheelVector *>(base + 0x90);
    } else {
        first = reinterpret_cast<WheelVector *>(base + 0x50);
        second = reinterpret_cast<WheelVector *>(base + 0xd0);
    }
    effects->Spawn(kind, intensity, value, high, first, second,
                   target ? target : reinterpret_cast<WheelVector *>(base + 0x70));
}

// Original 0x5f4960: a contact goes to the driver, or switches a car without a
// physics mover (+0x28 not 1 or 3) to type 1 with a race entry, else 3.
void MoverModeOwner::NotifyContact(MoverModeOwner *other) {
    if (participant2C) {
        participant2C->NotifyContact38(other, 0);
        return;
    }
    if (moverType28 == 1 || moverType28 == 3) return;
    SwitchMover(raceEntry18 ? 1 : 3);
}

// Original 0x6012b0: the car's per-step update, reached from car body step 0x5a9f10.
float MoverModeOwner::Update(float duration) {
    UpdateContactState();
    UpdateOpponentGap();
    float stepped = reinterpret_cast<VehicleMoverSlots *>(mover34)->Step(duration);
    UpdateGear(gear434);
    UpdateSmoothing(stepped);
    if (!context14->flag900) CheckBelowGround();
    if (resetRequested96C) {
        int mode = context14->value4;
        if (mode == 1 && !context14->flag900) {
            raceEntry18->Reset6009A0();
            resetRequested96C = 0;
            return stepped;
        }
        if (mode == 3) {
            bool missing = Collider()->HasGround(&State()->position20) == 0;
            CarResetHandler *handler =
                *reinterpret_cast<CarResetHandler **>(reinterpret_cast<char *>(collisionEventOwner890080) + 0x118);
            handler->Remove409770(this);
            if (missing) {
                (*reinterpret_cast<CarResetHandler **>(reinterpret_cast<char *>(collisionEventOwner890080) + 0x118))->Restore409960(this);
            }
        } else {
            ResetPlacement *placement = participant2C->GetResetPlacement();
            float speed = speed42C > hudZero7D7D3C ? speed42C : 0.0f;
            ParticipantRoute *route = participant2C->routeC;
            float distance = 0.0f;
            float x, y;
            if (carResetDirectionFlag88F2BE) {
                x = direction900;
                y = direction904;
            } else if (route && participant2C) {
                float direction[2];
                float *result = route->GetDirection(direction, participant2C->flag10 == 0);
                x = result[0];
                y = result[1];
            } else {
                float *matrix = reinterpret_cast<float *>(reinterpret_cast<char *>(this) + 0x90);
                x = matrix[0];
                y = matrix[1];
            }
            float *matrix = reinterpret_cast<float *>(reinterpret_cast<char *>(this) + 0x90);
            float alignment = x * matrix[0] + y * matrix[1];
            if (!(alignment > hudZero7D7D3C)) alignment = hudZero7D7D3C;
            speed = alignment * speed;
            if (!(speed < carResetSpeedLimit8021CC)) speed = carResetSpeedLimit8021CC;
            float heading = 1.0f;
            int kind = context14->value4;
            if (kind == 2 || kind == 4) {
                if (carResetDirectionFlag88F2BE) {
                    if (placement && (char)(placement->flags70 & 2)) heading = -1.0f;
                } else if (participant2C && !participant2C->flag10) {
                    heading = -1.0f;
                }
            }
            if (gameFlag89E7D8 || gameFlag890108) {
                int id = participant2C->GetResetId1C();
                if (!participant2C->flag10) ReleaseResetId403DB0(id);
                if (route && id != -1) distance = route->QueryDistance(id);
            }
            Reset5FBED0(placement, route, speed, 1, heading, 0, 0, distance);
        }
        resetRequested96C = 0;
        return stepped;
    }
    // Extend a running reset window by 0.5 s when the body was touched during
    // its last 0.5 s (collision body +0x13c holds the touch tick).
    int touched = collisionBody3C->tick13C;
    if (resetTick974 != 0 && resetTick974 != 0x7fffffff) {
        int now = drivetrainClock;
        int start = resetTick974;
        float elapsed = (float)(now - start) * drivetrainClockScale;
        if (elapsed < resetDuration978 && touched != 0 && touched != 0x7fffffff &&
            resetDuration978 - carResetExtension8021D0 < elapsed) {
            double since = (float)(now - touched) * drivetrainClockScale;
            if (since < carResetExtension8021D0 && since >= hudZero7D7D3C) {
                CarTicks ticks;
                resetTick974 = start + ticks.Construct(carResetExtension8021D0)->value0;
            }
        }
    }
    return stepped;
}
