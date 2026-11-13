// Original UG2 PC v1.2 0x72a1e9, 64 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void FUN_0072a1e9(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = *param_1;
  iVar3 = 0;
  if (iVar2 != 0) {
    do {
      iVar1 = iVar2;
      iVar2 = iVar1;
      if (*(ushort *)(param_2 + 0x18) <= *(ushort *)(iVar1 + 0x18)) break;
      iVar2 = *(int *)(iVar1 + 8);
      iVar3 = iVar1;
    } while (iVar2 != 0);
  }
  *(int *)(param_2 + 8) = iVar2;
  if (iVar2 != 0) {
    *(int *)(iVar2 + 0x10) = param_2;
  }
  if (iVar3 == 0) {
    *param_1 = param_2;
    return;
  }
  *(int *)(iVar3 + 8) = param_2;
  *(int *)(param_2 + 0x10) = iVar3;
  return;
}


