extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)
#include "Collision.h"
#include "../WorldUpdate.h"

// World-object promotion and break-up (UG2 PC v1.2).  A world object is a type 1 collision body
// whose +0x148 descriptor lists child shapes; hitting it replaces it by one body per child.
// Unknown fields stay offset-based.  The routines reached from the chain that are not recovered
// (collider construction and queries, mover pose, bounds refresh, the slot-pool test, the
// creation function behind 0x86af90 and the body's virtual slots) are declared by their
// recovered owners; the native platform layer stops the program if one is reached.

// Chain routines recovered here.  The thiscall originals take the body in ECX; the hand-off
// here passes it as the first argument.
CollisionBody *PromoteWorldObject5B1390(CollisionBody *body);                          // 0x5b1390, 428 bytes
void WorldObjectAttach5ADB20(CollisionBody *body, void *transform, WheelVector *velocity,
                             WheelVector *angular);                                    // 0x5adb20, 551 bytes
void WorldObjectUpdate5ADD50(CollisionBody *body);                                      // 0x5add50
// 0x86af90: function pointer set up at start-up; cdecl (id, descriptor entry, name hash).  The id is the
// zero-extended low word of the allocator hint (movzx).
extern CollisionBody *(*worldObjectCreate86AF90)(unsigned int id, void *entry, unsigned int hash);
// Original 0x43db80 (recovered as HudNameHash, verified by verify_pc_hud_minimap.py): h = h * 33 + c.
unsigned int HudNameHash(const char *text, unsigned int seed);
// Virtual slot 0x8c of the body (not recovered; see CollisionBodies.cpp): 0 for a body that breaks apart.
int WorldObjectKind(CollisionBody *body);

extern WorldObject8357E0 worldObject8357E0;
extern int worldObjectActivations86B378;
extern int collisionClock; // 0x86518c
extern void *worldObjectPool86B374;                // 0x86b374, slot pool of the world-object bodies
extern unsigned char worldObjectBitmap86AD78[];    // 0x86ad78, one bit per world-object entry index
extern int worldObjectUpdate8900F8;                // 0x8900f8
extern unsigned int worldObjectRandomState7F3940;  // 0x7f3940, xorshift state shared by the registrations
// .rdata constants read by address (the originals use FCOM/FMUL with memory operands).
extern float worldObjectZero7D7D3C;                // 0.0f
extern float worldObjectSettle7844F4;              // 0.05f
extern float worldObjectSpeedLimit7859D0;          // 6.0f
extern float worldObjectRandomScale78D864;         // 2^-31
extern float worldObjectRandomRange786D84;         // 2^31

// Original 0x7a1eec: "_CHILDx", the x being replaced by the child's number.
struct WorldObjectChildName {
    char text[8];
};
extern const WorldObjectChildName worldObjectChildName7A1EEC;

struct WorldObjectSeedSource {
    int value_0;
    int value_4;
    unsigned int seed8;
};

// Node of the placement tree that a body may hang below (WorldObjectPlacement::parent60): +8 is
// the sentinel of the list of its children, which link through +0.
struct WorldPlacementNode {
    WorldPlacementNode *next0;
    char unidentified4[4];
    WorldPlacementNode *first8;
    char unidentifiedC[0x44];
    WheelMatrix matrix50;
    char unidentified90[0x54];
    int marked0E4;
    void Mark6147E0();
};

// The object embedded at world-object body +0x480.
struct WorldObjectPlacement {
    int value_0;
    int value_4;
    WorldObjectSeedSource *source8;
    char unidentifiedC[4];
    int value10;
    char unidentified14[0xc];
    WheelMatrix matrix20;
    WorldPlacementNode *parent60;
    unsigned int GetSeed611000();
    void Place40F010(WheelMatrix *matrix);
};

// Registration node of the world object 0x8357e0 (BodyNode +0x4fc list entries point at these).
// The descriptor holds two (value, fraction) pairs, selected by the node's state.
struct WorldRegistrationDescriptor {
    char unidentified0[0x24];
    float first24;
    float second28;
    float first2C;
    float second30;
};
struct WorldRegistrationNode {
    WorldRegistrationNode *next0;
    WorldRegistrationNode *previous4;
    char unidentified8[8];
    int state10;
    float value14;
    int value18;
    WorldRegistrationDescriptor *descriptor1C;
    char unidentified20[4];
    unsigned int seed24;
};

// Block of world-object entries (0x70 bytes each from +0x10), chained from 0x86b4c0.  The
// entries of block B are numbered firstA .. firstA + countC - 1.
struct WorldObjectBlock {
    WorldObjectBlock *next0;
    char unidentified4[6];
    short firstA;
    short countC;
    char unidentifiedE[2];
    WorldObjectBlock *End(); // 0x502fd0 (shared list helper): the head is its own sentinel
};
extern WorldObjectBlock worldObjectBlocks86B4C0;

// The object embedded at body +0x130; only virtual slot 4 is reached from here.
struct WorldObjectSubobject130 {
    virtual void Slot0() = 0;
    virtual void Slot4() = 0;
};

// Original 0x502fd0.
WorldObjectBlock *WorldObjectBlock::End() {
    return this;
}

// Original 0x6147e0: mark this node and, recursively, the children not marked yet.
void WorldPlacementNode::Mark6147E0() {
    WorldPlacementNode *child = first8;
    WorldPlacementNode *end = reinterpret_cast<WorldPlacementNode *>(&first8);
    marked0E4 = 1;
    for (; child != end; child = child->next0) {
        if (!child->marked0E4) {
            child->Mark6147E0();
        }
    }
}

// Original 0x40f010: set the placement from a transform.  Without a transform the placement
// is cleared; otherwise it is copied to the parent node (which is marked unless it already is) or,
// without a parent, to +0x20.
void WorldObjectPlacement::Place40F010(WheelMatrix *matrix) {
    if (!matrix) {
        value10 = 0;
        return;
    }
    WorldPlacementNode *parent = parent60;
    value10 = 1;
    if (!parent) {
        BodyMatrixCopy(&matrix20, matrix);
        return;
    }
    BodyMatrixCopy(&parent->matrix50, matrix);
    if (!parent->marked0E4) {
        parent->Mark6147E0();
    }
}

// Original 0x5a8b70: the number of a world-object entry, from the block that contains it, or -1.
int FindWorldObjectIndex5A8B70(void *entry) {
    WorldObjectBlock *end = worldObjectBlocks86B4C0.End();
    for (WorldObjectBlock *block = worldObjectBlocks86B4C0.next0; block != end; block = block->next0) {
        int first = block->firstA;
        int index = (reinterpret_cast<int>(entry) - reinterpret_cast<int>(block) - 0x10) / 0x70 + first;
        if (index >= first && index < block->countC + first) {
            return index;
        }
    }
    return -1;
}

// Original 0x487360 (cdecl): advance the xorshift state and scale the low 31 bits to [0, scale).
float RandomFloat487360(float scale, unsigned int *state) {
    unsigned int value = *state;
    value ^= value << 17;
    value ^= value >> 13;
    value ^= value << 5;
    *state = value;
    return (float)(value & 0x7fffffff) * worldObjectRandomScale78D864 * scale;
}

// Original 0x48bc50: add a node to the world object's list.  A node without a seed first gets one
// from the shared state (the same sequence as 0x487360, open-coded).  A node whose descriptor is
// present and whose state is 0 or 1 takes a randomised value (0x14) and goes to the front;
// every other node goes to the back, state 2 unless it is 3.
void WorldObject8357E0::Register48BC50(WorldRegistrationNode *node) {
    unsigned int *seed = &node->seed24;
    if (*seed == 0) {
        unsigned int value = worldObjectRandomState7F3940;
        value ^= value << 17;
        value ^= value >> 13;
        value ^= value << 5;
        worldObjectRandomState7F3940 = value;
        *seed = (int)((float)(value & 0x7fffffff) * worldObjectRandomScale78D864 * worldObjectRandomRange786D84);
    }
    WorldRegistrationDescriptor *descriptor = node->descriptor1C;
    int state = node->state10;
    if (descriptor != 0 && state != 2) {
        if (state == 0 || state == 1) {
            float first = descriptor->first24;
            float second = descriptor->second28;
            if (state == 0) {
                first = descriptor->first2C;
                second = descriptor->second30;
            }
            float value = first;
            if (first != worldObjectZero7D7D3C) {
                float scale = second * first;
                value = RandomFloat487360(scale, seed) + (first - scale);
            }
            node->value14 = value;
            node->value18 = 0;
            WorldRegistrationNode *front = list8.next;
            WorldRegistrationNode *sentinel = reinterpret_cast<WorldRegistrationNode *>(&list8);
            sentinel->next0 = node;
            front->previous4 = node;
            node->previous4 = sentinel;
            node->next0 = front;
            ++count50;
            return;
        }
        if (state != 3) {
            node->state10 = 2;
        }
    } else {
        node->state10 = 2;
    }
    WorldRegistrationNode *sentinel = reinterpret_cast<WorldRegistrationNode *>(&list8);
    WorldRegistrationNode *back = sentinel->previous4;
    back->next0 = node;
    sentinel->previous4 = node;
    node->next0 = sentinel;
    node->previous4 = back;
    ++count50;
}

// Original 0x611000.
unsigned int WorldObjectPlacement::GetSeed611000() {
    return source8->seed8;
}

// Original 0x5b31f0: promote the body (seam 0x5b1390), then destroy the original through
// virtual slot 0 (deleting destructor, argument 1).  Returns what the seam returned.
CollisionBody *PromoteWorldObject(CollisionBody *body) {
    CollisionBody *promoted = PromoteWorldObject5B1390(body);
    if (body) {
        delete body;
    }
    return promoted;
}

// Original 0x5adf90: switch a freshly created child body on.
void WorldObjectActivate5ADF90(CollisionBody *body) {
    char *bytes = reinterpret_cast<char *>(body);
    bytes[0x506] = 1;
    body->active19 = 1;
    ++worldObjectActivations86B378;
    WorldObjectUpdate5ADD50(body);
    if (!bytes[0x505]) {
        int *head = reinterpret_cast<int *>(bytes + 0x4fc);
        for (int *node = reinterpret_cast<int *>(*head); node != head; node = reinterpret_cast<int *>(*node)) {
            worldObject8357E0.Register48BC50(reinterpret_cast<WorldRegistrationNode *>(node[2]));
        }
        bytes[0x505] = 1;
    }
    int stamp = *reinterpret_cast<int *>(bytes + 0x15c);
    if (stamp == 0 || stamp == 0x7fffffff) {
        *reinterpret_cast<int *>(bytes + 0x15c) = collisionClock;
    }
}

// Original 0x5ae000: create one child body per entry of the descriptor (+0x148, count at +0x8c,
// entries from +0x90), store them in `output` and return the count; the world object itself is
// then destroyed through virtual slot 0 (argument 1).
int BreakWorldObject(CollisionBody *body, CollisionBody **output) {
    char *bytes = reinterpret_cast<char *>(body);
    int count = *reinterpret_cast<int *>(*reinterpret_cast<char **>(bytes + 0x148) + 0x8c);
    WorldObjectChildName name = worldObjectChildName7A1EEC;
    if (count > 0) {
        WorldObjectPlacement *placement = reinterpret_cast<WorldObjectPlacement *>(bytes + 0x480);
        for (int i = 0; i < count; ++i) {
            unsigned int id = (unsigned short)collisionIds86B2BC->next8;
            name.text[6] = (char)(i + '1');
            unsigned int hash = HudNameHash(name.text, placement->GetSeed611000());
            void *entry = *reinterpret_cast<void **>(*reinterpret_cast<char **>(bytes + 0x148) + 0x90 + i * 4);
            CollisionBody *child = worldObjectCreate86AF90(id, entry, hash);
            char *parent = *reinterpret_cast<char **>(reinterpret_cast<char *>(placement) + 0x60);
            void *transform = parent ? static_cast<void *>(parent + 0x50)
                                     : static_cast<void *>(reinterpret_cast<char *>(placement) + 0x20);
            WorldObjectAttach5ADB20(child, transform, 0, 0);
            WorldObjectActivate5ADF90(child);
            output[i] = child;
        }
    }
    delete body;
    return count;
}

// Original 0x5adb20: attach a freshly created child body to the world: the transform without its
// translation (the translation row becomes 0, 0, 0, 1) is the orientation, the centre of the mover
// body rotated by it plus the translation is the position, the mover body is posed with the
// optional velocity and angular velocity (zero when null), the transform lowered by +0x168 goes
// to the placement (+0x480), the pose is stored at +0x70/+0xb0, the velocity at +0xc0 is cleared,
// the duration at +0x6c is 5000.0, slot 4 of the object at +0x130 is called, the bounds are
// refreshed and the transform is copied to +0x40 of every node of the two lists at +0x4f4 and +0x4fc.
void WorldObjectAttach5ADB20(CollisionBody *body, void *transform, WheelVector *velocity, WheelVector *angular) {
    char *bytes = reinterpret_cast<char *>(body);
    DynamicCollisionBody *dynamic = static_cast<DynamicCollisionBody *>(body);
    WheelMatrix *source = static_cast<WheelMatrix *>(transform);
    WheelVector translation = *reinterpret_cast<WheelVector *>(reinterpret_cast<char *>(source) + 0x30);
    WheelMatrix orientation;
    BodyMatrixCopy(&orientation, source);
    orientation.elements[12] = 0.0f;
    orientation.elements[13] = 0.0f;
    orientation.elements[14] = 0.0f;
    orientation.elements[15] = 1.0f;
    WheelVector velocityCopy = {0.0f, 0.0f, 0.0f};
    if (velocity) {
        velocityCopy = *velocity;
    }
    WheelVector angularCopy = {0.0f, 0.0f, 0.0f};
    if (angular) {
        angularCopy = *angular;
    }
    WheelVector rotated;
    WheelTransform(&rotated, &orientation, &dynamic->body68->centre1F0);
    WheelVector position;
    position.x = rotated.x + translation.x;
    position.y = rotated.y + translation.y;
    position.z = rotated.z + translation.z;
    dynamic->body68->SetPose(&position, &orientation, &velocityCopy, &angularCopy, 0);
    WheelMatrix lowered;
    BodyMatrixCopy(&lowered, source);
    lowered.elements[14] = lowered.elements[14] - *reinterpret_cast<float *>(bytes + 0x168);
    reinterpret_cast<WorldObjectPlacement *>(bytes + 0x480)->Place40F010(&lowered);
    dynamic->positionB0 = position;
    BodyMatrixCopy(&dynamic->matrix70, &orientation);
    dynamic->velocityC0.x = 0.0f;
    dynamic->velocityC0.y = 0.0f;
    dynamic->velocityC0.z = 0.0f;
    dynamic->duration6C = 5000.0f;
    reinterpret_cast<WorldObjectSubobject130 *>(bytes + 0x130)->Slot4();
    dynamic->RefreshBounds();
    int *first = reinterpret_cast<int *>(bytes + 0x4f4);
    for (int *node = reinterpret_cast<int *>(*first); node != first; node = reinterpret_cast<int *>(*node)) {
        BodyMatrixCopy(reinterpret_cast<WheelMatrix *>(node[2] + 0x40), source);
    }
    int *second = reinterpret_cast<int *>(bytes + 0x4fc);
    for (int *node = reinterpret_cast<int *>(*second); node != second; node = reinterpret_cast<int *>(*node)) {
        BodyMatrixCopy(reinterpret_cast<WheelMatrix *>(node[2] + 0x40), source);
    }
}

// Original 0x5add50: refresh the collider of a child body.  With 0x8900f8 set, the collider area
// is queried around the position with the body's radius plus its speed times 0.05 (at most 6.0)
// and its velocity; otherwise only the collider pack is updated (radius 0).  The bounds follow.
void WorldObjectUpdate5ADD50(CollisionBody *body) {
    char *bytes = reinterpret_cast<char *>(body);
    WCollider *collider = reinterpret_cast<WCollider *>(bytes + 0x16c);
    if (worldObjectUpdate8900F8) {
        float radius = body->radius1C;
        WheelVector *velocity = body->GetVelocity();
        float speed = (float)sqrt(velocity->x * velocity->x + velocity->y * velocity->y + velocity->z * velocity->z)
                    * worldObjectSettle7844F4;
        if (!(speed < worldObjectSpeedLimit7859D0)) {
            speed = 6.0f;
        }
        WheelVector *movement = body->GetVelocity();
        collider->QueryArea(&body->GetPosition()->x, speed + radius, &movement->x);
    } else {
        collider->UpdatePack(&body->GetPosition()->x, 0.0f);
    }
    static_cast<DynamicCollisionBody *>(body)->RefreshBounds();
}

// Original 0x5b1390: replace a world-object body by a new one created from its entry (+0x5c) and
// descriptor (+0x60) at the entry's transform (+0x30).  A body of kind 0 is lowered onto the
// ground below it: the ground height below the position (collider query), minus the lowest point
// of the new body's bounds (plus the descriptor's +0xb8 offset) along the ground normal and the
// position height, plus 0.05, when positive, becomes the body's +0x168 and is added to the height.
// The new body takes the entry's number (0x5a8b70) at +0x164, is attached, and its bit is set in
// the bitmap at 0x86ad78.  Null when the body pool is full.
CollisionBody *PromoteWorldObject5B1390(CollisionBody *body) {
    char *bytes = reinterpret_cast<char *>(body);
    if (bIsSlotPoolFull(worldObjectPool86B374)) {
        return 0;
    }
    char *entry = *reinterpret_cast<char **>(bytes + 0x5c);
    WheelMatrix matrix;
    BodyMatrixCopy(&matrix, reinterpret_cast<WheelMatrix *>(entry + 0x30));
    CollisionBody *created = worldObjectCreate86AF90((unsigned short)collisionIds86B2BC->next8,
                                                     *reinterpret_cast<void **>(bytes + 0x60),
                                                     *reinterpret_cast<unsigned int *>(entry + 8));
    if (WorldObjectKind(created) == 0) {
        WCollider collider;
        collider.Construct();
        collider.UpdatePack(&matrix.elements[12], 0.0f);
        if (collider.pack8) {
            WheelVector normal;
            char found;
            float height = collider.QueryGround(reinterpret_cast<WheelVector *>(&matrix.elements[12]), 0, &normal, &found);
            if (found) {
                WheelVector minimum;
                WheelVector maximum;
                static_cast<DynamicCollisionBody *>(created)->GetShapeBounds(&minimum, &maximum);
                float lowered = height
                              - ((minimum.z + *reinterpret_cast<float *>(*reinterpret_cast<char **>(reinterpret_cast<char *>(created) + 0x148) + 0xb8))
                                 * normal.z + matrix.elements[14])
                              + worldObjectSettle7844F4;
                if (lowered > worldObjectZero7D7D3C) {
                    *reinterpret_cast<float *>(reinterpret_cast<char *>(created) + 0x168) = lowered;
                    matrix.elements[14] = lowered + matrix.elements[14];
                }
            }
        }
        collider.Destruct();
    }
    int index = FindWorldObjectIndex5A8B70(*reinterpret_cast<void **>(bytes + 0x5c));
    *reinterpret_cast<int *>(reinterpret_cast<char *>(created) + 0x164) = index;
    WorldObjectAttach5ADB20(created, &matrix, 0, 0);
    worldObjectBitmap86AD78[index >> 3] |= (unsigned char)(1 << (index & 7));
    return created;
}

// Original 0x592a10: the shape bounds, +0xe0 and +0xf0, copied out.  (Defined after its caller so that
// the compiler, like the original build, cannot see that it leaves the x87 stack alone.)
void DynamicCollisionBody::GetShapeBounds(WheelVector *minimum, WheelVector *maximum) {
    float minimumY = minimumE0.y;
    float minimumZ = minimumE0.z;
    minimum->x = minimumE0.x;
    minimum->y = minimumY;
    minimum->z = minimumZ;
    float maximumY = maximumF0.y;
    float maximumZ = maximumF0.z;
    maximum->x = maximumF0.x;
    maximum->y = maximumY;
    maximum->z = maximumZ;
}
