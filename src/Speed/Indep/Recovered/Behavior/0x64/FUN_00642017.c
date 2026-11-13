// Original UG2 PC v1.2 0x642017, 249 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int __stdcall FUN_00642017(int *param_1,int *param_2,int *param_3)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  int iVar9;
  undefined4 *puVar10;
  
  piVar3 = param_1;
  piVar1 = (int *)*param_1;
  iVar7 = piVar1[1];
  iVar5 = *piVar1;
  iVar9 = piVar1[4];
  iVar4 = 0;
  if (iVar9 == 0) {
    iVar9 = 1;
  }
  if (-1 < iVar7) {
    if (iVar7 < 4) {
      uVar6 = piVar1[6] * piVar1[5] * iVar9;
      if (((iVar5 == 1) || (iVar5 == 2)) || (iVar5 == 3)) {
        iVar7 = uVar6 * 4;
        puVar8 = (undefined4 *)*param_3;
        puVar10 = (undefined4 *)*param_2;
        for (uVar6 = uVar6 & 0x3fffffff; uVar6 != 0; uVar6 = uVar6 - 1) {
          *puVar10 = *puVar8;
          puVar8 = puVar8 + 1;
          puVar10 = puVar10 + 1;
        }
        for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
          *(undefined1 *)puVar10 = *(undefined1 *)puVar8;
          puVar8 = (undefined4 *)((int)puVar8 + 1);
          puVar10 = (undefined4 *)((int)puVar10 + 1);
        }
        *param_2 = *param_2 + iVar7;
        *param_3 = *param_3 + iVar7;
        *param_1 = *param_1 + 0x1c;
        return 0;
      }
    }
    else if (iVar7 == 4) {
      switch(iVar5) {
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 0xf:
      case 0x10:
        *param_2 = *param_2 + iVar9 * 4;
        *param_3 = *param_3 + iVar9 * 4;
      case 10:
      case 0xb:
      case 0xc:
      case 0xd:
      case 0xe:
      case 0x11:
      case 0x12:
        *param_1 = *param_1 + 0x14;
        return 0;
      }
    }
    else if (iVar7 == 5) {
      piVar2 = (int *)piVar1[5];
      *param_1 = (int)(piVar1 + 6);
      do {
        if (iVar9 == 0) {
          return iVar4;
        }
        param_1 = (int *)0x0;
        *piVar3 = (int)(piVar1 + 6);
        if (piVar2 != (int *)0x0) {
          do {
            iVar4 = FUN_00642017(piVar3,param_2,param_3);
            if (iVar4 < 0) {
              return iVar4;
            }
            param_1 = (int *)((int)param_1 + 1);
          } while (param_1 < piVar2);
        }
        iVar9 = iVar9 + -1;
      } while( true );
    }
  }
  return -0x7fffbffb;
}


