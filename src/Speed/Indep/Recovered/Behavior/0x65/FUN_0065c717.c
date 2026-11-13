// Original UG2 PC v1.2 0x65c717, 163 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



uint __stdcall FUN_0065c717(int param_1,byte *param_2)

{
  byte bVar1;
  uint uVar2;
  int *piVar3;
  byte *pbVar4;
  byte *pbVar5;
  int *piVar6;
  bool bVar7;
  uint local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0xa0) != 0) {
    piVar6 = (int *)(*(int *)(param_1 + 0xa4) + 0x28);
    do {
      piVar3 = (int *)(*piVar6 + *(int *)(param_1 + 0x20));
      if ((*piVar3 == 0) || (pbVar4 = (byte *)(piVar3 + 1), pbVar4 == (byte *)0x0)) {
        pbVar4 = param_2;
LAB_0065c780:
        if (pbVar4 == (byte *)0x0) break;
      }
      else {
        pbVar5 = param_2;
        if (param_2 != (byte *)0x0) {
          do {
            bVar1 = *pbVar4;
            bVar7 = bVar1 < *pbVar5;
            if (bVar1 != *pbVar5) {
LAB_0065c773:
              pbVar4 = (byte *)((1 - (uint)bVar7) - (uint)(bVar7 != 0));
              goto LAB_0065c780;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar4[1];
            bVar7 = bVar1 < pbVar5[1];
            if (bVar1 != pbVar5[1]) goto LAB_0065c773;
            pbVar4 = pbVar4 + 2;
            pbVar5 = pbVar5 + 2;
          } while (bVar1 != 0);
          pbVar4 = (byte *)0x0;
          goto LAB_0065c780;
        }
      }
      local_8 = local_8 + 1;
      piVar6 = piVar6 + 0x2d;
    } while (local_8 < *(uint *)(param_1 + 0xa0));
  }
  if (*(uint *)(param_1 + 0xa0) == local_8) {
    uVar2 = 0;
  }
  else {
    uVar2 = ~(local_8 * 0xb4 + *(int *)(param_1 + 0xa4));
  }
  return uVar2;
}


