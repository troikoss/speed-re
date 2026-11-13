// Original UG2 PC v1.2 0x4aecb0, 46 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050d2a0();
int __cdecl FUN_0055dda0();
#define DAT_0083a9d8 (*(undefined4 *)0x0083a9d8)

void __stdcall FUN_004aecb0(byte *param_1, int param_2, undefined4 param_3, uint param_4, int _pad0)

{
  if (param_2 == 0xc407210) {
    DAT_0083a9d8 = FUN_0050d2a0(param_4);
    FUN_0055dda0(0xd05fc3a3,param_1);
  }
  return;
}


