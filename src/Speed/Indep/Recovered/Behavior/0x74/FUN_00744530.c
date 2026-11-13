// Original UG2 PC v1.2 0x744530, 174 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int ptr;

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
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
int __cdecl FUN_00440540();
int __cdecl FUN_0073d710();
int __cdecl FUN_0073d890();
int __cdecl FUN_0073da90();
int __cdecl FUN_00754d70();
int __cdecl FUN_00754e10();


void FUN_00744530(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  void *ptr;
  
  if (((-1 < param_2) && (param_2 < 0x24)) && (param_3 != 0)) {
    piVar1 = (int *)FUN_0073da90(param_3);
    iVar2 = FUN_00754e10(piVar1);
    while (iVar2 != 0) {
      FUN_0073d890(param_3,iVar2);
      FUN_00440540(ptr);
      iVar2 = FUN_00754e10(piVar1);
    }
    FUN_00754d70((int)piVar1);
    FUN_0073d710(param_3);
    iVar2 = param_1 + param_2 * 0x14;
    if (*(int *)(param_1 + 0x1164 + param_2 * 0x14) == param_3) {
      *(undefined4 *)(iVar2 + 0x1160) = 0;
      *(undefined4 *)(iVar2 + 0x1164) = 0;
    }
    *(undefined4 *)(iVar2 + 0x1168) = 0;
    *(undefined4 *)(param_1 + (param_2 * 5 + 0x45b) * 4) = 0;
  }
  return;
}


