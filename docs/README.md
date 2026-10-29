# Documentation

## Scope and development

- [Decompilation scope](DECOMPILATION_SCOPE.md): the SPEED engine objective, the UG2 first-target milestone and required completion evidence.
- [Repository guidelines](../AGENTS.md): contributor workflow, build commands, and recovery rules.
- [Native modding](../modding/README.md): optional C SDK, native plugins, widescreen HUD/FOV, and physics-provider seams.
- [Native demo](NATIVE_DEMO.md): recovered routines, platform seams, build/run instructions, and outstanding integration checks.
- [Native replacement inventory](NATIVE_REPLACEMENT_INVENTORY.md): every native substitute, its recovered replacement target, and difficulty/importance ratings.
- [Demo substitute audit](demo-substitute-audit.md): remaining native gameplay substitutes, recovery order, and verified pedal/kinematics integration.

## Recovery evidence

- [NFSPluginSDK reference leads](nfspluginsdk-reference.md): pinned external type/naming leads and cross-title layout limits.

- [Strings](pc-strings.md): nine byte-matching routines, behavioral checks, and local reproduction.
- [HUD minimap](pc-hud-minimap.md): angle/TrackInfo/indicator helper verification, city-map integration and rendering seams.
- [Collision notifications](pc-collision-notifications.md): collision sound wrappers, game message post and profile readers.
- [Track markers](pc-track-markers.md): position marker chunk, queries and the race-start grid used for the native spawn.
- [Freeroam HUD](pc-hud-freeroam.md): turbo/NOS/heat, shift light, career text, GPS and notification scheduling, mask dispatch and bounded animation evidence.
- [Pause menu](pc-pause-menu.md): `UI_Pause.fng` object, option lists and item handlers, with owned UTF-16 storage and SEH-aware original/rebuilt verification; runs in the native demo on `Esc`.
- [In-game settings](pc-pause-options.md): recovered category screens and option handlers, bounded verification, and native list/widget seams.
- [Controls](pc-controls.md): keyboard binding item recovery, native Controls page, and SDL gamepad/mouse UI routing.
- [Racing](pc-racing.md): drift ranking, career reward/eligibility recovery, and bounded verification.
- [Vehicle physics](pc-vehicle-physics.md): timestep, integrator, engine, and drivetrain investigation.
- [Human steering](pc-human-steering.md): timed-average and rate-helper recovery, constructor integration and remaining physics dependencies.
- [Camera](pc-camera.md): view-matrix helpers and the drive camera's world/car collision pushes.
- [SSE power](D3DX_CRT_POWER_SSE.md): original table-driven power, packed arithmetic, exception exits and hardware-assisted verification.
- [Secondary wheel quad](pc-wheel-secondary.md): tyre-mask geometry, corrected caller flags, exact Y rotation and native capture verification.
- [Wheel and body physics](pc-wheel-physics.md): vector math, wheel preparation/correction, coupling, and body verification.
- [Ginsu engine sound](pc-ginsu.md): granular `.gin` data, EA-XA decoding and packet synthesis, with verification under both x87 precisions.

The physics documents preserve the recovery sequence: later findings supersede
earlier leads. Use the native demo guide for integration status. Source presence,
finite behavioral comparisons, and byte matching are distinct evidence levels;
none alone establishes complete game recovery.

Retail inputs and generated disassembly/reports stay in ignored local directories.
Document their fingerprints, commands, and limitations here without redistributing
them. Session-specific commit state and temporary goals belong in chat or Git history.

- [Physics parameter layout and drive curve](pc-physics-parameters.md)
- [bWare slot pool](pc-bware-slotpool.md)
