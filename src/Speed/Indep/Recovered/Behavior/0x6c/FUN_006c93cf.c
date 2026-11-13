// Original UG2 PC v1.2 0x6c93cf, 243 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



undefined4 __fastcall FUN_006c93cf(int param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  uint local_10;
  int *local_c;
  int *local_8;
  
  uVar2 = **(uint **)(param_1 + 0x100) & 0xfffff;
  local_10 = 0;
  if (uVar2 != 0) {
    local_8 = *(int **)(*(int *)(param_1 + 0x100) + 8);
    local_c = local_8 + uVar2;
    do {
      iVar3 = *(int *)(*(int *)(param_1 + 0x14) + *local_8 * 4);
      iVar1 = *(int *)(*(int *)(param_1 + 0x14) + *local_c * 4);
      if ((((*(int *)(iVar3 + 4) != *(int *)(iVar1 + 4)) ||
           (*(int *)(iVar3 + 8) != *(int *)(iVar1 + 8))) ||
          (*(int *)(iVar3 + 0xc) != *(int *)(iVar1 + 0xc))) ||
         ((*(int *)(iVar3 + 0x10) != *(int *)(iVar1 + 0x10) ||
          ((*(uint *)(iVar1 + 0x3c) ^ *(uint *)(iVar3 + 0x3c)) != 0x80000)))) {
        return 1;
      }
      local_10 = local_10 + 1;
      local_8 = local_8 + 1;
      local_c = local_c + 1;
    } while (local_10 < uVar2);
  }
  **(uint **)(param_1 + 0x100) = uVar2 | 0x70000000;
  *(uint *)(*(int *)(param_1 + 0x100) + 4) = uVar2;
  if ((uVar2 != 0) &&
     ((*(byte *)(*(int *)(*(int *)(param_1 + 0x14) + **(int **)(*(int *)(param_1 + 0x100) + 8) * 4)
                + 0x3e) & 8) != 0)) {
    puVar5 = *(undefined4 **)(*(int *)(param_1 + 0x100) + 8);
    puVar4 = puVar5 + uVar2;
    for (; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar5 = *puVar4;
      puVar4 = puVar4 + 1;
      puVar5 = puVar5 + 1;
    }
    for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(undefined1 *)puVar5 = *(undefined1 *)puVar4;
      puVar4 = (undefined4 *)((int)puVar4 + 1);
      puVar5 = (undefined4 *)((int)puVar5 + 1);
    }
  }
  return 0;
}


