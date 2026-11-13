// Original UG2 PC v1.2 0x4c53a0, 40 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
typedef unsigned short wchar_t16;
int __cdecl FUN_004c4f50();
#define _DAT_008383a8 (*(undefined4 *)0x008383a8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004c53a0(void *_this, int _edx_unused, int param_1, undefined4 param_2, undefined4 param_3, int _pad0)

{
  if (param_1 == -0x3c69f147) {
    _DAT_008383a8 = 1;
    FUN_004c4f50(*(undefined4 *)((int)_this + 4),param_3);
  }
  return;
}


