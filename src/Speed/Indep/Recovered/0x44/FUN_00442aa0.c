// Original UG2 PC v1.2 0x442aa0, 57 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_00442aa0(int param_1)

{
  *(undefined4 *)(param_1 + 0x1c8c) = 0;
  *(undefined4 *)(param_1 + 0x1c90) = 0;
  *(undefined4 *)(param_1 + 0x1c94) = 0;
  *(undefined4 *)(param_1 + 0x1c98) = 0;
  *(undefined4 *)(param_1 + 0x1c9c) = 0;
  *(undefined4 *)(param_1 + 0x1ca0) = 0;
  *(undefined4 *)(param_1 + 0x1ca4) = 0;
  *(undefined4 *)(param_1 + 0x1ca8) = 0;
  *(undefined4 *)(param_1 + 0x1cac) = 0;
  return;
}


