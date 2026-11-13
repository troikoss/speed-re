// Original UG2 PC v1.2 0x500b60, 28 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_00850078 (*(undefined4 *)0x00850078)
#define DAT_0089e7b0 (*(undefined4 *)0x0089e7b0)

undefined4 FUN_00500b60(void)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  if ((DAT_00850078 != 1) || (DAT_0089e7b0 != 0)) {
    uVar1 = 0;
  }
  return uVar1;
}


