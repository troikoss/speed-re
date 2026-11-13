// Original UG2 PC v1.2 0x70322d, 176 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void FUN_0070322d(int param_1,uint *param_2)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  
  puVar6 = (uint *)(param_1 + 0x40);
  for (iVar2 = 0xe; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar6 = *param_2;
    param_2 = param_2 + 1;
    puVar6 = puVar6 + 1;
  }
  uVar1 = *(uint *)(param_1 + 0x44);
  iVar2 = *(int *)(param_1 + 0x48);
  uVar5 = *(uint *)(param_1 + 0x40) >> 3;
  *(uint *)(param_1 + 0x94) = uVar1 >> 3;
  uVar3 = (uVar1 >> 3) * uVar5;
  *(undefined4 *)(param_1 + 0x9c) = *(undefined4 *)(param_1 + 0x4c);
  *(uint *)(param_1 + 0x84) = uVar3;
  *(uint *)(param_1 + 0x8c) = uVar3 + (uVar3 >> 2) * 2;
  *(uint *)(param_1 + 0x90) = uVar5;
  uVar5 = iVar2 + uVar5 * -8;
  uVar4 = uVar5 >> 1;
  *(int *)(param_1 + 0x98) = iVar2;
  *(uint *)(param_1 + 0x78) = (iVar2 + 1) * uVar4;
  iVar2 = (uVar1 + uVar4 * 2) * iVar2;
  *(uint *)(param_1 + 0x88) = uVar3 >> 2;
  uVar5 = uVar5 >> 2;
  *(uint *)(param_1 + 0x7c) = (*(int *)(param_1 + 0x4c) + 1) * uVar5 + iVar2;
  *(uint *)(param_1 + 0xb4) = uVar4;
  *(uint *)(param_1 + 0x80) =
       uVar5 * *(int *)(param_1 + 0x4c) + ((uVar1 >> 1) + uVar4) * *(int *)(param_1 + 0x4c) + uVar5
       + iVar2;
  return;
}


