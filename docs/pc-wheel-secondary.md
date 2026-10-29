# Secondary wheel quad, caller flags and Y rotation (2026-10-29)

`CarWheelSecondary.cpp` recovers the complete secondary tyre-quad producer
`0x620600`, its raw rim-attribute helpers, quad initialization and view wrapper.
The empty native method is removed. The producer and wrapper execute recovered
code. A subsequent [quad submission recovery](pc-wheel-quad-submit.md) implements
`0x5c5580`; its actual effect/device services remain unsupported native boundaries.
Gate 3 is not selected by the current native session.
This does not activate a replacement tyre effect or establish rendering parity.

The reference is UG2 PC v1.2, executable SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
`config/SPEED2_EXE_1_2/wheel-secondary.json` pins the recovered bodies, caller
fragment and constants/name. Read-only Ghidra extraction from the saved project
confirmed all function extents and callees; live MCP discovery returned no running
instances. No project changes, retail bytes or generated disassembly are committed.

| Original address | Bytes | Recovered behavior |
| --- | ---: | --- |
| `0x620600` | 1,247 | Local tyre-mask quad generation and submission |
| `0x61b500` | 29 | First matching part-attribute payload, or raw fallback |
| `0x4fe120` | 13 | Attribute key `0xeb0101e2`, fallback zero |
| `0x5bb1c0` | 94 | Quad XYZ/UV/colour/flag initialization |
| `0x5c7e80` | 31 | View wrapper forwards four of five stack arguments |
| `0x5bada0` | 177 | Y rotation, independent sine narrowing and matrix product |
| `0x622eee..0x622f0c` | 31 | Caller fragment producing the wheel flag word |

These are **1,591 complete original function bytes**, plus the 31-byte caller
fragment. Attribute lookup composes with recovered `FindRecord` (`0x60fef0`),
including its invalid-index and duplicate-first-match behavior.

## Producer and exact arithmetic

The producer reads the front rim at `ri->carObject + 0x604`. A null part selects
17. Otherwise it uses the **signed low byte** of the raw attribute value; a missing
attribute returns zero. It hashes `TIREMASK`, requests texture lookup with
`fallback=1, rawMode=0`, initializes a quad and overwrites its UVs and four
`0x80808080` colours. The world matrix is forwarded unchanged to submission.

The local identity matrix is rotated about Y by `0x1999` angle units. Radius
calculation retains the original order: rim × binary32 constant at `0x7a889c`
(approximately 0.0127), then × 1.25 at `0x7a6728`. Positive and negative radii
are narrowed independently from the unrounded value. The plane offset is the
binary32 value at `0x7a672c` (approximately 0.122).

The four points preserve the original per-component grouping and float spills,
including the separately narrowed positive-axis products and translated bases.
They are not replaced with a generic transform. Quad initialization leaves each
position's fourth DWORD and bytes `0x71..0x73` untouched. Those bytes are checked
for preservation in standalone initialization tests and excluded from the
original producer's otherwise defined submission payload. XYZ, UVs, colours and
flag comprise 97 compared bytes.

The view wrapper ignores its receiver and fifth stack argument, forwards the
first four arguments to `0x5c5580`, and uses the original RET 0x14 ABI. The native
ABI bridge now forwards into the recovered 462-byte packing/render body; see
[the subsequent evidence](pc-wheel-quad-submit.md). Its GPU services still fail
visibly pending backend integration. The verification below retains submission
as a controlled boundary to isolate this producer.

`WheelRotateY.cpp` replaces the earlier Y-rotation approximation with the original
extended sine/cosine entry points, independent positive/negative binary32 stores
and the matrix-product ABI call. With angle `0x1999` and control word `0x067f`,
the original negative sine is `0xbf16760c`, while the positive sine is
`0x3f16760b`. Negating the narrowed positive value loses this distinction.
Separate volatile extended values retain conversion order under GCC's default
rounding assumptions. The radius product is staged as well: MSVC `/Ox` otherwise
reassociates the two scale factors, changing 24-bit-precision results. The raw
rim-byte range keeps those stored product intermediates exactly representable
in MSVC's double-sized `long double` at 53/64-bit precision.

`NativeMatrixDispatch.cpp` selects the already recovered default scalar D3DX
multiply/transpose implementations. It replaces the native multiplication loop
that accumulated into a float after each term. The original optimized CPU
variants and their startup selection remain separate. This ABI bridge is shared
by the native vehicle/camera code and the native secondary-wheel probe.

## Corrected caller provenance

The previous inventory incorrectly attributed wheel `flagsCC` to `0x60d1b0` at
`0x621b9e`. That function generates geometry and returns three times its input
count. Its result occupies an earlier lifetime of the caller's stack slot.

The later instructions at `0x622ede..0x622f06` load the gate `0x8654a4`, test it
against 6, and store **0x8000 if equal, zero otherwise** into the slot consumed
by wheel draws. The isolated fragment includes an unrelated store and TEST using
other caller values; verification compares the defined flag result, not a claim
to recover the entire caller or its EFLAGS. `CarWheelCallerFlags` expresses this
predicate, and `NativeRunCarWheelBlock` now supplies its result. The default gate
remains zero. Its native symbol now binds the original mapped `0x8654a4` word,
shared with recovered record/light preparation, instead of a private copy. Native
gate/state scheduling is still incomplete.

Read-only Ghidra references for facing normal X/Y/Z and the threshold contain
only reads. No direct original writer or encoded data reference was identified
for those variables. Native defaults 0/1/0 and threshold zero match the image;
indirect writes are not ruled out. Gate `0x8654a4` has original writers, including
`0x5800ba`, `0x581a2e` and others. Full native game-state ownership is unresolved;
this pass does not invent a normal producer or activate a different gate.

## Verification and reproduction

For each compiler, `tools/verify_pc_wheel_secondary.py` performs **28,672 exact
comparisons** across seven entry points, including the flag fragment and direct
Y rotation. The first 3,072 cases enumerate every signed rim byte under all
12 combinations of x87 precision and rounding. Additional cases cover null parts,
invalid indices, missing/duplicate attributes, wildcard keys, opaque null texture
results, wrapper arguments, high angle bits, finite nonidentity matrices and
in-place/distinct rotation outputs. Arena bytes, defined quad fields, call
arguments/order, return values, stack cleanup, preserved registers and x87
control/empty-stack state are checked. Ten source mutants are rejected per
compiler.

Both MSVC 7.1 `/Ox /Ob0` and GCC i386 `-O1` execute the complete recovered chain,
including `FindRecord`, string hashing, extended angle bodies, Y rotation and
scalar D3DX multiplication. Texture lookup and `0x5c5580` submission are explicit
controlled services. They do not substitute geometry or arithmetic results.

The native probe replays **4,096 exact original quad captures** with the actual
recovered production rotation, angle, lookup and matrix ABI implementations.
Texture lookup and final submission are controlled capture boundaries. It checks
97 payload bytes, the caller flag, texture/world arguments, one lookup and one
submission, x87 control and empty-stack state. Five physical mutants are rejected:
rim signedness, scale order, negative-sine rounding, corner order and flag gate.
Original-emulator FP flags and byte identity are not inferred from this replay;
no GPU code or retail machine code executes natively.

```sh
export UG2_EXE='/path/to/Need for Speed Underground 2/speed2.exe'
build/venv/bin/python tools/verify_pc_wheel_secondary.py --gcc --cases 4096 --mutation-checks --write-native-fixtures /tmp/wheel-secondary-fixtures.bin
# Run MSVC/wibo jobs sequentially.
build/venv/bin/python tools/verify_pc_wheel_secondary.py --cases 4096 --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/venv/bin/python tools/probe_pc_wheel_secondary_native.py /tmp/wheel-secondary-fixtures.bin --mutation-checks
```

Reports, captures and mutation logs remain under ignored build/temp paths.
The full native build and scalar-matrix/wheel-block regressions pass. SKYLINE
orbit (60 frames) retains fingerprints `ca993eba` / `7141bcda`; the finalized
drive-camera smoke (120 frames) reports `eb0428b7` / `69927c13`. Both release all
829 texture records and staged handles. These runs keep gate zero and therefore
do not claim production secondary-quad rendering. Rim population, LOD B/C,
spin-state inputs, slot selection, additional lights, frame scheduling and the
remaining GPU/effect seams stay open.
