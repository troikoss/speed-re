#ifndef UG2_D3DX_EFFECT_SETUP_H
#define UG2_D3DX_EFFECT_SETUP_H
#include "EffectDestruction.h"

// Parameter +34 points into a circular use ring. The first field read by setup
// is the group index at +14; neighbors are at +18/+1c. Other fields stay unknown.
struct D3DXEffectUseLink {
    unsigned int value00,value04,value08,value0C,value10;
    unsigned short value14,value16;
    D3DXEffectUseLink *previous18,*next1C;
};
typedef char D3DXEffectUseLinkSize[(sizeof(D3DXEffectUseLink)==0x20)?1:-1];
extern "C" {
// Native ABI bridges to recovered preparation/evaluation callers. The shader
// VM, floor and table operation are recovered; math/exception boundaries remain.
int UG2FxPrepareGroupA(D3DXEffectObject *,unsigned int); // 0x666df7
int UG2FxPrepareGroupB(D3DXEffectObject *,unsigned int); // 0x667d51
int UG2FxEvaluateRecord(D3DXEffectObject *,unsigned int,unsigned int); // 0x663a32
extern const double ug2FxIndexBias7A1E58;
}
#endif
