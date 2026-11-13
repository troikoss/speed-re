// Original UG2 PC v1.2 0x5764b0, 21 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043bdf0();
#define DAT_00865134 (*(undefined4 *)0x00865134)

void FUN_005764b0(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = DAT_00865134;
  if (DAT_00865134 != 0) {
    uVar2 = FUN_0043bdf0();
    *(undefined4 *)(iVar1 + 0x10) = uVar2;
  }
  return;
}


