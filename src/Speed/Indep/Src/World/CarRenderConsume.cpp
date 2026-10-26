#include "CarRenderConsume.h"

namespace {
void Unlink(CarRenderQueueLink *link) {
    link->previous04->next00 = link->next00;
    link->next00->previous04 = link->previous04;
}
}

// Original 0x5cdf10, __thiscall. Always re-read the head after returning a slot.
void CarRenderCommandList::Clear() {
    while (head00.next00 != &head00) {
        CarRenderQueueLink *command = head00.next00;
        Unlink(command);
        bFree(ug2SolidCommandPool87091C,command);
    }
}
// Original 0x5cea20, tail-call through the head at +0x18.
void CarRenderTextureGroup::ClearCommands() {
    reinterpret_cast<CarRenderCommandList *>(&commands18)->Clear();
}
// Original 0x48b1e0 / 0x60fe00 / 0x60fde0.
CarRenderViewRecord *CarRenderViewByIndex(int index) { return ug2RenderViews832DE0+index; }
unsigned int CarRenderSkyIndex() { return ug2RenderSkyIndex8A1C94; }
long double CarRenderSkyFraction() {
    return static_cast<long double>(ug2RenderSkyTime8A1C9C)/ug2RenderSkyPeriod802954;
}

// Original 0x5cf420. The 16*23 heads cover both texture queue sets, in address order.
void CarRenderConsumeCommands() {
    ++ug2RenderFrames872EDC;
    for (int row=0;row<16;++row) {
        for (int index=0;index<23;++index) {
            // The two native arrays need not be adjacent in host storage.
            CarRenderQueueLink *heads = row<8 ? ug2SolidGroups870D30 : ug2SolidGroups8712F0;
            CarRenderQueueLink *head = heads+(row&7)*23+index;
            if (head->next00 == head) continue;
            CarRenderTechnique *technique = ug2SolidTechniques86EB28[index];
            ug2RenderTechnique870910 = technique;
            UG2RenderTechniqueBegin(technique);
            UG2RenderSetDeclaration(ug2SolidDevice870974,technique->declaration24);
            unsigned int passes;
            UG2EffectBegin(technique->effect20,&passes,0);
            ++ug2RenderTechniques872F04;
            while (head->next00 != head) {
                CarRenderTextureGroup *group = reinterpret_cast<CarRenderTextureGroup *>(head->next00);
                Unlink(&group->link00);
                ++ug2RenderGroups872ED8;
                CarRenderBindTexture(group->texture10);
                while (group->commands18.next00 != &group->commands18) {
                    CarRenderCommand *command = reinterpret_cast<CarRenderCommand *>(group->commands18.next00);
                    Unlink(&command->link00);
                    CarRenderTexture *texture = group->texture10;
                    CarRenderUploadWorld(command->world24,ug2SolidViewMode870848[1]);
                    if (command->technique18->parameters1C->slots00[35].handle24)
                        CarRenderUploadHeadlights(command->world24,CarRenderViewByIndex(9));
                    if (command->record1C) CarRenderUploadRecord(command->record1C,command->world24);
                    if (command->lightPack20) CarRenderUploadLightPack(command->lightPack20,command->flags14);
                    CarRenderPrimitive *primitive = command->primitive08;
                    if (command->value0C == 0) {
                        if (command->value28 && (command->flags14&1))
                            CarRenderUploadBones(reinterpret_cast<void *>(command->value28),primitive);
                        CarRenderPlatformSolid *solid = command->table10->platform00;
                        if (!solid->vertexBuffer2C) {
                            solid->value40 = 1;
                            UG2SolidCreateVertexBuffer(ug2SolidDevice870974,solid->vertexBytes30,8,0,1,&solid->vertexBuffer2C,0);
                            void *output;
                            UG2SolidLockBuffer(solid->vertexBuffer2C,0,0,&output,0);
                            unsigned int *target = static_cast<unsigned int *>(output);
                            const unsigned int *input = reinterpret_cast<const unsigned int *>(solid->vertices28);
                            for (unsigned int words=solid->vertexBytes30>>2;words;--words) *target++ = *input++;
                            unsigned char *tail = reinterpret_cast<unsigned char *>(target);
                            const unsigned char *source = reinterpret_cast<const unsigned char *>(input);
                            for (unsigned int bytes=solid->vertexBytes30&3;bytes;--bytes) *tail++ = *source++;
                            UG2SolidUnlockBuffer(solid->vertexBuffer2C);
                        }
                        if (solid->vertexBuffer2C != ug2RenderVertex87092C) {
                            UG2RenderSetStream(ug2SolidDevice870974,0,solid->vertexBuffer2C,0,ug2RenderTechnique870910->stride08);
                            ug2RenderVertex87092C = solid->vertexBuffer2C;
                            ++ug2RenderVertexChanges872EE8;
                        }
                        if (solid->indexBuffer20 != ug2RenderIndex870930) {
                            UG2RenderSetIndices(ug2SolidDevice870974,solid->indexBuffer20);
                            ug2RenderIndex870930 = solid->indexBuffer20;
                            ++ug2RenderIndexChanges872EEC;
                        }
                        if (primitive->indexCount0C && solid->vertexCount34<0xffff) {
                            int count = command->technique18->passCount14;
                            int type = ug2RenderTechnique870910->type04;
                            if (type == 20) {
                                unsigned int sky = CarRenderSkyIndex();
                                unsigned int previous = ug2RenderSkyHashes8A07A8[sky] == texture->hash24 ? sky-2 : sky-1;
                                CarRenderTexture *other = UG2SolidFindTexture(ug2RenderSkyHashes8A07A8[previous&7],0,0);
                                UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
                                    ug2RenderTechnique870910->parameters1C->slots00[54].handle24,other->state00->texture18);
                                float fraction = static_cast<float>(CarRenderSkyFraction());
                                UG2EffectSetFloat(ug2RenderTechnique870910->effect20,
                                    ug2RenderTechnique870910->parameters1C->slots00[52].handle24,fraction);
                                unsigned int alpha = static_cast<unsigned int>(static_cast<int>(static_cast<long double>(fraction)*255.0f));
                                UG2EffectSetInt(ug2RenderTechnique870910->effect20,
                                    ug2RenderTechnique870910->parameters1C->slots00[53].handle24,alpha<<24);
                            } else if ((!ug2SolidDebug8707CD || *ug2SolidViewMode870848 != 0) &&
                                       ug2RenderTechnique870910->value18 && (type == 0 || type == 1 || type == 5 || type == 4)) {
                                count = 1;
                            }
                            UG2EffectCommit(command->technique18->effect20);
                            for (int pass=0;pass<count;++pass) {
                                UG2EffectBeginPass(command->technique18->effect20,pass);
                                int start = static_cast<int>(primitive->value34-solid->indexAddress18)>>1;
                                UG2RenderDrawIndexed(ug2SolidDevice870974,4,0,0,solid->vertexCount34,start,primitive->indexCount0C/3);
                                UG2EffectEndPass(command->technique18->effect20);
                            }
                            solid->value40 = 1;
                            ++ug2RenderIndexed872EE0;
                            ug2RenderTriangles872EF0 += solid->vertexCount34/3;
                        }
                    } else {
                        UG2RenderSetState(ug2SolidDevice870974,14,0);
                        ug2RenderVertex87092C = 0;
                        int count = command->technique18->passCount14;
                        UG2EffectCommit(command->technique18->effect20);
                        for (int pass=0;pass<count;++pass) {
                            UG2EffectBeginPass(command->technique18->effect20,pass);
                            unsigned int *vertices = reinterpret_cast<unsigned int *>(primitive);
                            UG2RenderDrawImmediate(ug2SolidDevice870974,5,vertices[0]-2,vertices+1,0x18);
                            UG2EffectEndPass(command->technique18->effect20);
                        }
                        UG2RenderSetState(ug2SolidDevice870974,14,1);
                        ++ug2RenderImmediate872EE4;
                        ug2RenderImmediatePool870954->Free(primitive);
                    }
                    bFree(ug2SolidCommandPool87091C,command);
                }
                group->texture10->state00->group1C = 0;
                group->ClearCommands();
                bFree(ug2SolidGroupPool870918,group);
            }
            UG2RenderTechniqueEnd(technique);
            UG2EffectEnd(technique->effect20);
            ug2RenderTechnique870910 = 0;
        }
    }
    ug2SolidGroupCount87094C = 0;
}
