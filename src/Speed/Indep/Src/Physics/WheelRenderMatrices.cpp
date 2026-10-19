#include "VehicleMover.h"
#include "WheelMath.h"

extern float wheelRenderAngleScale; // 0x78423c

void RotateMatrixY(WheelMatrix *output, WheelMatrix *input, unsigned int angle); // 0x5bada0
void RotateMatrixZ(WheelMatrix *output, WheelMatrix *input, unsigned int angle); // 0x5bae60

// Original 0x5f6820 (__thiscall, RET 0xc). Car-local tyre and brake frames for the draw routine
// 0x620ae0 (callers 0x631250 and 0x6316d0). When bit `view` of value5B0 is set both outputs are
// rest-pose frames (identity with the record's wheel position); otherwise they are copies of the
// frames kept by 0x5921b0 in the state at car +0x130 and +0x230.
void MoverModeOwner::GetWheelRenderMatrices(WheelMatrix *tires, WheelMatrix *brakes, int view) {
    if ((1 << view) & value5B0) {
        for (int wheel = 0; wheel < 4; ++wheel) {
            float *tire = tires[wheel].elements;
            float *brake = brakes[wheel].elements;
            for (int i = 0; i < 16; ++i) {
                tire[i] = 0.0f;
                brake[i] = 0.0f;
            }
            tire[0] = 1.0f;
            tire[5] = 1.0f;
            tire[10] = 1.0f;
            brake[0] = 1.0f;
            brake[5] = 1.0f;
            brake[10] = 1.0f;
            WheelVector *position = &record20->wheels10[wheel].position0;
            tire[12] = position->x;
            tire[13] = position->y;
            tire[14] = position->z;
            tire[15] = 1.0f;
            brake[12] = position->x;
            brake[13] = position->y;
            brake[14] = position->z;
            brake[15] = 1.0f;
        }
        return;
    }
    DrivetrainAuxiliary *aux = State();
    for (int wheel = 0; wheel < 4; ++wheel) {
        BodyMatrixCopy(&tires[wheel], &aux->wheelMatricesF0[wheel]);
        BodyMatrixCopy(&brakes[wheel], &aux->wheelMatrices1F0[wheel]);
    }
}

// Original 0x5ec7f0 (__thiscall, RET 0x8). Third per-wheel frame set passed to 0x620ae0; its purpose
// there is unproven. Rest-pose branch as above. Otherwise each frame is a rotation about Y by
// (int)(value4E8[wheel] * 0x78423c), then for the front wheels (record offset below 0x60) a rotation
// about Z by steering3CE, with the wheel position and suspension340[wheel] added to z.
void MoverModeOwner::GetWheelSpinMatrices(WheelMatrix *output, int view) {
    if ((1 << view) & value5B0) {
        for (int wheel = 0; wheel < 4; ++wheel) {
            float *m = output[wheel].elements;
            for (int i = 0; i < 16; ++i) {
                m[i] = 0.0f;
            }
            m[0] = 1.0f;
            m[5] = 1.0f;
            m[10] = 1.0f;
            WheelVector *position = &record20->wheels10[wheel].position0;
            m[12] = position->x;
            m[13] = position->y;
            m[14] = position->z;
            m[15] = 1.0f;
        }
        return;
    }
    for (int wheel = 0; wheel < 4; ++wheel) {
        WheelMatrix *m = &output[wheel];
        for (int i = 0; i < 16; ++i) {
            m->elements[i] = 0.0f;
        }
        m->elements[0] = 1.0f;
        m->elements[5] = 1.0f;
        m->elements[10] = 1.0f;
        m->elements[15] = 1.0f;
        RotateMatrixY(m, m, (int)(value4E8[wheel] * wheelRenderAngleScale));
        if (wheel * 0x30 < 0x60) {
            RotateMatrixZ(m, m, State()->steering3CE);
        }
        WheelVector *position = &record20->wheels10[wheel].position0;
        m->elements[12] = position->x;
        m->elements[13] = position->y;
        m->elements[15] = 1.0f;
        float z = position->z;
        m->elements[14] = z + State()->suspension340[wheel];
    }
}
