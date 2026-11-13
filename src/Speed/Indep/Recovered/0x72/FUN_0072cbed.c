// Original UG2 PC v1.2 0x72cbed, 61 bytes. Auto-matched: byte-identical to speed2.exe under
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
#define DAT_008b4814 (*(undefined4 *)0x008b4814)
#define DAT_008b4874 (*(undefined4 *)0x008b4874)
#define DAT_008b4818 (*(undefined4 *)0x008b4818)
#define DAT_008b4878 (*(undefined4 *)0x008b4878)
#define DAT_008b481c (*(undefined4 *)0x008b481c)
#define DAT_008b487c (*(undefined4 *)0x008b487c)

void FUN_0072cbed(void)

{
  DAT_008b4814 = FUN_0073332f;
  DAT_008b4874 = 0x2c;
  DAT_008b4818 = FUN_0073326a;
  DAT_008b4878 = 0x30;
  DAT_008b481c = FUN_007331b8;
  DAT_008b487c = 0x34;
  return;
}


