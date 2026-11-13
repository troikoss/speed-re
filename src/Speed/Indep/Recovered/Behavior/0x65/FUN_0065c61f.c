// Original UG2 PC v1.2 0x65c61f, 207 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



int __fastcall FUN_0065c61f(void *_this, int _edx_unused, byte *param_1, int _pad0)

{
  byte bVar1;
  int iVar2;
  int *piVar3;
  byte *pbVar4;
  byte *pbVar5;
  int *piVar6;
  bool bVar7;
  uint local_8;
  
  if ((byte *)((uint)param_1 & 0xffff) == param_1) {
LAB_0065c6e8:
    iVar2 = 0;
  }
  else {
    if ((int)param_1 < 1) {
      if ((*(int *)(~(uint)param_1 + 0x10) != 0) ||
         (local_8 = (uint)*(ushort *)(~(uint)param_1 + 0x14), *(uint *)((int)_this + 0xa0) <= local_8
         )) goto LAB_0065c6e8;
    }
    else {
      local_8 = 0;
      if (*(int *)((int)_this + 0xa0) != 0) {
        piVar6 = (int *)(*(int *)((int)_this + 0xa4) + 0x28);
        do {
          piVar3 = (int *)(*(int *)((int)_this + 0x20) + *piVar6);
          pbVar5 = param_1;
          if ((*piVar3 == 0) || (pbVar4 = (byte *)(piVar3 + 1), pbVar4 == (byte *)0x0)) {
joined_r0x0065c6a2:
            if (pbVar5 == (byte *)0x0) break;
          }
          else if (param_1 != (byte *)0x0) {
            do {
              bVar1 = *pbVar4;
              bVar7 = bVar1 < *pbVar5;
              if (bVar1 != *pbVar5) {
LAB_0065c692:
                pbVar5 = (byte *)((1 - (uint)bVar7) - (uint)(bVar7 != 0));
                goto joined_r0x0065c6a2;
              }
              if (bVar1 == 0) break;
              bVar1 = pbVar4[1];
              bVar7 = bVar1 < pbVar5[1];
              if (bVar1 != pbVar5[1]) goto LAB_0065c692;
              pbVar4 = pbVar4 + 2;
              pbVar5 = pbVar5 + 2;
            } while (bVar1 != 0);
            pbVar5 = (byte *)0x0;
            goto joined_r0x0065c6a2;
          }
          local_8 = local_8 + 1;
          piVar6 = piVar6 + 0x2d;
        } while (local_8 < *(uint *)((int)_this + 0xa0));
      }
      if (*(uint *)((int)_this + 0xa0) == local_8) goto LAB_0065c6e8;
    }
    iVar2 = local_8 * 0xb4 + *(int *)((int)_this + 0xa4);
  }
  return iVar2;
}


