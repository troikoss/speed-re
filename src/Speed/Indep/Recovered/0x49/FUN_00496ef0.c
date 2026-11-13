// Original UG2 PC v1.2 0x496ef0, 95 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050c900();
int __cdecl FUN_00537be0();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_00496ef0(int param_1,int _edx_unused,int param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = param_2 - (int)param_3;
  iVar2 = 5;
  do {
    iVar1 = *(int *)(iVar3 + (int)param_3);
    if (iVar1 != 0) {
      FUN_00537be0(iVar1,&DAT_00783fe5);
    }
    if (*param_3 != 0) {
      FUN_00537be0(*param_3,&DAT_00783fe5);
    }
    param_3 = param_3 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  FUN_0050c900(*(undefined4 *)(param_2 + 0x10),*(undefined4 *)(param_1 + 0x10));
  return;
}


