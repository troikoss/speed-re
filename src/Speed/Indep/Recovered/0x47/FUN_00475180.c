// Original UG2 PC v1.2 0x475180, 24 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0082ba30 (*(undefined4 *)0x0082ba30)

void __fastcall FUN_00475180(undefined4 param_1)

{
  int iVar1;
  
  if ((DAT_0082ba30 != 0) && (iVar1 = *(int *)(DAT_0082ba30 + 4), iVar1 != 0)) {
    *(undefined1 *)(iVar1 + 4) = 1;
    *(undefined4 *)(iVar1 + 0x54) = param_1;
  }
  return;
}


