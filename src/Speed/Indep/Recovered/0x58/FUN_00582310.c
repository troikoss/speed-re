// Original UG2 PC v1.2 0x582310, 40 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_0043dc00();
#define DAT_00869888 (*(undefined1 *)0x00869888)
#define DAT_0086993c (*(undefined1 *)0x0086993c)

void __stdcall FUN_00582310(int param_1)

{
  if ((param_1 != 0) && (DAT_00869888 != '\0')) {
    FUN_0043dc00(&DAT_0086993c,param_1,0x3ff);
  }
  return;
}


