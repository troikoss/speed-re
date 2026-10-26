# Native code replacement inventory

Snapshot: 2026-10-19 (after the cylinder shape recovery). Reference build: UG2 PC v1.2
`speed2.exe`.

This is a planning document. It lists the native code in `src/Platform/Native`
that stands in for original UG2 code, and the recovered routines that should
replace it. Addresses come from the in-code seam comments and the subsystem
docs. They are leads, not verified boundaries.

Ratings are estimates:

- **Difficulty** (1–5): 1 = verified recovery exists and only needs wiring in;
  3 = a bounded chain of a few routines; 5 = a large subsystem with many
  unrecovered callees or rendering/device parity questions.
- **Importance** (1–5): how much the substitute affects original behaviour in
  the playable demo, or blocks other recovery. 5 = every frame or many other
  checkpoints depend on it; 1 = cosmetic or unreachable in the demo.

Platform backends that are expected to stay native (SDL window/input devices,
OpenGL drawing, file reads, the original-image mapper) are listed last. They are
not replacement targets, although their *decisions* (render state, input mapping)
still should come from recovered code.

## Recommended order

Open items, ordered by importance (ties: lower difficulty first). Finished items are
recorded in the section tables below.

| # | Item | Diff. | Imp. |
| --- | --- | --- | --- |
| 1 | Collision: ~~GJK `0x5b3390`, impulse `0x5af0e0`~~ (verified 2026-10-19; one GJK fix), ~~cylinders `0x5a23d0`/`0x5a2780`~~ (recovered 2026-10-19), ~~world objects `0x5b31f0`/`0x5b1390`/`0x5adb20`/`0x5add50`/`0x48bc50`~~ (recovered 2026-10-19; slot `0x8c` and the creation function remain seams) | 3–4 | 4 |
| 2 | Car wheel draw: wheel block `0x62700e`..`0x62df66` of `0x620ae0` recovered (`CarRenderWheels.cpp`, verified 2026-10-20 by `verify_pc_wheel_draw.py`); layout `0x61fe10` recovered; pixel size `0x48acb0` and the LOD selection `0x62107e`..`0x621230` recovered (`CarRenderLod.cpp`, verified by `verify_pc_car_draw_lod.py`). Slot setter `0x48d860` and rim-record lookup `0x60fef0` recovered 2026-10-26 (`CarRenderRecords.cpp`, verified by `verify_pc_car_render_records.py`); native table population remains open. Facing helper `0x60d700` recovered 2026-10-26 (`CarWheelFacing.cpp`, exact raw-x87 checks with MSVC and native i386 GCC). Preparation `0x48f660` and its producers `0x48e6c0`/`0x48a6a0` recovered 2026-10-26 (`CarRenderRecordPrepare.cpp`); native activation still needs the upstream lighting/source record. Open: seams `0x5c5930` (GL state), the `0x48f660` caller inputs/lighting producer `0x48ed60`, `0x620600` (secondary draw), the facing-normal producer, the car spin sub-object `+0x353`/`+0x354`, the slot `k` (`ri+0x2c10`), the `flagsCC` producer `0x60d1b0`, and the LOD B and C solids (see [wheel seams](#car-wheel-rendering-seams)) | 3 | 4 |
| 3 | Camera seams: ~~pushes~~, ~~cockpit tests `0x445fc0`/`0x5f14f0`~~ (recovered 2026-10-19), ~~shake `0x572a50`/`0x452ed0`, finalisation `0x447280`~~ (recovered 2026-10-19), ~~collision shake `0x572e50`~~ with the view shake states `0x572810`/`0x5728c0`/`0x5729e0` (recovered 2026-10-19), ~~chase frame branch of `0x445e00`~~ (recovered 2026-10-19 with the heading follower steering `0x445c20`); item 3 is done | 2–3 | 3 |
| 4 | CarTypeInfo/tuning chunk loaders | 3 | 3 |
| 5 | HUD feature construction and mask caller | 3 | 3 |
| 6 | Frontend classes (IconScroller, options list, dialog, Controls) | 3–4 | 3 |
| 7 | Car body draw: CarRenderInfo `0x638190`, render callers `0x631250`/`0x6316d0`, draw `0x620ae0` body path (see [body seams](#car-body-rendering-seams)) | 5 | 3 |

## 1. Game loop, timing and update composition

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| ~~`StepNativeVehicle` composes body advance, collisions and post-update~~ | `WorldUpdate.cpp` | World update `0x609ce0` | 3 | 5 | Done 2026-10-18 ([vehicle physics](pc-vehicle-physics.md#world-update-0x609ce0-recovered)); about 20 per-frame callees are native no-op seams. |
| ~~Car update reduced to the mover step~~ | `CarUpdate.cpp` | Car update `0x6012b0` | 4 | 5 | Done 2026-10-18 ([vehicle physics](pc-vehicle-physics.md#car-update-0x6012b0-recovered)); reset path callees `0x5fbed0`, `0x40f690`, `0x6009a0`, `0x409770`/`0x409960` and effect spawn `0x615cc0` trap. |
| ~~World clock advanced after the step~~ | `NativeVehicle.cpp` | Clock call `0x57a0e0` at `0x5814ea` | 2 | 3 | Now after the world update, as in game loop `0x5811c0`; the loop itself remains native. |
| Main loop frame time, fixed 1/60 s for captures, clamp 0.05 s | `DemoMain.cpp:505` | Game loop `0x5811c0` timing | 4 | 3 | The loop body is mostly the platform; the timing decisions are original. |
| Real-time clock `0x8651ac` in assumed 4 kHz units | `NativePause.cpp:900` | Original real-time clock producer | 2 | 2 | Only gates frontend input acceptance. |
| Respawn after 150 airborne frames | `DemoMain.cpp:597` | Original reset/respawn flow (`0x5fc2a0` and caller) | 3 | 2 | Pure scaffold; exists because collision coverage is incomplete. |
| Free-drive entry; race phase fixed at 0; race-status, drivetrain context and random objects are scaffolds | `NativeVehicle.cpp:107`, `:203` | Race setup `0x5fc730`, `0x60a6d4`/`0x5ff537` | 4 | 3 | Race-phase-dependent branches are untested. |

## 2. Vehicle and session construction

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| ~~`CreateNativeVehicle`: allocation, vtable shims, defaults, yaw-matrix placement, collider attach~~ | `CarConstruction.cpp` | Car constructor `0x604260`, physics binding `0x601110`, car creation `0x606480`, participant setup `0x6065b0` | 4 | 5 | Done 2026-10-18 ([vehicle physics](pc-vehicle-physics.md#car-construction-0x604260-and-participant-setup-0x6065b0-recovered)); the native session fills one participant record. |
| Participant kind switch, mover factory and ground placement | `NativeVehicle.cpp` construction seams | `0x5eba30` (human `0x4266d0`), `0x5f46a0`, `0x5f5010`/`0x5f49f0` | 3–4 | 4 | Native controls and participant scaffold; null mover or PhysicsMover; a yaw matrix with no ground fit. |
| Participant records and race-start pools | `CreateNativeVehicle` | Frontend/race setup filling `0x89e810`; pool creation `0x598f10` | 4 | 3 | One human record with an explicit placement; only `NullMoverSlotPool` is created. |
| `New...` allocation factories (`NewMoverBody`, `NewEngine`, …), zero-filled blocks | `NativeVehicle.cpp:100–160` | Slot pools `0x440e50` and the original factories | 3 | 2 | Zero-filling hides uninitialised-read differences. |
| ~~Tuning bound to car by native code~~ | `CarConstruction.cpp` | Setup copy `0x413440` (tuning bind `0x59b060`) | — | — | Done 2026-10-18: the record copy binds the tuning; the composer now runs through refresh `0x639ce0`. |
| Verified routines not yet integrated: mode variants `0x5a7530`/`0x5a7750`/`0x5a77c0`/`0x5a7830`, offset `0x59aee0`, field switch `0x59b200`, normalisation `0x59b3c0`, reader `0x5a7da0`, range/setter leaves | not called | Already linked (2026-10-18 check) | — | 3 | Mode variants run from composer `0x5b4dc0`; the tuning routines run under `0x5b4780`, which the composer calls only when `CountActive() > 0`. No tuning entries exist natively, so they stay unreached until the tuning-table loader and binder are recovered. |
| GLOBALB.BUN CarTypeInfo chunk kept resident and installed at `0x8a1ccc` | `NativeVehicle.cpp:91` | CarTypeInfo chunk loader (unrecovered) | 3 | 3 | |
| Tuning-table loader absent, so full tuning never runs | — | Tuning-table loader | 3 | 3 | |
| Collision hull read (`0x80034020`) with stock `_CV` key | `NativeVehicle.cpp:133`, `NativeCollision.cpp:36` | `0x61c320`, `0x610310`, the car geometry loader | 2 | 2 | Hull loader `0x5a1750` is recovered. |
| Single human player record at `0x8900ac`/`0x8900b4` | `NativePlayer.h` | Player/session construction | 3 | 3 | Blocks multi-player and AI. |
| Stand-in kind-1 chase camera mover on the player | `NativeVehicle.cpp:354` | Player::SetCameraState `0x5f1fd0` mover construction | 2 | 2 | |

## 3. Input and controls

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| SDL capture writes action fields | `NativeVehicle.cpp:311`, `DemoMain.cpp` | Device capture `0x571a60` | 4 | 3 | The DirectInput backend stays native; the capture decisions should not. |
| Device/profile setup, slot 0 assumed | `NativeVehicle.cpp:247` | Profile/controller setup | 3 | 2 | |
| Pre-race brake release | entry seam | Original race start | 2 | 2 | |
| Key-name labels and session-only bindings | `NativeControls.cpp:155–157` | `0x5c2100`, `0x4acf40`, profile persistence | 3 | 2 | The Windows key-name backend stays native. |
| Controls screen list ownership and handler | `NativeControls.cpp:3` | Screen handler `0x4f9340`, list `0x4dba50` | 3 | 2 | |
| Force-feedback manager methods trap | `NativeVehicle.cpp:188–206` | DirectInput effects `0x5bfd70`… | 4 | 1 | Unreachable natively (null manager). |

## 4. Collision

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| ~~GJK and impulse resolution not yet verified~~ | — | `0x5b3390`, `0x5af0e0` | 3 | 4 | Verified 2026-10-19 ([native demo](NATIVE_DEMO.md)): `verify_pc_collision_gjk.py` 8,000 cases and `verify_pc_collision_impulse.py` 3,000 contacts, 0 failures; GJK now compares the stored float distance (`0x5b3c36`). ContactFrame `0x594000`, CreateEvent `0x5a9b10` and accumulate `0x5a9620` are hooked, not covered. The 42 simplex ties in `0x595040` are degenerate inputs, believed codegen-only (not checked at instruction level). |
| ~~World-object promotion/break chain traps~~ | `NativeCollision.cpp` (was `0x5b1390`, `0x5adb20`, `0x5add50`, `0x48bc50`) | `0x5b31f0`, `0x5b1390`, `0x5ae000`, `0x5adb20`, `0x5add50`, `0x5adf90`, `0x48bc50`, `0x487360`, `0x40f010`, `0x6147e0`, `0x5a8b70`, `0x592a10`, `0x611000` | 4 | 4 | Recovered 2026-10-19 ([native demo](NATIVE_DEMO.md)): `verify_pc_collision_world_object.py` 41,175 compared runs over the whole chain as machine code, 0 failures, 694/696 instructions (2 dead), 36/36 mutants rejected. Remaining seams: world-object kind (vtable slot `0x8c`, still a trap), the creation function behind `0x86af90`, the body's slots 8/0xc, and the hooked collider/mover/refresh callees, none covered by this verifier. |
| ~~Cylinder shapes trap~~ | `CollisionShape.cpp` | `0x5a23d0`, `0x5a2780` | 3 | 3 | Recovered 2026-10-19 ([native demo](NATIVE_DEMO.md)): `verify_pc_collision_cylinder.py` has 0 failures in 159,856 cases; GJK now generates cylinder pairs. The cylinder constructor (and the `+0x14` count it sets) is not located. |
| ~~Contact notification to the participant traps~~ | `CarUpdate.cpp` | `0x5f4960` | 3 | 3 | Recovered; the native participant scaffold's slot `0x38` is a no-op and the native mover factory seam traps on a switch away from the PhysicsMover. |
| Collision effects dropped (sparks, debris, sounds, replay, FF) | `NativeCollision.cpp:56–93` | `0x5a9620` tail, `0x59fb70`, `0x5dc4c0`, `0x4814e0`/`0x4792d0`, `0x56fb20` | 4 | 2 | Mostly presentation; sound needs an audio backend. |
| Point velocity, energy and car-body slot trap | `NativeCollision.cpp:111–121` | `0x59bec0`, `0x59bfd0`, `0x5a08b0` | 2 | 3 | Small leaves; likely reached once more contact paths run. |
| Serialisation traps | `NativeCollision.cpp:125–138` | `0x593cc0`, `0x593120`, `0x5a9e80`, `0x593d10` | 2 | 1 | Replay/save only. |
| Draft tracker and road navigator trap | `NativeVehicle.cpp:~227`, `NativeWorld.cpp:59` | `0x5fa540`, `0x5da370` | 3 | 2 | The navigator needs the road network. |
| Skid marks recorded for the demo renderer | `NativeVehicle.cpp:~207` | `0x5e6e70`, `0x5d3230` | 3 | 1 | |
| All collision packs loaded at startup | `NativeWorld.cpp:1` | Original streaming | 5 | 2 | Correctness is fine; memory and timing are not original. |

## 5. Camera

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| ~~World collision pushes for chase types~~ | `DriveCamera.cpp` update tail | `0x444530`, `0x44b040` | 3 | 3 | Done 2026-10-18 ([camera](pc-camera.md)); vtable slots and car list remain scaffold. |
| ~~Camera shake~~ | `CameraShake.cpp`, `Camera.cpp` | `0x452ed0`, `0x572a50` (with `0x446d00`, `0x5766e0`, `0x43d100`, `0x5ecdb0`, `0x572860`, `0x5722a0`) | 2 | 2 | Recovered 2026-10-19 ([camera](pc-camera.md#camera-shake-and-finalisation-2026-10-19)): `verify_pc_camera_shake.py` 30,720 comparisons (MSVC), 0 failures, 70 mutations rejected; D3DX transpose is a native seam. |
| ~~Camera finalisation~~ | `Camera.cpp` | `0x447280`, `0x441340`, `0x5baf80` | 2 | 2 | Recovered 2026-10-19 (same verifier); the Camera object itself is a zeroed native object. |
| ~~Collision shake~~ | `CameraCollisionShake.cpp`, `NativeVehicle.cpp` (trap `UpdateRace572E50` removed) | `0x572e50` (with `0x572810`, `0x5728c0`, `0x5729e0`, `0x5ea7c0`, `0x502500`; `0x5722a0` and `0x43cf90` run as original) | 2 | 2 | Recovered 2026-10-19 ([camera](pc-camera.md#collision-shake-2026-10-19)): `verify_pc_collision_shake.py` 6,144 comparisons (MSVC), 0 failures, 42 mutations rejected. Native seams: the car's race entry is the player record, the world gate `+0x108` and the shake reset, the tick before the world clock. |
| ~~Cockpit blocked tests~~ | `CameraCockpit.cpp` | `0x445fc0`, `0x5f14f0`, `0x43d7a0`, `0x5ed290`/`0x5ed300`, slot 0x24 `0x445a90`, Update prefix `0x453570..0x4536f4` | 2 | 2 | Recovered 2026-10-19 ([camera](pc-camera.md)): `verify_pc_camera_cockpit.py` 24,576 comparisons (clang-cl), 0 failures, 37 mutations rejected; the eye-duration divide at `0x4537c3` and the blocked-adjusted type in `previousType4C` are now original. Player car getter `0x5eac70` is recovered but unreached (native player pointer 0). |
| ~~Chase car frame from the heading follower~~ | `CameraChase.cpp` `GetCarFrame`, `SteerHeading` | `0x445e00` slot-0x24-true branch (`0x43c780`, `0x43cf10`, `0x43d1e0`), `0x445c20`, `0x445ac0`, `0x574cd0` | 2 | 3 | Done 2026-10-19 ([camera](pc-camera.md)); `0x445c20` reads the tick global `0x86518c` itself. |
| Acceleration vector at car `+0x80` | `WorldUpdate.cpp` | Producer inside `0x5fbbe0`/`0x604740` | 2 | 3 | Produced by their recovered kinematics prefixes, now called from world update `0x609ce0`; the rest of both routines is not recovered. |
| Rear-view mirror view not created | `NativeHud.cpp:504` | `0x5f1e10` | 3 | 2 | |

## 6. World, assets and rendering

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Solid/texture pack parsing | `NativeAssets.cpp` | Loaders `0x493bb0`, `0x494ea0`, `0x5cc280` | 4 | 3 | |
| COMP decoder absent | — | `0x574530` | 2 | 2 | |
| Scenery section selection (nearest-boundary fallback missing) | `NativeScene.cpp:368` | `0x5e96b0` complete, `0x5db770` | 3 | 3 | |
| View flags `0x02`/`0x01` left clear | `NativeScene.cpp:307` | `0x4888a0`, eView `+4`, `0x8707dc` | 3 | 2 | |
| Pixel-size LOD missing | — | `0x5d64d0` | 3 | 3 | |
| Render states derived from texture alpha fields | `NativeScene.cpp:225` | D3D render path (`0x5c7ea0`, `0x5c5020`, `0x5bcc30`) | 5 | 3 | GL stays as the backend; state selection should be original. |
| No depth sorting of blended draws | `NativeScene.cpp` | Original draw ordering | 4 | 2 | |
| Fixed paint colour for SKIN1; no vinyls, damage or light switching | `NativeScene.cpp:114–140` | Skin builder, `0x638190`, `0x631e20` | 4 | 2 | Details and the other body seams: [car body rendering seams](#car-body-rendering-seams). |
| Scaffold lighting and horizon colour | `DemoMain.cpp:266`, `:673` | Time-of-day/fog colour selection | 4 | 2 | |
| ~~Wheel render frames composed natively from `DrivetrainWheel` `matrix30`×`matrix70`~~ (done); wheel block recovered 2026-10-20 | `DemoMain.cpp` `DrawCarModel`, `NativeCarModel::Draw` | `0x5f6820`, `0x5ec7f0` (recovered and verified), `0x61fe10` and `0x62700e`..`0x62df66` (recovered and verified) | — | — | Remaining seams: [car wheel rendering seams](#car-wheel-rendering-seams). Fallback `DrawWheel` unchanged. |

### Car body rendering seams

Snapshot 2026-10-19. The car body is drawn natively from fixed part lists, with scaffold paint and
lighting. The original chain, from the call graph: the frame render driver `0x5caad0` (caller
`0x5d2850`; it also calls the scenery solid render `0x5cf420`) calls three car paths,
`0x630de0`, `0x631250` and `0x6316d0`. All three call the 66 KB car draw `0x620ae0` (Ghidra's
decompiler times out on it); `0x631250`/`0x6316d0` also call the wheel getters. The render info
behind them is built by the CarRenderInfo constructor `0x638190` (6 KB), created by `0x63a5a0`
(malloc; callers `0x5f4650`, `0x601110`) and `0x63a650` (callers `0x4a79d0`, `0x4d82d0`,
`0x4ea900`, `0x63b850`; likely frontend/showcase cars, unproven). The wheel block has its own
table below ([car wheel rendering seams](#car-wheel-rendering-seams)); the items there are not
repeated here. Addresses are call-graph leads, not verified boundaries.

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Part list: `<CAR>_BASE_A` plus 19 fixed KIT00 parts (BODY, BRAKELIGHT, DOOR_LEFT/RIGHT, ENGINE, EXHAUST, FRONT_BUMPER, HEADLIGHT_LEFT/RIGHT, HOOD, HOOD_UNDER, side mirrors, REAR_BUMPER, ROOF, SKIRT, SPOILER, TRUNK, TRUNK_UNDER), always LOD A; door panel/sill and trunk audio variants left out | `NativeScene.cpp` `NativeCarModel::Load` (reads `CARS/<car>/GEOMETRY.BIN`; texture packs `CARS/<car>/TEXTURES.BIN`, `CARS/TEXTURES.BIN`, `CARS/BRAKES/TEXTURES.BIN`, `GLOBAL/GLOBALB.BUN`) | CarRenderInfo constructor `0x638190` (part/kit selection from the car preset) | 4 | 3 | LOD selection is not recovered. Setup copy `0x413440` and `0x634630` are also called from `0x63a650`. `0x638190` also calls `0x634800` (8 KB, its only caller; role unknown, calls texture/solid helpers). |
| Texture replacement keys hard-coded, lights off, no damage: `WINDOW_*` to GLOBALB `WINDOW`, `DRIVER` to `DRIVER_MALE`, brakelight/headlight to `_OFF` textures | `NativeScene.cpp` `NativeCarModel::Load` | Used-texture setup `0x631e20` (also reached via `0x636840` and `0x6335d0` from `0x637840`) and keys built by `0x638190` | 3 | 2 | The window texture the game binds (damage/variant) is not recovered. The per-frame light on/off and damage switching are not located. |
| Paint: SKIN1 (`<CAR>_SKIN1`) and the unnamed body panel `0x3c84d757` / skirt `0xa7366ae6` keys drawn untextured in fixed colour (0.62, 0.64, 0.68); other untextured parts dark grey (0.12, 0.12, 0.13) | `NativeScene.cpp` `NativeCarModel::Build` | Skin layer built at run time from the preset paint and vinyls (builder not located) | 4 | 2 | Strings `VINYL_PAINT` and `DECAL_*` exist; their builder is not located. No vinyls or decals are drawn. |
| Render state and display lists: one GL list per solid, opaque and translucent lists split purely by the texture alpha flag; mesh-part flags (record `+0x38`, parsed into `part.flags`) ignored | `NativeScene.cpp` `NativeCarModel::Build` | Per-part state via solid draw `0x5c5930` and the D3D path `0x5c7ea0`/`0x5c5020` | 5 | 3 | Shares the solid-draw row of the wheel table. The solid chunk parser (`NativeAssets.cpp` `LoadSolid`: chunks `0x134011`/`0x134012`/`0x134900`/`0x134b01`/`0x134b02`/`0x134b03`, vertex strides 24 or 36 only, car vertex colours unused, normals used) stands in for the loaders `0x493bb0`/`0x494ea0` listed in the main table. |
| Lighting: one directional GL light (0.35, 0.25, 0.9), ambient 0.22/0.22/0.27, diffuse 0.5/0.48/0.45, `GL_COLOR_MATERIAL`, alpha test > 0.02; translucent pass with `SRC_ALPHA` blend, no depth write, no sorting. No envmap or gloss | `DemoMain.cpp` `DrawCarModel`, `NativeScene.cpp` | Car lighting/shader constants, envmap and gloss | 4 | 2 | Leads: string `EnvMapTexture` (`0x7a4428`) is referenced by `0x5d0f80` (effect parameter setup); `FRONTEND\ENVMAPS\%s.BIN` by `0x523d00`; a `CarSkin GlossMap` string (`0x7a4604`) has only a data-table reference. |
| Car shadow not drawn | — | Shadow consumer of `g_CarShadowEnable` (`0x7a386c`) | 3 | 2 | The string is referenced only by the option read/write pair `0x5bea20`/`0x5beea0`; the consumer is not located. |
| Damage not drawn | — | Damage consumer of `g_CarDamageEnable` (`0x7a382c`) | 4 | 1 | Same situation as the shadow option; the consumer is not located. |
| Unidentified car draw paths: `0x630de0` (a third `0x620ae0` caller), `0x6162e0` and `0x5f3ae0` (both called only from `0x6316d0`) | none natively | Identify roles | 3 | 2 | `0x630de0`: role unknown. `0x6162e0` (18 KB): draws solids via `0x5c5930`, uses `0x60d700` and `0x48f660` like `0x620ae0`. `0x5f3ae0`: draws a solid via `0x5c5930`, calls `0x5eac90`. Roles unproven. `0x60f880` draws solids via `0x5c5930`; only a lead, an unidentified solid draw in the frame driver, not shown to be car-related. |
| Body frame: `MoverBody` `matrixD0` with the translation replaced by `DrivetrainAuxiliary` `origin10` | `DemoMain.cpp` `DrawCarModel` | Body matrix the callers pass to `0x620ae0` (car `+0x5a0` or `+0x50`) | 2 | 3 | Car origin comes from `0x592400`. See the wheel table for the parent frame question. |
| Car skipped when `cameraView.attachedToCar` (bumper/driver cameras, camera `+0x280`) | `DemoMain.cpp` main loop | Whatever the original draw skips for those views | 2 | 1 | A native decision, unverified. The rear-view mirror render of the car is not drawn either (mirror view: camera section, `0x5f1e10`). |
| Only the player car is drawn; no AI/traffic cars | `DemoMain.cpp` main loop | Per-car render info for every participant | 3 | 2 | Ties to the single player record in section 2. Driver variation, headlight beams and light flares are not drawn. |
| Fallback box `DrawBody` when no model loads | `DemoMain.cpp` | None (pure scaffold) | — | 1 | Keep. |

### Car wheel rendering seams

Snapshot 2026-10-20. The wheel block of the car draw is recovered: `CarRenderWheelLayout` (`0x61fe10`) and
`CarWheelDraw` (`0x62700e`..`0x62df66`, the setup of the per-wheel sets, the spin pre-rotation, the solid links, the
eight draws and the LOD tail) in `src/Speed/Indep/Src/World/CarRenderWheels.cpp`, verified by
`tools/verify_pc_wheel_draw.py` (2048 block cases, 512 layout cases, mutation checks). The getters `0x5f6820` and
`0x5ec7f0` are recovered and verified by `verify_pc_wheel_render.py`. The callers `0x631250`/`0x6316d0` pass the
tyre, brake and third frames and the body matrix to `0x620ae0`; the demo calls the block the same way
(`DrawCarModel` -> `NativeCarModel::Draw` -> `NativeRunCarWheelBlock`). The block's chain per wheel is in
[the demo document](NATIVE_DEMO.md#car-wheel-block-recovered). The pixel size `0x48acb0` (`0x620c6b`..`0x620cc0`) and the
body LOD and wheel-flag selection (`0x62107e`..`0x621230`) are recovered in `CarRenderLod.cpp` and verified by
`tools/verify_pc_car_draw_lod.py` (512 producer and 2048 selection cases, 14 of 14 mutation checks rejected); see
[pixel size and LOD selection](NATIVE_DEMO.md#pixel-size-and-lod-selection-recovered).

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Solid draw `0x5c5930` (eight calls in the block) | `NativeCarWheels.cpp` `CarRenderView::DrawSolid` | Draw of the solid's native list with the world matrix (opaque or translucent pass) | 5 | 3 | Flags, extra record and the zero argument are ignored. `glFrontFace` is not changed for the R180 wheels (GL backend seam). |
| Render-record preparation `0x48f660` (wrapper and both producers recovered 2026-10-26) | `CarRenderRecordPrepare.cpp`; native `UG2WheelVisible` still stubs activation | Supply the caller's source/position inputs: clear `0x48ab10` is recovered; lighting builder `0x48ed60` remains open | 3 | 2 | Native GCC full-chain exact evidence: 32,768 wrapper runs, all 11 mutants rejected; MSVC caller checks use original callees to keep the known coefficient precision gap separate. The native block still passes a 64-byte placeholder for a 0x1c0-byte source record, and DrawSolid ignores the extra record. Lighting foundations `0x48a310`/`0x4895e0`/`0x489af0`/`0x489ed0` and clear `0x48ab10` recovered 2026-10-26; the builder, light selection, correct caller buffers and draw consumption remain open. See [preparation evidence](NATIVE_DEMO.md#render-record-producer-chain-2026-10-26). |
| Facing helper `0x60d700` (recovered 2026-10-26) and secondary draw `0x620600` (open) | `CarWheelFacing.cpp`; `NativeCarWheels.cpp` secondary-draw seam | Facing-normal producer and secondary draw | 3 | 2 | Facing helper now preserves the original one-valued disabled path, matrix copy, dot-product order and two float spills. Native globals use image defaults (enabled, normal 0/1/0); their producer is open. Gate `0x8654a4` stays 0, so neither path runs in the current demo. See [facing evidence](NATIVE_DEMO.md#wheel-facing-helper-2026-10-26). |
| Solid slot table `0x48d860` (`SetSlot`, recovered 2026-10-26) | `CarRenderRecords.cpp`; native solids have a null table at `+0xc` | Populate the solid slot table with keys `0x22719fa9`, `0xfd102a92`, `0x12049251` | 3 | 2 | Setter verified against the binary: signed byte count, ignore zero values, update every matching key. Native table construction and consumers remain open; link values `ri+0x390`/`0x3ac` are zero. |
| Rim part records `0x60fef0` (`SPINNEROFFSET`, lookup recovered 2026-10-26) | `CarRenderRecords.cpp`; native car has no rim parts | Populate the global part table and the car's rim parts | 3 | 2 | Lookup verified against the binary, including wildcard hash and previous-record continuation. Native part/table construction remains absent, so `ri+0x438` stays zero. See [record lookup evidence](NATIVE_DEMO.md#car-render-record-lookups-2026-10-26). |
| Spin term `0x59a420` on car `+0x10` | `CarRenderWheels.cpp` `SpinTerm` (recovered); car data not native | Car sub-object bytes `+0x353`/`+0x354` from the native car state | 3 | 2 | The block applies the spin only to wheels 0 and 1 (tyre and brake frames). The demo sets both bytes to zero, so the block pre-rotates no front wheel. The tyre roll is not in the block: `GetWheelRenderMatrices` (`0x5f6820`) copies the state frames built by `0x5921b0` (`RotateMatrixY` at `0x5922fb`), and `GetWheelSpinMatrices` (`0x5ec7f0`) builds the third set with `RotateMatrixY` (at `0x5eca3a`). |
| Native view for the pixel size: focal `view+0xc` and frame `view+0x40` (origin `+0x40`, depth axis `+0x50`) | `NativeCarWheels.cpp` `NativeSetCarView`, called from `DemoMain.cpp` before the car draw | The car draw's view object (camera origin, view direction and focal length of the projection) | 3 | 3 | The focal is `0.5 * height / tangentY` in pixels; the original's value is set by camera code that is not recovered. Without a view the pixel is 1000 (LOD 0). |
| View mode global `0x5b98a0` (`[0x8707dc]`, compared with 3) and LOD zero flag `0x499db0` (`[0x836352]`) | `NativeCarWheels.cpp` `UG2CarViewModeGlobal`, `UG2CarLodZeroFlag` | Camera mode and the LOD reset flag of the native game state | 2 | 2 | Both return 0 in the demo (the image values). The pixel is not scaled and the LOD is not reset. |
| Allocations `0x401e80` (matrices) and `0x401e30` (records) | `NativeCarWheels.cpp` | Scratch storage per block | 1 | 1 | Two allocations of each kind in the block's order. |
| Caller inputs of the block: body LOD `frame+0x6c`, view clamp `frame+0xf4`, flag `frame+0x1ac`, arg8 `0x401`, slot `k` (`[esp+0xd8]`), flags `[esp+0xcc]`, body pool block `EBX` | `NativeCarWheels.cpp` `NativeRunCarWheelBlock` | `CarLodPixelSize`/`CarLodSelect` (recovered) for the first four; the callers `0x631250`/`0x6316d0` for the rest | 4 | 3 | The LOD, clamp, flag and arg8 come from the recovered selection (`CarRenderLod.cpp`). `k` is 0 and `flagsCC` is 0 in the demo: `k` is `[obj+0x2c10]` with `obj` = `frame+0x60` (an object pointer on that path; `ri+0x2c10` is set by `0x634800`), and `flagsCC` is produced by `0x60d1b0` at `0x621b9e`. Neither is recovered. `EBX` is the body frame. |
| Solid choice for LOD B and C: wheel and brake solids of LOD 1 and 2 | `NativeScene.cpp` `NativeCarModel::Load` | The part table entries `ri+0x1b48` (part `0x1d`) and `ri+0x1b68` (part `0x1e`), filled by `0x634800` | 3 | 3 | Open. The demo loads only the LOD A solids (`FRONT_WHEEL_A`, `FRONT_BRAKE_A`, `REAR_BRAKE_A`) for every LOD index. The letter-to-LOD mapping of `0x634800` is not evidenced (the hashes of the candidate names do not match its keys). |
| Widths `ri+0x430`/`ri+0x434` (`MARKER_BRAKE`) | `NativeCarWheels.cpp` (zero) | Constructor `0x638190` values from the marker `+0x44` of the brake solids | 2 | 1 | Only feed `set2`, which no draw reads; no visible effect. |
| Solid choice: `<CAR>_KIT00_FRONT_WHEEL_A` for all four wheels; `FRONT_BRAKE_A`/`REAR_BRAKE_A` by axle, always LOD A | `NativeScene.cpp` `NativeCarModel::Load` | CarRenderInfo `0x638190` slots `ri+0x1b48`, `ri+0x1b58` (brake loop), `ri+0x1b68`, `ri+0x1b78`, `ri+0x1b6c` (LOD substitute) | 3 | 3 | Rear wheel mesh and kit choices are not original. The block's LOD index selects among these same LOD A solids (LOD B and C are open, see the row above). |
| Wheel/brake mirroring (`glScalef(1,-1,1)`), centring translation and `glFrontFace` toggling | removed | Recovered sets and R180 (wheels 0 and 3) in the block | — | — | Removed in favour of the recovered frames; the screenshots show the tyres in the arches. |
| Procedural fallback `DrawWheel` composes `DrivetrainWheel` `matrix30`×`matrix70` when no car model loads | `DemoMain.cpp` | None (pure scaffold) | — | 1 | Kept as the fallback. |
| Mod bridge exports `matrix30`/`matrix70` | `NativeModBridge.cpp` | — | — | — | Outside recovery scope; not a render seam. |
## 7. FEng engine (shared by HUD and menus)

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| ~~`AdvanceScripts` frame seam~~ | — | Frame manager `0x558c00` (recovered, wired in) | — | — | Remaining: fraction-carrying ms clock (`NativeFengMilliseconds`), draw-angle sync. |
| `Pump()`: input handled outside the frame, drained by zero-length `0x558c00` frames | `NativePause.cpp` `Pump` | Pad/mouse dispatch `0x54d4b0`/`0x54e020` with queue input +1 and pads +0xd4 | 4 | 3 | Replaces the native `Input()` bridge too. |
| HUD on its own queue (events delivered to an ignoring observer) | `NativeHud.cpp` `Update` | One shared queue with the frontend packages | 2 | 2 | HUD now runs through `0x558c00`. |
| Package loading/removal trap; native package chain and order | `NativePause.cpp:163`, `:513–524` | `0x555800`/`0x555720` load driver, `0x5362e0`, `0x51ab20`/`0x51ac10`/`0x51adb0`/`0x51b070` | 4 | 4 | Load `0x555860` and level insert `0x518280` recovered 2026-10-19 ([pause menu](pc-pause-menu.md#feng-package-and-object-readers-2026-10-18)); the trap moved to `0x555800`. The driver still needs `0x52b4d0`, `0x5049a0`, `0x52b550`, `0x5182e0`, `0x518380`, `0x551580`, `0x5184e0`, `0x52b370`. |
| Object/record/archive loading; parsed file order kept | `NativeFng.cpp:618`, `NativePause.cpp:197` | Wire in recovered `0x5554a0`/`0x555140` | 4 | 4 | Package reader `0x5554a0`, object reader `0x555140`, clones `0x554300`/`0x5548a0`, appends `0x517e90`/`0x495610` and visitor `0x517ee0` recovered 2026-10-19 (26,987 comparisons, 41 mutations rejected). `FengPackage::Load` still parses natively: its `FengObject` wrapper is larger than the original objects and object vtable calls cannot run natively. |
| Position, size and UV setters | `NativeFng.cpp` `SetPosition`/`GetSize`/`SetState` | Original FEng setters | 2 | 2 | |
| Bounded input for the script reader | `NativeFng.cpp:189` | None (keeps malformed data safe) | — | — | A permanent safety seam. |
| No type registry producer (`queue3C = 0`) | `NativeFng.cpp:213` | Registry construction feeding context `+0x3c` | 2 | 1 | Retail scripts contain no `Ti` tags. |
| Render angle from quaternion | `NativeFng.cpp:222` | Part of the FEng render path | 2 | 1 | |
| FEng rendering: render path translated to GL | `NativeFng.cpp:~900–960` | `0x536a50 → 0x51a5d0 → 0x5c7ea0 → 0x5c5020` | 5 | 3 | |
| NOS destination-alpha/stencil composition missing | `NativeFng.cpp:906` | `0x5c5020` stencil branches | 4 | 2 | |
| Minimap blend mode 5 approximated with a stencil | `NativeFng.cpp:941` | Original D3D blend mode 5 | 4 | 2 | |
| Font list, chunk loading and glyph drawing; layout port behavioural only | `NativeFng.cpp:1231–1402` | Verify `0x51bdc0`, `0x51c110`, `0x51bfc0` and helpers | 3 | 2 | |
| Language: English only; text widened from 8-bit | `NativeFng.cpp:783`, `NativeHud.cpp:285` | Localisation loader | 3 | 1 | |
| Type-6 navigation, slot filter, grid refill trap | `NativePause.cpp:547–553`, `:730` | `0x52bd70`, `0x50d1d0`, owner `+0x4c` | 2 | 1 | Not reached by the current screens. |

## 8. Frontend screens

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Screen show/close lifecycle, package reload per show | `NativePause.cpp:80–120`, `:330` | Package push/pop via the FEng queue | 3 | 3 | Follows from section 7. |
| IconScroller construction, ownership and remaining slots | `NativePause.cpp:374–445` | `0x543bf0`, `0x538b80`, `0x51f870` | 3 | 3 | |
| IconScrollerFNGObject constructors and handler | `NativePause.cpp:583–588` | `0x551de0`, `0x552330`, `0x543d40`, slots `0x40d080`/`0x570a70` | 3 | 3 | |
| In-game dialog | `NativePause.cpp:126`, `:695` | `0x558790` | 3 | 2 | |
| Options list widgets and mouse input | `NativePauseOptions.cpp` | Options scroller `0x556020` remainder, widget click handlers | 3 | 2 | |
| Display settings: session-only; no retail display reset | `NativePauseOptions.cpp:371–372` | Registry persistence, `0x5cec90` | 3 | 1 | |
| Theme: reference Init track substituted | `NativePauseOptions.cpp:173` | Original options package selection | 2 | 1 | |
| `JOY_EVENT_PAUSE` opens the menu directly | `DemoMain.cpp:478` | Game flow that pushes `UI_Pause.fng` | 3 | 2 | |
| Career and network values trap | `NativePause.cpp:765–769` | `0x513b30`, `0x5139c0`, `0x5f0120` | 3 | 1 | |
| SDL keyboard/mouse/pad mapped to FEng pad messages | `DemoMain.cpp:462`, `NativeUiInput.h` | Frontend input `0x54d4b0` upstream mapping | 2 | 2 | The device layer stays native. |

## 9. HUD features

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Feature allocation with fixed node storage | `NativeHud.cpp:436–446` | Constructor `0x4f73e0`, allocator `0x440ec0`, minimap pools `0x4ec950` | 3 | 3 | |
| Mask caller missing (controller called without a player) | `NativeHud.cpp:549` | `0x600590`, `0x5f1e10` | 3 | 3 | |
| Native one-player session; parent containers revealed natively | `NativeHud.cpp:60`, `:330` | Original player slots and recursive show | 2 | 2 | |
| Authoring sample strings cleared | `NativeHud.cpp:390` | Live career producers | 4 | 2 | |
| GPS manager, route position and arrival receiver trap | `NativeHud.cpp:76–78` | `0x501100`, `0x5010c0`, `0x458520`, `0x838590` | 4 | 2 | |
| Notification preparation, presentations and sound trap | `NativeHud.cpp:79–84` | `0x4c8ac0`, `0x4c8b80`, `0x4c8dd0`, `0x4abaf0`, `0x4aba30`, `0x51de30` | 3 | 2 | The scheduler is already verified. |
| 15 unrecovered feature updates trap | `NativeHud.cpp:100–114` | `0x4c7090`, `0x4ebb10`, `0x4da590`, … `0x4db8c0` | 4 | 2 | Race-only and career features. |
| Region trigger producer | — | `0x5eb040`, `0x5d9bf0`, `0x5d3de0` | 3 | 2 | |
| Laps, `tach_fill` colouring, replay RPM, HUDINDEX | `docs/NATIVE_DEMO.md` | Original producers | 2 | 1 | |
| TrackInfo chunk storage and language publication | `NativeHud.cpp:271`, `:285` | Asset loaders | 2 | 1 | |

## 10. Audio, replay and miscellaneous

Audio setup, controllers, parameter callbacks, car dispatch and native mixer binding updated 2026-10-20; the remaining game audio seams are still open.

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Audio enable flag forced to 0 (`0x7faaa8`) | `NativeVehicle.cpp:534` | Option reads in `0x580130` (one `0x57e2a0` read per flag; `0x7faaa8` is one of them) | 2 | 4 | Keeps every flag-gated audio path in recovered code dead: frontend sounds `0x458520`, HUD sound `0x51de30`, collision sound set `0x483e20`/`0x483e40`, car sound creation `0x4690e0`. The engine sound scaffold below does not read the flag. |
| Collision sound set traps | `NativeCollision.cpp:77–78` | `0x481cd0` (add), `0x481e50` (remove) | 4 | 2 | Unreachable while `0x7faaa8 = 0`. The collision path uses the CSIS/stitch banks (`MB_Stich_Collision.abk`). |
| Car sound state managers; ~~creation dispatch trap~~ | `AudioSystem.cpp`; native audio initialization | ~~`0x4690e0` dispatch~~ (recovered and linked 2026-10-20); manager pool/sound-object construction at EAXSound `+0xac`/`+0xb0`/`+0xb4` remains | 5 | 3 | Dispatcher now follows the original enable flag, context kind/`+0x900` override, null-manager gates and slot `+0x10` call. Audio constructor `0x4585a0`, manager kind/limit initializer `0x463ad0`, lookup/bind dispatch `0x463cc0` and queue `0x463f10` are recovered; native pause/options initialization calls the constructor. State-manager allocation/initialization `0x4853c0`, pool/sound-object creation and the virtual context/bind methods remain open, so this does not enable car audio. See [construction evidence](pc-ginsu.md#audio-object-and-car-dispatch-recovery-2026-10-20). |
| snd library: SDL2 packet player | `NativeAudio.cpp`, `NativeAudio.h` | snd library `0x721f99`/`0x721fa7` (critical section), `0x723124` overhead, `0x723138` create, `0x7231d4` start, `0x723515` submit, `0x72387b` stop, `0x72366a` destroy, `0x72423e`/`0x721a6b` defaults | 5 | 2 | EA middleware, statically linked; it may stay native like D3DX. Modelled: mono packet queues per player (8 players, 32 packets), linear resampling to 44.1 kHz stereo, release callbacks on the audio thread under the recursive SDL device lock, and headroom gain 0.5. Native adapters now apply recovered-controller volume and Q12 pitch to packet mixing; pan/elevation, cutoff and effect values are retained without audible DSP. These adapters do not recover the retail voice registry or mixer. Not modelled: voice graphs/generations, render modes, pan vectors, filters/effect sends, `samplerep`, and the frame callback. Without a device, create/start return -10 and the recovered code stays silent. Parameter dispatch `0x735580` is recovered separately; its WaitForSingleObject/ReleaseMutex imports use a dedicated recursive SDL mutex, tested with callback mutation, nested dispatch and thread contention (`ug2audiodispatch_native_test`). Callback registration/consumers remain unrecovered. The defaults match the retail bytes (`verify_native_snd_helpers.py`); the rest is covered only by `smoke_native_engine_audio.py`. |
| Engine sound control/manager input seams | `NativeEngineSound.cpp`/`.h`; `DemoMain.cpp` | Recovered component sort/scheduler/post passes, Physics, spatial, auxiliary, gains and playback are wired; manager lifecycle and registry remain | 4 | 4 | Recovered participant selection `0x4593b0`, car/engine construction, binding, bank loading and setup choose the actual default bank(s). Initialization calls recovered sorting `0x464a10`; each frame consumes the vehicle export at owner `+0x440` and runs car scheduling `0x469cf0`/`0x464c10` and playback pass `0x469cb0`. The linked input order is Physics `0x484c50` + post `0x470a60`, shifting `0x483130` + post `0x45c210`, acceleration transitions `0x4831a0`, complete auxiliary `0x47c010`, HybridMotor `0x47c3d0` + post `0x470940`, then spatial `0x471dc0`; playback tail `0x460ff0` reaches `0x474310`. The auxiliary caller now includes position capture, normalization, its `+0x1d0` envelope, idle, offsets, pulse sequencing, RPM/throttle/level, cutoff and high-RPM ramps in the original direct/virtual order. Shared integer/float random, sine, normalization and inverse curves execute recovered bodies. Spatial construction/bind/activation, selection, distance/angle calculation and Doppler averaging are recovered and linked through native ABI bridges. A persistent constructed audio object replaces the stack bank context. Complete listener producer `0x46d7e0` and list lookup `0x4690b0` now run before component updates, using the same finalized camera used for rendering. Original two-player blending and view-mode gates are recovered; the native session supplies one car through a minimal manager list, with sound-state `+0xa8` still zero until its producer is recovered. Acceleration and shifting construction/binding/activation and their complete state machines now run; the six sorted inputs use recovered original bodies. Global MasterVol and options mix application `0x469050` now run and share the UI record, including paused frames with a zero time step; original mixer binding, sound/input resolution, channel record decoding and spatial attachment now execute during native engine initialization (see the mixer row below); per-frame gain/pan production remains open. Notification bypass, original registry/list construction and full manager dispatch remain explicit seams. HybridMotor activation `0x45d050` is recovered and called; manager ownership/cleanup remain open. `parametersB0` remains null; initializer/registry/consumers are unwired. With retail mixer binding invoked, the latest 600-frame smokes pass for SKYLINE (pitch/RPM correlation 0.845) and 350Z (0.622), both with zero underruns. Earlier 350Z runs failed the unchanged pitch gate (0.417 and inverted high/low-RPM pitch); this single passing run does not establish repeatability or retail audio equivalence. Earlier acceleration-only results remain recorded. Native snd filters/pan remain retained state without retail DSP. Heap availability remains unbounded. See [latest mixer binding evidence](pc-ginsu.md#mixer-pointer-maps-and-object-resolution-2026-10-20). |
| Retail mixer binding wired; gain/pan output fixtures remain | `NativeEngineMixer.h`; `NativeEngineSound.cpp` | Binding `0x483900` → `0x478120` / `0x480420`; decoding/setup `0x479030` / `0x477a50`; spatial setup/attachment `0x4683e0` / `0x468430`; resolvers `0x468f70` / `0x468fe0` | 4 | 4 | The single-car native session loads the installed `SOUND/MIXMAPS/MAPOUTPUT.mxb`, supplies manager/list ownership and validates record spans. Recovered code decodes 16 master channels and five spatial entries, binds the actual engine output pointer and attaches its Spatial input. The old direct `playback_->level` pointer assignment is removed. **The output storage still contains the explicit 32767/centered-pan fixture: fixed-input graph binding, mix/event-control production and channel runtime mixing are not wired or recovered yet.** MasterVol therefore still does not drive the audible overall gain. Native allocation/profile selection and whole-group factories remain seams; this is binding integration, not complete mixer recovery. Missing/invalid maps report an engine-audio initialization error. Nine new routines have bounded original/rebuilt behavioral evidence, separately from byte matching. See [native binding evidence](pc-ginsu.md#native-retail-mixer-binding-2026-10-20). |
| ~~Replay recorder query traps~~ | `ReplayState.cpp` | `0x5efe70` | — | — | Recovered 2026-10-18; reached by the mover update for the human record. |
| bWare heap uses malloc/free | `NativeWorld.cpp:61` | `0x440390` `bMalloc`, `0x440540` | 2 | 1 | A reasonable permanent seam. |
| `bGetTicker` and `bSPrintf` | `NativeVehicle.cpp:179`, `NativeCollision.cpp:20` | `bSPrintf` `0x4400d0` | 1 | 1 | |
| D3DX matrix multiply | `NativeVehicle.cpp:161` | Statically linked D3DX (library, not game code) | — | — | Keep native. |

## Platform backends that stay native

- SDL2 window, events, game controllers (`DemoMain.cpp`, `NativeUiInput.h`,
  `NativeGamepad.h`).
- SDL2 audio device: `SDL_INIT_AUDIO`, open/close and statistics (`NativeAudioOpen`/
  `NativeAudioClose` in `NativeAudio.cpp`, called from `DemoMain.cpp`). `--no-audio` or a
  missing device runs the demo silent.
- OpenGL drawing of cars, scenery and FEng (`NativeScene.cpp`, the GL calls in
  `NativeFng.cpp`).
- File reads and DXT decoding (`NativeAssets.cpp`, `ReadWholeFile`).
- The original-image mapper with its SHA-256 check (`OriginalImage.cpp`) and
  `OriginalSymbols.ld`.
- The formatting counter and bad-pointer checks (`NativeFormatting.cpp`).
- The optional mod bridge (`NativeModBridge.cpp`); it is outside recovery scope.
- Test harnesses (`*NativeTest.cpp`, `CollisionProbe.cpp`, `PhysicsRecordProbe.cpp`,
  `SimMain.cpp`).
