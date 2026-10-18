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

## Other-car push (2026-10-18)

`Camera/CameraPush.cpp` also recovers:

- `0x44ac10` nearby car (thiscall: eye, racers only; `ret 8`). Sets guard bit 1 of
  `0x82a590`, takes the mover's own car from vtable slot 0x44 and walks the world list
  `*0x890080` (count +0x10, entries +0x1c, both reloaded every iteration). A car is
  skipped when it is the own car, when racers only is set and car `+0x14` -> `+4` is not 3,
  when byte `+0x550` is clear, or when bit `mover +0x10` of `+0x5b4` is clear. It must be
  within 12 (`0x7870c8`, squared, compared unrounded) of the eye. With an own car and the
  `0x571250` race query false, a relative speed above 15 (`0x7870c4`) whose direction meets
  the direction to the car with cosine below 0.8 (`0x7870c0`) also needs the direction from
  the own car's origin (`0x401250`, `0x441890`) to meet it with cosine 0.8 or more. The
  rounded squared distance must be below the rounded squared body sphere (|second box
  corner - centre| + 1.0), and the eye height must lie within the box corners transformed by
  the car frame (`0x401340`, matrix `+0x90`, origin `+0x50`) widened by 0.5 (`0x787088`),
  tested by clamping and comparing for equality. The car's id (short `+0xa`) goes to
  `0x7eb5c0` (written only when it changes); its origin and squared radius go to the
  static `0x82a580` (stored y, x, z, r²), which is returned; 0 when no car qualifies.
- `0x44b040` other-car push (thiscall: view, car, racers only; `ret 0xc`). The eye of the
  inverted view is lifted to sqrt(r² + 0.1 (`0x784358`) - horizontal distance²) above
  each found car until `0x44ac10` finds none; the car origin's view-space x and depth are
  stored as binary32, divided by the rounded depth through an unrounded reciprocal, and
  the view is re-aimed at the origin (`0x444390`). `0x7eb5c0` becomes -1 when no car was
  found.
- `0x401250` vector difference (thiscall, returns the output), `0x401340` frame from the
  rotation rows and a position row ending in 1 (cdecl) and `0x441890` car origin (+0x50).

Verification: `tools/verify_pc_camera_cars.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/camera-cars.json`): 12,288 comparisons (2,048 per
search, push, frame and origin, 4,096 for the difference including aliasing) under 53- and
64-bit precision over up to six cars around the eye with random activity, view masks,
racer types, velocities, yaw/pitch frames and boxes; 533 searches found a car and 526
pushes lifted at least once. The statics `0x82a580`, `0x82a590`, `0x7eb5c0` and the list
pointer are compared after each run. Normalise, transform, velocity `0x4018a0`, inverse
and re-aim run original code; the race query `0x571250` and D3DX multiply are traced stubs
and slot 0x44 (`0x44ba70` in the drive camera) is a fixture getter of mover +0x2c. 22
mutations rejected. Car data are finite, so signalling-NaN copies through `0x401340`
(which copies some elements through the x87) are not exercised. MSVC 7.1 and byte identity
are not established; not yet called by native code.

## Drive camera integration (2026-10-18)

The recovered update tail (`DriveCamera.cpp`, `0x453570`) now builds the view matrix with
`0x5ba9a0` from the zoomed eye, target and up, and for the chase types (setting types 0, 1
and 5) calls `PushFromWorld(matrix, car, 0)` then `PushFromCars(matrix, car, 1)`, as the
original does at `0x453bc8`/`0x453bdd`. Also recovered: drive camera slot 0x44 `0x44ba70`
(mover +0x2c) and the player query `0x5eb520` (player byte +0x2b clear), now executed as
original code in `verify_pc_camera_push.py` (12,288 comparisons, 23 mutations). That
verifier previously compared physical register 0 instead of ST(0) for slot 0x30's return;
it now reads ST(0) through TOP.

Platform seams:

- `DemoMain.cpp` supplies the mover's vtable with native thiscall entries for slot 0x30
  (`0x445a60`) and 0x44 (`0x44ba70`); the original table `0x787f28` holds x86 addresses.
  The mover's view is 1 (player one) and its player pointer stays 0 as the base constructor
  `0x44ab90` leaves it, so the push depth is 0.7.
- `NativeVehicle.cpp` lists the player's car in the world car list (`*0x890080` +0x10/+0x1c);
  the search skips it as the mover's own car, so no other-car lift occurs in the demo.
- Shake `0x572a50`/`0x452ed0` and finalisation `0x447280` are not run; the GL path reads the
  eye (inverse translation), forward (column 2) and up (minus column 1) back from the matrix
  and keeps the unpushed look distance for its target.

Native observation (plausibility, not equivalence): the camera's collider query finds the
world pack and 3 to 6 faces around the eye in the scripted Skyline drive. No push occurs there
(the eye never comes within 0.7 of a face). With `UG2_STEER=1 --camera 3` the far camera pushes
off a wall for frames 375 to 403 (up to 0.68 m sideways); before integration that frame showed
the camera inside the wall. `UG2_DEBUG_CAMERA=1` prints the pushed offsets.
