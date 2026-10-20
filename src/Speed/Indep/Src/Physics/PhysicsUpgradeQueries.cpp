#include "PhysicsUpgrades.h"

// Original 0x5995e0. The addition precedes the original CRT conversion.
int PhysicsUpgradeSelection::GetLevel(int index) {
    return (int)(values0[index] + 0.5f);
}

// 0x599610: byte selection query, without an index bound check.
bool PhysicsUpgradeSelection::IsSelected599610(int index) {
    return selected50[index] != 0;
}
