# PC pause menu recovery

Reference: UG2 PC v1.2 `speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
Ghidra decompilation was checked against x86 disassembly and original/rebuilt
execution. This pass covers `UI_Pause.fng` and its option items. The IconScroller base, most of FEng, the in-game dialog and the game-flow states the menu queues
remain unrecovered. Package mask, selection and focus recovery is recorded below.

## Recovered source

`src/Speed/Indep/Src/Frontend/PauseMenu.cpp` (community name `PauseFNGObject`,
vtable `0x797cb0`, 0x158 bytes, base `IconScrollerFNGObject` vtable `0x7974d4`):

| Routine | Address | Notes |
|---|---|---|
| `CreatePauseFNGObject` | `0x4f4790` | `FNGINIT_UI_Pause.fng` factory; `new` + constructor, null on failed allocation |
| Constructor | `0x4eb410` | Zeroes then copies timer `0x8651ac` to `+0x150`, sets control mask, clears `0x83aa04`, runs `Setup`, adds and syncs the help bar if absent |
| Destructor / scalar | `0x4eb510` / `0x4eb4f0` | Removes the help bar if loaded; scalar form is compiler generated |
| `UnknownSlot0C` | `0x49dfc0` | Shared folded override (slot 3): `0x50d1d0(arg1, arg5)` |
| `Setup` | `0x4d9300` | Header/back/select labels, freeroam or race option list by `0x89e7b0`, scroller slots `+0x20`/`+0x04`, virtual `SelectedIconChanged` tail call |
| `AcceptsInput` | `0x4a7bc0` | Timer changed since opening and the player's controller is assigned (`0x571a60`) |
| `OptionsAvailable` | `0x4a7bf0` | Scans the two-entry table `0x8900ac` for the current player |
| `QuitToCrib` / `QuitToCribKeepHelpBar` | `0x4a7c70` / `0x4a7cc0` | Career `0x513b30`, return package `UI_CareerCrib.fng`, flow message 6 |
| `QuitToProfile` | `0x4a7cf0` | Return package from profile `0x502ff0(0)`, flow message 6, career `0x5139c0` in mode 1 |
| `MessageHandler` | `0x4c2170` | Base handler gate, 13 message cases and the `+0x13c` dialog-result dispatch |
| `SetupFreeroam` | `0x4c2900` | Option list when `0x89e7b0 == 0` |
| `SetupRace` | `0x4c2bd0` | Option list otherwise; only the quit item when `0x8500d0 >= 3` |
| Item constructors | `0x4c2640`, `0x4c27f0` | Out-of-line constructors; the other items' constructors are inline in the original |
| Item `React` | `0x4c24c0`–`0x4c2820` | Nine handlers, all acting on message `0x0c407210` |

Option items derive from `IconScrollerIcon` (constructor `0x51f670`, 0x48 bytes)
and are named after their retail vtables. Each vtable is `[0x4aeb70, React]`;
`0x4aeb70` is the folded scalar deleting destructor calling `0x527770`.

| Vtable | React | Effect on select |
|---|---|---|
| `0x7954d0` | `0x4c24c0` | flow message 1, package value reset, help bar removed |
| `0x7954d8` | `0x4c2510` | flow message 2, package value reset, help bar removed |
| `0x7954e0` | `0x4c2560` | `UI_InGameDialog.fng`, result `0x33c42e03` |
| `0x7954e8` | `0x4c25d0` | dialog, result `0xe1a57d51` |
| `0x7954f0` | `0x4c26b0` | `ui_ReplayControl.fng` when replay data is available; stores the package at `0x8363e4` |
| `0x795510` | `0x4c2700` | `UI_PauseOptionsMain.fng` |
| `0x795518` | `0x4c2730` | dialog, result `0x43da9fd0` |
| `0x795520` | `0x4c2780` | dialog, result `0x451e768e` |
| `0x795528` | `0x4c2820` | dialog, result `0x30ed2368`; text hash depends on mode `0x850078`, network `0x5f0120`, `0x850098` and flow state |

The `neg/sbb/and/add` selections are written as ternaries with the summed
retail constants. Function names other than community IDC names
(`PauseFNGObject::MessageHandler`, `IconScrollerFNGObject::*`, `FNGINIT_UI_Pause.fng`)
are descriptive; unidentified fields and globals keep offset names. The
`QuitTo*` names follow the return packages they set; game-flow message meanings
(1, 2, 3, 4, 6, 0xb) are not established.

The native demo compiles this source unchanged (CMake glob `Src/Frontend`); its globals are
bound in `OriginalSymbols.ld` and the unrecovered interfaces are native seams, described in
[native demo: pause menu](NATIVE_DEMO.md#pause-menu-sixth-pass).

## Retail data

`UI_Pause.fng` (`0x32124083`) and `UI_InGameDialog.fng` (`0xaa469286`) are JDLZ-compressed
`0x30210` chunks in `GLOBAL/InGameCommon.bun`. The pause package has eight `option_N` icon
objects, a hidden `OPTION_MASTER` anchor, `ICON_TITLE`, `Header_text`, and the event handler
`0x47ff4e7c`, whose `Forward` script sends `EXIT_COMPLETE` at 320 ms. Labels resolve in
`English.bin`: the header is "Pause", items read Return to Free Run/Explore Mode/World Map, Resume
Race, Restart, Jump to Garage?, Options, Replay and the Quit variants.

## Reproduction and evidence

```sh
cd tools && ../build/venv/bin/python verify_pc_pause_menu.py
```

The verifier compiles the source with MSVC 7.1 `/Ox /Ob0 /Z7 /EHsc`, links the
COFF object with every external bound to its retail address, and runs original
and rebuilt code in Unicorn with a flat GDT and an FS-based TIB so the SEH
frames execute. Interfaces are hooked at the same retail addresses in both
machines, so cdecl/thiscall argument order and callee cleanup are compared
directly. Each comparison checks the interface trace (including the 0x48-byte
item passed to `AddIcon`), the return value, fixture memory, the written globals
`0x83aa04`/`0x8363e4`, stack cleanup, callee-saved registers and the restored
SEH head. Rebuilt vtables are checked slot by slot against the retail tables
(25 slots) and normalised to retail addresses in memory. Execution is guarded to
the listed routines and declared interfaces.

Current result: **11,723 passing comparisons**, recorded with fingerprints in
`build/analysis/pause-menu/verification.json`. Cases cover game modes
0/1/2/4/8/0x200, race types 0/1/7, `0x8500d0` 0/2/3/4, the `0x8635a8`/`0x85ba67`
flags, six player-table layouts, failed allocation, help-bar/script/controller
states, every handled message plus unhandled ones, every `+0x13c` dialog result,
flow states and replay availability. Mutating constants, conditions, item flags,
flow messages or handler targets in the source makes the verifier fail.

The verifier found that `0x571a60` returns an `int` tested as a full register
(a `bool` declaration fails the `0x100` case).

**Byte matching: not established.** The original inlines most item constructors
and the help-bar removal; the rebuilt `/Ob0` objects call them out of line.

### Controlled interfaces (not verified)

FEng (`0x505450`, `0x537a10`, `0x50c900`, `0x50cea0`,
`0x52cf60`, `0x5379a0`, `0x555f10`, `0x537980`, `0x537950`, `0x50d2d0`,
`0x537cc0`, `0x537c00`, `0x55dda0`, `0x558790`, `0x50c8d0`, `0x50d1d0`),
controller `0x571a60`, game flow `0x5f41e0`/`0x5f4240`, help bar
`0x54e6a0`/`0x54e6e0`, career `0x513b30`/`0x5139c0`, profile `0x502ff0`,
network `0x5f0120`, allocation `0x575620`/`0x575630`, IconScroller base
(`0x552330`, `0x4defb0`, `0x543d40`, `0x52fbf0`, `0x50da20`, `0x50da50`,
`0x50dab0`, `0x40d080`), `IconScrollerIcon` `0x51f670`/`0x527770`, and the
scroller object's slots `+0x04`/`+0x20` (fake vtable). The base constructor
stand-in writes the package pointer, `+0x20` and the scroller vtable that the
pause routines read.

## Remaining work

- Pause options bases and backends: `0x4a7d60`, `0x4a7f70`, `0x4a8010`, constructors
  `0x4eb580`/`0x4d93c0`, factories `0x4f47f0`/`0x4eb650`. These screens are now
  recovered and integrated separately; see [settings evidence](pc-pause-options.md).
- IconScroller base, in-game dialog `0x558790`, and the game-flow handling of the
  queued messages.
- Recover the IconScroller base and FEng message/script dispatch that the native demo
  currently supplies as seams (`NativePause.cpp`).

## FEng package masks, selection and focus

`Frontend/FEngPackage.cpp` recovers five routines from Ghidra and checked x86 disassembly:

| Routine | Address | Behavior |
|---|---|---|
| `FngSetControlMask` | `0x537950` | Writes all four masks `+0x1c/+0x20/+0x24/+0x28`; missing package is ignored |
| `FngGetPackageValue10` | `0x50b8a0` | Hashes package name and reads the static FNG record's `+0x10`; zero if absent |
| `FngSetPackageValue10` | `0x50b8c0` | Writes that same record; missing record is ignored |
| `FEPackage::SetFocus` | `0x52b3a0` | Queues old-object lost-focus `0x55d1e635` and new-object gained-focus `0xabc08912`, each first to the object then to target -5, before assigning `+0x8c` |
| `FEngManager::SetFocusByHash` | `0x52ce40` | Looks up archive node, reads package `+0x18`, scans the full focus array, and selects the last matching hash |

The selection field is **not** a live FEPackage field: `0x50b840` finds it in the
static 0x1c-byte FNG records beginning at `0x7f7dc8`. The earlier native seam and
handoff described its ownership incorrectly, although they preserved it across reloads.
`FEngPackage.h` now defines the observed FEPackage prefix through `+0x94`, the
archive node's package pointer and manager's queue pointer at `+8` with offset checks.
Unknown storage stays unnamed beyond its offset.

Native screens own an FEPackage prefix, an archive-node prefix and a separate persisted
record. The native loader builds the focus array from objects with flag `0x10000000`
(as read in original object reader `0x555140`). Icon selection calls the recovered
hash lookup; option widget selection calls recovered `SetFocus` directly. Input routing
reads `focus8C`. Native archive lookup, object loading, record lookup and queue storage
remain explicit seams; target -5 delivery remains unmodelled. This does not establish
recovery of the archive lists, manager lifecycle or original FEng dispatch.

`build/venv/bin/python tools/verify_pc_feng_package.py` compiles with MSVC 7.1 and
executes original/rebuilt code in Unicorn. **10,000 passing comparisons** cover null
package/record/node, null node package, arbitrary input masks and saved values, null/old/new/same
focus, notifications disabled or enabled (including noncanonical byte values), empty arrays,
missing hashes and duplicate hashes. It checks complete fixture memory, queue message/order/
arguments, focus at each queue call, return values where defined, stack cleanup and preserved
registers. Lookup/hash/queue boundaries are controlled interfaces at their original addresses;
their implementations are outside this proof. Fingerprints and results are saved locally in
`build/analysis/feng-package/verification.json`. **Byte matching is not established.**

Mutation checks reject zeroing one mask, changing the gained-focus message, selecting the
first duplicate hash and assigning focus before notifications. Existing pause verification
still passes 11,723 comparisons and 25 vtable slots; options passes 5,730 comparisons over
89 routines and 510 vtable slots. The native build and all 19 settings smoke scenarios,
including rendered slider checks, pass. Native smoke evidence establishes routing and
session integration only, separately from original/rebuilt execution.

## IconScroller navigation and dispatch

`Frontend/IconScrollerNavigation.cpp` recovers eight routines:

| Routine | Address | Behavior |
|---|---|---|
| `AtFirst` / `AtLast` | `0x5276d0` / `0x527700` | Current icon equals first/last; only AL is defined in the original |
| `Next` / `Previous` | `0x527730` / `0x527750` | Choose wrapping or non-wrapping stepping from byte `+0x2c` |
| `StepNoWrap` | `0x51f9f0` | Step toward the boundary, skipping disabled icons unless byte `+0x30` permits them |
| `StepWrap` | `0x51fac0` | Wrap at either boundary, with the same disabled policy |
| `IndexOf` | `0x51f820` | 1-based list position; -1 for null or absent icons |
| `Dispatch` | `0x50d980` | Call current icon's React slot only when sender equals its FEObject `+0x10` |

The observed scroller prefix through `+0x34` is now explicit in `IconScroller4C`:
node links at `+4/+8`, current icon `+0xc`, package `+0x1c`, layout flag `+0x2e`,
disabled-selection byte `+0x30` and last direction `+0x34`. Links point to icon
**+4**, including the sentinel at scroller +4. Both step routines call virtual
slot `+0x14` first, then set the layout flag and direction. The native scroller
now supplies this list and calls recovered stepping, indexing and dispatch;
its selection callback still sets current icon, synchronizes native layout and focus.
Mouse selection synchronizes the same current-icon field. List construction, icon
construction, slot `+0x14` itself, layout, scroll/fade animation and the owning
FNG object's message handler remain native seams.

The original wrapping routine does not terminate for direction ±1 when all icons
are disabled and `+0x30` is zero. No replacement guard is added. Normal native
pause lists include enabled items. Empty/null-current lists are not valid step
inputs; callers retain their existing empty-list checks.

`build/venv/bin/python tools/verify_pc_icon_scroller_navigation.py` compiles with
MSVC 7.1 and runs original/rebuilt routines in Unicorn. **34,670 comparisons**
exhaust all disabled masks and current positions for lists of 1–6 items, allowed
selection bytes 0/1/0x80, directions ±1/0/±10, and both wrapper modes; the known
nonterminating wrap inputs are excluded. Tests also cover empty/null/absent index
queries, null current icon and matching/mismatching dispatch sender, arbitrary
message arguments, and callbacks that either update or preserve current selection.
They compare fixture memory, virtual callback traces and pre-callback layout state,
stack cleanup, callee-saved registers and defined return bits. The selection and React
callbacks are controlled interfaces, not verified implementations. Fingerprints and
counts are stored in ignored `build/analysis/icon-scroller-navigation/verification.json`.
**Byte matching: not established.**

Four mutation checks reject inverted disabled policy, a zeroed last direction, a zero-based
index and inverted dispatch-sender gating. Existing pause (11,723) and options (5,730)
comparisons remain passing. Native compilation and all 19 settings smoke scenarios with
rendered slider checks pass separately as integration evidence.

An additional native run (`--frames 240 --pause-at 20 --pause-keys RRA`) skips the
disabled Replay icon and reaches the Quit dialog with result `0x30ed2368`. Its
screenshot/log stay in ignored `build/analysis/icon-scroller-navigation/`.

## Icon selection and the full scroller layout

`IconScrollerSelection.cpp` now recovers `SelectIcon` (`0x52f680`), indexed selection
(`0x52f7c0`) and intrusive list `NodeAt` (`0x43be60`). `FEngPackage.cpp` also recovers
focus-flag helper `0x51cc60`: only argument byte 1 clears bit `0x4000000`; every
other byte sets it. Disassembly and stack verification establish that this helper
is **stdcall**, including its 12-byte callee cleanup.

Both selection paths mark every icon's matching FEObject before deciding whether
selection can change. Direct selection then checks the disabled byte, assigns current
icon, clears the selected object's flag, queues package focus and stores its **1-based**
list index. Indexed selection is different: it passes a **zero-based** input to `NodeAt`,
rejects the current icon or a disallowed disabled icon, and stores the input unchanged.
No new range checks are added: `NodeAt` returns the sentinel when an index is outside
the list, and original indexed selection can interpret that sentinel as an icon.
These APIs retain their original caller preconditions.

`IconScroller4C` retains the complete observed `0xe8`-byte scroller extent, including
ScrollBar storage `+0x38` (0x58 bytes), tween storage `+0x90` (0x30), alignment `+0xc0`,
selected index `+0xc8`, window width `+0xcc`, anchor position `+0xd4/+0xd8`, running offset
`+0xe0` and fade byte `+0xe4`. This is layout evidence, not recovery of the constructors
or all fields. The native vector, ownership and other backend state moved to separate
storage keyed by the embedded scroller; it no longer occupies these original offsets.
The manager pointer at `0x8384c4` now binds to the native manager when pause assets load.
Native selection calls recovered `SelectIcon`; native layout still follows that call
and remains a seam. Original layout/animation/construction remains outstanding.

`build/venv/bin/python tools/verify_pc_icon_scroller_selection.py` compiles selection,
previously recovered navigation and package focus code together with MSVC 7.1.
**12,063 comparisons** execute original/rebuilt selection plus the actual focus-flag,
hash-focus, focus-queue and list-index routines. Only archive lookup `0x51bb70` and queue
storage `0x51aa20` are controlled interfaces. Cases cover 1–8 icons, null/current/other
selection, disabled policies, absent manager/node/package, empty or duplicate-hash focus
arrays, pre-existing focus, same-icon refusal, zero/negative/out-of-range indices and
empty list indexing. The sentinel fixture includes a valid object so out-of-range
selection is executable without adding a substitute guard. Checks compare complete
fixture memory, lookup/queue traces, defined return bits, stack cleanup and preserved
registers. Fingerprints stay in ignored `build/analysis/icon-scroller-selection/verification.json`.
The package verifier now includes focus flags for **12,000** comparisons. Navigation
still passes **34,670**. **Byte matching remains unestablished.**

Four mutation checks reject changing the stored indexed value, leaving the selected object's
focus flag set, reversing the disabled-selection gate and removing same-icon refusal.
The native build, existing pause/options verifiers and all 19 settings smoke scenarios pass.

## FEng coordinates, text metrics and script state

`Frontend/FEngCoordinates.cpp` recovers these original routines:

| Routine | Address | Behavior |
|---|---|---|
| Glyph binary search | `0x6f45b0` | Sorted ushort keys with variable stride |
| Text width / height | `0x50bec0` / `0x50bf40` | Maximum suffix/line measurements; low-byte glyph lookup and trailing-break policy |
| Horizontal / vertical justification | `0x50c020` / `0x50c050` | Bit priority, -0.5 scale or negation; stdcall |
| Vector equality tolerance | `0x504500` | Ordered inclusive bounds using `0x7992ec`; AL result |
| State offset lookup | `0x5046f0` | Keys 1–10; offset zero otherwise; stdcall |
| Vector state writer | `0x5178d0` | First matching track per script, then live state; set/add and clear the track's high flag bit |
| Top-left getter | `0x51d070` | Image half-size subtraction, string metrics/justification, group no-op |
| Top-left setter | `0x51d380` | Image half-size addition, string alignment subtraction; preserve z and guard type >= 0x100 |
| Centre setter | `0x51d4d0` | Image centre directly, string metric midpoint/alignment subtraction; same dirty/state-write path |
| Size setter | `0x50cb00` | Writes size and dirty bit, including non-image types |

The state writer **always updates live state**, including when scripts exist; it also
updates only the first matching 0x38-byte track of each script. The original state-key
table is bound at `0x7992f4`. Dirty-bit decisions use the original epsilon comparisons,
including ordered NaN behavior, rather than rounded coordinate differences.

Native strings now retain UTF-16 `+0x64`, justification `+0x6c`, leading `+0x70` and
wrap width `+0x74`; font resource hash is also at the original `+0x24`. Script headers
point to track prefixes with baseline `+0x18` and state selector byte `+7`. Loader evidence
from `0x5515e0` establishes that FI selectors advance by value size/4 and To overrides
that selector (it is not FI payload byte 7); Tb/first Kd values supply the inline base.
Ta/Tt update flags/interpolation. The loader remains a platform seam, not a recovered
or original/rebuilt-verified package loader. Native playback synchronizes its baseline
from those prefixes. Icon positioning/size calls the recovered centre and size setters;
rendered text height uses the recovered height routine. Font lookup `0x51bd70` and low-level
line measurement `0x50bce0` remain native boundaries. Their native implementations, full
track dispatch, scroller placement and tweening are outside this proof.

`build/venv/bin/python tools/verify_pc_feng_coordinates.py`: **24,024 comparisons** over
12 routines, with 53-bit and 64-bit x87 precision. The tests execute the actual recovered
state/metric/helper dependency chain, controlling only font lookup and line measurement.
Cases include null object/font/text, image/string/group/default/high object types,
justification combinations, multiline/trailing-break strings, glyph hits/misses and strides,
empty/multiple script lists and repeated matching tracks, set/add bytes, offset keys,
pre-existing flags/z and NaNs/infinities/epsilon edges for vector comparison. Checks cover
fixture memory, defined return bits, x87 float returns, font-call arguments, stack cleanup,
callee-saved registers and unchanged precision. Fingerprints/results remain local in
`build/analysis/feng-coordinates/verification.json`. Five mutations are rejected: reversed
half-size subtraction, updating multiple matching tracks, wrong position offset, changed
justification priority and an extra trailing line. **Byte matching is not established.**

The native build, pause verifier (11,723), options verifier (5,730), freeroam HUD verifier
(15,180), all 19 settings scenarios with slider pixel checks and all 10 Controls scenarios
pass. These smoke checks establish bounded integration separately from differential execution.

## Scroller placement and cubic tween

`Frontend/IconScrollerLayout.cpp` recovers nine routines:

| Routine | Address | Behavior |
|---|---|---|
| End-slope / acceleration limits | `0x5748c0` / `0x574910` | Retain coefficient arithmetic and observed float stores |
| Tween update | `0x574e30` | States 0/1/2, restart mode, cubic coefficients/evaluation and end-state handling |
| Edge fade factor | `0x50d8d0` | Window/edge comparisons and fractional fades |
| Window visibility | `0x50d9c0` | Includes both endpoints; delegates to recovered show/hide |
| Icon alpha | `0x51fd60` | Current RGB and icon alpha; disabled/not-allowed base alpha 150; float-to-int truncation |
| Icon placement | `0x52f8c0` | Full size and centre, then unselected size 0.8 and top-left alignment; optional alpha |
| Layout reset | `0x538a30` | Snap tween, place using old selection, then select and update scrollbar |
| Frame layout | `0x538940` | Relayout clock/target/arrow updates, lock scripts, place all icons, then advance tween |

`ScrollerTween` describes its original 0x30 bytes. It occupies scroller `+0x90`,
with states/mode `+0xbc/+0xbe`. The acceleration limiter stores twice-coefficient and
start acceleration as single precision at original `0x574925`/`0x574942`; preserving
those rounding points resolves a one-ULP mismatch. Subsequent scaled start acceleration
remains extended until the coefficient stores. End-slope calculation retains inverse-duration
multiplication rather than substituting one. No derivative coefficients or algorithms are invented.

Original placement alignment 0/1/2 is top/centre/bottom for smaller image icons. The old
native seam incorrectly described alignment 1 as bottom-aligned. Frame placement uses the
current tween value and updates the tween **after** positioning, so the rendered transition
retains the original frame ordering. Reset performs its selection callback **after** placement.

The native menu now calls recovered reset/frame/placement code; its old direct layout loop
is removed. The platform frame delta is supplied at `0x865198`, and the original scroller
clock is bound at `0x8641a0`. Native construction and icon insertion still supply initial
state, geometry and bindings and remain seams. Colour read/write and lock policy are now
recovered as described below. Scrollbar update and arrow policy are now recovered as described below. This is not complete IconScroller or FNG-base recovery.

`build/venv/bin/python tools/verify_pc_icon_scroller_layout.py`: **14,762 original/rebuilt
comparisons**. Tween cases run with 53-/64-bit x87 precision and cover states/modes, limits,
completion thresholds, zero/negative/infinite/NaN duration and arithmetic order. Placement
cases cover empty/null frame paths, every alignment, first/current/other selection, reset
indices, window edges, disabled alpha policy and fade flags. Actual recovered coordinate,
state-write, visibility, colour and lock-policy routines execute on both sides; selection,
formatted hash and script activation callbacks remain controlled and
traced, including tween state at callbacks. The original list
helper and `0x75bed4` conversion runtime are retained dependencies. Placement fixtures are
image objects, not proof of arbitrary font/group layout. Memory, global clock, callback
order/arguments, cleanup, preserved registers and precision are checked. Five mutations are
rejected: exclusive visibility edge, wrong disabled alpha, wrong alignment, changed cubic
coefficient and moving tween advancement before placement. Fingerprints/results remain
in ignored `build/analysis/icon-scroller-layout/verification.json`. **Byte matching is
not established.**

Native smoke: `python tools/smoke_native_pause_scroller.py` samples the Options graphic
at frames 53/60/75 after Right at frame 50, observing motion toward the fixed selection
frame and settled centring. The green selection frame itself stays fixed. Captures/logs
stay in `build/analysis/native-pause-scroller/`; a settled screenshot is at
`build/analysis/icon-scroller-layout/pause-options-icon.png`. The existing pause/options,
navigation and selection verifiers pass, as do all 19 native settings scenarios.

Finite scripted UI captures now ignore live keyboard/mouse/controller UI events and controller
repeat, while still accepting their explicit scripted device and pointer actions. This platform
fixture change prevents desktop pointer motion from selecting a different Controls row during
keyboard/gamepad smoke sequences; normal interactive input is unchanged.

After isolating finite scripted UI input, all 10 native Controls scenarios and all 19
settings scenarios pass, including explicit mouse/gamepad fixture actions. The rendered
Options graphic centres are 869.0, 758.5 and 640.0 pixels at the three sampled frames.


## FEng colours and icon lock scripts

`Frontend/FEngColour.cpp` and `Frontend/IconScrollerScripts.cpp` recover eight routines:

| Routine | Address | Behaviour |
|---|---|---|
| Pack colour | `0x5054d0` | Clamp signed R/G/B/A channels to 0–255 and pack ARGB |
| Copy object colour | `0x50ccc0` | Read state channels, or zero channels for a null object |
| Get packed colour | `0x495570` | Copy then pack |
| Colour proximity | `0x504640` | Signed comparisons against wrapping ±1 bounds |
| Write colour state | `0x517ad0` | Update first matching track in each script, then live state |
| Set colour | `0x51d7f0` | Gate on type < 0x100; mark dirty outside tolerance; always write |
| Script active | `0x51d040` | Hash even for null object; current hash and time ≤ length |
| Icon lock policy | `0x51df00` | Select lock SHOW/HIDE and button HIDE/FadeIn/Animate policy |

The state writer preserves modulo-32-bit additions and observed channel load/store order,
including aliased input. Matching tracks clear flag 0x80 and stop the search within that script.
The colour setter's tolerance only gates the dirty flag; it does not suppress the state write.
Icon byte +0x22 selects SHOW lock/HIDE button. Otherwise +0x23 enables button FadeIn,
unless the current FadeIn or Animate is still active. Script time equal to length remains active.
Unknown flag bytes retain offset-based storage.

`build/venv/bin/python tools/verify_pc_feng_colour_scripts.py`: **29,200 original/rebuilt
comparisons**, checking fixture memory, call traces, arguments, cleanup, preserved registers
and return values. Cases include signed extrema, clamp/tolerance edges, null objects,
type gates, multiple scripts and repeated matching tracks, additive/aliased writes,
noncanonical flag bytes and script expiry boundaries. Five source mutations are rejected:
wrong clamp, widened tolerance, processing later matching tracks, exclusive expiry and wrong
lock flag byte. Formatted hash `0x50b9c0` and script activation `0x51cf70` are controlled
interfaces; previously verified state-offset `0x5046f0` executes as a retained dependency.
Fingerprints/results remain in ignored `build/analysis/feng-colour-scripts/verification.json`.
**Finite behavioural verification passes; byte matching is not established.**

Native FEng colour writes now use the recovered setter, including script baseline updates.
Native construction binds the retail LOCK and BUTTON_MASTER objects so recovered lock policy
can run. Construction, formatted hashing and script activation/playback remain seams. The layout verifier passes its 14,762 comparisons with actual recovered colour
and lock-policy routines linked in. Native build, pause/options regression verifiers,
19 settings scenarios, 10 Controls scenarios and rendered scroller motion pass.


## Scroller arrow scripts and endpoint visibility

`IconScrollerScripts.cpp` now also recovers `IconScroller4C::UpdateArrows` (`0x52fa70`),
`ScrollerScrollBar::SetArrow2Static` (`0x527660`) and `SetArrow1Blink` (`0x495810`).
The scrollbar exposes its two original arrow pointers at +0x48/+0x4c (scroller +0x80/+0x84);
its remaining unknown fields stay offset-based.

Directions 1/-10 request Static on arrow 1, then Blink on arrow 2. Directions -1/10
call the original arrow-2 Static helper, then arrow-1 Blink helper. Other directions do
not request scripts. Null arrows skip hashing. Matching current scripts are active through
time equal to length, and only absent, mismatched or expired scripts restart. The Static
helper directly hashes/tests state; the Blink helper calls recovered `0x51d040`, preserving
that original call boundary.

After script policy, the routine compares current icon with first, then last. At first it
hides arrow 1 and returns; at last it hides arrow 2 and returns. Those early returns leave
the other arrow's visibility unchanged. At neither endpoint it shows both. Null links map
to null icons; an empty intrusive list uses its sentinel instead. Native construction binds
`ScrollBar_Arrow_1`/`ScrollBar_Arrow_2` by their original constructor names, and the arrow
no-op is removed. Full scrollbar/scroller construction remains unrecovered; geometry was subsequently recovered below.

The expanded `verify_pc_feng_colour_scripts.py` passes **39,700 comparisons**, including
**10,500 new arrow/helper comparisons**. Cases cover all four directional requests and
other directions, null arrows/current scripts, hash mismatches, inclusive expiry, signed
times, first/last/neither/null/sentinel selection and pre-existing visibility flags. Actual
recovered visibility routines execute on both sides; object memory, visibility global,
script/hash traces, arguments, cleanup and preserved registers are compared. Five arrow
mutations are rejected: wrong direction, wrong helper arrow, exclusive expiry, wrong
endpoint visibility, and removing the first endpoint's early return. Formatted hashing
and script activation remain controlled boundaries. **Byte matching is not established.**

The layout verifier's 14,762 comparisons now execute actual recovered arrow code along
with colour/lock/coordinate/visibility code. Native build, all 19 settings scenarios,
10 Controls scenarios and rendered icon motion pass. These finite smoke checks establish
integration for their exercised paths, not complete FEng animation or menu recovery.


## Scrollbar handle geometry and visibility

`Frontend/ScrollerScrollBar.cpp` recovers Update `0x538080`, scaled handle placement
`0x52e200` and fixed handle placement `0x52e320`. `FEngCoordinates.cpp` adds centre getter
`0x51d620`, size getter `0x51d730`, width setter `0x495590` and height setter `0x525db0`.
ScrollBar remains 0x58 bytes: orientation/mode/deferred bytes +0/+1/+2, positions +0xc/+0x10,
origins +0x1c/+0x20, backing extents +0x24/+0x28, handle extents +0x2c/+0x30,
step +0x34 and object pointers +0x40 through +0x54. Unknown fields retain offsets.

Update hides only handle and arrows when count ≤ page or count is zero; backing/end
visibility remains untouched. Otherwise it shows backing, conditionally shows handle
(selected zero hides it), then shows arrows, end 2 and end 1 before geometry dispatch.
Scaled geometry changes handle size and uses top-left placement; fixed geometry uses centre
placement with explicit first/last positions. Nonzero deferred byte updates saved positions
without moving the object. There is no replacement selection clamp.

Scaled arithmetic rounds page/count/size but stores the step without discarding its extended
x87 value before computing position. Fixed arithmetic rounds count/selection-minus-one/step,
keeps page as an integer in the denominator and uses the rounded step for interior positions.
Both denominators clamp differences below one. Source preserves these spill points and
arithmetic order; no forced-register code is used. A float-spill mutation requires an explicit
volatile temporary: a plain reread of the stored field lets MSVC reuse its x87 value.

`verify_pc_scroller_scrollbar.py` passes **15,641 original/rebuilt comparisons** under
53-/64-bit x87 precision, covering orientation, scaled/fixed/deferred flags, signed integer
extrema and ordinary counts, endpoint/interior/zero selections, null optional bindings,
pre-existing visibility, nonfinite extents and targeted zero-origin rounding cases. Fixtures
use valid image handles; all exercised coordinate, size, state-write and visibility dependencies
execute their recovered source. Font interfaces are linked but asserted unreachable here.
Five mutations are rejected: changing backing visibility in the hidden branch, wrong count
boundary, forcing an early step spill, using top-left instead of centre, and dropping the
selection-minus-one offset. Fingerprints/results remain in ignored
`build/analysis/scroller-scrollbar/verification.json`. **Byte matching is not established.**

The coordinate verifier expands to **29,970 comparisons**, including centre/size text paths
with controlled font lookup/line measurement. Width/height write tests use defined size-getter
inputs; original null/missing-font getters leave outputs untouched, and missing-font setter
scratch behaviour is outside this proof. The full layout chain still passes 14,762 comparisons
with actual recovered scrollbar geometry replacing its callback. Native construction now binds
all six retail ScrollBar objects and obtains initial geometry through recovered getters.
Full original construction remains a seam. Native build, pause/options regressions, all 19
settings scenarios, 10 Controls scenarios and rendered scroller motion pass.


## Scrollbar construction

`ScrollerScrollBar.cpp` recovers constructor `0x542ac0` and named image wrapper `0x4953e0`.
The constructor preserves flag bytes exactly, clears deferred +2 and observed fields,
but leaves step +0x34 and bytes +0x3c..+0x3f untouched. A null prefix skips all lookup and
geometry calls. Otherwise it formats six names into a 64-byte buffer, in order: _Backing,
_Handle, _Arrow_1, _Arrow_2, _Backing_End_1, _Backing_End_2. The wrapper hashes each name
before typed image lookup, including when package is null.

Construction reads backing top-left/size, then either handle size (fixed mode) or copies
the backing's opposite-axis extent into both handle dimensions (scaled mode). That unusual
scaled-mode axis choice is present in the original instructions and is preserved. Native
pause construction now calls the recovered constructor instead of assigning these fields.
The enclosing native scroller still clears its storage before this call; standalone proof
starts from arbitrary bytes to check the constructor's untouched fields independently.

The expanded scrollbar verifier passes **19,641 original/rebuilt comparisons**, including
2,000 constructors and 2,000 named wrappers. Constructor cases cover null/non-null package
and prefix, two prefixes, missing/present image results, noncanonical flag bytes and initial
memory preservation. Return values, cleanup, preserved registers, object memory and normalized
format/hash/lookup traces are checked. Five mutations are rejected: clearing the untouched
step, wrong scaled extent, wrong suffix, skipping lookup in fixed mode and hashing package
instead of object name. **Byte matching is not established.**

Formatted buffer `0x50c960`, uppercase hash `0x505450` and typed image lookup `0x537a60`
remain controlled interfaces in this proof. Native formatting uses vsnprintf; legacy
truncation/error semantics are unverified. Native image lookup enforces type 1 but its
package/manager search remains a seam. Fixtures keep generated names within the buffer.
The empty default constructor supports temporary enclosing native storage only; the original
parameterized constructor is used for actual setup. Full IconScroller construction remains
unrecovered. Native build, layout/pause/options regressions and settings/Controls/motion
smoke checks pass.

## Scroller deferred messages and input masks (2026-10-18)

`Frontend/IconScrollerInput.cpp` recovers deferred-message storage `0x50da20`
(43 bytes), StopInput `0x52fbf0` (65 bytes) and StartInput `0x52fba0` (67 bytes).
The original reference executable SHA-256 is
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`;
`config/SPEED2_EXE_1_2/icon-scroller-input.json` pins each body.

The message helper writes exactly four dwords at +0x13c..+0x148, leaving the
stopped byte +0x14c and surrounding bytes unchanged. StopInput clears scroller
byte +0x2f (owner +0x7b), looks up the package, calls the diagnostic sink, clears
package masks +0x1c/+0x24 and sets stopped +0x14c to 1. StartInput does nothing
when stopped is zero; any nonzero byte enables scroller +0x2f, looks up the
package, copies saved mask +0x20 into +0x1c/+0x28, calls the diagnostic sink,
then clears stopped. Stop leaves +0x20/+0x28 untouched; start leaves +0x24
untouched. Neither original routine checks for a missing package.

`FEngDiagnostic.cpp` recovers the retail diagnostic sink `0x50d510` (three bytes,
`xor eax,eax; ret`). Keeping it in a separate translation unit preserves the
diagnostic call boundaries without exposing its implementation to caller
optimization. Original StopInput relies on its ECX preservation; source instead
retains the package pointer across the normal C++ call. No register forcing is
used. Diagnostic format strings, arguments and state at call time are compared.

Native pause code now calls these recovered routines, reads scroller +0x2f for
its existing input gates and uses the explicit deferred fields. The native map's
`inputEnabled`/`stopped` booleans and native message-storage implementation are
removed. Full owner construction, message dispatch, package lookup and routing
remain native seams; this does not establish complete original input dispatch.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_input.py --mutation-checks
# Available fallback when the legacy compiler is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_input.py --compiler clang-cl --mutation-checks
cmake --build build/native -j4
build/venv/bin/python tools/smoke_native_pause_options.py --game '/path/to/game'
```

The Clang Windows i686 build passes **10,800 original/rebuilt routine comparisons**:
2,400 message writes, 3,600 stops, 3,600 starts and 1,200 diagnostic calls.
Fixtures begin with arbitrary bytes and cover stopped values 0/1/2/128/255,
arbitrary dword arguments/masks, repeated stops/starts and deferred writes
between transitions. Full guarded object/package memory, intermediate sequence
states, lookup/diagnostic traces, callee-saved registers and stack cleanup agree.
The real diagnostic body runs on both sides; package lookup alone is controlled
and returns a valid package. Five source mutations are rejected (wrong stop
mask, saved mask, noncanonical-byte handling, deferred argument and stopped byte).
Generated results remain in ignored `build/analysis/icon-scroller-input/`.

The native build, all 19 settings scenarios, all 10 Controls scenarios and
rendered scroller motion (869 -> 758.5 -> 640 px) pass. **MSVC 7.1/wibo is
absent in this checkout, so legacy-compiler validation remains outstanding.**
The verifier defaults to MSVC and records the explicit Clang fallback when used.
Finite synthetic comparisons exclude the original missing-package crash and
do not prove full GUI behavior. **Byte matching is not established.**

## Icon construction, insertion and texture dispatch (2026-10-18)

`Frontend/IconScrollerInsertion.cpp` recovers icon constructor `0x51f670`
(80 bytes), insertion `0x543c10` (291 bytes), and owner AddIcon wrapper
`0x50dab0` (46 bytes). `config/SPEED2_EXE_1_2/icon-scroller-insertion.json`
pins the same UG2 reference executable and the three function bodies.

The constructor initializes only the fields written by the original: texture
+0xc, object +0x10, offsets +0x14/+0x18, flags +0x20..+0x23, identifiers
+0x2c/+0x30, four floats +0x34..+0x40, and bytes +0x44/+0x45. Links
+4/+8, colour +0x1c, dimensions +0x24/+0x28 and padding +0x46/+0x47
remain untouched. This removes extra native zeroing. The compiler's abstract
icon vtable is normalized to original `0x79b4d8` for constructor comparison;
this is separate from the root destructor's `0x79afdc`. Destructor recovery
and allocation remain outside this checkpoint.

Insertion formats `%s%d` into a 32-byte buffer, looks up a typed named image,
and deletes a non-null icon on lookup failure. A successful lookup increments
+0x28 even when the icon argument is null, then returns null in that case.
With a valid icon it binds the image, reads size, samples colour, copies running
offset +0xe0, overwrites colour with white, reads size again, and reads size a
third time into scratch. The final width advances `(spacing + running) + width`
without an intermediate binary32 spill. It then appends the icon's +4 node and,
if current +0xc is null, selects the first list icon and requests focus through
the manager when its image exists. This is not necessarily the appended icon
when the list was already nonempty. The wrapper dispatches virtual slot +8,
then changes image texture and sets dirty bit `0x400000` only if texture differs.

Native code now delegates this slot to recovered InsertIcon and uses the
recovered owner wrapper. Its duplicate counter, running offset, geometry cache,
manual linking, texture setter and constructor implementation are removed.
The remaining native entry vector tracks ownership and pointer hit testing;
full scroller construction/destruction and owner message dispatch remain seams.
The native construction seam supplies prefix `option_` and spacing 40 in the
original fields instead of keeping separate insertion state.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_insertion.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is unavailable:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_insertion.py --compiler clang-cl --mutation-checks
```

The Clang Windows i686 build passes **6,000 original/rebuilt comparisons**:
2,000 each for constructor, insertion and wrapper, under 53-/64-bit x87 precision.
Compared evidence includes guarded memory, untouched constructor fields, return
values, stack cleanup, preserved registers and intermediate state at each helper
call. Fixtures cover empty/nonempty lists, existing/absent current icon,
manager presence, missing first-icon image, missing lookup, null insertion
argument, wrapping option indices, equal/different textures, three differing
size replies and a cancellation case detecting early binary32 rounding.
Six mutations are rejected: clearing untouched links, wrong numbering, wrong
offset, retaining sampled colour, wrong dirty bit and early float spill.

Formatting, named-image lookup, size, colour, focus and virtual scalar deletion
are controlled interfaces in this verifier; their complete composition is not
proved here. Formatting fixtures fit the buffer. The original wrapper reads
the icon texture after insertion even when insertion deletes the icon; the
controlled deletion fixture retains mapped storage, so post-deletion lifetime
is not verified. Null icons are excluded for the wrapper, which dereferences
them in the original. Native destruction and heap behavior remain scaffolding.
**MSVC 7.1/wibo validation is outstanding; byte matching is not established.**
Generated reports remain in ignored `build/analysis/icon-scroller-insertion/`.
The native build and the previous 10,800 input-helper comparisons pass after
integration. Desktop settings/Controls/motion checks pass; the same scripts now
default to SDL offscreen software rendering and retain their pixel assertions.
All 19 headless settings scenarios, 10 Controls scenarios and motion also pass,
as does the optional HUD verifier across
four aspect ratios, all eleven skins and its 120-frame capture. See the native
demo guide for `--game`, `--headless` and `--headed` options.

## Bound scroller labels and named-string lookup (2026-10-18)

`Frontend/IconScrollerLabels.cpp` recovers selection label update `0x50da50`
(81 bytes) and named-string lookup wrapper `0x4953c0` (22 bytes). The wrapper
executes the already recovered `cihash` at `0x505450` (47 bytes). All three
bodies and the reference executable are fingerprinted in
`config/SPEED2_EXE_1_2/icon-scroller-labels.json`.

The owner layout now separates the complete 0xe8-byte scroller from its bound
title object +0x134 and description object +0x138. These pointers formerly lay
in excess native scroller storage. Original constructor `0x552330` binds them
through `0x4953c0` with names `ICON_TITLE` and the empty string, respectively;
the native construction seam now performs those bindings through the recovered
wrapper. Full original construction is not recovered by this change.

`0x50da50` always calls the label setter twice. The first call uses the current
icon's +0x2c identifier or zero when no icon is selected. It then reloads the
current pointer and calls the second bound label with icon +0x30 or zero. A
setter can change the selection and description binding; caching either would
change observed behavior. The former native implementation looked up only
`ICON_TITLE` on each update and omitted the second call. Null bound labels are
still forwarded to the setter.

`0x4953c0` hashes the object name before typed string lookup, including null
package/name arguments. It now calls recovered `cihash` directly. The native
typed lookup adapter enforces type 2; the older generic `FngFindObject` adapter
remains a separate seam used by other code. Original manager/package traversal,
localization and text allocation are not established by this wrapper proof.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_labels.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_icon_scroller_labels.py --compiler clang-cl --mutation-checks
```

The Clang Windows i686 build passes **6,000 original/rebuilt comparisons**:
2,000 label updates, 2,000 named lookups and 2,000 direct hash calls. Named
lookup cases also execute actual recovered hash code on the rebuilt side.
Compared evidence includes guarded memory, integer/pointer returns, preserved
registers, stack cleanup and hash/lookup/setter traces with state at call time.
Fixtures cover selected/null icons, null/aliased/different label pointers,
setter-induced selection and description changes, null package/name, empty,
mixed-case, high-byte and 127-character names, and found/missing lookup results.
Typed lookup and the localized label setter alone are controlled interfaces.
Five mutations are rejected: omitted description, wrong description identifier,
cached selection, wrong bound label and hashing the package instead of the name.

The native build, all 19 headless settings scenarios, 10 Controls scenarios and
rendered scroller motion pass. The previous input and insertion verifiers also
pass their 10,800 and 6,000 comparisons with five and six rejected mutations,
respectively, after the owner-layout refinement. Generated reports remain in ignored
`build/analysis/icon-scroller-labels/`. **MSVC 7.1/wibo verification remains
outstanding; byte matching is not established for the newly recovered routines.**
Finite synthetic comparisons do not establish complete FEng localization,
constructor or GUI behavior.

Next bounded lead: static tracing of setter `0x50c900` shows a language hash at
object +0x60, conditional dirty writes at +0x1c, a virtual notification through
global `*0x838478`, and clearing flag bit 2 after notification. The native
immediate text replacement was a substitute for that state/callback path;
the recovery below supersedes this lead.

## Language-hash setter and observer notification (2026-10-18)

`Frontend/FEngText.cpp` recovers `0x50c900` (82 bytes), fingerprinted with the
composed label updater in `config/SPEED2_EXE_1_2/feng-text.json`. String language
hash +0x60 is represented as a union with group child count; all offsets and the
0x68-byte shared prefix remain unchanged. Only the first virtual slot of the
receiver at `*0x838478` is established; `FEngStringObserver` is a provisional
interface for that observed call boundary.

For a non-null object the setter saves incoming flags, conditionally writes
the changed hash and dirty bit `0x400000`, and calls the observer even when the
hash is unchanged. After notification it reloads flags, clears bit 2, and adds
the dirty bit if bit 2 was set **before** notification. Callback flag/hash writes
remain visible; the incoming bit controls the final dirty decision. Null objects
perform no writes or callback. There is no object-type guard in the original.

The native immediate text setter is removed. A native first-slot observer now
adapts type-2 objects to the existing language-table lookup and UTF-16 storage.
HUD and pause loading bind that receiver; initial SH hashes are preserved at
+0x60 instead of being overwritten by native child counts. Localization, initial
asset loading and allocation remain native seams, and the observer's other
original methods/behavior have not been recovered. The interactive renderer
still uses native text storage; this does not establish the original string
manager implementation.

The GPS updater `0x4c8410` also calls this address. Its former provisional helper
`HudObjectTexture` incorrectly sent language hashes to the native image-texture
setter. Those two recovered call sites now call `FngSetLabelString` directly;
GPS route production and complete presentation remain unresolved separately.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_text.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_text.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_hud_freeroam.py --compiler clang-cl
```

The Clang Windows i686 build passes **8,192 original/rebuilt comparisons**:
4,096 standalone setter cases and 4,096 label-updater compositions executing
the actual recovered setter. The observer virtual slot alone is controlled.
Fixtures cover null object/receiver, unchanged/changed hashes, arbitrary flags,
callback flag/hash mutation, disabling the observer, aliased/null labels and
callback-induced selection/binding changes. Guarded memory, state at callback,
global observer state, stack cleanup and preserved registers agree. Recursive
setter calls are not covered. Six mutations are rejected: skipping unchanged
hashes, wrong hash field, omitted dirty write, retaining flag 2, using the
callback's flag for the final dirty decision, and omitted notification.

The broader freeroam HUD verifier now supports an explicit Clang fallback and
passes **15,180 comparisons** under its 53-/64-bit x87 fixtures. That verifier
still controls presentation interfaces, including the setter, rather than
proving the full GPS/setter/observer composition. MSVC remains its default.
Native and mod-enabled builds pass. All 19 headless settings scenarios, 10
Controls scenarios, scroller-motion pixel checks and HUD captures (four aspect
ratios, eleven skins and a 120-frame run) pass after observer integration.
Generated logs remain in ignored
`build/analysis/feng-text/`. **MSVC 7.1/wibo verification is outstanding; byte
matching is not established.** Finite callbacks do not prove the complete
original localization or observer subsystem.

## Object-tree and typed package lookup (2026-10-18)

`Frontend/FEngLookup.cpp` recovers the following UG2 PC v1.2 routines, with
reference hashes in `config/SPEED2_EXE_1_2/feng-lookup.json`:

| Address | Bytes | Observed responsibility |
| --- | ---: | --- |
| `0x504830` | 74 | Visit a group's child chain |
| `0x504880` | 70 | Visit the package's root chain |
| `0x5048d0` | 25 | Stop at the first matching object hash |
| `0x517f80` | 81 | Construct the query and search a package |
| `0x505310` | 63 | Compare package names with signed-byte ASCII folding |
| `0x519e50` | 87 | Search package names by hash, then comparison |
| `0x52c570` | 90 | Search the queue's package chain |
| `0x5379c0` | 66 | Package-local lookup, then manager fallback |
| `0x537a10` | 73 | The same lookup with a type-2 string filter |
| `0x537a60` | 73 | The same lookup with a type-1 image filter |

Traversal calls the visitor before recursing into type-5 groups and reloads
sibling links after callbacks. A false callback byte stops the entire walk.
The local query uses vptr/hash/result at offsets 0/4/8. Package roots are at
+0x4c; package next/name/hash are +4/+0xc/+0x10. Queue collection +0xdc has
its first package at +8 (queue +0xe4). These are observed prefixes, not full
class recoveries. The virtual destructor in the query interface models its
observed slot; the original destructor body is not recovered here.

The queue searches one named package or, for a null name, all packages in
chain order. Wrappers first use `FngFindPackage` for a non-null name, then
read the current manager at `*0x8384c4` for fallback. A found object of the
wrong type is rejected without another fallback search. A non-null manager
requires a valid queue when fallback is reached. Name search hashes even an
empty list, handles null stored names separately, and resolves real hash
collisions through comparison. The comparator folds only ASCII lowercase;
high bytes and the final subtraction are signed.

Native typed string/image wrappers and `FngFindObject5379C0` adapters are
removed. The loader now links roots as well as children in parsed file order.
A native adapter links shown screens in fixed screen-array order into the
queue. Package loading, membership/order and `FngFindPackage` (`0x52cef0`)
remain seams; archive search `0x51bb70` is not recovered by this checkpoint.
The separate legacy native `FngFindObject` adapter still serves older callers.
HUD's flat native lookup is also separate. These changes do not establish
complete FEng resource loading or original package lifecycle behavior.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_package.py --compiler clang-cl
```

Clang Windows i686 verification passes **11,264 original/rebuilt comparisons**
(1,024 fixtures across eleven entries including recovered `cihash`). Lookup
compositions execute the actual traversal, query, hashing, name comparison,
package-list and queue routines. Only package locator `0x52cef0` is controlled;
direct walk fixtures additionally control the visitor callback. Cases include
nested acyclic groups, empty chains, duplicate object hashes, null/mixed-case/
missing/empty names, signed high bytes, the genuine name-hash collision
`B!.fng`/`AB.fng`, manager absence and callback-induced global changes.
Direct visitors exercise stopping, sibling/type mutations and noncanonical
true bytes. Guarded memory, callback/hash/comparison traces, return values,
stack cleanup, preserved registers and global state agree.

The original package query creates a compiler SEH frame. The verifier supplies
a valid FS/TIB and checks normal-return chain restoration. **Exception unwinding
is excluded**; generated compiler handlers must remain unexecuted. Null query
names with a non-null empty-name package are excluded because the original can
reach a null string comparison. Six mutations are rejected: continuing after a
match, omitting group recursion, wrong image type, ignoring name collisions,
skipping manager fallback and retrying a wrong-type string result.

Package regression passes **12,000 comparisons** with the extended headers.
Native build and headless checks pass: all 19 settings scenarios, all 10 Controls
scenarios and scroller-motion pixel assertions. Logs and reports remain ignored
under `build/analysis/feng-lookup/`. **MSVC 7.1/wibo verification is outstanding;
byte matching is not established.** Finite tree fixtures do not prove arbitrary
callback reentrancy, malformed/cyclic trees, exception behavior or the complete
GUI subsystem.

## Live-list and archive package locator (2026-10-18)

`Frontend/FEngLocator.cpp` recovers `0x52cef0` (106 bytes), added to the lookup
manifest. When the unidentified 32-bit value `0x8384d0` is zero and the manager
exists, it retains the manager's queue, returns null for a null queue, and
searches live collections +0xdc and +0xec in that order (heads +0xe4/+0xf4).
A live-list miss returns null without consulting the archive. Otherwise it calls
`FEngArchive::FindNode` on receiver `0x8637b0` and returns the node's package
+0x18, or null. No null-name guard is added. The native direct screen locator
is removed; native initialization explicitly selects mode zero and retains the
existing shown-screen chain. The second collection is currently empty in the
native demo. Original package membership and archive storage remain seams.

The extended `verify_pc_feng_lookup.py` now composes the **actual locator** with
all three lookup wrappers, both list searches, hashing, name comparison and
object traversal. Only archive search `0x51bb70` and the direct-walk test visitor
are controlled. Fixtures include independent mode/manager combinations,
noncanonical nonzero mode values, empty/duplicate-name/secondary-only lists,
archive misses, nodes with null packages, archive-induced manager changes and
standalone null-queue cases. Name-list argument traces also agree. The original
passes collection receivers +0xdc/+0xec; the search never reads that receiver,
and Clang can omit its register load despite disabled inlining. The verifier
compares the used arguments and results, rather than forcing a register load.

The Clang i686 run passes **12,352 original/rebuilt comparisons** (1,024 across
12 entries plus 64 standalone null-queue locator cases). All previous six
mutations plus four locator mutations are rejected: ignoring mode, omitting the
secondary list, preferring the secondary list and consulting the archive on a
live miss. Normal-return SEH checks retain the earlier limits. **MSVC 7.1/wibo,
byte matching, original archive search, arbitrary reentrancy and full package
lifecycle behavior remain unverified.** Commands are unchanged from the lookup
section above; generated locator logs remain under `build/analysis/feng-lookup/`.

Native and mod-enabled builds pass after locator integration. Headless validation
passes all 19 settings and 10 Controls scenarios plus scroller motion. Slider
fill widths are 0/99/206 pixels with monotonically moving knobs; motion settles
at the screen centre. These are integration checks, separate from PC execution
comparisons. The existing package regression also passes 12,000 comparisons.

## Archive hash search and intrusive-list promotion (2026-10-18)

`Frontend/FEngArchive.cpp` recovers `0x51bb70` (206 bytes), fingerprinted in
`config/SPEED2_EXE_1_2/feng-archive.json` with its already recovered helpers
`strlenWithNullcheck` (`0x43dbb0`, 26 bytes) and `cihash` (`0x505450`, 47 bytes).
The routine uses an archive sentinel at receiver +4 and node links at +4/+8;
node +0xc/+0x10 point to records, with live package +0x18. These remain partial
layouts; record names/ownership and allocation are not inferred here.

Name scanning begins at the terminating NUL and walks backward until reaching
the original pointer, stripping only a backslash found before that stopping
point. A backslash at the first byte is therefore retained. Forward slashes
are not separators. A null name passes the nullable length/hash helpers without
an added early return. Record kind `0x30210` supplies its hash at record +8.
Kind `0x30203` hashes data +0x28 only when data words +0/+8 equal `0xe76e4546` /
`0x64486b50` and unsigned version +0x10 is at least `0x20000`. Rejected headers
and other kinds yield candidate hash zero; the original still compares that
zero with the query. This can match a genuine zero-hash name such as bytes
`1f 41 21`. Hash equality alone selects a node, including collisions; there is
no subsequent name comparison in this routine.

The first hit is unlinked and prepended to the sentinel list, even if it was
already first. Source preserves the six link-write operations and their order.
A repeated lookup sees the promoted node. The list must be valid and circular;
a null head is not a valid empty-list representation for this routine. An empty
list points its head/tail to the sentinel.

The native `FindNode` screen adapter is removed. Native scaffolding builds
`0x30210` hash records and a circular list of shown screens, rebuilding it when
screen membership changes. Recovered search now performs subsequent promotion.
This retains a native membership/order/header-construction seam; it does not
recover original archive loading, decompression, allocation or persistence.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_archive.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_archive.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compiler clang-cl --compose-archive --mutation-checks
```

Clang i686 archive verification passes **8,192 original/rebuilt comparisons**:
4,096 fixtures each called twice without restoring the list between calls.
It executes both recovered string helpers; no callee is controlled. Comparisons
check guarded memory, exact ordered arena writes, helper-call arguments/order,
return values, stack cleanup and preserved registers. Cases cover empty and
one-to-five-node lists, duplicate/colliding/zero hashes, valid/rejected records,
unsigned version boundaries, null/empty/high-byte/path names and repeated hits.
Six mutations are rejected: omitted promotion, wrong magic, wrong inclusive
version boundary, refusing zero candidates, ignoring the basename and stripping
a leading backslash. The Clang verifier disables builtin substitution so the
nullable-length loop executes rather than a generated host `strlen` call.

The optional lookup composition passes **13,376 comparisons**, executing the
actual archive routine, locator, both live searches, object traversal and typed
wrappers. Its archive fixtures are empty or single-node `0x30210` records; the
larger standalone archive test covers the other records and list promotion.
Only direct visitor fixtures control a callback. Normal-return SEH checks retain
the preceding scope. **MSVC 7.1/wibo and byte matching remain outstanding.**
Finite valid lists do not establish behavior for malformed pointers, arbitrary
reentrancy or complete archive lifecycle. Generated reports/logs remain ignored
under `build/analysis/feng-archive/` and
`build/analysis/feng-lookup/archive-composition.json`.

Native and mod-enabled builds pass after archive integration. Headless checks
pass all 19 settings scenarios, all 10 Controls scenarios, slider fill/knob
pixel assertions and selected-icon motion/settled-centre assertions. These
integration checks are separate from the original/rebuilt execution evidence.
The composed lookup run also rejects all ten existing lookup/locator mutations;
its archive-specific mutations are exercised by the standalone verifier.

## Script selection and track activation (2026-10-18)

`Frontend/FEngScript.cpp` recovers the following PC v1.2 routines, fingerprinted
in `config/SPEED2_EXE_1_2/feng-script.json`:

| Address | Bytes | Observed responsibility |
| --- | ---: | --- |
| `0x4fe890` | 21 | Resolve a reference chain to its direct head |
| `0x4fe6b0` | 184 | Four-component quaternion multiplication |
| `0x505580` | 60 | Four wrapping integer differences |
| `0x517720` | 369 | Initialize the selected script's tracks |
| `0x5178b0` | 19 | Assign a script pointer and initialize if non-null |
| `0x51cf70` | 97 | Select by name and conditionally reset time |

`0x51cf70` hashes the name on **each** visited script, stopping at the first
matching hash. Null object and empty script list do not hash. A changed selection
writes current script +0x54, calls initialization directly, then reloads +0x54
before zeroing time +0x10. A nonzero restart byte additionally zeros the found
script's time. A same-pointer selection without restart performs no initialization
or reset. `0x5178b0` is a pointer setter, not the native hash-selection loop that
was previously labelled with that address: it always assigns, initializes any
non-null pointer (including the same pointer), and does not itself reset time.

Initialization snapshots the selected track table and unsigned count. Each
0x38-byte track has flag bit `0x80` cleared. Only interpolation bytes 3/4 compute
an activation difference. The signed track byte +7 indexes the live state in
four-byte units. Flag +0x2c chooses direct key pointer +0x30 or reference resolution
through the reference pointer at +0x30; a null resolved key is skipped. Parameter types 1–6 write
integer scalar, float scalar, float two-vector, float three-vector, quaternion,
or integer four-vector differences at key +0x10. Other parameter types leave
that value untouched. Live state is reloaded per track; table/count stay fixed.

The reference resolver reads nonzero flag +4 and follows +8 until a direct
receiver, returning its +8. Valid acyclic references are required. Integer
subtraction uses unsigned words to retain x86 wrapping without signed-overflow
assumptions and caches inputs before aliasing output writes. Quaternion recovery
retains operand grouping, caches inputs, and explicitly rounds the third cross
component to float as the original stack store does. Name selection, pointer
assignment, initialization and arithmetic helpers keep their separate observed
call boundaries; no artificial register loads are added for matching.

The native name-selection adapter is removed. The existing native hash adapter
now calls recovered pointer activation on a changed script; that adapter and
package-level wrappers remain seams. Static tracing of reader `0x5515e0` confirms
the first serialized Kd key goes inline at track +8 and later keys form the delta
list. The native loader now owns those later key-node prefixes, links them in
file order and exposes their head at track +0x30. Activation writes flow back to
the native playback data. Allocation, reference sharing and the complete original
reader are not recovered here.

The native script-hash map is removed; compatibility response conditions read
the recovered current header. Native event delivery observes selected headers,
resets its cursor on restart and retains completed cursors until selection changes
to avoid replaying completed events. Fractional frame timing, event delivery,
script-running predicates and the limited interpolation backend remain documented
native seams. In particular this checkpoint does **not** establish original
playback of interpolation modes 3/4, despite recovering their activation path.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_script.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_script.py --compiler clang-cl --mutation-checks
```

Clang Windows i686 (`/arch:IA32`) passes **14,336 original/rebuilt comparisons**:
1,024 fixtures across seven entries at each x87 control word `0x027f`/`0x037f`.
All called helpers are recovered and executed; no interface is controlled.
Guarded memory, hash/initialization/reference/arithmetic call traces, return
values where defined, stack cleanup, preserved registers, x87 control and stack
balance agree. Fixtures include empty/duplicate/missing name chains, null object/
name/script, changed/unchanged selection, noncanonical restart/reference bytes,
zero-to-eight tracks, signed offsets, null direct/resolved keys, types outside
1–6, output/input aliasing, and activation that aliases the current-script pointer.
Arithmetic inputs use finite normal floats of varied signs/exponents and arbitrary
integer words; non-finite/subnormal behavior is not established.

Nine mutations are rejected: retaining track flag `0x80`, omitting mode 4,
unsigned state offset, skipping reference resolution, omitted time reset,
resetting the original pointer instead of reloading selection, unconditional
same-pointer restart, retaining null selection and hashing once before the loop.
**MSVC 7.1/wibo and byte matching remain outstanding.** Finite valid lists and
normal floating fixtures do not prove arbitrary aliasing, malformed/cyclic data,
all floating special values, allocation or the full animation subsystem. Reports
and captures remain ignored under `build/analysis/feng-script/`.

Native and mod-enabled builds pass. Final headless integration checks pass all
19 settings and 10 Controls scenarios, slider pixel assertions and the selected
icon's motion/settled-centre assertions after the event adapter changes. The
native event observer, ownership and clock glue remain scaffolding rather than
original/rebuilt proof of the original playback/event subsystem.

## Hash selection, package script wrappers and running queries (2026-10-18)

`FEngScript.cpp` adds hash selection `0x51cfe0` (83 bytes) with the same direct
initialization and current-pointer reload order as name selection. The four
routines in `Frontend/FEngScriptLookup.cpp` are fingerprinted in
`config/SPEED2_EXE_1_2/feng-script-lookup.json`:

| Address | Bytes | Observed responsibility |
| --- | ---: | --- |
| `0x537c00` | 84 | Find the object, then select a script by name |
| `0x537c60` | 84 | Find the object, then select a script by hash |
| `0x537cc0` | 115 | Find the object, format/hash a name, then query current script |
| `0x537d40` | 96 | Find the object, then query a current script hash |

Each keeps local-package search and manager queue fallback explicit, rather than
calling a new shared lookup helper. Setters call their selector even for a null
object. Only the low restart byte is used; full-word `0x100` means no restart.
The named running query calls **formatted** hashing `0x50b9c0` after lookup,
even on a miss, and reads the selected header after that call. It is not a simple
case-insensitive hash wrapper. Both predicates require a current header with the
requested hash and **signed** `time10 <= lengthC`, including equality. Only AL
is the defined predicate return; upper EAX bits are not compared.

Native package wrappers, event-list-based running predicates and the duplicate
hash-selection loops are removed. `FengPackage::PlayScript` now delegates directly
to recovered name selection for HUD/Controls callers. Native response commands
call recovered object/hash selection, while the existing event observer follows
the selected header. The unused native frontend script adapter and owner search
are removed. Package membership/loading, formatted hashing, playback clocks,
limited interpolation and event delivery still have native seams.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_script.py --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compose-scripts --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent: add --compiler clang-cl.
```

The selector/activation suite now passes **16,384 original/rebuilt comparisons**
at x87 control words `0x027f`/`0x037f` and rejects ten mutations, adding a wrong
hash-selector comparison to the preceding checkpoint's nine. Its finite floating
and aliasing limits still apply.

The full `--compose-scripts` suite passes **17,472 comparisons** across lookup,
archive, locator and the four wrappers. It executes recovered object/name/hash
search, both live lists, archive search, selectors and initialization. Wrapper
activation fixtures use a single interpolation-1 track to check flag writes;
the separate selector suite covers the arithmetic modes and aliases. Only
formatted hashing `0x50b9c0` and the direct-walk visitor fixtures are controlled.
Cases include empty/missing/duplicate scripts, null lookup results, signed time
extremes and equality, mode/manager/list variations, noncanonical restart bytes,
a dedicated unchanged-selection/`0x100` case, and a formatter that clears current
selection. Hash/name/list/format/selector/initialization traces, guarded memory,
normal-return SEH restoration, stack cleanup and preserved registers agree.
Selector traces compare the defined restart byte; unused upper argument bits can
differ with compiler code generation. Void-return EAX values are not compared.

The mutation checks retain the ten lookup/locator challenges and additionally
reject skipped script fallback, exclusive finish comparison, unsigned time
comparison, omitted formatting on lookup miss, caching selection across
formatting, skipping a null selector call and treating full-word restart as a
boolean. Formatter call results/mutations are controlled evidence, not proof
of the original formatting implementation or all caller reentrancy. Original
`0x50b9c0` uses a 256-byte local buffer and calls `0x43fbc0`; the native formatter
currently uses bounded host formatting and remains a distinct seam.

**MSVC 7.1/wibo and byte matching remain outstanding.** These finite valid
fixtures do not establish malformed/cyclic data behavior or complete GUI script
playback. Reports remain ignored in `build/analysis/feng-script-lookup/` and
`build/analysis/feng-lookup/script-composition.json`.

Native and mod-enabled builds pass. Headless integration passes all 19 settings,
all 10 Controls scenarios, slider pixel checks and scroller motion. HUD captures
also pass four aspect ratios, all eleven skins and a 120-frame run after routing
HUD name selection through recovered code. These are native integration checks,
separate from original/rebuilt proof of event timing and interpolation.
