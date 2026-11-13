// Original UG2 PC v1.2 0x674d33, 406 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



bool __fastcall FUN_00674d33(void *_this,int _edx_unused,int *param_1,int *param_2,uint param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  bool bVar4;
  uint local_c;
  int *local_8;
  
  if (param_2 == (int *)0x2) {
    bVar4 = false;
    if (param_3 * param_4 != 0) {
      local_c = 0;
      if (param_3 != 0) {
        piVar3 = param_1;
        do {
          iVar1 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) + *piVar3 * 4);
          param_2 = (int *)0x0;
          if (param_4 != 0) {
            local_8 = piVar3;
            do {
              iVar2 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) + *local_8 * 4);
              if (*(int *)(iVar1 + 4) != *(int *)(iVar2 + 4)) goto LAB_00674ebb;
              if ((*(byte *)(*(int *)(*(int *)(*(int *)((int)_this + 8) + 0x10) +
                                     *(int *)(iVar1 + 4) * 4) + 4) & 0x40) == 0) {
                bVar4 = *(int *)(iVar1 + 0x48) == *(int *)(iVar2 + 0x48);
              }
              else {
                if (((uint)param_2 >> 2) + *(int *)(iVar1 + 0xc) != *(int *)(iVar2 + 0xc))
                goto LAB_00674ebb;
                bVar4 = ((uint)param_2 & 3) + *(int *)(iVar1 + 0x10) == *(int *)(iVar2 + 0x10);
              }
              if (!bVar4) goto LAB_00674ebb;
              param_2 = (int *)((int)param_2 + 1);
              local_8 = local_8 + 1;
            } while (param_2 < param_4);
          }
          local_c = local_c + 1;
          piVar3 = piVar3 + param_4;
        } while (local_c < param_3);
      }
      local_c = 0;
      if (param_4 != 0) {
        piVar3 = param_1;
        do {
          iVar1 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) + *piVar3 * 4);
          param_1 = (int *)0x0;
          if (param_3 != 0) {
            param_2 = piVar3;
            do {
              iVar2 = *(int *)(*(int *)(*(int *)((int)_this + 8) + 0x14) + *param_2 * 4);
              if (*(int *)(iVar1 + 4) != *(int *)(iVar2 + 4)) {
                return false;
              }
              if ((*(byte *)(*(int *)(*(int *)(*(int *)((int)_this + 8) + 0x10) +
                                     *(int *)(iVar1 + 4) * 4) + 4) & 0x40) == 0) {
                bVar4 = *(int *)(iVar1 + 0x48) == *(int *)(iVar2 + 0x48);
              }
              else {
                if (((uint)param_1 >> 2) + *(int *)(iVar1 + 0xc) != *(int *)(iVar2 + 0xc)) {
                  return false;
                }
                bVar4 = ((uint)param_1 & 3) + *(int *)(iVar1 + 0x10) == *(int *)(iVar2 + 0x10);
              }
              if (!bVar4) {
                return false;
              }
              param_1 = (int *)((int)param_1 + 1);
              param_2 = param_2 + param_4;
            } while (param_1 < param_3);
          }
          local_c = local_c + 1;
          piVar3 = piVar3 + 1;
        } while (local_c < param_4);
      }
LAB_00674ebb:
      bVar4 = true;
    }
  }
  else {
    bVar4 = 1 < param_3;
  }
  return bVar4;
}


