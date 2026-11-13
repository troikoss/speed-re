// Original UG2 PC v1.2 0x752280, 251 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_00751b10();
int __cdecl FUN_007520f0();


int FUN_00752280(int param_1,undefined4 *param_2,uint param_3,byte param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  
  if (*(int *)(param_1 + 0x8c) != 3) {
    return -2;
  }
  iVar3 = *(int *)(param_1 + 0xac);
  iVar2 = *(int *)(param_1 + 0xb4);
  iVar1 = *(int *)(param_1 + 0xb0);
  if ((iVar2 + iVar3) % iVar1 == *(int *)(param_1 + 0xb8)) {
    return 0;
  }
  if (iVar3 + -0xb < (int)param_3) {
    return -6;
  }
  if (param_3 == 0) {
    return (((iVar1 - *(int *)(param_1 + 0xb8)) + iVar2) % iVar1) / iVar3 + 1;
  }
  iVar2 = *(int *)(param_1 + 0xbc) + iVar2;
  *(uint *)(iVar2 + 4) = param_3;
  puVar5 = (undefined4 *)(iVar2 + 9);
  for (uVar4 = param_3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar5 = *param_2;
    param_2 = param_2 + 1;
    puVar5 = puVar5 + 1;
  }
  for (uVar4 = param_3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined1 *)puVar5 = *(undefined1 *)param_2;
    param_2 = (undefined4 *)((int)param_2 + 1);
    puVar5 = (undefined4 *)((int)puVar5 + 1);
  }
  if ((param_4 & 1) != 0) {
    *(char *)(iVar2 + 8) = *(char *)(param_1 + 200) + '@';
    *(uint *)(param_1 + 200) = *(int *)(param_1 + 200) + 1U & 0x3f;
    FUN_00751b10();
    return 1;
  }
  *(char *)(iVar2 + 8) = *(char *)(param_1 + 0xd0) + -0x80;
  *(uint *)(param_1 + 0xd0) = *(int *)(param_1 + 0xd0) + 1U & 0x3f;
  iVar3 = FUN_007520f0();
  if (iVar3 < 1) {
    iVar3 = 1;
  }
  return iVar3;
}


