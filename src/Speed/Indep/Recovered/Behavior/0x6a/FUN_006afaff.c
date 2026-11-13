// Original UG2 PC v1.2 0x6afaff, 578 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 __fastcall FUN_006afaff(int param_1)

{
  int *piVar1;
  bool bVar2;
  bool bVar3;
  int *piVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  int local_24;
  int local_20;
  int *local_1c;
  int *local_18;
  uint local_10;
  uint local_c;
  uint local_8;
  
  piVar1 = *(int **)(*(int *)(param_1 + 0x230) + -4 + *(int *)(param_1 + 0x224) * 4);
  if ((piVar1 != (int *)0x0) && (*piVar1 == 0x10000004)) {
    uVar7 = 0;
    do {
      iVar8 = *(int *)(*(int *)(param_1 + 0x14) + *(int *)(uVar7 + piVar1[4]) * 4);
      if (*(int *)(iVar8 + 4) != *(int *)(param_1 + 0x84)) {
        return 0;
      }
      if (*(int *)(iVar8 + 0x3c) != 0) {
        return 0;
      }
      if ((*(byte *)(*(int *)(*(int *)(param_1 + 0x14) + *(int *)(uVar7 + piVar1[2]) * 4) + 0x3e) &
          0x1f) != 0) {
        return 0;
      }
      uVar7 = uVar7 + 4;
    } while (uVar7 < 0x10);
    local_c = 0;
    local_1c = (int *)piVar1[2];
    local_8 = *(int *)(param_1 + 0x224) - 1;
    bVar3 = true;
    do {
      uVar7 = 0;
      bVar2 = false;
      if (local_8 != 0) {
        local_18 = *(int **)(param_1 + 0x230);
        do {
          piVar4 = (int *)*local_18;
          if ((piVar4 != (int *)0x0) && (*piVar4 != 0)) {
            iVar8 = piVar4[3];
            if (iVar8 != 0) {
              piVar4 = (int *)piVar4[4];
              do {
                if (*piVar4 == *local_1c) {
                  bVar2 = true;
                }
                piVar4 = piVar4 + 1;
                iVar8 = iVar8 + -1;
              } while (iVar8 != 0);
            }
            if (bVar2) break;
          }
          local_18 = local_18 + 1;
          uVar7 = uVar7 + 1;
        } while (uVar7 < local_8);
      }
      if ((*(uint *)(param_1 + 200) & 0xffff) == 0x104) {
        if ((uVar7 < *(uint *)(param_1 + 0x4c)) && (uVar7 != 0)) {
LAB_006afc04:
          bVar3 = false;
        }
      }
      else if (uVar7 < *(uint *)(param_1 + 0x4c)) goto LAB_006afc04;
      if (!bVar2) {
        return 0;
      }
      uVar7 = uVar7 + 1;
      if (uVar7 < local_8) {
        local_18 = (int *)(*(int *)(param_1 + 0x230) + uVar7 * 4);
        local_24 = local_8 - uVar7;
        do {
          piVar4 = (int *)*local_18;
          if ((piVar4 != (int *)0x0) && (*piVar4 != 0)) {
            local_20 = piVar4[3];
            if (local_20 != 0) {
              piVar5 = (int *)piVar4[4];
              do {
                iVar8 = *(int *)(*(int *)(param_1 + 0x14) + *piVar5 * 4);
                if (((*(int *)(iVar8 + 4) == *(int *)(param_1 + 0x88)) &&
                    (*(uint *)(iVar8 + 0x10) == local_c)) && (*(int *)(iVar8 + 0xc) == 0)) {
                  bVar3 = false;
                }
                piVar5 = piVar5 + 1;
                local_20 = local_20 + -1;
              } while (local_20 != 0);
            }
            local_20 = piVar4[1];
            if (local_20 != 0) {
              piVar4 = (int *)piVar4[2];
              do {
                iVar8 = *(int *)(*(int *)(param_1 + 0x14) + *piVar4 * 4);
                if (((*(int *)(iVar8 + 4) == *(int *)(param_1 + 0x88)) &&
                    (*(uint *)(iVar8 + 0x10) == local_c)) && (*(int *)(iVar8 + 0xc) == 0)) {
                  bVar3 = false;
                }
                piVar4 = piVar4 + 1;
                local_20 = local_20 + -1;
              } while (local_20 != 0);
            }
          }
          local_18 = local_18 + 1;
          local_24 = local_24 + -1;
        } while (local_24 != 0);
      }
      local_c = local_c + 1;
      local_1c = local_1c + 1;
    } while (local_c < 4);
    uVar7 = 0;
    if (bVar3) {
      do {
        local_10 = 0;
        if (local_8 != 0) {
          do {
            uVar6 = 0;
            piVar4 = *(int **)(*(int *)(param_1 + 0x230) + local_10 * 4);
            if (((piVar4 != (int *)0x0) && (*piVar4 != 0)) && (piVar4[3] != 0)) {
              do {
                iVar8 = *(int *)(piVar4[4] + uVar6 * 4);
                if (iVar8 == *(int *)(uVar7 + piVar1[2])) {
                  *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x14) + iVar8 * 4) + 0xc) = 0;
                }
                uVar6 = uVar6 + 1;
              } while (uVar6 < (uint)piVar4[3]);
            }
            local_10 = local_10 + 1;
            local_8 = *(int *)(param_1 + 0x224) - 1;
          } while (local_10 < local_8);
        }
        uVar7 = uVar7 + 4;
      } while (uVar7 < 0x10);
      *piVar1 = 0;
    }
  }
  return 0;
}


