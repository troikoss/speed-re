#include "AudioSystem.h"
#include "../EAXSound/Ginsu/EngineSoundSchedule.h"
// 0x463c50: locate the index again after the resolver's initial scan.
EngineSoundCar *AudioCarSoundManager::FindIndex463C50(int index) {
    for (AudioManagedCarSound *node = first10; node; node = node->next4)
        if (*reinterpret_cast<int *>(reinterpret_cast<char *>(node) + 0x10) == index)
            return reinterpret_cast<EngineSoundCar *>(node);
    return 0;
}
// 0x464cd0 / 0x464d00: playback and input lists use the same low-seven-bit key.
EngineSoundScheduledNode *EngineSoundCar::FindPlayback464CD0(unsigned int id) {
    for (EngineSoundScheduledNode *node = first20; node; node = node->next50)
        if (((node->key4C >> 4) & 0x7f) == id) return node;
    return 0;
}
EngineSoundScheduledNode *EngineSoundCar::FindInput464D00(unsigned int id) {
    EngineSoundScheduledNode *node = first1C;
    while (node && ((node->key4C >> 4) & 0x7f) != id) node = node->next50;
    return node;
}
// 0x468f70 / 0x468fe0: retain the pre-scan and class-tag checks before helper calls.
EngineSoundScheduledNode *Audio82B884::ResolveSound468F70(unsigned int id) {
    AudioCarSoundManager *manager = reinterpret_cast<AudioCarSoundManager **>(reinterpret_cast<char *>(this) + 0xa8)[(id >> 16) & 0xff];
    if (!manager) return 0;
    const unsigned int index = (id >> 11) & 0x1f, component = (id >> 4) & 0x7f;
    AudioManagedCarSound *node = manager->first10;
    while (node && *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(node) + 0x10) != index) node = node->next4;
    if (!node) return 0;
    if ((id & 0xe0000000) == 0x40000000)
        return manager->FindIndex463C50(index)->FindPlayback464CD0(component);
    return 0;
}
EngineSoundScheduledNode *Audio82B884::ResolveInput468FE0(unsigned int id) {
    AudioCarSoundManager *manager = reinterpret_cast<AudioCarSoundManager **>(reinterpret_cast<char *>(this) + 0xa8)[(id >> 16) & 0xff];
    if (!manager) return 0;
    const unsigned int index = (id >> 11) & 0x1f, component = (id >> 4) & 0x7f;
    AudioManagedCarSound *node = manager->first10;
    while (node && *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(node) + 0x10) != index) node = node->next4;
    if (node && (id & 0xe0000000) == 0x60000000) {
        EngineSoundScheduledNode *input = manager->FindIndex463C50(index)->FindInput464D00(component);
        if (input) return input;
    }
    return 0;
}
