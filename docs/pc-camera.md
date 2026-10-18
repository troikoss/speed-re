# Camera recovery

Reference: UG2 PC v1.2 `speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
The drive camera mover (`0x451660`/`0x453570`) is described in the
[native demo guide](NATIVE_DEMO.md#drive-camera-and-hud-fifth-pass). This page records
the recovery of the view-matrix and world-collision steps that follow it.

## Where the pushes run (2026-10-18)

Call order and arguments are from the disassembly; the behaviour summaries of the
pushes are read from decompiler output and still need confirming during their recovery.
The tail of drive camera update `0x453570` builds the view matrix with `0x5ba9a0`
from the zoomed eye and look-at points and the up follower, then runs shake `0x572a50`
and `0x452ed0` (table `0x7ebe90`/`0x7ebea8` by type, scaled by `0x7870b8` outside
drift/`0x890108`). For chase settings (types 0, 1 and 5) it then calls, both as
`thiscall` on the mover with `(view matrix, car, flag)`:

- `0x444530(view, car, 0)`: world push. It inverts the view (`0x5baf90`), takes the
  car's collider (car `+0x5c4`, skipped when null) and its cached face list, queries
  the collider around the camera (`0x5e6fa0`), pushes the eye out of nearby faces
  (`0x444190`, which also runs world ray `0x5d39f0`), keeps the eye above the car's
  bounding sphere (`0x5ec250`/`0x5ec240`, margin `0x78708c`) and re-aims (`0x444390`).
- `0x44b040(view, car, 1)`: other-car push. `0x44ac10` walks the car list
  `*0x890080` (+0x10 count, +0x1c entries) for nearby cars and lifts the eye above
  each one's sphere (`0x784358` margin); the view is re-aimed with `0x444390` and
  `0x7eb5c0` is cleared when nothing was found.

Finalisation `0x447280` then writes the matrix to the Camera object.

## View-matrix helpers (2026-10-18)

`Camera/CameraMatrix.cpp` recovers:

- `0x5ba9a0` look-at (cdecl): forward = normalize(target - eye), right =
  cross(-up, forward), up' = cross(forward, right), both normalised; the rotation is
  written to the output first, then translation(-eye) times it is formed with every
  zero entry still multiplied by the `.rdata` zero `0x7d7d3c` and copied over the
  output by `0x4012b0`. The eye is negated with `FCHS` (NaN signs flip).
- `0x5baf90` rigid inverse (cdecl, returns the output): translation(-origin) times
  the transposed rotation through the statically linked D3DX multiply `0x63cedb`.
- `0x444390` re-aim (stdcall, four arguments): keeps the view's up column, looks at
  the target, moves the target in view space by depth times `offset` (x by
  `offset[0]`, y by `offset[1]`) and rebuilds the view. View-space x and depth are
  stored as binary32; y stays on the x87 stack.

Verification: `tools/verify_pc_camera_matrix.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/camera-matrix.json`): 12,288 original/rebuilt
comparisons (4,096 per routine) under 53- and 64-bit x87 precision, including
zero forward vectors, up parallel to forward, axis-aligned vectors (signed zeros),
NaN/infinity and aliased outputs; 12 mutations rejected (cross order, second
normalise, folded zero and one terms, translation sum order, eye read order,
inverse operand order and transpose, re-aim rounding of x and y, offset axes, up
column). Normalise `0x43cf10`, cross `0x43d1e0` and copy `0x4012b0` execute original
code in both runs. `0x63cedb` jumps through the runtime-patched pointer `0x803e74`;
both runs reach a traced stub whose inputs must match and which writes the
binary32-rounded product, so the D3DX implementation itself is not executed. MSVC 7.1
compilation and byte identity are not established. Not yet integrated: the native
camera still hands eye/target/up to the renderer.

## Collider area query (2026-10-18)

`World/WColliderQuery.cpp` recovers `WCollider::QueryArea` `0x5e6fa0` (thiscall,
`point, radius, velocity`) and its 2D bounds helpers: reset `0x43d220` (minimum
+FLT_MAX, maximum -FLT_MAX), expand `0x43d2f0` (all inputs read before the four
stores; an unordered comparison takes the point's value), containment `0x43d470`
(each test fails on an unordered result) and scale-add `0x43d090` (y computed first).
The query follows the pack (`0x5e68d0` with radius 0), sets the sphere bounds at
+0x1c/+0x24 and returns when they lie in the face area +0x2c/+0x34. Otherwise it
rebuilds the area around the sphere grown by 0.5 (`0x784260`); with a velocity the
area also covers the point moved by 0.15 s of it, retried while the area diagonal is at
least 28.8 (`0x7a5338`, compared unrounded) with the look-ahead scaled by 0.75
(`0x785510`) and zeroed for the fifth pass. `0x5e5890` then refreshes the faces.

Verification: `tools/verify_pc_collider_query.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/collider-query.json`): 20,480 comparisons (4,096 per
routine; query paths inside 2,540, no velocity 290, swept 1,266, constructed diagonal
straddles 512) under 53- and 64-bit precision with NaN, infinity, FLT_MAX, equal edges
and overlapping outputs; 13 mutations rejected. `UpdatePack` and `RefreshArea` run as
traced stubs (arguments and collider bounds at each call compared). MSVC 7.1 and byte
identity are not established; not yet called by native code.

Next: face push `0x444190` (world ray `0x5d39f0`, face test
`0x5d35b0`), then `0x444530`, `0x44ac10`, `0x44b040` and integration into `0x453570`.
