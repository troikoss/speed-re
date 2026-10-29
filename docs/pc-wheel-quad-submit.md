# Wheel quad packing and submission (2026-10-29)

`CarRenderQuadSubmit.cpp` recovers the complete `0x5c5580` body (462 bytes)
and its counter tail `0x5bcfc0` (7 bytes). The native ABI bridge now forwards
through this source and the previously recovered texture/world upload bodies.
GPU device/effect services remain explicit unsupported boundaries in production.
The demo still selects gate zero; this does not activate secondary-wheel pixels.

The reference is UG2 PC v1.2, executable SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
`config/SPEED2_EXE_1_2/car-render-quad.json` fingerprints both complete spans.
Read-only Ghidra extraction from the saved project confirmed the bodies and
call boundaries after live MCP discovery returned no instances. Project files,
retail bytes, captures and generated disassembly remain outside commits.

## Defined payload and call order

The body snapshots four vertices before calling any helper. Each vertex occupies
32 bytes, of which six DWORDs are defined: XYZ, packed colour and UV. Position
fourth words, quad byte `+0x70` and the fourth function argument are unused.
The final two DWORDs of each output vertex are uninitialized in the binary;
the recovery does not add stores or claim equivalence for those 32 bytes.
All 96 defined bytes use integer copies, including signaling-NaN payloads.
Colour swaps input bytes zero and two while retaining bytes one and three.

The original sequence is preserved:

1. Bind texture through `0x5bcc30`.
2. Read the current mode pointer at `0x870848`, then its view index at `+4`,
   and call world upload `0x5bdde0`.
3. Read extra texture token `0x8708b4`. If nonzero, call the current effect's
   SetTexture (`vtable +0xd0`) with parameter slot 10's handle (`params +0x1b4`).
   A zero handle does **not** suppress this call.
4. Read byte `0x7ff05c`. If nonzero, Commit (`+0x10c`), reread current technique,
   clear vertex-cache word `0x87092c`, BeginPass (`+0x108`, pass zero), draw
   immediate (`device +0x14c`, kind 6, two primitives, stride 32), then reread
   current technique for EndPass (`+0x110`).
5. Always tail into `0x5bcfc0`, incrementing DWORD `0x872f08` with unsigned wrap.

All API HRESULTs are ignored. A disabled draw still binds, uploads, optionally
sets the extra texture and increments the counter. Callback-driven changes
therefore matter: binding may initialize the mode pointer or alter the input
quad after its snapshot; uploads may change the extra texture; SetTexture may
change the draw gate; Commit and Draw may select different effects for subsequent
calls. The native bridge adds no early context checks. Cached inactive calls
can validly finish with a null technique. The counter name describes this
observed increment, not an exhaustive audit of its other users.

`CarRenderViewByIndex` (`0x48b1e0`) moves unchanged into its own translation unit.
This lets world upload link the view leaf without pulling queue/pool ownership
into native callers. `OriginalSymbols.ld` binds the observed upload/cache globals.
`NativeCarRenderApi.cpp` provides 18 typed fallback boundaries; each reports its
name and aborts. Probe fixtures override these explicitly. No device operation
returns fabricated success in production.

## Verification and limits

For each of MSVC 7.1 `/Ox /Ob0` and GCC i386 `-O1`,
`tools/verify_pc_car_render_quad.py` compares **16,384 isolated calls** (8,192
submissions and 8,192 counter calls), plus **512 composed submissions**. The
composed cases execute recovered texture binding and world upload; graphics APIs
are controlled. The isolated cases control those two engine boundaries as well.
Both executable and function fingerprints are checked before execution.

Tests compare full arena/global state, API arguments, defined vertex payloads,
call order and state visible at each boundary. Cases include zero/random handles,
disabled/enabled gates, failing HRESULTs, counter wrap, mutated input/context,
cache hits with null technique and a mode pointer initialized during binding.
Isolated calls preserve caller registers, stack cleanup, all eight x87 TOPs,
zero to two caller-owned x87 values, sticky flags, twelve precision/rounding
combinations and MXCSR rounding/DAZ/FTZ state. Nineteen physical source mutants
are rejected per compiler. Composed arithmetic does not establish x87 exception
flag parity.

The native probe replays **1,152 original captures**, including **128 composed**
cases and **6,435 observed engine/API calls**, through the native ABI bridge.
Observation wrappers invoke unchanged
recovered helpers compiled under alternate names. It compares full serialized
traces, final arenas and all captured global regions byte for byte; CRCs are
additional diagnostics. Only the 96 defined draw bytes are captured. Every row
also checks a standalone counter call and floating-point preservation. Native
tests seed zero to two caller-owned x87 values, verify their contents, TOP/tags,
control and isolated sticky flags; they do not enumerate all eight native TOPs.
Nine native source mutants are rejected, and a missing API terminates with its
named diagnostic.

The consume/view regression, full native build, existing 4,096 secondary-wheel
captures and SKYLINE orbit/drive smokes pass. Orbit (60 frames) retains
`ca993eba` / `7141bcda`; scripted drive (120 frames) retains
`eb0428b7` / `69927c13`. All 829 texture records/staged handles retire.
These gate-zero smokes do not exercise GPU submission.

This establishes finite behavioral agreement, **not byte identity or rendering
parity**. Frame/texture graphs are synthetic and GPU services controlled. Actual
effect/device resources, frame/cache ownership, dispatch activation and the
remaining wheel part/state producers are open. Invalid pointers, reentrant
rendering, unmasked hardware exception delivery and complete input coverage are
not established.

```sh
export UG2_EXE='/path/to/Need for Speed Underground 2/speed2.exe'
build/venv/bin/python tools/verify_pc_car_render_quad.py --gcc --mutation-checks --write-native-fixtures /tmp/wheel-quad-submit-fixtures.bin
# Run MSVC/wibo jobs sequentially.
build/venv/bin/python tools/verify_pc_car_render_quad.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/venv/bin/python tools/probe_pc_car_render_quad_native.py /tmp/wheel-quad-submit-fixtures.bin --mutation-checks
build/venv/bin/python tools/verify_pc_car_render_consume.py --gcc
build/venv/bin/python tools/verify_pc_car_render_consume.py
```

Reports and mutation logs live under ignored `build/analysis/car-render-quad`.
The approximately 591 MiB native capture file is generated locally from the
player's executable and must not be committed.
