// Original UG2 PC v1.2 0x673caa, 323 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_00671ccb();
#define DAT_008a5d00 (*(undefined4 *)0x008a5d00)

undefined4 __fastcall FUN_00673caa(void *_this,int _edx_unused,uint param_1)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  uVar3 = param_1;
  uVar7 = 0;
  if (((*(int *)(param_1 + 0x14) == 0) || (*(int *)(param_1 + 0x28) == 0)) ||
     (*(int *)(param_1 + 0x38) != 0)) {
LAB_00673de4:
    uVar5 = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0x34) = 0;
    if (*(int *)(param_1 + 0x14) != 0) {
      iVar6 = *(int *)(param_1 + 0x10) << 2;
      do {
        uVar4 = *(int *)(*(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) +
                                 *(int *)(iVar6 + *(int *)((int)_this + 0x18)) * 4) + 0x70) + 1;
        if (*(uint *)(param_1 + 0x34) < uVar4) {
          *(uint *)(param_1 + 0x34) = uVar4;
        }
        uVar7 = uVar7 + 1;
        iVar6 = iVar6 + 4;
      } while (uVar7 < *(uint *)(param_1 + 0x14));
    }
    iVar6 = FUN_00671ccb(DAT_008a5d00,*(int *)(param_1 + 0x34) << 2,0x10);
    *(int *)(param_1 + 0x38) = iVar6;
    if (iVar6 != 0) {
      iVar6 = FUN_00671ccb(DAT_008a5d00,*(int *)(param_1 + 0x14) << 2,0x10);
      *(int *)(param_1 + 0x3c) = iVar6;
      if (iVar6 != 0) {
        puVar1 = (uint *)(param_1 + 0x34);
        puVar8 = (undefined4 *)(param_1 + 0x38);
        param_1 = 0;
        puVar8 = (undefined4 *)*puVar8;
        for (uVar7 = *puVar1 & 0x3fffffff; uVar7 != 0; uVar7 = uVar7 - 1) {
          *puVar8 = 0;
          puVar8 = puVar8 + 1;
        }
        for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
          *(undefined1 *)puVar8 = 0;
          puVar8 = (undefined4 *)((int)puVar8 + 1);
        }
        puVar8 = *(undefined4 **)(uVar3 + 0x3c);
        for (uVar7 = *(uint *)(uVar3 + 0x14) & 0x3fffffff; uVar7 != 0; uVar7 = uVar7 - 1) {
          *puVar8 = 0;
          puVar8 = puVar8 + 1;
        }
        for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
          *(undefined1 *)puVar8 = 0;
          puVar8 = (undefined4 *)((int)puVar8 + 1);
        }
        iVar6 = *(int *)(*(int *)(uVar3 + 0x28) + 0x20);
        if (*(int *)(uVar3 + 0x14) != 0) {
          do {
            if (iVar6 == 0) break;
            iVar2 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) +
                            *(int *)(*(int *)((int)_this + 0x18) +
                                    (*(int *)(uVar3 + 0x10) + param_1) * 4) * 4);
            if ((*(byte *)(*(int *)(*(int *)(*(int *)((int)_this + 8) + 0x10) +
                                   *(int *)(iVar2 + 4) * 4) + 4) & 0x80) == 0) {
              *(int *)(*(int *)(uVar3 + 0x38) + *(int *)(iVar2 + 0x70) * 4) =
                   *(int *)(iVar6 + 8) + 0x10;
              *(int *)(*(int *)(uVar3 + 0x3c) + *(int *)(iVar2 + 100) * 4) =
                   *(int *)(iVar6 + 8) + 0x10;
            }
            param_1 = param_1 + 1;
            iVar6 = *(int *)(iVar6 + 0xc);
          } while (param_1 < *(uint *)(uVar3 + 0x14));
        }
        goto LAB_00673de4;
      }
    }
    uVar5 = 0x8007000e;
  }
  return uVar5;
}


