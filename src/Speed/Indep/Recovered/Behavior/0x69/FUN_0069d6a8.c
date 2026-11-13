// Original UG2 PC v1.2 0x69d6a8, 365 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 __fastcall FUN_0069d6a8(void *_this,int _edx_unused,uint param_1,double *param_2,double *param_3)

{
  int iVar1;
  uint *puVar2;
  uint uVar3;
  double dVar4;
  undefined4 uVar5;
  uint *puVar6;
  int iVar7;
  undefined4 *puVar8;
  uint uVar9;
  uint uVar10;
  int *local_8;
  
  iVar7 = *(int *)(*(int *)((int)_this + 0x14) + param_1 * 4);
  iVar1 = *(int *)(iVar7 + 0x48);
  uVar5 = 0xffffffff;
  if ((iVar1 != -1) && (*(int *)(iVar7 + 8) == -1)) {
    puVar2 = *(uint **)(*(int *)((int)_this + 0x18) + iVar1 * 4);
    uVar9 = *puVar2 & 0xfff00000;
    if (uVar9 == 0x10100000) {
      uVar9 = 0;
      if (puVar2[3] != 0) {
        puVar6 = (uint *)puVar2[4];
        do {
          if (*puVar6 == param_1) break;
          uVar9 = uVar9 + 1;
          puVar6 = puVar6 + 1;
        } while (uVar9 < puVar2[3]);
      }
      if (param_2 != (double *)0x0) {
        *param_2 = -*param_2;
      }
      uVar5 = *(undefined4 *)(puVar2[2] + uVar9 * 4);
    }
    else if ((uVar9 == 0x20500000) || (uVar9 == 0x20400000)) {
      uVar10 = 0;
      if (puVar2[3] != 0) {
        puVar6 = (uint *)puVar2[4];
        do {
          if (*puVar6 == param_1) break;
          uVar10 = uVar10 + 1;
          puVar6 = puVar6 + 1;
        } while (uVar10 < puVar2[3]);
      }
      uVar3 = puVar2[3];
      param_1 = 0;
      iVar7 = uVar10 * 4;
      puVar8 = (undefined4 *)(iVar7 + puVar2[2]);
      local_8 = (int *)(puVar2[2] + (uVar3 + uVar10) * 4);
      do {
        uVar5 = *puVar8;
        iVar1 = *(int *)(*(int *)((int)_this + 0x14) + *local_8 * 4);
        if (((*(byte *)(*(int *)(*(int *)((int)_this + 0x10) + *(int *)(iVar1 + 4) * 4) + 5) & 1) !=
             0) && (*(int *)(iVar1 + 8) == -1)) {
          if (param_2 != (double *)0x0) {
            dVar4 = *(double *)(iVar1 + 0x20) * *param_2;
            if (uVar9 != 0x20500000) {
              dVar4 = dVar4 + *param_3;
              param_2 = param_3;
            }
            *param_2 = dVar4;
          }
          return uVar5;
        }
        param_1 = param_1 + 1;
        puVar8 = puVar8 + uVar3;
        local_8 = local_8 + -uVar3;
      } while (param_1 < 2);
      if ((uVar9 == 0x20400000) &&
         (*(int *)(puVar2[2] + iVar7) == *(int *)(puVar2[2] + (puVar2[3] + uVar10) * 4))) {
        if (param_2 != (double *)0x0) {
          *param_2 = *param_2 + *param_2;
        }
        uVar5 = *(undefined4 *)(iVar7 + puVar2[2]);
      }
      else {
        uVar5 = 0xffffffff;
      }
    }
  }
  return uVar5;
}


