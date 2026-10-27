#ifndef UG2_CAR_RENDER_TECHNIQUE_MAP_H
#define UG2_CAR_RENDER_TECHNIQUE_MAP_H
#include "CarRenderTechnique.h"

// Embedded library subobjects start at map+4 and map+0x10. Their opaque
// allocator bytes are deliberately not initialized by the recovered bodies.
struct CarRenderTechniqueMapList {
    unsigned char value00[4];
    CarRenderTechniqueMapNode *head04;
    unsigned int count08;
    void Erase(CarRenderTechniqueMapNode **output,CarRenderTechniqueMapNode *first,
        CarRenderTechniqueMapNode *last); // 0x5cf140, RET 12
};
UG2_CHECK_SIZE(CarRenderTechniqueMapList, 0xc);
struct CarRenderTechniqueMapVector {
    unsigned char value00[4];
    CarRenderTechniqueMapNode **begin04,**end08,**capacity0C;
    void Initialise(unsigned int count,CarRenderTechniqueMapNode *const *value); // 0x5d0250
    void Assign(unsigned int count,CarRenderTechniqueMapNode *const *value); // 0x5d0c50
    void Insert(CarRenderTechniqueMapNode **position,unsigned int count,
        CarRenderTechniqueMapNode *const *value); // 0x5d0890, RET 12
    CarRenderTechniqueMapNode **Copy(CarRenderTechniqueMapNode **first,
        CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode **output); // 0x5cf190
    CarRenderTechniqueMapNode **Fill(CarRenderTechniqueMapNode **output,
        unsigned int count,CarRenderTechniqueMapNode *const *value); // 0x5cfbc0
};
UG2_CHECK_SIZE(CarRenderTechniqueMapVector, 0x10);
CarRenderTechniqueMapNode *CarRenderTechniqueMapSentinel(); // 0x5ce070
void CarRenderTechniqueMapFill(CarRenderTechniqueMapNode **output,unsigned int count,
    CarRenderTechniqueMapNode *const *value,void *allocator,unsigned int unused); // 0x5ce0f0
CarRenderTechniqueMapNode **CarRenderTechniqueMapCopy(CarRenderTechniqueMapNode **first,
    CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode **output,
    void *allocator,unsigned int unused); // 0x5ca630
void CarRenderTechniqueMapFillRange(CarRenderTechniqueMapNode **first,
    CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode *const *value); // 0x5ca610
void CarRenderTechniqueMapCopyBackward(CarRenderTechniqueMapNode ***result,
    CarRenderTechniqueMapNode **first,CarRenderTechniqueMapNode **last,
    CarRenderTechniqueMapNode **output); // 0x5ce0c0
// Exception-object construction and Windows unwinding remain a CRT boundary.
extern "C" void UG2TechniqueVectorTooLong(); // 0x5d07e0, does not return in UG2
#endif
