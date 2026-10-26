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
- Shake `0x572a50`/`0x452ed0` and finalisation `0x447280` were not run at this point; they are
  recovered in [Camera shake and finalisation](#camera-shake-and-finalisation-2026-10-19), and
  the GL path now reads the finalised Camera object.

Native observation (plausibility, not equivalence): the camera's collider query finds the
world pack and 3 to 6 faces around the eye in the scripted Skyline drive. No push occurs there
(the eye never comes within 0.7 of a face). With `UG2_STEER=1 --camera 3` the far camera pushes
off a wall for frames 375 to 403 (up to 0.68 m sideways); before integration that frame showed
the camera inside the wall. `UG2_DEBUG_CAMERA=1` prints the pushed offsets.

## Cockpit blocked tests (2026-10-19)

`Camera/CameraCockpit.cpp` recovers the start of drive camera update `0x453570` (up to
`0x4536f4`) and its callees. Everything below is UG2 PC v1.2 only.

- `0x453570..0x4536f4` (`DriveCameraMover::SelectUpdateType`, a region of Update, not a
  function): clears `active54`; with a player at mover `+0x20`, `car2C = 0x5eac70(player)`.
  For type48 4 (hood) or 2 (bumper): `HumanBlocked` (below) and then `0x445fc0` are both
  evaluated; the view is blocked when the car's input object (`car +0x30`, non-null) reports
  slot `0x54` true (human input `0x410470`: byte `+0x2fb`), or `HumanBlocked`, or `0x445fc0`, or
  the collider (`car +0x5c4`) ray `0x5d39f0` (mode 1, two-sided, margin 0.5, scale 1.0, immediate
  pushes) from the camera point to that point plus `2.0` (`0x78715c`) times the car's `+0xb0`
  row (`0x43d0c0`) hits. The camera point is car `+0x60..+0x68`, replaced when the body (`+0x3c`)
  exists by the body centre (`+0x110`) through the car matrix (`+0x90`, `0x43c9c0`) plus the
  origin (`+0x50`) added x, y, z (the `+0x60` copy is kept only without a body). Blocked: type
  becomes 0 (the far camera) and `snap52 |= (previousType4C != 0)`; clear: `snap52 |=
  (previousType4C == 0)`. Other types leave the type unchanged. The value is stored to
  previousType4C at `0x45370d` and EVERY later read of the type in the original reads
  previousType4C, so the dynamics curve sample, the settings lookup and the hood test
  (`0x4538b3`) use the blocked-adjusted type (the previous recovery used type48 there).
- Inlined twice, at `0x4535a9..0x4535e2` and `0x4537c3..0x45380e` (`HumanBlocked`): vtable
  slot `0x24`, then `0x5ed290` || `0x5ed300` || (car `+0x38` non-null and car `+0x424` <= 2,
  signed). The second use divides the eye duration `key[0]` by `1.3` (`0x787210`, rounded once)
  for all three eye channels; the previous recovery omitted it.
- Slot `0x24` of vtable `0x787f28` is `0x445a90` (not `xor al, al` as an earlier comment said):
  true when the settings entry for previousType4C (else type 0) has setting type 1, 0 or 5.
- `0x445fc0` (thiscall, no arguments): setting for type48 (`0x5ecca0`), `0x445eb0` offsets
  (forward `x4`, lateral `zC`, height `y8`), frame `0x401340` from car `+0x90`/`+0x50`,
  `0x43c9c0`, own car by slot `0x44`, then `0x5f14f0` on the world list with margin 0.5
  (`0x787088`).
- `0x5f14f0` (thiscall `ret 0xc`; `this` unused, the list is `*0x890080`): for each car other
  than the skipped one with byte `+0x550` set and `(dx*dx + dz*dz) + dy*dy` (unrounded) of the
  point from the car origin below `margin + 5.0` (`0x7843e8`, unrounded): box corners
  (`+0x3c` body `+0xe0`/`+0xf0`), frame `0x401340`, inverted in place with `0x5baf90`, point
  transformed `0x43c9c0`, then `0x43d7a0` with the margin; true on the first containing car.
- `0x43d7a0` (cdecl, `Bounds3Contains` in `World/WCollisionRay.cpp`): per axis it rejects when
  `minimum - margin > point` (flags `0x41`) or `margin + maximum < point` (flags 5, `jnp`); an
  unordered comparison passes both, so NaN never rejects.
- `0x5ed290` (`HasWheelInputs`) and `0x5ed300` (`HasWheelTargets`) in `Camera/CameraPush.cpp`:
  any of the input object's `+0x1ec..+0x1f8`, respectively the interleaved car `+0x3f8..+0x404`
  and input `+0x1fc..+0x208`, is above zero (`0x7d7d3c`); NaN is not above. Both loops run once
  in the original. `0x5ed290` moved from `PhysicsMoverController.cpp` unchanged in behaviour.
- `0x5eac70` (`CameraPlayer::GetCar`): car `+4`, or when byte `+0x24a` is set and
  `*(*0x890118 + 0xbd0)` is non-null, that holder's first dword.

Verification: `tools/verify_pc_camera_cockpit.py --compiler clang-cl --mutation-checks`
(manifest `config/SPEED2_EXE_1_2/camera-cockpit.json`): 24,576 comparisons under 53- and 64-bit
precision (2,048 each of `0x43d7a0`, `0x5f14f0`, `0x445fc0`, `0x445a90`, `0x5eac70` and the
prefix, 6,144 each of `0x5ed290` and `0x5ed300`) with NaN, infinity, signed zeros and boundary
values; both outcomes of every test occur (for example 342 containments, 522 car hits, 107
cockpit hits, 63 ray hits and 128 ray misses in the prefix). The prefix is compared as the
original run from `0x453570` to `0x4536f4` (EBX) against the rebuilt function (EAX), with the
mover state, globals, the x87 tag word, the ordered entry events with their arguments (including
the ray's `from`/`to`/margin/scale) and the D3DX trace. Original code runs for `0x5baf90`,
`0x441000`, `0x43d0c0`, `0x445eb0`, `0x410470` and slot `0x44` (`0x44ba70`); the rebuilt
`0x401340`, `0x5d39f0` and `0x5d35b0` run as rebuilt. Stubs: the D3DX multiply `0x63cedb`, and a
38-byte shim for the platform call `FindOrDefault(type, viewState)` that stores `viewState` to
the global `0x8707dc` (compared) and runs original `0x441140` twice as `0x5ecca0` does. 37
mutations rejected. Camera push and cars verifiers still pass (`camera-push` 12,288 comparisons
and 23 mutations, `camera-cars` 12,288 and 22), as does `verify_pc_collision_ray.py` (32,768).

Limits: finite and special generated values only, no signalling-NaN copies through the x87
(`0x401340`, the `+0x60` copy, the body box); the prefix is compared as a region (the original
frame and registers are not); `0x445eb0` ran as original, so the reduced rebuild in
`DriveCamera.cpp` is not compared; MSVC 7.1 does not compile `CameraPush.cpp` (the `__thiscall`
pointer typedefs, already before this change), so only clang-cl was used; byte identity is not
established.

Platform seams: `DemoMain.cpp` supplies slot `0x24` (`0x445a90`) beside `0x30` and `0x44` in the
mover's native vtable; `NativeControls::GetWheelMode54` is the input object's slot `0x54`
(`0x410470`, previously a trap). The player pointer at mover `+0x20` stays 0, so `0x5eac70` is
not called in the demo. `CameraSettingsTable::FindOrDefault(type, viewState)` and the mover's
`settings`/`viewState` stay platform stand-ins for `0x5ecca0` on the car and the global
`0x8707dc`.

## Chase car frame and heading follower (recovered 2026-10-19)

`Camera/CameraChase.cpp` (the discrepancy noted earlier, where the chase cameras used the car
matrix, is fixed):

- `0x445e00` (`GetCarFrame`, thiscall `ret 4`): vtable slot `0x24` (`0x445a90`) true (every chase
  setting, type 1, 0 or 5): `0x43c780` identity, row 0 = `0x43cf10` of the three heading follower
  values (`heading40` channel `value0` at `+0x00`, `+0x30`, `+0x60`), row 1 = `0x43d1e0`
  (row 2, row 0), row 2 = `0x43d1e0` (row 0, row 1), translation the car origin (`+0x50`), w = 1.0.
  Slot false: `0x401340` on car `+0x90` and `+0x50`, as before.
- `0x445c20` (`SteerHeading`, thiscall `ret 8`, called first by `0x44de90` with the setting and the
  mover byte `+0x52`): a non-chase setting sets the target to row 0 of the car matrix through
  `0x574cd0` and resets the follower. A chase setting returns early when `!snap` and `0x445ac0`
  holds; otherwise the target is the car velocity (`+0x70`), reflected through matrix row 0 with
  `dot * -2.0` when the dot (accumulated z, y, x, unrounded) is negative, normalised, scaled by
  the heading curve `0x82a4e0` at car `+0x430`, scale-added with row 0 and `1.0 - s`, with z
  scaled by `1.0 - ((clock - groundTime58) * 0.00025) / 2.0` clamped to 0..1 (not above 0, NaN
  included, gives 0). With `snap`, the reset block `0x445dbb` runs: per channel
  `value0 = target8`, `velocity4 = targetVelocityC`, `state2C = 0`.
- `0x445ac0` is `DriveCameraMover::HumanBlocked` (moved from `CameraCockpit.cpp`; `Update` still
  inlines the same expression at `0x4535a9`/`0x4537c3`). `0x574cd0` is
  `CameraCubicVector::SetTarget(const float *)`, a thunk to `0x570ba0`.
- `ComputePose` now calls `SteerHeading(setting, snap52)` before `GetCarFrame`.

Verification: `tools/verify_pc_camera_chase.py --compiler msvc --mutation-checks` (manifest
`config/SPEED2_EXE_1_2/camera-chase.json`): 8,192 comparisons under 53- and 64-bit x87 precision
(2,048 each of `0x445ac0`, `0x574cd0`, `0x445e00`, `0x445c20`), whole fixture arena compared
bitwise (the output matrix, the 0x90 bytes of `heading40`, car and mover), return/ESP/preserved
registers/FPU control and tag words, the globals `0x86518c` and `0x82a4e0`, and the ordered entry
events (callee and arguments, stack pointers by content). Coverage: 956 chase and 1,092 non-chase
settings for `0x445c20`, snap and no snap, 593 computed targets (243 reflected, clamp at 0, 1 and
mid), 363 early returns through `0x445ac0`, slot `0x24` false for 400 of the frame cases;
dot-product cancellation and binary32 underflow cases are generated on purpose. 31 mutations are
rejected (type list, early return, dot order, reflect test/scale, curve input, per-component
scaling, blend, clamp, ground time, z scaling, snap stores, frame cross order, w, origin).
Also passes with `--compiler clang-cl`. Original code runs for `0x43c780`, `0x43cf10`, `0x43d1e0`,
`0x43d0c0`, `0x570ba0` and the curve lookup `0x576790`; `0x401340`, `0x5ed290` and `0x5ed300` run
as rebuilt; slot `0x24` is a 6-byte fixture stub returning an arena byte in both runs.
`verify_pc_camera_cockpit.py` (24,576 comparisons, 37 mutations; the `HumanBlocked` mutations now
target `CameraChase.cpp`), matrix, push, cars and wheel-math verifiers are unchanged.

Limits and seams:

- `0x445c20` reads the tick global `0x86518c` itself. Native: `DemoMain.cpp` passes `drivetrainClock`
  (`0x86518c`, advanced by `WorldClock.cpp`) to `Update`, so the global and the `clock` argument
  are the same value; `ComputePose` keeps the `clock` parameter (used for `groundTime58`) and
  `SteerHeading` reads the global through `cameraClock86518C` (alias in `OriginalSymbols.ld`).
  `DriveCameraMover::Construct` calls `ComputePose` with `clock` 0 but `SteerHeading` sees the
  global's current value, as the original does (z of the heading is scaled by that elapsed time).
- The curve static initialiser `0x77be00` is a native function; the verifier writes the curve.
  `0x576790` is recovered in `DriveCamera.cpp` but ran as original code here, so that rebuild is
  not compared against `0x445c20`'s use of it.
- The `t` clamp value is a binary32 spill in the original (`FST` at `0x445d58`, reloaded at
  `0x445d69`); MSVC 7.1 without `/Op` keeps it in ST0 and clang-cl ignores casts, so the source
  uses a `volatile float` for it (the dot product, by contrast, must stay unrounded, so `/Op`
  cannot be used). `volatile` is a verification-driven spelling, not original source evidence.
- MSVC 7.1 rejects the `__thiscall` keyword (C4234); the verifier defines it as `__fastcall`,
  which is the same call for the zero-argument slot `0x24`. The cockpit verifier still needs
  `--compiler clang-cl`.
- Signalling NaNs are not generated (the original copies x through integer registers and y, z
  through the x87; compiled copies may quieten them). Byte identity is not established.
  `Snap` on the follower is spelled inline in `SteerHeading`, not with
  `CameraCubicVector::Snap`, to keep the object linkable in isolation; the stores are identical.
- `UG2_DEBUG_CAMERA=1` in `DemoMain.cpp` now also prints `heading40`, the eye, target and car
  origin every 30 frames.

Native observation (chase cameras, plausibility, not equivalence): `ug2demo --camera 3`, `2`, `4`
and `0`, 420 frames static and 600 frames `--scripted`, headless SDL offscreen
(`UG2_DEBUG_CAMERA=1`, `--car SKYLINE`), all exit 0 with no traps. Static, `heading40` stays at
the car's forward (0.984, -0.177, 0.000) and the far eye sits 5.7 m behind the car origin at
+1.7 m (eye 1689.25/-882.52/25.71 for a car at 1694.96/-883.55/24.01); close 4.6 m behind, super
far 5.7 m behind and 3.4 m up. Scripted, the heading follows the car's velocity (0.984/-0.175
at frame 210, 0.988/0.154 at 390, 0.682/0.731 at 570) and the far eye stays 5.7 to 5.8 m behind
the car.

Native observation (plausibility, not equivalence): `ug2demo --camera 0` and `--camera 1`, 420
frames static and 600 frames `--scripted`, headless SDL offscreen, ran without traps; the bumper
and hood cameras stayed at their cockpit offsets (eyeLocal 0.94/0.00/1.01 and 0.58/0.00/1.27)
throughout, so none of the tests blocked the view in the scripted drive. The far camera (`--camera 3`) runs the new slot `0x24` and the eye-duration expression
without traps.

## Camera shake and finalisation (2026-10-19)

Recovered: Camera finalisation `0x447280` (`Camera::Finalise`), noise apply `0x441340`
(`Camera::ApplyNoise`) with `0x441270`, `0x4411b0`, `0x4412e0` and the rotation `0x5bacf0`, the
transpose wrapper `0x5baf80`, the drive camera shake `0x452ed0` (`DriveCameraMover::Shake`) with
the sample `0x446d00`, blend `0x5766e0`, scale-add `0x43d100` and car query `0x5ecdb0`, and the
view shake `0x572a50` with `0x572860` and `0x5722a0`. Sources: `Camera.h`/`Camera.cpp`
(partial offset-based `Camera`/`CameraState`, 0x288 bytes) and `CameraShake.cpp`.

Evidence worth recording:

- The fourth argument of `0x452ed0` pushed at `0x453b97` (`[ESP+0x5c]`) is the second table value:
  `0x453b6c` stores `[0x7ebea8 + previousType4C*4]` to `[ESP+0x74]`, which is that slot after
  `ADD ESP,0x18`. The third argument is `[0x7ebe90 + type*4]`, times `0x7870b8` when the byte
  `0x89e7d8` and the int `0x890108` are both zero. `0x452ed0` multiplies the noise sums by it
  (`[EBP+0x14]`).
- The Camera is `view +0x40` (the base constructor `0x44ab90` takes it from `0x48b1e0` and stores
  `10000.0` at `+0xc0`); finalisation copies `+0..+0xd0` to `+0xd0`, so the next frame differences
  the two. Velocities land at `+0x1e0..+0x264`; `+0x270` is a reset flag (state copied again, step 1),
  `+0x274` the step, `+0x280` the attached car.
- Update's Camera writes `0x4538c2..0x453975` are now recovered too: `+0x280` (car for non-chase
  settings unless hood without look-back), `+0xbc` near plane, `+0xb0` distance from the previous
  Camera position to car `+0x60` (`0x43ce10`), `+0xb4`/`+0xb8` (`0x78707c`/`0x787078`, or 0 with
  mover `+0x53`), all except `+0xbc` guarded by `0x828640`; `+0xc4` is `2 * 0x43e6d0(zoom,
  tan(half angle))`, replacing the earlier "half<<1" approximation (the native fov goes from 13020
  to 13018 for the far camera). `0x43e6d0`/`0x43c490` are `HudAngleUnits`/`HudArcSineUnits`.
- `0x5baf80` is `cdecl`, calls `0x63d12c` which is `JMP [0x803f00]`. `0x803f00` is a run-time
  filled function pointer (initial value `0x63d119`, a loader stub, like `0x803e74` for the
  multiply); it is not in the PE import tables. The name `D3DXMatrixTranspose` is inferred: a
  two-argument matrix function whose output `0x447280` consumes as the transpose (rows 0..2 give
  the position and the direction). Native: `D3DXMatrixTranspose` in `NativeVehicle.cpp`.
- `0x59fa30` (called with car `+0x40`) is `DrivetrainAuxiliary::GetNitrousIndex` (recovered);
  `0x5ecdb0` returns it when `|input +0x20c| <= 0.001` or `speed * 2.237 < 5`, else 0.
- The sample curve of `0x452ed0` is built on the stack (count 5, spacing `PhysicsInfo +0x20` or
  100.0, keys `0x7ec430`, 16 bytes each); bump sums use road surface `+0x18`/`+0x1c` of the four
  wheel surfaces (car `+0x394..+0x3a0`). The `TEST ECX,ECX` before the `0x59fa30` call tests
  `car + 0x40`, never null.

Verification: `tools/verify_pc_camera_shake.py --compiler msvc --mutation-checks` (manifest
`config/SPEED2_EXE_1_2/camera-shake.json`): 30,720 comparisons under 53- and 64-bit x87 precision
(2,048 each of the 15 targets: `0x447280`, `0x441340`, `0x441270`, `0x4411b0`, `0x4412e0`,
`0x5bacf0`, `0x5baf80`, `0x452ed0`, `0x446d00`, `0x5766e0`, `0x43d100`, `0x5ecdb0`, `0x5722a0`,
`0x572860`, `0x572a50`). The whole fixture arena, the camera globals, return value (ST(0) for the
float functions), ESP, callee-saved registers, FPU control/tag words and the ordered entry events
(arguments, pointers by content) are compared. Coverage includes the finalise debug branch, the
disabled global, reset flag set/clear, step positive and not, view shake gate closed and views 0..5,
curve null and non-null, nitrous stages 0..3. 70 mutations are rejected, none survive.
`--compiler clang-cl` also passes (1,500 cases, in-range mode, see below). Original code runs for
`0x43c780`, `0x43cf10`, `0x4012b0`, `0x43c2b0`, `0x43c370`, `0x43c490`, `0x5bada0`, `0x5bae60`,
`0x59fa30` (with `0x5925b0`, `0x592500`, `0x592480`, `0x5f00f0`) and `_ftol 0x75bed4`. Traced Python
stubs in both runs: D3DX multiply `0x63cedb` and transpose `0x63d12c`.

Limits and seams:

- The Camera object is a native zeroed object (`DemoMain.cpp`, `reset270` set); its constructor and
  the view object were not recovered at this checkpoint. The view constructor,
  complete plane/projection/focal producer and native focal integration are now
  recovered separately; see [view activation evidence](NATIVE_DEMO.md#view-construction-clip-planes-and-native-focal-activation-2026-10-27).
  The Camera constructor itself remains open. `Construct` stores `+0xc0 = 10000.0` as `0x44ab90` does; the
  rest of that constructor (copying `+0x280/+0x284`, `0x444040`) is not.
- `0x447280` copies `+0..+0xd0` to `+0xd0` before it rewrites the matrix, so every difference except position, direction and target (the focal, near, far, noise and field-of-view velocities, written after Update/Shake already changed those fields) is zero in the original; the verifier cannot observe a mutation of those lines (mutations of them were dropped as equivalent).
- The noise fields `+0x70..+0xac` are written only by the drive camera shake; other writers are
  not recovered. The per-view shake states at `0x865360` are written by the collision shake
  recovered below (and its tick and reset); the request flag `0x8650c4` is still written by game
  events that are not recovered, and the native demo never sets it. The view shake gate
  `*0x890080 +0x108` is 1.0 from the native scaffold, as the world constructor `0x5ff490` sets it.
- D3DX transpose: inferred from use; the real routine is not executed (stub in the verifier).
- `volatile` appears where the original stores a binary32 and reloads it (noise products, bump sums,
  translation read-back) and to keep the eight noise calls in order; it is a verification-driven
  spelling, not original source evidence. Verification only: the nitrous recorder state read through
  `0x5f00f0` is not varied; signalling NaNs are not generated; byte identity is not established.
- clang-cl converts out-of-range floats to int as `0x80000000` while MSVC's `_ftol` keeps the low
  32 bits, so `--compiler clang-cl` implies `--in-range` (no overflowing conversions, NaN or zero
  curve spacing) and uses `-fno-inline` and a `memcpy` stub; MSVC runs the full special values.
- `GetFieldAngle` returns the arcsine's EAX unchanged in the original (upper half garbage); the
  verifier compares angle arguments of `0x5bacf0`/`0x5bada0`/`0x5bae60`/`0x43c2b0`/`0x43c370` by
  their low 16 bits, which every callee masks.
- Native demo: `UG2_DEBUG_CAMERA=1` still works. Smoke (`--car SKYLINE --frames 300 --scripted`):
  the pre-shake eye/target/heading lines are identical to the previous build, the final eye differs
  from the pre-shake eye by about 0.025/-0.008/0.0 m (the shake), no NaNs; the screenshot differs
  from the old one in 4,563 pixels.

## Collision shake (2026-10-19)

Recovered: the collision shake `0x572e50` (`CameraCollisionShake`) that feeds the view shake, with
the state start `0x572810` (`CameraViewShakeState::Start`), the tick `0x5728c0`
(`CameraViewShakeTick`), the reset `0x5729e0` (`CameraViewShakeReset`), the player gate `0x5ea7c0`
(`PlayerRecord::QueryDriverCamera`) and the option mapping `0x502500` (`DriveCameraTypeForOption`,
recovered earlier in `DriveCamera.cpp` and verified here). Source: `CameraCollisionShake.cpp`. The
view shake `0x572a50` in `CameraShake.cpp` is unchanged and reads the states.

Evidence (retail `speed2.exe`, Ghidra listings and xrefs):

- `0x572e50` has one caller, the world update `0x609ce0` (`0x60a144`), for the leading cars whose
  race entry (car `+0x18`) is set; the entry's `index10` is the view.
- `0x5ea7c0` is true when the chase camera `+0x238` is non-null, its kind `+0xc` is 1, `+0x14` is
  non-zero and `0x502500(0x83aa74 + index10 * 0x28) == 3`. `0x502500` maps 0 to 2, 1 to 4, 2 to 1,
  3 to 0, 4 to 5 and anything else to 0, so it never returns the driver camera (3) and the gate is
  always false. The true branch is recorded as code only.
- The scaled vector is `(0.7 x, 0.5 y, 2.0 z)` (`0x7fb750..0x7fb758`), each component rounded to
  binary32 when stored (`0x572e7d..0x572e97`). The magnitude `m = sqrt((z*z + x*x) + y*y)` stays an
  unrounded x87 `FSQRT`. The test is `m < 250` (`0x79dcd4`) for a return, so `m == 250` and NaN
  continue (the NaN fails the amplitude test); `m` is clamped to 600 (`0x79dcd8`) and
  `t = (m - 250) / (600 - 250)` is stored as binary32.
- Amplitude `0.08 + (0.16 - 0.08) t` (`0x79dcdc`, `0x79dce0`) and duration `0.3 + (0.7 - 0.3) t`
  (`0x79dce4`, `0x79dce8`). The value to beat is the state's curve at `time14` (`0x5722a0`), or
  `0.0` (`0x7d7d3c`) without a curve; the shake starts only when `amplitude > current` (NaN returns).
- A player in state 7 (`+0x58`) scales the amplitude by 0.8 (`0x7850ec`) and the duration by 1.2
  (`0x78a0ec`). `0x43cf90` (`WheelSetLength`) writes the scaled vector to a local separate from its
  input, and `CameraViewShakeState::Start` takes that output with the duration and the frequency
  `0x79dc68` (17.0).
- `Start` (`0x572810`) negates the vector (`* -1.0` at `0x7842a0`), sets the curve `0x7fb7a0` (five
  keys: (0, 1), (0.1, 0.7), (0.3, 0.5), (0.5, 0.4), (1, 0)), time 0, duration and frequency.
- Tick `0x5728c0` runs from the game loop `0x5811c0` at `0x5814b2`, before the world clock `0x57a0e0`
  at `0x5814ea`. If the request flag `0x8650c4` is set, slot 0 starts inline with
  `-(0x7fb730, 0x7fb734, 0x7fb738)`, duration `0x79dc64` (0.6), frequency `0x79dc68`, and the flag is
  cleared. Then each slot with a curve adds `frame / duration` (`0x865154`) to `time14` and drops the
  curve when the time is below its first key or at or above its last key; an unordered time keeps it.
- Reset `0x5729e0` clears vector, curve and time of both slots, sets duration 1.0 and frequency
  `0x79dc68`, and leaves the padding at `+0xc` alone. The world constructor `0x5ff490` stores
  `world +0x108 = 1.0` (`0x5ff574`) and calls the reset (`0x5ff639`). Two more callers, `0x5fa820`
  (`0x5fa862`) and `0x5ff890` (`0x5ff984`), are not recovered.

Source map: `CameraCollisionShake.cpp` defines `CameraViewShakeState::Start`, `CameraViewShakeTick`,
`CameraViewShakeReset`, `CameraCollisionShake` and `PlayerRecord::QueryDriverCamera`. `Camera.h`
names `duration18` and declares them. `HudPlayerMasks.h` declares the gate. `WorldUpdate.h` splits
`CarWorld` at `+0x108` (named `viewShakeGate108`; sizes and offsets unchanged) and declares the call,
which `WorldUpdate.cpp` now makes where it called the trap.

Verification (set `UG2_EXE` to the retail `speed2.exe`): `tools/verify_pc_collision_shake.py --compiler msvc --mutation-checks` (manifest
`config/SPEED2_EXE_1_2/collision-shake.json`: `0x572810` 71 bytes, `0x5728c0` 274, `0x5729e0` 108,
`0x572e50` 373, `0x5ea7c0` 57, `0x502500` 43, each hashed): 6,144 comparisons, 1,024 for each of the
six targets, under 53- and 64-bit x87 precision, with no failures. The final run covers 450 collision
shakes that started and 574 that did not, 639 curve lookups, magnitudes exactly 250 (31 cases) and 600
(46), 233 directional magnitudes between 240 and 620, NaN and infinite magnitudes, views 0 (510) and 1
(514), player states 7 and others, camera kinds, active flags and a null camera, 673 tick requests
with 537 curve drops, reset padding, and every mapped option of `0x502500` with the defaults. 42
mutations are rejected and none survive. `--compiler clang-cl` also passes (6,144 comparisons, no
failures). Original code runs for `0x5722a0` and `0x43cf90`; the 44 other externals of
`DriveCamera.cpp` link to a trap that fails the comparison if it is reached. Two rounding-only
mutants (dropping `volatile` from `t` and `amplitude`, or using `float` for the magnitude) compile to
the same MSVC code, because the stored values are kept as binary32 either way, so they are not
counted. The z-square mutant `scaled[2] * scaled[2]` is equivalent too (doubling is exact), so the
suite uses a non-equivalent one.

Regressions run with the changed headers and callers: `verify_pc_camera_shake.py --compiler msvc
--cases 2048 --mutation-checks` gives 30,720 comparisons, 70 mutations rejected, none surviving;
`verify_pc_world_update.py` (default) gives 6,144 comparisons with no failures, and its stub for
`0x572e50` is now `?CameraCollisionShake@`.

Native seams (`NativeVehicle.cpp`):

- `UpdateRace572E50` (the trap) is removed; the world update calls the recovered shake.
- `BindNativePlayer` sets the human car's race entry (car `+0x18`) to the native player record
  (`index10` 0, the native chase mover of kind 1 and active). The other readers of `+0x18` were
  checked. `NotifyContact` (`0x5f4960`) takes the participant branch, since the scaffold participant
  is set. `UpdateOpponentGap` needs two players. The reset branch (`0x6009a0`) runs only after a reset
  request. Steering force feedback returns early because `forceFeedbackManager870760` is 0 in the
  retail image and nothing natively sets it, so `MoverRaceEntry::QueryFlag5EB420` is never reached.
- `InitialiseNativeRuntime` sets `worldScaffold.viewShakeGate108 = 1.0f` and calls
  `CameraViewShakeReset()`, which mirror `0x5ff574` and `0x5ff639`.
- `StepNativeVehicle` calls `CameraViewShakeTick()` after `frameDuration865154 = duration` and before
  `AdvanceWorldClock()`.

Native observations (temporary prints, removed afterwards):

- Headless `ug2sim ... SKYLINE 900` with `UG2_STEER=-0.4`: three wall impacts with magnitudes 592,
  830 (clamped to 600) and 704 (clamped). The first start is amplitude 0.158186 and duration 0.690931
  at m = 592.064, which matches the formulas; the others are 0.16 and 0.7.
- `ug2demo --camera 3 --scripted` with `UG2_STEER=-0.4`, 900 frames, offscreen: impacts at 612 and 783
  (clamped). Slot 0 starts at frame 202 and advances by 1/60 / 0.7 = 0.0238 per frame; 82 frames carry a
  live curve in total (two shakes of about 41 frames each); `0x572a50` reads these states on every
  frame of the demo.
- Both runs and `ug2demo --frames 300 --scripted` exit 0, with no trap and no NaN.

Limits:

- The gate `0x5ea7c0` is always false, as in the original (type 3 is never produced by `0x502500`).
- The request flag `0x8650c4` is written only by unrecovered game events. The native demo never sets
  it, so the tick's inline start is verified against the original but never runs natively.
- `0x5729e0` has the unrecovered callers `0x5fa820` and `0x5ff890`; the native demo does not run them.
- `0x572e50` runs for the leading cars (`count14`) that have a race entry; the native demo's single car
  qualifies.
- The original realigns the stack (`AND ESP,-16`) in `0x572e50`; the rebuilt frame is compared by content.
- The verifier compares binary32 results bit for bit at 53- and 64-bit precision; byte identity of the
  compiled code is not established.
- `0x572e50` reads `players8900AC[view]` (and `0x5ea7c0` its `+0x238`) without a null check, as the
  original does, so a missing player record stops the demo in both; the verifier never fixes that case.
- The gate of `0x572a50` still reads `carList890080 + 0x108` directly (`CameraCarList` does not name the
  offset), so `CameraShake.cpp` is unchanged.
