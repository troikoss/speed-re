// Original UG2 PC v1.2 0x578fd0, 78 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dc30();


int * __fastcall FUN_00578fd0(int param_1,int _edx_unused,undefined4 param_2,int param_3)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = *(int **)(param_1 + 0x98);
  if (piVar2 != (int *)(param_1 + 0x98)) {
    do {
      iVar1 = FUN_0043dc30(piVar2 + 2,param_2);
      if (iVar1 == 0) {
        if (param_3 == 0) {
          return piVar2;
        }
        param_3 = param_3 + -1;
      }
      piVar2 = (int *)*piVar2;
    } while (piVar2 != (int *)(param_1 + 0x98));
  }
  return (int *)0x0;
}


