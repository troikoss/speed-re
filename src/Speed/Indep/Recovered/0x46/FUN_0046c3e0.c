// Original UG2 PC v1.2 0x46c3e0, 70 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0046c130();
#define DAT_0082b620 (*(undefined4 *)0x0082b620)

int FUN_0046c3e0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = (&DAT_0082b620)[param_1];
  if (iVar1 == 0) {
    return -8;
  }
  if (*(char *)(iVar1 + 0x65) != '\0') {
    FUN_0046c130(0);
  }
  *param_2 = *(undefined4 *)(iVar1 + 0x48);
  *param_3 = *(undefined4 *)(iVar1 + 0x4c);
  iVar2 = *(int *)(iVar1 + 0x60);
  if (iVar2 == 0) {
    iVar2 = *(int *)(iVar1 + 0x5c);
  }
  return iVar2;
}


