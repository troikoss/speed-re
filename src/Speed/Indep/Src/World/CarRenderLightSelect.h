#ifndef UG2_CAR_RENDER_LIGHT_SELECT_H
#define UG2_CAR_RENDER_LIGHT_SELECT_H

#include "CarRenderLightBuild.h"

struct CarRenderLightTreeNode {
    float center00[3];
    unsigned short value0C;
    short count0E;
    float extent10[3];
    short indices1C[10];
    // 0x5746e0 (__thiscall, RET 0xc): strict bounds, no result float spill.
    unsigned int Contains(float x, float y, float z);
};
UG2_CHECK_OFFSET(CarRenderLightTreeNode, count0E, 0xe);
UG2_CHECK_OFFSET(CarRenderLightTreeNode, extent10, 0x10);
UG2_CHECK_OFFSET(CarRenderLightTreeNode, indices1C, 0x1c);
UG2_CHECK_SIZE(CarRenderLightTreeNode, 0x30);

struct CarRenderLightTree {
    CarRenderLightTreeNode *nodes00;
    // Original 0x574730 (__thiscall, RET 0x10): ordered recursive child search.
    CarRenderLightTreeNode *Descend(CarRenderLightTreeNode *node, float x, float y, float z);
    // Original 0x5747e0 (__thiscall, RET 0xc): root check and descent.
    CarRenderLightTreeNode *Find(float x, float y, float z);
    void LoadNoop(); // 0x4022c0, shared empty routine used on initial chunk fixup
};

struct CarRenderLightLink {
    CarRenderLightLink *next00;
    CarRenderLightLink *previous04;
};
struct CarRenderLightSector {
    CarRenderLightSector *next00;
    CarRenderLightSector *previous04;
    unsigned short version08;
    unsigned char initialized0A;
    unsigned char value0B;
    unsigned int section0C;
    CarRenderLightTree *tree10;
    unsigned int value14;
    CarRenderLight *lights18;
    unsigned char pad1C[12];
    unsigned int flag28;
    CarRenderLightLink list2C;
};
UG2_CHECK_OFFSET(CarRenderLightSector, tree10, 0x10);
UG2_CHECK_OFFSET(CarRenderLightSector, lights18, 0x18);
UG2_CHECK_OFFSET(CarRenderLightSector, flag28, 0x28);
UG2_CHECK_OFFSET(CarRenderLightSector, list2C, 0x2c);
struct CarRenderLightList {
    CarRenderLightSector *next00;
    CarRenderLightSector *previous04;
    // Original 0x502fd0 (__thiscall, RET 0): returns the receiver unchanged.
    CarRenderLightSector *End();
    void Initialise();                     // 0x57bcd0, RET 0
    CarRenderLightList *Construct();        // 0x598750, RET 0
};
UG2_CHECK_SIZE(CarRenderLightList, 8);
extern "C" {
extern CarRenderLightList ug2LightSectors830D14;
extern CarRenderLightList ug2AdditionalLights835850;
extern CarRenderLightList ug2LightAnimations830AE8;
extern CarRenderLightList ug2LightProfiles8358C0;
}

// Internal stack layout shared by 0x48e3d0 and 0x4893a0. Unknown cell +0xc4
// stays unspecified; the original initializes only the fields used below.
struct CarRenderLightSelectState {
    float fallback00[4];
    float ambient10[40];
    float *nearestB0;
    float *currentB4;
    float *endB8;
    float weightBC;
    float distanceC0;
    unsigned int valueC4;
    int countC8;
};
UG2_CHECK_OFFSET(CarRenderLightSelectState, nearestB0, 0xb0);
UG2_CHECK_OFFSET(CarRenderLightSelectState, currentB4, 0xb4);
UG2_CHECK_OFFSET(CarRenderLightSelectState, endB8, 0xb8);
UG2_CHECK_OFFSET(CarRenderLightSelectState, weightBC, 0xbc);
UG2_CHECK_OFFSET(CarRenderLightSelectState, distanceC0, 0xc0);
UG2_CHECK_OFFSET(CarRenderLightSelectState, countC8, 0xc8);
UG2_CHECK_SIZE(CarRenderLightSelectState, 0xcc);

// Original 0x4893a0 (cdecl): admission, attenuation and ambient/dynamic handling.
void CarRenderAdmitLight(CarRenderLightSelection *output, CarRenderLightSelectState *state,
                        CarRenderLight *light, float x, float y, float z);

#endif
