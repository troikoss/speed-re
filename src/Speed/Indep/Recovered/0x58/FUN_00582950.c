// Original UG2 PC v1.2 0x582950, 46 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dc00();
int __cdecl FUN_0043ddf0();
#define DAT_00869d40 (*(undefined4 *)0x00869d40)

undefined * FUN_00582950(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043ddf0(param_1 + 4,0x2e);
  if (iVar1 == 0) {
    return (undefined *)0x0;
  }
  FUN_0043dc00(&DAT_00869d40,iVar1 + 1,0x20);
  return &DAT_00869d40;
}


