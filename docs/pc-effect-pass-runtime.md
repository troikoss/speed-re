# Effect pass control and texture ownership (2026-10-29)

`EffectPassRuntime.cpp` recovers six complete UG2 PC v1.2 library bodies,
**1,179 original bytes**, behind secondary-wheel quad submission. Four native
renderer APIs now dispatch through the recovered effect class: BeginPass,
Commit, EndPass and SetTexture. Actual device-state application and GPU classes
remain open. The demo still uses gate zero; no shader pixels are activated.

The reference executable SHA-256 is
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.
`config/SPEED2_EXE_1_2/effect-pass.json` fingerprints the complete functions
and the four method slots in effect class table `0x7b6fe0`. Read-only headless
Ghidra extraction supplied decompilation and disassembly; live MCP discovery
returned no instances. The code follows instruction reads, writes, calls and
arithmetic rather than inferred decompiler types. No project changes or retail
bytes are committed.

| Original | Bytes | Recovered behavior |
| --- | ---: | --- |
| `0x66acd2` | 282 | BeginPass, effect slot `+0x108` |
| `0x66ac17` | 187 | Commit, slot `+0x10c` |
| `0x65adcb` | 90 | EndPass, slot `+0x110` |
| `0x6691a1` | 229 | Group application control |
| `0x666a68` | 179 | Deferred texture recording/retain |
| `0x66d7d9` | 212 | SetTexture, slot `+0xd0` |

## Pass ordering and state

BeginPass requires a nonzero effect `+0x3c` state and an unsigned index below
technique `+0x34`. The technique index is the high word at effect `+0x40`.
Changing the selected pass releases old group references unless the previous
pass is `0xffff`, stores only the low word at effect `+0x44`, then adds the new
references. Reference adjustment uses the existing recovered `0x65ae25` body.
The two technique-index words determine whether the all-reference delta applies.
Both failure paths preserve any earlier writes.

After reference selection, BeginPass clears flag `0x80`. An already set `0x100`
returns `0x8876086c`; otherwise it applies the pass group with index `0xffff`,
setting `0x100` only for a nonnegative result. Every exit checks flag 1, restores
saved x87 precision through recovered `controlfp(saved, 0x30000)`, and clears
flag 1. Other control modes remain those current at that point.

Group application narrows its group index to a word for lookup while forwarding
the original full argument to record calls. Flag 8 first requests two cache
operations; group flag 8 requests vertex or pixel preparation according to its
`+0x18` word. State 4 with group flag 1 selects the fast-group path. Its ordinary
record loop propagates errors even if a callback changes the state to 1. The
other state branch rereads the state after each failed record and suppresses
errors only in state 1. Positive record results are discarded at normal exit.

Commit sets flag 4, walks packed group/record words in the dirty queue and clears
record flag 2 before testing activity. A nonzero group reference word and dirty
record select either cached-record application (record flag 8) or general-record
application with index `0xffff`. These results are ignored: later entries and
cleanup still run. The queue pointer/count are reread across callbacks. Commit
then restores saved precision if required, zeroes the count, clears flag 4 and
returns zero.

EndPass clears byte `+0x119` bit zero, clears record flag 2 for every queued entry,
zeroes the count if nonempty and returns zero. It does not apply records or
restore FP controls. Queue records may alias the effect count; the recovery
retains the original rereads rather than snapshotting the loop limit.

## Texture parameters and deferred ownership

SetTexture accepts a complemented record directly. For a nonnegative handle it
first writes scratch descriptor kind 5, shape 4 and element count zero, then
calls the recovered parameter resolver with depth 1 and create 1. It rejects
unindexed arrays and kinds outside signed range 5 through 9. State 3 records a
deferred texture instead of updating resource references immediately.

Ordinary assignment compares the mapped reference first, using the owner's
shared-pool index map when present. Equal texture tokens return zero without
notification. A changed token invokes recovered object-resource writing
(`0x65ccc8`) and, if successful, notification (`0x66055c`). Existing buffer COM
retains/releases, shared mapping, dirty notification and queue insertion execute
in the composed tests. GPU texture classes remain unsupported natively.

Deferred recording allocates a 0x1c-byte value node and a 12-byte cookie/reference
array. Failure of the second allocation deletes the initialized value node.
Success retains a nonzero texture, sets reference kind 5 and value tag 12, then
publishes to the current tail. A retain callback can replace that tail, so it is
read after the callback. Untouched value fields retain their previous bytes.

`NativeCarRenderApi.cpp` validates the recovered effect class token before
calling these four bodies. Fourteen other renderer API fallbacks still abort.
`NativeFxPassDispatch.cpp` names five remaining state-dispatch boundaries:
`0x66403c`, `0x664197`, `0x668dfd`, `0x669286` and `0x66a25f`.
Production group preparation still reaches its previously recovered bodies;
its device services are separate. This is CPU integration, not GPU activation.

## Verification and reproduction

For each of MSVC 7.1 `/Ox /Ob0` and GCC i386 `-O1`,
`tools/verify_pc_effect_pass.py` compares **28,672 cases**: 4,096 for each of the
six bodies plus **4,096 full BeginPass → SetTexture → Commit → EndPass cycles**.
These cycles use real recovered notification/queue insertion and reference
adjustment, then drain the queued record through a controlled state service.
Forty-four physical source mutations are rejected per compiler.

Tests compare return values, full arenas/heaps, allocation ownership, exact call
arguments/order, effect state visible at boundaries, callee-saved registers,
stack cleanup and FP state. They cover unsigned/high-word pass indices, partial
reference changes, failing services, cache selectors, state changes during calls,
queue pointer/count changes, count aliases, mapped/equal texture references,
null textures, deferred-tail replacement and both forced allocation failures.
Some standalone cases explicitly control existing resolver/write/notification/
reference-adjustment bodies to check error propagation. Other cases execute
those bodies; the native captures and full lifecycles execute them throughout.
Shader preparation and device-state services are controlled here.

The native probe replays **896 original captures**, including **128 full cycles**,
through production renderer ABI adapters and the recovered code. Test-only
observation wrappers call unchanged reference, resolver and value bodies compiled
under alternate names. Allocation/free outcomes and device-state services are
controlled. Final arenas, the bounded 4 KiB heap model, full serialized call
traces, allocation counts and FP state match byte for byte. Fourteen native
source mutants are rejected.

Source tests seed all eight x87 TOPs, zero to two caller-owned values, six sticky
exception flags, twelve precision/rounding combinations and MXCSR rounding modes.
Native tests check the three TOPs obtained by pushing zero to two values,
contents/tags, sticky flags, defined control bits and MXCSR. Hardware forces
reserved CW bit 6 on while Unicorn permits it clear after a mapped CRT write;
that reserved bit is excluded from native control/trace comparisons.

The full native build, prior 1,152 quad-submission captures and orbit/drive smokes
pass. Orbit (60 frames) retains `ca993eba` / `7141bcda`; scripted drive (120 frames)
retains `eb0428b7` / `69927c13`. Both retire all 829 texture records/staged handles.
These gate-zero smokes do not exercise effect drawing.

These are finite behavioral checks, not byte identity, complete input coverage
or rendered parity. Actual shader/device-state application, GPU texture objects,
frame/cache ownership, native dispatch activation and remaining wheel part/state
producers remain open. Malformed pointers, unbounded queues, arbitrary callback
reentrancy and hardware exception delivery are not established.

```sh
export UG2_EXE='/path/to/Need for Speed Underground 2/speed2.exe'
build/venv/bin/python tools/verify_pc_effect_pass.py --gcc --mutation-checks --write-native-fixtures /tmp/effect-pass-native.bin
# Run MSVC/wibo jobs sequentially.
build/venv/bin/python tools/verify_pc_effect_pass.py --mutation-checks
cmake -S . -B build/native
cmake --build build/native
build/venv/bin/python tools/probe_pc_effect_pass_native.py /tmp/effect-pass-native.bin --mutation-checks
```

Reports and mutation logs remain under ignored `build/analysis/effect-pass`;
the approximately 120 MiB capture file is local and must not be committed.
