// Original UG2 PC v1.2 0x6496e6, 150 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



uint __fastcall FUN_006496e6(void *_this,int _edx_unused,byte *param_1)

{
  byte bVar1;
  uint uVar2;
  byte *pbVar3;
  int iVar4;
  int *piVar5;
  byte *pbVar6;
  bool bVar7;
  
  if ((byte *)((uint)param_1 & 0x7fffffff) == param_1) {
    for (uVar2 = *(uint *)((int)_this + 0x34c); uVar2 != 0; uVar2 = *(uint *)(uVar2 + 0x34)) {
      if (param_1 == (byte *)0x0) {
        iVar4 = *(int *)(uVar2 + 0x20);
LAB_00649766:
        if (iVar4 == 0) {
          return uVar2;
        }
      }
      else {
        pbVar6 = *(byte **)(uVar2 + 0x20);
        if (pbVar6 != (byte *)0x0) {
          pbVar3 = param_1;
          do {
            bVar1 = *pbVar3;
            bVar7 = bVar1 < *pbVar6;
            if (bVar1 != *pbVar6) {
LAB_0064975f:
              iVar4 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
              goto LAB_00649766;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar3[1];
            bVar7 = bVar1 < pbVar6[1];
            if (bVar1 != pbVar6[1]) goto LAB_0064975f;
            pbVar3 = pbVar3 + 2;
            pbVar6 = pbVar6 + 2;
          } while (bVar1 != 0);
          iVar4 = 0;
          goto LAB_00649766;
        }
      }
    }
    uVar2 = 0;
  }
  else {
    piVar5 = (int *)~(uint)param_1;
    if (((*piVar5 == 3) && (piVar5[6] == 0)) && (piVar5[4] == -1)) {
      uVar2 = -(uint)(*(int *)(piVar5[1] + 0x44) != 0) & (uint)piVar5;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}


