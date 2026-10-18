# Native code replacement inventory

Snapshot: 2026-10-18 (after commit `3007487`). Reference build: UG2 PC v1.2
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

| # | Item | Diff. | Imp. |
| --- | --- | --- | --- |
| 1 | ~~Car update `0x6012b0` and world update `0x609ce0`~~ (done; many callees remain seams) | 3 | 5 |
| 2 | ~~Wire in verified-but-unintegrated tuning routines~~ (already linked; reachable only once the tuning-table loader, item 8, exists) | 1 | 3 |
| 3 | FEng messaging: frame manager `0x558c00`, delivery `0x541fc0`, dispatch `0x5367e0` | 3 | 4 |
| 4 | Car constructor `0x604260` and participant setup `0x6065b0` (replace `CreateNativeVehicle`) | 4 | 5 |
| 5 | FEng package/object loader `0x5554a0`/`0x555860` | 4 | 4 |
| 6 | Collision: GJK `0x5b3390`, impulse `0x5af0e0`, world objects, cylinders | 3–4 | 4 |
| 7 | Camera seams: pushes, shake, cockpit tests | 2–3 | 3 |
| 8 | CarTypeInfo/tuning chunk loaders | 3 | 3 |
| 9 | Frontend classes (IconScroller, options list, dialog, Controls) | 3–4 | 3 |
| 10 | HUD feature construction and mask caller | 3 | 3 |

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
| `CreateNativeVehicle`: allocation, vtable shims, defaults, yaw-matrix placement, collider attach | `NativeVehicle.cpp:~228–310` | Car constructor `0x604260`, mover factory `0x5f46a0`, participant setup `0x6065b0` | 4 | 5 | Largest native substitute in the driving path. |
| `New...` allocation factories (`NewMoverBody`, `NewEngine`, …), zero-filled blocks | `NativeVehicle.cpp:100–160` | Slot pools `0x440e50` and the original factories | 3 | 2 | Zero-filling hides uninitialised-read differences. |
| Tuning bound to car by native code | `NativeVehicle.cpp:228` | The original caller that binds tuning to its car (unidentified) | 3 | 3 | Composer `0x5b4dc0` itself is recovered. |
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
| GJK and impulse resolution not yet verified | — | Verify `0x5b3390`, `0x5af0e0`; resolve 42 simplex ties in `0x595040` | 3 | 4 | |
| World-object promotion/break trap | `NativeCollision.cpp:101–107` | `0x5b31f0`, `0x5ae000` | 4 | 4 | Breakable props and barriers. |
| Cylinder shapes trap | `NativeCollision.cpp:141` | `0x5a23d0`, `0x5a2780` | 3 | 3 | |
| ~~Contact notification to the participant traps~~ | `CarUpdate.cpp` | `0x5f4960` | 3 | 3 | Recovered; the native participant scaffold's slot `0x38` is a no-op and mover switch `0x5f46a0` traps. |
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
| Camera shake | same | `0x452ed0`, `0x572a50` | 2 | 2 | |
| Camera finalisation | same | `0x447280` | 2 | 2 | |
| Cockpit blocked tests | same | `0x445fc0`, `0x5d39f0` | 2 | 2 | |
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
| Fixed paint colour for SKIN1; no vinyls, damage or light switching | `NativeScene.cpp:114–140` | Skin builder, `0x638190`, `0x631e20` | 4 | 2 | |
| Scaffold lighting and horizon colour | `DemoMain.cpp:266`, `:673` | Time-of-day/fog colour selection | 4 | 2 | |

## 7. FEng engine (shared by HUD and menus)

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| `AdvanceScripts` frame seam (≤20 ms steps, no drains between steps) | `NativeFng.cpp:722` | Frame manager `0x558c00` | 3 | 4 | Script playback beneath it is already recovered. |
| `Pump()` native delivery bridge and zero-length step for new screens | `NativePause.cpp:271` | Delivery `0x541fc0`, dispatch `0x5367e0`, operation drains | 3 | 4 | |
| HUD posts to a discarded local queue | `NativeHud.cpp:510` | Register the HUD package with the frame manager | 2 | 3 | Follows from `0x558c00`. |
| Package loading/removal trap; native package chain and order | `NativePause.cpp:163`, `:513–524` | `0x555860`, `0x5362e0`, `0x51ab20`/`0x51ac10`/`0x51adb0`/`0x51b070` | 4 | 4 | |
| Object/record/archive loading; parsed file order kept | `NativeFng.cpp:618`, `NativePause.cpp:197` | `0x5554a0` and its chunk walk | 4 | 4 | |
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

| Native piece | Where | Replace with | Diff. | Imp. | Notes |
| --- | --- | --- | --- | --- | --- |
| No sound device (`0x7faaa8 = 0`); collision sound set traps | `NativeVehicle.cpp:193`, `NativeCollision.cpp:93` | Audio system | 5 | 3 | Needs a native audio backend as well. |
| Replay recorder query traps | `NativeVehicle.cpp:186` | `0x5efe70` | 4 | 1 | |
| bWare heap uses malloc/free | `NativeWorld.cpp:61` | `0x440390` `bMalloc`, `0x440540` | 2 | 1 | A reasonable permanent seam. |
| `bGetTicker` and `bSPrintf` | `NativeVehicle.cpp:179`, `NativeCollision.cpp:20` | `bSPrintf` `0x4400d0` | 1 | 1 | |
| D3DX matrix multiply | `NativeVehicle.cpp:161` | Statically linked D3DX (library, not game code) | — | — | Keep native. |

## Platform backends that stay native

- SDL2 window, events, game controllers (`DemoMain.cpp`, `NativeUiInput.h`,
  `NativeGamepad.h`).
- OpenGL drawing of cars, scenery and FEng (`NativeScene.cpp`, the GL calls in
  `NativeFng.cpp`).
- File reads and DXT decoding (`NativeAssets.cpp`, `ReadWholeFile`).
- The original-image mapper with its SHA-256 check (`OriginalImage.cpp`) and
  `OriginalSymbols.ld`.
- The formatting counter and bad-pointer checks (`NativeFormatting.cpp`).
- The optional mod bridge (`NativeModBridge.cpp`); it is outside recovery scope.
- Test harnesses (`*NativeTest.cpp`, `CollisionProbe.cpp`, `PhysicsRecordProbe.cpp`,
  `SimMain.cpp`).
