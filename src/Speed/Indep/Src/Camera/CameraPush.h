#ifndef UG2_CAMERA_PUSH_H
#define UG2_CAMERA_PUSH_H

// Recovered UG2 PC v1.2 camera mover collision pushes (camera mover base 0x44ab90).

#include "../Physics/WheelMath.h"

struct MoverModeOwner;
struct WCollider;

// The player object at camera mover +0x20 (layout not recovered).
struct CameraPlayer {
    bool Query5EB520();  // 0x5eb520: byte +0x2b is clear
};

// Camera mover prefix read by the pushes; the drive camera mover (DriveCamera.h) starts with it.
struct CameraMoverBase {
    void **vtable0;
    char unidentified4[0xc];
    int view10;                  // bit tested in a car's +0x5b4 view mask
    char unidentified14[0xc];
    CameraPlayer *player20;
    char unidentified24[0x2d];
    unsigned char lookBack51;

    float NearRadius();                                                           // 0x445a60, drive camera slot 0x30
    bool PushFromFaces(WCollider *collider, float *from, float *point, float *offset); // 0x444190
    bool PushFromWorld(WheelMatrix *view, MoverModeOwner *car, float *focus);     // 0x444530
    float *FindNearbyCar(float *eye, bool racersOnly);                            // 0x44ac10
    void PushFromCars(WheelMatrix *view, MoverModeOwner *car, bool racersOnly);   // 0x44b040
};

// The world's car list at *0x890080 (count +0x10, entries +0x1c).
struct CameraCarList {
    char unidentified0[0x10];
    int count10;
    char unidentified14[8];
    MoverModeOwner *cars1C[1];
};

void WheelFrameAt(WheelMatrix *output, WheelMatrix *rotation, WheelVector *position);    // 0x401340

#endif
