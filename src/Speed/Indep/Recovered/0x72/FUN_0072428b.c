// Original UG2 PC v1.2 0x72428b, 61 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0072820c();
int __cdecl FUN_00728469();
int __cdecl FUN_00728744();
#define _DAT_008b4850 (*(undefined4 *)0x008b4850)
#define _DAT_008b4854 (*(undefined4 *)0x008b4854)
#define _DAT_008b4858 (*(undefined4 *)0x008b4858)
#define _DAT_008b48b0 (*(undefined4 *)0x008b48b0)
#define _DAT_008b48b4 (*(undefined4 *)0x008b48b4)
#define _DAT_008b48b8 (*(undefined4 *)0x008b48b8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072428b(void)

{
  _DAT_008b4850 = FUN_00728744;
  _DAT_008b4854 = FUN_00728469;
  _DAT_008b4858 = FUN_0072820c;
  _DAT_008b48b0 = 0x20;
  _DAT_008b48b4 = 0x34;
  _DAT_008b48b8 = 0x38;
  return;
}


