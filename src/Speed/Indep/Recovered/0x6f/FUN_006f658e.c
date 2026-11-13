// Original UG2 PC v1.2 0x6f658e, 21 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (CC8.0 /Ox /Ob0 /GS- dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_006f630c();
#define DAT_008ad4e8 (*(undefined4 *)0x008ad4e8)

void FUN_006f658e(void)

{
  if (DAT_008ad4e8 != 0) {
    FUN_006f630c(0x8ad4b0);
  }
  return;
}


