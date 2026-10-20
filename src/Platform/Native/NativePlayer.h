#ifndef UG2_NATIVE_PLAYER_H
#define UG2_NATIVE_PLAYER_H

// Platform scaffolding: the demo's single human player record (published at 0x8900ac/0x8900b4)
// and the camera-mover stand-in at its +0x238. Recovered player code reads the PlayerRecord
// prefix and the HUD region field at +0x9b8 of the same object.
#include "../../Speed/Indep/Src/Hud/HudPlayerMasks.h"
#include "../../Speed/Indep/Src/Hud/HudFreeroam.h"

struct NativeVehicle;
struct Camera;

// Native ownership bridge at +0x238 reports the attached chase mover's kind
// (1 from 0x451660 after 0x444040), and +0x1c points at the actual recovered
// Camera used for rendering and audio. Slot +0x28 is the original kind-1 query
// 0x404800 (false); native type-change handling reads the same profile option.
class NativeChaseCameraMover : public CameraMover {
public:
    NativeChaseCameraMover();
    void Virtual00();
    void Virtual04();
    void Virtual08();
    void Virtual0C();
    void Virtual10();
    void Virtual14();
    void Virtual18();
    void Virtual1C();
    void SetType20(int type);
    void Virtual24();
    bool Query28();
    char unidentified18[4];
    Camera *camera1C; // actual finalized camera, shared with recovered audio
};
UG2_CHECK_OFFSET(NativeChaseCameraMover, camera1C, 0x1c);

union NativePlayerStorage {
    PlayerRecord record;
    HudRegionPlayer region;
};

extern NativePlayerStorage nativePlayer;
extern NativeChaseCameraMover nativeChaseCamera;
// Native session seam: one local player in both original player tables, driving this vehicle.
void BindNativePlayer(NativeVehicle *vehicle);

#endif
