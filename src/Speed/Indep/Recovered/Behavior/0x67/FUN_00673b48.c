// Original UG2 PC v1.2 0x673b48, 354 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



undefined4 __fastcall FUN_00673b48(void *_this,int _edx_unused,uint *param_1,uint param_2,uint param_3,uint *param_4,uint *param_5,
            uint param_6,int *param_7,undefined4 param_8,int *param_9)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint local_1c;
  int local_c;
  int local_8;
  
  uVar7 = param_2;
  iVar6 = param_2 * param_3;
  if (iVar6 != 0) {
    if ((param_1 == (uint *)0x2) && (((uint)param_4 & 0x400) == 0)) {
      param_2 = param_3;
      bVar3 = true;
      param_3 = uVar7;
    }
    else {
      bVar3 = false;
    }
    uVar7 = param_3 + 3 >> 2;
    iVar8 = uVar7 * param_2;
    if (param_6 < *(uint *)(*(int *)((int)_this + 8) + 4)) {
      iVar5 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x10) + param_6 * 4);
    }
    else {
      iVar5 = 0;
    }
    if (iVar5 == 0) {
      return 0x80004005;
    }
    iVar2 = *(int *)(iVar5 + 0x10);
    iVar1 = iVar2 + iVar8;
    *(int *)(iVar5 + 0x10) = iVar1;
    if ((*(byte *)(iVar5 + 4) & 0x40) != 0) {
      *(int *)(iVar5 + 8) = iVar1;
    }
    if (param_2 != 0) {
      local_c = 0;
      local_8 = 0;
      param_4 = param_5;
      local_1c = param_2;
      do {
        uVar9 = 0;
        if (param_3 != 0) {
          param_1 = param_4;
          do {
            if (bVar3) {
              uVar4 = *param_1;
            }
            else {
              uVar4 = param_5[local_8 + uVar9];
            }
            if (uVar4 < *(uint *)(*(int *)((int)_this + 8) + 8)) {
              iVar5 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) + uVar4 * 4);
            }
            else {
              iVar5 = 0;
            }
            if (iVar5 != 0) {
              *(uint *)(iVar5 + 4) = param_6;
              iVar1 = (uVar9 >> 2) + local_c + iVar2;
              *(undefined4 *)(iVar5 + 0x68) = param_8;
              *(int *)(iVar5 + 0xc) = iVar1;
              *(uint *)(iVar5 + 0x10) = uVar9 & 3;
              *(uint *)(iVar5 + 0x70) = *param_7 + (iVar1 - iVar2) * 4 + (uVar9 & 3);
            }
            param_1 = param_1 + param_2;
            uVar9 = uVar9 + 1;
          } while (uVar9 < param_3);
        }
        param_4 = param_4 + 1;
        local_8 = local_8 + param_3;
        local_c = local_c + uVar7;
        local_1c = local_1c - 1;
      } while (local_1c != 0);
    }
    *param_7 = *param_7 + iVar8 * 4;
  }
  if (param_9 != (int *)0x0) {
    *param_9 = iVar6;
  }
  return 0;
}


