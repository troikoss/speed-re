// Original UG2 PC v1.2 0x69d5eb, 106 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



undefined4 __fastcall FUN_0069d5eb(void *_this,int _edx_unused,int param_1,int param_2,uint param_3,uint param_4)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  
  iVar1 = *(int *)(*(int *)((int)_this + 0x14) + param_1 * 4);
  uVar2 = *(uint *)(iVar1 + 0x48);
  if (uVar2 == 0xffffffff) {
LAB_0069d649:
    uVar4 = 1;
  }
  else {
    if ((param_3 <= uVar2) && (uVar2 < param_4)) {
      iVar3 = *(int *)(*(int *)((int)_this + 0x14) + **(int **)(param_2 + 0x10) * 4);
      iVar5 = *(int *)(iVar3 + 0x18);
      iVar3 = *(int *)(iVar3 + 0x14);
      if (iVar3 != -1) {
        do {
          if (iVar3 == *(int *)(iVar1 + 0x14)) break;
          iVar3 = *(int *)(*(int *)((int)_this + 0x14) + iVar3 * 4);
          iVar5 = *(int *)(iVar3 + 0x18);
          iVar3 = *(int *)(iVar3 + 0x14);
        } while (iVar3 != -1);
      }
      if ((iVar3 == *(int *)(iVar1 + 0x14)) && (iVar5 == *(int *)(iVar1 + 0x18))) goto LAB_0069d649;
    }
    uVar4 = 0;
  }
  return uVar4;
}


