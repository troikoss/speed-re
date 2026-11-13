// Original UG2 PC v1.2 0x72cc2a, 66 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_007331b8();
int __cdecl FUN_0073326a();
int __cdecl FUN_0073332f();
int __cdecl FUN_0073335e();
#define _DAT_008b4808 (*(undefined4 *)0x008b4808)
#define _DAT_008b4868 (*(undefined4 *)0x008b4868)
#define _DAT_008b480c (*(undefined4 *)0x008b480c)
#define _DAT_008b486c (*(undefined4 *)0x008b486c)
#define _DAT_008b4810 (*(undefined4 *)0x008b4810)
#define _DAT_008b4870 (*(undefined4 *)0x008b4870)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072cc2a(void)

{
  FUN_0073335e();
  _DAT_008b4808 = FUN_0073332f;
  _DAT_008b4868 = 0x2c;
  _DAT_008b480c = FUN_0073326a;
  _DAT_008b486c = 0x30;
  _DAT_008b4810 = FUN_007331b8;
  _DAT_008b4870 = 0x34;
  return;
}


