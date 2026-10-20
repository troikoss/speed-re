#include "AudioSystem.h"
#include "../EAXSound/Ginsu/EngineSound.h"

// 0x4690b0: manager absence and an exhausted list both return null.
EngineSoundCar *Audio82B884::FindListener4690B0(int index) {
    if (!managerAC) return 0;
    for (AudioManagedCarSound *sound = managerAC->first10; sound; sound = sound->next4) {
        EngineSoundCar *car = reinterpret_cast<EngineSoundCar *>(sound);
        if (car->bankIndex10 == index) return car;
    }
    return 0;
}
