#include "EngineSoundMixer.h"
#include "EngineSoundSchedule.h"
#include "../../Audio/AudioSystem.h"
extern Audio82B884 *audio82B884;
// 0x480420: construct master-channel spans, attach 3D inputs and bind packed maps.
void EngineSoundMixer::Bind480420() {
    unsigned int *map = reinterpret_cast<unsigned int *>(asset8 + *reinterpret_cast<int *>(asset8 + 0x14));
    for (int index = 0; index < masterCount6C; ++index) {
        EngineSoundMixerChannel *channel = master34[index];
        unsigned int *cursor = channel->inputs34;
        const int inputCount = channel->count30;
        int fixed = 0, spatial = 0;
        const unsigned int key = channel->vtable0->value10(channel);
        int group = 0;
        bool found = false;
        unsigned int previous = 0;
        for (int candidate = 0; candidate < masterCount6C; ++candidate) {
            const unsigned int current = master34[candidate]->vtable0->value10(master34[candidate]);
            if ((current & 0xfffffff0) == (key & 0xfffffff0)) {
                if (!found) { found = true; channel->index48 = group; }
                if (current == key) break;
            } else if (previous != current) ++group;
            previous = current;
        }
        if ((channel->value8 & 0xff000000) == 0xc0000000)
            channel->values4C = values40 + (unsigned int)channel->index48 * 16u;
        channel->map50 = map;
        map += (*map & 0xf) + 1;
        for (int input = 0; input < inputCount; ++input) {
            const unsigned int id = *cursor++;
            if ((id & 0xe0000000) == 0x80000000) ++spatial;
            else {
                const unsigned int inputKind = (id >> 16) & 0xff;
                if (inputKind == kind44) ++fixed;
                else fixed = (int)((unsigned int)fixed + reinterpret_cast<unsigned int *>(owner4)[inputKind + 1]);
            }
        }
        EngineSoundDiagnostic4691C0(0, engineSoundMixerMasterCount78BDC4, master34[index]->value8, master34[index]->valueC, fixed);
        EngineSoundDiagnostic4691C0(0, engineSoundMixerMasterSpatial78BD98, master34[index]->value8, spatial);
        master34[index]->count2C = spatial;
        EngineSoundMixerOwner *owner = owner4;
        int *storage = owner->masterInputs78 + owner->masterCountA4;
        owner->masterCountA4 = (int)((unsigned int)owner->masterCountA4 + (unsigned int)spatial + (unsigned int)fixed);
        EngineSoundMixerChannel *currentChannel = master34[index];
        currentChannel->all40 = storage; currentChannel->count28 = fixed; currentChannel->fixed38 = storage;
        currentChannel->spatial3C = currentChannel->count2C > 0 ? storage + fixed : 0;
        const int total = (int)((unsigned int)fixed + (unsigned int)spatial);
        for (int j = 0; j < total; ++j) storage[j] = 0;
        currentChannel = master34[index];
        cursor = currentChannel->inputs34;
        currentChannel->vtable0->value10(currentChannel);
        currentChannel = master34[index];
        const unsigned int instance = index50;
        const unsigned int soundKey = currentChannel->vtable0->value10(currentChannel);
        EngineSoundScheduledNode *sound = audio82B884->ResolveSound468F70((soundKey & 0xe0ffffff) | (instance << 11));
        if (!sound) {
            currentChannel = master34[index];
            const unsigned int missing = currentChannel->vtable0->value10(currentChannel);
            EngineSoundDiagnostic4691C0(0, engineSoundMixerMissing78BD24, missing);
            channel->values4C = 0;
            continue;
        }
        const int connections = master34[index]->count2C;
        if (!connections) {
            unsigned int *words = channel->map50;
            for (unsigned int remaining = *words++ & 0xf; remaining; --remaining) {
                const unsigned int value = *words;
                const unsigned int metadata = master34[index]->value18;
                reinterpret_cast<int **>(reinterpret_cast<char *>(sound) + 0x58)[(value >> 16) & 0xf] = channel->values4C + ((value >> 24) & 0xf);
                *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(sound) + 0x98) = metadata;
                ++words;
            }
        } else {
            for (int connection = 0; connection < connections; ++connection) {
                const unsigned int raw = *cursor++;
                const unsigned int lookup = (index50 << 11) | raw;
                EngineSoundDiagnostic4691C0(0, engineSoundMixerLookup78BD74, lookup, sound->key4C);
                int candidate = 0;
                while (candidate < count60) {
                    if (spatial24[candidate]->vtable0->value8(spatial24[candidate]) == lookup) break;
                    ++candidate;
                }
                EngineSoundMixerChannel *entry = candidate < count60 ? spatial24[candidate] : spatial24[lookup & 0xff];
                if (!entry) break;
                const int value = entry->vtable0->value18(entry);
                master34[index]->spatial3C[connection] = value;
                const unsigned int inputKey = entry->vtable0->valueC(entry);
                EngineSoundMixerChannelVtable *table = entry->vtable0;
                EngineSoundScheduledNode *input = audio82B884->ResolveInput468FE0((inputKey & 0x00fffff0) | 0x60000000);
                table->attach14(entry, 0, master34[index], reinterpret_cast<EngineSound *>(sound), input);
            }
            unsigned int *words = channel->map50;
            for (unsigned int remaining = *words++ & 0xf; remaining; --remaining) {
                const unsigned int value = *words++;
                reinterpret_cast<int **>(reinterpret_cast<char *>(sound) + 0x58)[(value >> 16) & 0xf] = channel->values4C + ((value >> 24) & 0xf);
                *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(sound) + 0x98) = 0;
            }
            EngineSoundDiagnostic4691C0(0, engineSoundMixerMap78BD4C, master34[index]->value8, *cursor);
            master34[index]->inputs34 = cursor;
        }
    }
}
