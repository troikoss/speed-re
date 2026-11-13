# Build and verification environment (Linux, Arch)

How the toolchain was provisioned on 2026-11-13 and what it verifies. Everything lives under
git-ignored paths (`build/`, `orig/`, `local/`, `Need for Speed Underground 2/`); no retail
files, compilers or executables are committed.

## Provisioning

| Piece | Where | Source |
| --- | --- | --- |
| Python deps | `build/venv` (venv) | `pip install -r requirements-pc-analysis.txt` (capstone 5.0.6, pefile 2024.8.26, unicorn 2.1.4) |
| wibo 1.2.0 | `build/tools/wibo` | decompals/wibo release `wibo-x86_64` |
| MSVC 7.1 (`cl.exe` 13.10.3077) | `build/compilers/Win32/7.1/` (`Bin/*` flattened next to `Include/`) | decomp.me community mirror (`msvc7.0.tar.gz`, the archive decompme/compilers uses for `msvc7.1`) |
| objdiff-cli 3.7.0 | `build/tools/objdiff-cli` | encounter/objdiff release |
| delink 0.16.4 | `build/tools/delink` | dbalatoni13/delink release |
| 32-bit SDL2 2.30.9 | `build/deps/sdl2-i386` | built from source with `-m32` (no `lib32-sdl2` package on Arch) |
| Reference exe | `orig/SPEED2_EXE_1_2/speed2.exe` and `Need for Speed Underground 2/speed2.exe` | copy / symlink of the player's `SPEED2.EXE` (SHA-256 `f9dd86c0…68a7`, matches `config/`) |
| Game data | `Need for Speed Underground 2/*`, `local/game/*` | symlinks into the player's install |
| Function index | `build/analysis/function-index.json` | `tools/recover_pc_index.py --idc SPEED2.idc` with the `idc` branch of yugecin/nfsu2-re |
| Call graph | `build/ghidra/callgraph.jsonl` | `tools/export_callgraph.py` (Capstone stand-in for the Ghidra export) |
| Ghidra 12.2 | `~/tools/ghidra_12.2_PUBLIC` (outside the repo) | NSA release; the ghidra-mcp server named in `AGENTS.md` is not installed |

Per-run environment: `export PATH=$PWD/build/venv/bin:$PATH INCLUDE=$PWD/build/compilers/Win32/7.1/Include`.
`INCLUDE` is needed for system headers (`stdarg.h`, `string.h`). Create the `build/analysis/<name>/`
output directory a verifier writes to if it does not exist yet (several do not create it).

Native build (needs the 32-bit libraries):

```sh
cmake -S . -B build/native -DUG2_I386_LIBDIRS="$PWD/build/deps/sdl2-i386/lib;/usr/lib32"
cmake --build build/native -j8
LD_LIBRARY_PATH=$PWD/build/deps/sdl2-i386/lib build/native/ug2sim local/game SKYLINE 1200
```

`ug2sim` wants the lowercase `speed2.exe` in the game directory, which `local/game` provides.
`UG2_I386_LIBDIRS` is a CMake cache variable (default `/usr/lib/i386-linux-gnu`).

## Verifier results (2026-11-13)

`tools/verify_pc_*.py`, run sequentially (wibo jobs share temporary files, so never in parallel).

- The MSVC 7.1 build above rejects the `__thiscall` keyword with `error C4234` (and silently compiles
  it as cdecl if the warning is disabled). Verifiers that use `__thiscall` therefore must be run
  with `--compiler clang-cl`, the second supported compiler.
- Result: all but the exceptions below pass (see the dated list in the commit/PR that adds this file).

Known remaining failures at the time of writing:

| Verifier | Symptom | Likely cause (unconfirmed) |
| --- | --- | --- |
| `verify_pc_car_construction` | unexpected external `CarNullMover::Body0C` | symbol resolution differs on this compiler build |
| `verify_pc_car_render_coefficients` | x87 coefficient differs by ~1e-10 | host x87/compiler difference |
| `verify_pc_effect_shader_resources` | `KeyError: _D3DXEffectDebugMute@4` | decorated-name lookup |
| `verify_pc_feng_inheritance`, `verify_pc_feng_lookup`, `verify_pc_icon_scroller_insertion` | `KeyError ??_E…@@UAEPAXI@Z` | deleting-destructor symbol missing under clang-cl |

These are environment/toolchain mismatches until shown otherwise; none has been traced to the
recovered source.
