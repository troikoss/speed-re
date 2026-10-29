#include "CarWheelSecondary.h"
#include "CarRenderConsume.h"

// Original 0x5bcfc0: unsigned DWORD counter, including wraparound.
void CarRenderFinishQuad() { ++ug2RenderQuadCalls872F08; }

// Original complete 0x5c5580..0x5c574d. Vertex words 6/7 are indeterminate
// in the original stack buffer. Preserve all defined fields as integer payloads;
// even signaling-NaN coordinates/UVs are not loaded into the FPU here.
void CarRenderSubmitQuadRecovered(CarRenderQuad *quad,CarRenderTexture *texture,WheelMatrix *world,unsigned int) {
    unsigned int vertices[4][8];
    const unsigned int *positions=reinterpret_cast<const unsigned int *>(quad->position00);
    const unsigned int *uv=reinterpret_cast<const unsigned int *>(quad->uv40);
    for (int j=0;j<4;++j) {
        vertices[j][0]=positions[j*4];vertices[j][1]=positions[j*4+1];vertices[j][2]=positions[j*4+2];
        unsigned int colour=quad->colour60[j];
        vertices[j][3]=(colour&0xff000000)|((colour&0xff)<<16)|(colour&0xff00)|((colour>>16)&0xff);
        vertices[j][4]=uv[j*2];vertices[j][5]=uv[j*2+1];
    }
    // Pack before these calls: callbacks may mutate the original quad/context.
    CarRenderBindTexture(texture);
    CarRenderUploadWorld(world,ug2SolidViewMode870848[1]);
    unsigned int extra=ug2RenderQuadTexture8708B4;
    if (extra) {
        CarRenderTechnique *technique=ug2RenderTechnique870910;
        UG2EffectSetTexture(technique->effect20,technique->parameters1C->slots00[10].handle24,extra);
    }
    if (ug2RenderQuadEnabled7FF05C) {
        UG2EffectCommit(ug2RenderTechnique870910->effect20);
        CarRenderTechnique *afterCommit=ug2RenderTechnique870910;
        ug2RenderVertex87092C=0;
        UG2EffectBeginPass(afterCommit->effect20,0);
        UG2RenderDrawImmediate(ug2SolidDevice870974,6,2,vertices,0x20);
        UG2EffectEndPass(ug2RenderTechnique870910->effect20);
    }
    CarRenderFinishQuad();
}
