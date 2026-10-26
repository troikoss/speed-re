#include "CarRenderSolid.h"
#include "CarRenderLightLoad.h"

void *bOMalloc(void *pool); // Original 0x440e50, recovered in bSlotPool.cpp.

// Original 0x4905a0. Resolve sentinel values first, then swap the first matching
// override into each texture slot. Duplicate keys/slots retain the binary's order.
void CarRenderSolid::ApplyTextures() {
    CarRenderSlotTable *table = table0C;
    int textureCount = table->textureCount19;
    CarRenderTextureOverride *overrides = overrides10;
    int count = overrideCount14;
    if (overrides && count != 0) {
        for (int i = 0; i < count; ++i) {
            if (overrides[i].value08 == reinterpret_cast<CarRenderTexture *>(-1)) {
                overrides[i].value08 = UG2SolidFindTexture(overrides[i].hash04, 1, 0);
            }
        }
        for (int i = 0; i < textureCount; ++i) {
            CarRenderTextureSlot *slot = table->textures2C + i;
            for (int j = 0; j < count; ++j) {
                CarRenderTextureOverride *entry = overrides + j;
                if (entry->hash04 != 0 && entry->key00 == table->textures2C[i].hash00) {
                    CarRenderTexture *old = slot->texture04;
                    slot->texture04 = entry->value08;
                    entry->location0C = &table->textures2C[i].texture04;
                    entry->value08 = old;
                    break;
                }
            }
        }
    }
}

// Original 0x488e40. Restoration is a swap, including the saved value; a reused
// override is not equivalent to saving a fresh copy of the texture table.
void CarRenderSolid::RestoreTextures() {
    int count = overrideCount14;
    if (overrides10 && table0C && count > 0) {
        CarRenderTextureOverride *entry = overrides10;
        do {
            CarRenderTexture **location = entry->location0C;
            if (location) {
                CarRenderTexture *old = *location;
                *location = entry->value08;
                entry->location0C = 0;
                entry->value08 = old;
            }
            ++entry;
        } while (--count != 0);
    }
}

// Original 0x48ac50 / 0x60fdf0.
CarRenderLightPack *CarRenderDefaultLightPack() { return ug2LightDefaultPack7F399C; }
bool CarRenderSpecialMode() { return ug2SolidMode8A1C8C == 4; }

// Original 0x5c5930, __thiscall on the view, RET 0x14. Dispatch only: GPU state
// and drawing occur later in 0x5cf420, after commands have been grouped.
void CarRenderView::DrawSolid(CarRenderSolid *solid, WheelMatrix *world, void *record,
                              unsigned int flags, unsigned int zero) {
    CarRenderSlotTable *table = solid->table0C;
    if (table) {
        CarRenderPlatformSolid *platform = table->platform00;
        CarRenderTechnique *technique = platform->technique3C;
        int initialType = technique->type04;
        if ((flags & 0x200) && ug2SolidFlag870CC4) {
            platform->remap38 = ug2SolidFlag870CD4 ? ug2SolidRemap7FF090 : ug2SolidRemap7FF0B4;
        } else if (!ug2SolidFlag870CD4) {
            platform->remap38 = ug2SolidRemap7FF0D8;
        }
        bool debug = ug2SolidDebug8707CD && *ug2SolidViewMode870848 == 0;
        if (platform->remap38) {
            int index = platform->remap38[technique->type04];
            if (index != -1) technique = ug2SolidTechniques86EB28[index];
            platform->remap38 = 0;
        }
        if (solid->overrides10) solid->ApplyTextures();
        if (solid->hash08 == 0xbe43edbb && CarRenderSpecialMode() && ug2SolidFlag870D14) {
            technique = ug2SolidTechniques86EB28[20];
        }
        if (technique->value10) {
            if (*ug2SolidViewMode870848 != 0 && technique->type04 != 5) {
                technique = ug2SolidTechniques86EB28[7];
            }
            for (int i = 0; i < platform->primitiveCount10; ++i) {
                CarRenderPrimitive *primitive = platform->primitives14 + i;
                if ((flags & 4) || (primitive->flags38 & 1) ||
                    TestBounds(primitive->minimum00, primitive->maximum10, world)) {
                    CarRenderTexture *texture = table->textures2C[primitive->texture1C].texture04;
                    if (texture->hash24 == ug2SolidSkipTexture870A18) continue;
                    CarRenderLightPack *pack = primitive->lightPack20 == 0xff ?
                        CarRenderDefaultLightPack() : table->slots3C[primitive->lightPack20].lightPack;
                    unsigned char material = texture->material56;
                    CarRenderTechnique *candidate = technique;
                    if (material == 2 && (technique->type04 == 1 || technique->type04 == 0)) {
                        candidate = ug2SolidTechniques86EB28[7];
                    }
                    if (debug) {
                        if (material != 2 && (initialType == 2 || initialType == 3)) {
                            switch (candidate->type04) {
                            case 0: candidate = ug2SolidTechniques86EB28[9]; break;
                            case 1: candidate = ug2SolidTechniques86EB28[10]; break;
                            case 2: candidate = ug2SolidTechniques86EB28[11]; break;
                            case 3: candidate = ug2SolidTechniques86EB28[12]; break;
                            }
                        } else if (material != 0 && candidate->type04 == 0) {
                            candidate = ug2SolidTechniques86EB28[13];
                        } else if (material != 0 && (candidate->type04 == 1 || candidate->type04 == 7)) {
                            candidate = ug2SolidTechniques86EB28[14];
                        } else if (candidate->type04 == 2) {
                            candidate = ug2SolidTechniques86EB28[11];
                        } else if (candidate->type04 == 3) {
                            candidate = ug2SolidTechniques86EB28[12];
                        }
                    }
                    if (candidate->type04 == 5 && !record) record = record54;
                    if (texture->hash24 == 0x06d57415 || texture->hash24 == 0xbd3d01a8) {
                        candidate = ug2SolidTechniques86EB28[22];
                    }
                    CarRenderQueuePrimitive(primitive, table, primitive->flags38, candidate,
                                            texture, world, record, pack, zero);
                }
            }
        }
        solid->RestoreTextures();
    }
}

// Original 0x5ceb30. Cached texture groups are reused only for the same technique.
// Both lists append at the tail; the consumer later unlinks and frees each node.
void CarRenderQueuePrimitive(CarRenderPrimitive *primitive, CarRenderSlotTable *table,
                             unsigned int flags, CarRenderTechnique *technique,
                             CarRenderTexture *texture, WheelMatrix *world, void *record,
                             CarRenderLightPack *lightPack, unsigned int zero) {
    CarRenderTextureState *state = texture->state00;
    if (!state->group1C || state->group1C->technique14 != technique) {
        CarRenderTextureGroup *group = static_cast<CarRenderTextureGroup *>(bOMalloc(ug2SolidGroupPool870918));
        if (group) {
            group->commands18.next00 = &group->commands18;
            group->commands18.previous04 = &group->commands18;
            group->texture10 = texture;
            group->count08 = 0;
            group->indexCount0C = 0;
        }
        state->group1C = group;
        CarRenderQueueLink *heads = texture->value54 == 0 ? ug2SolidGroups870D30 : ug2SolidGroups8712F0;
        CarRenderQueueLink *head = heads + texture->priority51 * 23 + technique->type04;
        CarRenderQueueLink *last = head->previous04;
        last->next00 = &group->link00;
        head->previous04 = &group->link00;
        group->link00.previous04 = last;
        group->link00.next00 = head;
        ++ug2SolidGroupCount87094C;
    }
    CarRenderTextureGroup *group = state->group1C;
    CarRenderCommand *command = static_cast<CarRenderCommand *>(bOMalloc(ug2SolidCommandPool87091C));
    if (command) {
        command->table10 = table;
        command->flags14 = flags;
        command->record1C = record;
        command->lightPack20 = lightPack;
        command->primitive08 = primitive;
        command->value0C = 0;
        command->world24 = world;
        command->technique18 = technique;
        command->value28 = zero;
    }
    CarRenderQueueLink *last = group->commands18.previous04;
    last->next00 = &command->link00;
    group->commands18.previous04 = &command->link00;
    command->link00.next00 = &group->commands18;
    command->link00.previous04 = last;
    group->technique14 = technique;
    ++group->count08;
    group->indexCount0C += primitive->indexCount0C;
}
