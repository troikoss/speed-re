# Ginsu engine-sound synthesizer

Reference: UG2 PC v1.2 `speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.

UG2 synthesizes player engine sound from `SOUND/ENGINE/GIN_*.gin` files (81 in the
retail copy checked; header `Gnsu20`, little-endian). The `GinsuSynthData` layout (size
`0xB0`) and the `GinsuSynthesis` layout (size `0x44`) are taken from the UG2 disassembly.
`BindToData` compares five magic bytes one at a time; the start format writes `samplerep` 8;
`SND_attrsetdef` writes `platformver` 0 and `rendermode` 0.

Sources: `src/Speed/Indep/Src/EAXSound/Ginsu/{Ginsu.h,GinsuData.cpp,GinsuSynth.cpp}`.
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

## Engine setup and controller (evidence, not recovered)

- `0x47d740` (engine SFX setup, not recovered) keeps slot records of `0x1c` bytes at
  `this+0xcc+0x1c*slot`: `+0` the `GinsuSynthData`, `+4` the `GinsuSynthesis`, `+8` the memory
  block, `+0xc` the maximum frequency, `+0x10` the minimum frequency, `+0x14` the start result.
  The mode `[[this+0x9c]+0x78]` selects one slot (mode 0), two slots (mode 1) or none.
- The gin for a slot is read from the table at `0x82b884`: slot 0 from `+0xdc[index]`, slot 1
  from `+0xe4[index]`, with `index = [[this+0x9c]+0x10]`. The table fill is not recovered.
- The memory block is `overhead(8) + 4 + 4 * ceil((0x788898 + 0x788894) * 0x78a5b0)` bytes.
  With the 48000.0 block rate at `0x78a5b0` that is 2624 bytes: two 552-sample packet buffers
  plus the player.
- `StartSynthesis` is called with the gin's minimum frequency (`0x47d8f9`).
- The engine controller `0x473e60` (not recovered) makes snd calls for the voice (`0x72412a`
  with the voice handle at `+0xe0` and a value clamped to 0..127 at `+0x108`, and further
  calls for pitch and pan), then calls
  `UpdateFrequency(arg, 60.0f)` on `[this+0xd0]`, which is slot 0's synthesizer. `0x473fc7`
  pushes `0x42700000` (60.0). The argument is passed through without a clamp; the clamp is
  `FrequencyToSample` returning the first table entry below the minimum.

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

`NativeEngineSound.cpp` binds the chosen gin, builds the synthesizer with the memory size
above, calls `SetSynthData` and `StartSynthesis(minFrequency4)`, and calls
`UpdateFrequency(rpm, 60.0f)` once per frame. The default gin is `GIN_Nissan_Skyline.gin`
(`--engine-gin` chooses another file). The per-car choice is not recovered.

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
- `0x473e60` and `0x474030` (engine controller), `0x47d740` (engine SFX setup, the per-car gin
  table at `0x82b884`) and the second slot's feed in mode 1 are not recovered. The demo feeds one
  synthesizer with the RPM.
- Not yet located in UG2: a stop or current-pitch routine of the synthesizer,
  and any callers of the synthesizer beyond the engine setup.
