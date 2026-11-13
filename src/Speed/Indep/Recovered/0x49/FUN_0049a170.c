// Original UG2 PC v1.2 0x49a170, 74 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00537c00();
#define DAT_0078f9d4 (*(undefined4 *)0x0078f9d4)

void __fastcall FUN_0049a170(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (*(int *)(param_1 + 0xe4) != -1) {
    puVar2 = (undefined4 *)(param_1 + 200);
    iVar1 = 3;
    do {
      FUN_00537c00(*(undefined4 *)(param_1 + 4),*puVar2,&DAT_0078f9d4,1);
      puVar2 = puVar2 + 1;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
    *(undefined4 *)(param_1 + 0xe4) = 0xffffffff;
  }
  return;
}


