# Repository Guidelines

## Objective and Recovery Rules

The final objective is to reverse engineer EA Black Box's **SPEED** engine into verified, reproducibly buildable source. To keep the scope small, Need for Speed Underground 2 (UG2) PC v1.2 is the first target: it is the reference build for all current evidence, and complete UG2 recovery is the first milestone. A playable demo is an intermediate milestone. Follow `docs/DECOMPILATION_SCOPE.md` for completion evidence.

Keep engine-level code (the `Speed` namespace and `src/Speed/Indep` libraries) separate from UG2-specific game code and data layouts, so later SPEED titles can reuse it. Do not generalize beyond evidence: record only what the UG2 binary shows, and mark anything believed to be shared with other titles as unverified until another build confirms it.

Recover original code and mechanics. Preserve evidence, call boundaries, and arithmetic order. Keep unknown fields offset-based until identified (for example, `value3F0`). Do not invent algorithms or force registers to fake a match. Report behavioral verification separately from byte matching.

Always decompile the original UG2 code; never author replacement game code yourself. Write only the scaffolding necessary for the demo to work, keep it separate from recovered code, and document its seams.

## Ghidra MCP

The `ghidra-mcp` server (tools named `mcp__ghidra-mcp__*`) gives live access to the UG2 binary in a Ghidra project. Use it to read disassembly, decompilation, xrefs, and strings while recovering routines.

- Start with `list_instances`, then `connect_instance` for the UG2 project, and `list_open_programs` to confirm the target. Pass the `program` argument explicitly when several programs are open.
- Prefer read-only tools (`disassemble_function`, `disassemble_bytes`, `read_memory`, `get_functions`, `search_strings`, `list_strings`, `get_language_metadata`). Use `dry_run: true` before any mutating call.
- Mutating tools (`rename_function`, `set_function_prototype`, `set_variables`, `set_variable_type`, `create_function`, `delete_function`, `clear_flow_and_repair`, `set_memory_block`, `save_program`, `exit_ghidra`, `import_file`) change the Ghidra project. Ask before using them. `clear_flow_and_repair` is not idempotent and can clear code.
- `run_ghidra_script` requires `GHIDRA_MCP_ALLOW_SCRIPTS=1` on the server.
- Ghidra names, types, and decompiler output are analysis aids, not evidence. Record only what the binary shows, cite the original address, and confirm matches with the `tools/verify_pc_*` verifiers. Ghidra project files (`speed-re.*`) are git-ignored; keep them out of commits.

## Project Structure & Module Organization

- `src/Speed/Indep/Src/`: recovered physics, collision, gameplay, camera, HUD, and world routines.
- `src/Speed/Indep/Libs/bWare/`: recovered string and compression code.
- `src/Platform/Native/`: Linux demo, rendering, asset loading, compatibility layers, and documented platform seams.
- `tools/`: Python analysis, probes, and original/rebuilt verification scripts.
- `config/SPEED2_EXE_1_2/`: reference function manifests and fingerprints.
- `docs/`: subsystem evidence, limitations, and demo instructions; start at `docs/README.md`.
- Ignored `build/`, `orig/`, and `local/`: generated results and local dependencies. Assets come from the player's installation.

## Build, Test, and Development Commands

```sh
cmake -S . -B build/native
cmake --build build/native
build/native/ug2demo --car SKYLINE
python -m venv build/venv
build/venv/bin/pip install -r requirements-pc-analysis.txt
build/venv/bin/python tools/verify_pc_wheel_math.py
```

CMake builds the 32-bit Linux demo and analysis executables; provide i386 compiler, SDL2, and OpenGL dependencies. Verification requires local MSVC 7.1 and wibo; string matching also uses delink and objdiff. Set `UG2_EXE` where supported, or use the verifier's executable argument/default. **Run MSVC/wibo jobs sequentially:** they share temporary filenames.

## Coding Style & Naming Conventions

Follow surrounding formatting; generally use four-space indentation in C++ and Python. Native targets use C++11; recovered code must remain compatible with its legacy verification compiler. Retain original addresses in recovery comments and explicit x86 layouts. Keep platform scaffolding separate from recovered routines. No repository-wide formatter or linter is configured.

## Testing Guidelines

Use `tools/verify_pc_<subsystem>.py` for verification and `probe_pc_<topic>.py` for investigation. Tests use Unicorn, Capstone, PE/COFF analysis, and byte comparisons. Run affected verifiers and native smoke checks. Record fingerprints, cases, unresolved callees, and limitations; finite behavioral tests do not establish complete coverage. Update subsystem documentation with evidence.

## Commit & Pull Request Guidelines

Use concise imperative subjects matching history: `Recover ...`, `Draw ...`, or `Add ...`. Keep changes focused. PRs should identify routines/addresses, explain behavior and platform seams, list validation commands/results, and disclose remaining gaps. Include screenshots for visual demo changes. Preserve imported-tool attribution; never commit retail assets, executables, compiler binaries, or generated disassembly.
