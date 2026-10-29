#include "../../Speed/Indep/Src/World/CarWheelSecondary.h"
#include <stdio.h>
#include <stdlib.h>
// Original quad producer and view wrapper execute recovered code. Packing and
// rendering 0x5c5580 still require the effect/device backend. Fail visibly if
// a future mode selects this path before that integration exists.
void CarRenderSubmitQuad(CarRenderQuad *,CarRenderTexture *,WheelMatrix *,unsigned int) {
    fprintf(stderr,"unimplemented native quad packing/render submission (UG2 0x5c5580)\n");abort();
}
