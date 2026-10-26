#include "CarRenderLod.h"

extern "C" volatile float ug2WheelOneRuntime; // 0x784250

// Original 0x60d700 (PC v1.2). The returned x87 value is consumed directly by
// the wheel gate. long double retains the x87 return on native i386 (MSVC 7.1
// treats it as double, retaining its register precision). Keep the explicit single-precision spills at 0x60d7bc/0x60d7c2.
extern "C" long double UG2WheelFn60d700(WheelMatrix *world, void *viewObject) {
    if (!ug2WheelFacingEnabled802914) {
        return ug2WheelOneRuntime;
    }
    const CarRenderView *view = static_cast<const CarRenderView *>(viewObject);
    const float axisX = view->frame40->axis50[0];
    const float axisY = view->frame40->axis50[1];
    const float axisZ = view->frame40->axis50[2];
    WheelMatrix copy;
    BodyMatrixCopy(&copy, world);
    const float normalX = ug2WheelFacingX8A1C54;
    const float normalY = ug2WheelFacingY802910;
    const float normalZ = ug2WheelFacingZ8A1C58;
    const float *m = copy.elements;
    volatile float x = (static_cast<long double>(m[8]) * normalZ +
                        static_cast<long double>(m[4]) * normalY) + static_cast<long double>(m[0]) * normalX;
    const long double y = (static_cast<long double>(m[9]) * normalZ +
                      static_cast<long double>(m[5]) * normalY) + static_cast<long double>(m[1]) * normalX;
    volatile float z = (static_cast<long double>(m[10]) * normalZ +
                        static_cast<long double>(m[6]) * normalY) + static_cast<long double>(m[2]) * normalX;
    return (static_cast<long double>(z) * axisZ + y * axisY) + static_cast<long double>(x) * axisX;
}
