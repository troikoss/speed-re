# UG2 demo port: Linux x86-64, Linux ARM64, then Android

Status: planning baseline; no x86-64 or ARM64 runtime support is implemented by
this document. Current CMake targets still build i386 code. This plan ports the
existing demo and its recovered paths, not the whole game.

## Branches and integration

- `x64` owns the shared 64-bit compatibility work and the first playable Linux
  x86-64 milestone.
- `arm64` starts from the `x64` documentation checkpoint and consumes shared
  portability fixes from `x64`. Its milestones are Linux AArch64 first, then
  Android `arm64-v8a` on the same branch.
- Merge completed, validated shared changes from `x64` into `arm64`; retain the
  ARM64 branch mission when resolving guidance conflicts. Return generally
  useful platform fixes to `x64` in focused commits. Do not build competing
  address/layout/callback implementations or merge Android-only work upstream
  merely to synchronize branches.
- Keep i386 working on both branches. Neither branch initially supports the
  optional mod loader on its new architectures. Existing i386 mod support stays.
- Preserve original addresses, unknown offset-named fields, call boundaries,
  arithmetic order, and engine/game/platform separation. Add no substitute
  gameplay algorithms. New compatibility, rendering and Android services are
  platform scaffolding and must be documented as such.

## Baseline and evidence

The common audio checkpoint is `9531e3d` (parent `c9315f6`). It preserves the
previously uncommitted gain recovery, sample-average recovery, playback wiring,
native adapters, tests and evidence updates without further gameplay changes.

Validation performed for this checkpoint on 2026-10-20:

- `cmake -S . -B build/native -DUG2_ENABLE_MODDING=OFF`: passed.
- `cmake --build build/native -j2`: passed, with existing layout warnings.
- `ctest --test-dir build/native --output-on-failure`: 6/6 passed outside the
  sandbox; all six were stopped by sandbox `SIGSYS` restrictions on the first run.
- Python AST parsing of both changed scripts, JSON parsing of the Ginsu manifest,
  and `git diff --check`: passed.
- Original/rebuilt verifiers and retail-dependent smoke checks were not rerun:
  this checkout lacks MSVC 7.1, wibo, Unicorn and retail inputs. Historical results
  in subsystem documents remain historical evidence, not fresh validation.

Before changing runtime behavior, capture a repeatable i386 baseline using the
player's fingerprint-checked PC v1.2 executable and assets. Record compiler and
dependency versions, fixture hashes, original executable hash, commands and
limitations. Run affected Ginsu/wheel/camera/frontend verifiers sequentially
when MSVC/wibo is involved; they share temporary filenames.

Capture fixed-tick steering, acceleration, braking, reverse, shifting, reset,
collision and camera scenarios; HUD, pause/options and control navigation; and
one-bank/two-bank audio. Store per-tick physics state, ordered boundary calls,
screenshots, PCM and underrun counters under ignored build directories. Never
commit retail files or generated disassembly/reports.

## Stage 1: build separation

Introduce explicit profiles `linux-i386`, `linux-x86_64`, `linux-aarch64`, and
`android-arm64-v8a`, with independent build directories. These profile names are
planned interfaces, not configure options that exist today.

- Move `-m32`, x87 options, non-PIE flags and `OriginalSymbols.ld` to i386-only
  target settings. Portable targets use position-independent code; Android's
  application is a shared library.
- Split targets into recovered routines, shared compatibility services, desktop
  platform services and rendering. Keep platform code C++11 and original verifier
  sources compatible with MSVC 7.1.
- Replace global include paths, Debian i386 library paths and the host-specific
  SDL config-header override with target-scoped dependency discovery. Keep those
  overrides out of portable include paths.
- Provide native and cross-toolchain configuration, an optional headless build,
  and a clear configuration error for unsupported portable mod-loader builds.
- Extend smoke tools to accept a build directory or executable instead of
  assuming `build/native`.

Gate: unchanged i386 build, CTest and affected verifier results before accepting
the build refactor. Do not claim support from compiler configuration alone.

## Stage 2: shared 64-bit layouts, addresses and callbacks

Simply removing `-m32`, packing classes or disabling assertions cannot preserve
the original ABI. Add a shared compatibility implementation under the platform
layer, with these internal contracts:

| Concept | Representation |
| --- | --- |
| Original data address | 32-bit guest address translated to managed host storage |
| Original pointer field | Four-byte stored guest value with explicit translation |
| Original vtable/callback entry | Four-byte token resolved through a typed adapter |
| Native platform object | Ordinary host object with full-width host pointers |

### Address space and data

- Manage original static data and dynamically allocated recovered records in
  one guest address model, independent of the host's chosen mapping addresses.
  Preserve guest identity for interior pointers and overlapping views.
- Load required executable data sections into managed storage after the existing
  SHA-256 check. Portable targets resolve globals instead of using fixed-address
  linker bindings. The executable remains a temporary runtime data dependency;
  no original instructions execute.
- Audit literal address dereferences, raw pointer casts, relocation loops,
  allocation/free paths, intrusive lists, sentinel values and pools. Updating
  named linker globals alone is insufficient.
- Keep pointer arithmetic in original byte units. Distinguish data pointers,
  code tokens and integer IDs; never truncate a host address into a guest field.
- Retain size/offset checks on guest records, including four-byte pointers.
  Preserve unknown fields. Use shared recovered algorithm bodies with suitable
  declaration/access adapters, not separately maintained replacement algorithms.
- Audit serialized fields, signed wrapping, `long`, `size_t`, varargs, UTF-16,
  unaligned reads and aliasing. Use fixed-width storage for original records and
  full-width host sizes. Preserve evidenced read/write order across callbacks.

### Dispatch and object ABI

- Replace portable reliance on compiler-inserted vptrs with explicit guest fields
  and slot dispatch. A host eight-byte vptr must not occupy a four-byte UG2 slot.
- Keep native ownership metadata and callback tables outside original records.
- Cover physics interfaces, camera slots, frontend deleting destructors, audio
  callbacks, and fastcall views containing unused EDX parameters. Removing
  calling-convention annotations alone does not fix signatures.
- Replace frontend writes of native function pointers into original data slots
  with tokens. Fail unresolved calls with the original target and caller context;
  never silently skip behavior or execute retail code.

Gate: asset-free address, layout, list/pool, callback and audio-packet tests pass
on i386 and x86-64. Portable tests must exercise host addresses above 4 GiB,
interior/aliased pointers, null/sentinel handling and allocation teardown.

## Stage 3: playable Linux x86-64

Bring up strings/compression and layout tests, then headless physics records and
vehicle simulation, world/collision and camera, frontend/HUD/pause/controls,
engine bank loading/synthesis/playback, and finally the desktop-rendered demo.
Retain current CLI behavior and user-supplied game installation paths.

Use x86-64 x87 diagnostic builds where the compiler supports them to isolate
layout defects from arithmetic changes. The portable runtime cannot depend on
those diagnostics or on `long double` having x87 semantics.

### Measured numeric parity

- Preserve expression order, explicit stores/conversions and integer widths.
  Disable fast math and FP contraction on recovered portable targets.
- Replace x87-only instructions, including Ginsu conversion idioms, with tested
  helpers. Cover ties, negative values, signed zero, overflow and nonfinite inputs.
- Require exact integer results, discrete decisions, event ordering and decoded
  integer samples where applicable in bounded fixtures.
- Start continuous-output comparison at absolute-plus-relative tolerance
  `1e-5 + 1e-5 * abs(reference)`. Compare NaN/inf classification and signed zero
  explicitly where behavior depends on them.
- For the same fixed 600-tick driving replay, start final-state limits at 0.25 m
  position, 0.1 m/s speed and 0.25 degrees orientation, with matching tested gear
  and collision-event sequences. The replay uses captured i386 timestep inputs;
  compare every tick to locate the first divergence, not just the endpoint.
- Investigate failures. Do not silently widen budgets; document causes and seek
  explicit approval for changed acceptance limits. Add narrow numerical
  compatibility where evidence shows consequential divergence.
- Report byte matching, bounded behavior and platform integration separately.

Gate: desktop x86-64 demo passes baseline scenarios and a 30-minute drive without
crashes, unbounded memory growth or recurring audio starvation. Record frame-time
and audio metrics; this establishes bounded demo parity, not full game recovery.

## Stage 4: Linux AArch64

Merge the validated shared implementation into `arm64`. Cross-compile and build
natively on real little-endian AArch64 Linux hardware. Reuse the same guest
records and dispatch layer; remove remaining x86 assembly and dependencies.
Run the same numeric and state fixtures to identify ARM-specific alignment,
conversion, library-math and rounding differences.

Use emulation for early headless tests only. Require hardware for graphics,
controller, audio and performance evidence. Validate the existing desktop GL
renderer first. Repeat the Linux acceptance scenarios and 30-minute drive.
Do not advertise a device as supported before testing it; no specific board has
been selected.

## Stage 5: GLES on Linux before Android

Add a selectable GLES 3.0 renderer while retaining desktop GL for comparison:

- Replace display lists with retained mesh/index buffers and immediate drawing
  with buffered geometry. Triangulate quads/strips consistently.
- Implement matrices, vertex colors, texture sampling and relevant fixed-function
  operations in shaders. Preserve draw order, transparency, depth/culling,
  texture orientation and required mipmaps.
- Cover world, vehicles/wheels, skid marks, HUD, FNG/text and diagnostic drawing.
- Retain enough CPU resource information to recreate the GPU state after context
  loss. Keep graphics changes in documented platform scaffolding.

Gate: x86-64 and Linux ARM64 run baseline GLES scenarios. Inspect matching-frame
captures against desktop GL; record driver-dependent differences rather than
claiming pixel identity across GPUs. Linux ARM64 GLES acceptance precedes Android.

## Stage 6: Android ARM64

Use an SDL2 application shell, matching SDL Java/native versions, Gradle and NDK
CMake integration. Defaults: minimum API 26, `arm64-v8a` only, GLES 3.0, landscape,
sideloadable APK. Pin SDK, NDK, Gradle, AGP and SDL versions in build configuration
when creating the Android project, and record the tested toolchain. Store
publication is outside this milestone.

- Provide a small platform setup flow using the document picker to select the
  player's game directory and copy required files into app-owned storage.
  Validate required paths, executable hash and free space. Handle interrupted
  imports with staging and completion checks; never treat partial data as ready.
  Feed the native runtime a normal local installation root. Package no retail
  data and require no broad shared-storage access.
- Route gamepad steering, pedals, handbrake, reset, cameras, pause and menu actions
  through existing native input seams. Show controller requirements; release held
  inputs and pause when the controller disconnects. Touch driving is deferred.
- Suspend simulation and audio when backgrounded. Reset the timing anchor on
  resume so background time cannot become one large physics step. Recreate
  graphics on surface/context loss and handle drawable changes, interruptions,
  process restart and missing imported assets.
- Replace fixed 4096-byte assumptions with runtime page-size handling. Validate
  native ELF and APK alignment and both 4 KiB/16 KiB environments.
- Adapt platform pointer-validation/clock/file services as needed without moving
  Android lifecycle behavior into recovered game code.

Gate: a real ARM64 device imports data, launches, drives with a controller,
exercises menus/cameras/audio, backgrounds/resumes and relaunches without
reimporting. Run a 30-minute drive and record device, OS, renderer, page size,
frame times, audio underruns and limitations. Emulator checks supplement this.

Official implementation references:

- [SDL2 Android shell and build integration](https://wiki.libsdl.org/SDL2/README-android)
- [Android NDK ABI guidance](https://developer.android.com/ndk/guides/abis)
- [Android native page-size support](https://developer.android.com/guide/practices/page-sizes)

## Verification and rollout

| Build | Required evidence |
| --- | --- |
| Linux i386 | Existing affected verifiers, CTest, baseline demo/audio checks |
| Linux x86-64 | Layout/address/dispatch tests, numeric fixtures, playable GL demo |
| Linux ARM64 | Portable tests, real-device GL and GLES demos |
| Android ARM64 | APK/alignment checks, real-device lifecycle/controller tests |

Normalize only documented pointer identities in cross-architecture state
comparisons, not gameplay fields. CI uses synthetic fixtures; retail-dependent
verification stays local. Run MSVC/wibo jobs sequentially. Keep each checkpoint
reviewable: build separation, address/layout support, dispatch, x86-64 demo,
ARM64 Linux demo, GLES and Android. After each, record exact revisions, commands,
hardware, dependency versions, evidence and unresolved calls/seams here and in
the affected subsystem documents.

Deferred: portable native plugins, Android touch driving, store publication,
other SPEED titles, full UG2 recovery, strict x87 reproduction everywhere, and
removal of the executable-data dependency. Finite test success cannot establish
complete equivalence or recovery.
