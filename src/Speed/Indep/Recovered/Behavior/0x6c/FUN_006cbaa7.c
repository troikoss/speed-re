// Original UG2 PC v1.2 0x6cbaa7, 467 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __fastcall FUN_006cbaa7(void *_this,int _edx_unused,uint param_1,uint *param_2,int *param_3,int param_4)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  uint uVar7;
  uint uVar8;
  int *piVar9;
  uint local_18;
  uint local_14;
  int local_10;
  
  uVar7 = *param_2 & 0xfffff;
  if (uVar7 != 0) {
    uVar3 = param_2[1] / uVar7;
    if (uVar3 != 0) {
      local_10 = 0;
      piVar9 = param_3;
      local_18 = uVar3;
      do {
        iVar5 = *(int *)(*(int *)((int)_this + 0x14) + *(int *)(local_10 + param_2[2]) * 4);
        uVar4 = *(uint *)(iVar5 + 0x54);
        uVar8 = *(uint *)(iVar5 + 0x48);
        if (uVar8 == 0xffffffff) {
LAB_006cbc3b:
          *piVar9 = 0;
        }
        else {
          puVar1 = *(uint **)(*(int *)((int)_this + 0x18) + uVar8 * 4);
          *piVar9 = (int)puVar1;
          if (uVar7 != 0) {
            iVar5 = local_10;
            local_14 = uVar7;
            do {
              iVar2 = *(int *)(*(int *)((int)_this + 0x14) + *(int *)(iVar5 + param_2[2]) * 4);
              if (*(int *)((int)_this + 0x88) != *(int *)(iVar2 + 4)) {
                *piVar9 = 0;
              }
              if (*(int *)(iVar2 + 0x3c) != 0) {
                *piVar9 = 0;
              }
              if ((uVar8 < *(uint *)(iVar2 + 0x50)) && (*(uint *)(iVar2 + 0x50) < uVar4)) {
                *piVar9 = 0;
              }
              if (*(int *)(iVar2 + 0x54) != *(int *)(iVar2 + 0x58)) {
                *piVar9 = 0;
              }
              iVar5 = iVar5 + 4;
              local_14 = local_14 - 1;
            } while (local_14 != 0);
          }
          if (*piVar9 != 0) {
            if (param_1 == *puVar1) {
              if (uVar7 != 0) {
                iVar5 = local_10;
                local_14 = uVar7;
                do {
                  if (*(uint *)(*(int *)(*(int *)((int)_this + 0x14) +
                                        *(int *)(iVar5 + param_2[2]) * 4) + 0x48) != uVar8) {
                    *piVar9 = 0;
                  }
                  iVar5 = iVar5 + 4;
                  local_14 = local_14 - 1;
                } while (local_14 != 0);
              }
              if (*piVar9 == 0) goto LAB_006cbc3e;
              if ((puVar1[3] == (*puVar1 & 0xfffff)) && (puVar1[1] <= (*puVar1 & 0xfffff) * param_4)
                 ) {
                uVar4 = param_2[1];
                uVar8 = 0;
                if (uVar4 != 0) {
                  piVar6 = (int *)param_2[2];
                  do {
                    if (*piVar6 == *(int *)puVar1[4]) break;
                    uVar8 = uVar8 + 1;
                    piVar6 = piVar6 + 1;
                  } while (uVar8 < uVar4);
                }
                if (uVar8 != uVar4) {
                  if (1 < puVar1[3]) {
                    if ((puVar1[3] != uVar7) || (uVar4 < uVar8 + uVar7)) {
                      *piVar9 = 0;
                    }
                    uVar4 = 0;
                    if (uVar7 != 0) {
                      iVar5 = uVar8 << 2;
                      do {
                        if (*(int *)(puVar1[4] + uVar4 * 4) != *(int *)(iVar5 + param_2[2])) {
                          *piVar9 = 0;
                        }
                        uVar4 = uVar4 + 1;
                        iVar5 = iVar5 + 4;
                      } while (uVar4 < uVar7);
                    }
                  }
                  goto LAB_006cbc3e;
                }
              }
            }
            goto LAB_006cbc3b;
          }
        }
LAB_006cbc3e:
        local_10 = local_10 + uVar7 * 4;
        piVar9 = piVar9 + 1;
        local_18 = local_18 - 1;
      } while (local_18 != 0);
    }
    uVar7 = 0;
    if (uVar3 != 0) {
      do {
        if (param_3[uVar7] != 0) {
          return 0;
        }
        uVar7 = uVar7 + 1;
      } while (uVar7 < uVar3);
    }
  }
  return 1;
}


