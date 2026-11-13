# Code guide and architecture

How the repository is organised and how a frame of the native demo flows through it. For the
per-file listing see [SOURCE_INDEX.md](SOURCE_INDEX.md) (generated); for what is still missing see
[COVERAGE.md](COVERAGE.md) (generated). Evidence for individual routines is in the `pc-*.md`
documents listed in [README.md](README.md).

## 1. What this repository is (and is not)

An **incomplete decompilation** of EA Black Box's SPEED engine, reference build
*Need for Speed Underground 2*, PC v1.2 (`SPEED2.EXE`, 32-bit x86, MSVC 7.1, image base
`0x400000`). Recovered routines are C++ that must compile with MSVC 7.1 *and* build natively with
`gcc -m32`. Everything else needed to run (window, GL drawing, file IO, asset mapping) is
**platform scaffolding** under `src/Platform/Native`.

It is not a standalone game: assets and the retail `SPEED2.EXE` come from the player's own install
(the runtime checks the executable's SHA-256 and maps reference data from it).

Three evidence levels are kept apart everywhere (see [DECOMPILATION_SCOPE.md](DECOMPILATION_SCOPE.md)):
**byte-matching**, **bounded behavioural verification** (Unicorn differential tests, native probes),
and **source presence** (which proves nothing by itself).

## 2. Layout

| Path | Role |
| --- | --- |
| `src/Speed/Indep/Src/<Area>/` | Recovered game/engine routines: `Physics`, `Physics/Collision`, `World`, `Camera`, `Hud`, `Frontend`, `Gameplay`, `Audio`, `EAXSound/Ginsu` |
| `src/Speed/Indep/Libs/bWare/` | EA's base library: strings, formatting, slot pool, random, timed averages, compression |
| `src/Speed/Indep/Libs/D3DX/` | The D3DX effect (`.fx`) framework the game statically links: effect objects, pass/state caches, VM, CRT math |
| `src/Platform/Native/` | Linux scaffolding: `DemoMain` (SDL2 + OpenGL), `Native*` seams, probes (`*Probe.cpp`), native tests, `OriginalImage` (maps the retail image), `OriginalSymbols.ld` (pins symbols to original addresses) |
| `modding/` | Optional C SDK (`sdk/ug2_mod_api.h`), loader (`runtime/`), examples (widescreen HUD, original physics) |
| `config/SPEED2_EXE_1_2/*.json` | Per-subsystem manifests: function name, original address, size, SHA-256 of the original bytes |
| `tools/` | `verify_pc_*` (original vs rebuilt), `probe_pc_*` (investigation), `smoke_native_*` (run the demo), analysis helpers |
| `docs/` | Evidence per subsystem; this guide; generated indexes |

## 3. Conventions that matter when reading the code

- **Original addresses are part of the code.** Comments and `extern` variable names carry the
  retail VA, e.g. `extern int drivetrainClock; // 0x86518c`, or `// Car update 0x6012b0`. Globals
  keep their original address via `OriginalSymbols.ld` so recovered code and mapped retail data
  agree.
- **Unknown layout stays offset-based.** Fields are named by offset (`value3F0`, `car4`,
  `unidentified0[4]`) until the binary identifies them; do not "clean up" these names.
- **Unrecovered callees are explicit seams**, declared with a comment such as
  `// 0x403db0, not recovered`, and are provided by the platform layer or trap.
- **Arithmetic order and call boundaries are preserved** (x87 evaluation, component write order,
  alias-safe reads) because verification compares against emulated original code.
- **Recovered vs scaffold is a hard split.** `src/Speed` is recovered; `src/Platform` is stand-in.
  The stand-ins are listed with their replacement targets in
  [NATIVE_REPLACEMENT_INVENTORY.md](NATIVE_REPLACEMENT_INVENTORY.md).
- Target C++11, no exceptions/RTTI assumptions beyond the legacy compiler, four-space indent.

## 4. Build targets (CMake)

- `ug2recovered` (static): globbed recovered sources, compiled `-m32 -fno-pie -mfpmath=387
  -ffp-contract=off -O1 -w` with `NativeCompat.h` force-included. x87 math and no contraction are
  required to reproduce original floating-point results.
- `ug2native` (static): the `NativeFx*` effect/CRT dispatch layer, `NativeVehicle`, `NativeWorld`,
  `NativeCollision`, audio and engine-sound seams, `OriginalImage`.
- `ug2demo`: SDL2/OpenGL driving demo (`DemoMain`, `NativeScene`, `NativeFng`, `NativeHud`,
  `NativePause*`, `NativeControls`, texture/solid record loaders).
- `ug2sim`: headless simulation host. `ug2*_probe` / `*_native_test`: one executable per verified
  subsystem, used by the Python verifiers.
- `UG2_ENABLE_MODDING=ON` adds the mod loader and examples.

## 5. Subsystem map

### Physics (`Src/Physics`, ~106 files)
Layered the way the original is: `PhysicsMover` update chain (`0x5b72d0`) → body
(`MoverBody`: force/derivative/matrix, `0x59b…`–`0x5a…`) → wheels and suspension arms →
engine and drivetrain (shift points, coupling, distribution, wheel feedback) → steering (human
steering, Ackermann, timed averages) → parameter/record layer (`PhysicsParameters`, tuning
records and upgrades, variants; chunk loaders). `CarConstruction`/`CarUpdate` bind a car
(`0x604260`, `0x6012b0`) to a mover; `WorldUpdate` (`0x609ce0`) composes body advance, collisions
and post-update in original order. `Collision/` holds GJK, simplex, impulse, ray and cylinder
shapes, and world-object queries. Docs: `pc-vehicle-physics`, `pc-wheel-physics`,
`pc-physics-parameters`, `pc-human-steering`, `pc-collision-notifications`.

### World (`Src/World`)
Track and car rendering data path: collision packs and position markers (`TrackPositionMarkers`,
start grid `0x5eb5d0`), solid (model) and texture pack loading (`0x493bb0`, `0x5cc280`), the
car-render chain (`CarRender*`: records, lighting, technique map, uploads, quad submission, LOD,
wheels). Document: `pc-track-markers`, `pc-wheel-quad-submit`, `pc-wheel-secondary`.

### Camera, HUD, Gameplay
`Camera/` – view matrices, chase and cockpit cameras, push-out against world/car, shake and
collision shake. `Hud/` – gauges, minimap, free-roam HUD, mask producer `0x600090`, animations.
`Gameplay/` – drift ranking and career reward/eligibility.

### Frontend (`Src/Frontend`)
The FEng UI runtime: packages, objects and factories, scripts and tracks, responses, lists/grids,
text, queue and delivery, plus the original frontend classes built on it (icon scroller, pause
menu, pause options, PC controls). The global UI manager at `*0x8384c4` is not recovered; native
code supplies package/archive lookups and queue dispatch (see `pc-pause-menu`).

### Audio (`Src/Audio`, `Src/EAXSound/Ginsu`)
Audio mix/resolve plus Ginsu, the granular engine synth: `.gin` data, EA-XA decoding and packet
synthesis. Playback backend is native. Doc: `pc-ginsu`.

### bWare and D3DX libraries
Strings and formatting (byte-matching), slot pools, compression; D3DX effect objects with a VM
that evaluates compiled effect bytecode, plus the statically linked CRT power/trig routines.
`NativeFx*Dispatch.cpp` is the seam that lets the original effect code call native GL services.

## 6. A frame of the native demo

1. `DemoMain` (SDL2) polls input via `NativeControls`, advances a timestep (fixed 1/60 s for
   captures, 0.05 s clamp). The real loop `0x5811c0` is not recovered; only its timing decisions are.
2. `NativeVehicle` steps the vehicle through recovered `WorldUpdate` → car update → mover chain
   (body, wheels, engine, drivetrain, steering) using collision data fed by `NativeWorld`/`NativeCollision`.
3. Camera (`DriveCamera`) and HUD (`HudController`, `HudFreeroam`, `HudMinimap`) update from the
   player state; the pause menu runs on Esc through the FEng runtime and `PauseMenu`/`PauseOptions`.
4. `NativeScene` and `NativeCarWheels` draw track and car; car records are prepared by the recovered
   car-render chain and consumed by native GL. Effects run through the recovered D3DX VM.
5. `NativeEngineSound` drives Ginsu synthesis into the SDL audio device.

## 7. Verification workflow

`tools/verify_pc_<subsystem>.py` rebuilds the recovered routine (MSVC 7.1 under wibo, or native i386
gcc), runs original and rebuilt code under Unicorn with generated cases, and compares results;
verifiers include mutation checks that must fail. `probe_pc_*` scripts investigate. Per-subsystem
manifests in `config/SPEED2_EXE_1_2/` pin function address, size and byte hash. Requires: MSVC 7.1,
wibo, Unicorn, Capstone, pefile, delink/objdiff for string matching, and the retail executable.

## 8. Tracking progress

- `python tools/coverage_inventory.py SPEED2.EXE [--names <yugecin funcs.html>]` regenerates
  [COVERAGE.md](COVERAGE.md): functions cited in `src/` versus discovered in `.text`.
- `python tools/gen_source_index.py` regenerates [SOURCE_INDEX.md](SOURCE_INDEX.md).
- Substitutes still standing in for original code: [NATIVE_REPLACEMENT_INVENTORY.md](NATIVE_REPLACEMENT_INVENTORY.md).

## 9. External references

- [yugecin/nfsu2-re](https://github.com/yugecin/nfsu2-re): hand-named function list (~1.7k names), struct
  and variable layouts, and UI/FNG notes for the same game. Used here only as *leads* for naming
  and for grouping unresolved functions; names are not evidence (see
  [nfspluginsdk-reference.md](nfspluginsdk-reference.md) for the same policy applied to another
  community source).
- `nfsu2-recomp` (static x86→C→ARM64 lifter, sibling project): produces generated, unmatched C for
  every function; it is a different goal and its output is not part of this tree.
