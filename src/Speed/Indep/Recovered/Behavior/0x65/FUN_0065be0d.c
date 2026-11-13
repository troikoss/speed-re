// Original UG2 PC v1.2 0x65be0d, 267 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



uint __fastcall FUN_0065be0d(void *_this, int _edx_unused, byte *param_1, int _pad0)

{
  byte bVar1;
  int iVar2;
  int *piVar3;
  byte *pbVar4;
  int *piVar5;
  byte *pbVar6;
  uint uVar7;
  bool bVar8;
  uint local_8;
  
  if ((byte *)((uint)param_1 & 0xffff) != param_1) {
    if ((int)param_1 < 1) {
      uVar7 = ~(uint)param_1;
      if ((((*(int *)(uVar7 + 0x10) == 3) && (*(uint *)(uVar7 + 0x14) < *(uint *)((int)_this + 0xa8))
           ) && (*(int *)(uVar7 + 0x18) == 0)) &&
         ((*(int *)(uVar7 + 0x1c) == 0 && (*(int *)(uVar7 + 8) == -1)))) {
        uVar7 = *(uint *)(*(int *)((int)_this + 0xb0) + *(uint *)(uVar7 + 0x14) * 4);
        return ~-(uint)((*(byte *)(uVar7 + 0x28) & 4) != 0) & uVar7;
      }
    }
    else {
      local_8 = 0;
      if (*(int *)((int)_this + 0x98) != 0) {
        piVar5 = (int *)(*(int *)((int)_this + 0x9c) + 0x2c);
        do {
          if (piVar5[5] == 0) {
            iVar2 = *(int *)((int)_this + 0x20);
          }
          else {
            iVar2 = *(int *)(*(int *)((int)_this + 0x38) + 0x10);
          }
          piVar3 = (int *)(iVar2 + *(int *)(iVar2 + 8 + *piVar5));
          pbVar6 = param_1;
          if ((*piVar3 == 0) || (pbVar4 = (byte *)(piVar3 + 1), pbVar4 == (byte *)0x0)) {
joined_r0x0065bea6:
            if (pbVar6 == (byte *)0x0) break;
          }
          else if (param_1 != (byte *)0x0) {
            do {
              bVar1 = *pbVar4;
              bVar8 = bVar1 < *pbVar6;
              if (bVar1 != *pbVar6) {
LAB_0065be94:
                pbVar6 = (byte *)((1 - (uint)bVar8) - (uint)(bVar8 != 0));
                goto joined_r0x0065bea6;
              }
              if (bVar1 == 0) break;
              bVar1 = pbVar4[1];
              bVar8 = bVar1 < pbVar6[1];
              if (bVar1 != pbVar6[1]) goto LAB_0065be94;
              pbVar4 = pbVar4 + 2;
              pbVar6 = pbVar6 + 2;
            } while (bVar1 != 0);
            pbVar6 = (byte *)0x0;
            goto joined_r0x0065bea6;
          }
          local_8 = local_8 + 1;
          piVar5 = piVar5 + 0x14;
        } while (local_8 < *(uint *)((int)_this + 0x98));
      }
      if (local_8 != *(uint *)((int)_this + 0x98)) {
        return local_8 * 0x50 + *(int *)((int)_this + 0x9c);
      }
    }
  }
  return 0;
}


