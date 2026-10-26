#include "CarRenderUploads.h"
#include "../../Libs/bWare/Strings.h"
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output,WheelMatrix *left,WheelMatrix *right);

namespace {
void BindHeadlightMaps() {
    CarRenderTexture *texture = UG2SolidFindTexture(cshash(ug2RenderHeadlights7A3658),1,0);
    UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[35].handle24,texture->state00->texture18);
    UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[33].handle24,ug2TechniqueRamp86BD18);
}
void BindReflectionTexture() {
    UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[45].handle24,ug2RenderTexture870894);
}
void UploadReflection() {
    WheelMatrix matrix;
    WheelIdentity(&matrix);
    matrix.elements[0] = 0.5f;
    matrix.elements[5] = -0.5f;
    matrix.elements[12] = matrix.elements[13] = 0.5f;
    CarRenderViewRecord *view = CarRenderViewByIndex(ug2SolidViewMode870848[1]);
    if (view) {
        D3DXMatrixMultiply(&matrix,view->matrices00+1,&matrix);
        matrix.elements[2] = matrix.elements[3];
        matrix.elements[6] = matrix.elements[7];
        matrix.elements[10] = matrix.elements[11];
        matrix.elements[14] = matrix.elements[15];
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[44].handle24,&matrix);
    }
}
void SetColourState60() {
    unsigned int device = ug2SolidDevice870974;
    unsigned int red = static_cast<unsigned int>(static_cast<int>(static_cast<long double>(ug2RenderColour800520[0])*255.0f));
    unsigned int green = static_cast<unsigned int>(static_cast<int>(static_cast<long double>(ug2RenderColour800520[1])*255.0f));
    unsigned int blue = static_cast<unsigned int>(static_cast<int>(static_cast<long double>(ug2RenderColour800520[2])*255.0f));
    UG2RenderSetState(device,60,((red<<8)|green)<<8|blue);
}
}

// Original 0x5bc6f0 / 0x5bc7b0. The receiver's shader byte gates these paths;
// parameters/effect always come from the separately stored current technique.
void CarRenderTechnique::BeginHeadlight() {
    BindHeadlightMaps();
    if (!value18) SetColourState60();
}
void CarRenderTechnique::BeginReflection() {
    BindReflectionTexture();
    if (!value18) UploadReflection();
}
// Original 0x5bc890. The shader path deliberately repeats all three map bindings.
void CarRenderTechnique::BeginHeadlightReflection() {
    BindHeadlightMaps();
    BindReflectionTexture();
    if (!value18) {
        UploadReflection();
        SetColourState60();
    } else {
        BindHeadlightMaps();
        BindReflectionTexture();
        UG2EffectSetFloat(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[46].handle24,ug2RenderReflectionMinimum7A27E4);
    }
}
// Original 0x5bcad0.
void CarRenderTechnique::BeginCar() {
    UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[18].handle24,
        ug2RenderMode8654A4 == 6 ? ug2RenderTexture870858 : ug2RenderTexture87088C);
}
// Original 0x5bcb10.
void CarRenderTechnique::BeginFilter() {
    for (unsigned int stage=0;stage<4;++stage)
        UG2RenderSetTextureStageState(ug2SolidDevice870974,stage,24,0);
}
// Original 0x4022c0, shared by all concrete End methods and generic Begin.
void CarRenderTechnique::End() {}

// Original 0x5d0880.
void CarRenderTechniqueParameters::Destroy() { map960.Destroy(); }
// Original 0x5d1b30: retain the dangling parameter pointer and declaration token.
void CarRenderTechnique::Destroy() {
    CarRenderTechniqueParameters *parameters = parameters1C;
    value00 = 0x7a41b0;
    if (parameters) { parameters->map960.Destroy(); UG2TextureFree(parameters); }
    if (effect20) { UG2TechniqueRelease(effect20); effect20 = 0; }
}
// Original 0x5d1b10, low-byte flags, RET 4.
CarRenderTechnique *CarRenderTechnique::Delete(unsigned char flags) {
    Destroy();
    if (flags&1) UG2TextureFree(this);
    return this;
}
// Original 0x5d0ca0. Its parameter cleanup uses the original wrapper boundary.
CarRenderTechnique *CarRenderTechnique::DeleteBase(unsigned char flags) {
    CarRenderTechniqueParameters *parameters = parameters1C;
    value00 = 0x7a41b0;
    if (parameters) { parameters->Destroy(); UG2TextureFree(parameters); }
    if (effect20) { UG2TechniqueRelease(effect20); effect20 = 0; }
    if (flags&1) UG2TextureFree(this);
    return this;
}
