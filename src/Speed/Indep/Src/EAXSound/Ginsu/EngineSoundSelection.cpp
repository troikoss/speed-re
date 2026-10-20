#include "EngineSoundBanks.h"
#include "../../Physics/VehicleMover.h"

// 0x459240: car type to the sound-choice row; default also covers type 0x1c.
int EngineSoundCarIndex459240(unsigned int carType) {
    switch (carType) {
    case 0: return 22;
    case 1: return 10;
    case 2: return 6;
    case 3: return 0;
    case 4: return 19;
    case 5: return 5;
    case 6: return 23;
    case 7: return 7;
    case 8: return 14;
    case 9: return 21;
    case 10: return 9;
    case 11: return 31;
    case 12: return 28;
    case 13: return 4;
    case 14: return 17;
    case 15: return 30;
    case 16: return 12;
    case 17: return 3;
    case 18: return 24;
    case 19: return 8;
    case 20: return 32;
    case 21: return 26;
    case 22: return 2;
    case 23: return 11;
    case 24: return 1;
    case 25: return 13;
    case 26: return 20;
    case 27: return 29;
    case 29: return 25;
    case 30: return 15;
    default: return 18;
    }
}

// 0x479660: writes only the original fields, including two byte stores.
EngineSoundCar *EngineSoundCar::Construct479660() {
    int *words = reinterpret_cast<int *>(this);
    words[0x1c / 4] = 0;
    words[0x20 / 4] = 0;
    words[0x04 / 4] = 0;
    words[0x08 / 4] = 0;
    words[0x28 / 4] = 0;
    words[0x2c / 4] = 0;
    words[0x0c / 4] = 0;
    bankIndex10 = 0;
    kind14 = 0;
    words[0] = 0x78baa8;
    words[0x30 / 4] = 0;
    words[0x3c / 4] = 0;
    words[0x34 / 4] = 0;
    words[0x38 / 4] = 0;
    words[0xac / 4] = 0;
    words[0x40 / 4] = 0;
    value7C = 0;
    value80 = engineSoundPrimary7F0DE0;
    value88 = engineSoundChoices7F2230;
    value8C = 0;
    mode78 = 0;
    words[0x58 / 4] = 0;
    reinterpret_cast<unsigned char *>(this)[0xb4] = 0;
    words[0xb8 / 4] = 0;
    words[0xbc / 4] = 0;
    reinterpret_cast<unsigned char *>(this)[0xc0] = 0;
    words[0xc4 / 4] = 0;
    words[0xc8 / 4] = 0;
    return this;
}

// 0x4593b0: participant setup +0x10 supplies car type; upgrades are at +0x300.
// Calls, priority and clamps follow the original instruction order.
void EngineSoundCar::Select4593B0(MoverContext *context) {
    context74 = context;
    value8C = EngineSoundCarIndex459240(context->setup10.value0);
    PhysicsUpgradeSelection *selection = &context->setup10.parameters10.selection2E0;
    if (selection->IsSelected599610(0x18)) value94 = 3;
    else if (selection->IsSelected599610(0x13)) value94 = 2;
    else if (selection->IsSelected599610(0xf)) value94 = 1;
    else value94 = 0;
    int level = selection->GetLevel(4);
    if (level <= 0) level = 0;
    else if (level > 3) level = 3;
    value98 = level;
    level = selection->GetLevel(2);
    if (level <= 0) level = 0;
    else if (level > 3) level = 3;
    value9C = level;
    level = selection->GetLevel(8);
    if (level <= 0) level = 0;
    else if (level > 3) level = 3;
    valueA0 = level;
    level = selection->GetLevel(1);
    if (level <= 0) level = 0;
    else if (level > 3) level = 3;
    valueA4 = level;
    value90 = value94;
    value88 = &engineSoundChoices7F2230[value94 + value8C * 4];
    value7C = value88->value0;
    value80 = &engineSoundPrimary7F0DE0[value7C];
    if (value80->value10 != 0 && engineParameterMode850078 != 4) {
        mode78 = 1;
        value84 = &engineSoundSecondary7F1AA8[value80->value14];
    } else {
        mode78 = 0;
        value84 = engineSoundSecondary7F1AA8;
    }
}
