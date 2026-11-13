// Original UG2 PC v1.2 0x5938c0, 118 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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

#define iRam00000020 (*(int *)0x00000020)

int __fastcall FUN_005938c0(int param_1)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 100) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  iVar1 = 0;
  if (*(int *)(param_1 + 0x5c) != 0) {
    iVar1 = *(int *)(param_1 + 0x130);
    if (*(int *)(iVar1 + 0x28) == 1) {
      if (*(int *)(iVar1 + 0x34) == 0) {
        iVar1 = 0;
      }
      else {
        iVar1 = *(int *)(iVar1 + 0x34) + 8;
      }
      *(int *)(param_1 + 100) = iVar1;
      if (iVar1 != 0) {
        *(undefined4 *)(param_1 + 0x68) = *(undefined4 *)(iVar1 + 0x18);
        return iVar1 + -8;
      }
      iVar1 = 0;
      *(int *)(param_1 + 0x68) = iRam00000020;
    }
    else if (*(int *)(iVar1 + 0x28) == 3) {
      if (*(int *)(iVar1 + 0x34) == 0) {
        iVar1 = 0;
      }
      else {
        iVar1 = *(int *)(iVar1 + 0x34) + 8;
      }
      *(int *)(param_1 + 100) = iVar1;
      if (iVar1 == 0) {
        iVar1 = iRam00000020;
        *(int *)(param_1 + 0x68) = iRam00000020;
        return iVar1;
      }
      iVar1 = *(int *)(iVar1 + 0x18);
      *(int *)(param_1 + 0x68) = iVar1;
      return iVar1;
    }
  }
  return iVar1;
}


