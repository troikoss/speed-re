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
- View flags, view setup `0x4888a0`: `view +0x44` is 0x10 always, 0x08/0x04 by
  `0x89e7a4` (0 for the forward direction), 0x02 for eView `+4` types 1/2, 0x01 when
  `0x8707dc` is 3 or 4, 0x40 for 0x1800 (mirror) views, 0x20 for type 3. The per-instance
  test in `0x5d6570` skips an instance when `(instance +0x1a ^ 0x60) & (byte)view +0x44`;
  the demo uses 0x18 for the player view. Instances flagged 0x10 (race barriers, shop
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
- The view mode `0x8707dc` and the zero flag `0x836352` are image globals (both 0 in the image). They are the seams
  `UG2CarViewModeGlobal` and `UG2CarLodZeroFlag`; the gate `0x8654a4` is the block's gate.
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
- `0x5bb190`, `CarRenderDirectionMatrix`: call the D3DX transpose seam at
  `0x63d12c`, then write zero to float indices 3, 7, 11, 15, 12, 13 and 14 in that
  order and return the output. The existing native D3DX backend is reused.

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

The camera binding is supported by `0x5c26e0`, which stores the Camera pointer at
view+0x40, and base-mover constructor `0x44ab90`, which retrieves that same object.
`CameraState` already has the finalized matrix at +0, position at +0x40 and depth
axis at +0x50. `CarRenderFrame` now exposes that matrix with all offsets retained.
Native drive views copy the finalized prefix, including camera shake/debug state,
instead of rebuilding an axis from target−eye. The native orbit inspection camera
uses recovered LookAt to supply its own prefix. The focal-length formula remains
the existing GL viewport/projection seam.

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
`0x4901d0` and successful pool allocation `0x440e50` remain explicit boundaries.
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
pack. Those bodies, effect/technique setup and retail solid fixups remain open.
They are the next evidence path for the GL seam; prepared records alone cannot
close it. Native build, wheel-block/preparation regressions and SKYLINE smoke
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
work. The technique table producer has been located at `0x5d21f0`; its setup and
the original `0x5cf420` consumer are still unrecovered seams.

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
  the nearest-boundary fallback `0x5db770`, and view flags 0x02/0x01 remain unresolved.
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
