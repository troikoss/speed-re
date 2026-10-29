# UG2 SSE power recovery (2026-10-29)

`CrtPowerSse.cpp` recovers the complete **2,861-byte** SSE power body at
`0x764749..0x765275` from UG2 PC v1.2. The existing power entry, private x87
wrapper and production effect-VM bridge now reach recovered arithmetic on both
branches. `NativeFxCrtPowerSsePlatform.cpp` forwards the wrapper's numeric service
to the recovered body. The CPU flag still starts at its original zero BSS value;
CRT CPU initialization is a separate unresolved producer.

The reference executable SHA-256 is
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
`config/SPEED2_EXE_1_2/crt-power-sse.json` fingerprints the function and tables.
Read-only Ghidra extraction of the saved `speed2.exe` project confirmed the
function extent and sole direct callee, the previously recovered SSE error
service at `0x7694d0`. The live MCP instance list was empty during this pass;
headless analysis opened the saved project read-only without reanalysis or saves.
No generated disassembly, executable bytes or table contents are redistributed.

## Arithmetic and boundaries

The source retains the original three reciprocal/log reduction stages, head/tail
splits, signed exponent encodings, packed polynomial operations, 128-way
exponential table selection and separate extreme/subnormal reconstruction paths.
Integer vector operations preserve lane width, unsigned wrapping, saturating
shift-count behavior, signed-word maxima and byte-sign masks. Labels retain
original branch addresses. Scratch values are stored C++ values, without binding
physical registers or selecting instructions to force a byte match.

| Original data | Extent | Use |
| --- | ---: | --- |
| `0x7d4310` / `0x7d4720` | 129 doubles / 129 pairs | First reciprocal/log reduction |
| `0x7d4f30` / `0x7d5340` | 129 doubles / 129 pairs | Second reduction |
| `0x7d5b50` / `0x7d6360` | 257 doubles / 257 pairs | Third reduction |
| `0x7d7370` | 32 bytes | Packed logarithm coefficients |
| `0x7d7390` | 128 pairs | Exponential reconstruction table |
| `0x7d7b90..0x7d7c37` | 168 bytes | Polynomial coefficients, masks and exceptional constants |

The 14,632-byte mapped data span includes alignment padding between tables.
Scalar reads copy eight bytes; packed reads copy sixteen. The player's own
SHA-checked image supplies these data through `OriginalImage` and the linker
binding. This remains a retail-data dependency.

`NativeFxCrtSseInstructions.cpp` supplies only scalar ADD/SUB/MUL/DIV, packed
ADD/MUL, CVTDQ2PD and rounding-sensitive CVTSD2SI semantics. Packed upper lanes,
operand order, MXCSR rounding, sticky flags, DAZ and FTZ remain observable. In
particular, the apparently dead division at `0x764ca9` and signaling-NaN multiply
at `0x764dfa` must execute for their flags. No host `pow` or replacement numerical
formula is used.

The raw result service copies double bits without an x87 load. Its caller
performs the original final FLD once. `D3DXCrtPowerSse80` is the equivalent explicit
extended-return adapter for verification. Integer payload copies avoid premature
NaN quieting and changes to x87 status.

| Error selector | Original effect |
| --- | --- |
| 24 / 25 | Overflow / underflow callback paths |
| 26 | Zero-base, zero-exponent result of one, without callback |
| 27 | Pole callback, including the division flag effect |
| 28 | Domain callback, including negative nonintegral bases |
| 29 | Base-NaN/zero-exponent handling through the first-input copy path |
| 1006 | Propagated NaN callback with the already computed result |

All seven selectors are reached by the corpus. Shared error handling, callback
record copies and errno behavior execute recovered routines; they are not
substituted by a synthetic result service in this suite.

## Incoming lanes

The original uses partial low-lane loads. Dataflow inspection shows that every
reachable packed FP operation has fully defined input lanes: full table loads,
CVTDQ2PD, PSHUFD or UNPCKLPD define the lanes before arithmetic. Other uses of
incoming upper lanes are discarded by low-byte masks or low-quad shift counts.

The unmasked full-byte predicate at `0x764e52` could depend on old upper lanes,
but requires base exactly -1 and a nonfinite exponent in the common logarithm
path. The entry routes that operand pair through `0x764ba0`, before this path;
the original operands have not changed. The recovered source initializes scratch
lanes to zero. An additional **8,192 operand cases × four full incoming XMM
states** (zero, all-one and two independently seeded sets of eight 128-bit values)
checks returns, flags, callbacks and thread state for this boundary. This finite
audit supplements the reachability reasoning; it is not arbitrary-state proof.

## Verification

`tools/verify_pc_crt_power_sse.py` runs both the original arithmetic and compiled
recovery. For each compiler, **32,768 comparisons** cover the body, private
wrapper, dispatch entry and production extended-return bridge. They compare all
ten return bytes, FP controls/status, x87 TOP/tags/caller-owned values, errno and
callback payloads. A further **1,024 composed effect-VM programs / 1,536 power
calls** check scalar/vector widths and overlapping outputs. Both MSVC 7.1
`/Ox /Ob0` and GCC i386 `-O1` are used. Ten source mutants are rejected per compiler. Mutation checks exercise reduction tables,
packed tails, negative-base parity, sign, domain/underflow exits and callback
arguments. Reports describe compiler-specific bounded behavior; byte identity
has not been established.

The corpus combines exceptional bit patterns, random raw operands, ordinary
mantissas sensitive to table selection, huge and tiny exponents, signed zeros,
subnormal bases, integer parity boundaries, extended incoming wrapper values,
all rounding modes, precision controls, seeded sticky flags and DAZ/FTZ states.
Emulated unmasked cases check control flow; actual fault delivery is excluded.

`tools/probe_pc_crt_power_sse_hardware.py` compares **8,192 native body executions**
against original control flow with independent physical instruction evaluation.
Unicorn retains the original integer instructions, branches, memory accesses and
table choices. Each SSE FP instruction and each x87 double load/store is evaluated
by test-only native ISA operations in `ug2crtpowersse_probe`, which use different
scratch registers from the production adapters. This covers the emulator's
NaN/DAZ/FTZ/status limitations without excluding those cases or executing retail
machine code natively. The recovered body executes separately with its production
ISA adapters.

This hardware corpus compares exact extended results, MXCSR including sticky
flags, x87 control/status/TOP/tags, retained -1/+1 caller values, errno and all
32 callback-record bytes plus callback FP state. It includes **3,676 default
callbacks**. Five native mutants are rejected: reversed subtraction, truncating
integer conversion, scalar replacement of packed multiply, removal of the dead
invalid-flag operation and incorrect second callback argument. All hardware
exceptions remain masked; custom callback edits are covered by the emulation
suite, not by this default-zero hardware callback fixture.

```sh
export UG2_EXE='/path/to/Need for Speed Underground 2/speed2.exe'
build/venv/bin/python tools/verify_pc_crt_power_sse.py --gcc --cases 8192 --entry-audit --mutation-checks
# Run MSVC/wibo verifiers sequentially.
build/venv/bin/python tools/verify_pc_crt_power_sse.py --cases 8192 --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/venv/bin/python tools/probe_pc_crt_power_sse_hardware.py --cases 8192 --mutation-checks
```

Generated reports and mutation logs stay in `build/analysis/crt-power-sse/`.
The full build, existing x87 power/VM regressions and 60-frame SKYLINE smoke are
checked with the integration. Wheel fingerprints remain `ca993eba` / `7141bcda`;
829 texture records and their staged handles retire to zero. The VM regression
uses 2,048 cases / 4,096 comparisons per compiler with explicit caller-service
fixtures, plus 128 cases / 256 comparisons per compiler with bounded original
runtime execution. An attempted expansion of that older unhooked-runtime corpus
to 2,048 cases faults in the original alone at case 922: errno reaches the
Win32 thread-data/TLS import (`0x761e3c`, unresolved target `0x7c809750`). That
fixture does not supply the thread service. The new power suites supply it
explicitly and cover the associated errors; the broader fixture is not claimed
as a passing 2,048-case original-runtime test.

Remaining boundaries include CRT CPU/thread/startup and bypass-flag producers,
custom native callback dispatch, actual OS and unmasked exception delivery,
native MSVC hardware execution, byte matching and native GPU/effect activation.
The other wheel scheduling, light, population and rendering seams remain open.
This recovery establishes behavior only for the specified UG2 reference build.
