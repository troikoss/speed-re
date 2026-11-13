// Original UG2 PC v1.2 0x5dc410, 79 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005dc360();
#define DAT_00883e5c (*(undefined4 *)0x00883e5c)
#define DAT_00883e60 (*(undefined4 *)0x00883e60)

void FUN_005dc410(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (((DAT_00883e5c != 0) && (DAT_00883e60 != 0)) && (iVar1 = 0, 0 < *(int *)(param_1 + 0x2e0))) {
    puVar2 = (undefined4 *)(param_1 + 0x3c);
    do {
      FUN_005dc360(param_2,*puVar2);
      iVar1 = iVar1 + 1;
      puVar2 = puVar2 + 1;
    } while (iVar1 < *(int *)(param_1 + 0x2e0));
  }
  return;
}


