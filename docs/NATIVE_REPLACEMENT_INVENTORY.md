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
| 2 | Car wheel draw `0x620ae0` wheel block: placement, side scale, width (see [wheel seams](#car-wheel-rendering-seams)); getters `0x5f6820`/`0x5ec7f0` done | 4 | 4 |
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
| ~~Wheel render frames composed natively from `DrivetrainWheel` `matrix30`×`matrix70`~~ (done) | `DemoMain.cpp` `DrawCarModel` | `0x5f6820` (recovered, wired in), `0x5ec7f0` (recovered, not drawn) | — | — | Remaining seam: unrecovered draw `0x620ae0` (body-frame composition and positive-y mirroring in `NativeCarModel::Draw`; the original zeroes frame y and scales from render-info `+0x420`). Fallback `DrawWheel` unchanged. Remaining seams: [car wheel rendering seams](#car-wheel-rendering-seams). |

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

Snapshot 2026-10-19. The wheels still do not look right in the demo: they sit too low in the
arches in both the old and new frame paths, and only the side choice of the mirroring was
corrected visually. The getters `0x5f6820`/`0x5ec7f0` are recovered and verified
(`verify_pc_wheel_render.py`). Everything after them is native. The original chain is the
render callers `0x631250`/`0x6316d0`, which call `0x5f6820` (tyres, brakes) and `0x5ec7f0` (a third
set) and pass them with the body matrix (car `+0x5a0` or `+0x50`) and car `+0x560`/`+0xd0` to the
car draw `0x620ae0` (66 KB, not recovered; Ghidra's decompiler times out on it). Addresses below
are disassembly leads, not verified boundaries.

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Body-frame composition: each car-local frame is multiplied under the body frame (`matrixD0` with `origin10`) | `NativeScene.cpp` `NativeCarModel::Draw` | `0x620ae0` wheel block (`0x62710d`..`0x6274b4` and the draws after it); `0x4012b0` is called 32 times there | 4 | 4 | The parent frame the original uses (its body argument versus `+0x560`/`+0xd0`) is not established. |
| Lateral placement and side: the record's wheel y is kept, and positive-y wheels are mirrored with `glScalef(1,-1,1)` | `DemoMain.cpp` `DrawCarModel`, `NativeCarModel::Draw` | `0x620ae0`: zeroes element 13 (y translation) of the tyre, brake and third-set frames (`0x627462`..`0x627481`), then builds four per-wheel matrix sets on the stack (`+0x200`, `+0x3d0`, `+0x4d0`, `+0x5d0`, stride 0x40) | 4 | 4 | Element 5 (y scale) = width × render-info `+0x420[w]` (falls back to `[0x8026c4]` when zero); element 13 = −(lateral offset); the front wheels (offset < 0x80) also subtract an offset from elements 14 of three sets (`0x627432`..`0x62745b`). Measured on SKYLINE: the tyre solid's outer face/normals point -y (y [-0.017, 0.234], radius 0.354), hence the mirror; the scaffold also translates the tyre by minus its mid y (brake unchanged) to centre it on the wheel plane. The tyre radius is not scaled to the spec radius 0.327. The stock `FRONT_WHEEL_A` is a hollow tyre: no rim solid is drawn, so the brake shows through the centre. Lead for the original brake placement: `0x638190` reads `+0x430`/`+0x434` from the `MARKER_BRAKE` marker (string `0x7a8d14`, field `+0x44`) of the wheel models; in SKYLINE GEOMETRY.BIN that marker (chunk `0x0013401a`) has y translation 0.0974. |
| Wheel width/size scaling absent (model drawn at authored size) | `NativeCarModel::Draw` | `0x620ae0` `0x62700e`..`0x627086`: front/rear widths from render-info `+0x430`/`+0x434` unless the globals `0x7a668c`/`0x7a6690` are non-zero | 3 | 4 | `+0x430`/`+0x434` are written by the CarRenderInfo constructor `0x638190` at `0x6390ef`/`0x639102` from field `+0x44` of the solids at render-info `+0x1b48`/`+0x1b68`. A likely cause of the low or oversized look. |
| Spin angle / extra rotation computed in the draw | — | `0x620ae0` `0x6270a2`..`0x627102`: `0x59a420` on car `+0x10`, scaled by `0x7a6754`/`0x784238`, `0x75bed4`, reduced with the 0xb60b60b7 reciprocal (divide by 360) | 3 | 2 | Purpose unproven; not applied natively. |
| Third frame set `0x5ec7f0` not drawn | `DemoMain.cpp` | `0x620ae0` argument 7 (`[ebp+0x20]`) | 2 | 2 | Recovered (`GetWheelSpinMatrices`); its role in the draw is unknown. |
| Wheel/brake solid selection: `<CAR>_KIT00_FRONT_WHEEL_A` for all four wheels, `FRONT_BRAKE_A`/`REAR_BRAKE_A` by axle, always LOD A | `NativeScene.cpp` `NativeCarModel::Load` | CarRenderInfo `0x638190` model slots `+0x1b48`/`+0x1b68` (wheel), `+0x1b78`, `+0x1ba8` (brake loop at `0x626f80` via `0x48d860`, `0x5c5930`) | 3 | 3 | Rear wheel and rim/kit choices are not original; no LOD selection. |
| Solid draw and render state: GL display lists, no culling, `glFrontFace` toggled for mirrored wheels, fixed-function lighting | `NativeScene.cpp` | Solid draw `0x5c5930` (8 calls in the wheel block), `0x48f660`, `0x620600`, `0x60d700`; D3D path (`0x5c7ea0`, `0x5c5020`) | 5 | 3 | GL stays the backend. The culling and lighting decisions should follow the original states. |
| View flag: `0x5f6820` is called with view 0, so the rest-pose branch (`value5B0` bit) is never chosen per view | `DemoMain.cpp` | View index from the callers `0x631250`/`0x6316d0` (`param_1 + 4`) | 1 | 1 | |
| Procedural fallback `DrawWheel` composes `DrivetrainWheel` `matrix30`×`matrix70` when no car model loads | `DemoMain.cpp` | None (pure scaffold) | — | 1 | Keep, or remove once model loading is reliable. |
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

Audio setup, controllers, parameter callbacks and car dispatch updated 2026-10-20; the remaining game audio seams are still open.

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| Audio enable flag forced to 0 (`0x7faaa8`) | `NativeVehicle.cpp:534` | Option reads in `0x580130` (one `0x57e2a0` read per flag; `0x7faaa8` is one of them) | 2 | 4 | Keeps every flag-gated audio path in recovered code dead: frontend sounds `0x458520`, HUD sound `0x51de30`, collision sound set `0x483e20`/`0x483e40`, car sound creation `0x4690e0`. The engine sound scaffold below does not read the flag. |
| Collision sound set traps | `NativeCollision.cpp:77–78` | `0x481cd0` (add), `0x481e50` (remove) | 4 | 2 | Unreachable while `0x7faaa8 = 0`. The collision path uses the CSIS/stitch banks (`MB_Stich_Collision.abk`). |
| Car sound state managers; ~~creation dispatch trap~~ | `AudioSystem.cpp`; native audio initialization | ~~`0x4690e0` dispatch~~ (recovered and linked 2026-10-20); manager pool/sound-object construction at EAXSound `+0xac`/`+0xb0`/`+0xb4` remains | 5 | 3 | Dispatcher now follows the original enable flag, context kind/`+0x900` override, null-manager gates and slot `+0x10` call. Audio constructor `0x4585a0`, manager kind/limit initializer `0x463ad0`, lookup/bind dispatch `0x463cc0` and queue `0x463f10` are recovered; native pause/options initialization calls the constructor. State-manager allocation/initialization `0x4853c0`, pool/sound-object creation and the virtual context/bind methods remain open, so this does not enable car audio. See [construction evidence](pc-ginsu.md#audio-object-and-car-dispatch-recovery-2026-10-20). |
| snd library: SDL2 packet player | `NativeAudio.cpp`, `NativeAudio.h` | snd library `0x721f99`/`0x721fa7` (critical section), `0x723124` overhead, `0x723138` create, `0x7231d4` start, `0x723515` submit, `0x72387b` stop, `0x72366a` destroy, `0x72423e`/`0x721a6b` defaults | 5 | 2 | EA middleware, statically linked; it may stay native like D3DX. Modelled: mono packet queues per player (8 players, 32 packets), linear resampling to 44.1 kHz stereo, release callbacks on the audio thread under the recursive SDL device lock, and a fixed gain of 0.5. Not modelled: voices, render modes, the mixer, the volume/pitch/pan setters (`0x72412a`, `0x724093`, `0x723d9b`, `0x72403e`, `0x723c41`, `0x723bc1`), `samplerep`, and the frame callback. Without a device, create/start return -10 and the recovered code stays silent. Parameter dispatch `0x735580` is recovered separately; its WaitForSingleObject/ReleaseMutex imports use a dedicated recursive SDL mutex, tested with callback mutation, nested dispatch and thread contention (`ug2audiodispatch_native_test`). Callback registration/consumers remain unrecovered. The defaults match the retail bytes (`verify_native_snd_helpers.py`); the rest is covered only by `smoke_native_engine_audio.py`. |
| Engine sound scaffold | `NativeEngineSound.cpp`/`.h`; `DemoMain.cpp:412–417`, `:784`, `:830` | ~~Engine SFX setup `0x47d740`~~ (recovered and wired 2026-10-20) and ~~engine controllers `0x473e60`/`0x474030` with caller `0x474310`~~ (recovered 2026-10-20; not wired) | 4 | 4 | Fixed gin (`--engine-gin`, default `GIN_Nissan_Skyline.gin`) instead of the per-car table at EAXSound `+0xdc`/`+0xe4`. Recovered setup now owns allocation, packet sizing, binding and start (`0x47d8f9`), including both mode-1 slots. Native initialization now calls the recovered constructor chain `0x45e350` → `0x473bf0` → `0x473d30` before setup, replacing zero-initialization and the manual slot default. It still supplies mode 0/index 0, a temporary car binding and a temporary bank view. See [construction evidence](pc-ginsu.md#engine-constructor-chain-2026-10-20). The recovered `0x440820` wrapper reaches the native `bMalloc`/`calloc` backend; the native heap-capacity query `0x43e910` is unbounded. See [setup evidence](pc-ginsu.md#engine-setup-recovery-2026-10-20). Each unpaused frame, `UpdateFrequency(rpm, 60.0f)` (latency from `0x473fc7`) with RPM read straight from the engine (`engine48->state14`); the recovered controllers read frequency `+0x104` and integer gain from vtable `+0x38`; Recovered caller `0x474310` copies frequency from control `+0xf8`, selects a controller and fills the additional parameter block. Parameter callback dispatch `0x735580` is now recovered with a separate native SDL mutex backend and a composed caller comparison; callback registration and actual consumers remain open. The packed value queries `0x45e390`/`0x45e3b0` and setup lifecycle `0x47d920` are now recovered and compared in composition, but not wired. Table `+0x58` record production, the auxiliary float query, control-state production, allocator/registry behavior and snd setter backend integration remain open. See [initialization evidence](pc-ginsu.md#engine-value-queries-and-initialization-2026-10-20). See [controller evidence](pc-ginsu.md#engine-controller-recovery-2026-10-20). Only the accel slot is fed (no decel slot in mode 1), with no volume or pan. |
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
