// Native ownership/file validation for the recovered single-car mixer binder.
// Output values retain the previous unity-gain fixture until fixed-input and
// channel runtime recovery is complete. This class does not simulate mixing.
#ifndef UG2_NATIVE_ENGINE_MIXER_H
#define UG2_NATIVE_ENGINE_MIXER_H
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMixer.h"
#include "../../Speed/Indep/Src/Audio/AudioSystem.h"
#include <vector>
#include <stdio.h>
#include <string.h>
extern Audio82B884 *audio82B884;

class NativeEngineMixer {
    std::vector<unsigned int> asset_;
    std::vector<EngineSoundMixerChannel> channels_;
    std::vector<EngineSoundMixerSpatial> spatial_;
    std::vector<EngineSoundMixerChannel *> channelsView_, spatialView_;
    std::vector<int> outputs_, inputs_;
    EngineSoundMixerChannelVtable channelTable_, spatialTable_;
    EngineSoundMixer mixer_;
    EngineSoundMixerOwner owner_;
    static void __fastcall SetOutput(EngineSoundMixerChannel *c, void *, unsigned int id) { c->SetOutput467400(id); }
    static unsigned int __fastcall Output(EngineSoundMixerChannel *c) { return c->Output467410(); }
    static unsigned int __fastcall SpatialKey(EngineSoundMixerChannel *c) { return reinterpret_cast<EngineSoundMixerSpatial *>(c)->Key503B00(); }
    static unsigned int __fastcall SpatialInput(EngineSoundMixerChannel *c) { return reinterpret_cast<EngineSoundMixerSpatial *>(c)->Input6E18A0(); }
    static int __fastcall SpatialValues(EngineSoundMixerChannel *c) { return reinterpret_cast<int>(reinterpret_cast<EngineSoundMixerSpatial *>(c)->Values40BDE0()); }
    static void __fastcall Attach(EngineSoundMixerChannel *c, void *, EngineSoundMixerChannel *channel, EngineSound *sound, EngineSoundScheduledNode *input) {
        reinterpret_cast<EngineSoundMixerSpatial *>(c)->Attach468430(channel, sound, input);
    }
    bool Span(unsigned int at, unsigned int words) const { return at <= asset_.size() && words <= asset_.size() - at; }
public:
    bool Load(const char *directory, Audio82B884 *audio, EngineSoundCar *car) {
        // File/profile selection and allocations remain native scaffolding.
        if (car->kind14 != 1 || car->bankIndex10 != 0) return false;
        char path[4096];
        if (snprintf(path, sizeof(path), "%s/SOUND/MIXMAPS/MAPOUTPUT.mxb", directory) >= (int)sizeof(path)) return false;
        FILE *file = fopen(path, "rb");
        if (!file) return false;
        fseek(file, 0, SEEK_END); const long length = ftell(file); rewind(file);
        if (length < 16 || length > 1024 * 1024 || length % 4) { fclose(file); return false; }
        asset_.resize(length / 4);
        const bool read = fread(&asset_[0], 1, length, file) == (size_t)length;
        fclose(file);
        if (!read || asset_[2] % 4 || !Span(asset_[2] / 4, asset_[1])) return false;
        unsigned int group = 0;
        for (unsigned int i = 0; i < asset_[1]; ++i) {
            const unsigned int offset = asset_[asset_[2] / 4 + i];
            if (offset % 4 || !Span(offset / 4, 8)) return false;
            if ((asset_[offset / 4] & 0xffff) == (unsigned int)car->kind14) { group = offset / 4; break; }
        }
        if (!group) return false;
        unsigned int section[3];
        for (int i = 0; i < 3; ++i) {
            const unsigned int offset = asset_[group + (i == 0 ? 2 : i == 1 ? 4 : 5)];
            if (offset % 4 || offset / 4 > asset_.size() - group) return false;
            section[i] = group + offset / 4;
            if (!Span(section[i], i == 2 ? 1 : 4)) return false;
        }
        const unsigned int spatialCount = asset_[section[0]], channelCount = asset_[section[1]], groups = asset_[section[1]+1];
        if (!spatialCount || spatialCount > 64 || !channelCount || channelCount > 64 || !groups || groups > 64) return false;
        // Validate every span before passing retail records to recovered code.
        unsigned int cursor = section[0] + 4;
        for (unsigned int i = 0; i < spatialCount; ++i) {
            if (!Span(cursor, 1)) return false;
            const unsigned int words = 1 + ((asset_[cursor] >> 24) & 15) * 6;
            if (words < 7 || !Span(cursor, words) || ((asset_[cursor] >> 16) & 255) > 8) return false;
            cursor += words;
        }
        cursor = section[1] + 4;
        unsigned int map = section[2], capacity = 0;
        for (unsigned int i = 0; i < channelCount; ++i) {
            if (!Span(cursor, 3) || !Span(map, 1)) return false;
            const unsigned int count = (asset_[cursor] >> 16) & 255;
            if (asset_[cursor] & 0x10000000 || !Span(cursor, 3 + count) || !Span(map, 1 + (asset_[map] & 15))) return false;
            if (((asset_[cursor + 2] >> 16) & 255) > 8) return false;
            bool fixedSeen = false;
            for (unsigned int j = 0; j < count; ++j) {
                const unsigned int id = asset_[cursor + 3 + j];
                if (((id >> 16) & 255) > 8) return false;
                if ((id & 0xe0000000) == 0x80000000) {
                    if (fixedSeen || (id & 255) >= spatialCount) return false;
                } else fixedSeen = true;
            }
            capacity += count; cursor += 3 + count; map += 1 + (asset_[map] & 15);
        }
        channels_.resize(channelCount); channelsView_.resize(channelCount);
        spatial_.resize(spatialCount); spatialView_.resize(spatialCount);
        outputs_.assign((groups > channelCount ? groups : channelCount) * 16, 32767); inputs_.assign(capacity, 0);
        memset(&mixer_, 0, sizeof(mixer_)); memset(&owner_, 0, sizeof(owner_));
        memset(&channelTable_, 0, sizeof(channelTable_)); memset(&spatialTable_, 0, sizeof(spatialTable_));
        channelTable_.valueC = reinterpret_cast<unsigned int (__fastcall *)(EngineSoundMixerChannel *)>(SetOutput);
        channelTable_.value10 = Output;
        spatialTable_.value8 = SpatialKey; spatialTable_.valueC = SpatialInput;
        spatialTable_.value18 = SpatialValues; spatialTable_.attach14 = Attach;
        cursor = section[0] + 4;
        for (unsigned int i = 0; i < spatialCount; ++i) {
            spatial_[i].vtable0 = &spatialTable_;
            spatial_[i].Setup4683E0(&asset_[cursor], car->bankIndex10 << 11, i);
            spatialView_[i] = reinterpret_cast<EngineSoundMixerChannel *>(&spatial_[i]);
            cursor += 1 + ((asset_[cursor] >> 24) & 15) * 6;
        }
        cursor = section[1] + 4;
        for (unsigned int i = 0; i < channelCount; ++i) {
            EngineSoundMixerChannel &channel = channels_[i];
            channel.vtable0 = &channelTable_; channelsView_[i] = &channel;
            channel.Setup477A50(asset_[cursor], &asset_[section[1]]);
            unsigned int *next = EngineSoundMixerDecode479030(&asset_[section[1]], &channel, &asset_[cursor]);
            cursor = next - &asset_[0] + channel.count30;
        }
        reinterpret_cast<int *>(&owner_)[1] = 1;
        reinterpret_cast<int *>(&owner_)[car->kind14 + 1] = 1;
        owner_.masterInputs78 = inputs_.empty() ? 0 : &inputs_[0];
        mixer_.owner4 = &owner_; mixer_.asset8 = reinterpret_cast<char *>(&asset_[group]);
        mixer_.spatial24 = &spatialView_[0]; mixer_.count60 = spatialCount;
        mixer_.master34 = &channelsView_[0]; mixer_.masterCount6C = channelCount;
        mixer_.values40 = &outputs_[0]; mixer_.kind44 = car->kind14; mixer_.index50 = car->bankIndex10;
        // Reject missing native input components before the original attachment
        // body dereferences them. This is a guard, not a replacement resolver.
        for (unsigned int i = 0; i < channelCount; ++i) {
            if (!audio->ResolveSound468F70(channels_[i].Output467410())) continue;
            for (int j = 0; j < channels_[i].count30; ++j) {
                const unsigned int id = channels_[i].inputs34[j];
                if ((id & 0xe0000000) != 0x80000000) break;
                unsigned int index = 0;
                while (index < spatialCount && spatial_[index].Key503B00() != id) ++index;
                if (index == spatialCount) index = id & 255;
                const unsigned int key = (spatial_[index].Input6E18A0() & 0x00fffff0) | 0x60000000;
                if (!audio->ResolveInput468FE0(key)) return false;
            }
        }
        EngineSoundMixerSetup setup = {};
        EngineSoundMixerTable table = {}; table.count4 = 1;
        EngineSoundMixerGroup entry = {}; entry.count48 = 1;
        EngineSoundMixer *session = &mixer_; entry.mixers78 = &session;
        EngineSoundMixerGroup *entryPointer = &entry;
        setup.table28 = &table; setup.groups38 = &entryPointer;
        Audio82B884 *previous = audio82B884; audio82B884 = audio;
        setup.Bind483900();
        audio82B884 = previous;
        fprintf(stderr, "ug2demo: mixer bound MAPOUTPUT.mxb kind %d: %u master channels, %u spatial entries (output values remain unity fixtures)\n", car->kind14, channelCount, spatialCount);
        return true;
    }
};
#endif
