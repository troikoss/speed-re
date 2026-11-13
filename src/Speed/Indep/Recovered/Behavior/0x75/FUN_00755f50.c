// Original UG2 PC v1.2 0x755f50, 321 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_007555c0();


uint FUN_00755f50(undefined4 *param_1,undefined4 *param_2,uint param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  iVar1 = param_1[0x42];
  if (iVar1 == 7) {
    return 0xfffffffe;
  }
  if ((iVar1 != 5) && (iVar1 != 6)) {
    return 0xfffffffd;
  }
  if (param_1[0x43] == 1) {
    return 0xfffffffc;
  }
  if ((int)param_4 < 1) {
    return 0;
  }
  if ((int)param_3 < 1) {
    param_3 = 1;
  }
  if ((int)param_4 < (int)param_3) {
    param_4 = param_3;
  }
  uVar3 = param_1[0x4a];
  if ((int)uVar3 < (int)param_3) {
    param_3 = uVar3;
  }
  if ((int)uVar3 < (int)param_4) {
    param_4 = uVar3;
  }
  iVar1 = param_1[0x4b];
  if ((int)(uVar3 - iVar1) < (int)param_3) {
    iVar2 = param_1[0x4c];
    puVar5 = (undefined4 *)((int)param_1[0x49] + iVar1);
    puVar6 = (undefined4 *)param_1[0x49];
    for (uVar3 = (uint)(iVar2 - iVar1) >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
      *puVar6 = *puVar5;
      puVar5 = puVar5 + 1;
      puVar6 = puVar6 + 1;
    }
    for (uVar3 = iVar2 - iVar1 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
    param_1[0x4c] = param_1[0x4c] - param_1[0x4b];
    param_1[0x4b] = 0;
    FUN_007555c0(param_1);
  }
  uVar3 = param_1[0x4c] - param_1[0x4b];
  if ((int)param_4 < (int)uVar3) {
    uVar3 = param_4;
  }
  if ((uVar3 == 0) && (param_1[0x42] == 6)) {
    return 0xffffffff;
  }
  if (((int)uVar3 < (int)param_3) && (param_1[0x4d] != param_1[0x48])) {
    return 0;
  }
  if (param_2 != (undefined4 *)0x0) {
    puVar5 = (undefined4 *)(param_1[0x49] + param_1[0x4b]);
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *param_2 = *puVar5;
      puVar5 = puVar5 + 1;
      param_2 = param_2 + 1;
    }
    for (uVar4 = uVar3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined1 *)param_2 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      param_2 = (undefined4 *)((int)param_2 + 1);
    }
  }
  param_1[0x4b] = param_1[0x4b] + uVar3;
  return uVar3;
}


