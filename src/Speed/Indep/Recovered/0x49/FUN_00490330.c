// Original UG2 PC v1.2 0x490330, 116 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0048b4e0();
int __cdecl FUN_004901d0();
#define DAT_00883dac (*(undefined4 *)0x00883dac)
#define _DAT_0082d5a8 (*(undefined4 *)0x0082d5a8)
#define _DAT_0082d5ac (*(undefined4 *)0x0082d5ac)
#define _DAT_0082d5b0 (*(undefined4 *)0x0082d5b0)
#define _DAT_0082d5b4 (*(undefined4 *)0x0082d5b4)
#define _DAT_0082d5b8 (*(undefined4 *)0x0082d5b8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00490330(void)

{
  FUN_0048b4e0(*(undefined4 *)(DAT_00883dac + 0x90));
  _DAT_0082d5a8 = FUN_004901d0(0x47beb4b6,1,0);
  _DAT_0082d5ac = FUN_004901d0(0xd4d26c59,1,0);
  _DAT_0082d5b0 = FUN_004901d0(0x1744b82d,1,0);
  _DAT_0082d5b4 = FUN_004901d0(0xad3c5239,1,0);
  _DAT_0082d5b8 = FUN_004901d0(0xd4d80a65,1,0);
  return;
}


