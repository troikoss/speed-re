# Auto-matched recovery

A compile-and-compare pipeline that turns decompiler output into C that compiles to the same
machine code as the original function. It exists to scale recovery beyond hand work and to give
every recovered function the strongest evidence level the repository uses: **byte matching**.

## Pipeline

1. **Function list**: `tools/recover_pc_index.py` builds `build/analysis/function-index.json`
   (boundaries and sizes) from the community IDC (`idc` branch of yugecin/nfsu2-re).
2. **Decompile**: Ghidra (headless, local, not committed) imports `speed2.exe`, creates functions at
   those boundaries, commits each function's inferred signature (a first pass, so callers in the
   second pass see callees' real parameters and calling conventions) and exports C to
   `build/ghidra/decomp2.jsonl`. `names.txt` maps Ghidra's real labels (CRT, etc.) to addresses.
   The helper scripts live in `build/ghidra/scripts/` (`DecompileAll2.java`, `ExportNames.java`).
3. **Wrap and compile**: `tools/auto_match.py` adds a small prelude (`undefined4`, `float10`, ...),
   declares callees from their own bytes (`ret N` means stdcall with N/4 arguments; reads `ecx` before
   writing it means thiscall), and compiles with MSVC 7.1 under wibo.
   - `__thiscall` is rewritten to `__fastcall` with a dummy `edx` parameter for the target function.
   - Calls to thiscall callees become C++ member calls (`((TC_x *)(a))->m(b)`), compiled with `/TP`,
     because MSVC 7.1 rejects the `__thiscall` keyword.
   - Globals become absolute-address macros, with the data type tried as int, float, double, byte, short.
   - Option sets tried: `/Ox /Ob0`, `/Ox /Ob2`, `/O2 /Ob0 /Gy`.
4. **Compare**: the compiled function's bytes must equal the original's. Relocated operands (call
   targets) are masked on both sides; trailing padding is ignored.
5. **Promote**: `tools/promote_auto_match.py` writes each match to
   `src/Speed/Indep/Recovered/<0xNN>/FUN_<addr>.c` and records address, size, SHA-256 of the original
   bytes and compiler flags in `config/SPEED2_EXE_1_2/auto-matched.json`.
6. **Re-verify**: `tools/verify_auto_matched.py` recompiles every promoted file from `src/` and checks
   it against the executable again. Run it after any edit, rename or retype of these files.

```sh
export PATH=$PWD/build/venv/bin:$PATH INCLUDE=$PWD/build/compilers/Win32/7.1/Include
python tools/auto_match.py --limit 100000 --max-size 400 --jobs 4   # attempt (resumable)
python tools/promote_auto_match.py                                   # write src/ + manifest
python tools/verify_auto_matched.py                                  # independent re-check
python tools/coverage_inventory.py orig/SPEED2_EXE_1_2/speed2.exe --index build/analysis/function-index.json
```

## Typed decompilation and the struct header

`tools/idc_to_c.py` converts the community IDC (never executing it) into `build/types/ug2_types.h`
(about 460 structs with member offsets and types, typedefs) and `protos.tsv` (about 1,660 function
prototypes, including calling conventions). A Ghidra script (`tools/ghidra/DecompileTyped.java`; `DecompileAll2.java` is the untyped two-pass variant and `ExportNames.java` exports Ghidra's own labels)
loads the header, applies the prototypes, commits signatures and re-decompiles, so the C uses named
struct fields and correct argument counts. `tools/auto_match.py` includes only the declarations a
function actually uses (and their dependencies). Measured effect on byte matching was small (41 more
matches out of about 15,000 retries): byte matching is limited by compiler code shape, not by types.
Types matter more for readability and for the behaviour tier.

## Behaviour tier (weaker evidence, kept separate)

Many decompilations compile to *different but equivalent* code. `tools/diff_test.py` runs the
candidate and the original function side by side in Unicorn on identical random arguments and memory
and compares return value, callee-saved registers, stack pointer, the x87 result, and every byte of
writable memory (plus any pages mapped on demand). `edx` is compared only for 64-bit returns.
Unmapped accesses are mapped lazily with contents derived from the address, identically in both runs.
`tools/behavior_pass.py` applies it to functions that compiled but did not byte-match and writes the
survivors to `src/Speed/Indep/Recovered/Behavior/` with `config/SPEED2_EXE_1_2/behavior-matched.json`.
`tools/verify_behavior_matched.py` re-checks every entry with different seeds and more trials.

Observed: a first run accepted 1,084 functions and the independent re-check (new seed, 64 trials) rejected 36 (3.3 %); a second run accepted 597 more and its re-check rejected 61 of the full 1,645 (3.7 %). Rejected functions are removed. Acceptance is never final until a re-check with a seed different from the acceptance run passes; `verify_behavior_matched.py --seed N` makes that explicit.
 The tier proves equivalence on random inputs only; branches the inputs do not
reach are untested, and results depend on the decompiler's guesses about types.

## What a match does and does not establish

- Establishes: for that function, the shown C compiles (MSVC 7.1) to the original instructions, with
  call-target and relocation operands masked.
- Does not establish: original names, types or structure; that a parameter is a float and not an
  int when the bytes allow both; behavioural equivalence in context; callee conventions beyond what
  the bytes of the callee show. The files are **not** part of the `ug2recovered` library (globals are
  absolute-address macros, callees are undefined externals).
- Readability work (names, struct types, comments) is expected afterwards. Because
  `verify_auto_matched.py` re-checks the bytes, a rename or retype that changes the compiled code
  is caught immediately. That makes automated or AI-assisted cleanup safe to attempt.

## Known limits

- Register-argument functions (`unaff_`, `in_` registers), jump tables and hand-written assembly are
  skipped.
- MSVC codegen is sensitive to source shape. Many decompilations compile to different but equivalent
  code (different register allocation, loop shape, condition folding) and count as misses.
- Library code (CRT, D3DX) matches only when Ghidra's output happens to fit the compiler's idioms.
- The compiler is the decomp.me community build of MSVC 7.1 (13.10.3077). Results are only as
  faithful as that build.
