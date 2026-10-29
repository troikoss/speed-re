# Native vehicle demo

A 32-bit native Linux build now drives a UG2 car using the recovered vehicle
sources end to end. This playable demo is an intermediate milestone,
not completion of the decompilation. Recovered routines in this pass are
reconstructed from disassembly but are **not yet behaviour-verified** against the
original by differential execution, except for the isolated comparisons in
[vehicle recovery](pc-vehicle-physics.md), [wheel/body verification](pc-wheel-physics.md),
and the compression evidence below. Recovery-pass labels describe implementation
history, not independent verification levels.

## What runs as recovered UG2 code

The full PhysicsMover update chain from `0x5b72d0`:

- Construction: mover `0x5b5cc0`, body `0x5a7f30`/`0x59b940`/`0x43c7c0`, wheel
  `0x5a8160`, suspension arm `0x5a86c0`, engine `0x5aa5a0`/`0x457b00`, drivetrain
  `0x5af9d0` with shift points `0x5ac7d0`, inline Ackermann steering and aero objects,
  initial placement `0x5a48a0`.
- Pre-steps: load refresh `0x5ab990` (centre `0x5a60f0`, mount `0x59e0c0`, centre move
  `0x59b8b0`), mode refresh `0x5a49a0` (response scale `0x599170`, slip `0x591d90`),
  control transfer `0x5abbf0` with gear selection `0x5aa960`/`0x5a1700`/`0x594f20`/
  `0x594670`, brake/grip distribution `0x5a4d80` with traction factor `0x5990c0`, launch
  assist `0x5b6300`.
- Substeps: integrator `0x5aa480` (earlier verified), orthonormalisation `0x5bb0d0`.
- Post-integration `0x5b04d0`/`0x5a0d80`: block post-updates (wheel `0x59d030`, arm
  `0x5a8790` with ground query `0x59d250`, surface noise `0x5d3ca0`), rest handling
  `0x5abb00`/`0x5a5e20`/`0x5992b0`, bump stop `0x59e1a0`, limited impulse `0x59bc60`.
- Controller forces `0x5b0520`: aero `0x59e620`, wheel contact `0x59d160`, the earlier
  verified coupling/drivetrain/engine chain, yaw assist `0x5ac260`, wheel lift
  `0x5a51b0`, remote correction `0x5a5c20`, guide force `0x5ecf70`.
- State export `0x5a5540` (origin `0x592400`, wheel render frames `0x5921b0` with
  rotation helpers `0x5bac90`/`0x5bada0`/`0x5bae60`), state reset `0x59f880`, nitrous
  timers `0x591cd0`/`0x592480`/`0x5925b0`/`0x592500`/`0x59fa30`.
- Car-object queries: `0x5ed290`, `0x5ed470`, `0x5ebfa0`, `0x5ed220`, `0x5ed1b0`,
  `0x5ebf60`, `0x5ee250`, `0x5ec790`, `0x5f67f0`, `0x592870`, draft/ambient
  `0x5ed5d0`/`0x61e380`/`0x6134e0`/`0x5f0b40`, helpers `0x401020`, `0x43c370`,
  `0x43c390`, `0x43cf90`, `0x440f70`, `0x5eb8f0`/`0x5eb900`/`0x5eb910`.
- Static initializers that build vehicle curves (`0x77de60`..`0x77ed80`), found with
  `tools/list_pc_static_initializers.py`.

All 50 physics sources compile with MSVC 7.1 (`/Ox /Ob0`), and every partial layout
carries compile-time offset checks (`Ug2Layout.h`). No routine in this list is counted
as byte-matching.

## Track collision (second pass)

The demo now drives on UG2's L4RA streets using recovered world-collision code in
`src/Speed/Indep/Src/World/WCollision.cpp`:

- Collision manager `0x88f768` (constructor `0x5e81d0`), chunk handler `0x5e78a0` for
  per-section packs `0x80034130` and barrier groups `0x3412f`, pack fixup `0x5d8140`,
  registration `0x5e6980`, section index `0x5d3c10`, lookup `0x591380`.
- Face cache `0x5d8350`, expansion `0x5d7f10`/`0x5d2cc0`, fetch `0x5e0c90`, eviction
  `0x5dcad0`/`0x5dca70`/`0x5d82c0`, edge test `0x5d3330`.
- Per-car collider (car `+0x5c4`): constructor `0x5e0b40`, release/area `0x5dc9c0`/
  `0x5d80a0`/`0x5e5890`, pack switching `0x5e6890`/`0x5e68d0`, and the ground query
  `0x5e72e0` (cached faces, 32 m cells, 8 m sub-cell masks, fixed-point bounds).
- Section topology `0x883e70` (chunk `0x80034150`, loader `0x5e9c10`), tracker
  `0x5dfeb0`, section search `0x5db850`/`0x5db770`, polygon tests `0x5d6dd0`,
  `0x5d7090`, `0x5d7270`, `0x5d70d0`.
- Its complete constructor `0x5e6360` and shared section-resource registry
  `0x5db680`/`0x5db6e0` are recovered in `TrackSectionResources.cpp`; native world
  initialization now calls the complete constructor. See lighting loading below.
- Skid-mark emitter `0x59dbb0` with rotation helper `0x59db50`.

The skid-mark trail methods (`0x5e6e70`, `0x5d3230`) are a renderer seam: the demo
records segments to draw them. Wall collision is recovered in the fifth pass below; the
demo still respawns a car that leaves the collision coverage (scaffolding).

## Wall collision (fifth pass)

Cars now collide with the track's walls through UG2's collision-body world, recovered in
`src/Speed/Indep/Src/Physics/Collision/` (class names from the original slot pools and
serialisation strings). Walls are not special-cased: every frame each moving body turns
the track faces near it into `TrackPolygonCollisionBody` objects, pairs overlapping bodies,
measures pairs with GJK and resolves touching pairs with an impulse.

- World step `0x5ffa50` (after `0x5f0f00` advances every dynamic body), stale pairs
  `0x592e60`, pair search `0x5b23b0` over the body list kept sorted by maximum X
  (`0x5a0680`, largest width `0x5a05a0`), contact list sort `0x43bee0`/`0x5a0eb0`/`0x5a0dc0`,
  contact removal `0x5af090`, orphan polygons `0x5a0cd0`, pools `0x5a0200`/`0x593db0`/`0x592b00`.
- Bodies: `CollisionBody` `0x5a04f0`/`0x5a9c00`, `DynamicCollisionBody` `0x5a9ce0` (pose
  snapshot `0x593210`, interpolated pose `0x593280`, rewind `0x593630`/`0x593590`, mover
  attach `0x5938c0`, shape extents `0x5ae520`, bounds `0x5a9de0`, polygon reach `0x5937b0`,
  polygon spawning `0x5ae930`), `CarCollisionBody` `0x5aec20` with slots `0x593b40`..`0x5a0be0`
  (impulse `0x5a0b00` → `MoverBody` `0x59bd80`, inverse mass/inertia `0x593a00`/`0x593a40`,
  restitution/friction `0x5a0c90`/`0x5a0be0`, shape choice `0x5a0a80`), `TrackPolygonCollisionBody`
  `0x5aa0b0`/`0x5aefa0` with its fixed-body slots and polygon shape `0x5a1a90`.
- Track faces near a body: collider area update `0x5e7100`, area fill `0x5e0d30`, planar
  face distance `0x5d3450`; car hulls from `CARS/<car>/GEOMETRY.BIN` chunk `0x80034020`
  (`0x5a1750`, lookup `0x5a18c0`, key = model name + `_CV`), falling back to the box `0x598760`.
- Shapes: vertices `0x5aad20`/`0x5a1970`/`0x5a2010`/`0x5987d0`, support points (hull hill
  climbing `0x5afba0`, polygon `0x5a2060`, box `0x5a2280`, point `0x5a2360`).
- Pair witness `0x5a0340`/`0x5b22b0`, shapes `0x598590`, poses `0x598620`, distance update
  `0x5b3d20`, GJK `0x5b3390` with the feature-pair sub-algorithm `0x595040` and simplex copies
  `0x598130`/`0x598240`, pair test `0x5b5050`, thick-polygon flags `0x592f00`/`0x593010`.
- Contacts: impact time bisection `0x5b4be0`, separation `0x5b42f0`, polygon normal side
  `0x593df0`, contact frame `0x594000`, resolve `0x5b2610` (rewind, impulse, re-advance),
  impulse with restitution and friction `0x5af0e0`/`0x5941c0`.
- Contact events (the mover's "contact bodies"): `0x5a9b10`/`0x5a9430`/`0x5a9570`/`0x5ae470`,
  lookup `0x5a0820`, wall/floor tests `0x5a0160`/`0x5a01b0`, accumulation part of `0x5a9620`.
- World clock `0x57a0e0` (60 Hz frames, 4 kHz ticks at `0x86518c`), which ages the events.

Verification: `tools/verify_pc_collision_simplex.py` runs the original and the MSVC 7.1
rebuild of `0x595040`/`0x598130`/`0x598240` in Unicorn on 6,400 synthetic simplices. All
non-degenerate cases agree within float rounding (about two thirds bit-identical); 42
cases with duplicated or collinear vertices pick a different vertex at an exact tie
(40 duplicate-vertex ties, where the two vertex indices name identical points, and 2
collinear-midpoint cases; attributed to x87 spill rounding, not examined at instruction
level). `tools/verify_pc_collision_gjk.py` runs the GJK distance `0x5b3390` (body
`0x5b3390..0x5b3d20`) against its MSVC 7.1 rebuild on 8,000 cases (argument `2000`) (2,000 shape pairs of hull,
polygon, box and point in all 15 type pairs and random poses, each cold-started, warm-started
twice from the original's own simplex with nudged poses, and warm-started from random valid
indices; both FPU control words): 0 failures, 4,520 bit-identical, largest return difference
1e-7 relative. Every callee of the rebuild links to the original (support `0x5afba0`/`0x5a2060`/
`0x5a2280`/`0x5a2360`, vertex dispatch `0x5aad20`, simplex `0x595040`/`0x598130`/`0x598240`,
`0x43c9c0`, `0x43cf10`); the only probe seams are a constant for `bGetTicker` `0x43bdf0`, a reset
of the `bRandom` seed, an SEH slot for the hull support, and a shim adapting the rebuild's cdecl
`CollisionShapeVertex` call to the original thiscall-style `0x5aad20` (a calling-convention
difference of the recovered source, not of behaviour). Cylinder pairs (type 5, original support
`0x5a23d0`) are generated since 2026-10-19; the default 1,500-trial run (6,000 comparisons over all
24 ordered pair kinds) has 0 failures. The cylinder's `+0x14` vertex count of 0x80 is a generator
assumption: no cylinder constructor has been located. The check found a real divergence, fixed in `CollisionWitness.cpp`: the
loop compared the unrounded x87 result of `Distance` with the previous distance, whereas the
original stores it as a float (`0x5b3c36`) and compares the reloaded value; the rebuild then
cycled forever on tie-like inputs (6 of 395 early cases). `tools/verify_pc_collision_cylinder.py` (2026-10-19) runs cylinder support `0x5a23d0` and vertex
`0x5a2780` against their rebuilds (`CollisionShape.cpp`): 6,000 support and 153,856 vertex cases
over all three axis rows of table `0x7a1930`, with zero, ±0, NaN, axis-aligned and ring-boundary
directions, under both FPU control words. It found 0 failures, produced all 128 support indices and
rejected 9 of 9 mutants. 384 of 398 original instructions run; the 14 unreached ones are the
±7 clamps (the cast peaked at 7.9999997 in a 400k search) and the negative `% 64` fix-up
(`i + 0x38` is always positive). The vertex routine writes only the point (`+0x10..+0x1c`).
`tools/verify_pc_collision_impulse.py`
runs the contact impulse `0x5af0e0` against its rebuild on 3,000 scripted contacts (car, track
polygon, wall polygon and world bodies; 1 to 4 loop iterations; early return, shares of 1, single
and negative, unbreakable sentinel; sliding below, at and above the ramp speed; mode 4 arm
flattening; resting clamp; both FPU control words) and compares the full call log (every
virtual slot with its arguments by value) and the fake-world memory: 0 failures, 1,418
bit-identical, largest log difference 8e-6 (vectors scaled by their largest component). The
bodies are probe-only fake objects with x86 stub vtables; `0x594000` (ContactFrame), `0x5a9b10`
(CreateEvent) and the event accumulate `0x5a9620` are hooked identically on both sides, so their
semantics are not covered, and `0x5941c0` runs original-versus-rebuilt. The remaining
impulse differences are codegen: MSVC reorders the three-term dot products and duplicates the
loop head, so the sum order differs from the original at rounding level. Nothing here is
byte-matching; the rest is behaviour observed in the demo only. In the headless run with `UG2_STEER=-0.4` the Skyline hits
walls at 71 and 90 km/h, loses roughly half its speed, and its centre stays 1.2 to 2.3 m
in front of the wall plane.

World-object chain (2026-10-19, `CollisionWorldObject.cpp`, complete except its leaf seams):
`PromoteWorldObject` `0x5b31f0` (promotes through `0x5b1390`, then destroys the body through vtable
slot 0 with argument 1), the promotion body `0x5b1390` (pool test `0x43cd10`; matrix copy of the
entry's transform at `+0x30`; creation through the pointer `0x86af90`; for a body of kind 0 -- virtual
slot `0x8c` -- a local collider `0x5e0b40`/`0x5e68d0`/`0x5e7e70`/`0x5e0bf0` finds the ground below, the new body's
shape bounds `0x592a10` give the lowering `ground - ((min.z + descriptor[0xb8]) * normal.z + z) + 0.05`,
stored at `+0x168` and added to the height when positive; the entry number `0x5a8b70` goes to `+0x164`
and its bit into the bitmap `0x86ad78`), `BreakWorldObject` `0x5ae000` (one child per descriptor entry,
name `"_CHILDx"`, hash `0x43db80` = `HudNameHash`), the child attach `0x5adb20` (transform without translation
as orientation, mover pose `0x59b9b0`, placement `0x40f010` with the lowered transform, `+0x70/+0xb0/+0xc0/+0x6c`,
slot 4 of the object at `+0x130`, bounds refresh `0x5a9de0`, transform copied into both node lists),
the child update `0x5add50` (area query `0x5e6fa0` with radius + 0.05 x speed, at most 6.0, else pack update `0x5e68d0`),
the activation `0x5adf90`, the registration `0x48bc50` (xorshift seed, state switch, list front or back; the float RNG
`0x487360` and the CRT `_ftol`), the placement leaves `0x40f010`/`0x6147e0`, the numbering `0x5a8b70` with its
list helper `0x502fd0`, the shape bounds `0x592a10` and the seed accessor `0x611000` are recovered.
`tools/verify_pc_collision_world_object.py` runs the originals and the MSVC 7.1 rebuild on 3,000 random worlds
per routine (41,175 compared runs, 225 identical faults: the null promotion argument), as real machine code
from the entry points down (Break -> create -> attach -> activate -> update/register; Promote -> promotion body -> attach),
and compares the seam call log (pointer arguments to stack data by content), EAX where it is a result, ESP,
callee-saved registers, the SEH chain head at `fs:[0]`, and 44 KB of fake-world memory plus the world object, RNG state and bitmap:
0 failures, 694 of 696 instructions reached (the two others, the `FADD 2^32` after `AND 0x7fffffff` in `0x487360` and `0x48bc50`, are dead
code), 36 of 36 mutants rejected (`--mutation-checks`). The matrix copy `0x4012b0`, the transform `0x43c9c0`, `HudNameHash` and `_ftol` run as the
original bytes on both sides (verified elsewhere; the rebuild's `__ftol2` is bound to `_ftol`). Seams hooked identically on both
sides, so not covered: `0x43cd10`, `0x5e0b40`/`0x5e0bf0`, `0x5e68d0` (the stub stores the scripted pack at collider `+8`),
`0x5e6fa0`, `0x5e7e70` (scripted height on the x87 stack, normal and found flag), `0x59b9b0`, `0x5a9de0`, the creation function,
and the body's virtual slots 0, 8, 0xc, 0x8c and the `+0x130` slot 4. Differences believed to be codegen only: the
original has an SEH frame around the collider and 16-byte stack alignment; MSVC picks other x87 sequences (e.g. `FUCOMPP` in
`0x48bc50`); `0x5adb20` moves the y and z of the optional vectors through the x87 stack, which quietens signalling NaNs (the random vectors contain none).
Descriptor (`+0x148`), count (`+0x8c`), entries (`+0x90`), placement (`+0x480`), the collider at `+0x16c`, `+0x164`/`+0x168` and child
fields `+0x4f4/+0x4fc/+0x505/+0x506/+0x15c` stay offset-based. Still a trap in `NativeCollision.cpp`: the world-object kind in virtual slot
`0x8c` (`WorldObjectKind`), so the demo stops there for a world object it promotes; the pointer at `0x86af90` keeps the value in
the mapped original image and no native replacement exists.

Scaffolding and seams (`src/Platform/Native/NativeCollision.cpp`): the car update `0x6012b0`
is recovered ([evidence](pc-vehicle-physics.md#car-update-0x6012b0-recovered)); its reset
callees trap and the mover step is reached through a native vtable seam; collision effects after the impulse sums (sparks, sounds,
replay, force feedback), audio registration `0x483e20`/`0x483e40` and the game message
`0x5e0360` are no-ops; the hull key `0x61c320` is computed by the loader for stock parts;
slot pools are the recovered bWare pools ([slot pool evidence](pc-bware-slotpool.md)) over a
calloc-backed heap seam; the world clock runs after the
step (its original call site is not recovered). World objects (type 1/2) now stop at the seams above;
replay serialisation and mover point velocity (`0x59bec0`) are trapped. `UG2_DEBUG_COLLISION=1`
prints every impulse with the wall normal and the car's distance to the wall plane.

## Original assets (third pass)

The demo now draws UG2's own car and track data from the player's installation instead of
placeholder boxes and collision polygons.

Recovered and behaviour-verified (`tools/verify_pc_compression.py`): the bWare
decompressors in `src/Speed/Indep/Libs/bWare/Compression.cpp`, namely dispatcher
`0x574670`, JDLZ `0x573700`, HUFF wrapper `0x5744c0` with the EA Huffman decoder
`0x573840`, and RAWW `0x5744f0`. All 2,404 compressed slots of the Skyline, shared car and
Skyline vinyl texture packs decode byte-for-byte like the original running in Unicorn,
including 64 guard bytes after each output. The sources compile with MSVC 7.1; they are not
counted as byte-matching. The COMP decoder `0x574530` is not recovered.

Asset reading and drawing (`NativeAssets.cpp`, `NativeScene.cpp`) is platform scaffolding.
The original loaders (`0x493bb0`, `0x494ea0`, `0x5cc280`) are not recovered. The data
layouts used, checked on the retail files:

- Solid (`0x80134010`): header `0x134011` with the full-name hash at `+0x10` (stored names
  are cut to 27 characters, so parts are matched by hash) and bounds at `+0x20`/`+0x30`;
  texture slots `0x134012`; platform info `0x134900` with the vertex count at `+0x34`;
  vertices `0x134b01` (128-aligned, 36-byte position/normal/colour/UV on cars, 24-byte
  position/colour/UV on 98% of track solids); 60-byte draw records `0x134b02` (index count
  `+0xc`, texture slot `+0x1c`, start `+0x34`, flags `+0x38`); 16-bit indices `0x134b03`.
- Texture pack (`0xb3300000`): `0x33310004` holds 0x7c-byte TextureInfo records after a
  fixed 12-byte pad (name, hash `+0x18`, data offset `+0x24`, palette offset `+0x28`, size
  `+0x2c`, width/height `+0x38`, compression `+0x3e`: 0x08 palette, 0x20 BGRA, 0x22/0x24/
  0x26 DXT1/3/5). Pixel data starts at the `0x33320002` payload aligned to 0x80. Car packs
  instead list 24-byte `0x33310003` slots (hash, file offset, packed size, size, TextureInfo
  distance from the end) of JDLZ/HUFF payloads holding pixels followed by the TextureInfo.
- Scenery section (`0x80034100`): `0x34102` 0x44-byte infos (name, LOD solid hashes at
  `+0x20`/`+0x24`); `0x34103` 0x40-byte instances (bounds, info index in the low half and
  flags in the high half of `+0x18`, position `+0x20`, int16 rotation rows at `+0x2c` with
  0x2000 = 1). All 48,270 L4RA instances resolve to loaded solids. The section number
  is at `0x34101` `+0xc` (letter `n / 100 + 0x40`, number `n % 100`).

### Scenery visibility (fourth pass)

Read from the original and applied by the demo (behaviour, not byte matching):

- Visible sections, `0x5e96b0`: the drivable section whose boundary holds the eye
  (`0x5e00d0`: bounds test `0x43d410`, even-odd crossing test `0x5d6dd0`) supplies a list
  of section numbers; the Z sections 2600-2699 (SKYDOME) are always added. With no
  drivable section the loaded sections numbered below 10 are used. The data is
  `0x80034150` in `L4RA.BUN`, loaded by `0x5e9c10`: `0x34151` header, `0x34152` boundaries
  (section `+0x8`, point count `+0xa`, bounds `+0xc`/`+0x14`, x/y points from `+0x24`),
  `0x34153` 0xa4-byte drivable records (section `+0x8`, count `+0x10`, sections from
  `+0x14`). The nearest-boundary fallback `0x5db770` is not recovered.
  Drawing every section put the low-detail panoramas (`PAN_*`, sections X9x) over the
  detailed scenery: that was the smeared ground and the black rectangle.
- Draw-context flags, setup `0x4888a0`: context `+0x44` is 0x10 always, 0x08/0x04 by
  `0x89e7a4` (0 for the forward direction), 0x02 for eView `+4` types 1/2, 0x01 when
  `0x8707dc` is 3 or 4, 0x40 for 0x1800 (mirror) views, 0x20 for type 3. The per-instance
  test in `0x5d6570` skips an instance when `(instance +0x1a ^ 0x60) & (byte)context +0x44`;
  the recovered setup now supplies 0x1a for the native player view. Instances flagged 0x10 (race barriers, shop
  beams, start/finish graphics) are therefore hidden in every view.
- Instance flag 0x100 queues the model with no matrix (its vertices are in world space).
- Scenery is reached through the `0x34104` 0x24-byte tree nodes (bounds, child count
  `+0x18`, up to five children; negative entries are nodes), traversed by `0x5d69b0`.
  Every L4RA instance is reachable, so the demo walks the instance array.

Scaffold render states (the D3D renderer is not recovered): TextureInfo `+0x49` reads as
alpha usage (0 none, 1 punch-through, 2 modulated) and `+0x4a` as blend type (0 none,
1 blended, 2 additive), judged from how they are spread across L4RA's textures. Blend
type 0 draws opaque and alpha-tests only punch-through textures, so the alpha of road and
terrain textures (modulated, used as a mask) is ignored. Types 1 and 2 are drawn after
the opaque pass without depth writes. Extra texture packs: `TRACKS/LOC4DYNTEX.BIN`
(SKYDOME), `GLOBAL/InGameCommon.bun` (SFX_LIGHT_BEAMA), `GLOBAL/GLOBALB.BUN` and
`GLOBAL/GlobalTexturesNA.bin`. The sky dome is drawn scaled about the eye because it is
wider than the demo's far plane, and the clear colour is a fixed dusk tone.

Debug aids: `UG2_DEBUG_AT="x y"` lists the instances whose bounds hold a point,
`UG2_DEBUG_PICK="x y"` names the scenery instance drawn at a screenshot pixel (top-left
origin, final frame; it redraws into the stencil buffer), and `UG2_DEBUG_SECTIONS=1`
prints the drivable section and its visible list whenever it changes.

The stock car is `<CAR>_BASE_A` plus the KIT00 parts of LOD A. The generic texture keys
those parts use are the replacement keys built by CarRenderInfo `0x638190` and the
used-texture setup `0x631e20`, resolved with lights off and no damage: `WINDOW_*`,
`DRIVER` → `DRIVER_MALE` (the in-game branch of `0x638190`), `BRAKELIGHT_LEFT/RIGHT` →
`<CAR>_KIT00_BRAKELIGHT_OFF`, `BRAKELIGHT_CENTRE` → `<CAR>_CENTRE_BRAKELIGHT_OFF`. Scaffold
choices: windows bind GLOBALB's `WINDOW`; the paint/vinyl layers (`<CAR>_SKIN1` and the
unnamed body-panel and skirt keys `0x3c84d757`/`0xa7366ae6`) are drawn in a fixed colour;
lighting is fixed-function OpenGL. The wheels, brakes and the third-set solids are drawn by the recovered wheel block
(`CarWheelDraw`, original `0x62700e`..`0x62df66` of the car draw `0x620ae0`; see
[car wheel block](#car-wheel-block-recovered)). `DrawCarModel` passes the car-local frames of
`MoverModeOwner::GetWheelRenderMatrices` (`0x5f6820`, tyres and brakes) and
`GetWheelSpinMatrices` (`0x5ec7f0`, third set), the body LOD and view clamp of the recovered selection (`CarLodSelect`),
and `NativeCarModel::Draw` passes the body frame
(`matrixD0` with `origin10`) as the block's body matrix. The old mirror (`glScalef(1,-1,1)`), the
centring translation and the `mirrored[]` choice are gone. The fallback `DrawWheel` (no model
loaded) still composes `DrivetrainWheel` `matrix30`/`matrix70` natively.

## Car wheel block (recovered)

Recovered in `src/Speed/Indep/Src/World/CarRenderWheels.cpp` (the pixel size and the LOD selection that feed it are in
`CarRenderLod.cpp`, see [pixel size and LOD selection](#pixel-size-and-lod-selection-recovered)). Routines of the wheel
block: `CarRenderWheelLayout` = `0x61fe10`,
`CarWheelDraw` = `0x62700e`..`0x62df66`, with the generated chain stages). Verifier:
`tools/verify_pc_wheel_draw.py` with `config/SPEED2_EXE_1_2/wheel-draw.json`. Behaviour and byte matching are
reported separately: the verifier compares the callee event sequence and the arena bytes of the original and of the
MSVC 7.1 build run in Unicorn; byte identity of the compiled code is not claimed.

Per block (wheel `w`, E = tyre or third frame of wheel `w`, spun by a rotation about x when the 16-bit spin term is set
for wheels 0 and 1; `set0`, `set1`, `set3` are the block's per-wheel frames; `BODY` is the body matrix):

| Block | Wheel | Frame | Product chain | Solid (native) | World matrix | Extra record | Flags |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 0 | tyre | `set0[0] x R180`, `x E`, `x set1[0]`, `x BODY` | front tyre (`ri+0x1b48`) | `A1+0x00` | `A3+0x000` | `[+0xcc]\|[+0x1ac]\|arg8` |
| 2 | 0 | brake | `set0[0] x R180`, `x E`, `x set3[0]`, `x BODY` | front brake (`ri+0x1b58`) | `A2+0x00` | `A4+0x000` | `[+0x1ac]\|arg8` |
| 3 | 1 | tyre | `set0[1] x E`, `x set1[1]`, `x BODY` (no R180) | front tyre | `A1+0x40` | `A3+0x1c0` | `[+0xcc]\|[+0x1ac]\|arg8` |
| 4 | 1 | brake | `set0[1] x E`, `x set3[1]`, `x BODY` | front brake | `A2+0x40` | `A4+0x1c0` | `[+0x1ac]\|arg8` |
| 5 | 2 | tyre | `set0[2] x E`, `x set1[2]`, `x BODY` | rear tyre (`ri+0x1b68`; `ri+0x1b6c` on the LOD path) | `A1+0x80` | `A3+0x380` | `[+0xcc]\|[+0x1ac]\|arg8` |
| 6 | 2 | brake | `set0[2] x E`, `x set3[2]`, `x BODY` | rear brake (`ri+0x1b78`) | `A2+0x80` | `A4+0x380` | `[+0x1ac]\|arg8` |
| 7 | 3 | tyre | `set0[3] x R180`, `x E`, `x set1[3]`, `x BODY` | rear tyre (`ri+0x1b68`; `ri+0x1b6c` on the LOD path, as block 5) | `A1+0xc0` | `A3+0x540` | `[+0xcc]\|[+0x1ac]\|arg8` |
| 8 | 3 | brake | `set0[3] x R180`, `x E`, `x set3[3]`, `x BODY` | rear brake (`ri+0x1b78`) | `A2+0xc0` | `A4+0x540` | `[+0x1ac]\|arg8` |

`A1`..`A4` are the four allocations of the block, in this order: `UG2WheelAllocMatrices(4)` twice (`A1` tyre worlds,
`A2` brake worlds) and `UG2WheelAllocRecords(0x700)` twice (`A3` tyre records, `A4` brake records). A block runs when its
wheel's presence byte (`ri+0x2d78/0x2d98/0x2db8/0x2dd8`) is set and, when the owner pointer `ri+0x000` is set, the owner's
byte `+0x3f4+w` is non-zero (`WheelPresence`). Brake blocks also need the spinner (`ri+0x1b58` non-null). Each draw needs a
non-null solid. The summation order of each element differs between blocks, so the stage routines `WheelChainBlock1`..
`WheelChainBlock8` in `CarRenderWheels.cpp` were transcribed per block from the instruction stream by a generator that is
not part of the repository. Blocks 2, 4, 6 and 8 use the `set3` frames: element 13 of `set3` is the brake-side translation.

The block's native seams are listed in
[the replacement inventory](NATIVE_REPLACEMENT_INVENTORY.md#car-wheel-rendering-seams).

### Pixel size and LOD selection (recovered)

Recovered in `src/Speed/Indep/Src/World/CarRenderLod.cpp` (header `CarRenderLod.h`). Verifier:
`tools/verify_pc_car_draw_lod.py` with `config/SPEED2_EXE_1_2/car-draw-lod.json`.

- `CarViewPixelRadius` (`0x48acb0`, `__thiscall` on the view, two stack arguments): the projected radius in pixels of a
  sphere of radius 3.0 at the car origin. It is 0 when the sphere lies behind the view (depth below minus the radius),
  `2 * focal` when the distance minus the radius is at most half the radius, and otherwise `(focal / (distance - radius))
  * radius`, truncated. The view fields are `view+0xc` (focal length in pixels) and `view+0x40` (the frame: origin at
  `+0x40`, depth axis at `+0x50`).
- `CarLodPixelSize` (`0x620c6b`..`0x620cc0`): the pixel is `CarViewPixelRadius(view, car origin, 3.0)`. When the view mode
  (`0x5b98a0`, dword `0x8707dc`) is 3 and the gate `0x8654a4` is not 3, the pixel is multiplied by `0.7` (`0x7a6700`,
  truncated) and the caller's LOD inputs `[EBP+0x34]` and `[EBP+0x30]` are set to 1. The car draw leaves when the pixel is
  below 4 (`0x620cba`).
- `CarLodSelect` (`0x62107e`..`0x621230`): the owner words (`frame+0x180` = `owner+0x958`; `frame+0x14c` = the owner and
  that word are non-zero); the car record search (`0x6210bb`..`0x62110b`: the first record of the table at `0x8a1cd4`, count
  `0x8a1cd8`, whose key dword equals `[row+0x840]` and whose value dword equals the sign-extended byte `[car+7]`; the index is
  kept in `frame+0x68`); the wheel flags (`frame+0xe8` and `frame+0x1ac` are 1 when `0x7fee4c` is zero or the pixel is below
  `0x7a66fc` = 18.0, strict); the body LOD and clamp (incremented while the pixel is below `0x7a66dc[lod]`, strict, with the
  float pixel after the first test; the table is `{40, 30, 20, 0, 40, 20, 10, 0}`); a non-zero result of `0x499db0`
  (byte `0x836352`) forces both to 0; otherwise the LOD is limited to 3, and a LOD of 3 becomes 2 when the owner has no
  `+0x424` word and `ri+0x2eb8` is non-zero; the overrides `0x802728` (LOD) and `0x80272c` (clamp) replace the values when
  they are not -1; both -1 leaves the draw (`0x62121c`); `arg8` gets `0x800` for a LOD of 1 or more (`0x621229`).

Evidence and limits:

- The body LOD `frame+0x6c` is read as an index into the car's solid table `ri+0x1ba8` at `0x626f80` (`lod + j`, `j` = 0,
  1) and the block reads it as `lod6C`. The car+0x610 branch (`0x626438`..`0x62646a`) stores an allocation pointer in
  `frame+0x6c` (`0x626466`). The native path does not run the car draw, so it never reaches that branch.
- Writers before the wheel block: `frame+0x6c`: `0x621153`, `0x62118b`, `0x62119a`, `0x6211b5`, `0x6211e8`, `0x6211f9`,
  `0x622184` (float), `0x626466`. `frame+0xf4`: `0x62115b`, `0x621184`, `0x62119e`, `0x621208`; the block itself clamps it to
  2 at `0x627653`. `frame+0x1ac`: `0x62111c`, `0x621142`. `frame+0xe8`: `0x621115`, `0x62113b`; the block overwrites it with
  its record allocation at `0x6274ea`.
- The view mode `0x8707dc` and the zero flag `0x836352` are image globals (both 0 in the image).
  `UG2CarViewModeGlobal` and `UG2CarLodZeroFlag` now execute their recovered global reads;
  the globals' full game-state producers remain separate. The gate `0x8654a4` is the block's gate.
- Open, slot index `k` (`frame+0xd8`, the block's `slotD8`): at `0x626476`..`0x626486` the car draw loads `k = [obj+0x2c10]`
  with `obj` = `frame+0x60`, which holds an object pointer on that path (stores of `ESI` at `0x622455`, `0x6243f7` and
  `0x622551`). `ri+0x2c10` is also written by `0x634800` (`ri+0x2c04`..`+0x2c10`, the results of four hashed lookups in
  `0x488fa0`, keys `0x90c81258`, `0x9b971511`, `0x0ee87ec4` and one earlier key). None of the candidate part names I tried
  hashes to these keys, so the value of `k` is not established. The demo uses 0.
- Open, `frame+0xcc` (`flagsCC`): `0x621b9e` stores the result of `0x60d1b0` applied to the previous value. Not recovered;
  the demo uses 0.
- Open, LOD B and C solids: the wheel solids are the part-table entries `ri+0x1b48` (part `0x1d`) and `ri+0x1b68`
  (part `0x1e`), filled by `0x634800`. The letter-to-LOD mapping is not evidenced, so the demo keeps LOD A for every LOD
  index (the fallback).
- Native seams: the view is the native camera (position, unit view direction) with the focal length `0.5 * height /
  tangentY` in pixels (the original's focal length is set by the camera code, which is not recovered). Without a view the
  pixel is taken as 1000 (LOD 0). The owner `ri+0` is null in the demo, so the clamp of LOD 3 to 2 does not apply: at a
  distance the body LOD is 3, and the block's own `lod6C > 2` rule draws no wheels.
- Debug output: `UG2_DEBUG_CAR_LOD=1` prints the pixel, body LOD, clamp, `flags1AC` and `arg8` once per car draw.

### Car render record lookups (2026-10-26)

`CarRenderRecords.cpp` recovers two wheel-rendering callees from PC v1.2:

- `0x48d860`, `CarRenderSolid::SetSlot`: read the table at solid `+0xc`, ignore a null
  table or zero value, sign-extend the byte count at table `+0x1a`, and update **every**
  matching key in the eight-byte entries at table `+0x3c`. Duplicate keys are not an
  early exit. Zero cannot be used to clear a slot.
- `0x60fef0`, `CarRenderPart::FindRecord`: the unsigned word at part `+0xa` is an offset
  in words into the index table at global `[0x8a1cc8]+0x14`; `0xffff` returns null
  before reading the global. The first word is a signed count; subsequent signed
  words index eight-byte records relative to global `+0x1c`. A previous record is
  located by pointer equality and skipped; an absent previous or a previous in the
  final entry returns null. Hash zero returns the next record without a hash test.
  Negative counts produce no result; negative record indices are sign-extended.

Evidence: live Ghidra decompilation and disassembly, checked against the executable
bytes. `config/SPEED2_EXE_1_2/car-render-records.json` pins the executable and the
49-byte/130-byte routine spans. No Ghidra project mutation was needed.

Verification: with `UG2_EXE` pointing to PC v1.2,
`build/venv/bin/python tools/verify_pc_car_render_records.py --mutation-checks`
compares MSVC 7.1 `/Ox /Ob0 /EHsc` objects with original machine code in Unicorn:
1,536 setter and 1,114 lookup runs, zero failures, ten of ten mutations rejected.
Compared: complete fixture memory, returned record pointers, thiscall stack cleanup,
and callee-saved registers. Setter EAX is unspecified and is not compared. Cases
include all signed byte counts, signed word count/index extremes, the unsigned
part-offset boundary, duplicate keys/record indices, zero value, wildcard/missing
hashes, and previous-record positions. This is bounded behavioral evidence;
byte identity and exhaustive input coverage are not established.

Native integration: both no-op method bodies were removed from `NativeCarWheels.cpp`.
`NativeDrawSolid` has a checked original-compatible table pointer at `+0xc`, initialized
null. The global part table is null and the native car still supplies no rim parts.
Table construction, slot consumers and rim selection remain explicit seams; this
recovery does not claim a visual change or enable spinner offsets in the demo.

Platform validation: `cmake -S . -B build/native` and `cmake --build build/native -j 6`
passed. `verify_pc_wheel_draw.py` passed its existing 512 layout and 2,048 block cases
(28,869 compared events); its method calls still use original helpers/hooks, so this
is regression evidence for the wheel block rather than a second helper verification.
The SKYLINE demo completed 60 frames with `--no-audio --orbit 90`; the capture in
`build/analysis/car-render-records/smoke.ppm` was inspected. The native build retains
its existing non-standard-layout `offsetof` warnings.

### Wheel facing helper (2026-10-26)

`CarWheelFacing.cpp` recovers `0x60d700`. If `[0x802914]` is zero, it returns
`[0x784250]` (1.0 in the reference image) without touching either argument; the
previous native stub incorrectly returned zero. Otherwise it snapshots the view's
axis at frame `+0x50`, copies the world matrix through `0x4012b0`, and transforms
normal `(0x8a1c54, 0x802910, 0x8a1c58)` by the matrix's three basis columns.
Each component sums Z, Y, then X products. X and Z are explicitly stored as floats
at `0x60d7c2` and `0x60d7bc`; Y remains in x87 precision. The final sum is
`(Z * axisZ + Y * axisY) + X * axisX`, returned directly in ST0.

The declaration uses `long double` to preserve the native i386 x87 return and
intermediate precision; legacy MSVC 7.1 treats that type as double while retaining
its register precision. No binary32 return conversion is inserted. Globals are
initialized to measured image values (enabled = 1, normal = 0/1/0); the native
normal producer remains open. The current demo's LOD gate is zero, so this helper
stays unreached there. Gate 6 uses it for rear-solid substitution; gate 3 uses it
for the secondary draw.

`tools/verify_pc_wheel_facing.py --mutation-checks` and the same command with
`--gcc` compare the original with MSVC 7.1 `/Ox /Ob0 /EHsc` and native i386 GCC
`-O1 -mfpmath=387 -fno-fast-math -ffp-contract=off` code. Each compiler passes
8,392 comparisons with **exact raw ST0 significand/exponent equality**, both x87
precision settings (`0x027f`, `0x037f`), and eight of eight mutations rejected.
The verifier also checks matrix-copy calls, untouched fixture memory, stack
cleanup, callee-saved registers and control-word preservation. Disabled cases
use invalid argument pointers to check the early return. The executable, helper,
copy routine and initial globals are pinned in `wheel-facing.json`. This is
bounded finite-input numeric behavior; byte matching and exhaustive coverage
are not claimed. The i386 demo build, the existing wheel verifier (512 layout and
2,048 block cases), and a 60-frame SKYLINE `--no-audio --orbit 90` smoke run passed.
The smoke validates integration with gate zero, not activation of the special draw.

Related evidence corrects an earlier inventory label: `0x48f660` is render-record
preparation, not a pure visibility/LOD test. It calls `0x48e6c0(record, arg6, world)`
then `0x48a6a0(record, world, arg3, arg4)` and returns 1. Both callees write the extra
record passed to DrawSolid. Their recovery and consumption remain open; ignoring
the return value does not eliminate their side effects.

### Render-record foundations (2026-10-26)

`CarRenderRecord.h` describes the observed 0x1c0-byte record consumed and produced
by the `0x48f660` chain. `CarRenderRecord.cpp` recovers three dependencies:

- `0x48a3b0`, `CarRenderRecord::CopyFrom`: four ordered matrix-copy calls at
  offsets 0, 0x40, 0x80 and 0xe0, with the intervening vector/words and final ten
  16-byte groups copied in the observed order. Lanes 1/2 in those groups pass
  through x87 loads/stores; other lanes retain their bit patterns. Overlapping
  records retain sequential group-copy behavior, rather than a whole-record
  snapshot. The return is the destination record.
- `0x43d010`, `CarRenderNormalize4`: the length is sqrt(((X² + Y²) + Z²) + W²),
  retained in x87 precision. Only output XYZ is written; output W is untouched.
  A zero length produces 1/0/0. Original source Y/Z are captured before storing X.
  This is distinct from the existing three-component normalizer `0x43cf10`.
- `0x5bb190`, `CarRenderDirectionMatrix`: call D3DX transpose through
  `0x63d12c`, then write zero to float indices 3, 7, 11, 15, 12, 13 and 14 in that
  order and return the output. The native ABI bridge now calls the recovered
  scalar body; see [transpose evidence](#scalar-d3dx-transpose-and-native-camera-bridge-2026-10-28).

Evidence comes from live Ghidra decompilation and original disassembly; the D3DX
operation was confirmed against its default scalar body `0x63d132` (dispatch table
entry at `0x804028`). No Ghidra project mutation was made. The new
`car-render-record-math.json` pins all three routines, matrix copy, scalar transpose
and the complete executable.

With `UG2_EXE` set, run `tools/verify_pc_car_render_record_math.py --mutation-checks`
using `build/venv/bin/python`, then repeat with `--gcc` (MSVC/wibo jobs sequential).
Each compiler passes 504 copy, 3,144 normalization and 512 direction-matrix runs,
zero failures; eight of eight mutations are rejected. Both x87 precision settings
are used. Compared state includes all fixture bytes, specified returns,
callee-saved registers, stack cleanup, x87 stack balance/control word and ordered
matrix-copy/transpose calls. Copy fixtures contain arbitrary binary32 payloads,
including NaNs, with bounded forward/backward aliases; normalization inputs are
finite, including zero, signed zero, very small/large magnitudes and overlapping
XYZ output. Direction matrices use finite inputs with separate or identical
operands. Byte identity and exhaustive coverage are not claimed. The verifier
selects actual scalar D3DX code on both sides; CPU dispatch is outside coverage.
The i386 demo target also builds successfully.

These helpers and the producer routines `0x48e6c0`/`0x48a6a0` are compiled into
the recovered library. Native preparation is now active with the caller's actual
buffer shapes: a complete 0x1c0-byte source record and four-word camera position.
The former `scratchB`/`scratchA` matrix placeholders are removed. The wheel verifier
now compares source/position contents; the native record verifier executes the
actual preparation chain and compares every resulting record byte. See
[caller inputs and native activation](#wheel-caller-inputs-and-native-record-activation-2026-10-26).

### Render-record coefficient transform (2026-10-26)

`CarRenderRecordTransform.cpp` recovers the operation sequence of `0x489ba0`,
called by `0x48e6c0`. It snapshots the matrix's nine basis components, then processes
four independent lanes of ten groups at record offsets 0..0x9c. Constant lanes
are unchanged; the other nine groups are evaluated in the original order. The
six Y-product float spills, three Z-product spills and the additional stored
E×W1 value at `0x489db3` are explicit. The latter is retained unrounded for the
XZ output and reloaded as binary32 for YZ. Addition order and doubled cross terms
follow disassembly, rather than an algebraically simplified polynomial.

Evidence: live Ghidra decompilation plus original instructions `0x489ba0`..`0x489ec5`;
`car-render-coefficients.json` pins that 806-byte span and the executable. The
recovered method and its producer callers are compiled into the native library;
their native activation is covered below. No claim of byte identity is made.

Native numeric verification (with `UG2_EXE` pointing at PC v1.2):

```
build/venv/bin/python tools/verify_pc_car_render_coefficients.py --gcc --cases 32768 --mutation-checks
```

Native i386 GCC `-O1 -mfpmath=387 -fno-fast-math -ffp-contract=off` passes 65,536
comparisons, zero failures, seven of seven mutations rejected. Both x87 precision
settings are tested; **all fixture bytes must match exactly**. Inputs include
finite coefficients with mixed magnitudes, identity/zero/signed-unit/general
matrices and matrix pointers overlapping different parts of the record. The
verifier also checks the ABI, preserved registers, control word and x87 stack
balance. It does not cover NaN/infinity inputs, exhaustive floats or the producer
caller. The i386 demo target builds successfully.

Legacy precision remains an explicit gap. The default MSVC 7.1 `/Ox /Ob0 /EHsc`
strict run fails; measurement is available without turning those failures into
acceptance:

```
build/venv/bin/python tools/verify_pc_car_render_coefficients.py --measure --cases 32768
```

This measurement **exits 1** and records all differences. Over 65,536 runs, the
53-bit x87 setting `0x027f` has zero mismatched cases; the 64-bit setting `0x037f`
has 71 mismatched cases / 78 words. The first is case 693, matrix alias +0x40,
record +0x54 (original 1.9028870146264865e-10, rebuilt 1.9028867370707303e-10).
The maximum absolute error is 0.03125 in the mixed-scale fixture. ULP counts can
be large near zero: the worst is original 0 versus rebuilt 9.020562075079397e-17
at record +0x60. Generated-code inspection identifies binary64 accumulator spills
in the legacy rebuild where the original keeps extended values in registers;
`/Op` did not remove the first difference. Native GCC's `long double` retains
extended spill precision and passes the same inputs exactly. These findings do
not establish a legacy acceptance limit or complete the producer seam. Further
legacy precision work remains; no registers were forced or assembly inserted.

### Render-record producer chain (2026-10-26)

`CarRenderRecordPrepare.cpp` recovers the original PC v1.2 preparation chain:

- `0x48e6c0`, `CarRenderPrepareWorldRecord`: zero the temporary coefficient cells,
  copy the source record, construct the direction matrix from the world frame,
  snapshot the source basis and write its transformed components into matrix40.
  The component arrangement is preserved; it is not replaced by a conventional
  point transform. Products accumulate Z, Y, X, then translation. Source
  coefficients at +0x120 are read **after** the basis stores, transformed via
  `0x489ba0`, and copied back. This order matters for overlapping records/world
  matrices.
- `0x48a6a0`, `CarRenderPrepareViewRecord`: always construct the direction matrix.
  With a position, preserve extended relative X and the binary32 relative Y/Z
  spills, project the position, and clamp its length. The lower bound compares
  the unrounded length; the upper bound compares the stored float. Gate 3 uses
  [2.78, 8]; other gates use [2.78, 2.78], with measured binary32 constants.
  Set vector W to zero before normalization, then write the scaled XYZ and W=1
  to valueC0. With both the third matrix input and position, compute the 16 matrixE0 cells in
  their individual observed product orders, copy the matrix, zero indices
  3/7/11, copy valueC0's XYZ bit patterns to indices 12..14 and zero index 15.
  Return 1 even when both optional inputs are null.
- `0x48f660`, `CarRenderPrepareRecord`: call world preparation, then view
  preparation, unconditionally and in that order. Argument 5 is unused; return 1.

Evidence: live Ghidra decompilation and original instruction listings. The
`car-render-prepare.json` and `car-render-view-record.json` manifests pin the
producer/wrapper spans, helper spans, executable and clamp constants. No project
mutation or assembly replacement was used.

With `UG2_EXE` set, native whole-chain checks are:

```
build/venv/bin/python tools/verify_pc_car_render_prepare.py --gcc --chain --cases 16384 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_view_record.py --gcc --chain --cases 16384 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_view_record.py --gcc --chain --kind wrapper --cases 16384 --mutation-checks
```

All three pass 32,768 comparisons each, zero failures. Mutation rejection is 6/6
for world preparation, 8/8 for view preparation and 11/11 for the wrapper. Native
chain mode links the recovered record copy, direction-matrix wrapper, normalizer,
coefficient transform and producers as actual i386 GCC code; original matrix-copy
and D3DX scalar transpose bodies execute as shared/library callees. ABI adapters
are used only when an isolated GCC caller invokes an original thiscall method.

MSVC isolated-caller checks also pass: world preparation 8,192 runs (6/6 mutants),
view preparation 32,768 runs (8/8), and wrapper 32,768 runs (11/11). Run the same
commands without `--gcc --chain` (world uses `--cases 4096`). These execute
original helper callees, while wrapper mode uses both recovered producers; this
separates caller verification from the documented legacy coefficient precision
gap. It does **not** establish full-chain MSVC parity at 64-bit x87 precision.

Both x87 precision settings are used. Every fixture byte, root return where
specified, stack cleanup, preserved registers, control word, x87 stack balance
and ordered callee inputs are checked. Cases exercise record/world/third-matrix/
position aliases, optional nulls, clamp thresholds and neighboring floats. A
cancellation fixture distinguishes `huge + 2 - huge + 1` from a reordered product
sum at 53-bit precision. Each view/wrapper run includes 24,612 position paths,
19,786 matrix paths and 13,128 gate-3 paths. Coverage is bounded and finite;
byte identity, NaN/infinity behavior and exhaustive inputs are not established.
The i386 demo target builds successfully. The native harness explicitly selects
C++11 to match CMake. Clamp constants now reside in binary32 objects so x87
excess-precision rules cannot promote their decimal literals early. The 32,768-run
native wrapper check and all 11 mutations pass with these settings.

The original car draw initializes the frame +0xb20
record with `0x48ab10` at `0x620fd3`, then fills it through lighting/source-record
producer `0x48ed60` at `0x620ff3`. The clear routine, lighting foundations and
builder and light selection are now recovered (see below); primary list population
is wired through recovered loaders. The caller fragments and normal native gate-0
activation are now recovered below. Additional-light producers remain open. The
initial position at frame +0x790 comes from view frame +0x40..+0x48 at
`0x620f04`..`0x620f2c`; the stack-depth and neighboring-range audit below confirms
its lifetime through the body draws. The full 66,053-byte car draw timed out in
Ghidra decompilation; the caller evidence uses original instructions. The third
matrix input `0x871b50` is the separate car-target reflection basis, recovered
below. The API keeps the generic name `matrixArg3`; it is not a projection matrix.
`NativeCarWheels.cpp` now executes preparation, while GL draw consumption remains open.

### Lighting-record foundations (2026-10-26)

`CarRenderLighting.cpp` recovers five dependencies of the source-record builder:

- `0x48ab10`, `CarRenderClearRecord`: zero all 112 words / 0x1c0 bytes. Bytes
  outside the record remain untouched.
- `0x48a310`, `CarRenderLightVector`: multiply azimuth and polar degrees by the
  binary32 value 65536, truncate each to a signed integer, divide by 360 with
  signed truncation, then pass the low 16 bits to `WheelSineCosine` (`0x43c390`).
  The ordered calls produce X = sin(polar) × cos(azimuth) × radius,
  Y = sin(polar) × sin(azimuth) × radius, Z = cos(polar) × radius. Only XYZ is
  written. The second angle is polar, rather than latitude.
- `0x4895e0`, `CarRenderRecord::AccumulateLight`: snapshot all four color lanes,
  store the common Z²×3−1 and X²−Y² terms as floats, then update ten coefficient
  groups per lane. Eight intermediate products per lane have explicit float
  spills; the remaining intermediates retain x87 precision. The original
  constants and product/addition order are preserved, including the shared
  signed contribution added to group 1 and subtracted from group 2.
- `0x489af0`, `CarRenderRecord::AddLightPair`: snapshot direction, accumulate a
  positive contribution at weight 1, **read color again after that call**, store
  color×−0.05 as floats, and accumulate at the negated direction. Reading color
  after the first call matters when color aliases the coefficient record.
- `0x489ed0`, `CarRenderRecord::NormalizeLightCoefficients`: independently gate
  normalization of lanes 0..2 and lane 3. The first lane's squared-sum group order
  is 2,3,4,5,6,7,8,0,9,1; the other sums use 0..9. First-three squared sums are
  stored as floats before selecting the maximum. Their square root and scale
  stay extended; the fourth lane instead stores its square root as a float
  before division. Zero norms skip writes. Flags and targets are not clamped.

Evidence comes from live Ghidra decompilation and original instructions.
`car-render-lighting.json` pins the executable, all five spans and the read-only
constants. Constants used in extended expressions are explicit binary32 objects:
GCC C++11 x87 excess-precision evaluation otherwise retains extra decimal-literal
precision, unlike the original memory loads. This also fixed the preparation
clamp constants. No floating-point tolerance, assembly insertion or register
forcing is used. CMake now explicitly disables fast math and contraction for
recovered code; the native verifier uses C++11 and those same settings.

With `UG2_EXE` set, verification commands are:

```
cmake --build build/native --target ug2demo -j 6
build/venv/bin/python tools/verify_pc_car_render_lighting.py --gcc --native-trig --cases 8192 --exhaustive-angles --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_lighting.py --cases 8192 --mutation-checks
```

Native results: 16,384 comparisons for each of clear, accumulate, pair and
normalize; 147,456 vector comparisons; **212,992 total, zero failures**. All ten
mutations are rejected. The 65,536 extra angle contexts visit every quantized
azimuth and polar angle once (polar uses a fixed offset), at both x87 precision
settings, with signed unit radius. They are not exhaustive combinations of
angles/radius or arbitrary float inputs. The native run links the actual
CMake-built `WheelSineCosine` object from `PhysicsMoverController.cpp`, with only
its reachable sections retained. Its code bytes were unchanged by the explicit
no-fast-math/no-contraction build flags; the report records the actual object
fingerprint used.

Legacy MSVC 7.1 `/Ox /Ob0 /EHsc`: 16,384 comparisons per helper, **81,920 total,
zero failures**, ten of ten mutations rejected. Original trig and conversion
bodies execute for this run. Both sides run the pair and accumulator bodies,
rather than stubbing their arithmetic. Both x87 precision settings are covered.
Compared: every fixture byte, ordered trig/accumulation inputs, stack cleanup,
callee-saved registers, control word and empty x87 stack. Fixtures include zero,
signed zero, mixed magnitudes, signed targets/flags and color/direction/record
aliases. Vector inputs stay within the original signed conversion range; NaN,
infinity, out-of-range conversions and exhaustive float inputs are not covered.
Byte identity is not claimed. The separate legacy precision gap in `0x489ba0`
remains open; these helper results do not change its acceptance status.

Platform check: the i386 demo target builds and the SKYLINE demo completes
60 frames with `--no-audio --orbit 90`. This is a baseline smoke, not activation
of the new lighting builder. Caller recovery and native source/preparation
activation are documented in the later activation section.

Further original-binary leads: callers `0x631250` and `0x6316d0` push the default
context `0x7f3f00` into `0x620ae0` at `0x631696`/`0x631879`; the image holds three
populated source entries there. Source-record builder `0x48ed60` is recovered
below; light selection `0x48e3d0` is recovered in the following selector section.
Its primary native list population is recovered below; additional-light producers
remain open. The third matrix at `0x871b50` is produced
in `0x5c3450`: the `0x5c3524` call uses recovered `CameraLookAt` (`0x5ba9a0`),
then indices 3/7/11/12/13/14 are zeroed and index 15 is set to 1 before the copy at
`0x5c356b`. Its up input is the image vector 0/0/1 at `0x800b70`. The later
activation section verifies that fragment, the Camera prefix and position-buffer
lifetime, then wires them into the native path.

### Lighting source-record builder (2026-10-26)

`CarRenderLightBuild.cpp` recovers original `0x48ed60`, with prefixes and observed
offset checks in `CarRenderLightBuild.h`. A null output returns -1 before reading
any other argument. Otherwise the builder:

- Uses the supplied fifth matrix, overridden by `view+0x40` when both pointers
  exist, and calls recovered direction-matrix helper `0x5bb190`.
- Builds three profile directions with azimuth `angle04+180` and polar
  `90-angle08`, explicitly stored as floats before calling `0x48a310`. The
  profile flag is a raw dword test, including the `0x80000000` bit pattern.
  Enabled entries rotate the reordered vector (-Y,-Z,X) through the basis.
  Colors use intensity × channel × 255 in that order.
- Calls sector-light selector `0x48e3d0`. If its signed count plus three exceeds
  19, the count becomes **15**, preserving the original discontinuity rather
  than filling all 19 entries. The selector's sector pass limits dynamic lights
  to ten, but its additional-list pass can reach 24; negative fixture counts
  separately probe the builder's own control flow.
- Produces relative directions and low-byte-first RGB colors for selected lights.
  Kind 3 follows the original sine threshold, in-place offset normalization and
  cone-weight calculation. Its Z+Y+X dot product and cone scale remain extended;
  the threshold is stored as a float. Other kinds preserve the distinct
  channel × 0.75 × weight order. There is no invented cone/weight clamp.
- Optionally transforms directions through the fourth matrix, normalizes all
  directions, scales colors by the image's binary32 1/255 value, and computes
  the fourth lane as (R×0.29 + B×0.17) + G×0.54 with the original constant widths.
- Writes only the observed nine cells each of matrix00/matrix40 and the 40
  coefficient words at +0x120. The coefficient prefix starts at zero; ordered
  recovered pair additions and independent lane normalization fill it. Other
  output cells retain their prior contents.

Live Ghidra disassembly confirms the arithmetic order and calling conventions;
`car-render-light-build.json` pins the full executable, 2,207-byte builder span
and constant bytes. With `UG2_EXE` set, run:

```
build/venv/bin/python tools/verify_pc_car_render_light_build.py --gcc --chain --cases 4096 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_light_build.py --chain --cases 4096 --mutation-checks
```

Each compiler completes 8,192 exact comparisons at the two x87 precision
settings; all 11 mutations are rejected. The linked chain includes recovered
light-vector, pair, accumulator and coefficient-normalization bodies. Per
precision setting, 3,963 non-null cases cover 5,945 direction-matrix calls,
11,889 vector calls, 51,828 three-component normalization calls, 41,456 pair
calls and 10,372 sine calls. Checks compare every fixture byte, ordered helper
arguments, return value, stack cleanup, callee-saved registers, control word and
empty x87 stack. Fixtures cover profile flags, both matrix paths, point/cone
lights, zero vectors, signed weights/normalization targets, count boundaries,
null output with invalid remaining pointers, and output aliases with inputs.
Byte identity and exhaustive/nonfinite inputs are not established. The separate
legacy coefficient-transform precision gap at `0x489ba0` remains open.

**Boundary of the commands above:** the sector-light selector uses a controlled fixture on both sides;
this verifies the builder's use of its results, not sector/tree traversal or
selection. Direction transpose, sine, three-component normalization and their
trig dependencies execute original machine code in this verifier. Native i386
and legacy MSVC caller results are therefore distinct from complete native
integration. The builder is now active in the normal native wheel path as
verified below. Selection `0x48e3d0` and its tree dependencies are now
recovered and checked without that fixture substitution below. Native primary
list/tree population is now wired through the recovered loader. Additional-light
producers and extra-record draw consumption remain open; the view-frame/position path is active below.
The i386 demo builds and completes the 60-frame SKYLINE no-audio orbit smoke;
that is a baseline platform check, not a lighting-activation claim.

### Sector-light selection and trees (2026-10-26)

`CarRenderLightSelect.cpp` and `CarRenderLightTree.cpp` recover all six routines
behind the builder's selection call:

- `0x502fd0`, list end: return the receiver unchanged. The selector calls it for
  both intrusive-list heads; it is kept across a translation-unit boundary.
- `0x5746e0`, node bounds: strict X/Y/Z extent checks. A coordinate exactly on a
  face is rejected; differences stay extended through absolute value/comparison.
- `0x574730`, tree descent: ordered recursive search through signed 16-bit child
  indices at stride 0x30. A nonpositive child count identifies a leaf; the first
  containing leaf wins, with backtracking when a containing branch has no result.
- `0x5747e0`, tree query: reject a null root or failed root bounds, return a root
  leaf directly, otherwise descend.
- `0x4893a0`, light admission: require byte +7 equal to 1; compare float-spilled
  (Z²+Y²)+X² against float-spilled outer-radius² using strict `<`. Inner-radius²
  stays extended; attenuation is stored as a float. Kind 0 / mode 2 accumulates
  up to ten weighted ambient vectors, advancing only for positive weight. Other
  kind-0 modes retain the strictly nearest ambient value. Kinds 2/3 append light
  pointers and weights while the count is below 24. Remaining kinds are ignored.
- `0x48e3d0`, selection: initialize only the first selected pointer/weight and four
  ambient cells; snapshot position after those stores; walk sector list head
  `0x830d14` and query each tree. Leaf indices select 0x60-byte light records.
  The sector pass stops at ten dynamic lights or a full ambient array. The
  additional list at `0x835850` still visits all enabled nested lists, ending on
  null or their own sentinel. Admission's 24-light limit still applies there.

The final ambient blend preserves the original unusual scale: remaining weight
is float(1−sum), and the contribution scale is **sum** when sum ≤ 1. Above 1,
remaining becomes zero and the scale is float(1/sum). Each stored ambient vector
already contains its individual attenuation. Four ordered sums stay extended;
the original B/fourth spill points are explicit. The fallback starts at 0/0/0/1.
No replacement normalization or extra clamp is introduced. Original instructions
also show **no write to the additional-list global**; a decompiler-rendered
assignment there was an analysis artifact.

`CarRenderLightSelect.h` checks the tree-node stride, sector prefixes, 0x60-byte
light records and shared 0xcc-byte stack-state offsets. Unknown cells retain
offset-based names. `car-render-light-select.json` pins the executable and all
six contiguous code spans. With `UG2_EXE` set:

```
build/venv/bin/python tools/verify_pc_car_render_light_select.py --gcc --cases 4096 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_light_select.py --cases 4096 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_light_build.py --gcc --chain --selector-chain --cases 4096
build/venv/bin/python tools/verify_pc_car_render_light_build.py --chain --selector-chain --cases 4096
```

Native GCC C++11 and legacy MSVC each pass **8,192 comparisons per routine,
49,152 total**, with all 12 mutations rejected. Both x87 precision settings run.
The selector checks include 219,904 ordered admission calls, 16,360 tree queries
and 21,050 recursive descent calls per compiler. Full fixture bytes, unchanged
list heads, return values, ordered helper inputs, initialized stack-state fields,
callee-saved registers, cleanup, control word and empty x87 stack match. Fixtures
include empty/multiple sectors, null roots, nested/overlapping branches, signed
indices, exact/adjacent bounds and radius probes, inactive/unsupported lights,
fractional inner radii, nearest/blended ambient paths, full arrays, both dynamic
limits, and null/sentinel list ends. These are finite constructed valid records,
not exhaustive trees or a captured retail traversal; nonfinite inputs are not
claimed covered. Byte identity is not established.

The builder's new `--selector-chain` mode executes the complete recovered
selector/tree/admission bodies and lighting foundations, with **8,192 exact
builder comparisons per compiler**, without replacing selection results. Per
precision setting its 3,963 non-null cases cover 31,823 vector normalizations,
25,186 pair additions and 6,637 sine calls. Direction transpose, sine,
three-component normalization and trig still execute original machine code in
this harness. The separate legacy `0x489ba0` precision gap is unchanged.

The i386 demo builds and completes the 60-frame SKYLINE no-audio orbit smoke.
The next section covers native primary light-list/tree population. Source-record/
additional-light producers and extra-record rendering remain open; the caller inputs and normal native activation are covered below. Original xrefs identify append routine
`0x48e380` for the additional list and primary-list data use at `0x48e248`;
startup references `0x77cbd0`/`0x77cc80` are initialization leads. Those producers
are recovered in the loading checkpoint below.

### Retail lighting chunks and shared registry (2026-10-26)

`CarRenderLightLoad.cpp` recovers complete chunk callbacks `0x48e070` and
`0x48e270`, plus additional-sector append/remove `0x48e380`/`0x48e3b0`.
The original registration at `0x77cc20`/`0x77cc40`/`0x77cc60` names three kinds:

- `0x135200`: multiply the two observed float fields by image value 1, even for
  an unsupported pack kind; kind 3 allocates its state through `bOMalloc`, applies
  the observed hash-based defaults, appends link +4 to list `0x8358c0`, and may
  update the default-pack pointer. Unload removes the link and restores the
  original fallback pointer for hash `0x2c420d64`.
- `0x80135100`: fix aligned header `0x135101`, preserve/set its initialized byte,
  require kind 3, initialize its nested sentinel and append `count50` records from
  `0x135102` at stride 0x30. Append the header to list `0x830ae8` and register it in
  the shared section record's +8 cell. Unload clears that cell and releases a
  registry reference.
- `0x80135000`: require header `0x135001` version 4, fix tree `0x135002` to nodes
  at tree+0x10, and point to the 0x60-byte lights in `0x135003`. The first fixup
  retains the shared empty call `0x4022c0`. Append to sector list `0x830d14` and
  store the header in registry record +0xc. Unload removes it, clears +0xc and
  releases its reference. Unknown child kinds are skipped as in the original.

Child payload alignment is `(child+0x17)&~15`, including the chunk header's
eight bytes. The implementation uses a pointer-width address type. No original
pointer is encoded by truncating a host pointer. The additional append/remove
routines preserve null returns and link-store order; the dynamic producer that
would call append in the native car session remains open.

`CarRenderLightLoadHelpers.cpp` recovers list constructor `0x598750`, pooled-state
setup `0x5bb590`, all 23 recognized hash paths in `0x5bb5b0`, and the shared empty
body. List initialization `0x57bcd0` sets both links to the receiver. Unknown hashes
retain both fields. The pool body is the already recovered bWare implementation;
the verifier executes its original bytes. Native track loading does not load
`0x135200` packs or initialize their separate pool/ownership lifecycle.

The registry is **the existing track topology at `0x883e70`**, not a separate
lighting registry. `TrackSectionResources.cpp` recovers complete constructor
`0x5e6360`, acquire `0x5db680` and release `0x5db6e0`. The checked object is 0x643c
bytes: nine initial sentinels; 2,800 entry pointers at +0x50; count at +0x2c10;
512 records of 0x1c bytes at +0x2c14; free-list head +0x6414; eight final words.
Construction leaves record payloads untouched. First acquire unlinks a free
record, clears exactly seven words and increments its reference count; subsequent
acquires share it. Final release appends it without clearing payloads. The original
assumes a valid index and available free record; no replacement allocator or
extra capacity behavior was added. `RunWorldStaticInitializers` now calls this
constructor rather than initializing only the topology prefix.

Evidence combines live Ghidra decompilation of the dependencies, callback/global
xrefs and manual decompilation of the original callback instructions (Ghidra had
no function objects at the two callback addresses). The executable and 12 spans
are pinned in `car-render-light-load.json`; the project was not modified.
With `UG2_EXE` set, run the MSVC jobs sequentially:

```
build/venv/bin/python tools/verify_pc_car_render_light_load.py --gcc --cases 2048 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_light_load.py --cases 2048 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_light_load.py --gcc --cases 1 --retail-track "$UG2_GAME/TRACKS/STREAML4RA.BUN"
build/venv/bin/python tools/verify_pc_car_render_light_load.py --cases 1 --retail-track "$UG2_GAME/TRACKS/STREAML4RA.BUN"
```

Each compiler passes **112,640 exact synthetic lifecycle comparisons**, with
all 11 mutations rejected. Both x87 precision settings run. Coverage includes
all 512 free records, zero/last valid IDs, repeated/final releases, untouched
constructor payloads, shared references, all default hashes, child alignment
phases, initialized flags, valid/unsupported kinds, unknown children, empty
records, null append/remove, and ordered helper calls. Every fixture/registry
byte, list head, default pointer, return, cleanup, preserved register, control
word and empty x87 stack must match. These are finite valid fixtures; byte
identity, arbitrary malformed lengths and exhausted-pool behavior are not
claimed. The primary unloader's non-advancing loop on a mismatched first child
is preserved, with no general invalid-input safety claim.

Retail verification loads all **249 sector-light and 250 animation chunks** from
`STREAML4RA.BUN`, builds **250 shared registry entries**, compares **520 selection
queries**, and unloads every chunk in reverse order, returning the registry count
to zero. Per compiler: 998 load comparisons, 998 unload comparisons and 520
selection comparisons at both precision settings, plus constructor/list checks;
zero failures. The report records the input SHA-256. Queries use actual light
positions and deterministic sampled coordinates; this is not a captured original
streaming schedule.

Native `LoadNativeTrack` now sends those same chunk kinds to the recovered handler
using the existing aligned copies retained for the session. It calls the original
list constructors in the observed initializer order. Native all-section loading
still replaces original asynchronous visibility/streaming and does not unload
chunks during this session. The `ug2demo` and `ug2sim` targets build. The 60-frame
SKYLINE no-audio orbit smoke passes; `UG2_DEBUG_LIGHTS=1` reports 249 sectors,
250 animations, 250 registry entries, two selected lights at spawn and ambient
0/0/0/1. Track-marker regression verification also passes with the complete
registry object linked.

This activates retail light data and selection. Wheel record production is now
active as described below; `DrawSolid` still ignores the extra record. Additional car-light production, the
separate pack lifecycle and the legacy coefficient-transform precision gap
remain open.

### Wheel caller inputs and native record activation (2026-10-26)

`CarRenderInputs.cpp` recovers four caller fragments, without claiming the full
containing functions:

- `0x61ac46`..`0x61ac6c`: build a reflection target from the selected car's position
  plus the image vector at `0x803e50` (0/0/0.95). Original caller `0x61ab50` reads
  the view Camera's position +0x40 and the selected car's position +0x50. This is
  **not** the camera's target +0x60. The native single-player session supplies
  its player's exported origin; full original car-selection/frame scheduling is
  still a platform seam.
- `0x5c3524`..`0x5c3570`: call recovered `CameraLookAt` with up 0/0/1 at `0x800b70`,
  zero matrix indices 3/7/11/12/13/14, set 15 to one, then copy to `0x871b50`.
  The extracted helper makes the destination explicit for verification.
- `0x620f04`..`0x620f33` and the W store at `0x620f3a`: sequentially copy the view
  Camera's position words to frame +0x790 and store W=1. Raw word copies preserve
  bit patterns and the original bounded-overlap behavior.
- `0x620fcb`..`0x620ff8`: clear the full source record at frame +0xb20, then call
  `0x48ed60` with the caller's profile/car position/view, null fourth argument and
  the reflection basis as fifth argument. Existing callers pass the stock profile
  at `0x7f3f00`. The builder still applies its original view-frame override.

Base-mover constructor `0x44ab90` retrieves the Camera through eView `+0x40`.
The separately recovered `0x5c26e0` instead stores an eView pointer at draw-context
`+0x40`; those two records must not be conflated.
`CameraState` already has the finalized matrix at +0, position at +0x40 and depth
axis at +0x50. `CarRenderFrame` now exposes that matrix with all offsets retained.
Native drive views copy the finalized prefix, including camera shake/debug state,
instead of rebuilding an axis from target−eye. The native orbit inspection camera
uses recovered LookAt to supply its own prefix. The later view-producer checkpoint
below replaces the focal-length scaffold with the original calculation.

`probe_pc_car_draw_inputs.py` checks **6,721 reachable instruction states**, with
no conflicting ESP depths and depth zero on entering `0x62700e`. It pins the
prefix, switch tables and 80 callee spans in `car-render-inputs.json`, validates
return cleanup including tail jumps, and infers four-byte cleanup for the single
virtual call from consistent CFG merges (zero/eight-byte alternatives conflict).
The apparently overlapping stores at `0x6242c3`..`0x624332` occur with ESP−0x20:
they write before +0x790. The `0x625140` store has ESP−8 and writes +0x78c.

This is a stack-depth/direct-reference analysis, not a general alias proof.
The adjacent ranges were audited separately: the +0x750 matrix ends at +0x790;
the +0x7a0 matrix begins after the four-word position; getters at `0x48b7f0`,
`0x6151d0`, `0x615200` write four words ending before or at +0xb20; `0x401220`
writes three. The indexed +0xa40/+0xa80 arrays advance by 0x10 for four iterations
(`0x62244d`..`0x622555`) and end before the source. Only the identified clear,
builder and preparation calls receive the source address. Preparation's source
is distinct from its pool output; its separately verified read/copy semantics
retain that source. The full wheel checks also compare its contents at each call.

Native `CarWheelDrawInputs` now receives `renderPosition` and `sourceRecord`
explicitly. The two unused local matrix placeholders and integer-cast global
address are gone. `NativeRunCarWheelBlock` captures position and builds the source
between the pixel early-out and LOD selection, matching their original relative
order. Record scratch storage is typed as four `CarRenderRecord`s per block.
`UG2WheelVisible` executes recovered `CarRenderPrepareRecord`, so all per-wheel
record side effects now occur. Native gate 0 remains the supported caller path;
the gate-6 remove/reappend of the car's additional-light record, other views and
the full containing draw/manager routines are not recovered here.

The native camera path exposed an implicit precision dependency in the existing
`WheelNormalize` recovery (`0x43cf10`). Its original FSQRT and reciprocal have no
binary32 spills. The code now states that extended arithmetic explicitly and
uses native `__builtin_sqrtl`, preserving the original snapshots and zero fallback.
MSVC regression: 2,048 cases per six wheel helpers, **12,288 exact comparisons**,
including partial aliases and both x87 settings. Byte status is reported separately;
normalization is not byte-identical. The verifier resolves new binary64 zero/one
loads to checked eight-byte image constants, rather than four-byte float cells.

With `UG2_EXE` set, run MSVC jobs sequentially:

```
build/venv/bin/python tools/probe_pc_car_draw_inputs.py
build/venv/bin/python tools/verify_pc_car_render_inputs.py --gcc --chain --cases 2048 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_inputs.py --chain --cases 2048 --mutation-checks
build/venv/bin/python tools/verify_pc_native_wheel_records.py --cases 2048
build/venv/bin/python tools/verify_pc_wheel_draw.py --cases 2048 --layout-cases 512 --mutation-checks
build/venv/bin/python tools/verify_pc_wheel_math.py
```

Each compiler passes **4,096 comparisons per caller fragment, 16,384 total**,
with all seven mutations rejected. The linked chain includes recovered camera
math/normalization, lighting/source builder, admission and tree traversal. Original
matrix copy and D3DX transpose remain leaf/library boundaries. These are finite
fixtures, including source/position overlaps and both x87 precision settings;
byte identity and full containing-function coverage are not claimed.

The native block verifier compiles the actual `UG2WheelVisible` definition and
recovered preparation bodies. It executes the original preparation on the other
side: **2,048 blocks, 6,739 prepared records, 29,030 ordered events**, zero failures
(1,001 cases at 53-bit precision, 1,047 at 64-bit). Every result byte in the allocated
record pools matches, along with matrices and ordered input snapshots. ABI-only
adaptation handles GCC member callbacks in the analysis harness. Renderer, slots,
facing, secondary draw, allocators and the prior D3DX-multiply fixture retain their
existing boundaries. The legacy block check still passes 512 layout/2,048 block
cases and now rejects **14** mutations, including wrong position/source buffers.
Its visibility boundary remains a caller fixture; the native test executes preparation.

`ug2demo`/`ug2sim` build. SKYLINE completes the 60-frame orbit and 120-frame scripted
drive smokes with no audio. `UG2_DEBUG_CAR_RECORDS=1` reports source/first-output
checksums and output C0: orbit 0.676807225/−2.48764729/1.04016507/1; drive
2.69134164/0.338087112/0.608914614/1 in these runs. This witnesses populated runtime
records; it is not a visual-lighting equivalence claim. **GL still ignores the
record.** Draw-state/consumption, additional-light producers and the separate legacy
`0x489ba0` precision gap remain open, along with the other inventory wheel seams.

### Solid dispatch, texture swaps and primitive queue (2026-10-26)

`CarRenderSolid.cpp` recovers six PC v1.2 routines: the view's solid dispatcher
`0x5c5930`, texture override application/restoration `0x4905a0`/`0x488e40`, primitive
queue `0x5ceb30`, default-light-pack getter `0x48ac50` and mode check `0x60fdf0`.
The function and jump-table fingerprints are in
`config/SPEED2_EXE_1_2/car-render-solid.json`. These routines were read through
the live Ghidra project and confirmed against the original instructions and image.
Names and decompiler types alone are not evidence.

The dispatcher does not draw immediately. It reads platform mesh metadata from
`solid+0xc -> table+0`, snapshots the original technique type, selects one of the
three nine-element maps at `0x7ff090`/`0x7ff0b4`/`0x7ff0d8`, and consumes the
one-shot map at platform `+0x38`. Flags, view mode, material byte `texture+0x56`,
debug state and the two special texture hashes select a technique for each
primitive. The pre-remap type still controls the debug branch. Bounds classification
is bypassed by draw flag 4 or primitive flag 1. Texture and light-pack indices are
individual unsigned bytes at primitive `+0x1c`/`+0x20`, with `0xff` selecting the
default light pack. The primitive stride is `0x3c`.

Texture application resolves sentinel `-1` values first, then swaps each matching
slot with the first eligible override. Texture count `table+0x19` is signed.
Restoration swaps the saved values back and clears their saved locations; duplicate
keys and repeated slot matches retain the original order, including its unusual
reuse behavior. Restoration still runs when a technique is disabled. For technique
5, a null extra record is replaced by `view+0x54` before the special-texture remap;
that record persists across subsequent primitives in the same dispatch.

`0x5ceb30` allocates `0x20`-byte texture groups and `0x2c`-byte commands through
the two pool globals `0x870918`/`0x87091c`. The texture state's cached group is
reused only while its technique matches. New groups append to one of the list
banks at `0x870d30`/`0x8712f0`, indexed by signed texture priority times 23 plus
technique type. Commands append at group `+0x18`; counts and index totals update
after insertion. The command retains the world, extra record, lighting pack,
primitive flags and final argument. No invented allocation-failure recovery is
added; verification covers successful pool allocation.

With `UG2_EXE` set, run the compiler jobs sequentially:

```
build/venv/bin/python tools/verify_pc_car_render_solid.py --gcc --cases 2048 --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_solid.py --cases 2048 --mutation-checks
```

Both native i386 GCC C++11 `-O1` and MSVC 7.1 `/Ox /Ob0` pass **12,288 comparisons
each**, with all **15 mutations rejected**. Per compiler, the dispatcher submits
**9,666 primitives**, and standalone queue sequences submit **24,576**. Fixtures
include all signed texture counts, unsigned index extremes, duplicate overrides,
initial/selected technique and material combinations, signed queue priorities,
shared texture-state caches, repeated appends and counter wrap. Whole fixture
bytes, touched global/list bytes, return values, calling conventions, preserved
registers and ordered boundary calls are checked. Bounds `0x5bc300`, texture lookup
`0x4901d0` and successful pool allocation `0x440e50` are explicit boundaries in
this standalone verifier; the bounds section below closes the first body gap.
This is finite behavioral evidence; byte matching and retail frame capture are
not established. Reports are under `build/analysis/car-render-solid/`.

The native `NativeDrawSolid` now contains a complete checked `CarRenderSolid`
prefix, with host solid pointers and GL handles stored after it. At this checkpoint
its hash was populated and table/override fields remained zero; the manager
activation below now supplies the table pointer. Wheel slots point to
this actual record. Previously GL handles occupied the original hash and other
prefix fields. The native boundary still draws the same display lists.
`CarRenderSolid.cpp` is excluded from the native library until the original table,
texture and technique setup can supply its inputs and the GPU consumer can drain
its queues. Native activation and visual lighting parity are not claimed.

The consumer is `0x5cf420`: its calls to `0x5bdde0` upload world/view transforms,
`0x5bd140` consumes the prepared extra record, and `0x5bd4a0` consumes the lighting
pack. Those bodies, effect/technique setup and retail solid fixups were open at
this checkpoint; the following sections recover them. Native effect support and
activation still separate the prepared records from GL consumption.
Native build, wheel-block/preparation regressions and SKYLINE smoke
checks pass separately from dispatcher equivalence: 2,048 native blocks/6,739
prepared records; 512 legacy layout/2,048 block cases with 14 rejected mutations;
record-lookup regressions with 10 rejected mutations; `ug2demo`/`ug2sim` build;
60-frame SKYLINE orbit and 120-frame scripted drive, no audio.

### Retail solid chunks and platform fixups (2026-10-26)

`CarRenderSolidLoad.cpp` and `CarRenderSolidPack.cpp` recover these PC v1.2 routines:

| Address | Recovered behavior |
| --- | --- |
| `0x493bb0` | Load an `0x80134010` solid chunk, resolve its slots, register it and update counters |
| `0x5cc280` | Load platform children, register the platform record and create/fill GPU buffers |
| `0x5c58a0` | Relocate primitive index offsets and select the platform technique |
| `0x5bc6b0` | Select the first matching technique rule |
| `0x4907a0` | Resolve texture hashes using the signed texture count |
| `0x488bc0` | Search sorted unsigned keys and return the first duplicate |
| `0x431780` | Convert a profile-list link to its enclosing record, including null |
| `0x40cb20` | Return the reference build's unconditional load permission |

The original addresses, function sizes and hashes are recorded in
`car-render-solid-load.json` and `car-render-solid-pack.json` under
`config/SPEED2_EXE_1_2/`. Ghidra decompilation was checked against the instructions,
including actual D3D arguments, the profile converter's fastcall ABI and the
primitive fixup's return word. No Ghidra project mutation was needed.

Platform headers and primitives align to 16 bytes; vertex payloads align to 128.
Primitive `+0x34` becomes `indexOffset*2 - offsetArgument + indexBase`, with original
32-bit wrap. The rule table at `0x800464` prioritizes mask 1/type 4, mask `0x40`/type
2, mask `0x80`/type 5, then zero-mask/type 0. Index upload uses usage 0, format
`0x65`, pool 1; vertex upload uses usage 8, format 0, pool 1. In load mode 4, only
type 5 gets a vertex upload. Zero vertex count bypasses it, while zero index bytes
still run primitive fixup. Uploads preserve the original forward word copies and
remaining byte copies. HRESULTs remain unchecked, as in the binary.

GPU object fields stay four-byte tokens. Typed create/lock/unlock adapters carry
all original arguments and keep host objects and callbacks outside original
records. The verification adapters supply CPU storage; they do not implement D3D
or a replacement game renderer. The original platform record prefix is checked
through `+0x40`, and the solid record prefix through `+0x90`. Existing observed
offsets remain asserted.

The outer loader fixes texture/profile arrays, auxiliary pointers and counts,
marker data, registration links and counters. Texture and lighting counts are
signed bytes; profile matches use the intrusive list at `0x8358c0`, with default
pack `0x7f399c` on a miss. Cross-pack matches propagate flags `0x100`/`0x200` in
the original order. Successful loads bind the first resource-index duplicate,
append to `0x835794` and increment the hash bucket; failed platform loads append
to `0x835858` and clear the matching resource entry. Synthetic failure fixtures
override the platform return explicitly: valid PC platform loading normally
returns 1. Total count, payload bytes and change flag update on either path.

With `UG2_EXE` set, run compiler jobs sequentially:

```
build/venv/bin/python tools/verify_pc_car_render_solid_load.py --gcc --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_solid_load.py --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_solid_pack.py --gcc --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_solid_pack.py --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
```

Both MSVC 7.1 `/Ox /Ob0` and native i386 GCC C++11 `-O1` pass **6,144 platform
comparisons** and **10,240 outer-loader/helper comparisons** each. Each verifier
rejects **14 mutations** per compiler. Synthetic coverage includes signed count
extremes, unsigned keys/duplicates, rule priority/default/fallthrough, null profile
conversion, index wrap, copy overlaps/remainders, mode gates, marker count
truncation, prior list entries, counter/bucket wrap and failed registration.
Complete fixture bytes, touched globals and list links, buffer bytes, return
values, stack cleanup, saved registers and ordered boundary calls are compared.

Each verifier also passes all **612 SKYLINE retail solids/platform chunks** per
compiler. `GEOMETRY.BIN` SHA-256:
`93cc878329d5783e61abb40cce41da6313c010105fb3c3643963df7e06904037`.
The complete outer chain performs 612 index and 612 vertex creates, with 1,224
lock/unlock pairs. File alignment residues and retail child bytes are retained.
Resource-index contexts and texture/cross-pack lookup returns are fixtures;
this is not a captured retail manager session or a visual parity claim. These
finite checks do not establish byte identity or malformed-data coverage. Reports
are in `build/analysis/car-render-solid-load/` and `car-render-solid-pack/`.

Native `LoadSolid` now reads primitive texture index `+0x1c` as a byte, matching
`MOVZX` at `0x5c5a8c`. All 2,061 SKYLINE primitive records have zero in the three
following bytes, so its current texture selection stays the same. Native
`ug2demo`/`ug2sim` build and SKYLINE's 60-frame orbit/120-frame scripted drive pass
with audio disabled; the existing source/prepared-record checksums stay the same.
The dispatcher regression still passes both compilers and its 15 mutations.

At this loader-recovery checkpoint, native activation remained open; the manager
and native car activation below now supply the real pack context and lifetimes.
`NativeAssets.cpp` still decodes arrays
for GL display lists. The manager section below connects real car pack contexts;
texture/technique objects, bounds classification and GPU consumption still gate
recovered dispatch. The confirmed outer-loader callers include `0x4940ae`
(inside `0x494060`) and `0x494140`; the latter reads its context from
caller argument 3 `+0xb8`. Texture callback `0x494ea0` handles a different set of
chunk IDs and remains open. Additional-light producers, secondary drawing and
the other inventory seams also remain open.

### Solid pack manager and native car records (2026-10-26)

`CarRenderSolidManager.cpp` recovers the remaining pack context and its lifecycle:

| Addresses | Behavior |
| --- | --- |
| `0x494060` / `0x4940e0` | Whole `0x80134000` pack load/unload callbacks |
| `0x48d910` / `0x493ea0` / `0x493790` | Header registration, index/stream pointers, header unlink and node cleanup |
| `0x48df90` / `0x48df40` | Loaded-solid and owning-pack lookup, including first resource duplicates and pack promotion |
| `0x490650` / `0x493650` | Resolve and detach solid references, including offline lists |
| `0x493f20` / `0x5ce7d0` | Solid/platform unload, bucket updates, queued-node removal and buffer retirement |
| `0x479130` | Append a deferred vertex-buffer token using the original bWare node pool |
| `0x5bb720` | Walk platform child headers without changing them |
| `0x43be20` | Profiling tick difference with wrapped subtraction, sign clamp and x87 arithmetic |

`0x494060` has no Ghidra function definition in the current project. Its original
instructions identify the callback bounds and calls: header `0x80134001` loads a
real context, each `0x80134010` solid uses it, and references finalize before a new
header and after the last solid. Unloading removes the header from the pack list
before unloading its solids. The callbacks and all dependencies are fingerprinted
in `config/SPEED2_EXE_1_2/car-render-solid-manager.json`; project mutation was not
needed. The real `0x134002` header is `0x90` bytes, with index `+0x74`, stream
`+0x78`, state words `+0x7c`/`+0x7e` and owned lists `+0x80`/`+0x88`.

Loaded lookup checks the byte hash bucket first. A bucket miss neither samples
the clock nor walks packs. Global lookup promotes the successful pack to the
front; a supplied context searches only that pack. Reference resolution snapshots
the next link before lookup and relinking. Detachment repeatedly removes the
first attached reference; offline detachment retains the original encoded byte
offset added to its solid pointer. Unloading removes only the first matching
queued platform node, queues the vertex buffer, releases the index buffer, and
clears the original fields. Profiling preserves original float stores and raw
x87 return precision; no numeric acceptance limit is relaxed.

With `UG2_EXE` set, run compiler jobs sequentially:

```
build/venv/bin/python tools/verify_pc_car_render_solid_manager.py --gcc --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_solid_manager.py --cases 2048 --retail "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --mutation-checks
```

MSVC 7.1 `/Ox /Ob0` and native i386 GCC C++11 `-O1` each pass **26,624 comparisons**
across 13 entry/lifecycle checks, with **17 mutations rejected**. Every entry has
1,024 cases at each x87 precision setting. Fixtures include real searches over
multiple packs, duplicate hashes across contexts, move-to-front behavior, online
and offline reference movement, nonempty cleanup lists, queued-node matches,
counter wrap and raw 80-bit elapsed-time results. Full fixture/global bytes,
ordered boundary calls, upload bytes, node contents and caller ABIs are checked.

The retail check executes both a complete SKYLINE root load and a load/unload
cycle, using the actual 612-entry resource index from `GEOMETRY.BIN` (SHA-256
`93cc878329d5783e61abb40cce41da6313c010105fb3c3643963df7e06904037`). Both compilers
match all record mutations and staged buffer bytes. Across those two runs there
are 1,224 vertex and 1,224 index creates; the unload queues 612 vertex tokens and
releases 612 index tokens. GPU APIs, texture lookup, node-pool allocation/free,
clock acquisition and the verified return-this leaf remain explicit boundaries.
Deferred vertex nodes are compared in their queued state. This finite evidence
does not establish code-byte identity, malformed-data coverage or D3D equivalence.
Reports are under `build/analysis/car-render-solid-manager/`.

`NativeSolidRecords` now retains an aligned, mutable copy of the car's retail
file and runs these callbacks. `NativeCarModel` binds its solid prefixes to the
actual resource-index entries, including the tyre/front-brake/rear-brake tables.
The platform adapter keeps CPU upload storage behind four-byte tokens, supplies
a monotonic profiling clock, and flushes retired vertex tokens at the record
owner's lifetime boundary. Reload/destruction run recovered unload before freeing
the file. Shared bWare pool allocation and release are used natively; host
containers/callbacks remain separate from original records. Car records are
active; track array decoding remains the existing native path.

The display-free integration probe uses the actual native adapter and recovered
code with player-owned assets:

```
cmake --build build/native --target ug2demo ug2sim ug2solidrecords_probe -j 6
UG2_DEBUG_SOLIDS=1 build/native/ug2solidrecords_probe "$UG2_EXE" "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN"
```

It passes three loads/reloads, **612 actual resource bindings per load**, and
checks empty pack/platform/solid/deferred lists, zero buckets/counts and zero used
node-pool slots after final unload. Assertions remain enabled for this probe in
release builds. COROLLA also passes three reloads with 571 bindings each and clean
release. Native SKYLINE orbit (60 frames) and scripted drive (120 frames),
with audio disabled, pass. Debug output confirms **3 of 3 wheel/brake tables**,
1,224 live upload buffers, then zero packs/solids/buffers after destruction.
Source/prepared wheel-record checksums and C0 values remain unchanged.

At this manager checkpoint, texture runtime records were an explicit missing-record
boundary: 1,781 lookups per SKYLINE load returned null. This is superseded by the
texture-pack activation below. Technique/effect objects remain
uninitialized. These records are therefore not submitted through recovered
dispatch yet; native drawing still uses its GL lists and decoded image cache.
No shader, material or lighting behavior is invented to fill that gap. Texture
loading, technique setup, bounds classification and GPU consumption remain open,
along with the other wheel inventory seams and the prior legacy coefficient
precision gap.

### Texture record lookup and slot lifetime (2026-10-26)

`CarRenderTexture.cpp` recovers the CPU texture records needed by solid dispatch:

| Addresses | Behavior |
| --- | --- |
| `0x48fc40` / `0x48fd10` | Construct/unlink a pack descriptor, copy its strings, hash its name and call inventory leaves |
| `0x48fd30` / `0x4919e0` / `0x491a30` | Register one record, register/unregister arrays, maintain counters, parent links and the default texture |
| `0x48fe00` / `0x4901d0` / `0x4902d0` | Loaded lookup, global lookup with pack promotion, and reference refresh |
| `0x494a50` / `0x494b10` | Attach/detach pixel and palette ranges, preserving GPU and reference-notification order |
| `0x494c10` / `0x494c80` | Register/attach and detach/unregister a decompressed streaming slot |
| `0x5b9690` | Assign a texture's state pointer |
| `0x50d510` / `0x4022c0` | The reference build's zero-return inventory lookup and empty release leaf |

These names describe the observed PC behavior; original symbols are unavailable.
The three callbacks at `0x494b10`, `0x494c10` and `0x494c80` have no function
definition in the current Ghidra project. Their instruction streams establish
their boundaries and the calls above without changing the project. The manifest
`config/SPEED2_EXE_1_2/car-render-texture.json` fingerprints all fifteen complete
address spans, including the array registration epilogue.

The serialized texture record is **0x7c bytes**. Its first 12 bytes precede the
name at `+0x0c`; they are part of every record, rather than padding before the
array. Hash `+0x24`, parent hash `+0x2c`, signed pixel/palette offsets `+0x30`/`+0x34`,
byte counts `+0x38`/`+0x3c`, dimensions `+0x44`/`+0x46`, material byte `+0x56`,
pack/parent pointers `+0x68`/`+0x6c` and pixel/palette pointers `+0x70`/`+0x74`
retain their original positions. The separate **0x20-byte** state has format
`+0x14`, GPU token `+0x18` and group `+0x1c`. Descriptor and pack header are each
`0x7c` bytes; sorted index entries are eight bytes and compressed stream slots
are `0x18` bytes. Unknown fields retain offset names and all layouts are checked.

Registration installs the index pointer before resolving a parent, so an array's
order affects which parent pointer is available. Counters include a wrapping
byte bucket indexed by `hash & 0x1fff`. The special hash `0x66994f4a`
(`DEFAULTALPHA`) updates the real default pointer; no substitute record is
created. Loaded lookup requires pixel data and, when palette bytes are nonzero,
palette data. It does not require a GPU token. Global lookup's raw mode skips
these readiness tests, successful pack hits move to the front and set `used53`,
and override/default hits leave that byte unchanged. The first matching null
override stops the override search. A missing requested default reaches the
original debug trap.

Range attachment tests starting offsets against a half-open interval using the
original wrapped addition and signed comparisons. It does not test the payload's
ending offset. Palette-only changes can upload retained pixels. Detachment only
releases when pixels are absent after a matching change; the six hashes
`0x530b82b0` through `0x530b82b5` then call the additional notification. Both
paths notify `0x4939b0`, `0x5c0dc0` and `0x51bf70` in order and set the changed
flag even for an empty pack. A decoded slot ends in its texture and state;
the callbacks use `end-0x9c` and `end-0x20`, retain pack-relative offsets, and
detach/notify before clearing the index and default pointers.

With `UG2_EXE` and `UG2_DIR` set, run compiler jobs sequentially:

```sh
build/venv/bin/python tools/verify_pc_car_render_texture.py --gcc --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
build/venv/bin/python tools/verify_pc_car_render_texture.py --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
```

Each compiler passes **30,720 comparisons** (2,048 per recovered routine) and
rejects **21 mutations**. Fixtures cover multiple packs, duplicate keys,
zero/one/255 bucket values, counter wrap, explicit and searched registration,
parent availability, null overrides, low-byte refresh modes, signed range edges,
and a controlled upload callback that shortens the pack's count. Full retained
bytes, globals, boundary argument order, return values and caller ABIs are checked.

The asset-backed cycles register, attach, look up every actual index entry,
detach, unregister and destroy **829 textures**: 211 from two uncompressed
`GLOBALB.BUN` packs, 72 compressed SKYLINE slots and 546 compressed shared-car
slots. Compressed inputs are decoded by the **original** JDLZ/HUFF/RAWW routines
to produce the fixture bytes. The fixture supplies header/index pointers normally
written by the header loader (the subsequent pack recovery below removes that
fixture boundary). Each compiler matches 829 upload and
829 release boundary calls, and all pack lists, counts, buckets and default
pointers are empty afterward. Input SHA-256 fingerprints and generated reports
are recorded under `build/analysis/car-render-texture/`; assets stay local.

This is finite behavioral evidence, not code-byte identity or GPU equivalence.
Formatting and the verified return-this leaf are controlled library boundaries;
GPU upload/release and game reference notifications remain external boundaries.
The trap fixture records a debugger-resumed return. At this checkpoint header/root
callbacks and native retained ownership were not activated; the following section
supersedes that limitation. Stream scheduling, technique setup and the recovered
draw consumer remain open.
The native demo/simulator build, 60-frame SKYLINE orbit, three-load 612-solid
lifecycle probe and both compiler solid-dispatch regressions pass. Wheel record checksums
and prepared C0 remain unchanged. The image decoder's misleading padding comment
is corrected; its GL presentation is unchanged.

### Texture pack callbacks and native ownership (2026-10-26)

`CarRenderTexturePack.cpp` adds twelve original routines to the record recovery:

| Addresses | Behavior |
| --- | --- |
| `0x48b1f0` | Create `TexturePackSlotPool` with `0x7c`-byte slots, 128 slots and heap flags zero |
| `0x494380` / `0x494dd0` | Parse/register and detach/unregister/free the `0xb3310000` header |
| `0x48fc00` / `0x48fbc0` | Queue an embedded `0xb3320000` pixel link and find its key |
| `0x494ea0` / `0x495040` | Root load/unload dispatch for `0xb3300000`, `0xb3320000` and `0xb0300100` |
| `0x48fe60` / `0x48ff40` / `0x48fed0` | Construct, resolve and destroy texture animation records |
| `0x5b96f0` / `0x5b9710` | Allocate a GPU-token snapshot and release it through its original pool |

The undefined Ghidra unload callback `0x495040` is recovered from its original
instructions; no project mutation is needed. Address spans are fingerprinted in
`config/SPEED2_EXE_1_2/car-render-texture-pack.json`. Header loading preserves
the original child order, count divisions and count-mismatch return after setting
`loaded6C`. Root loading queues pixel chunks, looks up the final header's key,
aligns its data pointer to 128 bytes, attaches the range, saves its byte count and
unlinks the pixel node. Root unloading detaches before unregistering/freeing the
descriptor. A direct pixel unload returns success without unlinking its node.

Animation records use `0x34`-byte entries and `0x10`-byte frames. Construction
assigns frame subarrays in order. Resolution checks the real pack index and CPU
readiness, allocates handle snapshots for ready frames, and sets each entry's
all-ready flag, including the one-valued empty-frame case. Destruction releases
snapshots only for nonnull texture pointers before unlinking the animation.
Snapshot creation captures the state pointer before allocation and reads its
GPU token afterward. Its receiver is fastcall; snapshot release uses `RET 4`.
Root loading also preserves the disabled-animation branch and the original null
receiver call on allocation failure; it introduces no new recovery policy.

With `UG2_EXE` and `UG2_DIR` set, run the compiler jobs sequentially:

```sh
build/venv/bin/python tools/verify_pc_car_render_texture_pack.py --gcc --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
build/venv/bin/python tools/verify_pc_car_render_texture_pack.py --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
```

Each compiler passes **28,672 comparisons**, covering twelve routines through
fourteen entry/lifecycle checks, and rejects **16 mutations**. Fixtures include
count mismatches, array count rounding, pixel alignment at 32 offsets, queue hits
and misses, ready/missing animation frames, empty entries, disabled animation
loads and root/header/animation unload cycles. The original and rebuilt routines
run with the recovered texture registration/lookup code in the same chain.
Complete fixture/global bytes, allocated descriptor/snapshot bytes, call order,
arguments, return values, caller registers and stack balance are compared. The
normal-return FS exception-list slot is checked; exception unwinding and invalid
allocation/data paths are not established by these finite fixtures.

The retail tests execute the actual root/header callbacks for the **829 textures**
in the same three files used above, including original decompression and every
index lookup. Both compilers match 829 upload/release boundary calls and free all
four descriptors, leaving empty pack/pixel queues, counts, buckets and defaults.
Pool allocation, formatting, the return-this leaf, GPU APIs and game reference
notifications remain controlled boundaries. This does not establish byte identity
or complete streaming, animation scheduling or rendering behavior. Reports stay
under `build/analysis/car-render-texture-pack/`.

`NativeTextureRecords` now retains aligned mutable file copies and decompressed
slots. The adapter invokes recovered root callbacks, uses the original bWare pool
and decompressor, preloads compressed car slots synchronously, and unloads slots
before roots and backing storage. It deduplicates files by cache key, accepts the
empty brake pack and frees the descriptor pool after its last owner. The texture
cache owns these records for the car packs and `GLOBALB.BUN`; track/HUD image
decoding stays native. Car solids are cleared before a reload and destroyed before
their texture owner. The missing-record lookup stub is removed.

The display-free probe now validates both lifetimes:

```sh
cmake --build build/native --target ug2demo ug2sim ug2solidrecords_probe -j 6
UG2_DEBUG_TEXTURES=1 UG2_DEBUG_SOLIDS=1 build/native/ug2solidrecords_probe "$UG2_EXE" "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN"
```

SKYLINE passes three complete reloads with **612 solids and 829 texture records**
per load, **5,343 bound texture slots** in total, clean lists/counts/buckets and
no used descriptor/node-pool slots after release. Its 1,781 slots per load resolve
to **534 matching hashes and 1,247 real `DEFAULTALPHA` fallbacks**, with zero null
records. The default fallbacks do not recover runtime skin/replacement producers.
COROLLA passes three reloads with 571 solids, 813 textures and 5,838 bound slots.
The native build, 60-frame orbit and 120-frame scripted drive pass; source/prepared
wheel checksums and C0 results remain unchanged.

At this pack checkpoint GPU texture tokens remained zero. The upload recovery
below supersedes that limitation. The reference recovery later in this section
also supersedes the no-op notification boundary. Shader/effect setup remains
open, and animations are not
scheduled by the native preload adapter. Existing GL lists still consume decoded
RGBA images rather than recovered draw queues or wheel lighting records. Bounds
classification, dispatch activation, `0x5cf420` and the other wheel inventory
items remain open.

### Texture upload and deferred retirement (2026-10-26)

`CarRenderTextureUpload.cpp` recovers four original routines:

| Address | Behavior |
| --- | --- |
| `0x5ce170` | Retire a previous handle, expand palettes, create/upload mip levels and register the state |
| `0x5caaa0` | Enqueue a texture handle, clear it and unlink its state |
| `0x5ce4c0` | Release at most three deferred nodes per enabled pass |
| `0x5ce400` | Unlink live states, release live/deferred handles to refcount zero and free deferred nodes |

The instructions establish a separate deferred texture queue at `0x8763b0`;
vertex buffers use `0x870d28`. The live texture-state list is `0x872b4c`.
Both texture paths call the already recovered enqueue `0x479130`. Upload retires
the previous handle before examining pixel availability. Palette format `0x29`
expands indexed pixels to four-byte palette entries across signed mip counts,
changes the submitted format to `0x15`, multiplies the copy-size word by four
and writes compression byte `0x20`. State/texture layouts remain unchanged.

The original mip-reduction branch divides dimensions by **twice the skip count**,
then advances over skipped source levels using signed division by four. Subsequent
copy lengths use arithmetic shifts with the x86 shift-count mask. The uploader
ignores the returned row pitch and copies flat words followed by trailing bytes,
with forward overlap behavior. It re-reads the handle before unlock and ignores
the GPU HRESULTs. Successful creation increments `0x864f50`, which is an upload
counter rather than a live-handle count. The bounded retirer ignores Release's
return count; the full flush repeats Release until zero. Live handles are re-read
between calls, while a deferred handle is captured once.

The allocation/free placement is preserved. Palette conversion occurs before
the upload-disable check, and its allocation is freed only after successful
texture creation. Disabled upload and failed creation can therefore retain that
temporary allocation in the original code. The source does not silently repair
those paths. Native captures use enabled uploads with successful creation.

With `UG2_EXE` and `UG2_DIR` set, run compiler jobs sequentially:

```sh
build/venv/bin/python tools/verify_pc_car_render_texture_upload.py --gcc --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
build/venv/bin/python tools/verify_pc_car_render_texture_upload.py --mutation-checks --retail "$UG2_DIR/GLOBAL/GLOBALB.BUN" --retail "$UG2_DIR/CARS/SKYLINE/TEXTURES.BIN" --retail "$UG2_DIR/CARS/TEXTURES.BIN"
```

MSVC 7.1 `/Ox /Ob0` and GCC i386 C++11 `-O1` each pass **10,240 comparisons**
(four entries plus their upload/release/flush cycle) and reject **14 mutations**.
Fixtures cover palette values, signed dimensions/counts, zero and trailing-byte
copies, forward aliases, mip skipping, shift wrap, replacement handles, counter
wrap, disabled/failed creation, nonzero HRESULTs with valid outputs, multi-node
queues and Release counts greater than one. Complete record/global/list bytes,
allocation guards, staged mip bytes, argument order, frees and caller ABI are
compared against the original executable. Fingerprints are in
`config/SPEED2_EXE_1_2/car-render-texture-upload.json`.

The retail check runs **829 upload/release/flush cycles** from the same three
files used by the pack verifier. It matches **2,654 mip locks/unlocks**, 829
create/defer/release/node-free calls and **14 palette allocations/frees** per
compiler. The input formats are 149 DXT1, 635 DXT3, 31 ARGB8 and 14 indexed
textures. Compressed fixture bytes come from the original decompressor. Raw
upload bytes, rather than a decoded RGBA approximation, are compared. Reports
remain under `build/analysis/car-render-texture-upload/`; retail data stays local.

GPU methods and heap allocation remain typed boundaries, as do the independently
verified deferred enqueue/node-pool operations. Finite tests do not establish
byte identity, complete API failure behavior, or Direct3D equivalence. Failed
locks that leave their outputs unwritten are outside the fixtures. The original
disabled/failed-create palette allocations are explicitly compared as unfreed.

`NativeTextureUpload` now provides guarded **CPU staging storage** behind
four-byte tokens for these recovered calls. It accepts the observed ARGB8 and
DXT formats, retains separate mip buffers, and bounds their storage by a
four-byte-per-texel capacity with small-block padding. It does not implement
Direct3D row layout, general format support, automatic mip generation or a GPU
driver. Native mip guards are checked on unlock and release. File/cache teardown
repeatedly invokes the recovered three-node retirer at its CPU lifetime boundary;
staging storage has one owner and no driver work in flight.

The demo/simulator/probe build passes. The strengthened native probe verifies
nonzero staging tokens in every bound solid slot, intact upload guards and empty
live/deferred lists after three reloads. SKYLINE has **829 staged texture handles**
per load and COROLLA has 813; both release all handles and node-pool slots. Native
60-frame orbit and 120-frame scripted drive pass with unchanged wheel-record
checksums, C0 results and image-cache counts. The no-op upload/release stubs are
removed. Game reference consumers are recovered in the following checkpoint.
Shader/effect setup, draw dispatch activation,
`0x5cf420`, and GL consumption of uploaded textures/lighting records remain open.

### Texture reference consumers and live rebinding (2026-10-26)

`CarRenderTextureReferences.cpp` recovers ten original routines:

| Addresses | Behavior |
| --- | --- |
| `0x4cbab0` | Prepend a pointer-bearing bWare node to an intrusive list |
| `0x4907e0` / `0x490850` | Refresh a solid's default or unloading-pack texture slots |
| `0x493800` | Rebuild a solid pack's texture dependencies and unresolved-slot list |
| `0x4938e0` | Encode offline texture references with the original byte displacement |
| `0x4939b0` | Notify loaded solids when a texture pack attaches or detaches data |
| `0x5c0dc0` / `0x51bf70` | Refresh standalone and font texture-reference prefixes |
| `0x611910` / `0x611a10` | Locate and clear one of six special texture cache records |

Instructions establish the member-call arguments and `RET 4` for both slot
refreshers, despite the decompiler's incomplete load-helper prototype. The
return contract is the low byte. Pack `+0x80` holds dependency nodes, `+0x88`
holds pending-slot pointers, and words `+0x7c`/`+0x7e` hold their counts, with
`0xffff` at `+0x7c` requesting a rebuild. Rebuilding first runs the already
recovered list cleanup `0x493790`. The helpers reuse the real texture lookup,
readiness checks and unsigned-key duplicate search.

Preserved details include checking only the low byte of notify mode, snapshotting
the next pending node before freeing it, updating dependencies before refreshing
all loaded solids, and rebuilding dependencies after unload changes. A successful
local unload lookup keeps the current slot; a failed lookup searches other packs
and then uses the real default. Offline mode retains the original encoded byte
offset rather than treating it as a host object. The pending-node duplicate check
compares against the default texture's hash, not the current requested key; that
observed behavior is preserved and mutation-checked.

The standalone prefix has hash/pointer at `+0x0c`/`+0x14`. The font-list prefix
has pointer/hash at `+8`/`+0x18`; its list at `0x863768` is also used by the
original font lookup/loader `0x51bd70`/`0x52cd00`. These are checked prefixes,
not recovered class constructors. The six cache records start at `0x89f050`,
have stride `0x1c8`, and are selected only by hashes `0x530b82b0` through
`0x530b82b5`. Invalidation clears exactly `0x72` words. Their unidentified
contents retain offset names.

With `UG2_EXE` set, run the compiler jobs sequentially:

```sh
build/venv/bin/python tools/verify_pc_car_render_texture_references.py --gcc --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_texture_references.py --mutation-checks
```

Both compilers pass **20,480 comparisons** (2,048 for each routine) and reject
**18 mutations**. Fixtures cover multiple packs, ready/missing data, duplicate
keys, signed slot counts, default restoration, dirty dependency lists, low-byte
modes, offline displacements, list deletion during iteration, both standalone
prefix layouts and all six cache records plus unknown hash boundaries. Complete
fixture/global/node bytes, ordered allocations/frees, typed return values and
caller ABIs are compared. The manifest
`config/SPEED2_EXE_1_2/car-render-texture-references.json` fingerprints complete
address spans, including alignment gaps in the notify routine. Reports are under
`build/analysis/car-render-texture-references/`.

Node allocation/free and the verified return-this leaf are controlled boundaries;
real pack cleanup, texture lookup and binary search execute in the same chain.
Finite valid record/list tests do not establish byte identity, malformed/OOM
behavior, rendering equivalence or full standalone/font class construction.

The native no-op notifications are removed. Shared solid-pack state is initialized
before texture loading, and native reference/font lists start with valid sentinel
heads. The existing display-free probe now supports loading car textures after
the solids and unloading those textures while the solids remain alive:

```sh
cmake --build build/native --target ug2demo ug2sim ug2solidrecords_probe -j 6
UG2_DEBUG_TEXTURES=1 build/native/ug2solidrecords_probe "$UG2_EXE" "$UG2_DIR/CARS/SKYLINE/GEOMETRY.BIN" --late-textures
```

Across three native passes, SKYLINE matching slots change **500 → 534 → 500**
(base `GLOBALB.BUN`, late car/shared packs, then their unload). COROLLA changes
**592 → 707 → 592**. Every slot is checked against the live texture registry
before it is dereferenced, then checked for readiness and a live staging handle.
The probe's standalone/font prefix fixtures become null on unload, and all six
cache records clear from nonzero test values. The full stages retain 829/813
textures, and final teardown leaves zero used node-pool slots, handles, buckets
and pack/reference lists. The ordinary texture-first SKYLINE probe also passes.

The native build and 60-frame orbit pass with unchanged wheel-record checksums
and C0 values. Normal native font construction, texture animation scheduling,
shader/effect setup, recovered draw activation and GL consumption remain separate
work. The technique producer/setup recovery below supersedes the earlier
producer gap; native effect support and the original `0x5cf420` consumer remain open.

### Technique construction, parameter semantics and setup (2026-10-26)

`CarRenderTechnique.cpp` recovers seven complete PC v1.2 routines:

| Address | Recovered behavior |
| --- | --- |
| `0x5d0f80` | Construct the `0x28` technique and allocate its `0x988` parameter block |
| `0x5d1b70` | Bind the first matching parameter hash in the static slots and library map |
| `0x5d1bd0` | Discover effect parameters using their **semantic**, rather than display name |
| `0x5d1e20` | Build a vertex declaration, create an effect, select a valid technique and count passes |
| `0x5d21f0` | Query capabilities, clamp filtering, create/reuse all 23 technique objects and set them up |
| `0x5be560` | Select magnification filtering from the selector, adapter vendor and capability bits |
| `0x5bc5d0` | Create, lock and populate the original 64-pixel ramp texture |

The full technique layout retains four-byte code/COM tokens and parameter pointers.
Its original class tokens are stored as data, separate from host C++ vtables.
The parameter block has 60 `0x28` slots followed by a `0x28` library map. Construction
preserves the exact names (including `ENVMAPANGE`), copy/hash/store order, zeroed
handles and untouched name tails. `RAMPMAP` is initialized before the two raindrop
slots. Binding writes the static handle before inserting/updating the map and
stops at the first duplicate hash. Library reset leaves the static handles intact.

Setup preserves the 23 original recipes, eight-byte declaration elements, the
end sentinel, 32-bit stride accumulation and 16-bit offsets. It releases an old
effect once, snapshots adapter IDs before resetting the parameter map, creates
an effect from the recipe resource and caches its parameter semantics. Candidate
selection retains the original shader annotation gate and SiS/Intel/Matrox
fallbacks. It applies the blend/filter/fog/color-write parameters before validation,
accepts only a zero validation result, and queries at most 16 passes even when
no technique was accepted. A failed selection leaves the old selected-index and
shader byte intact. The depth-world focal-range write checks the final type field.

The root queries capabilities twice, creates a pool without pre-clearing the old
pool value, constructs the 23 class identities only when the ready byte is zero,
then runs each setup before setting ready and creating the ramp. The ramp uses
all four identical channel bytes, the original float constants and x87 operation
order with truncating conversion; its complete output bytes must match at both
`0x027f` and `0x037f` precision settings. Pitch is ignored as in the original.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique.py --gcc --cases 2048 --root-cases 128 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique.py --cases 2048 --root-cases 128 --mutation-checks
```

Both compilers pass **12,416 comparisons** (2,048 per leaf/setup routine and
128 complete producer runs) and reject **17 mutations**. The producer runs call
all 23 setups, covering 4,992 setup calls in total per compiler when combined
with the standalone cases.

Verification compares all fixture/global/allocation bytes, ordered boundary
arguments and writes, return values, preserved registers, stack cleanup, x87
control word and normal-return exception-list state. Fixtures include different
parameter names/semantics, null/unknown/duplicate semantics, stale handles, all
23 recipes/class identities, bounded declaration permutations (including stride
wrap), both construction/reuse branches, vendor fallbacks, signed attempt limits,
annotation values, pass counts through and above 16, and API error results with
valid output records. The manifest `car-render-technique.json` fingerprints the
complete original address spans, including alignment gaps in the producer.
Reports are under `build/analysis/car-render-technique/`.

D3D/D3DX methods, allocation, and the library container's construct/insert/reset
remain controlled boundaries in this technique verifier. Container lifetime and
vector bodies now have separate original/rebuilt evidence below; hash insertion
remains unrecovered. Opaque empty allocator/comparator tag
padding is not interpreted. String copy/hash and all seven recovered engine
routines execute in the comparison chain. This establishes bounded behavioral
and raw ramp-byte evidence; it does not establish byte identity, exception/OOM
behavior, arbitrary library container behavior or shader/rendering equivalence.
The original minification local is undefined for selectors outside 0..2 with a
live parameter; those inputs are excluded from setup fixtures. The standalone
magnification leaf also checks negative and out-of-range selectors.

The native build still supplies an inactive technique table. This source is
kept outside the native link until effect API adapters and resource ownership
exist; successful shader objects are not fabricated. Native CPU texture staging
also requires a real API treatment of the ramp's zero mip-level request before
activation. Technique methods and destruction are recovered in the uploads
section below; the library container, shader resources/backend and bounds
classification remain open. Consumer `0x5cf420` is recovered below; its helper
bodies and native activation remain open. The native build/orbit and existing
solid-dispatch checks are regressions
for the expanded record layout, not execution of native effects. The demo,
simulator and display-free solid probe build successfully. The existing solid
dispatch verifier passes 12,288 comparisons on each compiler. Native SKYLINE
passes a 60-frame orbit and 120-frame scripted drive with unchanged source/record
checksums (`ca993eba`/`7141bcda` orbit, `eb0428b7`/`69927c13` drive). The three-pass
late-texture probe retains 612 solids and 829 textures per load, follows
500 → 534 → 500 matching slots, and unloads with clean buffers and pools.

### Queued solid consumption and draw dispatch (2026-10-26)

`CarRenderConsume.cpp` recovers the complete `0x5cf420` queue consumer, command-list
clear `0x5cdf10`, group clear `0x5cea20`, view lookup `0x48b1e0`, sky index `0x60fe00`
and sky fraction `0x60fde0`. `bSlotPool::Free` recovers the member-call slot return
at `0x43cdf0`. Existing recovered `bFree`, pool allocation and primitive queueing
also execute in the verifier's end-to-end queue/consume cases.

The consumer visits the original 16 rows of 23 technique heads in address order.
The two eight-row native arrays are addressed separately so host storage does not
need to be adjacent. Each nonempty head selects the table technique, runs its
Begin method, binds the vertex declaration and begins the effect. Groups and
commands are unlinked before their callbacks. World, headlights, extra-record,
light-pack and bone upload calls retain their original arguments, gates and order.
Every returned command/group updates the actual pool free list and used count;
the texture state's cached group is cleared before the group is returned.

Indexed commands lazily create a missing vertex buffer, lock it, copy whole words
then tail bytes, and unlock it. Stream and index bindings use the original caches.
The draw gate requires a nonzero primitive index count and vertex count below
`0xffff`. The start index preserves wrapped subtraction followed by arithmetic
right shift, and triangle counts use unsigned division. The shader/debug/type
checks can force one pass for types 0, 1, 4 and 5. Type 20 selects a preceding sky
texture with the original hash-dependent wrap, writes the float fraction, then
truncates the float-times-255 result into the high alpha byte.

Immediate commands disable state 14, invalidate only the stream cache, draw the
original 24-byte vertices once per configured pass, restore state 14 and return
the vertex record through the member pool operation. Both paths commit even
when the configured pass count is zero or negative. The effect Begin output does
not replace that pass count. Counters preserve the original behavior, including
the indexed statistic that adds **vertex count / 3**, rather than the primitive's
index count / 3. Technique End precedes effect End; the current-technique pointer
is then cleared, and the total queued-group count is zeroed after the full walk.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_consume.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_consume.py --mutation-checks
```

Both compilers pass **14,336 comparisons** (2,048 per routine) and reject
**23 mutations**. Per compiler, the consumer observes 8,189 groups, 7,935 indexed
draw calls, 3,945 immediate draw calls, 1,024 lazy uploads and 249 sky-morph updates.
The 1,024 queue/consume cases use the real recovered queue and pool allocation
bodies, including group reuse/replacement; the remaining consumer cases start
from independently constructed queue graphs.

The effect table at `0x7b6fe0`, installed by constructor `0x66509e`, confirms the
original slot identities: Begin `+0x104` → `0x664782`, BeginPass `+0x108` →
`0x66acd2`, CommitChanges `+0x10c` → `0x66ac17`, EndPass `+0x110` → `0x65adcb`,
and End `+0x114` → `0x65ace5`. The consumer's commit-before-pass order is retained;
no ordering is inferred from a modern effect header. These library bodies are
read as boundary evidence, not recovered or executed by the consumer verifier.

The fixtures compare complete arena/global/pool bytes, queued state before the
consumer, ordered call arguments, upload bytes, preserved registers, stack
cleanup and x87 control words. They cover all queue rows/types, empty and repeated
heads/groups, cached group reuse and replacement, ordinary and immediate draws,
negative/zero/multiple pass counts, vertex/index bind reuse, lazy uploads with
word/tail lengths and overlapping buffers, headlight/record/light/bone gates,
sky selection, wrapped counters and signed start offsets. The sky fraction also
has standalone **raw 80-bit x87 return** checks at both precision settings with
signed zero, subnormal, finite, infinity and NaN inputs. Complete original address
spans are pinned by `car-render-consume.json`, including consumer alignment gaps.
Reports are under `build/analysis/car-render-consume/`.

D3D/D3DX methods, texture lookup and the technique virtual Begin/End calls remain
controlled boundaries. Six engine helpers (`0x5bcc30`, `0x5bdde0`, `0x5bdaa0`,
`0x5bd140`, `0x5bd4a0`, `0x5bdcd0`) are observed call boundaries in this
standalone verifier. The uploads section below supersedes the body gaps for
world/headlight/bone uploads and exercises them in the consumer chain. The
material section below closes the other three engine-helper body gaps. Tests
use writable bounded records and controlled API results,
including invalid count/limit values; they do not establish arbitrary malformed,
OOM, API failure, shader or rendering equivalence. This is behavioral and upload/
x87-byte evidence, not instruction-byte identity.

The consumer is compiled into the recovered archive but is not called by the
native demo. The current GL display-list seam still ignores the original flags,
lighting records and shader effects. The uploads section below recovers the
technique methods/destruction and three upload helpers; the material section
below recovers texture binding and extra-record/light-pack uploads. Effect
resources/backend, full view/frame scheduling and native queue/consumer activation
remain necessary to close that seam. The existing
bWare pool verifier and native demo/probe build are regressions for the added
pool method; they do not establish native consumer execution. The pool verifier
passes its 300 trials of 60 operations. Native SKYLINE's 60-frame orbit keeps
source/record checksums `ca993eba`/`7141bcda`; the three-load late-texture probe
retains 612 solids and 829 textures per load, follows 500 → 534 → 500 matching
slots and finishes with clean handles, buffers and node pools.

### World, headlight and bone uploads; technique runtime (2026-10-27)

`CarRenderUploads.cpp` and `CarRenderTechniqueRuntime.cpp` recover fourteen
complete PC v1.2 routines. The original addresses and complete spans are pinned
by `car-render-uploads.json`.

| Address | Recovered body |
| --- | --- |
| `0x5bdde0` | World/view/projection cache and fixed-function/effect matrix uploads |
| `0x5bdcd0` | Sixteen selected bone matrices, uploaded as a matrix array |
| `0x5bdaa0` | Headlight projection/clip matrices and packed-color vector |
| `0x60f510` | Packed headlight color lookup through observed object prefixes |
| `0x5bc6f0` | Headlight technique Begin |
| `0x5bc7b0` | Reflection technique Begin |
| `0x5bc890` | Combined headlight/reflection technique Begin |
| `0x5bcad0` | Car technique Begin |
| `0x5bcb10` | Filter technique Begin |
| `0x4022c0` | Shared empty concrete End and generic Begin |
| `0x5d1b30` | Technique destruction |
| `0x5d1b10` | Deleting wrapper with low-byte flag bit 0 |
| `0x5d0ca0` | Base deleting destructor using the parameter wrapper |
| `0x5d0880` | Parameter wrapper forwarding destruction to map `+0x960` |

World upload retains the three-part world/technique/view cache, identity-pointer
counter, matrix-binding gates and cache update order. Fixed-function view and
projection bindings precede cache updates; world-view is conditional on its
parameter handle. The world transform call re-reads the shader byte before
choosing whether to also upload world-view-projection. The latter upload is
retained even for a zero parameter handle. Device callbacks that change that
byte are included in the comparison fixtures.

Primitive bytes `+0x21..+0x30` select sixteen entries from the supplied matrix
array. Its allocation/count is not established by this routine; the fixtures
provide 256 entries to cover the complete byte-index domain. Every matrix is
copied as sixteen raw words, preserving all float bit patterns. Index 255 is
read normally, and repeated indices are not removed.
The effect receives all sixteen matrices even when its handle is zero. The
primitive's observed size and offsets remain unchanged.

Headlights preserve both the shader projection path and fixed-function camera
inverse/projection/clip path, including the original in-place multiply aliases.
Packed color comes through the observed pointers at object `+0x554` and value
`+0x380`; the surrounding object types and ownership remain unidentified. Its
three channel bytes are multiplied by **1/128**, with a zero fourth component.
No conventional 1/255 normalization is substituted.

Technique methods use the receiver's shader byte while parameter/effect lookups
use the separately stored current technique. Reflection builds the original
half-scale bias and copies the fourth column into the third before upload. The
combined shader path deliberately repeats the headlight/ramp/environment
bindings. Car Begin chooses the original texture token for mode 6; filter Begin
clears stage state 24 on all four stages. Destruction preserves the base identity,
parameter-map cleanup, allocation release and single effect Release order. The
parameter pointer and declaration token remain unchanged, including the dangling
parameter pointer after its allocation is released. The library map's own
`0x5d0660` algorithm remains a controlled boundary.

`NativeTechniqueDispatch.cpp` supplies the typed callback bridge for the six
concrete class tokens recorded by construction. It calls recovered methods and
never places a host C++ vtable in the original record. It is compiled into the
native archive; the demo does not yet invoke the recovered GPU consumer. Unknown
or pure class tokens cannot dispatch through this bridge.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_uploads.py --gcc --cases 2048 --chain-cases 512 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_uploads.py --cases 2048 --chain-cases 512 --mutation-checks
```

Both compilers pass **29,184 comparisons** (2,048 per recovered routine plus
512 complete consumer-chain cases) and reject **31 mutations**. The palette
fixtures cover all 256 byte indices and include 788 cases with repeats. Nullable
color prefixes, null texture tokens and deletion/resource flags vary independently.

The comparison checks complete fixture/global/allocation bytes, ordered GPU
arguments and matrix/palette/vector bytes, multiply inputs and alias flags,
cache/counter state at API calls, release/free order, typed returns, stack cleanup,
preserved registers and x87 control words. Fixtures cover both precision settings,
cache reuse and changes, zero/live handles, all byte palette indices, alternate
receiver/current records, tuning values, shader/fixed-function branches, null
color prefixes, low-byte deletion flags and cleanup gates. The pure parameter
forwarding wrapper is checked by its effects because GCC folds it into its caller.

Consumer-chain cases link the real recovered queues, pools, world/headlight/bone
uploads and technique methods through the native token bridge. Recovered identity,
rigid inverse, string hash and view/sky helpers execute. The original scalar D3DX
multiply body executes on both sides; these results do not establish equivalence
of every native or optimized D3DX implementation. Three engine consumer
bodies remain controlled in this standalone verifier: texture binding `0x5bcc30`, record upload
`0x5bd140`, and light-pack upload `0x5bd4a0`. GPU/effect methods, texture lookup and
library map destruction remain controlled runtime boundaries. Finite bounded record
and matrix fixtures do not establish malformed/OOM/exception behavior, native
shader execution or rendering equivalence. Byte identity is not claimed.

The next section recovers those three bodies and links them into the consumer
chain. Native effect/API ownership and resource loading, the library map,
full view/frame scheduling and native consumer
activation remain open. Existing GL drawing still ignores the original lighting
records and flags. The demo, simulator and solid-record probe build successfully;
the existing consumer verifier also passes 14,336 comparisons per compiler after
its fixture/build utilities were extended for the linked chain. Native SKYLINE's
60-frame orbit retains source/record checksums `ca993eba`/`7141bcda`. Its three-load
late-texture probe retains 612 solids and 829 textures per load, follows
500 → 534 → 500 matching slots, and leaves clean handles, buffers and node pools.
These are archive/layout/lifecycle regressions, not native GPU execution of these
new helpers. The other wheel inventory rows and legacy coefficient transform
precision gap remain separate work.

### Texture binding and record/material uploads (2026-10-27)

`CarRenderMaterial.cpp` recovers the last three engine-helper bodies controlled
by the preceding consumer-chain verifier, together with seven dependencies:

| Address | Recovered body |
| --- | --- |
| `0x5bcc30` | Texture/technique cache, blend/alpha/depth/cull state and UV uploads |
| `0x5bd140` | Extra-record upload: directional lights or harmonic/eye vectors |
| `0x5bd4a0` | Light-pack upload: fixed-function material or effect vectors |
| `0x5bcb70` / `0x5bcbc0` | Alpha-test and blend state setters |
| `0x48b450` | Truncating texture phase/quantization helper, `RET 0x10` |
| `0x5b7d40` / `0x5b7d80` | Signed texture-coefficient U/V scroll methods |
| `0x5bd060` | Four-channel clamp with the original unordered/zero behavior |
| `0x486f50` | Four-component transform, reached through thunk `0x43c9b0` |

`car-render-material.json` pins the complete original body spans and reference
executable SHA-256. The executable fingerprint also covers the texture binder's
external switch table. Unknown record fields retain offset names. The texture
record remains `0x7c` bytes, with signed coefficients at `+0x58..+0x60`.
The material consumer reads through light-pack `+0x9c`; its separate `0xa0` read
view does not redefine the shorter chunk-loader prefix or establish allocation
ownership. API record layouts follow the original calls and Microsoft's
[D3DMATERIAL9](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dmaterial9)
and [D3DLIGHT9](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dlight9)
definitions.

Texture binding preserves the pair cache `(texture, current technique)`, including
updating it and incrementing the counter **before** returning for a zero GPU
token. Alpha testing requires byte `+0x55 == 1`; other nonzero values do not enable
it. Opaque/transparent paths retain their original cull/depth writes and blend
factors, and the effect receives the five-word blend array even with a zero
parameter handle. The UV handle gates the computation and dirty flag. Mode 3
uses the negated signed static offsets; other modes choose texture coefficients
or width/height-scaled overrides. The UV matrix uses elements **8 and 9**.
Mode 0 clears the dirty flag only at the end. The nonzero-token path ultimately
requires a current technique; no speculative null guard is added.

The wave helper truncates toward zero, preserving negative remainders. Mode 2
first truncates time/period. Both conversions use the signed low 32 bits of the
original `_ftol2` 64-bit result; large values can consequently return outside a
conventional fractional range. Tests include this low-word wrap and zero-period
i386 behavior, and capture the actual 80-bit x87 returns. They do not establish
portable out-of-range float-to-integer semantics on another architecture.

Record upload distinguishes technique types 4 and 5 and the shader byte. The
fixed-function type-5 path clamps the scaled ambient coefficient, packs ABGR,
transposes the three input directions, transforms them with `w=0`, and sends
three directional lights. It enables a light when any absolute direction
component fails the strict threshold test; small/zero directions produce **no
disable call**. The color clamp maps NaN to one and negative zero to positive
zero. The transform consumes every input before its first output store and
preserves the component-specific sum order, including overlapping output/input
and output/matrix cases. Shader uploads retain ten harmonic vectors and the
local-eye vector.

The device-light payload is partially initialized by the original routine.
Verification compares all fourteen written words: type; diffuse/specular RGB;
ambient RGBA; and direction XYZ. Diffuse/specular alpha, position and the other
fields are indeterminate and excluded. The recovery leaves them unwritten;
no claim is made about their bytes or general API treatment of them.

Light-pack upload retains the base/range float spills, environment flag `0x100`,
global enable and signed-low-byte gate. Fixed-function type 5 uploads the
original material, clamps power to the binary32 `0.1` minimum, and packs the
texture factor. Its alpha is `rangeAlpha * 128 + baseAlpha * 255`, with truncation;
substituting a conventional 127.5 average changes results. Shader type 4/5 uploads
the two base vectors, then **re-reads** the current type before the type-5-only
vectors and scalar parameters. Fixtures mutate that type and a late-read pack
field from controlled effect callbacks.

The clock scale, direction threshold and minimum power are loaded from rounded
binary32 constants, matching the binary's pooled constants. Using bare decimal
literals under GCC's x87 excess precision changed the light-enable decision at
the exact threshold. Explicit float memory loads resolve that difference without
changing the threshold or widening acceptance limits.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_material.py --gcc --cases 2048 --chain-cases 512 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_material.py --cases 2048 --chain-cases 512 --mutation-checks
```

Both i386 GCC and MSVC 7.1 pass **20,992 comparisons** each: 2,048 cases for each
of ten routines and 512 complete consumer chains. Each rejects **39 mutations**.
The fixtures vary cache pairs, texture modes/coefficient signs, alpha/material
bytes, zero/live parameter handles, immediate/effect state paths, signed flags,
threshold directions, NaN/infinity clamp inputs, matrix aliases, callbacks and
x87 control words `0x027f`/`0x037f`. Reports are generated under
`build/analysis/car-render-material/`. Comparisons include fixture/global bytes,
ordered API arguments and written upload bytes, cache state at API boundaries,
stack cleanup, preserved registers and raw x87 returns. Instruction-byte identity
is not established.

The consumer chain now executes every direct engine bind/upload helper, the
recovered concrete technique methods, queues and pool return chain. GPU/effect
APIs, texture lookup and library map destruction remain controlled boundaries.
The original scalar D3DX library runs on both sides where used by the preceding
uploads. Finite fixtures do not establish arbitrary malformed/OOM/exception
behavior or native rendering equivalence.

The new source builds into the recovered native archive; the demo still uses
its GL display lists. `ug2demo`, `ug2sim` and `ug2solidrecords_probe` build, the
60-frame SKYLINE orbit retains source/record checksums `ca993eba`/`7141bcda`, and
the three-load late-texture probe retains 612 solids/829 textures per load with
500 → 534 → 500 matching slots and clean staging handles, buffers and pools.
These are layout/lifecycle regressions. Native effect/API resources, the library
map, full view/frame/cache scheduling and dispatch/consumer
activation remain open, along with the other wheel inventory rows and the
separate legacy coefficient-transform precision gap.
The standalone GCC consumer and upload verifiers also retain their 14,336 and
29,184 passing comparisons after the shared harness extensions.

### Solid bounds classification and AABB transformation (2026-10-27)

`CarRenderBounds.cpp` recovers `CarRenderTransformBounds` (`0x5bc210`, 237 bytes)
and `CarRenderView::TestBounds` (`0x5bc300`, 325 bytes). Original spans and the
reference executable SHA-256 are pinned by `car-render-bounds.json`. The view's
previously opaque first word is the pointer used to reach six four-float planes
at target `+0xc0..+0x11f`. A checked read prefix records these offsets without
identifying the surrounding allocation or claiming its complete size. Existing
view focal, frame and extra-record offsets remain unchanged.

The transform starts six accumulators from matrix translation and processes
three input axes. For each matrix coefficient, the minimum-input product remains
extended, while the maximum-input product is **rounded to binary32 before the
comparison**. The ordered comparison chooses the two contributions, preserving
negative coefficients and reversed input bounds. All inputs are consumed before
writing minimum XYZ followed by maximum XYZ, including overlapping bounds and
matrix/output aliases. A volatile float temporary retains the observed spill:
MSVC otherwise compared its unrounded x87 register value, changing output words
even when both input bounds pointed at the same record.

The classifier copies both XYZ bounds, conditionally transforms them for a
nonnull world matrix, and rounds center and extent to binary32 before testing
planes in order. Its radius sum is `(abs(nz)*ez + abs(nx)*ex) + abs(ny)*ey`;
its center distance is `((cz*nz + cx*nx) + cy*ny) + d`. Both sums remain extended
for the strict comparisons against original global `0x7d7d3c`. An outside plane
returns **0** immediately; any crossing plane produces **1** after all planes;
otherwise the result is **2**. Touching either support boundary retains the
original strict comparison. Unordered plane comparisons do not invent a reject.
No Boolean collapse, conventional center/extent replacement or acceptance epsilon
is introduced.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_bounds.py --gcc --cases 4096 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_bounds.py --cases 4096 --mutation-checks
```

Both compilers pass **12,288 comparisons** each: 4,096 standalone transforms,
4,096 standalone classifications and 4,096 actual solid dispatcher cases linked
to the recovered bounds bodies. Each rejects **20 mutations**, including removal
of the maximum-product/center spills, strictness changes, lost plane/axis terms,
Boolean result collapse and premature output stores. Fixtures cover finite
bounded matrices/boxes, inverted boxes, a null world matrix, matrix and bounds
aliases, non-unit/signed normals, NaN/infinite plane components, and precision
settings `0x027f`/`0x037f`. Independent fixture blocks exercise all six plane
indices at both support boundaries, their adjacent binary32 values and both
precision settings. A separate halfway-center fixture detects the explicit
center rounding. Original instruction hooks confirm tests and early-out paths
at every plane index. Full fixture/global bytes, returns, ordered boundary calls,
queue/allocation bytes, stack cleanup, preserved registers, x87 control words
and empty x87 stack state are compared.
Reports are under `build/analysis/car-render-bounds/`.

The dispatcher chain retains controlled texture lookup and successful queue
allocation boundaries. At this checkpoint the original clip-plane producer, full view ownership,
malformed/nonfinite matrix behavior, OOM and rendering equivalence are not proved
by these finite fixtures. Instruction-byte identity is not established.

Both new bodies compile into the recovered native archive. The demo, simulator
and solid-record probe build; native SKYLINE's 60-frame orbit retains source/
record checksums `ca993eba`/`7141bcda`, and the late-texture probe retains 612 solids
and 829 textures per load across three loads, 500 → 534 → 500 matching slots and
clean buffers/handles/pools. These are layout/lifecycle regressions. The native
view had no recovered clip-plane producer at this checkpoint. The next section
recovers and activates that producer. GL still draws its display lists; native
dispatch/consumer activation and the other wheel seams remain open.

### View construction, clip planes and native focal activation (2026-10-27)

`CarRenderView.cpp` recovers the full view-matrix/plane producer `0x5c7ec0`, the
view constructor `0x491940` and storage getter `0x5bc5b0`. The original CRT
initializer at `0x77ccd0` constructs **22** view records at `0x832de0`, stride
`0x70`. Each constructor clears its complete record, restores the self-linked
head at `+0x44`, sets `+0x18 = 1`, derives the view index from the original array
base and binds `0x873380 + index * 0x120`. That storage contains three matrices
followed by six four-component planes. The complete view and storage layouts
now have size/offset assertions; no host vtable is placed in either record.

The producer's receiver is the storage, with four stack arguments: view, two
float overrides and an unused word (`RET 0x10`). It copies the finalized camera
matrix and builds the original projection, combined matrix and normalized
planes. A null camera resets only the first two matrices; combined matrix,
planes and focal remain unchanged. Main view index 1 uses far distance 15000;
other views read camera `+0xc0`. Both nonzero override arguments replace the
**near** value, with the second taking precedence. The original does not apply
the second argument as a far-distance override.

Viewport height/width is rounded to float, then types 1/2/4/5 apply the original
pixel-aspect constants. Types 9/10 use binary32 `0x3f19a956`. Quantized half-angle
conversion and the sine-result float spills precede division by the unspilled
cosine returns. Projection Y is negated; depth scale has its own float spill.
The copied camera's first three fourth-column entries are zeroed before the
combined product. Plane order is column 3 ± column 0, column 3 ± column 1,
column 2, then column 3 − column 2. Each plane is stored before normalization;
its reciprocal normal length remains extended for all four component stores.

The LOD focal value at view `+0xc` is **256 / tan(cameraFov / 2)** using the
original quantized helpers, or 10 when that tangent compares equal to zero.
It does not use the native window height or the pixel-aspect-adjusted angle.
The previous native `0.5 * height / tangentY` input has been removed.

Two scalar dependencies are recovered separately from native adapters:

| Address | Source and scope |
| --- | --- |
| `0x43c2b0` / `0x43c370` | `EngineAngleMath.cpp`: sine polynomial and quarter-turn cosine, returning the actual unspilled x87 value |
| `0x63cee1..0x63d118` | `D3DXMatrixScalar.cpp`: original scalar matrix multiply, component-specific addition order and alias handling |

The extended angle entry points are separate from the older float-returning
engine adapters; this checkpoint does not change those adapters' other callers.
The scalar library uses a temporary only for exact input/output aliases.
Distinct partial overlaps preserve its sequential direct writes. It was recovered
from the original instruction body; its entry is not defined as a function in
the current Ghidra project. No analysis-project mutation was required. The
original thunk's CPU dispatcher and optimized library variants remain separate;
the recovered native view producer explicitly uses the verified scalar body.
Existing native D3DX adapters used by other subsystems are unchanged.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_view.py --gcc --cases 2048 --angles 65536 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_view.py --cases 2048 --angles 65536 --mutation-checks
```

Each compiler passes **272,384 comparisons** and rejects **34 mutations**:
2,048 cases each for construction, storage lookup, update, scalar multiplication
and producer-to-bounds chains, plus all 65,536 input angles for both sine and
cosine under each of `0x027f`/`0x037f`. Angle returns are captured as ten raw x87
bytes before any conversion. Fixtures cover all 22 constructed indices, sentinel
record contents, view types/aspects/override precedence, odd and zero FOVs,
null cameras, exact and partial matrix aliases, and all three resulting bounds
classifications. Complete fixture/view/storage bytes, focal values, pointer
returns, stack cleanup, preserved registers, control words and empty x87 stack
are compared. The six recovered original spans and executable SHA-256 are pinned
by `car-render-view.json`; generated reports are under
`build/analysis/car-render-view/`. Instruction-byte identity is not established.

`NativeSetCarView` now constructs the original 22-record table once and supplies
entry 1 with the complete finalized Camera prefix and SDL viewport dimensions.
The native orbit camera still supplies its scaffold pose through recovered
LookAt. Every car-draw frame calls the recovered producer, so the original focal
feeds `CarLodPixelSize`/`CarLodSelect` and the view table holds live matrices and
planes. The following checkpoint recovers draw-context binding/flags
(`0x4888a0`, `0x5c26e0`). Full camera/multi-view ownership and frame/cache
scheduling (`0x5d2850`) remain open. GPU effects and
dispatch/consumer activation remain separate work; GL still draws its lists.

`UG2_CAR_VIEW_DUMP` optionally captures the first native car view for original
binary replay. The versioned 644-byte fixture contains camera, viewport, view
and produced storage; generated captures stay out of commits.

```sh
UG2_CAR_VIEW_DUMP=build/analysis/car-render-view/native.bin build/native/ug2demo --game "$UG2_DIR" --car SKYLINE --no-audio --frames 60 --orbit 90
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_view.py --gcc --cases 1 --angles 0 --kind update --native-dump build/analysis/car-render-view/native.bin
```

Both a 60-frame orbit and 120-frame scripted drive pass. Replaying their actual
native inputs through the original producer matches **all 292 output bytes**
(three matrices, six planes and focal) for each capture. Both use FOV 13018 and
focal `355.54095458984375`; storage SHA-256 values are
`b6743ed3afda3b974f95f4a5242287f79f0548b9b7f756a74cdd383aa9cc59a1` (orbit)
and `f8ef152491230c3a79aa199984985de83d276fd59b8822a8cbd52b4ae8c11a9f` (drive).
Source/record checksums remain `ca993eba`/`7141bcda` and `eb0428b7`/`69927c13`;
the orbit PPM is byte-identical before and after activation. Demo/simulator/probe
builds and the three-load late-texture probe pass with 612 solids/829 textures,
500 → 534 → 500 matching slots and clean pools/handles. The GCC bounds regression
retains 12,288 passing comparisons; the existing LOD verifier also passes its
512 producer and 2,048 selection cases. These are bounded numeric and native input/
producer checks, not complete rendering equivalence, optimized-library parity,
malformed/OOM/exception coverage or proof for another architecture.

### Scalar D3DX transpose and native camera bridge (2026-10-28)

`D3DXMatrixScalar.cpp` now recovers the default scalar transpose
`0x63d132..0x63d1a2` (113 bytes). Live Ghidra memory reads and local instruction
analysis establish its pair order and mixed x87/DWORD transfers; the body was
undefined as a Ghidra function, and no project mutation was needed.
`config/SPEED2_EXE_1_2/d3dx-transpose.json` pins the body and complete executable.
The original `0x63d12c` thunk dispatches through `0x803f00`; native
`D3DXMatrixTranspose` now forwards to this recovered scalar body.

The body swaps `(1,4)`, `(2,8)`, `(3,12)`, `(6,9)`, `(7,13)` and `(11,14)`
in that order. Both directions of the first pair pass through x87; the remaining
pairs preserve the higher-index source as a raw DWORD and convert the lower-index
source through x87. It copies diagonals 0/5/10/15 last, only for distinct pointers.
Partial overlaps therefore retain sequential reads and stores rather than a
whole-matrix snapshot. Volatile extended temporaries spell the observed x87
conversions; this is recovered behavior, not evidence of original source spelling.
Signaling NaNs passing through those conversions are quieted on native hardware.

With `UG2_EXE` set to the reference executable, run:

```sh
build/venv/bin/python tools/verify_pc_d3dx_transpose.py --gcc --native-checks --mutation-checks
build/venv/bin/python tools/verify_pc_d3dx_transpose.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/native/ug2d3dxtranspose_native_test
```

Each compiler passes **35,840 original/rebuilt comparisons**, with zero failures:
all 31 DWORD-aligned overlapping displacements, exact adjacency/separation in
both directions, arbitrary DWORD payloads, zeros, subnormals, infinities, NaNs,
two x87 precisions and four rounding modes. Checks cover the whole fixture arena,
output write bounds, returned pointer, stack cleanup, preserved registers, x87
control/exception bits and stack balance. Three structural/overlap mutations are
rejected per compiler. The GCC hardware suite additionally rejects three
conversion mutations and passes **1,920** lane/payload/alias/rounding fixtures.
The CMake-built probe uses the recovered library linked by the demo and passes
the same 1,920 hardware fixtures.

Unicorn preserves signaling NaN payloads and does not raise x87 invalid for these
loads/stores. The emulator comparisons alone do **not** establish NaN conversion
behavior. The native probe checks expected payload bits and invalid flags against
the conversion lanes read from the original instructions; it runs recovered GCC
source, not original PE code or the MSVC body. Only masked exceptions are covered;
CPU dispatch, optimized variants, byte identity and exhaustive coverage remain
unverified. This recovery supplies the transpose dependency of effect relocation
`0x6614a1`; its complete value-write chain is now [recovered separately](#effect-value-writes-relocation-and-deferred-ownership-2026-10-28).

The complete native build and 60-frame SKYLINE orbit and driving smoke runs pass.
Orbit source/record checksums remain `ca993eba`/`7141bcda`, with camera
`1696.05762 -878.670471 25.8273449` and output C0
`0.676807225 -2.48764729 1.04016507 1`. Texture records release to zero.

### Draw-context binding, flags and live scenery admission (2026-10-27)

`CarRenderContext.cpp` recovers four complete routines and one conditional fragment:

| Address | Recovery |
| --- | --- |
| `0x5c26e0` | Bind the selected eView to a separate draw context |
| `0x4888a0` | Derive context flags, bind the view, then store the flags |
| `0x5b98a0` | Read mode dword `0x8707dc` |
| `0x499db0` | Read LOD-zero byte `0x836352` |
| `0x5d65bc..0x5d65cf` | Twenty-byte instance admission gate within `0x5d6570` |

The draw context is distinct from the `0x70`-byte eView. Its checked binding
prefix preserves the unknown first `0x40` bytes, stores an eView pointer at
`+0x40` and flags at `+0x44`. This does not identify the complete context size,
its later queues or visibility-index ownership. An eView's own `+0x40` still
points to the Camera, and its `+0x44` remains its list head.

Binding retains the heading call on view-matrix elements 2/0, identity setup,
Z-rotation call and aliased scalar matrix product even though that local product
is discarded. Only the context's view pointer is written. Context setup snapshots
the mode and view type before binding, then writes flags afterwards. It preserves
incoming flags and adds `0x10` unconditionally; type 1/2 adds `0x02`, mode 3/4 adds
`0x01`, reverse value 0 adds `0x08`, reverse value 1 adds `0x04`, any `0x1800`
input adds `0x40`, and type 3 adds `0x20`. Other reverse values add neither
direction bit. No render-cache reset is introduced by either routine.

The instance gate admits exactly when
`(byte(context.flags44) & byte(instanceFlags ^ 0x60)) == 0`. It ignores the high
bytes. This fragment does not recover the rest of `0x5d6570`, which also performs
visibility, LOD selection, allocation and queueing.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_context.py --gcc --cases 4096 --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_context.py --cases 4096 --mutation-checks
```

Each compiler passes **81,920 comparisons** and rejects **25 mutations**:
4,096 per complete routine plus 65,536 admission comparisons. The latter cover
every pair of context/instance low bytes, all unsigned-short instance values
and varied higher context bits; 6,561 pairs admit and 58,975 reject. Fixtures
also cover context/view/storage aliases, arbitrary incoming flags, mode/reverse
branches, all LOD-zero byte values and x87 control words `0x027f`/`0x037f`.
Comparisons include complete fixture/global bytes, typed returns, heading and
rotation arguments, matrix input bytes/aliases, stack cleanup, preserved registers
and x87 control/empty-stack state for full function calls. Rotation angles are
compared by their consumed low 16 bits. The instruction fragment is checked by
its branch destination and memory effects, not by a fabricated function ABI.

The original heading and Z-rotation bodies execute on both sides of this
verifier. The rebuilt binding executes the recovered scalar matrix body; this
checkpoint does not extend numeric claims for the older native heading/rotation
adapters. Seeded texture/technique/UV/world/view/vertex/index caches are preserved;
a mutation adding a texture-cache clear is rejected. Complete original spans,
the fragment and executable SHA-256 are pinned by `car-render-context.json`.
Reports are generated under `build/analysis/car-render-context/`. Instruction-byte
identity, arbitrary malformed/OOM/exception behavior and full renderer equivalence
are not established.

Native frame setup now runs before both scenery and car draws, including views
where the car is hidden. Recovered context setup selects original view-table
entry 1 and currently produces flags **0x1a** with mode/reverse zero. Scenery
checks the recovered admission gate each frame, including sky and debug picking.
Loading excludes only the `0x10` cases that every recovered context rejects;
other candidates remain available for dynamic flags. L4RA retains 42,270
candidates from 48,270 instances and admits 42,192 under the current context.
The two LOD getters now read their original mapped globals instead of returning
constant zero; their full game-state producers remain open.

`UG2_CAR_CONTEXT_DUMP` optionally writes a versioned **489-byte** capture of the
view, clip storage, context prefix and mode/reverse/LOD-zero inputs. The verifier
remaps pointers and replays the original setup, comparing all **72 context bytes**.

```sh
UG2_CAR_CONTEXT_DUMP=build/analysis/car-render-context/native.bin build/native/ug2demo --game "$UG2_DIR" --car SKYLINE --no-audio --frames 60 --orbit 90
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_context.py --gcc --cases 1 --kind setup --native-dump build/analysis/car-render-context/native.bin
```

Orbit and scripted-drive captures replay exactly. Their source/record checksums
remain `ca993eba`/`7141bcda` and `eb0428b7`/`69927c13`; the 60-frame orbit PPM is
byte-identical before and after this activation. The separate view capture still
matches all 292 matrix/plane/focal bytes against the original producer. Demo,
simulator and record-probe builds pass; three late-texture load cycles retain
612 solids/829 textures, 500 → 534 → 500 matching slots and clean handles/pools.
Full camera/multi-view ownership, frame scheduling, effect/API resources and
native GPU dispatch/consumer activation remain open, along with the other wheel
inventory rows and the separate legacy coefficient-transform precision gap.

### Parameter-map lifetime and bucket-vector recovery (2026-10-27)

`CarRenderTechniqueMap.cpp` recovers the normal control flow of fourteen UG2
library bodies used by technique parameter construction, reset and destruction:

| Address | Recovered behavior |
| --- | --- |
| `0x5d0bb0` | Construct the map, sentinel and nine-pointer bucket vector |
| `0x5d0660` | Free vector storage, erase entries, free the sentinel and clear ownership |
| `0x5d0f20` | Reset map entries/buckets without changing the 60 parameter slots |
| `0x5cf140` | Erase a list range, skipping the sentinel |
| `0x5ce070` | Allocate the sentinel and set its forward/back links |
| `0x5d0250` | Initialize the vector's live range |
| `0x5d0c50` | Capture a fill value, clear the live range and assign new entries |
| `0x5d0890` | Insert a repeated value, growing or shifting vector storage |
| `0x5cf190`, `0x5cfbc0` | Allocator copy/fill wrappers |
| `0x5ca630`, `0x5ce0f0` | Forward copy and counted fill |
| `0x5ce0c0`, `0x5ca610` | Backward copy and range fill |

The checked subobject layouts start at map `+4` (list) and `+0x10` (vector).
Construction copies only the first byte from the first tag argument; the second
tag and other opaque allocator bytes are untouched. Sentinel key/value bytes are
also untouched. This is the UG2 layout; another title has not confirmed sharing.

Vector insertion snapshots the supplied value before moving entries, including
when that pointer aliases vector storage or the vector's end pointer. Growth
uses the original capacity plus half its capacity, raised to the required size,
with the original unsigned limit checks. The two in-place paths preserve the
original forward/backward copy boundaries and fill order. These are explicit
library bodies rather than calls to the host STL.

Reset erases list nodes, assigns nine sentinel pointers, and restores mask and
bucket count to one. It preserves the allocation capacity and all static parameter
slots, including stale handles. Destruction frees bucket storage first, clears
its three pointers, unlinks/frees entries, then frees/clears the sentinel and
count. Unknown tag/allocator fields and mask/bucket words are not zeroed by
destruction. List count decrements follow each free.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique_map.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique_map.py --mutation-checks
```

At the initial lifetime checkpoint, each compiler passed **7,680 comparisons**:
512 per body and 512 lifetime cycles. The commands above now run the expanded
hash-insertion verifier described in the following section.
The cycles construct a real map, perform three batches of original hash insertions
with duplicate/colliding keys, reset and repopulate it, then destroy it twice.
All allocations were released. Hash insertion `0x5d0cf0` executed original PE code
on both sides at that checkpoint; its later recovery is described below.
The vector-insertion cases include 18 oversized-vector handoffs, 84 zero-count
operations, 137 growth cases, and 93/180 cases for the two in-place tail paths.
Other cases include aliased and overlapping copies, partial ranges crossing the
sentinel, null heads, retained capacity, arbitrary opaque bytes and cached slots.

Comparisons include full fixture/heap bytes, meaningful returns, allocation/free
order and object/heap state at each allocator call. Normal returns check stack
cleanup, preserved registers, x87 state and the Windows exception-list slot.
Both compilers reject **22 mutations**, covering tags/opaque bytes, links/counts,
growth, aliased fill capture, copy direction, cleanup order, cached slots and
reset state. Complete contiguous original spans are fingerprinted in
`car-render-technique-map.json`; reports are under
`build/analysis/car-render-technique-map/`.

Allocation is controlled and successful in bounded fixtures. The oversized-vector
path compares only the handoff to `0x5d07e0`, exposed as
`UG2TechniqueVectorTooLong`; exception-object construction, Windows unwinding,
allocator failure and malformed-container behavior remain outside the evidence.
The original sentinel allocator does not safely handle a null allocation.
Instruction-byte matching and exhaustive input coverage are not established.

The source compiles into the recovered native archive. Native effects remain
inactive, so this checkpoint does not activate map-backed shader setup or claim
rendering equivalence. Demo/simulator/solid-probe builds pass, the 60-frame SKYLINE
orbit preserves source/record checksums `ca993eba`/`7141bcda`, and three late-texture
load/unload cycles retain 612 solids/829 textures with clean final ownership.
The existing technique verifier still tests its controlled map boundary; its GCC
regression passes 12,416 comparisons. Hash insertion and splitting are recovered
in the following checkpoint. Effect resources/API adapters, full frame scheduling
and native dispatch/consumer activation remain open.

### Parameter-map insertion, splitting and linked binding (2026-10-27)

`CarRenderTechniqueMap.cpp` now also recovers five normal-control-flow bodies:

| Address | Recovered behavior |
| --- | --- |
| `0x5d0cf0` | Linear-hash insertion, duplicate lookup and one-bucket splitting |
| `0x5ce090` | Allocate and populate an entry node after writing its links |
| `0x5d06c0` | Check the unsigned list-length limit, then increment count |
| `0x5d0b30` | Resize the bucket vector through insertion or range erasure |
| `0x5d02d0` | Erase a vector range by forward-copying its suffix |

Insertion splits before looking up the key when `buckets24 <= (count0C >> 2)`.
Consequently a duplicate can split a bucket without increasing the entry count.
Mask expansion, bucket-vector growth, propagation of shared bucket boundaries
and movement of high-bucket nodes retain the original statement and call order.
A high-bucket node already at the list tail takes the original early break.
The unreachable self-list count-adjustment block at `0x5d0e08` is not fabricated
in the recovered source; the original `CMP ECX,ECX` always skips it.

Lookup masks/folds the unsigned key, searches backwards within its bucket, and
uses unsigned comparisons for ordering. A duplicate returns the existing node
with a zero insertion byte and preserves its value. A new entry is allocated
before the length guard/count increment, linked into the list, and propagated
through preceding shared bucket boundaries. Only the result pointer and byte
are written; its three padding bytes remain untouched. Node key/value loads
follow allocation and the link writes, including overlapping pair/node fixtures.

The expanded `verify_pc_car_render_technique_map.py` compares all nineteen map,
list and vector bodies and two linked chains. Rebuilt lifetime cycles execute
the recovered insertion and every direct container callee. The new binding chain
also executes recovered `0x5d1b70` against the real map, checking first matching
static slots, unknown/duplicate hashes, handle updates, reset and repopulation.
It links `CarRenderTechnique.cpp` and the string library; effect APIs are never
called by this chain. The older technique/setup verifier retains its isolated
controlled-map fixtures, so its evidence is distinct from these linked checks.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique_map.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_car_render_technique_map.py --mutation-checks
```

Each compiler passes **10,752 comparisons**: 512 per body, 512 lifetime cycles
and 512 binding chains. Complete map/fixture/heap bytes and allocator observations
are compared, with a state trace after every insertion/bind in the linked chains.
Independent invariants check list links/counts, unique node ownership, contiguous
bucket ranges, unsigned key order and masked/folded bucket membership. Fixtures
include ascending/descending keys, heavy collisions, repeated keys, both sides
of the signed-key boundary, terminal/nonterminal high-node splits and retained
capacity after reset. All linked cycles release their allocations.
Original instruction traces include 33,728 splits, 2,628 bucket-vector growths
and 64 terminal high-node cases. Explicit hash-call fixtures record 360 duplicates
that also split a bucket; nested parameter binds are included in the instruction
counts rather than in that explicit-call result counter.

Both compilers reject **48 mutations**. The 26 additions exercise node copy order,
list limits, vector erasure/resizing, load threshold, mask/vector growth, split
boundaries and splicing, duplicate-value preservation, unsigned ordering, count
versus allocation order, result pointers and untouched padding. The manifest now
fingerprints the five additional spans and the previously recovered binding body.
Reports remain under `build/analysis/car-render-technique-map/`.

List overflow compares the handoff at `0x5d06ec`, represented by
`UG2TechniqueListTooLong`; the inline Windows exception-object construction and
unwinding are still a runtime boundary. The existing vector exception boundary
is unchanged. Allocation failure, malformed containers, exhaustive coverage and
instruction-byte identity are not established. Native demo/simulator/probe builds
and SKYLINE orbit/lifecycle checks pass. These bodies compile into the recovered
archive; native effect setup, shader execution and GPU dispatch remain inactive.

### Effect resource input, byte buffers and native ownership (2026-10-27)

`Libs/D3DX/EffectInput.cpp` recovers fifteen routines from UG2's statically linked
library. They provide the resource/byte-buffer inputs to effect creation; they
do not deserialize effects or execute shaders.

| Address | Recovered behavior |
| --- | --- |
| `0x670f13`, `0x670f20`, `0x670f2e` | Construct, destroy and clear the 12-byte resource record |
| `0x670f4f` | Load an ANSI/UTF-16 resource, optionally trying bitmap before raw data |
| `0x670e8e`, `0x670da2`, `0x670e24` | Construct, destroy and conditionally free the 16-byte byte buffer |
| `0x670db2` | Allocate buffer storage and update size only on success |
| `0x670e45`, `0x670dde`, `0x670df0` | Interface query, reference increment and release |
| `0x6954f7`, `0x670e15` | Return buffer pointer and byte length |
| `0x670ea6`, `0x670f08` | Create a buffer and forward the public creation entry |

The resource record retains the original partial-state behavior. Loading clears
old fields only when its old handle is nonzero, selects ANSI/UTF-16 by a nonzero
flag, and optionally tries resource type 2 before type 10. A found bitmap with
zero size does not trigger another raw-data search. Failed find/size/load/lock
steps leave the corresponding stale or partial fields intact, query last error,
and return `0x88760b59`. Destruction calls `DeleteObject` without clearing fields;
Clear calls it before zeroing all three words.

Buffers retain original class token `0x7b711c`, a wrapping unsigned reference
count, a byte size and an allocation pointer. Interface queries clear output
before comparing either full 16-byte identifier, then write the object pointer
before the virtual AddRef call. Release decrements first and deletes only when
the old count was one. Destruction reinstalls the class token and frees storage
without clearing its dangling pointer; deleting flags bit zero controls the
object free. Creation preserves the caller's output on invalid/output-allocation
failure paths and releases a failed initialized object in the original order.
Reallocation overwrites the data pointer without freeing earlier storage, as in
the original allocation method; creation uses a newly constructed buffer.
Native virtual dispatch is a separate bridge for this class token.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_input.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_input.py --mutation-checks
```

Each compiler passes **8,238 comparisons**: 512 per routine, 512 linked resource/
buffer cycles, and 46 cycles using all 23 retail effect payloads through both
lookup modes. Both reject **25 mutations**. Full fixture/heap bytes, allocation
and resource-call observations, returns, stack cleanup, preserved registers and
x87 state are compared. Cases cover bitmap/fallback selection, failed resource
steps, callback writes during deletion, both identifiers, output/IID/reference
aliases, unsigned refcount wrap, flags, null output, zero sizes and forced first/
second allocation failures. Win32 calls and allocation outcomes are controlled;
recovered direct/virtual buffer bodies execute. The executable, full function
spans, vtable and identifiers are pinned in `effect-input.json`; reports are
under `build/analysis/effect-input/`. Byte identity and arbitrary malformed/
exception behavior are not established.

`OriginalImage.cpp` now retains the PE resources from the same SHA-verified player
image. `NativeFxResource.cpp` supplies bounded current-module lookup for exact
ASCII names (ANSI or UTF-16), numeric identifiers and `#number` identifiers.
The reference image has numeric types and one language per resource; general
module/language/codepage/GDI behavior is outside this adapter. Loaded resource
bytes remain immutable and retained with the module. Resource handles are not
ordinary global-memory allocations ([Microsoft LoadResource contract](https://learn.microsoft.com/windows/win32/api/libloaderapi/nf-libloaderapi-loadresource)).
The recovered DeleteObject call is preserved; the native adapter returns false
for resource handles, which are not supported GDI objects ([Microsoft DeleteObject contract](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deleteobject)).

```sh
cmake -S . -B build/native
cmake --build build/native --target ug2effectinput_probe
build/native/ug2effectinput_probe "$UG2_DIR/speed2.exe" build/analysis/effect-input/native.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_input.py --gcc --cases 1 --kind chain --native-dump build/analysis/effect-input/native.bin
```

The display-free native probe follows all 23 original recipe resource names,
loads each through ANSI and UTF-16 paths, creates/copies/queries/releases real
recovered byte buffers, reloads/clears resource records and checks numeric lookup.
All **429,768 captured payload bytes** match an independent PE read exactly,
with **zero live buffer allocations** after 46 cycles. This is input/ownership
evidence, not an effect or rendered-image match. No retail payload is committed.

Retaining resources exposed a pre-existing collision-centre dispatch error:
the old helper read `body20` directly even while car `+0x34` held a 12-byte null
mover. Reused resource-name bytes beyond that object became a bogus body pointer.
`CollisionCentre.cpp` restores original `0x5ed400`'s two virtual slot-`0x0c` calls,
including re-reading the mover between them and reading the record fallback
after a failed query. Native null/PhysicsMover slots now supply the original
zero (`0x50d510`) or `body20` (`0x503b00`) result.
`verify_pc_collision_centre.py` passes **4,096 comparisons** and rejects **five
mutations** per compiler: 1,024 zero-call, 1,317 one-call and 1,755 two-call cases,
including poisoned null-mover padding, callback mover changes and fallback-record
changes. The real null-return leaf executes; other virtual callback effects are
controlled. `collision-centre.json` fingerprints the centre and null-return bodies.

Demo/simulator/solid-probe builds pass. Native SKYLINE orbit and scripted drive
preserve source/record checksums `ca993eba`/`7141bcda` and `eb0428b7`/`69927c13`;
the 120-frame display-free simulator smoke also passes. The
three late-texture load/unload cycles retain 612 solids/829 textures and finish
with clean ownership. The following checkpoint recovers effect construction,
factory control flow and pool lifetime. Deserialization `0x66f69c`, runtime setup
`0x66afe6`, full effect destruction, the API/GPU backend and native technique/
consumer activation remain open. The ramp's zero mip-level request
also remains unsupported by native CPU texture staging.

### Effect constructors, resource factory and shared-pool lifetime (2026-10-27)

`Libs/D3DX/EffectObject.cpp`, `EffectFactory.cpp` and `EffectPool.cpp` recover
sixteen complete constructor/controller/helper bodies:

| Address | Recovered behavior |
| --- | --- |
| `0x664ecf`, `0x65eb78` | Initialize the parameter record and value prefix |
| `0x66509e` | Construct the `0x130`-byte effect object |
| `0x65b8c6`, `0x66c361`, `0x66afc5` | Effect AddRef, Release and scalar deleting wrapper |
| `0x640c02` | Complete resource-factory control flow, compiled copy and compiler path |
| `0x64285f`, `0x640aac` | Construct/create the `0x20`-byte effect pool |
| `0x642556`, `0x642883` | Pool interface query and Release |
| `0x642504`, `0x642535` | Pool destruction and conditional object free |
| `0x641837`, `0x641878`, `0x641816` | Embedded-link cleanup and linked-node destruction/deletion |

Effect construction preserves unknown bytes and fields. It installs class tokens
`0x7b6fe0`/`0x7b6e00`, calls the parameter/value constructors at `+0x48`/`+0xe0`,
masks only the low **word** at `+0x118` with `0xe478`, performs the original
ordered clears and initializes reference count `+0x24` to one. The high flag word,
the halfword at `+0x32` and unspecified parameter/value fields are retained.
The checked source layout names unknown fields by offset. Effect Release wraps
the decrement and invokes the recovered deleting wrapper only at zero; that
wrapper delegates the full destructor `0x66adec` to a controlled boundary in the
original factory suite. The next checkpoint below recovers that destructor and
adds a factory mode that executes it.

The factory clears output and diagnostics before validating device/name, allocates
and constructs the effect, then runs the recovered resource loader. A resource
shorter than four bytes returns `0x80004005`. Signature `0xfeff0901` selects a
byte-for-byte copy into a recovered buffer, with dword copies followed by the
remaining bytes. In this branch a null effect output returns the loader's result
without allocating that buffer or invoking deserialization.

Other signatures clear the resource before calling the source compiler. A
successful compiler creation releases and clears an earlier diagnostic buffer;
compilation receives flags masked with `0xffffffe7`. Its later diagnostics remain
owned by the caller. Deserialization receives a zero cursor and the shared pool.
The factory publishes the object and adds its reference only after both
deserialization and runtime setup return nonnegative results. Cleanup retains
the original compiler → buffer → effect → resource order, including partial
outputs and failures. Positive success codes are preserved.

Pool queries compare the pool identifier and IUnknown identifier and leave output
untouched on failure. Pool AddRef uses original entry `0x670dde`, shared with the
byte-buffer class. Destruction frees `+0x10` storage before the `+0x1c` node chain.
Each node detaches its `+0x20` successor before recursive deletion. Disassembly
at `0x641899` confirms that link cleanup receives **node +0x0c**; its data/neighbor
fields are relative to that embedded link. The recovered code preserves this
adjustment, the free-before-unlink order and flags-bit-zero deletion behavior.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --mutation-checks
```

Each compiler passes **8,727 comparisons**: 512 per body, 512 linked factory/
caller-cleanup cases and 23 retail-payload factory cases. The factory fixtures
include 256 cases each for randomized inputs, compiled allocation paths, compiled
runtime outcomes and source-compiler outcomes, plus the 23 retail cases. Both
compilers reject **38 mutations**, including whole-object zeroing, widened flag
clears, reference offsets, failed-query output writes, the embedded-link offset,
node detachment, pre-clears, signatures/short inputs, compiler flags, diagnostic
ownership, copy tails, cursor initialization, early publication and release order.

Full fixture/heap bytes, allocator-time object/link state, returns, dependency
arguments and ownership, ABI and x87 state are compared. The controlled
deserializer observes all copied bytes; it may retain the input buffer and pool,
including on failure. The linked caller cleanup releases all modeled ownership.
The recovered resource/buffer bodies, constructors, references and pool/node
cleanup execute throughout. Compiler creation/compilation/release, deserialization,
runtime setup and the full effect destructor are controlled dependencies. These
checks establish factory input/copy/ownership behavior under those dependencies;
they do not establish real effect parsing, shader creation or rendering parity.
Function/vtable/identifier fingerprints are in `effect-factory.json`, and reports
are under `build/analysis/effect-factory/`. Finite fixtures, class identities and
forced allocation outcomes do not establish malformed-pointer, exception or
instruction-byte equivalence.

The native input probe now also creates/queries/releases real recovered pools
through **24 cycles**, including a synthetic three-node cleanup graph. All pool,
node and data allocations are released, and the sentinel links return to self.
The existing 23-recipe/46-cycle capture still matches all **429,768 bytes** against
the player's executable with zero live allocations. Native pool code uses a
separate ABI bridge for its original class token. Effect construction and
destruction are exercised natively in the next checkpoint below.

Demo, simulator and both probes build successfully. Native SKYLINE orbit retains
source/record checksums `ca993eba`/`7141bcda`; three late-texture load/unload cycles
retain 612 solids/829 textures, matching slots 500 → 534 → 500, and clean ownership.
The effect factory/constructor code compiles into the recovered archive. Native
factory/technique activation still needs its deserializer, runtime setup and
API/GPU backend; source compilation remains an explicit
library boundary. The other wheel-rendering inventory seams remain open.

### Effect destruction, metadata ownership and native cleanup (2026-10-27)

`Libs/D3DX/EffectDestruction.cpp` recovers the full effect destructor and its
cleanup dependencies. Record names and unknown fields describe observed layouts;
no undocumented D3DX semantic types are inferred.

| Address | Recovered behavior |
| --- | --- |
| `0x66adec`, `0x65b7f0`, `0x65e4ff` | Full effect cleanup, temporary pointer grids and secondary interface cleanup |
| `0x663e90`, `0x664efa`, `0x663fcf` | Common base, parameter and value-prefix destruction |
| `0x65ee40`, `0x65b7ad` | Type-gated references in 8-byte and 24-byte records |
| `0x663f15`, `0x66a419`, `0x665031`, `0x66a442` | Cleanup of 0x64-byte records/groups, 0x38-byte records and 0xb4-byte records |
| `0x64b5ff`, `0x6996f4` | Reverse array iterator and shared empty destructor |
| `0x65b6a8`, `0x664f42`, `0x66519e`, `0x6424e3` | Scalar deleting wrappers for base, parameter, value and pool-link records |
| `0x661405`, `0x66238c`, `0x661453`, `0x6651bf`, `0x664f63`, `0x66a4d5`, `0x665050`, `0x66a523`, `0x660f55` | Nine vector deleting wrappers with original strides/cookies |

The array iterator preserves 32-bit wrapped address multiplication and the signed
`count-1` guard. Deleting wrappers inspect the low flag byte: bit `0x02` selects
cookie-counted array destruction in reverse order, and bit `0x01` frees storage.
The array path returns the cookie address; the scalar path returns its receiver.
One-argument fastcall provides the original callbacks' ECX receiver and no stack
arguments without assembly or member-function pointer casts.

Disassembly at `0x663f45` passes **record +8** to common-base cleanup. That record
clears its `+0x28`/`+0x2c` fields before releasing its `+0x38` COM reference. The
parameter and other record bases use offset zero. Effect cleanup passes `+0xe0`,
`+0x48` and `+4` to the embedded value, parameter and secondary-interface bodies.
Linked children are detached before destruction; parameter pool links execute
the already recovered free/unlink helper. Nested arrays execute every recovered
cleanup body, rather than replacing metadata lifetime with a bulk free.

Signed type codes `4..9`, `15` and `16` own the first COM reference in the two
reference-record layouts. The 24-byte record's second reference is independent
of that gate. Value-prefix data is freed for codes `0`, `2`, `4` and `6..10`.
Other type codes retain that field. Cleanup preserves the original observable
ordering, including COM fields `+0x88`, `+0x94`, `+0x8c`, `+0x98`, `+0x90` in the
0xb4-byte record, and secondary fields `+0x0c` before `+8` relative to its receiver.
Temporary grids retain their freed pointer slots and the final scratch row in
`effect+0xb8`. Unconditional frees of `+0xbc` and `+0x108` leave those fields
unchanged; the source deliberately does not clear every freed pointer.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_destruction.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_destruction.py --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --gcc --real-destroy
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --real-destroy
```

Each compiler passes **13,824 comparisons** across 27 complete bodies and rejects
**28 mutations**. Fixtures compare complete arena/heap bytes, state at external
calls, free/release order, returned addresses, live allocation maps, stack,
nonvolatile registers and x87 state. They cover bounded nested/linked records,
unknown bytes, signed type gates, deletion flags, zero/short arrays, and high-bit
counts whose signed guard skips iteration. Controlled callbacks change a later
COM reference, a temporary-grid stride, or the iterator's stored cookie to check
selected rereads and retained loop state. The suite compares 97,174 free events,
12,906 COM releases and 938 iterator callback events per compiler. Counts which
would require billions of iterations, malformed graphs, exceptions and arbitrary
callback mutations remain outside this evidence. Allocation/free and COM Release
are controlled; every listed cleanup body executes original or recovered code.

The additional factory mode passes **535 ownership chains per compiler**, including
23 retail payloads. It executes the real destructor, buffer/pool lifetimes and
native COM class dispatch; compiler creation/compilation/release, deserialization
and runtime setup remain controlled. The original factory regression also passes
8,727 comparisons per compiler. These are behavioral checks, not instruction-byte
identity or evidence that real effect payloads have been parsed. Fingerprints are
in `config/SPEED2_EXE_1_2/effect-destruction.json`; generated reports remain under
`build/analysis/effect-destruction/` and `build/analysis/effect-factory/`.

The native input probe now runs **24 outer-effect constructor/release cycles**,
including synthetic nested effects, parameter/pool links, record arrays and shared
buffer/pool references. All allocations return to zero and sentinel links return
to self. This exercises the actual recovered constructors/destructors without
fabricating shader results. `NativeFxDestroyDispatch.cpp` adapts the factory ABI;
`NativeFxComDispatch.cpp` dispatches only the known recovered buffer, pool and
effect classes and traps unsupported GPU/device classes. The existing 46 input
cycles still replay all **429,768 bytes** exactly.

Demo/simulator/probes build successfully. The native SKYLINE orbit retains
checksums `ca993eba`/`7141bcda` and its prior camera/output values. Three late-texture
load/unload cycles retain 612 solids/829 textures, slots 500 → 534 → 500 and clean
ownership. The following checkpoint recovers runtime setup `0x66afe6`. Native
factory/technique activation still requires deserialization `0x66f69c`, shader
preparation/evaluation, source compilation support and the GPU/API backend. No
shader or draw activation is claimed by this checkpoint.

### Effect runtime setup and recursive reference traversal (2026-10-27)

`Libs/D3DX/EffectSetup.cpp` recovers three complete normal-control-flow bodies:

| Address | Recovered behavior |
| --- | --- |
| `0x66afe6` (1,082 bytes) | Build per-technique/per-parameter reference grids |
| `0x65ae25` (236 bytes) | Adjust two group counters and recursively adjust referenced groups |
| `0x66aa2f` (488 bytes) | Prepare selected records, evaluate selectors and recursively resolve references |

Setup sets byte `effect+0x119` bit `0x08`, allocates both `rows × parameters × 4`
pointer grids and a `parameters`-byte array, then checks all three results. It
zeros only the byte array. The row index occupies the low word at `+0x40`; its
high word is retained. For each row, setup increments the groups named by its
pass records and calls the recovered resolver. **The resolver's result is ignored
by this caller.** It then visits each parameter's circular use ring, beginning
with `head->previous18->next1C`, and classifies uses whose group counter `+0x3a`
is nonzero by group flag `+8` bit `0x08`.

The two temporary lists grow with capacities `2 * old + 1` and retain capacity
across parameters and rows. Uses with the flag clear go into a persistent array followed
by `secondCount + 1` zero words. Uses with the flag set also get a separate array
with one zero terminator. Empty entries are null. Rows finish by reversing their
initial counter increments. Successful traversal and row-allocation failure set
the row word to `0xffff`; failure of an initial grid/byte allocation retains its
prior value. Both paths free temporary lists in the original order and clear the
setup flag. Partial grids, unknown bytes and uninitialized slots are preserved;
no added clearing or invented failure recovery masks the binary's behavior.

Reference adjustment sign-extends the low 16 bits of each delta. A counter is
written only when its unsigned-word value plus that signed delta is nonnegative;
positive overflow still wraps when stored to a word. The routine follows the
record-index table and the pair at each record's `+0x48` offset. A zero descriptor
element count means one element. The second delta recurses over all referenced
elements; the first delta recurses to the selected element. Index arithmetic and
the original 16-bit loop narrowing are retained.

Resolution tests selected records with tags `0x0a000000` and `0x0b000000`, adjusts
counters around their preparation calls, and preserves negative return codes.
It walks reference records, copies a dirty record's `+4` value into `+0` before
evaluation, calls the record's buffer-pointer interface, then reads the selector
double through that returned description's `+0x0c` offset. It adds the original
**0.001** double at `0x7a1e58`, truncates through a signed 64-bit integer and retains
its low word. An unchanged selector bypasses the bounds check, matching the
binary. Changed selectors must be below the effective element count. Recursive
resolution preserves the original return propagation and evaluation order.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_setup.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_setup.py --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --gcc --real-setup
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_factory.py --real-setup
```

Each compiler passes **6,144 comparisons**: 1,024 fixtures per body under each
x87 control word (`0x027f`, `0x037f`). Both reject **31 mutations**, covering setup
flags/word width, allocation sizes/order, partial failure state, row strides,
classification, list growth/copies/terminators, count restoration, signed deltas,
recursive defaults, preparation/evaluation failures, dirty clearing and selector
bias/bounds behavior. Comparisons include complete arena/heap bytes, state at
all external calls, allocation/free order, returns, live allocations, stack,
nonvolatile registers and restored x87 control/tag state. Per compiler, fixtures
observe 17,278 allocation requests, 7,668 frees, 1,442 preparation-A calls, 1,052
preparation-B calls and 1,382 evaluations.

Preparation `0x666df7`/`0x667d51` and evaluation `0x663a32` remain controlled
boundaries in this suite. Buffer-pointer access and all three recovered bodies
execute source. The original float-to-integer helper `0x75dd28` executes on the
retail side and as the MSVC analysis runtime dependency; GCC uses its compiled
conversion. Finite in-range doubles include conversion boundaries, negative
values and values whose low integer word wraps. NaN/out-of-range conversion,
exception unwinding, malformed pointers, cyclic group dependencies and loops
requiring 65,536 or more element iterations remain unestablished. These checks
establish bounded behavior, not byte identity or real shader preparation.
Fingerprints, including the bias and conversion dependency, are in
`config/SPEED2_EXE_1_2/effect-setup.json`; generated reports are under
`build/analysis/effect-setup/`.

The factory's `--real-setup` mode executes recovered setup and destruction in
**535 chains per compiler**, including all 23 retail payload copies. Its controlled
deserializer explicitly supplies empty count metadata. This checks linking,
allocation and caller ownership; it does not parse those payloads or construct
shaders. The normal controlled-runtime factory regression remains available.

`ug2effectinput_probe` runs a separate native synthetic fixture with two rows,
three parameters and four circular uses. It checks every grid entry, padding,
terminator, byte-array clear, row sentinel/high word, restored counters and full
recovered destruction to **zero allocations**. The existing 24 effect cleanup
cycles and 46 input cycles still pass; all **429,768 input bytes** replay exactly.
`NativeFxSetupDispatch.cpp` provides the native ABI bridge and the original bias.
Unsupported preparation/evaluation calls trap explicitly; the fixture takes only
metadata paths whose required routines are recovered.

Demo, simulator and both probes build successfully. Native SKYLINE orbit checksums
remain `ca993eba`/`7141bcda`; the three late-texture lifecycle cycles retain 612
solids/829 textures, slots 500 → 534 → 500 and clean ownership. Native effect/
technique activation still requires deserialization `0x66f69c`, shader preparation
`0x666df7`/`0x667d51`, evaluation `0x663a32`, source compilation and the GPU/API
backend. The recovered setup is currently exercised through explicit metadata
fixtures rather than real effect activation.

### Effect type validation, metadata construction and size walks (2026-10-27)

`Libs/D3DX/EffectMetadata.cpp` and `EffectTypeValidation.cpp` recover fifteen
complete constructor/helper bodies used by the compiled-effect deserializer:

| Address | Recovered behavior |
| --- | --- |
| `0x401650` | Forward array construction through ECX callbacks |
| `0x65b680`, `0x65b694`, `0x65b6c9`, `0x65b78d` | Use-link, base, group and resource-reference constructors |
| `0x663ec5`, `0x663f4e`, `0x663f6c` | 0x64-byte record, 0x38-byte pass and 0xb4-byte technique constructors |
| `0x664fb1` | Grow/reuse a group's record array |
| `0x65bc5e`, `0x65bcba` | Validate a length-prefixed string and parameter name |
| `0x65bf18`, `0x65c033` | Walk padded storage sizes and actual value sizes |
| `0x65bd1e` | Populate/splice a use node and invalidate temporary grids |
| `0x66548b` | Validate a type and construct its runtime metadata |

Constructors preserve unspecified bytes, original bit masks and halfword stores.
Use links start self-linked and leave unknown payload words intact. The forward
array iterator retains the signed `count-1` guard and wrapped 32-bit address
steps. Group growth compares low-word capacity with the requested low-word count,
allocates and constructs a fresh cookie-counted array, then deletes the old array.
Failure retains the old pointer/capacity/active count; shrinking changes only the
active count. No vector copy or invented capacity policy replaces that behavior.

String validation uses the original wrapped absolute-address checks. A zero
length is accepted; nonzero lengths require a final NUL. Name validation requires
a length of at least two and rejects `.`, `@`, `[` and `]` before the first NUL.
The two recursive size walks remain distinct. Padded scalar/vector entries use
16 bytes per element and matrix entries use 64, while actual values use the
recorded dimensions times four. Object descriptors consume four data bytes per
element, but types 10 through 14 contribute zero to the returned storage size.
Structured descriptors walk their members once and scale the data extent/size
for the element count. Cursor write order and wrapping arithmetic are preserved,
including bounded fixtures with aliased size cursors.

Type validation checks names/semantics, dimensions and nested structures, then
performs the original metadata side effects. Types 10 through 14 reserve group
slots and construct table-driven numeric/resource records. Types 15/16 claim
resource references, create records with tags `0x0b000000`/`0x0a000000`, copy the
parameter prefix, allocate use nodes and link their parameter rings. Record/use
counts, flags, tags and pointers are written in the original order, including
partial writes before failure. The 182-entry type table remains pinned original
data at `0x7b582c`; it is not copied into the repository as invented declarations.
Record names and remaining fields continue to use observed sizes/offsets.

Two unusual ordering details are retained. Numeric extent checks compare the
calculated byte count with the **descriptor start**, rather than the data cursor.
The table lookup and record writes also precede the upper bound check on a
record code. Fixtures preserve these behaviors instead of adding stronger checks.
Use linking appends at the tail while retaining the ring head. It marks a dirty
use and, except for the two shader tags, the record's secondary dirty word.
Outside setup, an existing first grid triggers the recovered temporary cleanup.
No replacement ring or grid algorithm is introduced.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_metadata.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_metadata.py --mutation-checks
build/native/ug2effectinput_probe "$UG2_DIR/speed2.exe" /tmp/effect-input.bin /tmp/effect-metadata.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_metadata.py --gcc --cases 1 --kind validate --native-metadata /tmp/effect-metadata.bin
```

Each compiler passes **15,589 comparisons**: 1,024 fixtures per recovered body
plus **229 retail scalar descriptors** extracted independently from all 23 effect
recipes. Both reject **40 mutations** spanning partial constructor stores, flag
masks, array order/guards, growth/retirement, strings/names, size distinctions,
ring links/dirty flags, setup gating, descriptor extents, resource ownership,
lazy records, prefix copies, allocation sizes and the original storage-offset
arithmetic. Failure positions vary independently of descriptor families, including
record allocation and lazy-use allocation failures. Explicit fixtures cover
zero-element defaults and the descriptor-relative extent check.

The verifier compares complete arena/heap bytes, allocation/free-time state,
cursors/results, live allocation maps, stack, nonvolatile registers and x87 state.
All validation, constructors, growth, use linking, buffer/record lifetime and
temporary cleanup dependencies execute original or recovered bodies; only the
allocator/free boundary and the standalone generic iterator callback are
controlled. Constructors are checked for normal completion/stores rather than
unspecified EAX values. Function/table fingerprints are in
`config/SPEED2_EXE_1_2/effect-metadata.json`; reports remain under
`build/analysis/effect-metadata/`. This is bounded behavioral evidence, not byte
identity, malformed-pointer safety, exception unwinding or proof for huge/cyclic
descriptor graphs.

The native probe selects the same 229 scalar descriptors using a diagnostic
outer-header reader. That reader only selects test fixtures; it is not a game
replacement for `0x66f69c`. `UG2FXMD1` captures recipe/parameter identity, input
cursors, result, output cursors and source-blob hashes. Replay checks every result
and cursor against the original retail validator and the rebuilt source. All
229 native records match across 23 recipes, and the existing 46 resource captures
still replay all **429,768 bytes** exactly.

Two further native synthetic fixtures exercise metadata creation and full
recovered destruction: two lazy records with a circular parameter-use ring, and
a group containing table-driven numeric/resource records. They check flags,
indices, ring neighbors, counts and the original unrounded storage offsets,
then return to **zero allocations**. They create no GPU objects or shader result.
The prior setup, effect/pool cleanup probes remain successful.

Demo, simulator and both probes build successfully. Native orbit retains
`ca993eba`/`7141bcda`; the three late-texture lifecycle cycles retain 612 solids/
829 textures, slots 500 → 534 → 500 and clean ownership. The main deserializer
`0x66f69c`, its record readers and relocation/shared-pool producers remain open,
as do shader preparation/evaluation, source compilation and the GPU/API backend.
Real effect/technique activation is still pending those dependencies.

### Serialized effect record readers and retail metadata chains (2026-10-28)

`Libs/D3DX/EffectRecordReaders.cpp` recovers six complete bodies:

| Address | Recovered behavior |
| --- | --- |
| `0x66a574` (315 bytes) | Read an annotation and accumulate its storage requirements |
| `0x66b420` (493 bytes) | Read a parameter and its linked annotations |
| `0x66a6af` (397 bytes) | Read a pass, annotations and its group records |
| `0x66164d` (1,011 bytes) | Read/classify one table-driven group record |
| `0x66b60d` (1,505 bytes) | Read a technique and aggregate slot/register requirements |
| `0x65b019` (127 bytes) | Walk scalar/structured unit counts for a supplied divisor |

Each stream read calls the buffer length getter, then reloads the word cursor for
its comparison. If allowed, it calls the pointer getter and reloads that cursor
again before loading the DWORD. The cursor advances even when a read is skipped;
a separate final check rejects null/truncated input. The original comparison
uses the getter's returned size directly with this cursor; no added division by
four or stricter stream abstraction changes it. The source retains those call
boundaries through typed native bridges to the recovered buffer getters.

Parameters and annotations retain the original clears, flags and counter timing.
New annotations are constructed, parsed and linked in order. A failed pending
annotation is deleted while previously linked annotations remain owned. Scalar/
vector, matrix and structured descriptors update the separate `+0x128`, `+0x124`
and `+0x12c` storage totals. Structured data executes the recovered type validator
and padded-size walk. Stream and descriptor failures preserve partial writes.

Record reading starts with **unaligned DWORD stores at `+0x52` and `+0x54`**,
crossing the code/slot/unit halfwords. This matters at subsequent getter calls
and on truncated input; replacing them with narrow field stores changes observed
state. The reader checks the 182-entry code limit, table slot bounds and the
special `0x105` slot convention. It assigns resource kinds and shader selectors,
normalizes register codes, computes units with the original ceiling division,
and accumulates the group's storage offset without adding alignment rounding.
Tags `0x0e000000`/`0x0f000000` increment group word `+0x34`. Unknown fields remain
offset-named; no broader semantic types are inferred from those tags alone.

Technique parsing preserves pass/group numbering and halfword writes. It finds
unique category-8 slots in forward order, allocates the original `0x68`-byte
records and temporary bookkeeping, then scans each pass's records **backward** to
set dirty flags. Category 7 keeps the original last-record behavior; category 8
uses the unique-slot map and retains the forced flag for tag `0x0800000d`.
Register requirements update the original halfwords `+0xa4` through `+0xb2`,
including the `0x100` sentinel/bank rules. Allocation order, partial failure state,
unknown packing gaps and temporary free order are retained.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_readers.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_readers.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native --target ug2effectreaders_probe
build/native/ug2effectreaders_probe "$UG2_DIR/speed2.exe" /tmp/effect-readers.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_readers.py --gcc --kind retail --native-dump /tmp/effect-readers.bin
```

Each compiler passes **6,167 comparisons**: 1,024 fixtures per recovered body and
**23 retail reader chains**. Both reject **43 mutations** covering cursor/getter
order, skipped reads, wide stores, name/type validation, annotation ownership,
record allocation, slot bounds, resource kinds, selector/code normalization,
unit counts, storage offsets, pass numbering, unique-slot counting, scan order,
dirty flags, register banks and cleanup order. Structured element/member counts
vary independently. Explicit fixtures cover the `0x100` convention and the first
high-bank slot, rather than treating a behaviorally equivalent mutation as proof.

Comparisons include complete arena/heap bytes, getter/allocation/free-time state,
results/cursors, live allocation maps, stack, nonvolatile registers and restored
x87 state. Selected synthetic cases perturb the cursor at getter entry to check
caller reloads; the recovered getter bodies still execute. All reader, metadata,
validation, constructor, size-walk and destruction dependencies execute source.
Only allocator/free operations and those stated perturbations are controlled.
Finite mapped streams do not establish malformed-pointer, arbitrary-interface,
exception or giant-loop behavior, and instruction-byte identity is unproven.
Fingerprints are in `config/SPEED2_EXE_1_2/effect-readers.json`; generated reports
remain under `build/analysis/effect-readers/`.

`ug2effectreaders_probe` supplies a diagnostic outer header, copies the owned blob
and input buffer, constructs the original metadata arrays and runs the recovered
parameter/technique readers for every retail recipe. Its `UG2FXRD1` capture records
cursors, storage/count totals and a normalized summary of parsed fields, code/
slot/unit data, category flags and register limits. Native addresses and unknown
heap bytes are excluded from that format. Replay compares these results with
both original x86 and rebuilt execution; the separate Unicorn comparison checks
full fixture bytes. All **23 native chains match** and full recovered destruction
returns tracked ownership to **zero allocations** for every recipe.

The diagnostic header reader is test scaffolding, not an implementation of the
main deserializer. Parsing stops before relocation, shared-pool binding and the
remaining compiled shader payloads. No shader or successful effect/API result is
fabricated. The existing input/metadata/setup probes remain available separately.
Demo, simulator and all three probes build successfully. Native SKYLINE orbit
retains `ca993eba`/`7141bcda`; three late-texture lifecycle cycles retain 612 solids/
829 textures, slots 500 → 534 → 500 and clean ownership.

The remaining effect path still includes `0x66f69c`, its relocation/shared-pool
and compiled shader-payload dependencies, shader preparation/evaluation, source
compilation and the GPU/API backend. Real technique/consumer activation and the
other wheel-rendering inventory seams remain open.

### Effect dirty queues, group storage and device ownership (2026-10-28)

`Libs/D3DX/EffectRuntimeStorage.cpp` recovers five complete UG2 bodies used by
deserialization and subsequent parameter updates:

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x65b92d` | 67 | Reset the effect's dirty-record queue |
| `0x65aaaa` | 106 | Grow that queue |
| `0x65ab14` | 465 | Mark parameter uses/records dirty and optionally enqueue records |
| `0x65b733` | 90 | Allocate group data and append storage for index halfwords |
| `0x65e591` | 53 | Set device references through the secondary interface at effect `+4` |

Queue reset frees the old allocation before requesting `0x400` bytes, then stores
the returned pointer. Failure leaves the old count/capacity intact. Growth
**doubles capacity before allocation**, including on failure; success copies the
original wrapped DWORD count, frees old storage and publishes the new pointer.
No defensive saturation, rollback or extra initialization changes these states.

Before runtime setup, dirty notification resolves the owner through `+0xb0`,
walks its circular use ring from the head's predecessor and locates records
through the use's group/record halfwords. With `+0xb4` present, it checks the
parameter index against `+0xac`, sets the byte in `+0xbc` and walks the selected
null-terminated row from `+0xb8`; each use supplies its direct record pointer.
Bit 0 of effect byte `+0x119` enables queueing. Record bit 1 at `+0x30` suppresses
duplicates. Use and record dirty fields are written **before** queue growth,
so an allocation failure retains those writes without setting the queued bit or
incrementing the queue length. Entries retain separate group/record word stores.

Group allocation preserves the observed shift/mask expression, which retains an
odd data offset rather than rounding it. It appends two bytes per index, writes
`+0x14`/`+0x0c` before the unsigned capacity test, and allocates before freeing
old data. It does not copy old bytes. Failed growth retains the old allocation
and capacity alongside the new size fields. Size arithmetic wraps as x86 DWORDs.

The secondary-interface setter retains the incoming device first, then rereads
and releases its previous owned pointer at relative `+8`. It finally stores
both relative `+8` and `+4`, including on self-assignment. The native COM bridge
supports only the recovered buffer, pool and effect class tokens. GPU device
classes remain unsupported; a buffer fixture tests ownership without pretending
to supply a GPU implementation.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_storage.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_storage.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native --target ug2effectstorage_probe
build/native/ug2effectstorage_probe "$UG2_DIR/speed2.exe" /tmp/effect-storage.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_storage.py --gcc --native-dump /tmp/effect-storage.bin
```

Each compiler passes **5,120 comparisons**, 1,024 per body, and rejects **32
mutations**. Comparisons include the full fixture arena/heap, allocation/free/
synthetic-COM call-time state, HRESULT, allocation ownership, stack, nonvolatile
registers and restored x87 state. Fixtures cover allocation failures, wrapped
counts/sizes, odd offsets, repeated notifications, existing queued bits, ring/
grid order, inactive/null rows, direct versus indexed record pointers, and
null/self/distinct device assignment. Synthetic virtual callbacks perturb the
old device during AddRef and fields during Release to check rereads and final
stores; known buffer COM methods execute recovered bodies.

`ug2effectstorage_probe` runs eight synthetic native graphs combining ring/grid
selection, queue enablement and the active-parameter bound. The `UG2FXST1`
capture's normalized queue entries, dirty fields, group sizes/capacities and
reference counts match both original and rebuilt execution. All eight graphs
return tracked allocations to **zero**. Graph nodes are diagnostic stack
fixtures, not fully deserialized effects; their queue and group allocations
are released explicitly. Native pointer values and uninitialized allocation
contents are excluded from this capture; full mapped-byte comparisons belong
to the separate Unicorn tests.

Allocation outcomes and synthetic COM callbacks are controlled. Counter fixtures
include wrap and count/capacity mismatches in mapped guard storage; they are not
a memory-safety claim. Finite tests do not establish arbitrary malformed-pointer,
exception or unbounded-loop behavior. Instruction-byte identity is unproven.
Fingerprints are in `config/SPEED2_EXE_1_2/effect-storage.json`, with generated
reports under `build/analysis/effect-storage/`.

The native build, all 23 retail metadata reader chains, SKYLINE orbit and three
late-texture lifecycle cycles pass. Orbit checksums remain `ca993eba`/`7141bcda`;
the lifecycle retains 612 solids/829 textures and slots 500 → 534 → 500 with
clean ownership. Main deserialization `0x66f69c`, relocation, shared-pool binding,
compiled shader payload readers, shader preparation/evaluation and GPU/API
activation remain open, along with the other wheel inventory seams.

### Effect resource payloads and lazy shader records (2026-10-28)

`Libs/D3DX/EffectShaderResources.cpp` recovers four complete UG2 bodies:

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x65b970` | 94 | Vertex-shader creation caller through device vtable `+0x16c` |
| `0x65b9ce` | 94 | Pixel-shader creation caller through device vtable `+0x1a8` |
| `0x65ba2c` | 562 | Serialized indexed resource payload reader |
| `0x66520d` | 638 | Lazy shader-record payload reader and indirect-record conversion |

The creation callers enable debug mute before testing bit 1 of effect byte
`+0x119`. The disabled branch returns without a matching unmute. Otherwise they
cache the device's vtable before calling the recovered byte-buffer pointer
getter, then reread the device argument for the GPU call. Disassembly confirms
that the output pointer already on the stack belongs to the GPU call; the
buffer getter still takes one argument. Both success and failure unmute before
updating the reference's low two flag bits. Failure clears its shader pointer.
These callers return void, so readers retain their own HRESULT even when GPU
creation fails. Incidental register values are not forced into the source.

The indexed reader consumes index and byte size, validates the cursor and index,
then allocates a temporary block, including for size zero. It stages DWORDs and
the trailing bytes before checking whether the destination already owns either
resource pointer. Empty unowned entries return success. Kind 4 requires the
last payload byte to be zero and creates a recovered buffer in reference `+4`.
Kinds 15/16 create a buffer in `+8`, copy the bytes, call the appropriate shader
creation wrapper and update the group's selector/dirty record. Existing
resources and unsupported nonempty kinds fail. Temporary allocation, buffer
creation failures, cursor advancement and cleanup retain the original order.

The record reader returns immediately for other type-table kinds. For 15/16 it
reads size, resolves the owning parameter through `+0xb0`, and finds the resource
index through either the local data block or the shared-pool link's index map.
It publishes the new group word and increments the effect's group count before
record growth can fail. The lazy target uses code `0x92`/`0x93`, tag `0x0a000000`/
`0x0b000000`, the original selector/slot/unit fields and a ten-DWORD metadata
copy followed by clearing its two owned base pointers. Zero size returns at
this stage. A nonempty payload creates and fills a recovered buffer, calls
shader creation, then converts the source to code `0xb3`, tag `0x0e000000`, slot
`0xffff` and one unit. It increments group use count and appends four bytes of
storage without introducing alignment rounding. Failures preserve partial state.

As in the earlier readers, cursor units and comparisons with buffer length are
kept exactly as observed. Explicit getter boundaries precede cursor rereads;
there is no added division by four or stricter stream abstraction.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_shader_resources.py --gcc --mutation-checks
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_shader_resources.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native --target ug2effectshaderresources_probe
build/native/ug2effectshaderresources_probe "$UG2_DIR/speed2.exe" /tmp/effect-shader-resources.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_shader_resources.py --gcc --native-dump /tmp/effect-shader-resources.bin
```

Each compiler passes **4,096 comparisons**, 1,024 per body, and rejects **39
mutations**. Comparisons include full mapped fixture arena/heap bytes,
getter/allocation/free/mute/GPU call-time state, HRESULT, allocation ownership,
stack, nonvolatile registers and restored x87 state. Fixtures cover truncated
and wrapped cursors, byte tails, null/existing resources, valid and unterminated
strings, local/shared index lookup, fresh/existing lazy records, group bounds,
zero payloads, allocation failures, shader HRESULT signs and flag preservation.
Callbacks perturb cursors at getter entry and device/flags at getter or mute
entry. The recovered getter still executes; the tests check caller rereads and
the cached-vtable/reread-device distinction. Mute and GPU results are controlled
boundaries, not evidence of device or shader execution.

`ug2effectshaderresources_probe` runs **12 CPU-only fixtures**: eight resource
cases and four zero-byte/non-shader/group-bound record cases. It captures
HRESULT/cursor, buffer presence/length/payload digest, resource/group fields and
the lazy record's written fields in `UG2FXSR1`. Diagnostic source arrays are
deliberately preseeded; unspecified high bits in a freshly allocated target's
flags and native pointer values are excluded from normalization. Both original
and rebuilt execution match all captures. Full recovered destruction returns
every fixture to **zero tracked allocations**. No fixture enters debug mute or
GPU creation; shader bytecode is not interpreted by this probe.

Fingerprints are in `config/SPEED2_EXE_1_2/effect-shader-resources.json`;
generated reports remain under `build/analysis/effect-shader-resources/`.
Finite mapped cases do not establish arbitrary malformed-pointer, exception or
giant-loop behavior. Instruction-byte identity is unproven. The native build,
23 retail metadata chains and eight storage graphs pass. SKYLINE orbit retains
checksums `ca993eba`/`7141bcda`; three late-texture cycles retain 612 solids/829
textures, slots 500 → 534 → 500 and clean pools.

`NativeFxShaderDispatch.cpp` explicitly traps the still-unrecovered debug-mute
`0x671aed` and GPU creation calls. Main deserializer `0x66f69c`, relocation,
shared-pool producers, compiled payload readers `0x66c37e`/`0x66c645`, shader
preparation/evaluation, source compilation and GPU/API integration remain open.
These recovered readers do not activate effects or the native queue consumer.

### Compiled effect tokens, constant tables and record linking (2026-10-28)

`Libs/D3DX/EffectCompiledPayload.cpp` recovers an eight-body compiled-record chain:

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x671db5` | 231 | Find a tagged comment in a compiled token stream |
| `0x65b3f7` | 596 | Extract/validate CTAB metadata and register-bank totals/limits |
| `0x688d2c` | 96 | Walk compiled operand records |
| `0x688d8c` | 61 | Walk/check a compiled result record |
| `0x688dc9` | 176 | Find FXLC/CLIT sections and validate the operand/result walks |
| `0x65be0d` | 267 | Find a root parameter by name or complemented handle |
| `0x660d4e` | 519 | Bind CTAB entries to parameter-use rings and reserve storage |
| `0x661a40` | 485 | Read a compiled record, parse sections, bind constants and reserve units |

The comment scanner clears optional outputs before reading the stream, accepts
the original six version prefixes, skips negative tokens and uses the token's
comment/instruction lengths. It retains the legacy `0x51` six-DWORD step and
the distinct null/format/not-found results. It does not add a byte limit or a
general instruction decoder. The separate diagnostic fixture locator bounds
its own walk to each resource so only terminated inputs reach native probing.

CTAB extraction initializes all four word arrays and the table output in the
original order. Missing/unsupported comment streams leave an empty successful
table result; malformed found metadata fails. The original header bound uses
`constantCount * 4 + 0x1c`; it is not strengthened to the entry's actual
20-byte stride. Names must terminate within the declared comment. Register sets
0/1/2/3 map to the original array banks 1/2/0/3, with word count overflow,
unsigned minimum/maximum comparisons and truncation only at the observed
stores. Nonzero default-data spans use a **strict** end bound; equality fails.
All partial totals/minima/maxima survive an ensuing error.

Operand and result walkers publish their cursor advances before tag checks.
Intermediate operand kind 9 fails, while final operand kinds 1 through 9 are
accepted. Results require a zero prefix and kind 4/5/6/7; a nonzero prefix
advances only one DWORD. FXLC/CLIT parsing accepts the original three complete
version words, publishes section pointers and executes those walks. It does not
evaluate compiled arithmetic or claim that a GPU shader is executable.

Root lookup preserves the small-integer handle gate and complemented-address
checks, including the root kind, owner bound, zero offsets, unindexed value and
annotation flag. The unused second stack argument is retained (`RET 8`). Name
lookup follows the local/shared descriptor base and its length-prefixed name;
it remains byte-exact and does not add case folding or annotation filtering.

Constant binding refuses an existing count/use array, changes dirty fields
before parsing, allocates the cookie-backed use array even for zero constants,
and links each resolved CTAB entry in order. A missing later name retains prior
links and counts. Register maxima drive the original bit-spread mask and the
separate float/bool/int storage reservations. Odd offsets and wrapped DWORD
arithmetic are retained without adding alignment rounding or saturation.

The compiled-record reader allocates a `0x20`-byte header plus payload, initially
clears only header `+8`, and returns early for zero payload. It parses sections
**twice**, around the final stream-length check, before binding CTAB entries.
It then uses the recovered unit-count helper and the original mask/reservation
arithmetic. Allocation, parse and lookup failures preserve the cursor, owned
buffer and earlier metadata writes. All getters remain explicit boundaries
followed by cursor rereads, with the original cursor/length unit comparisons.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_compiled_payload.py --gcc --mutation-checks --write-native-fixtures /tmp/effect-compiled-fixtures.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_compiled_payload.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native --target ug2effectcompiledpayload_probe
build/native/ug2effectcompiledpayload_probe "$UG2_DIR/speed2.exe" /tmp/effect-compiled-fixtures.bin /tmp/effect-compiled-native.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_compiled_payload.py --gcc --native-dump /tmp/effect-compiled-native.bin
```

Each compiler passes **8,644 case/chain comparisons**: 1,024 synthetic fixtures
per recovered body plus **452 retail parser chains**. Both reject **61 mutations**
covering output clears, versions, token stepping, comment lengths, names,
register mapping/overflow/bounds, operand/result cursors, handle gates, local/
shared lookup, use ownership/counts, masks, storage offsets, allocation sizes,
zero payloads and the second parse. Known parser/metadata/buffer/link/destructor
bodies execute on both paths. Only allocation/free and stated getter-entry
cursor perturbations are controlled. Comparison includes full mapped fixture
bytes, dependency/allocator call-time state, HRESULT, ownership, stack,
nonvolatile registers and restored x87 state; incidental unreturned values are
not forced into the source.

The fixture locator identifies 452 terminated streams with recognized comment
tags across the 23 retail effect resources. CTAB is present in **368**; all
constant-table calls return the original successful result, with the other 84
leaving an empty table. **378** accept FXLC/CLIT section parsing; the remaining
74 retain the original format rejection for that parser. These are bounded
embedded token candidates, not proof that every shader or possible input is
covered. Reports retain recipe/offset/size, payload fingerprints and normalized
parser results. The generated fixture file contains retail offsets/sizes and
synthetic inputs; the native probe loads retail bytes from the player's EXE.
No retail payloads are committed.

`ug2effectcompiledpayload_probe` replays all **452 native parser chains** and
**sixteen native linked-record graphs**. Its `UG2FXCP1` capture normalizes parser
pointer offsets, word arrays, HRESULT/cursor, known header fields, use-ring
entries, storage counters and root lookup results. Unspecified header bytes and
native addresses are excluded; zero placeholders for unwritten header fields
are capture conventions, not game writes. Both original and rebuilt execution
match every capture. Graphs exercise successful and partial linking, missing
sections/names, strict bounds, sampler word overflow, zero payloads, format/tag
failures and alternate accepted versions. Full recovered destruction leaves
**zero tracked allocations** after every graph. Parser checks neither invoke
GPU APIs nor evaluate FXLC instructions.

Fingerprints are in `config/SPEED2_EXE_1_2/effect-compiled-payload.json`; generated
reports remain under `build/analysis/effect-compiled-payload/`. Finite mapped
fixtures do not establish arbitrary malformed pointers, unterminated token
walks, exceptions or giant counts. Instruction-byte identity is unproven.
Native metadata/resource/storage probes, SKYLINE orbit and three late-texture
lifecycle cycles pass. Orbit checksums remain `ca993eba`/`7141bcda`; lifecycle
slots remain 500 → 534 → 500 with 612 solids/829 textures and clean pools.

Main deserializer `0x66f69c`, relocation/shared-pool producers, general handle
resolution and symbol readers `0x66c37e`/`0x66c645`, debug mute, shader
preparation/evaluation, source compilation and GPU/API activation remain open.
The recovered parsing/linking chain does not activate native rendering effects.

### Effect handles, dynamic annotations and symbol readers (2026-10-28)

`Libs/D3DX/EffectHandles.cpp` recovers nine D3DX bodies and their original decimal
conversion dependency:

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x66bbee` | 401 | General parameter resolution and suffix dispatch |
| `0x65c123` | 465 | Indexed element creation and sorted cache lookup |
| `0x66a83c` | 464 | Nested structure-member resolution |
| `0x665a15` | 1,467 | Annotation lookup and dynamic numeric/string creation |
| `0x65c61f` | 207 | Technique lookup by name or complemented handle |
| `0x65c5de` | 65 | Pass lookup through the original technique/pass halfwords |
| `0x661c3e` | 474 | Member-array construction and type/data offset walks |
| `0x66c37e` | 711 | Symbol payload lookup, compatibility and use linking |
| `0x66c645` | 688 | Symbol plus compiled-record payload reader |
| `0x75d5d3` | 136 | CRT decimal conversion with locale access and wrapping arithmetic |
| `0x75d65b` | 5 | Original decimal-conversion jump thunk |

General lookup retains the small-integer gate, complemented-address checks,
raw-pointer versus complemented-handle returns, and `.`, `@` and `[index]`
dispatch. Annotation-create bit setup and clearing follow the original DWORD
expression. Technique/pass lookup retains its unused depth argument, `RET 8`
and low-word ordinal tests; no common stricter handle
validator changes those routines' different preconditions.

Member arrays are destroyed before replacement allocation. Both the packed
value-size walk and padded-size walk retain their separate type/data cursors.
Element nodes keep unsigned stride/size divisions, original offsets, sorted
sibling insertion and repeat-lookup reuse. Structure-array members require an
indexed parent when the original descriptor says so. Nested dot traversal and
indexed/member/annotation suffixes retain the observed call boundaries.

Annotation creation preserves its partial-publication order. It allocates the
new parameter table, blob-pointer table and pending parameter, then installs
the larger blob table and increments `+0x120` **before** validating/allocating
the value descriptor. A later error can leave a published null slot while
`+0xa8` and the annotation chain remain unchanged. Numeric descriptors retain
their word arithmetic and zeroed payload; string descriptors transfer the old
reference array by copying it, clearing the old records and deleting the old
array without releasing transferred objects. Name bytes, padding, descriptor
fields, table publication, tail linking and cleanup preserve their original
order. The producer leaves the new parameter's `+0x38` count field untouched;
the recovery does not add initialization.

Symbol readers stage their names, resolve the complete handle path and retain
the original kind-5/kind-10 compatibility ranges and kind-15/kind-16 equality
checks. They copy the original ten-DWORD prefix and clear its owned child
pointers before creating use links or converting to indirect records. The
compiled-symbol path retains its header fields, storage counters, two section
parses, CTAB binding and partial errors. No successful shader/GPU result is
fabricated by these readers.

The decimal body retains its CRT thread/locale accesses, byte classification,
sign handling and **modulo-32-bit arithmetic**. For example, the digits in
`p1[4294967297]` produce index 1; substituting host overflow saturation changes
handle selection. `NativeFxCrtLocale.cpp` is separate platform scaffolding: it
exposes only the observed thread `+0x64` locale pointer in the immutable initial
UG2 C context, and the recovered body reads the original mapped locale/ctype
data. The global locale pointer remains mutable in the recovered declaration.
Thread-locale updates and multibyte classification remain explicit native traps;
their service outcomes are controlled only in comparative tests. Other CRT
thread fields and arbitrary locale implementations are not recovered here.

```sh
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_handles.py --gcc --mutation-checks --write-native-fixtures /tmp/effect-handle-fixtures.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_handles.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native --target ug2effecthandles_probe
build/native/ug2effecthandles_probe "$UG2_DIR/speed2.exe" /tmp/effect-handle-fixtures.bin /tmp/effect-handle-native.bin
UG2_EXE="$UG2_DIR/speed2.exe" build/venv/bin/python tools/verify_pc_effect_handles.py --gcc --native-dump /tmp/effect-handle-native.bin
```

Each compiler passes **11,296 case/chain comparisons**: 1,024 per recovered body
and 32 complete typed chains, including destruction. Both reject **44 mutations**
covering CRT calls/arithmetic, lookup gates/offsets, sorted cache reuse, member
storage, create-bit handling, annotation publication/ownership, untouched fields,
symbol compatibility, use allocation and repeated compiled parsing. Full mapped
arena/heap bytes, dependency/allocator-time state, results, live allocation maps,
stack, nonvolatile registers and x87 state are compared. Recovered size walks,
metadata/compiled parsers, buffer operations, linking and cleanup execute source.
Only allocation/free, CRT thread/locale services and selected getter-entry cursor
changes are controlled. The shared verifier's cdecl extension also passes both
existing 15,589-case metadata regressions.

`ug2effecthandles_probe` runs **32 native typed graphs** covering nested and
indexed lookup, repeated cache reuse, numeric/string annotations, partial
publication on unsupported descriptors, technique/pass handles, compatible
symbols, empty/nonempty compiled records and decimal overflow. An existing
buffer reference survives string-annotation transfer and is released once at
cleanup. Every graph destroys to **zero tracked allocations**. `UG2FXHD1` captures
normalized handles, known cache/annotation fields, blob hashes, reference counts,
reader headers/cursors and storage counters. Native addresses, unwritten header
fields and the untouched new-annotation count are excluded. Original and rebuilt
execution match all captures; the separate Unicorn comparison still checks full
fixture bytes, including preservation of unspecified fields.

Fingerprints are in `config/SPEED2_EXE_1_2/effect-handles.json`, including the
original locale/ctype and effect type-table data. Generated reports remain under
`build/analysis/effect-handles/`. Fixtures are diagnostic typed graphs, not fully
deserialized effects. Finite checks do not establish arbitrary invalid pointers,
malformed unbounded graphs, exceptions, giant counts or unsupported locales.
Instruction-byte identity remains unproven. Native orbit and repeated late-texture
lifecycle checks retain `ca993eba`/`7141bcda`, 612 solids/829 textures, matching
slots 500 → 534 → 500 and clean ownership.

The two symbol readers and general handle family are now recovered. Main
deserialization `0x66f69c`, relocation/shared-pool producers, the stated CRT locale
services, debug mute, shader preparation/evaluation, source compilation and
GPU/API activation remain open. Effects and the native queue consumer remain
inactive in the demo.

### Effect value writes, relocation and deferred ownership (2026-10-28)

`Libs/D3DX/EffectValues.cpp` recovers the full value-write dependency chain of
relocation `0x6614a1`, using live Ghidra decompilation, memory reads and original
instruction order. No Ghidra project mutation was made. The native setter bridge
recognizes the original effect class token and calls the recovered implementation;
it does not treat the PE vtable as executable native code.

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x65cc10` | 184 | Shader-resource reference write and shared propagation |
| `0x65ccc8` | 164 | Object-resource reference write and shared propagation |
| `0x65cd6c` | 303 | String-buffer reuse/allocation, copying and shared ownership |
| `0x65ddc8` | 928 | Recursive typed copy with descriptor/data/input cursors |
| `0x661fc6` | 875 | Parameter value copy, indexed handles and struct arrays |
| `0x66055c` | 153 | Local, selected-shared-effect or broadcast dirty notification |
| `0x65ee33` | 13 | Eight-byte retained-reference constructor callback |
| `0x65eb8f` | 644 | Recursive count/retention walk over compact input values |
| `0x65ee13` | 32 | Value-reference wrapper with local cursors |
| `0x666c81` | 374 | Deferred input copy, retained references and tail publication |
| `0x66c8f5` | 108 | Effect vtable `+0x50` value-write entry |
| `0x6614a1` | 428 | Scalar/vector, matrix and struct storage relocation |

Scalar/vector writes copy raw DWORDs into sixteen-byte elements. Matrix writes
transpose compact row-major input into sixty-four-byte elements, normalizing
boolean DWORDs only for matrix kind 1. Struct copies recursively reset the
member descriptor cursor for every element; padding remains untouched. Other
matrix kinds preserve the original successful no-op paths rather than
adding validation. Indexed handles force one element in the top-level copy.

Resource writes retain the incoming object before releasing the old reference.
Exact self-assignment skips shader/object propagation. Shared links map the same
input index to each effect's reference array and skip null-effect nodes. Only the
originating shader write sets effect bit `0x40`; each assigned shader reference
sets bit 0. String writes count the terminator, reuse a sufficiently large buffer
without clearing its tail, and preserve the local retain/release around publication
to shared effects. Null strings clear the references. Allocation failure preserves
the prior destination and the original partial state.

Dirty notification respects shared-pool selection: zero active effects do nothing;
a selected effect uses its own linked parameter and receiver; broadcast visits
only effects with bit 0 of byte `+0x119`, then the originating effect. The recovered
queue implementation executes, including duplicate suppression and growth failures.

The setter accepts complemented record handles directly or resolves other handles
through the recovered general resolver. A byte count below record `+0xc` fails
with `0x8876086c`, except the original `0xffffffff` sentinel. State 3 records a
deferred write. The deferred path copies DWORDs and trailing bytes, counts resource
references, allocates the cookie array even for zero references, then performs the
ownership pass before linking the new tail. String references own copied buffers;
other references retain COM objects. The copied input still contains the original
resource/string pointers. Failures clean up partially retained references and leave
the old tail unchanged. This recovery does not execute deferred commands.

Relocation reads the type from the owning local/shared descriptor while copying
scalar/matrix source data from the local block. It expands compact data with the
original padding, transposes each whole matrix through recovered scalar
`0x63d132`, publishes `+0x30`, then advances the corresponding cursor. Structs use
the recovered padded-size walk on local storage, rebind record `+0/+4`, and invoke
the real value-write entry through the native ABI bridge. Shape 4 and unknown
unsigned shapes retain the original no-op return.

With `UG2_EXE` set to the reference executable, run MSVC/wibo jobs sequentially:

```sh
build/venv/bin/python tools/verify_pc_effect_values.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-effect-values-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectvalues_probe "$UG2_EXE" /tmp/ug2-effect-values-fixtures.bin /tmp/ug2-effect-values-native.bin
build/venv/bin/python tools/verify_pc_effect_values.py --gcc --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-values-native.bin
build/venv/bin/python tools/verify_pc_effect_values.py --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-values-native.bin
```

The verifier's recorded full run uses `--cases 1024`. Each compiler passes
**12,288 per-body comparisons plus 24 complete native-model lifecycles**, rejecting
**31 behavioral mutations** per compiler. The full
fixture arena and heap, preserved bytes, returns, allocation sizes/failures,
retain/release and buffer-getter event order, dirty-call receivers, transpose calls,
stack cleanup, saved registers and x87 stack/control state are compared. Synthetic
cases cover nested arrays/structs, numeric/resource kinds and invalid shapes,
local/shared index maps, selected and broadcast dirty traversal, raw input/output
aliases, zero dimensions/counts, trailing input bytes and partial allocation failures.
Fingerprints are in `config/SPEED2_EXE_1_2/effect-values.json`; generated reports
remain under `build/analysis/effect-values/`.

The native probe runs 24 typed CPU graphs: scalar/vector and boolean/integer/float
matrix writes, string reuse/reallocation/clearing, object/shader COM ownership,
deferred numeric/string/object writes, struct copies, all three relocation paths,
short-input rejection, named lookup, repeated writes, direct reference retention,
and a zero-byte deferred write. Every graph destroys to **zero tracked allocations**. Full original/rebuilt
model snapshots also match, including allocation and reference events through cleanup.
`UG2FXVD1` captures normalized cursor/prefix fields, blob hashes, reference flags,
counts and buffer hashes, and deferred bytes/reference contents. Captures match
original and both rebuilt compilers. Pointer identities in deferred copies are
normalized to fixture tokens; other pointer values are compared as offsets. The
model and native probe seed preserved flag bytes identically. Native COM tests
use recovered buffer classes to exercise ownership without creating GPU objects.

Allocation/free is controlled in the comparative verifier. Shared rings and input
metadata are bounded synthetic graphs, not output from the unrecovered shared-pool
producer. The native captures cover local graphs; shared notification/propagation
is covered by original/rebuilt execution. Arbitrary invalid pointers, giant counts,
reentrant external COM implementations, unmasked exceptions, byte identity and
exhaustive coverage remain unverified. Unicorn's signaling-NaN conversion limit
also applies to matrix relocation; the [separate transpose hardware probe](#scalar-d3dx-transpose-and-native-camera-bridge-2026-10-28)
checks those conversion lanes. Main deserialization and debug
mute, shader preparation/evaluation, source compilation, command execution and
GPU/API activation remain open. Native technique/draw activation is still inactive.

The complete native build, existing 32-graph native handle/reader lifecycle probe
and 60-frame SKYLINE orbit smoke run pass. Orbit source/record checksums remain
`ca993eba`/`7141bcda`; all texture records release to zero.

### Shared effect pools, binding and resource ownership (2026-10-28)

`Libs/D3DX/EffectSharedPool.cpp` recovers pool binding `0x64259c` and its complete
pool-side helper chain, with two original CRT comparison bodies. Evidence is live
Ghidra decompilation/memory plus original instructions; no project mutation was
made. `config/SPEED2_EXE_1_2/effect-shared-pool.json` pins all sixteen bodies and
the reference executable.

| Address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x767730` | 78 | ASCII case-insensitive comparison, exact -1/0/1 result |
| `0x75dd6f` | 105 | CRT locale selection and case-comparison wrapper |
| `0x6418a2` | 411 | Recursive descriptor/name/semantic compatibility |
| `0x641a3d` | 35 | Compatibility wrapper and resource-flag initialization |
| `0x641a60` | 438 | Pooled name, descriptor and padded-data sizing |
| `0x641c16` | 690 | Pooled descriptor/name/data construction and resource IDs |
| `0x641ec8` | 69 | Partitioned construction wrapper |
| `0x641f0d` | 240 | Recursive local resource-index map extraction |
| `0x641ffd` | 26 | Index extraction wrapper |
| `0x642017` | 249 | Compact numeric refresh with resource skipping |
| `0x642127` | 26 | Numeric refresh wrapper |
| `0x642141` | 838 | Recursive COM resource/reference transfer |
| `0x642487` | 43 | Resource transfer guard/wrapper |
| `0x641862` | 22 | Pool-node constructor, preserving unspecified fields |
| `0x6424b2` | 49 | Rebase existing parameter data pointers |
| `0x64259c` | 707 | Match/create pooled metadata and bind an effect parameter |

Compatibility requires equal kind, shape and array count. Names compare raw bytes,
including null-name presence; semantics use the original case-insensitive CRT
routine. Numeric shapes also compare row/column counts, while structs recursively
compare member descriptors. Object/shader kinds 5..9 and 15/16 mark the type as
device-bound; strings do not. A matching bound device-dependent node rejects an
effect with a different `+0xc` device pointer.

The original ASCII comparator folds only uppercase ASCII bytes on a raw mismatch
and returns -1/0/1, including for bytes above `0x7f`. Its CRT wrapper fetches thread
locale data, refreshes stale context and selects the original C-locale ASCII body
or the locale lowercase service. Native code uses the existing original immutable
C context. Non-C lowercase `0x75de10` remains an explicit unsupported native
boundary, like the earlier dynamic locale/classification services; comparative
tests control it and check arguments, call order and wrapped differences. No host
`strcasecmp` or locale conversion replaces the original comparison.

Sizing reserves DWORD-rounded name/semantic strings and four-byte prefixes.
Names are required by sizing/construction, as in the original. Struct-array
metadata and names are stored once while data repeats for each element. Numeric
pool construction copies full sixteen-/sixty-four-byte elements, retaining all
padding DWORDs; resource data becomes sequential shared IDs. Index extraction and
numeric/resource refresh preserve their separate **compact rows × columns** skips.
The source does not replace those distinct walks with a common inferred layout.
Mixed-struct fixtures keep any numeric words reached as shared IDs inside the
map range; this is a bounded fixture condition, not added runtime validation.

Binding scans existing nodes, checks device compatibility, and refreshes numeric
data when a matching node has no members. Otherwise it computes storage partitions
and grows the pool when needed. Capacity is published before allocation as
`max(0x10000, oldCapacity*2)`, without an added clamp to the requested total.
Growth copies every used byte, clears the remaining capacity, frees the prior
block and rebases **only parameter `+0`**. Type pointers deliberately remain at
the previous block until a later caller rebinds them. Failed allocation retains
the original partially published capacity.

New nodes publish descriptor/data offsets and resource count before allocating
the effect link and its index map. Links are prepended to the node's ring; the
parameter is rebound before resource transfer. Transfer takes its source from the
oldest member (`head.previous`), with a guard for self/empty destinations. It
retains source objects before releasing destination slots. Shader kinds copy both
owned slots, only flag bit 1, and `+0xc`; other resource kinds copy only the first
slot. Failures preserve the original partial metadata/ownership state and release
an unpublished link through the recovered cleanup path.

With `UG2_EXE` set to the reference executable, run:

```sh
build/venv/bin/python tools/verify_pc_effect_shared_pool.py --gcc --cases 1 --kind bind --write-native-fixtures /tmp/ug2-effect-pool-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectsharedpool_probe "$UG2_EXE" /tmp/ug2-effect-pool-fixtures.bin /tmp/ug2-effect-pool-native.bin
build/venv/bin/python tools/verify_pc_effect_shared_pool.py --gcc --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-pool-native.bin
build/venv/bin/python tools/verify_pc_effect_shared_pool.py --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-pool-native.bin
```

MSVC/wibo compilation remains sequential. Each compiler's full suite checks
**16,384 per-body comparisons plus 21 complete model lifecycles**, rejecting
**38 behavioral mutations** per compiler. Checks include full
arena/heap bytes, returns, allocation sizes/failures, buffer retains/releases,
locale service events, stack cleanup, saved registers and x87 stack/control state.
Fixtures cover nested structs/arrays, invalid kinds/shapes, null and differing
names/semantics, the byte comparison domain, distinct numeric inputs, ring aliases,
empty-node reuse, shared indices, partial allocation failures and growth/rebasing.
The native chains also compare full original/rebuilt state and ownership events
through cleanup. Generated reports remain under `build/analysis/effect-shared-pool/`.

`ug2effectsharedpool_probe` runs 21 typed lifecycles with two or three effects:
scalar/vector/matrix and mixed struct pools, strings/objects/shaders, differing
semantics/name case, matching-node reuse, empty-ring numeric refresh, rejected
device mismatch and allowed string/device mismatch, 64-to-128-KiB growth,
oldest-member selection, shared shader/setter writes and selected dirty notification.
For the three-member case, the newest existing member owns different resource data
so selecting it instead of the oldest member is observable. Native COM objects
are recovered CPU buffers, not GPU devices or shaders. Every lifecycle destroys
to **zero tracked allocations**. `UG2FXPD1` captures pool offsets/used bytes/capacity,
ring member order and maps, data hashes, data/type rebase state, dirty bytes,
resource flags/counts/content and final pool references. Local pointers are
normalized to fixture tokens, pooled pointers to offsets; no native PE code runs.

Allocation and non-C locale lowercase are controlled in the comparative verifier.
The metadata and device/COM inputs are bounded synthetic CPU graphs, not a fully
deserialized or GPU-backed effect. Arbitrary invalid pointers, missing required
names during construction, giant counts/requests, unmasked exceptions, reentrant
external COM implementations, instruction-byte identity and exhaustive coverage
remain unverified. In particular, a request larger than the original growth rule
can provide is not made safe by a new allocator algorithm. Main deserialization
`0x66f69c`, debug mute, shader preparation/evaluation, source compilation, deferred
command execution and GPU/API activation remain open. This closes the pool-binding
dependency of the main deserializer, not native technique/draw activation.

The complete native build, the existing 24-graph native value-write/relocation
probe and 60-frame SKYLINE orbit smoke check pass. Wheel source/record checksums
remain `ca993eba`/`7141bcda`, and all native texture records release to zero.

### Complete effect deserialization, debug mute and native stream lifecycles (2026-10-28)

`Libs/D3DX/EffectDeserialize.cpp` recovers the complete original main parser
`0x66f69c..0x67064a` (4,015 bytes). `EffectDebug.cpp` also recovers debug mute
`0x671aed` (213 bytes) and its registry helper `0x641539` (95 bytes). Evidence is
live read-only Ghidra decompilation/disassembly and the original instructions.
`config/SPEED2_EXE_1_2/effect-deserialize.json` pins the executable and all three
bodies. Ghidra prototypes were checked against actual register/stack arguments
and `RET` cleanup; the registry helper takes four arguments despite its inferred
project prototype. No Ghidra project mutation was made.

The parser now calls the recovered parameter/annotation/pass/technique readers,
type validation, resource and compiled-payload/symbol readers, relocation,
shared-pool binder, group storage and dirty notification bodies. It retains the
original order of allocations, cookie-array construction, device/input/pool
ownership and partial-state publication. Its array iterator receives an explicit
native ECX-only bridge to the recovered parameter constructor. No successful GPU
object or technique is fabricated.

The original stream limit is the buffer's byte length, although the cursor and
read extents are DWORD counts. This unusual comparison is preserved. Generic
header reads reload the cursor after the virtual pointer getter; the magic and
payload-selector reads instead retain their distinct cached cursor lifetimes.
The blob-copy input cursor is read after the pointer getter. Controlled getter
entry callbacks check these differences. A zero technique count returns `E_FAIL`
at `0x66f957`; allocation failures return `E_OUTOFMEMORY`. The final `+0xac`
parameter count is assigned even on early failure.

The copied compact blob is extended with matrix, scalar and struct storage. The
matrix start is **the next absolute 32-byte boundary**, even when the blob end
is already aligned. Root parameters validate before relocation; annotations
relocate/rebind before validation while retaining the old compact cursors for
validation and sizing. Parameter identifiers, annotation chains, shader group
counts, local/shared owner offsets and low-word selectors retain the original
widths. Resource/payload trailers route to the original kind-specific readers.
After optional shared binding, group storage is allocated/cleared and sampler
selectors are packed. The parser sets its ready/selection words, marks the
applicable roots dirty and rebinds all table and linked cache data/type pointers.
This final rebasing includes shared-pool owners; it is separate from the binder's
earlier data-only rebasing.

Debug mute probes the release/debug D3D module names before checking the cached
registry setting. Failed export discovery is retried while both slots are zero.
The module import targets are cached after the first module query; the second
probe retains those dispatch values across callback-time changes. Registry reads
reuse the original data-pointer-valued initial type word, query the requested
byte count, close an opened key even after query failure, and check the returned
type. Only the initial `-1` setting is queried/normalized. Callback slots are
reread before each call. The comparative verifier exercises module/load/export
failures, cached callbacks/settings, changed imports, query/type/byte-count
results and escaped-output mutations during close/callback calls.

`NativeFxDebugPlatform.cpp` supplies the known Windows module/key absence path
on Linux. The two D3D modules return absent and the known registry open returns
`ERROR_FILE_NOT_FOUND`; other loader, registry and debug-export calls remain
explicit unsupported boundaries. `NativeFxShaderDispatch.cpp` now reaches the
recovered debug body. Native GPU creation remains unsupported. The factory's
`UG2FxDeserializeEffect` ABI bridge now calls the recovered parser; runtime group
preparation/evaluation still prevents complete native factory/technique activation.

With `UG2_EXE` set to the locally installed reference executable, run:

```sh
build/venv/bin/python tools/verify_pc_effect_deserialize.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-effect-deserialize-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectdeserialize_probe "$UG2_EXE" /tmp/ug2-effect-deserialize-fixtures.bin /tmp/ug2-effect-deserialize-native.bin
build/venv/bin/python tools/verify_pc_effect_deserialize.py --gcc --cases 512 --mutation-checks --native-dump /tmp/ug2-effect-deserialize-native.bin
build/venv/bin/python tools/verify_pc_effect_deserialize.py --cases 512 --mutation-checks --native-dump /tmp/ug2-effect-deserialize-native.bin
```

Run MSVC/wibo jobs sequentially. Each compiler checks **1,647 case/stream
comparisons plus 110 complete model lifecycles**: 512 per new body, 64 valid
synthetic streams, sixteen getter/cursor changes, eight explicit import-cache/
callback cases and all 23 retail streams. Both compiler builds compare against
original machine code under Unicorn. All retail streams return success. Cases
include numeric shapes/arrays and structs,
strings, vertex/pixel resources, named and compiled-symbol bindings,
parameter/technique/pass annotations and records, zero techniques, corrupt magic/type/blob markers, null device/buffer, truncated
limits and allocation failures. Fifteen controlled GPU creation failures execute
in the case suite; they do not stand in for real native GPU results.

Final comparisons include exact full arena/two-MiB heap bytes, return values,
tracked live allocations and debug globals/import slots. Boundary observations
compare arguments, order and SHA-256 arena/allocated-heap/debug-state digests,
including validation versus relocation and pool binding. Array callback addresses
and stack-local output addresses are normalized where the compiler ABI requires it. Stack cleanup,
saved registers and x87 stack/control state are checked. The native-chain model
also compares full original/rebuilt state and ownership events through destruction.
The suite rejects **39 behavioral mutations per compiler**, covering cursor
reads, ownership, zero-technique errors, validation, alignment, resource routing,
shared binding, sampler selectors, ready/dirty/rebase state and registry/debug
ordering. Generated reports and mutation artifacts stay under
`build/analysis/effect-deserialize/`.

`ug2effectdeserialize_probe` runs **110 native lifecycles**: 64 synthetic streams
and every retail stream both standalone and as two effects sharing a pool and
the same device receiver. It uses the original `0x200` flag to disable GPU
creation (plus `0x100` to exercise the dirty queue in pooled cases). COM device
receivers are recovered CPU buffers. The parser retains actual serialized buffers,
creates complete metadata/resource/payload graphs, and destroys every graph to
**zero tracked allocations**. No setup/evaluation or PE function executes natively.

`UG2FXDD1` captures normalized counts, flags, dirty queues, compact/relocated data
hashes, parameter offsets and owner/cache relationships, resources and buffers,
group/record storage and selectors, technique/pass summaries, shared storage and
final caller/pool reference counts. Local relocated offsets are relative to the
matrix partition so host allocation alignment can differ; absolute alignment is
checked separately. Code-buffer summaries omit unwritten header slots. Native
summaries also exclude original unwritten shader-group storage fields, the unused
reference-zero group index and preserved heap high bits. Exact preservation of those bytes is checked in the emulator,
not inferred from a host allocator's initial contents.

These are finite behavioral and CPU ownership checks. Byte identity, arbitrary
malformed pointers/giant counts, unmasked x87 exceptions, external COM reentrancy,
non-C locale paths, D3D/GL equivalence and rendered effect output remain
unestablished. Earlier sections' open main-parser/debug entries are superseded by
this recovery. Shader preparation `0x666df7`/`0x667d51`, evaluation `0x663a32`,
source compilation, deferred command execution, GPU resources/states and native
technique/draw activation remain open.

The complete native build, 60-frame SKYLINE orbit and three-load solid/texture
lifecycle regression pass. Wheel source/record checksums remain
`ca993eba`/`7141bcda`; each load has 612 solids and 829 textures, late texture
slots return `500 → 534 → 500`, and unload leaves clean pools.

### Shader-preparation dependencies: constant binding, use removal and bytecode capture (2026-10-28)

`Libs/D3DX/EffectPreparation.cpp` recovers five dependencies of the complete
vertex/pixel group preparation routines. The live Ghidra MCP bridge was offline;
evidence was extracted from the saved `speed-re` project using Ghidra headless
`-readOnly -noanalysis`, without saving or changing the program. The saved
program's executable SHA-256 matches the pinned reference. Decompilation and
instruction listings were extracted locally; original executable bytes, actual
stack cleanup and comparative execution establish the evidence. Generated
listings and project files remain uncommitted.

| Address | Bytes | Behavior |
| --- | --- | --- |
| `0x65b098` | 90 | Scalar/vector/struct-only descriptor test and cursor walk |
| `0x660f9c` | 518 | Bind preparation CTAB constants and reserve register storage |
| `0x6611a2` | 127 | Remove a parameter use and invalidate temporary grids |
| `0x661221` | 242 | Capture vertex shader bytecode and handle device ownership |
| `0x661313` | 242 | Corresponding pixel shader bytecode capture |

The shape test accepts shapes 0/1 and recursively matching structs (shape 5).
It ignores kind, dimensions and array count. It advances seven words for a
scalar/vector, six before struct children, and leaves a failing child's cursor
at the position reached so far. Matrices, objects and unknown/negative shapes
return false. The source does not infer a stronger type validator.

Preparation binding reads tokens at header `+0x68`, retaining the opaque header
words elsewhere. It rejects an already allocated/nonzero-capacity use array,
clears the record's queued flag and parses the original CTAB. Unlike the earlier
compiled-record binder, absent parameter names are skipped. Successfully found
names append actual use nodes through the recovered root lookup/link routines.
The original allocates the cookie array for the CTAB count but records **twice
that count in a 16-bit capacity word**; these are kept separate. Allocation and
partial parsing/linking failures retain the original published state. Storage
mask construction, bank widths, odd offsets and unsigned wrapping are preserved.
The mask is written at header `+0x18`, and reserved bank offsets at `+0x34..+0x3c`.

Use removal first splices both neighbors, selects the owning parameter through
the child record's `+0x14`, and updates its head only when the removed node was
the head. Singleton heads become zero; other heads select the successor. The
removed node is reset to a self-ring without clearing its payload or decrementing
the record counts. Existing temporary grids are destroyed only when the original
setup flag permits it, through recovered `0x65b7f0`. That body retains its original
scratch/dangling slot behavior.

Each shader capture clears reference flag bit 0 before its first API call. The
size variable initially contains the incoming reference pointer, matching the
original reused argument slot. GetFunction queries the size, then GetDevice
obtains an owned device reference. A sufficiently large byte buffer is reused;
otherwise a recovered buffer is created before releasing/replacing the old one.
The shader dispatch table is read **before** the code-buffer pointer getter and
retained for the subsequent GetFunction call. After successful readback from a
different device, the old shader is released and the original vertex/pixel
creation caller runs. Temporary buffers and acquired devices are released on
both success and failure, including output ownership published by a failed API.
The two capture bodies retain distinct creation call boundaries.

`NativeFxShaderReadback.cpp` declares GetDevice/GetFunction unsupported until
there is an actual native GPU shader implementation. The recovered capture bodies
are compiled into the native archive, but CPU probes do not enter those methods.
Comparative fixtures control these API results; they do not claim GPU behavior
or manufacture a successful native shader. They use recovered buffers for the
remaining COM lifetime operations and execute the actual allocation, buffer,
shader-creation caller and debug-mute bodies. GPU creation is disabled through
the original flag when the recreation path is reached.

With `UG2_EXE` set to the installed reference executable, run:

```sh
build/venv/bin/python tools/verify_pc_effect_preparation.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-effect-preparation-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectpreparation_probe "$UG2_EXE" /tmp/ug2-effect-preparation-fixtures.bin /tmp/ug2-effect-preparation-native.bin
build/venv/bin/python tools/verify_pc_effect_preparation.py --gcc --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-preparation-native.bin
build/venv/bin/python tools/verify_pc_effect_preparation.py --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-preparation-native.bin
```

Run MSVC/wibo jobs sequentially. Each compiler checks **5,120 per-body comparisons
plus 32 complete CPU/model lifecycles and 128 complete shader-capture model
lifecycles**. Comparisons include exact arena/two-MiB heap bytes, results, live
allocations, debug state/import slots, call arguments/order and state digests,
stack cleanup, preserved registers and x87 stack/control state. Cases cover
nested/mismatching descriptors, CTAB banks/counts/names/default bounds, missing
names, partial OOM, word/offset arithmetic, ring head/middle/singleton removal,
child owners and temporary cleanup gates. Readback fixtures cover initial and
second API failures, failed device queries with owned outputs, buffer growth and
reuse, aliasing device/shader/code references, and cleanup/recreation order.
Callback fixtures change the escaped size during GetDevice and the shader class
during the buffer getter to check the cached dispatch. A size query that does not
write its output is exercised with an error result; pointer-sized successful
allocation requests are outside these bounded fixtures. No extra runtime guard
or size clamp is added to the recovered source.

`ug2effectpreparation_probe` runs **32 CPU lifecycles**: eight descriptor trees,
twelve CTAB binding graphs and twelve use-removal graphs. `UG2FXQD1` captures byte
cursor offsets, the full initialized opaque header, record flags/counts, CTAB
presence, actual cookie counts, use ring order/entries, owner heads, detached
self-links and remaining grids. Every graph destroys to **zero tracked
allocations**. Captures replay against both original and rebuilt model results.
The 128 readback lifecycles are emulator-only and also clean every tracked
allocation; their controlled shader services are not native GPU evidence.

The suite rejects **31 behavioral mutations per compiler**, covering descriptor
advancement/failure, binding guards/capacity/missing names/storage, both ring
neighbors and owner/head selection, readback flags/failures/buffer thresholds,
cached dispatch, device comparison, releases and the distinct pixel caller.

`config/SPEED2_EXE_1_2/effect-preparation.json` pins all five bodies and the
executable. Reports and mutation artifacts remain under
`build/analysis/effect-preparation/`. Finite comparisons do not establish byte
identity, arbitrary malformed/giant inputs, external COM reentrancy, complete
shader preparation, evaluation or rendered equivalence. The complete group
preparation bodies `0x666df7`/`0x667d51`, their helpers/sort, evaluator
`0x663a32` and shader VM `0x688e79` are recovered in later sections. Power/asin/acos, general FP
exception handling, GPU resources/states and native technique/draw
activation remain open.

The complete native build, the existing 110 parser lifecycles and the 60-frame
SKYLINE orbit regression pass. Wheel source/record checksums remain
`ca993eba`/`7141bcda`, and texture records/staged handles release to zero.

### Complete shader group preparation and original constant sorting (2026-10-29)

`EffectPrepareGroups.cpp` expands the common transcription in
`EffectPrepareGroup.inc` into the two complete original members. Their constants
and entries remain separate: vertex preparation `0x666df7..0x667d50` (3,930 bytes)
and pixel preparation `0x667d51..0x668cbf` (3,951 bytes). `EffectSort.cpp` recovers
their constant comparator `0x65b64b` (53 bytes), original CRT quicksort `0x75cc70`
(643 bytes) and private short sort `0x75cc00` (109-byte span, 106 instruction-body
bytes plus three padding bytes). The manifest pins the full spans and the bank
priority words at `0x7b5048`: **2, 1, 0, 3**.

Evidence is the saved reference Ghidra project's read-only decompilation and
instructions, confirmed against the installed executable. The live MCP bridge
was offline. Headless extraction used `-readOnly -noanalysis`; no project changes
were saved. Instruction comparison established the stage differences rather than
assuming that the routines were interchangeable. In particular, vertex uses
bank maxima `+0x1c`, bulk bytes `+0x2c`, selector `+0x18`, ready bit `0x2` and sampler
slot bias `0x101`; pixel uses `+0x24`, `+0x30`, `+0x1a`, preserves the existing
ready bit and applies no sampler bias. Their generated constant and PRES record
codes/tags also differ. No shared recovered helper call replaces either entry.

Preparation first rebinds the primary record's data/type caches and selects its
local resource index, including shared-pool index-map indirection. A nonpending
reference can append matching second-grid use nodes to the first grid in row and
node order; the second grid remains intact. The stage flag is then cleared.
A pending reference obtains bytecode through the recovered capture member, or
handles a missing shader through the original empty path. Prior generated uses
are unlinked, and prior PRES buffers are released before the active record count
and stage counters reset. All these calls execute recovered bodies.

The original shader CTAB and optional nested PRES/PRSI are parsed. The code keeps
three direct error returns that bypass the final `free(NULL)` call. It derives
bulk flags from minimum/count/maximum words, retains 16-bit sums and reserves the
original records. Reallocation transfers the primary use/code ownership before
destroying the old array. The allocated cookie count is `CTAB count + prefix`
(prefix one or two), while the **reported capacity is only `CTAB count + 1`**.
This difference is preserved, including its effects on repeated preparation.

Register storage, PRES masks/relative offsets and bank lists follow the binary's
exact expressions; the shift/mask expressions that retain odd offsets are not
replaced by inferred alignment. PRES metadata is copied into an owned byte buffer,
its sections are parsed, and its CTAB binds through recovered `0x660f9c` before
group storage allocation. Coverage flags, storage limits and packed register
ranges retain their original widths and update order.

Constants are sorted by the original bank priority and unsigned register start,
then placed into float/int/bool/sampler ranges. Missing names preserve their
original inactive-record behavior. Existing use arrays are reused where the
binary permits it. Defaults are copied as raw DWORDs; integer registers without
defaults receive **0/0/1/0**, with integer word 1. Float caches preserve the scalar,
vector, supported matrix and vector-only struct tests, pointer and byte-count
limits; matrix-containing structs are rejected by the original shape helper.
Sampler storage, indices and stage-specific slot bias are retained. The original
bulk coalescing clears earlier record bits as later records in a bank arrive.

The CRT sort retains bytewise swaps, median-of-three comparisons, pivot movement,
equal-value trimming, the threshold of eight and the explicit 30-slot partition
stack. Equal keys are not given a new stable-order rule. The private short sort
receives its high endpoint in **EAX**, with low/width/comparator on the original
stack. The recovered C++ interface makes that private argument explicit; the
emulator supplies EAX only when invoking the original ABI. No recovered source
forces registers or claims instruction-byte identity. Both the final bytes and
callback sequence/state are compared, so a host library sort is not substituted.

`NativeFxSetupDispatch.cpp` now calls the recovered preparation members. Production
shader GetDevice/GetFunction remain unsupported, and GPU creation is still an
explicit boundary. `ug2effectpreparegroups_probe` links its own **test-only CPU
readback adapters**, returning staged bytes and an owned CPU buffer device. It
creates no GPU shader or rendering state. COM buffer ownership, the capture
callers, parsers, binding, sorting, storage and destruction all run recovered code.
These fixtures establish CPU behavior and composition, not D3D/GL equivalence.

With `UG2_EXE` set to the installed reference executable, run:

```sh
build/venv/bin/python tools/verify_pc_effect_prepare_groups.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-effect-groups-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectpreparegroups_probe "$UG2_EXE" /tmp/ug2-effect-groups-fixtures.bin /tmp/ug2-effect-groups-native.bin
build/venv/bin/python tools/verify_pc_effect_prepare_groups.py --gcc --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-groups-native.bin
build/venv/bin/python tools/verify_pc_effect_prepare_groups.py --cases 1024 --mutation-checks --native-dump /tmp/ug2-effect-groups-native.bin
```

Run MSVC/wibo jobs sequentially. Each compiler checks **5,120 per-body comparisons**
and **23 retail deserializer/preparation/destruction chains**. The retail graphs
make **136 preparation calls**: 68 initial calls and 68 after changing the bytecode
object through recovered `WriteShaderResource`. Alternate retail graphs use the
actual recovered pool constructor/binder and complete pool ownership. The bounded
retail service results include zero and the original positive success result 1;
negative HRESULTs remain separately tested on synthetic graphs. Full original and
rebuilt state/ownership is compared through cleanup.

Each compiler also rejects **55 mutations** across cache rebinding, shared index
maps, pending/grid gates, ownership transfer, bytecode/PRES failures, reported
capacity, header masks/offsets, default registers, cache shape/span decisions,
sampler bias/storage/indices, pixel layout/flags and CRT comparison/partition
order. Directed fixtures keep nonzero primary dirty state, an owned spare use
array and explicit integer/bool defaults, so preserved values and ownership are
observable even when retail fixtures do not enter those cases.

Cases cover pending and nonpending references, shared index layouts, row/node
filtering, primary dirty-state/owned-array transfer, new/reused capacity, malformed/absent CTAB
and PRES, register banks/defaults, matrix/array/struct cache decisions and allocation
failures. Directed cases exercise each cache decision. Synthetic shared-index
layout fixtures use a nonidentity slot map; they do not stand in for the recovered
pool binder, which runs separately in the retail chains. Generic sort fixtures
cover zero/singleton counts, sizes around the short-sort threshold, up to 257
elements, widths 0/1/2/3/4/7/8/12, ascending/reversed/equal/tied/mixed values, and
unsigned register starts across the word range.

Final comparisons check exact arena/two-MiB heap bytes, live allocations, returns,
register preservation, stack cleanup and x87 stack/control state. Boundary traces
include arguments/order and SHA-256 arena/allocated-heap/debug-state digests, with
callback/code addresses and the private short-sort ABI normalized. Allocation
failures retain the original partially published state. Generated reports and
mutation artifacts remain under `build/analysis/effect-prepare-groups/`.

The native probe runs **87 CPU lifecycles**: 32 synthetic effect graphs, 32 generic
sort traces and all 23 retail effect graphs. `UG2FXGD1` captures defined group
fields, advertised versus cookie capacities, reference contents/ownership, bank
ranges, generated record flags/types/default bytes, PRES headers/program hashes,
cache offsets/spans, rings and grids. Generic sort captures include every callback's
relative arguments, key bytes and full-array digest. Changed objects go through
the recovered write producer before repeated preparation. Every lifecycle destroys
to **zero tracked allocations**.

Native summaries hash only written register/sampler storage slices and skip
unwritten header/record fields and preserved heap high bits. Local relocated cache
offsets use the main parser's next-32-byte-boundary partition normalization;
shared offsets use their pool base. This permits different host allocation
addresses while checking the original layout rule. Exact preservation of the
excluded bytes remains an emulator check. Negative/partial preparation results
are captured without interpreting uninitialized generated record types.

Native build and regression checks also pass: the existing preparation probe's
32 CPU lifecycles and deserializer probe's 110 lifecycles finish with zero tracked
allocations. A 60-frame SKYLINE orbit at 90 degrees retains source/record
fingerprints **`ca993eba` / `7141bcda`**, camera
`1696.05762 -878.670471 25.8273449`, and output C0
`0.676807225 -2.48764729 1.04016507 1`; all 829 texture records/staged handles
retire to zero. This checkpoint changes CPU preparation, without activating
native effect rendering.

`config/SPEED2_EXE_1_2/effect-prepare-groups.json` pins the executable, all five
function spans and priority data. These are finite behavioral and CPU lifetime
checks. Byte identity, arbitrary malformed/giant inputs, unmasked exceptions,
external COM reentrancy and rendered shader equivalence remain unestablished.
Earlier sections' open group-preparation entries are superseded by this recovery.
The evaluation caller, CRT control state and VM dispatcher `0x688e79` are
recovered below. Power/asin/acos, general FP exception handling, production GPU bytecode/
resource/state services, source compilation and native technique/draw activation
remain open.

### Effect evaluation input copies and recursive double storage (2026-10-29)

`EffectEvaluationCopy.cpp` expands `EffectEvaluationCopy.inc` into four complete
original entries. The vector copy is `0x662ecc..0x663264` (921 bytes), scalar copy
`0x663265..0x66356d` (777 bytes), recursive vector walker
`0x65fab5..0x65febe` (1,034 bytes), and recursive scalar walker
`0x65febf..0x660204` (838 bytes). All take seven stack arguments and return with
`RET 0x1c`; ECX is unused in their bodies. Their ordinary source interfaces use
`__stdcall`. No forced registers or extra recovered helper calls are introduced.

The live Ghidra MCP bridge reported no instances. Evidence extraction used the
saved reference project with `-readOnly -noanalysis` and no project save. Both
instructions and decompilation were read; inferred decompiler float/pointer types
were corrected from the actual `FILD dword`, `FLD float` and `FSTP double`
instructions. `config/SPEED2_EXE_1_2/effect-evaluation-copy.json` pins the executable
and four function spans. The source records only the UG2 PC v1.2 build.

These routines feed record evaluation `0x663a32`. They read the parameter's data
and type pointers, choose shape from the descriptor or an optional CTAB type,
and widen values into double storage. Source kinds 1 and 2 use **signed integer
loads**, including arbitrary raw boolean words; kind 3 uses float loads. An array
count of `0xffffffff` selects the descriptor count with zero treated as one;
an explicit count of zero remains zero. Shapes 0/1/2/3/5 retain distinct paths.
Unknown shapes/kinds preserve the original cursor/no-write behavior.

Struct walkers reset the child type cursor for each array, follow optional CTAB
member offsets in eight-byte entries, and recurse with shared source/output/
remaining-count cursors. Numeric leaves advance the type cursor by 28 bytes even
when no values remain; structs publish their child cursor before the zero-count
check. The top-level copies keep local cursors and report output distance in
32-byte vector units or eight-byte scalar units. Recursive calls execute the
recovered walkers, with no controlled conversion or traversal result.

Several original layout quirks are retained:

- Shape-2 matrices use 64-byte source-array strides. Integer matrix arrays reread
  the first matrix's values, while their recursive source cursor still advances
  by 64 bytes for each entered array. Float matrices advance their read base.
- Vector matrices read transposed components at `row + column * 4`; scalar
  matrices read one component per row. Partial limits consume original units.
- Ordinary float paths write **four doubles**, including the scalar path. Scalar
  output advances by only **one double**, so successive writes overlap. Integer
  scalar paths write only one double; vector integer paths use the descriptor's
  row count, preserving unwritten components.
- Shape 3 adds `(4 - columns) * 4` source words after each array. This retains the
  original byte stride rather than an inferred packed-array layout.

Build and reproduce with `UG2_EXE` set to the installed reference executable:

```sh
build/venv/bin/python tools/verify_pc_effect_evaluation_copy.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-evaluation-copy-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectevaluationcopy_probe /tmp/ug2-evaluation-copy-fixtures.bin /tmp/ug2-evaluation-copy-native.bin
build/venv/bin/python tools/verify_pc_effect_evaluation_copy.py --gcc --cases 2048 --mutation-checks --native-dump /tmp/ug2-evaluation-copy-native.bin
build/venv/bin/python tools/verify_pc_effect_evaluation_copy.py --cases 2048 --mutation-checks --native-dump /tmp/ug2-evaluation-copy-native.bin
```

Run MSVC/wibo jobs sequentially. Each compiler passes **8,192 per-body comparisons**
with **13,986 calls to each recursive walker**. Cases cover descriptor versus CTAB
shape selection, explicit/default/zero array counts, scalar/vector/both matrix
layouts, nested structs, unsupported kinds/shapes, partial/zero/full remaining
counts, negative integer words, nonfinite float payloads and overlapping source/
output. Final arena/256-KiB heap bytes, returns, stack/register preservation and
x87 empty-stack/control-word state are checked. Recursive entry traces normalize
local cursor addresses to their values and compare order, arguments and SHA-256
arena/allocated-heap state. No allocator or GPU service is used by these bodies.

Both compilers reject **36 mutations** covering shape/array gates, child resets,
member offsets, recursive array counts, exhausted-member guards, matrix input/
output strides, repeated integer reads, signed loads, four-lane float tails,
overlapping scalar outputs, padding, published cursors, reported counts and
scalar/vector callee selection. Generated reports and mutation artifacts remain
under `build/analysis/effect-evaluation-copy/`.

`ug2effectevaluationcopy_probe` replays **512 native CPU graphs** (128 fixtures
through all four entries). Captures compare normalized type/source/output
cursors, remaining/written counts, and the complete 32-KiB input/output/guard
region, including overlapping writes. Source words in this replay use a bounded
set of integer extrema, signed zeros, ordinary floats and quiet NaNs. It is kept
separate from signaling-NaN hardware checks because Unicorn preserves signaling
NaNs when these original load/store sequences quiet them on hardware.

The same native executable runs **4,096 hardware copy checks** across all four
entries under x87 24-bit and 53-bit precision, each checking four output lanes.
Its oracle executes the original `FLD m32 / FSTP m64` micro-operation for edge and
seeded raw float inputs, including denormals, infinities, signed quiet/signaling
NaNs and their payloads. Recovered outputs, six masked exception-status bits,
control-word preservation, cursors/counts and trailing guards match the oracle.
This is a hardware conversion check, not native execution of mapped PE code or
an exemption from the original/rebuilt comparisons. It does not establish all
aliasing cases with signaling NaNs, unmasked exceptions or other rounding modes.

Native build and the 60-frame SKYLINE orbit regression pass. Source/record
fingerprints remain **`ca993eba` / `7141bcda`**, and all 829 texture records/staged
handles retire to zero. The copy chain is compiled into the recovered archive;
it does not activate evaluation, the shader VM, GPU resources or native effects.
The evaluator caller, CRT control mapping and VM `0x688e79` dispatcher are
recovered below. Power/asin/acos, general FP exception handling and real effect rendering
remain open. These finite
CPU checks do not establish instruction-byte identity, arbitrary malformed/giant input behavior,
external reentrancy or rendered shader equivalence.

### Record evaluation caller and CRT x87 control state (2026-10-29)

`EffectEvaluation.cpp` recovers `0x663a32..0x663bb0` (383 bytes).
`EffectFpControl.cpp` recovers four original CRT entries: abstract control mapping
`0x75da18` (146 bytes), hardware mapping `0x75daaa` (142 bytes), `control87`
`0x75db38` (50 bytes), and `controlfp` `0x75db6a` (22 bytes). The manifest
`config/SPEED2_EXE_1_2/effect-evaluation.json` pins all five spans and the reference
executable. Read-only saved-project Ghidra extraction supplied decompilation and
instructions; the live MCP bridge had no instances. No project mutation/save
was performed. This evidence concerns UG2 PC v1.2 only.

The caller reads its record buffer once, retains output offset/mask from its
header, and initializes the original local copy counters. On the first call for
an object (`value118 & 1` clear), it saves the **CRT abstract control value** into
`value114`, clears precision bits `0x30000` through `controlfp`, then sets bit 0.
This selects x87 extended precision. It does not restore the control word on
return. A later call with the ready bit set skips the control operations, even
if external code has changed the hardware precision in between.

When the record has CTAB metadata, active use nodes publish `value00 = value04`
**before** their bank is inspected. Bank 0 calls the recovered scalar converter
with eight-byte register strides; bank 1 calls the vector converter at the header's
integer offset with 32-byte strides; bank 2 uses the float-vector offset with the
same 32-byte strides. Bank/start/count are narrowed from the original words.
Other banks still retain the dirty publication but make no conversion call.
Inactive nodes are skipped without reading their entry pointer. All four copy
bodies and recursive walkers execute recovered source in comparative tests.

After copying, the input pointer/mask come from header words 4/2. A record without
CTAB passes zero input/mask. The VM receives the original **14 arguments**, with
three equal output pointers and three equal output masks, and the caller returns
its result without suppressing positive or negative values. Argument order,
cleared fields, source-copy order and the first-call FP transition are retained.
`NativeFxSetupDispatch.cpp` now reaches this recovered member through the existing
ABI bridge. The VM dispatcher is recovered in the next section; its remaining
math/exception dependencies remain explicit production boundaries.

The CRT routines retain their actual bit mappings and return contract:

- Abstract/hardware exception masks have different bit positions, including
  denormal masking at abstract bit `0x80000`. Rounding and infinity-control bits
  map separately. Unsupported precision encodings collapse as in the original.
- `control87` merges `(old & ~mask) | (new & mask)`, writes the converted hardware
  word, and returns the merged **abstract** value, including requested bits that
  the hardware cannot represent. It does not reread the hardware for its return.
- Even a zero-mask query rewrites the control word through both mappings, which
  canonicalizes reserved precision encodings. `controlfp` always strips
  `0x80000` from its mask before calling `control87`.

The private mapping entries originally receive input in EBX; ordinary source
interfaces make that argument explicit. The harness supplies EBX only for the
original ABI and normalizes the otherwise irrelevant high hardware-word bits.
No recovered source forces registers. `NativeFxFpControl.cpp` contains only the
platform operations for the original waiting `FSTCW` and `FLDCW`; translation
and mask/return logic stay in recovered C++.

Reproduce with `UG2_EXE` set to the installed reference executable:

```sh
build/venv/bin/python tools/verify_pc_effect_evaluation.py --gcc --cases 1 --write-native-fixtures /tmp/ug2-evaluation-fixtures.bin
cmake -S . -B build/native
cmake --build build/native
build/native/ug2effectevaluation_probe /tmp/ug2-evaluation-fixtures.bin /tmp/ug2-evaluation-native.bin
build/venv/bin/python tools/verify_pc_effect_evaluation.py --gcc --cases 1024 --mutation-checks --native-dump /tmp/ug2-evaluation-native.bin
build/venv/bin/python tools/verify_pc_effect_evaluation.py --cases 1024 --mutation-checks --native-dump /tmp/ug2-evaluation-native.bin
```

Run MSVC/wibo jobs sequentially. Each compiler passes **5,120 comparisons** across
all five bodies and rejects **41 mutations**. The evaluator makes **1,791 VM
calls**: **1,433 execute the original VM** on both sides for bounded move/add/multiply
programs, while **358 explicitly controlled calls** check return propagation and
output writes. The original VM runs in Unicorn, not natively. These composed
checks make 1,669 vector-copy and 848 scalar-copy calls, including recursive
struct walks. The final evaluator fixture walks **65,535 inactive uses** to check
the full word-sized count. Cases also cover groups/records, header offsets/masks,
absent CTAB, dirty-state values, unsupported banks, bank routing, nested types,
repeated calls and precision changed externally between calls.

Comparisons check the complete arena and modeled heap (`0x220000` bytes), live
allocation state, returns, callee-saved registers, stack cleanup, x87 empty-stack
state and the resulting control word. Call traces compare normalized arguments,
order, control words and SHA-256 arena/used-heap state. Mutations cover selection,
header fields, first-call gating/saving/precision, use publication, routing and
strides, VM bank/mask arguments, negative results, both control mappings,
unknown abstract return bits, zero-mask writes and denormal protection. Reports
and generated mutation artifacts remain under `build/analysis/effect-evaluation/`.

`ug2effectevaluation_probe` runs **64 native caller-contract graphs**, with
recovered copies and control routines. A clearly test-only VM service captures
the 14 arguments and pre-call FP/dirty/data state, performs a controlled optional
write and returns the requested result. It does not implement shader execution.
Native captures compare first/repeated returns, saved/ready state, full 16-KiB
register-storage bytes, all use-node dirty words and callback summaries.

The probe additionally checks **131,072 control mappings** against results from
the original routines: every low hardware word (with varying ignored high bits),
and all combinations of relevant abstract bits with ignored-bit variation.
**2,048 physical control updates** compare returned abstract bits and actual x87
words under every precision/rounding/infinity combination. The first 64 explicitly
query with mask zero through both CRT wrappers; the rest use seeded new values
and masks. Exception flags are cleared for these hardware-control fixtures.
Native x87 forces reserved control bit 6 to one, while Unicorn retains the raw
loaded bit. Only that hardware-fixed bit is normalized in native control-word
summaries; saved abstract values, masks and returns are compared exactly.

Native build and regression checks pass: preparation's 87 CPU lifecycles, the
copy probe's 512 graphs and 4,096 conversion/exception/cursor checks, and the
60-frame SKYLINE orbit. Wheel fingerprints remain **`ca993eba` / `7141bcda`**;
all 829 texture records/staged handles retire to zero. The preceding copy-only
verifier's heap-size description is corrected to its actual 256 KiB; this
verifier allocates the larger modeled heap for the 65,535-use fixture.

The caller and control mapping are recovered. The dispatcher, inline x87
operations, indexed conversions and `0x688ce2` wrapper are recovered below.
Power/asin/acos, general FP exception handling, full shader execution and native effect
activation remain open. Finite checks do not prove instruction-byte identity,
arbitrary malformed/aliasing/reentrant inputs, unmasked pending exceptions,
thread changes or rendered shader equivalence.

### Complete effect VM dispatcher and operation wrapper (2026-10-29)

`EffectVm.cpp` recovers the complete **2,324-byte dispatcher `0x688e79`** and
**74-byte operation wrapper `0x688ce2`**. `NativeFxVmDispatch.cpp` now calls the
recovered dispatcher through its original fourteen-argument interface. This
advances CPU effect evaluation; native effect/GPU rendering remains inactive.
`config/SPEED2_EXE_1_2/effect-vm.json` fingerprints both bodies and four original
double constants against the reference executable SHA-256.

The dispatcher retains 256 uninitialized scratch doubles and 256 persistent
operand pointers. Indexed terms multiply the previous low word by four before
adding the next offset, truncate a selected double to signed 64-bit integer,
and keep its low 32 bits. Scratch indices use `& 0xff`; literal indices use
unsigned modulo the literal count. Banks 2/3/4/5/6 use masked eight-byte offsets;
final bank 9 operands use masked **four-byte** offsets. Kind 9 is not an indexed
term dereference. Unsupported operand kinds retain the preceding pointer for
that slot; unsupported output kinds retain the preceding destination. Only
banks 7/4/5/6 select destinations. Neither stale-state behavior nor unchecked
sizes are replaced by new validation or initialization.

The low twenty operation bits specify the width. The upper opcode selects:

| Opcode (hex, bits 20–30) | Original operation |
| --- | --- |
| `100` / `101` | Forward DWORD copy / negate |
| `103` / `104` / `105` | Reciprocal / input reloaded after floor call, minus its result / power with base two |
| `106` / `107` | Guarded logarithm / reciprocal square root of absolute value |
| `108` / `109` / `10a` / `10b` / `10c` | Sine / cosine / asin / acos / atan |
| `200` / `201` / `202` / `203` | Minimum / maximum / less-than / greater-or-equal |
| `204` / `205` / `206` | Add / multiply / atan2 |
| `300` / `500` / `502` / `70e` | Conditional select / dot / operation wrapper / pairwise sum over two pointer-list halves |

A negative operation word enables scalar broadcasting only for `200` through
`206` and `500`; all other negative opcodes do nothing. Unknown positive opcodes
also do nothing. Forward copies preserve overlapping DWORD behavior. Dot paths
zero the destination before reading inputs, then reload both the destination
and any broadcast scalar on every term. Odd `70e` input counts truncate when
splitting the list. Min/max choose the right value on ties and unordered
comparisons; unordered conditions choose the negative select branch.

The logarithm's guard reads **`*inputs[lane]`**, while its actual value reads
**`inputs[0][lane]`**. It computes the original `FLDLN2; FYL2X` product followed
by multiplication by the stored double at `0x7baa88`; replacing this with host
`log2` changes arithmetic and rounding. Native x86 instruction adapters live
separately in `NativeFxVmIntrinsics.h`. They preserve FCOMP NaN behavior, x87
range handling, memory operand order and direct destination stores. An extra
double spill/reload can add a denormal exception to a newly underflowed result,
so these adapters do not return a temporary double through the compiler.

The `502` wrapper narrows at most the first three input doubles to floats,
zero-fills unused float words, calls `0x63f2b5`, and stores its x87 result as a
double. Its dependency takes a float pointer with **stdcall `RET 4`**; this ABI
is confirmed from the original instructions and comparative stack checks.
Count zero reads no input value. The 619-byte dependency is recovered in the
next section.

Verification commands (set `UG2_EXE` to the local reference executable):

```sh
build/venv/bin/python tools/verify_pc_effect_vm.py --gcc --cases 2048 --write-native-fixtures /tmp/ug2-effect-vm-fixtures.bin
build/native/ug2effectvm_probe "$UG2_EXE" /tmp/ug2-effect-vm-fixtures.bin /tmp/ug2-effect-vm-native.bin
build/venv/bin/python tools/verify_pc_effect_vm.py --gcc --cases 2048 --native-dump /tmp/ug2-effect-vm-native.bin --mutation-checks --native-mutation-checks
build/venv/bin/python tools/verify_pc_effect_vm.py --cases 2048 --native-dump /tmp/ug2-effect-vm-native.bin --mutation-checks
build/venv/bin/python tools/verify_pc_effect_vm.py --gcc --cases 128 --real-runtime
build/venv/bin/python tools/verify_pc_effect_vm.py --cases 128 --real-runtime
```

Each compiler compares **4,096 body executions** with the original, checking
complete modeled memory, call arguments/order, returns, preserved registers,
stack cleanup, control word and live x87 stack/tag contents. Controlled math
results expose ABI and call-order behavior, including a floor service that
changes the input so the caller must reload it. Separate bounded runs execute
the **actual original CRT and operation code** on both sides: 256 body
comparisons per compiler. Each normal run also compares **128 complete evaluator
graphs containing 224 calls to the recovered VM**, without redirecting the
rebuilt dispatcher to the original. Native captures replay **368 graphs**,
including 64 operation wrappers with test-only float-packet capture/results.
GCC rejects **49 source mutations** and MSVC rejects **48**, covering instruction and
operand cursors, indexed masks/conversion widths, overlap, broadcast arithmetic,
selection, logarithm pointers, floor reload and wrapper layout. The hardware
oracle independently rejects **nine native mutations**, including preloaded
comparison/arithmetic operands, an extra double spill, broadcast load order,
missing absolute value and narrowed indexed conversion. A compile/launch error
is not counted as a behavioral rejection. MSVC emits the same `_ftol2`
low-word conversion for the `int` and `__int64` casts here, so that source
change is excluded from its mutant count; GCC and hardware tests distinguish
the 32-bit conversion. Mutation workers cache unchanged MSVC dependencies
using source/header/compiler hashes and verify cached object hashes. Ordinary
verification always rebuilds all sources.

Generated objects, fixtures, captures, mutation logs and JSON results stay in
`build/analysis/effect-vm/` or `/tmp`.

The native probe additionally checks **176,928 hardware opcode/value/alias/FP
state cases**, **3,072 indexed conversions**, and **1,536 float-narrowing cases**.
Its independent micro-oracles transcribe the original x87 instructions; they
execute no retail PE code. All twelve combinations of 24/53/64-bit precision
and rounding are exercised, with signed zeros, subnormals, finite extremes,
infinities, quiet/signaling NaNs, scalar broadcast, zero/multiple widths and
four/eight-byte overlaps. Buffer bytes, control word, stack TOP/full tags and
six exception flags must agree; sine/cosine also compare C2 range status.
This hardware evidence applies to the Linux i386 GCC build. MSVC source is
compared under Unicorn; native MSVC exception-flag equivalence is not claimed.
Legacy MSVC integer casts still link original compiler helper `0x75dd28` in
the comparative harness. Native GCC lowers the signed-64 truncation to x87
instructions, checked by the hardware probe. Recovery of that CRT helper
itself is not claimed.

Unicorn's zero-input FYL2X emulation leaves an extra stack value, unlike the
physical instruction. Comparative logarithm fixtures therefore use positive
actual values while varying negative/unordered pointer-list guards. The
hardware oracle separately checks zero, negative and nonfinite logarithms and
requires the proper empty stack. Neither that emulator defect nor a matching
emulator result establishes hardware correctness by itself. All scratch and
pointer counts in these fixtures stay within the original 256-slot arrays;
unchecked out-of-range programs and first-use uninitialized pointers are not
claimed safe or completely verified.

Production `NativeFxVmCrtPlatform.cpp` still aborts at **power `0x75c9b0`, asin
`0x75eea0`, and acos `0x75db80`**. Floor and the operation bridge reach recovered
bodies below; general FP exception handling remains an explicit boundary. Private
original x87 math arguments are adapted only in the comparative harness;
there is no production host-libm replacement. Source compilation, GPU
resources/readback/states, native effect activation and remaining wheel seams
stay open. Behavioral agreement is separate from instruction-byte identity,
which has not been established. Unmasked pending exceptions, arbitrary
malformed/reentrant inputs and rendered shader equivalence remain unverified.

The factory `--real-setup` regression uses an explicit test-only bridge with
empty parsed metadata; unexpected preparation/evaluation calls abort. This
keeps that fixture's original scope while composed evaluator/VM tests exercise
the larger recovered graph. The native build and 60-frame SKYLINE smoke pass;
wheel fingerprints remain `ca993eba` / `7141bcda`, and all 829 texture records
and staged handles retire to zero.

### Table-driven effect VM operation and extended return (2026-10-29)

`EffectVmOperation.cpp` recovers the complete **619-byte `0x63f2b5`** body. The
production `UG2FxVmOperation63F2B5` bridge now calls this source, preserving its
stdcall float-pointer argument, `RET 4`, and extended x87 return. Opcode `502`
therefore reaches the recovered operation through the recovered narrowing
wrapper and VM dispatcher. Floor and its masked runtime are recovered in the following section. Power,
asin/acos and general FP exception handling remain explicit production
boundaries; native GPU/effect rendering remains inactive.

The original reads three floats, adds the single-precision **10000.0** constant
to each, truncates each extended sum to signed 64-bit integer, and uses the
**signed low 32 bits** in the fractional subtraction. This is truncation rather
than floor, including coordinates below the bias and values outside signed
32-bit range. Low-byte indices and their wrapped successors select nested
permutation values, then three-float gradient records. The pinned retail data
contains **514 permutation words at `0x7a9460`** and **514 gradient triples at
`0x7a9c68`**, each with its 256 entries repeated and two trailing repeats. Source
reads those mapped data bytes; it does not recreate or randomize the tables.
The manifest fingerprints both tables, the function and its four float constants.
Read-only Ghidra extraction confirms the reference executable and original body;
direct table references found in the project belong to this consumer.

The eight corner products accumulate **z, then y, then x**, with their original
parenthesization. The cubic weights use `(3 - (t+t)) * t * t`; the final output
is scaled by the original **1.5** float. This is the evidenced UG2 routine,
without substituting another noise/interpolation implementation. Its asymmetric
precision is retained:

- `x - 1` is stored as float early, while its extended value remains live for
  three corner products. Only the last corner reloads the stored float.
- `y - 1`, `z - 1` and the saved z fraction are floats. Early z products still
  use the unrounded extended fraction.
- The x/y weights, lower-plane result and upper-plane first-row result are
  explicitly narrowed to floats. Other row intermediates remain extended.
- The final z weight is computed from the stored float fraction in extended
  arithmetic; it is not narrowed like the x/y weights.

`long double` expresses retained x87 values on native i386; explicit volatile
floats express the observed single-precision spills. MSVC 7.1 uses its legacy
register precision for these expressions. No registers are forced to manufacture
an instruction-byte match. The production bridge also returns `long double`;
an intervening double spill would discard observable return bits. Legacy MSVC
integer conversions still use original compiler helper `0x75dd28` in the
comparative harness. Recovery of that helper itself is not claimed; native GCC
lowers the conversion to x87 truncation with the caller's control word restored.

Verification (set `UG2_EXE` to the local reference executable and build native
targets first):

```sh
build/venv/bin/python tools/verify_pc_effect_vm_operation.py --gcc --cases 16384 --write-native-fixtures /tmp/ug2-vm-operation-fixtures.bin
build/native/ug2effectvmoperation_probe "$UG2_EXE" /tmp/ug2-vm-operation-fixtures.bin /tmp/ug2-vm-operation-native.bin
build/venv/bin/python tools/verify_pc_effect_vm_operation.py --gcc --cases 16384 --native-dump /tmp/ug2-vm-operation-native.bin --mutation-checks --native-mutation-checks
build/venv/bin/python tools/verify_pc_effect_vm_operation.py --cases 16384 --mutation-checks
```

Both compilers pass **16,384 original/rebuilt executions**, comparing all live
x87 return bits, TOP/tags, unchanged input bytes, control word, callee-saved
registers and stdcall cleanup. Cases include finite biased coordinates,
sub-float-spill differences, all 4,096 triples of sixteen selected zeros,
finite values, infinities, quiet/signaling NaNs, subnormals and extremes, bias
neighbors, signed integer boundaries, and seeded arbitrary float bit patterns.
All twelve valid 24/53/64-bit precision and rounding combinations occur. Each
suite additionally passes **256 wrapper cases and 256 VM opcode-502 programs**,
with zero/multiple widths and overlapping outputs, calling the actual recovered
operation and bridge rather than redirecting them to the original body.

**33 GCC and 32 MSVC source mutants** are rejected: wrong bias, signed fraction,
indices/permutation nesting, corner selection, lost/extra float spills,
extended-lifetime/final-reload mistakes, polynomial/blend/component changes and
final scale. MSVC's equivalent integer-cast mutant is excluded for the same
`_ftol2` low-word reason described in the dispatcher section. Compile errors
are not accepted as behavioral rejection. Reports, object caches and mutation
logs remain under `build/analysis/effect-vm-operation/`.

The native probe checks **16,384 exact returns and physical FP-state cases**
through the production operation bridge. Its instruction micro-oracles stage
float/extended loads, truncation, products, blends and narrowing separately from
the recovered C++ expressions. They compare all 80 return bits and, after
storing that return, the control word, stack TOP/full tags and six exception
flags. The same captures replay byte-for-byte against original emulation;
no retail PE code executes natively. Another **2,560 native wrapper/dispatcher
checks** cover counts 0–4, partial/unaligned overlaps and complete buffer/FP
state. Five native mutants are rejected, including a bridge that narrows the
80-bit return to double. These physical exception-flag claims apply to the
Linux i386 GCC build; native MSVC hardware equivalence is not established.

The complete native build, existing VM dispatcher/evaluator regressions and
60-frame SKYLINE orbit pass. Wheel fingerprints remain **`ca993eba` /
`7141bcda`** and all 829 texture records/staged handles retire to zero. These
finite behavioral checks do not establish instruction-byte identity, arbitrary
invalid pointers/table mutation, unmasked pending exceptions, reentrancy or
rendered shader equivalence. Floor and its masked runtime are recovered below. Power/asin/acos, general FP
exception handling, source compilation, GPU resources/states, native effect
activation and the other wheel seams remain
open.

### CRT floor SSE/x87 dispatch, controls and matherr callers (2026-10-29)

The complete floor body is recovered in `CrtFloor.cpp`: **289 bytes at
`0x75eb50..0x75ec70` plus 209 bytes at `0x769764..0x769834`**, one discontiguous
498-byte original function. Nine supporting bodies are also recovered:

| Original address | Bytes | Recovered behavior |
| --- | --- | --- |
| `0x7676a4` | 36 | Raw hardware control merge; signed 16-bit previous-word return |
| `0x76cc73` | 19 | Current-mode FRNDINT with an explicit double spill/reload |
| `0x766ea9` | 91 | Integer-only infinity / quiet-NaN / signaling-NaN classification |
| `0x7661ba` | 9 | CRT thread-data errno pointer at offset `+8` |
| `0x767485` | 40 | Math error to errno mapping, including pointer-valued returns |
| `0x727e6b` | 3 | Original default callback, returning zero |
| `0x7674da` | 158 | 29-entry operation/name lookup, direct default callback and control restoration |
| `0x767578` | 83 | Quiet-NaN dispatch or direct errno/control path |
| `0x7694d0` | 660 | Complete SSE math-error selection, aliasing, callback and publication |

The production floor bridge now calls recovered source. Its SSE2 gate reads
`0x8ba480`, checks all MXCSR exception masks and the low seven x87 control bits.
The fast path uses the original exponent/shift data, packed logical shifts,
signaling packed comparisons, UCOMISD and scalar SSE subtraction. Negative
values execute a comparison **before** the large-exponent gate; removing it
changes the invalid flag for negative quiet NaNs. Negative tiny values also
retain DAZ behavior: the SSE comparison can select negative zero when DAZ treats
the input as zero. Positive values below one return positive zero without
loading the input through x87. These paths are not replaced by host `floor`,
x87 arithmetic or a bit-only approximation of SSE comparisons.

The x87 path merges the original raw control value **`0x173f`** from `0x81aa60`,
performs the original load/store preparation, classifies nonfinite arguments,
and runs FRNDINT for finite values. It compares the spilled result with the
original input, restores the saved control on the appropriate paths, or calls
the original general-result boundary with operation **11** and flags **8** or
**16**. This raw control helper returns the previous word **sign-extended from
16 bits**; it is distinct from the earlier CRT abstract-control mapping.
Hardware forces reserved control bits as usual; native tests compare actual
hardware words rather than assuming all requested reserved bits survive.

`CrtMathError.cpp` retains the original callback and ownership details. SSE
errors copy selected inputs to the output before constructing the callback
record, so whole/partial overlaps affect subsequent reads. Callback changes to
the result and control word are preserved. Operation **162** has type 4 but
still sets **ERANGE (34)** after a zero callback return; type-4 operations 15 and
25 do not set errno. The general user-matherr caller searches all 29 names and
calls the original default function directly, regardless of the indirect SSE
callback token. Its three double payloads are copied with integer MOV semantics.
The source uses explicit eight-byte word-pair arguments with the same CDECL
stack layout, retaining signaling-NaN representations until an original FP
load is required. The default callback has its own translation unit to retain
the original direct call boundary.

The VM fraction opcode also retains its original **load/store before floor**
(`0x6892ec..0x6892f0`), then reloads the input for subtraction after the call.
That preparation quiets signaling NaNs before SSE dispatch and is observable in
both x87 and MXCSR flags. Keeping only the numerical subtraction would miss
that behavior. The native hardware mutation suite detects its omission.

Verification (set `UG2_EXE` to the reference executable, build native targets,
and keep MSVC/wibo jobs sequential):

```sh
build/venv/bin/python tools/verify_pc_crt_floor.py --gcc --cases 8192 --write-native-fixtures /tmp/ug2-crt-floor-fixtures.bin --mutation-checks
build/native/ug2crtfloor_probe "$UG2_EXE" /tmp/ug2-crt-floor-fixtures.bin /tmp/ug2-crt-floor-native.bin
build/venv/bin/python tools/verify_pc_crt_floor.py --gcc --cases 8192 --native-dump /tmp/ug2-crt-floor-native.bin --native-mutation-checks
build/venv/bin/python tools/verify_pc_crt_floor.py --cases 8192 --mutation-checks
build/venv/bin/python tools/verify_pc_crt_floor.py --gcc --cases 1024 --real-result-runtime
build/venv/bin/python tools/verify_pc_crt_floor.py --cases 1024 --real-result-runtime
```

Each compiler passes **81,920 per-body comparisons**, **512 composed VM fraction
programs**, and **38 rejected source mutations**. Captures check returns and live
x87 values/tags/TOP, control and MXCSR state, callee-saved registers, CDECL
cleanup, input/output/thread bytes and callback payload/order. Cases include
signed zeros, fractional and exponent boundaries, infinities, quiet/signaling
NaNs, subnormals, arbitrary double/word patterns, all recognized selectors, unknown codes and
all 29 name rows, partial output aliases, callback edits and independently
requested control words. The main floor corpus takes **3,954 SSE paths and
4,238 x87 paths**. The initial general-result service is explicitly controlled
for call-contract verification; it is not a numerical substitute in production.

Separate runs add **10,240 body comparisons per compiler** while executing the
actual original shared result, exception-mask, status and exception-record code
on both sides. Only OS RaiseException is controlled there, including bounded
changes to its defined record fields and result. Uninitialized reserved record
bits are excluded from those callback summaries. This is composed caller
evidence for the unrecovered exception runtime, not recovery of that runtime.

The native probe passes **8,192 exact return/physical-state cases**, **65,536 raw
control updates**, **256 VM fraction graphs with zero/multiple widths and
four-byte overlaps**, **192 integer-payload/control checks**, and **eight x87
copy audits**. Independent instruction oracles compare all return bits, stored
buffers, errno, control, empty stack/TOP/tags, six x87 exception flags and MXCSR.
The payload checks ensure unused or integer-copied SNaN/subnormal arguments do
not incur FP loads. Seven native mutants are rejected, including removal of
the otherwise-dead negative comparison, a DAZ-ignoring test, floating payload
copies, wrong raw control/errno, and the missing VM argument preparation.
No retail PE code is executed natively. Physical flag claims apply to the GCC
i386 build; native MSVC hardware equivalence is not established.

The emulator audit records important limits: Unicorn fails to quiet SNaNs on
FLD/FSTP, omits their hardware exception flags, and incorrectly applies MXCSR
FTZ to an x87 double store. Eight physical copy checks independently confirm
the real instruction behavior. Therefore native/original-emulator replay is
restricted to **1,997 supported cases**; **6,195** SNaN or DAZ/FTZ cases are
excluded from that replay, while **all 8,192** retain physical instruction-oracle
checks. SSE sticky status flags are checked by the hardware oracle, not treated
as an emulator-derived golden value. Emulated SNaNs can consequently take a different handler path; those traces
do not establish physical branch behavior. The recovered production code is
unchanged by these verifier exclusions. Reports, copy audits and mutation logs stay in
`build/analysis/crt-floor/`.

Native ownership remains explicit: a thread-local block provides only the
original errno field at `+8`; Win32 CRT thread initialization is not recovered.
The indirect callback adapter recognizes the recovered default PE token and
traps other tokens. The original CRT CPU/SSE initialization is also open:
production retains the copied zero BSS flag and uses the masked x87 path;
fixtures select both evidenced dispatch states. **General FP result/exception
handler `0x7675cb` remains a production trap**, as do power, asin and acos.
Unmasked/pending exception delivery, arbitrary invalid pointers, custom callback
porting, concurrency and rendered shader equivalence remain unverified. Finite
behavioral agreement is separate from instruction-byte identity.

The complete native build, affected VM dispatcher regressions and 60-frame
SKYLINE smoke pass. Wheel fingerprints remain `ca993eba` / `7141bcda`; all 829
texture records and staged handles retire to zero. Native GPU/effect activation
and the other wheel-rendering seams remain open.

### Verification (pixel size and LOD)

- `build/venv/bin/python tools/verify_pc_car_draw_lod.py --mutation-checks`, with `UG2_DIR` set. The executable's SHA-256
  and the manifest bytes are checked first. Results in `build/analysis/car-draw-lod/verification.json`: 512 producer cases
  (`0x620c6b`..`0x620cc0`, which runs `0x48acb0` inside: 123 behind, 107 near and 282 far by the same tests, 142 with the
  view-mode scale, 398 leaving the draw) and 2048 selection cases (`0x62107e`..`0x621230`: 303 leaving through `0x630d1b`,
  1088 with LOD 1 or more, 1526 with an override, 708 at the exact thresholds 4, 18, 20, 30 and 40). The frame cells
  `+0x64`, `+0x68`, `+0x6c`, `+0xe8`, `+0xf4`, `+0x14c`, `+0x180` and `+0x1ac`, the arguments `arg8`/clamp/LOD and the exit
  are compared. The original runs the image bytes; the rebuilt routines are MSVC 7.1 `/Ox /Ob0 /EHsc` objects linked to the
  same image addresses. Both x87 precision settings (`0x027f`, `0x037f`) are used.
- Mutation checks: 14 of 14 wrong variants rejected with the "state or call order mismatch" message (table threshold `<=`,
  wheel flag `<=`, float loop compare, owner rule, arg8 threshold, exit condition, override order, zero-flag clamp, search past
  the count, owner flag, scale gate, scale value, near result, behind test).
- Limits: random and special values only; byte identity is not claimed; the car records, the owner and the view-mode
  globals are fixture values.
- Native check (log and screenshots): `UG2_DEBUG_CAR_LOD=1 build/native/ug2demo --game "$UG2_DIR" --car SKYLINE --frames 60
  --orbit 90 --orbit-distance D --screenshot build/wheels_lod_orbit90_dD.ppm`. `--orbit-distance` (default 5 m) moves the
  inspection camera. Measured (focal about 743 pixels): 5 m: LOD 0, wheels in the arches (`build/wheels_lod_orbit90.png`);
  40 m: pixel 61, LOD 0; 60 m: pixel 39, LOD 1; 80 m: pixel 29, LOD 2; 100 m: pixel 23, LOD 2; 130 m: pixel 17, LOD 3 and
  `flags1AC` 1. At 60 m and 130 m (90 degrees) and 130 m (180 degrees) the scenery blocks the view, so those screenshots show
  no car; the log is the evidence for them. `--camera 4` keeps the drive camera about 6 m from the car, so it stays at LOD 0.

### Verification (wheel block)

- `build/venv/bin/python tools/verify_pc_wheel_draw.py --mutation-checks`, with `UG2_DIR` set to the game directory (or
  `UG2_EXE` to the executable). The executable's SHA-256 is checked against `config/SPEED2_EXE_1_2/wheel-draw.json`.
  Results in `build/analysis/wheel-draw/verification.json`: 512 layout cases (`0x61fe10`, real `0x60fef0` lookup) and
  2048 block cases (`0x62700e`..`0x62df66`), with 28869 compared callee events (`Alloc4` 3062, `Alloc700` 3062, `D3DX`
  3320, `SetSlot` 5460, `Visible` 6804, `DrawSolid` 4963, `Fn60d700` 1586, `Fn620600` 612), the four set frames compared
  with the original frame cells, and the whole arena.
  Case coverage: 899 with non-zero spin bytes at body LOD 0 and 1 (spin pre-rotation), 324 with LOD gate 6 at view clamp 0
  (the rear LOD substitution is reachable; it applies when the `Fn60d700` result is below the threshold), and 352 with
  LOD gate 3 (the tail, body LOD 0 to 2).
- Mutation checks: 14 of 14 wrong variants rejected. Each variant runs 128 block cases and 64 layout cases and must fail
  with the "state or call order mismatch" message: `tyre-flag-cc-dropped`, `spin-sign-wheel1`,
  `spin-translation-not-restored`, `stage-result-rounding`, `lod-threshold-direction`, `tail-gate-value`,
  `rear-brake-solid`, `brake-zero-element`, `pivot-column`, `block1-copy-source`, `solid-link-key`, `layout-rear-flag`,
  `wrong-position-buffer`, `wrong-source-buffer`.
- Limitations reported by the verifier: random and special values only (exhaustive float coverage is not established);
  the LOD gate, threshold and width globals are fixture values, not game runtime values; the body matrix (`EBX`) is an
  explicit input; the spin values produced by the image constants are only 0, 637 and 1274; byte identity of the
  compiled code is not established.
- Behaviour (native, visual check only): `build/native/ug2demo --game "$UG2_DIR" --car SKYLINE --frames 60 --orbit 90
  --screenshot build/wheels_lod_orbit90.ppm`, converted to PNG (`build/wheels_lod_orbit90.png`). The wheels and brake discs
  sit in the arches at LOD 0, with the spin zero in the demo. This is not a pixel comparison. The LOD 1 to 3 distances are
  listed in the pixel size section above.

### Remaining seams and limitations

- `DrawSolid` (`0x5c5930`) and queue `0x5ceb30` are recovered and verified, with texture swaps. Native activation
  needs texture/technique setup and GPU consumer `0x5cf420`; native car tables now load through recovered pack callbacks. The current native seam draws GL lists,
  ignores flags/extra/zero and does not set `glFrontFace` for R180 wheels; see the dispatcher evidence above.
- Preparation `0x48f660` (recovered and active; GL record consumption remains open), facing helper
  `0x60d700` (recovered) and secondary draw `0x620600` (open; the demo
  gate is 0, so neither is called), solid slots `0x48d860` (setter and native retail table binding recovered; link values `ri+0x390`/`0x3ac` are zero
  in the demo), rim-part records `0x60fef0` (none in the demo; the `SPINNEROFFSET` offsets stay zero).
- The spin term reads the car's sub-object bytes `+0x353`/`+0x354` (`0x59a420`). The block applies it only to wheels 0
  and 1 (tyre and brake frames, as a rotation about x; the table above). The demo has no such state, so the spin is zero
  and the block pre-rotates no front wheel; the rear wheels are never pre-rotated by the block. The tyre roll is not in the
  block either. `GetWheelRenderMatrices` (`0x5f6820`) copies the state frames `wheelMatricesF0[w]` (car `+0x130`), which
  `0x5921b0` builds with `RotateMatrixY` (called at `0x5922fb`); `GetWheelSpinMatrices` (`0x5ec7f0`) builds the third set
  with `RotateMatrixY` (called at `0x5eca3a`). The tyres therefore turn through the state frames and the third set, not
  through the block's spin term.
- The layout inputs of the wheel block `ri+0x400..0x43c` come from `CarTypeInfo` and the parts. The other caller inputs
  are the body LOD (`frame+0x6c`), the view clamp (`frame+0xf4`) and the wheel flags (`frame+0x1ac`, with `arg8` `|0x800`)
  from the recovered selection; the slot index `k` (`frame+0xd8`) is zero in the demo, its source open (see above); and
  `frame+0xcc` is zero in the demo (producer `0x60d1b0` not recovered).
- Widths `ri+0x430`/`ri+0x434` (the `MARKER_BRAKE` values, see the replacement inventory) are zero in the demo. They only
  feed `set2`, which no draw reads, so they do not change the drawn wheels.
- Constants of the image are checked by the verifier (`IMAGE_CONSTANTS`). The LOD gate and threshold globals
  (`0x8654a4`, `0x8a1c5c`) and the width overrides (`0x7a668c`, `0x7a6690`) are fixture values in the verifier. The demo
  sets the gate, the threshold and the overrides to 0, and the one and beta runtime values (`0x784250`, `0x8026c4`) to
  1.0 and 0.184.
- Intermediate float values are kept at the original precision (53-bit or 64-bit, both checked by the verifier). The
  native build uses x87 without `-ffloat-store`, so its intermediate precision follows the compiler and may differ in the
  last bit.

## Drive camera and HUD (fifth pass)

Recovered and applied by the demo (behaviour, not byte matching; `DriveCamera.cpp` and `HudGauge.cpp`
compile with MSVC 7.1 `/Ox /Ob0`):

- **Drive camera**, the mover Player::SetCameraState (`0x5f1fd0`) builds for state 1: constructor
  `0x451660`, update `0x453570`, pose `0x44de90`, setting read `0x445eb0`, car frame `0x445e00`.
  Eye, look-at and up are cubic Hermite followers (`0x5749e0`/`0x574df0`, restarted on every target
  change, duration from the dynamics key) in car-local axes. Targets come from the per-car camera
  table at CarTypeInfo `+0x720` (GLOBALB.BUN: six 16-byte `{type, angle, x, height, lateral}`
  entries, types 0 far, 1 close, 2 bumper, 3 driver, 4 hood, 5 super far; option to type map
  `0x502500`) plus the 4-frame average of the car acceleration (`car +0x80`) scaled and clamped by the
  static dynamics curve table `0x7ec610` sampled at the constant `0x787154`. Field of view per type
  is the table `0x7eb5b4` (71.5 degrees chase, 90 cockpit); near plane 0.5 (0.4 driver); projection
  from `0x5c7ec0` (half-angle tangents from the field of view and the 1.1 / 0.8696 aspect constants).
  Bumper and driver types hide the car (Camera `+0x280`). Keys: `C` cycles far/close/hood/bumper,
  `V` looks back (hold); `--camera N` picks the profile option 0..4 (default 3, outside far).
- **HUD**: `HUD_SingleRace.fng` from `GLOBAL/InGameFreeRoam.bun`. FNG layout (package `FEn\xe7`:
  `PkHd`, `TypS`, `Res\xcc` with `RsNm`/`RsRq` (50 six-dword requests, name hash, name offset,
  kind), `Obj\xcc` with `FOb\xea` blocks of `ObjD` 16-bit tag/size records and `Scrp` scripts) is read
  from the loaders `0x5554a0`/`0x555140`/`0x5515e0`/`0x518380`. Object state block: colour (B, G, R, A
  dwords), pivot, position, rotation quaternion, size and for images UVs. Position `OP +0xc` low half is
  the resource index, `PA` the parent GUID, flag bit 0 hides (`0x50ca00`/`0x50ca50`). Coordinates are
  640x480 with the origin at the screen centre; lower z draws in front. Object hashes are the case
  folded `h = h * 33 + c` (`0x505450`) of names found in the executable (`3rdPersonNeedle`,
  `3rdPersonSpeed`, `3rdPersonGear`, ...). The gauge feature is the recovered `0x4f5100`/`0x4c68d0` (`HudGaugeFeature::Update` in
  `HudFreeroam.cpp`):
  dial type `0x60fe10` (7500..10500 scale), needle sweep `(maximum rpm / scale) * 235` degrees, needle
  angle `0x4aa0e0`, speed `|v| * 2.237` mph or `* 3.6` km/h, gear characters from `0x7f66a4`, gear
  colour while shifting, skin textures `TACH_FILL_CUSTOM_nn`, `3RDPERSON_*LINES_CUSTOM_nn` and
  `3RDPERSON_RPMDIAL_CUSTOM_nn` from `HUD_CustomTextures_ALL.bin` (`--hud-skin N`, 0 default), unit
  text `MPH`/`KM/H` by hash from `LANGUAGES/English.bin` (`--metric`).

Scaffolding in this pass: the FEng object model and renderer (`NativeFng.cpp`), UI text (see
[UI fonts](#ui-fonts)), the acceleration vector at car
`+0x80` (its original producer is not recovered; it is the change of the exported velocity per second,
inferred from its camera use and from `0x5ec3a0` dotting it with the forward axis), the player
record and its camera mover (`NativePlayer.h`: one player bound at vehicle creation in
`0x8900ac`/`0x8900b4`, car `+0xc` set to its CarTypeInfo record; a stand-in kind-1 chase mover
at `+0x238` because the camera is native, whose type-change slot `+0x20` leaves the switch to the
native camera), and the controller
call without a player argument (so the rear-view mirror view `0x5f1e10` is not created).
Recovered `0x600090` produces the masks and recovered `0x4ca090` applies them; laps and
`tach_fill` colouring remain seams.
The full gauge update `0x4c68d0` (speed, needle, gear, shift light), turbo and NOS updates are
now recovered; the demo's `--metric` choice is written to the profile option `0x83aa1a`. Region/cash/reward
widgets accept external career state; their visibility follows the recovered masks
(cash needs career mode `0x850078` = 1, which the demo leaves 0). The bank balance is stored at the
original `0x861e74` and read by recovered `0x500f90`; the native loader publishes
language 0 (English) at `0x7f70d0` for recovered `0x4ff970`. GPS, heat and notification scheduling have bounded source
verification but are not live native features. Script playback is original (see
the pause-menu playback evidence). Complete NOS stencil rendering and FEng
messaging/loading remain unresolved; see
[freeroam HUD evidence](pc-hud-freeroam.md).
The minimap uses recovered `0x4ca840`, TrackInfo record 4000, `TRACKMAP4000.BIN`,
and the retail map/mask/north/player objects. Its border group follows recovered
mask application `0x4c6690` and recursive FEng show/hide `0x50ca50`/`0x50ca00`;
native free drive supplies mask bit 0. Its native pool setup and OpenGL
RGB/stencil mask rendering remain platform seams; see [minimap evidence](pc-hud-minimap.md).

The update tail builds the view matrix with recovered look-at `0x5ba9a0` and, for the chase
types, runs the recovered world and other-car pushes `0x444530`/`0x44b040`
([camera recovery](pc-camera.md#drive-camera-integration-2026-10-18)). Platform seams for them:
the mover's vtable slots 0x30/0x44, the world car list holding only the player's car, and the
GL path reading eye/target/up back from the matrix.

Camera shake (`0x452ed0`, `0x572a50`) and finalisation (`0x447280`) are recovered ([camera](pc-camera.md#camera-shake-and-finalisation-2026-10-19)); the Camera object is a zeroed native object and the GL path reads it. The collision shake that feeds the view shake (`0x572e50`, with the view shake states `0x572810`, `0x5728c0` and `0x5729e0`) is recovered too ([collision shake](pc-camera.md#collision-shake-2026-10-19)): the human car's race entry (car `+0x18`) is bound to the native player record in `BindNativePlayer`, the world gate `+0x108` is 1.0 and the states are reset in `InitialiseNativeRuntime`, and `CameraViewShakeTick` runs before the world clock in `StepNativeVehicle`. Not recovered, left as seams: the replay RPM source and
HUDINDEX option. The cockpit blocked tests are recovered; the mover's vtable also supplies slot 0x24
and the car input's slot 0x54 is `NativeControls::GetWheelMode54`.

## Pause menu (sixth pass)

`Esc` opens UG2's pause menu over the frozen game. The menu logic is the recovered
`PauseFNGObject` (`src/Speed/Indep/Src/Frontend/PauseMenu.cpp`, constructor `0x4eb410`, handler
`0x4c2170`, option lists `0x4c2900`/`0x4c2bd0`, item `React` handlers), verified separately in
[pause menu evidence](pc-pause-menu.md). In the demo's free drive (`0x89e7b0` = 0, game mode
`0x850078` = 0, both untouched .bss) it builds Return to Free Run, Options, Replay (disabled: no
replay data, so navigation skips it) and Quit to Main Menu. Left/right (or A/D) move, Enter/Space
selects, Esc/Backspace goes back.

Data: `UI_Pause.fng` and `UI_InGameDialog.fng` are not plain `0x30203` packages; they are
`0x30210` chunks in `GLOBAL/InGameCommon.bun` holding the package-name hash (`0x505450`) and a JDLZ
block, unpacked with the recovered decompressor. Textures come from `InGameCommon.bun` (icons) and
`GLOBALB.BUN` (menu frames). Script `EV` records (12 bytes: message, target, milliseconds) are now
read; `UI_Pause.fng`'s `FORWARD` script sends `EXIT_COMPLETE` (`0xe1fde1d1`) to the package at
320 ms, which completes back, resume and the non-dialog items.

Seams in `src/Platform/Native/NativePause.cpp` (scaffolding, behaviour as read, not verified):

- Remaining IconScroller base construction/dispatch (`0x552330`, `0x543d40`):
  icons fill `option_1..n` 40 units apart,
  anchored on the hidden `OPTION_MASTER`; unselected icons are drawn at 0.8 and centre aligned
  by recovered `0x52f8c0`, shown inside a 480-unit window (`0x50d9c0`); moves skip disabled icons without
  wrapping using recovered `0x51f9f0`; accept reacts immediately for dialog items (`+0x45`) and after
  `EXIT_COMPLETE` otherwise, only for the focused icon object (`0x50d980`). Scroll animation now uses recovered tweening;
  sounds and Dim/UNDIM scripts are not modelled.
- Deferred message storage `0x50da20` and input stop/start `0x52fbf0`/`0x52fba0`
  now call recovered code, including the original package-mask writes. Native
  input gates use scroller +0x2f; owner stopped state lives at +0x14c. Full input
  dispatch and package membership remain seams. See [bounded evidence](pc-pause-menu.md#scroller-deferred-messages-and-input-masks-2026-10-18).
- Icon construction `0x51f670`, insertion `0x543c10` and owner wrapper `0x50dab0`
  now call recovered code. Insertion binds typed images, preserves size/colour
  call order, appends original list links and requests first-icon focus; the
  wrapper writes texture/dirty state. Native vector ownership remains a seam.
  See [insertion evidence](pc-pause-menu.md#icon-construction-insertion-and-texture-dispatch-2026-10-18).
- Selection label updates now call recovered `0x50da50`, using bound title
  +0x134 and description +0x138 objects. Named-string wrapper `0x4953c0`
  executes recovered hashing before typed string lookup. Native construction
  binds `ICON_TITLE` and the original empty-name description; localization remains a seam. See [label evidence](pc-pause-menu.md#bound-scroller-labels-and-named-string-lookup-2026-10-18).
- Language-hash setter `0x50c900` now calls recovered code for both menu labels
  and the GPS label requests. Hash +0x60, dirty writes and observer notification
  follow the original. A native observer at `*0x838478` supplies language lookup
  and byte conversion; UTF-16 ownership now uses recovered storage methods
  `0x511630`/`0x51de90`/`0x51dee0`/`0x52d610`/`0x50d390`. String-manager
  internals and complete string ownership remain seams. The native loader
  preserves string SH hashes instead of overwriting +0x60 with a child count.
  See [setter evidence](pc-pause-menu.md#language-hash-setter-and-observer-notification-2026-10-18)
  and [owned UTF-16 storage](pc-pause-menu.md#owned-utf-16-storage-in-native-strings).
- String property reader `0x534fc0` now applies all seven original string tags,
  calling recovered member label setters `0x4fe940`/`0x519f40`, actual hash and
  UTF-16 storage methods. Native parsing defaults optional `SL` retention off and
  retains bounds, terminated input and renderer byte projections as seams.
  Observer binding uses relocated native storage; original localization and
  surrounding reader ownership remain pending. String teardown now calls recovered
  normal destruction `0x4956a0` and base cleanup after a native adapter detaches
  borrowed inline state and vector-backed list prefixes; scalar deleting wrapper
  `0x495780` is separately recovered and verified. Complete original ownership and
  production virtual dispatch remain pending.
  See [string reader evidence](pc-pause-menu.md#complete-string-property-dispatch-and-member-label-helpers).
- Object name setter `0x5176b0` and inheritance lookup/resolver
  `0x504960`/`0x50b240`/`0x5186c0` now have original-execution evidence:
  8,192 comparisons and 13 mutation rejections with clang-cl. Optional native
  `Load(..., retainNames=true)` applies recovered `On` name and `SL` label
  storage with bounded input and normal destruction; existing callers default
  false. Three asset-free CTests include retained names/labels and the actual
  resolver/GUID tree, including first-package failure and aliased outputs.
  Production inheritance binding and cloning remain pending; this does not
  establish the original initialization of reader byte `+9`, MSVC matching,
  or complete outer-reader recovery. See `docs/pc-pause-menu.md`.
- Response parsing now calls recovered command constructor `0x4fe910` for
  three zeroed words. Command cleanup `0x517600`, byte-string setter `0x517630`
  and assignment `0x52ad80` pass 16,384 original/rebuilt comparisons and 12
  mutation rejections. Native fixtures cover deep-copy and self-assignment
  behavior. Recovered resize `0x52ae00` and CRT iterator normal paths now own
  command arrays; MC sizes storage and bounded Rs strings call the recovered
  setter. Native movable headers release arrays via recovered resize-to-zero.
  Resize/iterator verification passes 12,288 comparisons and 14 mutations.
  Response headers now come from recovered 64-slot blocks: 11 constructor,
  allocation, release, list and destruction routines pass 5,632 comparisons and
  19 mutations. A native vtable/global bridge supplies the original virtual call
  boundaries; wrappers own pooled references. Complete response reader
  `0x54d1d0`, message lookup `0x5046d0` and package insertion `0x512120`
  pass 6,144 comparisons and 17 mutations. Native tag dispatch, manual response
  links and duplicate message metadata are removed; bounded records call the
  recovered reader, and wrappers adopt its headers. Original context setup,
  owner teardown, exception rollback and byte matching remain pending.
  See [reader evidence](pc-pause-menu.md#complete-response-reader-and-original-owner-list-insertion-2026-10-18).
  See [pool evidence](pc-pause-menu.md#response-header-pool-and-virtual-cleanup-2026-10-18)
  and [command ownership evidence](pc-pause-menu.md#response-command-construction-ownership-and-assignment-2026-10-18)
  and [array ownership](pc-pause-menu.md#response-array-resizing-and-native-ownership-2026-10-18).
- Script name `0x518730` and event storage resize `0x519a10` pass 8,192
  comparisons and 15 mutations. Native event vectors and duplicate hash/duration
  metadata are removed; EV uses recovered arrays and repeated EV replaces them.
  Optional Sn retention calls the recovered name setter. Clock/event firing,
  full script parsing/copy binding and production owner-list teardown remain seams. See [script storage evidence](pc-pause-menu.md#script-names-and-owned-event-array-storage-2026-10-18).
- Script track replacement `0x535210`, owned/borrowed key cleanup, the 256-slot
  key pool, and normal script construction/destruction now execute recovered
  code. Fourteen routines pass 3,584 comparisons and reject 21 mutations.
  Native track vectors are removed; bounded parsing projects payloads into
  original arrays and pooled key nodes. Key pointer indexes, initial baseline/time
  defaults, playback clocks and full script copying remain adapters or pending
  recovery. Byte identity and exceptional unwind remain unverified.
  See [track ownership evidence](pc-pause-menu.md#script-track-ownership-key-pool-and-normal-script-teardown-2026-10-18).
  Track assignment `0x54d350` is also recovered and exercised by the native
  fixture: owned sources clone pooled keys; borrowed sources retain their direct
  reference. Full script copying/reading has not yet been bound to it in the demo.
  See [assignment evidence](pc-pause-menu.md#track-assignment-and-ownedborrowed-key-copying-2026-10-18).
  Complete script copy construction `0x551be0` is recovered too, including
  shallow reference headers and deep assignment. Native fixtures exercise its
  ownership and teardown; the production script reader remains pending.
  See [copy evidence](pc-pause-menu.md#complete-script-copy-construction-2026-10-18).
- Script headers now come from recovered pool `0x54beb0` and return through
  deleting wrapper `0x53ee30`/release `0x525b90`. Blocks contain 32 headers;
  construction/destruction and empty-block cleanup execute recovered code.
  Six routines pass 1,536 original comparisons and reject 12 mutations.
  Native wrappers transfer stable pointers; playback metadata and event-clock
  adapters no longer cast original headers to embedded native wrappers.
  Original reader `0x5515e0`, its Ti registry binding and owner-list cleanup remain pending.
- The reader's Ti lookups `0x504a00`/`0x505480`/`0x5147f0` are recovered and
  verified (12,288 comparisons, six mutations); the registry at manager +0x4134 is
  typed but its producer is unrecovered. Retail scripts contain no Ti tags.
  See [Ti lookup evidence](pc-pause-menu.md#script-reader-ti-lookups-2026-10-18).
- Every `Scrp` chunk is read by recovered `0x5515e0`, which creates, fills and
  links the original headers, tracks and keys; the native tag parser and header
  binding are removed. Seams: bounded input (incomplete tags, short EV, streams
  without Sn/Sh), a null type-registry queue, and native playback metadata
  mirrored from the original tracks. 4,096 streams, 22 mutations.
  See [reader evidence](pc-pause-menu.md#complete-script-reader-and-native-integration-2026-10-18).
- Script playback is original: per-object update `0x54ce60`, apply `0x541db0`,
  events `0x534c70` and the evaluators/interpolators beneath them. The native
  frame seam converts seconds to whole milliseconds in ≤20 ms steps (plus a
  zero-length step for screens shown during the message pump); the HUD
  posts events to a discarded local queue. Native `ApplyTrack` and the pause
  event scheduler are removed.
  See [playback evidence](pc-pause-menu.md#original-script-playback-in-the-native-demo-2026-10-18).
  See [script pool evidence](pc-pause-menu.md#script-header-pool-and-native-stable-ownership-2026-10-18).
- Native image/group/list teardown now invokes recovered `0x532660`/`0x534bf0`/
  `0x534a50`, alongside string destruction. Original list storage/cell release
  `0x516dc0`/`0x5042e0` and the three deleting wrappers are verified together with
  base/pool/list cleanup. Native adapters detach borrowed children, state
  and vector-owned headers; original production ownership remains pending.
  See [typed cleanup evidence](pc-pause-menu.md#image-group-and-type-4-list-cleanup).
- Type-4 `Ld` parsing now calls recovered column/row allocation `0x52a910`/
  `0x52ab30`, including actual `0x401650` constructor iteration and `0x504150`
  cell construction. Axis vectors are removed; navigation uses original allocated
  arrays. A native per-wrapper heap owner releases surviving allocations after
  recovered teardown while preserving original resize/release quirks. Full list
  property dispatch and cell payload handling are now active through recovered
  `0x52b740` and its actual colour/type/pair/triple/four-word/UTF-16 callees.
  Native bounds, context setup and heap ownership remain adapters; outer reader,
  inheritance and cell rendering remain pending.
  See [list storage evidence](pc-pause-menu.md#original-type-4-axis-and-cell-allocation)
  and [complete list reader](pc-pause-menu.md#complete-type-4-list-property-reader-and-cell-payload-writes).
- Typed string/image lookup (`0x537a10`/`0x537a60`), generic wrapper `0x5379c0`,
  recursive object traversal and package-list fallback now execute recovered code.
  Native loading constructs root/child links; shown-screen membership and order,
  archive record construction, and the legacy `FngFindObject` adapter remain seams.
  Package locator `0x52cef0` now selects the two live lists or archive boundary
  through recovered code; the native demo initializes its mode flag to zero.
  Archive search `0x51bb70` also executes recovered code, including hash/header
  checks and list promotion. Native scaffolding still supplies shown-screen
  archive membership and simple hash records.
  See [lookup evidence](pc-pause-menu.md#object-tree-and-typed-package-lookup-2026-10-18).
- FEng (`0x537a10`, `0x50c900`, `0x537c00`/`0x537cc0`, `0x55dda0`, ...):
  packages are reloaded on every showing; messages are queued and delivered to the screen's object;
  scripts fire package events and the bounded animation tracks described in the settings pass below.
- Pause icon placement/reset/frame tick and cubic tweening now use recovered routines.
  Smaller icons centre-align; the icon row moves under a fixed selection frame, with placement
  preceding tween advancement. Native scroller construction remains a seam. Name selection `0x51cf70`,
  pointer assignment `0x5178b0` and track activation `0x517720` now use recovered
  code and arithmetic helpers; native loading supplies key nodes and event
  delivery observes the selected header. Playback clocks and event delivery
  remain seams. See [activation evidence](pc-pause-menu.md#script-selection-and-track-activation-2026-10-18).
  Colour, lock/arrow-script policies and scrollbar geometry use recovered routines;
  LOCK/BUTTON_MASTER and all ScrollBar objects now bind the retail objects.
  ScrollBar setup calls recovered constructor `0x542ac0`. See [placement evidence](pc-pause-menu.md#scroller-placement-and-cubic-tween)
  and run `python tools/smoke_native_pause_scroller.py` for rendered motion checks.
- Script hash selection `0x51cfe0`, package setters `0x537c00`/`0x537c60` and
  running queries `0x537cc0`/`0x537d40` now execute recovered code. Predicates read
  selected headers with signed inclusive time checks; native event-list queries
  and hash-selection loops are removed. HUD/Controls name selection also calls
  recovered code. Playback/event delivery remain seams.
  See [script wrapper evidence](pc-pause-menu.md#hash-selection-package-script-wrappers-and-running-queries-2026-10-18).
- Native package GUID/hash helpers now call recovered first-match tree queries
  (`0x517fe0`/`0x517f80`), replacing their flat-vector lookup loops. GUID callback
  `0x5048f0` compares object +0xc independently of the name hash. Native loading
  still supplies tree links and observed package prefixes.
  See [GUID lookup evidence](pc-pause-menu.md#guid-lookup-and-native-tree-adapters-2026-10-18).
- Response conditional traversal `0x504490` now executes recovered code over
  decoded 12-byte command records, including the original count boundary.
  Full response dispatch `0x52c7c0`, script lookup `0x5046b0`, focus cache
  `0x50b300`/`0x50b360` and stack reset `0x50b390` now execute recovered code.
  Message delivery `0x541fc0`, object delivery `0x5367e0`, and directional wrappers
  `0x51b000`/`0x52c750` now execute recovered code. Queued response masks are preserved.
  Native loading projects linked response prefixes and explicit Targ subscription rows.
  Operation consumer `0x5559c0` also executes recovered code. Package load/remove,
  four package-operation enqueue boundaries and the type 6 navigation mutator still
  trap; parser ownership, owner callbacks and registry data remain native.
  Type 4 navigation, offset setup, selection and motion now use recovered code
  over a native 0xac-byte prefix/axis-vector projection. Its allocator, cells,
  cell rendering and frame clock remain seams. Asset-free CTest `feng_list_native`
  checks the actual native parser through navigation and settling.
  See [list-state evidence](pc-pause-menu.md#list-setup-motion-and-native-projection-2026-10-18).
  Type 6 navigation and both gates are now recovered/verified with controlled
  refill interfaces. Production activation still needs the 0xc8-byte grid model
  and original owner behavior. Initial fill `0x504c40` is also recovered and
  verified, including cached extents/origins and reloaded callback targets and
  logical divisors; production reader/owner binding remains pending. No native
  cell-population algorithm is supplied. Asset-free CTest `feng_grid_native`
  exercises navigation and initial fill with synthetic virtual/cdecl callbacks.
  Grid resize `0x52bb30`, cell reset `0x504290` and logical dimension setters
  `0x518bb0`/`0x518c00` are recovered too. The native probe composes storage,
  fill and navigation with synthetic cells/pool data; native allocation/free
  are documented malloc/free seams. Production constructor/reader/owner binding
  remains pending; the type-6 navigation entry still traps.
  String-pool resize/migration `0x518c60` and rectangular cell setters
  `0x5196b0`/`0x5197d0`/`0x5198f0` are also recovered. The synthetic native probe
  checks UTF-16 truncation/rebinding and aliased pair writes; original reader
  composition, text assignment and owner population remain pending.
  The common FEng constructor `0x52af20` now executes in the native FNG loader,
  replacing whole-prefix zeroing and native GUID-zero defaults. Grid constructor
  `0x52b9e0` is recovered and composes with storage/fill/navigation in the asset-free
  probe; production type-6 construction/reader binding remains pending. Unknown
  base fields retain explicit native defaults, and native root/child chain links
  remain a loader seam. See [constructor evidence](pc-pause-menu.md#common-feng-and-grid-construction-2026-10-18).
  Type-6 property dispatch `0x535060` is recovered with its actual storage/pool
  and cell-property callees. The native component probe executes this reader;
  native chunk iteration/factory/owner binding still require recovery, so this
  does not activate production type-6 navigation or supply cell text population.
  Normal-return grid/base cleanup, pooled-state release and owned-list clearing
  are recovered and compose in the synthetic native probe. Production cleanup
  still uses native ownership: remaining virtual destructors, native state/factory
  integration and SEH unwind remain pending. See [lifecycle evidence](pc-pause-menu.md#normal-return-grid-and-object-cleanup-2026-10-18).
  State-pool acquisition and concrete block/class virtual destruction are also
  recovered. A synthetic native round trip allocates 40 slots across two blocks,
  distinguishes full-word class keys, and returns all storage through recovered
  release/cleanup. Fixed virtual slots are relocated only inside that test;
  production state/factory ownership binding remains pending.
  All branches of object factory `0x534df0` and its remaining constructors are
  recovered. Native image/string/list/group and types 7/9/10/11 now execute those
  constructors once; the type-4 default memset/assignments are removed. Native
  type selection, larger wrapper allocation and inline state remain documented
  seams; production type-6 prefix/owner binding remains pending.
  Name hashing `0x505450` now executes recovered source; the native hash loop is
  removed. See [grid/hash evidence](pc-pause-menu.md#type-6-grid-navigation-and-original-name-hashing-2026-10-18).
  See [message delivery evidence](pc-pause-menu.md#full-queued-message-delivery-2026-10-18).
  Native loading now selects/initializes Init as observed in the original readers. See [response evidence](pc-pause-menu.md#response-conditional-traversal-2026-10-18).
- GUI formatted hashing `0x50b9c0`, buffer wrapper `0x50c960`, shared profiling
  wrapper `0x43fbc0` and elapsed accounting `0x43e5c0` now execute recovered code.
  The buffer wrapper ignores capacity as the original does. Native SDL counter
  access/calibration remain platform seams.
  See [formatting evidence](pc-pause-menu.md#gui-formatting-wrappers-and-profiling-2026-10-18).
- Formatter core `0x43e950`, output helper `0x43cb40`, pointer validator
  `0x43bd60` and recursive vector wrapper `0x4400d0` now execute recovered code.
  The native host parser and temporary allocation are removed. Linux page-permission
  probes and SDL clock/calibration remain adapters; parser/string/locale data still
  bind the mapped reference image. Finite original/rebuilt verification covers
  bounded conversions, including custom vectors and fixed-point values; unsupported
  conversion-class paths remain unverified.
  See [core evidence and limits](pc-pause-menu.md#formatter-state-machine-recovery-2026-10-18).
- Message/input enqueue `0x51aa20`/`0x51af50` and removal `0x505400` now use
  recovered 32-byte nodes and separate linked lists. The native message vector
  is removed. Message/operation destructor bodies and payload cleanup are recovered;
  delivery and native cancellation call recovered message deletion. Native malloc/free,
  owner-to-screen delivery and deleting-vtable ABI bindings remain adapters.
  Asset-free queue integration is available as CTest `feng_queue_native`.
  See [queue/consumer evidence](pc-pause-menu.md#full-queued-operation-consumer-2026-10-18).
- FEng image centre/size writes and rendered text height now call recovered coordinate
  and metric routines. String and script-track prefixes expose the original offsets;
  position writes update live state and matching script baselines. Low-level font lookup,
  line measurement and script playback remain seams; see [coordinate evidence](pc-pause-menu.md#feng-coordinates-text-metrics-and-script-state).
- Icon selection now calls recovered `0x52f680`; indexed selection `0x52f7c0`, list
  helper `0x43be60` and focus-flag helper `0x51cc60` are recovered too. The embedded
  scroller has its complete observed 0xe8-byte extent, with native vector/ownership state
  stored separately. Focus flags and the saved scroller index are written by original
  code; scroller construction remains a seam. See
  [selection evidence](pc-pause-menu.md#icon-selection-and-the-full-scroller-layout).
- Icon stepping/indexing/selected-item dispatch now use recovered `0x51f9f0`, `0x51fac0`,
  `0x5276d0`/`0x527700`, `0x527730`/`0x527750`, `0x51f820` and `0x50d980`.
  The native scroller owns the original list/current-icon prefix; its constructor,
  construction and rendering remain seams. See [navigation evidence](pc-pause-menu.md#iconscroller-navigation-and-dispatch).
- Package masks, persisted selection and focus now use recovered routines `0x537950`,
  `0x50b8a0`/`0x50b8c0`, `0x52b3a0` and `0x52ce40`. Native screens retain FEPackage
  masks/focus/array offsets, archive-node package `+0x18` and a separate static FNG selection
  record. Object/record/archive loading and queue dispatch remain seams; see
  [package evidence](pc-pause-menu.md#feng-package-masks-selection-and-focus).
- FEng sprite rendering: the object quaternion is projected onto the screen plane, so the
  180-degree x/y turns UI_Pause uses to mirror frame corners and bar caps draw as mirrors; sprites
  with UVs inside [0,1] sample with GL_CLAMP_TO_EDGE so linear filtering does not wrap edge texels.
- In-game dialog `0x558790`: two-button form of `UI_InGameDialog.fng` with the caller's labels,
  text and result messages; value `0x1C` is read as the initially highlighted button (Cancel).
- Game flow `0x5f4240`/`0x5f41e0`: messages 1/2 resume, 3 restarts like `R`, 6 (Quit to Main
  Menu) exits the demo, others resume with a log line. The replay
  screen is not available and resume the game. The PC help bar (`0x54e6a0`/`0x54e6e0`) is tracked,
  not drawn. Real clock `0x8651ac` advances at an assumed 4 kHz so `AcceptsInput` (`0x4a7bc0`)
  opens input one frame after the screen.
- The pointer table at `0x8900ac` shares the native HUD player slot, or a native
  fallback when the HUD is disabled, so `0x4a7bf0` offers Options.

Text justification (`Sj`) is read as flags from retail strings: bit 0 centres and bit 1
right-aligns (dialog button labels sit at their button centres with `Sj` 1; HUD speed digits
use 2). This corrected the native HUD's swapped centre/right handling.

`--pause-at N` opens the menu at frame N and `--pause-keys LRABUDS` presses left/right/accept/back/up/down/start
every 30 frames afterwards; finite `--frames` captures with `--pause-keys` ignore external
keyboard/mouse/controller UI actions while accepting the scripted input.
 `UG2_DEBUG_PAUSE=1` prints delivered messages and game-flow requests.

## Platform scaffolding (not original code)

`src/Platform/Native` contains everything that is not recovered UG2 code:

- `OriginalImage.cpp` maps the player's own `speed2.exe` `.rdata`/`.data` at their
  original addresses after a SHA-256 check; `OriginalSymbols.ld` binds recovered
  `extern` globals to those addresses. No original instruction executes.
- `NativeVehicle.cpp`: vtable shims forwarding each original slot to the recovered
  method, the `New...` allocation factories, the D3DX matrix multiply,
  scaffold race-status/context/random
  objects, keyboard capture, and the GLOBALB.BUN CarTypeInfo chunk kept resident
  and installed as original table pointer `0x8a1ccc` (the chunk loader is
  unrecovered). `CreateNativeVehicle` binds the tuning object to its car,
  selection and record (the original caller is unrecovered) and runs recovered
  composer `0x5b4dc0` in free-drive mode without the performance run: stock
  build, extras, tuning when active, drive curve and the original class index.
  The tuning-table loader is unrecovered, so no tuning entries are installed and
  full tuning does not run natively yet. See
  [physics parameter evidence](pc-physics-parameters.md).
- `DemoMain.cpp`: SDL2/OpenGL window and input; `NativeFng.cpp`/`NativeHud.cpp`: FNG HUD;
  `NativePause.cpp`: pause menu seams (see above); `NativeAssets.cpp`/
  `NativeScene.cpp` read and draw the original car and scenery assets (see above).
  `--placeholder` restores the box car and collision-polygon world.
- `NativeAudio.cpp`: the snd library seam over SDL2 (a packet player with resampling; see
  [Ginsu evidence](pc-ginsu.md#native-packet-player-and-engine-sound-scaffold)). The snd
  library itself is not recovered. `NativeEngineSound.cpp`: the engine sound of the demo
  (gin binding, the synthesizer and the per-frame RPM feed); the per-car gin choice and the
  engine controller are not recovered.
- Unrecovered routines the demo state should never reach stop the program with a
  message instead of being stubbed (`NativeTrap`).

## Engine sound

The demo plays the engine through the recovered Ginsu synthesizer when an audio device is
open. Recovered participant selection chooses one or two files under
`<game>/SOUND/ENGINE` from the car type and upgrades. Recovered construction,
binding, bank loading and setup initialize the slots. Each frame runs
the recovered car scheduler: Physics and its post-update, shifting and its post-update, acceleration transitions, complete auxiliary,
HybridMotor and its post-update, spatial, then the separate playback pass for
both selected banks. The original component sort determines input order. Physics consumes the recovered vehicle export at owner
`+0x440`. Listener producer `0x46d7e0` reads the same finalized camera used
for rendering, then the spatial component computes distances, angles and
Doppler averaging. Recovered global MasterVol runs before per-car inputs and
shares the audio options mix record. Paused frames use the original zero time
step for the component updates. The native session supplies its one-car manager list;
registry/list construction, full manager dispatch, overall level/pan,
notifications and parameter consumers remain seams. See
[recovery and integration evidence](pc-ginsu.md#audio-mix-and-global-mastervol-2026-10-20).

```sh
build/native/ug2demo --game "$UG2_DIR" --car SKYLINE                       # engine sound on
build/native/ug2demo --game "$UG2_DIR" --engine-gin GIN_Mazda_Miata.gin    # another gin file
build/native/ug2demo --game "$UG2_DIR" --no-audio                          # silent run
UG2_ENGINE_LOG=build/engine-rpm.log build/native/ug2demo --game "$UG2_DIR" # one line per frame
```

- By default the recovered selector `0x4593b0` uses the participant car type and upgrades.
  `--engine-gin` explicitly overrides its primary filename and forces one-bank mode.
  See [selection evidence and remaining seams](pc-ginsu.md#per-car-engine-record-selection-2026-10-20).
- `--no-audio` skips the device. Without a device (no audio driver, or SDL refuses it) the
  demo prints a message and runs silent. The engine sound is not created then.
- `UG2_ENGINE_LOG` writes a header `# audio_start_ticks N` (the device timeline) and one line
  `ticks frame rpm` per frame, plus playback/control/auxiliary/spatial/acceleration/shifting/mix diagnostics.
  The smoke test pairs the output with this log.
- At exit the demo prints the audio counters: callbacks, packets released to the
  synthesizer, and device frames with no packet (underruns).

Headless check with SDL's disk driver, the scripted throttle and no display:

```sh
build/venv/bin/python tools/smoke_native_engine_audio.py --game "$UG2_DIR" --frames 600
build/venv/bin/python tools/smoke_native_engine_audio.py --game "$UG2_DIR" --frames 600 --car 350Z
```

It writes `build/analysis/ginsu/engine.raw` (S16 stereo, 44100 Hz) and checks that the output
is not silent, that underruns are rare, and that the pitch rises with the rpm. The
environment is `SDL_VIDEODRIVER=offscreen`, `LIBGL_ALWAYS_SOFTWARE=1`, `SDL_AUDIODRIVER=disk`
and `SDL_DISKAUDIOFILE` for the raw file; the script removes `DISPLAY` and `WAYLAND_DISPLAY`.
The disk driver paces output in real time, so the rpm log and the output share a timeline.

## Build and run

Requires i386 SDL2/GL runtime libraries and a multilib GCC.

```sh
cmake -S . -B build/native && cmake --build build/native
build/native/ug2sim "Need for Speed Underground 2" SKYLINE 960   # headless check on L4RA
build/native/ug2probe -579.8 533.2                                 # collision coverage probe
build/native/ug2demo --car SKYLINE                                 # W/S/A/D, Space, R, Esc pause
build/native/ug2demo --frames 60 --orbit 30 --screenshot car.ppm   # scripted capture
build/native/ug2demo --frames 125 --pause-at 20 --pause-keys RRA --screenshot quit.ppm  # quit dialog
```

The headless Skyline run accelerates through automatic shifts near 7,800 rpm and reaches
about 95 km/h when its scripted steering takes it into the roadside wall at eight seconds,
where it now slides along the wall instead of leaving the map. These are plausibility
observations, not equivalence evidence.

## Known limitations

- Visible-section selection and the player-view flag test are applied (see
  [scenery visibility](#scenery-visibility-fourth-pass)). Pixel-size LOD `0x5d64d0`,
  and the nearest-boundary fallback `0x5db770` remain unresolved. Context flags
  and their byte admission gate are now recovered and active, as documented above.
  Blended draws are not depth-sorted; streaming, original shaders, and time-of-day
  colour selection remain unrecovered. Missing textures include 18 TRN_SH parts
  (`0x9c3ce745`) and four XS signs, as recorded in the scenery investigation.
- Paint, vinyls, damage textures and headlight/brakelight switching are not applied.
- Every collision pack is loaded at start-up
  instead of the original streaming. The spawn is the original start grid
  ([track markers](pc-track-markers.md)); the participant/car setup around it is native.
- Steering input scaling (±30° road-wheel angle), steering smoothing, device capture
  and partial controls construction are scaffold. Original pedal/reverse processing
  `0x407a30` and forward selection `0x407310` now run after native action capture;
  see [the substitute audit](demo-substitute-audit.md) for verification and remaining work.
- Native GCC x87 code is not bit-exact with MSVC; FPU control word is set to 0x027f.
- Free-drive race phase 0 is used; race-phase-dependent branches are untested.

## Outstanding recovery checks

- Resolve the 42 degenerate simplex tie differences in `0x595040`.
- Recover car-vs-car/world-object paths (`0x5b31f0`, `0x5ae000`, slot 0x8c),
  the cylinder constructor (not located), and collision effects/sounds (`0x5a9620`, `0x59fb70`).
- Recover the car-update reset path (`0x5fbed0`, `0x40f690`, `0x6009a0`) and the world-update
  callees that are still no-op seams (`0x5f6cd0`, `0x5f5580`, `0x5f5730`, `0x602650`, `0x6076d0`, ...).
- Recover the remaining car pre/post-update logic
  beyond the verified acceleration prefixes, remaining HUD inputs/features, and full
  FEng script playback/rendering. Additional HUD
  source recovery requires separate verification and integration evidence.
- A free/aerial camera remains useful for wider scenery checks.

For reproducible wall checks, run:

```sh
UG2_STEER=-0.4 UG2_DEBUG_COLLISION=1 build/native/ug2sim "Need for Speed Underground 2" SKYLINE 900
```

## UI fonts

FEng strings use the retail bitmap fonts, not GDI (the only `CreateFontA` caller, `0x70d5f7`, rasterises
an 8-pixel debug font). Each font is a `0x30201` chunk: name at +0, texture name at +0x100 and an
`FNTF` glyph table at +0x200. `LANGUAGES/English.bin` holds Conduit ITC 14i and 38bi, Arial, Arial 12,
Impact 36 and LCD 48 (textures in `LANGUAGES/LanguageTextures.bin`); `GLOBAL/GLOBALA.BUN` holds
Conduit ITC 21i with its texture. A string object's font is its package resource (object `+0x24`).

Followed from the retail code: font construction `0x51bdc0` (via the `0x30201` handler `0x52cd00`),
lookup `0x51bd70` with the substitute table `0x7f9184` (38bi falls back to 21i), the per-font
offset/leading table `0x7f918c`, string layout `0x51c110` with glyph quads `0x51bfc0`, glyph lookup
`0x4fda70`/`0x6f45b0`, kerning `0x6f45f0`, line width `0x50bce0`, text height `0x50bf40`, line
count `0x50c080` and `$token$` skipping `0x50ba70`/`0x50bac0`. 'Sj' justification, 'Sl' leading,
'Sw' wrap width and `^`/newline breaks follow those routines. 'SH' is a string label hash
(`0x534fc0` passes it to `0x4fe940`), not a font.

Seams: the font list, chunk loading and OpenGL drawing are native; the layout port is behavioural
and not byte-verified. Glyph UVs are inset by half a texel so bilinear filtering does not reach the
neighbouring glyphs of the packed texture. The string's size scales its FEng units, glyph colour
uses the image colour path, and the `0x100` flag argument of the glyph quad is ignored. Only
English fonts are loaded, so the language-suffix stripping of `0x51bdc0` is not needed. Text is
8-bit, widened to the retail 16-bit characters. Strings whose font is missing fall back to a system
italic sans face rasterised with the vendored `imstb_truetype.h` (public domain/MIT);
`--hud-font PATH` forces that TrueType face for all text.

## Optional native mods

Configure with `-DUG2_ENABLE_MODDING=ON` to build the separate native plugin loader.
The default build remains unchanged in scope. See [Native modding](../modding/README.md)
for SDK/lifetime rules, installation, load order and physics-provider contracts.
The bundled `widescreen_hud` plugin moves verified driving-HUD anchors outward
and retains the recovered 4:3 vertical view while widening horizontal FOV.
Use `--width W --height H`; `--no-mods` restores the original native presentation.
Rendering and debug picking share the adjusted projection. Recovered camera and
physics routines are unchanged. `original_physics` forwards to the recovered solver;
custom physics is supplied by third-party providers through documented native seams.

## In-game settings integration

The pause menu's Options item now opens the recovered Audio/Display/Gameplay/Player
category screen and recovered option handlers. The native demo supplies the unrecovered
list/widget bases using the player's retail FNG objects and scripts. See
[settings evidence and limitations](pc-pause-options.md) for the distinction between
verified original code and platform seams.

Use left/right for category and value changes, up/down (or W/S) for rows, Enter to
accept, Esc/Backspace to return, and P/Pause to resume from settings. Units and camera
choices feed the demo; audio, display backends, persistence and other gameplay setting
effects remain incomplete. Changes are session-only. Scripted keys also support U/D/S.

```sh
python tools/smoke_native_pause_options.py
build/native/ug2demo --frames 260 --pause-at 20 --pause-keys RAALDL --screenshot build/audio.ppm
```

The settings, Controls and scroller-motion Python smoke scripts default to
headless SDL `offscreen` rendering with software OpenGL. Their child processes
have `DISPLAY` and `WAYLAND_DISPLAY` removed; no display server or Xvfb is needed
on the validated SDL/Mesa installation. The screenshot and pixel assertions still
run. Use `--game '/path/to/game'` and optionally `--demo '/path/to/ug2demo'` to
avoid a local asset symlink. `--headed` uses the inherited desktop configuration;
`--headless` explicitly selects the default. Systems without SDL offscreen OpenGL
support need the headed mode or an externally supplied display server. The demo
itself remains interactive by default. Emulation verifiers and `ug2sim` already
run without a display.

Generated screenshots for this pass: `build/analysis/native-options/{audio,display,gameplay,player}.ppm`.

The native slider fill and knob use the geometry read from retail `0x540d20`.
For the supplied frontend reference, the adapter reads the dark track tint from
retail `UI_Options.fng`; the pause package otherwise uses a white track. Arrows
appear on the selected slider. These visual choices are native presentation
seams; see `docs/pc-pause-options.md` for the original-code verification and
rendered slider checks.

The native pause/settings UI also accepts SDL gamepad and mouse input.
Start pauses, D-pad/left stick navigate, A accepts, B returns, and Y opens
Controls. Mouse clicks activate icons/rows, the wheel moves rows, and
right-click returns. Click or drag slider tracks to set values. Player → Controls
(or F2 while paused) opens the retail binding package. Tab, shoulders or device
arrows switch Keyboard/Gamepad for session rebinding. While driving, left stick
steers, right/left triggers accelerate/brake, A handbrakes, right-stick press
looks back, Y changes camera, Back resets, and Start pauses. Full support covers
the demo's existing actions; unrecovered gameplay actions remain unavailable. See [controls evidence and seams](pc-controls.md).
