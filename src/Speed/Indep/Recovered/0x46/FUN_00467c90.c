// Original UG2 PC v1.2 0x467c90, 139 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043f920();
int __cdecl FUN_00440540();
#define DAT_0082af34 (*(undefined4 *)0x0082af34)
#define DAT_0082af38 (*(undefined4 *)0x0082af38)

void __fastcall FUN_00467c90(int param_1)

{
  int iVar1;
  
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 0x5c)) {
    do {
      FUN_0043f920(DAT_0082af34,*(undefined4 *)(*(int *)(param_1 + 0x20) + iVar1 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0x20) + iVar1 * 4) = 0;
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(param_1 + 0x5c));
  }
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 0x58)) {
    do {
      FUN_0043f920(DAT_0082af38,*(undefined4 *)(*(int *)(param_1 + 0x2c) + iVar1 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0x2c) + iVar1 * 4) = 0;
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)(param_1 + 0x58));
  }
  if (*(int *)(param_1 + 0x20) != 0) {
    FUN_00440540(*(int *)(param_1 + 0x20));
  }
  if (*(int *)(param_1 + 0x2c) != 0) {
    FUN_00440540(*(int *)(param_1 + 0x2c));
  }
  return;
}


