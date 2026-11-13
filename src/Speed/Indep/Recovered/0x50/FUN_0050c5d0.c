// Original UG2 PC v1.2 0x50c5d0, 33 bytes. Auto-matched: byte-identical to speed2.exe under
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
int FUN_004cb480();
#define DAT_007f91d4 (*(undefined4 *)0x007f91d4)

void FUN_0050c5d0(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = &DAT_007f91d4 + param_1;
  iVar2 = 0x15;
  do {
    *puVar1 = 0;
    puVar1 = puVar1 + 7;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  FUN_004cb480();
  return;
}


