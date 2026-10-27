#include "Collision.h"
#include "../VehicleMover.h"

extern "C" MoverBody *UG2CollisionMoverGetBody(VehicleMover *mover); // vtable +0x0c

// Original 0x5ed400. The null mover and PhysicsMover have different +0x0c
// implementations. Keep both virtual calls and re-read mover34 between them.
WheelVector *CarCollisionCentre(MoverModeOwner *car) {
    if (car->mover34 && UG2CollisionMoverGetBody(car->mover34)) {
        return &UG2CollisionMoverGetBody(car->mover34)->centre1F0;
    }
    return reinterpret_cast<WheelVector *>(car->record20);
}
