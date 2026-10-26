#include "../../Speed/Indep/Src/World/CarRenderConsume.h"
extern "C" void __cdecl abort(void);

// Host callback bridge for the original class tokens. Never use a host vtable
// in the four-byte original record; unknown/pure class identities cannot dispatch.
extern "C" void UG2RenderTechniqueBegin(CarRenderTechnique *technique) {
    switch (technique->value00) {
    case 0x7a453c: technique->BeginHeadlight(); return;
    case 0x7a4548: technique->BeginReflection(); return;
    case 0x7a4554: technique->BeginHeadlightReflection(); return;
    case 0x7a4560: technique->BeginCar(); return;
    case 0x7a456c: technique->BeginFilter(); return;
    case 0x7a4578: technique->End(); return;
    default: abort();
    }
}
extern "C" void UG2RenderTechniqueEnd(CarRenderTechnique *technique) {
    switch (technique->value00) {
    case 0x7a453c: case 0x7a4548: case 0x7a4554: case 0x7a4560: case 0x7a456c: case 0x7a4578:
        technique->End(); return;
    default: abort();
    }
}
