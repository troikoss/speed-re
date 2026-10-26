#ifndef UG2_CAR_RENDER_TECHNIQUE_H
#define UG2_CAR_RENDER_TECHNIQUE_H
#include "CarRenderSolid.h"
#include "CarRenderTextureUpload.h"

struct CarRenderTechniqueParameter {
    char name00[32];
    unsigned int hash20;
    unsigned int handle24;
};
UG2_CHECK_OFFSET(CarRenderTechniqueParameter, hash20, 0x20);
UG2_CHECK_OFFSET(CarRenderTechniqueParameter, handle24, 0x24);
UG2_CHECK_SIZE(CarRenderTechniqueParameter, 0x28);
struct CarRenderTechniqueMapNode {
    CarRenderTechniqueMapNode *next00;
    CarRenderTechniqueMapNode *previous04;
    unsigned int key08;
    unsigned int value0C;
};
UG2_CHECK_SIZE(CarRenderTechniqueMapNode, 0x10);
struct CarRenderTechniqueMapResult {
    CarRenderTechniqueMapNode *node00;
    unsigned char inserted04;
    unsigned char pad05[3];
};
UG2_CHECK_SIZE(CarRenderTechniqueMapResult, 8);
struct CarRenderTechniqueMapPair {
    unsigned int hash00;
    unsigned int value04;
};
UG2_CHECK_SIZE(CarRenderTechniqueMapPair, 8);
// Original library container records; their algorithms remain separate boundaries.
struct CarRenderTechniqueMap {
    unsigned char value00[8];
    CarRenderTechniqueMapNode *head08;
    unsigned int count0C;
    unsigned int value10;
    CarRenderTechniqueMapNode **begin14, **end18, **capacity1C;
    unsigned int mask20, buckets24;
    CarRenderTechniqueMap *Construct(const unsigned char *tagA,const unsigned char *tagB); // 0x5d0bb0, RET 8
    CarRenderTechniqueMapResult *Insert(CarRenderTechniqueMapResult *result,const CarRenderTechniqueMapPair *pair); // 0x5d0cf0, RET 8
};
UG2_CHECK_SIZE(CarRenderTechniqueMap, 0x28);
struct CarRenderTechniqueParameters {
    CarRenderTechniqueParameter slots00[60];
    CarRenderTechniqueMap map960;
    void Reset(); // 0x5d0f20, library-container reset boundary
    void Bind(unsigned int hash,unsigned int handle); // 0x5d1b70, RET 8
};
UG2_CHECK_OFFSET(CarRenderTechniqueParameters, map960, 0x960);
UG2_CHECK_SIZE(CarRenderTechniqueParameters, 0x988);
struct CarRenderTechniqueRecipe {
    unsigned int elements00[16][2];
    unsigned int value80;
    const char *resource84;
    unsigned int value88;
};
UG2_CHECK_OFFSET(CarRenderTechniqueRecipe, resource84, 0x84);
UG2_CHECK_SIZE(CarRenderTechniqueRecipe, 0x8c);
struct CarRenderVertexElement {
    unsigned short stream00,offset02;
    unsigned char type04,method05,usage06,index07;
};
UG2_CHECK_SIZE(CarRenderVertexElement, 8);
struct CarRenderEffectDescription {
    const char *creator00;
    unsigned int parameters04,techniques08,functions0C;
};
UG2_CHECK_SIZE(CarRenderEffectDescription, 0x10);
struct CarRenderEffectParameterDescription {
    const char *name00,*semantic04;
    unsigned int value08[9];
};
UG2_CHECK_SIZE(CarRenderEffectParameterDescription, 0x2c);

extern "C" {
extern unsigned char ug2TechniqueCaps86E5C8[],ug2TechniqueAdapter86C120[];
extern unsigned int ug2TechniqueSystem870970,ug2TechniqueAdapter8709AC,ug2TechniquePool870914;
extern int ug2TechniqueFilter870D18,ug2TechniqueMaximum876530,ug2TechniqueSkip870908;
extern unsigned char ug2TechniqueReady87090C;
extern const CarRenderTechniqueRecipe ug2TechniqueRecipes7FF7D0[23];
extern const unsigned int ug2TechniqueTypes800494[8],ug2TechniqueUsages8004B4[9],ug2TechniqueSizes8004E4[8];
extern const unsigned char ug2TechniqueIndices8004D8[12];
extern int ug2TechniqueAttempts800BF4,ug2TechniqueBlend86EAD8[5];
extern unsigned int ug2TechniqueRamp86BD18;
extern const char ug2TechniqueShader7A4584[];
// Typed platform/runtime boundaries; tokens never contain host COM objects.
int UG2TechniqueGetCaps(unsigned int device,void *output);
int UG2TechniqueGetAdapter(unsigned int system,unsigned int adapter,unsigned int flags,void *output);
unsigned int UG2TechniqueRelease(unsigned int token);
int UG2TechniqueCreatePool(unsigned int *output);
int UG2TechniqueCreateDeclaration(unsigned int device,const CarRenderVertexElement *elements,unsigned int *output);
int UG2TechniqueCreateEffect(unsigned int device,unsigned int module,const char *resource,void *defines,
    void *includes,unsigned int flags,unsigned int pool,unsigned int *output,unsigned int *errors);
int UG2EffectGetDescription(unsigned int effect,CarRenderEffectDescription *output);
unsigned int UG2EffectGetParameter(unsigned int effect,unsigned int parent,unsigned int index);
int UG2EffectGetParameterDescription(unsigned int effect,unsigned int parameter,CarRenderEffectParameterDescription *output);
unsigned int UG2EffectGetTechnique(unsigned int effect,unsigned int index);
unsigned int UG2EffectGetAnnotation(unsigned int effect,unsigned int technique,const char *name);
int UG2EffectGetInt(unsigned int effect,unsigned int parameter,int *output);
int UG2EffectSetInt(unsigned int effect,unsigned int parameter,int value);
int UG2EffectSetIntArray(unsigned int effect,unsigned int parameter,const int *values,unsigned int count);
int UG2EffectValidate(unsigned int effect,unsigned int technique);
int UG2EffectSetTechnique(unsigned int effect,unsigned int technique);
unsigned int UG2EffectGetPass(unsigned int effect,unsigned int technique,unsigned int index);
int UG2EffectSetFloat(unsigned int effect,unsigned int parameter,float value);
}
unsigned int CarRenderTechniqueMagFilter(); // 0x5be560
void CarRenderCreateTechniqueRamp(); // 0x5bc5d0
void CarRenderInitialiseTechniques(); // 0x5d21f0
#endif
