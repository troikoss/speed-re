// Original UG2 PC v1.2 0x69cc9a, 360 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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



undefined4 __fastcall FUN_0069cc9a(void *_this,int _edx_unused,uint param_1)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint local_14;
  uint local_10;
  int *local_8;
  
  iVar5 = param_1 * 4;
  local_8 = (int *)(*(int *)((int)_this + 0x18) + iVar5);
  iVar7 = *local_8;
  uVar1 = *(uint *)(iVar7 + 0x2c);
  uVar2 = param_1;
  if ((uVar1 & 1) == 0) {
    return 0;
  }
  while ((uVar2 != 0 && (local_8 = local_8 + -1, uVar1 < *(uint *)(*local_8 + 0x2c)))) {
    uVar2 = uVar2 - 1;
  }
  local_10 = param_1 - uVar2 >> 1;
  if (local_10 != 0) {
    local_8 = (int *)(uVar2 << 2);
    iVar6 = iVar5;
    do {
      puVar4 = (undefined4 *)((int)local_8 + *(int *)((int)_this + 0x18));
      uVar3 = *puVar4;
      *puVar4 = *(undefined4 *)(iVar6 + -4 + *(int *)((int)_this + 0x18));
      *(undefined4 *)(iVar6 + -4 + *(int *)((int)_this + 0x18)) = uVar3;
      local_8 = (int *)((int)local_8 + 4);
      iVar6 = iVar6 + -4;
      local_10 = local_10 - 1;
    } while (local_10 != 0);
  }
  local_14 = uVar2;
  if (uVar2 != 0) {
    uVar1 = *(uint *)(iVar7 + 0x2c);
    local_8 = (int *)(*(int *)((int)_this + 0x18) + -4 + uVar2 * 4);
    if (*(uint *)(*local_8 + 0x2c) < uVar1) {
      local_14 = uVar2 - 1;
      goto LAB_0069cda9;
    }
    do {
      if (uVar1 != *(uint *)(*local_8 + 0x2c)) break;
      local_8 = local_8 + -1;
      local_14 = local_14 - 1;
    } while (local_14 != 0);
  }
  local_8 = (int *)(uVar2 - local_14 >> 1);
  if (local_8 != (int *)0x0) {
    local_10 = local_14 << 2;
    iVar7 = uVar2 << 2;
    do {
      puVar4 = (undefined4 *)(local_10 + *(int *)((int)_this + 0x18));
      uVar3 = *puVar4;
      *puVar4 = *(undefined4 *)(iVar7 + -4 + *(int *)((int)_this + 0x18));
      *(undefined4 *)(iVar7 + -4 + *(int *)((int)_this + 0x18)) = uVar3;
      local_10 = local_10 + 4;
      iVar7 = iVar7 + -4;
      local_8 = (int *)((int)local_8 - 1);
    } while (local_8 != (int *)0x0);
  }
LAB_0069cda9:
  if (local_14 == uVar2) {
    uVar3 = 0;
  }
  else {
    param_1 = param_1 - local_14 >> 1;
    if (param_1 != 0) {
      iVar6 = local_14 << 2;
      iVar7 = iVar5;
      do {
        puVar4 = (undefined4 *)(iVar6 + *(int *)((int)_this + 0x18));
        uVar3 = *puVar4;
        *puVar4 = *(undefined4 *)(iVar7 + -4 + *(int *)((int)_this + 0x18));
        *(undefined4 *)(iVar7 + -4 + *(int *)((int)_this + 0x18)) = uVar3;
        iVar6 = iVar6 + 4;
        iVar7 = iVar7 + -4;
        param_1 = param_1 - 1;
      } while (param_1 != 0);
    }
    puVar4 = (undefined4 *)(*(int *)((int)_this + 0x18) + iVar5);
    uVar3 = puVar4[-1];
    puVar4[-1] = *puVar4;
    *(undefined4 *)(iVar5 + *(int *)((int)_this + 0x18)) = uVar3;
    uVar3 = 1;
  }
  return uVar3;
}


