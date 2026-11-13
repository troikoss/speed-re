// Original UG2 PC v1.2 0x6c94c2, 210 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



undefined4 __fastcall FUN_006c94c2(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int *local_10;
  int *local_c;
  uint local_8;
  
  uVar5 = **(uint **)(param_1 + 0x100) & 0xfffff;
  local_8 = 0;
  if (uVar5 != 0) {
    uVar1 = (*(uint **)(param_1 + 0x100))[2];
    local_10 = (int *)(uVar1 + uVar5 * 4);
    local_c = (int *)(uVar1 + uVar5 * 8);
    do {
      iVar2 = *(int *)(uVar1 + local_8 * 4);
      iVar3 = *(int *)(*(int *)(param_1 + 0x14) + iVar2 * 4);
      iVar4 = *(int *)(*(int *)(param_1 + 0x14) + *local_c * 4);
      if ((iVar2 != *local_10) ||
         (((iVar2 = *(int *)(iVar4 + 0x38),
           iVar2 != *(int *)(local_8 * 4 + *(int *)(*(int *)(param_1 + 0x100) + 8)) &&
           ((iVar2 == -1 || (iVar2 != *(int *)(iVar3 + 0x38))))) ||
          ((*(uint *)(iVar4 + 0x3c) ^ 0x80000) != *(uint *)(iVar3 + 0x3c))))) {
        return 1;
      }
      local_8 = local_8 + 1;
      local_c = local_c + 1;
      local_10 = local_10 + 1;
    } while (local_8 < uVar5);
  }
  **(uint **)(param_1 + 0x100) = uVar5 | 0x70000000;
  *(uint *)(*(int *)(param_1 + 0x100) + 4) = uVar5;
  return 0;
}


