# Session handoff — 2026-10-12

## Goal and standing instructions

Audit the native UG2 demo and replace every replaceable gameplay substitute
with original UG2 PC v1.2 code recovered from the supplied executable. Continue
through steering, vehicle/session construction, update timing, reset/camera,
HUD/frontend, and asset/world/presentation logic. Preserve necessary Linux
backends, verify recovered routines and integration, and document unresolved
dependencies and unavoidable platform seams.

The larger repository objective is a reverse-engineered SPEED engine, with
complete, verified, reproducibly buildable UG2 source as the first target; a
playable demo is intermediate. Completion evidence is defined in
`docs/DECOMPILATION_SCOPE.md`. Do not redefine completion around isolated tests.

The user requires verified changes to be committed and pushed between pauses.
Push authorization persists for `main` at
`https://github.com/troikoss/speed-re.git`; do not ask again. Do not commit retail
assets, executables, compiler binaries, or generated disassembly. Keep unverified
work clearly identified rather than describing it as a verified checkpoint.

Follow `AGENTS.md`: always recover original code, never invent replacement game
algorithms. Preserve arithmetic order, call boundaries, addresses and x86
layouts; keep unknown fields offset-based. Report behavioral equivalence
separately from byte matching. Native code uses C++11; recovered source must
also compile with MSVC 7.1. Run MSVC/wibo jobs sequentially. Do not spawn agents
unless the user or applicable repository/skill instructions request delegation.

## Authoritative checkpoint

Workspace: `/home/dq/Documents/Development/speed-re`, branch `main`.
Latest verified checkpoint: **Recover FEng language-hash setter and observer notification**
(see Git history for its commit). Navigation checkpoint: `c0b18f1`; package mask/focus
checkpoint: `3878b16`. Direct user authorization to commit and push to `main` was renewed on 2026-10-12. The preceding code checkpoint is `3e58488` — **Give native
FEng objects the original object and script prefix**.

Recovered and verified since the previous handoff (each with a Unicorn differential verifier and
mutation tests; byte matching not established):

- HUD: mask producer `0x600090` + helpers (`verify_pc_hud_player_masks.py`), gauge update
  `0x4c68d0`, map feature `0x4da640`, feature dispatch `0x4ec5a0`, career/language readers
  (`verify_pc_hud_freeroam.py`). Evidence: `docs/pc-hud-freeroam.md`.
- Start grid `0x5eb5d0` and track position markers (`verify_pc_track_markers.py`); the native
  spawn is now the original grid (smoke 46.6/15.8 km/h from the parking-bay start).
  Evidence: `docs/pc-track-markers.md`.
- Collision sound wrappers, game message post, profile readers, camera option step
  `0x5f2fa0`, game-event record/apply `0x578ef0`/`0x4a7b30`, transmission setter `0x407280`,
  replay camera score `0x578e90`, control mask, return package (`verify_pc_small_queries.py`,
  `verify_pc_replay_events.py`). Evidence: `docs/pc-collision-notifications.md`.
- Native player record and camera-mover seam live in `src/Platform/Native/NativePlayer.h`.

## Current focus: replace native pause-menu code with original code

The user asked to focus on the pause menu. `NativePause.cpp`/`NativePauseOptions.cpp` are mostly
a stand-in for the FEng UI engine plus native versions of original frontend classes. The original
classes write FEng internals directly, so the work is staged:

1. **Done (`3e58488`)**: native FEng objects keep the original FEObject prefix
   (`HudVisibilityObject` in `Hud/HudVisibility.h`: guid `+0xc`, hash `+0x10`, type `+0x18`,
   flags `+0x1c` (0x400000 dirty, 0x4000000 focus), texture `+0x24`, state block `+0x2c`
   (size at `+0x38/+0x3c`), script list `+0x4c`, current script `+0x54`) and the FEScript
   prefix (length `+0xc`, time `+0x10`, hash `+0x30`). The native script player runs on
   `currentScript54->time10`. HUD/pause frames byte-identical.
2. **Done**: native screens retain FEPackage masks `+0x1c/+0x20/+0x24/+0x28`, focus
   `+0x8c`, focusable-object array `+0x90`/count `+0x94` (flag 0x10000000, reader `0x555140`),
   archive node's package `+0x18` and manager's queue pointer `+8`. `FEngPackage.cpp` recovers
   mask setter `0x537950`, selection get/set `0x50b8a0`/`0x50b8c0`, focus setter `0x52b3a0`
   and hash focus `0x52ce40`. `verify_pc_feng_package.py`: 10,000 comparisons; four rejected
   source mutations. Native icon selection uses hash focus; option widget focus uses SetFocus.
   Focus notifications are queued before assignment. **Correction:** selection `+0x10` is
   in the static FNG record (table `0x7f7dc8`, lookup `0x50b840`), not the live package.
   Native package/archive/record lookups and queue dispatch remain seams, including unmodelled
   target -5 delivery. The global manager at `*0x8384c4` and original lookup lists are still
   unrecovered (`0x52cef0`, `0x51bb70`, `0x517f80`, `0x52c570`). Native build, pause verifier
   (11,723), options verifier (5,730) and all 19 native settings scenarios pass.
   Evidence: `docs/pc-pause-menu.md#feng-package-masks-selection-and-focus`.
3. **Navigation/dispatch done**: `IconScrollerNavigation.cpp` recovers `0x51f9f0`/`0x51fac0`,
   `0x5276d0`/`0x527700`, `0x527730`/`0x527750`, `0x51f820` and `0x50d980`. Native
   scroller retains the original prefix through `+0x34`, supplies icon +4 intrusive links,
   and uses recovered navigation/indexing/dispatch. Post-selection layout, construction,
   insertion and the remainder of the scroller still use native seams. The original wrap routine
   loops when every icon is disabled and disabled selection is disallowed; do not add a guard.
   `verify_pc_icon_scroller_navigation.py`: 34,670 comparisons, four rejected mutations;
   byte matching not established. Pause/options verifiers and all 19 native settings scenarios pass.
   **Selection/layout storage done**: `IconScrollerSelection.cpp` recovers `0x52f680`,
   `0x52f7c0`, and list `0x43be60`; FEng focus flag helper `0x51cc60` is stdcall (12-byte
   cleanup). Selection marks all icon objects before the disabled/same-item gates. Direct
   selection stores a 1-based index, indexed selection stores its zero-based input unchanged.
   Native selection now calls recovered SelectIcon. The complete scroller extent is 0xe8:
   ScrollBar +0x38, tween +0x90, alignment +0xc0, index +0xc8, window +0xcc, master +0xd4/+0xd8,
   running offset +0xe0 and fade +0xe4. Native ownership/vector state lives separately.
   The mapped manager pointer at 0x8384c4 is bound during native pause loading.
   `verify_pc_icon_scroller_selection.py`: 12,063 comparisons; four rejected mutations,
   package verifier now 12,000; native build and all 19 settings smoke scenarios pass.
   **Coordinate/state helpers done**: `FEngCoordinates.cpp` recovers `0x51d070`, `0x51d380`,
   `0x51d4d0`, `0x50cb00`, `0x5178d0`, `0x504500`, `0x5046f0`, `0x50bec0`, `0x50bf40`,
   `0x50c020`/`0x50c050` and glyph search `0x6f45b0`. Native image positioning/size and rendered
   text height call recovered code. Native strings retain text +0x64/justify +0x6c/leading +0x70;
   native scripts expose 0x38-byte track baselines. The state writer updates both first matching
   track per script and live state; it is not a deferred-only write. Selector byte +7 comes from
   cumulative FI value sizes or To; FI payload byte 7 is not the selector (loader `0x5515e0`).
   The full native package/script loader is still a seam. Font lookup `0x51bd70` and low-level
   line width `0x50bce0` remain controlled/native boundaries. `verify_pc_feng_coordinates.py`:
   24,024 comparisons under 53-/64-bit x87, five rejected mutations; byte matching unestablished.
   Native build, pause/options/HUD verifiers, 19 settings and 10 Controls smoke scenarios pass.
   Layout recovery must use top-left `0x51d070`/`0x51d380` versus centre `0x51d4d0` correctly.
   Alignment 1 in `0x52f8c0` appears centre-aligned; verify before replacing the old native
   bottom-alignment comment. `0x537a10` is a type-2 string lookup, not the generic native seam.
   **Placement/tween done**: `IconScrollerLayout.cpp` recovers `0x5748c0`/`0x574910`/`0x574e30`,
   `0x50d8d0`, `0x50d9c0`, `0x51fd60`, `0x52f8c0`, `0x538a30` and `0x538940`. Native direct
   layout is removed; reset and frame tick use original placement and cubic motion. Alignment 1
   is centre alignment (old native bottom-alignment comment was wrong). Positioning precedes
   tween advancement; reset selects only after placement. `ScrollerTween` is explicit at +0x90.
   Native frame delta binds `0x865198`; scroller relayout clock is `0x8641a0`. End-slope preserves
   inv-duration multiplication, and acceleration preserves original float spills at 0x574925/942.
   `verify_pc_icon_scroller_layout.py`: 14,762 comparisons, five rejected mutations; native
   19 settings, 10 Controls and regression verifiers pass. Motion is checked by
   `smoke_native_pause_scroller.py` (869 -> 758.5 -> 640 px). Finite scripted UI captures
   ignore external UI events/repeat to prevent desktop pointer interference; explicit script
   pointer/gamepad actions and normal interactive input remain supported.
   **Colour/lock policy done**: `FEngColour.cpp` and `IconScrollerScripts.cpp` recover
   `0x5054d0`, `0x50ccc0`, `0x495570`, `0x504640`, `0x517ad0`, `0x51d7f0`, `0x51d040`
   and `0x51df00`. Signed clamps and wrapping ±1 tolerance, first matching track per script,
   live state writes, channel alias/load/store order and inclusive script expiry are verified.
   `verify_pc_feng_colour_scripts.py`: 29,200 comparisons, five rejected mutations.
   Native colours delegate to recovered setter; LOCK/BUTTON_MASTER bind retail objects.
   Layout chain's 14,762 comparisons now execute real recovered colours and lock policy.
   Formatted hash `0x50b9c0`, script activation `0x51cf70`, scrollbar `0x538080` and
   arrows were subsequently recovered below. Native build, pause/options verifiers, 19 settings,
   10 Controls and rendered motion pass. Byte matching remains unestablished.
   **Arrows done**: `0x52fa70`, `0x527660`, `0x495810` preserve Static/Blink requests,
   helper call boundaries, inclusive expiry and first/last visibility early returns.
   ScrollBar exposes arrows +0x48/+0x4c and native construction binds retail
   ScrollBar_Arrow_1/2. Expanded colour/scripts verifier passes 39,700 comparisons
   (10,500 new), with five rejected arrow mutations; layout's 14,762 comparisons now
   execute actual recovered arrow code. Native build, 19 settings, 10 Controls and
   rendered motion pass. Scrollbar update and geometry/constructor remain seams.
   **Scrollbar geometry done**: `ScrollerScrollBar.cpp` recovers `0x538080`, `0x52e200`,
   `0x52e320`; coordinate helpers add `0x51d620`, `0x51d730`, `0x495590`, `0x525db0`.
   Preserve scaled step's extended reuse versus fixed step's rounded reuse, top-left versus
   centre placement, integer-page denominator in fixed mode and visibility order/hidden-branch
   preservation. `verify_pc_scroller_scrollbar.py`: 15,641 comparisons under 53-/64-bit x87,
   five rejected mutations (a forced volatile step spill is needed; a plain field reread is
   optimized back into x87 reuse). Coordinate verifier expands to 29,970; layout remains
   14,762 with actual scrollbar code. Native binds all six retail scrollbar objects and reads
   geometry via recovered getters. Full construction remains a seam. Native build, pause/options,
   19 settings, 10 Controls and rendered motion pass; byte matching remains unestablished.
   **Scrollbar constructor done**: `0x542ac0` and named image wrapper `0x4953e0`
   replace native setup. Preserve untouched +0x34/+0x3c fields, null-prefix lookup skip,
   six suffixes/order, noncanonical bytes and original opposite-axis initial scaled extent.
   Scrollbar verifier expands to 19,641 (2,000 constructors + 2,000 wrappers); five
   constructor/wrapper mutations rejected. Format `0x50c960`, hash `0x505450` and typed
   image lookup `0x537a60` remain controlled/native boundaries; type 1 is enforced natively,
   package/manager lookup and legacy truncation remain unverified. Native build, layout,
   pause/options and 19 settings/10 Controls/motion checks pass. Full scroller construction
   remains a seam. Default empty scrollbar ctor only supports temporary native storage.
   **Next**: recover the rest of IconScroller (vtable `0x79c0f4`, base `0x795cbc`, 0xe8 bytes at
   IconScrollerFNGObject `+0x4c`) replacing `NativeIconScroller`:
   ctors `0x543840`/`0x543a90`, slots `0x543bf0`,
   `0x538b80`, `0x51f870`; helpers `0x51fb70`,
   `0x538130`, `0x52e4e0`, `0x52e670`, `0x51fd10`, `0x51fd60`,
   list helper `0x43be80`; script activation `0x51cf70`/`0x517720`.
   Layout notes: `+4/+8` icon list sentinel (icon node at icon `+4`), `+0xc` current icon,
   `+0x10/+0x14/+0x18` OPTION/LOCK/BUTTON_MASTER objects, `+0x1c` package, `+0x20` "option_"
   prefix, `+0x24` spacing (40), `+0x28` next option number, `+0x2c` vertical, `+0x2e` relayout,
   `+0x30` allow disabled, `+0x34` last direction, `+0x38` ScrollBar (0x58), `+0x90` tween
   (0x30), `+0xc0` alignment, `+0xc8` selected index, `+0xcc` window width (480),
   `+0xd4/+0xd8` master position, `+0xe0` running offset, `+0xe4` fade flag.
   Icon (0x48, ctor `0x51f670`): `+0xc` texture, `+0x10` object, `+0x14` offset, `+0x1c`
   colour, `+0x20` disabled, `+0x24/+0x28` size, `+0x2c/+0x30` title/description,
   `+0x34..+0x44` fade state, `+0x45` reacts.
   **Input/deferred helpers done (2026-10-18)**: `IconScrollerInput.cpp` recovers
   storage `0x50da20`, stop/start `0x52fbf0`/`0x52fba0`; `FEngDiagnostic.cpp`
   recovers the retail no-op sink `0x50d510`. Native booleans are removed in favor
   of scroller +0x2f and owner +0x14c; package mask writes now follow the original.
   `verify_pc_icon_scroller_input.py --compiler clang-cl --mutation-checks`
   passes 10,800 comparisons and rejects five mutations. Native build, 19 settings
   and 10 Controls scenarios pass. MSVC 7.1/wibo is absent from this checkout;
   legacy-compiler validation and byte matching remain unestablished. Lookup
   `0x52cef0`, construction and message dispatch remain native seams.
   **Icon construction/insertion done (2026-10-18)**: `IconScrollerInsertion.cpp`
   recovers `0x51f670`, `0x543c10` and owner wrapper `0x50dab0`. Original partial
   initialization replaces native zeroing; insertion owns numbering, dimensions,
   colour writes, running offset, list links and initial focus. Native storage
   now retains only ownership/hit-test entries and the selected-index adapter.
   `verify_pc_icon_scroller_insertion.py --compiler clang-cl --mutation-checks`
   passes 6,000 comparisons under 53-/64-bit x87 and rejects six mutations.
   UI helper/deletion interfaces are controlled in that proof; native build,
   19 settings, 10 Controls and rendered scroller motion pass. MSVC and byte
   matching remain outstanding. Full scroller construction/destruction and
   owner dispatch remain seams. Native GUI smoke scripts now default to SDL
   offscreen software rendering, with `--headed` available; see NATIVE_DEMO.md.
   **Bound labels done (2026-10-18)**: `IconScrollerLabels.cpp` recovers
   `0x50da50` and named string lookup `0x4953c0`, executing the already recovered
   `cihash` at `0x505450`. Owner storage separates scroller +0x4c..+0x133 from
   title +0x134 and description +0x138. Native construction binds both original
   names (`ICON_TITLE` and empty), and no longer looks up just the title on every
   selection change. The label setter can change selection; the second read is
   retained. `verify_pc_icon_scroller_labels.py --compiler clang-cl --mutation-checks`
   passes 6,000 comparisons and rejects five mutations. Typed lookup and localized
   label storage remain controlled interfaces; full constructor recovery, MSVC
   verification and byte matching remain outstanding. Native build, 19 headless
   settings, 10 Controls and rendered motion pass; input/insertion regression
   verifiers and their mutations also pass. Next small target: setter `0x50c900`
   (hash +0x60, flags +0x1c, virtual observer through `*0x838478`), still native.
   **Language setter done (2026-10-18)**: `FEngText.cpp` recovers `0x50c900`,
   including unchanged-hash notification, incoming flag-bit snapshot and
   post-callback dirty/bit-2 handling. The shared object prefix exposes string
   language hash +0x60 as a union with group child count. Native loading preserves
   initial SH hashes; a native first-slot observer retains language lookup and
   UTF-16 ownership. The GPS caller's misleading native `HudObjectTexture`
   adapter is removed in favor of the recovered setter. `verify_pc_feng_text.py
   --compiler clang-cl --mutation-checks` passes 8,192 setter/composed-label
   comparisons and rejects six mutations. Freeroam HUD now supports explicit
   Clang fallback and passes 15,180 comparisons; that verifier still controls
   the setter. Native/mod builds pass. Original observer internals, localization,
   asset loading, MSVC validation and byte matching remain unestablished.
   Headless 19 settings/10 Controls, scroller motion and HUD aspect/skin captures
   pass with the new observer. Next bounded candidates: typed object lookups
   `0x537a10`/`0x537a60` and their `0x517f80`/`0x52c570` search dependencies,
   then script activation `0x51cf70`/`0x517720`.
4. Recover IconScrollerFNGObject: ctors `0x551de0`/`0x552330`, dtor `0x4defb0`, message handler
   `0x543d40` (sounds via table `0x837fe0` -> `0x458520`),
   `0x538c00`/
   `0x538ca0` (dim scripts on object `0x47ff4e7c`).
5. Then the options scroller and option items in `NativePauseOptions.cpp`, the in-game dialog
   `0x558790`, and game-flow queue `0x5f41e0`/`0x5f4240`.

Decompilations of the scroller set were saved under the session scratchpad (`scroller/`,
`feng/`); regenerate with ghidra-rpc if missing. Recreate missing Ghidra functions with
`create-function` first (several of these addresses were undefined).

## Other open work

- Native respawn (DemoMain airborne reset) is the remaining invented gameplay logic. The
  original reset `0x6009a0` (via fall check `0x5ecba0`) needs road navigation: `WRoadNav`
  (ctor `0x5da2f0` recovered; advance `0x5da370` -> `0x5d5c00`/`0x5d9e30`/`0x5da240`, spline
  helpers `0x570640`/`0x577710`) over the road network in `TRACKS/L4RA.BUN`: chunk `0x34107`
  (loader `0x5df8a0`, also `0x80034100` sub-chunks, table at `0x883df0`/`0x883df4`), `0x34108`
  (`0x5db2e0`, list `0x88f760`), `0x34110..0x34113` (`0x5def30` -> grid `0x88eb00` `0x5da7a0`).
  Loaders are registered by the static initializers at `0x77e7d0..0x77e8d0`.
- HUD caller `0x600590` (blocked on rear-view mirror view `0x5f1e10`), participant setup
  `0x6065b0`, car constructor `0x604260`, world update `0x609ce0`, asset chunk loaders.
- Earlier physics items (tuning-table loader, CarTypeInfo binding) remain as listed in
  `docs/demo-substitute-audit.md`.

## Evidence, tools and useful commands

Start with `docs/README.md`, `docs/demo-substitute-audit.md`,
`docs/pc-physics-parameters.md`, `docs/pc-human-steering.md`, and
`docs/NATIVE_DEMO.md`. Manifests live in `config/SPEED2_EXE_1_2/`.

Reference: local `Need for Speed Underground 2/speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
Retail source fixtures come from local `GLOBAL/GLOBALB.BUN`, not committed data.

Ghidra skill: `/home/dq/.codex/skills/ghidra-rpc` (`uv run ghidra-rpc ...` from that directory).
Project: `build/ghidra/ug2.gpr`, program `speed2.exe`. Check current daemon
status before relying on prior state. Local daemon access may need escalation.
Ignored `build/analysis/demo-recovery/` contains decompilation JSON and disassembly
for the draft tuning helpers; `build/analysis/function-index.json` has boundaries
and fingerprints. Ghidra's x87 types/signatures can be misleading: disassembly
and verified execution determine arithmetic and calling convention.

```sh
cmake --build build/native -j4
build/venv/bin/python tools/verify_pc_physics_extra_tuning.py
build/venv/bin/python tools/verify_pc_physics_construction.py
build/venv/bin/python tools/verify_pc_physics_parameters.py
build/venv/bin/python tools/verify_pc_hud_player_masks.py
build/venv/bin/python tools/verify_pc_hud_freeroam.py
build/venv/bin/python tools/verify_pc_scroller_scrollbar.py
build/venv/bin/python tools/verify_pc_feng_colour_scripts.py
build/venv/bin/python tools/verify_pc_icon_scroller_layout.py
build/venv/bin/python tools/verify_pc_feng_coordinates.py
build/venv/bin/python tools/verify_pc_icon_scroller_selection.py
build/venv/bin/python tools/verify_pc_icon_scroller_navigation.py
build/venv/bin/python tools/verify_pc_feng_package.py
build/venv/bin/python tools/verify_pc_pause_menu.py
build/venv/bin/python tools/verify_pc_pause_options.py
build/venv/bin/python tools/verify_pc_track_markers.py
build/venv/bin/python tools/verify_pc_small_queries.py
build/venv/bin/python tools/verify_pc_replay_events.py
python tools/smoke_native_pause_options.py
python tools/smoke_native_controls.py
python tools/smoke_native_pause_scroller.py
build/venv/bin/python tools/verify_pc_physics_record_upgrades.py --native-stock build/native/ug2recordprobe
python tools/smoke_native_driving.py
```

Run the verifier commands sequentially because MSVC/wibo shares temporary
filenames. Local tools are `build/tools/wibo` and
`build/compilers/Win32/7.1/cl.exe`. Native32 execution requires escalation in this
sandbox; a sandbox exit 159 does not establish a native program failure.
Git mutations/push also require escalation because `.git` is sandbox read-only.
No analysis jobs are left running at this checkpoint.

24. **Object-tree and typed package lookup checkpoint (2026-10-18).**
    `FEngLookup.cpp` recovers visitor walks `0x504830`/`0x504880`, query
    `0x5048d0`, package object search `0x517f80`, signed ASCII name comparison
    `0x505310`, package-list search `0x519e50`, queue fallback `0x52c570`, and
    wrappers `0x5379c0`/`0x537a10`/`0x537a60`. This supersedes the lookup gaps
    listed above. Root/child links now feed recovered traversal; native screen
    membership/order and package locator `0x52cef0` remain seams. The legacy
    generic `FngFindObject` is still a compatibility adapter, not `0x537a10`.
    Clang i686 verification passes 11,264 comparisons with actual composed
    helpers and rejects six mutations. Package regression passes 12,000;
    native build, all 19 settings, all 10 Controls and motion pixel checks pass
    headlessly. Normal-return SEH-chain restoration is verified; exception
    unwinding, MSVC 7.1 and byte matching remain outstanding. See full evidence
    in `pc-pause-menu.md`. Next bounded candidates: package locator `0x52cef0`
    and archive search `0x51bb70`, then script activation `0x51cf70`/`0x517720`.

25. **Package locator checkpoint (2026-10-18).** `FEngLocator.cpp` recovers
    `0x52cef0` (106 bytes): mode `0x8384d0` plus manager availability chooses
    live lists +0xdc/+0xec or archive node search. Live misses and a null live
    queue do not fall through to the archive. The native locator adapter is
    removed; native startup explicitly selects mode zero. Archive search and
    list membership/loading remain native seams. The extended lookup verifier
    now executes the actual locator in typed-wrapper compositions and controls
    only archive `FindNode` (plus direct-test visitor callbacks). It passes
    12,352 comparisons and rejects ten mutations; package regression passes
    12,000. Native/mod-enabled builds and headless 19 settings, 10 Controls and
    motion checks pass. See `pc-pause-menu.md` for validation and compiler limits. Next: archive search
    `0x51bb70`, whose hash match moves the node to the front of its intrusive
    list, then script activation `0x51cf70`/`0x517720`.

User supplied `https://github.com/berkayylmao/NFSPluginSDK` during the locator
checkpoint. Reviewed revision `d238bdffc648498840d17133edb256fa0310d355`;
see `nfspluginsdk-reference.md` for pinned ProStreet FEng/script/string leads
and the MW05 virtual-layout difference. No source imported or UG2 fields
renamed from cross-title evidence alone.

26. **Archive search checkpoint (2026-10-18).** `FEngArchive.cpp` recovers
    `0x51bb70` (206 bytes): nullable length/hash, backward backslash scan,
    two record formats, first hash hit and ordered unlink/prepend. Leading
    backslash and zero-candidate behavior are preserved. Native `FindNode` is
    removed; native shown-screen membership and hash-record construction remain
    seams. Standalone Clang i686 verification passes 8,192 comparisons and
    rejects six mutations, including exact link-write order and repeated calls.
    The lookup verifier's `--compose-archive` mode executes the real archive
    dependency and passes 13,376 comparisons, rejecting the ten existing
    lookup/locator mutations. Native/mod builds and headless all 19 settings,
    all 10 Controls, slider pixels and motion checks pass. Legacy compiler/byte matching,
    arbitrary malformed lists and archive lifecycle remain unverified. Next:
    script activation `0x51cf70`/`0x517720`; tracing shows name hashing on each
    search iteration and script-current-pointer reload after activation.

27. **Script selection/activation checkpoint (2026-10-18).** `FEngScript.cpp`
    recovers name selection `0x51cf70`, pointer assignment `0x5178b0`, track
    initialization `0x517720`, reference resolver `0x4fe890`, quaternion helper
    `0x4fe6b0` and integer helper `0x505580`. Byte restart signature replaces
    provisional bool. Actual helper composition passes 14,336 Clang i686
    comparisons at x87 53/64-bit precision and rejects nine mutations. Aliased
    current-script reload and helper output aliasing are checked; special float
    values, arbitrary aliasing and byte matching remain unestablished.
    Native name adapter/hash map removed; native loader owns delta-key prefixes
    per reader `0x5515e0` and event cursors follow selected headers. Hash/package
    wrappers, script-running predicates, allocation and playback remain seams.
    Native/mod builds and final headless all 19 settings, all 10 Controls,
    slider pixels and scroller motion pass after event-observer integration.
    Next candidates: hash selector `0x51cfe0`, wrappers `0x537c00`/`0x537c60`, predicates `0x537cc0`/
    `0x537d40` and original playback/update dependencies. Do not confuse pointer
    setter `0x5178b0` with the native hash-selection adapter.

28. **Hash selection and package script wrappers (2026-10-18).** Hash selector
    `0x51cfe0`, setters `0x537c00`/`0x537c60` and predicates `0x537cc0`/
    `0x537d40` now use recovered code. Keep low-byte restart, null selector calls,
    formatter-after-lookup order, current-header reload after formatting, and
    signed inclusive time checks. Native hash loops/event-list predicates and
    the unused frontend script adapter are removed; HUD/Controls name calls
    also use the recovered selector. Selector suite passes 16,384 Clang i686
    comparisons at 53/64-bit precision and rejects ten mutations. Full
    `--compose-scripts` lookup passes 17,472 comparisons; only formatted hashing
    and direct visitor callbacks are controlled; seventeen mutations are rejected.
    Native/mod builds, all headless 19 settings/10 Controls, motion and HUD
    captures (four ratios, eleven skins, 120 frames) pass. See `pc-pause-menu.md` for
    coverage and remaining compiler/byte-match limits. Next bounded leads:
    formatted hash `0x50b9c0` and its profiled formatting boundary `0x43fbc0`,
    then command execution `0x52c7c0` and original playback/update dependencies.

29. **GUI formatting/profile checkpoint (2026-10-18).** `FEngFormatting.cpp`
    recovers `0x50b9c0` inline formatted hash and `0x50c960` capacity-ignoring
    variadic forwarding. Engine `Libs/bWare/Formatting.cpp` recovers profile
    wrapper `0x43fbc0` and elapsed accounting `0x43e5c0`, including low-byte
    saturating signed shift, wrapped/clamped low-word delta, signed scale factor,
    callback-visible total and wrapping count. Native old wrappers removed;
    `NativeFormatting.cpp` supplies SDL clock/calibration and host memory-sink
    formatting at the unrecovered `0x43e950` core boundary. Standalone Clang
    verification passes 8,192 comparisons at 53/64-bit x87 precision and rejects
    ten mutations; full `--compose-formatting` lookup/script composition passes
    17,472 and rejects seventeen existing mutations. Compiler-runtime shift
    helper `0x75c600` is reference-executed where needed, not engine recovery.
    Native/mod builds and all headless 19 settings/10 Controls, slider pixels
    and scroller motion pass. Original core formatting, profiling initialization/units, special/failure
    behavior, legacy compiler and byte matching remain unresolved. Next leads:
    formatter state machine `0x43e950` and sink helper `0x43cb40`, then native
    response execution/playback. See `pc-pause-menu.md` for evidence and limits.

30. **Formatter sink checkpoint (2026-10-18).** `FormattingSink.cpp` recovers
    `0x43cb40` (130 bytes) with entry-mode-dependent count order, forward copying,
    per-byte ring/memory mode reload and cached ring-index publication. 4,096
    Clang i686 comparisons check exact ordered writes and selected aliases;
    six mutations are rejected. Native/mod builds, all headless 19 settings/10
    Controls, slider pixels and motion checks pass. Native host formatting now emits temporary
    output through the recovered helper; parser/allocation and conversion/output
    interleaving remain seams. Full core `0x43e950` spans through `0x43f5d9`.
    `probe_pc_format_core.py` records 70 original-only cases (35 at each x87
    precision), controlling only pointer validation `0x43bd60`. The probe observes
    `%b`, `%z`, default three-place floats and three-digit scientific
    exponents; static tracing identifies `%v2`/`%v3` separately. See evidence
    for the `%.0z` edge case and packed table `0x786cf0`.
    Do not treat the probe as recovered-source verification. Pointer validator's
    IsBadReadPtr/IsBadWritePtr semantics and full state machine remain next.


31. **Formatter core checkpoint (2026-10-18).** `FormattingCore.cpp` recovers
    core `0x43e950`, validator `0x43bd60` and recursive `bSFormat` `0x4400d0`.
    Native host parser/allocation removed; packed table, locale and string data
    still bind the reference image. Linux permission probes and SDL clock are
    documented platform adapters. Clang i686 passes 6,534 comparisons across
    memory/count-only/ring at both x87 precision settings, including recursive
    vectors, fixed-point, special floats and selected `%n` aliases; eight
    mutations rejected. Only Win32 pointer probes and clock imports are
    controlled; CRT conversion/shift helpers reference-execute where emitted.
    Unsupported conversion-class paths retain unset/prior size and are excluded;
    arbitrary overflow/aliasing, FP flags, MSVC and byte matching remain open.
    See `pc-pause-menu.md` for exact evidence, commands and platform seams.
    Native/mod builds and headless 19 settings/10 Controls, slider pixels,
    scroller motion and mod HUD four-ratio/11-skin/120-frame checks pass.
    Next GUI targets remain response execution `0x52c7c0`, playback/update and
    lifecycle dependencies; do not infer them from the now-recovered formatter.
    Bounded next lead: original conditional skip helper `0x504490` spans 105
    bytes through `0x5044f8`, takes response layout count +0x10/commands +0x14,
    and returns with `ret 4`. Native `SkipBlock` currently uses `index < count-1`
    where the original loop tests `index < count` after reading/incrementing.
    Preserve the reference boundary (including its possible extra record read)
    when recovering it; source/decompile resemblance is not verification.


32. **Response conditional helper checkpoint (2026-10-18).**
    `FEngResponse.cpp` recovers `FEResponse::SkipConditionalBlock` `0x504490`
    (105 bytes), with response +0x10 count/+0x14 pointer and 12-byte command
    layout. Native `SkipBlock` removed; native dispatch adapts its decoded vector
    to the observed prefix and calls recovered code. Original count-boundary
    extra read preserved, including mapped malformed/count-zero cases in tests.
    Clang i686 passes 8,192 comparisons of ordered record reads, memory, return
    index and ABI, with no controlled callees; six mutations rejected.
    MSVC/byte matching and arbitrary pointer/index/depth overflow are unverified.
    Native/mod builds and headless 19 settings/10 Controls, slider pixels and
    scroller motion/settled centring pass.
    Full response dispatch `0x52c7c0` remains next: queue thiscall, response,
    object, package and input-mask parameters. Assembly/decompilation records
    remain local under `/tmp/ug2-response-dispatch-asm.json` and
    `/tmp/ug2-command-execution.json`. Candidate dependencies include target
    resolver `0x517fe0`, script lookup `0x5046b0`, focus persistence
    `0x50b300`/`0x50b360`, package stack operations and existing message queue.
    Further traced leads (not recovered/verified here): resolver `0x517fe0`
    uses visitor vtable `0x79aafc`, slot +4 -> `0x5048f0`, comparing object +0xc
    against query +4 and storing first match at query +8 via recovered tree
    traversal `0x504880`. Native `FindGuid` currently scans its flat vector.
    Script lookup `0x5046b0` scans object +0x4c via script +4 against hash +0x30.
    Persistence store/load `0x50b300`/`0x50b360` use 32 pairs at queue +0x5150
    and signed insertion index +0x514c; do not replace signed modulo with a mask
    without verifying negative cases. These are candidates for full-dispatch
    composition, not evidence of recovered native behavior.


33. **GUID resolver checkpoint (2026-10-18).** `FEngLookup.cpp` recovers
    `0x5048f0` GUID visitor and `0x517fe0` package wrapper; query +4 GUID/+8
    result, object GUID +0xc. Actual recovered tree traversal stops at first
    match. Both native `FengPackage::FindGuid` and `Find` flat-vector loops are
    removed in favor of recovered GUID/hash wrappers over a native package
    prefix and decoded root chain. Loader linking and prefix construction remain
    seams. Composed Clang suite passes 19,520 comparisons, including 1,024 direct
    GUID and 1,024 full GUID calls, and rejects 21 mutations. The broader suite
    still controls formatter core/clock and direct visitor callbacks; full GUID
    composition has no controlled callees. Normal-return FS restoration checked;
    exceptions/unwind, malformed/cyclic trees, MSVC and byte matching unverified.
    Native/mod builds and headless 19 settings/10 Controls, slider pixels,
    scroller motion and four-ratio/11-skin/120-frame mod HUD checks pass.
    Next dependency candidates remain `0x5046b0` script lookup, queue focus
    persistence `0x50b300`/`0x50b360`, package-stack reset `0x50b390`, then full
    response dispatch `0x52c7c0`. Reset tracing now shows exactly 16 byte clears
    at queue +0x414c with stride 0x100, followed by a 32-bit zero at +0x4148.
    This stack is separate from focus insertion index +0x514c and 32 key/value
    pairs at +0x5150. Keep these observed structures distinct. The reset has
    not yet been recovered or behaviorally verified.


34. **Full response-dispatch checkpoint (2026-10-18).** `FEngResponse.cpp`
    recovers `0x52c7c0` all command families, preserving cached count, per-loop
    command-pointer reload, local script reset and original callee boundaries.
    `FEngQueueState.cpp` recovers `0x5046b0`, `0x50b300`, `0x50b360`, `0x50b390`;
    queue observed prefix now reaches the 32-pair focus cache at +0x5150.
    Clang i686: 12,800 comparisons, 16 mutations; skip helper 8,192/six mutations.
    Dispatcher executes actual skip/state helpers, controls nine external
    boundaries; checks ordered writes/calls, negative-index aliases and ABI.
    Native manual switch removed. Loader now selects/initializes Init script
    using observed `0x5515e0`/`0x5554a0` setup; missing selection caused a native
    conditional crash, now addressed at the loader seam. Original readers and
    Init event timing remain unrecovered. Native delivery/mask (fixed 0xff)
    remain adapters. Target -4 and package methods `0x51af50`, `0x51ab20`,
    `0x51ac10`, `0x51adb0`, `0x51b070` explicitly trap; do not claim complete
    dispatch integration. MSVC, byte matching, all aliases/malformed inputs and
    full callee composition remain outstanding. Native/mod builds pass; after
    Init selection repair, headless 19 settings/10 Controls, slider pixels,
    scroller motion and four-ratio/11-skin/120-frame mod HUD checks pass.
    Next targets: queue Post
    `0x51aa20`/delivery `0x541fc0`, input and package-operation callees.


35. **Queue node checkpoint (2026-10-18).** `FEngQueueNodes.cpp` recovers
    Post `0x51aa20`, input enqueue `0x51af50` and Remove `0x505400`. Header
    exposes 32-byte node/list ABI, queue lists +0x4114/+0x4124, observer +0x10c
    and byte +0x5251. Exact node/link write and observer ordering verified:
    12,288 Clang comparisons, eleven mutations. Actual queue composition in
    dispatcher passes 12,800 comparisons/sixteen mutations, controlling allocation
    and remaining callees; observer disabled only in composition, separately
    tested in node suite. Native message vector/Post substitute removed; all
    producers use recovered enqueue, Pump/cancellation use recovered unlink.
    Native malloc/free and delivery/cancellation/256-message guard remain seams;
    original node vptrs are tags, virtual destruction not invoked natively.
    Input enqueue no longer traps, but its unrecovered consumer does. Target -4
    delivery and four remaining package operations still trap. Full delivery
    `0x541fc0` decompile is local `/tmp/ug2-feng-delivery.json`, with node observer
    slot +0x58, target routing, package +0x5c responses/+0x6c/+0x70 associations
    and object delivery `0x5367e0`. Next trace input-operation consumer via
    vtable `0x79ab64`, then delivery/destruction and original mask propagation.
    Allocation failure, arbitrary aliases/pointers/cycles, MSVC and byte matching
    remain unverified. Do not claim full queue or GUI recovery.
    Native/mod builds and headless 19 settings/10 Controls, slider pixels and
    scroller motion/settled centring pass. Vtable slots now identify message
    deleting destructor `0x51aaf0` and operation destructor `0x51abe0`; bodies
    remain unrecovered. Frame leads `0x5663b0` -> `0x5662c0` -> `0x558c00`
    are saved under `/tmp/ug2-feng-frame*.json` for consumer tracing.


36. **Node ownership checkpoint (2026-10-18).** Recovered deleting wrappers
    `0x51aaf0`/`0x51abe0`, message base `0x51ab10`, operation thunk `0x51ac00`
    and owned-payload base `0x505350` in `FEngQueueNodes.cpp`. Preserve helper
    boundaries, intermediate/final vptrs, payload-only free and deleting flag
    bit zero. Deallocation boundary `0x575630` is platform free. Expanded node
    suite: 32,768 Clang comparisons/eight routines,17 mutations; normal-return
    FS restoration checked, exception unwind/allocator internals unverified.
    Native Pump/cancellation now call recovered DestroyMessage(1), not raw free;
    dynamic vtable dispatch still bypassed by the native known-message bridge.
    Original operation consumer is `0x5559c0`, reached from frame queue manager
    `0x558c00`. Full local decompile `/tmp/ug2-feng-operation-consumer.json`:
    inline unlink, flags1/2/4/8/0x10, package+0x3c parent/input-mask transfers,
    target-4 notifications, package load/remove and virtual deletion. Preserve
    all branches and inlined unlink boundaries when recovering it. Caller frame
    drains messages, consumes operations, then drains any new messages again.
    Input consumer/target-4 still trap natively. MSVC and byte matching pending.
    Native/mod builds and headless 19 settings/10 Controls, slider pixels and
    scroller motion pass; 12,800 queue-composed dispatcher comparisons also pass.
    Local Ghidra listing issue report saved at
    `/tmp/ghidra-rpc-issues-20261018-queue-ownership.md`; requested newly-created
    destructor listings skipped entry bytes. Direct PE/Capstone decoding and
    original execution supplied the evidence; no recovery work is blocked.


37. **Full operation consumer checkpoint (2026-10-18).** `FEngOperations.cpp`
    recovers `0x5559c0` all flags1/2/4/8/0x10, not just current input opcodes.
    Prefixes add package level18/parent3C and registry sentinel manager+10.
    Preserve inline unlink, cached signed level, cross-node replacement state,
    byte-wrapped next level, registry matching, selective masks, first child,
    zero-mask notification and callback reloads. Clang original/rebuilt:4,097
    sequence comparisons,18 mutations; tracked original block hits required.
    Actual Post/string/diagnostic/deleting bodies compose; load/remove/allocator
    and selected observer boundaries controlled. Native consumer trap removed,
    original operation vtable slot bound through a native thiscall adapter;
    Pump follows message/operations/message phases. Empty native registry and
    package load/remove traps remain; target -4 message delivery and four other
    package-operation enqueue boundaries still trap. New asset-free native
    `ug2fengqueue_native_test` / CTest `feng_queue_native` verifies mask transfers,
    zero notifications, virtual ABI, deletion and balanced allocation/freeing.
    CTest available with modding disabled; no assets/display needed for this test.
    MSVC/wibo unavailable, byte matching/exception unwind/full lifecycle and
    malformed/alias completeness unverified. Next: full message delivery
    `0x541fc0` and object message dispatch `0x5367e0`, preserving package response
    and association-table layouts; then registration/lifecycle and original masks.
    Native/mod builds pass, asset-free CTest passes, and headless 19 settings/10
    Controls, slider pixels and scroller motion/settled centring pass. No new
    Ghidra RPC problems encountered in this checkpoint. User .gitignore edits
    remain unrelated and must stay unstaged.

38. **Script reader Ti lookups (2026-10-18).** `FEngTrackLookup.cpp` recovers
    `0x504a00` FindTrack, `0x505480` registry find and `0x5147f0` property
    index; `FEngQueue::types4134` replaces the opaque 0x14 bytes. Clang
    verifier: 12,288 comparisons, six mutations rejected. Retail scan: no Ti
    (or Sn/Tb/Ta/Tt/Sf/Sl/SI) tags in 56,804 scripts. Retail install for tools:
    `UG2_DIR="/home/dq/Games/Need for Speed Underground 2"`. Next: complete
    reader `0x5515e0` replacing the native Scrp parse in `NativeFng.cpp`.

39. **Complete script reader (2026-10-18).** `FEngScriptReader.cpp` recovers
    `0x5515e0`; `NativeFng.cpp` calls it per Scrp chunk and derives native
    playback metadata from the original tracks (native parse and header
    binding removed). Verifier: 4,096 streams, 22 mutations (clang-cl).
    Captures byte-identical; CTests, 19 settings, 10 Controls, scroller pass.
    Smoke scripts need `--game "$UG2_DIR"`. Next: native playback/interpolation
    (`ApplyTrack`/`AdvanceScripts`) against original script update, then
    message delivery `0x541fc0` and object dispatch `0x5367e0`.

40. **Track interpolation/evaluators (2026-10-18).** `FEngInterpolation.cpp`
    (11 routines incl. slerp `0x519b00`) and `FEngTrackEvaluate.cpp`
    (`0x52bdc0`, `0x519d40`, `0x5354f0`, `0x541f60`) verified: 45,567 and
    21,278 comparisons, 15 and 16 mutations. Not yet integrated. Next: apply
    `0x541db0`, events `0x534c70`, update `0x54ce60` (+ type ticks `0x504910`,
    `0x517230`, `0x504dc0`), then replace native `AdvanceScripts`/`ApplyTrack`.
    Global mask `0x7f7c8c` is read by the update/apply flag writes.

41. **Original script playback integrated (2026-10-18).** `FEngObjectUpdate.cpp`
    recovers `0x54ce60`, `0x541db0`, `0x534c70`, `0x504910`, `0x504dc0`
    (13,297 comparisons, 21 mutations). `AdvanceScripts(receiver, queue, s)`
    runs them in ≤20 ms steps; native `ApplyTrack` and pause event scheduler
    removed; `Sc` hashes resolved as in `0x5554a0`. HUD uses a discarded local
    queue (seam). Slider fill widths changed to [0,107,214]. Next (user order):
    messaging — frame manager `0x558c00`, delivery/dispatch around `0x541fc0`/
    `0x5367e0`, registering HUD with the queue; then the package/object loader.
