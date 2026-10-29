# Effect state-block caches and reset chain (2026-10-29)

`EffectStateCache.cpp` recovers eleven complete UG2 PC v1.2 routines behind the
pass layer's vertex/pixel cache calls: **2,846 original bytes**. The two native
cache ABI adapters now execute recovered code. GPU state-block, shader, constant
and sampler operations remain typed unsupported platform services; this does not
activate effect rendering in the demo.

The reference executable SHA-256 is
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
`config/SPEED2_EXE_1_2/effect-state-cache.json` fingerprints every body, the
392-byte table at `0x7b6c54` and the default-value prefixes it dereferences.
Read-only headless Ghidra extraction supplied disassembly and decompilation after
MCP discovery found no instances. Instruction reads, writes and call results
establish the behavior below; decompiler signatures alone do not. Project files,
retail bytes, generated listings and captures stay outside commits.

| Original | Bytes | Recovered behavior |
| --- | ---: | --- |
| `0x66403c`, `0x664197` | 347 each | Vertex/pixel cache admission and capacity updates |
| `0x66356e`, `0x66369f`, `0x6637d0`, `0x663901` | 305 each | Four state-block capture/replace paths |
| `0x660205`, `0x6603b4` | 289 / 293 | Vertex/pixel shader and constant reset |
| `0x660326`, `0x6604d9` | 142 / 131 | Vertex/pixel texture and sampler defaults |
| `0x65aa5d` | 77 | Per-element vertex boolean conversion/upload |

Two include files instantiate the instruction-equivalent gate/capture patterns
at their verified offsets. The constant reset routines share their identical
allocation prefix as a source helper; that helper is not an additional original
function. Unknown technique fields remain addressed by observed offsets.

## Admission, capture and ownership

Both gates select the technique using effect `+0x42` and stride `0xb4`. Flag 8
permits caching. Flag `0x10` enables constant/shader capture: existing block,
shader-ready state and three unsigned word capacities decide whether it is
needed. Requested sizes combine with old maxima, and a retained shader-ready
flag cannot be dropped by a later disabled request. Capacity fields update only
after successful capture. Sampler capture uses the fourth word independently,
including when constant capture is disabled. Its maximum is retained on failure.

Vertex slots are `+0x94` for constants and `+0x8c` for samplers; pixel slots are
`+0x98` and `+0x90`. Each creation path saves its slot address before callbacks,
then records two state blocks. Each recording calls BeginStateBlock, recovered
debug mute (`0x671aed`) with 1, the appropriate reset, debug mute with 0, and
EndStateBlock. A failed reset still ends recording, ignores that End result and
keeps the reset failure. Other Begin/End failures stop immediately.

After recording, the order is: capture block zero, apply the current old block
if present, capture block one, apply block zero, release the old block and publish
block one. Temporary blocks release in index order on exit. The published block
transfers ownership and is not released by cleanup. The manager token and old
block value are reread at the original call boundaries; the destination slot
address remains the saved one. A callback can change technique selection or
replace a slot before it is applied/released.

Partial state is retained on errors. In particular, a grown scratch buffer is
not rolled back when a later GPU operation fails, and a successful constant
capture can remain published if subsequent sampler capture fails. Final
nonnegative results retain their observed values rather than being normalized to zero.

## Reset data and table reads

The shared scratch buffer grows to the maximum of word zero × 16, word one × 4
and word two × 16. Growth allocates and zeroes before freeing the previous
buffer, then publishes its pointer/capacity. OOM preserves the old fields.
Reuse preserves the existing bytes; the routines do not zero the buffer again.
Tests retain raw signaling-NaN payloads in reused buffers without FP conversion.

Vertex reset optionally clears the vertex shader, then uploads float vectors,
individual booleans and integer vectors in that order. The boolean helper maps
each nonzero DWORD to `0xffffffff`, makes one upload per element, uses wrapping
start-index addition and stops on a negative result. It returns zero after any
nonnegative sequence. Pixel reset optionally clears the pixel shader, then
uploads floats, booleans in one call, and integers.

Vertex sampler stages begin at **0x101**, using table rows 1 through 13. Pixel
stages begin at zero and use rows 1 through 12. Every stage clears its texture,
then loads each row's low-24-bit key and either zero or the second DWORD at its
default-value pointer. The fourth size word is reread between stages. Disassembly
shows that `0x660326` returns the last API result, despite its void decompiler
signature. Both zero-count paths return zero.

Sampler-only capture leaves the first three local size words indeterminate.
Its reset helper reads only word three; tests do not compare those unused words.
Capture fixtures keep flag `0x200` clear. The retail set-bit path skips creation
and reaches a null temporary block; successful behavior and native invalid-
pointer exception identity are not established for that path.

## Verification and native boundary

For each of MSVC 7.1 `/Ox /Ob0` and GCC i386 `-O1`,
`tools/verify_pc_effect_state_cache.py` compares **22,528 original/rebuilt cases**,
2,048 per routine, and rejects **38 physical source mutations**. Gate cases
execute the complete recovered capture/reset/boolean chain. Only device/state-
block calls, debug mute and allocation outcomes are controlled.

Checks include full final arenas/heaps, allocation ownership, exact arguments,
defined upload payloads, call order, effect/technique state at boundaries,
HRESULTs, callee-saved registers and stack cleanup. Cases cover 256-vector
buffers, retained data, OOM, failed reset/end/capture/apply, output objects even
on failed End, partial publication, manager/technique/slot changes, request/data
changes during calls, differing sampler tables, boolean masks and start wrap.
All eight x87 TOPs, zero to two caller-owned values, sticky flags, twelve
precision/rounding combinations and MXCSR rounding/DAZ/FTZ states are checked.

`ug2effectstatecache_probe` replays **1,364 original captures** from 124 selected
contexts, observing **27,896 controlled GPU calls** through the production cache
ABI adapters. Full call traces, final arenas and the bounded 20 KiB heap model
match byte for byte, including allocation counts and synthetic state-block
reference/capture/apply fields. **Twenty native source mutations** are rejected.
Native checks preserve FP controls, sticky flags, tags, three push-derived TOPs,
caller-owned values and MXCSR. Synthetic GPU objects and debug mute remain
controlled; no retail machine code executes natively.

The full native build, previous 896 pass/texture captures and gate-zero SKYLINE
orbit/drive smokes pass. Orbit (60 frames) retains `ca993eba` / `7141bcda`;
scripted drive (120 frames) retains `eb0428b7` / `69927c13`. All 829 texture
records/staged handles retire. These runs do not exercise GPU effect drawing.

This establishes finite behavioral agreement, not byte identity or rendered
parity. Native GPU objects, actual recording/restoration, general record/group
application (`0x669286`, `0x66a25f`, `0x668dfd`), frame/cache scheduling and the
remaining wheel part/state producers are open. Arbitrary pointers, unbounded
counts, full debug/OS behavior in this chain and reentrant callbacks are not
established. Other SPEED titles/builds remain unverified.

```sh
export UG2_EXE='/path/to/Need for Speed Underground 2/speed2.exe'
build/venv/bin/python tools/verify_pc_effect_state_cache.py --gcc --mutation-checks --write-native-fixtures /tmp/effect-state-cache-native.bin
# Run MSVC/wibo jobs sequentially.
build/venv/bin/python tools/verify_pc_effect_state_cache.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/venv/bin/python tools/probe_pc_effect_state_cache_native.py /tmp/effect-state-cache-native.bin --mutation-checks
```

Reports and mutations stay under ignored `build/analysis/effect-state-cache`.
The approximately 246 MiB original capture file is local and must not be committed.
