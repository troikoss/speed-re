# Ginsu engine-sound synthesizer

Reference: UG2 PC v1.2 `speed2.exe`, SHA-256
`f9dd86c054878ce6276beb07c1fd61874f7a1e4bf1f241b084c65b73e24168a7`.

UG2 synthesizes player engine sound from `SOUND/ENGINE/GIN_*.gin` files (84 in the
retail install; header `Gnsu20`, little-endian). The `GinsuSynthData` layout (size `0xB0`)
and the `GinsuSynthesis` fields used here are taken from the UG2 disassembly. `BindToData`
compares five magic bytes one at a time.

Sources: `src/Speed/Indep/Src/EAXSound/Ginsu/{Ginsu.h,GinsuData.cpp,GinsuSynth.cpp}`.

## Recovered routines (2026-10-19)

| Address | Routine | Notes |
| --- | --- | --- |
| `0x466750` | `GinsuSynthData::GinsuSynthData` | vtable `0x789ec8` kept as a raw retail word |
| `0x466770` | `GinsuSynthData::BindToData` | magic `Gnsu2`; tables follow the 0x20 header; `minPeriod28` = smallest cycle length |
| `0x466840` | `FrequencyToSample` | linear interpolation over `segCount + 1` frequency positions |
| `0x466910` | `CycleToSample` | linear interpolation over `cycleCount + 1` cycle positions |
| `0x4669d0` | `CyclePeriod` | central difference (scale `0x784260` = 0.5), result left unrounded in ST0 |
| `0x466af0` | `SampleToCycle` | interpolating search; Ghidra split the body at `0x466b2c` |
| `0x466590` | `DecodeBlock` | EA-XA: 19-byte blocks, 32 samples; tables `0x7ed92c`/`0x7ed93c`/`0x7ed950` |
| `0x466c60` | `GetSamples` | int16 saturation; decodes the next block eagerly at a block edge; callers discard the result |
| `0x466560` | `GinsuCeilToInt` (cdecl) | ceiling via round-to-nearest and a sign fix-up |
| `0x467160` | `GinsuSynthesis::SetSynthData` | packet sizes = sample rate × `0x788890`/`0x788894`/`0x788898` |
| `0x467230` | `GinsuSynthesis::UpdateFrequency` | target sample from frequency; countdown = latency / (`0x788894` × 1000) |
| `0x466d30` | `GinsuSynthesis::HandlePacketRelease` | advance, optional cycle jump with linear crossfade, packet submit |

The frequency range in the `.gin` header is engine RPM: `GIN_Mazda_Miata_DCL.gin`
spans about 2171 to 6852.

## Verification

`tools/verify_pc_ginsu.py` builds the sources with MSVC 7.1 and runs the original
and rebuilt routines under Unicorn at 53- and 64-bit x87 precision. Six retail `.gin`
files are bound as fixtures. The comparison covers the arena byte for byte, the return
values, the logical x87 stack and control word, and the snd calls (packets by
content). The routine sizes and hashes are pinned in `config/SPEED2_EXE_1_2/ginsu.json`.

```sh
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py
UG2_DIR=/path/to/UG2 build/venv/bin/python tools/verify_pc_ginsu.py --mutation-checks --cases 384
```

Result: 12,288 comparisons (1,024 per routine), 0 failures. Mutations (384 cases): 12 of
22 rejected. Of the 10 survivors:

- Boundary flips (`freq-min-strict`, `cycle-max-strict`, `sample-cand-equal`) return the
  same value on either branch.
- Three `volatile` removals (`cycle-`, `handle-wp-`, `handle-u-volatile`) compile to the same
  rounding.
- `handle-key-bit` is masked by the zeroed stack.
- `handle-z-order` and `handle-z1-zero` are not reached by any fixture.
- `handle-multiplier` needs `y1 == 0` exactly, which no fixture produces.

These survivors are explanations, not proof of equivalence.
Byte identity is not established. The rebuild models the original's float stores
with `volatile float` temporaries. `DecodeBlock` routes each prediction through small
static helpers so that MSVC 7.1 cannot share a product across statements and round
it to float. The original keeps that product unrounded in ST0.

Limitations (see the verifier's report for the full list):

- The original keys the packet with an uninitialised frame word (bit 31) and leaves
  two packet fields unwritten. The verifier zeroes the stack and the rebuild writes zero.
- Status-word condition bits and exception flags are not compared.
- NaN inputs are only exercised for `CyclePeriod` and the ceiling helper.

## Seams and next steps

The snd library calls `0x721f99` (lock), `0x721fa7` (unlock) and `0x723515`
(packet submit) are declared as `GinsuAudioLock`, `GinsuAudioUnlock` and
`GinsuAudioSubmit`. The native build traps on them (`src/Platform/Native/NativeAudio.cpp`).
The native demo does not reach the synthesizer yet. Next:

1. `0x4778c0`: synthesis start (packet allocation `0x7231d4`, first submits).
2. `0x47d740`: engine SFX setup (binds the `.gin`, calls `SetSynthData`).
3. `0x473e60`/`0x474030`: engine controllers that feed RPM to `UpdateFrequency`.
4. A native SDL2 audio backend behind the snd packet calls, so the demo plays the engine.
