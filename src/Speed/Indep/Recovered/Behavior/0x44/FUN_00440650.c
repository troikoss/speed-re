// Original UG2 PC v1.2 0x440650, 337 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int ptr;

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
int __cdecl FUN_00440540();


void __fastcall FUN_00440650(int param_1)

{
  int *piVar1;
  void *pvVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  void *pvVar5;
  undefined4 *puVar6;
  void *ptr;
  uint uVar7;
  int iVar8;
  void *pvVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  
  iVar12 = *(int *)(param_1 + 8);
  if (iVar12 != 0) {
    do {
      *(undefined4 *)(iVar12 + 0x18) = *(undefined4 *)(iVar12 + 0x24);
      iVar12 = *(int *)(iVar12 + 8);
    } while (iVar12 != 0);
    piVar10 = *(int **)(param_1 + 0x10);
    if (piVar10 != (int *)0x0) {
      do {
        for (iVar12 = *(int *)(param_1 + 8); iVar12 != 0; iVar12 = *(int *)(iVar12 + 8)) {
          iVar11 = 0;
          if (iVar12 != 0) {
            iVar8 = iVar12;
            do {
              uVar7 = (uint)((int)piVar10 + (-0x30 - iVar8)) / *(uint *)(iVar12 + 0x28);
              if ((-1 < (int)uVar7) && ((int)uVar7 < *(int *)(iVar8 + 0x24))) {
                if ((-1 < (int)(uVar7 + iVar11)) &&
                   ((int)(uVar7 + iVar11) < *(int *)(iVar12 + 0x24))) {
                  *(int *)(iVar12 + 0x18) = *(int *)(iVar12 + 0x18) + -1;
                  goto LAB_004406cb;
                }
                break;
              }
              piVar1 = (int *)(iVar8 + 0x24);
              iVar8 = *(int *)(iVar8 + 8);
              iVar11 = iVar11 + *piVar1;
            } while (iVar8 != 0);
          }
        }
LAB_004406cb:
        piVar10 = (int *)*piVar10;
      } while (piVar10 != (int *)0x0);
    }
    pvVar2 = *(void **)(param_1 + 8);
    ptr = (void *)param_1;
    while( true ) {
      pvVar5 = ptr;
      ptr = pvVar2;
      if (ptr == (void *)0x0) break;
      pvVar2 = *(void **)((int)ptr + 8);
      if (*(int *)((int)ptr + 0x18) == 0) {
        puVar3 = *(undefined4 **)(param_1 + 0x10);
        puVar6 = (undefined4 *)(param_1 + 0x10);
joined_r0x0044070e:
        puVar4 = puVar6;
        puVar6 = puVar3;
        if (puVar6 != (undefined4 *)0x0) {
          puVar3 = (undefined4 *)*puVar6;
          iVar12 = 0;
          pvVar9 = ptr;
          do {
            uVar7 = (uint)((int)puVar6 + (-0x30 - (int)pvVar9)) / *(uint *)((int)ptr + 0x28);
            if ((-1 < (int)uVar7) && ((int)uVar7 < *(int *)((int)pvVar9 + 0x24))) {
              if (-1 < (int)(uVar7 + iVar12)) {
                if ((int)(uVar7 + iVar12) < *(int *)((int)ptr + 0x24)) {
                  *puVar4 = puVar3;
                  puVar6 = puVar4;
                }
              }
              break;
            }
            piVar10 = (int *)((int)pvVar9 + 0x24);
            pvVar9 = *(void **)((int)pvVar9 + 8);
            iVar12 = iVar12 + *piVar10;
          } while (pvVar9 != (void *)0x0);
          goto joined_r0x0044070e;
        }
        *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) - *(int *)((int)ptr + 0x24);
        *(undefined4 *)((int)pvVar5 + 8) = *(undefined4 *)((int)ptr + 8);
        FUN_00440540(ptr);
        ptr = pvVar5;
      }
    }
  }
  return;
}


