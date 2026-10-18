#ifndef UG2_CAMERA_MATRIX_H
#define UG2_CAMERA_MATRIX_H

// Recovered UG2 PC v1.2 view-matrix helpers used by the camera movers.

#include "../Physics/WheelMath.h"

// Original 0x5ba9a0 (cdecl): view matrix looking from `eye` at `target`; `up` is negated
// before the cross products, so the matrix's second column follows -up.
void CameraLookAt(WheelMatrix *output, WheelVector *eye, WheelVector *target, WheelVector *up);

// Original 0x5baf90 (cdecl): inverse of a rigid matrix, translation(-origin) times the
// transposed rotation through the statically linked D3DX multiply 0x63cedb.
WheelMatrix *CameraInvertRigid(WheelMatrix *output, WheelMatrix *input);

// Original 0x444390 (stdcall): rebuild `view` from `eye` towards `target` moved in view
// space by its depth times `offset` (offset[0] on x, offset[1] on y), keeping the view's
// up column.
void __stdcall CameraReaim(WheelMatrix *view, WheelVector *eye, WheelVector *target, float *offset);

#endif
