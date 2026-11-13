// Original UG2 PC v1.2 0x5bf940, 104 bytes. Auto-matched: byte-identical to speed2.exe under
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



undefined4 __fastcall FUN_005bf940(int param_1)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  
  if ((*(char *)(param_1 + 0x22) == '\0') && (*(char *)(param_1 + 0x23) == '\0')) {
    bVar1 = false;
  }
  else {
    bVar1 = true;
  }
  if ((*(char *)(param_1 + 0x25) == '\0') && (*(char *)(param_1 + 0x26) == '\0')) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  if ((*(char *)(param_1 + 0x28) == '\0') && (*(char *)(param_1 + 0x29) == '\0')) {
    bVar3 = false;
  }
  else {
    bVar3 = true;
  }
  if ((((bVar1) || (bVar2)) || (*(int *)(param_1 + 0x1c) != 0)) ||
     (((bVar3 || (*(int *)(param_1 + 0x14) != 0)) || (*(int *)(param_1 + 0x18) != 0)))) {
    return 1;
  }
  return 0;
}


