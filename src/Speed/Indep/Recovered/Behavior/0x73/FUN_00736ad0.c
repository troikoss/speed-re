// Original UG2 PC v1.2 0x736ad0, 399 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00736350();
int __cdecl FUN_00736700();
int __cdecl FUN_007389e0();
#define _DAT_008b4f80 (*(undefined4 *)0x008b4f80)
#define DAT_008b504f (*(undefined4 *)0x008b504f)
#define DAT_008b4f60 (*(undefined4 *)0x008b4f60)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00736ad0(int *param_1,uint param_2)

{
  ushort uVar1;
  ushort uVar2;
  short *psVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  ushort uVar7;
  int iVar8;
  int iVar9;
  short *psVar10;
  uint uVar11;
  byte *pbVar12;
  int local_28;
  uint local_24;
  int local_1c;
  undefined4 local_8;
  int *local_4;
  
  iVar8 = FUN_007389e0();
  iVar8 = iVar8 + _DAT_008b4f80;
  uVar7 = 0;
  local_1c = -1;
  local_24 = 0xffffffff;
  uVar2 = 0;
  local_28 = 0;
  pbVar12 = &DAT_008b504f;
  do {
    iVar9 = 0;
    if ((pbVar12[-1] != 0) && (*pbVar12 == param_2)) {
      psVar3 = *(short **)(pbVar12 + 1);
      bVar6 = false;
      local_8 = 0;
      bVar4 = true;
      if (param_1 != (int *)0x0) {
        bVar4 = false;
        if (0 < *param_1) {
          psVar10 = (short *)param_1[1];
          do {
            if (*psVar10 == *psVar3) {
              bVar4 = true;
              break;
            }
            iVar9 = iVar9 + 1;
            psVar10 = psVar10 + 1;
          } while (iVar9 < *param_1);
        }
      }
      uVar11 = iVar8 - *(int *)(pbVar12 + -7);
      if ((psVar3[1] != 0) && ((ushort)psVar3[1] < uVar11)) {
        bVar6 = true;
      }
      if ((*(int *)(&DAT_008b4f60 + param_2 * 4) == 1) && ((*(byte *)(psVar3 + 5) & 2) != 0)) {
        local_8 = **(undefined4 **)(pbVar12 + 5);
        bVar5 = false;
        iVar9 = FUN_00736350((int)&local_8,&local_4);
        if ((iVar9 != 0) && ((ushort)psVar3[2] < *(ushort *)(*local_4 + 0x16))) {
          bVar5 = true;
        }
      }
      else {
        bVar5 = false;
      }
      if (bVar6) {
        FUN_00736700(local_28);
      }
      else if ((bVar4) && (!bVar5)) {
        uVar1 = psVar3[2];
        if ((uVar7 < uVar1) ||
           ((uVar1 == uVar7 &&
            ((uVar1 = uVar7, uVar11 < local_24 ||
             ((uVar11 == local_24 && (uVar2 < *(ushort *)(pbVar12 + -3))))))))) {
          uVar2 = *(ushort *)(pbVar12 + -3);
          local_1c = local_28;
          local_24 = uVar11;
          uVar7 = uVar1;
        }
      }
    }
    local_28 = local_28 + 1;
    pbVar12 = pbVar12 + 0x10;
    if (0x8b514e < (int)pbVar12) {
      return local_1c;
    }
  } while( true );
}


