// Original UG2 PC v1.2 0x5e6360, 300 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


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



int __fastcall FUN_005e6360(int param_1)

{
  int *piVar1;
  int *piVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  
  *(int *)param_1 = param_1;
  *(int *)(param_1 + 4) = param_1;
  iVar6 = param_1 + 8;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0xc) = iVar6;
  iVar6 = param_1 + 0x10;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x14) = iVar6;
  iVar6 = param_1 + 0x18;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x1c) = iVar6;
  iVar6 = param_1 + 0x20;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x24) = iVar6;
  iVar6 = param_1 + 0x28;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x2c) = iVar6;
  iVar6 = param_1 + 0x30;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x34) = iVar6;
  iVar6 = param_1 + 0x38;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x3c) = iVar6;
  iVar6 = param_1 + 0x40;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x44) = iVar6;
  iVar6 = param_1 + 0x6414;
  *(int *)iVar6 = iVar6;
  *(int *)(param_1 + 0x6418) = iVar6;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  puVar4 = (undefined4 *)(param_1 + 0x50);
  for (iVar5 = 0xaf0; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  *(undefined4 *)(param_1 + 0x2c10) = 0;
  puVar4 = (undefined4 *)(param_1 + 0x2c34);
  iVar5 = 0x40;
  do {
    piVar2 = *(int **)(param_1 + 0x6418);
    piVar1 = puVar4 + -8;
    *piVar2 = (int)piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    puVar4[-7] = piVar2;
    piVar2 = *(int **)(param_1 + 0x6418);
    piVar1 = puVar4 + -1;
    *piVar2 = (int)piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    *puVar4 = piVar2;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 6;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    puVar4[7] = puVar3;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 0xd;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    puVar4[0xe] = puVar3;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 0x14;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    puVar4[0x15] = puVar3;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 0x1b;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    *piVar1 = iVar6;
    puVar4[0x1c] = puVar3;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 0x22;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    puVar4[0x23] = puVar3;
    *piVar1 = iVar6;
    puVar3 = *(undefined4 **)(param_1 + 0x6418);
    piVar1 = puVar4 + 0x29;
    *puVar3 = piVar1;
    *(int **)(param_1 + 0x6418) = piVar1;
    puVar4[0x2a] = puVar3;
    puVar4 = puVar4 + 0x38;
    iVar5 = iVar5 + -1;
    *piVar1 = iVar6;
  } while (iVar5 != 0);
  puVar4 = (undefined4 *)(param_1 + 0x641c);
  for (iVar6 = 8; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  return param_1;
}


