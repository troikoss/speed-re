# Ginsu engine-sound synthesizer

Reference: UG2 PC v1.2 `speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.

UG2 synthesizes player engine sound from `SOUND/ENGINE/GIN_*.gin` files (81 in the
retail copy checked; header `Gnsu20`, little-endian). The `GinsuSynthData` layout (size
`0xB0`) and the `GinsuSynthesis` layout (size `0x44`) are taken from the UG2 disassembly.
`BindToData` compares five magic bytes one at a time; the start format writes `samplerep` 8;
`SND_attrsetdef` writes `platformver` 0 and `rendermode` 0.

Sources: `src/Speed/Indep/Src/EAXSound/Ginsu/{Ginsu.h,GinsuData.cpp,GinsuSynth.cpp}`.
Engine setup: `src/Speed/Indep/Src/EAXSound/Ginsu/{EngineSound.h,EngineSoundSetup.cpp,EngineSoundController.cpp,EngineSoundUpdate.cpp}`.
Native seams: `src/Platform/Native/{NativeAudio.cpp,NativeAudio.h,NativeEngineSound.cpp,NativeEngineSound.h}`.

Current native status: recovered selection/loading/setup and playback caller/controllers
run in the demo, including both mode-1 banks. The newly recovered prepare/gain
layer is verified separately and awaits original component input production.
See [gain recovery](#engine-prepare-and-gain-production-2026-10-20) and
[native wiring](#native-playback-controller-wiring-2026-10-20) for current
results and remaining seams. Dated checkpoints preserve their earlier results;
the latest sections supersede earlier statements about native wiring.

## Recovered routines

| Address | Routine | Notes |
| --- | --- | --- |
| `0x466750` | `GinsuSynthData::GinsuSynthData` | vtable `0x789ec8` kept as a raw retail word |
| `0x466770` | `GinsuSynthData::BindToData` | magic `Gnsu2`; tables follow the 0x20 header; `minPeriod28` = smallest cycle length |
| `0x466840` | `FrequencyToSample` | linear interpolation over `segCount + 1` frequency positions; clamps to the table ends |
| `0x466910` | `CycleToSample` | linear interpolation over `cycleCount + 1` cycle positions |
| `0x4669d0` | `CyclePeriod` | central difference (scale `0x784260` = 0.5), result left unrounded in ST0 |
| `0x466af0` | `SampleToCycle` | interpolating search; Ghidra split the body at `0x466b2c` |
| `0x466590` | `DecodeBlock` | EA-XA: 19-byte blocks, 32 samples; tables `0x7ed92c`/`0x7ed93c`/`0x7ed950` |
| `0x466c60` | `GetSamples` | int16 saturation; decodes the next block eagerly at a block edge; callers discard the result |
| `0x466560` | `GinsuCeilToInt` (cdecl) | ceiling via round-to-nearest and a sign fix-up |
| `0x467160` | `GinsuSynthesis::SetSynthData` | packet sizes = sample rate × `0x788890`/`0x788894`/`0x788898` |
| `0x467230` | `GinsuSynthesis::UpdateFrequency` | target sample from frequency; countdown = latency / (`0x788894` × 1000) |
| `0x466d30` | `GinsuSynthesis::HandlePacketRelease` | advance, optional cycle jump with linear crossfade, packet submit |
| `0x477800` | `GinsuSynthesis::GinsuSynthesis(void *memblock, int size)` (thiscall, RET 8) | vtable `0x78b71c` raw; `n = (size - SNDPKTPLAY_overhead(8)) >> 3`, `maxPacketSizeC = 2n`; buffers at `memblock` and `memblock + 4n`; the player is created at `memblock + 8n` only when `2n >= 0x50`, otherwise the handle stays -1 |
| `0x477870` | `GinsuSynthesis::~GinsuSynthesis` | stops the player when `sampleRate24 != 0`, destroys it; vtable `0x789ce4` written last |
| `0x4778c0` | `GinsuSynthesis::StartSynthesis(float)` (thiscall, RET 4) | returns the `SNDPKTPLAY_start` result, also kept at `+0x20`; on success clears both buffers, records the rate at `+0x24` and submits both packets |
| `0x4777f0` | `GinsuSynthesis::PacketReleaseCallback` (cdecl) | calls `HandlePacketRelease(samples)` on the client data |
| `0x4585a0` | `Audio82B884::Construct4585A0` | initializes the audio object while retaining original unwritten bytes; called by native pause/options setup |
| `0x4690e0` | `Audio82B884::CreateCarSound` | enabled gate and manager dispatch; state-manager factories remain seams |
| `0x463ad0` | `AudioCarSoundManager::Initialize463AD0` | writes kind/limit at +0xc/+0x14 only |
| `0x463cc0` | `AudioCarSoundManager::FindCarSound463CC0` | looks up and binds a sound object; query/bind methods remain seams |
| `0x463f10` | `AudioQueuedCarSoundManager::QueueCar463F10` | queues the car at +0x20 and increments count +0x60; returns null |
| `0x440820` | `EngineSoundAllocate440820` | bWare allocation wrapper; heap backend remains native |
| `0x47d740` | `EngineSound::Setup47D740` | one/two-slot setup; native demo uses recovered per-car mode/records and a temporary bank view |
| `0x473e60` | `EngineSound::Update473E60` | one-slot controller; recovered, verified and wired |
| `0x474030` | `EngineSound::Update474030` | two-slot controller; recovered, verified and wired |
| `0x474310` | `EngineSound::Update474310` | control frequency source, one/two-slot selection and parameter-block tail; playback wired, parameter consumers open |

The frequency range in the `.gin` header is engine RPM: `GIN_Mazda_Miata_DCL.gin`
spans about 2171 to 6852.

### StartSynthesis and the constructor (evidence)

- Order of the snd calls in `StartSynthesis`: stop when started (lock, stop, unlock), then
  `FrequencyToSample`, `SampleToCycle`, `SND_attrsetdef`, `SNDplaysetdef`, lock,
  `SNDPKTPLAY_start`, and on success two submits, then unlock.
- Start format `{u16 samplerate = sampleRate18 (low word), u8 channels = 1, u8 samplerep = 8}`.
  Attributes: `SND_attrsetdef`, then `rendermode` (`+0x08`) = 4. Options: `SNDplaysetdef`, then
  `vol` (`+0x00`) = 0.
- Packets: frame count in bits 0..30 (`packetSize14`); bit 31 comes from an uninitialised
  stack word, modelled as zero. Both packets point at the two buffers, which are cleared to
  `2 * packetSize14` bytes first.
- `+0x20` holds the start result (`sndHandle`); `+0x24` holds the rate of the started synthesis and doubles as the started flag in the destructor and `StartSynthesis`.

## Snd seam

The recovered code calls the snd library through the declarations in `Ginsu.h` (retail
addresses in the comments). Stateful entry points are events in the verifier; the
native build implements them in `NativeAudio.cpp`.

| Retail | Name | Native behaviour |
| --- | --- | --- |
| `0x721f99` / `0x721fa7` | `SNDSYS_entercritical` / `leavecritical` | SDL audio device lock (recursive; see below) |
| `0x723124` | `SNDPKTPLAY_overhead(maxpackets)` | `32 * maxpackets + 0x9c` (retail code: the same formula) |
| `0x723138` | `SNDPKTPLAY_create(release, frames, clientdata, mem, size)` | a free slot index; -9 when all slots are taken, -6 when the block holds fewer than two packets, -10 without a device |
| `0x7231d4` | `SNDPKTPLAY_start(handle, format, attr, options)` | resets the queue and starts playback at the format rate (mono only); returns the slot index |
| `0x723515` | `SNDPKTPLAY_submit(handle, packet)` | queues `numframes` (bits 0..30) and `psamples[0]`; -13 when the queue is full |
| `0x72387b` | `SNDPKTPLAY_stop(handle)` | stops playback and drops the queue without release callbacks |
| `0x72366a` | `SNDPKTPLAY_destroy(handle)` | frees the slot |
| `0x72423e` | `SND_attrsetdef(attr)` | retail bytes (see below); runs as retail code in the verifier |
| `0x721a6b` | `SNDplaysetdef(options)` | retail bytes (see below); runs as retail code in the verifier |

`SND_attrsetdef` writes `detune` 0, `priority` 0, `vol` 0x7f, `pan` 0x40, `fxlevel0` 0,
`bendrange` 0, `platformver` 0, `rendermode` 0, `azimuth[6]`, `ptsdata[6]`, `puserdata[4]` and
`userdatasize[4]`. It leaves `padchar` and `tsdatasize` alone. `SNDplaysetdef` writes `vol`
0x7f, `bend` 0x40, `keynum` 0x3c, `velocity` 0x7f, `drylevel` 0x7f, `fxlevel0` 0, `azimuth` 0,
`elevation` 0, `pitchmult`/`timemult`/`tempomult` 0x1000, `lowpasscutoff` 0xffff and
`highpasscutoff` 0, and leaves the two pad words alone.

`tools/verify_native_snd_helpers.py` compares the native `SND_attrsetdef` and `SNDplaysetdef`
with the retail bytes on 256 random initial buffers (including the bytes each routine leaves
alone): 0 mismatches.

## Engine setup and controller

- `0x47d740` (engine SFX setup, recovered 2026-10-20) keeps slot records of `0x1c` bytes at
  `this+0xcc+0x1c*slot`: `+0` the `GinsuSynthData`, `+4` the `GinsuSynthesis`, `+8` the memory
  block, `+0xc` the maximum frequency, `+0x10` the minimum frequency, `+0x14` the start result.
  The mode `[[this+0x9c]+0x78]` selects one slot (mode 0), two slots (mode 1) or none.
- The gin for a slot is read from the table at `0x82b884`: slot 0 from `+0xdc[index]`, slot 1
  from `+0xe4[index]`, with `index = [[this+0x9c]+0x10]`. Recovered loader `0x465c80` fills these tables through the native I/O backend.
- The memory block is `overhead(8) + 4 + 4 * ceil((0x788898 + 0x788894) * 0x78a5b0)` bytes.
  With the 48000.0 block rate at `0x78a5b0` that is 2624 bytes: two 552-sample packet buffers
  plus the player.
- `StartSynthesis` is called with the gin's minimum frequency (`0x47d8f9`).
- The engine controller `0x473e60` (recovered and wired natively) makes snd calls for the voice (`0x72412a`
  with the voice handle at `+0xe0` and a value clamped to 0..127 at `+0x108`, and further
  calls for pitch and pan), then calls
  `UpdateFrequency(arg, 60.0f)` on `[this+0xd0]`, which is slot 0's synthesizer. `0x473fc7`
  pushes `0x42700000` (60.0). The frequency argument is raised to the first slot's minimum when below it; the
  ratio of requested to minimum frequency is retained for the snd control and
  frequency feedback. The original decompilation omitted the saved target and
  this ratio's role; the instruction-level comparison below verifies both.

## Native packet player and engine sound scaffold

`NativeAudio.cpp` implements the snd seam over SDL2 in callback mode (44100 Hz, stereo S16).
Each player mixes mono packets to both outputs with linear resampling from the packet rate
(24000 or 32000 Hz in the retail gins) and a scaffold gain of 0.5. A packet is released when
its last frame is read; the release callback runs on the audio thread with the device lock
held, and HandlePacketRelease submits the same buffer again. Up to eight players, each with a
queue of up to 32 packets. Controller volume and Q12 pitch now reach this mixer through
native snd adapters. Initial start attributes/options are still ignored. Pan/elevation,
cutoff and effect values are retained without audible DSP. Voice graphs/generations,
render modes, pan vectors, filters/effect sends and the frame-count callback remain open.

SDL's device lock is recursive. A scratch program (SDL 2.32.10, disk driver) locked the device
twice on the main thread and once inside the audio callback, and completed 27 callbacks in
300 ms without deadlock. The demo exercises the same path: every released packet nests
`SNDSYS_entercritical` inside the callback.

`NativeEngineSound.cpp` runs recovered car-state construction `0x479660` and
selection `0x4593b0` from the native participant, then engine construction
`0x473d30`, state binding `0x463500`, bank loading `0x465c80` with queued native
file I/O, and setup `0x47d740`. The selected car type and upgrade state now
choose the filename(s) and mode. `--engine-gin` explicitly overrides this with
one bank. A temporary EAXSound view and manager ownership/index assignment
remain native; the prior audio object is restored before returning. Both
setup slots and the engine/car session state persist until cleanup. Each unpaused frame
now executes recovered caller `0x474310` and its selected playback controller, updating
both mode-1 banks. Raw RPM and constant full-level/centered inputs remain native seams;
the original prepare/gain/input-production chain is not wired.
See the [selection checkpoint](#per-car-engine-record-selection-2026-10-20).
The
recovered allocator wrapper `0x440820` calls the existing native `bMalloc` backend
(`calloc`); native heap-capacity query `0x43e910` returns an unbounded capacity
(`INT_MAX`) because this heap has no bWare partition budget. The game-side
capacity checks and allocation flags remain in recovered code.

Earlier scaffold headless check (`tools/smoke_native_engine_audio.py --frames 600`): 6.92 s of
output, RMS 6125 LSB, peak 16072 with no clipped samples, 595 callbacks, 627 packets released,
0 underrun frames. Across 47 windows of 100 ms the rpm ranged from 850 to 5756. The
zero-crossing pitch averaged 21.7 Hz in the low-rpm third and 208.3 Hz in the high-rpm third;
the Spearman correlation between pitch and rpm was 0.871. An earlier run of the same command
gave 7.45 s of output and a Spearman correlation of 0.875; the duration varies with real-time
pacing. For a four-cylinder four-stroke engine the firing frequency at 850 rpm is about 28 Hz.

## Verification

`tools/verify_pc_ginsu.py` builds the sources with MSVC 7.1 and runs the original and rebuilt
routines under Unicorn at 53- and 64-bit x87 precision. Six retail `.gin` files are bound as
fixtures. The comparison covers the arena byte for byte, the return values, the logical x87
stack and control word, and the snd calls (structs and the packet record by content). The
stateful snd entry points are events with scripted return values (`SNDPKTPLAY_overhead`,
`create` and `start` take success and error codes); `SND_attrsetdef` and `SNDplaysetdef` run as
retail bytes on both sides. The routine sizes and hashes are pinned in
`config/SPEED2_EXE_1_2/ginsu.json`.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --mutation-checks --cases 384
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_native_snd_helpers.py
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

The last command needs the native build (`cmake --build build/native`) and runs `ug2demo` with
SDL's disk audio driver and no display; see [the native demo](NATIVE_DEMO.md#engine-sound).

Result (1,024 cases): 16 targets, 16,384 comparisons (1,024 per target), 0 failures. Coverage
counts: `SynthesisConstruct` with and without `SNDPKTPLAY_create` 782 and 242 (505 of the
fixtures drawn near the `2n = 0x50` threshold), `SynthesisDestruct` with and without
`SNDPKTPLAY_stop` 509 and 515, `StartSynthesis` started 137, refused 887, stopped first 233.

Mutations (384 cases): 39 mutants, 28 rejected, 11 survived. The 22 mutants that existed before
are unchanged: 12 rejected, 10 survived. The 17 new mutants cover the constructor (threshold,
packet split, player memory, handle initialisation), the destructor (the final vtable word),
`StartSynthesis` (handle check, format and attribute fields, option volume, buffer clearing,
second packet buffer, unlock, return value) and the release callback (`this`): 16 rejected,
1 survived. Every rejection reports a state or call-order mismatch.

The survivors are explanations, not proof of equivalence:

- `freq-min-strict`, `cycle-max-strict`, `sample-cand-equal`: boundary flips that return the same
  value on either branch.
- `cycle-volatile`, `handle-wp-volatile`, `handle-u-volatile`: `volatile` removals that compile to
  the same rounding.
- `handle-key-bit`, `start-key-bit`: the masked bit of the frame word is never set in the
  fixtures (frame counts stay in 0..1024), so the mask does not change the word.
- `handle-z-order`, `handle-z1-zero`: not reached by any fixture.
- `handle-multiplier`: needs `y1 == 0` exactly, which no fixture produces.

Byte identity is not established. The rebuild models the original's float stores with
`volatile float` temporaries, as before.

Limitations (see the verifier's report for the full list):

- The original keys the packet with an uninitialised frame word (bit 31) and leaves
  packet fields unwritten (the SNDPACKET `ssf` and `sizeofsamples` words). The verifier zeroes
  the stack and the rebuild writes zero.
- The stateful snd entry points are scripted in the verifier: their retail bodies do not run
  there. The native player is checked by the behaviour smoke test and the helper byte check
  above, not by Unicorn.
- Status-word condition bits and exception flags are not compared.
- Nonfinite inputs are exercised for selected math/average/control routines, not every pointer or state field.

## Seams and next steps

- Native volume and Q12 pitch adapters now reach SDL output. The retail voice
  registry, pan vectors, filters, effects and output callback consumers remain open.
- Recovered Physics, auxiliary RPM/throttle and prepare/gain/outer-control code
  now feed both native playback controllers. Shift/accel and spatial
  producers remain fixtures; spatial attenuation/pan and manager
  lifecycle/registry are still native seams. See the latest wiring checkpoint below.
- Parameter callback dispatch is recovered, while registry behavior and actual
  consumers remain open. Manager pools, forced audio disable, collision and
  frontend/HUD audio remain inventoried gaps.
- Earlier dated checkpoints below retain their historical verification results;
  current integration and gain verification are described in the final sections.


## Engine setup recovery (2026-10-20)

`EngineSound::Setup47D740` recovers `0x47d740..0x47d91a`; the allocation wrapper
`EngineSoundAllocate440820` recovers `0x440820..0x440836`. The original instruction
listings and decompilation were inspected through Ghidra MCP against `speed2.exe`.
The manifest pins both routines' byte ranges and SHA-256 fingerprints. No Ghidra
project mutations were needed.

Setup re-reads the 32-bit mode at car `+0x78` on each iteration: mode 0 sets up
one slot, mode 1 two, and other modes return. It reuses existing data, memory and
synth pointers. Allocations require available bytes of at least `0x4b0`, packet
block size plus `0x400`, and `0x444`, respectively, using heap selector `0x7ec840`
and flags `(selector & 7) | 0x1000`. The packet size retains the original float
store and ceiling sequence, giving 2624 bytes with the retail constants.

Slot 0 binds the word at audio object `+0xdc + 4*index`; slot 1 binds
`+0xe4 + 4*index`, with index from car `+0x10`. A successful bind calls
`SetSynthData` (its result is ignored), copies the frequency bounds, and calls
`StartSynthesis(minimum)` only if the slot's existing start result is negative.
The original has no general allocation-failure early return; recovery preserves
this. Its null data/synth dereferences require caller and heap invariants that
are not yet recovered. The native demo still chooses a fixed gin and mode 0;
this recovery does not establish the original bank-table fill or car sound
construction.

The extended `verify_pc_ginsu.py` executes setup together with the recovered
constructors, binder, packet sizing and synthesis start. Only heap capacity,
`bMalloc` and stateful snd operations are scripted boundaries. It compares all
arena bytes, ordered calls and defined snd arguments, stack cleanup, preserved
registers and x87 stack/control word. Fixtures cover modes 0/1/other, both bank
indexes 0/1, reuse/allocation, valid/invalid magic, start gates and scripted snd
success/failure. Invalid-magic cases cover packet/synth capacity boundaries and
allocation failures without invoking null synth pointers. Data allocation
failure and valid-bank allocation-failure dereferences remain outside this
bounded comparison; indexes 0/1 do not prove the tables' extent.

Composed calls revealed indeterminate stack fields that isolated entry tests
had read as zero. For composed setup only, event comparisons omit attribute
padding and `tsdatasize`, option padding, packet format/size words and the
packet continuation bit. Every defined field is still compared; isolated entry
tests retain their previous full struct checks. The defaults helper verifier
continues to compare all bytes, including the ones it leaves unchanged.

Reproduce with `UG2_DIR` set to the local installation (MSVC/wibo jobs sequential):

```sh
build/venv/bin/python tools/verify_pc_ginsu.py
build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineSetup,EngineAllocate --cases 384 --mutation-checks --mutation-source setup
build/venv/bin/python tools/verify_native_snd_helpers.py
cmake --build build/native
build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

The setup checkpoint's full MSVC run passed all 18 targets: 18,432 comparisons (1,024 per target),
zero failures, with 504 setup cases at 53-bit x87 precision and 520 at 64-bit.
Setup reached zero/one/two slots in 392/250/382 cases and made zero/one/two
start calls in 775/212/37 cases. The native defaults check also passed 256
randomized buffers with zero mismatches.

Setup-target mutation checks (384 cases per target) rejected all 10 mutants
with state or call-order mismatches; none survived or failed merely to compile.
The mutations cover mode slot count, all three capacity thresholds, allocation
flags, both bank selectors, the start gate/frequency, and packet allocation size.

The native smoke after wiring setup produced 8.42 s of output, RMS 5729 LSB,
724 callbacks, 763 released packets and zero underrun frames. Pitch/RPM Spearman
correlation was 0.837 (low/high thirds 25.3/204.4 Hz). This checks native playback,
not original controller or middleware equivalence. A separate `--no-audio` run
completed 30 scripted headless frames. Byte identity is not claimed.


## Engine controller recovery (2026-10-20)

`EngineSoundController.cpp` recovers the complete bodies of one-slot controller
`0x473e60..0x474028` and two-slot controller `0x474030..0x47430a`. Ghidra MCP
provided the original instruction listings and decompilation. The instruction
listings resolve missing target, ratio, conversion and feedback expressions in
the decompiled text. The two ranges are fingerprinted in `ginsu.json`.

Both controllers return when byte `+0xc8` is zero. Vtable slot `+0x38` receives
one zero argument and returns the integer gain. Requested frequency is field
`+0x104`; it is raised to slot 0's minimum frequency, saving a float ratio
`requested/minimum` below the minimum (otherwise 1). Volume is the wrapped low
32-bit product of control `+0x80` and gain, shifted arithmetically by 15 then 8,
and clamped to 0..127. Two-slot mode uses control `+0x84` for the second volume.
Global `0x82a998 >> 8` and the wrapped product of global `0x82b078` with gain,
shifted by 23, are independently clamped to 0..127.

Each slot sets six snd values, in retail order: `0x72412a` gets its stored start
handle and volume; `0x724093` gets that handle, the result of virtual `+0x3c(0)`,
and an additional zero; `0x723d9b` gets the truncating conversion of
`ratio * this[+0xbc] * 4096`; `0x72403e` gets the truncating conversion of control
`+0xbc`; `0x723c41` gets the shifted global; `0x723bc1` gets index 0 and the
second global/gain product. The six middleware implementations remain seams;
their full parameter semantics and native behavior are not established here.
In particular, the extra zero of `0x724093` survives the virtual call (which
pops one argument), explaining the caller's stack cleanup. It was absent from
the initial decompilation.

Both synths receive the same saved target with latency 60. One-slot mode unlocks
snd before the synth update. Two-slot mode retains the outer lock across the
first update, frequency feedback, the second slot's six setters and its update,
and unlocks last. The synth updates themselves nest their recovered locks.

Feedback reads slot 0's sample rate. For nonzero rate it computes
`float(rate) / CyclePeriod(float(currentCycle + 0.5))`, retaining the x87 result
without an intervening float store, then multiplies by the saved ratio and 120
and stores it to frequency `+0x104`. Zero rate substitutes global float zero
before the same two multiplies. No feedback is taken from slot 1. MSVC 7.1
cannot spell a free thiscall function pointer, so the source uses a fastcall ABI
view with an unused EDX for the virtual calls; ECX, the single stack argument,
RET 4 cleanup and ordered calls are compared with the retail thiscall ABI.

Caller lead: original bytes `0x474310..0x47434b`, read through Ghidra MCP and
independently decoded with Capstone, copy control `+0xf8` to `+0x104`, set `+0xbc`
to 1, and call one-slot mode when the car mode at `+0x78` is zero, two-slot
otherwise. At the controller checkpoint Ghidra had no function defined for this code.
The full caller is now recovered and compared below; the producer of
control `+0xf8` remains open
or covered by the controller verifier. This corrects the earlier lead that
virtual `+0x38` supplied engine RPM.

The extended Ginsu verifier runs the original/rebuilt controllers with the real
recovered `UpdateFrequency`, `FrequencyToSample` and `CyclePeriod` bodies.
Virtual queries and stateful snd setters are logged, scripted boundaries. The
retail CRT float-to-integer helper at `0x75bed4` runs on both sides. Comparisons
include all arena bytes (both slot states and the control object), ordered calls
and arguments, stack cleanup, preserved registers and logical x87 stack/control
word, under both precisions. Fixtures include active/inactive bytes, independent
slot handles/control gains, wrapped integer multiplication, below/at/above-min
frequencies, zero/nonzero signed sample rates, cycles spanning below/above the
table range, and finite,
infinite and NaN values for the two snd conversion inputs. NaN requested
frequency, virtual-query side effects and the actual setter implementations are
outside this comparison. Byte identity is not claimed.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineUpdateOne,EngineUpdateTwo --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineUpdateOne,EngineUpdateTwo --cases 384 --mutation-checks --mutation-source controller
```

Native playback still uses the direct-RPM update scaffold. The recovered
controllers are compiled into the recovery library but are not called by the
demo until their virtual queries, control producer and snd backend are available.


Controller-only check: 2,048 original/rebuilt comparisons (1,024 per controller),
zero failures. All 11 controller mutants were rejected with state/call-order
mismatches at 384 cases per controller; none survived or failed merely to
compile. They cover active gates, both clamp limits, product arithmetic, cycle
offset, zero-rate feedback, second-slot gain/synth/volume and lock ordering.
The native build passed, and the existing playback smoke passed with 658 packet
releases, zero underruns and pitch/RPM Spearman correlation 0.858. Native smoke
continues to exercise the direct-RPM scaffold, not the recovered controllers.


The final full MSVC run passed all 20 targets: 20,480 comparisons (1,024 per
target), zero failures. Each controller ran 855 active and 169 inactive cases;
one-slot below/at-or-above-minimum counts were 382/473, two-slot 426/429. Each
controller ran 506 cases at 53-bit and 518 at 64-bit x87 precision. These are
bounded comparisons with scripted virtual/snd boundaries, not complete caller
or middleware verification.


## Audio object and car dispatch recovery (2026-10-20)

`src/Speed/Indep/Src/Audio/AudioSystem.cpp` recovers the constructor
`0x4585a0..0x458671`, car-sound dispatcher `0x4690e0..0x469155`, manager kind/limit initializer `0x463ad0..0x463ae0`, sound-object lookup
`0x463cc0..0x463d03` and car queue `0x463f10..0x463f22`. Their original Ghidra decompilations
and instruction listings were inspected, and their ranges and hashes are pinned
in `ginsu.json`. Unknown constructor fields stay offset-based.

The constructor writes retail vtable word `0x789cf0`, 1 at word `+0x90` and byte
`+0x10`, the retail `SOUND` string pointer `0x789ce8` at `+0x60`, and the original
set of zero words/bytes. It conditionally clears nine manager words at
`+0xa8..+0xc8` and clears shared global `0x82ba30`. Other bytes retain their
initial contents. Native pause/options setup now calls this recovered
constructor before installing its audio object. This initializes the object;
it does not replace device or state-manager setup `0x4853c0`. Raw retail vtable
and string words are evidence-bearing data in the mapped image; native code
uses the recovered direct methods, not the original audio vtable as code.

The dispatcher first tests enable global `0x7faaa8`, returning null without
reading the car when disabled. Otherwise it reads context pointer at car
`+0x14`, kind at context `+4`, and byte at context `+0x900`. Clear byte plus kind
1 selects audio manager `+0xac`; kind 2 or any set byte selects `+0xb0`; clear
byte plus kind 3 selects `+0xb4`. Other kinds with a clear byte return null.
A null selected manager returns null; otherwise slot `+0x10` receives the car
and its returned word is passed through. The native trap for this routine was
removed. The flag remains disabled in the demo, and its manager pointers are
null: recovery of this dispatcher alone does not provide car sound objects.
The manager initializer only writes its two arguments at `+0xc` and `+0x14`.
Lookup `0x463cc0` traverses sound objects from manager `+0x10` via object `+4`.
For each candidate it re-reads the car context before querying virtual `+0x2c`.
The first equal context with object word `+0x30 == 0` is bound through virtual
`+0xc(car)` and returned. No allocation occurs in this method. The kind-3 queue
`0x463f10` stores the car at `manager +0x20 +4*count60`, increments `+0x60` and
returns null. It has no capacity guard; the caller's queue invariants remain
unrecovered. Its 16-entry view and count fit the 0x64-byte allocation observed
in setup; the verifier restricts entry indexes to 0..15.

Construction leads from original initialization `0x4853c0`: the selected
manager vtables are `0x78a3b0`, `0x78a418` and `0x78a3e0`. Their slot `+0x10`
words, read through Ghidra MCP, point to `0x463cc0`, `0x463cc0` and `0x463f10`.
Ghidra decompilation and original instruction listings of the two manager
entries confirm the lookup/queue recovery above. Controller caller `0x474310` was also decompiled and subsequently recovered
as described below, including its additional sound-parameter updates. Manager pool creation and sound-object context/bind implementations
remain separate dependencies.

The extended Ginsu verifier compares the constructor over randomized object
contents (including mixtures of zero/nonzero manager words) and compares the
shared global it clears. Dispatch fixtures cover disabled audio with an
unmapped/null car, enabled flags including negative values, kinds 1/2/3/other,
clear/set override bytes including 128/255, independently null manager pointers,
and null/arbitrary manager returns. Half the dispatch fixtures use distinct
scripted manager entries, while half dispatch through the actual recovered
lookup/queue bodies on both sides. Standalone lookup fixtures span lists of
zero through six nodes, context mismatches and word +0x30 gates; standalone
queue fixtures compare the inserted word and updated count. Sound-object
context/bind methods remain scripted thiscall boundaries. The verifier compares
selected receivers, car arguments, ordered calls, returned words, arena/globals,
callee cleanup, preserved registers and x87 stack/control state. For composed
manager tables, the verifier installs the corresponding original/rebuilt code
address, checks that each slot remains exactly that address, then normalizes
only those known code pointer words for arena comparison. The source's unused-EDX fastcall ABI view preserves
the retail caller's ECX and one stack argument under MSVC 7.1.

The sound-object virtual query/bind methods are scripted boundaries here.
The composed car constructor still uses its separate car-sound event hook; neither this test
nor native playback proves state-manager lifecycle, actual sound-object
construction, bank loading, gain/spatial queries or enabled-audio integration.
Behavioral comparison is bounded; byte identity is not claimed.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AudioConstruct,CarSoundCreate,ManagerInitialize,ManagerFind,ManagerQueue --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AudioConstruct,CarSoundCreate,ManagerInitialize,ManagerFind,ManagerQueue --cases 384 --mutation-checks --mutation-source audio
```


All 18 audio-construction/manager mutants were rejected with state or call-order
mismatches (384 cases per each of the five targets); none survived or failed
merely to compile. They cover manager selection/override/null gates, constructor
flags/manager clearing/shared global/unwritten bytes, initializer fields,
lookup context/state/bind/return, and queue index/increment. The mutation runner
now includes each original source directory, so moving the mutated audio source
to the analysis directory still resolves its headers.

The native build passed. Playback smoke after constructor integration produced
686 packet releases, zero underruns and a pitch/RPM Spearman correlation of
0.867. A 30-frame scripted `--no-audio` run also passed after the manager
recovery. These cover native startup and the existing playback scaffold; they
do not execute enabled car-audio construction. Native manager/sound-object
vtables and any required platform ABI bridges are still outside this change.


Final full-suite result: 25 targets, 25,600 original/rebuilt comparisons
(1,024 per target), zero failures. Car dispatch included 512 scripted and 512
composed fixtures: 152 called the actual lookup body and 22 called the actual
queue body. Standalone lookup bound an object in 525 cases and returned null
in 499, with zero through six context queries. The separate car-construction
regression passed 12,288 comparisons (1,024 per target), including 387
car-sound-hook cases. That regression retains its car-sound stub and does not
establish a composed car constructor through these managers.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_car_construction.py --cases 1024
```

MSVC/wibo checks ran sequentially. Byte matching and enabled-audio runtime
integration remain unestablished.


## Engine controller caller recovery (2026-10-20)

`EngineSoundUpdate.cpp` recovers the full `0x474310..0x4745a1` body (658 bytes),
pinned in `ginsu.json`. Ghidra decompilation and original disassembly were
inspected. Unknown meanings of the parameter block remain offset-based; the
source does not assign guessed volume/pan semantics to those fields.

The caller copies the raw word at control `+0xf8` to engine sound `+0x104`,
sets sound `+0xbc` to float 1, and calls `0x473e60` for car mode 0 or `0x474030`
for every nonzero mode. The raw copy preserves NaN payloads. The parameter
block at sound `+0xb0` is optional; the tail runs independently of the child
controllers' active-byte gate. Its additional integer gain comes from another
virtual `+0x38(0)` call, after the child controller has updated frequency.

Let `mul32(a,b)` retain the low 32 product bits, and `sar` denote signed right
shift. The complete tail writes:

| Parameter offset | Original input/operation | Clamp |
| --- | --- | --- |
| `+0x14`, `+0x18` | `sar(mul32(control[+0x7c], gain), 15) - 32767` | -32767..0 |
| `+0x68` | `sar(mul32(control[+0xb8], gain), 15)` | 0..32767 |
| `+0x08` | truncation of **post-controller** sound frequency `+0x104` | 0..10000 |
| `+0x44` | wrapped `-16383 - trunc(sound[+0xbc] * float[0x78af84])`; scale is -16383 | -16383..16383 |
| `+0x10` | truncation of auxiliary `+0xb8` object's virtual `+0x38()` float result times float `0x78a1c8` (binary32 10.24 approximation) | 0..1024 |
| `+0x48` | auxiliary object word `+0x134`, read after its virtual query | 0..1024 |
| `+0x40` | sound object's virtual `+0x3c(0)` | 0..65536 |
| `+0x38` | wrapped `saved global[0x82b880] - 32767` | -32767..0 |
| `+0x3c` | saved `sar(mul32(global[0x82b070], gain), 15)` | 0..32767 |
| `+0x58` | auxiliary object word `+0x138` | 0..25000 |
| `+0x5c` | 11000 when control byte `+0xc0` is set, otherwise 0 | — |
| `+0x64` | 0 | — |

The two saved global values are captured before later virtual calls. The float
return stays in ST0 until multiplication and truncation; adding a float store
changes boundary results. If parameter word `+0` is nonzero, it becomes the
receiver of middleware method `0x735580`, with the payload pointer at parameter
`+4` as its single stack argument. At this caller checkpoint it was a traced boundary. The subsequent
[parameter-dispatch recovery](#parameter-dispatch-recovery-2026-10-20) below
executes that body together with this caller and checks native synchronization
separately.

The extended verifier executes the caller together with the actual recovered
one/two-slot controller and Ginsu frequency/cycle bodies. Virtual integer/float
queries, snd setters and `0x735580` remain traced boundaries; CRT `0x75bed4`
runs as retail code on both sides. The auxiliary query returns binary64 fixture
values directly in ST0, including conversion boundaries, infinities and NaNs.
Scripted callbacks change `+0xbc` after synthesis, update auxiliary/control
fields before their subsequent reads, and overwrite both globals after the
caller has saved their values. This tests the captured values and read order,
not just a static object snapshot. Inactive synthesis cases include signalling
and quiet NaN words in the frequency source to check bit-preserving copying.

Compared: complete arena and selected globals, ordered calls and payload bytes,
stack cleanup, preserved registers, and logical x87 stack/control word under
both precisions. This remains bounded evidence; it does not establish the real
virtual-query implementations, the producer of control `+0xf8`, native setter
behavior, or enabled car-audio runtime integration. Byte identity is not claimed.
The native demo still uses the direct-RPM scaffold; this caller is compiled
into the recovery library and is not wired into playback yet.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineUpdate --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineUpdate --cases 384 --mutation-checks --mutation-source caller
```

All 13 caller mutants were rejected with state or call-order mismatches at 384
cases; none survived or failed merely to compile. Mutations cover controller
selection, factor reset, clamp endpoints, wrapped product, pre/post-controller
frequency, both captured globals, float-return rounding, auxiliary field
selection, the flag-controlled value, the zero store and the callback payload.


Final full-suite result: 26 targets, 26,624 original/rebuilt comparisons (1,024
per target), zero failures. Caller cases selected one/two slots in 269/755 runs,
used a parameter block in 778 runs and dispatched its payload in 395. There
were 848 active and 176 inactive synthesis cases, with 543 at 53-bit and 481
at 64-bit x87 precision. The native build passed; playback smoke reported
699 released packets, zero underrun frames and pitch/RPM Spearman correlation
0.868. Native smoke still exercises the direct-RPM scaffold, not this caller's
parameter/virtual/snd seams.


## Parameter dispatch recovery (2026-10-20)

`ParameterDispatch.cpp` recovers the complete 69-byte body at
`0x735580..0x7355c4` (inclusive), pinned in `ginsu.json`. Live Ghidra
instructions establish a dispatcher head at `+8`; each 16-byte node contains
next at `+0`, callback at `+8`, and client data at `+0xc`. Node `+4` remains
unidentified. The callback is cdecl with `(values, clientData)`; the dispatch
method is thiscall with one stack argument and returns zero.

The method waits indefinitely using the handle at `0x8b4f38` through
WaitForSingleObject IAT `0x783218`. It reads the list head **after** waiting,
calls every node and reads its next link **after** the callback. It then
reloads the global handle and calls ReleaseMutex IAT `0x783250`, including
for an empty list. Both OS results are ignored. These details are preserved;
no registration, removal, callback meaning, or unknown node fields are inferred.

The verifier now executes this body on both sides, also when reached from
`0x474310`. Only the two OS imports and callback implementations are scripted.
Fixtures cover zero through four nodes, a wait that changes the head,
callbacks that change links, payload contents and the mutex handle, full-word
client data and varied OS results. It compares call order, arguments and
payload bytes, arena/globals, return, stack cleanup, preserved registers and
x87 state. This is finite behavioral evidence; byte identity is not claimed.

`NativeAudio.cpp` provides a separate recursive SDL mutex for these OS calls;
it is created after successful device negotiation and destroyed after device
close. Its backend supports the observed infinite wait only. It is separate
from the packet player's device lock. `ug2audiodispatch_native_test` uses the
recovered body with actual SDL synchronization and no assets or audio device:
empty traversal, callback link mutation, nested dispatch and a blocked worker
thread are checked. These tests do not establish Windows mutex equivalence
beyond those exercised uses.

The engine caller remains outside native playback. Callback registration and
actual consumers, control-state production, per-car bank loading, virtual
queries, snd setters, sound-object construction, frontend/HUD paths and the
forced audio-disable flag remain open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ParameterDispatch,EngineUpdate --cases 64 --mutation-checks --mutation-source dispatch
cmake -S . -B build/native
cmake --build build/native
ctest --test-dir build/native -R audio_dispatch_native --output-on-failure
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_native_snd_helpers.py
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

Native build and asset-free synchronization probe passed. Both snd defaults
passed 256 byte-for-byte cases with zero mismatches. The 600-frame audio smoke
produced 687 packet releases, zero underruns and pitch/RPM rank correlation
0.885; playback still exercises the direct-RPM scaffold.

All nine final dispatch mutants were rejected with state or call-order
mismatches at 64 cases per target; none survived or failed merely to compile.
They cover a missing wait, reading the head before waiting, caching the next
link or mutex handle, wrong client data/payload, wrong timeout, missing release
and wrong return.

The final full suite passed 27,648 original/rebuilt comparisons (27 targets,
1,024 cases each), with zero failures. Standalone dispatch traversed zero,
one, two, three and four callbacks in 495/383/100/23/23 cases; the engine caller
reached actual dispatch in 385 cases. Both 53-bit and 64-bit x87 precisions
were exercised.


## Engine value queries and initialization (2026-10-20)

`EngineSoundValues.cpp` recovers `0x45e390` (27 bytes) and `0x45e3b0`
(24 bytes). Both fetch the pointer at `this + 0x58 + 4*index` and return zero
when it is null. The former reads a 32-bit word and masks it with `0x7fff`;
the latter uses `MOVZX` on the word at record `+2`, returning the full unsigned
16-bit value in EAX. The latter is deliberately not assigned Ghidra's
inferred narrow return type: the instructions establish zero extension.
The engine vtable words at `0x78af68`/`0x78af6c` point to these bodies, matching
the controller's `+0x38`/`+0x3c` calls; constructor `0x473d30` installs
vtable `0x78af30`. The table's record producer and bounds remain
unknown. Standalone fixture indices 0..16 establish no allocation bound.

`EngineSoundInitialize.cpp` recovers the normal path of setup caller
`0x47d920..0x47dade` (447 bytes):

- Call the recovered `0x47d740` slot setup. Set active byte `+0xc8` to one
  for mode 0 with slot 0 started, or mode 1 with both slots started. Other
  cases retain the previous active byte; there is no clearing store.
- Capture `global[0x850078] != 4`, obtain the parameter allocator through
  `0x734df0`, enter its mutex through `0x734e40`, and call allocator virtual
  `+0` for `0x6c` bytes with a zeroed 12-byte context. Leave through `0x734e60`.
- Write parameter `+4` from car `+0x80` object's `+8`, clamped to 0..60;
  zero `+8..+0x30`; write the captured boolean to `+0x34` (original clamp
  0..2 retained); zero `+0x38..+0x68`. Allocation does not initialize `+0`.
- Call `0x7358c0(global 0x82c538, parameters+4, parameters)`. On a signed
  negative result, invoke registry method `0x735840` with descriptor
  `0x7ede18` and retry once, ignoring the retry result.
- Store the parameter pointer at sound `+0xb0`. If parameter `+0` names a
  dispatcher, query its word `+4` through `0x735070` and store the low 16 bits
  at sound `+0x48`; otherwise store zero.

Ghidra rendered the captured boolean as an uninitialized stack variable in
one use. Original stack-address tracking connects its store at `0x47d98c`
to the reload at `0x47d9f6`; this is not an invented fallback value. The
original dereferences the allocation even after a null result; no new
failure return is introduced. Allocation failure is excluded from execution
fixtures because it faults in the original.

`ParameterLifecycle.cpp` also recovers allocator getter `0x734df0`, mutex
wrappers `0x734e40`/`0x734e60`, allocator free wrapper `0x458160`, and dispatcher
value query `0x735070`. The two mutex wrappers discard OS status and return
zero. The allocator getter returns zero or -6 according to its global pointer;
the value query writes all 32 bits before returning zero. Allocator virtual
methods and registry methods `0x7358c0`/`0x735840` remain scripted boundaries.

The original caller installs a compiler SEH frame and arms cleanup of its
allocation until ownership transfer. Its unwind entry `0x772610` calls
`0x458160`; recovered source expresses that lifetime with a scoped owner,
including an armed null pointer. The verifier checks normal-return FS chain
restoration for original and rebuilt execution; it does not compare the
compiler-generated frame layout. Exception dispatch/unwinding is **not
verified**, and no byte identity is claimed.

The extended verifier executes actual setup/Ginsu bodies inside initialization,
including partial two-slot starts, and actual `0x735070` after registration.
Allocator callbacks change car data before its later read; a wait callback
changes the mode global after capture. Registration can change the result
pointer and payload before retry or the final query. Every other controller
fixture installs the actual two value-query bodies in the synthetic vtable;
the other half retains scripted callbacks to exercise wider integer inputs
and reentrant state changes. Both modes compare ordered calls, payload bytes,
complete arena/globals, stack cleanup, preserved registers and x87 state.
The existing masks for unwritten packet/attribute bytes also apply when setup
is nested under initialization; no initialized payload fields are omitted.

These routines are built into the recovery library. Native playback still
uses direct RPM updates. Native parameter allocation/registry behavior,
record production for table `+0x58`, control-frequency production, the auxiliary
float query, bank loading, sound-object construction, setter backends and
enabled audio/frontend/HUD integration remain open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineGain,EngineValue,EngineInitialize,ParameterGetAllocator,ParameterLock,ParameterUnlock,ParameterFree,ParameterValue,EngineUpdate,EngineUpdateOne,EngineUpdateTwo --cases 64 --mutation-checks --mutation-source values --mutation-source initialize --mutation-source lifecycle
cmake --build build/native
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

All 21 new mutants were rejected with state or call-order mismatches at 64
cases per target; none survived or failed merely to compile. They exercise
query masks/indexing/null handling/zero extension, setup call and activation
gates, preserved active state, mode capture, clamp/zero ranges, registration
retry/payload, parameter ownership store, final query, allocator status,
mutex calls, full-word output and allocator-free arguments. MSVC/wibo jobs
ran sequentially.

The native build passed. The 600-frame smoke check released 687 packets with
zero underruns and pitch/RPM rank correlation 0.851. That smoke still exercises
the direct-RPM scaffold; it does not execute the new engine initialization.

Final full-suite result: 35 targets, 35,840 original/rebuilt comparisons
(1,024 per target), zero failures. Initialization exercised 408 registration
retries, 520 final dispatcher queries and 605 active/419 inactive final states.
Across the three controller/caller targets, 1,536 fixtures used the recovered
queries and 1,536 used scripted queries. Both x87 precisions were exercised.

Separate byte evidence: the MSVC 7.1 `/O2 /Ob0` builds of `0x45e390` and
`0x45e3b0` are identical to all 27 and 24 original bytes, respectively. The
verifier reports this in `query_byte_matching`, requiring a single function
in its COFF section, no relocations, equal size and equal bytes. This claim
is limited to those two leaves; initialization, allocator/mutex wrappers,
caller composition and exception behavior are not claimed byte-matched.


## Engine constructor chain (2026-10-20)

`EngineSoundConstruct.cpp` recovers the three original construction bodies
`0x45e350` → `0x473bf0` → `0x473d30`, using live Ghidra decompilation and
instruction listings. Their complete address spans are pinned in `ginsu.json`:
64, 97 and 84 bytes respectively. The last span includes a three-byte alignment
gap between its entry jump and loop; Ghidra's body-size measure omits that gap
and reports 81. It is not used as the contiguous fingerprint length.

The shared sound base sets its intermediate vtable `0x789ed4`, clears 16 words
at object `+4..+0x40`, and writes zero to the 16-bit field `+0x48` during that
loop. It then installs `0x78a0f0`, clears word `+0x50`, and clears 16 pointer
words at `+0x58..+0x94`. The other bytes retain their prior contents. The
sixteen cleared pointers identify the constructor's initialized region;
unchecked query indexing still does not establish valid caller bounds.

The intermediate engine base calls that body, then clears byte `+0xa0`, words
`+0xac`, `+0xa8`, car pointer `+0x9c`, and parameters/auxiliary/value fields
`+0xb0`, `+0xb8`, `+0xb4`; it installs `0x78ae90` and clears byte `+0x54`.
It also clears six global words at `0x82af90..0x82afa4`, sets word `0x7ed440`
to -1, and clears the raw word at `+0xbc`. Those global resets occur on every
construction. Their meanings remain unidentified.

The final body installs `0x78af30` and initializes both `0x1c`-byte slots:
null data/synth/memory pointers, zero frequency words, start result -1, and
one zero byte at slot `+0x18`. The next three bytes are untouched, so the
slot view now distinguishes that byte from the three unidentified bytes. Finally it clears control
pointer `+0xc0`, auxiliary pointer `+0xb8`, and active byte `+0xc8`. Each body
returns the original receiver in EAX. No whole-object zero-fill is substituted.

`NativeEngineSound::Load` now calls the recovered final constructor on its
temporary engine object before attaching the scaffold car view and invoking
recovered slot setup. It no longer zero-initializes that engine view or
manually supplies slot 0's -1 start result. The constructor's raw vtable values
are retained as data; this native adapter calls recovered methods directly.
It does not execute retail virtual functions. File choice, mode/index 0,
temporary bank/car binding, direct RPM updates and native cleanup remain seams.
This does not wire parameter initialization or the original controllers.

The verifier starts each constructor from dirty arena and global bytes and
compares the entire arena, selected globals, ordered nested calls, return,
stack cleanup, preserved registers and x87 state. One quarter of setup and
parameter-initialization fixtures now run the actual final constructor first,
attach only the temporary car pointer, then run their actual target. These
fixtures allocate fresh slots; the remaining fixtures retain reuse/allocation
cases. The known masks for unwritten packet/attribute stack bytes also apply
to this composition. Constructor object bytes and globals are never masked.

No byte identity is claimed for these constructors. Both query leaves retain
their separately reported MSVC byte identity; behavioral comparisons do not
establish untested interleavings or exception behavior. Bank-table and record
producers, the car state constructor/binding methods, control-frequency inputs,
registry/allocator behavior, setter backends, enabled game audio, collision and
frontend/HUD paths remain open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets SoundBaseConstruct,EngineBaseConstruct,EngineConstruct,EngineSetup,EngineInitialize --cases 64 --mutation-checks --mutation-source construction
cmake --build build/native
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

Native build and 600-frame audio smoke passed: 1,004 packet releases, zero
underrun frames, pitch/RPM rank correlation 0.767. The smoke executes the new
constructor chain and recovered setup, followed by the existing direct-RPM
scaffold. It does not establish original controller integration.


Final full-suite result: 38 targets, 38,912 original/rebuilt comparisons
(1,024 per target), zero failures. Setup/initialization included 512 cases
with actual construction followed by the temporary car binding, and 1,536
cases retaining fixture-created objects for allocation/reuse coverage. Both
query leaves retained their separate MSVC byte identity. All 13 constructor
mutants were rejected with state or call-order mismatches at 64 cases per
target, with no survivors or compile-only rejections. Mutants cover clear
counts, byte/word widths, parent calls, globals, slot counts/start defaults,
control pointer and final vtable. A separate 30-frame `--no-audio` native run
also passed.

The bank loader identified at `0x465c80` is recovered in the next checkpoint.
Its input record producer remains open.

## Engine bank loading and state binding (2026-10-20)

`EngineSoundBanks.cpp` recovers loader `0x465c80..0x465e97` (536 contiguous
bytes), state binding `0x463500` (59 bytes), and the two 12-byte completion
callbacks `0x461550`/`0x461560`. `bWare/StringConcat.cpp` recovers the 66-byte
concatenation helper `0x43dda0`; its original name is unknown. All five spans
are fingerprinted in `ginsu.json`, based on original instructions and live
Ghidra reads. No byte identity is claimed for these bodies.

The loader fills an 11-word record at `0x82b098 + *count * 0x2c`. Every field
store re-reads `*count`, including when that pointer aliases record storage.
It sets word 0 to 1, chooses word 4 and opaque word 8 using global
`0x850078 == 4` and car `+0x80` words 4/0, copies directory pointer
`0x7ed164`, clears words `+0x10..+0x24`, sets `+0x28` to -1, and increments
`*count` with 32-bit wrap. The directory is `sound\Engine\` at `0x789230`.
The opaque words are preserved without inferring the remaining record layout.

It concatenates that directory and car `+0x80` record's filename at `+0x0c`
into an unchecked 64-byte local buffer, then calls size query `0x57d900`.
Positive sizes compare signed, wrapped `size + 0x400` against heap capacity
`0x43e910`; allocation `0x440820` uses `(heap & 7) | 0x1000`. The resulting
pointer is stored through the re-read audio global and car bank index at
EAXSound `+0xdc`. Even a null allocation is passed to queued read
`0x57b4a0(destination, filename, 0, size, callback, clientdata)`. This is a
six-argument cdecl boundary, confirmed by the caller's stack cleanup.
Missing/nonpositive sizes retain the old bank pointer. After the first read
call returns, mode at car `+0x78` is re-read; mode 1 repeats the process with
car `+0x84` record's filename at `+0` and EAXSound `+0xe4`. Completion writes
exactly one byte at engine `+0xe4` or `+0x100`, respectively.

The binder retains the supplied state at engine `+0x44`, sets byte `+0xa0`,
and sets engine `+0x9c` only for state kind `+0x14` equal to 1 or 2. It has no
null-input guard. Native playback still binds its temporary car pointer
directly; recovery of the binder does not establish the state producer.

`NativeEngineSound::Load` now runs the recovered loader between construction
and setup. The temporary record still supplies the CLI filename, mode/index
0, and zero opaque words. A native guard rejects paths too long for the
original local buffer. `NativeEngineBankIO.cpp` translates Windows separators,
resolves Linux filename case, queries size, and queues copied filenames and
read arguments. Explicit draining reads the queued ranges in order and then
calls the recovered completion bodies before setup. Failed native reads
report an error without calling completion. This backend is platform
scaffolding: original file metadata caching, request identifiers, streaming
scheduler, error behavior and interleavings are not established equivalent.
The asset-free `ug2enginebankio_native_test` covers copied filename lifetime,
deferred ordered callbacks, range reads, case resolution, missing files,
null destinations and short reads.

The original/rebuilt verifier compares dirty object/record bytes, global
bank pointers, ordered boundary calls, return/stack cleanup, preserved
registers and x87 state. Cases include count aliasing record 0, both global
modes, missing/empty files, equality and signed overflow at the capacity
check, null allocation, both banks, and scripted file callbacks that change
mode, bank index and audio receiver. A selected prefix of queued completion
bodies executes after the loader returns. Fixture record indices 0..2 and
bank indices 0..1 are not inferred valid bounds; fixture paths fit 64 bytes.
Filesystem and original queue implementation equivalence are not claimed.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineBankLoad,EngineStateBind,EngineBankPath,BankReadPrimary,BankReadSecondary --cases 64 --mutation-checks --mutation-source banks --mutation-source bank_path
cmake --build build/native
build/native/ug2enginebankio_native_test
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600
```

All 17 bank/binding/path mutants were rejected with comparison mismatches,
with no survivors or compile-only rejections. They cover record mode/word
selection, counter, terminal word, capacity equality/wrap, allocation flags,
null read queueing, zero size, secondary mode/name/table/callback, callback
byte width, binding kind, path terminator and directory copying.

Actual per-car record production, state construction, enabled game audio,
controller/control-state production, parameter registration and consumers,
snd setter backends, collision and frontend/HUD audio remain open. The
original table at `0x7f0de0` has a `0x4c` record stride; the Skyline record at
`0x7f108c` contains pointers to `CAR_09_ENG_MB_EE.abk`,
`CAR_09_ENG_MB_SPU.abk`, and `GIN_Nissan_Skyline.gin`. These binary reads
identify data, not the runtime selection algorithm or valid table bounds.

The final native build and asset-free CTest check passed. The final 600-frame
audio smoke produced 949 packet releases with zero underrun frames and
pitch/RPM rank correlation 0.792. This executes recovered construction,
bank loading, completion callbacks and setup; per-frame RPM still follows
the native scaffold, so the smoke does not verify original controllers.

Final full regression: 43 targets, 44,032 original/rebuilt comparisons
(1,024 per target), zero failures. The two existing value-query leaves
retained their separately reported MSVC byte identity. Bank fixtures exercised
zero/one/two queued reads in 200/654/170 cases, zero/one/two completions in
480/499/45 cases, and count-pointer aliasing in 256 cases.

## Per-car engine record selection (2026-10-20)

`EngineSoundSelection.cpp` recovers car type mapping `0x459240` (203 bytes),
car-state base construction `0x479660` (130 bytes), and participant selection
`0x4593b0` (382 bytes). Original disassembly confirms that the latter is
virtual slot `+0x28` in the base table `0x78baa8`; manager setup `0x476a80`
passes a participant record to that slot. No direct caller xref was required
to identify this indirect call. The three bodies and upgrade selected-byte
query `0x599610` (20 bytes) are fingerprinted in `ginsu.json`. The already
recovered upgrade level query `0x5995e0` (21 bytes) moved unchanged from
`CarUpdate.cpp` to `PhysicsUpgradeQueries.cpp`, allowing both verifiers to
compile and execute the same body in composition.

Selection stores the participant at state `+0x74`, maps its setup car type at
participant `+0x10` to sound row `+0x8c`, and reads upgrades at participant
`+0x300`. Selected bytes at indices `0x18`, `0x13`, `0xf` choose state `+0x94`
in that priority order (3, 2, 1, otherwise 0). Upgrade levels at indices
4, 2, 8, 1 are queried in order, each clamped to 0..3 and stored at
`+0x98`, `+0x9c`, `+0xa0`, `+0xa4`. The level query adds 0.5 before the original
CRT truncation helper; the selection query tests one byte for nonzero.

It copies `+0x94` to `+0x90`, selects a 12-byte choice record at
`0x7f2230 + (4 * row + grade) * 12`, copies its first word to `+0x7c`, and
selects the `0x4c`-byte primary record at `0x7f0de0`. If primary word `+0x10`
is nonzero and global `0x850078` is not 4, it sets mode `+0x78` to 1 and
selects the `0x34`-byte secondary record at `0x7f1aa8` using primary word
`+0x14`. Otherwise mode is 0 and the secondary pointer is the table base.
The remaining record fields stay offset-based. The original type mapping has
explicit cases 0..30 except 28; 28 and unsigned values above 30 use row 18.
These observed mapping outputs do not establish bounds for other consumers.

Construction preserves dirty bytes outside its observed stores, installs raw
vtable word `0x78baa8`, resets manager/index/kind fields, selects the default
primary and choice table bases, and clears the two bytes at `+0xb4`/`+0xc0`
without widening them. It leaves `+0x84`, `+0x74` and other uninitialized words
untouched. Selection fills both record pointers before native loading.
Native code calls recovered methods directly; retail vtable entries are data.

The default native path now passes `vehicle.owner->context14` to this selector
and the resulting state to recovered binding/loading/setup. It retains the
selected mode and both slots. Manager allocation/ownership is still supplied
by the native session (kind from participant, bank index 0, temporary audio
view), and the participant itself is still produced by native race setup.
The CLI override copies the selected primary record, changes its filename,
and forces mode 0 as an explicit diagnostic option. It is no longer the
default bank-choice algorithm. The native file backend reports both queued
sizes so both headers can be validated before setup, and cleanup releases
both players, data objects, packet buffers and banks.

This wires selection, binding and loading, not the original per-frame sound
controller. Slot 0 still receives direct engine RPM; a mode-1 secondary slot
stays at its setup frequency with the current fixed middleware gain. Original
accel/decel blending, volume/pan, control-state production, parameter consumers,
manager pools, enabled game audio, collision and frontend/HUD audio remain
open. A two-bank playback smoke does not establish those missing mechanics.

The verifier executes the actual mapping, selected-byte and level queries
inside selection. Fixtures span all explicit car mappings plus the default,
every priority flag combination including non-binary true bytes, level clamp
boundaries, dirty state bytes, both global modes and original constant-table
records. One quarter of bank-load fixtures prepend actual car construction,
selection, engine construction and binding, with only manager kind/index
provided as a fixture. The full arena, selected globals, ordered calls,
stack cleanup, return contracts, preserved registers and x87 state are
compared. Finite levels are used here; exceptions and invalid conversion
behavior are not established by these fixtures. Original immutable table data
remains a mapped-image dependency, not recovered data-loader source.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets CarSoundSelect,CarSoundIndex,CarStateConstruct,UpgradeSelected,UpgradeLevel,EngineBankLoad --cases 384 --mutation-checks --mutation-source selection --mutation-source upgrade_queries
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_car_update.py --cases 4096
cmake --build build/native
ctest --test-dir build/native -R engine_bank_io_native --output-on-failure
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car SKYLINE
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car 350Z
```

Focused comparison passed 2,304 cases across six targets, including 96
composed bank loads. Native build and the asset-free I/O test passed. A
600-frame SKYLINE run selected `GIN_Nissan_Skyline.gin`, mode 0, with 727
packet releases, zero underruns, and pitch/RPM rank correlation 0.872. A
600-frame 350Z run selected `GIN_G35_350ZX_Comp_v2_AB.gin` and
`GIN_Infinit_G35_DCL.gin`, mode 1, with 1,656 releases, zero underruns, and
correlation 0.546. The latter includes the secondary slot at its setup
frequency and is not controller equivalence. Missing/overlong filename
checks and a 30-frame no-audio run are checked separately.

Next control-state lead (binary analysis only): registration record `0x7ed2e4`
has packed word `0x00010060`, name `SFXCTL_HybridMotor`, and factory
`0x4700f0`, which allocates `0x10c` bytes and calls constructor `0x45ce20`.
Its table `0x789fe4` points at update `0x47c3d0` in slot `+0x14`.
That update calls `0x45d0a0`, queries linked object `+0x6c` through virtual
`+0x34`, and writes control frequency `+0xf8` using primary-record floats at
`+0x1c`/`+0x20` before choosing `0x4701a0` or `0x470540` by the selected
mode. It then calls `0x45d190`. Engine link method `0x4614e0` installs its
`+0xc0` control pointer when a queried descriptor's bits 4..15 equal 6.
These bodies are decompiled leads, not recovered/verified source or native
integration. In particular the linked query's producer and control gains
must be recovered before replacing the remaining direct-RPM update.

HybridMotor dependency query `0x45d070` requests types 5, 3 and 4 in that
order. Attachment method `0x470150` installs them at `+0x6c`, `+0x70` and
`+0x74`, respectively, based on descriptor bits 4..15. Registration records
identify them as `SFXCTL_Engine` (`0x7ed2c4`, factory `0x47bfd0`),
`SFXCTL_Shifting` (`0x7ed2a4`, factory `0x46ebd0`), and `SFXCTL_AccelTrans`
(`0x7ed2d4`, factory `0x46ffa0`). These remain analysis leads. Adjacent raw
vtables must be bounded independently; following words are not assumed to be
additional virtual slots of HybridMotor.

Selection mutation checks rejected all 11 final mutants with comparison
mismatches at 64 cases per target; no survivors or compile-only rejections.
The two upgrade-query mutants were rejected at 384 cases per target. An
initial threshold change from `> 3` to `> 2` was equivalent for integer levels
and is excluded; the final clamp mutant uses `> 4`, which incorrectly retains
level 4, and produced 18 mismatches. The affected car-update verifier passed
40,960 comparisons (4,096 per each of ten targets) after the shared query move.
Separate 30-frame native checks passed for a valid one-bank override on 350Z,
a missing bank, an overlong path, and `--no-audio`.

The linked `SFXCTL_Engine` factory `0x47bfd0` allocates `0x1f0` bytes and
calls constructor `0x46f1b0`, installing table `0x789f70`. Its update is
`0x47c010`; slots `+0x34`/`+0x38` point to the seven-byte queries
`0x45c530`/`0x45c540`, which load floats at `+0x128`/`+0x130` into ST0.
Their producers and the update's nested callees remain unrecovered; these
observations do not justify substituting raw physics RPM for either query.

Final full audio regression passed 49,152 original/rebuilt comparisons across
48 targets (1,024 per target), zero failures. It includes 256 composed bank
loads after recovered car construction, selection, engine construction and
binding, plus 768 bank-load fixtures retaining direct input control for
aliasing and boundary coverage. The two pre-existing value-query leaves
retain their separate MSVC byte identity. No new byte-matching claim is made
for this selection checkpoint.

## Outer engine control update and component binding (2026-10-20)

`EngineSoundControl.cpp` recovers six original routines, each fingerprinted
in `ginsu.json`: engine dependency enumeration `0x461490` (54 bytes),
component attachment `0x4614e0` (84 bytes), linked float queries `0x45c530` and
`0x45c540` (7 bytes each), control conversion tail `0x45d190` (96 bytes), and
outer HybridMotor update `0x47c3d0` (192 contiguous bytes). The original
instruction listings and decompilation establish the read/call boundaries;
existing Ghidra labels are not treated as source evidence.

Dependency indices 0..3 return IDs 5..8; unsigned values outside that range
return -1. Attachment calls the supplied component's virtual `+4` descriptor
query, then uses bits 4..15 of its first word to install the supplied pointer
at engine `+0xb8` (5), `+0xc0` (6), `+0xc4` (7), or `+0xb4` (8). Unknown IDs
leave those fields alone. No null guard or fallback component is added.
The supplied pointer is retained across the query.

The linked queries load binary32 fields `+0x128` and `+0x130` into ST0 and
return without another store. MSVC byte matching for these leaves checks
single-function COFF sections, no relocations, exact seven-byte sizes and
identical bytes. Their field producers remain open. This matching claim
is separate from behavioral evidence for the remaining routines.

The outer update conditionally copies duration to control `+0x60` and adds
it to time `+0x5c` when byte `+0x58` is nonzero. It always calls prepare
`0x45d0a0`, then re-reads linked object `+0x6c` and calls virtual `+0x34`.
An FST copies that result to binary32 while the first comparison retains
the returned ST0 value. The lower/upper constants at `0x784550`/`0x784500`
are 1000/10000. It writes frequency `+0xf8` using
`(clamped - lower) * float[0x78a290] * (primary[+0x1c] - primary[+0x20]) + primary[+0x20]`.
The scale's binary32 bits are `0x38e90453`. NaN query comparisons take the
lower-bound branch. Record fields remain offset-named; no tuning semantics
beyond those original reads are inferred.

Mode at linked car state `+0x78` is then read: zero calls `0x4701a0`, any
nonzero word calls `0x470540`, both with the original duration word. Finally
`0x45d190` re-reads the linked object, multiplies integer control `+0xb8` by
its float `+0xfc`, and integers `+0x7c`/`+0x80`/`+0x84` by float `+0x118`,
with one original CRT truncation after each product. The integer loads are
exact FILD/FIMUL operands, without a preceding binary32 rounding. The source
retains the loads of the next integer before storing the preceding result.
The linked pointer is saved once, while its scaling field is re-read for
each product, matching the original.

The verifier compares dirty arenas, ordered calls, selected globals, stack
cleanup, preserved registers, and x87 stack/control word under 53- and 64-bit
round-to-nearest precision. Standalone cases cover dependency defaults,
descriptor fields and query-time writes, dirty neighboring bytes, float
query return contracts, full signed integer scaling, non-finite factors,
conditional time updates, both modes including non-1 nonzero values, clamp
boundaries and nextafter inputs. Virtual `+0x34` alternates actual recovered
query bodies with scripted binary64 returns. Prepare and mode callbacks can
change the linked car/object pointers and result fields to check later reads.
Status/exception flags and concurrent interleavings are not established.

One quarter of existing `EngineUpdate` fixtures now prepend actual attachment
of the auxiliary and control components, then the actual outer control update,
then execute recovered `0x474310`, its selected controller, and parameter
dispatch. Both auxiliary queries use recovered bodies in that composition.
The original and rebuilt conversion tails execute independently; the original
CRT helper remains shared. Prepare `0x45d0a0` and gain producers
`0x4701a0`/`0x470540` remain scripted boundaries, so the comparison does not
establish complete gain production, component construction, or native use.

This checkpoint adds verified recovered source to the native archive, but
**does not wire this new control layer into demo playback**. Its unrecovered
prepare/mode dependencies have no native replacement implementation. The demo
continues to use recovered selection/binding/loading/setup and its documented
slot-0 direct-RPM scaffold. Control-state production, actual mode gains,
parameter records/consumers, snd setters, manager pools, the forced disable
flag, collision and frontend/HUD audio remain open. Replacing those inputs
with guessed values would not establish original controller integration.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 1024
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets EngineDependency,EngineAttach,AuxFrequency,AuxGain,ControlScale,ControlUpdate,EngineUpdate --cases 64 --mutation-checks --mutation-source control
cmake --build build/native
ctest --test-dir build/native -R 'engine_bank_io_native|audio_dispatch_native' --output-on-failure
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car SKYLINE
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car 350Z
```

Focused comparison passed 896 cases (128 per seven targets), including 32
composed engine updates after component attachment and control production.
Native build and both asset-free audio tests passed. The SKYLINE 600-frame
regression retained zero underruns, 841 packet releases and pitch/RPM rank
correlation 0.774. These playback checks exercise the existing native path,
not the newly recovered control layer.

The 350Z 600-frame regression produced 1,274 packet releases and zero
underruns, but **failed** the pitch/RPM smoke criterion: correlation 0.435
against the existing 0.50 threshold. This remains an open native integration
failure. The demo still feeds only slot 0 while its selected secondary bank
runs at setup frequency with fixed backend gain; the incomplete blend limits
interpretation of a mixed-output zero-crossing pitch measure. The new control
routines are not linked into `ug2demo` (confirmed by native symbol inspection),
so this run does not exercise or validate them. No threshold was weakened and
the failure is not counted as a playback pass.

Setter analysis leads for the remaining backend (no implementation claim):
`0x72412a` scales its integer argument by float `0x78a94c` and updates linked
voice word `+0x38`; `0x724093` uses signed 16-bit pan words and calls
`0x724fc5`, whose channel-vector consumer is `0x725878`; `0x723d9b` stores a
16-bit value at voice `+0x8c` and recomputes `+0x8e` through `0x7277da` with a
12-bit product shift. `0x72403e` delegates to `0x72505e`, which calls an optional
function pointer at output record `+0x78` with its client word `+0x60` and the
supplied value. These observations do not establish a filter algorithm.
`0x723c41` updates one byte at voice `+0x5f`, while `0x723bc1` writes a scaled
per-output float through voice `+0x64`. Original handle validation `0x725480`
checks low-byte slot, active byte `+0x6d`, and full generation word, returning
-8 for invalid handles; every setter first returns -10 when the device flag
is clear. Native packet-player slot handles do not yet model that full voice
registry. Their consumers must be investigated before claiming setter or
mixer equivalence.

All 14 control-layer mutants were rejected with comparison mismatches at
64 cases per each of seven targets, with no survivors or compile-only
rejections. They cover dependency bounds, descriptor bits/destination,
query offsets, scaling fields/final store, time gate, upper clamp, frequency
scale/endpoints, nonzero mode choice and the conversion-tail call. The final
full-suite fixture additionally composes both recovered packed gain/value
queries with the new attachment/control prelude; their record producer is
still supplied by fixtures.

Final full regression passed 55,296 original/rebuilt comparisons across 54
targets (1,024 per target), zero failures. It includes 256 engine updates
composed after actual dependency attachment, outer control production,
conversion tail, both auxiliary float queries and packed gain/value queries;
prepare and mode gain producers remain scripted. The four queried leaves
retain separately reported MSVC byte identity. The 350Z native pitch smoke
failure remains open and is excluded from this behavioral-verification pass.

## Engine prepare and gain production (2026-10-20)

This checkpoint supersedes the prepare/gain boundaries in the preceding
outer-control checkpoint. `EngineSoundControlGains.cpp` recovers prepare
`0x45d0a0` (230 bytes), one-bank gain production `0x4701a0` (893 contiguous
bytes), and two-bank gain production `0x470540` (996 contiguous bytes).
The latter spans end before their jump tables. `SampleAverageUpdate.cpp`
recovers ring advance `0x576970` (84 bytes), fill `0x576a10` (70 bytes),
and recalculation `0x576910` (92 bytes). The composed verifier also executes
existing recovered curve lookup `0x576790` (190 bytes) and pending-shift query
`0x5ecd70` (26 bytes), rather than scripting their outputs. All eight targets
have fingerprints in `config/SPEED2_EXE_1_2/ginsu.json`.

Prepare computes car `+0x48` minus the saved value, and auxiliary `+0x128`
minus `+0x12c`, then selects ring advance or fill using linked component
`+0xd8`, accel `+0xc8`, and shift `+0x78`/`+0x7c`. Its final virtual call
recalculates the average; it is not a float getter. Recalculation multiplies
the old sum by zero before accumulating, preserving propagation of an initial
nonfinite sum. Capacity zero, inline/external storage, byte ring/count fields,
and the two sum stores in advance are retained.

One-bank production selects fixed or interpolated primary-record targets,
limits changes by 0.2 and cutoff changes by 6000, and derives cutoff from a
virtual auxiliary query with a 24000 upper bound and the linked integer
`+0x138`. Its temporary idle flag is visible to that query and subsequently
cleared. The record pointer is retained across the query while later record
words and the current linked-object pointer are re-read. Integer output
conversion uses normalization float `0x78aab4` (bits `0x3803126f`).

Two-bank production uses absolute average delta, separate primary/secondary
thresholds and tuning, and a curve sampled at control frequency `+0xf8`.
Five ordered virtual gain queries contribute to the blend and three output
gains. The first two queries retain the engine pointer saved before curve
lookup; subsequent queries re-read the current pointer. Record pointers and
individual words have different read boundaries across these calls. Its
normalization float `0x78a6c8` (bits `0x38000100`) differs from the one-bank
constant and is rounded to binary32 before the third query. Original wrapped
integer subtraction, truncation order and asymmetric intermediate float
stores in smoothing are preserved. Duration is passed/popped but unused by
both gain routines.

The fixtures alternate recovered auxiliary query bodies with scripted
binary64 query returns that mutate engine/car pointers and tuning words.
They observe the transient idle flag at query entry. Actual curve, average
and pending-shift routines run in composition. Deterministic scenarios cover
fixed/interpolated tuning, direct/smoothed output, accel/shift decisions and
car `+0x54` threshold equality; standalone average cases include capacity zero
and nonfinite values. In addition to independent fixtures, 64 successive
control updates per x87 precision carry the compared arena forward while
mode, RPM, throttle query value, shift/accel inputs and pending-shift state
change. These are finite behavioral comparisons, not complete coverage or
new byte-matching claims. Component producers/construction remain open, and
integer gain fixtures are bounded to 0..32767.

## Native playback controller wiring (2026-10-20)

`NativeEngineSound` now owns persistent car/override/control/engine state.
Each unpaused frame places the existing raw physics RPM input in control
`+0xf8` and calls actual recovered `EngineSound::Update474310`. That caller
selects actual `Update473E60` or `Update474030`; both mode-1 synthesizers now
receive per-frame frequency updates. The recovered packed queries
`0x45e390`/`0x45e3b0` execute through native ABI bridges. Symbol inspection
confirms all three playback routines and both packed queries in `ug2demo`.
A `# playback` log row records frame, mode, frequency feedback, both volumes,
and both synthesizer target positions, alongside the existing RPM log.

This wiring deliberately retains the previously documented native raw-RPM,
constant full-level and centered-input seams. It does not call the newly
recovered prepare/gain/outer-control routines with guessed component state.
Original component initialization, auxiliary/shift/accel/physics producers,
parameter registry/consumers and activation remain required for that next
integration. The parameter pointer remains null; the caller returns after
its playback controller. Manager/session ownership, bank view and cleanup
remain native. This is not original game-audio equivalence.

The native snd adapters accept the recovered controller's volume and unsigned
Q12 pitch multiplier and apply them to SDL packet mixing. They preserve the
existing 0.5 mix headroom and centered mono output. Pan/elevation, cutoff and
effect values are retained without implementing a guessed pan vector,
filter or effect algorithm. Slot validation still replaces the retail voice
generation registry. These are explicit middleware backend seams, not
recovered snd implementations; voice graphs, DSP and optional output callback
consumers remain unverified. Setters are called inside the recovered
controllers' device critical sections.

Native build, audio-dispatch/bank-I/O tests and the new asset-free
`audio_playback_native` test passed. The latter feeds a constant packet through
actual SDL disk output and verifies full, zero and partial volume levels in
the captured PCM. Native snd defaults still passed 256 byte-for-byte fixture
pairs. A 600-frame SKYLINE smoke had 594 packet releases, zero underruns,
RMS 6108.9 and pitch/RPM correlation 0.878. A 600-frame 350Z smoke had 970
releases, zero underruns, RMS 11684.5 and correlation 0.537, passing the unchanged
0.50 criterion. Its log contains 130 distinct primary and 111 secondary
synthesizer target positions across 600 controller frames. These checks
validate native playback integration only. The earlier 350Z failure remains
recorded above; this single passing mixed-output run does not establish
original accel/decel blending or robust pitch behavior across all cars.

```sh
cmake --build build/native
ctest --test-dir build/native -R 'audio_playback_native|audio_dispatch_native|engine_bank_io_native' --output-on-failure
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car SKYLINE
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/smoke_native_engine_audio.py --frames 600 --car 350Z
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_native_snd_helpers.py
```

Next input-producer lead (binary analysis, not recovered source): Physics
component table `0x78a008` has update `0x484c50` in slot `+0x14`, constructor
`0x45d210`, bind `0x45d4f0` and activation `0x45d370`. Its normal update reads
owner gear `+0x434`, input throttle `+0x20c` and engine value `+0x440`, publishing
car-state `+0x44`/`+0x48`/`+0x4c`/`+0x50`/`+0x54`. The `+0x48` value is
normalized using component endpoints `+0x78`/`+0x7c`, multiplication by float
`0x78c194` (9000), and addition of float `0x784550` (1000); it is not raw RPM.
Car-state `+0x54` receives the percentage throttle input. The component also
has mode/override and blown-engine notification branches, which cannot be
silently omitted when recovering it. Auxiliary query producer `0x46f7d0`
chooses among this state and shift/accel state, maintains an average and
publishes `+0x128`/`+0x12c`; `0x45c710` produces `+0x130` through a separate
average and offset. These findings locate the next integration work, and do
not justify a raw-RPM/throttle stand-in for these original producers.

Final source regression passed 16,000 original/rebuilt comparisons across
62 targets (256 independent cases per target plus 128 carried control steps),
zero failures. Query-time mutations additionally change primary/secondary
integer gain words and auxiliary normalization words to check reads across
virtual calls. All four previously matched packed/auxiliary query leaves
retain their separately reported MSVC byte identity. The shared timed-average
regression passed 6,030 comparisons after the header extension. All 21 gain
mutants and four average-update mutants were rejected by state/call mismatches
at 64 cases per selected target, with no survivors or compile-only rejections.
The sequence failure-reporting path now wraps contextual errors correctly,
and mutation survivors make the verifier exit unsuccessfully. Native override,
missing-bank, overlong-filename and no-audio checks also passed at 30 frames.
The full inventoried audio goal remains open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ControlPrepare,ControlOne,ControlTwo,ControlUpdate,EngineUpdate --cases 64 --mutation-checks --mutation-source gains
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets SampleAdvance,SampleFill,SampleRecalculate --cases 64 --mutation-checks --mutation-source sample_update
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_timed_average.py
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
```

## Physics audio input component (2026-10-20)

`EngineSoundPhysics.cpp` recovers component construction `0x45d210` (231 bytes),
binding `0x45d4f0` (55), activation `0x45d370` (90), normal update `0x484c50`
(715), complete scripted override `0x46fa30` (1329), completion callback
`0x45d360` (16), three mode-query leaves `0x430560`/`0x430420`/`0x430350`
(11 each), and the override's ramp constructor/evaluator `0x46b240` (82) /
`0x4599e0` (107). This supersedes the Physics-component analysis lead above.
These are UG2-only layouts and observations, without a cross-title claim.

Construction preserves unwritten bytes, initializes the embedded two-sample
average, and installs original table word `0x78a008`. Binding stores the supplied
car state, recognizes kinds 1..3, and publishes the component at car `+0x40`.
The invalid-kind path still contains the original null write; recovery does
not add a fallback. Activation queries the owner `+0x40` subobject and physics
record, retaining call/read order. `EngineRecordMaximum.cpp` recovers query
`0x591090` (148 bytes) and wrapper `0x5928d0` (13), selecting the maximum of
zero and nine engine-spec samples at `+0x10`; this differs from the existing
`0x457b00` query over samples at `+0x34`.

The normal update retains the three short-circuited mode queries and override
gates. It copies gear state, takes throttle input times 100, advances and
recalculates the throttle-delta average, tracks throttle edge times, limits
the filtered throttle's per-update change, and normalizes engine speed using
the activated endpoints. It publishes car `+0x44`/`+0x48`/`+0x4c`/`+0x50`/
`+0x54` in original order. The extended throttle product survives the first
binary32 store before subtraction; the filtered throttle survives its store
before later publication. The global engine-speed override is re-used at
`0x7fbed8`, with sentinel 100 at `0x784258`.

The blown-engine branch retains exact byte/kind/mode gates, query order,
receiver offsets, callback identity and post-call re-reads, including the
null audio-owner check after hashing. It executes the existing recovered
`cshash` (`0x43db50`), rather than scripting the hash. Engine-blown detection,
notification queries/play/clear, and `PhysicsParameters::GetRecord` are logged
subsystem boundaries in this verifier; GetRecord has separate recovered
source and coverage. These boundaries are not native replacement bodies.

The override retains both countdown and sequence states, all fourteen explicit
sequence selections plus the default, four-update segment advancement,
low-byte throttle extraction from packed words, and the RPM/throttle clamps.
Its duration argument is passed/popped but unused. The original frame-based
advancement is retained. Ramp evaluation preserves the extended quotient for
its first comparison and rounds a copy for the upper clamp/interpolation.
Original sequence tables remain addressed retail data; they are not copied
into this repository.

The registered factory `0x470a00` requests `0xf4` bytes only when heap headroom
is strictly greater than `0x4f3`, uses the recovered allocator wrapper, and
calls the component constructor only after allocation succeeds. Its body is
in `EngineSoundPhysicsFactory.cpp`. This also confirms the component size.

The fixture arena and ordered call logs compare at both x87 precisions.
One quarter of normal-update fixtures prepend actual construction, binding
and activation. The constructed average's raw vtable slot is explicitly
relocated to each side's recovered recalculation body; no retail routine is
executed as the rebuilt implementation. Independent cases retain dirty
unwritten bytes and control over direct state. Carried trajectories cover
normal driving, complete sequence playback, countdown overrides and returns
to normal input. Allocation failure/exception unwinding, invalid-kind writes,
concurrent interleavings and full notification/manager composition remain
outside these finite checks. Standalone sample-constructor fixtures use nonnegative
capacities; failure of its separate array allocation is not exercised.

This component is compiled into the native recovery archive but is not yet
on the demo's control-input path. Auxiliary RPM/throttle producers, shift/
accel components, HybridMotor lifecycle/binding, manager construction and
notification consumers are still needed. The demo's raw-RPM/full-level seam
remains documented; source recovery does not establish its replacement.

Final regression passed 20,480 original/rebuilt comparisons across 78 targets:
256 independent cases per target, 384 carried Physics updates and 128 existing
HybridMotor control steps. All states/calls compared without failures. Coverage
includes all three mode-suppression values (2, 3, 4), insufficient heap,
allocation failure, actual factory-to-constructor composition, callback-time
owner/flag changes and the subsequent null-owner gate. All 15 component, three
factory and two record-query mutants were rejected by state/call mismatches,
with no survivors or compile-only rejections. The four previously matched
packed/auxiliary query leaves retain their separate MSVC byte identity; no new
byte-matching claim is made for this component.

Native build and the three asset-free audio tests passed. Separate 30-frame
SKYLINE and 350Z checks produced 245 / 492 packet releases and zero underruns.
These runs check the existing playback integration after the layout additions;
they do not execute the new Physics component or establish its native wiring.
The audio goal and remaining inventory entries stay open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets PhysicsConstruct,PhysicsBind,PhysicsActivate,PhysicsUpdate,PhysicsOverride,PhysicsNotify,PhysicsMode40,PhysicsMode28,PhysicsMode10,PhysicsRampConstruct,PhysicsRampEvaluate,PhysicsSpecMaximum,PhysicsEngineMaximum,PhysicsHash,SampleConstruct --cases 64 --mutation-checks --mutation-source physics
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets PhysicsFactory --cases 64 --mutation-checks --mutation-source physics_factory
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets PhysicsSpecMaximum,PhysicsEngineMaximum,PhysicsActivate --cases 64 --mutation-checks --mutation-source physics_max
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
cmake --build build/native
ctest --test-dir build/native -R 'audio_playback_native|audio_dispatch_native|engine_bank_io_native' --output-on-failure
```

The corrected factory coverage counter records 84 allocation attempts: 42
constructed components and 42 null allocation results, plus 172 insufficient-heap
cases. The final comparison run retains zero failures after that reporting fix.

## Auxiliary input production and native gain wiring (2026-10-20)

`EngineSoundAuxiliary.cpp` recovers `SFXCTL_Engine` construction `0x46f1b0`
(518 bytes), factory `0x47bfd0` (64), binding `0x45c550` (68), activation
`0x45c5a0` (194), dependency attachment `0x46f3e0` (89 contiguous bytes before
its jump table), descriptor/dependency queries `0x45c4b0` (6) / `0x45c670`
(51), car query `0x479700` (7), RPM production `0x46f7d0` (504), and throttle
production `0x45c710` (130). The component occupies `0x1f0` bytes. Constructor
and activation writes are distinct: the constructor does not initialize all
fields later consumed by a complete frame update. Dirty padding and unwritten
fields are retained in the recovered code.

RPM production selects shift, accel-transition or car input in that order.
The optional clamp saves the car input and lower margin before virtual `+0x34`,
then uses its extended return and a rounded copy differently. The upper margin
is read after the query; the old `+0x128` word is copied to `+0x12c` afterward,
including signaling-NaN payload bits. Publication adds `+0x144` twice, with the
sequence contribution between those additions. A separate averaging path uses
shift/accel inputs or a carried oscillator, recalculates the two-sample ring,
then re-reads the car and Physics links to map the result into car `+0x58`.
Throttle production fills the three-sample ring for shift/accel overrides and
advances it for normal car input, recalculates, then adds the current `+0x148`.

`EngineSoundSequence.cpp` recovers approach `0x468e30` (51 bytes), sequence
construction `0x46af20` (78), and append `0x4598f0` (123). Approach retains the
extended return for the caller's comparison. Append uses exact integer duration
conversion, a minimum duration only at the original comparison, and the exact
`count == 5` capacity gate. Sequence stepping and the complete auxiliary frame
`0x47c010` remain open.

The input verifier compares independent dirty-state fixtures, callback-time
pointer/global changes, allocation/headroom cases, both gain modes, and actual
Physics → auxiliary → HybridMotor composition. Carried trajectories add 256
updates across both x87 precisions, including complete NIS override playback
and a return to normal input. Shift/accel inputs, idle/oscillator gates and
spatial callbacks remain controlled fixtures; these checks do not establish the
complete original component graph or exception unwinding.

`EngineSoundControlLifecycle.cpp` also recovers HybridMotor constructor
`0x45ce20` (275 bytes) and bind `0x45cf90` (190). The constructor creates the
four-sample average and five-point curve, clears only the observed fields,
and preserves the dependency links and other unwritten words. Binding calls
car virtual `+0x44` before re-reading the car, computes its wrapping integer
curve span, and retains the exact FILD value through both interior-point
calculations. Pair-curve constructor `0x576770` (18 bytes) is moved unchanged
from the static-initializer file to `PairCurveConstruct.cpp` for independent
verification and reuse. Full HybridMotor activation/manager lifecycle remains
open.

`NativeEngineSound` now owns persistent Physics and auxiliary components. Load
runs their recovered construction/binding/activation, HybridMotor construction/
binding, and installs native ABI
bridges to the recovered car, float-query and sample-recalculation methods.
The recovered vehicle export `0x5a5540` already publishes filtered engine
state at owner `+0x440` (the embedded state `+0x400`). Each unpaused audio frame
consumes that field directly and runs Physics `0x484c50`, auxiliary RPM/throttle, HybridMotor `0x47c3d0`, and
engine playback `0x474310`. Frequency mapping, throttle filtering, both mode
gain producers and both bank controllers execute recovered code. `# control`
records in `UG2_ENGINE_LOG` expose the intermediate normalized RPM, filtered
throttle, mapped frequency, gains and cutoff. The smoke check requires changing
gains and throttle and rejects the previous direct-RPM feed.

Remaining native seams are explicit: the session still schedules and links the
components, supplies neutral shift/accel fixtures, keeps idle/oscillator/effect
offsets at neutral values, supplies base auxiliary level 32767, and retains
constructor ramp state. Spatial callbacks remain centered, notifications are
bypassed with Physics `+0x6c = 1`, and unexpected notification entry traps.
Parameter registry/consumers, manager pools, original activation/cleanup,
backfire/cutoff/random and ramp producers still need recovery/integration.
`parametersB0` remains null. This is partial native integration of recovered
input/gain code, not recovery of the complete auxiliary frame or audio manager.

Native validation after consuming the recovered vehicle export directly:
SKYLINE ran 600 frames with RMS 1898 LSB, 828 packet releases, zero underruns,
152 distinct primary gain values and pitch correlation 0.848 (pass). The 350Z
ran 600 frames with RMS 7886 LSB, 1312 releases, zero underruns, 158 primary
and 11 secondary gain values. Its pitch correlation was 0.418, below the
unchanged 0.50 smoke threshold (failure). These are native observations, not
original/rebuilt acoustic comparisons. The one/two-bank gain and frequency
checks passed, but the two-bank pitch heuristic remains an unresolved native
validation failure. All three asset-free audio/backend tests passed, as did
30-frame no-audio and missing-bank cleanup checks.

Focused verification first passed 3584 comparisons for the 13 auxiliary/helper
targets, including 256 carried input/gain steps. The initial mutation batch
rejected 22 auxiliary errors. Two nominal float-cast mutations survived; the
final variants explicitly force binary32 stores, and the frequency fixture
isolates rounding from subsequent offsets. A second run passed a 576-comparison
baseline for constructor/binding, curve construction, RPM and sequence append,
then rejected all six selected mutations by state/call mismatches: four new
lifecycle errors and both corrected rounding errors. No compile-only rejection
or surviving final mutation remains in those batches. These are finite,
controlled comparisons, separate from native sound fidelity.

The strict MSVC leaf check confirms complete byte identity for packed engine
queries `0x45e390` / `0x45e3b0`, auxiliary float queries `0x45c530` / `0x45c540`,
car query `0x479700`, approach `0x468e30`, and curve construction `0x576770`.
No byte-identity claim is made for the larger recovered routines.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ControlConstruct,ControlBind,GainCurveConstruct,AuxRpm,AuxSequenceAppend --cases 64 --mutation-checks --mutation-source control_lifecycle --mutation-source auxiliary --mutation-source aux_sequence --mutation-name control-constructor-average --mutation-name control-constructor-unwritten --mutation-name control-bind-distance --mutation-name control-bind-query --mutation-name aux-rpm-query-rounding --mutation-name aux-sequence-integer
ctest --test-dir build/native -R 'audio_playback_native|audio_dispatch_native|engine_bank_io_native' --output-on-failure
build/venv/bin/python tools/smoke_native_engine_audio.py --game /path/to/UG2 --car SKYLINE --frames 600
build/venv/bin/python tools/smoke_native_engine_audio.py --game /path/to/UG2 --car 350Z --frames 600
```

Final regression passed 24832 original/rebuilt comparisons across 94 targets:
256 independent cases each, 128 carried control updates, 384 carried Physics
updates and 256 carried auxiliary/input/gain steps. Both gain modes occurred
in 32 composed RPM fixtures each. All seven leaf byte matches remain intact.
The audio goal remains open; the native integration and remaining seams above
are not evidence of complete audio recovery.

## Idle, level, envelopes and high-RPM producers (2026-10-20)

`EngineSoundAuxiliaryIdle.cpp` recovers `0x46f9d0` (81 bytes). It checks the
owner context kind, context `+0x900` flag and accel-transition `+0xc8` before
querying virtual `+0x34`. The binary's `TEST AH,0x41` accepts unordered as well
as less/equal, so a NaN query sets idle `+0x13c`; a C++ `<=` replacement would
change that behavior. The threshold is the addressed float at `0x7885d0`
(2500). The extended virtual return is not rounded before comparison.

`EngineSoundEnvelope.cpp` recovers configuration `0x459990` (70 bytes) and
step `0x46b100` (281 contiguous bytes before its jump table). This `0x1c`-byte
time envelope is distinct from the same-sized Physics interpolation helper.
Configuration converts the integer duration with FILD, stores a float copy,
then compares the unrounded product before applying the minimum duration.
Step rounds elapsed time before testing strict `>` completion. Kind 1 holds
the previous value; kinds 2/3 retain the original square/interpolation order;
kind 6 converts/clamps its position and calls curve `0x468160`. Completion
copies the endpoint word rather than performing float arithmetic on it.

`EngineSoundSequenceUpdate.cpp` recovers reset `0x4598c0` (48 bytes) and step
`0x46af70` (362 contiguous bytes). Reset does not append a segment. Step
consumes at most one completed segment per call, retains excess time, and
conditionally copies the previous endpoint into the next segment's start.
It retains the same hold, square and curve branches, including re-reading the
current index after the curve call. Invalid indices remain unchecked.

`EngineSoundCurve.cpp` recovers curve `0x468160` (494 contiguous bytes) and
conversion `0x468050` (232). The original tables remain addressed player data.
The source preserves reflected tail branches, actual recursive call boundaries,
low 32-bit products before arithmetic shifts, low-byte mode, the `0x1e` mask,
and the distinct floors: curve results below -9600 become -10000; converter
results are clamped at -10000. Conversion writes scratch `0x82bc88 = -1` even
on early returns. Checked inputs lie in 0..32767; this is a fixture domain,
not proof of safe arbitrary table indices.

`EngineSoundAuxiliaryEffects.cpp` recovers level `0x45c6c0` (79 bytes) and
high-RPM production `0x46f610` (435). The apparent random calls in the initial
decompilation are the CRT float-to-integer conversion helper `0x75bed4`.
Level starts at 32767, applies the shift-linked `+0xf8` contribution when
shift `+0x78` is nonzero, then adds truncated auxiliary `+0x140`, preserving
integer wrapping. High-RPM production bypasses all work for state kind 2.
Other kinds use the original 9800 threshold and shift gate, query through the
actual virtual boundary, evaluate shared gear-dependent curve `0x7f0050`,
configure both envelopes and step them in order. Entering also initializes
the carried RPM oscillator. The exit path rechecks the active flag after the
virtual query. Threshold equality retains the original state.

The focused helper run passed 896 comparisons. The expanded run passed 3328,
including 256 carried envelope steps, 256 carried sequence steps and 256
existing composed input/gain steps. Fixtures compare both x87 precisions,
context gates, binary64 query boundaries, nonfinite idle/level inputs, curve
kinds/modes, dirty storage, all envelope kinds and single-step sequence
transitions. Spatial callbacks and shift/accel states remain controlled inputs;
these are finite behavioral comparisons, not complete caller recovery or new
byte-match claims.

Native frames now call recovered idle, sequence step, RPM/throttle, level and
high-RPM producers before HybridMotor/playback. The fixed base level and
forced idle/oscillator/ramp inputs are removed. `# auxiliary` log records show
idle/high-RPM flags, level, gain ramps, oscillator and sequence state. The full
auxiliary frame, offset/position producer, backfire and cutoff still remain
open, as do shift/accel components, spatial attenuation/pan, notifications,
manager lifecycle, parameter registry and consumers. The unrelated envelope
at auxiliary `+0x1d0` is not stepped until its original caller is recovered.

Native build and all three audio/backend tests passed. A 600-frame SKYLINE
run passed with RMS 2023 LSB, 689 releases, zero underruns, 278 idle-active
frames and pitch correlation 0.876. The 350Z run produced RMS 7503 LSB,
1224 releases, zero underruns and 482 idle-active frames, but correlation
0.486 still fails the unchanged 0.50 threshold. Neither native run entered
the high-RPM gate; standalone/composed binary comparisons provide that
transition evidence. These observations do not establish acoustic equivalence.

The 64-case focused mutation baseline passed 1088 comparisons, including
512 carried envelope/sequence steps. All ten deliberate errors were rejected
by state/call mismatches, with no survivors or compile-only rejections:
unordered idle comparison, context gate, shift-level subtraction, kind-2
bypass, threshold equality, completion equality, minimum duration, discarded
sequence excess time, table interpolation mask and omitted scratch write.
The final composed trajectory carries the actual high-RPM flag rather than
forcing it, and drives a high-RPM window after the NIS override returns to
normal input. Shift/accel states remain explicitly supplied fixtures.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AuxIdle,AuxLevel,AuxEffects,EnvelopeConfigure,EnvelopeStep,SequenceReset,SequenceStep,SoundCurve,SoundCurveConvert --cases 64 --mutation-checks --mutation-source aux_idle --mutation-source aux_effects --mutation-source envelope --mutation-source sequence_update --mutation-source sound_curve
```

Final regression passed 27648 comparisons across 103 targets: 256 independent
cases each, 128 carried HybridMotor updates, 384 Physics updates, 256 composed
auxiliary/input/gain steps and 512 envelope/sequence steps. The composed path
entered and exited the high-RPM state once per x87 precision, carrying 16
active frames in total. Standalone fixtures also covered 24 entries, 49 exits,
and 88 kind-2 bypasses. Idle fixtures covered both query and short-circuit
paths. All seven existing leaf byte matches remain intact; no new byte-match
claim is made here. The full audio objective remains open.

## Complete auxiliary frame and remaining spatial seam (2026-10-20)

`EngineSoundAuxiliaryProducers.cpp` recovers offsets `0x45c7a0` (498 bytes),
backfire `0x46f580` (132) and cutoff `0x46f460` (273). Offsets snapshot three
signed amplitudes and periods from the shift component. Each period clamps to
1..10000 before the calls. A phase step uses exact integer loading, milliseconds,
the supplied duration and the addressed cycle scale. The result truncates to an
integer, subtracts as a 16-bit word, then takes remainder modulo 65535. The three
sine calls remain ordered RPM, throttle, level, with exact signed amplitudes.
A null or inactive shift resets the output words and all three phase words.

Backfire first steps the existing sequence. A request at `+0x152` then consumes
one bounded integer random value and one float random value from the shared
seed, rounds the peak once, resets and appends both kind-6 pulse segments,
clears the request and writes the observed prefix event word at `+8`. Request
production beyond the constructor's initial request remains with the missing
shift/accel/manager graph; recovering the pulse body does not recover that graph.

Cutoff retains its normal/special range gate for context kind 1, context flag
`+0x900`, a nonnull race entry and race-entry mode 6/7. It keeps the spatial
`+8` integer exact through subtraction/division, stores a rounded fraction,
clamps it, then executes the original curve, inverse conversion and integer
cutoff clamp. Spatial `+8` production is not inferred from this consumer.

`EngineSoundCurveInverse.cpp` recovers `0x467ff0` (92 bytes), including the
zero-input return that leaves scratch untouched, signed quotient/remainder,
negative table indices relative to `0x7ee7ec`, low-five-bit arithmetic shift,
and all three scratch writes. `Libs/bWare/Random.cpp` recovers float random
`0x43c1f0` (78), using the previous unsigned seed modulo INT_MAX before advancing
the shared seed. The integer random body `0x43c1c0`, angle sine `0x43c2b0` and
normalization `0x43cf10` are compiled unchanged from existing recovered sources
and independently compared here; no retail helper body substitutes for them.
Unexecuted collision-body relocations are present in that shared source; the
execution guard excludes them, including the ticker.

`EngineSoundAuxiliaryUpdate.cpp` recovers the complete caller `0x47c010`
(395 bytes). It retains the active timing gate, spatial orientation dispatch,
post-dispatch owner reload, position/orientation capture, actual normalization,
rounded position products, envelope `+0x1d0`, idle, virtual offset update,
backfire, virtual RPM/throttle/level updates, cutoff and virtual high-RPM update.
The native frame now calls this body instead of manually scheduling its parts.
Spatial callbacks, shift/accel states and subsequent manager consumers are still
separate seams. The native spatial `+8` input stays an explicit zero fixture;
filters/pan are retained by the snd adapter but are not applied as retail DSP.

The first run passed 832 comparisons and the expanded run 2560, each including
256 carried complete auxiliary frames. Expanded fixtures cover zero orientations,
callback-time car/spatial pointer changes, period/phase boundaries, signed
amplitudes, special cutoff modes and null race entries, full random-seed words,
integer conversion and shared scratch state at both x87 precisions. Spatial
callbacks and shift/accel production remain fixtures. Positions/orientations
are finite; invalid dependencies, concurrent mutation and exceptions are not
established by these checks. Behavioral evidence is separate from byte matching.

The native build and three audio/backend tests pass. The 600-frame SKYLINE
smoke passes with RMS 1772 LSB, 893 releases, zero underruns and correlation
0.809. The 350Z run has RMS 7467 LSB, 1208 releases and zero underruns, but
its correlation 0.486 remains below the unchanged 0.50 threshold. These native
observations do not establish original/rebuilt acoustic equivalence.

Spatial follow-up: descriptor `0x7ed368` identifies `SFXCTL_3DCarPos`, type 8,
factory `0x47c7a0`, allocation `0xd8`, constructor `0x471d60` and table
`0x78ab68`. Its `+0x2c`/`+0x30` methods `0x45e1d0`/`0x45e1e0` store the
supplied pointers at `+0x70`/`+0x74`. They are setters, not output copies.
The real component update, listener state, attenuation/pan and lifecycle remain
required; the new component view does not claim their recovery.

The two spatial pointer setters are now recovered source in
`EngineSoundSpatialPosition.cpp` / `EngineSoundSpatialOrientation.cpp` and
replace the native output-copy/no-op callbacks. The verifier relocates fixture
spatial storage to keep its `0xd8` view separate from its vtable and executes
these setter bodies in constructed/composed frames; selected callback-mutation
fixtures retain scripted virtual boundaries to test owner/spatial reloads.

The 64-case mutation baseline passed 832 comparisons, including 256 carried
full auxiliary frames. All nine mutations were rejected by state/call
mismatches, with no survivors or compile-only rejections: phase modulus,
period clamp, pulse retrigger, special cutoff mode, zero-input scratch write,
shift mask, random-seed timing, backfire call order and position scaling.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AuxOffsets,AuxBackfire,AuxCutoff,AuxFrame,SoundInverse,SoundRandom,SoundRandomFloat,SoundSine,SoundNormalize --cases 64 --mutation-checks --mutation-source aux_producers --mutation-source aux_frame --mutation-source sound_inverse --mutation-source random_float
```

Final regression passed 30720 comparisons across 114 targets: 256 independent
cases each, 128 carried HybridMotor steps, 384 Physics steps, 256 existing
input/gain steps, 256 complete auxiliary frames, and 512 envelope/sequence
steps. Complete-frame fixtures include both active timing states, 64 callback
mutation cases and 192 pulse-producing cases. The spatial setters independently
match their complete 10-byte original bodies; together with the seven existing
leaves, nine strict MSVC byte checks pass. The larger caller/producers retain
behavioral verification only.

After the setter correction, final 600-frame native runs produced SKYLINE
RMS 1980 LSB, 730 releases, zero underruns and correlation 0.861 (pass); 350Z
RMS 7517 LSB, 1172 releases, zero underruns and correlation 0.498 (failure
against 0.50). The earlier numbers above describe the preceding native build.
All three audio/backend tests pass on the final build. Neither profile enters
the high-RPM gate; that transition is covered by the carried binary comparisons.
The complete audio objective remains open, including real shift/accel/spatial
production, overall level/pan, notification/parameter consumers, manager
lifecycle and the other inventoried audio paths.

## Spatial component recovery and native integration (2026-10-20)

The native session now constructs, binds and activates `SFXCTL_3DCarPos`,
executes its recovered update `0x471dc0` each frame, and destroys its sample
average on cleanup. The session owns a persistent `Audio82B884`, initialized
by original constructor `0x4585a0`, for both bank loading and frame execution.
The global root is scoped to those calls and restored afterward. Native ABI
bridges dispatch selection, averaging and the already recovered pointer setters.

| Original address | Recovered body |
| --- | --- |
| `0x45dd60`, `0x471d60`, `0x47c7a0` | Base construction, derived construction and factory |
| `0x45dac0`, `0x45bad0` | Kind-dependent binding and activation |
| `0x45de30`, `0x45de50` | Limit update and integer-to-ST0 query |
| `0x45de60`, `0x45df00` | Angle word and mode-specific planar distances |
| `0x45e100`, `0x45e1f0`, `0x471dc0` | Listener selection, Doppler average and frame caller |
| `0x401150` | Planar distance helper |
| `0x576880`, `0x5768a0` | Default sample-average construction and separate allocation |

The complete `0xd8` layout preserves dirty fields left unwritten by construction.
The getter at `0x45de50` consumes an unused stack argument (`RET 4`) and leaves
its integer exact in ST0. Default average construction leaves its sample pointer
untouched; separate allocation preserves the count, ring index, sum and average.
The factory retains the strict heap threshold and null-allocation return.

Selection keeps three rounded deltas and one extended delta. Both square roots
remain extended through the comparison; spilling the first to binary64 changes
some two-listener decisions under 64-bit x87 precision. Equality/unordered
selects listener 1. Geometry preserves shared point/direction pointers, the low
angle word, mode-specific partial writes, and the early not-ready return.
Doppler uses the component's stored delta, honors the root's `+0x13` gate,
executes actual average insertion/recalculation, then re-reads the current value
for the final raw-word copy. Existing `Normalize2` (`0x43ce50`) and
`HudArcSineUnits` (`0x43c490`) compile unchanged and execute as recovered code.
Only AX is defined by the latter's return. Unexecuted relocations elsewhere in
those shared files are reported and point to unmapped execution traps.

The focused 256-case run passed 4608 comparisons, including 256 carried spatial
updates at both x87 precisions. An additional 64-case baseline passed 1344
comparisons and rejected all eight mutations by state/call mismatches: readiness
gate, partial writes, listener tie, stored delta, cross product, heap threshold,
unwritten sample pointer and allocation reset. Scripted average callbacks test
the post-callback current-value read. The listener arrays remain fixtures;
finite positions, valid listener indices and nonnegative capacities limit this
evidence. These new bodies have behavioral verification; the existing nine
strict leaf byte checks remain separate.

The native parent schedule is Physics, spatial, auxiliary, HybridMotor, playback.
Its manager ordering is still a platform seam. Spatial's retained position pointer
addresses the auxiliary's latest snapshot. Listener-state producer `0x46d7e0`
has been located but is not yet recovered or linked. The demo therefore follows
the original not-ready branch: both distances are -1 and the Doppler average is
1. No camera/listener coordinates are synthesized. `UG2_ENGINE_LOG` includes
`# spatial frame listener distance4 distance8 average` diagnostics. This replaces
the previous spatial `+8` zero fixture without claiming full positional audio.
Overall packed level/pan, shift/accel, notifications, parameter consumers and
manager lifecycle remain open, along with the other inventoried audio paths.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets SpatialBaseConstruct,SpatialConstruct,SpatialFactory,SpatialBind,SpatialActivate,SpatialAngle,SpatialCompute,SpatialSelect,SpatialDoppler,SpatialUpdate,SpatialLimit,SpatialValue,SpatialDistance2D,SpatialNormalize2D,SpatialArcSine,SampleDefault,SampleAllocate --cases 64 --mutation-checks --mutation-source spatial --mutation-source spatial_geometry --mutation-source spatial_lifecycle --mutation-source average_allocate
```

The final native build and all three audio/backend tests pass. Final 600-frame
SKYLINE output has RMS 1728 LSB, 931 packet releases, zero underruns and
pitch/RPM correlation 0.752 (pass). The two-bank 350Z has RMS 8441 LSB,
1390 releases and zero underruns, but correlation 0.484 remains below the
unchanged 0.50 threshold. Neither profile enters the high-RPM gate. Both
produce 600 spatial diagnostics with the original not-ready distances and
unity average. These are native integration observations, not a comparison
against retail acoustic output.

The final full regression passed 35328 comparisons across 131 targets with zero
failures: 256 independent cases each plus 1792 carried steps (including 256
spatial steps). It exercised 64 constructed and 192 direct spatial updates,
55 average-callback mutations, both selected listeners and modes 0..5.
All nine existing strict leaf byte checks pass. The complete audio goal remains
active; integrating this component does not close the listener/manager or other
remaining audio seams.

## Listener production and camera integration (2026-10-20)

`EngineSoundListeners.cpp` recovers the complete listener producer `0x46d7e0`
(1777 bytes). `AudioFindListener.cpp` recovers the separate list lookup
`0x4690b0` (38 bytes). The producer's repeated inline searches remain distinct
from its actual lookup calls. The original audio frame `0x483b40` calls the
listener producer before its manager-update passes; that larger frame remains
unrecovered, and its other branches are not enabled by this integration.

The first path obtains index 0's owner, player record and camera mover, then
reads Camera through mover `+0x1c`. It copies camera position/direction/target,
normalizes the planar camera and car directions, captures car position and
car-minus-camera deltas, saves the previous view mode and applies the original
race/mode/override, player-state, game-flow and sound-state `+0xa8` gates.
The index-1 path retains its separate mode logic; it does not acquire the first
player's race/override/game-flow gates. Missing owner/player/mover paths leave
previous listener state intact. There is no invented clearing or null-camera
fallback. The first race-state comparison still dereferences a null race-state
pointer on the original invalid path; normal fixtures supply a valid pointer.

The complete second-player path also preserves the 3D distance comparison
against `20 - 5`, the strictly-less blend gate, the rounded division result,
all planar differences, negated differences and product spills, and the four
ordered angle-helper calls whose returned angles are discarded. Most weighted
products round before accumulation, while the first direction's Y product stays
extended through its addition. Existing `bDistance` (`0x43ce10`) and
`HudAngleUnits` (`0x43e6d0`) compile unchanged and execute recovered bodies.
Their standalone cases are included in this verifier. Unexecuted relocations in
the shared helper files remain unmapped traps and are listed in the report.

The first 128-case run exposed missing product spills; after correcting those
boundaries, 512 cases per target passed 2048 comparisons. The composed 256-case
run passed 1280 comparisons: four targets plus 256 carried listener → spatial →
cutoff frames. Its standalone producer cases include 128 first-camera paths,
96 second-camera paths, 64 blends, 64 no-blend paths and 80 constructed
listener/spatial/cutoff chains. Carried frames vary camera transforms, owner
positions, one/two-listener selection, camera availability, list membership,
clock suppression and overrides, retaining the shared globals and Doppler ring
between steps. Missing-camera stale state is exercised. Fixtures exclude cycles,
invalid pointers, missing first owner with a live second owner, nonfinite
coordinates, exceptions and concurrent list changes. This is bounded behavioral
evidence, separate from strict byte matching.

The native bridge now publishes the finalized Camera at the existing player
mover's `+0x1c`. Listener production therefore consumes the same recovered
Camera state used for drawing, without a replacement coordinate algorithm.
The persistent audio context owns a minimal one-car manager list pointing at
the already constructed car sound state. Manager allocation/registration and
its full lifecycle remain seams. Sound-state `+0xa8` remains the zero from
native session initialization until its producer is recovered. The native
session remains one-player; two-player behavior is covered by binary comparison.

Native scheduling now calls listener production, Physics, spatial, auxiliary,
HybridMotor and playback. Relative scheduling of those manager components is
still unverified. The retained auxiliary position pointer initially addresses
its activation-time zero snapshot, so the first spatial update can see a large
initial distance; later updates see the preceding auxiliary snapshot. This is
recorded as part of the scheduling seam, not hidden by an invented initialization.
Spatial distance feeds the actual cutoff consumer. Doppler averaging now has
live inputs, but its downstream voice consumers and retail filter/pan DSP are
still open. Overall packed level/pan, shift/accel, notification/parameter paths,
manager lifecycle and the remaining inventory rows keep the audio goal active.

The native build and all six CTest checks pass. The 600-frame SKYLINE smoke has
RMS 1962 LSB, 681 packet releases, zero underruns and pitch/RPM correlation
0.824 (pass). Its 600 spatial diagnostics have distance `+4` 0..1911,
distance `+8` 6..1905 and average 0.4985..1, replacing the previous -1/unity
not-ready output. The 350Z smoke has RMS 9113 LSB, 1726 releases and zero
underruns, but correlation 0.346 and its high/low RPM pitch ordering fail the
unchanged smoke gates. Neither profile enters the high-RPM gate. These are
native integration observations, not retail acoustic equivalence.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ListenerUpdate,ListenerFind,ListenerDistance,ListenerAngle --cases 512
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ListenerUpdate,ListenerFind,ListenerDistance,ListenerAngle --cases 128 --mutation-checks --mutation-source listeners --mutation-source listener_find
```

The six listener mutations all fail by state/call mismatches, with no survivors
or compile-only rejections: stale readiness, previous-mode capture, game-flow
mode, blend equality, product rounding and list index. The 128-case mutation
baseline passed 768 comparisons, including 256 carried steps. Final full
regression passed 36608 comparisons across 135 targets with zero failures,
including 2048 carried steps; all nine existing strict leaf byte checks pass.

Next parent-scheduler evidence: `0x464c10` (111 bytes) advances car timing subject
to the root's `+0x13` byte, walks car `+0x1c` through component `+0x50` links,
calling each `+0x14(duration)` then `+0x18()`, then walks car `+0x20` through
those links calling `+0x24(duration)` then `+0x34()`. `0x469cf0` and derived
`0x479590` gate that call on a live owner; the latter also calls `0x469a10`.
`0x469cb0` separately walks car `+0x20` and calls component `+0x28()`.
These routines have been inspected in Ghidra but are not yet recovered/verified;
the linked-list construction/order and the remaining virtual bodies are needed
to replace the current manual native schedule faithfully.

## Component sort, scheduler and post passes (2026-10-20)

`EngineSoundSchedule.cpp` recovers input sorting `0x464a10` (184 bytes),
car binding `0x464ba0` (10), the two-list scheduler `0x464c10` (111), its
owner-gated wrapper `0x469cf0` (38), separate playback pass `0x469cb0` (32),
playback tail-dispatch `0x460ff0` (5) and shared empty `RET`/`RET 4` leaves
`0x4022c0`/`0x4031c0`. The sort collects into the original 64-entry scratch,
uses `(key4C >> 4) & 127`, retains the initial comparison ceiling of 64,
and preserves the strict tie comparison. It changes only the first list and
its links. Counts, list lengths and keys remain unchecked; the recovery adds
no bounds or clamping not present in the binary.

The scheduler stores the raw duration at car `+0x38` and advances `+0x34`
when the root's `+0x13` byte is clear; otherwise it zeroes `+0x38` only.
That gate does not replace the duration passed to components. Each input
receives `+0x14(duration)` followed by a freshly loaded table's `+0x18()`.
The second head is read after that entire pass; each playback node receives
`+0x24(duration)` and then `+0x34()`. Each next link is read after callbacks.
The owner-gated wrapper checks its owner once, calls car virtual `+0x34`,
runs that scheduler, then clears only byte `+0xc0`. The separate owner-gated
playback pass calls each node's `+0x28`; the engine's implementation tail-calls
its actual update at virtual `+0x48` (`0x474310`). This preserves the original
phase boundary between component production and playback consumption.

`EngineSoundSchedulePost.cpp` recovers Physics post-update `0x470a60`
(617 bytes), HybridMotor post-update `0x470940` (186), four-flag fraction
`0x5ec1f0` (73), and integer approach `0x468e70` (32). Physics post-update
reloads owner speed for four independent extended product/conversion/clamp
chains, emits the fifth packed value from car `+0x48`, queries the four
owner flags at `+0x390..+0x393`, ramps prefix `+0x1c` by the original wrapped
integer steps according to car `+0xa8`, produces player/camera flags, and
retains the race-count/participant branch. That branch keeps signed bytes,
wrapping additions, exact integer loading, the original conversion boundary,
and the 491-step integer approach. The original CRT conversion remains the
analysis dependency already documented above.

HybridMotor post-update checks the converted owner speed and absolute
sample-average delta, retains the raw-word last-time copy, and checks the
extended last-time-plus-delay expression. An active path consumes a bounded
random value only when its unsigned 16-bit countdown is zero, adds 60, requests
auxiliary `+0x152`, and decrements the countdown. Prefix `+4` ramps by 983 or
196 using wrapped arithmetic. The random seed and deferred request are shared
with the already recovered auxiliary pulse producer; this call was previously
absent from native frames. Unknown packed fields remain offset-based.

Native ownership still supplies the four recovered input objects and one
playback object. The key encoding is traced to factory selector `0x476890`;
the original sort now places Physics (ID 0), auxiliary (5), HybridMotor (6)
and spatial (8) in that order. The original registry, dependency resolver and
list construction (`0x476890`, `0x476f80`, `0x47f710`) remain unrecovered, as do
shift and accel producers. Their neutral fixture inputs remain explicit.
Native ABI bridges supply original virtual methods; both newly recovered
post-updates now run. Spatial consumes the auxiliary snapshot from the same
frame, replacing the previously documented one-frame snapshot seam. Manager
allocation/dispatch and the complete game-wide audio frame remain open.

The native frame calls listener production, the actual car update wrapper,
and the actual separate playback pass. It no longer manually increments car
time or schedules Physics/auxiliary/HybridMotor/spatial/playback itself.
Overall level/pan remains the native packed-source fixture, and parameter and
notification ownership remain incomplete. Recovering packed prefix producers
does not establish all consumers or complete positional/filter DSP.

The first 512-case standalone check passed 6144 comparisons over the twelve
new targets. Scripted callback cases cover changed vtables, next links and
second-list heads, the root clock gate, owner changes during the pre-callback,
raw duration and byte cleanup, stable ties, zero/negative/short counts, full
32-bit approach inputs, fractional flag combinations and the independent post
branches. Sort fixtures use at most six nodes, scratch counts through seven,
and keys 0..63; cycles, overlong lists and invalid-key original faults are
excluded. Constructed engine cases supply the valid kind-1/2 binding inputs. Explicit
vtable relocations make every recovered virtual callee execute rebuilt code;
the execution guard rejects entry into its retail body. The composed baseline
passed 2048 comparisons, including 512 carried car frames at both x87 precisions. Playback is inactive in those binary
compositions; its controllers and active native GIN playback are checked
separately. These claims are behavioral evidence, not a full byte match.

The native build and all six CTest checks pass. The 600-frame SKYLINE smoke
has RMS 2006 LSB, 656 releases, zero underruns and pitch/RPM correlation 0.836
(pass). The 350Z has RMS 7595 LSB, 1170 releases and zero underruns, but its
correlation 0.497 still fails the unchanged 0.50 gate. Neither profile enters
the high-RPM gate. Retail acoustic equivalence is not established.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets CarBind,CarSort,CarSchedule,CarFrame,CarPlayback,EngineForward,ScheduleNoop,ScheduleNoopArg,PhysicsPost,ControlPost,PhysicsFraction,ApproachInt --cases 512
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets CarBind,CarSort,CarSchedule,CarFrame,CarPlayback,EngineForward,ScheduleNoop,ScheduleNoopArg,PhysicsPost,ControlPost,PhysicsFraction,ApproachInt --cases 128 --mutation-checks --mutation-source schedule --mutation-source schedule_post
```

The strengthened carried run holds filtered RPM steady for a bounded interval
and keeps the original average producer active. It reaches the delayed
HybridMotor pulse request once per x87 precision (two total), then carries
that request into the actual auxiliary pulse producer on the following frame.
All nine scheduler/post mutations fail by state/call mismatches, with no
survivors or compile-only rejections: clock-duration substitution, cached
vtable, unstable ties, key mask, owner gate, fraction equality, race-count
sign, countdown threshold and packed ramp. The mutation baseline passed 2048
comparisons, including all 512 carried frames.

Final integration follow-up recovers HybridMotor activation `0x45d050`
(22 bytes): set `+0x58` and clear only `+0x7c`, `+0x80`, `+0x84`. The native
session now calls this original body after binding, so its component clock
advances and the post-update can reach the original delayed request. Composed
scheduler fixtures likewise call actual activation instead of writing its flag.
The four added scheduling leaves (car bind, engine tail and both empty leaves)
match their complete 10/5/1/3-byte original bodies in the strict MSVC check.
Before that activation addition, the full 147-target regression passed 40192
comparisons with zero failures; the final activation-inclusive run follows.

Native sorted spatial diagnostics begin at `distance4=0`, `distance8=6`,
with unity average. Over the 600-frame SKYLINE run the ranges are 0, 6..7 and
1 respectively. This removes the previous first-frame 1905-distance artifact
through the recovered component ordering rather than a coordinate override.

Next component descriptors are confirmed in the binary: `SFXCTL_Shifting`
(type 3, descriptor `0x7ed2a4`, factory `0x46ebd0`, size `0x15c`, constructor
`0x45c0d0`, vtable `0x789f28`) and `SFXCTL_AccelTrans` (type 4, descriptor
`0x7ed2d4`, factory `0x46ffa0`, size `0xdc`, constructor `0x45c9c0`, table
`0x789fb8`). Engine dependency 7 is `SFXCTL_Tunnel` (`0x7ed314`, factory
`0x471000`, size `0xe0`, constructor `0x45d950`, table `0x78a064`). These
remain investigation targets, not recovered source. `SFXCTL_MasterVol`
(`0x7ed334`, factory `0x471930`, constructor `0x45dc80`) also remains open.

Final activation-inclusive full regression passed 40448 comparisons across
148 targets with zero failures, including 2560 carried steps. After aligning
the three activation stores with their binary order, a focused rerun passed
896 comparisons (including 512 complete car frames), and both activation
mutations were rejected by state/call mismatches. Activation now matches its
complete 22-byte original body; together with the four new scheduling leaves
and nine existing leaves, fourteen strict byte checks pass. The comparison
commands above remain separate from the active native audio observations.

On the final native build, all six CTest checks pass. The last 600-frame
SKYLINE run has RMS 1794 LSB, 904 packet releases, zero underruns and
correlation 0.808 (pass); 350Z has RMS 7378 LSB, 1150 releases and zero
underruns, but correlation 0.390 remains below 0.50. Earlier smoke numbers
in this section describe the preceding integration stages. All profiles
still miss the high-RPM gate. The complete audio objective remains open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets ControlActivate,CarFrame,CarSchedule --cases 128 --mutation-checks --mutation-source control_lifecycle --mutation-name schedule-control-activate --mutation-name schedule-control-gains
```

## Acceleration-transition component and native input (2026-10-20)

`EngineSoundAccel.cpp` / `EngineSoundAccelLifecycle.cpp` recover the complete
`SFXCTL_AccelTrans` component (type 4, allocation `0xdc`, table `0x789fb8`).
The native session now constructs, binds and activates it, publishes its
auxiliary/shifting dependencies, and runs it through the original scheduler.
Sorting places it after Physics and before auxiliary, HybridMotor and spatial.
The previous acceleration-state fixture is removed. Shifting remains a
separate explicit fixture until its complete producer is recovered.

| Original address | Recovered routine |
| --- | --- |
| `0x45c9c0`, `0x46ffa0` | Construction and factory |
| `0x45ca80`, `0x45cb10` | Binding and activation |
| `0x45cb40`, `0x46ffe0` | Dependency IDs and attachment |
| `0x45cb70`, `0x45cc30` | General and launch envelope setup |
| `0x45cd30`, `0x470030`, `0x45cdc0` | Launch, general and release queries |
| `0x47c1a0`, `0x47c220`, `0x47c290` | RPM, throttle and complete advancement |
| `0x4831a0` | Complete component update |
| `0x45c090` | Shifting activity query, complete EAX result |
| `0x5ecd30`, `0x5947b0` | Owner and drivetrain shift-point queries |
| `0x47aaa0` | Envelope step with a replaced target |

Construction preserves dirty padding and unidentified fields; activation
initializes only the observed flag/state words. Binding retains its kind-1/2/3
constraint and subsequent invalid-kind dereference, performs the actual car
virtual query, then reloads kind and the typed car before choosing tuning.
Kind 1 indexes the `0x7f3248` records from primary-record `+0x18`; other valid
kinds consume one bounded random value to choose `0x7f3284` or `0x7f32c0`.
Attachment queries the supplied descriptor before selecting dependency 3 or 5;
unknown IDs retain the existing diagnostic boundary.

The three queries retain extended subtraction/product comparisons and their
x87 unordered behavior. Several rejection tests reject a strict less-than
rather than accepting only greater-or-equal; quiet NaNs can therefore pass.
The general query snapshots RPM before the actual owner shift-point call, then
reloads later car state. The owner query preserves the separate drivetrain
call/tail boundary for mover kind 1, the alternate layout for kind 3, and the
zero result for other kinds. Neither query introduces a gear bound. These
bodies are recovered separately from the existing HUD convenience wrapper,
which had inlined the kind-1 field read.

Setup preserves exact integer loading for tuning deltas and milliseconds,
rounded duration stores followed by unrounded zero comparisons, raw snapshots,
and explicit envelope initialization instead of replacing those stores with a
configure call. Advancement steps the level, RPM and throttle paths in order,
then transitions launch state 2 to return state 3, converts active transitions
to release state 4 on throttle release, and finishes states 1/3/4 when the RPM
envelope completes. It captures the old RPM and release duration before the
auxiliary virtual frequency call, then reloads tuning and level for the next
configuration. The full caller retains active timing, raw previous/current
throttle flags, rising-edge selection, falling-edge notification and its final
advancement call. Existing envelope stepping and curves execute recovered code.

`EngineSoundEnvelope::StepTarget47AAA0` replaces the target word, calls the
actual step, and copies the supplied target to the output if the envelope is
complete. That final copy also matters when stepping an already-complete
object, whose ordinary step leaves the output untouched. The new paths keep
unknown fields offset-based and do not synthesize shift or transition rules.

The first 128-case run passed 2432 comparisons across the nineteen new targets.
The expanded composed run passed 6400 comparisons: twenty-one selected targets
at 256 cases each, 512 carried acceleration updates, and 512 carried complete
car-scheduler frames. The acceleration trajectory visits states 0/1/2/3/4 for
280/36/36/60/100 frames and produces four release notifications across the two
x87 precisions. Separate cases cover negative/out-of-range states, large signed
tuning integers, zero/negative durations, raw byte flags, both random tuning
choices, callback-time car/kind/tuning/level changes, and selected quiet NaNs.
Both results of every transition query are exercised. Explicit binary guards
prevent rebuilt execution from entering the retail helper bodies.

The actual acceleration constructor/bind/activation now also run in composed
car frames, with the original raw vtable relocated to the rebuilt update.
The standalone auxiliary/gain cases retain controlled acceleration inputs where
that is their documented boundary. Shifting production, original registry/list
construction, invalid pointers/kinds, aliases between tuning and component
storage, nonfinite envelope inputs, exceptions and concurrency remain excluded.
Gear indices 0..7 are fixtures rather than a claim that the original checks
those bounds. These comparisons establish bounded behavior, not a whole-game
or acoustic match.

`UG2_ENGINE_LOG` now includes `# acceleration frame state rpm88 throttleC0
levelA4 releaseD8` diagnostics. The native scheduler owns five recovered inputs
(IDs 0/4/5/6/8) and its existing playback object. Registry and manager ownership,
shifting, Tunnel/MasterVol, overall packed level/pan, notification/parameter
consumers, collision audio and flag-gated frontend/HUD paths remain open.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AccelConstruct,AccelFactory,AccelBind,AccelActivate,AccelDependency,AccelAttach,AccelConfigure,AccelLaunch,AccelQueryLaunch,AccelQuery,AccelRelease,AccelRpm,AccelThrottle,AccelAdvance,AccelUpdate,ShiftActive,OwnerShiftPoint,GearShiftPoint,EnvelopeTarget,CarFrame,CarSchedule --cases 256
```

Eight of the nine initial mutations were rejected by state/call mismatches.
The integer-duration mutation initially used a cast that MSVC retained in
extended precision; it did not actually introduce the intended early rounding.
Replacing it with an explicit volatile binary32 store produced the intended
fault and was rejected in a separate run. All nine intended errors are thus
rejected, with no compile-only rejections. The mutation baselines each passed
3712 comparisons, including 1024 carried acceleration/car frames.
Gear shift-point reading, shifting activity and acceleration activation also
match their complete 14/19/37-byte original bodies; dependency selection retains
behavioral verification without a byte-match claim.

The native build and all six CTest checks pass. Latest 600-frame SKYLINE
output has RMS 2222 LSB, 650 packet releases, zero underruns and pitch/RPM
correlation 0.854 (pass); 350Z has RMS 9154 LSB, 1700 releases, zero underruns
and correlation 0.517 (pass against 0.50). The acceleration diagnostics show
SKYLINE states 0/2/3 for 510/29/61 frames and 350Z for 522/29/49 frames.
Neither native profile enters general/release state or sets the release
notification flag; those paths are covered by the carried binary comparisons.
Neither enters the high-RPM gate. These finite native observations supersede
the preceding smoke results without establishing stability or retail acoustic
equivalence; the complete audio goal remains active.

Next shifting graph: frame `0x483130` calls upshift `0x46ec70` or downshift
`0x46ee60`, then advancement `0x482d50`. Its helpers are `0x47bf20`,
`0x47bea0`, `0x46f000`, `0x46f080` and reset `0x45c430`. Post-update is
`0x45c210`. Binding `0x45c330` selects tuning records with a confirmed
`0x60` byte stride (`LEA index*3; SHL 5`); constructor `0x45c0d0`, activation
`0x45c3d0` and full `0x15c` layout still need recovery. Physics virtual `+0x24`
is `0x45d300`, a float query of Physics `+0x84`. These inspected bodies remain
open; only the separate activity query is recovered in this checkpoint.

Final full regression passed 45824 comparisons across 167 targets with zero
failures, including 3072 carried steps. Seventeen strict leaf byte checks
pass; the larger acceleration routines retain behavioral verification only.
The five-component scheduler sequence still reaches both delayed HybridMotor
pulse requests, while the acceleration sequence retains all state/notification
coverage described above. Native fixture removal does not close the original
shifting, registry/manager or other inventoried audio seams.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --cases 256
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --targets AccelConstruct,AccelFactory,AccelBind,AccelActivate,AccelDependency,AccelAttach,AccelConfigure,AccelLaunch,AccelQueryLaunch,AccelQuery,AccelRelease,AccelRpm,AccelThrottle,AccelAdvance,AccelUpdate,ShiftActive,OwnerShiftPoint,GearShiftPoint,EnvelopeTarget,CarFrame,CarSchedule --cases 128 --mutation-checks --mutation-name accel-edge --mutation-name accel-unordered-release --mutation-name accel-delay-equality --mutation-name accel-bind-reload --mutation-name accel-duration-integer --mutation-name accel-release-snapshot --mutation-name accel-return-state --mutation-name accel-step-order --mutation-name accel-target-copy
```
