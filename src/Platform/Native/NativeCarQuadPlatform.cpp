#include "../../Speed/Indep/Src/World/CarWheelSecondary.h"
// Native ABI bridge. Context is consumed at the original call/read boundaries:
// texture binding may initialize or replace it. Typed effect/device services
// retain explicit unsupported boundaries until renderer integration.
void CarRenderSubmitQuad(CarRenderQuad *quad,CarRenderTexture *texture,WheelMatrix *world,unsigned int fourth) {
    CarRenderSubmitQuadRecovered(quad,texture,world,fourth);
}
