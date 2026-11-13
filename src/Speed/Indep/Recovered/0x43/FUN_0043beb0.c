// Original UG2 PC v1.2 0x43beb0, 38 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_0043beb0(int param_1,int _edx_unused,int *param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  
  piVar1 = (int *)*param_2;
  piVar2 = (int *)param_2[1];
  if (piVar1 != param_2) {
    piVar3 = *(int **)(param_1 + 4);
    *piVar2 = param_1;
    piVar1[1] = (int)piVar3;
    *piVar3 = (int)piVar1;
    *(int **)(param_1 + 4) = piVar2;
    *param_2 = (int)param_2;
    param_2[1] = (int)param_2;
  }
  return;
}


