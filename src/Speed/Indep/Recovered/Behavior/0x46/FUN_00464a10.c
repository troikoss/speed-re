// Original UG2 PC v1.2 0x464a10, 184 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



void __fastcall FUN_00464a10(int param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int local_108 [65];
  
  piVar4 = local_108;
  for (iVar3 = 0x40; iVar3 != 0; iVar3 = iVar3 + -1) {
    *piVar4 = 0;
    piVar4 = piVar4 + 1;
  }
  iVar3 = *(int *)(param_1 + 0x1c);
  if (iVar3 != 0) {
    piVar4 = local_108;
    do {
      *piVar4 = iVar3;
      iVar3 = *(int *)(iVar3 + 0x50);
      piVar4 = piVar4 + 1;
    } while (iVar3 != 0);
  }
  *(undefined4 *)(param_1 + 0x1c) = 0;
  iVar3 = 0;
  while( true ) {
    bVar1 = false;
    iVar7 = 0;
    iVar5 = 0;
    uVar6 = 0x40;
    if (*(int *)(param_1 + 0x2c) < 1) {
      return;
    }
    do {
      if (local_108[iVar5] != 0) {
        uVar2 = *(int *)(local_108[iVar5] + 0x4c) >> 4 & 0x7f;
        bVar1 = true;
        if (uVar2 < uVar6) {
          uVar6 = uVar2;
          iVar7 = iVar5;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < *(int *)(param_1 + 0x2c));
    if (!bVar1) break;
    iVar5 = local_108[iVar7];
    if (*(int *)(param_1 + 0x1c) == 0) {
      *(int *)(param_1 + 0x1c) = iVar5;
      *(undefined4 *)(iVar5 + 0x50) = 0;
      local_108[iVar7] = 0;
      iVar3 = iVar5;
    }
    else {
      *(int *)(iVar3 + 0x50) = iVar5;
      *(undefined4 *)(iVar5 + 0x50) = 0;
      local_108[iVar7] = 0;
      iVar3 = iVar5;
    }
  }
  return;
}


