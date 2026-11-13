// Original UG2 PC v1.2 0x6c96c6, 279 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_00786e68 (*(undefined4 *)0x00786e68)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_006c96c6(void *_this,int _edx_unused,int param_1,int *param_2,int *param_3)

{
  int *piVar1;
  int iVar2;
  uint *puVar3;
  uint uVar4;
  int *piVar5;
  int *piVar6;
  uint local_8;
  
  iVar2 = *(int *)(*(int *)((int)_this + 0x14) + param_1 * 4);
  if ((*(int *)(iVar2 + 0x58) == *(int *)(iVar2 + 0x54)) &&
     (uVar4 = 0xffffffff, *(int *)(iVar2 + 0x48) != -1)) {
    puVar3 = *(uint **)(*(int *)((int)_this + 0x18) + *(int *)(iVar2 + 0x48) * 4);
    if ((*puVar3 & 0xfff00000) == 0x20400000) {
      local_8 = 0;
      if (puVar3[3] != 0) {
        piVar6 = (int *)puVar3[4];
        do {
          if (*piVar6 == param_1) {
            uVar4 = local_8;
          }
          local_8 = local_8 + 1;
          piVar6 = piVar6 + 1;
        } while (local_8 < puVar3[3]);
        if (uVar4 != 0xffffffff) {
          piVar6 = (int *)(puVar3[2] + uVar4 * 4);
          piVar1 = (int *)(puVar3[2] + (uVar4 + (*puVar3 & 0xfffff)) * 4);
          if ((*(byte *)(*(int *)(*(int *)((int)_this + 0x10) +
                                 *(int *)(*(int *)(*(int *)((int)_this + 0x14) + *piVar1 * 4) + 4) *
                                 4) + 5) & 1) == 0) {
            if ((*(byte *)(*(int *)(*(int *)((int)_this + 0x10) +
                                   *(int *)(*(int *)(*(int *)((int)_this + 0x14) + *piVar6 * 4) + 4)
                                   * 4) + 5) & 1) == 0) {
              return 1;
            }
            piVar5 = piVar1;
          }
          else {
            piVar5 = piVar6;
            piVar6 = piVar1;
          }
          if ((*(double *)(*(int *)(*(int *)((int)_this + 0x14) + *piVar6 * 4) + 0x20) ==
               _DAT_00786e68) &&
             ((*(uint *)(*(int *)(*(int *)((int)_this + 0x14) + *piVar5 * 4) + 0x3c) & 0x1f0000) ==
              0x80000)) {
            *param_3 = *piVar5;
            *param_2 = (int)puVar3;
            return 0;
          }
        }
      }
    }
  }
  return 1;
}


