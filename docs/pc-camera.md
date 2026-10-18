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

## Collision ray (2026-10-18)

`World/WCollisionRay.cpp` recovers the ray the camera's face push casts from the car
to the camera:

- 3D bounds helpers: set `0x43d540`, add point `0x43d570` (each component written in
  place), add sphere `0x43d610` (all six extents computed first) and strict overlap
  `0x43d820` (touching or unordered bounds do not overlap).
- `WCollisionFace::ContainsPoint` `0x5d35b0`: edge cross products of the vertex vectors
  (3 or 4 vertices, byte `+9` signed) against the normal; a negative dot rejects, a NaN
  dot passes; the optional distance is minus the first vertex vector's normal component.
- `WCollisionFace::SegmentHit` `0x5d36f0`: the end distances must have strictly opposite
  signs (unrounded `to` distance); the fraction |from|/(|to rounded|+|from|) interpolates
  the crossing, which must pass `ContainsPoint`.
- `WCollisionFace::RayHit` `0x5d37f0`: face bounds must overlap the ray bounds; one-sided
  rays need the hit direction against the normal; the unrounded hit distance must be
  below `*best`, which receives it rounded.
- `WCollider::Ray` `0x5d39f0` (thiscall: mode, two-sided, from, to, margin, scale): ray
  bounds from `from` and the `margin` sphere at `to`, nearest hit over the area faces
  (start 999999), compared with the binary32 segment length. Mode 1 pulls `to` back to
  `margin` before the hit; mode 0 does the same plus a height lift of
  `(1 - travel/length) * scale` for faces with flag 2, otherwise puts `to` on the face
  plane's height at its x/y offset by `margin` (below with flag 8); other modes only
  report the hit. Store order to x, z, y is kept.

Verification: `tools/verify_pc_collision_ray.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/collision-ray.json`): 32,768 comparisons (4,096 per
routine) under 53- and 64-bit precision over random planar triangles and quads, segments
across them, exact axis-aligned hits, near-endpoint hits, inverted and touching bounds and
aliased outputs; paths include 726 contained points, 693 segment hits, 207 face hits and
1,168 collider-ray hits. 21 mutations rejected. Cross `0x43d1e0`, scale-add `0x43d0c0`
and set-length `0x43cf90` run original code. A nearest hit exactly equal to the segment
length was not constructed. MSVC 7.1 and byte identity are not established; not yet called
by native code.

## Face and world pushes (2026-10-18)

`Camera/CameraPush.cpp` recovers, on the camera mover prefix `CameraMoverBase`:

- `0x445a60`, drive camera vtable slot 0x30: push depth 3.0 (`0x787094`) while looking
  back with a player whose `0x5eb520` query is true, else 0.7 (`0x787090`).
- Face push `0x444190` (thiscall: collider, from, point, offset): without a pack it
  returns 0 and leaves `offset` untouched; otherwise it casts the two-sided mode-1 ray
  from `from` to `point` (margin 0.7 `0x7870a0`, scale 1), then up to three passes add to
  the movement the mean of the normals of faces containing the moved point within the
  push depth (slot 0x30 for faces with flag 2, else 0.7 `0x78709c`), each weighted by
  depth minus the face distance; surfaces whose byte at `0x800eed + 0x98 * surface` is set
  are skipped. `offset` receives the movement with w = 0; the ray's hit is returned.
- World push `0x444530` (thiscall: view, car, focus override): focus = override, else the
  car body centre (`+0x3c` body `+0x110`) through the car matrix (`+0x90`) plus the origin
  (`+0x50`), else the origin. A local collider (constructed with `0x5e0b40`, the car
  collider's barrier groups copied) queries the area around the eye/focus midpoint with
  radius (distance + 1) / 2. The eye is pushed by `0x444190`, then raised onto the car
  body's bounding sphere (|`0x5ec250` second corner - `0x5ec240` centre| + 1.0
  `0x78708c`) above the focus when inside it, and the view is re-aimed at the focus with
  its projected view-space x/y (`0x444390`).
- Car accessors `0x5ec250` (body +0xe0 and +0xf0 corners) and `0x5ec240` (body +0x110).

`0x444390` is called as thiscall with an unused `this`; the stdcall rebuild is
ABI-equivalent for the callee.

Verification: `tools/verify_pc_camera_push.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/camera-push.json`): 10,240 comparisons (2,048 per
routine) under 53- and 64-bit precision; face pushes over up to eight faces around the
probe (754 ray hits, 1,529 moved results, NaN/infinite normals), world pushes with and
without focus override, body and car (609 ray hits). The collider ray, face test,
scale-add, transform, distance, inverse and re-aim run original code; the player query,
collider constructor (installing the fixture face area), area query, destructor and D3DX
multiply are traced stubs. 22 mutations rejected. The verifier compiles with
`-fno-math-errno`, matching the MSVC intrinsic `sqrt`. MSVC 7.1 and byte identity are
not established; not yet called by native code.

Next: other-car scan `0x44ac10` and push `0x44b040`, then integration into `0x453570`.
