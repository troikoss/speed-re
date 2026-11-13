// Original UG2 PC v1.2 0x4a99f0, 43 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0057d670();
int __cdecl FUN_0057e480();
#define DAT_008650d4 (*(undefined4 *)0x008650d4)

void __stdcall FUN_004a99f0(undefined4 param_1)

{
  if (DAT_008650d4 != 0) {
    FUN_0057d670(0xc,4,8);
  }
  FUN_0057e480(param_1,0x10,8);
  return;
}


