// Original UG2 PC v1.2 0x6af7d9, 300 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



undefined4 __fastcall FUN_006af7d9(int param_1)

{
  byte *pbVar1;
  int *piVar2;
  uint *puVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint local_c;
  uint local_8;
  
  if (((**(uint **)(param_1 + 0x100) & 0xfff00000) == 0x10000000) &&
     ((*(uint *)(param_1 + 200) & 0xffff) < 0x104)) {
    puVar3 = *(uint **)(param_1 + 0x100);
    uVar4 = *puVar3;
    iVar6 = *(int *)(*(int *)(param_1 + 0x14) + *(int *)puVar3[2] * 4);
    iVar5 = *(int *)(*(int *)(param_1 + 0x14) + *(int *)puVar3[4] * 4);
    if ((*(int *)(iVar6 + 0x3c) == 0) &&
       ((((*(byte *)(iVar5 + 0x3d) & 2) != 0 && (*(char *)(iVar6 + 0x6c) == '\x03')) &&
        (*(int *)(iVar5 + 4) == *(int *)(param_1 + 0x88))))) {
      local_c = 0;
      if ((uVar4 & 0xfffff) != 0) {
        do {
          iVar6 = local_c * 4;
          pbVar1 = (byte *)(*(int *)(*(int *)(param_1 + 0x14) +
                                    *(int *)(iVar6 + *(int *)(*(int *)(param_1 + 0x100) + 8)) * 4) +
                           0x3d);
          *pbVar1 = *pbVar1 | 2;
          for (uVar7 = *(uint *)(param_1 + 0xfc); uVar7 < *(uint *)(param_1 + 0xc);
              uVar7 = uVar7 + 1) {
            iVar5 = *(int *)(*(int *)(param_1 + 0x18) + uVar7 * 4);
            local_8 = 0;
            if (*(int *)(iVar5 + 4) != 0) {
              do {
                piVar2 = (int *)(*(int *)(iVar5 + 8) + local_8 * 4);
                if (*piVar2 == *(int *)(iVar6 + *(int *)(*(int *)(param_1 + 0x100) + 0x10))) {
                  *piVar2 = *(int *)(iVar6 + *(int *)(*(int *)(param_1 + 0x100) + 8));
                }
                local_8 = local_8 + 1;
              } while (local_8 < *(uint *)(iVar5 + 4));
            }
          }
          local_c = local_c + 1;
        } while (local_c < (uVar4 & 0xfffff));
      }
      **(undefined4 **)(param_1 + 0x100) = 0;
    }
  }
  return 0;
}


