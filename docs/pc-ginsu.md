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
| `0x47d740` | `EngineSound::Setup47D740` | one/two-slot setup; native demo calls mode 0 with a scaffold bank view |
| `0x473e60` | `EngineSound::Update473E60` | one-slot controller; recovered and verified, native integration open |
| `0x474030` | `EngineSound::Update474030` | two-slot controller; recovered and verified, native integration open |
| `0x474310` | `EngineSound::Update474310` | control frequency source, one/two-slot selection and parameter-block tail; recovered and verified, native integration open |

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
  from `+0xe4[index]`, with `index = [[this+0x9c]+0x10]`. The table fill is not recovered.
- The memory block is `overhead(8) + 4 + 4 * ceil((0x788898 + 0x788894) * 0x78a5b0)` bytes.
  With the 48000.0 block rate at `0x78a5b0` that is 2624 bytes: two 552-sample packet buffers
  plus the player.
- `StartSynthesis` is called with the gin's minimum frequency (`0x47d8f9`).
- The engine controller `0x473e60` (recovered, not wired natively) makes snd calls for the voice (`0x72412a`
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
queue of up to 32 packets. Not modelled: the volume, pitch and pan options (the retail
controller sets the voice volume, so the native output does not depend on `vol = 0`), voices,
render modes and the frame-count callback.

SDL's device lock is recursive. A scratch program (SDL 2.32.10, disk driver) locked the device
twice on the main thread and once inside the audio callback, and completed 27 callbacks in
300 ms without deadlock. The demo exercises the same path: every released packet nests
`SNDSYS_entercritical` inside the callback.

`NativeEngineSound.cpp` reads the chosen gin and calls recovered setup `0x47d740`, then calls
`UpdateFrequency(rpm, 60.0f)` once per frame. The default gin is `GIN_Nissan_Skyline.gin`
(`--engine-gin` chooses another file). The per-car choice is not recovered. Setup temporarily receives a native EAXSound bank
view, bank index 0, car mode 0 and a fresh slot with start result -1; the prior audio
object is restored before returning. These are explicit construction seams. The
recovered allocator wrapper `0x440820` calls the existing native `bMalloc` backend
(`calloc`); native heap-capacity query `0x43e910` returns an unbounded capacity
(`INT_MAX`) because this heap has no bWare partition budget. The game-side
capacity checks and allocation flags remain in recovered code.

Headless check (`tools/smoke_native_engine_audio.py --frames 600`, final run): 6.92 s of
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
- NaN inputs are only exercised for `CyclePeriod` and the ceiling helper.

## Seams and next steps

- The snd library behind the seam (voices, mixer, volume `0x72412a`, pitch and pan) is not
  recovered. The native player applies no volume, so the controller's volume is absent.
- The controllers `0x473e60` and `0x474030` are recovered and behaviorally checked,
  including the second slot's feed. Native integration remains open: the control-state
  producer, virtual queries, parameter callback registration/consumers, the producer of the per-car gin table
  at `0x82b884`, virtual gain/spatial queries and snd setters. The demo still feeds
  one synthesizer directly with engine RPM.
- Not yet located in UG2: a stop or current-pitch routine of the synthesizer,
  and any callers of the synthesizer beyond the engine setup.


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
