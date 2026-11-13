// Original UG2 PC v1.2 0x498000, 55 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004ffa80();
int __cdecl FUN_00558750();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_00498000(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar1 = *(undefined4 *)(param_1 + 4);
  uVar2 = FUN_004ffa80(0xc4f4678e);
  FUN_00558750(uVar1,&DAT_00783fe5,0xf770,0x61c,0x43a743ab,0x1190b317,0,0,uVar2);
  return;
}


