# SPEED engine decompilation: UG2 as the first target

The final objective is to reverse engineer EA Black Box's **SPEED** engine into
verified, reproducibly buildable source. To keep the scope small, Need for Speed
Underground 2 (UG2) PC v1.2 is the first target. The earlier objective (revised
2026-10-11) was complete UG2 recovery; it is now the first milestone toward the
engine objective, and its completion criteria below are unchanged. A basic
playable demo remains an intermediate milestone; it does not complete either.

Engine scope: code under the `Speed` namespace and `src/Speed/Indep` is treated
as engine code and kept separate from UG2-specific game logic, data layouts and
platform scaffolding. Evidence comes only from the UG2 binary. Claims that a
routine or layout is shared by other SPEED titles stay unverified until another
build confirms them; do not generalize ahead of evidence. Recovery of further
titles is out of scope until the UG2 milestone is complete.

Recover original code and mechanics from the locally supplied reference
executable. Preserve evidence, call boundaries and arithmetic order. Keep
unknown layouts explicit and distinguish platform scaffolding from recovered
game routines. Do not substitute invented gameplay algorithms for original
behavior.

Prioritize the driving and integration chain already under investigation, then
extend recovery across the whole game: asset loading, rendering, audio, input,
world simulation, collision, AI, racing, career, police, customization, menus,
and persistence. Maintain a coverage inventory with unresolved routines,
layouts, dependencies and verification limits; source presence alone is not
evidence of completion.

Complete decompilation requires:

- Accounted-for executable code and data, with remaining unknowns resolved or
  explicitly justified as external compiler/platform runtime dependencies.
- Reproducible source builds of the game implementation, independent of a
  retail executable as the runtime implementation.
- Original UG2 assets loaded from the player's local installation, without
  redistributing retail files.
- Verification of recovered routines and their actual caller composition,
  parameter loading, state transitions and subsystem integration.
- A playable rebuilt game covering the original gameplay systems, with
  repeatable end-to-end checks and documented limitations.
- Byte-matching status reported separately from bounded behavioral evidence;
  neither isolated emulation tests nor a playable driving demo establish
  executable-wide completion.

Record provenance and verification evidence as recovery proceeds. Retail
assets, executables, compilers and generated disassembly remain local. Current
verification still depends on an original conversion helper and mapped retail
data; these analysis dependencies are not a completed native runtime.

Work on the separate application project is deferred. This repository's
objective is original UG2 recovery.
