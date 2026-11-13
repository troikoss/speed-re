// Original UG2 PC v1.2 0x498040, 47 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00558790();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_00498040(int param_1)

{
  FUN_00558790(*(undefined4 *)(param_1 + 4),&DAT_00783fe5,0xf770,0x61c,0x43a743aa,0x1190b316,0,0,
               0xc4f4678e);
  return;
}


