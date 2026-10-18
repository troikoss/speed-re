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
used native text storage at this checkpoint. The later
[owned UTF-16 storage checkpoint](#owned-utf-16-storage-in-native-strings) replaces
that buffer ownership; full original string-manager behavior remains pending.

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

## GUI formatting wrappers and profiling (2026-10-18)

`Frontend/FEngFormatting.cpp` and `Libs/bWare/Formatting.cpp` recover four
routines, fingerprinted in `config/SPEED2_EXE_1_2/feng-formatting.json`:

| Address | Bytes | Observed responsibility |
| --- | ---: | --- |
| `0x43e5c0` | 86 | Shifted-counter elapsed calculation |
| `0x43fbc0` | 117 | Formatter context, call and profiling accounting |
| `0x50b9c0` | 84 | Format into a 256-byte local and hash inline |
| `0x50c960` | 24 | Forward variadic arguments and ignore advertised capacity |

`0x50b9c0` calls the shared formatter once and computes the case-folded 33 hash
inline from seed `0xffffffff`, preserving byte order and the terminating NUL.
It does not call `cihash`. `0x50c960` does **not** use its size parameter and
propagates the formatter's result. The previous native bounded-formatting
wrappers are removed; this restores original capacity behavior rather than
claiming legacy truncation semantics.

The shared wrapper queries the imported performance counter, shifts the signed
64-bit value by the low byte of global `0x8284e4`, and retains the shifted low
word as its start value. Its 12-byte context sets destination +0, byte +4 to
zero and word +8 to zero; padding +5..+7 is not initialized. It calls original
formatter core `0x43e950`, then elapsed helper `0x43e5c0`, adds elapsed to the
current float total at `0x828518`, increments count `0x82851c` with wrapping,
and returns the preserved formatter result.

Elapsed calculation queries the counter again, then reloads shift. It subtracts
start modulo 32 bits, clamps a negative signed difference to zero, multiplies
by signed `1 << (shift & 31)`, then by float scale `0x828028`, returning through
x87. Arithmetic right shift saturates to the counter's sign for counts at least
64, matching MSVC runtime helper `0x75c600` (33 fingerprinted bytes); it does not
mask the count modulo 64. Shift 31's signed scale factor is preserved. The total
is read after elapsed calculation, so callback writes to profiling globals are
visible. Original profiling units/initialization have not been established.

`NativeFormatting.cpp` now supplies the clock import and the still-unrecovered
formatter core. Its SDL counter initialization chooses shift zero and a scale
of `1 / SDL_GetPerformanceFrequency()` (seconds), explicitly a native clock
seam. The host formatting adapter handles the memory-sink path supplied by the
recovered wrapper and advances its destination pointer. The original formatter
state machine at `0x43e950`, debug/ring sinks, custom flags/conversions and error
behavior remain unrecovered; host formatting is not proof of those behaviors.
No retail formatter output or original engine parser body is imported here.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_formatting.py --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compose-formatting --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent: add --compiler clang-cl.
```

Clang i686 standalone verification passes **8,192 original/rebuilt comparisons**
(1,024 fixtures across four entries at x87 `0x027f`/`0x037f`). Only the clock
import and formatter core are controlled. Comparisons include used context fields,
six supplied variadic words, helper-call order, guarded output, raw 80-bit elapsed
return capture, float total/count/shift/scale, integer returns, stack cleanup,
preserved registers and x87 control/stack balance. Fixtures include zero-to-255
output bytes, high bytes and lowercase, capacities zero-to-seven, arbitrary
formatter returns, signed counters, shift boundaries 31/32/63/64/255 and low-byte
wrapping, negative-delta clamps, count wrapping, and core/clock mutations to
profiling globals. Uninitialized context padding is not compared.

Ten mutations are rejected: wrong hash seed, omitted case folding, honoring zero
capacity, shifting the variadic argument pointer, omitted count increment,
omitted negative-delta clamp, unsigned scale factor, masking shift modulo 64,
caching total before the clock callback and wrong context initialization.
The original compiler's signed-shift helper executes from the reference image
where required by original/rebuilt code; this is a compiler-runtime dependency,
not a newly recovered engine function.

The full `--compose-formatting` lookup/script suite passes **17,472 comparisons**
and rejects the seventeen existing lookup/script mutations. Formatted hashing,
profiling, elapsed calculation, script selection and the earlier lookup/archive
chain now execute recovered bodies together. Controlled interfaces are the
formatter core, clock import and the separate direct-walk visitor fixtures.
Composition uses short controlled core output and fixed increasing clocks;
the standalone verifier exercises the broader output/clock boundaries. This
supersedes formatted hashing as an opaque interface in the preceding checkpoint.

**MSVC 7.1/wibo and byte matching remain outstanding.** These finite tests assume
well-terminated formatter output within the hash wrapper's 256-byte local and a
clock result written by the import. They do not establish original core parsing,
all special floating values, failed-clock/unwritten-output behavior or complete
GUI formatting. Reports remain ignored under `build/analysis/feng-formatting/`
and `build/analysis/feng-lookup/formatting-composition.json`.

Native and mod-enabled builds pass. Headless integration passes all 19 settings,
all 10 Controls scenarios, slider fill/knob pixel assertions and scroller
motion/settled-centre checks after restoring original capacity forwarding and
adding profiling. These checks cover the current demo's format usage; they do
not validate the unrecovered core's complete conversion language or failure
behavior.

## Formatter output helper and parser investigation (2026-10-18)

`Libs/bWare/FormattingSink.cpp` recovers `0x43cb40` (130 bytes), fingerprinted
in `config/SPEED2_EXE_1_2/bware-format-sink.json`. The helper copies an explicit
byte count, including embedded NULs; it neither scans nor terminates a string.
For entry mode zero it adds the entire length to the caller's 32-bit count
**before** checking the destination. A null destination still counts bytes.
A non-null destination copies forward byte by byte, reloading/incrementing the
context pointer after each store. This is not `memmove` behavior for overlapping
source/destination buffers.

For entry mode nonzero and length nonzero, it caches ring index `0x828514`,
then reloads context mode for each byte. Nonzero mode writes to the 128-byte
ring `0x828178` and masks the local index with `0x7f`; mode zero redirects later
bytes to the memory destination if present. Each byte increments the caller's
count after its output operation. The cached ring index is written back only
at the end. A count aliased to the mode or global index, or a context within the
ring, makes this ordering observable. Zero-length ring output leaves the index
untouched. Unknown context fields keep their offset-based names.

The native host formatter now allocates its temporary output with `vasprintf`,
then calls the recovered helper for output/count/pointer handling and performs
the observed final NUL write. Entry ring metadata uses the low byte of context
+8. Native allocation and the host parser remain scaffolding. In particular,
host parsing and conversion side effects can precede the output copy, whereas
the original parser interleaves conversion and output. This integration does
not establish original `%n` aliasing, custom conversions, debug delivery or
error handling. Current GUI callers use the memory-sink path.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_format_sink.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_format_sink.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/probe_pc_format_core.py
```

Clang i686 verification passes **4,096 original/rebuilt comparisons**, with no
controlled callees. It compares guarded memory, ring contents/index, exact ordered
writes, stack cleanup and preserved registers. Eleven fixture families cover
memory/ring/count-only output, forward/reverse overlaps, count aliased to the
destination word or mode word in both paths, count aliased to the ring index,
a context within the ring and source aliased to the count. Lengths include
0/1/2/127/128/129/255/300; ring indices include wrap boundaries and counts wrap
modulo 32 bits. The Clang fallback explicitly disables strict alias optimization,
consistent with the native build and legacy compiler assumptions.

Six mutations are rejected: omitting count changes for null output, wrong ring
mask, publishing the ring index each byte, omitting mode reload, delaying the
full memory count update until after copying and advancing the destination
before storing the byte. **MSVC 7.1/wibo and byte matching remain outstanding.**
Finite mapped pointers/lengths and selected aliases do not prove arbitrary
malformed inputs or the complete formatter. Reports remain ignored under
`build/analysis/bware-format-sink/`.

`probe_pc_format_core.py` is **original-only investigation**, not a rebuilt
implementation or equivalence test. Its 35 cases at each x87 control word
`0x027f`/`0x037f` complete against core `0x43e950`, controlling pointer validation
`0x43bd60` alone. The validator itself calls `IsBadReadPtr(pointer, 1)` and, on a
bad read, `IsBadWritePtr(pointer, 1)`; those import semantics remain to be ported.
The probe records return count, output, `%n` storage and validation calls.
Observed leads for full parser recovery include:

| Input | Original output |
| --- | --- |
| `%b`, 10 | `1010` |
| `%p`, `0x1234abcd` | `1234ABCD` |
| `%z`, `0x18000` | `1.50` |
| `%.0z`, `0x18000` | `1/` (preserve this observed edge case) |
| `%f`, 1.25 | `1.250` |
| `%e`, 0.125 | `1.250e-001` |
| `%g`, 123.5 | `1.235e+002` |
| `%q` | `q` |
| `trailing%` | `trailing` |

Null/bad strings use `(null)`/`(badptr)`. The reference non-finite strings are
`1.#INF `, `-1.#INF` and `1.#QNAN`. The packed 91-byte table at `0x786cf0`
combines low-nibble character classes (ASCII 32–122) with high-nibble state
transitions for nine classes and eight states. The core body spans
`0x43e950`–`0x43f5d9`; a shorter guessed end would incorrectly classify its
loop tail as a dependency. Static tracing also identifies `%v2`/`%v3`
formatting through `0x4400d0`; vector conversions are not executed by this probe.
These observations guide the pending state-machine
recovery and do not justify replacing it with the host formatter.

Native and mod-enabled builds pass. Headless checks pass all 19 settings and
10 Controls scenarios, slider fill/knob pixel assertions, and scroller motion
with settled centring after routing host output through the recovered helper.
These are current-demo integration checks, separate from full original parser
or ring-consumer verification.


## Formatter state machine recovery (2026-10-18)

`Libs/bWare/FormattingCore.cpp` recovers the original core `0x43e950`, pointer
validator `0x43bd60` and recursive variadic wrapper `0x4400d0`. The manifest
`config/SPEED2_EXE_1_2/bware-format-core.json` fingerprints the 3,210-byte core
span through `0x43f5d9` (including internal gaps), the 42/24-byte helpers and
91-byte packed parser table. This supersedes the host-parser seam and pending
recovery statements in the preceding checkpoint. No SDK implementation is used.

The state machine interleaves literal output, flags/width/precision parsing,
conversion and padding. The low table nibble classifies characters, while the
high nibble selects transitions. Assembly tracing resolves reachable alternate
prefix blocks that the initial decompilation omitted. Integer bases 2/8/10/16,
short signed/unsigned loads, pointer formatting, signs, alternate prefixes and
locale grouping retain original ordering. Floating conversions retain x87
intermediates, double-precision `0.1` constants, default precision three,
three-digit scientific exponents and the original special-value strings.
`%g`/`%G` use the observed scientific path. Signed 16.16 `%z` keeps the unusual
`>= 0x7fffffff` sign threshold and the `%.0z` rounding output `1/` for `0x18000`.

`%s` probes one byte through the validator; it then reads one byte beyond the
precision-limited output, including a read at precision zero. `%n` stores a
32-bit count even with length flags and can change later parsing/output when
aliased to context mode, format bytes or locale bytes. The validator accepts a
successful read probe immediately, otherwise accepts a successful write probe;
both imports receive length one.

`%v2`/`%v3` cache the dimension before pointer validation, promote float components
to double, and recurse through `bSFormat` -> `bVFormat` -> the core. The original
literal forms are `[%*.*f,%*.*f]` and `[%*.*f,%*.*f,%*.*f]`. Recursive bytes plus
the NUL pass through the sink with a separate ignored count, followed by pointer
and conditional ring-index rewind. Vector payload therefore does **not** add to
the outer return count. Failed vector validation consumes its argument but
leaves the dimension character to print literally. Invalid dimensions consume
no argument. These quirks are preserved, not normalized to standard `printf`.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_format_core.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_format_core.py --compiler clang-cl --mutation-checks
```

Clang i686 passes **6,534 original/rebuilt comparisons**: 1,089 deterministic
fixtures, each in memory/count-only/ring output at x87 control words `0x027f`
and `0x037f`. Fixtures include 768 seeded bounded random conversions, integer
edges, short loads, fixed-point rounding, recursive vectors, invalid/null
pointers, raw signaling/quiet NaNs, infinities, scientific carry, mutable
locale grouping and selected `%n` aliases. Snapshots compare guarded arena
memory, output/ring/index, return count, locale and profiling globals, stack
cleanup, preserved registers, x87 control word and empty stack. Ordered traces
compare sink payload calls, exact selected string reads, short-circuit pointer
imports and recursive clock calls. All recovered engine formatter callees
execute directly; only `IsBadReadPtr`, `IsBadWritePtr` and
`QueryPerformanceCounter` are controlled. Reference compiler-runtime float
conversion and signed-shift helpers execute where the rebuilt compiler emits
them; this is not recovery of those CRT helpers.

Eight mutations are rejected: wrong float default precision, omitted alternate
flag, adding vector bytes to the outer count, consuming invalid-vector suffix,
changing pointer-probe AND to OR, omitting the extra string read, omitted octal
prefix and skipped zero-precision fixed-point rounding. Reports and mutation
logs remain ignored under `build/analysis/bware-format-core/`.

The native demo now links the recovered parser directly, removing the host
`vasprintf` parser and temporary allocation. `NativeFormatting.cpp` only supplies
clock/calibration and Linux permission adapters for the two Win32 probes. The
latter inspect `/proc/self/maps` for the requested read/write permission. They
do not establish identical Win32 guard-page or mapping-race behavior. Original
parser table, string literals/pointer slots and locale bytes remain reference
image data bindings in `OriginalSymbols.ld`; standalone ownership of those data
is pending. Playback clocks, script command execution and event delivery remain
separate GUI recovery targets.

**Limits:** these are finite behavioral comparisons, not byte matching. MSVC 7.1
and wibo are unavailable. Arbitrary width/precision overflow, buffer overflow,
malformed inputs, arbitrary aliasing and FP exception flags are not established.
Table-classified conversions `B`, `C`, `S`, `t`, `w`, `y` reach unset/prior local
state in the reference; the source retains an unset/prior size and these paths
are excluded from equivalence claims. Existing composed lookup verification
still controls the core boundary; this new verifier separately executes the
actual core and its recursive engine dependencies.


Native and mod-enabled builds pass with the recovered core. Headless integration
passes all 19 settings scenarios, all 10 Controls scenarios, slider fill/knob
pixel assertions and scroller motion/settled centring. Mod HUD checks pass all
four aspect ratios, all 11 skins and the 120-frame capture. These establish
current demo integration separately from the bounded original/rebuilt core
comparisons above.


## Response conditional traversal (2026-10-18)

`Frontend/FEngResponse.cpp` recovers conditional-block helper `0x504490`
(105 bytes through `0x5044f8`), fingerprinted in
`config/SPEED2_EXE_1_2/feng-response.json`. Its response prefix reads a 32-bit
command count at +0x10 and command pointer at +0x14; commands have stride 12.
Dispatcher `0x52c7c0` separately shows command type/parameter/target at +0/+4/+8.
Unknown response prefix fields remain offset-based. The helper uses thiscall
and returns with `ret 4`.

An index equal to unsigned `count-1` returns count without reading command
storage. Otherwise depth starts at one, and the helper reads the **next** record
before incrementing the index. Types `0x300` and `0x301` increase depth;
`0x500` closes only depth one, and `0x501` decreases depth. Other types leave
depth unchanged. Iteration continues while the incremented unsigned index is
less than count and depth is nonzero. Consequently missing delimiters can read
record index **count** before returning count. Count zero/index zero still
reads one next record; count zero/index `0xffffffff` returns zero immediately.
This observed behavior is preserved rather than changing the bound to count-1.

The native `SkipBlock` implementation is removed. The remaining native response
dispatcher constructs this observed response prefix over its decoded vector and
calls the recovered helper directly for `0x300`/`0x301`/`0x500`. Native commands
now use the shared recovered 12-byte record type. Decoding, prefix construction,
dispatch `0x52c7c0`, target resolution and queued delivery remain seams; this
checkpoint does not establish equivalence of the native dispatcher. It also
does not add padding records or redefine the original count to hide the
reference's possible extra read.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_response.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_response.py --compiler clang-cl --mutation-checks
```

Clang i686 passes **8,192 original/rebuilt comparisons**, with no controlled
callees. Fixtures include nested/inverted conditions, depth-one/nested else,
closing/unknown types, no matching delimiter, final/start-beyond-count indices,
counts 0–64, invalid command pointers on the no-read return and unsigned
count-zero/index-`0xffffffff` wrap. Extra records beyond the declared count are
mapped explicitly in the verifier. It compares exact ordered command reads,
guarded arena memory and writes, return index, stack cleanup and preserved
registers. These finite mapped-storage tests do not prove arbitrary bad pointers,
index/depth overflow or caller dispatch.

Six mutations are rejected: restoring the old native count-1 loop bound,
skipping reads for every index past the final record, letting else close nested
depth, ignoring inverted conditions, omitting close-depth decrement and reading
the current record instead of the next. Legacy MSVC 7.1/wibo and byte matching
remain outstanding. Reports stay ignored under `build/analysis/feng-response/`.


Native and mod-enabled builds pass. Headless checks pass all 19 settings and
10 Controls scenarios, slider fill/knob pixels and scroller motion/settled
centring with the recovered helper. These are integration checks of the current
native dispatcher and decoded storage, separate from original/rebuilt evidence.


## GUID lookup and native tree adapters (2026-10-18)

`FEngLookup.cpp` now recovers GUID visitor `0x5048f0` (25 bytes) and package
resolver `0x517fe0` (81 bytes). Both fingerprints are added to
`config/SPEED2_EXE_1_2/feng-lookup.json`. The query layout matches the observed
visitor ABI: vptr +0, requested GUID +4, found object +8. Original vtable
`0x79aafc`, slot +4, points to the GUID callback. It compares object **+0xc**,
independently of name hash +0x10, stores the first matching object and returns
false to stop traversal. A direct visitor miss leaves the existing result field
untouched. The package wrapper initializes result to null, calls recovered
`VisitObjects` `0x504880` and returns that result.

Traversal retains callback-before-recursion order, group type 5 recursion,
sibling reloads and early return. Duplicate GUIDs therefore select the first
object in tree order, not necessarily the first record in a flat decoded vector.
The original wrapper establishes a temporary query's SEH lifetime frame; normal
return restores the FS chain. Exception delivery/unwind is outside this evidence.

Native `FengPackage::FindGuid` no longer scans `objects_`. It constructs the
observed package prefix over `FirstRoot()` and calls recovered GUID resolution.
Native `FengPackage::Find` now similarly calls the already-recovered hash
resolver `0x517f80`; its flat-vector substitute is also removed. Loader ownership,
root/child chain construction and temporary package-prefix construction remain
native scaffolding. Parent linking during loading remains a separate native
flat scan; renderer iteration over decoded storage is not replaced by this
checkpoint. Full response dispatch `0x52c7c0` and message delivery remain seams.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compose-formatting --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lookup.py --compiler clang-cl --compose-formatting --mutation-checks
```

The composed Clang i686 suite passes **19,520 original/rebuilt comparisons**,
including 1,024 direct GUID callbacks and 1,024 full GUID resolver calls. GUID
fixtures include empty trees, misses, duplicate targets, nested groups, zero,
`0xffffffff` and `0x80000000`, with independent name hashes. Duplicate child and
sibling GUIDs exercise a tree order different from the node-vector order.
Guarded arena memory, return pointers/AL, callback order/query state, stack cleanup,
preserved registers and normal-return FS chain restoration are compared. The
full GUID resolver uses actual recovered traversal and visitor calls with no
controlled GUID callees.

All **21 mutations** are rejected: the previous seventeen plus GUID comparison
against the name-hash field, continuing after a GUID match, using a name-hash
query in the GUID resolver and returning null despite a found GUID. The broader
composition still controls the visitor only in direct walk fixtures, formatter
core `0x43e950` and clock import; actual formatter-core verification is recorded
separately above. Finite acyclic mapped trees do not prove arbitrary pointers,
cycles, full native loader topology or dispatcher equivalence. MSVC 7.1/wibo,
exception unwind and byte matching remain outstanding. Reports stay ignored
under `build/analysis/feng-lookup/formatting-composition.json`.


Native and mod-enabled builds pass. Headless integration passes all 19 settings
and 10 Controls scenarios, slider fill/knob pixels and scroller motion/settled
centring. Mod HUD checks pass all four aspect ratios, 11 skins and the 120-frame
capture. These cover current demo integration with recovered GUID/hash adapters,
not complete loader or dispatcher equivalence.


## Response dispatch and queue state (2026-10-18)

`FEngResponse.cpp` now recovers full command dispatcher `0x52c7c0` (935-byte
span through `0x52cb66`, with two fingerprinted jump tables).
`FEngQueueState.cpp` recovers script lookup `0x5046b0` (31 bytes), focus store
`0x50b300` (92), focus lookup `0x50b360` (46) and package-stack reset `0x50b390`
(38). `feng-response.json` fingerprints these functions and tables. Queue
fields retain offset-based names: byte +1, stack depth +0x4148, 16 256-byte
slots at +0x414c, signed insertion index +0x514c and 32 key/value pairs at
+0x5150. These are an observed prefix, not a recovered complete queue object.

Dispatch caches the initial response count and reloads its command pointer on
each iteration. Script command 0 calls first-match script lookup, then
`SetScript` `0x5178b0` **even when already selected**, and resets the local
script's time after that call. It does not use the hash selector's distinct
already-selected shortcut. Message command 1 preserves target sentinels -4/-1,
resolves other targets by GUID, and posts even when resolution returns null.
Commands 2/3 post to -1/-5. Focus selection 0x100 and saved-focus fallback 0x103
retain parameter reloads after lookup callbacks and the original notify byte;
0x102 stores the focused GUID (or zero), keyed by package hash. Byte +1 is set
only for parameter exactly 1, not arbitrary truthy values.

Input operations 0x105–0x108 preserve on/off values and either caller mask or
0xff. Package commands 0x200–0x204 preserve their original callees and parameter
order. 0x202 visits only the low eight mask bits in ascending order, making one
call per selected bit. Stack command 0x2c1 decrements before calculating its
256-byte slot and calls the observed package boundary; 0x2c0 uses the current
package name and 0x2c2 calls recovered stack reset. Conditional commands use
recovered `SkipConditionalBlock`; unlike the previous native switch, they
require the original non-null object/current-script precondition. Other
commands, including 0x104/0x501 and unrecognized values, have no dispatch action.

Focus storage first clears **every** matching key in ascending slot order,
leaving stale values intact. It reloads the insertion index between the key and
value stores, and again before signed modulo-32 advancement. Explicit unsigned
arithmetic preserves x86 overflow and negative-index aliases without clamping.
Lookup returns the first match, including key zero. Stack reset clears only the
first byte of each of the sixteen slots, followed by the depth word; it does
not zero the string buffers or focus cache.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_dispatch.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_dispatch.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_response.py --compiler clang-cl --mutation-checks
```

Clang i686 passes **12,800 original/rebuilt comparisons**: 4,608 dispatcher
calls and 2,048 each of the four state helpers. The dispatcher executes actual
recovered conditional traversal, script lookup, focus store/load and stack
reset. Its nine other call boundaries are controlled, with full call ordering,
argument/context checks, volatile-register clobbering and selected callback
mutations. Cases cover every command family, unknown commands, target sentinels
and misses, masks, nested conditionals, count changes and pointer relocation
after callbacks, selected-script changes and focus parameter reloads. Direct
helper cases include duplicates, zero keys, index wrap, mapped negative-index
aliases (including an alias to the insertion-index word), first matching
scripts, empty lists and byte-only stack clearing. Checks compare guarded arena
memory, exact ordered writes, return values where defined, stack cleanup and
preserved registers. Controlled GUID and SetScript/focus callees are separate
from their earlier actual-callee verification; this suite does not claim their
full composition or platform package operations are recovered.

Sixteen mutations are rejected, covering cached count/pointer differences,
already-selected script skipping, resetting a changed current script instead
of the saved local pointer, sentinel/null-target handling, truthy flag writes,
mask-bit truncation, stale focus parameters, missing stack pop, unsigned focus
modulo, zero-key lookup, clearing stale values, wide slot clears, missing depth
reset and wrong script-hash field. The existing conditional suite also passes
8,192 comparisons and rejects its six mutations after linking the expanded
component. Reports remain ignored under `build/analysis/feng-dispatch/` and
`build/analysis/feng-response/`. Finite mapped lists/state and selected callbacks
do not prove arbitrary pointers, cycles, all aliases or full GUI dispatch.
MSVC 7.1/wibo and byte matching remain outstanding.

The native hand-written response switch is removed: it now adapts decoded
commands to the observed prefix and calls the recovered dispatcher. Its mask
is a documented native 0xff value; original per-message input-mask propagation
remains unresolved. Queue delivery still uses native messages; the adapter now
routes -1/-3/-5 to game/package/input destinations. Target -4 and original package
operations `0x51af50`, `0x51ab20`, `0x51ac10`, `0x51adb0`, `0x51b070` remain
**explicit traps**, not invented implementations. Current headless demo
scenarios must not be treated as coverage of these operations.

Integration exposed a native loader defect: it applied Init track values but
left `currentScript54` null. Original script reader `0x5515e0` selects hash
`0x1744b3`; package reader `0x5554a0` searches/selects Init, initializes tracks
and resets its time. The native loader adapter now performs that observed setup
using recovered script lookup/initialization. This fixes the original
conditional precondition rather than adding null guards to recovered dispatch.
The loader itself, Init event-delivery timing and complete package lifecycle
remain scaffolding/unverified; this is not recovery of either reader.


Native and mod-enabled builds pass. After restoring the observed Init selection,
headless integration passes all 19 settings and 10 Controls scenarios, slider
fill/knob pixels, scroller motion/settled centring and all four HUD aspect
ratios/11 skins/120-frame capture. The initial null-selection crash is resolved
by loader initialization; recovered conditionals retain their original
preconditions. These checks cover current demo scenarios, not the trapped
package operations, complete queue delivery or all original scripts.


## Message nodes and queue insertion (2026-10-18)

`FEngQueueNodes.cpp` recovers message posting `0x51aa20` (206 bytes), input
operation posting `0x51af50` (176) and list removal `0x505400` (75).
`feng-queue-nodes.json` fingerprints these spans and the two original node-vtable
slots at `0x79ab60`/`0x79ab64`. The observed 32-byte node has vptr +0, next +4,
previous +8 and five payload words +0xc through +0x1c. The 16-byte list prefix
has unknown +0, count +4, first +8 and insertion cursor +0xc. Queue message and
operation lists are at +0x4114/+0x4124; observer pointer +0x10c and enable byte
+0x5251 are separate from the previously recovered focus/stack fields.

Message allocation calls original boundary `0x575620` with size 32. The inline
constructor stores link sentinels `0xabadcafe` and vptr `0x79ab60`. Payload writes
occur in original order: package +0x14, message +0x18, sender +0xc, target +0x10,
mask +0x1c. A nonzero observer byte triggers virtual slot +0x54 after payload
construction and before list reads/insertion. The observer receives all five
original arguments. The byte is read after allocation, and the insertion cursor
is read after the observer returns; neither is cached before its callback.

A non-null cursor inserts the node after that cursor, preserving its existing
next node; a null cursor prepends to the first pointer. Neighbor links are
written in the original order, the cursor is conditionally updated after
linking and count increments last. This is not always a simple tail append.
Input posting uses vptr `0x79ab64`, zeros payload +0xc/+0x10, stores package at
+0x1c, stores `package.inputMask1C & mask` at +0x18 and opcode 4/8 at +0x14.
Only the enable argument's low byte selects that opcode. It inserts into the
**separate operation list** and has no message-observer callback.

Removal is null-safe. It updates matching list endpoints and both neighboring
links, marks the removed node's links `0xabadcafe`, then decrements the wrapping
count. Unknown prefix fields remain offset-named. This does not recover the
list constructor, complete queue type, allocation runtime or node destruction.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_queue_nodes.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_queue_nodes.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_dispatch.py --compiler clang-cl --compose-queue --mutation-checks
```

Clang i686 passes **12,288 node comparisons**, 4,096 per routine, comparing
exact ordered arena writes, guarded memory, allocation/observer calls, payload
at callback entry, stack cleanup and preserved registers. Fixtures cover empty
and nonempty mapped lists, middle/end cursors, null and interior removals,
wrapping counts, noncanonical observer bytes, allocation callbacks enabling the
observer, observer changes to cursor/first/count, enable-byte truncation and
full-width mask filtering. Allocation and observer are controlled; node
construction/insertion/removal execute actual recovered code. Eleven mutations
are rejected: cursor cached before observer, observer before payload, skipped
observer, word-wide enable, ignored package mask, wrong node vptr, omitted
message/input count increments, omitted neighbor backlink, wrong sentinels and
omitted remove decrement.

`--compose-queue` also passes **12,800 dispatcher/state comparisons** and rejects
all sixteen existing dispatcher mutations. It executes recovered Post and input
posting directly, controlling allocation instead of those two call boundaries.
Other dispatcher callees remain controlled, and the observer is disabled in
this composition (covered separately by the node suite). Normal and composed
reports are distinct; results remain ignored under `build/analysis/`.
Allocation failure, arbitrary aliases/pointers/cycles, original virtual node
destruction, MSVC 7.1/wibo and byte matching remain unverified. Finite ordered
write tests do not establish complete queue or GUI equivalence.

The native `std::vector<PendingMessage>` queue and native `FEngQueue::Post`
implementation are removed. Both native producers and recovered callers now
allocate/enqueue through recovered code. Native Pump and screen cancellation
unlink through recovered Remove. A native delivery bridge translates node
payloads to the existing screen/object handlers and frees nodes after delivery;
it retains the current 256-message guard, routing and cancellation seams.
The allocation boundary uses native `malloc`, and raw `free` replaces the
unrecovered virtual node destruction. Original vptr values remain data tags in
these nodes; native code does not invoke the mapped retail destructor table.
Input **enqueueing** no longer traps, but pending input-operation **delivery**
explicitly traps until its original consumer is recovered. Target -4 delivery
and four other package-operation callees still trap. Full delivery `0x541fc0`,
owner callbacks, observer delivery slot +0x58 and per-message mask propagation
remain next targets; the delivery bridge is not a recovered implementation.


Native and mod-enabled builds pass with recovered queue nodes. Headless
integration passes all 19 settings and 10 Controls scenarios, slider fill/knob
pixel assertions and scroller motion/settled centring. These exercise the native
delivery bridge and current demo messages; they do not validate the trapped
operation consumer, target -4, original destructors or complete delivery.


## Queue node ownership (2026-10-18)

`FEngQueueNodes.cpp` additionally recovers message deleting destructor
`0x51aaf0` (30 bytes), operation deleting destructor `0x51abe0` (30), message
base cleanup `0x51ab10` (7), operation tail thunk `0x51ac00` (5) and owned-payload
base cleanup `0x505350` (81). Their fingerprints extend the existing node
manifest. Original vtable slots `0x79ab60`/`0x79ab64` identify the two deleting
entry points; decoding the original bytes resolves their exact helper calls.

Message base cleanup only writes vptr `0x79a9e8`; it does not free sender +0xc
or change links/payload. Operation base cleanup first writes vptr `0x79ab0c`,
conditionally deallocates non-null payload +0xc through `0x575630`, then writes
vptr `0x79a9e8`. The operation thunk retains its own boundary and tail-calls
that base cleanup. Both deleting wrappers execute their respective base/helper
before testing flag bit zero, optionally deallocate the node through the same
boundary and return the original node pointer. Other flag bits alone do not
free the node. Payload cleanup occurs even with a zero deleting flag; it does
not clear the stored payload pointer.

The expanded node verifier passes **32,768 Clang i686 comparisons**: 4,096 for
each of eight routines. It executes the actual recovered destructor helpers,
compares their call order, exact vptr/link/payload writes, guarded memory, free
calls and callback-visible node bytes, return pointers, stack cleanup and
preserved registers. Original owned-payload cleanup establishes an SEH frame;
normal-return FS chain restoration is checked at every fixture. Exception
unwind, throwing deallocation, invalid pointers, arbitrary aliases and actual
allocator internals remain unverified. Deallocation is controlled, clobbers
volatile registers and observes the node before returning. Flags include
0/1/2/3/128/255/0x100/0x101/0xffffffff, and payloads include null/non-null.

All **17 mutations** are rejected: the previous eleven plus truthy instead of
bit-zero deleting flags, omitted message base cleanup, omitted owned-payload
free, freeing a null payload, wrong intermediate operation vptr and wrong final
vptr. The same `verify_pc_feng_queue_nodes.py --compiler clang-cl
--mutation-checks` command runs these cases. Reports remain ignored under
`build/analysis/feng-queue-nodes/`. MSVC 7.1/wibo and byte matching remain pending;
normal-return source recovery does not establish original exception handling.

Native delivery and screen-cancellation adapters now call recovered
`DestroyMessage(1)` after unlinking instead of raw `free`. They know these are
message nodes; original dynamic vtable dispatch is still not implemented by
this bridge. The deallocation boundary uses native `free` as a platform adapter.
Operation destruction is recovered/verified but has no native consumer yet;
queued operation delivery still traps. Message delivery, cancellation policy,
virtual dispatch, allocator behavior and the fixed native mask remain seams.

Static consumer tracing now identifies **`0x5559c0`**, called by frame manager
`0x558c00` after message delivery and before a second message-drain pass. It
inlines operation unlinking, processes flags 1/2/4/8/0x10, updates package parent
links +0x3c and input masks, posts target -4 notifications, and invokes virtual
node destruction. This is a lead, not recovered or behaviorally verified code.
Do not substitute only the currently queued 4/8 cases for the full routine.


Native and mod-enabled builds pass. Headless integration passes all 19 settings
and 10 Controls scenarios, slider fill/knob pixels and scroller motion/settled
centring with recovered message destruction. The 12,800 composed-dispatch
comparisons also pass after linking the expanded ownership component. These
checks do not execute original exception unwind, dynamic virtual deletion or
the pending input-operation consumer.


## Full queued-operation consumer (2026-10-18)

`FEngOperations.cpp` recovers **all flag paths** of `0x5559c0`, an 826-byte span
through `0x555cf9`. `feng-operations.json` fingerprints that body, actual string
comparison `0x43dcc0` and the recovered retail diagnostic `0x50d510`. The
consumer composes actual message enqueueing and operation deleting-destructor
helpers; it does not implement only the currently generated 4/8 opcodes.
Observed layouts add package signed level +0x18, parent link +0x3c and the
manager's circular registry sentinel +0x10. Registry records have next +0,
previous +4 and name +8; the sentinel's name word is not used or identified.
Unknown fields remain offset-based; these are partial UG2 prefixes.

The consumer inlines operation unlinking before reading payload state, retaining
original node-link/count write order and the absence of an extra Remove call.
It reloads the queue head after deletion, so callbacks can enqueue more work.
A package's signed level is cached before lifecycle operations. Flag 1 with a
nonnegative level validates the parent against live packages, diagnoses/clears a
stale parent, repairs child links or saves replacement state, restores parent
masks, calls original removal boundary `0x5362e0` and decrements the cached level.
The saved package/parent pair persists across operation nodes, including later
replacement flags whose negative level bypasses removal.

Flag 2 searches the circular registry with actual case-insensitive comparison.
On a match it calls load boundary `0x555860` with name, wrapped low-byte
`level+1` and mask. It preserves parent/child replacement and selective mask
writes. Flag 0x10 updates other packages' temporary mask +0x24 even when loading
returns null; it is not conditional on a successful result. Unmatched names do
not load. Flags 4 and 8 transfer only currently owned requested mask bits to a
parent or the **first** matching child, writing all four masks in original
order. They post `0x334c5493` to target -4 with the original requested mask,
including a zero mask. Flag/mask fields are reloaded across callbacks. Every
consumed operation invokes deleting slot +0 with flag 1 after processing.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_operations.py --mutation-checks
# Explicit fallback when MSVC 7.1/wibo is absent:
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_operations.py --compiler clang-cl --mutation-checks
cmake --build build/native --target ug2fengqueue_native_test
ctest --test-dir build/native -R '^feng_queue_native$' --output-on-failure
```

Clang i686 passes **4,097 original/rebuilt comparisons** over complete queued
sequences. The verifier compares exact ordered writes, guarded arena memory,
callee arguments/order, destructor/helper calls, stack cleanup, preserved
registers and normal-return FS restoration. Actual recovered Post, string
comparison, diagnostic and operation destruction execute directly. Loading,
removal, allocation, deallocation and the selected Post observer are controlled.
Fixtures include empty/multi-node queues, every low-five-bit flag combination,
extra high bits, signed level extremes/byte wrap, successful/failed/missing
loads, stale/self/mapped parent aliases, replacement state across nodes,
selective mask writes, zero-mask notifications and callbacks changing flags,
masks or the pending queue. The controlled loader keeps live chains acyclic;
it does not model real allocation, package reader or ownership behavior.

Every tracked major original block must be reached: unlink/remove/stale-parent,
child repair, saved replacement, parent/all-mask restoration, registry misses,
loading, attaching/replacing parents and children, exclusive-mask writes,
parent/child transfers and both deleting-call sites. The report separately
records successful and failed loads. This is a finite coverage gate for named
blocks, not proof of complete branch combinations or arbitrary state.

All **18 mutations** are rejected, including skipped lifecycle/transfer paths,
retained stale parents, lost cross-node saved state, inverted registry matching,
wrong level advancement, extra mask writes, skipping exclusive writes on load
failure, wrong transferred mask, omitted zero-mask notification/count/deletion,
an extra Remove boundary, and stale queue head/flag/mask caches across callbacks.
Reports remain ignored under `build/analysis/feng-operations/`. MSVC 7.1/wibo are
unavailable; byte matching, exception unwind, real package lifecycle/allocator
composition, arbitrary pointers/aliases and malformed cycles remain unverified.

The **asset-free native CTest probe** composes real enqueue/consume, parent and
first-child transfers, notification construction, zero-mask notification and
recovered destruction with actual Linux allocation/freeing. Assertions remain
enabled in release builds. It maps one private data page for the original
operation vtable slot and uses a native `thiscall` adapter to the recovered
deleting body. It checks masks, untouched later children, list/count state,
notification payloads and balanced allocation/free counts; it needs no retail
executable/assets, SDL/OpenGL or display server. CTest is now available in the
ordinary native build, independently of the modding option.

The native operation-consumer trap is removed. Native Pump now follows the
observed `0x558c00` message/operations/message phase order, retaining native
message-routing, cancellation and 256-message guards. The operation vtable slot
is bound to a native ABI adapter calling recovered deletion. This binding
changes mapped process data, not the player's executable. Native registry
sentinel initialization currently supplies an **empty** registry; original
registration/storage is a data seam. Package load/remove boundaries still trap,
as do four other package-operation enqueue boundaries. Target -4 delivery still
traps in the message bridge: consumer recovery does not establish delivery of
its notifications. The CTest probe inspects/unlinks these messages directly;
current retail menu checks do not cover all lifecycle operations. Full message
delivery `0x541fc0` and object delivery `0x5367e0` remain next.


Native and mod-enabled builds pass. CTest `feng_queue_native` passes without
assets or a display. Headless integration also passes all 19 settings and 10
Controls scenarios, slider fill/knob pixel checks and scroller motion/settled
centring. Those retail scenarios cover current menu behavior; the asset-free
probe specifically exercises nonempty mask-transfer queues. Neither establishes
original target -4 delivery, package reader/removal or registry construction.

## Full queued-message delivery (2026-10-18)

`FEngDelivery.cpp` recovers message consumer **0x541fc0** (887 bytes), object
response delivery **0x5367e0** (109 bytes), and the type 4/type 6 directional
wrappers **0x51b000 / 0x52c750** (111 bytes each). Fingerprints are recorded in
`config/SPEED2_EXE_1_2/feng-delivery.json`. This supersedes the native delivery
bridge and target -4 limitations of the preceding checkpoints.

The consumer calls `Remove` for the first message, uses the original inline
unlink for subsequent messages, and dispatches virtual deleting slot zero with
flag 1. It rereads the queue head after deletion, allowing callbacks to append
messages during the drain. Observer slot **+0x58** receives
`(message, target, sender, package, mask)` before routing fields are read.

| Target | Original routing |
|---|---|
| 0 | Every live package: first matching package response, then first matching subscription row's objects |
| -4 | Same package/objects route, once, only if the specified package is live |
| -3 | Specified package responses only, without a live-membership check |
| -2 | Every live package's responses only |
| -1 / -5 | Owner slots +0x18 / +0x1c, with `(message, sender, mask, package)` |
| -6 | Hash 0x59bed120 sets queue byte +1; 0x5d4ce32d clears it |
| Other | Direct object delivery |

Package responses use the message and mask read before execution. Subscription
lookup reads the message/table/count after the package callback. Object fanout
caches the selected row's count and message, but rereads its object-array pointer
and the node mask on each iteration. Live package links are read after callbacks.
Duplicate response/message rows use the first match. Object delivery independently
checks type 4 and type 6, then scans responses at object +0x3c. The four directional
hashes call 0x516e90 or 0x52bd70 with signed unit-axis deltas; only the returned AL
is defined and compared. These underlying mutators still trap in the native demo.

Original reader **0x5184e0** establishes `Targ` data provenance: `Tc` declares a
row count; each `Mt` supplies a message DWORD followed by GUIDs. GUID lookup
results, including null, append in file order. Rows have message/capacity/count/
objects at +0/+4/+8/+0xc; FEPackage stores count/table at +0x6c/+0x70. The native
loader now projects valid-file `Mt` rows and linked package/object response
prefixes, resolving GUIDs through recovered lookup. It does not recover original
allocation, capacity policy, malformed input handling, response inheritance,
`Rs`/`MC` command semantics or original parser ownership. It ignores `Tc` capacity
metadata. Constructor 0x4fe910 confirms all three command DWORDs initially zero;
0x517600 is command cleanup, not its constructor.

Validation command:

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_delivery.py --compiler clang-cl --mutation-checks
```

The verifier compares **32,768** original/rebuilt invocations: 8,192 each of the
consumer, object delivery and both directional wrappers. It checks complete
mapped arena state, ordered writes, call tuples, stack cleanup and preserved
registers, with byte returns for the wrappers. Its coverage gate observes every
named target route and both subscription fanout blocks in original execution.
Cases include empty/nonempty queues, live/nonlive packages, duplicate matches,
raw masks, observer mutation, package response mutation, subscription-array/count
changes and callback-generated messages. Actual object delivery, directional
wrappers, unlink and deleting helpers execute together. `ExecuteResponse`, owner/
observer callbacks, navigation mutators and freeing are controlled interfaces.
The existing dispatcher verifier covers response execution separately; this run
does not establish arbitrary composition with every command/lifecycle callback.
All **22 mutations** are rejected by state or call-order mismatches, including
wrong observer arguments, omitted package/owner/object routes, skipped live
membership, extra package fanout, fresh count/message versus cached fanout
state, stale masks, wrong internal state, wrong deletion flags, repeated unlink
calls and directional deltas. Compilation failures do not count as rejections.
Reports and mutation logs are ignored under `build/analysis/feng-delivery/`.
MSVC 7.1/wibo remain unavailable; this is behavioral verification using Clang,
not byte matching. Arbitrary aliases, malformed cycles and exception unwinding
remain outside the finite evidence.

Native Pump now calls the recovered consumer in both message phases around the
recovered operation consumer. Its manual response routing and fixed response-mask
adapter are removed; queued masks reach recovered responses unchanged. Native
entry points still choose their own initial masks. Mapped message/operation
vtable slots bind native ABI adapters to recovered deleting bodies; the retail
file is unchanged. Native game-owner delivery still selects one of the demo's
screens, drops the owner mask, and uses its existing shown-state policy. Input
owner delivery remains a no-op. Observer callbacks remain no-ops. Deferred screen
lifecycle, cancellation, an empty registry, allocator bridges, package load/remove
and four package-operation enqueue boundaries remain seams.

The asset-free `feng_queue_native` CTest now drains the three mask-transfer
notifications through recovered target -4 routing, package responses, associated
object responses and virtual deletion. Its response executor is a recording test
boundary; it checks six callback invocations, transferred/zero masks and balanced
six allocations/frees. Native and mod-enabled builds pass. Headless integration
passes all 19 settings, 10 Controls scenarios, slider pixel checks and scroller
motion/settled centring. Modded HUD checks pass at 640x480, 1280x720, 1680x720
and 2560x720. These checks exercise current demo data and behavior;
they do not prove complete retail UI/package lifecycle recovery.

## Type 4 list navigation leaf (2026-10-18)

`FEngList.cpp` recovers the complete **0x516e90** leaf (927 bytes) as
`FEListObject::Navigate`. Its observed 0xac-byte prefix and 12-byte axis entries
are defined separately in `FEngList.h`; unidentified fields retain offset names.
`config/SPEED2_EXE_1_2/feng-list.json` pins the reference span and executable.

Bits 0x20/0x40 independently suppress busy axes, and both bits suppress the
whole update. Bit 4 selects clamp behavior. Each axis uses its own count, index,
entry array and viewport value. Negative/positive wrapping preserves the original
loops, stores and unsigned division. In particular, the negative candidate is
ultimately divided **as unsigned**; replacing this with ordinary signed modulo
changes the result. Both wrap branches initially set bit 8, including the vertical
branch. Later vertical viewport insufficiency sets bit 0x10. These asymmetries
are retained. The routine computes both target deltas, stores them, normalizes
above the observed float threshold 0.1, or snaps to target and clears motion bits.
The original parity test also normalizes an unordered length; `!(length < 0.1f)`
preserves that behavior, whereas `length >= 0.1f` does not.

Wrap-loop float stores/reloads at +0x70/+0x74/+0x94/+0x98 are explicit precision
boundaries. These fields are volatile in this recovered prefix so the compiler
cannot carry an unrounded accumulator across iterations. Wider local arithmetic
preserves the computations between field stores; the Clang verifier requests
extended x87 evaluation. An initial float-rounded length and an unrounded wrap
accumulator each produced differential failures before correction. No registers,
inline assembly, invented navigation helper or replacement algorithm is used.
Clang coalesces some nonvolatile flag writes, so ordered-write comparison excludes
only +0x5c; its complete final bytes are still compared. This is a behavioral
recovery checkpoint, not a byte match or a proof of identical instruction/store
sequences for every compiler.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_list.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_queue_native$' --output-on-failure
```

The standalone verifier passes **16,896 comparisons** and rejects **19 mutations**
by state/write mismatches; compilation errors do not count as rejections. It
executes the actual leaf bodies with **no controlled callees**. It checks arena bytes, ordered non-flag writes, preserved registers,
stack cleanup and termination. Coverage gates observe both busy axes, upper/lower
clamps, positive/negative wraps, both viewport adjustments, wrap flags, settling
and normalization in the original. Cases use valid nonempty arrays, small signed
deltas and both x87 precision control words 0x27f and 0x37f, with exact binary
fractions, random finite values and selected exceptional floating-point values.
Arbitrary invalid pointers, empty active axes, large out-of-range deltas, all NaN
payloads, alternate rounding modes and unmasked floating exceptions are outside
the evidence. MSVC 7.1/wibo and byte matching remain pending.

The asset-free native queue probe now composes recovered object delivery and the
type-4 directional wrapper with the new leaf via a **test-only prefix adapter**.
Its synthetic 0xac-byte fixture checks horizontal movement, busy-axis rejection,
vertical movement, direction normalization and early return before response
execution. Native and mod-enabled builds pass, and the expanded CTest passes
without a retail installation, display, SDL or OpenGL.

**Retail integration was pending at this checkpoint; the following section
supersedes that limitation.** Native `FengObject` extends the string
prefix and uses later bytes for native metadata; calling the leaf on it would
corrupt that metadata. The production type-4 navigation trap therefore remains.
The original factory 0x534df0 allocates 0xac bytes and calls constructor 0x5349c0;
the latter initializes flags +0x5c to 4, counts/pointers/selection/motion fields to
zero, +0x80/+0x84/+0xa4 to 1, and +0xa8 to bits 0xbab60b61. Reader 0x52b740 handles
`Ld` via 0x52a910/0x52ab30, fills `Lc`/`Lr` entries at +0/+8, and loads `Ls`, `Lv`
and `Lw`. Entry +4 initialization/update and the native storage/parser projection
still need tracing before replacing the production trap. These reader/constructor
observations are static evidence, not recovered implementations in this commit.
Type-6 0x52bd70 calls 0x518f20 twice; that helper reshuffles 0x30-byte cells and
invokes callbacks. Neither its body nor a native grid projection is recovered.

## List setup, motion and native projection (2026-10-18)

`FEngListState.cpp` adds seven original routines to the recovered type-4 list:

| Routine | Address | Full span | Behavior |
|---|---|---|---|
| `SetColumn` / `SetRow` | 0x504180 / 0x5041b0 | 45 each | Unsigned clamp; empty axis selects zero |
| `AdvanceCell` | 0x504230 | 57 | Advance columns, then rows, wrapping both |
| `SetWrap` | 0x504270 | 29 | Low-byte truth clears clamp bit 4; false sets it |
| `CalculateOffsets` | 0x504360 | 75 | Write entry +4 before accumulating entry +0, independently for each axis |
| `FinishMotion` | 0x5043b0 | 66 | Snap to targets, clear motion flags and reset zero-index offsets |
| `Tick` | 0x517230 | 221 | Bounded phase/reversal, directional motion, overshoot test and actual call to FinishMotion |

Manifest `feng-list.json` fingerprints every complete span, including RET
immediates. Reader 0x5554a0 calls 0x504360 after object records and before Init
track application. Its cumulative sum stays in x87 precision despite writing a
float at each entry +4. Tick likewise retains wide phase and position values;
its vertical step alone has an observed float stack spill before adding the
current vertical position. Dot-product overshoot uses the unrounded positions,
then calls 0x5043b0 through the original boundary. Assembly operand order
retains the vertical product before the horizontal product in sums (including
the navigation length). Source preserves these
arithmetic and call boundaries. A compiler-intrinsic spelling maps wide absolute
value to the original FABS operation; no replacement motion algorithm is added.
A phase just above 1 by less than a double ULP reverses at x87 precision 0x37f,
but not at 0x27f; the verifier checks that distinction. The legacy MSVC spelling
remains compilable but is unverified without that compiler/runtime.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_list_state.py --compiler clang-cl --mutation-checks
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_delivery.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list)_native$' --output-on-failure
```

The state verifier passes **69,632 original/rebuilt comparisons**. These include
4,096 sequences of CalculateOffsets, Navigate and eight Tick calls, comparing
state after every call, with actual FinishMotion execution and call tracing.
There are **no controlled callees**. Cases cover empty/nonempty axes, arbitrary
selection words, low-byte wrap arguments, finite random values, negative/zero/
positive elapsed values, clamp boundaries and both x87 control words. Coverage
gates hit both offset loops, selection/wrap branches, phase clamps/reversal,
motion and the finishing call. All **23 mutations** are rejected by state or
call-order mismatches; compilation failures do not count. The delivery verifier
also passes its **32,768 comparisons / 22 mutations** with the typed Navigate
call replacing the old external method name.

All mapped final bytes, preserved registers and stack cleanup are checked.
Ordered writes exclude only nonvolatile flags, selection indices and phase
+0xa4, which Clang can coalesce; their final bytes remain exact. Floating stores
at the wrap/motion positions and offset arrays remain in the write comparison.
The Clang state run uses extended evaluation and an 80-bit long-double type.
This finite result does not prove arbitrary aliasing, malformed arrays, all
floating values/rounding modes, unmasked exceptions, native frame orchestration
or byte matching. MSVC 7.1/wibo remain unavailable. Reports stay ignored under
`build/analysis/feng-list-state/`.

The **production type-4 trap is removed**. The directional handler calls the
recovered typed Navigate directly, retaining its original call boundary. Native
`FengObject` reserves the full prefix through 0xac, with native metadata beginning
at 0xac and a compile-time offset check. Constructor defaults and valid-file
`Ld`/`Lc`/`Lr`/`Ls`/`Lv`/`Lw` records supply its observed fields and owned axis
vectors. `Cc` records call recovered AdvanceCell after the first record; they
must not leave selection at the constructor's zero. Recovered setters handle
initial selection/wrapping, and CalculateOffsets runs before Init application.
Group-link projection no longer overwrites the list counts at +0x60/+0x64.

Native AdvanceScripts calls recovered Tick after script-state application,
using its existing seconds-to-milliseconds clock adapter. The original 0x54ce60
converts an integer elapsed argument; complete original frame traversal, clock
rounding and lifecycle remain seams. The native current-script guard is retained.
The original allocating constructor, dimension/cell ownership, cell payloads,
cell rendering, malformed/inherited records and the type-6 grid remain
unrecovered. The vectors/prefix projection are native scaffolding, not a claim
that the original object reader/allocator is recovered.

The queue CTest now calls the typed recovered Navigate directly, removing its
former test-only navigation adapter. New **`feng_list_native`** uses the actual
native FNG parser with synthetic dimensions, axis data, three `Cc` records and
an empty-track Init script. It checks count preservation, +4 offsets, hash/GUID
lookup through the list prefix, native metadata integrity, clamp/wrap movement
and settling through AdvanceScripts/Tick. Unexercised localization/script trap
interfaces abort if reached. Its unused rendering sections are discarded; it
links neither SDL nor OpenGL and needs no assets or display. Both native CTests
and native/mod-enabled builds pass.

Headless retail integration passes all 19 settings scenarios, slider fill/knob
pixels, all 10 Controls scenarios, scroller motion/settled centring and modded
HUD checks at 640x480, 1280x720, 1680x720 and 2560x720. Those tests check current
demo behavior; the synthetic fixture specifically covers nonempty type-4 lists.
They do not establish original reader/cell-renderer/frame-manager recovery.

## Type 6 grid navigation and original name hashing (2026-10-18)

`FEngGrid.cpp` recovers the **complete axis-shift body**, both gates, index wrap
and the two-axis wrapper directly from UG2 v1.2. `FEngHash.cpp` also recovers the
actual name-hash callee used by its boundary notifications:

| Routine | Address | Reference span |
|---|---|---|
| `FEngGridWrapIndex` | 0x504790 | 38 bytes |
| `FngHash` | 0x505450 | 47 bytes |
| `FEGridObject::CanShift` | 0x518e70 | 163 bytes |
| `FEGridObject::MoveSelection` | 0x504e40 | 347 bytes |
| `FEGridObject::ShiftAxis` | 0x518f20 | 1,933 bytes, including 9 padding bytes outside Ghidra's 1,924-byte body |
| `FEGridObject::Navigate` | 0x52bd70 | 70 bytes |

`config/SPEED2_EXE_1_2/feng-grid.json` fingerprints the complete spans and pinned
executable. The 0xc8-byte type-6 prefix, physical counts, logical dimensions,
selection/offset words, 0x30-byte cells and callback/context at +0xc0/+0xc4 are
represented in `FEngGrid.h`; unknown fields retain offsets.

A zero delta returns before reading selection pointers. Otherwise the first
gate checks flags/ranges and can report `CLB AT MIN` / `CLB AT MAX`; a full-window
rejection sends **both**, even if the first callback changes owner/flags. The
second gate updates selection/offset, with distinct coupled/windowed flag paths.
It can change the offset while returning false, which suppresses physical cell
shifting. Alias cases where both pointers address the same word are preserved.
Only AL is defined for the gate/shift return values.

Successful shifting preserves all eight positive/negative, horizontal/vertical,
custom-callback/owner branches. Whole cells are **shallow-copied**; only DWORD
+0x20 is recycled onto the exposed edge. It is not a full-record rotation. The
positive path caches its copy extent and exposed coordinate while later callbacks
can change fields used by addressing and loop bounds. Cell-buffer, callback,
context and owner reads retain the original reload behavior. The negative-vertical
refill uses the same base column for each column iteration; no `+column` fix is
introduced. Negative exact-multiple wrapping likewise returns the total rather
than zero; `INT_MIN` negation retains the original 32-bit wrap. Calls to 0x504790
and its inlined arithmetic remain separate where the binary distinguishes them.
Navigate reads the second axis's dimensions after first-axis callbacks.

The owner identity is grounded in a concrete pointer chain: queue wrapper
**0x555800** pushes queue +0x10c into reader **0x555720**; the reader stores it at
+0x38; factory **0x534df0** copies that pointer into type-6 object +0x5c. Thus the
grid uses the same observed owner interface as queue delivery. Its virtual
**+0x4c** has three stack arguments `(grid, column, row)`; its custom callback
has four cdecl arguments `(context, grid, column, row)`. The queue observer's
formerly unidentified +0x4c declaration is now typed `RefillGrid`. This establishes
the pointer/slot contract, not the original owning class's implementation or
an identity shared with other titles.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_grid.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

The verifier passes **24,922 original/rebuilt comparisons** across all six
routines, including **30 rejected mutations**. Actual hash, wrap, both gates
and full shift bodies execute together; only owner/custom interfaces are
controlled. It checks final mapped bytes, full arena snapshots at each callback,
ordered call targets/arguments, return bytes or words, preserved registers and
stack cleanup. Callback targets, owner pointers, contexts, cell buffers, physical
counts, coordinates, logical totals and cell payloads change under controlled
callbacks. Cases include physical sizes 1..4, logical totals larger/equal/smaller
than windows, aliases of selection/offset pointers, zero-delta null pointers,
selected non-dividing zero-total paths, low-byte horizontal flags, signed extrema,
null/empty strings and random high bytes. Coverage gates hit all eight refill
paths and both boundary-notification sites. Compilation/link failures do not count
as mutation rejections; a fault in rebuilt execution is rejected only after the
original completes the same mapped input.

This is finite behavioral evidence. Record-copy store scheduling may differ, so
individual DWORD store order is not claimed; callback snapshots and complete final
bytes are compared. Arbitrary memory aliases, invalid grids/divisors, unbounded
callback reconfiguration, exceptions/unwind and all state combinations remain
unverified. MSVC 7.1/wibo are absent; byte matching remains pending. Reports stay
ignored under `build/analysis/feng-grid/`.

Native CTest **`feng_grid_native`** composes the recovered two-axis wrapper,
gates and copies with real native virtual/cdecl callbacks. It checks recycled
payloads versus retained edge metadata, the negative-vertical coordinate asymmetry,
custom refill selection, owner changes between boundary messages, signed-wrap
quirks and name hashing. Its two fixed-address strings and cell data are synthetic;
it requires no retail files, display, SDL or OpenGL. Existing queue/list probes
also pass.

**Native name hashing now executes recovered code.** The old NativePause FngHash
implementation and NativeFng hashing loop are removed; the remaining FengHash
name is a thin platform naming adapter. Null returns 0xffffffff; ASCII lowercase
folding, unsigned high bytes and 32-bit arithmetic follow original execution.
The hash header is shared by the recovered frontend and native loader.

**Production type-6 activation remains pending.** Native FengObject still reserves
a 0xac-byte prefix for type 4; its later metadata cannot be treated as the 0xc8-byte
grid. Moreover, post-reader **0x5554a0** invokes initial refill **0x504c40** before
Init application. Native `RefillGrid` remains an explicit trap; no substitute cell
populator is invented and the existing type-6 navigation trap is retained. The
native owner's null-package boundary-message handling is also unverified for grids.

Next dependencies have concrete original leads: reader **0x535060** handles
uppercase `LD` via **0x52bb30** (physical cells), `SI` via **0x518c60** (string-pool
storage, not logical dimensions), `LT` flags and `Lv` at +0x84/+0x88. Dimension setup
calls **0x518bb0 / 0x518c00** for logical counts; `Cc`/`Cs`/`Lj` use **0x5196b0 /
0x5197d0 / 0x5198f0**. Original allocation, initial fill, cell ownership/rendering,
type-6 ticking and the owner implementations still require recovery. The earlier
statements that the entire type-6 body was unrecovered are superseded by this
checkpoint; these data/lifecycle seams remain.

Native and mod-enabled builds pass. Headless integration also passes all 19
settings cases (including slider fill/knob pixels), all 10 Controls cases,
scroller motion/settled centering, and modded HUD checks at 640x480, 1280x720,
1680x720 and 2560x720 with recovered hashing active. These checks cover the
current demo; the asset-free grid fixture covers the recovered grid component.

## Type 6 initial fill (2026-10-18)

`FEGridObject::InitialFill` now recovers the complete **0x504c40..0x504db1**
body (370 bytes). This supersedes the unrecovered-initial-fill limitation above;
production grid allocation, reader/owner binding and owner implementations still
remain. Post-reader `0x5554a0` calls it at `0x55562c`, then overwrites EAX at
`0x555631`. It is represented as a void thiscall method: incidental EAX contents
vary across exits and are not claimed as a recovered return contract.

The original checks logical columns, logical rows, physical rows and physical
columns for zero in that order. It snapshots unsigned `min(logical, physical)`
extents and starting +0x74/+0x78 coordinates, then chooses the custom-callback
branch or owner branch once. Each row restarts at the original column; initial
coordinates are passed raw. Every refill reloads custom +0xc0/context +0xc4 or
owner +0x5c/virtual +0x4c. After every callback, including the last column of the
last row, the next column wraps using the current logical +0x6c. Each completed
row likewise wraps using current +0x70. These are inline signed divisions,
including wrapped increment/negation and the negative exact-multiple quirk;
they do not call `FEngGridWrapIndex`. Callback changes to extents, origins or
branch choice do not change the cached loop counts/origins/branch.

The existing grid verifier now executes this body alongside the six previously
recovered routines. The baseline passes **33,434 comparisons**, including
**8,512 initial-fill cases**: both dispatch paths, neither interface, logical
dimensions below/equal/above physical dimensions, all four empty guards with
invalid callback/owner pointers (including an unmapped callback-field boundary
for empty physical columns), raw negative/extreme origins, unsigned large
logical dimensions, and controlled callback changes to counts, origins, logical
totals, targets, context, owner, buffers and payloads. Ordered callback snapshots,
final mapped bytes, preserved registers and stack balance are compared. Return
EAX is ignored only for the two void routines; all actual hash/wrap/gate return
contracts remain checked. Original block counters cover both initial-fill
callback sites. The manifest fingerprints the complete new span.

The combined run rejects **40 semantic mutations**, including **10 new fill
mutations**: premature origin normalization, live origins/extents, extra wrap
calls, cached owner/custom dispatch, omitted logical clipping and omitted empty
column guarding. Navigation mutations are scoped to the navigation portion of
the source so similar fill expressions do not introduce compilation failures.
Compilation/link failures remain excluded from accepted mutation rejections.

Native CTest `feng_grid_native` also calls the recovered fill directly through
the shared native owner interface and custom cdecl callback. Its synthetic cases
check row-major coordinates, logical clipping, wrapping and empty physical
columns. All three queue/list/grid CTests and native/mod-enabled builds pass.
The headless pause/scroller smoke check also passes motion and settled centering
with this checkpoint linked.
These component tests do not activate the production grid reader or establish
its data ownership/rendering. Finite coverage, exception/unwind, arbitrary
aliases and byte-matching limitations from the previous section still apply;
verification uses Clang because MSVC 7.1/wibo remain absent.

## Type 6 cell storage and dimension setup (2026-10-18)

`FEngGridStorage.cpp` recovers four complete UG2 v1.2 bodies:

| Routine | Address | Reference span |
|---|---|---|
| `FEGridObject::SetLogicalColumns` | 0x518bb0 | 79 bytes |
| `FEGridObject::SetLogicalRows` | 0x518c00 | 82 bytes |
| `FEngGridResetCells` | 0x504290 | 79 bytes |
| `FEGridObject::ResizeCells` | 0x52bb30 | 573 bytes |

`config/SPEED2_EXE_1_2/feng-grid-storage.json` pins all four spans and the reference
executable. The two setters store logical totals, locally clamp selection
+0x7c/+0x80 without changing those selection fields, and write origins +0x74/+0x78.
Flag 8 subtracts half the physical count (logical unsigned shift) and applies
inline signed wrapping. Negative exact multiples still return the total. Their
observed allocation caller discards EAX; void methods preserve the observed
contract without manufacturing quotient register values. Centered zero totals
can divide by zero in the original; no new guard is added.

Resize snapshots the old buffer/counts, stores new physical dimensions, allocates
`columns * rows * 0x30` bytes at boundary **0x575620**, and partially constructs
cells before calling the actual full reset body. The partial initialization
leaves +0x1c/+0x28/+0x2c untouched and gives +0x24 a -1 value; the subsequent
reset writes every DWORD, including +0x24 = 0 and +0x28/+0x2c = float 1 bits.
Reset count and setter arguments reload physical dimensions after allocation.
Both setter bodies execute as actual callees.

When preexisting flag 1 allows ownership handling and the old buffer is nonnull,
each retained row copies **new physical width times 0x30 bytes**, not the minimum
old/new column extent. This can consume padded bytes beyond the old row when
expanding; that behavior is retained. The source represents the original inline
`REP MOVSD` as a forward DWORD copy with the same wrapped byte count; no synthetic
memcpy call boundary is inserted. Removed columns and rows whose cell +0x18 is
2 return DWORD +0x20 into `value9C[--value98]`. Old storage is freed through
**0x575630** only in the ownership path; flag 1 is set afterward, reloading flags
following the free call. Previously unknown +0x98/+0x9c now have explicit
word/pointer fields; other pool fields remain offset-named and unclaimed.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_grid_storage.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

The verifier passes **16,448 original/rebuilt comparisons** (4,096 per setter,
4,096 reset cases and 4,160 resize cases) and rejects **16 mutations**. Only
allocation/deallocation are controlled. Helper-entry snapshots prove partial
construction and call ordering before reset overwrites those words. Full mapped
bytes, ordered allocation/free/helper arguments, stack cleanup and preserved
registers are compared. Padded buffers cover physical sizes 1..6, expansion,
shrinkage, ownership bits, null old buffers, zero-sized null allocation and empty
reset with a null pointer. Setters include signed/unsigned total extrema and
small-total negative paths. Allocation can change physical counts, selections,
current buffer, ownership flags and pool pointers/count; free can change flags.
The tests preserve snapshots versus reloads and payload-return order across
those changes. Coverage gates hit positive/negative/small-total setter paths,
partial construction, retained-row copying, both retirement loops and old free.
Compile/link failures do not count as mutation rejections.

Native CTest `feng_grid_native` now composes recovered allocation, reset, setters,
initial fill, navigation and shrink with real native virtual callbacks. It checks
initial cell words, centering, fill coordinates, payload retirement order and old
buffer free. Its cells/pool data are synthetic. Native malloc/free adapters in
`NativePause.cpp` implement the two platform allocation seams; original heap
implementation and allocator failure with nonzero count are unverified. Native
and mod-enabled builds, all three queue/list/grid CTests, and headless scroller
motion/settled-centering checks pass.

This is bounded behavioral evidence, using Clang because MSVC 7.1/wibo remain
absent. Arbitrary aliases, unpadded expansion, overflow-sized allocations,
nonzero allocation failure, exceptions/unwind and byte matching remain unverified.
Reports remain ignored in `build/analysis/feng-grid-storage/`. Production type-6
constructor, reader, string pool setup, cell rendering/ticking and owner callback
implementations still need recovery/binding. The native type-6 entry stays an
explicit trap; this checkpoint supplies storage dependencies without inventing
an alternate cell population or ownership implementation.

## Type 6 string-pool migration and cell properties (2026-10-18)

`FEngGridCells.cpp` recovers four more complete UG2 v1.2 bodies, fingerprinted
with the executable in `config/SPEED2_EXE_1_2/feng-grid-cells.json`:

| Routine | Address | Reference span |
|---|---|---|
| `FEGridObject::ResizeStringPool` | 0x518c60 | 446 bytes |
| `FEGridObject::SetCellValue0` | 0x5196b0 | 273 bytes |
| `FEGridObject::SetCellPair4` | 0x5197d0 | 284 bytes |
| `FEGridObject::SetCellValue1C` | 0x5198f0 | 274 bytes |

Pool resize snapshots old +0xa0/+0x9c, then clears +0x90/+0x98/+0x94/+0xa0/+0x9c.
Nonzero count/capacity allocate `count * capacity * 2` bytes and `count * 4`
pointer bytes through the existing **0x575620** seam. Original inline DWORD/tail
zeroing and per-slot address construction are represented directly; capacity is
in 16-bit units. Current storage fields reload after allocation callbacks.
Count/capacity are set after allocation/table construction. When both old pointers
were nonnull, each physical type-2 cell obtains the next `value9C[value98++]`
slot, receives a new payload pointer, and copies at most capacity-minus-one
16-bit units from its old payload before adding a zero terminator. Null old
payloads become empty strings. Physical dimensions and cell buffers reload during
migration. Old data and old table free in that order, using their snapshots even
if allocation/free callbacks change current fields.

Zero count or capacity disables the pool and frees old allocations. It does
**not** clear cell payload pointers. No new guard, placeholder text or substitute
pool ownership policy is added. This supersedes the previously unrecovered pool
setup body, while production reader binding and cell text-assignment routines
remain pending.

The three property setters apply a rectangular unsigned start/end traversal.
They inline the same projection twice per cell: conditionally reduce signed
coordinates by logical dimensions, subtract origins, add the logical dimension
once for negative differences, and wrap against physical dimensions. A zero
logical dimension projects to -1 rather than suppressing the write. Negative
exact multiples retain their existing wrap quirk; wrapped end bounds can suppress
all writes. They set cell DWORD +0, the two words +4/+8, and DWORD +0x1c respectively.
Names remain offset-based because these bodies alone do not establish property
semantics. The pair setter loads/stores its first word before loading its second;
input aliases that observe the first store are preserved. No helper-call boundary
is added to the inlined projections.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_grid_cells.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

Verification passes **16,512 comparisons** (4,096 pool cases, 4,096 per scalar
setter, 4,224 pair cases), rejecting **18 semantic mutations**. All four original
bodies execute; only allocation/free are controlled. It compares final mapped
bytes, ordered allocation/free arguments and snapshots, preserved registers and
stack cleanup. Pool cases cover count/capacity zero, capacities 1..9, all old
pointer-presence combinations, null/empty/non-ASCII UTF-16 payloads, clipping,
word/tail zeroing, and allocation changes to dimensions, buffer, pool fields and
used count; free changes current fields while snapshot frees remain ordered.
Property cases cover zero/nonzero logical dimensions, padded physical grids,
negative/extreme raw coordinates and origins, wrapped/empty rectangles, exact
negative multiples and explicit pair-source aliases. Coverage gates require both
pool zeroing paths, slot creation, migration/copy/termination and both frees.
Compiler/link failures are excluded from mutation rejection counts.

Clang verification disables the compiler's invented `memset` libcall with
`-fno-builtin-memset`, so zeroing stays inside the recovered body. The source does
not introduce a memset seam or stub. Native/mod-enabled builds, all three native
CTest probes and headless scroller motion/centering pass. The expanded synthetic
native grid probe composes pool creation and migration with truncation, null text,
all three property setters, the aliased pair write and pool disabling; it checks
that disabling retains cell payload bits while clearing pool fields.

This remains finite behavioral evidence, not byte matching or a claim about
per-instruction memory access ordering. Arbitrary aliases, undersized buffers,
pool exhaustion, allocation failure/overflow, exceptions/unwind and all states
remain unverified; padded mapped buffers permit original negative-index behavior
to be observed. MSVC 7.1/wibo are absent; reports stay ignored under
`build/analysis/feng-grid-cells/`. Production constructor/base-object lifecycle,
reader, text assignment, cell rendering/ticking and owner callbacks still require
recovery/binding. The native type-6 dispatch trap remains explicit.

## Common FEng and grid construction (2026-10-18)

`FEngConstruct.cpp` recovers common object constructor **0x52af20** (97 bytes)
and grid constructor **0x52b9e0** (185 bytes), including the actual base call.
`config/SPEED2_EXE_1_2/feng-construct.json` fingerprints both complete spans and
original data words **0x7f7cf8** (initial GUID counter 1) and **0x7f7cc4** (initial
opaque word 0x518e20). These globals have explicit verification bindings;
`NativeFng.cpp` supplies independent native storage with those initial values.
The original vtable/default address words remain numeric tokens in native data,
not callable native implementations.

The common constructor initializes the +0/+0x34/+0x44 vtable words, marks +4/+8
links `0xabadcafe`, clears the observed list/count/pointer/flag words, reads and
increments the GUID counter with 32-bit wrap, writes the previous counter into
+0xc, and returns the object pointer. It deliberately leaves type +0x18,
resource +0x22, current script +0x54 and derived storage untouched. Only the
16-bit word at +0x20 is cleared: widening this to a DWORD would erase +0x22.

The grid constructor calls that body, zeros the observed grid fields, sets flags
+0x68 to 2, copies exact phase/default words (+0xa4 float 1 bits, +0xa8 0xbab60b61,
+0xac/+0xb0/+0xb4/+0xb8 each DWORD 0xff), zeros +0xc0/+0xc4, copies the current
0x7f7cc4 word into +0xbc, and sets object type +0x18 to 6. Its object-pointer
return is verified. The +0xbc source retains an offset-based name; its eventual
callback dispatch is not recovered by this constructor checkpoint.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_construct.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

The verifier passes **8,192 comparisons**, 4,096 per constructor, and rejects
**14 mutations**. The actual base body runs inside grid construction; there are
no stubbed calls. Cases use dirty surrounding bytes, random counter/default data,
counter values 0/1/0xfffffffe/0xffffffff, and selected object/global overlaps
that expose increment-versus-GUID-store order and the late +0xbc default read.
Full normal-object and global-region bytes, ordered constructor-entry snapshots,
full EAX pointer returns, stack cleanup and preserved registers are compared.
Mutations include widened clearing at +0x20, clearing untouched +0x54, wrong
GUID progression, omitted base call, changed grid defaults and early default
capture. Compiler/link failures do not count as mutation rejections.

**Common construction is active in the native loader.** Whole-prefix memset and
manual hash/GUID/flags/texture zeroing are removed in favor of `ConstructBase`.
The native projection still explicitly initializes fields the original leaves
untouched (including currentScript54 and the currently generic derived prefix),
binds data2C to native-owned state, and constructs native root/child forward and
back links from parsed file order. Those are platform/reader seams, not recovered
constructor behavior. Parsed `OP` values can still override generated GUIDs.
The asset-free list fixture now checks both a missing-OP generated GUID and an
explicit OP GUID of zero, including lookup and counter progression, for both
navigation modes. It no longer assumes all unassigned native GUIDs are zero.

The native grid probe starts with dirty bytes, calls the recovered grid
constructor, checks counter/type/defaults, then composes recovered allocation,
pool operations, initial fill and navigation. Production type-6 objects still
need a valid 0xc8-byte native prefix and original reader/owner/text/render/lifecycle
binding; the grid dispatch trap remains. Fixed address tokens are not dispatched.

This remains bounded behavioral evidence. Arbitrary aliases, concurrent global
access, original virtual dispatch/destruction and full reader/lifecycle behavior
are unverified. MSVC 7.1/wibo and byte matching remain pending; Clang is the stated
verification compiler. Reports remain ignored under `build/analysis/feng-construct/`.

Native and mod-enabled builds pass, as do all three queue/list/grid CTests.
With recovered common construction active, headless integration passes all 19
settings scenarios (including slider fill/knob pixels), all 10 Controls scenarios,
scroller motion/settled centering and modded HUD checks at 640x480, 1280x720,
1680x720 and 2560x720. These establish current demo regression coverage, not full
original constructor/reader/virtual-lifecycle recovery.

## Type 6 property dispatch and actual callee composition (2026-10-18)

`FEngGridPropertyReader::ReadProperty` recovers the complete **0x535060** body
(259 bytes) in `FEngGridReader.cpp`. `FEngGridReader.h` represents only the observed
reader prefix with the current grid pointer at +0xc. It does not represent the
full reader's allocation, factory, surrounding chunk iteration or ownership.
`config/SPEED2_EXE_1_2/feng-grid-reader.json` pins the span/executable; verification
also checks all eight callee fingerprints from the storage and cell manifests.

The original dispatch reads a 16-bit tag and uses payload +4, ignoring the length
word in this method. Uppercase `LD` (0x444c) calls recovered `ResizeCells`; `SI`
(0x4953) calls `ResizeStringPool`; `Cc`/`Cs`/`Lj` call the recovered word/pair/word
property setters across current physical dimensions with starts (0,0). `Cs`
copies both raw property words into a local pair before entering the setter. `LT`
(0x744c) stores `flags68 & 1` first, then reads the payload, merges all other bits,
and stores again. That first store is observable for aliased property data.
Unknown tags do nothing, including with an invalid current-grid pointer; they
need no length or payload bytes.

`Lv` (0x764c) loads the second float, loads the first float, then stores +0x84
and +0x88 in that order through x87. The source uses float assignments that
compile to those loads/stores with the verification target. It deliberately does
not replace these with raw word copies: signaling NaNs can quiet and set status.
The second input is captured before the first output store even if they alias.
None of these tags assigns cell text; actual text population remains in the
unrecovered owner/text-assignment path.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_grid_reader.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

Verification passes **4,352 full-reader comparisons** and rejects **11 mutations**.
All eight recovered callees execute together: physical allocation/reset, both
logical setters, pool migration and all three cell-property setters. Only allocation
0x575620 and free 0x575630 are controlled. Cases cover all seven supported tags,
three unknown tags, arbitrary ignored length words, physical extents 1..4, valid
owned-cell retirement pools, existing/null old pool pointers, and allocator/free
changes to current fields. Selected properties alias the LT flags word, Lv's first
output and Cs's destination. An unknown tag at the last mapped two bytes uses an
invalid grid pointer, proving no extra payload/grid read along that tested path.

The verifier compares final mapped bytes, ordered helper/allocator/free targets,
arguments and snapshots, preserved registers and stack cleanup. It normalizes
only Cs's compiler-specific local-pair stack address into its two word contents;
that callee still executes and selected aliases detect passing the original
payload directly. Six x87 control words cover selected precision/rounding modes;
FPCW, FPSW, empty tags and balanced TOP are checked with signed zero, denormals,
infinities, quiet/signaling NaNs and finite aliased values. Mutation checks catch
argument/callee/extent changes, ownership-bit replacement, omitting LT's first
store, bypassing Cs's snapshot and changing Lv load/store behavior. Compile/link
failures do not count as rejected mutations. MSVC 7.1/wibo remain absent; Clang
verification retains the existing no-invented-memset flag for the actual pool body.

The native grid component fixture now reaches allocation/reset/dimension setup,
shrink retirement, pool creation/migration/disable and all property setters through
this recovered dispatcher, then composes initial fill/navigation with native
callbacks. Constructor-based fixtures and direct setter alias checks remain.
Native/mod-enabled builds, all three queue/list/grid CTests and headless scroller
motion/settled-centering checks pass. These are component/integration results;
the native file reader still requires original grid factory/iteration/owner
binding and a valid 0xc8-byte production prefix. Its type-6 navigation trap remains.

Coverage remains finite. Invalid known-tag lengths, arbitrary aliases, undersized
or exhausted pools, allocation failure/overflow, unmasked FP exceptions/unwind,
concurrent mutation and byte matching remain unverified. Reports remain ignored
under `build/analysis/feng-grid-reader/`. The earlier statements that this property
dispatch was unrecovered are superseded; the surrounding reader, text assignment,
owner implementation and rendering/lifecycle seams remain.

## Normal-return grid and object cleanup (2026-10-18)

`FEngLifecycle.cpp` recovers five UG2 v1.2 cleanup/release bodies. Destructors are
explicit **normal-return** reconstructions; original compiler SEH registration
and unwind funclets are not reproduced by these methods.

| Routine | Address | Reference span |
|---|---|---|
| `FEngQueueList::ClearOwned` | 0x511670 | 110 bytes |
| `FEngStatePoolClass::ReleaseSlot` | 0x5174a0 | 124 bytes |
| `FEngStatePoolManager::ReleaseState` | 0x517520 | 68 bytes |
| `HudVisibilityObject::DestructBase` | 0x52af90 | 148 bytes |
| `FEGridObject::Destruct` | 0x52baa0 | 131 bytes |

`config/SPEED2_EXE_1_2/feng-lifecycle.json` pins the full spans, the existing
75-byte `Remove` callee at 0x505400 and mapped BSS data at 0x838474/0x863770.
BSS fingerprints refer to mapped zero-initialized bytes, not absent raw-file data.
`FEngLifecycle.h` records observed block/class/list layouts and the observer slot;
all evidence remains specific to UG2 v1.2.

Grid cleanup writes its vtable token, frees nonnull +0x8c, +0xa0 and +0x9c in
that order and calls actual base cleanup. It does not gate frees on flag 1 or
clear released pointer fields. Each pointer reload follows the prior free call.
Base cleanup writes its base vtable, calls optional observer slot zero through
0x838474, releases current data2C through the 0x863770 state manager, frees current
+0x14 if nonzero, clears the +0x44 script list before the +0x34 response list, and
finishes with vtable 0x79a9e8. Observer and pooled-node deletion callbacks can
change later fields; those reads remain fresh.

Pool classes search blocks by an unsigned half-open `data14 .. data14+strideC*32`
range, with 32-bit bound wrap. Stride and used count are 16-bit. Slot index is
unsigned byte displacement divided by stride: no alignment check is invented.
Release decrements usedE, clears the bitmap bit, and, on zero, calls actual
`Remove` before virtual block deletion. The manager searches classes by the AL
result and, if the matching class's block count is now zero, removes/deletes that
class. A nonnull nonmember pointer reaches an original null-class dereference;
it is not converted into a new graceful-failure policy.

Owned-list clearing calls `Remove` for its first node, then uses the original
inlined unlink sequence for later nodes. Each node is poisoned with link
sentinels before virtual deletion; the list head is reloaded after the callback.
This preserves deletion of a newly inserted head and later virtual-target changes.
No extra `Remove` boundary or cached-next traversal is introduced.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_lifecycle.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

Verification passes **20,736 comparisons**: 4,096 each for list clear, manager
release, base cleanup and grid cleanup, plus 4,352 slot-release cases. It rejects
**16 semantic mutations**. Actual `Remove`, slot release, manager release, list
clear and base cleanup compose; only free 0x575630, observer notification and
virtual deleting-destructor interfaces are controlled. Cases include list sizes
0..3, callback head insertion and target changes, strides 1..64, all 32 slots,
unaligned slot interiors, used-count edges including 16-bit underflow, pool misses,
empty block lists, exclusive upper/lower boundaries and wrapping range endpoints.
Callbacks change subsequent free pointers, observer availability, payloads, class
counts and later lists. Mutations catch extra unlink calls, skipped reinsertion,
wrong slot/range/bitmap math, omitted deletion/notification, wrong cleanup order
and cached pool pointers. Compiler/link failures do not count as rejections.

Tests compare final mapped bytes, ordered helper/virtual/free targets, arguments
and snapshots, AL for slot release, preserved registers, stack balance and final
FS:[0] restoration. Original destructors execute their normal SEH prologues under
a mapped TEB/GDT; rebuilt methods do not synthesize those frames. Intermediate
SEH registration/frame identity, callbacks that inspect the exception chain,
throwing callbacks and exceptional unwinding are outside this evidence. Void
cleanup EAX contents are not a recovered return contract.

The synthetic native grid probe composes recovered construction, allocation,
pool creation and normal cleanup, with an observer that allocates a new +0x14
payload; the recovered base cleanup releases it. A native virtual node fixture
inserts a new head during deletion and confirms original deletion order and empty
list state. These tests use private empty state-manager storage and native virtual
callbacks. All three native queue/list/grid CTests, native/mod-enabled builds and
headless scroller motion/centering pass.

**Production lifecycle integration remains pending.** Native file objects still
own vectors/state through their existing wrappers; original factory/state
allocation, virtual node destructor implementations, callback owners and SEH
unwind must be recovered/bound before replacing that ownership. This does not
activate production grid dispatch or claim native pooled-state allocation.
Arbitrary aliases, invalid/nonmember releases, unbounded callback reconfiguration,
concurrency and byte matching remain unverified. Clang is the stated verifier;
MSVC 7.1/wibo are absent. Reports stay ignored under `build/analysis/feng-lifecycle/`.

## State-pool allocation and concrete virtual destruction (2026-10-18)

`FEngStatePool.cpp` recovers eight more UG2 v1.2 routines, paired with the release
chain above. Constructors and destructors remain normal-return reconstructions;
original compiler SEH registration/unwind frames are not synthesized.

| Routine | Address | Reference span |
|---|---|---|
| `FEngStatePoolBlock::Construct` | 0x517310 | 123 bytes |
| `FEngStatePoolBlock::AcquireSlot` | 0x504400 | 135 bytes, including both return paths |
| `FEngStatePoolClass::Acquire` | 0x517400 | 157 bytes |
| `FEngStatePoolManager::AcquireState` | 0x52ace0 | 145 bytes |
| `FEngStatePoolBlock::Destruct` | 0x5173b0 | 77 bytes |
| `FEngStatePoolBlock::Destroy` | 0x517390 | 30 bytes |
| `FEngStatePoolClass::Destruct` | 0x534ac0 | 73 bytes |
| `FEngStatePoolClass::Destroy` | 0x534aa0 | 30 bytes |

`config/SPEED2_EXE_1_2/feng-state-pool.json` fingerprints the spans and first
virtual slots at 0x79b13c/0x79b8dc. Verification imports the lifecycle/Remove
fingerprints too. Missing Ghidra definitions were annotated; the existing
one-byte-body metadata issue also affects some new definitions, so full-span
PE bytes and original execution remain the verification authority.

Block construction writes link sentinels, low-16 stride, vtable 0x79b13c, used
count zero and null data pointer, allocates `stride * 32` bytes through 0x575620,
stores the returned pointer and clears the bitmap **after** allocation. It does
not initialize slot payloads. Claiming a slot returns null only when usedE equals
32; it skips bitmap bytes equal to 0xff, finds the first clear bit, increments
the 16-bit used count, sets that bit and returns `data14 + strideC * slot` with
32-bit arithmetic. No popcount reconciliation, bounds fix or metadata repair is
invented for inconsistent counters/bitmaps.

Class acquire searches the first block whose used count is not 32. If necessary,
it allocates a 0x18-byte block, calls the actual constructor with the current
low-16 class stride, inserts the new block at the current list head and calls
actual `AcquireSlot`. Manager acquire returns null for key zero, otherwise searches
full DWORD class keys. Missing keys allocate a 0x20-byte class with an empty block
list and the observed vtable words, insert it using current post-allocation head
and count fields, then call actual class acquire. Keys with different upper words
can therefore be separate classes with the same low-16 block stride.

Block destruction unconditionally frees data14, including null, and sets its
final node vtable. Class destruction resets the embedded list vtable, calls actual
`ClearOwned` on all blocks and finishes with the node vtable; it does not first
rewrite its own vtable to the derived token. Both deleting wrappers call their
normal destructor, free the object only for flag bit zero, and return the original
object-pointer value. Concrete block/class virtual deletion now executes recovered
bodies rather than controlled deleting-destructor mocks in this verifier.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_state_pool.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

The verifier passes **33,088 comparisons** and rejects **17 semantic mutations**:
4,096 cases per new entry point, 256 extra standalone constructor/destructor
edge cases and 64 acquire/release sequences. Compile/link failures do not count
as mutation rejections. It exercises all eight entry points, actual release/clear/Remove
callees and acquire-then-release sequences. It relocates only the two virtual
slots in rebuilt emulation to the matching rebuilt deleting wrappers; the original
machine retains its retail slots. Virtual calls execute actual corresponding
bodies on both sides. Only allocation/free at 0x575620/0x575630 are controlled.
Recorded helper identities, allocator arguments/snapshots, full-word returns,
metadata/global bytes, stack cleanup, preserved registers and final FS:[0]
restoration are compared. Heap-payload write hooks separately establish that
allocation/release bodies do not add payload initialization.

Cases include dirty block/class storage, all bitmap positions and full bytes,
full-count equality versus 16-bit counter edges, class misses/hits, empty and
full block lists, head-versus-later block selection, high-word class keys, strides
through 0xffff, zero-stride standalone construction, null-data destruction and
full-word deleting flags. Allocator callbacks change class stride/list heads and
block metadata, exposing snapshot/reload and post-allocation bitmap-clear order.
Selected standalone class destructors start with an unrelated vtable word to
check the absence of an invented initial rewrite.

The native fixture relocates the two virtual slots in its existing private
fixed-address test page to ABI adapters that call the recovered deleting wrappers.
It allocates 40 unique 64-byte slots (two blocks), creates a distinct 0x10040-key
class with the same low-16 stride, releases a slot through recovered object/base
cleanup, then releases the rest. Eight allocated class/block/data objects are
freed through the recovered paths and the manager ends empty. These are test-only
virtual relocations, not production dispatch changes. Native/mod-enabled builds,
all three native CTests and headless scroller motion/centering checks pass.

Production state ownership and object factory binding remain pending. Existing
native FNG objects still use their documented native-owned state. Allocation
failure, inconsistent/full bitmap scans, arbitrary aliases, concurrency, callbacks
that inspect SEH frames, throwing callbacks and unwind are unverified. MSVC 7.1
and byte matching remain pending; Clang is the stated verification compiler.
Reports stay ignored under `build/analysis/feng-state-pool/`.

## Full object factory and typed native construction (2026-10-18)

`FEngObjectFactory.cpp` recovers the complete **0x534df0** factory (403 bytes),
including every switch branch and actual constructor/state calls. The 12-entry
jump table at **0x534f84** is fingerprinted with the code and executable in
`config/SPEED2_EXE_1_2/feng-object-factory.json`. Its remaining constructors are
recovered in `FEngObjectConstructors.cpp`:

| Routine | Address | Reference span |
|---|---|---|
| image/type-1 constructor | 0x532640 | 32 bytes |
| string/type-2 constructor | 0x495630 | 100 bytes, normal return |
| list/type-4 constructor | 0x5349c0 | 130 bytes |
| group/type-5 constructor | 0x52b080 | 43 bytes |
| type-7 constructor | 0x52b0b0 | 32 bytes |
| type-9 constructor | 0x534b90 | 32 bytes |
| type-10 constructor | 0x534db0 | 32 bytes |
| type-11 constructor | 0x52b480 | 25 bytes |
| nested text-storage constructor | 0x51de80 | 16 bytes |

Every derived constructor calls actual common construction. The string constructor
sets its vtable/derived defaults, language hash 0xffffffff and actual nested
text-storage initialization at +0x64/+0x68 before setting the remaining string
properties/type. List construction sets all original fields through +0xa8,
including the phase bits and unit defaults. Group construction creates the
observed child-list header. Other leaf types retain offset-based tail fields;
unknown type semantics are not inferred from their vtable identities. Type 11
has only a 0x5c-byte footprint, with no invented +0x5c tail write.

The factory returns null immediately for type 0. Types 1/2/4/5/6/7/9/10/11 allocate
0x60/0x78/0xac/0x6c/0xc8/0x60/0x60/0x60/0x5c bytes and execute their respective
constructors; all other types allocate a 0x5c-byte common object. Type 6 receives
the current reader +0x38 owner at object +0x5c after construction. Common finalization
writes the requested type and scans the current reader's +0x30/+0x34 specification
table for its **first** matching full-word type. It calls the actual global state
manager release even for the constructor's null data2C, clears data2C, acquires
using the selected stride/key, then stores that key at +0x30 and the returned
state at +0x2c. The selected key survives later allocator changes to the reader.
No new allocation-failure policy is added; original nonzero-type null allocations
can dereference null, including type 6's owner store.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_object_factory.py --compiler clang-cl --mutation-checks
ctest --test-dir build/native -R '^feng_(queue|list|grid)_native$' --output-on-failure
```

Verification passes **22,528 comparisons**: 2,048 dirty-input cases for each of
nine new constructor entries and 4,096 complete factory cases. It rejects
**15 mutations**. Actual common/grid/typed/text-storage constructors and the
state manager/pool allocation chain execute together, with allocation/free as
the controlled interfaces. The factory's initial release uses the constructors'
null state; nonnull release behavior is covered separately by pool/lifecycle
verification and native teardown. Cases cover every factory branch, default and
negative/high-bit types, absent/duplicate state specs, zero/nonzero/high-word keys,
existing/full/empty state pools, GUID edges and allocator changes to specs, owner,
counter, untouched fields and intermediate object-state pointers. Full metadata
and globals, ordered helper/heap arguments/snapshots, full-word returns, preserved
registers, stack balance and final FS:[0] restoration are compared. Heap-payload
write hooks retain the pool's uninitialized-payload contract. Mutations catch
allocation size, owner/type, first-match selection, omitted state calls, string
sentinel/nested construction, list/group defaults and type-11 overrun changes.
Compile/link failures do not count as semantic mutation rejections.

**Recovered typed constructors are active in the native loader.** A bounded native
prepass obtains the first `Ot` value before allocating the larger C++ wrapper,
then calls image/string/list/group/type-7/9/10/11 construction exactly once. Their
base call advances the GUID once. The old type-4 prefix memset/manual defaults
and duplicate native string defaults are removed. Parsing still applies `Ot`/`OP`
and later properties in file order; native ownership, larger wrapper allocation,
state-array binding and surrounding reader iteration remain scaffolding. Missing,
duplicate or malformed type-property behavior is not a recovered full-reader
contract. Type 6 retains the existing generic native prefix path and explicit
navigation trap until its 0xc8-byte prefix and owner/lifecycle binding are ready.

The native factory component fixture separately invokes the actual recovered
factory across all explicit and default branches, checks generated GUIDs, state
keys/pointers and type-6 owner propagation, and returns state through recovered
base cleanup before freeing raw fixture objects. Their derived-owned fields are
still empty; this teardown does not establish all derived virtual destructors.
All three native CTests and native/mod-enabled builds pass. Production factory
allocation/state integration, complete reader/ownership and SEH unwind remain
pending. Verification uses Clang; MSVC 7.1/wibo and byte matching are pending.
Reports remain ignored under `build/analysis/feng-object-factory/`.

With recovered typed constructors active, headless checks pass all 19 settings
scenarios and slider fill/knob pixels, all 10 Controls scenarios, pause/scroller
motion and settled centering, and modded HUD translation at 640x480, 1280x720,
1680x720 and 2560x720. These check current demo behavior; full original factory
ownership and reader composition are not established by the native wrapper.

### Owned UTF-16 storage in native strings

`FEngTextStorage.cpp` recovers five further UG2 PC v1.2 entries. The existing
8-byte storage constructor at `0x51de80` remains in `FEngObjectConstructors.cpp`.
String-object `+0x64` is its text pointer and `+0x68` is its capacity word.

| Entry | Address | Full PE span | Observed behavior |
| --- | --- | ---: | --- |
| `FEngCopyText` | `0x511630` | 49 bytes | Forward UTF-16 copy; null destination is untouched, null source empties destination |
| `Grow` | `0x51de90` | 75 bytes | Compare request to current text length; allocate `request*2+2`, copy, free old text, store new pointer |
| `GetBuffer` | `0x51dee0` | 26 bytes | Call actual `Grow` only when request exceeds capacity; return fresh pointer |
| `Assign` | `0x52d610` | 62 bytes | Null source is unchanged; otherwise compare length+1 to capacity, grow if needed, then copy |
| `Destruct` | `0x50d390` | 14 bytes | Free nonnull text; leave both fields unchanged |

The growth comparison is deliberately **length**, rather than capacity: direct
`Grow` can replace a larger allocation with a smaller one. Capacity is written
before allocation. The copy source is reloaded after the allocator returns; the
old pointer is reloaded before free, and the new pointer is stored after free.
Arithmetic remains unsigned 32-bit, including allocation-size wrap. No extra
null/allocation guards or field resets have been added to recovered code.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_text_storage.py --compiler clang-cl --mutation-checks
```

The verifier checks the executable SHA-256 and full function spans in
`config/SPEED2_EXE_1_2/feng-text-storage.json`. It executes original and rebuilt
routines with actual recovered nested calls, controlling only allocation/free at
`0x575620` and `0x575630`. **5,120 comparisons** pass, including null/empty and
non-ASCII code units, identical/forward-overlapping copy buffers, capacity/length
boundaries, wrapped size arguments, null allocation returns and heap callbacks
that change pointer/capacity fields. It compares full fixture memory, ordered
callee/heap arguments and memory snapshots, defined pointer returns, preserved
registers and stack balance. **Nine semantic mutations** are rejected; build/link
failures are not counted as semantic rejections. The heap fixture supplies mapped
storage without unmapping freed blocks or enforcing requested allocation sizes;
wrapped-size and alias cases establish instruction behavior in that fixture,
not memory safety or a valid allocator contract. Reverse-overlapping nonterminating
copies and exhaustive input/heap-failure coverage are not claimed. Generated
reports and mutation logs stay under ignored `build/analysis/feng-text-storage/`.
Clang behavioral verification passes; MSVC 7.1/wibo and byte matching remain pending.

The native FNG loader now sends bounded `St` UTF-16 input through actual `Assign`;
`FengPackage::SetText` converts renderer/localization bytes to a temporary UTF-16
input and uses the same recovered method. The persistent `originalText` vector is
removed. Native finalization no longer overwrites string `+0x64` with a child
pointer. The wrapper's destructor calls recovered storage `Destruct` for type 2.
Native heap adapters were moved from the pause screen to the FNG module so this
ownership path also links into the asset-free parser fixture.

This is storage integration, not complete string-object or reader recovery.
At this checkpoint, normal string destructor `0x4956a0`, property reader
`0x534fc0`, label helpers, original localization, complete object/state/list
ownership, reader iteration and SEH unwind were separate work. The following
checkpoint recovers the property reader and label helpers. The C++ wrapper, temporary bounded input,
byte rendering projection and native observer are documented platform seams.
The native synthetic package fixture checks retained non-ASCII UTF-16, pointer
reuse for shorter/empty assignments, longer growth and repeated package teardown.

Native and mod-enabled builds and all three asset-free CTests pass with this
storage active. Headless retail-asset integration passes all 19 settings
scenarios and slider fill/knob pixels, all 10 Controls scenarios, pause/scroller
motion and settled centering, and modded HUD translation at 640x480, 1280x720,
1680x720 and 2560x720. These establish current native integration behavior,
separately from the bounded original/rebuilt storage comparisons above.

### Complete string-property dispatch and member label helpers

`FEngStringReader.cpp` recovers member hash setter `0x4fe940` (42-byte span),
owned byte-label setter `0x519f40` (121 bytes), and the full string property
reader `0x534fc0` (159 bytes). The reference SHA-256 and complete function spans
are recorded in `config/SPEED2_EXE_1_2/feng-string-reader.json`.

The member hash setter writes language hash `+0x60` and dirty bit `0x400000` only
when the hash changes, then notifies the current observer's first slot whenever
that observer exists. It **preserves bit 2**, including observer changes to flags;
the extra saved-bit/clear-bit behavior of cdecl `0x50c900` does not apply here.
Original tail jumps are represented by ordinary source calls with the same
arguments and normal return behavior; matching instruction selection is pending.

The byte-label setter unconditionally frees old label `+0x5c`, resets it to null,
optionally allocates/copies source bytes including their terminator, calls actual
recovered `FngHash` on fresh `+0x5c`, writes `+0x60` and dirty, then notifies the
current observer. Null input still frees/reset the label and hashes null to
`0xffffffff`. Nonnull input has no allocation-failure guard in the original.
This byte label and UTF-16 text `+0x64` have separate ownership.

| Property | Original action |
| --- | --- |
| `SH` / `0x4853` | Actual member hash setter with payload DWORD |
| `SL` / `0x4c53` | Actual byte-label setter with payload string, only if reader byte `+9` is nonzero |
| `Sb` / `0x6253` | Actual storage `Grow(payload DWORD)` at object `+0x64` |
| `Sj` / `0x6a53` | Write justification `+0x6c` |
| `Sl` / `0x6c53` | Write leading `+0x70` |
| `St` / `0x7453` | Actual storage `Assign(payload UTF-16)` at object `+0x64` |
| `Sw` / `0x7753` | Write width `+0x74` |

The reader's current object is at `+0xc`. It ignores the property length word;
unknown tags and disabled `SL` do not dereference the object. `SL` and `Sl` are
different tags and are not interchangeable. Reader ownership and iteration
remain separate from this complete dispatch method.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_string_reader.py --compiler clang-cl --mutation-checks
```

**6,144 comparisons** pass, with 2,048 per new entry. All seven recognized tags
and nine unknown/adjacent values receive 128 direct reader cases each. Actual
member setters, hash and storage callees execute together; allocation, free and
observer slot 0 are the only controlled interfaces. Cases include unchanged and
changed hashes, null/empty/non-ASCII byte labels, UTF-16 payloads, absent/present
observers, zero/high-bit reader gates, ignored length words, capacity/length
boundaries and heap/observer callbacks that change flags, hashes, pointers,
capacity or observer binding. Full fixture memory and ordered helper/heap/observer
arguments and snapshots, stack balance and preserved registers are compared.
Unknown/disabled properties also run with null object pointers. **Fourteen
semantic mutations** are rejected, covering notification/dirty/bit2 handling,
label free/reset/allocation/hash, length/gate dispatch and storage/field writes.
Compile/link failures are not semantic rejections. Reports stay in ignored
`build/analysis/feng-string-reader/`. Controlled heaps retain mapped freed memory;
nonnull label allocation failure, invalid/unterminated pointers, arbitrary stack
aliasing and exhaustive behavior are not established. Clang behavioral checks
pass; MSVC 7.1/wibo and byte matching remain pending.

**The complete string reader now executes in the native FNG loader.** Native
manual `SH`/`Sj`/`Sl`/`St`/`Sw` field writes are removed; `Sb` now reaches original
growth instead of being ignored. The native adapter leaves reader byte `+9` zero,
so optional `SL` allocation is disabled. Both gate states are verified against
UG2, but recovery of how the original surrounding reader initializes that byte
remains pending. At this checkpoint, full byte-label lifetime integration awaited the normal
string object destructor and complete object ownership; the following lifecycle
checkpoint recovers the normal destructor and its deleting wrapper. Existing recovered UTF-16
teardown remains active.

The adapter retains native property bounds and temporary terminated `St` input;
malformed/truncated asset behavior is not the original reader contract. Renderer
byte text and the legacy `font` label-hash field are projections of recovered
results. Repeated `St` replaces text, and finalization preserves the constructor's
`0xffffffff` hash when no label property is present. The observer binding has
relocated native storage instead of an absolute linker assignment at `0x838478`;
its initial null value and native receiver remain platform scaffolding, with
all recovered observer users resolving the same symbol. Localization itself,
outer reader `0x555140`, factory/state ownership and string destructor `0x4956a0`
remain separate work.

The synthetic native fixture exercises all seven properties, unchanged-hash
notifications, retained bit2, disabled `SL`, scalar offsets, repeated `St`, and a
`Sb` request of 6 with existing capacity 9 and text length 4: actual `Grow`
reallocates to capacity 6. It also retains the prior non-ASCII storage and package
teardown checks. All three asset-free native CTests pass.

Native and mod-enabled builds pass. With recovered dispatch active, headless
retail-asset checks pass all 19 settings scenarios and slider pixels, all 10
Controls scenarios, pause/scroller motion and settled centering, and all tested
baseline HUD at 640x480, 1280x720, 1680x720 and 2560x720, plus all 11 skins at 1280x720. A settings capture was
visually inspected; the review PNG remains ignored at
`build/analysis/feng-string-reader/settings.png`. These are native integration
checks, separately from the original/rebuilt execution evidence above.

### String-object normal and scalar deleting destruction

`FEngStringLifecycle.cpp` recovers the normal-return body of string destructor
`0x4956a0` (99-byte full span) and scalar deleting wrapper `0x495780` (30 bytes).
The original first string-vtable slot at `0x78f53c` points to `0x495780`; that
4-byte slot and both complete function spans are fingerprinted in
`config/SPEED2_EXE_1_2/feng-string-lifecycle.json`.

Normal destruction writes string vtable token `0x78f53c`, unconditionally frees
the owned byte label at `+0x5c`, calls actual UTF-16 storage `Destruct` on `+0x64`,
then calls actual base destruction. Label/text pointer fields retain their bits.
The wrapper always performs this cleanup, frees the object only when `flags & 1`,
and returns the original object pointer even when freed. High flag bits do not
select deletion. The original normal destructor's SEH registration/unwind is
not reconstructed by the source body.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_string_lifecycle.py --compiler clang-cl --mutation-checks
```

**8,192 comparisons** pass: 4,096 per new entry. Actual recovered text/base
cleanup, manager/class/slot release, concrete block/class destructors and deleting
wrappers, list clear and unlink run together. Only free `0x575630`, destruction
observer slot 0 and synthetic script/response-node deleting slots are controlled.
The concrete state-pool deleting slots execute actual code, with only rebuilt
slot targets relocated in the verification fixture. Cases include null/non-null
label/text/state/`+0x14` pointers, zero/one/multiple list nodes, dirty pool metadata,
last-used-slot deletion, absent/present observers, inserted/rebound list nodes,
free/observer changes to subsequent pointers and binding, and flags 0/1/2/3,
`0x100`/`0x101`/`0x80000000`/`0xffffffff`. Full fixture memory, ordered helper/free/
virtual arguments and snapshots, preserved registers, stack balance and full-word
wrapper returns are compared. Final original FS:[0] restoration is checked on
every normal return. **Ten semantic mutations** are rejected for omitted or
conditional label free, text/base omissions, vtable changes, cached text storage,
field clearing, wrong deletion flag, omitted object free and wrong return pointer.
Build/link failures do not count as semantic rejection.

This is bounded normal-return evidence: exception paths, callback inspection of
SEH frames, arbitrary aliases, concurrency and complete coverage are not proved.
Controlled heaps retain mapped freed storage. Actual script/response-node
destructors are still separate from their controlled deleting-slot interfaces.
Clang behavioral verification passes; MSVC 7.1/wibo and byte matching remain
pending. Reports and mutation logs stay in ignored
`build/analysis/feng-string-lifecycle/`.

**Native type-2 wrapper teardown now calls recovered string and base destruction**
instead of calling only text-storage cleanup. Its inline state and vector-owned
script/response headers are borrowed by the retail prefix: the explicit native
ownership adapter detaches `data2C`, list counts, heads and tails before calling
`DestructString`. C++ containers then release their own storage. Original label,
UTF-16 and any original-owned `+0x14` cleanup remain in recovered code. The native
state-manager and destruction-observer symbols have separate native storage;
production strings do not yet allocate original state or owned list nodes.
Complete factory/reader/state/list ownership and production vtable dispatch are
still pending, and this adapter is not the original ownership model.

The native owned-object fixture constructs a string with actual byte-label and
UTF-16 methods, acquires actual pooled state, supplies owned `+0x14` storage and
observes all six frees (label, text, state data/block/class, `+0x14`) through the
actual cleanup chain. It also checks retained pointer bits, final base vtable,
and scalar wrapper low-bit behavior. A separate native FNG package fixture has
both a script and response backed by vectors. Its destruction observer verifies
that original base cleanup sees detached borrowed prefixes while both C++ vectors
are still alive, across three package lifetimes. All three native CTests pass.

Native and mod-enabled builds pass. With original normal string/base teardown
active, all affected headless checks pass: 19 settings scenarios and slider
fill/knob pixels, 10 Controls scenarios, scroller motion/settled centering, and
HUD skin translation at 640x480, 1280x720, 1680x720 and 2560x720. Package resets,
return/resume and reopening checks exercise production wrapper teardown; they do
not establish original state/list ownership or exception unwinding.

### Image, group and type-4 list cleanup

`FEngTypedLifecycle.cpp` recovers eight further UG2 PC v1.2 entries. Full PE
spans and the three original scalar-deleting vtable slots are fingerprinted in
`config/SPEED2_EXE_1_2/feng-typed-lifecycle.json`.

| Entry | Address | Full span | Original behavior |
| --- | --- | ---: | --- |
| Image normal destruction | `0x532660` | 11 bytes | Set image vtable `0x79ba0c`, tail-jump to actual base destruction |
| Image scalar deleting wrapper | `0x534dd0` | 30 bytes | Actual normal cleanup, free object if `flags & 1`, return original pointer |
| Group normal destruction | `0x534bf0` | 82 bytes | Set child-list `+0x5c` value0 to `0x79a9ec`, actual `ClearOwned`, then actual base destruction |
| Group scalar deleting wrapper | `0x534bd0` | 30 bytes | Actual normal cleanup, optional low-bit object free, return original pointer |
| List normal destruction | `0x534a50` | 79 bytes | Set list vtable `0x79bae4`, actual storage release, then actual base destruction |
| List scalar deleting wrapper | `0x541d90` | 30 bytes | Actual normal cleanup, optional low-bit object free, return original pointer |
| List storage release | `0x516dc0` | 78 bytes | If owned bit0 at `+0x5c`, clear it, release counted axes and reset their pointers/counts, then tail-jump to cell release |
| List cell release | `0x5042e0` | 114 bytes | Scan unsigned32 `count64*count60` cells at stride `0x30`; free nonnull `+0x20` only for `+0x18 == 2`; free/reset cell array |

Group normal destruction does **not** rewrite the object's own vtable before
clearing children. Axis frees are selected by nonzero counts, including a null
axis pointer, and reset pointer/count only after the free callback. Cell cleanup
reloads the array after each owned-cell free before clearing `+0x20`, and reloads
it again for the final array free. A zero count product performs no frees or
writes, even with a nonnull array. The product wraps as unsigned 32-bit arithmetic.

Preserve the original storage-order quirk: ordinary owned-axis release zeros
both counts before calling cell release, so cell release sees product zero and
leaves `+0x90` untouched. The implementation does not move cell release earlier
or reset that pointer opportunistically. Direct cell release has its separately
verified nonzero-product behavior. Original tail jumps are represented by source
calls with the original helper boundaries; byte matching remains separate.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_typed_lifecycle.py --compiler clang-cl --mutation-checks
```

**16,384 comparisons** pass, 2,048 per entry. Actual base destruction, pool
manager/class/slot release, concrete pool block/class destruction and scalar
wrappers, queue clear/unlink and list-storage/cell helpers execute together.
Free, destruction observer and synthetic child/script/response deleting slots
are controlled interfaces; pool deleting slots execute actual recovered code.
Cases include empty/nonempty owned lists, dirty fields and pool metadata,
zero/nonzero/null axes, zero and wrapped-zero count products, finite cell scans,
cell tags 0/1/2/3, null/non-null cell payloads, low/high deleting flags, last-state
release and callbacks rebinding later pointers, counts, observer or list nodes.
Full fixture memory and ordered helper/free/virtual arguments and snapshots,
stack balance, preserved registers, scalar full-word returns and final original
FS:[0] restoration are compared. **Sixteen semantic mutations** are rejected,
including early cell cleanup, missing ownership/reset/free operations, cached
cell bases, extra group-vtable writes and wrong deleting flags/returns. Build or
link failures do not count as semantic rejection.

This is finite normal-return behavior, not complete coverage, arbitrary aliasing,
concurrency, SEH registration/unwind or callback inspection of exception frames.
Controlled heaps retain mapped freed storage. Reports remain ignored under
`build/analysis/feng-typed-lifecycle/`. Clang verification passes; MSVC 7.1/wibo
and byte matching remain pending. Ghidra names/signatures/comments are updated;
its repeated one-byte `create-function` body metadata issue is recorded locally,
and full PE spans plus original execution remain authoritative.

**Native type-1 images, type-5 groups and type-4 lists now run recovered normal
cleanup**, alongside type-2 strings. The common native adapter detaches borrowed
inline state and vector-backed script/response list prefixes. Groups detach the
borrowed child-list prefix because the package's object vector owns the child
wrappers. At this checkpoint, lists cleared the original owned-axis bit because native
vectors owned the axes. The following storage checkpoint replaces those vectors
and preserves the bit, with a separate native heap owner. C++ members then release their own storage. These explicit ownership seams
remain separate from recovered code; production allocation, original owned
state/list nodes and general virtual dispatch still require recovery/integration.
Other object types are not redirected to these cleanup bodies without evidence.

The native owned-object fixture uses actual group cleanup to delete two image
children through a private ABI bridge into actual image scalar destruction. It
checks real cell-payload/array frees, counted axis release, the retained cell-array
quirk and all three scalar wrappers' low-bit behavior. The parser fixture checks
that list vectors and a group's child vector remain alive while base cleanup sees
detached borrowed prefixes. All three native CTests pass. The image `If` property
case is inline in outer reader `0x555140`; no independent image-reader function
is claimed or invented by this checkpoint.

Native and mod-enabled builds pass. With recovered typed cleanup active, all
19 headless settings scenarios and slider fill/knob pixels, all 10 Controls
scenarios, pause/scroller motion and settled centering, and baseline HUD at 640x480,
1280x720, 1680x720 and 2560x720 plus all 11 skins at 1280x720 pass. These check production wrapper integration
and package resets, separately from original/rebuilt cleanup evidence. Original
production state/list allocation, outer reader/inheritance and SEH unwind remain
pending.

### Original type-4 axis and cell allocation

`FEngListStorage.cpp` recovers column resize `0x52a910` (536-byte full PE span)
and row resize `0x52ab30` (424 bytes). `FEngCellConstruct.cpp` recovers cell
constructor `0x504150` (40 bytes) and constructor iterator `0x401650` (48 bytes).
Fingerprints are in `config/SPEED2_EXE_1_2/feng-list-storage.json`.

Each nonzero resize allocates `count*12` axis bytes, copies the retained axis
entries forward as DWORDs, frees the fresh old axis pointer, and zeros new axis
entries. It computes the opposite-axis count product, allocates `product*0x30`
cells when nonzero, invokes the actual constructor iterator/callback and actual
`FEngGridResetCells`, migrates retained data when an old array exists, and frees
the fresh old cell array. Final count, cell and axis pointer stores retain their
original order. Heap callbacks can change later field reads. The owned bit at
`+0x5c` is not set by either resize routine.

Two original behaviors are deliberately preserved. Column migration copies
**retained-column bytes per row**, while row migration copies **columns times
retained-row bytes**: these are not multiplied by the 0x30-byte cell stride.
Resetting the added tail still uses cell indices and counts. Zero-axis requests
clear a counted axis pointer/count before calling actual cell release; that
release then sees a zero product and leaves the cell pointer untouched. A later
zero-product resize can overwrite that pointer with null without freeing the
old allocation. This checkpoint does not repair these behaviors.

The constructor writes words 0/3/4/5/6/8 to zero, words 1/2 to `0x3f800000`, and
word 9 to `0xffffffff`, leaving words 7/10/11 untouched. Its observed ABI takes
one ECX pointer and returns that pointer in EAX. The source expresses this with
a legacy-compatible one-argument fastcall interface; original C++ member
provenance is not claimed. The stdcall iterator takes four stack arguments and
uses signed `(count-1) >= 0` as its entry gate, calls the ECX-only constructor
exactly `count` times on an entered loop, and advances the pointer by the supplied
stride in 32-bit arithmetic. The native compatibility header supplies the GCC
fastcall spelling; recovered code uses the legacy compiler keyword.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_list_storage.py --compiler clang-cl --mutation-checks
```

**8,192 comparisons** pass, 2,048 per new entry, with actual constructor
iteration/callback, reset and cell-release callees. Allocation/free are the only
controlled boundaries. Cases include growth/shrink/unchanged/zero dimensions,
empty/nonempty previous cells, partial-byte migration, zeroed axis tails,
wrapped-zero allocation size, dirty constructor words, zero/nonzero iterator
strides, high-bit iterator skips and heap callbacks changing counts or later
axis/cell pointers. Full fixture memory and ordered helper/heap arguments and
snapshots, preserved registers, stack balance, full pointer returns where
defined and final original FS:[0] restoration are compared. Constructor callback
addresses are normalized to their original/rebuilt entry identity. **Thirteen
semantic mutations** are rejected, covering byte-versus-cell migration, missing
free/zero/construct/reset operations, early zero-axis release, wrong stores,
unwritten constructor fields, sentinels, returns and iterator gate/stride.
Compile/link failures do not count as semantic rejection. Finite mapped heap
fixtures do not establish allocation-failure behavior, arbitrary aliases,
concurrency, enormous entered loops or SEH registration/unwind/exception-frame
inspection. Reports stay ignored under `build/analysis/feng-list-storage/`.
Clang behavior passes; MSVC 7.1/wibo and byte matching remain pending.

**Native `Ld` now invokes actual column and row resizing.** The two persistent
axis vectors and final pointer projection are removed. Native `Lc`/`Lr` projection
writes the actual allocated axes, and recovered offset/navigation code consumes
those arrays. Newly allocated cell storage also comes from the original path.
At this checkpoint the full list-property reader `0x52b740`, cell payload
helpers and surrounding reader ownership remained separate work. The following
checkpoint recovers the complete property dispatch and payload helpers; native property bounds/iteration and
selection setup still form an adapter.

The Linux heap boundary has an explicit per-wrapper allocation owner while
executing list allocation. Original free calls remove their records before
calling native free. Any surviving allocations are released by the native heap
owner **after** recovered normal teardown, including allocations the original
zero/product paths no longer reference. This is platform heap lifetime
scaffolding, not a recovered game cleanup step or a change to the original
resize/release order. The constructor/resize ownership bit is preserved; native
teardown no longer clears it to protect axis vectors. Inline state and
script/response/child ownership adapters remain as previously documented.

The asset-free parser fixture checks actual initialized cells, raw axes and
allocation records still alive at base cleanup, and a 3x2 -> 0x2 dimension
sequence whose lost cell allocation stays registered for native heap-owner
teardown. All three native CTests pass. Native and mod-enabled builds and all
affected headless checks pass: 19 settings scenarios and slider pixels, 10
Controls scenarios, scroller motion/settled centering, and baseline HUD at 640x480,
1280x720, 1680x720 and 2560x720 plus all 11 skins at 1280x720. These are native integration checks, separately
from original/rebuilt storage evidence.

### Complete type-4 list property reader and cell payload writes

`FEngListReader.cpp` now recovers the complete 532-byte `0x52b740` dispatch.
`FEngListCells.cpp` recovers its six cell-write helpers, and `FEngColour.cpp`
adds packed-colour unpacking. Full spans and the existing pack callee are pinned
in `config/SPEED2_EXE_1_2/feng-list-reader.json`.

| Entry | Address | Full span | Behavior |
| --- | --- | ---: | --- |
| Cell type | `0x5041e0` | 80 bytes | On changed type, free nonnull `+0x20` only when old `+0x18 == 2`; clear payload and set new type |
| Cell colour | `0x516cd0` | 47 bytes | Actual `FEngColour::Pack`, then selected-cell `+0` write |
| Cell pair | `0x516d00` | 51 bytes | Forward DWORD writes at `+4`/`+8` |
| Cell triple | `0x516d40` | 57 bytes | Three DWORD arguments at `+0xc`/`+0x10`/`+0x14` |
| Cell four-word payload | `0x516d80` | 63 bytes | Forward DWORD writes at `+0x20` through `+0x2c`, without freeing/type checking |
| Cell UTF-16 | `0x516e10` | 122 bytes | Free old nonnull payload regardless of type; optionally allocate/copy terminated UTF-16 |
| Colour unpack | `0x5054a0` | 42 bytes | Extract A/R/G/B into original channel fields; observed EAX returns the receiver pointer |
| Complete reader | `0x52b740` | 532 bytes | All twelve property cases below; ignore declared length and unknown tags |

All cell offsets use unsigned32 `(row8C * count60 + column88) * 0x30`.
Type/text setters cache their selected cell across heap callbacks. This differs
from cell-array release, which reloads the array after a free. Text assignment
clears an existing payload even for null input; nonnull empty input allocates
2 bytes. It copies forward DWORDs and trailing bytes including the terminator,
and leaves the cell type unchanged. Pair/four-word input aliases retain their
sequential forward-copy effects. The unpack receiver return is verified as a
register-level fact; original C++ constructor/method provenance is not asserted.

| Property | Original dispatch |
| --- | --- |
| `CT` | Cell type helper with payload DWORD |
| `Cc` | Increment reader `+0x2c`; advance selection after the first; actual unpack -> cell colour -> pack |
| `Lc` / `Lr` | Increment reader `+0x24` / `+0x28`, write axis entry `+0` and `+8` |
| `Ld` | Actual column/row resizers; reset reader `+0x2c`, `+0x28`, `+0x24` to -1; select column/row 0 |
| `Ci` | Four-word temporary into the cell `+0x20` helper |
| `Cr` | Cell triple helper with `0, 0, payload DWORD` |
| `Cs` | Two-word temporary into the cell pair helper |
| `Ct` | Cell UTF-16 helper with payload pointer |
| `Ls` | x87 second-then-first loads into `+0x80` / `+0x84` |
| `Lv` | x87 second-then-first loads into `+0x68` / `+0x6c` |
| `Lw` | Actual wrap helper with boolean DWORD test |

The reader captures object `+0xc` once and retains it across callees even if a
heap callback changes the context's object pointer. Payload reads retain their
call boundaries, including the row-count read after column resizing. Length
validation and outer reader lifecycle/iteration are not part of this method.

```sh
UG2_EXE='/path/to/speed2.exe' build/venv/bin/python tools/verify_pc_feng_list_reader.py --compiler clang-cl --mutation-checks
```

**16,384 comparisons** pass, 2,048 per new entry. All twelve recognized tags and
four unknown/adjacent tags receive 128 direct reader cases each. Actual resize,
constructor iteration/callback, reset/release, selection/wrap, colour unpack/pack
and cell payload routines execute together; only allocation/free are controlled.
Cases include unchanged/changed cell types, null payloads/input, empty/non-ASCII
UTF-16, signed colour clamps, wrapping selected-cell arithmetic, forward input
aliases, zero/nonzero dimensions, index sentinels, ignored length words, x87
zero/negative-zero/infinity/quiet/signalling-NaN pairs and heap callbacks changing
later array/axis/selection/context/payload state. Unknown tags also run with null
object pointers. Full fixture memory and ordered helper/heap arguments and
snapshots, preserved registers, stack balance, final original FS:[0] restoration
and defined full-word unpack returns are compared. Compiler stack temporaries
are compared by their contents, constructor targets by entry identity, and the
wrap byte parameter by its defined low byte rather than incidental upper bits.
**Seventeen semantic mutations** are rejected across dispatch, offsets, selection,
type/free behavior, text allocation/copy/destination capture and unpack return.
Compile/link failures do not count as semantic rejection.

Finite mapped heaps do not establish allocation failure, lifetime validity after
free, arbitrary stack aliases, concurrency, exception-frame inspection or SEH
unwind. Reports stay ignored in `build/analysis/feng-list-reader/`. Clang behavior
passes; MSVC 7.1/wibo and byte matching remain pending. Ghidra labels, comments and
member signatures are updated with the verified boundaries and limits.

**Native type-4 parsing now calls the complete recovered reader.** The manual
axis/scalar/selection branches and native list counters are removed. A native
context supplies initial sentinels, property length/axis/selection bounds and
terminated `Ct` input. The existing scoped heap owner covers every recognized
property, including text allocations and frees on type changes; its final native
heap-lifetime cleanup remains separate from original game code. Outer reader
`0x555140`, inheritance, resource binding, original state/script/response ownership
and cell rendering are still separate work.

The asset-free parser fixture checks actual packed colours, `Ci` words, raw
`Cs` signalling-NaN/negative-zero bits, `Cr` triple placement, UTF-16 surrogate
code units, receiver-pointer unpack return and removal of a text allocation when
its type changes. All three native CTests and native/mod-enabled builds pass.
Headless checks pass all 19 settings scenarios and slider pixels, all 10 Controls
scenarios, pause/scroller motion and settled centering, and baseline HUD at 640x480,
1280x720, 1680x720 and 2560x720 plus all 11 skins at 1280x720. These establish native integration separately
from original/rebuilt evidence and do not establish the original cell renderer.


### Owned object names and inheritance resolution (2026-10-18)

Recovered four complete routines from UG2 PC v1.2: owned name setter
`0x5176b0` (110 bytes), inheritance-record lookup `0x504960` (62 bytes),
encoded package-name lookup `0x50b240` (82 bytes), and reader inheritance
resolver `0x5186c0` (102 bytes). Fingerprints are recorded in
`config/SPEED2_EXE_1_2/feng-inheritance.json`.

The name setter frees a nonnull `+14`, clears it, resets `hash10` to
`0xffffffff`, then allocates/copies a supplied byte string including its NUL
and calls the recovered hash on the fresh field. It does not dirty the object
or call an observer. A null input leaves the sentinel hash without calling
the hash helper. Allocator side effects distinguish field reloads and ordering.

Package records are 12 bytes (`key0`, `packageHash4`, `guid8`), addressed by
captured count `+84` and base `+88`; lookup returns the first matching key.
Queue lookup uses a separate chain at `+104`, following package `next4`, and
hashes bytes at `value38 + 2` inline. It ignores `nameC` and `nameHash10`.
Only the resulting name pointer is null-tested: `value38 == 0xfffffffe`
wraps to null and produces the sentinel hash; a null `value38` is not a
valid empty-name encoding.

The resolver is a thiscall method over package `+0` and queue `+3c`, with
three stack arguments and only AL defined as its result. It clears object
then package outputs, obtains the first record/package, and calls the actual
recovered GUID-tree query. It keeps the found package output when the GUID
is absent, and never retries another identically named package. Package
output is stored before the record GUID is read, which matters when those
addresses alias. An aliased pair of output slots ends with the object result.

Validation: `UG2_EXE=... build/venv/bin/python
tools/verify_pc_feng_inheritance.py --compiler clang-cl --mutation-checks`
passes 8,192 comparisons (2,048 per root) and rejects 13 semantic mutations.
Original execution enters the real hash, package/record lookup, GUID query,
object traversal, nested-child traversal and visitor bodies; only the two
heap boundaries are controlled. Rebuilt execution is restricted to linked
source ranges. Comparisons include arena bytes, ordered helper-entry state,
normalized temporary query descriptors, ABI-preserved registers, stack
cleanup, and restored FS exception chain. Cases include high byte names,
case folding, duplicate keys/packages/GUIDs, empty lists/trees, null names,
allocator rebinding and output/record aliases. Finite valid mapped heaps do
not establish allocation failure, arbitrary stack aliases, invalid lifetimes,
concurrency or SEH unwinding. MSVC 7.1/wibo and byte matching remain pending.

Native `FengPackage::Load(..., retainNames)` binds the observed reader byte
`+9` to a default-false adapter option. Enabled `On` properties call the
recovered setter, and enabled `SL` properties call the existing complete
string reader. Native bounds supply terminated byte strings; native heap
ownership tracks names through replacement and recovered normal destruction.
Synthetic asset-free fixtures exercise both option values, replacement,
unterminated input bounding, retained labels, first-match inheritance,
nested GUID lookup, missing-result outputs and alias-sensitive GUID reloads.
The original caller's initialization of byte `+9` is still unverified.
The resolver is verified and exercised natively but not wired into production
loading: queue/archive bindings, base/group clone `0x554300`/`0x5548a0`,
and the full outer object reader `0x555140` remain separate work.

Native and mod-enabled builds pass. All three native CTests and the headless
settings (19 scenarios plus slider pixels), Controls (10 scenarios),
pause/scroller motion/centering, and baseline HUD at four resolutions plus all 11 skins at 1280x720 pass.
These integration checks do not establish original rendering or production
inheritance ownership. Ghidra signatures and plate comments record the four
verified spans and the resolver's calling convention and alias constraints.


### Response-command construction, ownership and assignment (2026-10-18)

Tracing base clone `0x554300` reached response-array resize `0x52ae00` and
command assignment `0x52ad80`. This checkpoint recovers the complete command
constructor `0x4fe910` (13 bytes), cleanup callback `0x517600` (47 bytes),
byte-string setter `0x517630` (115 bytes), and assignment `0x52ad80`
(118 bytes). All spans are fingerprinted in
`config/SPEED2_EXE_1_2/feng-response-command.json`.

The constructor zeros all three words and returns the receiver in EAX.
Constructor and cleanup are ECX-only array callbacks; recovered free functions
use fastcall to preserve that observed ABI without claiming original C++ member
provenance. Cleanup tests an unsigned type range: `0x200` through `0x204`,
excluding `0x203`, own a byte-string parameter. It calls free even for a null
parameter, then clears parameter only; type and target remain unchanged.

`SetString` repeats that ownership test/free, clears parameter, and optionally
allocates `strlen + 1` bytes. It copies forward including the terminator and
stores the allocated pointer only after copying. Assignment frees the old
owned parameter, clears it, copies source type, then calls the actual setter
for string-owning types. The setter's second `free(NULL)` is preserved.
Assignment reloads source target after the nested setter/heap callbacks and
returns the receiver. Other types copy the raw parameter then target. There
is no self-assignment guard: owning self-assignment frees the old string and
ends with a null parameter. These are observed original mechanics, including
their quirks, rather than a replacement ownership policy.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_response_command.py
--compiler clang-cl --mutation-checks` passes 16,384 comparisons (4,096 per
root), entering the actual setter 5,735 times, and rejects 12 semantic
mutations. Original and rebuilt runs execute the entire nested setter body;
only heap allocation `0x575620` and free `0x575630` are controlled. Comparisons
include full mapped arena bytes, ordered callee/heap entry snapshots, receiver
returns, preserved registers and stack cleanup. Rebuilt execution is restricted
to linked source ranges. Cases include all ownership boundaries, high bytes,
empty/null strings, source/self aliases, input aliasing the parameter field,
and heap callbacks rebinding fields/source records. A field-alias case rejects
storing the new pointer before copying. Finite mapped storage does not prove
allocation failure, freed-memory lifetime validity, arbitrary stack aliases,
exception unwinding, concurrency or byte identity. MSVC 7.1/wibo remain pending.

The native response parser now calls the recovered constructor instead of
initializing command defaults itself. Parsing of `Ri`/`Ru`/`Rt`, vector ownership
and response-prefix projection remain native seams. Native fixtures check
zero defaults through parsing, numeric payload projection, deep-copy pointer
independence, string replacement, two free calls during assignment,
self-assignment and non-owning `0x203` cleanup. The existing conditional-traversal
verifier also passes all 8,192 comparisons and its six mutation rejections.
Original response-array resizing/allocation `0x52ae00`, full response parsing
`0x54d1d0`, base/script copying and production inheritance binding remain work
required before replacing the complete outer object reader.

Native and mod-enabled builds and all three asset-free CTests pass. Headless
integration passes 19 settings scenarios plus slider pixels, 10 Controls
scenarios, pause/scroller motion and settled centering, baseline HUD translation
at 640x480/1280x720/1680x720/2560x720, all 11 HUD skins at 1280x720, and the
scripted HUD capture. These are integration evidence, separate from recovered
command behavior. Ghidra signatures/comments are updated. Earlier HUD coverage
wording above is clarified to distinguish the four-resolution baseline from
the skin sweep performed at 1280x720.


### Response-array resizing and native ownership (2026-10-18)

Recovered the normal-return body of `FEResponse::ResizeCommands` at
`0x52ae00` (285 bytes), plus the normal paths of the CRT constructor iterator
`0x75c461` (74-byte main span) and reverse destructor iterator `0x75c401`
(72-byte main span). `config/SPEED2_EXE_1_2/feng-response-array.json` also
fingerprints their finally funclets and actual SEH prolog/epilog helpers.
These are UG2 observations; the source does not assert original file names
or establish behavior in another SPEED title.

Equal logical counts return without touching the array. A nonzero resize
allocates `count * 12 + 4` with 32-bit arithmetic, stores the count cookie,
and calls the actual constructor iterator with both callback arguments.
It captures the fresh logical count after construction and assigns the smaller
of it and the requested count. Each copy reloads the old array field and calls
the actual command assignment, including its nested string setter. Cleanup
reloads the old array, captures its allocation base before callbacks, destroys
using the count cookie at `array - 4` rather than the logical count, and frees
the captured base. Only then does it install the new pointer and logical count.
Zero-size cleanup follows the same cookie/destruction/free sequence, then clears
pointer and count. Callback-induced field rebinding distinguishes these loads.

Constructor iteration advances forward and compares signed `i < count`.
Destructor iteration starts at `base + stride * count`, decrements the count,
tests its sign, and subtracts stride before each callback. Count/stride arithmetic
wraps in 32 bits. The callback ABI is ECX-only; the constructor iterator takes
five stdcall arguments and the destructor iterator takes four. The unused normal-
path destructor argument is still passed by callers. Separate translation units
keep that call boundary observable without forcing registers or inventing
volatile accesses. The source callback types are scoped to verified response
commands; other CRT callback uses have not been established by this checkpoint.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_response_array.py
--compiler clang-cl --mutation-checks` passes 12,288 comparisons, 4,096 per
root, and rejects 14 semantic mutations. Actual original execution enters
constructor callbacks 21,002 times, cleanup callbacks 21,169 times, command
assignment 5,370 times, and the string setter 2,386 times. It executes the
original iterators, finally guards and SEH setup/teardown; only heap calls at
`0x575620`/`0x575630` are controlled. Rebuilt execution is restricted to linked
source ranges. Comparisons include arena state, ordered helper/heap snapshots,
callback identity, stack cleanup, preserved registers and restored FS chain.
Cases vary logical/cookie counts, growth/shrink/equal/zero requests, null old
storage, string-owning types, retained strings, heap rebindings, negative signed
iterator counts and zero/forward/reverse strides. Failed array allocation is
covered only when the retained count is zero; it is not general allocation-
failure recovery. Invalid lifetimes, arbitrary stack aliases, huge positive
iterations, exceptions, concurrency and byte identity remain unproven.

**Exceptional rollback is not recovered.** The original iterator's exceptional
finally path uses the destructor callback to roll back partially constructed
arrays. Recovered sources implement normal-return behavior only. Full SEH
unwinding, MSVC 7.1/wibo verification and byte matching remain pending. Restoring
the FS chain after a normal call does not prove exception equivalence.

Native responses now own actual recovered command arrays instead of borrowing
`std::vector` command data. The bounded native parser applies `MC` through
`ResizeCommands`, writes `Ri`/`Ru`/`Rt` into constructed cells, and applies bounded
terminated `Rs` strings through the recovered setter. Native response headers
remain movable wrappers: transfer clears the old owner, and member teardown
calls recovered `ResizeCommands(0)`. This is an ownership adapter, not recovery
of the original pool-backed header destructor or its teardown order. Package
and object header links are still projected natively. Bounds/context guards
skip out-of-range commands and clear a cached command when MC replaces storage;
these guards are separate from the original unbounded reader.

Native fixtures exercise growth/shrink/equal/zero storage, cookies, initialized
cells, string replacement and deep copying, independent package/object arrays,
unterminated/high-byte Rs input, numeric `0x203` parameters, header-vector
relocation, explicit move construction/assignment and release of a previous
owner. The existing conditional traversal verifier also passes 8,192 comparisons
and six mutations. The full response reader `0x54d1d0`, response-pool allocation
`0x54bdf0`, base/script cloning and production inheritance remain pending.

Native and mod-enabled builds and all three asset-free CTests pass. Headless
integration passes 19 settings scenarios plus slider pixels, 10 Controls
scenarios, pause/scroller motion and settled centering, baseline HUD translation
at four resolutions, all 11 skins at 1280x720 and the scripted HUD capture.
These check native integration separately from original/rebuilt evidence.
They do not establish the full original reader, pool ownership, exception-frame
observations during callbacks or exceptional unwind. Ghidra signatures and
comments record the verified boundaries and these normal-path limits.


### Response-header pool and virtual cleanup (2026-10-18)

Recovered eleven bounded UG2 routines, fingerprinted in
`config/SPEED2_EXE_1_2/feng-response-pool.json`:

| Routine | Address | Span |
| --- | --- | --- |
| Header constructor | `0x4fe920` | 31 bytes |
| Header normal destructor | `0x534b10` | 117 bytes |
| Header deleting wrapper | `0x53ee60` | 40 bytes |
| Pool-block constructor | `0x53e710` | 501 bytes |
| Pool-block normal destructor | `0x53eb40` | 211 bytes |
| Pool-block deleting wrapper | `0x53eb20` | 30 bytes |
| Pool acquire | `0x54bdf0` | 186 bytes |
| Pool release | `0x525b30` | 86 bytes |
| List insertion | `0x5053b0` | 76 bytes |
| Cursor insertion wrapper | `0x4951b0` | 17 bytes |
| Base-node deleting wrapper | `0x4fdb00` | 31 bytes |

A block is `0x620` bytes: node prefix, 64 embedded `0x18`-byte response
headers at `+0xc`, a free list at `+0x60c`, and checked-out count at `+0x61c`.
Construction calls the actual CRT constructor iterator with header callbacks,
initializes the free list and inserts all 64 slots in order. The retail compiler
unrolls four insertions; recovered source preserves their pointer-store order
without adding an out-of-line insertion call. Acquisition chooses the first
block with a nonzero free count, or allocates/constructs a new block and prepends
it using fresh pool fields. It removes the first free header and increments
the checked-out count, even when inconsistent metadata supplies a null first
header. Allocation failure retains the original null-block dereference path;
this checkpoint does not establish recovery from it.

Release tests unsigned half-open byte membership `[block+0xc, block+0x60c)`;
it does not require slot alignment. Nonmembers produce no action. It calls the
actual cursor insertion wrapper, decrements checked-out count, and when zero
removes the block and calls its virtual deleting slot with flag 1. The block
destructor drains its free list inline without deleting individual headers,
then calls the actual `ClearOwned` and reverse iterator over all 64 headers.
Finally it restores the base-node vtable; the deleting wrapper optionally frees
the block. Inconsistent lists, including a nonzero count with no first node,
are outside the valid-list fixture coverage and are not silently repaired.

Header construction marks links `0xabadcafe`, writes vtable `0x79aa38` and
zeros message/count/array fields. Its actual destructor frees a present command
array using its allocation cookie **even if the logical count is zero**, clears
pointer then count, and restores vtable `0x79a9e8`. This is distinct from
`ResizeCommands(0)`, whose equal-count fast path can leave such an array alone.
The header deleting wrapper calls the actual normal destructor, then for low
flag bit 1 invokes the base virtual slot with flag 0 and calls pool release at
original global `0x8637a0`. It returns receiver bits even if releasing the last
header frees its containing block. Other flag bits do not select deallocation.

The shared CRT iterator source now uses generic ECX-only callback pointers and
names `FEngConstructArrayWithCleanup` / `FEngDestructArrayWithCleanup`.
Both 12-byte command and 24-byte header callback chains are now verified in UG2;
this extends the earlier command-only evidence without assuming another title's
implementation. Exceptional rollback remains unrecovered. The command-array
verifier still passes its 12,288 comparisons and 14 mutations after this change.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_response_pool.py
--compiler clang-cl --mutation-checks` passes 5,632 comparisons (512 per root)
and rejects 19 semantic mutations. Original execution enters header construction
41,664 times, header destruction 71,936 times, block construction 643 times,
block destruction 1,108 times and actual command cleanup 2,522 times. Actual
iterators, list insertion/removal, empty-list cleanup, base virtual dispatch,
block virtual dispatch and original normal SEH setup/teardown execute; only
heap allocation/free are controlled. Rebuilt execution is restricted to linked
source ranges, with native vtable entries pointing to recovered bodies. The
verifier compares arena and global-pool state, ordered helper/heap snapshots,
callback identities, defined pointer returns, preserved registers, stack cleanup
and the restored FS chain. Cases vary free/checked-out counts, empty/exhausted
chains, second-block selection, new blocks, last-header release, flag bytes,
unaligned byte membership, boundary nonmembers, logical/cookie count mismatch
and heap-induced field rebinding. Finite mapped records do not establish
allocation failure, arbitrary bad pointers/lifetimes, malformed list cycles,
exception-frame inspection, exceptional unwind, concurrency or byte identity.
MSVC 7.1/wibo and byte matching remain pending.

Native `FengResponse` now owns a pointer acquired from the recovered pool rather
than an inline response header. Constructor/reset, command-array destruction,
base virtual cleanup, return to the pool and empty-block cleanup all execute
recovered code. Native move operations transfer the header reference; vectors
own only those references. Package/object list projection, wrapper message
metadata and the bounded reader are still adapters. Native object teardown
still detaches borrowed response-list prefixes before recovered base cleanup;
C++ wrapper teardown then releases pooled headers, so complete original owner
teardown ordering is not claimed.

The native bridge relocates pool-global storage and binds three virtual slots:
base node `0x79a9e8`, response `0x79aa38`, and pool block `0x79bf50`. Production
reuses `MapOriginalImage`'s SHA-256-verified writable retail data mapping,
checks the original slot values and changes only those three entries to native
functions. Asset-free fixtures reserve private pages instead. An initial asset
check caught an attempted duplicate page reservation; mapping ownership is now
queried explicitly, preserving the retail constants beside the vtables.
This mapping/dispatch arrangement is platform scaffolding, not original code.

Asset-free native fixtures exercise 70 simultaneous headers across two blocks,
owned strings/arrays, pointer-preserving header moves, and complete empty-pool
cleanup. Native and mod-enabled builds and all three CTests pass. Full response
reader `0x54d1d0`, duplicate-message lookup/merge, original list ownership,
base/script clones, production inheritance and SEH unwind remain pending.

After the mapping fix, all affected headless checks pass: 19 settings scenarios
plus slider fill/knob pixels, 10 Controls scenarios, pause/scroller motion and
settled centering, baseline HUD translation at 640x480/1280x720/1680x720/2560x720,
all 11 skins at 1280x720, and the scripted HUD capture. The existing conditional
traversal verifier also passes 8,192 comparisons and six mutations. These are
separate integration/regression results, not byte-matching or full-reader proof.
Ghidra labels/comments cover all eleven routines; available function signatures
and the shared iterator names are updated without fabricating missing function
bodies in the partially analyzed project.


### Complete response reader and original owner-list insertion (2026-10-18)

Recovered the complete `0x54d1d0` reader body (378 bytes), object response
lookup `0x5046d0` (31 bytes), and package response insertion `0x512120`
(75 bytes). Fingerprints are in
`config/SPEED2_EXE_1_2/feng-response-reader.json`. The reader is a thiscall
method with data, byte count and byte-valued package-mode arguments; it returns
1 in AL, leaving the other return bits outside the contract. Its observed
context contains package `+0`, object `+0xc`, and merge-control byte `+0x18`.

| Tag | Original behavior |
| --- | --- |
| `Mi` / `0x694d` | Select/create response, reset command index to `0xffffffff` |
| `MC` / `0x434d` | Call actual command-array resize |
| `Ri` / `0x6952` | Increment index, capture cell at array + index * 12, store type |
| `Rs` / `0x7352` | Call actual byte-string setter on cached command |
| `Rt` / `0x7452` | Store target at cached command +8 |
| `Ru` / `0x7552` | Store parameter at cached command +4 |

Unknown properties are skipped by their length word. End is captured with
32-bit addition, and cursor/end comparison is unsigned. The property length
word is reloaded **after** callbacks, so a heap-induced length change affects
which record is read next. Message is captured before allocation and remains
that value if heap callbacks change its source bytes.

For object mode with nonzero context byte `+0x18`, `Mi` calls the actual object
lookup and reuses its first matching message. It destroys/frees the prior array
inline using the count cookie, clears array/count, and preserves header identity
and list position. It does not call the header destructor. Otherwise it acquires
a header from the actual pool, initializes its prefix inline, writes message,
and calls either object cursor insertion `0x4951b0` or package insertion
`0x512120`. Package mode always creates responses, regardless of merge-control
byte. Only the low byte of the mode argument controls these branches.

`Mi` resets index but does not clear the cached command pointer; `MC` also leaves
it intact. A subsequent `Rt` before another `Ri` can therefore update the command
selected in a different response. This behavior is retained, rather than adding
reset/validation logic. The original reader trusts record widths, sequence,
indices and string termination. In-place reuse can make stale cached pointers
invalid; no lifetime repair or general malformed-input safety is claimed.

Package insertion identifies count `+0x58`, first response `+0x5c`, and cursor
`+0x60`. It captures cursor, inserts after it or at the head, repairs previous
links, conditionally updates cursor and increments count; a null node makes no
changes. The body is inline and does not call generic list insertion. Object
lookup compares `messageC`, follows `next4`, and returns the first match or null
in full EAX, independently of GUID/type/count fields.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_response_reader.py
--compiler clang-cl --mutation-checks` passes 6,144 comparisons (2,048 per root)
and rejects 17 semantic mutations. Actual original execution enters lookup 3,952
times, package insertion 5,492 times, pool acquisition 4,954 times, array resize
6,026 times, string setter 15,304 times, command assignment 287 times, and inline
merge cleanup's actual reverse iterator/command destructors. New-block paths
execute all 64 actual header constructors through the CRT iterator. Only heap
allocation/free are controlled; no reader, lookup, append, pool, command or
iterator body is replaced by a test stand-in. Rebuilt execution is restricted to
linked source ranges. Comparisons include arena/input/global-pool bytes, ordered
callee/heap snapshots, callback identity, defined returns, ABI-preserved
registers, stack cleanup and restored FS chain.

Cases include duplicate keys, merge hits/misses, package/object modes and high
mode bits, exhausted/new/existing pool blocks, logical/cookie count mismatch,
repeated strings and high bytes, odd-sized unknown records, empty input and
wrapped end arithmetic, cached commands across Mi/MC, context rebinding, and
heap-induced message/length changes. A nested MC inside Mi's extra payload
becomes a record when a callback shortens Mi, rejecting cached-length code.
Finite mapped storage does not establish arbitrary invalid pointers, freed-
memory lifetime validity, all malformed sequences, allocation failure,
exception-frame inspection, exceptional unwind, concurrency or byte matching.
MSVC 7.1/wibo and helper exceptional rollback remain pending.

**Native tagged-response dispatch is removed.** The adapter builds bounded
records, terminates missing-NUL Rs payloads (updating their length), and invokes
the complete recovered reader once per response chunk. It supplies package/
object context and default-zero merge byte; the original outer reader's context
initialization is still pending. Native width/container guards are separate
from original code and may stop a malformed record prefix before invoking it.
The source reader has no added sequence/index guards.

Native wrappers adopt the pool headers created by the reader without allocating
or initializing extra headers. Vectors track ownership references only. Manual
response-link projection and duplicate message metadata are removed; original
code now writes first/previous/next/cursor/count fields. The native package
adapter supplies a package-list prefix and copies those three response-list
fields into the bound package. Header/global/vtable relocation and C++ reference
teardown remain platform seams. Native object base teardown still detaches the
response list before wrapper destruction, so original owner teardown ordering
is not claimed.

Asset-free fixtures check package/object counts and both link directions,
independent arrays, reader merge preserving header identity, replaced strings,
cached target writes across Mi and MC, and adopted/moved reference cleanup.
Native and mod-enabled builds and all three CTests pass. Outer object reader
`0x555140`, base/script cloning, original context setup/owner teardown, production
inheritance, localization and renderer seams remain separate work.

Headless integration also passes all 19 settings scenarios and slider pixels,
all 10 Controls scenarios, pause/scroller motion and settled centering, baseline
HUD translation at four resolutions, all 11 HUD skins at 1280x720 and the
scripted HUD capture. These checks exercise the complete recovered reader in
the native demo and remain separate from behavioral and byte-matching evidence.
Ghidra function names, method signatures, return contracts and plate comments
are updated for all three routines.


### Script names and owned event-array storage (2026-10-18)

Tracing script copy `0x551be0` recovered its name-storage dependency
`0x518730` (110 bytes) and event resize `0x519a10` (196 bytes), fingerprinted
in `config/SPEED2_EXE_1_2/feng-script-storage.json`. The resize receiver is the
count/data descriptor at script `+0x24`, confirmed by `lea ecx,[ebx+0x24]`
at caller `0x551683`. It is not a method over the whole script prefix.
Event records are three DWORDs: message, target and signed millisecond time.
Their array has no count cookie; this differs from command/header arrays.

The name setter frees nonnull `name2C`, clears it, resets `hash30` to
`0xffffffff`, then optionally allocates/copies a byte string including NUL.
It installs the allocated pointer before copying and calls the actual recovered
hash on the fresh field afterward. Null input leaves the sentinel without a
hash call. It does not alter dirty flags or invoke an observer. It retains its
own original call boundary rather than calling the similarly shaped object
name setter at `0x5176b0`.

Event resize returns immediately for equal counts. A zero request conditionally
frees old data, then clears pointer and count. Other requests allocate
`uint(count) * 12` bytes with wrapped 32-bit arithmetic. A **signed** comparison
against the post-allocation count chooses whether to copy requested records
or all old records. Copying advances forward by DWORDs. Growth reloads count
after copying, computes the tail address and byte count from that fresh value,
and zeroes added records. Cleanup reloads the old data field after copying,
calls free unconditionally on that nonzero-request path (including null),
then installs the new pointer and requested count. These reloads matter for
controlled descriptor aliases and heap-induced field changes.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_script_storage.py
--compiler clang-cl --mutation-checks` passes 8,192 comparisons, 4,096 per root,
and rejects 15 semantic mutations. The actual hash executes 3,276 times;
only heap allocation/free at `0x575620`/`0x575630` are controlled. Rebuilt
execution is restricted to linked source ranges. Comparisons include full
arena bytes, ordered method/hash/heap snapshots, preserved registers and
stack cleanup. Both routines have void return contracts; incidental EAX values
are not treated as pointer results.

Cases cover empty/null/high-byte names, replacement and hash reset, equal/
zero/grow/shrink event counts, zero-filled tails, heap rebinding, forward
copy overlap, and selected signed high-bit counts whose wrapped byte counts
remain finite. A descriptor-alias case overwrites count/data during copying;
it rejects caching either the post-copy count or the pointer passed to free.
These are mapped-state/arithmetic tests, not proof of allocator overlap or
freed-memory lifetime validity. Allocation failure is sampled only for a
zero-byte copy path; general failure behavior, huge positive iterations,
invalid pointers, arbitrary stack aliases, concurrency and byte identity
remain unproven. MSVC 7.1/wibo verification remains pending.

**Native script event vectors are removed.** The existing bounded script
adapter invokes recovered resize on each `EV` and fills the allocated records.
Repeated EV properties now replace the event array, matching original reader
`0x5515e0`, instead of appending extra events as the prior adapter did. The native
record loop retains its zero/truncated-payload guard; the original EV copy loop
enters unconditionally and does not provide that malformed-input guarantee.
Native event scheduling reads `eventCount24`/`events28` directly. Duplicate
native hash/duration fields are also removed; name/hash, duration and event
storage remain in the observed script header. Timing, event firing and most
script/track parsing still execute native scaffolding, not recovered routines.

Optional native `retainNames` also bounds `Sn` strings and calls the actual
script name setter. The original script reader tests a separate byte `+0xa`
at `0x551b02`; object/string optional names use byte `+9`. The native option
binds both gates and defaults false; this does not establish their original
initial values. Native Sh projection can overwrite the hash while retaining
the copied name. Multiple script-node creation/selection inside the complete
original script reader is still pending; the adapter retains its existing
one-wrapper-per-Scrp organization.

Native script wrappers now transfer owned name/event pointers during moves.
Their cleanup calls recovered name-set-null and event resize-to-zero, while
track/key vectors and inline header ownership remain platform seams. This is
not recovery of normal script destructor `0x535170`, whose track cleanup and
unconditional event free have separate ordering. Full script copy `0x551be0`,
track allocation `0x535210`, deep/shallow key-reference copying `0x54d350`,
script header pools and original owner teardown remain required work.

Asset-free native fixtures check repeated EV replacement, retained record bits,
zeroed growth, equal/shrink/zero paths, both optional-name settings, Sh hash
override, independent script arrays, script-vector relocation and explicit
move ownership/release. Native and mod-enabled builds and all three CTests
pass. Ghidra method signatures and comments record the descriptor receiver,
signed arithmetic and remaining limits.

All affected headless checks pass: 19 settings scenarios plus slider fill/knob
pixels, 10 Controls scenarios, pause/scroller motion and settled centering,
baseline HUD translation at 640x480/1280x720/1680x720/2560x720, all 11 skins at
1280x720 and the scripted HUD capture. These verify native integration of owned
script storage separately from original execution and byte-matching evidence;
they do not establish original event timing or the full script reader.

## Script track ownership, key pool and normal script teardown (2026-10-18)

This checkpoint supersedes the native track/key ownership and script-normal-
destructor limitations in the preceding entry. The complete script reader,
copying and header-pool ownership remain pending.

`FEngScriptTrack.cpp` and `FEngScriptKeyPool.cpp` recover fourteen routines:
key constructor/deleting wrapper `0x4fe8b0`/`0x531fe0`; pool block constructor,
normal destructor and deleting wrapper `0x5319c0`/`0x531be0`/`0x531bc0`;
acquire/release `0x53e650`/`0x525bf0`; count-free reference remove/clear
`0x5055c0`/`0x511d00`; track constructor/destructor `0x525e80`/`0x52b960`;
track replacement `0x535210`; and script constructor/normal destructor
`0x511d70`/`0x535170`. Reference headers are 16 bytes with head/tail at +8/+C.
Key nodes are 32 bytes. The pool at `0x839bd0` has 256 slots per 0x2020-byte
block, independently verified from the 64-slot response-header pool.

Track construction initializes links, vtables, flags and empty ownership but
leaves inline key time +0x14, baseline +0x18..+0x27 and padding +0x2d..+0x2f
untouched. Destruction captures flag +0x2c and clears keys only when it is zero;
borrowed references retain their fields. Reference clear removes the first node
through the original method, then reloads the head after every virtual deleting
callback. Key deletion calls the shared normal destructor at `0x51ab10`, then
its base virtual slot and pool release when flags bit zero is set. The source
has two ABI views of that shared original entry (message member and ECX-only
array callback); verification canonicalizes both to `0x51ab10`, with no extra
wrapper call in the recovered boundary.

`SetTracks` always destroys and rebuilds storage, including equal-count requests.
It uses the allocation cookie for reverse destruction, frees the captured base,
clears the pointer and writes count before allocation, allocates `count*0x38+4`
with 32-bit arithmetic, and invokes the actual array constructor iterator.
Only the new pointer is installed afterward; heap-induced count changes persist.
Script construction leaves time +0x10 untouched. Normal script destruction
cleans tracks first, then conditionally frees a fresh name pointer, then
unconditionally frees the fresh event pointer including null. It clears these
pointers, but retains counts, hash and time. Exceptional cleanup is not recovered.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_script_tracks.py
--compiler clang-cl --mutation-checks` passes **3,584 comparisons** (256 per
root) and rejects **21 semantic mutations**. Fingerprints are recorded in
`config/SPEED2_EXE_1_2/feng-script-tracks.json`. Original execution includes
156,116 shared key-destructor calls, actual virtual pool cleanup and original
CRT array/SEH prolog, normal finally and epilog paths; only heap calls are
controlled. Comparisons include arena/global bytes, ordered callback/heap
snapshots, preserved registers and stack cleanup. Cases cover owned/borrowed
references, head changes during deletion, cookies differing from logical counts,
untouched constructor bytes, allocation-time rebinding, wrapped sizes with finite
iterations, pool growth, last-slot block deletion and release boundaries.
Selected malformed mapped states do not prove allocator/lifetime validity.
Invalid pointers, cyclic lists, arbitrary huge iteration counts, allocation
failure throughout all paths, exceptional unwind and byte identity remain
unproven. MSVC 7.1/wibo remains unavailable; this is clang-cl behavioral evidence.
The existing script selection/activation verifier also passes 16,384 comparisons
and rejects 10 mutations after the reference layout extension.

**Native track vectors are removed.** The bounded reader now calls recovered
`SetTracks` and allocates key nodes through the actual key pool. Native vectors
retain parsed metadata and a non-owning pointer index; actual track references
own keys. Native script moves transfer track/name/event ownership and normal
cleanup calls recovered `0x535170`. The adapter still supplies baseline zeros,
initial inline key time, duration bits and bounded payload copies. These defaults
are native bindings, not writes attributed to the original constructors.
`AdvanceScripts` reads original track baselines and pointed key payloads; its
clock, interpolation and event delivery remain scaffolding. Full script reader
`0x5515e0`, copy `0x551be0`, deep/shallow assignment `0x54d350`, script-header
pool/deleting wrappers and production owner teardown remain required work.

The native vtable bridge verifies and binds key/key-block deleting slots
`0x79aa34`/`0x79b9ec`, alongside the three existing base/response slots. Retail
text is never executed by the demo. Asset-free fixtures use private data pages;
production bindings preserve the mapped retail data. Fixtures cover actual
reader projection, same-count destructive replacement, borrowed-key retention,
head insertion during deleting callbacks and 270 keys spanning two blocks.
Native and mod-enabled builds and all three CTests pass. Headless checks pass
19 settings scenarios plus slider pixels, 10 Controls scenarios, pause/scroller
motion, baseline HUD translation at four resolutions, all 11 skins at 1280x720
and the scripted HUD capture. These are integration checks, separate from
original-execution and byte-matching evidence. Ghidra labels/comments cover all
fourteen entries; known method signatures and the pool global are annotated.

## Track assignment and owned/borrowed key copying (2026-10-18)

`FEScriptTrack::Assign` recovers the complete 337-byte routine at `0x54d350`,
called by original script reader `0x5515e0` and script copy `0x551be0`. Its
receiver is the 0x38-byte track and its sole stack argument is the source track;
it has a void contract and pops four argument bytes. The script-copy caller is
still pending recovery and production binding; this checkpoint supplies its
verified dependency and adds native fixture coverage.

Assignment first calls actual count-free reference clear `0x511d00`
**unconditionally**, including self assignment. It copies bytes +0..+3, low
24 bits at +4, state +7, inline key time +0x14 and four raw baseline DWORDs
+0x18..+0x24 in original order. Inline key vtable/links +8..+0x10 and reference
padding are retained. There is no same-pointer guard or destructor/constructor
pair substituted for the original call sequence.

For an owned source (byte +0x2c zero), it traverses source head +0x30,
acquires each node through actual pool `0x53e650`, performs the inline key
constructor stores, copies time and all sixteen payload bytes, then performs
inline count-free insertion at the destination cursor. It reloads source next
only after allocation, copying and insertion. Source time/payload reads are
also after allocation. It does not reset the destination reference flag on
this path. Allocation failure is followed by the original unchecked payload
write; the recovery adds no successful-failure behavior.

For a borrowed source it retains the direct target +0x30, without calling
`Resolve` or cloning its referents. A null target clears destination head,
cursor and flag. A nonnull target conditionally calls clear a second time
when the destination flag is zero, then installs the target and sets flag one;
it does not copy the source cursor. Unconditional initial clear has no borrowed
reference guard. Tests of nonzero destination flags use empty references;
this does not establish valid behavior for assigning over arbitrary live
borrowed references, overlapping tracks or cyclic reference chains.

The expanded `verify_pc_feng_script_tracks.py --compiler clang-cl
--mutation-checks` verifies all fifteen roots with 256 cases each, including
256 direct assignment cases: **3,840 comparisons** and **32 rejected mutations**
across the complete ownership suite. The original key pool, cleanup and virtual deletion
execute; only heap boundaries are controlled. Source/destination field bits,
full arena/global state, ordered calls and snapshots, preserved registers and
stack cleanup are compared. Cases include independent owned clones, empty
sources, direct/null borrowed targets, destructive self assignment, untouched
inline links/padding and pool growth. One controlled heap callback changes the
current source key's next/time/payload, testing fresh reads across acquisition.
The mutation suite includes self-fastpath, missing initial clear, forced owned
flag, copied inline links/source cursor, incorrect null handling, omitted key
time/payload, reversed insertion and premature next/time caching.

Native asset-free fixtures call the recovered assignment on parsed track data,
check independent pooled keys and raw baselines, verify destructive self
assignment and both borrowed-target branches, and release through actual track
destruction. Native and mod-enabled builds and all three CTests pass. There
is no production playback or rendered-layout change in this checkpoint.
Byte matching, exceptional unwind, invalid-pointer behavior and complete script
copy/reader recovery remain unproven. MSVC 7.1/wibo remains unavailable.

## Complete script copy construction (2026-10-18)

`FEngScriptCopy.cpp` recovers the complete 510-byte routine `0x551be0` as
`FEScript::CopyConstruct(other, shallow)`. The receiver is unconstructed script
storage; this is not assignment over an owned script. The original inlines
initialization, then calls name setter `0x518730`, track replacement `0x535210`
and event resize `0x519a10`. It returns the receiver and pops eight argument
bytes. Shallow mode tests only the low argument byte; every nonzero byte selects
that branch. The reference SHA and function fingerprint are recorded in
`config/SPEED2_EXE_1_2/feng-script-copy.json`.

Construction writes vtable/link sentinels, zeros owned fields, length, flags and
chain, and leaves current time untouched until copying it after the name setter.
It owns a new copy of the name, then overwrites its computed hash with the fresh
source hash, copies length/time/flags and allocates the source track count.
The source chain is not copied. Original construction does not call the normal
script constructor as an out-of-line helper and does not free fields that
previously occupied the destination bytes.

Deep mode calls actual `0x54d350` for each track. Shallow mode instead copies
bytes +0..+7, inline key links +0xc/+0x10, time +0x14 and the four baseline
DWORDs +0x18..+0x24. It preserves the newly constructed inline vtable at +8.
It conditionally clears the fresh destination reference and points it at
**the source track's reference header at +0x28**, then sets the borrowed flag.
It does not copy the source head directly or resolve a reference chain.
The null-target branch exists in the original arithmetic, though ordinary
mapped source tracks do not reach it. Both loops reload destination logical
track count after the body; heap-induced count changes in allocation can
therefore reduce how many initialized tracks receive copied values.

Finally, event resize creates independent storage. Copy construction reads the
source event pointer and destination count/data after that call and performs
forward DWORD copying with wrapped `uint(count)*12` byte arithmetic. The
retail residual-byte loop has an identically zero count because the byte
size is a multiple of four; the source records this without inventing a
memcpy boundary or a separate tail algorithm.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_script_copy.py
--compiler clang-cl --mutation-checks` compares the complete routine with actual
name/hash, track/array construction, assignment, key pool and event resize
execution. Only allocation/free are controlled; original normal SEH setup and
restoration execute. Arena/global bytes and ordered method/heap snapshots,
receiver return, preserved registers, stack cleanup and restored FS exception
head are compared. **1,024 comparisons pass and 16 semantic mutations are rejected.**
Cases include zero through three tracks, zero through four
events, null/empty/high-byte names, all reference-flag categories, low-byte
shallow values 0/1/0x80/0xff with varying high argument bytes, name hash override,
and selected heap-induced source metadata/event-pointer and destination-count
changes. Selected same-receiver mapped states show the original destructive
initialization; they do not establish valid C++ object lifetime or safe assignment.

Native fixtures construct both deep and shallow copies of an actual parsed
track, check independent names/events, hash override, copied time/flags, reset
chain, independent deep key storage and shallow source-reference retention.
They then run normal recovered script teardown and confirm source keys remain
live. Native and mod-enabled builds and all three headless CTests pass.
The demo's production script reader has not yet been replaced, so this recovery
is a verified dependency and fixture integration, not a new runtime cloning path.
Complete reader `0x5515e0`, header pools/deleting wrappers, original owner
teardown, clocks and interpolation remain pending. Exceptional unwind, general
allocation failure, arbitrary overlapping/invalid pointers, enormous iteration
counts and byte identity remain unproven. MSVC 7.1/wibo remains unavailable.

## Script-header pool and native stable ownership (2026-10-18)

This checkpoint removes inline native script-header storage. `FEngScriptPool`
at original `0x839c30` allocates 32 0x34-byte headers per 0x6a0-byte block:
slots start at +0xc, free-list prefix +0x68c (count +0x690, head +0x694,
cursor +0x698), used count +0x69c. These dimensions are independently verified
against UG2, separately from response and key pools; no cross-title sharing
is inferred.

The recovered entries are block constructor `0x53e910` (518 bytes), normal
block destructor `0x53ec40` (211), block deleting wrapper `0x53ec20` (30),
acquire `0x54beb0` (186), release `0x525b90` (86) and script deleting wrapper
`0x53ee30` (40). Fingerprints are in `feng-script-pool.json`.
Block construction calls the actual CRT iterator with script ctor/dtor
`0x511d70`/`0x535170`, initializes the observed free list, then performs ordered
inline insertion (retail unrolls four slots). It leaves each script time field
untouched. Acquire scans free counts, allocates/constructs a block when needed,
reloads the pool head before prepending, removes the first slot through actual
`0x505400` and increments used count.

Release uses unsigned half-open membership `[block+0xc, block+0x68c)` with no
alignment check. It calls actual insertion wrapper `0x4951b0`, decrements used
count, and on zero removes the block through actual `0x505400` before virtual
deletion with flags one. Block destruction drains the free list inline, calls
actual `0x511670`, reverse-destructs all 32 embedded scripts and writes the base
vtable. Each embedded script normal destructor includes unconditional event
free, even for null pointers. The script deleting wrapper calls actual normal
script destruction first; flags bit zero then selects base virtual destruction
with flags zero and header-pool release. It returns the original receiver bits,
including when release freed its containing block.

`UG2_EXE=... build/venv/bin/python tools/verify_pc_feng_script_pool.py
--compiler clang-cl --mutation-checks` passes **1,536 comparisons** (256 for
each root) and rejects **12 semantic mutations**. Original helpers include
19,456 normal script destructions, 9,536 script constructions, 765 track
destructions and actual list/virtual cleanup. Only heap calls are controlled.
Full arena and both pool globals, ordered method/heap snapshots, receiver
results where contracted, preserved registers, stack cleanup and restored FS
exception head are compared. Cases cover partially full/exhausted/chained
blocks, allocation-time pool-head changes, name/event pointer changes during
free, cookie-based nested track destruction, null/outside/boundary/misaligned
release pointers, last-slot block deletion and flags 0/1/0x100/0x101/0xffffffff.
Mapped malformed states are arithmetic/control-flow evidence, not proof of
allocator or C++ lifetime validity. Allocation failure throughout all paths,
cycles, invalid pointers, concurrent mutation, exceptional unwind and byte
identity remain unproven. MSVC 7.1/wibo remains unavailable.

Native `FengScript` now owns a pooled pointer, accessed through `Header()`.
Construction still supplies time zero as an explicit reader binding. Moves
transfer the pointer and native metadata; destruction calls actual deleting
wrapper with flags one. Native event scheduling now keeps an `FEScript` pointer
directly. Native interpolation finds its metadata by matching that stable header
pointer within the object's wrappers. Both replace casts that assumed an inline
header occupied the first bytes of a native wrapper. Metadata, clocks and
interpolation remain native seams. Script owner lists are still projected and
detached by native object adapters before wrapper teardown.

The native vtable bridge verifies and patches script/header-block slots
`0x79ad28`/`0x79bf54` in addition to its five prior slots. No retail text is
executed in the demo. Asset-free fixtures retain private data pages. New fixture
coverage allocates 70 named scripts across three blocks, gives each track/event
storage, verifies header addresses survive native vector growth, and releases
all blocks through actual normal/deleting routines. Move tests now check an
empty source owner pointer rather than cleared fields in an embedded header.
Native and mod-enabled builds and all three CTests pass.

The full reader trace identifies additional Ti dependencies before production
replacement: `0x504a00` maps selector through `0x7992f4` and compares signed
track state bytes. At `0x55194a..0x551972`, context +0x3c supplies the registry
base (+0x4134); `0x505480` takes object type +0x18 and returns a descriptor,
then `0x5147f0` takes the Ti selector as its index. Each callee pops four bytes;
the first leaves the already-pushed selector for the second. Earlier decompiler
void signatures hid this dependency. Registry construction/binding and complete
reader ownership remain pending; no invented Ti metadata is supplied.

## Script reader Ti lookups (2026-10-18)

`FEngTrackLookup.cpp` recovers the three lookups reached only from the script
reader's `Ti` branch (`0x5518f6`, `0x55196b`, `0x551972`):

- `0x504a00` (67 bytes) `FEScript::FindTrack`: thiscall, 4-byte cleanup. It
  maps the selector through `0x7992f4` before testing the count, then compares
  `MOVSX` of track +7 against the key across `count1C` tracks with an unsigned
  bound (`JBE`/`JC`), returning the first match or null. It is also called from
  `0x4abbd7`.
- `0x505480` (31 bytes) `FEngTypeRegistry::Find`: walks +8/next +4 and returns
  the first descriptor whose +0x24 equals the argument, or null. The reader
  passes object type +0x18 with ECX = context +0x3c → manager +0x4134, so the
  registry is now typed as `FEngQueue::types4134` (0x14 bytes).
- `0x5147f0` (31 bytes) `FEngTypeDescriptor::GetProperty`: zero-based node of
  the +0x1c list (next +4), null past the end. The reader passes its result as
  ECX unchecked and later reads property +0x14/+0x18/+0x1c, so a missing
  descriptor faults in the original; no guard is added.

A scan of `InGameCommon.bun`, `FrontB.lzc` and every `GLOBAL` bundle found
56,804 scripts using only `Sh`, `FI`, `To`, `Kd`, `EV` and `Sc`; no retail
script contains `Ti`, `Sn`, `Tb`, `Ta`, `Tt`, `Sf`, `Sl` or `SI`. The registry's
producer is therefore not exercised by retail packages and remains unrecovered;
no descriptor contents are invented.

`UG2_DIR=... build/venv/bin/python tools/verify_pc_feng_track_lookup.py
--compiler clang-cl --mutation-checks` passes 12,288 comparisons (4,096 per
routine; 1,144/2,408/1,211 non-null results) and rejects six mutations:
unsigned state byte, last match, signed count, unmapped selector, skipped list
head and one-based indexing. A quarter of track cases substitute controlled
key-table values to expose the signed compare; negative counts are covered only
with an in-arena match. Cycles, invalid pointers, MSVC 7.1 and byte identity
remain unverified. Native integration follows with reader `0x5515e0`.
