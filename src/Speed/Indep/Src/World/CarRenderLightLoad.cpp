#include "CarRenderLightLoad.h"

#ifdef __GNUC__
typedef __UINTPTR_TYPE__ LightAddress;
#else
typedef unsigned __int64 LightAddress;
#endif
namespace {
const volatile float light78D780 = 1.0f;
}

// Original 0x48e070. Valid retail chunks have complete child records and the
// expected header before dependent children. No invented repair of malformed
// lengths, missing headers, duplicate registrations or out-of-range IDs occurs.
unsigned int CarRenderLoadLightChunk(unsigned int *chunk) {
    if (chunk[0] == 0x135200) {
        CarRenderLightPack *pack = reinterpret_cast<CarRenderLightPack *>(chunk + 2);
        pack->value80 = static_cast<long double>(light78D780) * pack->value80;
        pack->value90 = static_cast<long double>(light78D780) * pack->value90;
        if (pack->kind10 == 3) {
            pack->AllocateState();
            pack->ApplyDefaults();
            CarRenderLightLink *link = &pack->link04;
            CarRenderLightLink *last = reinterpret_cast<CarRenderLightLink *>(ug2LightProfiles8358C0.previous04);
            last->next00 = link;
            ug2LightProfiles8358C0.previous04 = reinterpret_cast<CarRenderLightSector *>(link);
            link->previous04 = last;
            link->next00 = reinterpret_cast<CarRenderLightLink *>(&ug2LightProfiles8358C0);
            if (pack->hash0C == 0x2c420d64) ug2LightDefaultPack7F399C = pack;
        }
        return 1;
    }
    if (chunk[0] == 0x80135100) {
        unsigned char *end = reinterpret_cast<unsigned char *>(chunk) + chunk[1] + 8;
        unsigned int *child = chunk + 2;
        CarRenderLightAnimation *animation = 0;
        while (reinterpret_cast<unsigned char *>(child) != end) {
            if (child[0] == 0x135101) {
                animation = reinterpret_cast<CarRenderLightAnimation *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
                if (!animation->initialized52) animation->initialized52 = 1;
                if (animation->kind08 != 3) animation = 0;
                else {
                    animation->list58.next00 = &animation->list58;
                    animation->list58.previous04 = &animation->list58;
                }
            } else if (child[0] == 0x135102 && animation) {
                const unsigned int count = animation->count50;
                CarRenderLightLink *record = reinterpret_cast<CarRenderLightLink *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
                for (unsigned int i = 0; i < count; ++i) {
                    CarRenderLightLink *last = animation->list58.previous04;
                    last->next00 = record;
                    animation->list58.previous04 = record;
                    record->previous04 = last;
                    record->next00 = &animation->list58;
                    record = reinterpret_cast<CarRenderLightLink *>(reinterpret_cast<unsigned char *>(record) + 0x30);
                }
            }
            child = reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child) + child[1] + 8);
        }
        if (animation) {
            CarRenderLightLink *link = &animation->link00;
            CarRenderLightLink *last = reinterpret_cast<CarRenderLightLink *>(ug2LightAnimations830AE8.previous04);
            last->next00 = link;
            ug2LightAnimations830AE8.previous04 = reinterpret_cast<CarRenderLightSector *>(link);
            link->previous04 = last;
            link->next00 = reinterpret_cast<CarRenderLightLink *>(&ug2LightAnimations830AE8);
            trackTopology.AcquireResource(animation->section54)->value8 = animation;
        }
        return 1;
    }
    if (chunk[0] == 0x80135000) {
        unsigned char *end = reinterpret_cast<unsigned char *>(chunk) + chunk[1] + 8;
        unsigned int *child = chunk + 2;
        CarRenderLightSector *sector = 0;
        bool fresh = false;
        while (reinterpret_cast<unsigned char *>(child) != end) {
            if (child[0] == 0x135001) {
                sector = reinterpret_cast<CarRenderLightSector *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
                fresh = sector->initialized0A == 0;
                if (!sector->initialized0A) sector->initialized0A = 1;
                if (sector->version08 != 4) return 1;
            } else if (child[0] == 0x135002) {
                CarRenderLightTree *tree = reinterpret_cast<CarRenderLightTree *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
                sector->tree10 = tree;
                tree->nodes00 = reinterpret_cast<CarRenderLightTreeNode *>(reinterpret_cast<unsigned char *>(tree) + 0x10);
                if (fresh) tree->LoadNoop();
            } else if (child[0] == 0x135003) {
                sector->lights18 = reinterpret_cast<CarRenderLight *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
            }
            child = reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child) + child[1] + 8);
        }
        if (sector) {
            CarRenderLightSector *last = ug2LightSectors830D14.previous04;
            last->next00 = sector;
            ug2LightSectors830D14.previous04 = sector;
            sector->previous04 = last;
            sector->next00 = reinterpret_cast<CarRenderLightSector *>(&ug2LightSectors830D14);
            trackTopology.AcquireResource(sector->section0C)->valueC = sector;
        }
        return 1;
    }
    return 0;
}

// Original 0x48e270. The primary-container path assumes its first child is
// 0x135001: a mismatched child loops at 0x48e330 without advancing. Preserve it.
unsigned int CarRenderUnloadLightChunk(unsigned int *chunk) {
    if (chunk[0] == 0x135200) {
        if (chunk[6] == 3) {
            CarRenderLightLink *link = reinterpret_cast<CarRenderLightLink *>(chunk + 3);
            CarRenderLightLink *next = link->next00;
            CarRenderLightLink *previous = link->previous04;
            previous->next00 = next;
            next->previous04 = previous;
            if (chunk[5] == 0x2c420d64) ug2LightDefaultPack7F399C = &ug2LightFallbackPack830BD8;
        }
        return 1;
    }
    if (chunk[0] == 0x80135100 || chunk[0] == 0x80135000) {
        unsigned char *end = reinterpret_cast<unsigned char *>(chunk) + chunk[1] + 8;
        unsigned int *child = chunk + 2;
        if (reinterpret_cast<unsigned char *>(child) == end) return 1;
        if (chunk[0] == 0x80135100) {
            while (child[0] != 0x135101) {
                child = reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(child) + child[1] + 8);
                if (reinterpret_cast<unsigned char *>(child) == end) return 1;
            }
            CarRenderLightAnimation *animation = reinterpret_cast<CarRenderLightAnimation *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
            if (animation->kind08 == 3) {
                CarRenderLightLink *next = animation->link00.next00;
                CarRenderLightLink *previous = animation->link00.previous04;
                previous->next00 = next;
                next->previous04 = previous;
                trackTopology.resources50[animation->section54]->value8 = 0;
                trackTopology.ReleaseResource(animation->section54);
            }
        } else {
            while (child[0] != 0x135001) {}
            CarRenderLightSector *sector = reinterpret_cast<CarRenderLightSector *>((reinterpret_cast<LightAddress>(child) + 0x17) & ~LightAddress(15));
            if (sector->version08 == 4) {
                CarRenderLightSector *next = sector->next00;
                CarRenderLightSector *previous = sector->previous04;
                previous->next00 = next;
                next->previous04 = previous;
                trackTopology.resources50[sector->section0C]->valueC = 0;
                trackTopology.ReleaseResource(sector->section0C);
            }
        }
        return 1;
    }
    return 0;
}

// Original 0x48e380.
unsigned int CarRenderRegisterAdditionalLights(CarRenderLightSector *sector) {
    CarRenderLightSector *last = ug2AdditionalLights835850.previous04;
    if (!sector) return 0;
    last->next00 = sector;
    ug2AdditionalLights835850.previous04 = sector;
    sector->previous04 = last;
    sector->next00 = reinterpret_cast<CarRenderLightSector *>(&ug2AdditionalLights835850);
    return 1;
}

// Original 0x48e3b0.
unsigned int CarRenderRemoveLightSector(CarRenderLightSector *sector) {
    if (!sector) return 0;
    CarRenderLightSector *next = sector->next00;
    CarRenderLightSector *previous = sector->previous04;
    previous->next00 = next;
    next->previous04 = previous;
    return 1;
}
