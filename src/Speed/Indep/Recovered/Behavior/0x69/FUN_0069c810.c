// Original UG2 PC v1.2 0x69c810, 229 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



undefined4 __fastcall FUN_0069c810(void *_this,int _edx_unused,uint *param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint local_14;
  uint local_c;
  int local_8;
  
  if ((*param_1 & 0xfff00000) != 0) {
    uVar3 = *param_1 & 0xfffff;
    local_14 = param_1[1] / uVar3;
    if (local_14 != 0) {
      local_8 = 0;
      do {
        if (uVar3 != 0) {
          iVar4 = local_8;
          local_c = uVar3;
          do {
            iVar1 = *(int *)(*(int *)((int)_this + 0x14) + *(int *)(iVar4 + param_1[2]) * 4);
            if ((*(int *)(iVar1 + 0x48) != -1) && (*(int *)(iVar1 + 8) == -1)) {
              iVar2 = *(int *)(*(int *)((int)_this + 0x18) + *(int *)(iVar1 + 0x48) * 4);
              *(uint *)(iVar1 + 0x40) =
                   *(uint *)(iVar1 + 0x40) | 1 << ((byte)*(undefined4 *)(iVar2 + 0x24) & 0x1f);
              *(undefined4 *)(iVar2 + 0x28) = 1;
            }
            iVar4 = iVar4 + 4;
            local_c = local_c - 1;
          } while (local_c != 0);
          iVar4 = local_8;
          uVar5 = uVar3;
          do {
            iVar1 = *(int *)(*(int *)((int)_this + 0x14) + *(int *)(iVar4 + param_1[2]) * 4);
            iVar2 = *(int *)(iVar1 + 0x48);
            if (((iVar2 != -1) && (*(int *)(iVar1 + 8) == -1)) &&
               (iVar1 = *(int *)(*(int *)((int)_this + 0x18) + iVar2 * 4),
               *(int *)(iVar1 + 0x28) != 0)) {
              *(undefined4 *)(iVar1 + 0x28) = 0;
              *(uint *)(iVar1 + 0x24) = *(int *)(iVar1 + 0x24) + 1U & 0x1f;
            }
            iVar4 = iVar4 + 4;
            uVar5 = uVar5 - 1;
          } while (uVar5 != 0);
        }
        local_8 = local_8 + uVar3 * 4;
        local_14 = local_14 - 1;
      } while (local_14 != 0);
    }
  }
  return 0;
}


