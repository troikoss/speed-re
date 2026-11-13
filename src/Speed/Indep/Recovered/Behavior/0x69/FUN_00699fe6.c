// Original UG2 PC v1.2 0x699fe6, 79 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __stdcall FUN_00699fe6(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = *(int *)(*(int *)(param_3 + 0x14) + param_2 * 4);
  iVar2 = *(int *)(*(int *)(param_3 + 0x14) + param_1 * 4);
  if (*(uint *)(iVar2 + 4) < *(uint *)(iVar1 + 4)) {
LAB_0069a017:
    uVar3 = 0xffffffff;
  }
  else {
    if (*(uint *)(iVar2 + 4) <= *(uint *)(iVar1 + 4)) {
      if (*(double *)(iVar2 + 0x20) < *(double *)(iVar1 + 0x20)) goto LAB_0069a017;
      if (*(double *)(iVar2 + 0x20) <= *(double *)(iVar1 + 0x20)) {
        return 0;
      }
    }
    uVar3 = 1;
  }
  return uVar3;
}


