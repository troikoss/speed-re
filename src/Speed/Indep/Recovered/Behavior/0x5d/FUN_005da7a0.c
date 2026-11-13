// Original UG2 PC v1.2 0x5da7a0, 225 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 __fastcall FUN_005da7a0(void *_this,int _edx_unused,int *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  
  iVar4 = *param_1;
  if (iVar4 == 0x34110) {
    *(int **)_this = param_1 + 2;
    *(uint *)((int)_this + 4) = (uint)param_1[1] / 0x50;
    return 1;
  }
  if (iVar4 != 0x34113) {
    if (iVar4 == 0x34111) {
      *(int **)((int)_this + 0x10) = param_1 + 2;
      return 1;
    }
    if (iVar4 == 0x34112) {
      *(int **)((int)_this + 0x18) = param_1 + 2;
      *(uint *)((int)_this + 0x14) = (uint)param_1[1] >> 4;
      return 1;
    }
    return 0;
  }
  piVar3 = param_1 + 2;
  *(int **)((int)_this + 8) = piVar3;
  piVar5 = (int *)(param_1[1] + 8 + (int)param_1);
  *(int **)((int)_this + 0xc) = piVar5;
  if (piVar3 < piVar5) {
    do {
      iVar4 = 0;
      if ('\0' < *(char *)((int)piVar3 + 0x13)) {
        piVar5 = piVar3 + 5;
        do {
          iVar2 = 0;
          if (0 < *(int *)((int)_this + 4)) {
            iVar1 = *(int *)_this;
            do {
              if (*(short *)(iVar1 + 8) == (short)*piVar5) goto LAB_005da821;
              iVar2 = iVar2 + 1;
              iVar1 = iVar1 + 0x50;
            } while (iVar2 < *(int *)((int)_this + 4));
          }
          iVar1 = 0;
LAB_005da821:
          piVar5[1] = iVar1;
          iVar4 = iVar4 + 1;
          piVar5 = piVar5 + 2;
        } while (iVar4 < *(char *)((int)piVar3 + 0x13));
      }
      piVar3 = piVar3 + *(char *)((int)piVar3 + 0x13) * 2 + 5;
    } while (piVar3 < *(int **)((int)_this + 0xc));
  }
  return 1;
}


