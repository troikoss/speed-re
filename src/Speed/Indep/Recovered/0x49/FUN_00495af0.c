// Original UG2 PC v1.2 0x495af0, 31 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050b8c0();
int __cdecl FUN_005309e0();
#define DAT_00863480 (*(undefined4 *)0x00863480)

void __fastcall FUN_00495af0(int param_1)

{
  FUN_0050b8c0(*(undefined4 *)(param_1 + 4),0);
  FUN_005309e0(0,DAT_00863480,0);
  return;
}


