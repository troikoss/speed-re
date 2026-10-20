#include "EngineSoundMixer.h"
// 0x478120: only local-kind 3D entries count as spatial sub-channel inputs.
void EngineSoundMixer::Allocate478120() {
    for (int index = 0; index < subCount68; ++index) {
        EngineSoundMixerChannel *channel = sub30[index];
        int remaining = channel->count30, fixed = 0, spatial = 0;
        unsigned int *input = channel->inputs34;
        const unsigned int kind = kind44;
        while (remaining > 0) {
            const unsigned int id = *input++;
            const unsigned int inputKind = (id >> 16) & 0xff;
            if (inputKind == kind) {
                if ((id & 0xe0000000) == 0x80000000) ++spatial;
                else ++fixed;
            } else fixed = (int)((unsigned int)fixed + reinterpret_cast<unsigned int *>(owner4)[inputKind + 1]);
            --remaining;
        }
        EngineSoundDiagnostic4691C0(0, engineSoundMixerSubCount78B7C8, channel->value8, fixed);
        EngineSoundDiagnostic4691C0(0, engineSoundMixerSubSpatial78B7A0, sub30[index]->value8, spatial);
        sub30[index]->count2C = spatial;
        EngineSoundMixerOwner *owner = owner4;
        int *storage = owner->subInputs7C + owner->subCountA8;
        owner->subCountA8 = (int)((unsigned int)owner->subCountA8 + (unsigned int)spatial + (unsigned int)fixed);
        channel = sub30[index];
        channel->all40 = storage; channel->count28 = fixed; channel->fixed38 = storage;
        channel->spatial3C = channel->count2C > 0 ? storage + fixed : 0;
        const int total = (int)((unsigned int)fixed + (unsigned int)spatial);
        for (int j = 0; j < total; ++j) storage[j] = 0;
    }
}
// 0x483900: reload the group/session arrays and counts after each callback.
void EngineSoundMixerSetup::Bind483900() {
    for (int group = 0; group < table28->count4; ++group) {
        EngineSoundMixerGroup *value = groups38[group];
        if (value && value->count48 > 0) {
            for (int index = 0; index < groups38[group]->count48; ++index) {
                groups38[group]->mixers78[index]->Allocate478120();
                groups38[group]->mixers78[index]->Bind480420();
            }
        }
    }
}
