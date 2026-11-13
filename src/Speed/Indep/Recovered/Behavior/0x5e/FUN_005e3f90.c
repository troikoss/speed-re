// Original UG2 PC v1.2 0x5e3f90, 96 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int __fastcall FUN_005e3f90(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = param_1 + 8;
  *(int *)param_1 = param_1;
  *(int *)(param_1 + 4) = param_1;
  *(int *)iVar1 = iVar1;
  *(int *)(param_1 + 0xc) = iVar1;
  *(undefined4 *)(param_1 + 0x468c) = 0x32;
  *(undefined4 *)(param_1 + 0x4690) = 0x32;
  *(undefined4 *)(param_1 + 0x467c) = 0;
  *(undefined4 *)(param_1 + 0x4680) = 0;
  *(undefined4 *)(param_1 + 0x4684) = 0;
  *(undefined4 *)(param_1 + 0x4688) = 3;
  puVar2 = (undefined4 *)(param_1 + 0x14);
  for (iVar1 = 0x5dc; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined4 *)(param_1 + 0x4674) = 0;
  *(undefined4 *)(param_1 + 0x4670) = 0;
  *(undefined4 *)(param_1 + 0x10) = 0;
  return param_1;
}


