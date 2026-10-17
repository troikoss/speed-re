# NFSPluginSDK reference leads

User-supplied reference: [berkayylmao/NFSPluginSDK](https://github.com/berkayylmao/NFSPluginSDK).
Reviewed 2026-10-18 at revision
[`d238bdffc648498840d17133edb256fa0310d355`](https://github.com/berkayylmao/NFSPluginSDK/tree/d238bdffc648498840d17133edb256fa0310d355).
The project covers Most Wanted 2005, Carbon and ProStreet, rather than UG2.
It provides reverse-engineered types and wrappers that call retail addresses;
those wrappers do not supply the original routine bodies needed by this project.
Its license is BSD-3-Clause (Berkay Yigit); no SDK source is imported here.

The following are **cross-title leads**, not evidence that UG2 and ProStreet
share an implementation. Offsets below are inferred from the SDK declarations
under the Windows x86 layout and compared with independently recovered UG2
fields. Verify each additional name/field against the UG2 v1.2 binary before
renaming an offset-based field or changing source.

| SDK reference | Useful lead | UG2 evidence and limit |
| --- | --- | --- |
| [ProStreet FEObject](https://github.com/berkayylmao/NFSPluginSDK/blob/d238bdffc648498840d17133edb256fa0310d355/NFSPluginSDK/Game.ProStreet/Types/FEObject.h) | Name hash +0x10, type +0x18, flags +0x1c, current script +0x54 | Corresponding UG2 fields already observed; intervening names such as handle/user parameter/data size still require UG2 evidence. |
| [ProStreet FEString](https://github.com/berkayylmao/NFSPluginSDK/blob/d238bdffc648498840d17133edb256fa0310d355/NFSPluginSDK/Game.ProStreet/Types/FEString.h) | Label hash +0x60, wide-string storage after it | UG2 setter `0x50c900` independently writes +0x60. This does not recover UG2 string allocation or observer internals. |
| [ProStreet FEGroup](https://github.com/berkayylmao/NFSPluginSDK/blob/d238bdffc648498840d17133edb256fa0310d355/NFSPluginSDK/Game.ProStreet/Types/FEGroup.h) | Child list and callback traversal | UG2 `0x504830`/`0x504880` already establish traversal and child head +0x64; SDK callback adapters are not original UG2 code. |
| [ProStreet FEScript](https://github.com/berkayylmao/NFSPluginSDK/blob/d238bdffc648498840d17133edb256fa0310d355/NFSPluginSDK/Game.ProStreet/Types/FEScript.h) | Length/time/flags/chain/tracks/name ID layout | Consistent naming leads for the existing UG2 script prefix; use when tracing `0x51cf70`/`0x517720`. Playback remains unrecovered. |
| [Shared enums](https://github.com/berkayylmao/NFSPluginSDK/blob/d238bdffc648498840d17133edb256fa0310d355/NFSPluginSDK/Shared/Enums.h) | Image/string/group values 1/2/5 | These three values independently agree with UG2 users; other values/flags remain unverified for UG2. |

A concrete portability limit: SDK ProStreet `FEMinNode` and `FEMinList` have
virtual destructors; its MW05 declarations do not. This changes member offsets.
Do not copy MW05 offsets or callable addresses into UG2, or declare the layouts
shared across the engine based on these header similarities. Original UG2
execution, fingerprints and bounded differential tests remain authoritative.

For the active frontend work, the SDK is most useful for script, object and
string naming hypotheses. The inspected types do not provide the original archive search body
`0x51bb70` (now independently recovered here), package lifecycle or Linux
rendering/storage implementations documented in
[the pause-menu evidence](pc-pause-menu.md).
