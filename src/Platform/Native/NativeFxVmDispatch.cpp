#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
// Native ABI bridge. Original CRT/operation dependencies retain their own
// unsupported boundaries; the complete VM dispatcher is recovered source.
extern "C" int __stdcall UG2FxExecuteVm(unsigned int *program,unsigned int *literals,unsigned int a,unsigned int b,unsigned int c,
    unsigned int d,unsigned int e,unsigned int f,unsigned int g,unsigned int h,unsigned int i,unsigned int j,unsigned int k,unsigned int l) {
    return D3DXEffectVm(program,literals,a,b,c,d,e,f,g,h,i,j,k,l);
}
