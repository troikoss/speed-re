#include "Collision.h"

// World-object promotion and break-up (UG2 PC v1.2).  A world object is a type 1 collision body
// whose +0x148 descriptor lists child shapes; hitting it replaces it by one body per child.
// Unknown fields stay offset-based.  Callees that are not recovered are seams, declared below
// with their original address; the native platform layer stops the program if one is reached.

struct WorldObject8357E0;

// Seams (not recovered).  The originals take the body in ECX (thiscall); the hand-off here
// passes it as the first argument.
CollisionBody *PromoteWorldObject5B1390(CollisionBody *body);                          // 0x5b1390, 428 bytes
void WorldObjectAttach5ADB20(CollisionBody *body, void *transform, int zero0, int zero1);  // 0x5adb20, 551 bytes
void WorldObjectUpdate5ADD50(CollisionBody *body);                                      // 0x5add50
void WorldObjectRegister48BC50(WorldObject8357E0 *world, int node);                     // 0x48bc50
// 0x86af90: function pointer set up at start-up; cdecl (id, descriptor entry, name hash).  The id is the
// zero-extended low word of the allocator hint (movzx).
extern CollisionBody *(*worldObjectCreate86AF90)(unsigned int id, void *entry, unsigned int hash);
// Original 0x43db80 (recovered as HudNameHash, verified by verify_pc_hud_minimap.py): h = h * 33 + c.
unsigned int HudNameHash(const char *text, unsigned int seed);

extern WorldObject8357E0 worldObject8357E0;
extern int worldObjectActivations86B378;
extern int collisionClock; // 0x86518c

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

// The object embedded at world-object body +0x480.
struct WorldObjectPlacement {
    int value_0;
    int value_4;
    WorldObjectSeedSource *source8;
    unsigned int GetSeed611000();
};

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
            WorldObjectRegister48BC50(&worldObject8357E0, node[2]);
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
