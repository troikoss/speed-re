// Original UG2 PC v1.2 0x720fd1, 39 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



int FUN_00720fd1(byte *param_1)

{
  int iVar1;
  byte *pbVar2;
  int iVar3;
  
  iVar1 = *(int *)(param_1 + 8);
  if (1 < *param_1) {
    pbVar2 = param_1 + 0xc;
    iVar3 = *param_1 - 1;
    do {
      iVar1 = iVar1 + *(int *)pbVar2;
      pbVar2 = pbVar2 + 4;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  if (*(int *)(param_1 + 4) < iVar1) {
    iVar1 = *(int *)(param_1 + 4);
  }
  return iVar1;
}


