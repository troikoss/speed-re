// Original UG2 PC v1.2 0x45aa30, 208 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __stdcall FUN_0045aa30(int param_1,int *param_2,int *param_3,int *param_4)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  uVar3 = *param_2 * 0x1000000 + *param_3;
  iVar5 = 0;
  iVar4 = *(int *)(param_1 + 0x54) + -1;
  iVar2 = iVar4 >> 1;
  uVar1 = *(uint *)(iVar2 * 0x10 + 0x58 + param_1);
  while (((uVar1 != uVar3 && (iVar2 != iVar5)) && (iVar2 != iVar4))) {
    if (uVar1 < uVar3) {
      iVar5 = iVar2;
    }
    else {
      iVar4 = iVar2;
    }
    iVar2 = iVar4 + iVar5 >> 1;
    uVar1 = *(uint *)(iVar2 * 0x10 + 0x58 + param_1);
  }
  if (*(uint *)(iVar2 * 0x10 + 0x58 + param_1) != uVar3) {
    if (*(uint *)(iVar5 * 0x10 + 0x58 + param_1) == uVar3) {
      iVar2 = iVar5;
    }
    else {
      if (*(uint *)(iVar4 * 0x10 + 0x58 + param_1) != uVar3) goto LAB_0045aaba;
      iVar2 = iVar4;
    }
  }
  iVar5 = iVar2 * 0x10 + param_1;
  if (*(int *)(iVar2 * 0x10 + 0x5c + param_1) != 0) {
    *param_4 = *(int *)((iVar2 + 6) * 0x10 + param_1) + param_1;
    param_4[1] = *(int *)(iVar5 + 0x5c);
    param_4[2] = *(int *)(iVar5 + 100);
    return 0;
  }
LAB_0045aaba:
  *param_4 = 0;
  param_4[2] = 0;
  param_4[1] = -1;
  return 0xffffffff;
}


